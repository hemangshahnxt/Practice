// ConfigureEMRTabsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "ConfigureEMRTabsDlg.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "client.h"
#include "Color.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CConfigureEMRTabsDlg dialog

enum EMNCategoriesListColumns
{
	eclcCheck = 0, // (z.manning, 04/04/2007) - PLID 25485 - Added a new column for the checkbox to determine if category should be associated with currently selected chart.
	eclcID,
	eclcDescription,
	eclcPriority,
};

enum EMNChartListColumns
{
	chartID = 0,
	chartDescription,
	chartPriority,
	chartLocation, // (z.manning 2011-05-18 09:33) - PLID 43759
	chartColor,
	chartColorData,
};

// (z.manning 2008-06-30 14:58) - PLID 25574 - Added history category list
enum HistoryCategoryListColumns
{
	hclcCheck = 0,
	hclcID,
	hclcDescription,
};


CConfigureEMRTabsDlg::CConfigureEMRTabsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureEMRTabsDlg::IDD, pParent)
{
	m_bChartOrderChanged = FALSE;
	m_bCatOrderChanged = FALSE;
	m_bLinksChanged = FALSE;
	//{{AFX_DATA_INIT(CConfigureEMRTabsDlg)
	//}}AFX_DATA_INIT
}


void CConfigureEMRTabsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureEMRTabsDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_MOVE_EMN_CAT_PRIORITY_UP_BTN, m_btnMoveEMNCatPriorityUp);
	DDX_Control(pDX, IDC_MOVE_EMN_CAT_PRIORITY_DOWN_BTN, m_btnMoveEMNCatPriorityDown);
	DDX_Control(pDX, IDC_DELETE_EMN_CATEGORY, m_btnDeleteEMNCategory);
	DDX_Control(pDX, IDC_ADD_EMN_CATEGORY, m_btnAddEMNCategory);
	DDX_Control(pDX, IDC_MOVE_EMN_CHART_PRIORITY_UP_BTN, m_btnMoveEMNChartPriorityUp);
	DDX_Control(pDX, IDC_MOVE_EMN_CHART_PRIORITY_DOWN_BTN, m_btnMoveEMNChartPriorityDown);
	DDX_Control(pDX, IDC_DELETE_EMN_CHART_TYPE, m_btnDeleteEMNChart);
	DDX_Control(pDX, IDC_ADD_EMN_CHART_TYPE, m_btnAddEMNChart);
	DDX_Control(pDX, IDC_HISTORY_CAT_LABEL, m_nxstaticHistoryCatLabel);
	DDX_Control(pDX, IDC_CONFIG_EMR_TABS_BACKGROUND, m_nxcolorBackground);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureEMRTabsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureEMRTabsDlg)
	ON_BN_CLICKED(IDC_ADD_EMN_CATEGORY, OnAddEmnCategory)
	ON_BN_CLICKED(IDC_DELETE_EMN_CATEGORY, OnDeleteEmnCategory)
	ON_BN_CLICKED(IDC_MOVE_EMN_CAT_PRIORITY_DOWN_BTN, OnMoveEmnCatPriorityDownBtn)
	ON_BN_CLICKED(IDC_MOVE_EMN_CAT_PRIORITY_UP_BTN, OnMoveEmnCatPriorityUpBtn)
	ON_BN_CLICKED(IDC_ADD_EMN_CHART_TYPE, OnAddEmnChart)
	ON_BN_CLICKED(IDC_DELETE_EMN_CHART_TYPE, OnDeleteEmnChart)
	ON_BN_CLICKED(IDC_MOVE_EMN_CHART_PRIORITY_DOWN_BTN, OnMoveEmnChartPriorityDownBtn)
	ON_BN_CLICKED(IDC_MOVE_EMN_CHART_PRIORITY_UP_BTN, OnMoveEmnChartPriorityUpBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CConfigureEMRTabsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureEMRTabsDlg)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_TAB_CAT_LIST, 2 /* SelChanged */, OnSelChangedEMNCategoryList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_TAB_CAT_LIST, 10 /* EditingFinished */, OnEditingFinishedEMNCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_TAB_CAT_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmnTabCatList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_CHART_TYPES, 2 /* SelChanged */, OnSelChangedEMNChartList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_CHART_TYPES, 10 /* EditingFinished */, OnEditingFinishedEMNCharts, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_CHART_TYPES, 9 /* EditingFinishing */, OnEditingFinishingEmnTabChartList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_CHART_TYPES, 7 /* RButtonUp */, OnRButtonUpEmnChartTypes, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_CHART_TYPES, 18 /* RequeryFinished */, OnRequeryFinishedEmnChartTypes, VTS_I2)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_EMN_CHART_TYPES, 5 /* LButtonUp */, OnLButtonUpEmnChartTypes, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CConfigureEMRTabsDlg, IDC_HISTORY_CAT_LIST, 10 /* EditingFinished */, OnEditingFinishedHistoryCategories, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureEMRTabsDlg message handlers

