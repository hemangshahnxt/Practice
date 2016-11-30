// EditMedicationListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientdialog.h"
#include "EditMedicationListDlg.h"
#include "GlobalDataUtils.h"
#include "InactiveMedicationsDlg.h"
#include "LatinPrescriptionMacrosDlg.h"
#include "MultiSelectDlg.h"
#include "EmrUtils.h"
#include "AuditTrail.h"
#include "DontShowDlg.h"
#include "AddComplexMedicationNameDlg.h"
#include "SureScriptsUtils.h"
#include "FirstDataBankUtils.h"
#include "SureScriptsPractice.h"
#include "FirstDatabankImportDlg.h"
#include "PrescriptionUtilsAPI.h"	// (j.jones 2013-03-27 17:25) - PLID 55920 - we really do need the API version here
#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (d.thompson 2009-01-29) - PLID 32175
#define WM_DELAY_DONTSHOW	(WM_USER + 1000)

#define ADD_FDB_MED	37247
#define ADD_FREE_TEXT_MED  37248
// (b.savon 2013-06-20 14:18) - PLID 56880 - Added commands
#define IDM_UPDATE_FDB_MED		37249
#define IDM_SHOW_MONO			37250
#define IDM_SHOW_LEAFLET		37251

// (j.fouts 2013-02-07 10:09) - PLID 55058 - Removed the QuickList column
enum eListColumns {
	elcID = 0,
	elcFullDescription,
	elcDrugName,
	elcStrength,
	elcUnits,
	elcDosageForm,
	elcDefaultRefills,
	elcDefaultQuantity,
	elcQuantityUnits,			// (d.thompson 2009-03-11) - PLID 33477
	elcPatientInstructions,
	elcDEASchedule,		//TES 5/13/2009 - PLID 34260
	elcAllergies,
	//elcQuickList,
	elcNDCNumber,
	elcNotes,		// (d.thompson 2009-01-08) - PLID 32175
	elcFDBID,		// (j.gruber 2010-11-02 12:09) - PLID 39048
	elcDrugTypeID,	// (j.fouts 2012-11-27 16:47) - PLID 51889 - Added a DrugType
	elcDrugType,	// (j.fouts 2012-11-27 16:47) - PLID 51889 - Added a user friendly version
	elcDosageUnitID,	// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage UnitID
	elcDosageUnit,		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
	elcRouteID,			// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage RouteID
	elcRoute,			// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
	elcFDBOutOfDate,	//TES 5/9/2013 - PLID 56614
};

/////////////////////////////////////////////////////////////////////////////
// CEditMedicationListDlg dialog


CEditMedicationListDlg::CEditMedicationListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditMedicationListDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMedicationListDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (a.walling 2009-11-16 13:24) - PLID 36239 - Prevent prompting all the time if the database is inaccessible
	m_bHasEnsuredFirstDataBankOnce = false;
	// (b.savon 2013-07-31 17:28) - PLID 57799 - Init
	m_nDrugListHasOutOfDateFDBMed = 0;
}

void CEditMedicationListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMedicationListDlg)
	DDX_Control(pDX, IDC_BTN_EDIT_LATIN_NOTATION, m_btnLatinNotation);
	DDX_Control(pDX, IDC_MARK_MEDICATION_INACTIVE, m_Inactivate);
	DDX_Control(pDX, IDC_SHOW_INACTIVE_MEDS, m_Show_Inactive);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_MED_SHOW_ADV_DRUG_NAMES, m_btnShowAdvDrugNames);
	DDX_Control(pDX, IDC_SHOW_DRUG_NOTES, m_btnShowDrugNotes);
	DDX_Control(pDX, IDC_ADV_MED_HELP, m_btnHelp);
	DDX_Control(pDX, IDC_EDIT_COMBO_BKG, m_nxcBack);
	DDX_Control(pDX, IDC_UPDATE_ALL_MEDICATIONS, m_btnUpdateAllMedications);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEditMedicationListDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditMedicationListDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_EDIT, OnEdit)
	ON_BN_CLICKED(IDC_MARK_MEDICATION_INACTIVE, OnMarkMedicationInactive)
	ON_BN_CLICKED(IDC_SHOW_INACTIVE_MEDS, OnShowInactiveMeds)
	ON_BN_CLICKED(IDC_BTN_EDIT_LATIN_NOTATION, OnBtnEditLatinNotation)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MED_SHOW_ADV_DRUG_NAMES, &CEditMedicationListDlg::OnBnClickedMedShowAdvDrugNames)
	ON_BN_CLICKED(IDC_SHOW_DRUG_NOTES, OnBnClickedShowDrugNotes)
	ON_MESSAGE(WM_DELAY_DONTSHOW, OnDelayedDontShow)
	ON_BN_CLICKED(IDC_ADV_MED_HELP, &CEditMedicationListDlg::OnBnClickedAdvMedHelp)
	ON_COMMAND(ADD_FDB_MED, OnImportMedications)
	ON_COMMAND(ADD_FREE_TEXT_MED, OnAddFreeTextMed)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_UPDATE_ALL_MEDICATIONS, &CEditMedicationListDlg::OnUpdateAllMedications)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMedicationListDlg message handlers

BEGIN_EVENTSINK_MAP(CEditMedicationListDlg, CNxDialog)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 10, CEditMedicationListDlg::OnEditingFinishedEditList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 9, CEditMedicationListDlg::OnEditingFinishingEditList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 2, CEditMedicationListDlg::OnSelChangedEditList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 5, CEditMedicationListDlg::OnLButtonUpEditList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 18, CEditMedicationListDlg::OnRequeryFinishedEditList, VTS_I2)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 17, CEditMedicationListDlg::OnColumnClickingMedicationList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 8, CEditMedicationListDlg::EditingStartingEditList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEditMedicationListDlg, IDC_EDIT_LIST, 7, CEditMedicationListDlg::RButtonUpEditList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

