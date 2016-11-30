// AdvInsurancePayDescriptionSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdvInsurancePayDescriptionSetupDlg.h"
#include "EditComboBox.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "PayCatDlg.h" // (j.gruber 2012-11-15 13:55) - PLID 53752

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-07-11 09:29) - PLID 30679 - created

/////////////////////////////////////////////////////////////////////////////
// CAdvInsurancePayDescriptionSetupDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum CategoryComboColumns {

	cccID = 0,
	cccName,
};

enum InsuranceListColumns {

	ilcID = 0,
	ilcName,
	ilcPayDesc,
	ilcPayCatID,
	ilcPayCatName,
	ilcAdjDesc,
	ilcAdjCatID,
	ilcAdjCatName,
};

CAdvInsurancePayDescriptionSetupDlg::CAdvInsurancePayDescriptionSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvInsurancePayDescriptionSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvInsurancePayDescriptionSetupDlg)
		m_nCurInsCoID = -1;
		m_bChangesMadeToCurrentIns = FALSE;
	//}}AFX_DATA_INIT
}


void CAdvInsurancePayDescriptionSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvInsurancePayDescriptionSetupDlg)
	DDX_Control(pDX, IDC_RADIO_ADV_PAY_DESC, m_radioPayment);
	DDX_Control(pDX, IDC_RADIO_ADV_ADJ_DESC, m_radioAdjustment);
	DDX_Control(pDX, IDC_ADV_INS_PAY_DESCRIPTION_LABEL, m_nxstaticDescriptionLabel);
	DDX_Control(pDX, IDC_CATEGORY_LABEL, m_nxstaticCategoryLabel);
	DDX_Control(pDX, IDC_EDIT_DEF_DESCRIPTION, m_nxeditDescription);
	DDX_Control(pDX, IDC_BTN_EDIT_PAY_CATS, m_btnEditCats);
	DDX_Control(pDX, IDC_BTN_APPLY_DESC, m_btnApplyDesc);
	DDX_Control(pDX, IDC_BTN_APPLY_CATEGORY, m_btnApplyCategory);
	DDX_Control(pDX, IDC_BTN_ADV_INS_DESC_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADV_INS_DESC_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_ADV_INS_DESC_UNSELECT_ONE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_ADV_INS_DESC_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_ADV_INS_DESC_SELECT_ONE, m_btnSelectOne);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvInsurancePayDescriptionSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAdvInsurancePayDescriptionSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADV_INS_DESC_SELECT_ONE, OnBtnAdvInsDescSelectOne)
	ON_BN_CLICKED(IDC_BTN_ADV_INS_DESC_SELECT_ALL, OnBtnAdvInsDescSelectAll)
	ON_BN_CLICKED(IDC_BTN_ADV_INS_DESC_UNSELECT_ONE, OnBtnAdvInsDescUnselectOne)
	ON_BN_CLICKED(IDC_BTN_ADV_INS_DESC_UNSELECT_ALL, OnBtnAdvInsDescUnselectAll)
	ON_BN_CLICKED(IDC_BTN_EDIT_PAY_CATS, OnBtnEditPayCats)
	ON_BN_CLICKED(IDC_BTN_APPLY_DESC, OnBtnApplyDesc)
	ON_BN_CLICKED(IDC_BTN_APPLY_CATEGORY, OnBtnApplyCategory)
	ON_BN_CLICKED(IDC_RADIO_ADV_PAY_DESC, OnRadioAdvPayDesc)
	ON_BN_CLICKED(IDC_RADIO_ADV_ADJ_DESC, OnRadioAdvAdjDesc)
	ON_BN_CLICKED(IDC_BTN_ADV_INS_DESC_CLOSE, OnBtnAdvInsDescClose)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvInsurancePayDescriptionSetupDlg message handlers

