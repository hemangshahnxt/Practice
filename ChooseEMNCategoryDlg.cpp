// ChooseEMNCategoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ChooseEMNCategoryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChooseEMNCategoryDlg dialog

enum EEMNCategoriesListColumns
{
	eclcID = 0,
	eclcDescription,
	eclcPriority,
};

CChooseEMNCategoryDlg::CChooseEMNCategoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChooseEMNCategoryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChooseEMNCategoryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nEMNCategoryID = -1;
	m_nChartID = -1;
}


void CChooseEMNCategoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChooseEMNCategoryDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChooseEMNCategoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CChooseEMNCategoryDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChooseEMNCategoryDlg message handlers

BOOL CChooseEMNCategoryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-25 16:49) - PLID 29796 - NxIconify the OK button
		m_btnOK.AutoSet(NXB_OK);
		// (c.haag 2008-05-21 16:35) - PLID 29796 - I added a cancel button
		// because this dialog has always had true OK/Cancel behavior (just
		// without the cancel button)
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Connect our variable to the control
		m_EMNCategoriesList = BindNxDataList2Ctrl(this, IDC_EMN_CAT_LIST, GetRemoteData(), false);
		if(m_nChartID != -1) {
			// (z.manning, 06/04/2007) - PLID 26214 - If we have a valid chart ID, make sure to we only
			// load categories associated with the given chart.
			m_EMNCategoriesList->WhereClause = _bstr_t(FormatString(
				"EmnTabCategoriesT.ID IN (SELECT EmnTabCategoryID FROM EMNTabChartCategoryLinkT WHERE EmnTabChartID = %li)"
				, m_nChartID));
		}
		m_EMNCategoriesList->Requery();	

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_EMNCategoriesList->GetNewRow();

		pRow->PutValue(eclcID, (long)-1);
		pRow->PutValue(eclcDescription, _variant_t("{No EMN Category}"));
		pRow->PutValue(eclcPriority, (long)-1);
		
		m_EMNCategoriesList->AddRowSorted(pRow, NULL);
		m_EMNCategoriesList->SetSelByColumn(eclcID, _variant_t((long)-1));

	} NxCatchAll("CConfigureEMRTabsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChooseEMNCategoryDlg::OnOK() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_EMNCategoriesList->GetCurSel();

	// (z.manning, 05/07/2007) - PLID 25731 - Also track the category name.
	if (pRow) {
		m_nEMNCategoryID = pRow->GetValue(eclcID);
		m_strEMNCategoryName = VarString(pRow->GetValue(eclcDescription), "");
	}
	else {
		m_nEMNCategoryID = -1;
		m_strEMNCategoryName.Empty();
	}
	
	CDialog::OnOK();
}