BOOL CEditMedicationListDlg::OnInitDialog() 
{
	//DRT 11/25/2008 - PLID 32175 - Cleaned up to use a datalist 2, reworked to add the EnsureControls function
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 12:28) - PLID 29790 - NxIconified buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnEdit.AutoSet(NXB_MODIFY);
		m_Inactivate.AutoSet(NXB_MODIFY);
		// (d.thompson 2009-03-18) - PLID 33479
		m_btnHelp.SetIcon(IDI_BLUE_QUESTION);
		// (b.savon 2013-01-21 12:38) - PLID 54722
		m_nxcBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		// (d.thompson 2009-01-29) - PLID 32175 - Add bulk caching for new preferences
		g_propManager.CachePropertiesInBulk("EditMedicationList", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'MedicationShowAdvDrugNames' OR "
				"Name = 'MedicationShowDrugNotes' "
				")",
				_Q(GetCurrentUserName()));

		// (a.walling 2007-04-04 15:19) - PLID 25459 - Failsafe - if user lacks Edit Medication List permission,
		// exit the dialog! (will NOT prompt for password; assume it is known. It is the responsibility of the
		// caller to check permissions. This is just a failsafe!)
		if(!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0, FALSE, 0, TRUE, TRUE)) {
			ASSERT(FALSE); // the caller should be checking permissions, not us.
			LogDetail("ERROR: CEditMedicationListDlg::OnInitDialog() cannot initialize due to lack of permissions.");
			CDialog::OnCancel();
		}

		//Initialize the datalist
		m_pList = BindNxDataList2Ctrl(IDC_EDIT_LIST, false);

		// (j.fouts 2013-02-07 10:23) - PLID 55058 - We have changed the way quick lists work
		//TES 5/16/2008 - PLID 28523 - Pull the remote property defining our "quick list" of medications, and fill that column
		// accordingly.
		//CArray<int,int> aryQuickListMeds;
		//GetRemotePropertyArray("MedicationsQuickList", aryQuickListMeds, 0, GetCurrentUserName());

		//m_pList->GetColumn(elcQuickList)->FieldName = _bstr_t(CString("convert(bit,CASE WHEN DrugList.ID IN (" + ArrayAsString(aryQuickListMeds, false) + ") THEN 1 ELSE 0 END)"));
		m_pList->Requery();

		// (c.haag 2007-02-13 08:50) - PLID 24728 - If this is the current medications
		// item, we have to limit the row name size to 255
		// (d.thompson 2008-12-10) - PLID 32175 - This is the "description" field now, not the drug name
		m_pList->GetColumn(elcFullDescription)->MaxTextLen = 255;

		//Update buttons appropriately
		EnsureControls();

		// (a.walling 2009-11-18 11:01) - PLID 36339 - Show the NDC column by default in the medication list if we are licensed for SureScripts
		// (j.jones 2010-01-26 16:17) - PLID 37077 - or if they have NewCrop
		if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptNewCrop || SureScripts::IsEnabled()) {
			// show the NDC column
			try {
				IColumnSettingsPtr pCol = m_pList->GetColumn(elcNDCNumber);
				pCol->PutStoredWidth(80);
				pCol->PutColumnStyle(csVisible|csFixedWidth|csEditable);
			} NxCatchAll("Error sizing NDC column");
		}

		// (d.thompson 2008-11-25) - PLID 32175 - Advanced display options
		//	Default AdvDrugNames to on (they will show)
		CheckDlgButton(IDC_MED_SHOW_ADV_DRUG_NAMES, GetRemotePropertyInt("MedicationShowAdvDrugNames", 1, 0, GetCurrentUserName(), true));
		OnBnClickedMedShowAdvDrugNames();

		// (d.thompson 2009-01-08) - PLID 32175 - Same deal, but for the notes field
		CheckDlgButton(IDC_SHOW_DRUG_NOTES, GetRemotePropertyInt("MedicationShowDrugNotes", 0, 0, GetCurrentUserName(), true));
		OnBnClickedShowDrugNotes();

		// (d.thompson 2009-01-29) - PLID 32175 - If we popup a modal dialog during initDialog, this dialog will position
		//	itself oddly.  So let this finish and post a message to pop up the dontshowme dialog.
		PostMessage(WM_DELAY_DONTSHOW, 0, 0);

	} NxCatchAll("Error in CEditMedicationListDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditMedicationListDlg::OnEditingFinishedEditList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		//If we aren't committing... don't commit
		if(bCommit == FALSE)
			return;

		//ensure a valid row
		if(lpRow == NULL) {
			return;
		}

		IRowSettingsPtr pCurrentRow(lpRow);

		long nID = VarLong(pCurrentRow->GetValue(elcID));
		switch (nCol) {
			case elcFullDescription:
				{
					// (d.thompson 2008-12-10) - PLID 32175 - Previously this was doing a comparison to see if the name
					//	changed by looking up the old name in data.  But we get the old value here as a parameter, so
					//	just use that and save the data access.  Since SQL queries are case insensitive, this will be
					//	as well.
					CString strOldName = VarString(varOldValue);
					CString strNewName = VarString(varNewValue);
					if (strOldName.CompareNoCase(strNewName) != 0) {
						ExecuteNameChange(nID, strNewName);
					}
				}
				break;
			case elcDefaultRefills:
				{
					ExecuteParamSql("UPDATE DrugList SET DefaultRefills = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;

			case elcDefaultQuantity:
				{
					ExecuteParamSql("UPDATE DrugList SET DefaultQuantity = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;
			// (s.dhole 2012-11-16 10:53) - PLID 53698
			//case elcQuantityUnits: 
			//	{
			//		// (d.thompson 2009-03-11) - PLID 33477 - Added quantity units
			//		//Workaround for PLID 33469.  Even though the selection is NULL, it's returning a VT_I4 with a value of 0.  We need
			//		//	to manually fix that.
			//		_variant_t varValueToSave = varNewValue;
			//		if(varValueToSave.vt == VT_I4 && VarLong(varValueToSave) == 0) {
			//			varValueToSave.vt = VT_NULL;
			//		}
			//		ExecuteParamSql("UPDATE DrugList SET QuantityUnitID = {VT_I4} WHERE ID = {INT};", varValueToSave, nID);
			//	}
			//	break;

			case elcPatientInstructions:
				{
					ExecuteParamSql("UPDATE DrugList SET PatientInstructions = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;
// (j.fouts 2013-02-07 10:23) - PLID 55058 - This check box has been removed
//			case elcQuickList:
//				{
//					//TES 5/16/2008 - PLID 28523 - Find all the checked rows, and update the ConfigRT property.
//					CArray <int,int> aryQuickListMeds;
//					IRowSettingsPtr pRowSearch = m_pList->GetFirstRow();
//					while (pRowSearch != NULL) {
//						if(VarBool(pRowSearch->GetValue(elcQuickList))) {
//							aryQuickListMeds.Add(VarLong(pRowSearch->GetValue(elcID)));
//						}
//
//						pRowSearch = pRowSearch->GetNextRow();
//					}
//
//					CString strQuickList = GetCommaDeliminatedText(aryQuickListMeds);
//					// (z.manning 2009-09-02 11:15) - plid 34691 - Failsafe to avoid sql error
//					if(strQuickList.GetLength() > 3500) {
//						MessageBox("You have reached the limit of quick list medications.");
//						pCurrentRow->PutValue(elcQuickList, g_cvarFalse);
//					}
//					else {
//						SetRemotePropertyArray("MedicationsQuickList", aryQuickListMeds, 0, GetCurrentUserName());
//					}
//				}
//				break;

			case elcNDCNumber:
				{
					ExecuteParamSql("UPDATE DrugList SET NDCNumber = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;

			// (d.thompson 2009-01-08) - PLID 32175 - Notes field is editable
			case elcNotes:
				{
					ExecuteParamSql("UPDATE DrugList SET Notes = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;

			//TES 5/13/2009 - PLID 34260 - Added a field for the DEA Schedule (I, II, III, IV, or V)
			case elcDEASchedule:
				{
					ExecuteParamSql("UPDATE DrugList SET DEASchedule = {STRING} WHERE ID = {INT};", VarString(varNewValue,""), nID);
				}
				break;

		}	//end switch

	} NxCatchAll("Error in OnEditingFinishedEditList");
}

void CEditMedicationListDlg::OnAdd() 
{
	try {
		// (j.fouts 2012-09-25 09:27) - PLID 52825 - Only allow importing if they have the FDB license
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			// (j.gruber 2010-10-27 12:14) - PLID 39049 make a menu for entering free text or FDB medications
			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;
			

			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ADD_FDB_MED, "Import &Medication");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ADD_FREE_TEXT_MED, "&Add Free Text Medication");

			CRect rBtn;
			GetDlgItem(IDC_ADD)->GetWindowRect(rBtn);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
		}
		else
		{
			OnAddFreeTextMed();
		}

	}NxCatchAll(__FUNCTION__);
}

void CEditMedicationListDlg::OnImportMedications() 
{

	try {

		CFirstDataBankImportDlg dlg(this);
		long nResult = dlg.DoModal();

		//requery
		if (nResult == IDOK) {
			m_pList->Requery();		
		}

	}NxCatchAll(__FUNCTION__);

}

void CEditMedicationListDlg::OnAddFreeTextMed() {

	try {

		// (j.fouts 2012-09-25 09:27) - PLID 52825 - Only allow importing if they have the FDB license
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			// (j.gruber 2010-11-01 11:45) - PLID 3909 - pop up a message saying we don't recommend this
			// (j.fouts 2013-02-08 10:07) - PLID 53958 - Reworded this to be more accurate now
			if (IDYES == MsgBox(MB_YESNO, "The following dialog will allow you to enter a Free Text Medication, Supply, or Compound Medication. Supplies and Compound Medications "
				"created this way may still be used with E-Prescribing, however we strongly recommend using an imported medication as a compound base.\n\n"
				"It is recommended that you search for a non-Free Text Medication and add that instead.\n\n"
				"Would you like to CANCEL adding a Free Text Medication and search for a non-Free Text Medication instead?")) {
					OnImportMedications();
					return;
			}
		}

		// (b.savon 2013-05-10 15:48) - PLID 56650 - Assure message box for controlled substance.
		if( IDNO == MessageBox("NexTech may not e-prescribe controlled substances and it is against the law to do so.\r\n\r\n"
							   "By clicking 'Yes' below, you are assuring the NDC, medication, supply, or compound you enter is NOT a controlled "
							   "substance.\r\n\r\nDo you assure this is NOT a controlled substance?", "NexTech Practice", MB_ICONQUESTION|MB_YESNO) ){
			return;
		}

		// (d.thompson 2008-11-25) - PLID 32175 - Instead of entering a silly "enter medication name" item
		//	and forcing the user to save, we will now offer a popup dialog which contains all the drug info
		//	that can be entered.  It will then auto calculate the name for the user.
		CAddComplexMedicationNameDlg dlg(this);
		//no default values to provide
		if(dlg.BeginAddNew() == IDOK) {
			//Get the values back and create a datalist row for them.
			IRowSettingsPtr pRow = m_pList->GetNewRow();
			long nNewID = NewNumber("DrugList", "ID");

			_variant_t varStrengthUnit(g_cvarNull);
			_variant_t varDosageForm(g_cvarNull);
			if(dlg.m_nStrengthUnitID != -1) {
				varStrengthUnit = (long)dlg.m_nStrengthUnitID;
			}
			if(dlg.m_nDosageFormID != -1) {
				varDosageForm = (long)dlg.m_nDosageFormID;
			}
			// (s.dhole 2012-11-16 10:55) - PLID 53698
			_variant_t varQuntityUnit(g_cvarNull);
			if(dlg.m_nQuntityUnitID != -1) {
				varQuntityUnit = (long)dlg.m_nQuntityUnitID;
			}

			// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
			_variant_t varDosageUnitID(g_cvarNull);
			if(dlg.m_nDosageUnitID != -1)
			{
				varDosageUnitID = (long)dlg.m_nDosageUnitID;
			}

			// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
			_variant_t varDosageRouteID(g_cvarNull);
			if(dlg.m_nRouteID != -1)
			{
				varDosageRouteID = (long)dlg.m_nRouteID;
			}


			pRow->PutValue(elcID, nNewID);
			pRow->PutValue(elcFullDescription, _bstr_t(dlg.m_strFullDrugDescription));
			pRow->PutValue(elcDrugName, _bstr_t(dlg.m_strDrugName));
			pRow->PutValue(elcStrength, _bstr_t(dlg.m_strStrength));
			pRow->PutValue(elcUnits, varStrengthUnit);
			pRow->PutValue(elcDosageForm, varDosageForm);
			pRow->PutValue(elcDefaultRefills, _bstr_t("0"));
			pRow->PutValue(elcDefaultQuantity, _bstr_t("0"));
			// (d.thompson 2009-03-11) - PLID 33477 - Quantity units default to NULL until we do the addition
			pRow->PutValue(elcQuantityUnits, varQuntityUnit);	// (s.dhole 2012-11-16 10:55) - PLID 53698
			pRow->PutValue(elcPatientInstructions, _bstr_t(""));
			//TES 5/14/2009 - PLID 34260 - DEA Schedule defaults to a blank string
			pRow->PutValue(elcDEASchedule, _bstr_t(""));
			pRow->PutValue(elcAllergies, _bstr_t("<None>"));
			// (j.fouts 2013-02-07 10:23) - PLID 55058 - Removed this column
			//pRow->PutValue(elcQuickList, g_cvarFalse);
			pRow->PutValue(elcNDCNumber, _bstr_t(""));
			pRow->PutValue(elcNotes, _bstr_t(""));
			// (j.gruber 2010-11-02 12:10) - PLID 30948
			pRow->PutValue(elcFDBID, g_cvarNull);

			// (j.fouts 2012-11-27 16:47) - PLID 51889 - Add new fields to the datalist
			pRow->PutValue(elcNotes, _bstr_t(dlg.m_strNotes));
			pRow->PutValue(elcDrugTypeID, MapDrugTypeToData(dlg.m_drugType));
			// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
			pRow->PutValue(elcDosageUnitID, varDosageUnitID);
			pRow->PutValue(elcDosageUnit, _bstr_t(dlg.m_strDosageUnit));
			// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
			pRow->PutValue(elcRouteID, varDosageRouteID);
			pRow->PutValue(elcRoute, _bstr_t(dlg.m_strDosageRoute));
			switch(dlg.m_drugType)
			{
			case NexTech_Accessor::DrugType_NDC:
				pRow->PutValue(elcDrugType, _bstr_t("Medication"));
				break;
			case NexTech_Accessor::DrugType_Supply:
				pRow->PutValue(elcDrugType, _bstr_t("Supply"));
				break;
			case NexTech_Accessor::DrugType_Compound:
				pRow->PutValue(elcDrugType, _bstr_t("Compound"));
				break;
			default:
				pRow->PutValue(elcDrugType, _bstr_t(""));
				break;
			}

			// (c.haag 2007-01-30 16:27) - PLID 24422 - We now add the record to data within
			// ExecuteAddition. This also updates the system Current Medications info item
			// (d.thompson 2008-12-01) - PLID 32175 - Added the drug description to the addition setup
			// (s.dhole 2012-11-16 11:00) - PLID 53698 Added varQuntityUnit
			// (j.fouts 2012-11-27 16:47) - PLID 51889 - Added DrugType and Notes
			// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
			// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit	
			ExecuteAddition(nNewID, dlg.m_strFullDrugDescription, dlg.m_strDrugName, dlg.m_strStrength, 
				varStrengthUnit, varDosageForm, varQuntityUnit, dlg.m_strNotes, dlg.m_drugType, varDosageUnitID, varDosageRouteID);

			// (d.thompson 2009-03-11) - PLID 33477 - Set the qty units if we got a return value
			// (s.dhole 2012-11-16 11:02) - PLID 53698
			//if(nQtyUnitID != -1) {
			//	pRow->PutValue(elcQuantityUnits, (long)nQtyUnitID);
			//}

			//Done, add it to the datalist
			m_pList->AddRowSorted(pRow, NULL);
			// (d.thompson 2008-12-03) - PLID 32175 - We want the combo columns to also display as links, but
			//	there's no default setting in the datalist for it, so we need to apply it manually.
			ApplyLinkStyleToCell(pRow, elcDosageForm);
			ApplyLinkStyleToCell(pRow, elcUnits);
			ApplyLinkStyleToCell(pRow, elcQuantityUnits);// (s.dhole 2012-11-16 10:55) - PLID 53698
			m_pList->SetSelByColumn(elcID, nNewID);
			EnsureControls();
		}

	} NxCatchAll("Error in OnAdd");
}

void CEditMedicationListDlg::OnDelete() 
{
	try {
		//check to see that something is selected
		IRowSettingsPtr pRow = m_pList->CurSel;

		if (pRow != NULL) {
			// (c.haag 2007-03-05 09:15) - PLID 25056 - Medications are linked with EMR table items.
			// An EMR table item must have at least one active row. Given that, we cannot allow a user
			// to delete the last medication.
			if (!HasMultipleActiveMedications())
				return;

			//check to see that there are no patients with that medication
			long nDelID = VarLong(pRow->GetValue(elcID));
			CString strMessage;
			CString strCounts;
			if (IsMedicationInUse(nDelID, strCounts)) {
				strMessage.Format("This medication is in use on:\n%s\nYou may not delete it.", strCounts);
				MessageBox(strMessage, "NexTech Practice", MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			// (z.manning 2009-03-17 14:20) - PLID 33242 - Don't allow them to delete this medication
			// if it has somehow been involved in table spawning.
			if(IsDataIDUsedBySpawnedObject(FormatString("SELECT EmrDataID FROM DrugList WHERE ID = %li",nDelID))) {
				MessageBox("This medication is referenced by spawned objects on patient EMNs. It cannot be deleted.");
				return;
			}

			// (c.haag 2009-03-12 09:59) - PLID 32589 - See if this medication is linked with EMR actions.
			// If so, we need to use a different message.
			_RecordsetPtr prs = CreateParamRecordset(
				FormatString(
					"DECLARE @DelID INT\r\n "
					"SET @DelID = {INT}\r\n "
					"SELECT Name FROM ("
						"/* EMR Items */ "
						"SELECT EmrInfoT.Name "
						"FROM EmrActionsT "
						"INNER JOIN EMRInfoT ON EMRActionsT.SourceID = EMRInfoT.ID "
						"WHERE EmrActionsT.Deleted = 0 AND EMRActionsT.SourceType = %d AND EmrActionsT.DestType = %d AND EmrActionsT.DestID = @DelID "
						"/* EMR List Options */ "
						"UNION SELECT EmrInfoT.Name "
						"FROM EmrActionsT "
						"INNER JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID "
						"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
						"WHERE EmrActionsT.Deleted = 0 AND EMRActionsT.SourceType = %d AND EmrActionsT.DestType = %d AND EmrActionsT.DestID = @DelID "
						"/* EMR HotSpots */ "
						"UNION SELECT EmrInfoT.Name "
						"FROM EmrActionsT "
						"INNER JOIN EmrImageHotSpotsT ON EMRActionsT.SourceID = EmrImageHotSpotsT.ID "
						"INNER JOIN EMRInfoT ON EmrImageHotSpotsT.EMRInfoID = EMRInfoT.ID "
						"WHERE EmrActionsT.Deleted = 0 AND EMRActionsT.SourceType = %d AND EmrActionsT.DestType = %d AND EmrActionsT.DestID = @DelID "
						"/* EMR Table Dropdowns */ "
						"UNION SELECT EmrInfoT.Name "
						"FROM EmrActionsT "
						"INNER JOIN EmrTableDropdownInfoT ON EMRActionsT.SourceID = EmrTableDropdownInfoT.ID "
						"INNER JOIN EMRDataT ON EmrTableDropdownInfoT.EMRDataID = EMRDataT.ID "
						"INNER JOIN EMRInfoT ON EMRDataT.EMRInfoID = EMRInfoT.ID "
						"WHERE EmrActionsT.Deleted = 0 AND EMRActionsT.SourceType = %d AND EmrActionsT.DestType = %d AND EmrActionsT.DestID = @DelID "
						//"/* Smart Stamps */ "
						// (z.manning 2010-03-02 14:48) - PLID 37571 - Smart stamp actions are not tied to info items
						// so we don't handle them here.
					") SubQ "
					"GROUP BY Name "
					"ORDER BY Name \r\n"
					//(e.lally 2009-05-11) PLID 28553 - Piggy back this sql query with another one for Order Set Templates
					"SELECT COUNT(DISTINCT OrderSetTemplateID) as OrderSetTmplCount "
					"FROM OrderSetTemplateMedicationsT "
					//@DelID is actually our medicationID
					"WHERE MedicationID = @DelID \r\n"
					,eaoEmrItem, eaoMedication
					,eaoEmrDataItem, eaoMedication
					,eaoEmrImageHotSpot, eaoMedication
					,eaoEmrTableDropDownItem, eaoMedication
					)
				, nDelID);
			//(e.lally 2009-05-11) PLID 28553 - Start with the standard message and we'll overwrite it if we need to.
			//standard message
			strMessage = "Are you sure you want to delete this Medication?";

			if (!prs->eof) {
				FieldsPtr f = prs->Fields;
				strMessage = "This medication is configured to spawn by the following EMR items:\r\n\r\n";
				int i = 0;
				while (!prs->eof) {
					if (i > 20) {
						strMessage += "<more>\r\n";
						break;
					} else {
						strMessage += AdoFldString(f, "Name", "") + "\r\n";
						i++;
					}
					prs->MoveNext();
				}
				strMessage += "\r\nAre you sure you want to delete this Medication?";
			}
			//Move to the next query
			prs = prs->NextRecordset(NULL);
			if (!prs->eof) {
				//(e.lally 2009-05-11) PLID 28553 - How many Order Set Templates use this medication.
				long nTemplateCount = AdoFldLong(prs, "OrderSetTmplCount", 0);
				if(nTemplateCount > 0){
					strMessage.Format("This Medication will be removed from %li Order Set Template(s).\n"
						"Are you sure you want to delete this Medication?", nTemplateCount);
				}
			}
			prs->Close();

			if (IDYES == MessageBox(strMessage, "NexTech Practice", MB_YESNO|MB_ICONQUESTION)) {				
				//we can delete it!

				// (c.haag 2007-01-30 16:27) - PLID 24422 - We now delete the record from data within
				// ExecuteDeletion. This also updates the system Current Medications info item
				ExecuteDeletion(nDelID);

				m_pList->RemoveRow(pRow);
			}
			else {
				//they don't want to delete, so do nothing
			}
		}
		else {
			MessageBox("Please select a Medication to Delete");
		}
	} NxCatchAll("Error in OnDelete()");
}

void CEditMedicationListDlg::OnEdit() 
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;

		// (d.thompson 2008-11-26) - PLID 32175 - Since the name is in a popup now, 
		//	let's just pop it up.
		OnLButtonUpEditList(pRow, elcUnits, 0, 0, 0);
	} NxCatchAll("Error in OnEdit");	
}

void CEditMedicationListDlg::OnOK() 
{
	try {
		//close the dialog
		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

BOOL CEditMedicationListDlg::IsMedicationInUse(long nMedicationID, CString &strCounts)
{
	//DRT 11/26/2008 - PLID 32177 - Parameterized and cleaned up bad querying practices by joining many 
	//	SELECT statements into a single batch.
	CString strSqlBatch;
	CNxParamSqlArray args;

	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Count(ID) AS Count FROM PatientMedications WHERE MedicationID = {INT} AND Deleted = 0;\r\n", nMedicationID);
	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Count(MedicationID) as Count FROM CurrentPatientMedsT WHERE MedicationID = {INT};\r\n", nMedicationID);
	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Count(MedicationID) as Count FROM MedSchedDetailsT WHERE MedicationID = {INT};\r\n", nMedicationID);
	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Count(ID) as Count FROM PatientMedications WHERE MedicationID = {INT} "
		" AND ID IN (SELECT MedicationID FROM EMRMedicationsT);\r\n", nMedicationID);
	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Count(DISTINCT EMRTemplateID) as Count FROM EMRTemplatePrescriptionsT WHERE MedicationID = {INT};\r\n", nMedicationID);
	// (a.walling 2009-04-23 10:23) - PLID 34046 - Prevent deletion if tied to _any_ surescripts messages
	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT Count(SureScriptsMessagesT.ID) AS Count FROM SureScriptsMessagesT INNER JOIN PatientMedications ON SureScriptsMessagesT.PatientMedicationID = PatientMedications.ID WHERE PatientMedications.MedicationID = {INT};\r\n", nMedicationID);
	// (c.haag 2007-02-12 17:22) - PLID 24422 - Do not delete if the medication is in use by an EMR detail
	AddParamStatementToSqlBatch(strSqlBatch, args,
		"SELECT Count(*) as Count FROM EMRDetailTableDataT "
		"LEFT JOIN EmrDataT ON EmrDataT.ID = EMRDetailTableDataT.EMRDataID_Y "
		"WHERE "
		// The table data belongs to a detail that is not deleted
		"EmrDetailID IN (SELECT ID FROM EMRDetailsT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Deleted = 0)) "
		// The table data is not populated (regardless of column)
		// (z.manning 2008-12-10 16:00) - PLID 32389 - We now check for non-blank data in all column types
		// (a.walling 2009-04-03 10:44) - PLID 33831 - blank data is no longer saved to the database
		/*
		"	AND Len(EMRDetailTableDataT.Data) > 0 \r\n"
		"	AND ( \r\n"
		"		(ListType = 4 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // dropdown
		"		OR (ListType = 5 AND EMRDetailTableDataT.Data <> '0' AND EMRDetailTableDataT.Data <> '-1') \r\n" // checkbox
		"	) \r\n"					
		*/
		// The table row belongs to this medication
		"AND EMRDataID_X IN (SELECT EMRDataID FROM DrugList WHERE ID = {INT});\r\n", nMedicationID);
	// (b.savon 2013-03-21 17:40) - PLID 54831 - They can't delete a med that is used on a quick list.
	AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT COUNT(DrugListID) AS Count FROM NexERxQuickListT WHERE DrugListID = {INT}; \r\n", nMedicationID);

	//execute all the queries
	// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
	_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);

	CString str;
	strCounts = "";
	BOOL bNoDelete = FALSE;


	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count", -1);
		if (nCount > 0) {
			//we can't delete
			str.Format(" - %li prescriptions\n",  nCount);
			strCounts += str;
			bNoDelete = TRUE;
		}
	}

	//Move to the next query
	prs = prs->NextRecordset(NULL);
	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count");
		if (nCount > 0) {
			//we can't delete
			str.Format(" - %li current medications\n",  nCount);
			strCounts += str;
			bNoDelete = TRUE;
		}
	}

	//Move to the next query
	prs = prs->NextRecordset(NULL);
	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count");

		if (nCount > 0) {
			//we can't delete
			str.Format(" - %li medication schedules\n",  nCount);
			strCounts += str;
			bNoDelete = TRUE;
		}
	}

	//Move to the next query
	prs = prs->NextRecordset(NULL);
	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count");
		if (nCount > 0) {
			//we can't delete
			str.Format(" - %li EMN prescriptions (They may be marked as deleted.)\n",  nCount);
			strCounts += str;
			bNoDelete = TRUE;
		}
	}

	//Move to the next query
	prs = prs->NextRecordset(NULL);
	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count");

		if (nCount > 0) {
			//we can't delete
			str.Format(" - %li EMN Templates. (They may be marked as deleted.)\n", nCount);
			strCounts += str;
			bNoDelete = TRUE;
			
		}
	}

	//Move to the next query
	prs = prs->NextRecordset(NULL);
	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count");

		if (nCount > 0) {
			//we can't delete
			// (a.walling 2009-04-23 10:25) - PLID 34046
			str.Format(" - %li SureScripts Messages (They may be marked as deleted.)\n", nCount);
			strCounts += str;
			bNoDelete = TRUE;			
		}
	}

	//Move to the next query
	prs = prs->NextRecordset(NULL);
	if (!prs->eof) {
		long nCount = AdoFldLong(prs, "Count");
		if (nCount > 0) {
			str.Format(" - %li patient medical records", nCount);
			strCounts += str;
			bNoDelete = TRUE;
		}
	}

	// (b.savon 2013-03-21 17:43) - PLID 54831
	//Move on
	prs = prs->NextRecordset(NULL);
	if( !prs->eof ){
		long nCount = AdoFldLong(prs, "Count");
		if( nCount > 0 ){
			if( nCount == 1 ){
				str.Format(" - %li prescription quick list", nCount);
			}else{
				str.Format(" - %li prescription quick lists", nCount);
			}
			strCounts += str;
			bNoDelete = TRUE;
		}
	}

	return bNoDelete;
}

