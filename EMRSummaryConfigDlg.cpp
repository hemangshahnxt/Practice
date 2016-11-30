// EMRSummaryConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRSummaryConfigDlg.h"
#include "SingleSelectDlg.h"
#include "PatientsRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-06-19 09:20) - PLID 30436 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum CategoryListColumns {

	clcID = 0,
	clcName,
	clcSortOrder,
};

/////////////////////////////////////////////////////////////////////////////
// CEMRSummaryConfigDlg dialog

CEMRSummaryConfigDlg::CEMRSummaryConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRSummaryConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRSummaryConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEMRSummaryConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRSummaryConfigDlg)
	DDX_Control(pDX, IDC_BTN_REMOVE_EMR_CATEGORY, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_BTN_ADD_EMR_CATEGORY, m_btnAddCategory);
	DDX_Control(pDX, IDC_CHECK_EMR_DOCUMENTS, m_checkEMRDocuments);
	DDX_Control(pDX, IDC_CHECK_PROBLEM_LIST, m_checkProblemList);
	DDX_Control(pDX, IDC_CHECK_PATIENT_EMR_PROBLEMS, m_checkPatientEmrProblems);
	DDX_Control(pDX, IDC_CHECK_PATIENT_NONEMR_PROBLEMS, m_checkPatientNonEmrProblems);
	DDX_Control(pDX, IDC_CHECK_LABS, m_checkLabs);
	DDX_Control(pDX, IDC_CHECK_PRESCRIBED_MEDICATIONS, m_checkPrescribedMeds);
	DDX_Control(pDX, IDC_CHECK_CURRENT_MEDICATIONS, m_checkCurrentMeds);
	DDX_Control(pDX, IDC_CHECK_SHOW_ALLERGIES, m_checkAllergies);
	DDX_Control(pDX, IDC_BTN_MOVE_CATEGORY_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_BTN_MOVE_CATEGORY_UP, m_btnMoveUp);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRSummaryConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRSummaryConfigDlg)
	ON_BN_CLICKED(IDC_BTN_MOVE_CATEGORY_UP, OnBtnMoveCategoryUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_CATEGORY_DOWN, OnBtnMoveCategoryDown)
	ON_BN_CLICKED(IDC_BTN_ADD_EMR_CATEGORY, OnBtnAddEmrCategory)
	ON_BN_CLICKED(IDC_BTN_REMOVE_EMR_CATEGORY, OnBtnRemoveEmrCategory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRSummaryConfigDlg message handlers

BOOL CEMRSummaryConfigDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);
		m_btnAddCategory.AutoSet(NXB_NEW);
		m_btnRemoveCategory.AutoSet(NXB_DELETE);

		m_CategoryList = BindNxDataList2Ctrl(IDC_CONFIG_CATEGORY_LIST, true);

		//load our settings
		// (c.haag 2008-12-16 15:28) - PLID 32467 - Include support for showing problems in
		// the general box at the top of the window
		g_propManager.CachePropertiesInBulk("CEMRSummaryConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMRSummaryShowAllergies' OR "
			"Name = 'EMRSummaryShowCurrentMeds' OR "
			"Name = 'EMRSummaryShowPrescriptions' OR "
			"Name = 'EMRSummaryShowLabs' OR "
			"Name = 'EMRSummaryShowProblemList' OR "
			"Name = 'EMRSummaryShowPtEmrProblems' OR "
			"Name = 'EMRSummaryShowPtNonEmrProblems' OR "
			"Name = 'EMRSummaryShowEMRDocuments' "
			")",
			_Q(GetCurrentUserName()));

		BOOL bShowAllergies = GetRemotePropertyInt("EMRSummaryShowAllergies", 1, 0, GetCurrentUserName(), true) == 1;
		BOOL bShowCurrentMeds = GetRemotePropertyInt("EMRSummaryShowCurrentMeds", 1, 0, GetCurrentUserName(), true) == 1;
		BOOL bShowPrescriptions = GetRemotePropertyInt("EMRSummaryShowPrescriptions", 1, 0, GetCurrentUserName(), true) == 1;
		BOOL bShowLabs = GetRemotePropertyInt("EMRSummaryShowLabs", 1, 0, GetCurrentUserName(), true) == 1;
		BOOL bShowProblemList = GetRemotePropertyInt("EMRSummaryShowProblemList", 1, 0, GetCurrentUserName(), true) == 1;
		BOOL bShowEMRDocuments = GetRemotePropertyInt("EMRSummaryShowEMRDocuments", 1, 0, GetCurrentUserName(), true) == 1;
		// (c.haag 2008-12-16 15:28) - PLID 32467
		BOOL bShowPtEmrProblems = GetRemotePropertyInt("EMRSummaryShowPtEmrProblems", 1, 0, GetCurrentUserName(), true) == 1;
		BOOL bShowPtNonEmrProblems = GetRemotePropertyInt("EMRSummaryShowPtNonEmrProblems", 1, 0, GetCurrentUserName(), true) == 1;

		m_checkAllergies.SetCheck(bShowAllergies);
		m_checkCurrentMeds.SetCheck(bShowCurrentMeds);
		m_checkPrescribedMeds.SetCheck(bShowPrescriptions);
		m_checkLabs.SetCheck(bShowLabs);
		m_checkProblemList.SetCheck(bShowProblemList);
		m_checkEMRDocuments.SetCheck(bShowEMRDocuments);
		// (c.haag 2008-12-16 15:29) - PLID 32467
		m_checkPatientEmrProblems.SetCheck(bShowPtEmrProblems);
		m_checkPatientNonEmrProblems.SetCheck(bShowPtNonEmrProblems);

		//hide the labs option if they are not licensed
		if(!g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
			GetDlgItem(IDC_CHECK_LABS)->ShowWindow(SW_HIDE);
		}
	
	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRSummaryConfigDlg::OnOK() 
{
	try {

		//in the interest of correct sorting, simply clear all sort orders,
		//and re-generate the new orders as they are displayed in this list

		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "UPDATE EMRCategoriesT SET SummarySortOrder = NULL WHERE SummarySortOrder Is Not Null");

		long nSortOrder = 0;
		IRowSettingsPtr pRow = m_CategoryList->GetFirstRow();
		while(pRow) {

			nSortOrder++;
			
			long nID = VarLong(pRow->GetValue(clcID));

			AddStatementToSqlBatch(strSqlBatch, "UPDATE EMRCategoriesT SET SummarySortOrder = %li WHERE ID = %li", nSortOrder, nID);

			pRow = pRow->GetNextRow();
		}

		ExecuteSqlBatch(strSqlBatch);

		SetRemotePropertyInt("EMRSummaryShowAllergies", m_checkAllergies.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		SetRemotePropertyInt("EMRSummaryShowCurrentMeds", m_checkCurrentMeds.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		SetRemotePropertyInt("EMRSummaryShowPrescriptions", m_checkPrescribedMeds.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		SetRemotePropertyInt("EMRSummaryShowLabs", m_checkLabs.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		SetRemotePropertyInt("EMRSummaryShowProblemList", m_checkProblemList.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		SetRemotePropertyInt("EMRSummaryShowEMRDocuments", m_checkEMRDocuments.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		// (c.haag 2008-12-16 15:30) - PLID 32467
		SetRemotePropertyInt("EMRSummaryShowPtEmrProblems", m_checkPatientEmrProblems.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		SetRemotePropertyInt("EMRSummaryShowPtNonEmrProblems", m_checkPatientNonEmrProblems.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		CNxDialog::OnOK();

	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnOK");
}

void CEMRSummaryConfigDlg::OnCancel() 
{
	try {
	
		CNxDialog::OnCancel();

	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnCancel");
}

void CEMRSummaryConfigDlg::OnBtnMoveCategoryUp() 
{
	try {

		IRowSettingsPtr pRow = m_CategoryList->GetCurSel();
		if(pRow == NULL) {
			return;
		}

		long nSortOrder = VarLong(pRow->GetValue(clcSortOrder));

		if(nSortOrder <= 1 || pRow == m_CategoryList->GetFirstRow()) {
			return;
		}

		//swap with the row above this one
		IRowSettingsPtr pSwapRow = pRow->GetPreviousRow();
		if(pSwapRow) {

			pSwapRow->PutValue(clcSortOrder, nSortOrder);
			pRow->PutValue(clcSortOrder, (long)(nSortOrder - 1));

			m_CategoryList->Sort();
		}

	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnBtnMoveCategoryUp");
}

void CEMRSummaryConfigDlg::OnBtnMoveCategoryDown() 
{
	try {

		IRowSettingsPtr pRow = m_CategoryList->GetCurSel();
		if(pRow == NULL) {
			return;
		}
		
		long nSortOrder = VarLong(pRow->GetValue(clcSortOrder));

		if(nSortOrder >= m_CategoryList->GetRowCount() || pRow == m_CategoryList->GetLastRow()) {
			return;
		}

		//swap with the row below this one
		IRowSettingsPtr pSwapRow = pRow->GetNextRow();
		if(pSwapRow) {

			pSwapRow->PutValue(clcSortOrder, nSortOrder);
			pRow->PutValue(clcSortOrder, (long)(nSortOrder + 1));

			m_CategoryList->Sort();
		}

	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnBtnMoveCategoryDown");
}

void CEMRSummaryConfigDlg::OnBtnAddEmrCategory() 
{
	try {

		CString strIDs;

		{
			IRowSettingsPtr pRow = m_CategoryList->GetFirstRow();
			while(pRow) {
				if(!strIDs.IsEmpty()) {
					strIDs += ",";
				}
				strIDs += AsString(VarLong(pRow->GetValue(clcID)));

				pRow = pRow->GetNextRow();
			}
		}

		CString strWhere;

		if(!strIDs.IsEmpty()) {
			strWhere.Format("ID NOT IN (%s)", strIDs);

			//doublecheck that we haven't used all the categories
			if(IsRecordsetEmpty("SELECT TOP 1 ID FROM EMRCategoriesT WHERE %s", strWhere)) {
				AfxMessageBox("You have already added all EMR Categories to this list.");
				return;
			}
		}

		CSingleSelectDlg dlg(this);
		if(IDOK == dlg.Open("EMRCategoriesT", strWhere, "ID", "Name", "Select a category:")) {

			//add to our list
			
			long nID = dlg.GetSelectedID();
			if(nID == -1) {
				return;
			}
			_variant_t var = dlg.GetSelectedDisplayValue();
			CString strName = VarString(var, "");

			IRowSettingsPtr pRow = m_CategoryList->GetNewRow();
			pRow->PutValue(clcID, nID);
			pRow->PutValue(clcName, _bstr_t(strName));
			pRow->PutValue(clcSortOrder, (long)(m_CategoryList->GetRowCount() + 1));
			m_CategoryList->AddRowSorted(pRow, NULL);
		}

	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnBtnAddEmrCategory");
}

void CEMRSummaryConfigDlg::OnBtnRemoveEmrCategory() 
{
	try {

		{

			IRowSettingsPtr pRow = m_CategoryList->GetCurSel();
			if(pRow == NULL) {
				AfxMessageBox("You must first select a category to remove.");
				return;
			}
			
			m_CategoryList->RemoveRow(pRow);
		}

		//renumber all rows and re-sort
		{
			long nCount = 0;
			IRowSettingsPtr pRow = m_CategoryList->GetFirstRow();
			while(pRow) {

				nCount++;
				
				pRow->PutValue(clcSortOrder, nCount);

				pRow = pRow->GetNextRow();
			}

			m_CategoryList->Sort();
		}

	}NxCatchAll("Error in CEMRSummaryConfigDlg::OnBtnRemoveEmrCategory");
}
