// ConfigureAAFPRSSurveyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ConfigureAAFPRSSurveyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigureAAFPRSSurveyDlg dialog


CConfigureAAFPRSSurveyDlg::CConfigureAAFPRSSurveyDlg(CWnd* pParent)
	: CNxDialog(CConfigureAAFPRSSurveyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureAAFPRSSurveyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CConfigureAAFPRSSurveyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureAAFPRSSurveyDlg)
	DDX_Control(pDX, IDC_AAFPRS_CLOSE, m_Close);
	DDX_Control(pDX, IDC_AAFPRS_MOVE_ONE_RIGHT, m_Move_One_Right);
	DDX_Control(pDX, IDC_AAFPRS_MOVE_ONE_LEFT, m_Move_One_Left);
	DDX_Control(pDX, IDC_AAFPRS_MOVE_ALL_RIGHT, m_Move_All_Right);
	DDX_Control(pDX, IDC_AAFPRS_MOVE_ALL_LEFT, m_Move_All_Left);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CConfigureAAFPRSSurveyDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureAAFPRSSurveyDlg)
	ON_BN_CLICKED(IDC_AAFPRS_CLOSE, OnAafprsClose)
	ON_BN_CLICKED(IDC_AAFPRS_MOVE_ALL_RIGHT, OnAafprsMoveAllRight)
	ON_BN_CLICKED(IDC_AAFPRS_MOVE_ONE_RIGHT, OnAafprsMoveOneRight)
	ON_BN_CLICKED(IDC_AAFPRS_MOVE_ONE_LEFT, OnAafprsMoveOneLeft)
	ON_BN_CLICKED(IDC_AAFPRS_MOVE_ALL_LEFT, OnAafprsMoveAllLeft)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureAAFPRSSurveyDlg message handlers

void CConfigureAAFPRSSurveyDlg::OnAafprsClose() 
{
	CDialog::OnOK();
	
}

void CConfigureAAFPRSSurveyDlg::OnCancel() {

	CDialog::OnCancel();
}

BOOL CConfigureAAFPRSSurveyDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	//Bind our datalist2s
	m_pTypeList = BindNxDataList2Ctrl(IDC_AAFPRS_TYPE_LIST, false);
	m_pAvailList  = BindNxDataList2Ctrl(IDC_AVAIL_PROC_LIST, false);
	m_pSelectedList = BindNxDataList2Ctrl(IDC_SELECTED_PROC_LIST, false);

	m_Move_One_Right.AutoSet(NXB_RIGHT);
	m_Move_One_Left.AutoSet(NXB_LEFT);
	m_Move_All_Left.AutoSet(NXB_LLEFT);
	m_Move_All_Right.AutoSet(NXB_RRIGHT);
	m_Close.AutoSet(NXB_CLOSE);

	//now add the rows to the Type list
	//
	// (c.haag 2007-01-12 08:57) - PLID 7048 - I added a second column that helps
	// show the user where on the report that the row is used
	//
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0, (long)afsCosmeticSurgical);
	pRow->PutValue(1, _variant_t("Cosmetic Surgical Procedures"));
	pRow->PutValue(2, _variant_t("\"Total number of procedures performed by gender\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);

	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0, (long)afsCosmeticNonSurgical);
	pRow->PutValue(1, _variant_t("Cosmetic Non-Surgical Procedures"));
	pRow->PutValue(2, _variant_t("\"Total number of procedures performed by gender\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);

	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0,(long) afsReconstructiveSurgical);
	pRow->PutValue(1, _variant_t("Reconstructive Surgical Procedures"));
	pRow->PutValue(2, _variant_t("\"Total number of procedures performed by gender\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);
	
	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0, (long)afsFacial);
	pRow->PutValue(1, _variant_t("Facial Procedures"));
	pRow->PutValue(2, _variant_t("\"Percentage of patients on whom multiple facial procedures were performed (at the same time or during the course of the same year)\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);

	//
	// (c.haag 2007-01-12 08:57) - PLID 7048 - Needed more fields
	//
	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0, (long)afsFacialCosmetic);
	pRow->PutValue(1, _variant_t("Facial Cosmetic Surgical Procedures"));
	pRow->PutValue(2, _variant_t("\"Facial cosmetic surgical procedures performed by racial groups\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);

	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0, (long)afsSurgicalProcedures);
	pRow->PutValue(1, _variant_t("Surgical Procedures"));
	pRow->PutValue(2, _variant_t("\"Number of procedures performed on female patients\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);

	pRow = m_pTypeList->GetNewRow();
	pRow->PutValue(0, (long)afsMinimallyInvasiveProcedures);
	pRow->PutValue(1, _variant_t("Minimally Invasive Procedures"));
	pRow->PutValue(2, _variant_t("\"Number of procedures performed on female patients\""));
	m_pTypeList->AddRowAtEnd(pRow, NULL);


	//now put the first row as the selected
	m_pTypeList->SetSelByColumn(0, (long)afsCosmeticSurgical);
	OnSelChosenAafprsTypeList(m_pTypeList->CurSel);

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CConfigureAAFPRSSurveyDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureAAFPRSSurveyDlg)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_AVAIL_PROC_LIST, 16 /* SelChosen */, OnSelChosenAvailProcList, VTS_DISPATCH)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_AVAIL_PROC_LIST, 3 /* DblClickCell */, OnDblClickCellAvailProcList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_SELECTED_PROC_LIST, 16 /* SelChosen */, OnSelChosenSelectedProcList, VTS_DISPATCH)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_SELECTED_PROC_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedProcList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_AAFPRS_TYPE_LIST, 16 /* SelChosen */, OnSelChosenAafprsTypeList, VTS_DISPATCH)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_AAFPRS_TYPE_LIST, 1 /* SelChanging */, OnSelChangingAafprsTypeList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_SELECTED_PROC_LIST, 1 /* SelChanging */, OnSelChangingSelectedProcList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CConfigureAAFPRSSurveyDlg, IDC_AVAIL_PROC_LIST, 1 /* SelChanging */, OnSelChangingAvailProcList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigureAAFPRSSurveyDlg::OnSelChosenAvailProcList(LPDISPATCH lpRow) 
{
	
	
}


void CConfigureAAFPRSSurveyDlg::MoveOneRight(LPDISPATCH lpRow) {

	try {
	
		NXDATALIST2Lib::IRowSettingsPtr pTypeRow;
		pTypeRow = m_pTypeList->GetCurSel();
		long nTypeID = VarLong(pTypeRow->GetValue(0));

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			long nProcID = VarLong(pRow->GetValue(0));

			ExecuteSql("INSERT INTO ConfigureAAFPRSSurveyT(ProcTypeID, ProcedureID) VALUES (%li, %li)", nTypeID, nProcID);

			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
			m_pSelectedList->TakeRowAddSorted(pRow);
		}
	}NxCatchAll("Error in CConfigureAAFPRSSurveyDlg::MoveOneRight(LPDISPATCH lpRow)");
	
}

void CConfigureAAFPRSSurveyDlg::MoveOneLeft(LPDISPATCH lpRow) {

	
	try {

		NXDATALIST2Lib::IRowSettingsPtr pTypeRow;
		pTypeRow = m_pTypeList->GetCurSel();
		long nTypeID = VarLong(pTypeRow->GetValue(0));


		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			long nProcID = VarLong(pRow->GetValue(0));

			ExecuteSql("DELETE FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = %li AND ProcedureID = %li", nTypeID, nProcID);

			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
			m_pAvailList->TakeRowAddSorted(pRow);
		}
	}NxCatchAll("Error in CConfigureAAFPRSSurveyDlg::MoveOneRight(LPDISPATCH lpRow)");

}

void CConfigureAAFPRSSurveyDlg::OnDblClickCellAvailProcList(LPDISPATCH lpRow, short nColIndex) 
{
	MoveOneRight(lpRow);
	
}

void CConfigureAAFPRSSurveyDlg::OnSelChosenSelectedProcList(LPDISPATCH lpRow) 
{
		
}

void CConfigureAAFPRSSurveyDlg::OnDblClickCellSelectedProcList(LPDISPATCH lpRow, short nColIndex) 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	MoveOneLeft(pRow);
	
}