BOOL CConfigureEMRTabsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-05-01 10:44) - PLID 29863 - NxIconify buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnDeleteEMNCategory.AutoSet(NXB_DELETE);
		m_btnAddEMNCategory.AutoSet(NXB_NEW);
		m_btnDeleteEMNChart.AutoSet(NXB_DELETE);
		m_btnAddEMNChart.AutoSet(NXB_NEW);

		// (z.manning 2011-05-18 09:26) - PLID 43756 - Added an NxColor control to this dialog
		// (b.spivey, May 21, 2012) - PLID 50558 - We use the default patient blue always.
		m_nxcolorBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1)); 

		// Connect our variable to the control
		m_EMNTabCategoriesList = BindNxDataList2Ctrl(this, IDC_EMN_TAB_CAT_LIST, GetRemoteData(), false);
		m_EMNTabCategoriesList->GetColumn(eclcCheck)->PutEditable(VARIANT_TRUE);
		m_pdlEmnCharts = BindNxDataList2Ctrl(this, IDC_EMN_CHART_TYPES, GetRemoteData(), true);
		m_pdlHistoryCategories = BindNxDataList2Ctrl(IDC_HISTORY_CAT_LIST, GetRemoteData(), true);

		m_EMNTabCategoriesList->Requery();

		m_btnMoveEMNCatPriorityUp.AutoSet(NXB_UP);
		m_btnMoveEMNCatPriorityDown.AutoSet(NXB_DOWN);
		m_btnMoveEMNChartPriorityUp.AutoSet(NXB_UP);
		m_btnMoveEMNChartPriorityDown.AutoSet(NXB_DOWN);

		UpdateView();

	} NxCatchAll("CConfigureEMRTabsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureEMRTabsDlg::OnAddEmnCategory() 
{
	try {
		CString strNewName;
		// (z.manning, 04/09/2007) - PLID 25535 - The length should be limited to 50, not 255.
		if(IDOK == InputBoxLimited(this, "Enter a name for the new EMR Tab Category", strNewName, "",50,false,false,NULL)) {
			strNewName.TrimLeft();
			strNewName.TrimRight();
			if(strNewName.IsEmpty()) {
				MsgBox("You cannot have a EMR Tab Category with a blank name.");
				return;
			}
			// (a.wetta 2007-01-23 14:35) - PLID 14635 - Let's make sure they aren't trying to create a duplicate tab name
			else if(ReturnsRecords("SELECT ID FROM EMNTabCategoriesT WHERE Description = '%s'", _Q(strNewName))) {
				MsgBox("There is already an EMR Tab Category with the name '%s'", strNewName);
				return;
			}

			//Add a new row into the list
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_EMNTabCategoriesList->GetNewRow();

			long nNewID = NewNumber("EMNTabCategoriesT", "ID");
			long nNewPriority = NewNumber("EMNTabCategoriesT", "Priority");
			pRow->PutValue(eclcCheck, VARIANT_FALSE);
			pRow->PutValue(eclcID, (long)nNewID);
			pRow->PutValue(eclcDescription, _variant_t(strNewName));  //name
			pRow->PutValue(eclcPriority, (long)nNewPriority);
			
			ExecuteSql("INSERT INTO EMNTabCategoriesT  (ID, Description, Priority) VALUES (%li, '%s', %li)", nNewID, _Q(strNewName), nNewPriority);

			// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
			CClient::RefreshTable(NetUtils::EMNTabCategoriesT, nNewID);
			
			m_EMNTabCategoriesList->AddRowAtEnd(pRow, NULL);
			m_EMNTabCategoriesList->SetSelByColumn(eclcID, nNewID);

			UpdateView();
		}
	}NxCatchAll("CConfigureEMRTabsDlg::OnAddEmnCategory");	
}

void CConfigureEMRTabsDlg::OnDeleteEmnCategory() 
{	
	long nAuditID = -1;
	try
	{
		//check to see that something is selected
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_EMNTabCategoriesList->GetCurSel();
		if (pRow != NULL)
		{
			if (IDYES == MessageBox("Are you sure you want to delete this category?", "NexTech Practice", MB_YESNO|MB_ICONQUESTION)) {
	
				long nDelID = VarLong(pRow->GetValue(eclcID));
				CString strCategoryName = VarString(pRow->GetValue(eclcDescription));
				
				_RecordsetPtr rsEmn;
				rsEmn = CreateRecordset(
					"SELECT EmnID, EmrMasterT.PatientID \r\n"
					"FROM EMNTabCategoriesLinkT  \r\n"
					"LEFT JOIN EmrMasterT ON EmnTabCategoriesLinkT.EmnID = EmrMasterT.ID  \r\n"
					"WHERE EMNTabCategoryID = %li AND EmrMasterT.Deleted = 0"
					, nDelID);
				// (z.manning, 04/19/2007) - PLID 25714 - Need to keep track of all the EMNs we remove the category
				// for so we can audit later.
				CArray<long,long> arynEmnIDs;
				// (a.walling 2008-08-20 13:02) - PLID 29900 - We also need to keep track of the patient IDs so we can
				// audit appropriately
				CArray<long, long> arynPatIDs;
				while(!rsEmn->eof) {
					arynEmnIDs.Add(AdoFldLong(rsEmn, "EmnID"));
					arynPatIDs.Add(AdoFldLong(rsEmn, "PatientID"));
					rsEmn->MoveNext();
				}
				rsEmn->Close();

				_RecordsetPtr rsTemplate = CreateRecordset(
					"SELECT EmrTemplateT.ID FROM EmrTemplateT WHERE EmnTabCategoryID = %li", nDelID);
				CArray<long,long> arynTemplateIDs;
				while(!rsTemplate->eof) {
					arynTemplateIDs.Add(AdoFldLong(rsTemplate, "ID"));
					rsTemplate->MoveNext();
				}
				rsTemplate->Close();

				if (arynEmnIDs.GetSize() > 0 || arynTemplateIDs.GetSize()) {
					//we can't delete
					// (z.manning, 04/05/2007) - PLID 25515 - Correction, we CAN delete. But let's warn them about it.
					CString strWarning = FormatString("There are %li EMNs and %li EMN templates associated with the '%s' category.\r\n\r\nAre you sure you want to delete this category?"
						, arynEmnIDs.GetSize(), arynTemplateIDs.GetSize(), VarString(pRow->GetValue(eclcDescription),""));
					if(IDYES != MessageBox(strWarning, "Confirm Deletion", MB_YESNO)) {
						return;
					}
				}

				
				//we can delete it!
				// (z.manning, 04/04/2007) - PLID 25485 - Make sure we delete from the category-chart linking as well.
				// (z.manning, 04/05/2007) - PLID 25515 - Also delete from the EMN-category linking table.
				// (z.manning 2008-07-03 10:51) - PLID 25574 - Delete from EmrHistoryCategoryLinkT
				CString strSql = BeginSqlBatch();
				AddStatementToSqlBatch(strSql, "UPDATE EmrTemplateT SET EmnTabCategoryID = NULL WHERE EmnTabCategoryID = %li", nDelID);
				AddStatementToSqlBatch(strSql, "DELETE FROM EmrHistoryCategoryLinkT WHERE EmnTabCategoryID = %li", nDelID);
				AddStatementToSqlBatch(strSql, "DELETE FROM EmnTabCategoriesLinkT WHERE EmnTabCategoryID = %li", nDelID);
				AddStatementToSqlBatch(strSql, "DELETE FROM EmnTabChartCategoryLinkT WHERE EmnTabCategoryID = %li", nDelID);
				AddStatementToSqlBatch(strSql, "DELETE FROM EMNTabCategoriesT WHERE ID = %li", nDelID);
				ExecuteSqlBatch(strSql);
				m_EMNTabCategoriesList->RemoveRow(pRow);

				// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
				CClient::RefreshTable(NetUtils::EMNTabCategoriesT, nDelID);

				// (z.manning, 04/19/2007) - PLID 25714 - If we deleted a category that was associated with EMNs,
				// audit the EMN record.
				if(arynEmnIDs.GetSize() > 0) {
					if(nAuditID == -1) { nAuditID = BeginAuditTransaction(); }
					for(int i = 0; i < arynEmnIDs.GetSize(); i++) {
						// (a.walling 2008-08-20 13:04) - PLID 29900 - Audit with appropriate patient names
						AuditEvent(arynPatIDs.GetAt(i), GetExistingPatientName(arynPatIDs.GetAt(i)), nAuditID, aeiEMNCategory, arynEmnIDs.GetAt(i), strCategoryName, "{No Category}", aepMedium, aetChanged);
					}
				}

				// (z.manning, 04/20/2007) - PLID 25731 - If we deleted a category associated with templates, audit it.
				if(arynTemplateIDs.GetSize() > 0) {
					if(nAuditID == -1) { nAuditID = BeginAuditTransaction(); }
					for(int i = 0; i < arynTemplateIDs.GetSize(); i++) {
						AuditEvent(-1, "", nAuditID, aeiEMNTemplateCategory, arynTemplateIDs.GetAt(i), strCategoryName, "{No Category}", aepMedium, aetChanged);
					}
				}

				if(nAuditID != -1) {
					CommitAuditTransaction(nAuditID);
					nAuditID = -1;
				}

				UpdateView();
			}
			else {
				//they don't want to delete, so do nothing
			}
		}
		else {
			MessageBox("Please select a category to delete");
		}
			
	}NxCatchAllCall("Error in CConfigureEMRTabsDlg::OnDeleteEmnCategory()", if(nAuditID != -1) { RollbackAuditTransaction(nAuditID); });
	
}

void CConfigureEMRTabsDlg::OnMoveEmnCatPriorityDownBtn() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow1 = m_EMNTabCategoriesList->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pRow2 = pRow1->GetNextRow();
	
	SwapEMNCatPriorities(pRow1, pRow2);

	UpdatePriorityButtons();
}