void CEditMedicationListDlg::OnEditingFinishingEditList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		//already chosen to quit
		if(*pbCommit == FALSE)
			return;

		IRowSettingsPtr pCurrentRow(lpRow);
		if(pCurrentRow == NULL) {
			return;
		}

		switch (nCol) {
			case elcFullDescription:
				{
					// (d.thompson 2008-12-10) - PLID 32175 - Reworked this slightly so all the checks use the same
					//	variable for checking the text.  I threw out 'strUserEntered' in favor of 'pvarNewValue'.
					CString strNewDesc = VarString(*pvarNewValue);
					strNewDesc.TrimLeft();
					strNewDesc.TrimRight();
					if (strNewDesc.IsEmpty()) {
						//Not allowed to submit an empty name
						MessageBox("You may not save a medication with a blank name, please edit your description and try again.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						return;
					}

					long nMedicationID = VarLong(pCurrentRow->GetValue(elcID));
					_RecordsetPtr prsTest = CreateParamRecordset("SELECT EmrDataT.Data FROM DrugList "
						"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
						"WHERE DrugList.ID <> {INT} AND EmrDataT.Data = {STRING}", nMedicationID, strNewDesc);
					if(!prsTest->eof) {
						MessageBox("The data you entered already exists in the list.  Please enter a unique name.");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						return;
					}

					// (j.gruber 2007-08-09 12:54) - PLID 26973 - don't let them change medication names if they are in use
					if (VarString(varOldValue, "").Compare(strNewDesc) != 0) {
						//Only compare if the name actually changed
						CString strMessage;
						CString strCounts;

						if (IsMedicationInUse(nMedicationID, strCounts)) {
							strMessage.Format("This medication is in use on:\n%s\nYou may not rename it.", strCounts);
							MsgBox(strMessage);
							*pbCommit = FALSE;
							*pbContinue = TRUE;
							return;
						}
					}
				}
				break;
			case elcDefaultRefills:
			case elcDefaultQuantity:
				{
					CString strTemp = strUserEntered;
					strTemp.TrimLeft();
					strTemp.TrimRight();
					if(strTemp.IsEmpty()) {
						//Don't commit.
						MessageBox("You cannot enter a blank amount.", "NexTech Practice", MB_OK|MB_ICONEXCLAMATION);
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
					if(strTemp.GetLength() > 50) {
						MessageBox("The amount cannot have more than 50 characters.", "NexTech Practice", MB_OK|MB_ICONEXCLAMATION);
						*pbCommit = FALSE;
						*pbContinue = FALSE;
					}
				}
				break;

			case elcNDCNumber:
				{
					//TES 10/8/2009 - PLID 35877 - Only do this validation if they have the FDB license
					if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
						//TES 9/29/2009 - PLID 34252 - Validate this number against our FirstDataBank data.					
						CString strTemp = strUserEntered;
						strTemp.TrimLeft();
						strTemp.TrimRight();
						if(strTemp.IsEmpty()) {
							//TES 9/29/2009 - PLID 34252 - Don't give them messages on an empty string, they know it's empty.
							return;
						}
						//TES 9/29/2009 - PLID 34252 - First off, if this isn't 11 numeric digits, then it's not valid.
						CString strClean = SureScripts::FormatNDCNumberType(strTemp);
						if(strClean.GetLength() != 11) {
							if(IDYES == MsgBox(MB_YESNO, "Warning: The NDC Number you have entered is not a valid NDC Number.  Any prescriptions submitted using this number may be rejected.\r\n"
								"Would you like to return to editing this field?")) {
								*pbCommit = FALSE;
								*pbContinue = FALSE;
							}
						}
						else {
							//TES 10/1/2009 - PLID 34252 - Ask FirstDataBank if it's legit.
							CString strDrugName = VarString(pCurrentRow->GetValue(elcFullDescription),"");
							bool bContinueEditing = false;
							// (a.walling 2009-11-16 13:24) - PLID 36239 - Ensure we have a db
							bool bIsDatabaseValid = false;
							if (!m_bHasEnsuredFirstDataBankOnce) {
								bIsDatabaseValid = FirstDataBank::EnsureDatabase(this, true);
								m_bHasEnsuredFirstDataBankOnce = true;
							} else {
								bIsDatabaseValid = FirstDataBank::EnsureDatabase(NULL, true); // no UI
							}

							if (bIsDatabaseValid) {
								FirstDataBank::ValidateNDCNumber(strClean, strDrugName, bContinueEditing);
							}

							if(bContinueEditing) {
								*pbCommit = FALSE;
								*pbContinue = FALSE;
							}
						}
					}
				}
				break;
		}
	} NxCatchAll("Error in OnEditingFinishingEditList");
}

