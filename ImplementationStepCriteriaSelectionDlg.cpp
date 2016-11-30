// ImplementationStepCriteriaSelectionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "ImplementationStepCriteriaSelectionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImplementationStepCriteriaSelectionDlg dialog

enum CriteriaColumns {
	ccStepID = 0,
	ccClientVisible,
	ccClientComplete,
	ccComplete,
	ccActionItemID,
	ccName,
};

//this is also used in ImplementationDlg.cpp
enum ListType {
	ltProcedures = 1,
	ltEMR,
};

CImplementationStepCriteriaSelectionDlg::CImplementationStepCriteriaSelectionDlg(long nListType, long nStepID, CWnd* pParent)
	: CNxDialog(CImplementationStepCriteriaSelectionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImplementationStepCriteriaSelectionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nListType = nListType;
	m_nStepID = nStepID;
}


void CImplementationStepCriteriaSelectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImplementationStepCriteriaSelectionDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CImplementationStepCriteriaSelectionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CImplementationStepCriteriaSelectionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImplementationStepCriteriaSelectionDlg message handlers

BOOL CImplementationStepCriteriaSelectionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pCriteriaList = BindNxDataList2Ctrl(IDC_IMPLEMENTATION_ACTION_LIST, false);

	CString strFrom;

	switch (m_nListType) {

		case ltProcedures:
			strFrom = " ClientImplementationStepCriteriaT LEFT JOIN ImplementationProcedureT ON ClientImplementationStepCriteriaT.ActionItemID = ImplementationProcedureT.ID ";
		break;

		case ltEMR:
			strFrom = "ClientImplementationStepCriteriaT LEFT JOIN ImplementationEMRTemplatesT ON ClientImplementationStepCriteriaT.ActionItemID = ImplementationEMRTemplatesT.ID ";
		break;

	}

	m_pCriteriaList->FromClause = _bstr_t(strFrom);
	m_pCriteriaList->WhereClause = _bstr_t("StepID = " + AsString(m_nStepID));

	m_pCriteriaList->Requery();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CImplementationStepCriteriaSelectionDlg::OnOK() 
{
	try {

		//loop through the list and see what is selected and then mark it in the data
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCriteriaList->GetFirstRow();
		CString strSql = BeginSqlBatch();

		while (pRow) {
			BOOL bIsClientVisible;
			BOOL bIsClientComplete;
			BOOL bIsComplete;
			long nValue;

			bIsClientVisible = VarBool(pRow->GetValue(ccClientVisible), FALSE);
			bIsClientComplete = VarBool(pRow->GetValue(ccClientComplete), FALSE);
			bIsComplete = VarBool(pRow->GetValue(ccComplete), FALSE);
			nValue = VarLong(pRow->GetValue(ccActionItemID));

			AddStatementToSqlBatch(strSql, " UPDATE ClientImplementationStepCriteriaT SET ClientVisible = %li, ClientComplete = %li, Complete = %li WHERE StepID = %li AND ActionItemID = %li ",
				bIsClientVisible, bIsClientComplete, bIsComplete, m_nStepID, nValue);

			pRow = pRow->GetNextRow();
		}
		
		ExecuteSqlBatch(strSql);

		CDialog::OnOK();
	}NxCatchAll("Error in CImplementationStepCriteriaSelectionDlg::OnOK");
}

void CImplementationStepCriteriaSelectionDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