void CConfigureEMRTabsDlg::OnMoveEmnCatPriorityUpBtn() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow1 = m_EMNTabCategoriesList->GetCurSel();
	NXDATALIST2Lib::IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
	
	SwapEMNCatPriorities(pRow1, pRow2);

	UpdatePriorityButtons();
}

// Returns the category ID referred to by nRow1
void CConfigureEMRTabsDlg::SwapEMNCatPriorities(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2)
{
	try {
		// Get the priority values
		long nPriority1 = VarLong(pRow1->GetValue(eclcPriority));
		long nPriority2 = VarLong(pRow2->GetValue(eclcPriority));

		// Swap the values
		pRow1->PutValue(eclcPriority, _variant_t(nPriority2));
		pRow2->PutValue(eclcPriority, _variant_t(nPriority1));

		// Update the database
		ExecuteSql("UPDATE EMNTabCategoriesT SET Priority = %li WHERE ID = %li",
			nPriority2, VarLong(pRow1->GetValue(eclcID)));

		ExecuteSql("UPDATE EMNTabCategoriesT SET Priority = %li WHERE ID = %li",
			nPriority1, VarLong(pRow2->GetValue(eclcID)));

		m_bCatOrderChanged = TRUE;
		
		// Sort the list
		m_EMNTabCategoriesList->Sort();

		// Make sure row 1 is still selected
		m_EMNTabCategoriesList->SetSelByColumn(eclcID, pRow1->GetValue(eclcID));
	}NxCatchAll("Error in CConfigureEMRTabsDlg::SwapEMNCatPriorities");
}

void CConfigureEMRTabsDlg::OnSelChangedEMNCategoryList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		UpdatePriorityButtons();
		UpdateHistoryCategories();

	}NxCatchAll("Error in CConfigureEMRTabsDlg::OnSelChangedEMNCategoryList");
}