void CEditMedicationListDlg::OnMarkMedicationInactive() 
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;
		if (pRow == NULL) {
			MessageBox("Please select a medication to inactivate.", "NexTech Practice", MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		// (c.haag 2007-03-05 09:15) - PLID 25056 - Medications are linked with EMR table items.
		// An EMR table item must have at least one active row. Given that, we cannot allow a user
		// to inactivate the last medication.
		if (!HasMultipleActiveMedications())
			return;

		if (IDYES == MessageBox("Are you sure you wish to inactivate this medication?", "NexTech Practice", MB_YESNO|MB_ICONQUESTION)) {
			long nMedID = VarLong(pRow->GetValue(elcID));

			// (c.haag 2007-01-31 09:13) - PLID 24422 - We now do all the data work for the
			// DrugList table and EMR in one function
			ExecuteInactivation(nMedID);			

			//No reason to requery here, just remove the one we inactivated
			m_pList->RemoveRow(pRow);
		}
	} NxCatchAll("Error inactivation Medication");
}

void CEditMedicationListDlg::OnShowInactiveMeds() 
{
	try {
		CInactiveMedicationsDlg dlg(this);
		dlg.DoModal();
		//requery in case we changed anything
		m_pList->Requery();
	} NxCatchAll("Error in OnShowInactiveMeds");
}

void CEditMedicationListDlg::EnsureControls()
{
	//Nothing selected, disable functionality buttons
	BOOL bEnable = TRUE;
	if(m_pList->CurSel == NULL) {
		bEnable = FALSE;
	}

	//Selection made, enable buttons
	GetDlgItem(IDC_EDIT)->EnableWindow(bEnable);
	GetDlgItem(IDC_DELETE)->EnableWindow(bEnable);
	GetDlgItem(IDC_MARK_MEDICATION_INACTIVE)->EnableWindow(bEnable);

	//TES 6/5/2013 - PLID 56631 - If they don't have the FDB license, don't show the FDB button and label
	// (b.savon 2013-07-31 17:26) - PLID 57799 - Instead of checking if they have the license, my other note in this item
	// for this pl explains why, let's only hide it if they have no Out of date FDB meds in the list.	
	m_btnUpdateAllMedications.EnableWindow(m_nDrugListHasOutOfDateFDBMed > 0);
}

void CEditMedicationListDlg::OnSelChangedEditList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		EnsureControls();
	} NxCatchAll("Error in OnSelChangedEditList");
}

