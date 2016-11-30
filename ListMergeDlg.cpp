// ListMergeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ListMergeDlg.h"
#include "AuditTrail.h"
#include "HL7Utils.h"
#include "HL7ParseUtils.h"
#include "PatientsRc.h"
#include "GlobalSchedUtils.h"
#include "RemoteDataCache.h"
#include "PatientDashboardDlg.h"
#include "ConfigurePatDashboardDlg.h"

#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

//(e.lally 2011-07-07) PLID 31585
enum MergeListColumns
{
	mlID,
	mlChecked,
	mlName,
	mlExtra1, //Address
	mlExtra2, //Contact
	mlExtra3, //Phone
};

/////////////////////////////////////////////////////////////////////////////
// CListMergeDlg dialog


CListMergeDlg::CListMergeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CListMergeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CListMergeDlg)
		m_bCombined = FALSE;
		//(e.lally 2011-07-07) PLID 31585 - track the list type
		m_eListType = mltInvalid;
	//}}AFX_DATA_INIT
}


void CListMergeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CListMergeDlg)
	DDX_Control(pDX, IDC_COMBINE, m_btnCombine);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_INS, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_INS, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_INS, m_btnSelectOne);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MERGE_LIST_WARNING_TOP_LBL, m_nxstaticTopWarningLabel);
	DDX_Control(pDX, IDC_MERGE_LIST_WARNING_BOTTOM_LBL, m_nxstaticBottomWarningLabel);
	DDX_Control(pDX, IDC_UNSELECTED_MERGE_LIST_LBL, m_nxstaticUnselectedLabel);
	DDX_Control(pDX, IDC_SELECTED_MERGE_LIST_LBL, m_nxstaticSelectedLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CListMergeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CListMergeDlg)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_INS, OnBtnSelectOne)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_INS, OnBtnUnselectOne)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_INS, OnBtnUnselectAll)
	ON_BN_CLICKED(IDC_COMBINE, OnCombine)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListMergeDlg message handlers

BOOL CListMergeDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_btnSelectOne.SetIcon(IDI_DARROW);
		m_btnUnselectOne.SetIcon(IDI_UARROW);
		m_btnUnselectAll.SetIcon(IDI_UUARROW);
		// (c.haag 2008-04-30 13:49) - PLID 29847 - NxIconify more buttons
		m_btnCombine.AutoSet(NXB_MODIFY);
		m_btnCancel.AutoSet(NXB_CLOSE);

		//(e.lally 2011-07-07) PLID 31585 - Changed to DL2 controls
		m_pUnselectedList = BindNxDataList2Ctrl(IDC_UNSELECTED_MERGE_LIST, false);
		m_pSelectedList = BindNxDataList2Ctrl(IDC_SELECTED_MERGE_LIST, false);

		//(e.lally 2011-07-07) PLID 31585 - Make sure the caller set what list type this is
		if(m_eListType == mltInvalid){
			ThrowNxException(_T("Merge list type is not set!"));
		}

		switch(m_eListType){
			case mltInsuranceCompanies:
			{
				//(e.lally 2011-11-09) PLID 31585 - Bulk cache preferences in use for this list type
				g_propManager.BulkCache("ListMerge-InsCo", propbitNumber,
				"(Username IN('<None>', '%s') AND Name IN("
				"'DefaultInsuranceCoNewPatient' "
				",'DefaultInsurancePlanNewPatient' "
				"))", _Q(GetCurrentUserName()));
			}
			break;
			case mltNoteCategories:
			{
				//(e.lally 2011-11-09) PLID 31585 - Bulk cache preferences in use for this list type
				g_propManager.BulkCache("ListMerge-NoteCategories", propbitNumber,
				"(Username IN('<None>', '%s') AND Name IN("
				"'LabFollowUpDefaultCategory'"
				"))", _Q(GetCurrentUserName()));
			}
			break;
			// (j.jones 2012-04-10 16:39) - PLID 44174 - added ability to merge resources
			case mltSchedulerResources:
			{
				//there are no preferences to bulk cache
			}
			break;
		}

		//(e.lally 2011-07-07) PLID 31585 - Set up the controls for this list type
		EnsureControls();

		//(e.lally 2011-07-07) PLID 31585 - Now we can requery the list
		m_pUnselectedList->Requery();
	}
	NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//(e.lally 2011-07-07) PLID 31585 - Modifies the tables, labels, buttons etc to be specific to the current list type