void CConfigureEMRTabsDlg::UpdatePriorityButtons()
{
	try {
		IRowSettingsPtr pCurRow = m_EMNTabCategoriesList->GetCurSel();
		if (pCurRow == NULL) {
			GetDlgItem(IDC_MOVE_EMN_CAT_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_EMN_CAT_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
		}
		else {
			if (pCurRow->GetPreviousRow() == NULL) {
				GetDlgItem(IDC_MOVE_EMN_CAT_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_EMN_CAT_PRIORITY_UP_BTN)->EnableWindow(TRUE);
			}

			if (pCurRow->GetNextRow() == NULL) {
				GetDlgItem(IDC_MOVE_EMN_CAT_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_EMN_CAT_PRIORITY_DOWN_BTN)->EnableWindow(TRUE);
			}
		}

		// (z.manning, 04/03/2007) - PLID 25485 - Update the priority buttons for the chart list based on the current selection.
		pCurRow = m_pdlEmnCharts->GetCurSel();
		if (pCurRow == NULL) {
			GetDlgItem(IDC_MOVE_EMN_CHART_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_EMN_CHART_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
		}
		else {
			if (pCurRow->GetPreviousRow() == NULL) {
				GetDlgItem(IDC_MOVE_EMN_CHART_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_EMN_CHART_PRIORITY_UP_BTN)->EnableWindow(TRUE);
			}

			if (pCurRow->GetNextRow() == NULL) {
				GetDlgItem(IDC_MOVE_EMN_CHART_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_EMN_CHART_PRIORITY_DOWN_BTN)->EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CConfigureEMRTabsDlg::UpdatePriorityButtons");
}

void CConfigureEMRTabsDlg::OnEditingFinishedEMNCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if(lpRow == NULL || !bCommit) return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		switch(nCol)
		{
			case eclcDescription:
			{
				long lID = VarLong(pRow->GetValue(eclcID));
				CString strNewValue = VarString(varNewValue);
				strNewValue.TrimLeft();
				strNewValue.TrimRight();
				ExecuteSql("UPDATE EMNTabCategoriesT SET Description = '%s' WHERE ID = %li",
					_Q(strNewValue), lID);

				// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
				CClient::RefreshTable(NetUtils::EMNTabCategoriesT, lID);
			}
			break;

			case eclcCheck:
			{
				IRowSettingsPtr pChartRow = m_pdlEmnCharts->CurSel;
				if(pChartRow == NULL) {
					ASSERT(FALSE);
					return;
				}
				long nChartID = VarLong(pChartRow->GetValue(chartID));
				long nCategoryID = VarLong(pRow->GetValue(eclcID));
				if(VarBool(varNewValue)) {
					// (z.manning, 04/04/2007) - PLID 25485 - They added an association, so insert it into data.
					ExecuteSql("INSERT INTO EmnTabChartCategoryLinkT (EmnTabChartID, EmnTabCategoryID) VALUES (%li, %li) "
						, nChartID, nCategoryID);
				}
				else {
					// (z.manning, 04/04/2007) - PLID 25485 - They removed an association, so delete it from data.
					ExecuteSql("DELETE FROM EmnTabChartCategoryLinkT WHERE EmnTabChartID = %li AND EmnTabCategoryID = %li"
						, nChartID, nCategoryID);
				}

					
				// Well, since the association changes never affect saved data, and it won't affect an EMN save
				// (unlike a deleted chart/category), so we should not be too concerned about it. Just send
				// the table checker when the dialog closes rather than every single time a box is checked/unchecked.

				// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
				m_bLinksChanged = TRUE;
			}
			break;
		}

	}NxCatchAll("CConfigureEMRTabsDlg::OnEditingFinishedEMNCategories");
}

void CConfigureEMRTabsDlg::OnEditingFinishingEmnTabCatList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(lpRow == NULL || !*pbCommit) return;

		IRowSettingsPtr pRow(lpRow);

		switch(nCol)
		{
			case eclcDescription:
			{
				CString strNewValue = VarString(*pvarNewValue,"");
				strNewValue.TrimLeft();
				strNewValue.TrimRight();
				if (VarString(varOldValue) != strNewValue) {
					if(strNewValue.IsEmpty()) {
						// (z.manning, 04/04/2007) - PLID 25535 - Don't allow blank category names.
						MessageBox("Blank category names are not allowed.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
					// (a.wetta 2007-01-23 14:35) - PLID 14635 - Let's make sure they aren't trying to create a duplicate tab name
					else if(ReturnsRecords("SELECT ID FROM EMNTabCategoriesT WHERE Description = '%s'", _Q(strNewValue))) {
						MsgBox("There is already an EMR Tab Category with the name '%s'", strNewValue);		
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
					else {
						//TES 8/7/2007 - PLID 26986 - If this is in use, warn the user (but they can still rename it if they want).
						IRowSettingsPtr pRow(lpRow);
						_RecordsetPtr rsUseCount = CreateRecordset("SELECT Count(EmnID) AS UseCount FROM EMNTabCategoriesLinkT "
							"WHERE EMNTabCategoryID = %li", VarLong(pRow->GetValue(eclcID)));
						long nUseCount = AdoFldLong(rsUseCount, "UseCount", 0);
						if(nUseCount > 0) {
							if(IDYES == MsgBox(MB_YESNO, "There are %li EMNs assigned to this category.  If you continue, the category "
								"for all of these EMNs will be renamed from \"%s\" to \"%s\".  Are you sure you wish to do this?",
								nUseCount, VarString(varOldValue), strNewValue)) {
								*pbCommit = TRUE;
								*pbContinue = TRUE;
							}
							else {
								*pbCommit = FALSE;
								*pbContinue = FALSE;
							}
						}
						else {
							*pbCommit = TRUE;
							*pbContinue = TRUE;
						}
					}
				}
				else {
					*pbCommit = FALSE;
					*pbContinue = TRUE;
				}
			}
			break;

			case eclcCheck:
			{
				// (z.manning, 05/23/2007) - PLID 25489 - If they unchecked an assocation, check and see if this
				// chart/category combination is used on any EMNs or EMN templates. If so, do not allow them to
				// uncheck it.
				if(!VarBool(pvarNewValue)) {
					long nChartID = VarLong(m_pdlEmnCharts->GetCurSel()->GetValue(chartID), -1);
					if(nChartID != -1) {
						long nCategoryID = VarLong(pRow->GetValue(eclcID));
						BOOL bChartCategoryComboIsUsed = ReturnsRecords(
							"SELECT EmrMasterT.ID FROM EmrMasterT "
							"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
							"LEFT JOIN EmnTabCategoriesLinkT ON EmrMasterT.ID = EmnTabCategoriesLinkT.EmnID "
							"WHERE EmnTabCategoryID = %li AND EmnTabChartID = %li "
							"UNION "
							"SELECT EmrTemplateT.ID FROM EmrTemplateT "
							"WHERE EmnTabCategoryID = %li AND EmnTabChartID = %li "
							, nCategoryID, nChartID, nCategoryID, nChartID);
						if(bChartCategoryComboIsUsed) {
							MessageBox("This category/chart association is in use on EMNs, so you may not uncheck it.");
							*pbCommit = FALSE;
						}
					}
				}
			}
			break;
		}

	}NxCatchAll("Error in CConfigureEMRTabsDlg::OnEditingFinishingEmnTabCatList");
	
}

void CConfigureEMRTabsDlg::OnAddEmnChart() 
{
	try {
		CString strNewName;
		if(IDOK == InputBoxLimited(this, "Enter a name for the new EMN Chart Type", strNewName,"",50,false,false,NULL)) 
		{
			strNewName.TrimLeft();
			strNewName.TrimRight();
			if(strNewName.IsEmpty()) {
				MessageBox("You cannot have a EMN Chart Type with a blank name.");
				return;
			}
			// (z.manning, 04/03/2007) - PLID 25485 - Let's make sure they aren't trying to create a duplicate tab name
			else if(ReturnsRecordsParam("SELECT ID FROM EmnTabChartsT WHERE Description = {STRING}", strNewName)) {
				MessageBox(FormatString("There is already an EMN Tab Chart Type with the name '%s'", strNewName));
				return;
			}

			// (z.manning, 04/04/2007) - PLID 25485 - Add the new row to the list and to data.
			IRowSettingsPtr pRow;
			pRow = m_pdlEmnCharts->GetNewRow();
			long nNewID = NewNumber("EmnTabChartsT", "ID");
			long nNewPriority = NewNumber("EmnTabChartsT", "Priority");
			pRow->PutValue(chartID, (long)nNewID);
			pRow->PutValue(chartDescription, _variant_t(strNewName));
			pRow->PutValue(chartPriority, (long)nNewPriority);
			pRow->PutValue(chartLocation, g_cvarNull); // (z.manning 2011-05-18 09:51) - PLID 43756
			// (a.wetta 2007-06-29 11:37) - PLID 26094 - Set the chart's color
			pRow->PutCellBackColor(chartColor, (long)pRow->GetCellBackColor(chartColorData));
			pRow->PutCellBackColorSel(chartColor, (long)pRow->GetCellBackColorSel(chartColorData));
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->PutValue(chartColorData, varNull);
			
			// (z.manning 2011-05-18 17:37) - PLID 43767 - We now have permissions for each EMR chart so we need to create
			// a new permission object for the new chart. Let's also run this in a transaction to ensure valid data.
			CSqlTransaction sqlTran;
			sqlTran.Begin();
			ExecuteParamSql(
				"INSERT INTO EmnTabChartsT (ID, Description, Priority) VALUES ({INT}, {STRING}, {INT})"
				, nNewID, strNewName, nNewPriority);
			// (z.manning 2011-05-19 11:48) - PLID 43767 - EMR charts only have a view/access permission. Give everyone this permission
			// by default. They're still, of course, subject to normal EMR permissions to be able to access EMR at all.
			// (z.manning 2013-10-18 09:59) - PLID 59082 - Use the global define for description
			long nNewSecurityObjectID = AddUserDefinedPermissionToData(nNewID, sptView, strNewName, PERMISSION_DESC_EMN_CHART, bioEmrCharts, sptView);
			sqlTran.Commit();
			AddUserDefinedPermissionToMemory(nNewSecurityObjectID, nNewID, sptView, strNewName, PERMISSION_DESC_EMN_CHART, bioEmrCharts, sptView);

			// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
			CClient::RefreshTable(NetUtils::EMNTabChartsT, nNewID);
			
			m_pdlEmnCharts->AddRowAtEnd(pRow, NULL);
			m_pdlEmnCharts->SetSelByColumn(chartID, nNewID);

			UpdateView();
		}
	}NxCatchAll("CConfigureEMRTabsDlg::OnAddEmnChart");
}

void CConfigureEMRTabsDlg::OnDeleteEmnChart()
{
	long nAuditID = -1;
	try {

		//check to see that something is selected
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlEmnCharts->GetCurSel();
	
		if (pRow != NULL) 
		{
			if (IDYES == MessageBox("Are you sure you want to delete this chart type?", "Confirm Delete", MB_YESNO|MB_ICONQUESTION)) 
			{	
				long nDelID = VarLong(pRow->GetValue(chartID));
				CString strChartName = VarString(pRow->GetValue(chartDescription));
				
				_RecordsetPtr rsEmn = CreateParamRecordset(
					"SELECT EmnID, EmrMasterT.PatientID  \r\n"
					"FROM EmnTabChartsLinkT  \r\n"
					"LEFT JOIN EmrMasterT ON EmnTabChartsLinkT.EmnID = EmrMasterT.ID  \r\n"
					"WHERE EmnTabChartID = {INT} AND EmrMasterT.Deleted = 0 "
					, nDelID);
				// (z.manning, 04/19/2007) - PLID 25714 - Save the EMNs whose chart we'll be lost so we can audit that fact.
				CArray<long,long> arynEmnIDs;
				// (a.walling 2008-08-20 13:02) - PLID 29900 - We also need to keep track of the patient IDs so we can
				// audit appropriately
				CArray<long, long> arynPatIDs;
				while(!rsEmn->eof) {
					arynEmnIDs.Add(AdoFldLong(rsEmn, "EmnID"));
					arynPatIDs.Add(AdoFldLong(rsEmn, "PatientID"));
					rsEmn->MoveNext();
				}
				rsEmn->Close();

				_RecordsetPtr rsTemplate = CreateParamRecordset(
					"SELECT EmrTemplateT.ID FROM EmrTemplateT WHERE EmnTabChartID = {INT}", nDelID);
				CArray<long,long> arynTemplateIDs;
				while(!rsTemplate->eof) {
					arynTemplateIDs.Add(AdoFldLong(rsTemplate, "ID"));
					rsTemplate->MoveNext();
				}
				rsTemplate->Close();

				if(arynEmnIDs.GetSize() > 0 || arynTemplateIDs.GetSize()) {
					// (z.manning, 04/05/2007) - PLID 25515 - We allow deletion even if the charts have associated
					// EMNs, but let's warn them again jus to be safe.
					CString strWarning = FormatString("There are %li EMNs and %li EMN templates associated with chart type '%s'.\r\n\r\nAre you sure you want to delete this chart type?."
						, arynEmnIDs.GetSize(), arynTemplateIDs.GetSize(), VarString(pRow->GetValue(chartDescription),""));
					if(IDYES != MessageBox(strWarning, "Confirm Deletion", MB_YESNO)) {
						return;
					}
				}
				
				// (z.manning, 04/04/2007) - PLID 25485 - Ok to delete.
				// (z.manning, 04/05/2007) - PLID 25515 - Also delete from the table that links charts to EMNs.
				// (z.manning 2011-05-18 17:37) - PLID 43767 - We now have permissions for each EMR chart so we need to delete
				// the permission object for the deleted chart. Let's also run this in a transaction to ensure valid data.
				CSqlTransaction sqlTran;
				sqlTran.Begin();
				ExecuteParamSql(
					"DECLARE @nChartID int \r\n"
					"SET @nChartID = {INT} \r\n"
					"UPDATE EmrTemplateT SET EmnTabChartID = NULL WHERE EmnTabChartID = @nChartID \r\n"
					"DELETE FROM EmnTabChartsLinkT WHERE EmnTabChartID = @nChartID \r\n"
					"DELETE FROM EmnTabChartCategoryLinkT WHERE EmnTabChartID = @nChartID \r\n"
					"DELETE FROM EmnTabChartsT WHERE ID = @nChartID \r\n"
					, nDelID);
				long nSecurityObjectID = DeleteUserDefinedPermissionFromData(bioEmrCharts, nDelID);
				sqlTran.Commit();
				DeleteUserDefinedPermissionFromMemory(nSecurityObjectID);

				m_pdlEmnCharts->RemoveRow(pRow);

				// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
				CClient::RefreshTable(NetUtils::EMNTabChartsT, nDelID);

				// (z.manning, 04/19/2007) - PLID 25714 - If we deleted a chart that was associated with EMNs,
				// audit the EMN record.
				if(arynEmnIDs.GetSize() > 0) {
					if(nAuditID == -1) { nAuditID = BeginAuditTransaction(); }
					for(int i = 0; i < arynEmnIDs.GetSize(); i++) {
						// (a.walling 2008-08-20 13:04) - PLID 29900 - Audit with appropriate patient names
						AuditEvent(arynPatIDs.GetAt(i), GetExistingPatientName(arynPatIDs.GetAt(i)), nAuditID, aeiEMNChart, arynEmnIDs.GetAt(i), strChartName, "{No Chart}", aepMedium, aetChanged);
					}
				}

				// (z.manning, 04/20/2007) - PLID 25731 - If we deleted a chart associated with templates, audit it.
				if(arynTemplateIDs.GetSize() > 0) {
					if(nAuditID == -1) { nAuditID = BeginAuditTransaction(); }
					for(int i = 0; i < arynTemplateIDs.GetSize(); i++) {
						AuditEvent(-1, "", nAuditID, aeiEMNTemplateChart, arynTemplateIDs.GetAt(i), strChartName, "{No Chart}", aepMedium, aetChanged);
					}
				}

				if(nAuditID != -1) {
					CommitAuditTransaction(nAuditID);
					nAuditID = -1;
				}

				UpdateView();
			}
			else {
				//they don't want to delete, so do nothing
			}
		}
		else {
			MessageBox("Please select a chart type to delete");
		}

	}NxCatchAllCall("CConfigureEMRTabsDlg::OnDeleteEmnChart", if(nAuditID != -1) { RollbackAuditTransaction(nAuditID); });
}

// (z.manning, 04/04/2007) - PLID 25485 - Moves the priority of the selected chart up one.
void CConfigureEMRTabsDlg::OnMoveEmnChartPriorityDownBtn() 
{
	try {

		IRowSettingsPtr pRow1 = m_pdlEmnCharts->GetCurSel();
		IRowSettingsPtr pRow2 = pRow1->GetNextRow();
		
		SwapEmnChartPriorities(pRow1, pRow2);

		UpdatePriorityButtons();

	}NxCatchAll("CConfigureEMRTabsDlg::OnMoveEmnChartPriorityDownBtn");
}

// (z.manning, 04/04/2007) - PLID 25485 - Moves the priority of the selected chart down one.
void CConfigureEMRTabsDlg::OnMoveEmnChartPriorityUpBtn() 
{
	try {

		IRowSettingsPtr pRow1 = m_pdlEmnCharts->GetCurSel();
		IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
		
		SwapEmnChartPriorities(pRow1, pRow2);

		UpdatePriorityButtons();

	}NxCatchAll("CConfigureEMRTabsDlg::OnMoveEmnChartPriorityUpBtn");
}

// (z.manning, 04/04/2007) - PLID 25485 - Given 2 rows, swaps their priorties in both the list and in data.
void CConfigureEMRTabsDlg::SwapEmnChartPriorities(IRowSettingsPtr pRow1, IRowSettingsPtr pRow2)
{
	// Get the priority values
	long nPriority1 = VarLong(pRow1->GetValue(chartPriority));
	long nPriority2 = VarLong(pRow2->GetValue(chartPriority));

	// Swap the values
	pRow1->PutValue(chartPriority, _variant_t(nPriority2));
	pRow2->PutValue(chartPriority, _variant_t(nPriority1));

	// Update the database
	ExecuteSql("UPDATE EmnTabChartsT SET Priority = %li WHERE ID = %li",
		nPriority2, VarLong(pRow1->GetValue(chartID)));

	ExecuteSql("UPDATE EmnTabChartsT SET Priority = %li WHERE ID = %li",
		nPriority1, VarLong(pRow2->GetValue(chartID)));

	m_bChartOrderChanged = TRUE;
	
	// Sort the list
	m_pdlEmnCharts->Sort();

	// Make sure row 1 is still selected
	m_pdlEmnCharts->SetSelByColumn(chartID, pRow1->GetValue(chartID));
}

void CConfigureEMRTabsDlg::OnSelChangedEMNChartList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		UpdateView();

	}NxCatchAll("CConfigureEMRTabsDlg::OnSelChangedEMNChartList");
}

void CConfigureEMRTabsDlg::OnEditingFinishedEMNCharts(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if(lpRow == NULL || !bCommit) return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long lID = VarLong(pRow->GetValue(chartID));

		switch(nCol)
		{
			case chartLocation:
				{
					// (z.manning 2011-05-18 09:53) - PLID 43756 - We can now have a default location for a chart
					long nOldLocation = VarLong(varOldValue, -1);
					long nNewLocation = VarLong(varNewValue, -1);
					if(nOldLocation != nNewLocation) {
						_variant_t varLocationID = (nNewLocation == -1 ? g_cvarNull : nNewLocation);
						ExecuteParamSql("UPDATE EmnTabChartsT SET LocationID = {VT_I4} WHERE ID = {INT} \r\n", varLocationID, lID);
					}
				}
				break;

			// (z.manning, 04/04/2007) - PLID 25485 - If we got here, we should be ok to update the description.
			case chartDescription:
				CString strNewValue = VarString(varNewValue);
				strNewValue.TrimLeft();
				strNewValue.TrimRight();
				// (z.manning 2011-05-19 11:27) - PLID 43767 - We also have to rename the permission for this chart.
				// Do that in a transaction to ensure valid data.
				CSqlTransaction sqlTran;
				sqlTran.Begin();
				ExecuteParamSql("UPDATE EMNTabChartsT SET Description = {STRING} WHERE ID = {INT} \r\n", strNewValue, lID);
				long nSecurityObjectID = UpdateUserDefinedPermissionNameInData(bioEmrCharts, lID, strNewValue);
				sqlTran.Commit();
				UpdateUserDefinedPermissionNameInMemory(nSecurityObjectID, strNewValue);

				// (a.walling 2007-07-03 12:53) - PLID 26545 - Refresh the table checker
				CClient::RefreshTable(NetUtils::EMNTabChartsT, lID);
				break;
		}

	}NxCatchAll("CConfigureEMRTabsDlg::OnEditingFinishedEMNCharts");
}

void CConfigureEMRTabsDlg::OnEditingFinishingEmnTabChartList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(lpRow == NULL || !*pbCommit) return;

		switch(nCol)
		{
			case chartDescription:
				CString strNewValue = VarString(*pvarNewValue,"");
				strNewValue.TrimLeft();
				strNewValue.TrimRight();
				if (VarString(varOldValue) != strNewValue) {
					if(strNewValue.IsEmpty()) {
						// (z.manning, 04/04/2007) - PLID 25485 - Don't allow blank chart names.
						MessageBox("Blank chart names are not allowed.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
					// (z.manning, 04/04/2007) - PLID 25485 - Don't allow duplicate chart names.
					else if(ReturnsRecords("SELECT ID FROM EMNTabChartsT WHERE Description = '%s'", _Q(strNewValue))) {
						MessageBox(FormatString("There is already an EMN Chart Type with the name '%s'", strNewValue));
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
					else {
						//TES 8/7/2007 - PLID 26986 - If this is in use, warn the user (but they can still rename it if they want).
						IRowSettingsPtr pRow(lpRow);
						_RecordsetPtr rsUseCount = CreateRecordset("SELECT Count(EmnID) AS UseCount FROM EmnTabChartsLinkT "
							"WHERE EmnTabChartID = %li", VarLong(pRow->GetValue(chartID)));
						long nUseCount = AdoFldLong(rsUseCount, "UseCount", 0);
						if(nUseCount > 0) {
							if(IDYES == MsgBox(MB_YESNO, "There are %li EMNs assigned to this chart type.  If you continue, the "
								"chart type for all of these EMNs will be renamed from \"%s\" to \"%s\".  "
								"Are you sure you wish to do this?",
								nUseCount, VarString(varOldValue), strNewValue)) {
								*pbCommit = TRUE;
								*pbContinue = TRUE;
							}
							else {
								*pbCommit = FALSE;
								*pbContinue = FALSE;
							}
						}
						else {
							*pbCommit = TRUE;
							*pbContinue = TRUE;
						}
					}
				}
				else {
					*pbCommit = FALSE;
					*pbContinue = TRUE;
				}
				break;
		}

	}NxCatchAll("CConfigureEMRTabsDlg::OnEditingFinishingEmnTabChartList");
	
}

// (z.manning, 04/04/2007) - PLID 25485 - Categories can be selected per chart (same as apt types and purposes)
// so this function updates the selected categories for the currently selected chart.
void CConfigureEMRTabsDlg::UpdateCategoryListCheckboxes()
{
	// (z.manning, 04/04/2007) - PLID 25485 - Need to update the category list depending on what chart is selected.
	IRowSettingsPtr pCurChartSel = m_pdlEmnCharts->CurSel;
	IColumnSettingsPtr pCheckCol = m_EMNTabCategoriesList->GetColumn(eclcCheck);

	// (a.walling 2007-11-05 13:07) - PLID 27977 - VS2008 - for() loops
	IRowSettingsPtr pCategoryRow = NULL;

	for(pCategoryRow = m_EMNTabCategoriesList->GetFirstRow(); pCategoryRow != NULL; pCategoryRow = pCategoryRow->GetNextRow()) {
		pCategoryRow->PutValue(eclcCheck, VARIANT_FALSE);
	}
	if(pCurChartSel == NULL) {
		// (z.manning, 04/04/2007) - No selection, so just hide the category checkboxes.
		pCheckCol->StoredWidth = 0;
	}
	else {
		// (z.manning, 04/04/2007) - PLID 25485 - We have a chart row. Find and then select all
		// categories associated with this chart.
		long nChartID = VarLong(pCurChartSel->GetValue(chartID));
		pCheckCol->StoredWidth = 25;
		_RecordsetPtr prs = CreateRecordset(
			"SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabChartID = %li", nChartID);
		while(!prs->eof) {
			long nCategoryID = AdoFldLong(prs, "EmnTabCategoryID");
			pCategoryRow = m_EMNTabCategoriesList->FindByColumn(eclcID, nCategoryID, NULL, VARIANT_FALSE);
			if(pCategoryRow) {
				pCategoryRow->PutValue(eclcCheck, VARIANT_TRUE);
			}
			prs->MoveNext();
		}
	}
}

void CConfigureEMRTabsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	UpdatePriorityButtons();
	UpdateCategoryListCheckboxes();
	UpdateHistoryCategories();
}

void CConfigureEMRTabsDlg::OnRButtonUpEmnChartTypes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (a.wetta 2007-06-28 15:36) - PLID 26094 - Create a pop up menu so that the user can reset the color of the chart
		IRowSettingsPtr pRow(lpRow);
		
		if (lpRow) {
			// Make sure the row is selected
			m_pdlEmnCharts->SetSelByColumn(chartID, pRow->GetValue(chartID));

			// Create the pop up menu
			CMenu mPopup;
			mPopup.CreatePopupMenu();

			const int RESET_COLOR = 1;

			mPopup.AppendMenu(pRow->GetValue(chartColorData).vt != VT_NULL ? MF_ENABLED : MF_DISABLED|MF_GRAYED, RESET_COLOR, "&Reset Color");

			// Pop up the menu
			CPoint pt;
			GetCursorPos(&pt);
			long nResult = mPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this);

			// Handle the result
			if (nResult == RESET_COLOR) {
				pRow->PutCellBackColor(chartColor, (long)pRow->GetCellBackColor(chartColorData));
				pRow->PutCellBackColorSel(chartColor, (long)pRow->GetCellBackColorSel(chartColorData));
				_variant_t varNull;
				varNull.vt = VT_NULL;
				pRow->PutValue(chartColorData, varNull);

				ExecuteSql("UPDATE EMNTabChartsT SET Color = NULL WHERE ID = %li", VarLong(pRow->GetValue(chartID)));
			}
		}
	}NxCatchAll("Error in CConfigureEMRTabsDlg::OnContextMenu");	
}