void CEditMedicationListDlg::OnBtnEditLatinNotation() 
{
	try {
		CLatinPrescriptionMacrosDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll("Error in OnBtnEditLatinNotation");
}

void CEditMedicationListDlg::OnLButtonUpEditList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);

		switch(nCol) {
			case elcDrugName:
			case elcStrength:
			case elcUnits:
			case elcQuantityUnits: // (s.dhole 2012-11-16 10:53) - PLID 53698
			case elcDosageForm:
				{
					//Hyperlinks block the CurSel setting, so let's set it manually
					m_pList->CurSel = pRow;

					long nID = VarLong(pRow->GetValue(elcID));

					// (d.thompson 2008-11-25) - PLID 32175 - These now operate as hyperlinks which popup the advanced 
					//	medication name editing dialog.
					CAddComplexMedicationNameDlg dlg(this);
					//Preset the values currently selected
					dlg.m_strDrugName = VarString(pRow->GetValue(elcDrugName));
					dlg.m_strStrength = VarString(pRow->GetValue(elcStrength));
					dlg.m_nStrengthUnitID = VarLong(pRow->GetValue(elcUnits), -1);
					dlg.m_nDosageFormID = VarLong(pRow->GetValue(elcDosageForm), -1);
					dlg.m_strFullDrugDescription = VarString(pRow->GetValue(elcFullDescription));
					dlg.m_nQuntityUnitID = VarLong(pRow->GetValue(elcQuantityUnits), -1); // (s.dhole 2012-11-16 10:53) - PLID 53698
					// (j.jones 2009-06-12 15:21) - PLID 34613 - pass in the medication ID
					dlg.m_nMedicationID = nID;
					// (j.gruber 2010-11-02 12:25) - PLID 39048 - send whether there is an FDBID
					long nFDBID = VarLong(pRow->GetValue(elcFDBID), -1);
					dlg.m_nFDBID = nFDBID;

					// (j.fouts 2012-11-27 16:47) - PLID 51889 - Set notes and drug type
					dlg.m_strNotes = VarString(pRow->GetValue(elcNotes), "");
					dlg.m_drugType = MapDrugTypeToAccessor(VarLong(pRow->GetValue(elcDrugTypeID), -1));
					// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
					dlg.m_nDosageUnitID = VarLong(pRow->GetValue(elcDosageUnitID), -1);
					// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
					dlg.m_nRouteID = VarLong(pRow->GetValue(elcRouteID), -1);

					// (d.thompson 2008-12-10) - PLID 32175 - We must check if this medication is in use before
					//	allowing edits.  If it is in use, the drug description field will be unchangeable.  You
					//	may still change the subfields.					
					// (j.fouts 2012-11-30 11:59) - PLID 51889 - Changed the warning to include the drug type
					CString strCounts;
					CString strMessage;
					if(IsMedicationInUse(nID, strCounts)) {	
						if (nFDBID == -1) {
							strMessage.Format("This medication is in use on:\r\n%s\r\nYou will not be allowed to edit the drug description or the drug type, "
								"however you may edit the sub-data fields.", strCounts);						
						}
						else {
							// (s.dhole 2013-03-18 16:06) - PLID 55347  Change text
							// (s.dhole 2013-04-25 12:55) - PLID 55345 Change text
							strMessage.Format("This medication is in use on:\r\n%s\r\nIt is also not a Free Text Medication. You will only be able to edit the Qty Units and Strength Units.", strCounts);						
						}

						MsgBox(strMessage);
						dlg.m_bAllowDrugDescriptionToChange = false;
					}
					else {
						// (j.gruber 2010-11-02 12:26) - PLID 39048 - if there is an FDBID, then they won't be able to edit some fields, so let them know
						if (nFDBID != -1) {
							// (s.dhole 2013-03-18 16:06) - PLID 55347  Change text
							// (s.dhole 2013-04-25 12:55) - PLID 55345 Change text
							strMessage.Format("This medication is not a Free Text Medication, you will only be able to edit the Qty Units and Strength Units.");
							MsgBox(strMessage);
							dlg.m_bAllowDrugDescriptionToChange = false;
						}
					
					}
					


					//Save the name before the change
					CString strSavedFullName = VarString(pRow->GetValue(elcFullDescription));

					if(dlg.DoModal() == IDOK) {
						//Use variants for the IDs
						_variant_t varStrengthUnit(g_cvarNull);
						_variant_t varDosageForm(g_cvarNull);
						_variant_t varQuntityUnit(g_cvarNull);
						// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
						_variant_t varDosageUnitID(g_cvarNull);
						// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
						_variant_t varDosageRouteID(g_cvarNull);

						if(dlg.m_nStrengthUnitID != -1) {
							varStrengthUnit = (long)dlg.m_nStrengthUnitID;
						}
						if(dlg.m_nDosageFormID != -1) {
							varDosageForm = (long)dlg.m_nDosageFormID;
						}
						if(dlg.m_nQuntityUnitID != -1) {
							varQuntityUnit = (long)dlg.m_nQuntityUnitID;
						}

						// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
						if(dlg.m_nDosageUnitID != -1)
						{
							varDosageUnitID = (long)dlg.m_nDosageUnitID;
						}

						// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
						if(dlg.m_nRouteID != -1)
						{
							varDosageRouteID = (long)dlg.m_nRouteID;
						}					

						//Update this dialog with the changes made in the popup dialog
						pRow->PutValue(elcFullDescription, _bstr_t(dlg.m_strFullDrugDescription));
						pRow->PutValue(elcDrugName, _bstr_t(dlg.m_strDrugName));
						pRow->PutValue(elcStrength, _bstr_t(dlg.m_strStrength));
						pRow->PutValue(elcUnits, varStrengthUnit);
						pRow->PutValue(elcDosageForm, varDosageForm);
						pRow->PutValue(elcQuantityUnits, varQuntityUnit); // (s.dhole 2012-11-16 10:53) - PLID 53698
						// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
						pRow->PutValue(elcDosageUnitID, varDosageUnitID);
						pRow->PutValue(elcDosageUnit, _bstr_t(dlg.m_strDosageUnit));
						// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
						pRow->PutValue(elcRouteID, varDosageRouteID);
						pRow->PutValue(elcRoute, _bstr_t(dlg.m_strDosageRoute));
									
						// (j.fouts 2012-11-27 16:50) - PLID 51889 - Update datalist with changes to Notes and Drug Type
						pRow->PutValue(elcNotes, _bstr_t(dlg.m_strNotes));
						pRow->PutValue(elcDrugTypeID, MapDrugTypeToData(dlg.m_drugType));
						switch(dlg.m_drugType)
						{
						case NexTech_Accessor::DrugType_NDC:
							pRow->PutValue(elcDrugType, _bstr_t("Medication"));
							break;
						case NexTech_Accessor::DrugType_Supply:
							pRow->PutValue(elcDrugType, _bstr_t("Supply"));
							break;
						case NexTech_Accessor::DrugType_Compound:
							pRow->PutValue(elcDrugType, _bstr_t("Compound"));
							break;
						default:
							pRow->PutValue(elcDrugType, _bstr_t(""));
							break;
						}

						// (c.haag 2007-01-30 16:25) - PLID 24422 - We now only update the data if the
						// old name and new name are actually different. When the data is updated, we
						// do it from ExecuteNameChange because we have considerations to make with the
						// system Current Medications item. We do an exact comparison; no whitespace
						// trimming, because we never did trimming in the past
						if(strSavedFullName != dlg.m_strFullDrugDescription) {
							ExecuteNameChange(nID, dlg.m_strFullDrugDescription);
						}

						//The name change succeeded, so save the other 4 values as well.
						// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
						// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
						ExecuteParamSql("UPDATE DrugList SET DrugName = {STRING}, Strength = {STRING}, StrengthUnitID = {VT_I4}, "
							"DosageFormID = {VT_I4}, QuantityUnitID = {VT_I4}, Notes = {STRING}, DrugType = {INT}, DosageUnitID = {VT_I4}, DosageRouteID = {VT_I4} WHERE ID = {INT};",
							dlg.m_strDrugName, dlg.m_strStrength, varStrengthUnit, varDosageForm, 
							varQuntityUnit, dlg.m_strNotes, dlg.m_drugType, varDosageUnitID, varDosageRouteID, nID);
					}
				}
				break;

			case elcAllergies:
				{
					// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
					CMultiSelectDlg dlg(this, "DrugAllergyT");
					CDWordArray dwaAllergies;
					long nDrugID = VarLong(pRow->GetValue(elcID));
					// (d.thompson 2008-12-01) - PLID 32177 - Parameterized
					_RecordsetPtr rsAllergies = CreateParamRecordset("SELECT AllergyID FROM DrugAllergyT WHERE DrugID = {INT}", nDrugID);
					while(!rsAllergies->eof) {
						dwaAllergies.Add(AdoFldLong(rsAllergies, "AllergyID"));
						rsAllergies->MoveNext();
					}
					rsAllergies->Close();
					dlg.PreSelect(dwaAllergies);
					// (c.haag 2007-04-03 14:40) - PLID 25482 - We now get the allergy name from EmrDataT
					// since AllergyT.Name has been depreciated
					// (c.haag 2007-04-09 15:30) - PLID 25504 - Only show inactive allergies if they've already been assigned
					// to medications
					if(IDOK == dlg.Open("AllergyT INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID",
						FormatString("Inactive = 0 OR AllergyT.ID IN (SELECT AllergyID FROM DrugAllergyT WHERE DrugID = %d)", nDrugID),
						"AllergyT.ID", "Data", "Select any allergies which would interact with this drug."))
					{
						dlg.FillArrayWithIDs(dwaAllergies);
						//DRT 11/26/2008 - PLID 32177 - Optimized, do not run queries in a loop!
						CString strSqlBatch;
						CNxParamSqlArray args;
						AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM DrugAllergyT WHERE DrugID = {INT};\r\n", nDrugID);
						for(int i = 0; i < dwaAllergies.GetSize(); i++) {
							AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO DrugAllergyT (DrugID, AllergyID) VALUES ({INT}, {INT});\r\n", nDrugID, dwaAllergies[i]);
						}
						//execute sql batch
						// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
						ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
						CString strDescription = dlg.GetMultiSelectString();
						if(strDescription.IsEmpty()) strDescription = "<None>";
						pRow->PutValue(elcAllergies, _bstr_t(strDescription));
					}
				}
				break;
		}
	} NxCatchAll("Error in OnLButtonUpEditList");
}

void CEditMedicationListDlg::ApplyLinkStyleToCell(NXDATALIST2Lib::IRowSettingsPtr pRow, short nColumn)
{
	pRow->PutCellLinkStyle(nColumn, dlLinkStyleTrue);
}