BOOL CAdvInsurancePayDescriptionSetupDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		m_btnApplyCategory.AutoSet(NXB_MODIFY);
		m_btnApplyDesc.AutoSet(NXB_MODIFY);

		m_nxeditDescription.SetLimitText(255);

		m_radioPayment.SetCheck(TRUE);

		m_UnselectedList = BindNxDataList2Ctrl(IDC_ADV_INS_DESC_UNSELECTED_LIST);
		m_SelectedList = BindNxDataList2Ctrl(IDC_ADV_INS_DESC_SELECTED_LIST, false);
		m_CategoryCombo = BindNxDataList2Ctrl(IDC_ADV_INS_DESC_CATEGORY_COMBO);

		IRowSettingsPtr pRow = m_CategoryCombo->GetNewRow();
		pRow->PutValue(cccID, (long)-1);
		pRow->PutValue(cccName, _bstr_t(" <No Default Category>"));
		m_CategoryCombo->AddRowSorted(pRow, NULL);
	
	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescSelectOne() 
{
	try {

		if(m_UnselectedList->GetCurSel() == NULL) {
			return;
		}

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescSelectOne");
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescSelectAll() 
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescSelectAll");
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescUnselectOne() 
{
	try {

		if(m_SelectedList->GetCurSel() == NULL) {
			return;
		}

		// (b.cardillo 2008-07-15 13:08) - PLID 30737 - Changed to use TakeCurrentRowAddSorted() for efficiency
		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescUnselectOne");
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescUnselectAll() 
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescUnselectAll");
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnEditPayCats() 
{
	try {

		//don't need to store the selection, just edit the list normally
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// (j.gruber 2012-11-15 13:54) - PLID 53752 - change the dialog
		CPayCatDlg dlg(this);
		if (dlg.DoModal()) {
			m_CategoryCombo->Requery();			
			m_CategoryCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		}
		
		//CEditComboBox(this, 3, m_CategoryCombo, "Edit Combo Box").DoModal();

		IRowSettingsPtr pRow = m_CategoryCombo->GetNewRow();
		pRow->PutValue(cccID, (long)-1);
		pRow->PutValue(cccName, _bstr_t(" <No Default Category>"));
		m_CategoryCombo->AddRowSorted(pRow, NULL);

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnEditPayCats");
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnApplyDesc() 
{
	long nAuditTransactionID = -1;

	try {

		BOOL bIsAdjustment = m_radioAdjustment.GetCheck();

		CString strType = "payment";
		if(bIsAdjustment) {
			strType = "adjustment";
		}

		if(m_SelectedList->GetRowCount() == 0) {
			CString strWarning;
			strWarning.Format("You must have at least one insurance company selected before applying %s descriptions.", strType);
			AfxMessageBox(strWarning);
			return;
		}

		//get the description
		CString strNewDescription = "";
		m_nxeditDescription.GetWindowText(strNewDescription);
		strNewDescription.TrimLeft();
		strNewDescription.TrimRight();
		
		CString strWarning;

		if(strNewDescription.IsEmpty()) {
			strWarning.Format("The %s description is blank. If you continue, this action will clear out the default %s description for all selected insurance companies.\n\n"
				"Are you sure you wish to do this?", strType, strType);
		}
		else {
			strWarning.Format("This action will update the default %s description for all selected insurance companies to use the following description:\n\n"
				"%s\n\n"
				"Are you sure you wish to do this?", strType, strNewDescription);
		}

		if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		CWaitCursor pWait;

		//now update each insurance company, and audit the changes

		CString strSqlBatch;
		{
			IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
			while(pRow) {

				long nInsCoID = VarLong(pRow->GetValue(ilcID));
				CString strInsName = VarString(pRow->GetValue(ilcName));

				if(bIsAdjustment) {
					AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET DefaultAdjDesc = '%s' WHERE PersonID = %li", _Q(strNewDescription), nInsCoID);

					//track if we just updated the insurance company we are editing on the edit ins. list
					if(nInsCoID == m_nCurInsCoID) {
						m_bChangesMadeToCurrentIns = TRUE;
					}

					CString strOldDescription = VarString(pRow->GetValue(ilcAdjDesc), "");

					if(strOldDescription != strNewDescription) {

						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, strInsName, nAuditTransactionID, aeiInsCoDefAdjustmentDescription, nInsCoID, strOldDescription, strNewDescription, aepMedium, aetChanged);
					}
				}
				else {
					AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET DefaultPayDesc = '%s' WHERE PersonID = %li", _Q(strNewDescription), nInsCoID);

					//track if we just updated the insurance company we are editing on the edit ins. list
					if(nInsCoID == m_nCurInsCoID) {
						m_bChangesMadeToCurrentIns = TRUE;
					}

					CString strOldDescription = VarString(pRow->GetValue(ilcPayDesc), "");

					if(strOldDescription != strNewDescription) {

						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(-1, strInsName, nAuditTransactionID, aeiInsCoDefPaymentDescription, nInsCoID, strOldDescription, strNewDescription, aepMedium, aetChanged);
					}
				}

				pRow = pRow->GetNextRow();
			}
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		//now we have to update the datalist contents
		{
			IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
			while(pRow) {

				if(bIsAdjustment) {
					pRow->PutValue(ilcAdjDesc, _bstr_t(strNewDescription));
				}
				else {
					pRow->PutValue(ilcPayDesc, _bstr_t(strNewDescription));
				}

				pRow = pRow->GetNextRow();
			}
		}

		AfxMessageBox("Description update complete.");

	}NxCatchAllCall("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnApplyDesc",

		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnApplyCategory() 
{
	long nAuditTransactionID = -1;

	try {

		BOOL bIsAdjustment = m_radioAdjustment.GetCheck();

		CString strType = "payment";
		if(bIsAdjustment) {
			strType = "adjustment";
		}

		if(m_SelectedList->GetRowCount() == 0) {
			CString strWarning;
			strWarning.Format("You must have at least one insurance company selected before applying %s categories.", strType);
			AfxMessageBox(strWarning);
			return;
		}

		long nNewCategoryID = -1;
		CString strNewCategoryName = "";

		{
			IRowSettingsPtr pRow = m_CategoryCombo->GetCurSel();
			if(pRow) {
				nNewCategoryID = VarLong(pRow->GetValue(cccID));				
				strNewCategoryName = VarString(pRow->GetValue(cccName));
			}
		}

		CString strWarning;

		CString strNewCategoryID = "NULL";

		if(nNewCategoryID == -1) {
			strWarning.Format("There is no %s category selected. If you continue, this action will clear out the default %s category for all selected insurance companies.\n\n"
				"Are you sure you wish to do this?", strType, strType);
		}
		else {
			strWarning.Format("This action will update the default %s category for all selected insurance companies to use the '%s' category.\n\n"
				"Are you sure you wish to do this?", strType, strNewCategoryName);
			
			strNewCategoryID.Format("%li", nNewCategoryID);
		}

		if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		CWaitCursor pWait;

		//now update each insurance company, and audit the changes

		CString strSqlBatch;
		IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
		while(pRow) {

			long nInsCoID = VarLong(pRow->GetValue(ilcID));
			CString strInsName = VarString(pRow->GetValue(ilcName));			

			if(bIsAdjustment) {
				AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET DefaultAdjCategoryID = %s WHERE PersonID = %li", strNewCategoryID, nInsCoID);

				//track if we just updated the insurance company we are editing on the edit ins. list
				if(nInsCoID == m_nCurInsCoID) {
					m_bChangesMadeToCurrentIns = TRUE;
				}

				long nOldCategoryID = VarLong(pRow->GetValue(ilcAdjCatID), -1);
				CString strOldCategoryName = VarString(pRow->GetValue(ilcAdjCatName), "");

				if(nNewCategoryID != nOldCategoryID) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strInsName, nAuditTransactionID, aeiInsCoDefAdjustmentCategory, nInsCoID, strOldCategoryName, strNewCategoryName, aepMedium, aetChanged);
				}
			}
			else {
				AddStatementToSqlBatch(strSqlBatch, "UPDATE InsuranceCoT SET DefaultPayCategoryID = %s WHERE PersonID = %li", strNewCategoryID, nInsCoID);

				//track if we just updated the insurance company we are editing on the edit ins. list
				if(nInsCoID == m_nCurInsCoID) {
					m_bChangesMadeToCurrentIns = TRUE;
				}

				long nOldCategoryID = VarLong(pRow->GetValue(ilcPayCatID), -1);
				CString strOldCategoryName = VarString(pRow->GetValue(ilcPayCatName), "");

				if(nNewCategoryID != nOldCategoryID) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strInsName, nAuditTransactionID, aeiInsCoDefPaymentCategory, nInsCoID, strOldCategoryName, strNewCategoryName, aepMedium, aetChanged);
				}
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		//now we have to update the datalist contents
		{
			IRowSettingsPtr pRow = m_SelectedList->GetFirstRow();
			while(pRow) {

				if(bIsAdjustment) {
					if(nNewCategoryID == -1) {
						pRow->PutValue(ilcAdjCatID, g_cvarNull);
					}
					else {
						pRow->PutValue(ilcAdjCatID, nNewCategoryID);
					}
					pRow->PutValue(ilcAdjCatName, _bstr_t(strNewCategoryName));
				}
				else {
					if(nNewCategoryID == -1) {
						pRow->PutValue(ilcPayCatID, g_cvarNull);
					}
					else {
						pRow->PutValue(ilcPayCatID, nNewCategoryID);
					}
					pRow->PutValue(ilcPayCatName, _bstr_t(strNewCategoryName));
				}

				pRow = pRow->GetNextRow();
			}
		}

		AfxMessageBox("Category update complete.");

	}NxCatchAllCall("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnApplyCategory",

		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CAdvInsurancePayDescriptionSetupDlg::OnRadioAdvPayDesc() 
{
	try {

		if(m_radioAdjustment.GetCheck()) {
			m_nxstaticCategoryLabel.SetWindowText("Default Adjustment Category");
			m_nxstaticDescriptionLabel.SetWindowText("Default Adjustment Description");
		}
		else {
			m_nxstaticCategoryLabel.SetWindowText("Default Payment Category");
			m_nxstaticDescriptionLabel.SetWindowText("Default Payment Description");
		}

		//clear out the category and description
		m_CategoryCombo->PutCurSel(NULL);
		m_nxeditDescription.SetWindowText("");

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnRadioAdvPayDesc");
}

void CAdvInsurancePayDescriptionSetupDlg::OnRadioAdvAdjDesc() 
{
	try {

		OnRadioAdvPayDesc();

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnRadioAdvAdjDesc");
}

BEGIN_EVENTSINK_MAP(CAdvInsurancePayDescriptionSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvInsurancePayDescriptionSetupDlg)
	ON_EVENT(CAdvInsurancePayDescriptionSetupDlg, IDC_ADV_INS_DESC_UNSELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellAdvInsDescUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAdvInsurancePayDescriptionSetupDlg, IDC_ADV_INS_DESC_SELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellAdvInsDescSelectedList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvInsurancePayDescriptionSetupDlg::OnDblClickCellAdvInsDescUnselectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_UnselectedList->PutCurSel(pRow);
		OnBtnAdvInsDescSelectOne();

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnDblClickCellAdvInsDescUnselectedList");
}

void CAdvInsurancePayDescriptionSetupDlg::OnDblClickCellAdvInsDescSelectedList(LPDISPATCH lpRow, short nColIndex) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_SelectedList->PutCurSel(pRow);
		OnBtnAdvInsDescUnselectOne();

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnDblClickCellAdvInsDescSelectedList");
}

void CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescClose() 
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in CAdvInsurancePayDescriptionSetupDlg::OnBtnAdvInsDescClose");
}