void CConfigureEMRTabsDlg::OnRequeryFinishedEmnChartTypes(short nFlags) 
{
	try {
		if (nFlags == dlRequeryFinishedCompleted) {
			// (a.wetta 2007-06-28 15:56) - PLID 26094 - Color the rows appropriately
			IRowSettingsPtr pRow = m_pdlEmnCharts->GetFirstRow();
			while (pRow) {
				pRow->PutCellBackColor(chartColor, VarLong(pRow->GetValue(chartColorData), (long)pRow->GetCellBackColor(chartColorData)));
				pRow->PutCellBackColorSel(chartColor, VarLong(pRow->GetValue(chartColorData), (long)pRow->GetCellBackColorSel(chartColorData)));

				pRow = pRow->GetNextRow();
			}
		}
	}NxCatchAll("Error in CConfigureEMRTabsDlg::OnRequeryFinishedEmnChartTypes");
}

void CConfigureEMRTabsDlg::OnLButtonUpEmnChartTypes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if (lpRow) {
			IRowSettingsPtr pRow(lpRow);

			if (nCol == chartColor) {
				// (a.wetta 2007-06-28 16:11) - PLID 26094 - If the user clicks in the color column, pop up the color chooser so they can
				// can change the color of the selected row
				// Make sure the row is selected
				m_pdlEmnCharts->SetSelByColumn(chartID, pRow->GetValue(chartID));

				long nChartID = VarLong(pRow->GetValue(chartID));

				CColor color = VarLong(pRow->GetValue(chartColorData), (long)pRow->GetCellBackColor(chartColorData)), newColor;

				// Open up the color chooser dialog
				CColorDialog dlg(color, CC_ANYCOLOR|CC_RGBINIT);
				if (dlg.DoModal() == IDOK) {
					newColor = dlg.m_cc.rgbResult;
				
					if ((DWORD)color != (DWORD)newColor) {
						pRow->PutValue(chartColorData, _variant_t(newColor));
						pRow->PutCellBackColor(chartColor, newColor);
						pRow->PutCellBackColorSel(chartColor, newColor);
	
						ExecuteSql("UPDATE EMNTabChartsT SET Color = %li WHERE ID = %li", newColor, nChartID);
					}
				}
			}
		}
	}NxCatchAll("Error in CConfigureEMRTabsDlg::OnLButtonUpEmnChartTypes");
}