void CEditMedicationListDlg::OnRequeryFinishedEditList(short nFlags)
{
	// (b.cardillo 2007-09-06 16:03) - PLID 27313 - Changed the implementation here to be 
	// significantly more efficient while doing the same thing.  It generates an xml string (as 
	// efficiently as possible) while (efficiently) picking up a reference to each row of the list, 
	// and then passes the xml into a parameterized query that will return the names of each known 
	// drug allergy.  It then proceeds through that list, appending each allergy name to the name 
	// cell for the appropriate drug row in the list.  Any left over are set to <None>.
	try {	

		// (d.thompson 2008-12-03) - PLID 32175 - We want the combo columns to also display as links, but
		//	there's no default setting in the datalist for it, so we need to apply it manually.
		IRowSettingsPtr pRow = m_pList->GetFirstRow();
		while(pRow != NULL) {
			ApplyLinkStyleToCell(pRow, elcDosageForm);
			ApplyLinkStyleToCell(pRow, elcUnits);
			ApplyLinkStyleToCell(pRow, elcQuantityUnits);// (s.dhole 2012-11-16 10:55) - PLID 53698

			// (b.savon 2013-01-07 12:44) - PLID 54459
			//This is an FDB imported med.
			long nFDBID = VarLong(pRow->GetValue(elcFDBID), -1);
			if (nFDBID != -1) {
				//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
				// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
				if (nFDBID > 0 && VarBool(pRow->GetValue(elcFDBOutOfDate), 0)) {
					pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					// (b.savon 2013-07-31 17:28) - PLID 57799 - Set the flag; we have an FDB med AND at least 1 is out of date
					++m_nDrugListHasOutOfDateFDBMed;
				}
				else {
					pRow->PutBackColor(ERX_IMPORTED_COLOR);
				}
			}
			pRow = pRow->GetNextRow();
		}
		// (b.savon 2013-07-31 17:28) - PLID 57799 - Ensure the button is enabled/disabled
		EnsureControls();

		// Loop through all rows, getting a row pointer for each and generating the 
		// xml to send to the query at the same time
		CMapPtrToPtr mapAllDrugRows;
		CString strXml;
		{
			long nMaxLen = 6 + 21 * m_pList->GetRowCount() + 7;
			LPTSTR pstr = strXml.GetBuffer(nMaxLen);
			LPCTSTR pstrOrig = pstr;
			pstr += sprintf(pstr, "<ROOT>");
			//DRT 11/25/2008 - PLID 32175 - Changed looping mechanism for datalist2
			IRowSettingsPtr pRowSearch = m_pList->GetFirstRow();
			while (pRowSearch != NULL) {
				long nID = VarLong(pRowSearch->GetValue(elcID));
				IRowSettingsPtr pNextRow = pRowSearch->GetNextRow();
				mapAllDrugRows.SetAt((void *)nID, pRowSearch.Detach());
				pstr += sprintf(pstr, "<P ID=\"%li\" />", nID);
				ASSERT(pstr - pstrOrig <= nMaxLen);

				pRowSearch = pNextRow;
			}
			pstr += sprintf(pstr, "</ROOT>");
			ASSERT(pstr - pstrOrig <= nMaxLen);
			strXml.ReleaseBuffer(pstr - pstrOrig);
		}
		// Run the query, getting the names of all known drug allergies
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"DECLARE @hDoc INT \r\n"
			"EXEC sp_xml_preparedocument @hDoc OUTPUT, {STRING} \r\n"
			"SELECT DrugAllergyT.DrugID, [Data] AS Name \r\n"
			"FROM AllergyT \r\n"
			"INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID \r\n"
			"INNER JOIN DrugAllergyT ON AllergyT.ID = DrugAllergyT.AllergyID \r\n"
			"WHERE DrugAllergyT.DrugID IN (SELECT A.ID FROM OPENXML(@hDoc, '/ROOT/P', 1) WITH (ID INT) A) \r\n"
			"EXEC sp_xml_removedocument @hDoc \r\n"
			"SET NOCOUNT OFF \r\n"
			,
			strXml);
		// Set the names of all the known allergies
		CMapPtrToPtr mapWrittenDrugRows;
		if (!prs->eof) {
			FieldsPtr pflds = prs->GetFields();
			FieldPtr fldDrugID = pflds->GetItem("DrugID");
			FieldPtr fldName = pflds->GetItem("Name");
			while (!prs->eof) {
				long nDrugID = AdoFldLong(fldDrugID);
				IRowSettings *lpRow;
				// See which map it's in
				if (mapAllDrugRows.Lookup((void *)nDrugID, (void *&)lpRow)) {
					// In the main map, so just set the initial allergy name
					mapWrittenDrugRows.SetAt((void *)nDrugID, lpRow);
					mapAllDrugRows.RemoveKey((void *)nDrugID);
					lpRow->PutValue(elcAllergies, fldName->GetValue());
				} else if (mapWrittenDrugRows.Lookup((void *)nDrugID, (void *&)lpRow)) {
					// Secondary map, which means we already set its first allergy, so append
					// (j.fouts 2013-05-08 15:34) - PLID 56204 - Use the enum
					CString strExistingAllergy = AsString(lpRow->GetValue(elcAllergies));
					strExistingAllergy += "," + AdoFldString(fldName);
					lpRow->PutValue(elcAllergies, AsBstr(strExistingAllergy));
				} else {
					// Wasn't in either map.  That should be impossible
					ASSERT(FALSE);
					ThrowNxException("Allergy not found!");
				}
				prs->MoveNext();
			}
		}
		// Set all the rest to <None> and clear the main map
		{
			_variant_t varNone(_bstr_t("<None>"));
			POSITION pos = mapAllDrugRows.GetStartPosition();
			while (pos) {
				IRowSettings *lpRow;
				long nDrugID;
				mapAllDrugRows.GetNextAssoc(pos, (void *&)nDrugID, (void *&)lpRow);
				lpRow->PutValue(elcAllergies, varNone);
				lpRow->Release();
			}
			mapAllDrugRows.RemoveAll();
		}
		// Clear the secondary map
		{
			POSITION pos = mapWrittenDrugRows.GetStartPosition();
			while (pos) {
				IRowSettings *lpRow;
				long nDrugID;
				mapWrittenDrugRows.GetNextAssoc(pos, (void *&)nDrugID, (void *&)lpRow);
				lpRow->Release();
			}
			mapWrittenDrugRows.RemoveAll();
		}
	} NxCatchAll("CEditMedicationListDlg::OnRequeryFinishedEditList");
}

void CEditMedicationListDlg::ExecuteNameChange(long nDrugID, const CString& strNewName)
{
	//
	// (c.haag 2007-01-30 16:26) - PLID 24422 - This function is called to change the name
	// of a medication. We must update the EMR data as well as the Druglist table
	//
	//DRT 11/26/2008 - PLID 32177 - Optimized bad query setup
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteNameChange")

		long nEmrInfoID = GetActiveCurrentMedicationsInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchCurrentMedicationsInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateParamRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = (SELECT EMRDataID FROM DrugList WHERE ID = {INT});", nDrugID);
		strOldValue.Format("Item: 'Current Medications' - Table Row: '%s'", AdoFldString(prs, "Data"));
		strNewValue.Format("Table Row: '%s'", strNewName);
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemName, nEmrInfoID, strOldValue, strNewValue, aepHigh, aetChanged);

		// Now update the DrugList table
		// (c.haag 2007-02-02 16:13) - PLID 24561 - The DrugList table no longer has a name field
		//ExecuteSql("UPDATE Druglist SET Name = '%s' WHERE ID = %d", _Q(strNewName), nDrugID);

		// Now update the system Current Medications info item
		ExecuteParamSql("UPDATE EMRDataT SET Data = {STRING} WHERE ID IN (SELECT EMRDataID FROM DrugList WHERE ID = {INT})", strNewName, nDrugID);

	END_TRANS("ExecuteNameChange")

	// (c.haag 2007-02-07 13:49) - PLID 24422 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in DrugList with a bad EmrDataID, or a record exists
	// in EmrDataT for the active Current Medications item that does not correspond to DrugList
	WarnEmrDataDiscrepanciesWithDrugList();
}

// (d.thompson 2008-12-01) - PLID 32175 - Now requires the drug description fields as well.
// (d.thompson 2009-03-17) - PLID 33477 - Return value is now the QuantityUnitID
// (s.dhole 2012-11-16 11:02) - PLID 53698
// (j.fouts 2012-11-27 16:47) - PLID 51889 - Added notes and Drug Type
// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
void  CEditMedicationListDlg::ExecuteAddition(long nDrugID, CString strFullDrugDescription, CString strDrugName, CString strStrength, 
			_variant_t varStrengthUnitID, _variant_t varDosageFormID,_variant_t  varQuntityUnit, CString strNotes, NexTech_Accessor::DrugType drugType,
			_variant_t varDosageUnitID, _variant_t varDosageRouteID)
{
	//for return value
	//long nQtyUnitID = -1;

	//
	// (c.haag 2007-01-30 16:32) - PLID 24422 - This function is called to add a medication to the DrugList
	// table. We must update the EMR data as well.
	//
	//DRT 11/26/2008 - PLID 32177 - Optimized bad query setup
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteAddition")

		long nEmrInfoID = GetActiveCurrentMedicationsInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchCurrentMedicationsInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		strOldValue.Format("Item: 'Current Medications'");
		strNewValue.Format("New Table Row: '%s'", strFullDrugDescription);
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemCreated, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetCreated);

		CString strSqlBatch;
		CNxParamSqlArray args;

		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON;\r\n");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDataGroupID int;\r\n");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewDataID int;\r\n");
		// (j.jones 2007-08-14 16:02) - PLID 27053 - added EMRDataGroupsT
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataGroupID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataGroupsT);\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SET @NewDataID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDataT);\r\n");

		AddParamStatementToSqlBatch(strSqlBatch, args, "INSERT INTO EMRDataGroupsT (ID) VALUES (@NewDataGroupID);\r\n");

		// Now add the new medication to the EMRDataT table
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			"INSERT INTO EMRDataT (ID, EMRInfoID, Data, SortOrder, ListType, EmrDataGroupID) "
			"SELECT @NewDataID, {INT}, {STRING}, "
			"(SELECT COALESCE(Max(SortOrder),0) + 1 FROM EMRDataT WHERE EMRInfoID = {INT}), "
			"2, @NewDataGroupID ",
			nEmrInfoID, strFullDrugDescription, nEmrInfoID);

		// Now add the medication to DrugList
		// (c.haag 2007-02-02 16:14) - PLID 24561 - The DrugList table no longer has a name field
		// (d.thompson 2008-12-01) - PLID 32175 - Cleaned up for new fields.
		// (d.thompson 2009-03-11) - PLID 33477 - Added QuantityUnitID, which is always 'Tablet' to start
		// (s.dhole 2012-11-16 11:02) - PLID 53698 User has to select this value
		//AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @QtyUnitID int;\r\n");
		//AddStatementToSqlBatch(strSqlBatch, "SET @QtyUnitID = (SELECT ID FROM DrugStrengthUnitsT WHERE Name = 'Tablet');\r\n");
		// (j.fouts 2012-11-27 16:48) - PLID 51889 - Added Notes and DrugType
		// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Added Dosage Unit
		AddParamStatementToSqlBatch(strSqlBatch, args, 
			"INSERT INTO DrugList (ID, DefaultRefills, DefaultQuantity, EMRDataID, DrugName, Strength, StrengthUnitID, DosageFormID, QuantityUnitID, Notes, DrugType, DosageUnitID, DosageRouteID ) "
			"VALUES ({INT}, '0', '0', @NewDataID, {STRING}, {STRING}, {VT_I4}, {VT_I4}, {VT_I4}, {STRING}, {INT}, {VT_I4}, {VT_I4})", nDrugID, strDrugName, strStrength, 
			varStrengthUnitID, varDosageFormID, varQuntityUnit, strNotes, drugType, varDosageUnitID, varDosageRouteID);

		/*AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT OFF;\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SELECT @QtyUnitID AS QtyUnitID");*/

		//Do the execution
		// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
		// (s.dhole 2012-11-16 11:02) - PLID 53698
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
		//_RecordsetPtr prs = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
		// (s.dhole 2012-11-16 11:02) - PLID 53698 User has to select this value	
		//We should get the quantity unit ID back
		/*if(!prs->eof) {
			nQtyUnitID = AdoFldLong(prs, "QtyUnitID", -1);
		}*/

	END_TRANS("ExecuteAddition")

	// (c.haag 2007-02-07 13:49) - PLID 24422 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in DrugList with a bad EmrDataID, or a record exists
	// in EmrDataT for the active Current Medications item that does not correspond to DrugList
	WarnEmrDataDiscrepanciesWithDrugList();
	// (s.dhole 2012-11-16 11:02) - PLID 53698
	//return nQtyUnitID;
}