void CListMergeDlg::EnsureControls()
{
	IColumnSettingsPtr pColID = m_pUnselectedList->GetColumn(mlID);
	IColumnSettingsPtr pColChecked = m_pUnselectedList->GetColumn(mlChecked);
	IColumnSettingsPtr pColName = m_pUnselectedList->GetColumn(mlName);
	IColumnSettingsPtr pColExtra1 = m_pUnselectedList->GetColumn(mlExtra1); //Address
	IColumnSettingsPtr pColExtra2 = m_pUnselectedList->GetColumn(mlExtra2); //Contact
	IColumnSettingsPtr pColExtra3 = m_pUnselectedList->GetColumn(mlExtra3); //Phone

	IColumnSettingsPtr pColSelectID = m_pSelectedList->GetColumn(mlID);
	IColumnSettingsPtr pColSelectChecked = m_pSelectedList->GetColumn(mlChecked);
	IColumnSettingsPtr pColSelectName = m_pSelectedList->GetColumn(mlName);
	IColumnSettingsPtr pColSelectExtra1 = m_pSelectedList->GetColumn(mlExtra1); //Address
	IColumnSettingsPtr pColSelectExtra2 = m_pSelectedList->GetColumn(mlExtra2); //Contact
	IColumnSettingsPtr pColSelectExtra3 = m_pSelectedList->GetColumn(mlExtra3); //Phone


	switch(m_eListType){
		case mltInsuranceCompanies:
		{
			SetWindowText("Manage Insurance Contacts");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_TOP_LBL, "WARNING: This utility will combine multiple insurance companies into one company with multiple contacts.\nDo not make changes here unless you are absolutely sure you want to combine two companies together.\nChanges made here are irreversible!");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_BOTTOM_LBL, "Please check the main company that you would like to combine the rest into, and click \"Combine Selected Companies\".\nThe rest of the companies will now use the selected name and address, with all the contacts available and assigned appropriately to each patient's account.");
			m_pUnselectedList->FromClause = _bstr_t("InsuranceCoT INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID");

			
			pColID->FieldName = "PersonID";
			pColChecked->FieldName = "CONVERT(BIT, 0)";
			pColName->FieldName = "Name";
			pColExtra1->FieldName = "Address1 + ' ' + Address2 + ' ' + City + ', ' + State + ' ' + Zip";
			pColExtra2->FieldName = "Last + ', ' + First";
			pColExtra3->FieldName = "WorkPhone";
		}
		break;

		case mltNoteCategories:
		{
			SetWindowText("Combine Note Categories");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_TOP_LBL, "WARNING: This utility will combine multiple categories into one category.\nDo not make changes here unless you are absolutely sure you want to combine two categories together.\nChanges made here are irreversible!");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_BOTTOM_LBL, "Please check the main category that you would like to combine the rest into, and click \"Combine Selected Categories\".\nThe rest of the categories will now be assigned appropriately to each patient's account as the new category.");
			SetDlgItemText(IDC_UNSELECTED_MERGE_LIST_LBL, "Unselected Categories");
			SetDlgItemText(IDC_SELECTED_MERGE_LIST_LBL, "Selected Categories");
			SetDlgItemText(IDC_COMBINE, "Combine Selected Categories");

			m_pUnselectedList->FromClause = _bstr_t("NoteCatsF");

			pColID->FieldName = "ID";
			pColID->PutStoredWidth(0);
			pColID->ColumnStyle = csFixedWidth|csVisible;
			pColSelectID->PutStoredWidth(0);
			pColSelectID->ColumnStyle = csFixedWidth|csVisible;

			pColChecked->FieldName = "CONVERT(BIT, 0)";
			pColSelectChecked->PutStoredWidth(25);
			pColSelectChecked->ColumnStyle = csFixedWidth|csVisible|csEditable;

			pColName->FieldName = "Description";
			pColName->ColumnTitle = "Category";
			pColName->ColumnStyle = csWidthAuto|csVisible;
			pColSelectName->ColumnTitle = "Category";
			pColSelectName->ColumnStyle = csWidthAuto|csVisible;

			pColExtra1->FieldName = "''";
			pColExtra1->PutStoredWidth(0);
			pColExtra1->ColumnStyle = csFixedWidth|csVisible;
			pColSelectExtra1->PutStoredWidth(0);
			pColSelectExtra1->ColumnStyle = csFixedWidth|csVisible;

			pColExtra2->FieldName = "''";
			pColExtra2->PutStoredWidth(0);
			pColExtra2->ColumnStyle = csFixedWidth|csVisible;
			pColSelectExtra2->PutStoredWidth(0);
			pColSelectExtra2->ColumnStyle = csFixedWidth|csVisible;

			pColExtra3->FieldName = "''";
			pColExtra3->PutStoredWidth(0);
			pColExtra3->ColumnStyle = csFixedWidth|csVisible;
			pColSelectExtra3->PutStoredWidth(0);
			pColSelectExtra3->ColumnStyle = csFixedWidth|csVisible;
		}
		break;

		// (j.jones 2012-04-10 16:39) - PLID 44174 - added ability to merge resources
		case mltSchedulerResources:
		{
			SetWindowText("Combine Scheduler Resources");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_TOP_LBL, "WARNING: This utility will combine multiple scheduler resources into one resource.\nDo not make changes here unless you are absolutely sure you want to combine resources together.\nChanges made here are irreversible!");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_BOTTOM_LBL, "Please check the main resource that you would like to combine the rest into, and click \"Combine Selected Resources\".\nAll appointments and scheduler templates for the remaining resources will be assigned to the checked resource.");
			SetDlgItemText(IDC_UNSELECTED_MERGE_LIST_LBL, "Unselected Resources");
			SetDlgItemText(IDC_SELECTED_MERGE_LIST_LBL, "Selected Resources");
			SetDlgItemText(IDC_COMBINE, "Combine Selected Resources");

			//this includes inactive resources
			m_pUnselectedList->FromClause = _bstr_t("ResourceT");

			pColID->FieldName = "ID";
			pColID->PutStoredWidth(0);
			pColID->ColumnStyle = csFixedWidth|csVisible;
			pColSelectID->PutStoredWidth(0);
			pColSelectID->ColumnStyle = csFixedWidth|csVisible;

			pColChecked->FieldName = "CONVERT(BIT, 0)";
			pColSelectChecked->PutStoredWidth(25);
			pColSelectChecked->ColumnStyle = csFixedWidth|csVisible|csEditable;

			pColName->FieldName = "Item";
			pColName->ColumnTitle = "Resource";
			pColName->ColumnStyle = csWidthAuto|csVisible;
			pColSelectName->ColumnTitle = "Resource";
			pColSelectName->ColumnStyle = csWidthAuto|csVisible;

			pColExtra1->FieldName = "Inactive";
			pColExtra1->PutStoredWidth(100);
			pColExtra1->ColumnTitle = "Inactive";
			pColExtra1->ColumnStyle = csFixedWidth|csVisible;
			pColExtra1->FieldType = cftBoolYesNo;
			pColSelectExtra1->PutStoredWidth(100);
			pColSelectExtra1->ColumnTitle = "Inactive";
			pColSelectExtra1->ColumnStyle = csFixedWidth|csVisible;
			pColSelectExtra1->FieldType = cftBoolYesNo;

			pColExtra2->FieldName = "''";
			pColExtra2->PutStoredWidth(0);
			pColExtra2->ColumnStyle = csFixedWidth|csVisible;
			pColSelectExtra2->PutStoredWidth(0);
			pColSelectExtra2->ColumnStyle = csFixedWidth|csVisible;

			pColExtra3->FieldName = "''";
			pColExtra3->PutStoredWidth(0);
			pColExtra3->ColumnStyle = csFixedWidth|csVisible;
			pColSelectExtra3->PutStoredWidth(0);
			pColSelectExtra3->ColumnStyle = csFixedWidth|csVisible;
		}
		break;
		case mltProviders: // (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
		{
			SetWindowText("Manage Providers");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_TOP_LBL, "WARNING: This utility will combine multiple providers into one.\nDo not make changes here unless you are absolutely sure you want to combine two providers together.\nChanges made here are irreversible!");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_BOTTOM_LBL, "Please check the main provider that you would like to combine the rest into, and click \"Combine Selected Providers\".");
			SetDlgItemText(IDC_UNSELECTED_MERGE_LIST_LBL, "Unselected Providers");
			SetDlgItemText(IDC_SELECTED_MERGE_LIST_LBL, "Selected Providers");
			SetDlgItemText(IDC_COMBINE, "Combine Selected Providers");

			m_pUnselectedList->FromClause = _bstr_t("ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID");

			pColName->ColumnTitle = "Provider";
			pColSelectName->ColumnTitle = "Provider";

			pColID->FieldName = "PersonID";
			pColChecked->FieldName = "CONVERT(BIT, 0)";
			pColName->FieldName = "FullName";
			pColExtra1->FieldName = "Address1 + ' ' + Address2 + ' ' + City + ', ' + State + ' ' + Zip";
			pColExtra3->FieldName = "WorkPhone";
			
			pColExtra2->FieldName = "''";
			pColExtra2->PutStoredWidth(0);
			pColExtra2->ColumnStyle = csFixedWidth | csVisible;
			pColSelectExtra2->PutStoredWidth(0);
			pColSelectExtra2->ColumnStyle = csFixedWidth | csVisible;
		}		
		break;
		case mltLocations: // (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
		{
			SetWindowText("Combine Locations");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_TOP_LBL, "WARNING: This utility will combine multiple locations into one.\nDo not make changes here unless you are absolutely sure you want to combine locations together.\nChanges made here are irreversible!");
			SetDlgItemText(IDC_MERGE_LIST_WARNING_BOTTOM_LBL, "Please check the main location that you would like to combine the rest into, and click \"Combine Selected Locations\".");
			SetDlgItemText(IDC_UNSELECTED_MERGE_LIST_LBL, "Unselected Locations");
			SetDlgItemText(IDC_SELECTED_MERGE_LIST_LBL, "Selected Locations");
			SetDlgItemText(IDC_COMBINE, "Combine Selected Locations");

			//this includes inactive resources
			m_pUnselectedList->FromClause = "LocationsT";
			m_pUnselectedList->WhereClause = "LocationsT.TypeID = 1";

			pColID->FieldName = "ID";
			pColID->PutStoredWidth(0);
			pColID->ColumnStyle = csFixedWidth | csVisible;
			pColSelectID->PutStoredWidth(0);
			pColSelectID->ColumnStyle = csFixedWidth | csVisible;

			pColChecked->FieldName = "CONVERT(BIT, 0)";
			pColSelectChecked->PutStoredWidth(25);
			pColSelectChecked->ColumnStyle = csFixedWidth | csVisible | csEditable;

			pColName->FieldName = "Name";
			pColName->ColumnTitle = "Name";
			pColName->ColumnStyle = csWidthAuto | csVisible;
			pColSelectName->ColumnTitle = "Name";
			pColSelectName->ColumnStyle = csWidthAuto | csVisible;

			pColExtra1->FieldName = "''";
			pColExtra1->PutStoredWidth(0);
			pColExtra1->ColumnStyle = csFixedWidth | csVisible;
			pColSelectExtra1->PutStoredWidth(0);
			pColSelectExtra1->ColumnStyle = csFixedWidth | csVisible;

			pColExtra2->FieldName = "''";
			pColExtra2->PutStoredWidth(0);
			pColExtra2->ColumnStyle = csFixedWidth | csVisible;
			pColSelectExtra2->PutStoredWidth(0);
			pColSelectExtra2->ColumnStyle = csFixedWidth | csVisible;

			pColExtra3->FieldName = "''";
			pColExtra3->PutStoredWidth(0);
			pColExtra3->ColumnStyle = csFixedWidth | csVisible;
			pColSelectExtra3->PutStoredWidth(0);
			pColSelectExtra3->ColumnStyle = csFixedWidth | csVisible;
		}
		break;
		default: 
			ASSERT(FALSE);
			break;
	}

}

void CListMergeDlg::OnBtnSelectOne() 
{
	try {
		//(e.lally 2011-07-07) PLID 31585 - Changed to support the DL2
		if(m_pUnselectedList->GetCurSel()!= NULL) {
			m_pSelectedList->TakeCurrentRowAddAtEnd(m_pUnselectedList, NULL);
		}
	}NxCatchAll(__FUNCTION__)
}