void CConfigureEMRTabsDlg::OnOK() 
{
	// (a.walling 2007-07-03 13:27) - PLID 26545 - Refresh the table checker here, rather than refresh every single time they change the priority
	try {
		if (m_bChartOrderChanged) {
			CClient::RefreshTable(NetUtils::EMNTabChartsT);
		}
		if (m_bCatOrderChanged) {
			CClient::RefreshTable(NetUtils::EMNTabCategoriesT);
		}
		if (m_bLinksChanged) {
			CClient::RefreshTable(NetUtils::EmnTabChartCategoryLinkT);
		}
	} NxCatchAll("Error refreshing tables in CConfigureEMRTabsDlg::OnOK");
	
	CDialog::OnOK();
}

void CConfigureEMRTabsDlg::OnCancel() 
{
	// (a.walling 2007-07-03 13:27) - PLID 26545 - Refresh the table checker here, rather than refresh every single time they change the priority
	try {
		if (m_bChartOrderChanged) {
			CClient::RefreshTable(NetUtils::EMNTabChartsT);
		}
		if (m_bCatOrderChanged) {
			CClient::RefreshTable(NetUtils::EMNTabCategoriesT);
		}
		if (m_bLinksChanged) {
			CClient::RefreshTable(NetUtils::EmnTabChartCategoryLinkT);
		}
	} NxCatchAll("Error refreshing tables in CConfigureEMRTabsDlg::OnCancel");
	
	CDialog::OnCancel();
}

