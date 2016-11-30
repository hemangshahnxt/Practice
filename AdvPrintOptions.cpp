// AdvPrintOptions.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "AdvPrintOptions.h"
#include "GlobalDataUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_DETAILED 0
#define ID_SUMMARY	1
#define ID_SPECIAL	15

#define ID_SPECIAL_BUTTON 1003

/////////////////////////////////////////////////////////////////////////////
// CAdvPrintOptions dialog


CAdvPrintOptions::CAdvPrintOptions(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvPrintOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvPrintOptions)
	//}}AFX_DATA_INIT

	m_nResult = -1;
	m_pwndSpecialButton = NULL;
}

CAdvPrintOptions::~CAdvPrintOptions()
{
	if (m_pwndSpecialButton) {
		delete m_pwndSpecialButton;
		m_pwndSpecialButton = NULL;
	}
}

void CAdvPrintOptions::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvPrintOptions)
	DDX_Control(pDX, IDC_DETAILED_OPTION, m_rDetailOptions);
	DDX_Control(pDX, IDC_SUMMARY_OPTION, m_rSummaryOptions);
	DDX_Control(pDX, IDC_ALL_OPTION, m_rAllOptions);
	DDX_Control(pDX, IDC_SINGLE_OPTION, m_rSingleOptions);
	DDX_Control(pDX, IDC_FROM_DATE, m_FromDate);
	DDX_Control(pDX, IDC_TO_DATE, m_ToDate);
	DDX_Control(pDX, IDC_FROMDATE_LABEL, m_nxstaticFromdateLabel);
	DDX_Control(pDX, IDC_TODATE_LABEL, m_nxstaticTodateLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvPrintOptions, CNxDialog)
	//{{AFX_MSG_MAP(CAdvPrintOptions)
	ON_BN_CLICKED(IDC_ALL_OPTION, OnClickAllOption)
	ON_BN_CLICKED(IDC_SINGLE_OPTION, OnClickSingleOption)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvPrintOptions message handlers

BOOL CAdvPrintOptions::OnInitDialog() 
{
	try {	
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 09:52) - PLID 29863 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		if (m_bDetailed) 
			((CButton*)GetDlgItem(IDC_DETAILED_OPTION))->SetCheck(TRUE);
		else {
			GetDlgItem(IDC_DETAILED_OPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SUMMARY_OPTION)->ShowWindow(SW_HIDE);
		}

		if (m_bDetailed) {
			// (a.walling 2009-11-24 14:33) - PLID 36418
			if (!m_strSpecialCaption.IsEmpty()) {
				m_pwndSpecialButton = new NxButton();

				CRect rcDetail, rcSummary, rcCancel;
				m_rDetailOptions.GetWindowRect(rcDetail);
				m_rSummaryOptions.GetWindowRect(rcSummary);
				m_btnCancel.GetWindowRect(rcCancel);

				ScreenToClient(rcDetail);
				ScreenToClient(rcSummary);
				ScreenToClient(rcCancel);

				long nSpacing = rcSummary.top - rcDetail.bottom;

				CRect rcSpecial(rcSummary.left, rcSummary.bottom + nSpacing, rcCancel.left - 5, rcSummary.bottom + nSpacing + rcSummary.Height());

				m_pwndSpecialButton->Create(m_strSpecialCaption, WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON, rcSpecial, this, ID_SPECIAL_BUTTON);
				ChangeZOrder(m_pwndSpecialButton, &m_rSummaryOptions);
			}
		}
		
		if (m_bDateRange) {
			// (a.walling 2008-05-28 17:47) - PLID 27591 - Explicitly use a COleDateTime to set the value
			if (m_dtInitFrom.GetStatus() == COleDateTime::valid && m_dtInitFrom.GetYear() >= 1700 && m_dtInitFrom.GetYear() <= 2222)
				m_FromDate.SetValue(m_dtInitFrom);
			if (m_dtInitTo.GetStatus() == COleDateTime::valid && m_dtInitTo.GetYear() >= 1700 && m_dtInitTo.GetYear() <= 2222)
				m_ToDate.SetValue(m_dtInitTo);
		}
		else {
			GetDlgItem(IDC_FROM_DATE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FROMDATE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TODATE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_TO_DATE)->ShowWindow(SW_HIDE);
		}

		if (!m_bOptionCombo) {
			GetDlgItem(IDC_ALL_OPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SINGLE_OPTION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_OPTION_LIST)->ShowWindow(SW_HIDE);
		}
		else {
			((CButton*)GetDlgItem(IDC_ALL_OPTION))->SetCheck(TRUE);
			m_bAllOptions = true;
			SetDlgItemText(IDC_ALL_OPTION,m_cAllCaption);
			SetDlgItemText(IDC_SINGLE_OPTION,m_cSingleCaption);
			//set up the datalist
			try {
				m_OptionList = BindNxDataListCtrl(this,IDC_OPTION_LIST,GetRemoteData(),false);
				if (m_OptionList == NULL) {
					HandleException(NULL, "Error in CAdvPrintOptions::OnInitDialog \n DataList is Null, Cannot Continue");
					return FALSE;
				}
			m_OptionList->PutEnabled(FALSE);
			m_OptionList->FromClause = (LPCTSTR)m_cSQL;
			m_OptionList->Requery();
			m_OptionList->PutCurSel(0);
			}NxCatchAll("Error in CNotesDlg::OnInitDialog \n Could Not Bind DataList");
		}

		SetDlgItemText(IDOK,m_btnCaption);
	}
	NxCatchAll("Error in CAdvPrintOptions::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvPrintOptions::OnOK() 
{
	if (m_bAllOptions)
	{
		if(((CButton*)GetDlgItem(IDC_ALL_OPTION))->GetCheck())
			m_nResult = -1;
		else
			m_nResult = VarLong(m_OptionList->GetValue(m_OptionList->GetCurSel(), 0));
	}
	else
		m_nResult = -1;

	if(m_bDateRange)
	{
		m_dtFromDate = VarDateTime(m_FromDate.GetValue());
		m_dtToDate = VarDateTime(m_ToDate.GetValue());
	}

	if (m_bDetailed) {
		// (a.walling 2009-11-24 14:46) - PLID 36418
		if (!m_strSpecialCaption.IsEmpty() && m_pwndSpecialButton->GetCheck() != 0) {
			EndDialog(ID_SPECIAL);
		} else if (m_rDetailOptions.GetCheck() != 0) {
			EndDialog(ID_DETAILED);
		} else {
			EndDialog(ID_SUMMARY);
		}
	}

	else EndDialog(IDOK);

}

void CAdvPrintOptions::OnClickAllOption() 
{
	if (m_rAllOptions.GetCheck() == TRUE) {
		m_OptionList->PutEnabled(FALSE);
	}
}

void CAdvPrintOptions::OnClickSingleOption() 
{
	if (m_rSingleOptions.GetCheck() == TRUE) {
		m_OptionList->PutEnabled(true);
	}
}
