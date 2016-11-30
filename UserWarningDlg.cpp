// UserWarningDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "UserWarningDlg.h"
#include "GlobalUtils.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CUserWarningDlg dialog


CUserWarningDlg::CUserWarningDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUserWarningDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUserWarningDlg)
	m_strWarningMsg = _T("");
	m_bKeepWarning = FALSE;
	m_strTitleLabel = _T("");
	m_colorWarning = 0;
	m_nWarningCategoryID = -1;
	//}}AFX_DATA_INIT
}


void CUserWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserWarningDlg)
	DDX_Control(pDX, IDC_KEEP_WARNING_CHECK, m_chkKeepWarning);
	DDX_Text(pDX, IDC_WARNING_MSG, m_strWarningMsg);
	DDX_Check(pDX, IDC_KEEP_WARNING_CHECK, m_bKeepWarning);
	DDX_Text(pDX, IDC_TITLE_LABEL, m_strTitleLabel);
	DDX_Control(pDX, IDC_WARNING_MSG, m_nxeditWarningMsg);
	DDX_Control(pDX, IDC_TITLE_LABEL, m_nxstaticTitleLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUserWarningDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUserWarningDlg)
	ON_BN_CLICKED(IDC_KEEP_WARNING_CHECK, OnKeepWarningCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUserWarningDlg message handlers

// (a.walling 2010-07-01 16:15) - PLID 18081 - Warning categories - backgroundColor parameter (0 for default)
BOOL CUserWarningDlg::DoModalWarning(const CString& strText, BOOL bKeepWarning, const CString& strTitle, const CString& strCaption, COLORREF backgroundColor, int nWarningCategoryID, const CString& strWarningCategoryName)
{	
	// Set appearance of the pop-up window
	m_strWarningMsg = strText;
	m_bKeepWarning = bKeepWarning;
	m_strCaption = strCaption;
	// (a.walling 2010-07-02 08:19) - PLID 18081 - Warning categories - Set the color
	m_colorWarning = backgroundColor;	
	m_nWarningCategoryID = nWarningCategoryID;
	m_strWarningCategoryName = strWarningCategoryName;
	if (!strTitle.IsEmpty()) m_strTitleLabel = strTitle;

	if (m_nWarningCategoryID == -1) {
		if (m_colorWarning == 0) {
			// (a.walling 2010-08-05 09:31) - PLID 39514 - Get the default category warning color if necessary
			m_colorWarning = GetRemotePropertyInt("DefaultWarningCategoryColor", 0, 0, "<None>", true);		
		}
		m_strWarningCategoryName = GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true);
	}

	if (!m_strWarningCategoryName.IsEmpty()) {
		if (!m_strCaption.IsEmpty()) {
			m_strCaption += " - ";
		}
		m_strCaption += m_strWarningCategoryName;
	}
	
	// Run the pop-up modally
	DoModal();
	
	// Return the check state of the check box
	return m_bKeepWarning;
}

BOOL CUserWarningDlg::DoModalWarning(long nPatientID, BOOL bKeepWarning)
{
	try {
		// (a.walling 2010-07-01 16:18) - PLID 18081 - Warning categories
		// (a.walling 2010-10-31 23:08) - PLID 40965 - Parameteriize
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT DisplayWarning, WarningMessage, PersonT.First + ' ' + PersonT.Last As FullName, UserName, WarningDate, WarningCategoriesT.Color AS WarningColor, WarningCategoriesT.ID AS WarningID, WarningCategoriesT.Name AS WarningName "
			"FROM PersonT "
			"LEFT JOIN WarningCategoriesT ON PersonT.WarningCategoryID = WarningCategoriesT.ID "
			"LEFT JOIN UsersT "
				"ON UsersT.PersonID = PersonT.WarningUserID "
			"WHERE PersonT.ID = {INT}",
			nPatientID);

		if (rs->eof)
			return TRUE;
		FieldsPtr flds = rs->Fields;
		BOOL bDispWarning = AdoFldBool(flds, "DisplayWarning");
		if (bDispWarning)
		{
			CString strTitle;
			if (!GetPropertyInt("G2ShowWarningStats", 1, 0))
				strTitle = AdoFldString(flds, "FullName") + ":";
			else
			{
				if (flds->Item["UserName"]->Value.vt == VT_NULL ||
					flds->Item["WarningDate"]->Value.vt == VT_NULL)
				{
					strTitle = AdoFldString(flds, "FullName") + ":";
				}
				else
				{
					strTitle = CString("Warning for ") + AdoFldString(flds, "FullName") + " entered by " + AdoFldString(flds, "UserName") + " on " + FormatDateTimeForInterface(AdoFldDateTime(flds, "WarningDate"), DTF_STRIP_SECONDS, dtoDate);
				}
			}

			// (a.walling 2010-07-01 16:18) - PLID 18081 - Warning categories
			return DoModalWarning(AdoFldString(flds, "WarningMessage"), 
						bKeepWarning, strTitle, "Patient Warning", AdoFldLong(flds, "WarningColor", 0), AdoFldLong(flds, "WarningID", -1), AdoFldString(flds, "WarningName", ""));
		}
		return 0;
	}
	NxCatchAll("Failed to open patient warning");
	return -1;
}

BOOL CUserWarningDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (a.walling 2010-07-02 08:22) - PLID 18081 - Warning categories - Set the warning's background color
		if (m_colorWarning == 0) {
			m_nxeditWarningMsg.ResetBackgroundColorStandard();
			m_nxeditWarningMsg.ResetBackgroundColorFocus();
			m_nxeditWarningMsg.ResetBackgroundColorHovered();
			m_nxeditWarningMsg.ResetBackgroundColorHoveredFocus();
		} else {
			// (a.walling 2010-07-02 10:40) - PLID 39498 - Apply these colors even when disabled or readonly
			m_nxeditWarningMsg.SetBackgroundColorStandard(m_colorWarning, true, true);
			m_nxeditWarningMsg.SetBackgroundColorHovered(m_colorWarning, true, true);

			// Focus and HoveredFocus will not be touched (for readability)
			m_nxeditWarningMsg.ResetBackgroundColorFocus();
			m_nxeditWarningMsg.ResetBackgroundColorHoveredFocus();
		}

		m_nxeditWarningMsg.Invalidate();

		// (c.haag 2008-04-25 14:53) - PLID 29793 - NxIconify buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		
		// If this user doesn't have access to change the status of the warnings, disable the checkbox
		if (g_userPermission[ChangeWarning] == 0) {
			m_chkKeepWarning.EnableWindow(FALSE);
		}

		if (!m_strCaption.IsEmpty()) SetWindowText(m_strCaption);

		//beep at them (PL ID 5709)
		MessageBeep(MB_ICONEXCLAMATION);
	}
	NxCatchAll("Error in CUserWarningDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUserWarningDlg::OnKeepWarningCheck() 
{
	if (!UserPermission(ChangeWarning)){
		if (m_chkKeepWarning.GetCheck() == 0) {
			m_chkKeepWarning.SetCheck(1);
		} else {
			m_chkKeepWarning.SetCheck(0);
		}
	}
}