void CConfigureAAFPRSSurveyDlg::OnAafprsMoveAllRight() 
{
	try {
		//get which type we are on
		NXDATALIST2Lib::IRowSettingsPtr pTypeRow = m_pTypeList->GetCurSel();
		long nTypeID = VarLong(pTypeRow->GetValue(0));
				
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAvailList->GetFirstRow();

		long nCount = m_pAvailList->GetRowCount();
			
		LPDISPATCH lpDisp = NULL;
		CString strSQL = BeginSqlBatch();
		while (pRow) {
					
			//insert them into this group
			long nProcID = VarLong(pRow->GetValue(0));
			AddStatementToSqlBatch(strSQL, "INSERT INTO ConfigureAAFPRSSurveyT(ProcTypeID, ProcedureID) VALUES "
				" (%li, %li) ", nTypeID, nProcID);
		
			pRow = pRow->GetNextRow();
		}
	

		if (nCount > 0) {
			ExecuteSql(strSQL);	

			//now move the row
			m_pSelectedList->TakeAllRows(m_pAvailList);
		}

	}NxCatchAll("Error in CConfigureAAFPRSSurveyDlg::OnAafprsMoveAllRight()");

	
}

void CConfigureAAFPRSSurveyDlg::OnAafprsMoveOneRight() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pAvailList->GetCurSel();

	MoveOneRight(pRow);
	
}

void CConfigureAAFPRSSurveyDlg::OnAafprsMoveOneLeft() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	pRow = m_pSelectedList->GetCurSel();

	MoveOneLeft(pRow);
	
	
}

void CConfigureAAFPRSSurveyDlg::OnAafprsMoveAllLeft() 
{
	try {
		//get which type we are on
		NXDATALIST2Lib::IRowSettingsPtr pTypeRow = m_pTypeList->GetCurSel();
		long nTypeID = VarLong(pTypeRow->GetValue(0));
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();

		long nCount = m_pSelectedList->GetRowCount();
			
		CString strSQL = BeginSqlBatch();
		while (pRow) {
					
			//insert them into this group
			long nProcID = VarLong(pRow->GetValue(0));
			AddStatementToSqlBatch(strSQL, "DELETE FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = %li AND ProcedureID = %li"
				, nTypeID, nProcID);

			pRow = pRow->GetNextRow();
		}
	

		if (nCount > 0) {

			ExecuteSql(strSQL);	
		
			//now move the row
			m_pAvailList->TakeAllRows(m_pSelectedList);
		}

	}NxCatchAll("Error in CConfigureAAFPRSSurveyDlg::OnAafprsMoveAllLeft()");
	
}

void CConfigureAAFPRSSurveyDlg::OnSelChosenAafprsTypeList(LPDISPATCH lpRow) 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	long nTypeID = VarLong(pRow->GetValue(0));

	CString strWhere;
	strWhere.Format("MasterProcedureID IS NULL AND ID NOT IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = %li)", nTypeID);
	m_pAvailList->WhereClause = _bstr_t(strWhere);

	strWhere.Format("MasterProcedureID IS NULL AND  ID IN (SELECT ProcedureID FROM ConfigureAAFPRSSurveyT WHERE ProcTypeID = %li)", nTypeID);
	m_pSelectedList->WhereClause = _bstr_t(strWhere);

	//now requery the lists
	m_pAvailList->Requery();
	m_pSelectedList->Requery();
	
}

void CConfigureAAFPRSSurveyDlg::OnSelChangingAafprsTypeList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}

void CConfigureAAFPRSSurveyDlg::OnSelChangingSelectedProcList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}

void CConfigureAAFPRSSurveyDlg::OnSelChangingAvailProcList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	
}