void CConfigureEMRTabsDlg::UpdateHistoryCategories()
{
	_variant_t varFalse(false);
	for(IRowSettingsPtr pRow = m_pdlHistoryCategories->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
		pRow->PutValue(hclcCheck, varFalse);
	}

	IRowSettingsPtr pEmnCategoryRow = m_EMNTabCategoriesList->GetCurSel();
	if(pEmnCategoryRow == NULL) {
		// (z.manning 2008-06-30 16:00) - PLID 25574 - No EMN category is selected so make the history
		// category list read-only.
		m_pdlHistoryCategories->PutReadOnly(VARIANT_TRUE);
		m_nxstaticHistoryCatLabel.SetWindowText("Select an EMN category to the left to associated with history categories.");
	}
	else
	{
		// (z.manning 2008-06-30 16:01) - PLID 25574 - Find which history categories are linked
		// to the selected EMN category and check the appropriate rows.
		m_nxstaticHistoryCatLabel.SetWindowText(FormatString("Select the history categories that you want to link with EMN category '%s.'", VarString(pEmnCategoryRow->GetValue(eclcDescription))));
		m_pdlHistoryCategories->PutReadOnly(VARIANT_FALSE);
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT NoteCategoryID FROM EmrHistoryCategoryLinkT WHERE EmnTabCategoryID = {INT}"
			, VarLong(pEmnCategoryRow->GetValue(eclcID)));
		for(; !prs->eof; prs->MoveNext())
		{
			long nHistoryCategoryID = AdoFldLong(prs, "NoteCategoryID");
			IRowSettingsPtr pHistoryCategoryRow = m_pdlHistoryCategories->FindByColumn(hclcID, nHistoryCategoryID, NULL, VARIANT_FALSE);
			if(pHistoryCategoryRow != NULL) {
				pHistoryCategoryRow->PutValue(hclcCheck, _variant_t(true));
			}
			else {
				ThrowNxException("CConfigureEMRTabsDlg::UpdateHistoryCategories - Did not find history category row for ID %li", nHistoryCategoryID);
			}
		}
	}
}