void CEditMedicationListDlg::ExecuteDeletion(long nDrugID)
{
	//
	// (c.haag 2007-01-30 17:09) - PLID 24422 - This function is called to remove a medication from the DrugList
	// table. We must update the EMR data as well.
	//
	//DRT 11/26/2008 - PLID 32177 - Optimized bad query setup
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteDeletion")

		long nEmrInfoID = GetActiveCurrentMedicationsInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchCurrentMedicationsInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateParamRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = (SELECT EMRDataID FROM DrugList WHERE ID = {INT});\r\n", nDrugID);
		strOldValue.Format("Item: 'Current Medications'");
		strNewValue.Format("Deleted Table Row: '%s'", AdoFldString(prs, "Data"));
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemDeleted, nEmrInfoID, strOldValue, strNewValue, aepHigh, aetDeleted);

		//// Begin legacy delete code /////

		CSqlFragment sqlDelete;

		sqlDelete += CSqlFragment(
			"DECLARE @DrugID int;\r\n"
			"DECLARE @EMRDataID int;\r\n"
			"SET @DrugID = {INT};\r\n"
			"SET @EMRDataID = (SELECT EMRDataID FROM DrugList WHERE ID = {INT});\r\n"
			, nDrugID, nDrugID);

		//also delete prescriptions that are marked deleted and reference to it
		//TES 2/18/2009 - PLID 28522 - Also delete from PatientMedicationDiagCodesT
		sqlDelete += CSqlFragment(
			"DELETE FROM PatientMedicationDiagCodesT WHERE PatientMedicationID IN "
			"	(SELECT ID FROM PatientMedications WHERE MedicationID = @DrugID AND Deleted = 1);\r\n"
			"DELETE FROM PatientMedications WHERE MedicationID = @DrugID AND Deleted = 1;\r\n"
			);

		// (z.manning 2013-03-11 13:21) - PLID 55554 - Moved deleting EMR data record SQL to its own method
		GetDeleteEmrDataSql(CSqlFragment("@EMRDataID"), sqlDelete);

		// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
		ExecuteParamSql(sqlDelete);

	END_TRANS("ExecuteDeletion")

	// (c.haag 2007-02-07 13:49) - PLID 24422 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in DrugList with a bad EmrDataID, or a record exists
	// in EmrDataT for the active Current Medications item that does not correspond to DrugList
	WarnEmrDataDiscrepanciesWithDrugList();
}

void CEditMedicationListDlg::ExecuteInactivation(long nDrugID)
{
	//
	// (c.haag 2007-01-30 17:09) - PLID 24422 - This function is called to inactivate a medication
	// in the DrugList table. We must update the EMR data as well.
	//
	//DRT 11/26/2008 - PLID 32177 - Optimized bad query setup
	CWaitCursor wc;
	BEGIN_TRANS("ExecuteInactivation")

		long nEmrInfoID = GetActiveCurrentMedicationsInfoID();
		if (IsEMRInfoItemInUse(nEmrInfoID)) {
			nEmrInfoID = BranchCurrentMedicationsInfoItem();
		} else {
			TouchEMRInfoItem(nEmrInfoID); // (a.walling 2013-03-27 09:47) - PLID 55898 - Update EmrInfoT.ModifiedDate (and vicariously, the .Revision rowversion)
		}

		// Now do EMR auditing.
		CString strOldValue, strNewValue;
		_RecordsetPtr prs = CreateParamRecordset("SELECT EmrDataT.Data FROM EMRDataT WHERE EMRDataT.ID = (SELECT EMRDataID FROM DrugList WHERE ID = {INT})", nDrugID);
		strOldValue.Format("Item: 'Current Medications' - Table Row: %s (Active)", AdoFldString(prs, "Data"));
		strNewValue.Format("Table Row: '%s' (Inactive)", AdoFldString(prs, "Data"));
		prs->Close();
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEMRItemTableItemInactive, nEmrInfoID, strOldValue, strNewValue, aepMedium, aetChanged);

		//// Begin legacy delete code /////
		// (c.haag 2007-02-02 19:05) - PLID 24565 - DrugList no longer has an inactive flag
		//ExecuteSql("UPDATE DrugList SET Inactive = 1 WHERE ID = %li", nDrugID);

		//// End legacy delete code /////

		ExecuteParamSql("UPDATE EMRDataT SET Inactive = 1 WHERE ID IN (SELECT EMRDataID FROM DrugList WHERE ID = {INT})", nDrugID);

	END_TRANS("ExecuteInactivation")

	// (c.haag 2007-02-07 13:49) - PLID 24422 - This function will pop up an exception message if,
	// for some unexpected reason, a record exists in DrugList with a bad EmrDataID, or a record exists
	// in EmrDataT for the active Current Medications item that does not correspond to DrugList
	WarnEmrDataDiscrepanciesWithDrugList();
}

BOOL CEditMedicationListDlg::HasMultipleActiveMedications()
{
	// (c.haag 2007-03-05 09:15) - PLID 25056 - This function determines if there is exactly one
	// active medication in Practice, and returns TRUE if there is not. If necessary, the medication
	// list will also be refreshed.
	//DRT 11/26/2008 - PLID 32177 - Parameterized
	_RecordsetPtr prs = CreateRecordset("SELECT Count(*) AS Cnt FROM DrugList INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID WHERE Inactive = 0");
	long nActiveMedications = 0;
	if(!prs->eof) {
		nActiveMedications = AdoFldLong(prs, "Cnt");
	}

	if (1 == nActiveMedications) {
		MessageBox("You must have at least one active medication in your database.", "NexTech Practice", MB_OK|MB_ICONINFORMATION);
		// It may be possible to get the previous warning if someone else was deleting medications from another
		// computer. If the active medication count and list count are inconsistent, refresh the list.
		const int nListRows = m_pList->GetRowCount();
		if (nActiveMedications != nListRows) {
			MessageBox("The medication list has been modified by other users. The list will now be refreshed.", "NexTech Practice", MB_OK|MB_ICONINFORMATION);
			m_pList->Requery();
		}
		return FALSE;
	} else {
		return TRUE;
	}
}

void CEditMedicationListDlg::OnBnClickedMedShowAdvDrugNames()
{
	try {
		//For speed of drawing
		m_pList->SetRedraw(VARIANT_FALSE);

		BOOL bChecked = IsDlgButtonChecked(IDC_MED_SHOW_ADV_DRUG_NAMES);
		if(bChecked) {
			//The user wants to see the extra 4 fields.
			IColumnSettingsPtr pCol = m_pList->GetColumn(elcDrugName);
			pCol->PutStoredWidth(40);
			pCol->PutColumnStyle(csVisible|csWidthAuto);
			pCol = m_pList->GetColumn(elcStrength);
			pCol->PutStoredWidth(40);
			pCol->PutColumnStyle(csVisible|csWidthAuto);
			pCol = m_pList->GetColumn(elcUnits);
			pCol->PutStoredWidth(40);
			pCol->PutColumnStyle(csVisible|csWidthAuto);
			pCol = m_pList->GetColumn(elcDosageForm);
			pCol->PutStoredWidth(40);
			pCol->PutColumnStyle(csVisible|csWidthAuto);
		}
		else {
			//Hide the bonus fields
			IColumnSettingsPtr pCol = m_pList->GetColumn(elcDrugName);
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol = m_pList->GetColumn(elcStrength);
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol = m_pList->GetColumn(elcUnits);
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol = m_pList->GetColumn(elcDosageForm);
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
		}

		//For speed of drawing
		m_pList->SetRedraw(VARIANT_TRUE);

		//Save the setting
		SetRemotePropertyInt("MedicationShowAdvDrugNames", bChecked ? 1 : 0, 0, GetCurrentUserName());

	} NxCatchAll("Error in OnBnClickedMedShowAdvDrugNames");
}

void CEditMedicationListDlg::OnBnClickedShowDrugNotes()
{
	try {
		//For speed of drawing
		m_pList->SetRedraw(VARIANT_FALSE);

		BOOL bChecked = IsDlgButtonChecked(IDC_SHOW_DRUG_NOTES);
		if(bChecked) {
			//The user wants to see the extra notes
			IColumnSettingsPtr pCol = m_pList->GetColumn(elcNotes);
			pCol->PutStoredWidth(40);
			pCol->PutColumnStyle(csVisible|csWidthAuto|csEditable);
		}
		else {
			//Hide the notes
			IColumnSettingsPtr pCol = m_pList->GetColumn(elcNotes);
			pCol->PutStoredWidth(0);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
		}

		//For speed of drawing
		m_pList->SetRedraw(VARIANT_TRUE);

		//Save the setting
		SetRemotePropertyInt("MedicationShowDrugNotes", bChecked ? 1 : 0, 0, GetCurrentUserName());

	} NxCatchAll("Error in OnBnClickedShowDrugNotes");
}

void CEditMedicationListDlg::OnColumnClickingMedicationList(short nCol, BOOL* bAllowSort)
{
	try {
		// (d.thompson 2008-12-05) - PLID 32175 - We do not allow sorting on the patient instructions because
		//	it is an ntext field, and SQL cannot handle that.
		// (d.thompson 2009-01-29) - PLID 32175 - Notes for the same reason.
		if(nCol == elcPatientInstructions || nCol == elcNotes) {
			*bAllowSort = FALSE;
		}

	} NxCatchAll("Error in OnColumnClickingMedicationList");
}

LRESULT CEditMedicationListDlg::OnDelayedDontShow(WPARAM wParam, LPARAM lParam)
{
	try {
		// (d.thompson 2008-11-25) - PLID 32175 - I added a warning to explain the new data fields added along
		//	with SureScripts development.
		DontShowMeAgain(this, "Medication descriptions can now be broken down into more detailed descriptions.  You may choose to fill in "
			"all drug descriptions using the Drug Name, Strength, Units, and Dosage Form fields.  These will automatically combine "
			"to form the Drug Description field.\r\n"
			"This information is optional, you may just fill all text in the Drug Name field otherwise.\r\n"
			"It is recommended that these fields are used if you intend to do electronic prescribing.", 
			"SureScriptsMedicationFieldsWarning");
	} NxCatchAll("Error in OnDelayedDontShow");

	return 0;
}

