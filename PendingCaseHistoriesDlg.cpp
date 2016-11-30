// PendingCaseHistoriesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PendingCaseHistoriesDlg.h"
#include "CaseHistoryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPendingCaseHistoriesDlg dialog

// (j.jones 2008-03-31 17:23) - PLID 29267 - added enum so that the hyperlink works
enum PendingCaseColumns {

	pccID = 0,
	pccPatientName,
	pccDescription,
	pccQuantity,
	pccProvider,
	pccSurgeryDate,
};

using namespace ADODB;

CPendingCaseHistoriesDlg::CPendingCaseHistoriesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPendingCaseHistoriesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPendingCaseHistoriesDlg)
		m_nProductID = -1;
		m_nLocationID = -1;
	//}}AFX_DATA_INIT
}


void CPendingCaseHistoriesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPendingCaseHistoriesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_PRODUCT_NAME, m_nxstaticProductName);
	DDX_Control(pDX, IDC_LOCATION_NAME, m_nxstaticLocationName);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPendingCaseHistoriesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPendingCaseHistoriesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPendingCaseHistoriesDlg message handlers

BOOL CPendingCaseHistoriesDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 11:17) - PLID 29820 - NxIconify the Close button
		m_btnOK.AutoSet(NXB_CLOSE);
		
		m_List = BindNxDataListCtrl(this,IDC_PENDING_CASES_LIST,GetRemoteData(),false);

		CString str;
		str.Format("CompletedDate Is Null AND LocationID = %li AND CaseHistoryDetailsT.ItemID = %li AND CaseHistoryDetailsT.ItemType = -1",m_nLocationID,m_nProductID);
		m_List->WhereClause = _bstr_t(str);
		m_List->Requery();

		CString strProductName, strLocationName;
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",m_nProductID);
		if(!rs->eof) {
			strProductName = AdoFldString(rs, "Name","");
		}
		rs->Close();
		rs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li",m_nLocationID);
		if(!rs->eof) {
			strLocationName = AdoFldString(rs, "Name","");
		}
		rs->Close();

		SetDlgItemText(IDC_PRODUCT_NAME,strProductName);
		SetDlgItemText(IDC_LOCATION_NAME,strLocationName);
	}
	NxCatchAll("Error in CPendingCaseHistoriesDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CPendingCaseHistoriesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPendingCaseHistoriesDlg)
	ON_EVENT(CPendingCaseHistoriesDlg, IDC_PENDING_CASES_LIST, 19 /* LeftClick */, OnLeftClickPendingCasesList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPendingCaseHistoriesDlg::OnLeftClickPendingCasesList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow != -1 && nCol == pccDescription) {
		try {
			// Edit the case that was double-clicked on
			long nCaseHistoryID = VarLong(m_List->GetValue(nRow, pccID));
			CCaseHistoryDlg dlg(this);
			dlg.OpenExistingCase(nCaseHistoryID);
			m_List->Requery();

		} NxCatchAll("Error loading case history.");
	}
}