void CConfigureEMRTabsDlg::OnEditingFinishedHistoryCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case hclcCheck:
				IRowSettingsPtr pEmnRow = m_EMNTabCategoriesList->GetCurSel();
				if(pEmnRow == NULL) {
					// (z.manning 2008-06-30 15:51) - PLID 25574 - They shouldn't have been able to check this list
					ASSERT(FALSE);
					return;
				}

				long nEmnCategoryID = VarLong(pEmnRow->GetValue(eclcID));
				long nHistoryCategoryID = VarLong(pRow->GetValue(hclcID));

				if(VarBool(varNewValue))
				{
					// (z.manning 2008-06-30 15:56) - PLID 25574 - They just checked the column so insert
					// a record in data.
					ExecuteParamSql(
						"INSERT INTO EmrHistoryCategoryLinkT (EmnTabCategoryID, NoteCategoryID) "
						"VALUES ({INT}, {INT}) "
						, nEmnCategoryID, nHistoryCategoryID);
				}
				else
				{
					// (z.manning 2008-06-30 15:57) - PLID 25574 - They unchecked a column, so delete that
					// record from data.
					ExecuteParamSql(
						"DELETE FROM EmrHistoryCategoryLinkT "
						"WHERE EmnTabCategoryID = {INT} AND NoteCategoryID = {INT} "
						, nEmnCategoryID, nHistoryCategoryID);
				}
		}

	}NxCatchAll("CConfigureEMRTabsDlg::OnEditingFinishedEMNCategories");
}