// (d.thompson 2009-03-18) - PLID 33479 - Added help documentation
void CEditMedicationListDlg::OnBnClickedAdvMedHelp()
{
	try {
		OpenManual("NexTech_Practice_Manual.chm", "Patient_Information/Medications/setting_up_medications.htm");

	} NxCatchAll("Error in OnBnClickedMedicationAdvHelp");
}

// (j.gruber 2010-11-02 12:08) - PLID 39048
void CEditMedicationListDlg::EditingStartingEditList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			long nFDBID = VarLong(pRow->GetValue(elcFDBID), -1);
			DrugType dgtType = (DrugType)VarLong(pRow->GetValue(elcDrugTypeID));

			if (nFDBID != -1) {
				//if there is a FDBID, they cannot edit name, strength, dosage form, NDC and DEA Schedule
				CString strField;
				switch (nCol) {	
					case elcFullDescription:						
						strField = "Description";
					break;
					case elcDrugName:
						strField = "Name";
					break;
					case elcStrength:					
						strField = "Strength";
					break;
					case elcDosageForm:
						strField = "Dosage Form";
					break;
					case elcDEASchedule:
						strField = "DEA Schedule";
					break;
					case elcNDCNumber:
						strField = "NDC Number";
					break;
				}

				if (!strField.IsEmpty()) {
					MsgBox("This medication is not a Free Text Medication. You may not edit the %s field for medications that are not free text.", strField);
					*pbContinue = FALSE;
				}
				
			}
			if(dgtType != dgtNDC)
			{
				if(nCol == elcNDCNumber)
				{
					//This is not an NDC medication and they are trying to edit the NDC
					MessageBox("You cannot set an NDC for Compound Medications and Supplies.", "NexTech Practice", MB_OK|MB_ICONINFORMATION);
					*pbContinue = FALSE;
				}
			}			
		}				
		
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-21 12:42) - PLID 54722
void CEditMedicationListDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);
		SetControlPositions();

	}NxCatchAll(__FUNCTION__);
}

void CEditMedicationListDlg::OnUpdateAllMedications()
{
	try {
		//TES 6/5/2013 - PLID 56631 - Make sure they have the FDB license
		// (b.savon 2013-07-31 17:23) - PLID 57799 - Not all clients have the FDB license, but still have FDB linked meds because
		// of how we sync them back from NewCrop.  Since back from the beginning, we just made all the qty units Tablet; this is 
		// coming back to haunt us, because, clearly, it is incorrect.  Gels, creams, etc are not tablets.  This handy update FDB
		// meds came along, which fixes these problems, but we still need to be able to do it if they have NewCrop
		//if(!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
		//	return;
		//}

		//TES 5/13/2013 - PLID 56631 - Call the API to update all linked medications from FDB information
		CArray<NexTech_Accessor::_FDBUpdateMedicationInputPtr, NexTech_Accessor::_FDBUpdateMedicationInputPtr> aryMeds;

		//TES 5/13/2013 - PLID 56631 - Create our SAFEARRAY to be passed to the UpdateMedications function in the API
		Nx::SafeArray<IUnknown *> saryMeds = Nx::SafeArray<IUnknown *>::From(aryMeds);
		
		CWaitCursor cwait;

		//TES 5/13/2013 - PLID 56631 - Call the API to update the meds and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		// Note that we pass in an empty array, this will tell the function to update all linked records
		NexTech_Accessor::_FDBUpdateMedicationResultsArrayPtr updateResults = GetAPI()->UpdateMedications(GetAPISubkey(), GetAPILoginToken(), saryMeds);

		//TES 5/13/2013 - PLID 56631 - Refresh the list to re-color the linked medications
		m_pList->Requery();

		Nx::SafeArray<IUnknown *> saryMedResults(updateResults->FDBUpdateMedicationResults);

		int nSuccessCount = 0, nFailedCount = 0;
		foreach(NexTech_Accessor::_FDBUpdateMedicationOutputPtr pUpdateResult, saryMedResults) {
			if(pUpdateResult->Success) {
				nSuccessCount++;
				// (b.savon 2013-08-01 12:18) - PLID 57822 - Clear the flag if we don't have any failures.
				--m_nDrugListHasOutOfDateFDBMed;
			}
			else {
				nFailedCount++;
			}
		}

		//TES 5/13/2013 - PLID 56631 - Report success
		if(nFailedCount == 0) {
			if(nSuccessCount == 0) {
				MsgBox("All medications were already up to date");
			}
			else {
				MsgBox("Successfully updated %i medications", nSuccessCount);
			}
		}
		else {
			//TES 5/13/2013 - PLID 56631 - There is currently no case where any codes should fail to update, so raise an exception
			ThrowNxException("Failed to update %i medications from FirstDataBank", nFailedCount);
		}


	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-20 14:20) - PLID 56880 - Moved this from the DOWN event to the UP event
void CEditMedicationListDlg::RButtonUpEditList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_pList->CurSel = pRow;
		
		// (b.savon 2013-06-20 14:19) - PLID 56880 - Use CMenu if were not in the EMR Frame
		CMenu pMenu;
		pMenu.CreatePopupMenu();
		int nMenuIndex = 0;

		// (b.savon 2013-06-20 14:19) - PLID 56880 - made these variables
		BOOL bFDBOutOfDate = FALSE;
		//TES 5/13/2013 - PLID 56631 - If the row we're on is linked to FDB, and is out of date, give a menu option to update the record from FDB
		long nFDBMedID = VarLong(pRow->GetValue(elcFDBID), -1);
		if (nFDBMedID != -1) {
			// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
			if (nFDBMedID > 0 && VarBool(pRow->GetValue(elcFDBOutOfDate), FALSE)) {
				//TES 6/5/2013 - PLID 56631 - Make sure they have the FDB license
				// (b.savon 2013-08-01 11:29) - PLID 57799 - Show even if they don't have the license
				bFDBOutOfDate = TRUE;
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_UPDATE_FDB_MED, "Update Medication Information From FirstDataBank");
			}
		}

		// (b.savon 2013-06-19 16:53) - PLID 56880 - Only add the menu options if they have NexERx
		BOOL bNexERxLicense = g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts;
		if(bNexERxLicense) {

			if( bFDBOutOfDate ){
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, MF_SEPARATOR);
			}

			if(nFDBMedID >= 0)
			{
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_SHOW_MONO, "View Monograph");
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_SHOW_LEAFLET, "View Leaflet");
			}
			else
			{
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_MONO, "No Monograph Available");
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_LEAFLET, "No Leaflet Available");
			}
		}

		// (b.savon 2013-06-20 14:19) - PLID 56880 - Pop it up if they have one of them
		if( bFDBOutOfDate || bNexERxLicense ){
			CPoint pt;
			GetCursorPos(&pt);
			int nCmd = pMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD,pt.x, pt.y,this);
			switch( nCmd ){
				case IDM_UPDATE_FDB_MED: //FDB Update
					{
						//TES 5/13/2013 - PLID 56631 - Call the API to update this medication
						CArray<NexTech_Accessor::_FDBUpdateMedicationInputPtr, NexTech_Accessor::_FDBUpdateMedicationInputPtr> aryMedications;
						NexTech_Accessor::_FDBUpdateMedicationInputPtr med(__uuidof(NexTech_Accessor::FDBUpdateMedicationInput));
						med->MedicationID = VarLong(pRow->GetValue(elcID));
						aryMedications.Add(med);

						//TES 5/13/2013 - PLID 56631 - Create our SAFEARRAY to be passed to the UpdateMedications function in the API
						Nx::SafeArray<IUnknown *> saryMedications = Nx::SafeArray<IUnknown *>::From(aryMedications);
			
						CWaitCursor cwait;

						//TES 5/13/2013 - PLID 56631 - Call the API to update the meds and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
						NexTech_Accessor::_FDBUpdateMedicationResultsArrayPtr updateResults = GetAPI()->UpdateMedications(GetAPISubkey(), GetAPILoginToken(), saryMedications);

						Nx::SafeArray<IUnknown *> saryMedicationResults(updateResults->FDBUpdateMedicationResults);

						int nSuccessCount = 0, nFailedCount = 0;
						foreach(NexTech_Accessor::_FDBUpdateMedicationOutputPtr pUpdateResult, saryMedicationResults) {
						if(pUpdateResult->Success) {
								nSuccessCount++;
							}
							else {
								nFailedCount++;
							}
						}

						//TES 5/13/2013 - PLID 56631 - If we succeeded, just update the row color
						if(nFailedCount == 0) {
							pRow->PutBackColor(ERX_IMPORTED_COLOR);
							// (b.savon 2013-08-01 12:02) - PLID 57822 - Clear the out of date flag
							pRow->PutValue(elcFDBOutOfDate, g_cvarFalse);
							--m_nDrugListHasOutOfDateFDBMed;
						}
						else {
							//TES 5/10/2013 - PLID 56631 - There is currently no case where any medications should fail to update, so raise an exception
							ThrowNxException("Failed to update single medication from FirstDataBank");
						}

						// (b.savon 2013-08-01 11:49) - PLID 57799 - Update the controls
						EnsureControls();
					}
					break;
				// (b.savon 2013-06-19 17:07) - PLID 56880 - Show monograph
				case IDM_SHOW_MONO: // Monograph
					try
					{
						// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
						if(pRow)
						{
							//Get the MedId for the selected drug
							long nFDBMedID = VarLong(pRow->GetValue(elcFDBID), -1);

							// (j.fouts 2012-08-20 09:26) - PLID 51719 - Don't give an option for monograph to non FDB drugs
							if(nFDBMedID >= 0)
							{
								// (j.fouts 2012-09-25 09:27) - PLID 52825 - Check that the database exists
								if(FirstDataBank::EnsureDatabase(this, true))
								{
									ShowMonograph(nFDBMedID, this);
								}
							}
						}
					}
					NxCatchAll(__FUNCTION__);
					break;
				// (b.savon 2013-06-19 17:29) - PLID 56880 - Show Leaflet
				case IDM_SHOW_LEAFLET: // Leaflet 
					try
					{
						// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
						if(pRow)
						{
							long nFDBMedID = VarLong(pRow->GetValue(elcFDBID), -1);

							if(nFDBMedID >= 0)
							{
								// (j.fouts 2013-06-10 11:18) - PLID 56808 - If the database does not exist we cannot query it
								if(FirstDataBank::EnsureDatabase(this, true))
								{
									ShowLeaflet(nFDBMedID, this);
								}
							}
						}
					}
					NxCatchAll(__FUNCTION__);
					break;
			}
		}
		
	}NxCatchAll(__FUNCTION__);
}