void CListMergeDlg::OnBtnUnselectOne() 
{
	try {
		//(e.lally 2011-07-07) PLID 31585 - Changed to support the DL2
		if(m_pSelectedList->GetCurSel()!= NULL) {
			m_pSelectedList->GetCurSel()->PutValue(mlChecked, VARIANT_FALSE);
			m_pUnselectedList->TakeCurrentRowAddAtEnd(m_pSelectedList, NULL);
		}
	}NxCatchAll(__FUNCTION__)
}

void CListMergeDlg::OnBtnUnselectAll() 
{
	try{
		//(e.lally 2011-07-07) PLID 31585 - Changed to support the DL2
		m_pUnselectedList->TakeAllRows(m_pSelectedList);

		//Ensure all unselected rows are unchecked
		m_pUnselectedList->SetRedraw(VARIANT_FALSE);
		IRowSettingsPtr pRow = m_pUnselectedList->GetFirstRow();
		while(pRow != NULL){
			pRow->PutValue(mlChecked, VARIANT_FALSE);
			pRow = pRow->GetNextRow();
		}
		m_pUnselectedList->SetRedraw(VARIANT_TRUE);
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CListMergeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CListMergeDlg)
	ON_EVENT(CListMergeDlg, IDC_SELECTED_MERGE_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedMergeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CListMergeDlg, IDC_UNSELECTED_MERGE_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedMergeList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CListMergeDlg, IDC_SELECTED_MERGE_LIST, 10 /* EditingFinished */, OnEditingFinishedSelectedMergeList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CListMergeDlg::OnDblClickCellSelectedMergeList(LPDISPATCH lpRow, short nColIndex) 
{
	OnBtnUnselectOne();
}

void CListMergeDlg::OnDblClickCellUnselectedMergeList(LPDISPATCH lpRow, short nColIndex) 
{
	OnBtnSelectOne();
}

void CListMergeDlg::OnCombine() 
{
	try {

		long count = m_pSelectedList->GetRowCount();
		
		if(count == 0)
			return;

		//(e.lally 2011-07-07) PLID 31585 - Changed to support the DL2
		IRowSettingsPtr pRowToKeep = NULL;
		bool bHasMultSelected = false;

		for (IRowSettingsPtr pSearchRow = m_pSelectedList->GetFirstRow(); pSearchRow != NULL; pSearchRow = pSearchRow->GetNextRow()) {

			BOOL bChecked = AsBool(pSearchRow->GetValue(mlChecked));
			if (bChecked) {
				if(pRowToKeep != NULL){
					bHasMultSelected = true;
				}
				else {
					pRowToKeep = pSearchRow;
				}
			}
		}

		if(pRowToKeep == NULL) {
			AfxMessageBox("Please check the entry you wish to combine the rest into.");
			return;
		}
		else if(bHasMultSelected){
			//This shouldn't be possible
			AfxMessageBox("Please check only one entry you wish to combine the rest into.");
			return;
		}

		//warn them HEAVILY
		if(m_eListType == mltInsuranceCompanies){
			// (d.lange 2015-10-15 11:54) - PLID 67117 - Renamed multifees to new fees
			if(IDNO==MessageBox("This action will combine the selected companies into the checked company.\n"
				"This process will DELETE these companies, including any IDs or new fees associated with them.\n"
				"They will all use the IDs, configuration, and new fees associated with the checked company.\n"
				"Patients using these companies will now be using the checked company, with their old contact.\n\n"
				"Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO) ||
				IDNO==MessageBox("This action is UNRECOVERABLE!\n"
					"Are you ABSOLUTELY sure you wish to combine these companies?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
			//Do the merge
			MergeInsuranceCompanies(pRowToKeep);
		}
		//(e.lally 2011-07-07) PLID 31585 - Added support for combining note categories
		else if(m_eListType == mltNoteCategories){
			if(IDNO==MessageBox("This action will combine the selected categories into the checked category.\n"
				"This process will DELETE these categories.\n"
				"Patient notes, ToDo tasks and history documents using these categories will now be using the checked category.\n\n"
				"Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO) ||
				IDNO==MessageBox("This action is UNRECOVERABLE!\n"
					"Are you ABSOLUTELY sure you wish to combine these categories?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
			//(e.lally 2011-07-07) PLID 31585 - Do the merge
			MergeNoteCategories(pRowToKeep);
		}
		// (j.jones 2012-04-10 16:39) - PLID 44174 - added ability to merge resources
		else if(m_eListType == mltSchedulerResources) {
			if(IDNO==MessageBox("This action will combine the selected scheduler resources into the checked resource.\n\n"
				"This process will DELETE these resources.\n\n"
				"Appointments and scheduler templates using these resources will now be using the checked resource.\n\n"
				"Are you sure you wish to continue?","Practice",MB_ICONEXCLAMATION|MB_YESNO) ||
				IDNO==MessageBox("This action is UNRECOVERABLE!\n"
					"Are you ABSOLUTELY sure you wish to combine these resources?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
			//merge now
			MergeSchedulerResources(pRowToKeep);
		}
		else if (m_eListType == mltProviders) {
			// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
			if (IDNO == MessageBox("This action will combine the selected providers into the checked provider.\n\n"
				"This process will DELETE these providers.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONEXCLAMATION | MB_YESNO) ||
				IDNO == MessageBox("This action is UNRECOVERABLE!\n"
					"Are you ABSOLUTELY sure you wish to combine these providers?", "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
				return;
			}
			//merge now
			MergeProviders(pRowToKeep);
		}
		else if (m_eListType == mltLocations) {
			// (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
			if (IDNO == MessageBox("This action will combine the selected locations into the checked location.\n\n"
				"This process will DELETE these locations.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONEXCLAMATION | MB_YESNO) ||
				IDNO == MessageBox("This action is UNRECOVERABLE!\n"
					"Are you ABSOLUTELY sure you wish to combine these locations?", "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
				return;
			}
			//merge now
			MergeLocations(pRowToKeep);
		}
		else {
			ASSERT(FALSE);
		}

		m_pSelectedList->Clear();
		m_pUnselectedList->Requery();

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-07-07) PLID 31585 - Moved into its own function, added DL2 support
void CListMergeDlg::MergeInsuranceCompanies(IRowSettingsPtr pRowToKeep)
{
	try {
		long MainInsCoID = -1;
		CString strMainInsCoName;
		MainInsCoID = VarLong(pRowToKeep->GetValue(mlID));
		strMainInsCoName = VarString(pRowToKeep->GetValue(mlName));


		//get the first insurance plan for the main ins. co. and update appropriately
		_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ID FROM InsurancePlansT WHERE InsCoID = %li",MainInsCoID);
		// (b.spivey, March 23, 2015) PLID 59417 - use a variant cause this can be null
		_variant_t vtFirstInsPlan = g_cvarNull; 
		if(!rs->eof) {
			vtFirstInsPlan = AdoFldLong(rs, "ID", 0);
		}
		rs->Close();
		
		BOOL bInsPlansMerged = FALSE;

		//loop through each Ins. Co. and convert to the new ins. co.
		IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();
		while(pRow != NULL) {

			long ID = VarLong(pRow->GetValue(mlID),-1);

			if(ID != MainInsCoID) {

				//now do the dirty work (notice that we do NOT change the insured party's insurance contact id)

				// (j.jones 2013-07-18 09:25) - PLID 41823 - merge our secondary exclusion tables
				ExecuteParamSql("UPDATE HCFASecondaryExclusionsT SET InsuranceCoID = {INT} WHERE InsuranceCoID = {INT} AND NOT EXISTS (SELECT InsuranceCoID FROM HCFASecondaryExclusionsT WHERE InsuranceCoID = {INT})", MainInsCoID, ID, MainInsCoID);
				ExecuteParamSql("UPDATE UBSecondaryExclusionsT SET InsuranceCoID = {INT} WHERE InsuranceCoID = {INT} AND NOT EXISTS (SELECT InsuranceCoID FROM UBSecondaryExclusionsT WHERE InsuranceCoID = {INT})", MainInsCoID, ID, MainInsCoID);

				// (j.jones 2009-08-05 08:42) - PLID 34467 - keep InsuranceLocationPayerIDsT records if we don't already have them
				ExecuteParamSql("UPDATE InsuranceLocationPayerIDsT SET InsuranceCoID = {INT} WHERE InsuranceCoID = {INT} AND LocationID NOT IN (SELECT LocationID FROM InsuranceLocationPayerIDsT WHERE InsuranceCoID = {INT})", MainInsCoID, ID, MainInsCoID);
				// (j.jones 2005-05-03 10:20) - PLID 16325 - retain the InsPlan if it's a different name
				// first, if no InsPlan is selected, update it to the first one for our new company
				ExecuteParamSql("UPDATE InsuredPartyT SET InsPlan = {VT_I4} WHERE InsuranceCoID = {INT} AND InsPlan NOT IN (SELECT ID FROM InsurancePlansT)", vtFirstInsPlan, ID);
				// next, if an InsPlan is selected and its name does not exist as a plan in the new company, add it to the new company
				ExecuteSql("UPDATE InsurancePlansT SET InsCoID = %li WHERE InsCoID = %li AND PlanName NOT IN (SELECT PlanName FROM InsurancePlansT WHERE InsCoID = %li)",MainInsCoID,ID,MainInsCoID);
				// last, if an InsPlan is selected and its name is the same as a plan in the new company, change to that plan
				_RecordsetPtr rs = CreateRecordset("SELECT PersonID, InsurancePlansT.PlanName FROM InsuredPartyT INNER JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID WHERE InsuranceCoID = %li AND InsCoID = %li",ID,ID);
				while(!rs->eof) {
					long nPersonID = AdoFldLong(rs, "PersonID",-1);
					CString strInsPlanName = AdoFldString(rs, "PlanName","");

					_RecordsetPtr rs2 = CreateRecordset("SELECT ID FROM InsurancePlansT WHERE InsCoID = %li AND PlanName = '%s'",MainInsCoID,_Q(strInsPlanName));
					if(!rs2->eof) {
						long NewInsPlan = AdoFldLong(rs2, "ID",0);
						ExecuteSql("UPDATE InsuredPartyT SET InsPlan = %li WHERE PersonID = %li",NewInsPlan,nPersonID);
						bInsPlansMerged = TRUE;
					}
					rs2->Close();

					rs->MoveNext();
				}
				rs->Close();
				//there should not be any plans left but update just incase
				ExecuteParamSql("UPDATE InsuredPartyT SET InsPlan = {VT_I4} WHERE InsuranceCoID = {INT} AND InsPlan NOT IN (SELECT ID FROM InsurancePlansT WHERE InsCoID = {INT})", vtFirstInsPlan, ID, MainInsCoID);

				//switch all insured parties using this insurance company to use the new insurance company
				ExecuteSql("UPDATE InsuredPartyT SET InsuranceCoID = %li WHERE InsuranceCoID = %li",MainInsCoID,ID);

				//switch the insurance contact to be part of this insurance company
				ExecuteSql("UPDATE InsuranceContactsT SET InsuranceCoID = %li WHERE InsuranceCoID = %li",MainInsCoID,ID);

				// (j.jones 2010-10-20 09:19) - PLID 27897 - update BatchPaymentsT
				ExecuteParamSql("UPDATE BatchPaymentsT SET InsuranceCoID = {INT} WHERE InsuranceCoID = {INT}",MainInsCoID,ID);
				
				// (j.camacho 2013-06-28 11:40) - PLID 51069 - Update the diagnosis from the insurance group to the new insurance
				ExecuteParamSql("UPDATE diaginsnotest SET InsuranceCoID = {INT} WHERE InsuranceCoID = {INT}",MainInsCoID,ID);
				
				// (j.camacho 2013-06-28 12:06) - PLID 51069 - Update the mail sent table to show the new insurance company 
				ExecuteParamSql("UPDATE mailsent SET personid = {INT} WHERE personid = {INT}",MainInsCoID,ID);

				// (s.tullis 2015-01-05 10:51) - PLID 64440 - Need to update the scheduler rules that don't already have the merged insurance co linked to it
				ExecuteParamSql("UPDATE A SET InsuranceCOID = {INT}  FROM ScheduleMixRuleInsuranceCosT A "
					"Left Join ScheduleMixRuleInsuranceCosT B "
					"ON A.RuleID = B.RuleID AND B.InsuranceCoID = {INT} "
					"WHERE A.InsuranceCoID = {INT} AND B.RuleID IS Null "
					, MainInsCoID, MainInsCoID,ID );

				// (s.tullis 2016-02-19 16:21) - PLID 68318
				ExecuteParamSql("UPDATE A SET insuranceID = {INT} FROM ClaimFormLocationInsuranceSetupT A "
					" LEFT JOIN ClaimFormLocationInsuranceSetupT B "
					" ON B.LocationID = A.LocationID AND B.InsuranceID = {INT}  "
					" WHERE A.InsuranceID = {INT} AND B.LocationID IS NULL ", MainInsCoID, MainInsCoID, ID);

				// (d.thompson 2009-03-20) - PLID 33061 - We have a "default ins co" preference now, if it is 
				//	this ins co, we have to clear it.
				long nDefaultInsCoID = GetRemotePropertyInt("DefaultInsuranceCoNewPatient", -1, 0, "<None>", true);
				if(nDefaultInsCoID == ID) {
					//The configRT setting matches this ins co, so wipe it.  We also wipe the plan too, as it is ins co - dependant
					// (j.jones 2010-10-07 09:12) - PLID 37818 - don't wipe it, instead update to the same master company
					// and insurance plan that we're updating everything else to, that way instead of forcing them to change
					// the preference, we're setting it to the preferred company, they can tweak it later if they want
					SetRemotePropertyInt("DefaultInsuranceCoNewPatient", MainInsCoID, 0, "<None>");

					long nDefaultPlanID = GetRemotePropertyInt("DefaultInsurancePlanNewPatient", -1, 0, "<None>", true);
					if(nDefaultPlanID != -1) {
						//does the current plan have the same name as one in the new master insco?
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT ID FROM InsurancePlansT WHERE InsCoID = {INT} "
							"AND PlanName = (SELECT PlanName FROM InsurancePlansT WHERE ID = {INT})", MainInsCoID, nDefaultPlanID);
						if(!rs2->eof) {
							long NewInsPlan = AdoFldLong(rs2, "ID");
							SetRemotePropertyInt("DefaultInsurancePlanNewPatient", NewInsPlan, 0, "<None>");
						}
						else {
							//no match was found, so set to the first ins. plan on the company, which is what
							//we did to all the insured parties
							long nInsPlan = -1; 
							if (vtFirstInsPlan != g_cvarNull) {
								nInsPlan = VarLong(vtFirstInsPlan);
							}
							SetRemotePropertyInt("DefaultInsurancePlanNewPatient",  nInsPlan, 0, "<None>");
						}
						rs2->Close();
					}					
				}
				//delete the insurance company!
				// (s.tullis 2015-01-05 10:51) - PLID 64440 - delete remaining records
				ExecuteParamSql("Delete FROM ScheduleMixRuleInsuranceCosT WHERE InsuranceCoID={INT} ", ID);
				// (j.jones 2013-07-18 09:25) - PLID 41823 - delete from our secondary exclusion tables
				ExecuteParamSql("DELETE FROM HCFASecondaryExclusionsT WHERE InsuranceCoID = {INT}", ID);
				ExecuteParamSql("DELETE FROM UBSecondaryExclusionsT WHERE InsuranceCoID = {INT}", ID);
				// (j.jones 2010-08-03 08:58) - PLID 39912 - clear out InsCoServicePayGroupLinkT
				ExecuteParamSql("DELETE FROM InsCoServicePayGroupLinkT WHERE InsuranceCoID = {INT}", ID);
				ExecuteSql("DELETE FROM Tops_InsuranceLinkT WHERE PracInsuranceID = %li", ID);
				ExecuteSql("DELETE FROM GroupDetailsT WHERE PersonID = %li",ID);
				ExecuteSql("DELETE FROM MultiFeeInsuranceT WHERE InsuranceCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceAcceptedT WHERE InsuranceCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceGroups WHERE InsCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceBox24J WHERE InsCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceNetworkID WHERE InsCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceBox31 WHERE InsCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceBox51 WHERE InsCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsuranceFacilityID WHERE InsCoID = %li;",ID);
				ExecuteSql("DELETE FROM InsurancePlansT WHERE InsCoID = %li",ID);				
				ExecuteSql("DELETE FROM CPTInsNotesT WHERE InsuranceCoID = %li",ID);
				ExecuteSql("DELETE FROM ServiceRevCodesT WHERE InsuranceCompanyID = %li", ID);
				// (j.jones 2008-08-01 10:27) - PLID 30917 - ensure we delete from HCFAClaimProvidersT
				ExecuteParamSql("DELETE FROM HCFAClaimProvidersT WHERE InsuranceCoID = {INT}", ID);
				// (j.jones 2009-08-05 08:42) - PLID 34467 - delete from InsuranceLocationPayerIDsT
				ExecuteParamSql("DELETE FROM InsuranceLocationPayerIDsT WHERE InsuranceCoID = {INT}", ID);
				//TES 7/23/2010 - PLID 39320 - Clear out any linked HL7 codes
				ExecuteParamSql("DELETE FROM HL7CodeLinkT WHERE PracticeID = {INT} AND Type = {INT};\r\n", ID, hclrtInsCo);
				// (j.jones 2011-04-05 15:14) - PLID 42372 - remove CLIA setup
				ExecuteParamSql("DELETE FROM CLIANumbersT WHERE InsuranceCoID = {INT}", ID);
				//(e.lally 2011-05-04) PLID 43481 - remove NexWeb display setup
				ExecuteParamSql("DELETE FROM NexWebDisplayT WHERE InsuranceCoID = {INT}", ID);
				// (s.tullis 2016-02-19 16:21) - PLID 68318
				ExecuteParamSql("DELETE FROM ClaimFormLocationInsuranceSetupT WHERE InsuranceID = {INT}", ID);

				ExecuteSql("DELETE FROM InsuranceCoT WHERE PersonID = %li",ID);
				ExecuteSql("DELETE FROM PersonT WHERE ID = %li",ID);

				// (c.haag 2007-03-20 10:39) - PLID 25274 - Audit the merge
				CString strInsCoName = VarString(pRow->GetValue(mlName));
				CString strOld = FormatString("%s, %s", strInsCoName, strMainInsCoName);
				CString strNew = strMainInsCoName;

				long AuditID = BeginNewAuditEvent();
				AuditEvent(-1, strInsCoName,AuditID,aeiInsCoMerged,ID,strOld,strNew,aepHigh,aetChanged);

			} // if(ID != MainInsCoID) {

			pRow = pRow->GetNextRow();
		} // while pRow is not null

		//remove the defaults
		ExecuteSql("UPDATE InsuranceContactsT SET [Default] = 0 WHERE InsuranceCoID = %li",MainInsCoID);

		//this lets the insurance window know we made changes
		m_bCombined = TRUE;

		//send network messages
		CClient::RefreshTable(NetUtils::InsuranceCoT);
		CClient::RefreshTable(NetUtils::InsuranceContactsT);
		CClient::RefreshTable(NetUtils::InsurancePlansT);

		if(bInsPlansMerged) {
			AfxMessageBox("At least one Insurance Plan that is used by a patient was merged between these insurance companies.\n"
				"Please double check the resulting company record to make sure your insurance plan types are correct.");
		}


		// (z.manning 2009-01-09 09:21) - PLID 32663 - Update any relevant patients in HL7.
		UpdateExistingHL7PatientsByInsCoID(MainInsCoID);

	}NxCatchAll("Error combining insurance companies.");
}

//(e.lally 2011-07-07) PLID 31585 - Merges categories for Note, ToDo Tasks, History documents
void CListMergeDlg::MergeNoteCategories(IRowSettingsPtr pRowToKeep)
{
	try {
		long nKeepCategoryID = VarLong(pRowToKeep->GetValue(mlID));
		CString strKeepCategoryName = VarString(pRowToKeep->GetValue(mlName));

		CParamSqlBatch batch;		
		batch.Declare("SET XACT_ABORT ON \r\n");
		batch.Declare("DECLARE @deleteCategoryT TABLE (CategoryID INT NOT NULL)\r\n"
			"DECLARE @keepCategoryID INT\r\n");
		batch.Add("SET @keepCategoryID = (SELECT ID FROM NoteCatsF WHERE ID = {INT})", nKeepCategoryID);
		batch.Add(FormatString("IF @keepCategoryID IS NULL BEGIN RAISERROR(N'Category to keep ''%s'' does not exist', 16, 1) RETURN END", strKeepCategoryName));

		CAuditTransaction audit;

		//loop through each one and convert to the new category
		for(IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {		

			long nID = VarLong(pRow->GetValue(mlID),-1);

			if(nID != nKeepCategoryID) {
				batch.Add("INSERT INTO @deleteCategoryT(CategoryID) VALUES({INT});", nID);

				//Audit the merge
				CString strCategoryName = VarString(pRow->GetValue(mlName));
				CString strOld = FormatString("%s, %s", strCategoryName, strKeepCategoryName);
				CString strNew = strKeepCategoryName;

				//(e.lally 2011-07-08) PLID 44495 - audit the merge
				AuditEvent(-1, "", audit, aeiHistoryCategoriesMerged, nID, strOld, strNew, aepHigh, aetChanged);

			}//end if
		} //end for

		if(IsNexTechInternal()){
			//(e.lally 2011-11-29) PLID 31585 - Update the internal only tables
			batch.Add(
			"UPDATE ClientImplementationStepsT      SET ToDoCategoryID           = @keepCategoryID FROM ClientImplementationStepsT		INNER JOIN @deleteCategoryT AS deleteCategoryT ON ClientImplementationStepsT.ToDoCategoryID = deleteCategoryT.CategoryID \r\n"
			"UPDATE ClientImplementationStepsT      SET DocumentCategoryID       = @keepCategoryID FROM ClientImplementationStepsT		INNER JOIN @deleteCategoryT AS deleteCategoryT ON ClientImplementationStepsT.DocumentCategoryID = deleteCategoryT.CategoryID \r\n"
			"UPDATE ImplementationStepTemplatesT    SET ToDoCategoryID           = @keepCategoryID FROM ImplementationStepTemplatesT	INNER JOIN @deleteCategoryT AS deleteCategoryT ON ImplementationStepTemplatesT.ToDoCategoryID = deleteCategoryT.CategoryID \r\n"
			"UPDATE ImplementationStepTemplatesT    SET DocumentCategoryID       = @keepCategoryID FROM ImplementationStepTemplatesT	INNER JOIN @deleteCategoryT AS deleteCategoryT ON ImplementationStepTemplatesT.DocumentCategoryID = deleteCategoryT.CategoryID \r\n"
			);
		}

		batch.Add(
		"UPDATE MailSent                 SET CategoryID       = @keepCategoryID FROM Mailsent					INNER JOIN @deleteCategoryT AS deleteCategoryT ON MailSent.CategoryID = deleteCategoryT.CategoryID \r\n"
		"UPDATE Notes                    SET Category         = @keepCategoryID FROM Notes						INNER JOIN @deleteCategoryT AS deleteCategoryT ON Notes.Category = deleteCategoryT.CategoryID \r\n"
		"UPDATE StepTemplatesT           SET TodoCategory     = @keepCategoryID FROM StepTemplatesT				INNER JOIN @deleteCategoryT AS deleteCategoryT ON StepTemplatesT.TodoCategory = deleteCategoryT.CategoryID \r\n"
		"UPDATE EMRActionsTodoDataT      SET CategoryID       = @keepCategoryID FROM EMRActionsTodoDataT		INNER JOIN @deleteCategoryT AS deleteCategoryT ON EMRActionsTodoDataT.CategoryID = deleteCategoryT.CategoryID \r\n"
		"UPDATE EmrHistoryCategoryLinkT  SET NoteCategoryID   = @keepCategoryID FROM EmrHistoryCategoryLinkT	INNER JOIN @deleteCategoryT AS deleteCategoryT ON EmrHistoryCategoryLinkT.NoteCategoryID = deleteCategoryT.CategoryID \r\n"
		//(e.lally 2011-07-12) PLID 31585 - Skip any that are already assigned to the destination tab categories.
			"WHERE EmrHistoryCategoryLinkT.EMNTabCategoryID NOT IN(SELECT EmrHistoryCategoryLinkT.EmnTabCategoryID FROM EmrHistoryCategoryLinkT WHERE NoteCategoryID = @keepCategoryID) \r\n"
		//Delete any duplicates that couldn't be moved
		"DELETE EmrHistoryCategoryLinkT  FROM EmrHistoryCategoryLinkT	INNER JOIN @deleteCategoryT AS deleteCategoryT ON EmrHistoryCategoryLinkT.NoteCategoryID = deleteCategoryT.CategoryID \r\n"

		"UPDATE EMRWordTemplateCategoryT SET NoteCatID        = @keepCategoryID FROM EMRWordTemplateCategoryT	INNER JOIN @deleteCategoryT AS deleteCategoryT ON EMRWordTemplateCategoryT.NoteCatID = deleteCategoryT.CategoryID \r\n"
		"UPDATE ExportHistoryCategoriesT SET CategoryID       = @keepCategoryID FROM ExportHistoryCategoriesT	INNER JOIN @deleteCategoryT AS deleteCategoryT ON ExportHistoryCategoriesT.CategoryID = deleteCategoryT.CategoryID \r\n"
		"UPDATE LabProcedureStepsT       SET TodoCategoryID   = @keepCategoryID FROM LabProcedureStepsT			INNER JOIN @deleteCategoryT AS deleteCategoryT ON LabProcedureStepsT.TodoCategoryID = deleteCategoryT.CategoryID \r\n"
		// (j.armen 2011-11-22 10:53) - PLID 40420 - Updated to use NexWebEventT
		"UPDATE NexWebEventT			 SET ToDoCategoryID	  = @keepCategoryID FROM NexWebEventT				INNER JOIN @deleteCategoryT AS deleteCategoryT ON NexWebEventT.ToDoCategoryID = deleteCategoryT.CategoryID \r\n"
		"UPDATE NoteDataT                SET Category         = @keepCategoryID FROM NoteDataT					INNER JOIN @deleteCategoryT AS deleteCategoryT ON NoteDataT.Category = deleteCategoryT.CategoryID \r\n"
		"UPDATE NoteMacroT				 SET CategoryID		  = @keepCategoryID FROM NoteMacroT					INNER JOIN @deleteCategoryT AS deleteCategoryT ON NoteMacroT.CategoryID = deleteCategoryT.CategoryID \r\n"
		"UPDATE PacketsT                 SET PacketCategoryID = @keepCategoryID FROM PacketsT					INNER JOIN @deleteCategoryT AS deleteCategoryT ON PacketsT.PacketCategoryID = deleteCategoryT.CategoryID \r\n"
		"UPDATE ToDoList                 SET CategoryID       = @keepCategoryID FROM ToDoList					INNER JOIN @deleteCategoryT AS deleteCategoryT ON ToDoList.CategoryID = deleteCategoryT.CategoryID \r\n"
		// (b.savon 2013-05-29 13:59) - PLID 42902
		"UPDATE DevicePluginConfigT		 SET DefaultCategoryID =@keepCategoryID FROM DevicePluginConfigT		INNER JOIN @deleteCategoryT AS deleteCategoryT ON DevicePluginConfigT.DefaultCategoryID = deleteCategoryT.CategoryID \r\n"

		//(e.lally 2011-11-29) PLID 31585 - Update the preferences that we know are tied to a single category. Note that we aren't doing anything with usernames because this is a system wide change.
		//	We are also assuming that a ConfigRT.number <> 0 needs the same change applied even though at this time none are known to have multiple number instances
		// (j.jones 2012-08-14 14:14) - PLID 50285 - handle LineItemPosting_DedBillingNote_DefCategory
		// (r.goldschmidt 2014-08-05 13:05) - PLID 62717 - handle SttmntSendToHistoryCategory
		// (r.goldschmidt 2015-04-17 13:27) - PLID 64755 - For many preferences/settings that set a category for notes, the preferences are not set up to handle the merging of that category.
		"UPDATE ConfigRT				 SET IntParam		  = @keepCategoryID FROM ConfigRT					INNER JOIN @deleteCategoryT AS deleteCategoryT ON ConfigRT.IntParam = deleteCategoryT.CategoryID "
			"WHERE ConfigRT.Name IN('ERemit_AddCASPR_BillingNote_DefCategory' "
				", 'ERemit_AddDetailedInfo_DefCategory' "
				", 'LabAttachmentsDefaultCategory' "
				", 'LabFollowUpDefaultCategory' "
				", 'LabNotesDefaultCategory' "
				", 'ParseOptionsDefaultCategoryID' "
				", 'PatientHistoryDefaultCategory' "
				", 'PatientHistoryDefaultImageCategory' "
				", 'ScanMultiDocCategory' "
				", 'LineItemPosting_DedBillingNote_DefCategory' "
				", 'SttmntSendToHistoryCategory' "
				", 'ERemitPostAdjustmentNoteWhenZeroPays_DefaultNoteCategory' "
				", 'OMRDefaultImportCategory' "
				", 'SentReminderNoteDefaultCategory' "
				", 'ApptNotesDefaultCategory' "
				", 'RecallNotesDefaultCategory' "
				", 'AttendanceToDoCategory' "
			") \r\n"

		"DELETE NoteCatsF FROM NoteCatsF INNER JOIN @deleteCategoryT AS deleteCategoryT ON NoteCatsF.ID = deleteCategoryT.CategoryID \r\n"

		// (j.jones 2012-04-18 09:51) - PLID 13109 - since we're potentially deleting multiple records with different sort orders,
		// it's easier to just renumber all the sort orders from the beginning
		"DECLARE @orderedCats TABLE (ID INT NOT NULL PRIMARY KEY IDENTITY(1,1), NoteCatID INT NOT NULL) "
		"INSERT INTO @orderedCats (NoteCatID) "
		"SELECT ID FROM NoteCatsF ORDER BY SortOrder "

		"UPDATE NoteCatsF SET SortOrder = C.ID "
		"FROM NoteCatsF "
		"INNER JOIN @orderedCats C ON NoteCatsF.ID = C.NoteCatID"
		);

		try
		{
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(-1);
			GetRemoteData()->BeginTrans();
			batch.Execute(GetRemoteData());
			// (c.haag 2016-05-05 14:59) - NX-100441 - Update the dashboard controls to no longer have the doomed filters
			CConfigurePatDashboardDlg::RemoveMissingFilterValues(pdtHistoryAttachments, HistoryCategory, "NoteCatsF", "ID");
			GetRemoteData()->CommitTrans();
		}
		catch (...)
		{
			GetRemoteData()->RollbackTrans();
			throw;
		}


		//TES 8/25/2011 - PLID 44716 - We need to update the user-defined permission objects for any categories that got deleted.
		for(IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {		
			long nID = VarLong(pRow->GetValue(mlID),-1);
			if(nID != nKeepCategoryID) {
				long nSecurityObjectID = DeleteUserDefinedPermissionFromData(bioHistoryCategories, nID);
				DeleteUserDefinedPermissionFromMemory(nSecurityObjectID);
				// (r.farnworth 2016-03-15 09:46) - PLID 68453 - We need to clear the Online Visits default category if it matches the one we are deleting
				if (GetRemotePropertyInt("OnlineVisitsDocumentCategory", -1, 0, "<None>") == nID) {
					SetRemotePropertyInt("OnlineVisitsDocumentCategory", -1, 0, "<None>");
				}
			}
		}

		//Commit the audit trans
		audit.Commit();

		//this lets the caller know we made changes
		m_bCombined = TRUE;

		//send network messages
		CClient::RefreshTable(NetUtils::NoteCatsF);

	}NxCatchAll(__FUNCTION__);
}


//(e.lally 2011-07-07) PLID 31585 - renamed, changed to DL2 support
void CListMergeDlg::OnEditingFinishedSelectedMergeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		IRowSettingsPtr pCurRow(lpRow);
		if(pCurRow == NULL){
			return;
		}

		if(nCol != mlChecked){
			return;
		}

		//We can only have one row checked at a time, so uncheck all the other selected rows
		if(AsBool(varNewValue) != FALSE){
			m_pSelectedList->SetRedraw(VARIANT_FALSE);
			IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();
			while(pRow != NULL){
				if(!pRow->IsSameRow(pCurRow)){
					pRow->PutValue(mlChecked, VARIANT_FALSE);
				}
				pRow = pRow->GetNextRow();
			}
			m_pSelectedList->SetRedraw(VARIANT_TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-04-10 17:16) - PLID 44174 - added ability to merge resources
void CListMergeDlg::MergeSchedulerResources(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep)
{
	long nAuditTransactionID = -1;
	try {

		CWaitCursor pWait;

		long nResourceIDToKeep = VarLong(pRowToKeep->GetValue(mlID));
		CString strResourceNameToKeep = VarString(pRowToKeep->GetValue(mlName));

		CArray<long, long> aryResourcesToDelete;
		CString strSqlBatch;
		
	
		IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow();
		// (s.tullis 2014-12-12 11:17) - PLID 64440 - Need to prevent this merge if any of the source resources( deleted ones) have schedule mix rule details associated with them.
		CArray<long,long> aryResources;
		{
			while (pRow != NULL)
			{
				if (nResourceIDToKeep != VarLong(pRow->GetValue(mlID)))
				{
					aryResources.Add(VarLong(pRow->GetValue(mlID)));
				}

				pRow = pRow->GetNextRow();
			}

			 
	
			 _RecordsetPtr prs = CreateParamRecordset(" Select ScheduleMixRulesT.Name , ScheduleMixRuleDetailsT.ResourceID FROM ScheduleMixRuleDetailsT "
				"Inner join ScheduleMixRulesT "
				"ON ScheduleMixRulesT.ID = ScheduleMixRuleDetailsT.RuleID "
				"Inner Join ResourceT "
				"ON ResourceT.ID = ScheduleMixRuleDetailsT.ResourceID "
				"where ScheduleMixRuleDetailsT.ResourceID IN ({INTARRAY})", aryResources);

			if (!prs->eof){
				MessageBox("The resources selected to be merged have at least one scheduling mix rule(s) associated with them. Please remove the resources from the scheduling mix rule(s) to complete this merge.", NULL, MB_ICONERROR);
				return;
			}
		}
		//loop through each resource and merge to the selected resource
		pRow = m_pSelectedList->GetFirstRow();
		while(pRow != NULL) {

			long nResourceIDToDelete = VarLong(pRow->GetValue(mlID),-1);
			CString strResourceNameToDelete = VarString(pRow->GetValue(mlName));

			if(nResourceIDToDelete != nResourceIDToKeep) {

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//track this resource
				aryResourcesToDelete.Add(nResourceIDToDelete);

				//update all related records to the new resource, unless the destination record already exists

				AddStatementToSqlBatch(strSqlBatch, "UPDATE AppointmentResourceT SET ResourceID = %li "
					"WHERE ResourceID = %li AND AppointmentID NOT IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID = %li)",
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE PalmResourcesToPurposeSetsT SET ResourceID = %li "
					"WHERE ResourceID = %li AND PurposeSet NOT IN (SELECT PurposeSet FROM PalmResourcesToPurposeSetsT WHERE ResourceID = %li)",
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE PalmSettingsResourceT SET ResourceID = %li "
					"WHERE ResourceID = %li AND PalmSettingsTID NOT IN (SELECT PalmSettingsTID FROM PalmSettingsResourceT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE OutlookAptResourceT SET AptResourceID = %li "
					"WHERE AptResourceID = %li AND CategoryID NOT IN (SELECT CategoryID FROM OutlookAptResourceT WHERE AptResourceID = %li)",
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE WaitingListItemResourceT SET ResourceID = %li "
					"WHERE ResourceID = %li AND ItemID NOT IN (SELECT ItemID FROM WaitingListItemResourceT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE SuperbillTemplateResourceT SET ResourceID = %li "
					"WHERE ResourceID = %li AND GroupID NOT IN (SELECT GroupID FROM SuperbillTemplateResourceT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ApptPrototypePropertySetResourceSetDetailT SET ResourceID = %li "
					"WHERE ResourceID = %li AND ApptPrototypePropertySetResourceSetID NOT IN (SELECT ApptPrototypePropertySetResourceSetID FROM ApptPrototypePropertySetResourceSetDetailT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				if(IsNexTechInternal()) {
					AddStatementToSqlBatch(strSqlBatch, "UPDATE DepartmentResourceLinkT SET ResourceID = %li "
						"WHERE ResourceID = %li AND DepartmentID NOT IN (SELECT DepartmentID FROM DepartmentResourceLinkT WHERE ResourceID = %li)", 
						nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);
				}

				AddStatementToSqlBatch(strSqlBatch, "UPDATE TemplateItemResourceT SET ResourceID = %li "
					"WHERE ResourceID = %li AND TemplateItemID NOT IN (SELECT TemplateItemID FROM TemplateItemResourceT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				//merge only entries that don't exist already for the resource to keep
				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourcePurposeTypeT SET ResourcePurposeTypeT.ResourceID = %li "
					"FROM ResourcePurposeTypeT "
					"LEFT JOIN (SELECT * FROM ResourcePurposeTypeT WHERE ResourceID = %li) AS ResourcePurposeTypeT_KeepExisting ON ResourcePurposeTypeT.AptTypeID = ResourcePurposeTypeT_KeepExisting.AptTypeID AND ResourcePurposeTypeT.AptPurposeID = ResourcePurposeTypeT_KeepExisting.AptPurposeID "
					"WHERE ResourcePurposeTypeT.ResourceID = %li AND ResourcePurposeTypeT_KeepExisting.ResourceID Is Null",
					nResourceIDToKeep, nResourceIDToKeep, nResourceIDToDelete);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceViewDetailsT SET ResourceID = %li "
					"WHERE ResourceID = %li AND ResourceViewID NOT IN (SELECT ResourceViewID FROM ResourceViewDetailsT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE UserResourcesT SET ResourceID = %li "
					"WHERE ResourceID = %li AND UserID NOT IN (SELECT UserID FROM UserResourcesT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				//We can't merge resource locations if the destination resource already has one. 
				//Despite the fact that the data structure permits a resource to have two locations,
				//our existing code only allows and expects one entry per resource in this table.
				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceLocationConnectT SET ResourceID = %li "
					"WHERE ResourceID = %li AND NOT EXISTS (SELECT ResourceID FROM ResourceLocationConnectT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				//Just like resources, we can't merge if the destination resource already has a provider.
				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceProviderLinkT SET ResourceID = %li "
					"WHERE ResourceID = %li AND NOT EXISTS (SELECT ProviderID FROM ResourceProviderLinkT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceUserLinkT SET ResourceID = %li "
					"WHERE ResourceID = %li AND UserID NOT IN (SELECT UserID FROM ResourceUserLinkT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceSetDetailsT SET ResourceID = %li "
					"WHERE ResourceID = %li AND ResourceSetID NOT IN (SELECT ResourceSetID FROM ResourceSetDetailsT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				//merge only entries that don't exist already for the resource to keep
				AddStatementToSqlBatch(strSqlBatch, "UPDATE HL7CodeLinkT SET HL7CodeLinkT.PracticeID = %li "
					"FROM HL7CodeLinkT "
					"LEFT JOIN (SELECT * FROM HL7CodeLinkT WHERE PracticeID = %li AND Type = %li) AS HL7CodeLinkT_KeepExisting "
					"	ON HL7CodeLinkT.HL7GroupID = HL7CodeLinkT_KeepExisting.HL7GroupID "
					"	AND HL7CodeLinkT.ThirdPartyCode = HL7CodeLinkT_KeepExisting.ThirdPartyCode "
					"WHERE HL7CodeLinkT.PracticeID = %li AND HL7CodeLinkT.Type = %li "
					"AND HL7CodeLinkT_KeepExisting.HL7GroupID Is Null",
					nResourceIDToKeep,
					nResourceIDToKeep, hclrtResource,
					nResourceIDToDelete, hclrtResource);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ApptReminderFiltersT SET ApptReminderFiltersT.AppointmentResource = %li "
					"FROM ApptReminderFiltersT "
					"LEFT JOIN (SELECT * FROM ApptReminderFiltersT WHERE AppointmentResource = %li) AS ApptReminderFiltersT_KeepExisting ON ApptReminderFiltersT.AppointmentType = ApptReminderFiltersT_KeepExisting.AppointmentType AND ApptReminderFiltersT.RuleID = ApptReminderFiltersT_KeepExisting.RuleID "
					"WHERE ApptReminderFiltersT.AppointmentResource = %li AND ApptReminderFiltersT.AppointmentResource Is Null",
					nResourceIDToKeep, nResourceIDToKeep, nResourceIDToDelete);

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ResourceAvailTemplateItemResourceT SET ResourceID = %li "
					"WHERE ResourceID = %li AND TemplateItemID NOT IN (SELECT TemplateItemID FROM ResourceAvailTemplateItemResourceT WHERE ResourceID = %li)", 
					nResourceIDToKeep, nResourceIDToDelete, nResourceIDToKeep);

				// (r.goldschmidt 2014-11-07 13:36) - PLID 63835 - Merging resources together does not update the "requested resource field" properly
				AddStatementToSqlBatch(strSqlBatch, "UPDATE AppointmentsT SET RequestedResourceID = %li WHERE RequestedResourceID = %li",
					nResourceIDToKeep, nResourceIDToDelete);

				// (s.tullis 2014-12-12 11:17) - PLID 64440 - Update the resource
				AddStatementToSqlBatch(strSqlBatch, "UPDATE A SET ResourceID = %li  FROM ScheduleMixRuleDetailsT A WHERE ResourceID = %li ", nResourceIDToKeep, nResourceIDToDelete);
				//DeleteSchedulerResource does not support deleting from these tables, because technically you should never
				//be allowed to delete a resource if it is in use in these tables UNLESS you're using this merge process.
				{
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM OutlookAptResourceT WHERE AptResourceID = %li", nResourceIDToDelete);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM WaitingListItemResourceT WHERE ResourceID = %li", nResourceIDToDelete);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM SuperbillTemplateResourceT WHERE ResourceID = %li", nResourceIDToDelete);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ApptPrototypePropertySetResourceSetDetailT WHERE ResourceID = %li", nResourceIDToDelete);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM AppointmentResourceT WHERE ResourceID = %li", nResourceIDToDelete);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PalmResourcesToPurposeSetsT WHERE ResourceID = %li", nResourceIDToDelete);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PalmSettingsResourceT WHERE ResourceID = %li", nResourceIDToDelete);
				}

				// (v.maida 2016-01-22 11:27) - PLID 68035 - For set the default FFA resource preference to the new resource for the appropriate users.
				AddStatementToSqlBatch(strSqlBatch,
					"UPDATE ConfigRT "
					"SET IntParam = %li "
					"WHERE Name = 'FFA_DefaultResourceID' "
					"AND UserName IN (SELECT Username FROM ConfigRT WHERE Name = 'FFA_DefaultResourceID' AND IntParam = %li)", nResourceIDToKeep, nResourceIDToDelete);

				//delete the resource, and any remaining records tied to it
				DeleteSchedulerResource(strSqlBatch, nResourceIDToDelete);
		
				//audit the merge
				CString strOld = FormatString("%s, %s", strResourceNameToDelete, strResourceNameToKeep);
				AuditEvent(-1, strResourceNameToDelete, nAuditTransactionID, aeiResourceMerge, nResourceIDToDelete, strOld, strResourceNameToKeep, aepHigh, aetChanged);
			}

			pRow = pRow->GetNextRow();
		}

		//before committing, get the current resource information for all appointments we are about to change, for auditing
		CArray<long, long> aryApptsToAudit;
		CMap<long, long, CString, LPCTSTR> mapApptIDToOldResourceValue;

		if(aryResourcesToDelete.GetSize() > 0) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT AppointmentID, dbo.GetResourceString(AppointmentID) AS ResourceList "
				"FROM AppointmentResourceT "
				"WHERE ResourceID IN ({INTARRAY}) "
				"GROUP BY AppointmentID", aryResourcesToDelete);
			while(!rs->eof) {
				long nAppointmentID = VarLong(rs->Fields->Item["AppointmentID"]->Value);
				CString strOldResourceList = VarString(rs->Fields->Item["ResourceList"]->Value);

				BOOL bFound = FALSE;
				for(int i=0; i<aryApptsToAudit.GetSize() && !bFound; i++) {
					if(aryApptsToAudit.GetAt(i) == nAppointmentID) {
						bFound = TRUE;
					}
				}
				if(!bFound) {
					aryApptsToAudit.Add(nAppointmentID);
				}

				mapApptIDToOldResourceValue.SetAt(nAppointmentID, strOldResourceList);

				rs->MoveNext();
			}
			rs->Close();
		}

		//commit the changes
		if(!strSqlBatch.IsEmpty()) {
			//this can't be parameterized due to an open-ended count of parameters
			ExecuteSqlBatch(strSqlBatch);

			//this lets the parent window know we made changes
			m_bCombined = TRUE;

			//delete the permissions only after the batch succeeds
			for(int i=0; i<aryResourcesToDelete.GetSize(); i++) {
				long nResourceIDToDelete = aryResourcesToDelete.GetAt(i);
				DeleteUserDefinedPermission(bioSchedIndivResources, nResourceIDToDelete);
			}
		}

		//now audit the changed appointments
		if(aryApptsToAudit.GetSize() > 0) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.ID AS PatientID, "
				"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.[Middle] AS PatientName, "
				"AppointmentsT.ID AS AppointmentID, "
				"dbo.GetResourceString(AppointmentsT.ID) AS ResourceList "
				"FROM AppointmentsT "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"WHERE AppointmentsT.ID IN ({INTARRAY})", aryApptsToAudit);
			while(!rs->eof) {

				long nPatientID = VarLong(rs->Fields->Item["PatientID"]->Value);
				CString strPatientName = VarString(rs->Fields->Item["PatientName"]->Value);
				long nAppointmentID = VarLong(rs->Fields->Item["AppointmentID"]->Value);
				CString strNewResourceList = VarString(rs->Fields->Item["ResourceList"]->Value);
				CString strOldResourceList = "";
				mapApptIDToOldResourceValue.Lookup(nAppointmentID, strOldResourceList);
				
				//audit the change
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiApptResource, nAppointmentID, strOldResourceList, strNewResourceList, aepMedium, aetChanged);

				rs->MoveNext();
			}
			rs->Close();
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		//send network message
		CClient::RefreshTable(NetUtils::Resources);

	}NxCatchAllCall(__FUNCTION__,
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
void CListMergeDlg::MergeProviders(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep)
{
	auto describeResults = [](Nx::SafeArray<IUnknown*>& ar) {

		CString results;

		for (NexTech_Accessor::_DataMerge_MergeDependencyPtr pDep : ar) {
			results.AppendFormat("%li references: %s.\r\n", pDep->References, (const char*)pDep->description);
		}

		results.TrimRight("\r\n");

		return results;
	};


	long nAuditTransactionID = -1;
	try {
		long into = VarLong(pRowToKeep->GetValue(mlID));

		CWaitCursor pWait;

		std::map<long, CString> providers;

		for (IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			long id = VarLong(pRow->GetValue(mlID));

			if (id == into) {
				continue;
			}

			providers.emplace(id, GetExistingContactName(id));
		}

		for (auto&& prov : providers) {
			long from = prov.first;
			const CString& name = prov.second;

			auto pResults = GetAPI()->DataMerge_Providers_Merge(GetAPISubkey(), GetAPILoginToken(), into, from);

			Nx::SafeArray<IUnknown*> arNonmergeable(pResults->Nonmergeable);

			if (0 == arNonmergeable.GetLength()) {
				m_bCombined = TRUE;
			}
			else {
				CString resultStr = describeResults(arNonmergeable);

				MessageBox(FormatString("The provider %s could not be merged due to the following data:\r\n\r\n%s", name, resultStr));
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
void CListMergeDlg::MergeLocations(NXDATALIST2Lib::IRowSettingsPtr pRowToKeep)
{
	auto describeResults = [](Nx::SafeArray<IUnknown*>& ar) {

		CString results;

		for (NexTech_Accessor::_DataMerge_MergeDependencyPtr pDep : ar) {
			results.AppendFormat("%li references: %s.\r\n", pDep->References, (const char*)pDep->description);
		}

		results.TrimRight("\r\n");

		return results;
	};


	long nAuditTransactionID = -1;
	try {
		long into = VarLong(pRowToKeep->GetValue(mlID));

		CWaitCursor pWait;

		std::map<long, CString> locations;

		for (IRowSettingsPtr pRow = m_pSelectedList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			long id = VarLong(pRow->GetValue(mlID));

			if (id == into) {
				continue;
			}

			locations.emplace(id, GetRemoteDataCache().GetLocationName(id));
		}

		for (auto&& loc : locations) {
			long from = loc.first;
			const CString& name = loc.second;

			auto pResults = GetAPI()->DataMerge_Locations_Merge(GetAPISubkey(), GetAPILoginToken(), into, from);

			Nx::SafeArray<IUnknown*> arNonmergeable(pResults->Nonmergeable);

			if (0 == arNonmergeable.GetLength()) {
				m_bCombined = TRUE;
			}
			else {
				CString resultStr = describeResults(arNonmergeable);

				MessageBox(FormatString("The location '%s' could not be merged due to the following data:\r\n\r\n%s", name, resultStr));
			}
		}
	} NxCatchAll(__FUNCTION__);
}

