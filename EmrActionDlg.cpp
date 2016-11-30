// EmrActionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "EmrActionDlg.h"
#include "EmrItemEntryDlg.h"
#include "CPTAddNew.h"
#include "InvNew.h"
#include "AuditTrail.h"
#include "EmrProblemActionsDlg.h"
#include "practicerc.h"
#include "MultiSelectDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "DiagSearchUtils.h" // (s.dhole 07\08\2014 8:56) - PLID 62592 
#include "PrescriptionUtilsAPI.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>
#include "NexGEM.h"
#include "NexCodeDlg.h"
#include "SelectDlg.h" 
#include "EmrActionFilterAnatomyDlg.h"

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;


// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CEmrActionDlg dialog

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_REMOVE_CHARGE	50000
#define ID_REMOVE_DIAG	50001
#define ID_REMOVE_ITEM	50002
#define ID_REMOVE_MINT	50003
#define ID_REMOVE_MEDICATION	50004
#define ID_REMOVE_PROCEDURE		50005
#define ID_REMOVE_MINT_ITEMS	50006
#define ID_EDIT_ITEM	50007
#define ID_NEW_ITEM		50008
#define ID_NEW_TODO		50009
#define ID_EDIT_TODO	50010
#define ID_REMOVE_TODO	50011
// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
// consistency with other areas in the program
#define ID_CONFIGURE_CPT_PROBLEMS		50012
#define ID_CONFIGURE_DIAG_PROBLEMS		50013
#define ID_CONFIGURE_MED_PROBLEMS		50014
#define ID_CONFIGURE_ITEM_PROBLEMS		50015
#define ID_CONFIGURE_MINT_PROBLEMS		50016
#define ID_CONFIGURE_MINTITEM_PROBLEMS	50017
// (z.manning 2008-10-21 13:38) - PLID 31556 - Added option to remove action from lab list.
#define ID_REMOVE_LAB					50018
#define ID_EDIT_ACTION_ANATOMIC_LOCATIONS	50019 // (z.manning 2011-06-28 17:56) - PLID 44347
// (j.jones 2012-08-24 14:10) - PLID 52091 - added ability to copy the anatomic location filter from another action
#define ID_COPY_THIS_ACTION_ANATOMIC_LOCATIONS	50020
#define ID_COPY_ANY_ACTION_ANATOMIC_LOCATIONS	50021

//(s.dhole 8/1/2014 10:12 AM ) - PLID 62597
#define ID_DIAG_SEARCH_FOR_ICD10	50022

extern CPracticeApp theApp;


//(s.dhole 7/14/2014 10:40 AM ) - PLID 62593
//(s.dhole 7/17/2014 4:51 PM ) - PLID 62723
enum ActionDiagColumn {	
	adcActionID = 0,
	adcICD9DiagCodeID = 1,
	adcICD10DiagCodeID = 2,
	adcICD9Code = 3,
	adcICD9Desc = 4,
	adcICD10Code = 5,
	adcICD10Desc = 6,
	adcOrder = 7,
	adcPopup = 8,
	adcProblemBitmap = 9,
	adcProblemArray = 10,
	adcLabAnatomyIDs = 11,
	adcMatchtype = 12,
	
	
};
//(s.dhole 7/17/2014 4:57 PM ) - PLID 62597
struct CEmrActionDlg::DiagnosisCodeDrpoDown{
	CArray<NexTech_Accessor::_DiagnosisCodeCommitPtr, NexTech_Accessor::_DiagnosisCodeCommitPtr> aryMultiMatchDD;
};

CEmrActionDlg::CEmrActionDlg(CWnd* pParent)
	: CNxDialog(CEmrActionDlg::IDD, pParent),
	m_pDiagCodeDDMultiMatch(new DiagnosisCodeDrpoDown())
{
	//{{AFX_DATA_INIT(CEmrActionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strSourceObjectName = "unknown object";
	m_nOriginatingID = -1;
	m_pCurrentEMN = NULL;
	m_pdlLastRightClickedDatalist = NULL;
	m_LastRightClickedAction = eaoInvalid;
	m_pDiagCodeDDMultiMatch->aryMultiMatchDD.RemoveAll();
	//(s.dhole 7/29/2014 8:22 AM ) - PLID 63083 initilaize Lastselected Datalist2
	m_pdlLastRightClickedDatalistDiag = NULL;
}

CEmrActionDlg::~CEmrActionDlg()
{}

void CEmrActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrActionDlg)
	DDX_Control(pDX, IDC_BTN_TODO_UP, m_btnTodoUp);
	DDX_Control(pDX, IDC_BTN_TODO_DOWN, m_btnTodoDown);
	DDX_Control(pDX, IDC_RADIO_SERVICE_CODE, m_btnSvcCode);
	DDX_Control(pDX, IDC_RADIO_PRODUCT, m_btnProduct);
	DDX_Control(pDX, IDC_BTN_EMR_MINT_ITEM_DOWN, m_btnMintItemDown);
	DDX_Control(pDX, IDC_BTN_EMR_MINT_ITEM_UP, m_btnMintItemUp);
	DDX_Control(pDX, IDC_BTN_EMR_ITEM_DOWN, m_btnItemDown);
	DDX_Control(pDX, IDC_BTN_EMR_ITEM_UP, m_btnItemUp);
	DDX_Control(pDX, IDC_BTN_CHARGE_DOWN, m_btnChargeDown);
	DDX_Control(pDX, IDC_BTN_CHARGE_UP, m_btnChargeUp);
	DDX_Control(pDX, IDC_BTN_DIAG_DOWN, m_btnDiagDown);
	DDX_Control(pDX, IDC_BTN_DIAG_UP, m_btnDiagUp);
	DDX_Control(pDX, IDC_BTN_LAB_DOWN, m_btnLabDown);
	DDX_Control(pDX, IDC_BTN_LAB_UP, m_btnLabUp);
	DDX_Control(pDX, IDC_BTN_MEDICATION_DOWN, m_btnMedicationDown);
	DDX_Control(pDX, IDC_BTN_MEDICATION_UP, m_btnMedicationUp);
	DDX_Control(pDX, IDC_BTN_EMN_DOWN, m_btnEmnDown);
	DDX_Control(pDX, IDC_BTN_EMN_UP, m_btnEmnUp);
	DDX_Control(pDX, IDC_BTN_PROCEDURE_DOWN, m_btnProcedureDown);
	DDX_Control(pDX, IDC_BTN_PROCEDURE_UP, m_btnProcedureUp);
	DDX_Control(pDX, IDC_BTN_NEW_TODO_TASK, m_btnNewTodoTask);
	DDX_Control(pDX, IDC_ACTION_CAPTION, m_nxstaticActionCaption);
	DDX_Control(pDX, IDC_MEDICATION_LABEL, m_nxstaticMedicationLabel);
	DDX_Control(pDX, IDC_EMR_ITEM_LABEL, m_nxstaticEmrItemLabel);
	DDX_Control(pDX, IDC_EMN_TEMPLATE_LABEL, m_nxstaticEmnTemplateLabel);
	DDX_Control(pDX, IDC_EMR_ACTION_ANCHOR_SMALL, m_nxstaticEmrActionAnchorSmall);
	DDX_Control(pDX, IDC_EMR_ACTION_ANCHOR_LARGE, m_nxstaticEmrActionAnchorLarge);
	DDX_Control(pDX, IDC_PROCEDURE_LABEL, m_nxstaticProcedureLabel);
	DDX_Control(pDX, IDC_EMN_TEMPLATE_ITEM_LABEL, m_nxstaticEmnTemplateItemLabel);
	DDX_Control(pDX, IDC_TODO_LABEL, m_nxstaticEmrTodoLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_ACTION_INCLUDE_FREE_TEXT_MEDS, m_checkIncludeFreeTextMeds);
	DDX_Control(pDX, IDC_ABOUT_EMRACTION_MEDICATION_COLORS, m_icoAboutEMRActiontMedsColors); 
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrActionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrActionDlg)
	ON_COMMAND(ID_REMOVE_CHARGE, OnRemoveCpt)
	ON_COMMAND(ID_REMOVE_DIAG, OnRemoveDiag)
	ON_COMMAND(ID_REMOVE_LAB, OnRemoveLab)
	ON_COMMAND(ID_REMOVE_ITEM, OnRemoveItem)
	ON_COMMAND(ID_REMOVE_MINT, OnRemoveMint)
	ON_COMMAND(ID_REMOVE_MINT_ITEMS, OnRemoveMintItems)
	ON_BN_CLICKED(IDC_RADIO_SERVICE_CODE, OnRadioServiceCode)
	ON_BN_CLICKED(IDC_RADIO_PRODUCT, OnRadioProduct)
	ON_COMMAND(ID_REMOVE_MEDICATION, OnRemoveMedication)
	ON_COMMAND(ID_REMOVE_PROCEDURE, OnRemoveProcedure)
	ON_COMMAND(ID_EDIT_ITEM, OnEditItem)
	ON_COMMAND(ID_NEW_ITEM, OnNewItem)
	ON_BN_CLICKED(IDC_BTN_EMR_MINT_ITEM_UP, OnBtnEmrMintItemUp)
	ON_BN_CLICKED(IDC_BTN_EMR_MINT_ITEM_DOWN, OnBtnEmrMintItemDown)
	ON_COMMAND(IDC_CHARGE_EDIT_BTN, OnChargeEditBtn)
	ON_BN_CLICKED(IDC_BTN_EMR_ITEM_UP, OnBtnEmrItemUp)
	ON_BN_CLICKED(IDC_BTN_EMR_ITEM_DOWN, OnBtnEmrItemDown)
	ON_BN_CLICKED(IDC_BTN_CHARGE_UP, OnBtnChargeUp)
	ON_BN_CLICKED(IDC_BTN_CHARGE_DOWN, OnBtnChargeDown)
	ON_BN_CLICKED(IDC_BTN_DIAG_UP, OnBtnDiagUp)
	ON_BN_CLICKED(IDC_BTN_DIAG_DOWN, OnBtnDiagDown)
	ON_BN_CLICKED(IDC_BTN_LAB_UP, OnBtnLabUp)
	ON_BN_CLICKED(IDC_BTN_LAB_DOWN, OnBtnLabDown)
	ON_BN_CLICKED(IDC_BTN_MEDICATION_UP, OnBtnMedicationUp)
	ON_BN_CLICKED(IDC_BTN_MEDICATION_DOWN, OnBtnMedicationDown)
	ON_BN_CLICKED(IDC_BTN_EMN_UP, OnBtnEmnUp)
	ON_BN_CLICKED(IDC_BTN_EMN_DOWN, OnBtnEmnDown)
	ON_BN_CLICKED(IDC_BTN_PROCEDURE_UP, OnBtnProcedureUp)
	ON_BN_CLICKED(IDC_BTN_PROCEDURE_DOWN, OnBtnProcedureDown)
	ON_BN_CLICKED(IDC_BTN_NEW_TODO_TASK, OnBtnNewTodoTask)
	ON_COMMAND(ID_NEW_TODO, OnNewTodo)
	ON_COMMAND(ID_EDIT_TODO, OnEditTodo)
	ON_COMMAND(ID_REMOVE_TODO, OnRemoveTodo)
	ON_BN_CLICKED(IDC_BTN_TODO_UP, OnBtnTodoUp)
	ON_BN_CLICKED(IDC_BTN_TODO_DOWN, OnBtnTodoDown)
	ON_COMMAND(ID_CONFIGURE_CPT_PROBLEMS, OnConfigureCptProblems)
	ON_COMMAND(ID_CONFIGURE_DIAG_PROBLEMS, OnConfigureDiagProblems)
	ON_COMMAND(ID_CONFIGURE_MED_PROBLEMS, OnConfigureMedProblems)
	ON_COMMAND(ID_CONFIGURE_ITEM_PROBLEMS, OnConfigureItemProblems)
	ON_COMMAND(ID_CONFIGURE_MINT_PROBLEMS, OnConfigureMintProblems)
	ON_COMMAND(ID_CONFIGURE_MINTITEM_PROBLEMS, OnConfigureMintItemProblems)
	ON_COMMAND(ID_EDIT_ACTION_ANATOMIC_LOCATIONS, OnEditActionAnatomicLocations)
	ON_COMMAND(ID_COPY_THIS_ACTION_ANATOMIC_LOCATIONS, OnCopyThisActionAnatomicLocations)
	ON_COMMAND(ID_COPY_ANY_ACTION_ANATOMIC_LOCATIONS, OnCopyAnyActionAnatomicLocations)
	ON_COMMAND(ID_DIAG_SEARCH_FOR_ICD10, OnSearchMMatchingICD10)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_ACTION_INCLUDE_FREE_TEXT_MEDS, OnCheckIncludeFreeTextMeds)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrActionDlg message handlers

using namespace ADODB;

BOOL CEmrActionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2008-04-29 16:50) - PLID 29837 - NxIconify the OK and Cancel buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (c.haag 2008-05-30 11:22) - PLID 30221 - Button for adding todo tasks
		m_btnNewTodoTask.AutoSet(NXB_NEW);

		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		BOOL bIsLevel2 = (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
		if(bIsLevel2) {
			SetWindowText("EMR Automatic Generation");
			SetDlgItemText(IDC_EMR_ITEM_LABEL, "EMR Items");
		}
		else {
			SetWindowText("Automatic Record Generation");
			SetDlgItemText(IDC_EMR_ITEM_LABEL, "Items");
		}

		// (j.jones 2016-01-25 15:29) - PLID 67998 - cached IncludeFreeTextFDBSearchResults
		g_propManager.CachePropertiesInBulk("EmrActionDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'IncludeFreeTextFDBSearchResults'" \
			")",
			_Q(GetCurrentUserName()));

		m_btnMintItemUp.AutoSet(NXB_UP);
		m_btnMintItemDown.AutoSet(NXB_DOWN);
		// (z.manning, 05/31/2007) - PLID 24096 - All actions now have up & down arrow buttons.
		m_btnItemUp.AutoSet(NXB_UP);
		m_btnItemDown.AutoSet(NXB_DOWN);
		m_btnChargeUp.AutoSet(NXB_UP);
		m_btnChargeDown.AutoSet(NXB_DOWN);
		m_btnDiagUp.AutoSet(NXB_UP);
		m_btnDiagDown.AutoSet(NXB_DOWN);
		m_btnMedicationUp.AutoSet(NXB_UP);
		m_btnMedicationDown.AutoSet(NXB_DOWN);
		m_btnEmnUp.AutoSet(NXB_UP);
		m_btnEmnDown.AutoSet(NXB_DOWN);
		m_btnProcedureUp.AutoSet(NXB_UP);
		m_btnProcedureDown.AutoSet(NXB_DOWN);
		// (c.haag 2008-06-02 17:04) - PLID 30221 - Buttons for todo items
		m_btnTodoUp.AutoSet(NXB_UP);
		m_btnTodoDown.AutoSet(NXB_DOWN);
		// (z.manning 2008-10-01 14:23) - PLID 31556 - Lab actions
		m_btnLabUp.AutoSet(NXB_UP);
		m_btnLabDown.AutoSet(NXB_DOWN);

		// Set the descriptive header text for this dialog
		{
			if (m_nSourceID != bisidNotBoundToData) {
				_RecordsetPtr rsSourceName = CreateRecordset("SELECT %s FROM %s WHERE %s = %li", GetEmrActionObjectSourceNameField(m_SourceType), GetEmrActionObjectSourceTable(m_SourceType), GetEmrActionObjectSourceIdField(m_SourceType), m_nSourceID);
				if (!rsSourceName->eof) {
					m_strSourceActionName = ConvertToControlText(VarString(rsSourceName->Fields->Item[0L]->Value, ""));
				} else {
					MsgBox("Attempted to load Action dialog for invalid object!");
					CNxDialog::OnOK();
					return TRUE;
				}
			} else {
				m_strSourceActionName = m_strSourceObjectName;
			}
			CString strCaption;
			// (r.gonet 08/03/2012) - PLID 51947 - Wound Care has a bit different wording.
			if(m_SourceType != eaoWoundCareCodingCondition) {
				strCaption.Format("When the %s \"%s\" is added to %s, automatically add the following items:", 
					GetEmrActionObjectName(m_SourceType, bIsLevel2), m_strSourceActionName, bIsLevel2 ? "an EMR" : "a record");
			} else {
				strCaption.Format("When the %s \"%s\" is met, automatically add the following items:", 
					GetEmrActionObjectName(m_SourceType, bIsLevel2), m_strSourceActionName);
			}
			
			SetDlgItemText(IDC_ACTION_CAPTION, strCaption);
		}


		// For the service code combo, default to cpt code
		CheckDlgButton(IDC_RADIO_SERVICE_CODE, TRUE);

		// Bind all the combos only requerying the ones that are guaranteed to be visible at startup
		m_pCptCombo = BindNxDataListCtrl(IDC_ACTION_CPT_COMBO);
		//(s.dhole 07/08/2014 8:56 AM ) - PLID 62592
		m_pDiagSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_ACTION_DIAG_SEARCH, GetRemoteData()); 
		m_pProductCombo = BindNxDataListCtrl(IDC_ACTION_PRODUCT_COMBO,false);
		m_pProductCombo->PutWhereClause(_bstr_t("ServiceT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE Billable = 1) AND ServiceT.Active = 1"));
		m_pProductCombo->Requery();
		m_pItemCombo = BindNxDataListCtrl(IDC_ACTION_ITEM_COMBO, false);//Don't requery if it won't be visible.
		m_pMintActionCombo = BindNxDataListCtrl(IDC_ACTION_EMN_TEMPLATE_COMBO, false);

		// (j.jones 2016-01-25 15:31) - PLID 67998 - if they do not have FDB, don't show the free text medication search option
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			//no FDB? no checkbox, and always include free-text meds, because that's all you've got
			m_checkIncludeFreeTextMeds.SetCheck(TRUE);
			m_checkIncludeFreeTextMeds.ShowWindow(SW_HIDE);
			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color, hide if no fdb
			m_icoAboutEMRActiontMedsColors.ShowWindow(SW_HIDE);
		}
		else {
			// (j.jones 2016-01-25 15:31) - PLID 67998 - added option to include free text meds in the medication search
			long nIncludeFreeTextFDBSearchResults = GetRemotePropertyInt("IncludeFreeTextFDBSearchResults", 0, 0, GetCurrentUserName(), true);
			//use the prescription option, since spawned meds are always prescriptions
			m_checkIncludeFreeTextMeds.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_PRESCRIPTIONS);

			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color
			CString strToolTipText = "All medications with a salmon background are imported and are checked for interactions. \r\n"
				"All medications with a red background have changed since being imported, and must be updated before being used on new prescriptions. \r\n"
				"Using free text medications (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			m_icoAboutEMRActiontMedsColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strToolTipText, false, false, false);
			m_icoAboutEMRActiontMedsColors.EnableClickOverride();
		}

		// (b.savon 2015-12-30 11:16) - PLID 67758
		// (j.jones 2016-01-25 15:28) - PLID 67998 - moved the medication bind to ResetMedicationSearchProvider
		ResetMedicationSearchProvider();
		
		m_pProcedureCombo = BindNxDataListCtrl(IDC_ACTION_PROCEDURE_COMBO, false);
		m_pMintItemActionCombo = BindNxDataListCtrl(IDC_ACTION_EMN_TEMPLATE_ITEM_COMBO, false);
		// (z.manning 2008-10-01 14:19) - PLID 31556 - Lab actions
		m_pLabCombo = BindNxDataListCtrl(IDC_ACTION_LAB_COMBO, false);

		
		// Now bind all the lists, but don't requery any of them because even if they're visible, we may end up 
		// loading them manually in case our m_nSourceID is bisidNotBoundToData.
		m_pChargeList = BindNxDataListCtrl(IDC_ACTION_CPT_LIST, false);
		//(s.dhole 7/14/2014 10:48 AM ) - PLID 62593
		m_pDiagList = BindNxDataList2Ctrl (IDC_ACTION_DIAG_LIST, false);
		m_pItemList = BindNxDataListCtrl(IDC_ACTION_ITEM_LIST, false);
		m_pMintActionList = BindNxDataListCtrl(IDC_ACTION_EMN_TEMPLATE_LIST, false);
		m_pMintItemActionList = BindNxDataListCtrl(IDC_ACTION_EMN_TEMPLATE_ITEM_LIST, false);
		m_pMedicationList = BindNxDataListCtrl(IDC_ACTION_MEDICATION_LIST, false);
		m_pProcedureList = BindNxDataListCtrl(IDC_ACTION_PROCEDURE_LIST, false);
		// (c.haag 2008-05-30 12:37) - PLID 30221 - Bind the todo list
		m_pTodoList = BindNxDataListCtrl(IDC_ACTION_TODO_LIST, false);
		m_pTodoList->GetColumn(ectlCategoryID)->ComboSource = _bstr_t("SELECT ID, Description FROM NoteCatsF ORDER BY Description");
		// (z.manning 2008-10-01 14:21) - PLID 31566 - Lab actions
		m_pLabList = BindNxDataListCtrl(IDC_ACTION_LAB_LIST, false);
		
		//DRT 1/10/2007 - PLID 24181 - Added some modifier combos to the charge list.  Since we don't
		//	requery the whole list, we have to set these manually for them to refresh.
		{
			//Copied from Billing
			// (z.manning, 05/01/2007) - PLID 16623 - Don't show inactive modifiers.
			CString strSource = "SELECT Number AS ID, (Number + '   ' + Note) AS Text, Active FROM CPTModifierT UNION SELECT '' AS ID, '     (None)' AS Text, 1 ORDER BY Text ASC";
			IColumnSettingsPtr pCol = m_pChargeList->GetColumn(clcDefMod1);
			pCol->ComboSource = _bstr_t(strSource);
			pCol = m_pChargeList->GetColumn(clcDefMod2);
			pCol->ComboSource = _bstr_t(strSource);
			pCol = m_pChargeList->GetColumn(clcDefMod3);
			pCol->ComboSource = _bstr_t(strSource);
			pCol = m_pChargeList->GetColumn(clcDefMod4);
			pCol->ComboSource = _bstr_t(strSource);
		}

		
		
		OnSelChangedActionEmnTemplateItemList(-1);
		OnSelChangedActionItemList(-1);
		OnSelChangedChargeList(-1);
		//OnSelChangedDiagList(-1);
		OnSelChangedMedicationList(-1);
		OnSelChangedEmnList(-1);
		OnSelChangedProcedureList(-1);
		OnSelChangedActionTodoList(-1);
		OnSelChangedLabList(-1); // (z.manning 2008-10-01 14:30) - PLID 31556

		//TES 12/28/2005 - Next, move all the controls to where they need to be.
		//DRT 1/17/2008 - PLID 28602 - Image hot spots can spawn details.
		// (z.manning 2009-02-10 15:17) - PLID 33026 - So can EMR table dropdown elements
		// (z.manning 2010-02-15 11:11) - PLID 37226 - So can smart stamps
		if(m_SourceType == eaoEmrDataItem || m_SourceType == eaoEmrImageHotSpot || m_SourceType == eaoEmrTableDropDownItem
			|| m_SourceType == eaoSmartStamp)
		{
			//This is allowed to spawn emr items.  Expand the dialog accordingly.
			CRect rAnchor;
			GetDlgItem(IDC_EMR_ACTION_ANCHOR_LARGE)->GetWindowRect(rAnchor);
			CRect rWindow;
			GetWindowRect(rWindow);
			MoveWindow(CRect(rWindow.left, rWindow.top, rAnchor.right, rAnchor.bottom));
			ScreenToClient(rAnchor);
			CRect rOK;
			GetDlgItem(IDOK)->GetWindowRect(rOK);
			ScreenToClient(rOK);
			GetDlgItem(IDOK)->MoveWindow(CRect(rOK.left, rAnchor.top-5-rOK.Height(), rOK.right, rAnchor.top-5));
			CRect rCancel;
			GetDlgItem(IDCANCEL)->GetWindowRect(rCancel);
			ScreenToClient(rCancel);
			GetDlgItem(IDCANCEL)->MoveWindow(CRect(rCancel.left, rAnchor.top-5-rCancel.Height(), rCancel.right, rAnchor.top-5));
			CRect rColor;
			GetDlgItem(IDC_NXCOLORCTRL1)->GetWindowRect(rColor);
			ScreenToClient(rColor);
			GetDlgItem(IDC_NXCOLORCTRL1)->MoveWindow(CRect(rColor.left, rColor.top, rColor.right, rAnchor.top + 10));
			GetDlgItem(IDC_EMR_ITEM_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ACTION_ITEM_COMBO)->ShowWindow(SW_SHOW);
			// (c.haag 2006-02-28 16:26) - PLID 12763 - Don't allow the user
			// to choose the same item they're modifying to spawn
			if (-1 != m_nOriginatingID) {
				CString strWhere;
				// (j.jones 2010-06-04 16:51) - PLID 39029 - this has never been supposed to spawn ID -27,
				// I fixed it while also disallowing generic tables (DataSubType = 3)
				strWhere.Format("Inactive = 0 AND EmrInfoT.ID NOT IN (%li, %li) AND EMRInfoT.DataSubType <> %li", EMR_BUILT_IN_INFO__TEXT_MACRO, m_nOriginatingID, eistGenericTable);
				m_pItemCombo->WhereClause = _bstr_t(strWhere);
			}
			m_pItemCombo->Requery();
			GetDlgItem(IDC_ACTION_ITEM_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_COMBO)->ShowWindow(SW_SHOW);
			m_pMintActionCombo->Requery();
			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_LIST)->ShowWindow(SW_SHOW);

			//TES 1/3/2007 - PLID 21878 - The Medications are now always visible.
			/*GetDlgItem(IDC_ACTION_MEDICATION_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_MEDICATION_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MEDICATION_LABEL)->ShowWindow(SW_HIDE);*/

			//GetDlgItem(IDC_ACTION_ALLERGY_COMBO)->ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_ACTION_ALLERGY_LIST)->ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_ALLERY_LABEL)->ShowWindow(SW_SHOW);

			GetDlgItem(IDC_ACTION_PROCEDURE_COMBO)->ShowWindow(SW_SHOW);
			m_pProcedureCombo->Requery();
			GetDlgItem(IDC_ACTION_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_PROCEDURE_LABEL)->ShowWindow(SW_SHOW);

			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_ITEM_COMBO)->ShowWindow(SW_SHOW);
			m_pMintItemActionCombo->Requery();
			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_ITEM_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EMN_TEMPLATE_ITEM_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_EMR_MINT_ITEM_UP)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BTN_EMR_MINT_ITEM_DOWN)->ShowWindow(SW_SHOW);

			// (z.manning 2008-10-01 14:27) - PLID 31556
			int nLabShowCmd = SW_HIDE;
			if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
				nLabShowCmd = SW_SHOW;
				m_pLabCombo->Requery();
			}
			GetDlgItem(IDC_ACTION_LAB_COMBO)->ShowWindow(nLabShowCmd);
			GetDlgItem(IDC_ACTION_LAB_LIST)->ShowWindow(nLabShowCmd);
			GetDlgItem(IDC_LABS_LABEL)->ShowWindow(nLabShowCmd);
			GetDlgItem(IDC_BTN_LAB_UP)->ShowWindow(nLabShowCmd);
			GetDlgItem(IDC_BTN_LAB_DOWN)->ShowWindow(nLabShowCmd);

		}
		else {
			//This is not allowed to spawn emr items.  Shrink the dialog accordingly.
			CRect rAnchor;
			GetDlgItem(IDC_EMR_ACTION_ANCHOR_SMALL)->GetWindowRect(rAnchor);
			CRect rWindow;
			GetWindowRect(rWindow);
			MoveWindow(CRect(rWindow.left, rWindow.top, rAnchor.right, rAnchor.bottom));
			ScreenToClient(rAnchor);
			CRect rcDialog;
			GetWindowRect(rcDialog);
			ScreenToClient(rcDialog);
			CRect rOK;
			GetDlgItem(IDOK)->GetWindowRect(rOK);
			ScreenToClient(rOK);
			GetDlgItem(IDOK)->MoveWindow(CRect(rcDialog.Width() / 4, rAnchor.top-5-rOK.Height(), rcDialog.Width() / 2 - 10, rAnchor.top-5));
			CRect rCancel;
			GetDlgItem(IDCANCEL)->GetWindowRect(rCancel);
			ScreenToClient(rCancel);
			GetDlgItem(IDCANCEL)->MoveWindow(CRect(rcDialog.Width() / 2 + 10, rAnchor.top-5-rCancel.Height(), rcDialog.Width() * 3 / 4, rAnchor.top-5));
			CRect rColor;
			GetDlgItem(IDC_NXCOLORCTRL1)->GetWindowRect(rColor);
			ScreenToClient(rColor);
			GetDlgItem(IDC_NXCOLORCTRL1)->MoveWindow(CRect(rColor.left, rColor.top, rColor.right, rAnchor.top + 5));
			GetDlgItem(IDC_EMR_ITEM_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_ITEM_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_ITEM_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMN_TEMPLATE_LABEL)->ShowWindow(SW_HIDE);
			
			//TES 1/3/2007 - PLID 21878 - The Medications are now always visible.
			/*GetDlgItem(IDC_ACTION_MEDICATION_COMBO)->ShowWindow(SW_SHOW);
			m_pMedicationCombo->Requery();
			GetDlgItem(IDC_ACTION_MEDICATION_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_MEDICATION_LABEL)->ShowWindow(SW_SHOW);*/

			//GetDlgItem(IDC_ACTION_ALLERGY_COMBO)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_ACTION_ALLERGY_LIST)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_ALLERY_LABEL)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_ACTION_PROCEDURE_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_PROCEDURE_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROCEDURE_LABEL)->ShowWindow(SW_HIDE);

			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_ITEM_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_EMN_TEMPLATE_ITEM_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EMN_TEMPLATE_ITEM_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_EMR_MINT_ITEM_UP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_EMR_MINT_ITEM_DOWN)->ShowWindow(SW_HIDE);

			// (z.manning 2008-10-01 14:27) - PLID 31556
			GetDlgItem(IDC_ACTION_LAB_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_LAB_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LABS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_LAB_UP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_LAB_DOWN)->ShowWindow(SW_HIDE);
		}

		// (c.haag 2008-07-22 16:09) - PLID 30723 - If the source is a hotspot, then we don't allow associating
		// mint items with problems since problems cannot be bound to either hotspots or mint items
		// (z.manning 2009-02-10 15:19) - PLID 33026 - Same with EMR table dropdown items
		// (z.manning 2011-11-11 15:19) - PLID 46231 - Actually problems can be associated with topics so no need to restrict this.
		/*if (m_SourceType == eaoEmrImageHotSpot || m_SourceType == eaoEmrTableDropDownItem || m_SourceType == eaoSmartStamp) {
			m_pMintItemActionList->GetColumn(alcProblemBitmap)->StoredWidth = 0L;
		}*/

	
		//Now, load the existing actions.
		MFCArray<EmrAction> arActions;
		if(m_nSourceID != bisidNotBoundToData) {
			LoadActionInfo(CSqlFragment("SourceType = {INT} AND SourceID = {INT} AND Deleted = 0", m_SourceType, m_nSourceID), m_arActions);
		}
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		int i = 0;
		for(i = 0; i < m_arActions.GetSize(); i++) {
			arActions.Add(m_arActions[i]);
		}
		for(i = 0; i < arActions.GetSize(); i++) {
			EmrAction ea = arActions[i];
			// (c.haag 2008-07-17 10:33) - PLID 30723 - Whenever a new row is added for any type, the result
			// always goes into pRow.
			IRowSettingsPtr pRow;
			NXDATALIST2Lib::IRowSettingsPtr pRow2;
			switch(ea.eaoDestType) {
			case eaoCpt:
				{
					long nSourceRow = -1;

					//TODO - (d.thompson - 1/10/2007 - While implementing 24181, I noticed that
					//	the requery was ALWAYS happening when I loaded here, never once did it
					//	finish.  Meaning we're doing a query in the else case every time, when
					//	we really might just need to wait a few milliseconds.  Patience is a virtue!
					//	I put in PLID 24202 to ponder this behavior.

					//If the combo has finished loading, pull it from the combo.
					if(!m_pCptCombo->IsRequerying()) {
						nSourceRow = m_pCptCombo->FindByColumn(cdcID,ea.nDestID,0,FALSE);
					}
					if(nSourceRow != -1) {
						IRowSettingsPtr pSourceRow = m_pCptCombo->GetRow(nSourceRow);
						pRow = m_pChargeList->GetRow(-1);
						pRow->PutValue(clcActionID, ea.nID);
						pRow->PutValue(clcID, ea.nDestID);
						pRow->PutValue(clcName, pSourceRow->GetValue(cdcCode));
						//TES 5/12/2008 - PLID 29577 - Fill in the SubCode column.
						pRow->PutValue(clcSubCode, pSourceRow->GetValue(cdcSubCode));
						pRow->PutValue(clcDescription, pSourceRow->GetValue(cdcDescription));
						// (j.jones 2010-01-11 09:56) - PLID 24504 - added Price
						pRow->PutValue(clcPrice, pSourceRow->GetValue(cdcPrice));
						pRow->PutValue(clcSortOrder, ea.nSortOrder);
						pRow->PutValue(clcPopup, VARIANT_FALSE);
						//DRT 1/10/2007 - PLID 24181 - Add new charge fields to the charge list only
						pRow->PutValue(clcPrompt, ea.bPrompt ? g_cvarTrue : g_cvarFalse);
						pRow->PutValue(clcDefQty, (double)ea.dblDefaultQuantity);
						pRow->PutValue(clcDefMod1, _bstr_t(ea.strMod1));
						pRow->PutValue(clcDefMod2, _bstr_t(ea.strMod2));
						pRow->PutValue(clcDefMod3, _bstr_t(ea.strMod3));
						pRow->PutValue(clcDefMod4, _bstr_t(ea.strMod4));
						m_pChargeList->AddRow(pRow);
					}
					else {
						//We'll have to look it up.
						_RecordsetPtr rsCodeInfo = CreateParamRecordset("SELECT ServiceT.Name, ServiceT.Price, "
							"CptCodeT.Code, CptCodeT.SubCode "
							"FROM ServiceT LEFT JOIN CptCodeT ON ServiceT.ID = CptCodeT.ID "
							"WHERE ServiceT.ID = {INT}", ea.nDestID);
						if(!rsCodeInfo->eof) {
							pRow = m_pChargeList->GetRow(-1);
							pRow->PutValue(clcActionID, ea.nID);
							pRow->PutValue(clcID, ea.nDestID);
							pRow->PutValue(clcName, rsCodeInfo->Fields->GetItem("Code")->Value);
							//TES 5/12/2008 - PLID 29577 - Fill in the SubCode column.
							pRow->PutValue(clcSubCode, rsCodeInfo->Fields->GetItem("SubCode")->Value);
							pRow->PutValue(clcDescription, rsCodeInfo->Fields->GetItem("Name")->Value);
							// (j.jones 2010-01-11 09:56) - PLID 24504 - added Price
							pRow->PutValue(clcPrice, rsCodeInfo->Fields->GetItem("Price")->Value);
							pRow->PutValue(clcSortOrder, ea.nSortOrder);
							pRow->PutValue(clcPopup, VARIANT_FALSE);

							//DRT 1/10/2007 - PLID 24181 - We need to lookup the charge specific data in the
							//	EMRActionChargeDataT table, only if we're on a Cpt type action.  Since we're in
							//	the section of the load for charges, this MUST be true
							if(ea.eaoDestType != eaoCpt) {
								ASSERT(FALSE);
							}
							else {
								//If the action ID is -1, this hasn't yet been saved.  Just reload the data from 
								//	the EmrAction structure.
								pRow->PutValue(clcPrompt, ea.bPrompt ? g_cvarTrue : g_cvarFalse);
								pRow->PutValue(clcDefQty, (double)ea.dblDefaultQuantity);
								pRow->PutValue(clcDefMod1, _bstr_t(ea.strMod1));
								pRow->PutValue(clcDefMod2, _bstr_t(ea.strMod2));
								pRow->PutValue(clcDefMod3, _bstr_t(ea.strMod3));
								pRow->PutValue(clcDefMod4, _bstr_t(ea.strMod4));
							}

							m_pChargeList->AddRow(pRow);
						}
						else {
							ASSERT(FALSE);
						}
						rsCodeInfo->Close();
					}
				}
				break;
			// (b.savon 2014-07-14 11:12) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
			///case eaoDiag:
			case eaoDiagnosis:
				{
				//(s.dhole 7/18/2014 11:04 AM ) - PLID 62724 Will load code from database
					NXDATALIST2Lib::IRowSettingsPtr pSourceRow;
					_RecordsetPtr rsCodeInfo = CreateParamRecordset("SELECT ICD9ID, CodeNumber9, CodeDesc9, ICD10ID, CodeNumber10, CodeDesc10 "
					"FROM (SELECT ID AS ICD9ID, CodeNumber AS CodeNumber9, CodeDesc AS CodeDesc9, 1 AS IDJoin FROM DiagCodes  WHERE  ICD10 = 0 AND ID = {INT}) As  DiagCodes9Q "
					"FULL OUTER JOIN "
					"(SELECT ID AS ICD10ID, CodeNumber AS CodeNumber10, CodeDesc AS CodeDesc10, 1 AS IDJoin FROM DiagCodes WHERE  ICD10 = 1 AND ID =  {INT}) as DiagCodes10Q "
					"ON DiagCodes9Q.IDJoin = DiagCodes9Q.IDJoin ", ea.diaDiagnosis.nDiagCodeID_ICD9, ea.diaDiagnosis.nDiagCodeID_ICD10);
					if (!rsCodeInfo->eof) {
						pRow2 = m_pDiagList->GetNewRow();
						DiagCodeList diagCodeList;
						diagCodeList.nActionID = ea.nID;
						diagCodeList.nICD9 = ea.diaDiagnosis.nDiagCodeID_ICD9;
						diagCodeList.nICD10 = ea.diaDiagnosis.nDiagCodeID_ICD10;
						diagCodeList.strICD9Code = AdoFldString(rsCodeInfo, "CodeNumber9", "");
						diagCodeList.strICD10Code = AdoFldString(rsCodeInfo, "CodeNumber10", "");
						diagCodeList.strICD9Description = AdoFldString(rsCodeInfo, "CodeDesc9", "");
						diagCodeList.strICD10Description = AdoFldString(rsCodeInfo, "CodeDesc10", "");
						diagCodeList.matchType = nexgemtDone;
						HandelDatalistRowUpdate(pRow2, diagCodeList);
						pRow2->PutValue(adcOrder, ea.nSortOrder);
						pRow2->PutValue(adcPopup, VARIANT_FALSE);
						m_pDiagList->AddRowSorted(pRow2, NULL);
					}
				}
				break;
			case eaoProcedure:
				{
					long nSourceRow = -1;

					//If the combo has finished loading, pull it from the combo.
					if(!m_pProcedureCombo->IsRequerying()) {
						nSourceRow = m_pProcedureCombo->FindByColumn(0,ea.nDestID,0,FALSE);
					}
					if(nSourceRow != -1) {
						IRowSettingsPtr pSourceRow = m_pProcedureCombo->GetRow(nSourceRow);
						pRow = m_pProcedureList->GetRow(-1);
						pRow->PutValue(alcActionID, ea.nID);
						pRow->PutValue(alcID, ea.nDestID);
						pRow->PutValue(alcName, pSourceRow->GetValue(1));
						pRow->PutValue(alcName2, _bstr_t(""));
						pRow->PutValue(alcSortOrder, ea.nSortOrder);
						pRow->PutValue(alcPopup, VARIANT_FALSE);
						m_pProcedureList->AddRow(pRow);
					}
					else {
						//We'll have to look it up.
						_RecordsetPtr rsInfo = CreateRecordset("SELECT Name "
							"FROM ProcedureT "
							"WHERE ProcedureT.ID = %li", ea.nDestID);
						if(!rsInfo->eof) {
							pRow = m_pProcedureList->GetRow(-1);
							pRow->PutValue(alcActionID, ea.nID);
							pRow->PutValue(alcID, ea.nDestID);
							pRow->PutValue(alcName, rsInfo->Fields->GetItem("Name")->Value);
							pRow->PutValue(alcName2, _bstr_t(""));
							pRow->PutValue(alcSortOrder, ea.nSortOrder);
							pRow->PutValue(alcPopup, VARIANT_FALSE);
							m_pProcedureList->AddRow(pRow);
						}
						else {
							ASSERT(FALSE);
						}
					}
				}
				break;
			case eaoEmrItem:
				{
					long nSourceRow = -1;

					//If the combo has finished loading, pull it from the combo.
					if(!m_pItemCombo->IsRequerying()) {
						nSourceRow = m_pItemCombo->FindByColumn(0,ea.nDestID,0,FALSE);
					}
					if(nSourceRow != -1) {
						IRowSettingsPtr pSourceRow = m_pItemCombo->GetRow(nSourceRow);
						pRow = m_pItemList->GetRow(-1);
						pRow->PutValue(alcActionID, ea.nID);
						pRow->PutValue(alcID, ea.nDestID);
						pRow->PutValue(alcName, pSourceRow->GetValue(1));
						pRow->PutValue(alcName2, _bstr_t(""));
						pRow->PutValue(alcSortOrder, ea.nSortOrder);
						// (b.savon 2014-07-08 08:22) - PLID 62760 - Add DataType
						pRow->PutValue(alcDataType, pSourceRow->GetValue(4));
						pRow->PutValue(alcPopup, ea.bPopup?VARIANT_TRUE:VARIANT_FALSE);
						BYTE nDataSubType = VarByte(pSourceRow->GetValue(3));
						// (c.haag 2007-04-09 12:05) - PLID 25458 - Special coloring
						// for system EMR items
						if (nDataSubType == eistCurrentMedicationsTable ||
							nDataSubType == eistAllergiesTable)
						{
							pRow->ForeColor = RGB(0,0,255);
						}
						m_pItemList->AddRow(pRow);
					}
					else {
						//We'll have to look it up.
						// (b.savon 2014-07-08 08:22) - PLID 62760 - Add DataType
						_RecordsetPtr rsInfo = CreateRecordset("SELECT EmrInfoT.Name, EmrInfoT.DataSubType, "
							"CASE WHEN DataType = 1 THEN 'Text' WHEN DataType = 2 THEN 'Single-Select List' WHEN DataType = 3 THEN 'Multi-Select List' WHEN DataType = 4 THEN 'Image' WHEN DataType = 5 THEN 'Slider' WHEN DataType = 6 THEN 'Narrative' WHEN DataType = 7 THEN 'Table' END AS DataType "
							"FROM EmrInfoT INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
							"WHERE EmrInfoMasterT.ID = %li", ea.nDestID);
						if(!rsInfo->eof) {
							pRow = m_pItemList->GetRow(-1);
							pRow->PutValue(alcActionID, ea.nID);
							pRow->PutValue(alcID, ea.nDestID);
							pRow->PutValue(alcName, rsInfo->Fields->GetItem("Name")->Value);
							pRow->PutValue(alcName2, _bstr_t(""));
							pRow->PutValue(alcSortOrder, ea.nSortOrder);
							// (b.savon 2014-07-08 08:22) - PLID 62760 - Add DataType
							pRow->PutValue(alcDataType, rsInfo->Fields->GetItem("DataType")->Value);
							pRow->PutValue(alcPopup, ea.bPopup?VARIANT_TRUE:VARIANT_FALSE);
							BYTE nDataSubType = VarByte(rsInfo->Fields->GetItem("DataSubType")->Value);
							// (c.haag 2007-04-09 12:05) - PLID 25458 - Special coloring
							// for system EMR items
							if (nDataSubType == eistCurrentMedicationsTable ||
								nDataSubType == eistAllergiesTable)
							{
								pRow->ForeColor = RGB(0,0,255);
							}
							m_pItemList->AddRow(pRow);
						}
						else {
							ASSERT(FALSE);
						}
					}
				}
				break;
			case eaoMint:
				{
					long nSourceRow = -1;

					//If the combo has finished loading, pull it from the combo.
					if(!m_pMintActionCombo->IsRequerying()) {
						nSourceRow = m_pMintActionCombo->FindByColumn(0,ea.nDestID,0,FALSE);
					}
					if(nSourceRow != -1) {
						IRowSettingsPtr pSourceRow = m_pMintActionCombo->GetRow(nSourceRow);
						pRow = m_pMintActionList->GetRow(-1);
						pRow->PutValue(alcActionID, ea.nID);
						pRow->PutValue(alcID, ea.nDestID);
						pRow->PutValue(alcName, pSourceRow->GetValue(1));
						pRow->PutValue(alcName2, pSourceRow->GetValue(2));
						pRow->PutValue(alcSortOrder, ea.nSortOrder);
						pRow->PutValue(alcPopup, VARIANT_FALSE);
						m_pMintActionList->AddRow(pRow);
					}
					else {
						//We'll have to look it up.
						_RecordsetPtr rsInfo = CreateRecordset("SELECT EmrTemplateT.Name, EmrCollectionT.Name AS Collection "
							"FROM EmrTemplateT LEFT JOIN EmrCollectionT ON EmrTemplateT.CollectionID = EmrCollectionT.ID "
							"WHERE EmrTemplateT.ID = %li", ea.nDestID);
						if(!rsInfo->eof) {
							pRow = m_pMintActionList->GetRow(-1);
							pRow->PutValue(alcActionID, ea.nID);
							pRow->PutValue(alcID, ea.nDestID);
							pRow->PutValue(alcName, rsInfo->Fields->GetItem("Name")->Value);
							pRow->PutValue(alcName2, rsInfo->Fields->GetItem("Collection")->Value);
							pRow->PutValue(alcSortOrder, ea.nSortOrder);
							pRow->PutValue(alcPopup, VARIANT_FALSE);
							m_pMintActionList->AddRow(pRow);
						}
						else {
							ASSERT(FALSE);
						}
					}
				}
				break;
			case eaoMintItems:
				{
					long nSourceRow = -1;

					//If the combo has finished loading, pull it from the combo.
					if(!m_pMintItemActionCombo->IsRequerying()) {
						nSourceRow = m_pMintItemActionCombo->FindByColumn(0,ea.nDestID,0,FALSE);
					}
					if(nSourceRow != -1) {
						IRowSettingsPtr pSourceRow = m_pMintItemActionCombo->GetRow(nSourceRow);
						pRow = m_pMintItemActionList->GetRow(-1);
						pRow->PutValue(alcActionID, ea.nID);
						pRow->PutValue(alcID, ea.nDestID);
						pRow->PutValue(alcName, pSourceRow->GetValue(1));
						pRow->PutValue(alcName2, pSourceRow->GetValue(2));
						pRow->PutValue(alcSortOrder, ea.nSortOrder);
						pRow->PutValue(alcPopup, ea.bSpawnAsChild?(long)1:(long)0);
						m_pMintItemActionList->AddRow(pRow);
					}
					else {
						//We'll have to look it up.
						_RecordsetPtr rsInfo = CreateRecordset("SELECT EmrTemplateT.Name, EmrCollectionT.Name AS Collection "
							"FROM EmrTemplateT LEFT JOIN EmrCollectionT ON EmrTemplateT.CollectionID = EmrCollectionT.ID "
							"WHERE EmrTemplateT.ID = %li", ea.nDestID);
						if(!rsInfo->eof) {
							pRow = m_pMintItemActionList->GetRow(-1);
							pRow->PutValue(alcActionID, ea.nID);
							pRow->PutValue(alcID, ea.nDestID);
							pRow->PutValue(alcName, rsInfo->Fields->GetItem("Name")->Value);
							pRow->PutValue(alcName2, rsInfo->Fields->GetItem("Collection")->Value);
							pRow->PutValue(alcSortOrder, ea.nSortOrder);
							pRow->PutValue(alcPopup, ea.bSpawnAsChild?(long)1:(long)0);
							m_pMintItemActionList->AddRow(pRow);
						}
						else {
							ASSERT(FALSE);
						}
					}
				}
				break;
			case eaoMedication:
				{
					//We'll have to look it up.
					// (c.haag 2007-02-02 18:20) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
					// (b.savon 2015-12-30 10:42) - PLID 67758 - Added FDBID and FDBOutOfDate; call function
					_RecordsetPtr rsInfo = CreateRecordset("SELECT EMRDataT.Data AS Name, ISNULL(DrugList.FDBID, -1) AS FDBID, DrugList.FDBOutOfDate "
						"FROM DrugList "
						"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
						"WHERE DrugList.ID = %li", ea.nDestID);
					if(!rsInfo->eof) {
						AddMedicationToList(
							ea.nDestID,
							rsInfo->Fields->GetItem("Name")->Value,
							AdoFldLong(rsInfo->Fields, "FDBID"),
							AdoFldBool(rsInfo->Fields, "FDBOutOfDate", FALSE),
							ea.nID,
							ea.bPopup ? VARIANT_TRUE : VARIANT_FALSE,
							new CProblemActionAry(ea.aProblemActions),
							ea.nSortOrder
						);
					}
					else {
						ASSERT(FALSE);
					}
				}
				break;
			case eaoAllergy:
				//Obsolete
				break;
			case eaoTodo:
				// (c.haag 2008-06-04 11:02) - PLID 30221 - Load todo information
				{
					pRow = m_pTodoList->GetRow(-1);
					pRow->PutValue(ectlActionID, (long)ea.nID);
					pRow->PutValue(ectlSortOrder,(long)(m_pTodoList->GetRowCount() + 1));
					pRow->Value[ectlCategoryID] = ea.nTodoCategoryID;
					pRow->Value[ectlMethod] = _bstr_t(ea.strTodoMethod);
					pRow->Value[ectlNotes] = _bstr_t(ea.strTodoNotes);
					pRow->Value[ectlAssignTo] = _bstr_t(ArrayAsString(ea.anTodoAssignTo, false)); // (a.walling 2014-07-01 15:28) - PLID 62697
					pRow->Value[ectlPriority] = ea.nTodoPriority;
					pRow->Value[ectlRemindType] = ea.nTodoRemindType;
					pRow->Value[ectlRemindInterval] = ea.nTodoRemindInterval;
					pRow->Value[ectlDeadlineType] = ea.nTodoDeadlineType;
					pRow->Value[ectlDeadlineInterval] = ea.nTodoDeadlineInterval;
					m_pTodoList->AddRow(pRow);
				}
				break;
			case eaoLab:
				// (z.manning 2008-10-01 14:31) - PLID 31556 - Load lab action info
				{
					long nSourceRow = -1;

					//If the combo has finished loading, pull it from the combo.
					if(!m_pLabCombo->IsRequerying()) {
						nSourceRow = m_pLabCombo->FindByColumn(0,ea.nDestID,0,FALSE);
					}					
					if(nSourceRow != -1) {
						IRowSettingsPtr pSourceRow = m_pLabCombo->GetRow(nSourceRow);
						pRow = m_pLabList->GetRow(-1);
						pRow->PutValue(alcActionID, ea.nID);
						pRow->PutValue(alcID, ea.nDestID);
						pRow->PutValue(alcName, pSourceRow->GetValue(1));
						pRow->PutValue(alcName2, "");
						pRow->PutValue(alcSortOrder, ea.nSortOrder);
						pRow->PutValue(alcPopup, VARIANT_FALSE);
						m_pLabList->AddRow(pRow);
					}
					else {
						//We'll have to look it up.
						_RecordsetPtr rsLabInfo = CreateParamRecordset(
							"SELECT Name "
							"FROM LabProceduresT "
							"WHERE LabProceduresT.ID = {INT} ", ea.nDestID);
						if(!rsLabInfo->eof) {
							pRow = m_pLabList->GetRow(-1);
							pRow->PutValue(alcActionID, ea.nID);
							pRow->PutValue(alcID, ea.nDestID);
							pRow->PutValue(alcName, rsLabInfo->Fields->GetItem("Name")->Value);
							pRow->PutValue(alcSortOrder, ea.nSortOrder);
							pRow->PutValue(alcPopup, VARIANT_FALSE);
							m_pLabList->AddRow(pRow);
						}
						else {
							ASSERT(FALSE);
						}
					}
				}
				break;
			default:
				ASSERT(FALSE);
				{
					CString strMsg;
					strMsg.Format("An unexpected action destination type of %li was stored for action ID %li.  This action will be skipped.", 
						ea.eaoDestType, ea.nDestID);
					MessageBox(strMsg);
				}
				break;
			}

			if ((NULL != pRow && ea.eaoDestType != eaoDiagnosis) || (ea.eaoDestType == eaoDiagnosis &&  pRow2!=NULL))
			{
				// (z.manning 2011-06-28 16:11) - PLID 44347 - Keep track of the anatomic location IDs in the datalist
				short nLabAnatomyColumn = GetAnatomicLocationColumnFromDestType(ea.eaoDestType);
				if(nLabAnatomyColumn != -1) {
					// (a.walling 2014-08-04 09:39) - PLID 62687 - New structure for filter
					std::string strLabAnatomyIDs = ea.filter.ToXml();
		
					//(s.dhole 7/16/2014 3:14 PM ) - PLID 62724  Based on type call idividual functionality , apply to Dignossis list
					if (ea.eaoDestType==eaoDiagnosis )
					{ 
						//apply to Dignossis list
						pRow2->PutValue(nLabAnatomyColumn, _bstr_t(strLabAnatomyIDs.c_str()));
						UpdateDiagRowForAnatomyChange(pRow2, nLabAnatomyColumn);
					}
					else
					{
						pRow->PutValue(nLabAnatomyColumn, _bstr_t(strLabAnatomyIDs.c_str()));
						UpdateRowForAnatomyChange(pRow, nLabAnatomyColumn);
					}
				}

				// (c.haag 2008-07-17 10:22) - PLID 30723 - Load in problem-related data
				CProblemActionAry* pary = NULL;
				switch(ea.eaoDestType) {
				case eaoCpt:
					pary = new CProblemActionAry(ea.aProblemActions); // (a.walling 2014-07-01 15:28) - PLID 62697
					pRow->PutValue(clcProblemArray, (long)pary);
					UpdateActionProblemRow(pRow, clcProblemBitmap, clcProblemArray);
					break;
				case eaoDiagnosis:
					pary = new CProblemActionAry(ea.aProblemActions);
					//(s.dhole 7/18/2014 11:19 AM ) - PLID 62593
					pRow2->PutValue(adcProblemArray, (long)pary);
					UpdateDiagActionProblemRow(pRow2, adcProblemBitmap, adcProblemArray);
					break;
				case eaoProcedure:
				case eaoTodo:
				case eaoLab:
					// Not supported
					break;
				default:
					pary = new CProblemActionAry(ea.aProblemActions); // (a.walling 2014-07-01 15:28) - PLID 62697
					pRow->PutValue(alcProblemArray, (long)pary);
					UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
					break;
				}
			}

		} // for(i = 0; i < arActions.GetSize(); i++) {


		// (c.haag 2007-02-06 11:56) - PLID 24423 - If we are editing the actions
		// for the official Current Medications item, then disable medications actions
		// (c.haag 2007-04-03 09:03) - PLID 25468 - Same for the official allergies item
		// (b.savon 2015-12-30 09:51) - PLID 67758
		if (GetActiveCurrentMedicationsInfoID() == m_nOriginatingID ||
			GetActiveAllergiesInfoID() == m_nOriginatingID) {
			GetDlgItem(IDC_NXDL_ADD_MED_ACTION)->EnableWindow(FALSE);
			GetDlgItem(IDC_ACTION_MEDICATION_LIST)->EnableWindow(FALSE);
		}

		// (c.haag 2008-06-26 16:32) - PLID 30221 - Hide the todo action window if the source type is
		// anything other than EMR items or EMR Data Items
		// (z.manning 2009-02-10 15:20) - PLID 33026 - Allow EMR table dropdown items to spawn to-dos
		// (z.manning 2010-02-15 11:14) - PLID 37226 - Smart stamps can spawn to-dos
		if (m_SourceType != eaoEmrItem && m_SourceType != eaoEmrDataItem && m_SourceType != eaoEmrImageHotSpot
			&& m_SourceType != eaoEmrTableDropDownItem && m_SourceType != eaoSmartStamp)
		{
			GetDlgItem(IDC_TODO_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_NEW_TODO_TASK)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACTION_TODO_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_TODO_UP)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_TODO_DOWN)->ShowWindow(SW_HIDE);
		}
		UpdateDiagCodeArrows();//(s.dhole 7/14/2014 3:12 PM ) - PLID 62593 Update  Up.Down Buttons
		// (s.dhole 07\09\2014 8:56) - PLID 63114  Set dignosis column 
		UpdateDiagnosisListColumnSizes();

	}NxCatchAll("Error in CEmrActionDlg::OnInitDialog()");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

namespace {
	// (z.manning 2011-06-29 09:53) - PLID 44347 - Save anatomic location filter data
	// (a.walling 2014-08-06 12:40) - PLID 62692 - Laterality - Saving
	void SaveActionFilterToBatch(CString& strBatch, const EmrAction& ea, const CString& strActionIDorVar)
	{
		// (a.walling 2014-09-11 15:22) - PLID 62692 - Do not bother ignoring redundant details; just save everything.

		long lastInsertedLocation = -1;
		for (const auto& detail : ea.filter.anatomicLocationFilters)
		{
			if (lastInsertedLocation != detail.anatomicLocationID) {
				lastInsertedLocation = detail.anatomicLocationID;
				
				long anySide = ea.filter.GetAnyQualifierSide(detail.anatomicLocationID);
				long noSide = ea.filter.GetNoQualifierSide(detail.anatomicLocationID);

				AddStatementToSqlBatch(strBatch,
					"INSERT INTO EmrActionAnatomicLocationsT (EmrActionID, LabAnatomyID, AnyQualifierAnatomySide, NoQualifierAnatomySide) VALUES (%s, %li, %li, %li)"
					, strActionIDorVar, detail.anatomicLocationID, anySide, noSide);
			}

			if (detail.qualifierID > 0 && detail.anatomySide != Emr::sideNull) {
				AddStatementToSqlBatch(strBatch,
					"INSERT INTO EmrActionAnatomicLocationQualifiersT (EmrActionID, LabAnatomyID, AnatomicQualifierID, AnatomySide) VALUES (%s, %li, %li, %li)"
					, strActionIDorVar, detail.anatomicLocationID, detail.qualifierID, detail.anatomySide);
			}

			//TODO: Handle aeiEmrActionAnatomicLocationFilter when we audit actions in this dialog.
		}
	}
}

void CEmrActionDlg::OnOK() 
{
	try {

		CWaitCursor wc;

		//Get the current list of actions from the interface.
		MFCArray<EmrAction> arCurrentActions;



		//m.hancock - 5/1/2006 - PLID 20366 - Bad variable type exception when pressing ok on the EMR Automatic Generation dialog
		//A call to ea.bPopup = (eeaotyp == eaoEmrItem)?VarBool(pRow->GetValue(alcPopup)):false; was failing because old data
		//had -1 and was being treated as a Short instead of a bool.
		// (j.jones 2007-07-16 16:16) - PLID 26694 - bOnePerEmn is a value of EmrAction for the
		// purposes of efficiency, but is not a property of the action, and isn't needed here
		// for saving purposes.
		// (c.haag 2008-07-18 09:54) - PLID 30723 - Store problem-related data
		// (a.walling 2009-04-22 11:02) - PLID 28957 - Support popup for medications
		// (z.manning 2011-06-29 09:57) - PLID 44347 - Support anatomic locations IDs
		// (a.walling 2014-08-04 10:31) - PLID 62692 - This used to be a #define macro
		auto CEAD__OO_AddActionsToArray = [&](_DNxDataListPtr pdl, EmrActionObject eeaotyp)	{
			int nSortOrder = 1;
			long p = pdl->GetFirstRowEnum();
			while (p) {
				LPDISPATCH lpDisp = NULL;
				pdl->GetNextRowEnum(&p, &lpDisp);
				if (lpDisp) {
					IRowSettingsPtr pRow(lpDisp);
					lpDisp->Release();
					EmrAction ea;
					ea.nSourceID = m_nSourceID;
					ea.eaoSourceType = m_SourceType;
					ea.eaoDestType = eeaotyp;
					ea.nID = VarLong(pRow->GetValue(alcActionID), -1);
					ea.nDestID = VarLong(pRow->GetValue(alcID));
					ea.nSortOrder = nSortOrder;
					if (eeaotyp == eaoEmrItem || eeaotyp == eaoMedication) {
						if (pRow->GetValue(alcPopup).vt == VT_I2)
							ea.bPopup = (VarShort(pRow->GetValue(alcPopup)) != 0) ? TRUE : FALSE;
						else
							ea.bPopup = VarBool(pRow->GetValue(alcPopup), FALSE);
					}
					else
						ea.bPopup = false;
					ea.bSpawnAsChild = (eeaotyp == eaoMintItems) ? VarLong(pRow->GetValue(alcPopup)) : false;
					StoreProblemActions(pRow, alcProblemArray, ea);
					// (a.walling 2014-08-04 09:39) - PLID 62687 - New structure for filter
					CString strFilterXml = VarString(pRow->GetValue(alcLabAnatomyIDs), "");
					ea.filter = Emr::ActionFilter::FromXml(strFilterXml);
					arCurrentActions.Add(ea);
					nSortOrder++;
				}
			}
		};

		//CEAD__OO_AddActionsToArray(m_pChargeList, eaoCpt);
		
		CEAD__OO_AddActionsToArray(m_pProcedureList, eaoProcedure);
		CEAD__OO_AddActionsToArray(m_pItemList, eaoEmrItem);
		CEAD__OO_AddActionsToArray(m_pMintActionList, eaoMint);
		CEAD__OO_AddActionsToArray(m_pMintItemActionList, eaoMintItems);
		CEAD__OO_AddActionsToArray(m_pMedicationList, eaoMedication);
		CEAD__OO_AddActionsToArray(m_pLabList, eaoLab);


		{

			//(s.dhole 7/15/2014 8:06 AM ) - PLID 62724 set Diagnosis action chages to  arrea, since this functionality do not use common strcture.
			int nSortOrder = 1;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->GetFirstRow();
			while (pRow) {
				EmrAction ea;
				ea.nSourceID = m_nSourceID;
				ea.eaoSourceType = m_SourceType;
				// (b.savon 2014-07-15 15:50) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
				ea.eaoDestType = eaoDiagnosis/*eaoDiag*/;
				ea.nID = VarLong(pRow->GetValue(adcActionID), -1);
		
				ea.diaDiagnosis.nDiagCodeID_ICD9 = VarLong(pRow->GetValue(adcICD9DiagCodeID));
				ea.diaDiagnosis.nDiagCodeID_ICD10 = VarLong(pRow->GetValue(adcICD10DiagCodeID));
				ea.nSortOrder = nSortOrder;
				ea.bPopup = false;
				ea.bSpawnAsChild = false;
				// (c.haag 2008-07-18 09:54) - PLID 30723 - Store problem-related data
				StoreProblemActions(pRow, adcProblemArray, ea);
				// (a.walling 2014-08-04 09:39) - PLID 62687 - New structure for filter
				CString strFilterXml = VarString(pRow->GetValue(adcLabAnatomyIDs), "");
				ea.filter = Emr::ActionFilter::FromXml(strFilterXml);
				// for saving purposes.
				arCurrentActions.Add(ea);
				pRow =pRow->GetNextRow();
				nSortOrder++;
			}
		
		}

		//DRT 1/10/2007 - Removed charge actions from the macro above, they're now unique with
		//	separate column enumeration and additional fields.  This should be kept roughly the same
		//	as the macro above, if changes are made in the future.
		{
			int nSortOrder = 1;
			long p = m_pChargeList->GetFirstRowEnum();
			while (p) {
				LPDISPATCH lpDisp = NULL;
				m_pChargeList->GetNextRowEnum(&p, &lpDisp);
				if (lpDisp) {
					IRowSettingsPtr pRow(lpDisp);
					lpDisp->Release();
					EmrAction ea;
					ea.nSourceID = m_nSourceID;
					ea.eaoSourceType = m_SourceType;
					ea.eaoDestType = eaoCpt;
					ea.nID = VarLong(pRow->GetValue(clcActionID), -1);
					ea.nDestID = VarLong(pRow->GetValue(clcID));
					ea.nSortOrder = nSortOrder;
					ea.bPopup = false;
					ea.bSpawnAsChild = false;
					ea.bPrompt = VarBool(pRow->GetValue(clcPrompt), FALSE);
					ea.dblDefaultQuantity = VarDouble(pRow->GetValue(clcDefQty), 1.0);
					ea.strMod1 = VarString(pRow->GetValue(clcDefMod1), "");
					ea.strMod2 = VarString(pRow->GetValue(clcDefMod2), "");
					ea.strMod3 = VarString(pRow->GetValue(clcDefMod3), "");
					ea.strMod4 = VarString(pRow->GetValue(clcDefMod4), "");
					// (c.haag 2008-07-18 09:54) - PLID 30723 - Store problem-related data
					StoreProblemActions(pRow, clcProblemArray, ea);
					// (z.manning 2011-06-29 10:00) - PLID 44347 - Anatomic location IDs
					// (a.walling 2014-08-04 09:39) - PLID 62687 - New structure for filter
					CString strFilterXml = VarString(pRow->GetValue(clcLabAnatomyIDs), "");
					ea.filter = Emr::ActionFilter::FromXml(strFilterXml);

					// (j.jones 2007-07-16 16:16) - PLID 26694 - bOnePerEmn is a value of EmrAction for the
					// purposes of efficiency, but is not a property of the action, and isn't needed here
					// for saving purposes.

					arCurrentActions.Add(ea);
					nSortOrder++;
				}
			}
		}

		// (c.haag 2008-06-03 10:54) - PLID 30221 - Handle todo alarm actions
		{
			int nSortOrder = 1;
			long p = m_pTodoList->GetFirstRowEnum();
			while (p) {
				LPDISPATCH lpDisp = NULL;
				m_pTodoList->GetNextRowEnum(&p, &lpDisp);
				if (lpDisp) {
					IRowSettingsPtr pRow(lpDisp);
					lpDisp->Release();
					EmrAction ea;
					ea.nSourceID = m_nSourceID;
					ea.eaoSourceType = m_SourceType;
					ea.eaoDestType = eaoTodo;
					ea.nID = VarLong(pRow->GetValue(ectlActionID), -1);
					ea.nDestID = VarLong(pRow->GetValue(ectlActionID), -1);
					ea.nSortOrder = nSortOrder++;
					ea.bPopup = FALSE;
					ea.bSpawnAsChild = FALSE;
					// (z.manning 2011-06-29 10:00) - PLID 44347 - Anatomic location IDs
					// (a.walling 2014-08-04 09:39) - PLID 62687 - New structure for filter
					CString strFilterXml = VarString(pRow->GetValue(ectlLabAnatomyIDs), "");
					ea.filter = Emr::ActionFilter::FromXml(strFilterXml);

					ea.nTodoCategoryID = VarLong(pRow->GetValue(ectlCategoryID),-1);
					ea.strTodoMethod = VarString(pRow->GetValue(ectlMethod));
					ea.strTodoNotes = VarString(pRow->GetValue(ectlNotes));

					// (a.walling 2014-07-01 15:28) - PLID 62697
					StringAsArray(VarString(pRow->Value[ectlAssignTo]), ea.anTodoAssignTo, true);

					ea.nTodoPriority = VarLong(pRow->GetValue(ectlPriority));
					ea.nTodoRemindType = VarLong(pRow->GetValue(ectlRemindType));
					ea.nTodoRemindInterval = VarLong(pRow->GetValue(ectlRemindInterval));
					ea.nTodoDeadlineType = VarLong(pRow->GetValue(ectlDeadlineType));
					ea.nTodoDeadlineInterval = VarLong(pRow->GetValue(ectlDeadlineInterval));

					arCurrentActions.Add(ea);
				}
			}
		}

		// Now only write back to data if we're bound to a specific source id
		if (m_nSourceID != bisidNotBoundToData) {
			//Parse out what we need to do.
			MFCArray<EmrAction> arActionsToAdd, arActionsToUpdate, arActionsToRemove;
			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			for(i = 0; i < arCurrentActions.GetSize(); i++) arActionsToAdd.Add(arCurrentActions[i]);
			for(i = 0; i < m_arActions.GetSize(); i++) arActionsToRemove.Add(m_arActions[i]);
			
			for(i = arActionsToAdd.GetSize() - 1; i >= 0; i--)
			{
				EmrAction eaNew = arActionsToAdd[i];
				bool bMatched = false;
				for(int j = 0; j < arActionsToRemove.GetSize() && !bMatched; j++) {
					EmrAction eaOld = arActionsToRemove[j];
					//(s.dhole 8/15/2014 9:46 AM ) - PLID 62724  for dignosis code we should check pair of ICD id's , Since gany session there wont be duplicate ICD code  pair
					if ((eaNew.eaoDestType != eaoDiagnosis && eaNew.eaoDestType == eaOld.eaoDestType && eaNew.nDestID == eaOld.nDestID)
						|| (eaNew.eaoDestType == eaoDiagnosis &&
						eaNew.diaDiagnosis.nDiagCodeID_ICD10 == eaOld.diaDiagnosis.nDiagCodeID_ICD10 &&
						eaNew.diaDiagnosis.nDiagCodeID_ICD9 == eaOld.diaDiagnosis.nDiagCodeID_ICD9))
					{
						//Match!  We don't need to add or remove.
						bMatched = true;
						arActionsToAdd.RemoveAt(i);
						arActionsToRemove.RemoveAt(j);
						//Do we need to update?
						// (c.haag 2008-07-18 09:11) - PLID 30775 - Use a utility function to detect whether
						// the actions are different
						if (!DoesEmrActionContentMatch(eaOld, eaNew)) {
							// (z.manning, 04/09/2007) - PLID 25542 - If we removed and then re-added the same action
							// then the action ID of the new one may be -1 and thus won't save anything. Let's fix that.
							if(eaNew.nID == -1) {
								eaNew.nID = eaOld.nID;
							}
							arActionsToUpdate.Add(eaNew);
						}
					}
				}
			}
			// We're going to do this in a sql statement batch rather than in separate statements
			CString strBatch = BeginSqlBatch();

			// (c.haag 2008-06-03 11:14) - PLID 30221 - We need to preserve the Action ID through multiple inserts
			AddStatementToSqlBatch(strBatch, "DECLARE @ActionID INT\r\n");

			for(i = 0; i < arActionsToAdd.GetSize(); i++)
			{
				const EmrAction& ea = arActionsToAdd[i];

				AddStatementToSqlBatch(strBatch, "INSERT INTO EmrActionsT (SourceID, SourceType, DestID, DestType, Popup, SortOrder, SpawnAsChild) "
					"VALUES (%li, %li, %li, %li, %li, %li, %i) \r\nSET @ActionID = @@identity", arActionsToAdd[i].nSourceID, arActionsToAdd[i].eaoSourceType, 
					arActionsToAdd[i].nDestID, arActionsToAdd[i].eaoDestType, arActionsToAdd[i].bPopup?1:0, arActionsToAdd[i].bSpawnAsChild?1:0, arActionsToAdd[i].nSortOrder);

				// (z.manning 2011-06-29 09:53) - PLID 44347 - Save anatomic location filter data
				// (a.walling 2014-08-06 12:40) - PLID 62692 - Laterality - Saving		
				SaveActionFilterToBatch(strBatch, ea, "@ActionID");

				//TODO: Handle aeiEmrActionAnatomicLocationFilter when we audit actions in this dialog.

				//DRT 1/18/2007 - PLID 24181 - We need to additionally insert data into the ChargeAction specific data table, only if
				//	it is a charge type action
				if(arActionsToAdd[i].eaoDestType == eaoCpt) {
					CString strMod1, strMod2, strMod3, strMod4;
					if(arActionsToAdd[i].strMod1.IsEmpty()) {	strMod1 = "NULL";	}	else {	strMod1.Format("'%s'", arActionsToAdd[i].strMod1);	}
					if(arActionsToAdd[i].strMod2.IsEmpty()) {	strMod2 = "NULL";	}	else {	strMod2.Format("'%s'", arActionsToAdd[i].strMod2);	}
					if(arActionsToAdd[i].strMod3.IsEmpty()) {	strMod3 = "NULL";	}	else {	strMod3.Format("'%s'", arActionsToAdd[i].strMod3);	}
					if(arActionsToAdd[i].strMod4.IsEmpty()) {	strMod4 = "NULL";	}	else {	strMod4.Format("'%s'", arActionsToAdd[i].strMod4);	}

					AddStatementToSqlBatch(strBatch, "INSERT INTO EmrActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, "
							"Modifier2Number, Modifier3Number, Modifier4Number) values (@ActionID, %li, %g, %s, %s, %s, %s);", arActionsToAdd[i].bPrompt, 
							arActionsToAdd[i].dblDefaultQuantity, strMod1, strMod2, strMod3, strMod4);
				}
				// (c.haag 2008-06-03 11:00) - PLID 30221 - Support for todo actions
				else if(arActionsToAdd[i].eaoDestType == eaoTodo) {

					AddStatementToSqlBatch(strBatch, "INSERT INTO EMRActionsTodoDataT "
						"(ActionID, RemindType, RemindInterval, DeadlineType, DeadlineInterval, "
						"Notes, Priority, Task, CategoryID) "
						"VALUES "
						"(@ActionID, %d, %d, %d, %d, "
						"'%s', %d, '%s', %s)",
						ea.nTodoRemindType, ea.nTodoRemindInterval,
						ea.nTodoDeadlineType, ea.nTodoDeadlineInterval,
						_Q(ea.strTodoNotes), ea.nTodoPriority, 	_Q(ea.strTodoMethod),
						(-1 == ea.nTodoCategoryID) ? "NULL" : AsString(ea.nTodoCategoryID));

					// (a.walling 2014-07-01 15:28) - PLID 62697
					for (long assignTo : ea.anTodoAssignTo) {
						AddStatementToSqlBatch(strBatch, "INSERT INTO EMRActionsTodoAssignToT (ActionID, AssignTo) "
							"VALUES (@ActionID, %d)", assignTo);
					}
				}
				// (b.savon 2014-07-16 09:20) - PLID 62707 - Handle saving the new Diagnosis DestType action to EmrActionsT and EmrActionsDiagnosisDataT
				else if (arActionsToAdd[i].eaoDestType == eaoDiagnosis){
					AddStatementToSqlBatch(
						strBatch,
						R"(
						INSERT INTO EMRActionDiagnosisDataT (EmrActionID, DiagCodeID_ICD9, DiagCodeID_ICD10)
						VALUES (@ActionID, %s, %s)
						)",
						ea.diaDiagnosis.nDiagCodeID_ICD9 == -1 ? "NULL" : FormatString("%li", ea.diaDiagnosis.nDiagCodeID_ICD9),
						ea.diaDiagnosis.nDiagCodeID_ICD10 == -1 ? "NULL" : FormatString("%li", ea.diaDiagnosis.nDiagCodeID_ICD10)
					);
				}

				// (c.haag 2008-08-04 12:39) - PLID 30941 - Save EMR problem spawning information
				// (a.walling 2014-07-01 15:28) - PLID 62697
				// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
				// (s.tullis 2015-02-24 11:31) - PLID 64724 
				// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
				for (const EmrProblemAction& epa : ea.aProblemActions) {
					AddStatementToSqlBatch(strBatch, "INSERT INTO EmrProblemActionsT (EmrActionID, DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) "
						"VALUES (@ActionID, '%s', %d, %d, %s, %li, %li)",
						_Q(epa.strDescription), epa.nStatus, epa.bSpawnToSourceItem, epa.GetSNOMEDValueForSQL(),epa.bDoNotShowOnCCDA, epa.bDoNotShowOnProblemPrompt);
				}
			}

			for(i = 0; i < arActionsToUpdate.GetSize(); i++)
			{
				// (c.haag 2008-08-04 16:30) - PLID 30941 - Get the current and existing actions
				EmrAction acCurrent = arActionsToUpdate.GetAt(i);
				EmrAction acOld;
				BOOL bFound = FALSE;
				for (int nOld=0; nOld < m_arActions.GetSize() && !bFound; nOld++) {
					if (m_arActions[nOld].nID == acCurrent.nID) {
						acOld = m_arActions[nOld];
						bFound = TRUE;
					}
				}
				if (!bFound) {				
					ASSERT(FALSE); // We should never get here
					ThrowNxException(FormatString("Could not find original action for saving changes! (ID = %li)",acCurrent.nID));
				}

				AddStatementToSqlBatch(strBatch, "UPDATE EmrActionsT SET Popup = %li, SpawnAsChild = %li, SortOrder = %li WHERE ID = %li",arActionsToUpdate[i].bPopup,
					arActionsToUpdate[i].bSpawnAsChild, arActionsToUpdate[i].nSortOrder, arActionsToUpdate[i].nID);

				// (z.manning 2011-06-29 09:53) - PLID 44347 - Save anatomic location filter data
				// (a.walling 2014-08-06 12:40) - PLID 62692 - Laterality - Saving
				AddStatementToSqlBatch(strBatch, "DELETE FROM EmrActionAnatomicLocationQualifiersT WHERE EmrActionID = %li", acCurrent.nID);
				AddStatementToSqlBatch(strBatch, "DELETE FROM EmrActionAnatomicLocationsT WHERE EmrActionID = %li", acCurrent.nID);

				SaveActionFilterToBatch(strBatch, acCurrent, AsString(acCurrent.nID));

				//DRT 1/18/2007 - PLID 24181 - When charge type actions are updated, we need to update the charge-specific data
				if(arActionsToUpdate[i].eaoDestType == eaoCpt) {
					CString strMod1, strMod2, strMod3, strMod4;
					if(arActionsToUpdate[i].strMod1.IsEmpty()) {	strMod1 = "NULL";	}	else {	strMod1.Format("'%s'", arActionsToUpdate[i].strMod1);	}
					if(arActionsToUpdate[i].strMod2.IsEmpty()) {	strMod2 = "NULL";	}	else {	strMod2.Format("'%s'", arActionsToUpdate[i].strMod2);	}
					if(arActionsToUpdate[i].strMod3.IsEmpty()) {	strMod3 = "NULL";	}	else {	strMod3.Format("'%s'", arActionsToUpdate[i].strMod3);	}
					if(arActionsToUpdate[i].strMod4.IsEmpty()) {	strMod4 = "NULL";	}	else {	strMod4.Format("'%s'", arActionsToUpdate[i].strMod4);	}

					AddStatementToSqlBatch(strBatch, "UPDATE EmrActionChargeDataT SET Prompt = %li, DefaultQuantity = %g, Modifier1Number = %s, "
						"Modifier2Number = %s, Modifier3Number = %s, Modifier4Number = %s WHERE ActionID = %li", arActionsToUpdate[i].bPrompt, 
						arActionsToUpdate[i].dblDefaultQuantity, strMod1, strMod2, strMod3, strMod4, arActionsToUpdate[i].nID);
				}
				// (c.haag 2008-06-04 11:50) - PLID 30221 - Update existing todo actions
				if(arActionsToUpdate[i].eaoDestType == eaoTodo) {
					EmrAction ea = arActionsToUpdate[i];

					// (b.savon 2012-07-05 09:12) - PLID 51347 - Only update the TodoData for the specified actionid
					AddStatementToSqlBatch(strBatch, "UPDATE EMRActionsTodoDataT SET RemindType = %d, RemindInterval = %d, DeadlineType = %d,"
						"DeadlineInterval = %d, Notes = '%s', Priority = %d, Task = '%s', CategoryID = %s WHERE ActionID = %d",
						ea.nTodoRemindType, ea.nTodoRemindInterval,
						ea.nTodoDeadlineType, ea.nTodoDeadlineInterval,
						_Q(ea.strTodoNotes), ea.nTodoPriority, 	_Q(ea.strTodoMethod),
						(-1 == ea.nTodoCategoryID) ? "NULL" : AsString(ea.nTodoCategoryID), ea.nID);
					

					AddStatementToSqlBatch(strBatch, "DELETE FROM EMRActionsTodoAssignToT WHERE ActionID = %d", arActionsToUpdate[i].nID);
					// (a.walling 2014-07-01 15:28) - PLID 62697
					for (long assignTo : ea.anTodoAssignTo) {
						AddStatementToSqlBatch(strBatch, "INSERT INTO EMRActionsTodoAssignToT (ActionID, AssignTo) "
							"VALUES (%d, %d)", arActionsToUpdate[i].nID, assignTo);
					}
				}

				// (b.savon 2014-07-22 13:03) - PLID 62707 - Update
				if (arActionsToUpdate[i].eaoDestType == eaoDiagnosis){
					EmrAction ea = arActionsToUpdate[i];

					AddStatementToSqlBatch(
						strBatch,
						R"(
						UPDATE EMRActionDiagnosisDataT SET DiagCodeID_ICD9 = %s, DiagCodeID_ICD10 = %s WHERE EmrActionID = %li
						)",
						ea.diaDiagnosis.nDiagCodeID_ICD9 == -1 ? "NULL" : FormatString("%li", ea.diaDiagnosis.nDiagCodeID_ICD9),
						ea.diaDiagnosis.nDiagCodeID_ICD10 == -1 ? "NULL" : FormatString("%li", ea.diaDiagnosis.nDiagCodeID_ICD10),
						ea.nID
						);
				}
				// (c.haag 2008-08-04 12:43) - PLID 30941 - Save EMR problem spawning information. It's not as simple as just clearing out
				// and re-adding the problems like I first expected. Unlike editing problem spawning information from the EMR item entry dialog,
				// we do not keep historic versions of data here. We'll instead need to do very precise data changes. In the case of deleted problems,
				// we'll need to inactivate them.
				CProblemActionAry aCreated, aDeleted, aModified;
				GetEmrActionProblemDiffs(acOld, acCurrent, aCreated, aDeleted, aModified);
				int iProb = 0;

				// (a.walling 2014-07-01 15:28) - PLID 62697
				// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
				// (s.tullis 2015-02-24 11:31) - PLID 64724 
				// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
				for (const EmrProblemAction& epa : aCreated) {
					AddStatementToSqlBatch(strBatch, "INSERT INTO EmrProblemActionsT (EmrActionID, DefaultDescription, DefaultStatus, SpawnToSourceItem, SNOMEDCodeID, DoNotShowOnCCDA, DoNotShowOnProblemPrompt) "
						"VALUES (%d, '%s', %d, %d, %s, %li, %li)",
						arActionsToUpdate[i].nID, _Q(epa.strDescription), epa.nStatus, epa.bSpawnToSourceItem, epa.GetSNOMEDValueForSQL(),epa.bDoNotShowOnCCDA, epa.bDoNotShowOnProblemPrompt);
				}
				for (const EmrProblemAction& epa : aDeleted) {
					AddStatementToSqlBatch(strBatch, "UPDATE EmrProblemActionsT SET Inactive = 1 WHERE ID = %d", epa.nID);
				}
				for (const EmrProblemAction& epa : aModified) {
					// (c.haag 2014-07-22) - PLID 62789 - Added SNOMEDCodeID
					// (s.tullis 2015-02-24 11:31) - PLID 64724 
					// (r.gonet 2015-03-10 14:48) - PLID 65013 - Added DoNotShowOnProblemPrompt.
					AddStatementToSqlBatch(strBatch, "UPDATE EmrProblemActionsT SET DefaultDescription = '%s', DefaultStatus = %d, SpawnToSourceItem = %d, SNOMEDCodeID = %s , DoNotShowOnCCDA = %li, DoNotShowOnProblemPrompt = %li "
						"WHERE ID = %d", _Q(epa.strDescription), epa.nStatus, epa.bSpawnToSourceItem, epa.GetSNOMEDValueForSQL(), epa.bDoNotShowOnCCDA, epa.bDoNotShowOnProblemPrompt, epa.nID);
				}
			}

			for(i = 0; i < arActionsToRemove.GetSize(); i++) {
				AddStatementToSqlBatch(strBatch, "UPDATE EmrActionsT SET Deleted = 1 WHERE ID = %li", arActionsToRemove[i].nID);
				//DRT 1/18/2007 - PLID 24181 - When removing actions, we leave EMRActionChargeDataT records intact where they are, they 
				//	can be seen as deleted from the existing EmrActionsT.Deleted flag.
				// (z.manning 2011-06-29 10:07) - PLID 44347 - Same with EmrActionAnatomicLocationsT
			}

			ExecuteSqlBatch(strBatch);			
		}
		
		//Now copy the current actions to our official list.
		m_arActions.RemoveAll();
		for(int i = 0; i < arCurrentActions.GetSize(); i++) {
			m_arActions.Add(arCurrentActions[i]);
		}

		// (c.haag 2008-07-17 10:47) - PLID 30723 - Delete any allocated problem arrays within the list rows
		DeleteProblemArrays();

		CNxDialog::OnOK();
	}NxCatchAll("Error in CEmrActionDlg::OnOK()");
}

void CEmrActionDlg::OnCancel() 
{
	try {
		// (c.haag 2008-07-17 10:47) - PLID 30723 - Delete any allocated problem arrays within the list rows
		DeleteProblemArrays();
		CNxDialog::OnCancel();
	}
	NxCatchAll("Error in CEmrActionDlg::OnCancel");
}

BEGIN_EVENTSINK_MAP(CEmrActionDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrActionDlg)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_CPT_COMBO, 16 /* SelChosen */, OnSelChosenActionCptCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_LAB_COMBO, 16 /* SelChosen */, OnSelChosenActionLabCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_CPT_LIST, 7 /* RButtonUp */, OnRButtonUpActionCptList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_LAB_LIST, 7 /* RButtonUp */, OnRButtonUpActionLabList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_COMBO, 16 /* SelChosen */, OnSelChosenActionItemCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_LIST, 7 /* RButtonUp */, OnRButtonUpActionItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_COMBO, 16 /* SelChosen */, OnSelChosenActionEMNTemplateCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_LIST, 7 /* RButtonUp */, OnRButtonUpActionEMNTemplateList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_ITEM_COMBO, 16 /* SelChosen */, OnSelChosenActionEMNTemplateItemCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_ITEM_LIST, 7 /* RButtonUp */, OnRButtonUpActionEMNTemplateItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_PRODUCT_COMBO, 16 /* SelChosen */, OnSelChosenActionProductCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_MEDICATION_LIST, 6 /* RButtonDown */, OnRButtonDownActionMedicationList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_PROCEDURE_COMBO, 16 /* SelChosen */, OnSelChosenActionProcedureCombo, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_PROCEDURE_LIST, 6 /* RButtonDown */, OnRButtonDownActionProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellActionItemList, VTS_I4 VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_CPT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionCptList, VTS_I2)
	
	ON_EVENT(CEmrActionDlg, IDC_ACTION_LAB_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionLabList, VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_MEDICATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionMedicationList, VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionItemList, VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_PROCEDURE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionProcedureList, VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_ITEM_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionEmnTemplateItemList, VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedActionEmnTemplateList, VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_ITEM_LIST, 2 /* SelChanged */, OnSelChangedActionEmnTemplateItemList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_LIST, 10 /* EditingFinished */, OnEditingFinishedActionItemList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_ITEM_LIST, 10 /* EditingFinished */, OnEditingFinishedActionEmnTemplateItemList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_CPT_LIST, 9 /* EditingFinishing */, OnEditingFinishingActionCptList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_LIST, 2 /* SelChanged */, OnSelChangedActionItemList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_CPT_LIST, 2 /* SelChanged */, OnSelChangedChargeList, VTS_I4)
	
	ON_EVENT(CEmrActionDlg, IDC_ACTION_LAB_LIST, 2 /* SelChanged */, OnSelChangedLabList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_MEDICATION_LIST, 2 /* SelChanged */, OnSelChangedMedicationList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_LIST, 2 /* SelChanged */, OnSelChangedEmnList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_PROCEDURE_LIST, 2 /* SelChanged */, OnSelChangedProcedureList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_TODO_LIST, 2 /* SelChanged */, OnSelChangedActionTodoList, VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_TODO_LIST, 7 /* RButtonUp */, OnRButtonUpActionTodoList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_TODO_LIST, 3 /* DblClickCell */, OnDblClickCellActionTodoList, VTS_I4 VTS_I2)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_CPT_LIST, 4 /* LButtonDown */, OnLButtonDownActionCptList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	
	ON_EVENT(CEmrActionDlg, IDC_ACTION_MEDICATION_LIST, 4 /* LButtonDown */, OnLButtonDownActionMedicationList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_ITEM_LIST, 4 /* LButtonDown */, OnLButtonDownActionEmnTemplateItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_ITEM_LIST, 4 /* LButtonDown */, OnLButtonDownActionItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_EMN_TEMPLATE_LIST, 4 /* LButtonDown */, OnLButtonDownActionEmnTemplateList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CEmrActionDlg, IDC_ACTION_DIAG_SEARCH, 16, CEmrActionDlg::SelChosenActionDiagSearch, VTS_DISPATCH)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_DIAG_LIST, 2, CEmrActionDlg::SelChangedActionDiagList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_DIAG_LIST, 4, CEmrActionDlg::LButtonDownActionDiagList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionDlg, IDC_ACTION_DIAG_LIST, 7, CEmrActionDlg::RButtonUpActionDiagList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	
	ON_EVENT(CEmrActionDlg, IDC_ACTION_DIAG_LIST, 10, CEmrActionDlg::EditingFinishedActionDiagList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrActionDlg, IDC_NXDL_ADD_MED_ACTION, 16, CEmrActionDlg::SelChosenNxdlAddMedAction, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEmrActionDlg::OnSelChosenActionCptCombo(long nRow) 
{
	if(nRow != -1 && m_pChargeList->FindByColumn(1, m_pCptCombo->GetValue(nRow,cdcID), 0, true) == -1) {
		IRowSettingsPtr pRow = m_pChargeList->GetRow(-1);
		pRow->PutValue(clcActionID, (long)-1);
		pRow->PutValue(clcID,m_pCptCombo->GetValue(nRow, cdcID));
		pRow->PutValue(clcName,m_pCptCombo->GetValue(nRow, cdcCode));
		//TES 5/12/2008 - PLID 29577 - Fill in the SubCode column.
		pRow->PutValue(clcSubCode, m_pCptCombo->GetValue(nRow, cdcSubCode));
		pRow->PutValue(clcDescription,m_pCptCombo->GetValue(nRow, cdcDescription));
		// (j.jones 2010-01-11 09:56) - PLID 24504 - added Price
		pRow->PutValue(clcPrice,m_pCptCombo->GetValue(nRow, cdcPrice));
		pRow->PutValue(clcSortOrder,(long)(m_pChargeList->GetRowCount() + 1));
		//DRT 1/10/2007 - PLID 24181 - Adding a new charge from the cpt list, just default all these to basic values
		pRow->PutValue(clcPrompt, g_cvarFalse);
		pRow->PutValue(clcDefQty, (double)1.0);
		pRow->PutValue(clcDefMod1, _bstr_t(""));
		pRow->PutValue(clcDefMod2, _bstr_t(""));
		pRow->PutValue(clcDefMod3, _bstr_t(""));
		pRow->PutValue(clcDefMod4, _bstr_t(""));
		pRow->PutValue(clcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
		pRow->PutValue(clcProblemArray, (long)(new CProblemActionAry));
		// (c.haag 2008-07-18 1:22) - PLID 30723 - Update the bitmap
		UpdateActionProblemRow(pRow, clcProblemBitmap, clcProblemArray);
		m_pChargeList->AddRow(pRow);
	}

	//now clear out the selection
	m_pCptCombo->CurSel = -1;

	// (z.manning, 05/31/2007) - PLID 24096 - Make sure we update the up and down arrows.
	OnSelChangedChargeList(m_pChargeList->CurSel);
}



// (z.manning 2008-10-01 14:35) - PLID 31556
void CEmrActionDlg::OnSelChosenActionLabCombo(long nRow)
{
	try
	{
		if(nRow != -1 && m_pLabList->FindByColumn(1, m_pLabCombo->GetValue(nRow,0), 0, VARIANT_TRUE) == -1)
		{
			IRowSettingsPtr pRow = m_pLabList->GetRow(-1);
			pRow->PutValue(alcActionID, (long)-1);
			pRow->PutValue(alcID, m_pLabCombo->GetValue(m_pLabCombo->GetCurSel(),0));
			pRow->PutValue(alcName, m_pLabCombo->GetValue(m_pLabCombo->GetCurSel(),1));
			pRow->PutValue(alcName2, "");
			pRow->PutValue(alcSortOrder, (long)(m_pLabList->GetRowCount() + 1));
			pRow->PutValue(alcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
			m_pLabList->AddRow(pRow);
		}

		//now clear out the selection
		m_pLabCombo->CurSel = -1;

		// Make sure we update the up and down arrows.
		OnSelChangedLabList(m_pLabList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnSelChosenActionLabCombo");
}

void CEmrActionDlg::OnRButtonUpActionCptList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlLastRightClickedDatalist = m_pChargeList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoCpt;
		if(nRow != -1) {
			m_pChargeList->CurSel = nRow;
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
			// consistency with other areas in the program
			mPopup.AppendMenu(MF_BYPOSITION, ID_CONFIGURE_CPT_PROBLEMS, "Configure &Problems...");
			AppendCommonRightClickMenuOptions(&mPopup);
			mPopup.AppendMenu(MF_SEPARATOR);
			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_CHARGE, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}	
	}
	NxCatchAll("Error in CEmrActionDlg::OnRButtonUpActionCptList");
}



// (z.manning 2008-10-01 14:40) - PLID 31556
void CEmrActionDlg::OnRButtonUpActionLabList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		m_pdlLastRightClickedDatalist = m_pLabList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoLab;
		if(nRow != -1)
		{
			m_pLabList->CurSel = nRow;
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			AppendCommonRightClickMenuOptions(&mPopup);
			if(mPopup.GetMenuItemCount() > 0) {
				mPopup.AppendMenu(MF_SEPARATOR);
			}
			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_LAB, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll("CEmrActionDlg::OnRButtonUpActionLabList");
}

void CEmrActionDlg::OnRemoveCpt()
{
	try
	{
		// (z.manning 2009-03-24 11:58) - PLID 33647 - Need to deallocate problem array
		DeleteProblemArrayByRow(m_pChargeList, m_pChargeList->GetCurSel(), clcProblemArray);
		m_pChargeList->RemoveRow(m_pChargeList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnRemoveCpt");
}

void CEmrActionDlg::OnRemoveDiag()
{
	try
	{
		//(s.dhole 7/30/2014 2:23 PM ) - PLID 62593
		DeleteDiagProblemArrayByRow( m_pDiagList->GetCurSel(), adcProblemArray);
		m_pDiagList->RemoveRow(m_pDiagList->CurSel);
		// (s.dhole 07\09\2014 8:56) - PLID 63114  Set dignosis column 
		UpdateDiagnosisListColumnSizes();
	}NxCatchAll("CEmrActionDlg::OnRemoveDiag");
}

// (z.manning 2008-10-01 14:41) - PLID 31556
void CEmrActionDlg::OnRemoveLab()
{
	try
	{
		m_pLabList->RemoveRow(m_pLabList->CurSel);
		OnSelChangedLabList(m_pLabList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnRemoveLab");
}

void CEmrActionDlg::OnSelChosenActionItemCombo(long nRow) 
{
	if(nRow != -1 && m_pItemList->FindByColumn(1, m_pItemCombo->GetValue(nRow,0), 0, true) == -1) {

		IRowSettingsPtr pRow = m_pItemList->GetRow(-1);
		pRow->PutValue(alcActionID, (long)-1);
		pRow->PutValue(alcID,m_pItemCombo->GetValue(m_pItemCombo->GetCurSel(),0));
		pRow->PutValue(alcName,m_pItemCombo->GetValue(m_pItemCombo->GetCurSel(),1));
		pRow->PutValue(alcName2, _bstr_t(""));
		pRow->PutValue(alcSortOrder,(long)(m_pItemList->GetRowCount() + 1));
		// (b.savon 2014-07-08 08:40) - PLID 62760 - EMR Item Editor > Action Window: Add a column in the item action window to display the type of item (i.e. slider).
		pRow->PutValue(alcDataType, m_pItemCombo->GetValue(m_pItemCombo->GetCurSel(), 4));
		pRow->PutValue(alcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
		_variant_t g_cvarFalse;
		g_cvarFalse.vt = VT_BOOL;
		g_cvarFalse.boolVal = false;
		pRow->PutValue(alcPopup, g_cvarFalse);
		BYTE nDataSubType = VarByte(m_pItemCombo->GetValue(m_pItemCombo->GetCurSel(),3));
		// (c.haag 2007-04-09 12:05) - PLID 25458 - Special coloring
		// for system EMR items
		if (nDataSubType == eistCurrentMedicationsTable ||
			nDataSubType == eistAllergiesTable)
		{
			pRow->ForeColor = RGB(0,0,255);
		}
		pRow->PutValue(alcProblemArray, (long)(new CProblemActionAry));
		// (c.haag 2008-07-18 1:22) - PLID 30723 - Update the bitmap
		UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
		m_pItemList->AddRow(pRow);
	}

	//now clear out the selection
	m_pItemCombo->CurSel = -1;

	// (z.manning, 05/31/2007) - PLID 24096 - Make sure we update the up and down arrows.
	OnSelChangedActionItemList(m_pItemList->CurSel);
}

void CEmrActionDlg::OnRButtonUpActionItemList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlLastRightClickedDatalist = m_pItemList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoEmrItem;

		// Set the selection to this row
		m_pItemList->CurSel = nRow;

		// Create the menu
		CMenu mPopup;
		mPopup.CreatePopupMenu();
		mPopup.AppendMenu(MF_BYCOMMAND|MF_STRING|MF_ENABLED, ID_NEW_ITEM, "&New...");
		mPopup.AppendMenu(MF_BYCOMMAND|MF_STRING|(nRow==sriNoRow ? MF_DISABLED|MF_GRAYED : MF_DEFAULT|MF_ENABLED), ID_EDIT_ITEM, "&Edit...");
		// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
		// consistency with other areas in the program
		mPopup.AppendMenu(MF_BYCOMMAND|MF_STRING|MF_ENABLED, ID_CONFIGURE_ITEM_PROBLEMS, "Configure &Problems...");
		if(nRow != sriNoRow) {
			AppendCommonRightClickMenuOptions(&mPopup);
		}
		mPopup.AppendMenu(MF_SEPARATOR);
		mPopup.AppendMenu(MF_BYCOMMAND|MF_STRING|(nRow==sriNoRow ? MF_DISABLED|MF_GRAYED : MF_ENABLED), ID_REMOVE_ITEM, "&Remove");
		mPopup.SetDefaultItem(ID_EDIT_ITEM, FALSE);

		CPoint pt;
		GetCursorPos(&pt);
		mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	} NxCatchAll("CEmrActionDlg::OnRButtonUpActionItemList");
}

void CEmrActionDlg::OnRemoveItem()
{
	try
	{
		// (z.manning 2009-03-24 11:58) - PLID 33647 - Need to deallocate problem array
		DeleteProblemArrayByRow(m_pItemList, m_pItemList->GetCurSel(), alcProblemArray);
		m_pItemList->RemoveRow(m_pItemList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnRemoveItem");
}

void CEmrActionDlg::OnSelChosenActionEMNTemplateCombo(long nRow) 
{
	if(nRow != -1 && m_pMintActionList->FindByColumn(1, m_pMintActionCombo->GetValue(nRow,0), 0, true) == -1) {
		//TES 11/6/2009 - PLID 35807 - Do they want to be warned if this is the NexWeb template?
		if(GetRemotePropertyInt("WarnWhenCreatingNexWebEmn", 1, 0, GetCurrentUserName())) {
			//TES 11/6/2009 - PLID 35807 - They do, so check whether it is.
			//(e.lally 2011-05-04) PLID 43537 - Changed to NexWebVisible bool
			BOOL bNexWebVisible = VarBool(m_pMintActionCombo->GetValue(nRow,3), FALSE);
			if(bNexWebVisible) {
				// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
				if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: This template is a NexWeb template.  It is designed "
					"to be created and filled out by your patients, through your website.  "
					"Are you sure you wish to configure this template to be spawned?")) {
					m_pMintActionCombo->CurSel = -1;
					return;
				}
			}
		}
		IRowSettingsPtr pRow = m_pMintActionList->GetRow(-1);
		pRow->PutValue(alcActionID, (long)-1);
		pRow->PutValue(alcID,m_pMintActionCombo->GetValue(m_pMintActionCombo->GetCurSel(),0));
		pRow->PutValue(alcName,m_pMintActionCombo->GetValue(m_pMintActionCombo->GetCurSel(),1));
		pRow->PutValue(alcName2,m_pMintActionCombo->GetValue(m_pMintActionCombo->GetCurSel(),2));
		pRow->PutValue(alcSortOrder,(long)(m_pMintActionList->GetRowCount() + 1));
		pRow->PutValue(alcProblemArray, (long)(new CProblemActionAry));
		pRow->PutValue(alcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
		// (c.haag 2008-07-18 1:22) - PLID 30723 - Update the bitmap
		UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
		m_pMintActionList->AddRow(pRow);
	}

	//now clear out the selection
	m_pMintActionCombo->CurSel = -1;

	// (z.manning, 05/31/2007) - PLID 24096 - Make sure we update the up and down arrows.
	OnSelChangedEmnList(m_pMintActionList->CurSel);
}

void CEmrActionDlg::OnRButtonUpActionEMNTemplateList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlLastRightClickedDatalist = m_pMintActionList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoMint;
		if(nRow != -1) {
			m_pMintActionList->CurSel = nRow;
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
			// consistency with other areas in the program
			mPopup.AppendMenu(MF_BYPOSITION, ID_CONFIGURE_MINT_PROBLEMS, "Configure &Problems...");
			AppendCommonRightClickMenuOptions(&mPopup);
			mPopup.AppendMenu(MF_SEPARATOR);
			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_MINT, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnRButtonUpActionEMNTemplateList");
}

void CEmrActionDlg::OnSelChosenActionEMNTemplateItemCombo(long nRow)
{
	if(nRow != -1 && m_pMintItemActionList->FindByColumn(1, m_pMintItemActionCombo->GetValue(nRow,0), 0, true) == -1) {
		IRowSettingsPtr pRow = m_pMintItemActionList->GetRow(-1);
		pRow->PutValue(alcActionID, (long)-1);
		pRow->PutValue(alcID,m_pMintItemActionCombo->GetValue(m_pMintItemActionCombo->GetCurSel(),0));
		pRow->PutValue(alcName,m_pMintItemActionCombo->GetValue(m_pMintItemActionCombo->GetCurSel(),1));
		pRow->PutValue(alcName2,m_pMintItemActionCombo->GetValue(m_pMintItemActionCombo->GetCurSel(),2));
		pRow->PutValue(alcSortOrder,(long)(m_pMintItemActionList->GetRowCount() + 1));
		pRow->PutValue(alcPopup, (long)0);
		pRow->PutValue(alcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
		pRow->PutValue(alcProblemArray, (long)(new CProblemActionAry));
		// (c.haag 2008-07-18 1:22) - PLID 30723 - Update the bitmap
		UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
		m_pMintItemActionList->AddRow(pRow);
	}

	//now clear out the selection
	m_pMintItemActionCombo->CurSel = -1;

	// (z.manning, 05/31/2007) - PLID 24096 - Make sure we update the up and down arrows.
	OnSelChangedActionEmnTemplateItemList(m_pMintItemActionList->CurSel);
}

void CEmrActionDlg::OnRButtonUpActionEMNTemplateItemList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlLastRightClickedDatalist = m_pMintItemActionList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoMintItems;
		if(nRow != -1) {
			m_pMintItemActionList->CurSel = nRow;
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			if (eaoEmrImageHotSpot != m_SourceType && m_SourceType != eaoEmrTableDropDownItem && m_SourceType != eaoSmartStamp) {
				// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
				// consistency with other areas in the program
				mPopup.AppendMenu(MF_BYPOSITION, ID_CONFIGURE_MINTITEM_PROBLEMS, "Configure &Problems...");
			}
			AppendCommonRightClickMenuOptions(&mPopup);
			if(mPopup.GetMenuItemCount() > 0) {
				mPopup.AppendMenu(MF_SEPARATOR);
			}

			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_MINT_ITEMS, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnRButtonUpActionEMNTemplateItemList");
}

void CEmrActionDlg::OnRemoveMint()
{
	try
	{
		// (z.manning 2009-03-24 11:58) - PLID 33647 - Need to deallocate problem array
		DeleteProblemArrayByRow(m_pMintActionList, m_pMintActionList->GetCurSel(), alcProblemArray);
		m_pMintActionList->RemoveRow(m_pMintActionList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnRemoveMint");
}

void CEmrActionDlg::OnRemoveMintItems()
{
	try
	{
		// (z.manning 2009-03-24 11:58) - PLID 33647 - Need to deallocate problem array
		DeleteProblemArrayByRow(m_pMintItemActionList, m_pMintItemActionList->GetCurSel(), alcProblemArray);
		m_pMintItemActionList->RemoveRow(m_pMintItemActionList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnRemoveMintItems");
}

void CEmrActionDlg::OnRadioServiceCode() 
{
	ChangeChargeType();
}

void CEmrActionDlg::OnRadioProduct() 
{
	ChangeChargeType();
}

void CEmrActionDlg::ChangeChargeType()
{
	if(IsDlgButtonChecked(IDC_RADIO_PRODUCT)) {
		GetDlgItem(IDC_ACTION_PRODUCT_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ACTION_CPT_COMBO)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_ACTION_CPT_COMBO)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ACTION_PRODUCT_COMBO)->ShowWindow(SW_HIDE);
	}
}

void CEmrActionDlg::OnSelChosenActionProductCombo(long nRow) 
{
	if(nRow != -1 && m_pChargeList->FindByColumn(1, m_pProductCombo->GetValue(nRow,pdcID), 0, true) == -1) {
		IRowSettingsPtr pRow = m_pChargeList->GetRow(-1);
		pRow->PutValue(clcActionID, (long)-1);
		pRow->PutValue(clcID,m_pProductCombo->GetValue(nRow,pdcID));
		pRow->PutValue(clcName,_bstr_t(""));
		//TES 5/12/2008 - PLID 29577 - Fill in the SubCode column.
		pRow->PutValue(clcSubCode, _bstr_t(""));
		pRow->PutValue(clcDescription,m_pProductCombo->GetValue(nRow,pdcName));
		// (j.jones 2010-01-11 09:56) - PLID 24504 - added Price
		pRow->PutValue(clcPrice,m_pProductCombo->GetValue(nRow, pdcPrice));
		pRow->PutValue(clcSortOrder,(long)(m_pChargeList->GetRowCount() + 1));
		//DRT 1/10/2007 - PLID 24181 - Adding a new charge, fill in some basic values for these
		pRow->PutValue(clcPrompt, g_cvarFalse);
		pRow->PutValue(clcDefQty, (double)1.0);
		pRow->PutValue(clcDefMod1, _bstr_t(""));
		pRow->PutValue(clcDefMod2, _bstr_t(""));
		pRow->PutValue(clcDefMod3, _bstr_t(""));
		pRow->PutValue(clcDefMod4, _bstr_t(""));
		pRow->PutValue(clcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
		// (c.haag 2008-07-18 1:22) - PLID 30723 - Update the bitmap
		pRow->PutValue(clcProblemArray, (long)(new CProblemActionAry));
		UpdateActionProblemRow(pRow, clcProblemBitmap, clcProblemArray);
		m_pChargeList->AddRow(pRow);
	}

	//now clear out the selection
	m_pProductCombo->CurSel = -1;

	// (z.manning, 05/31/2007) - PLID 24096 - Make sure we update the up and down arrows.
	OnSelChangedChargeList(m_pChargeList->CurSel);
}

void CEmrActionDlg::OnRButtonDownActionMedicationList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlLastRightClickedDatalist = m_pMedicationList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoMedication;
		if(nRow != -1) {
			m_pMedicationList->CurSel = nRow;
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
			// consistency with other areas in the program
			mPopup.AppendMenu(MF_BYPOSITION, ID_CONFIGURE_MED_PROBLEMS, "Configure &Problems...");
			AppendCommonRightClickMenuOptions(&mPopup);
			mPopup.AppendMenu(MF_SEPARATOR);
			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_MEDICATION, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnRButtonDownActionMedicationList");
}

/*
void CEmrActionDlg::OnSelChosenActionAllergyCombo(long nRow) 
{
	if(nRow != -1 && m_pAllergyList->FindByColumn(0, m_pAllergyCombo->GetValue(nRow,0), 0, true) == -1) {
		IRowSettingsPtr pRow = m_pAllergyList->GetRow(-1);
		pRow->PutValue(0,m_pAllergyCombo->GetValue(nRow,0));
		pRow->PutValue(1,m_pAllergyCombo->GetValue(nRow,1));
		pRow->PutValue(2,(long)(m_pAllergyList->GetRowCount() + 1));
		m_pAllergyList->AddRow(pRow);
	}

	//now clear out the selection
	m_pAllergyCombo->CurSel = -1;
}
*/

void CEmrActionDlg::OnSelChosenActionProcedureCombo(long nRow) 
{
	if(nRow != -1 && m_pProcedureList->FindByColumn(1, m_pProcedureCombo->GetValue(nRow,0), 0, true) == -1) {
		IRowSettingsPtr pRow = m_pProcedureList->GetRow(-1);
		pRow->PutValue(alcActionID, (long)-1);
		pRow->PutValue(alcID,m_pProcedureCombo->GetValue(nRow,0));
		pRow->PutValue(alcName,m_pProcedureCombo->GetValue(nRow,1));
		pRow->PutValue(alcName2, _bstr_t(""));
		pRow->PutValue(alcSortOrder,(long)(m_pProcedureList->GetRowCount() + 1));
		pRow->PutValue(alcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
		m_pProcedureList->AddRow(pRow);
	}

	//now clear out the selection
	m_pProcedureCombo->CurSel = -1;

	// (z.manning, 05/31/2007) - PLID 24096 - Make sure we update the up and down arrows.
	OnSelChangedProcedureList(m_pProcedureList->CurSel);
}

/*
void CEmrActionDlg::OnRButtonDownActionAllergyList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow != -1) {
		m_pAllergyList->CurSel = nRow;
		CMenu mPopup;
		mPopup.CreatePopupMenu();
		mPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_ALLERGY, "&Remove");

		CPoint pt;
		GetCursorPos(&pt);
		mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
}
*/

void CEmrActionDlg::OnRButtonDownActionProcedureList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		m_pdlLastRightClickedDatalist = m_pProcedureList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoProcedure;
		if(nRow != -1) {
			m_pProcedureList->CurSel = nRow;
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			AppendCommonRightClickMenuOptions(&mPopup);
			if(mPopup.GetMenuItemCount() > 0) {
				mPopup.AppendMenu(MF_SEPARATOR);
			}
			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_PROCEDURE, "&Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrActionDlg::OnRemoveMedication()
{
	try
	{
		// (z.manning 2009-03-24 11:58) - PLID 33647 - Need to deallocate problem array
		DeleteProblemArrayByRow(m_pMedicationList, m_pMedicationList->GetCurSel(), alcProblemArray);
		m_pMedicationList->RemoveRow(m_pMedicationList->CurSel);

	}NxCatchAll("CEmrActionDlg::OnRemoveMedication");
}

void CEmrActionDlg::OnRemoveProcedure()
{
	m_pProcedureList->RemoveRow(m_pProcedureList->CurSel);
}

void CEmrActionDlg::OnDblClickCellActionItemList(long nRowIndex, short nColIndex) 
{
	if (nRowIndex != sriNoRow) {
		OnEditItem();
	}
}

void CEmrActionDlg::OnEditItem()
{
	try {
		long nCurSel = m_pItemList->GetCurSel();
		if (nCurSel != sriNoRow) {
			// Let the user edit this item
			long nEmrInfoMasterID = VarLong(m_pItemList->GetValue(nCurSel, alcID));
			CEmrItemEntryDlg dlg(this);
			if(m_pCurrentEMN) {
				dlg.SetCurrentEMN(m_pCurrentEMN);
			}
			if (dlg.OpenWithMasterID(nEmrInfoMasterID) == IDOK) {
				// The user hit OK so we're done.  Well, we're done except that the user might have changed the 
				// name of the item, in which case we need to update our datalists.
				CString strName = dlg.GetName();
				
				// Check the list
				IRowSettingsPtr pListRow = m_pItemList->GetRow(nCurSel);
				if (VarString(pListRow->GetValue(alcName)) != strName) {
					pListRow->PutValue(alcName, (LPCTSTR)strName);
					m_pItemList->Sort();
				}
				// Check the combo
				long nComboRow = m_pItemCombo->FindByColumn(0, nEmrInfoMasterID, 0, VARIANT_FALSE);
				if (nComboRow != sriNoRow) {
					IRowSettingsPtr pRow = m_pItemCombo->GetRow(nComboRow);
					if (VarString(pRow->GetValue(1)) != strName) {
						pRow->PutValue(1, (LPCTSTR)strName);
						m_pItemCombo->Sort();
					}
				}
			}
		}
	} NxCatchAll("CEmrActionDlg::OnEditItem");
}

void CEmrActionDlg::OnNewItem()
{
	try {
		// Let the user create a new item
		CEmrItemEntryDlg dlg(this);
		if(m_pCurrentEMN) {
			dlg.SetCurrentEMN(m_pCurrentEMN);
		}
		if (dlg.OpenWithMasterID(-1) == IDOK) {
			// The user hit OK, so add the new item to both the combo AND the list (we're assuming the 
			// user wants to select the item he just created)
			
			// (b.cardillo 2005-07-15 08:46) - PLID 17026 - First add to the combo
			{
				IRowSettingsPtr pRow = m_pItemCombo->GetRow(sriGetNewRow);
				pRow->PutValue(0, dlg.GetInfoMasterID());
				pRow->PutValue(1, (LPCTSTR)dlg.GetName());
				pRow->PutValue(3, (BYTE)0); // (c.haag 2007-04-09 12:32) - PLID 25458 - Set the data sub type to 0.
											// It's impossible for users to create system EMR items with non-zero
											// sub types
				m_pItemCombo->AddRow(pRow);
			}

			// (b.cardillo 2005-07-15 08:46) - PLID 17026 - Then add to the list
			{
				IRowSettingsPtr pRow = m_pItemList->GetRow(sriGetNewRow);
				pRow->PutValue(alcActionID, (long)-1);
				pRow->PutValue(alcID, dlg.GetInfoMasterID());
				pRow->PutValue(alcName, (LPCTSTR)dlg.GetName());
				pRow->PutValue(alcName2, _bstr_t(""));
				pRow->PutValue(alcSortOrder, (long)(m_pItemList->GetRowCount() + 1));
				pRow->PutValue(alcPopup, g_cvarFalse);
				pRow->PutValue(alcLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
				// (c.haag 2008-07-18 1:22) - PLID 30723 - Update the bitmap
				pRow->PutValue(alcProblemArray, (long)(new CProblemActionAry));
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
				m_pItemList->AddRow(pRow);
			}
		}
	} NxCatchAll("CEmrActionDlg::OnNewItem");
}

void CEmrActionDlg::OnRequeryFinishedActionCptList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions.GetAt(i);
		switch (ea.eaoDestType) {
		case eaoCpt: {
			int nResult = m_pChargeList->FindByColumn(1, ea.nDestID, 0, FALSE);
			if(nResult != -1)
				m_pChargeList->PutValue(nResult,alcSortOrder,ea.nSortOrder);
			}
			break;
		}
	}

	m_pChargeList->Sort();
}


// (z.manning 2008-10-01 14:42) - PLID 31556
void CEmrActionDlg::OnRequeryFinishedActionLabList(short nFlags) 
{
	try
	{
		//configure the sort orders
		for (long i = 0; i < m_arActions.GetSize(); i++)
		{
			EmrAction ea = m_arActions.GetAt(i);
			switch (ea.eaoDestType)
			{
				case eaoLab: {
					int nResult = m_pLabList->FindByColumn(1, ea.nDestID, 0, FALSE);
					if(nResult != -1)
						m_pLabList->PutValue(nResult, alcSortOrder, ea.nSortOrder);
					}
					break;
			}
		}

		m_pLabList->Sort();

	}NxCatchAll("CEmrActionDlg::OnRequeryFinishedActionLabList");
}

void CEmrActionDlg::OnRequeryFinishedActionMedicationList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions.GetAt(i);
		switch (ea.eaoDestType) {
		case eaoMedication: {
			int nResult = m_pMedicationList->FindByColumn(1, ea.nDestID, 0, FALSE);
			if(nResult != -1)
				m_pMedicationList->PutValue(nResult,alcSortOrder, ea.nSortOrder);
			}
			break;
		}
	}

	m_pMedicationList->Sort();
}

void CEmrActionDlg::OnRequeryFinishedActionItemList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions.GetAt(i);
		switch (ea.eaoDestType) {
		case eaoEmrItem: {
			int nResult = m_pItemList->FindByColumn(1, ea.nDestID, 0, FALSE);
			if(nResult != -1) 
				m_pItemList->PutValue(nResult,alcSortOrder,ea.nSortOrder);
			}
			break;
		}
	}

	m_pItemList->Sort();
}

/*
void CEmrActionDlg::OnRequeryFinishedActionAllergyList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_aryActions.GetSize(); i++) {
		CEmrAction *pAction = m_aryActions.GetAt(i);
		switch (pAction->m_nDestType) {
		case eaoAllergy: {
			int nResult = m_pAllergyList->FindByColumn(0, pAction->m_nDestID, 0, FALSE);
			if(nResult != -1)
				m_pAllergyList->PutValue(nResult,2,pAction->m_nSortOrder);
			}
			break;
		}
	}

	m_pAllergyList->Sort();
}
*/

void CEmrActionDlg::OnRequeryFinishedActionProcedureList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions.GetAt(i);
		switch (ea.eaoDestType) {
		case eaoProcedure: {
			int nResult = m_pProcedureList->FindByColumn(1, ea.nDestID, 0, FALSE);
			if(nResult != -1)
				m_pProcedureList->PutValue(nResult,alcSortOrder,ea.nSortOrder);
			}
			break;
		}
	}

	m_pProcedureList->Sort();
}

void CEmrActionDlg::OnRequeryFinishedActionEmnTemplateItemList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions.GetAt(i);
		switch (ea.eaoDestType) {
		case eaoMintItems: {
			int nResult = m_pMintItemActionList->FindByColumn(1, ea.nDestID, 0, FALSE);
			if(nResult != -1)
				m_pMintItemActionList->PutValue(nResult,alcSortOrder,ea.nSortOrder);
			}
			break;
		}
	}

	m_pMintItemActionList->Sort();
}

void CEmrActionDlg::OnRequeryFinishedActionEmnTemplateList(short nFlags) 
{
	//configure the sort orders
	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions.GetAt(i);
		switch (ea.eaoDestType) {
		case eaoMint:{
			int nResult = m_pMintActionList->FindByColumn(1, ea.nDestID, 0, FALSE);
			if(nResult != -1)
				m_pMintActionList->PutValue(nResult,alcSortOrder,ea.nSortOrder);
			}
			break;
		}
	}

	m_pMintActionList->Sort();
}

void CEmrActionDlg::OnBtnEmrMintItemUp() 
{
	try {

		long nCurSel = m_pMintItemActionList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0)
			return;

		int nID = VarLong(m_pMintItemActionList->GetValue(nCurSel,1));

		m_pMintItemActionList->PutValue(nCurSel, alcSortOrder, (long)nCurSel);
		m_pMintItemActionList->PutValue(nCurSel-1, alcSortOrder, (long)(nCurSel+1));
		m_pMintItemActionList->Sort();

		OnSelChangedActionEmnTemplateItemList(m_pMintItemActionList->SetSelByColumn(1, (long)nID));

	}NxCatchAll("Error moving EMR template item up.");
}

void CEmrActionDlg::OnBtnEmrMintItemDown() 
{
	try {

		long nCurSel = m_pMintItemActionList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pMintItemActionList->GetRowCount() - 1)
			return;

		int nID = VarLong(m_pMintItemActionList->GetValue(nCurSel,1));

		m_pMintItemActionList->PutValue(nCurSel, alcSortOrder, (long)(nCurSel+1));
		m_pMintItemActionList->PutValue(nCurSel+1, alcSortOrder, (long)nCurSel);
		m_pMintItemActionList->Sort();

		OnSelChangedActionEmnTemplateItemList(m_pMintItemActionList->SetSelByColumn(1, (long)nID));

	}NxCatchAll("Error moving EMR template item up.");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected EMR item action up one spot.
void CEmrActionDlg::OnBtnEmrItemUp() 
{
	try {

		long nCurSel = m_pItemList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0) {
			return;
		}

		int nID = VarLong(m_pItemList->GetValue(nCurSel,alcID));

		m_pItemList->PutValue(nCurSel, alcSortOrder, (long)nCurSel);
		m_pItemList->PutValue(nCurSel-1, alcSortOrder, (long)(nCurSel+1));
		m_pItemList->Sort();

		OnSelChangedActionItemList(m_pItemList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnEmrItemUp");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected EMR item action down one spot.
void CEmrActionDlg::OnBtnEmrItemDown() 
{
	try {

		long nCurSel = m_pItemList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pItemList->GetRowCount() - 1) {
			return;
		}

		int nID = VarLong(m_pItemList->GetValue(nCurSel,alcID));

		m_pItemList->PutValue(nCurSel, alcSortOrder, (long)(nCurSel+1));
		m_pItemList->PutValue(nCurSel+1, alcSortOrder, (long)nCurSel);
		m_pItemList->Sort();

		OnSelChangedActionItemList(m_pItemList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnEmrItemDown");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected charge action up one spot.
void CEmrActionDlg::OnBtnChargeUp() 
{
	try {

		long nCurSel = m_pChargeList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0) {
			return;
		}

		int nID = VarLong(m_pChargeList->GetValue(nCurSel,clcID));

		m_pChargeList->PutValue(nCurSel, clcSortOrder, (long)nCurSel);
		m_pChargeList->PutValue(nCurSel-1, clcSortOrder, (long)(nCurSel+1));
		m_pChargeList->Sort();

		OnSelChangedChargeList(m_pChargeList->SetSelByColumn(clcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnChargeUp");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected charge action down one spot.
void CEmrActionDlg::OnBtnChargeDown() 
{
	try {

		long nCurSel = m_pChargeList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pChargeList->GetRowCount() - 1) {
			return;
		}

		int nID = VarLong(m_pChargeList->GetValue(nCurSel,clcID));

		m_pChargeList->PutValue(nCurSel, clcSortOrder, (long)(nCurSel+1));
		m_pChargeList->PutValue(nCurSel+1, clcSortOrder, (long)nCurSel);
		m_pChargeList->Sort();

		OnSelChangedChargeList(m_pChargeList->SetSelByColumn(clcID, (long)nID));

	}NxCatchAll("");
}

//(s.dhole 7/15/2014 8:08 AM ) - PLID 62593
void CEmrActionDlg::OnBtnDiagUp() 
{
	try {

		MoveUpOrDownDiagRow(TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected diag code action down one spot.
//(s.dhole 7/15/2014 8:09 AM ) - PLID 62593
//(s.dhole 7/17/2014 4:51 PM ) - PLID 62723
void CEmrActionDlg::OnBtnDiagDown() 
{
	try {
		MoveUpOrDownDiagRow(FALSE);
	}NxCatchAll(__FUNCTION__);
}


////(s.dhole 8/1/2014 8:03 AM ) - PPLID 62593 update up / down arrow
void CEmrActionDlg::MoveUpOrDownDiagRow(BOOL bIsMoveUp /*=TRUE*/)
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->CurSel;
	if (pRow == NULL) {
		//neither of these should be possible
		ASSERT(FALSE);
		UpdateDiagCodeArrows();
		return;
	}

	//swap orders
	NXDATALIST2Lib::IRowSettingsPtr pMovedRow = pRow;
	NXDATALIST2Lib::IRowSettingsPtr pSwappedRow;
	if (bIsMoveUp){
		pSwappedRow = pRow->GetPreviousRow();
	}
	else{
		pSwappedRow = pRow->GetNextRow();
	}

	long nMovedICD9ID = VarLong(pMovedRow->GetValue(adcICD9DiagCodeID), -1);
	long nMovedICD10ID = VarLong(pMovedRow->GetValue(adcICD10DiagCodeID), -1);
	long nMovedOrderIndex = VarLong(pMovedRow->GetValue(adcOrder));
	if (bIsMoveUp){
		nMovedOrderIndex--;
	}
	else{
		nMovedOrderIndex++;
	}

	pMovedRow->PutValue(adcOrder, nMovedOrderIndex);

	long nSwappedICD9ID = VarLong(pSwappedRow->GetValue(adcICD9DiagCodeID), -1);
	long nSwappedICD10ID = VarLong(pSwappedRow->GetValue(adcICD10DiagCodeID), -1);
	long nSwappedOrderIndex = VarLong(pSwappedRow->GetValue(adcOrder));
	if (bIsMoveUp){
		nSwappedOrderIndex++;
	}
	else{
		nSwappedOrderIndex--;
	}
	pSwappedRow->PutValue(adcOrder, nSwappedOrderIndex);
	m_pDiagList->Sort();
	UpdateDiagCodeArrows();
}

// (z.manning 2008-10-01 14:43) - PLID 31556
void CEmrActionDlg::OnBtnLabUp() 
{
	try
	{
		long nCurSel = m_pLabList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0) {
			return;
		}

		int nID = VarLong(m_pLabList->GetValue(nCurSel, alcID));

		m_pLabList->PutValue(nCurSel, alcSortOrder, (long)nCurSel);
		m_pLabList->PutValue(nCurSel-1, alcSortOrder, (long)(nCurSel+1));
		m_pLabList->Sort();

		OnSelChangedLabList(m_pLabList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnLabUp");
}

// (z.manning 2008-10-01 14:43) - PLID 31556
void CEmrActionDlg::OnBtnLabDown() 
{
	try {

		long nCurSel = m_pLabList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pLabList->GetRowCount() - 1) {
			return;
		}

		int nID = VarLong(m_pLabList->GetValue(nCurSel,alcID));

		m_pLabList->PutValue(nCurSel, alcSortOrder, (long)(nCurSel+1));
		m_pLabList->PutValue(nCurSel+1, alcSortOrder, (long)nCurSel);
		m_pLabList->Sort();

		OnSelChangedLabList(m_pLabList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnLabDown");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected medication action up one spot.
void CEmrActionDlg::OnBtnMedicationUp() 
{
	try {

		long nCurSel = m_pMedicationList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0) {
			return;
		}

		int nID = VarLong(m_pMedicationList->GetValue(nCurSel,alcID));

		m_pMedicationList->PutValue(nCurSel, alcSortOrder, (long)nCurSel);
		m_pMedicationList->PutValue(nCurSel-1, alcSortOrder, (long)(nCurSel+1));
		m_pMedicationList->Sort();

		OnSelChangedMedicationList(m_pMedicationList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnMedicationUp");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected medication action down one spot.
void CEmrActionDlg::OnBtnMedicationDown() 
{
	try {

		long nCurSel = m_pMedicationList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pMedicationList->GetRowCount() - 1) {
			return;
		}

		int nID = VarLong(m_pMedicationList->GetValue(nCurSel,alcID));

		m_pMedicationList->PutValue(nCurSel, alcSortOrder, (long)(nCurSel+1));
		m_pMedicationList->PutValue(nCurSel+1, alcSortOrder, (long)nCurSel);
		m_pMedicationList->Sort();

		OnSelChangedMedicationList(m_pMedicationList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnMedicationDown");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected EMN action up one spot.
void CEmrActionDlg::OnBtnEmnUp() 
{
	try {

		long nCurSel = m_pMintActionList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0) {
			return;
		}

		int nID = VarLong(m_pMintActionList->GetValue(nCurSel,alcID));

		m_pMintActionList->PutValue(nCurSel, alcSortOrder, (long)nCurSel);
		m_pMintActionList->PutValue(nCurSel-1, alcSortOrder, (long)(nCurSel+1));
		m_pMintActionList->Sort();

		OnSelChangedEmnList(m_pMintActionList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnEmnUp");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected EMN action down one spot.
void CEmrActionDlg::OnBtnEmnDown() 
{
	try {

		long nCurSel = m_pMintActionList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pMintActionList->GetRowCount() - 1) {
			return;
		}

		int nID = VarLong(m_pMintActionList->GetValue(nCurSel,alcID));

		m_pMintActionList->PutValue(nCurSel, alcSortOrder, (long)(nCurSel+1));
		m_pMintActionList->PutValue(nCurSel+1, alcSortOrder, (long)nCurSel);
		m_pMintActionList->Sort();

		OnSelChangedEmnList(m_pMintActionList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnEmnDown");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected procedure action up one spot.
void CEmrActionDlg::OnBtnProcedureUp() 
{
	try {

		long nCurSel = m_pProcedureList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0) {
			return;
		}

		int nID = VarLong(m_pProcedureList->GetValue(nCurSel,alcID));

		m_pProcedureList->PutValue(nCurSel, alcSortOrder, (long)nCurSel);
		m_pProcedureList->PutValue(nCurSel-1, alcSortOrder, (long)(nCurSel+1));
		m_pProcedureList->Sort();

		OnSelChangedProcedureList(m_pProcedureList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnProcedureUp");
}

// (z.manning, 05/31/2007) - PLID 24096 - Will move the currently selected procedure action down one spot.
void CEmrActionDlg::OnBtnProcedureDown() 
{
	try {

		long nCurSel = m_pProcedureList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pProcedureList->GetRowCount() - 1) {
			return;
		}

		int nID = VarLong(m_pProcedureList->GetValue(nCurSel,alcID));

		m_pProcedureList->PutValue(nCurSel, alcSortOrder, (long)(nCurSel+1));
		m_pProcedureList->PutValue(nCurSel+1, alcSortOrder, (long)nCurSel);
		m_pProcedureList->Sort();

		OnSelChangedProcedureList(m_pProcedureList->SetSelByColumn(alcID, (long)nID));

	}NxCatchAll("CEmrActionDlg::OnBtnProcedureDown");
}

void CEmrActionDlg::OnSelChangedActionEmnTemplateItemList(long nNewSel) 
{
	if(nNewSel == sriNoRow) {
		GetDlgItem(IDC_BTN_EMR_MINT_ITEM_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_EMR_MINT_ITEM_DOWN)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_EMR_MINT_ITEM_UP)->EnableWindow(nNewSel != 0);
		GetDlgItem(IDC_BTN_EMR_MINT_ITEM_DOWN)->EnableWindow(nNewSel != m_pMintItemActionList->GetRowCount() - 1);
	}
}

// (z.manning, 05/31/2007) - PLID 24096 - Update the up & down arrow buttons for the EMR item action list
// based on its current selection.
void CEmrActionDlg::OnSelChangedActionItemList(long nNewSel) 
{
	if(nNewSel == sriNoRow) {
		GetDlgItem(IDC_BTN_EMR_ITEM_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_EMR_ITEM_DOWN)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_EMR_ITEM_UP)->EnableWindow(nNewSel != 0);
		GetDlgItem(IDC_BTN_EMR_ITEM_DOWN)->EnableWindow(nNewSel != m_pItemList->GetRowCount() - 1);
	}
}

// (z.manning, 05/31/2007) - PLID 24096 - Update the up & down arrow buttons based on the the current selection.
void CEmrActionDlg::OnSelChangedChargeList(long nNewSel) 
{
	if(nNewSel == sriNoRow) {
		GetDlgItem(IDC_BTN_CHARGE_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CHARGE_DOWN)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_CHARGE_UP)->EnableWindow(nNewSel != 0);
		GetDlgItem(IDC_BTN_CHARGE_DOWN)->EnableWindow(nNewSel != m_pChargeList->GetRowCount() - 1);
	}
}


// (z.manning 2008-10-01 14:45) - PLID 31556
void CEmrActionDlg::OnSelChangedLabList(long nNewSel) 
{
	try
	{
		if(nNewSel == sriNoRow) {
			GetDlgItem(IDC_BTN_LAB_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_LAB_DOWN)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_BTN_LAB_UP)->EnableWindow(nNewSel != 0);
			GetDlgItem(IDC_BTN_LAB_DOWN)->EnableWindow(nNewSel != m_pLabList->GetRowCount() - 1);
		}

	}NxCatchAll("CEmrActionDlg::OnSelChangedLabList");
}

// (z.manning, 05/31/2007) - PLID 24096 - Update the up & down arrow buttons based on the the current selection.
void CEmrActionDlg::OnSelChangedMedicationList(long nNewSel) 
{
	if(nNewSel == sriNoRow) {
		GetDlgItem(IDC_BTN_MEDICATION_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MEDICATION_DOWN)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_MEDICATION_UP)->EnableWindow(nNewSel != 0);
		GetDlgItem(IDC_BTN_MEDICATION_DOWN)->EnableWindow(nNewSel != m_pMedicationList->GetRowCount() - 1);
	}
}

// (z.manning, 05/31/2007) - PLID 24096 - Update the up & down arrow buttons based on the the current selection.
void CEmrActionDlg::OnSelChangedEmnList(long nNewSel) 
{
	if(nNewSel == sriNoRow) {
		GetDlgItem(IDC_BTN_EMN_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_EMN_DOWN)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_EMN_UP)->EnableWindow(nNewSel != 0);
		GetDlgItem(IDC_BTN_EMN_DOWN)->EnableWindow(nNewSel != m_pMintActionList->GetRowCount() - 1);
	}
}

// (z.manning, 05/31/2007) - PLID 24096 - Update the up & down arrow buttons based on the the current selection.
void CEmrActionDlg::OnSelChangedProcedureList(long nNewSel) 
{
	if(nNewSel == sriNoRow) {
		GetDlgItem(IDC_BTN_PROCEDURE_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_PROCEDURE_DOWN)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_BTN_PROCEDURE_UP)->EnableWindow(nNewSel != 0);
		GetDlgItem(IDC_BTN_PROCEDURE_DOWN)->EnableWindow(nNewSel != m_pProcedureList->GetRowCount() - 1);
	}
}

void CEmrActionDlg::OnEditingFinishedActionItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	ASSERT(nCol == alcPopup);//The other columns aren't editable.

	//TES 4/6/2007 - PLID 25456 - We can now pop up all types of items.
	/*//This is not very efficient, but whatev.
	if(VarBool(varNewValue)) {
		int nType = VarByte(GetTableField("EmrInfoT INNER JOIN EmrInfoMasterT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID", "DataType", "EmrInfoMasterT.ID", VarLong(m_pItemList->GetValue(nRow,alcID))));
		if(nType == 6) {	//Narrative.
			MsgBox("This type of item cannot be popped up.");
			_variant_t g_cvarFalse;
			g_cvarFalse.vt = VT_BOOL;
			g_cvarFalse.boolVal = VARIANT_FALSE;
			m_pItemList->PutValue(nRow,alcPopup,g_cvarFalse);
			return;
		}
	}*/

	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions[i];
		switch (ea.eaoDestType) {
		case eaoEmrItem: 
			{
				if(VarLong(m_pItemList->GetValue(nRow,alcID)) == ea.nDestID) {
					ea.bPopup = VarBool(varNewValue);
				}
			}
			break;
		}
	}
	
}

// (b.cardillo 2005-06-30 13:08) - PLID 16691 - Handle the charge and diag code ellipsis buttons.

void CEmrActionDlg::OnChargeEditBtn()
{
	// Determine if we're in "service code" mode or "inventory item" mode
	if(IsDlgButtonChecked(IDC_RADIO_PRODUCT)) {
		// We're in "inventory item" mode, so add an inventory item
		try {
			// Make sure the user has permission to add products
			if (!CheckCurrentUserPermissions(bioInvItem, sptCreate)) {
				return;
			}

			// Now prompt to add the product
			CInvNew dlg;
			dlg.m_strWarnIfNotBillable = 
				"You have chosen not to make this inventory item billable.  If you proceed without this option "
				"the item will still be saved in inventory, but it will not be available for this EMR action.";
			long nNewID = dlg.DoModal(-1);
			if (nNewID != 0) {
				
				// To account for a flaw in CInvNew, we have to send the table checker 
				// message to notify everyone else the new product was added.
				{
					// Let everyone else know a new product was added
					CClient::RefreshTable(NetUtils::Products, nNewID);
				}

				// The user clicked OK, which means a new product exists in the system.  Add that 
				// new product to both the combo and the list, because we expect that in 99% of the 
				// cases the user wants to select the product he just made, otherwise why 
				// would he have chosen to make it right now.
				// But only add it if it was billable
				if (dlg.m_bFinalBillable) {
					long nProductRow = -1;
					// Add it to the product combo
					{
						IRowSettingsPtr pRow = m_pProductCombo->GetRow(-1);
						pRow->PutValue(pdcID, nNewID);//id
						pRow->PutValue(pdcCategory, "");//category						
						pRow->PutValue(pdcSubCode, "");//TES 5/12/2008 - PLID 29577 - Fill in the SubCode column.
						pRow->PutValue(pdcSupplier, "");//supplier
						pRow->PutValue(pdcName, (_bstr_t)dlg.m_strFinalName);//name
						pRow->PutValue(pdcPrice, _variant_t(COleCurrency(0,0)));//price
						pRow->PutValue(pdcLastCost, _variant_t(COleCurrency(0,0)));//last cost
						nProductRow = m_pProductCombo->AddRow(pRow);
					}
					// Add it to the charge list
					{
						//DRT 1/10/2007 - PLID 24181 - Instead of duplicating code, I'm just going to call the
						//	function which already performs adding a row to the list from the combo
						OnSelChosenActionProductCombo(nProductRow);
					}
				}
			}
		} NxCatchAll("CEmrActionDlg::OnChargeEditBtn:Product");
		
	} else {
		// We're in "service code" mode, so add a service code
		try {
			// Make sure the user has permission to add service codes
			if (!CheckCurrentUserPermissions(bioServiceCodes, sptCreate)) {
				return;
			}

			// Now prompt to add the cpt code
			CCPTAddNew dlg(this);
			if (dlg.DoModal() == IDOK) {
				
				// To account for a flaw in CCPTAddNew, we have to audit the creation ourselves and 
				// send the table checker message to notify everyone else the new code was added.
				{
					// Audit the creation
					try{
						long nAuditID = BeginNewAuditEvent();
						if (nAuditID != -1) {
							AuditEvent(-1, "", nAuditID, aeiCPTCreate, -1, "", dlg.strCode, aepMedium, aetCreated);
						} else {
							ThrowNxException("Could not begin new audit event!");
						}
					} NxCatchAll("CEmrActionDlg::OnChargeEditBtn:Service: Audit failed!");		

					// Let everyone else know a new cpt was added
					CClient::RefreshTable(NetUtils::CPTCodeT, dlg.m_nID);
				}

				// The user clicked OK, which means a new CPT exists in the system.  Add that 
				// new CPT to both the combo and the list, because we expect that in 99% of the 
				// cases the user wants to select the CPT code he just made, otherwise why 
				// would he have chosen to make it right now.
				{
					// Add it to the service combo
					long nCPTRow = -1;
					{
						IRowSettingsPtr pRow = m_pCptCombo->GetRow(-1);
						pRow->PutValue(cdcID, (long)dlg.m_nID);
						pRow->PutValue(cdcCode, (_bstr_t)dlg.strCode);
						//TES 5/12/2008 - PLID 29577 - Fill in the SubCode column.
						pRow->PutValue(cdcSubCode, (_bstr_t)dlg.strSubCode);
						pRow->PutValue(cdcDescription, (_bstr_t)dlg.strName);
						// (j.jones 2010-01-11 09:00) - PLID 24504 - added price
						pRow->PutValue(cdcPrice, _variant_t(dlg.m_cyPrice));
						nCPTRow = m_pCptCombo->AddRow(pRow);
					}
					// Add it to the charge list
					{
						//DRT 1/10/2007 - PLID 24181 - Instead of duplicating code, I'm just going to call the
						//	function which already performs adding a row to the list from the combo
						OnSelChosenActionCptCombo(nCPTRow);
					}
				}
			}
		} NxCatchAll("CEmrActionDlg::OnChargeEditBtn:Service");
	}
}


void CEmrActionDlg::SetCurrentEMN(CEMN* pEMN)
{
	m_pCurrentEMN = pEMN;
}

void CEmrActionDlg::OnEditingFinishedActionEmnTemplateItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	ASSERT(nCol == alcPopup);//The other columns aren't editable.

	for (long i=0; i<m_arActions.GetSize(); i++) {
		EmrAction ea = m_arActions[i];
		switch (ea.eaoDestType) {
		case eaoMintItems:
			{
				if(VarLong(m_pMintItemActionList->GetValue(nRow,alcID)) == ea.nDestID) {
					ea.bSpawnAsChild = VarLong(varNewValue)?true:false;
				}
			}
			break;
		}
	}
}

void CEmrActionDlg::OnEditingFinishingActionCptList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		//If already cancelling, don't need to do any checking
		if(*pbCommit == FALSE)
			return;

		switch(nCol) {
		case clcDefQty:
			//Ensure that the quantity is > 0
			double dbl = VarDouble(pvarNewValue, 0.0);
			if(LooseCompareDouble(dbl, 0.0, 0.001) <= 0) {
				//0 or negative
				MessageBox("You must enter a positive quantity.", "NexTech", MB_OK);
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
			break;
		}

	} NxCatchAll("Error in OnEditingFinishingActionCptList");
}

void CEmrActionDlg::OnBtnNewTodoTask() 
{
	// The following function itself is a message handler; no need for exception handling here
	OnNewTodo();
}

void CEmrActionDlg::OnSelChangedActionTodoList(long nNewSel) 
{
	try {
		// (c.haag 2008-05-30 12:37) - PLID 30221 - Handle selection changes
		// for the todo alarm list
		if(nNewSel == sriNoRow) {
			GetDlgItem(IDC_BTN_TODO_UP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_TODO_DOWN)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_BTN_TODO_UP)->EnableWindow(nNewSel != 0);
			GetDlgItem(IDC_BTN_TODO_DOWN)->EnableWindow(nNewSel != m_pTodoList->GetRowCount() - 1);
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnSelChangedActionTodoList");	
}

void CEmrActionDlg::OnRButtonUpActionTodoList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_pdlLastRightClickedDatalist = m_pTodoList;
		//(s.dhole 7/29/2014 8:27 AM ) - PLID 63083 Set null value
		m_pdlLastRightClickedDatalistDiag = NULL;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		m_LastRightClickedAction = eaoTodo;
		m_pTodoList->CurSel = nRow;
		CMenu mPopup;
		mPopup.CreatePopupMenu();
		mPopup.AppendMenu(MF_BYPOSITION, ID_NEW_TODO, "&Add...");
		if (nRow >= 0) {
			AppendCommonRightClickMenuOptions(&mPopup);
			mPopup.AppendMenu(MF_SEPARATOR);
			mPopup.AppendMenu(MF_BYPOSITION|MF_DEFAULT, ID_EDIT_TODO, "&Edit");
			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_TODO, "&Remove");
		}
		CPoint pt;
		GetCursorPos(&pt);
		mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
	}
	NxCatchAll("Error in CEmrActionDlg::OnRButtonUpActionTodoList");
}

void CEmrActionDlg::UpdateTodoRow(IRowSettingsPtr& pRow, CEMRTodoActionConfigDlg& dlg)
{
	// (c.haag 2008-06-03 09:58) - PLID 30221 - Update the contents of a row in the
	// todo list
	pRow->Value[ectlCategoryID] = dlg.m_nCommit_Category;
	pRow->Value[ectlMethod] = _bstr_t(dlg.m_strCommit_Method);
	pRow->Value[ectlNotes] = _bstr_t(dlg.m_strCommit_Notes);
	pRow->Value[ectlAssignTo] = _bstr_t(ArrayAsString(dlg.m_anCommit_AssignTo, false));
	pRow->Value[ectlPriority] = dlg.m_nCommit_Priority;
	pRow->Value[ectlRemindType] = dlg.m_nCommit_RemindType;
	pRow->Value[ectlRemindInterval] = dlg.m_nCommit_RemindInterval;
	pRow->Value[ectlDeadlineType] = dlg.m_nCommit_DeadlineType;
	pRow->Value[ectlDeadlineInterval] = dlg.m_nCommit_DeadlineInterval;
}

void CEmrActionDlg::OnNewTodo()
{
	// (c.haag 2008-05-30 12:37) - PLID 30221 - Create a new todo alarm action
	try {
		CEMRTodoActionConfigDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			// Add the todo to the action list
			IRowSettingsPtr pRow = m_pTodoList->GetRow(-1);
			pRow->PutValue(ectlActionID, (long)-1);
			pRow->PutValue(ectlSortOrder,(long)(m_pTodoList->GetRowCount() + 1));
			pRow->PutValue(ectlLabAnatomyIDs, ""); // (z.manning 2011-06-28 16:20) - PLID 44347
			UpdateTodoRow(pRow, dlg);
			m_pTodoList->AddRow(pRow);
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnNewTodo");
}

void CEmrActionDlg::OnEditTodo()
{
	// (c.haag 2008-06-03 09:51) - PLID 30221 - Edit a todo alarm
	try {
		CEMRTodoActionConfigDlg dlg(this);
		IRowSettingsPtr pRow = m_pTodoList->GetRow(m_pTodoList->CurSel);
		if (NULL != pRow) {
			dlg.m_bIsNew = FALSE;
			dlg.m_nCommit_Category = VarLong(pRow->Value[ectlCategoryID], -1);
			dlg.m_strCommit_Method = VarString(pRow->Value[ectlMethod]);
			dlg.m_strCommit_Notes = VarString(pRow->Value[ectlNotes]);
			ParseDelimitedStringToLongArray(VarString(pRow->Value[ectlAssignTo]), ",", dlg.m_anCommit_AssignTo);
			dlg.m_nCommit_Priority = VarLong(pRow->Value[ectlPriority]);
			dlg.m_nCommit_RemindType = VarLong(pRow->Value[ectlRemindType]);
			dlg.m_nCommit_RemindInterval = VarLong(pRow->Value[ectlRemindInterval]);
			dlg.m_nCommit_DeadlineType = VarLong(pRow->Value[ectlDeadlineType]);
			dlg.m_nCommit_DeadlineInterval = VarLong(pRow->Value[ectlDeadlineInterval]);
			if (IDOK == dlg.DoModal()) {
				UpdateTodoRow(pRow, dlg);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnEditTodo");
}

void CEmrActionDlg::OnRemoveTodo()
{
	// (c.haag 2008-06-03 09:59) - PLID 30221 - Removes a todo row
	try {
		m_pTodoList->RemoveRow(m_pTodoList->CurSel);
	}
	NxCatchAll("Error in CEmrActionDlg::OnRemoveTodo");
}

void CEmrActionDlg::OnDblClickCellActionTodoList(long nRowIndex, short nColIndex) 
{
	// (c.haag 2008-06-03 10:30) - PLID 30221 - Edits a todo row through a double-click
	try {
		if (nRowIndex > -1) {
			m_pTodoList->CurSel = nRowIndex;
			OnEditTodo();
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnDblClickCellActionTodoList");
}

void CEmrActionDlg::OnBtnTodoUp() 
{
	// (c.haag 2008-06-03 10:44) - PLID 30221 - Move a todo item up
	try {
		long nCurSel = m_pTodoList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == 0)
			return;

		int nID = VarLong(m_pTodoList->GetValue(nCurSel,1));

		m_pTodoList->PutValue(nCurSel, ectlSortOrder, (long)nCurSel);
		m_pTodoList->PutValue(nCurSel-1, ectlSortOrder, (long)(nCurSel+1));
		m_pTodoList->Sort();

		OnSelChangedActionTodoList(m_pTodoList->SetSelByColumn(1, (long)nID));
	}
	NxCatchAll("Error in CEmrActionDlg::OnBtnTodoUp");
	
}

void CEmrActionDlg::OnBtnTodoDown() 
{
	// (c.haag 2008-06-03 10:44) - PLID 30221 - Move a todo item down
	try {
		long nCurSel = m_pTodoList->CurSel;

		if(nCurSel == sriNoRow || nCurSel == m_pTodoList->GetRowCount() - 1)
			return;

		int nID = VarLong(m_pTodoList->GetValue(nCurSel,1));

		m_pTodoList->PutValue(nCurSel, ectlSortOrder, (long)(nCurSel+1));
		m_pTodoList->PutValue(nCurSel+1, ectlSortOrder, (long)nCurSel);
		m_pTodoList->Sort();

		OnSelChangedActionTodoList(m_pTodoList->SetSelByColumn(1, (long)nID));
	}
	NxCatchAll("Error in CEmrActionDlg::OnBtnTodoDown");
}

// (c.haag 2008-07-17 10:39) - PLID 30723 - Update the appearance of the problem flag column in a given row
void CEmrActionDlg::UpdateActionProblemRow(IRowSettingsPtr& pRow, short nColProblemBitmap, short nColProblemAry)
{
	CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(nColProblemAry));
	pRow->PutValue(nColProblemBitmap, (long)((!pary->empty()) ? theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG) : theApp.LoadIcon(IDI_EMR_PROBLEM_EMPTY)));
}

//(s.dhole 7/15/2014 8:10 AM ) - PLID 62593 Update the appearance of the problem flag column 
void CEmrActionDlg::UpdateDiagActionProblemRow(NXDATALIST2Lib::IRowSettingsPtr& pRow, short nColProblemBitmap, short nColProblemAry)
{
	CProblemActionAry* pary = (CProblemActionAry*)VarLong(pRow->GetValue(nColProblemAry));

	pRow->PutValue(nColProblemBitmap, (long)((!pary->empty()) ? 
		(HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16, 16, 0) 
		: (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_EMPTY), IMAGE_ICON, 16, 16, 0)));
	
}


// (c.haag 2008-07-17 10:43) - PLID 30723 - Deletes all arrays of problems embedded in all lists
void CEmrActionDlg::DeleteProblemArrays()
{
	DeleteProblemArrays(m_pChargeList, clcProblemArray);
	//(s.dhole 7/18/2014 11:21 AM ) - PLID 62593
	DeleteDiagProblemArrays(m_pDiagList, adcProblemArray);
	DeleteProblemArrays(m_pItemList, alcProblemArray);
	DeleteProblemArrays(m_pMintActionList, alcProblemArray);
	DeleteProblemArrays(m_pMintItemActionList, alcProblemArray);
	DeleteProblemArrays(m_pMedicationList, alcProblemArray);
}

// (c.haag 2008-07-17 10:43) - PLID 30723 - Deletes all arrays of problems embedded in a specified list
void CEmrActionDlg::DeleteProblemArrays(_DNxDataListPtr& dl, short nColProblemAry)
{
	const long nRows = dl->GetRowCount();
	for (long i=0; i < nRows; i++) {
		DeleteProblemArrayByRow(dl, i, nColProblemAry);
	}
}

//(s.dhole 7/18/2014 11:21 AM ) - PLID 62593 Delete all array of problems embedded in a list
void CEmrActionDlg::DeleteDiagProblemArrays(NXDATALIST2Lib::_DNxDataListPtr& dl, short nColProblemAry)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = dl->GetFirstRow();
	while (pRow)
	{
		DeleteDiagProblemArrayByRow(pRow, nColProblemAry);
		pRow = pRow->GetNextRow();
	}
}


// (z.manning 2009-03-24 11:48) - PLID 33647
void CEmrActionDlg::DeleteProblemArrayByRow(_DNxDataListPtr& dl, const long nRow, const short nColProblemAry)
{
	CProblemActionAry* pary	= (CProblemActionAry*)VarLong(dl->GetValue(nRow, nColProblemAry));
	if(pary != NULL) {
		delete pary;
		dl->PutValue(nRow, nColProblemAry, NULL);
	}
}

//(s.dhole 7/18/2014 11:21 AM ) - PLID 62593
void CEmrActionDlg::DeleteDiagProblemArrayByRow(NXDATALIST2Lib::IRowSettingsPtr pRow, const short nColProblemAry)
{
	CProblemActionAry* pary = (CProblemActionAry*)VarLong(pRow->GetValue(nColProblemAry));
	if (pary != NULL) {
		delete pary;
		pRow->PutValue(nColProblemAry, NULL);
	}
}
//(s.dhole 7/18/2014 11:16 AM ) - PLID 62593 Support Datalist2
void CEmrActionDlg::OnLButtonDownActionCptList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-07-16 17:41) - PLID 30723 - Edit the problem spawning list for this action
	try {
		if (nRow > -1 && clcProblemBitmap == nCol) {
			IRowSettingsPtr pRow = m_pChargeList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(clcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoCpt, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, clcProblemBitmap, clcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnLButtonDownActionCptList");
}



void CEmrActionDlg::OnLButtonDownActionMedicationList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-07-22 10:30) - PLID 30723 - Edit the problem spawning list for this action
	if (nRow > -1 && alcProblemBitmap == nCol) {
		IRowSettingsPtr pRow = m_pMedicationList->GetRow(nRow);
		CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
		CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoMedication, this);
		if (IDOK == dlg.DoModal()) {
			// Update the row appearance. The dialog will have already updated pary.
			UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
		}
	}
}

void CEmrActionDlg::OnLButtonDownActionEmnTemplateItemList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-07-16 17:41) - PLID 30723 - Edit the problem spawning list for this action
	try {
		if (nRow > -1 && alcProblemBitmap == nCol) {
			IRowSettingsPtr pRow = m_pMintItemActionList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoMintItems, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnLButtonDownActionEmnTemplateItemList");
}

void CEmrActionDlg::OnLButtonDownActionItemList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-07-16 17:41) - PLID 30723 - Edit the problem spawning list for this action
	try {
		if (nRow > -1 && alcProblemBitmap == nCol) {
			IRowSettingsPtr pRow = m_pItemList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoEmrItem, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnLButtonDownActionItemList");
}

void CEmrActionDlg::OnLButtonDownActionEmnTemplateList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2008-07-16 17:41) - PLID 30723 - Edit the problem spawning list for this action
	try {
		if (nRow > -1 && alcProblemBitmap == nCol) {
			IRowSettingsPtr pRow = m_pMintActionList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoMint, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnLButtonDownActionEmnTemplateList");
}

// (c.haag 2008-07-18 09:56) - PLID 30723 - Pulls problem action data from pRow and stores it in ea
void CEmrActionDlg::StoreProblemActions(IRowSettingsPtr& pRow, short nColProblemAry, OUT EmrAction& ea)
{
	if (VT_I4 == pRow->GetValue(nColProblemAry).vt) {
		// If we get here, this row supports problem actions. Populate the array.
		CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(nColProblemAry));
		ea.aProblemActions = *pary; // (a.walling 2014-07-01 15:28) - PLID 62697
	} else {
		// If we get here, this row does not support problem actions. Do nothing.
	}
}


//(s.dhole 7/15/2014 8:11 AM ) - PLID  62593 - Pulls problem action data from pRow and stores it in ea
void CEmrActionDlg::StoreProblemActions(NXDATALIST2Lib::IRowSettingsPtr& pRow, short nColProblemAry, OUT EmrAction& ea)
{
	if (VT_I4 == pRow->GetValue(nColProblemAry).vt) {
		// If we get here, this row supports problem actions. Populate the array.
		CProblemActionAry* pary = (CProblemActionAry*)VarLong(pRow->GetValue(nColProblemAry));
		ea.aProblemActions = *pary;
	}
	else {
		// If we get here, this row does not support problem actions. Do nothing.
	}
}

void CEmrActionDlg::OnConfigureCptProblems()
{
	// (c.haag 2008-07-24 10:45) - PLID 30723 - Configure service code problems
	try {
		long nRow = m_pChargeList->CurSel;
		if(nRow!= -1) {
			IRowSettingsPtr pRow = m_pChargeList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(clcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoCpt, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, clcProblemBitmap, clcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnConfigureCptProblems");
}

//(s.dhole 7/14/2014 2:08 PM ) - PLID 62593
void CEmrActionDlg::OnConfigureDiagProblems()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->CurSel;
		if (pRow) {
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(adcProblemArray));
			// (b.savon 2014-07-15 15:50) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoDiagnosis/*eaoDiag*/, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateDiagActionProblemRow(pRow, adcProblemBitmap, adcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnConfigureDiagProblems");
}

void CEmrActionDlg::OnConfigureMedProblems()
{
	// (c.haag 2008-07-24 10:45) - PLID 30723 - Configure Medication problems
	try {
		long nRow = m_pMedicationList->CurSel;
		if(nRow!= -1) {
			IRowSettingsPtr pRow = m_pMedicationList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoMedication, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnConfigureMedProblems");
}

void CEmrActionDlg::OnConfigureItemProblems()
{
	// (c.haag 2008-07-24 10:45) - PLID 30723 - Configure EMR item problems
	try {
		long nRow = m_pItemList->CurSel;
		if(nRow!= -1) {
			IRowSettingsPtr pRow = m_pItemList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoEmrItem, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnConfigureItemProblems");
}

void CEmrActionDlg::OnConfigureMintProblems()
{
	// (c.haag 2008-07-24 10:45) - PLID 30723 - Configure EMR item problems
	try {
		long nRow = m_pMintActionList->CurSel;
		if(nRow!= -1) {
			IRowSettingsPtr pRow = m_pMintActionList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoMint, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnConfigureMintProblems");
}

void CEmrActionDlg::OnConfigureMintItemProblems()
{
	// (c.haag 2008-07-24 10:45) - PLID 30723 - Configure EMR item problems
	try {
		long nRow = m_pMintItemActionList->CurSel;
		if(nRow!= -1) {
			IRowSettingsPtr pRow = m_pMintItemActionList->GetRow(nRow);
			CProblemActionAry* pary	= (CProblemActionAry*)VarLong(pRow->GetValue(alcProblemArray));
			CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoMintItems, this);
			if (IDOK == dlg.DoModal()) {
				// Update the row appearance. The dialog will have already updated pary.
				UpdateActionProblemRow(pRow, alcProblemBitmap, alcProblemArray);
			}
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnConfigureMintItemProblems");
}

// (z.manning 2011-06-28 09:59) - PLID 44347
BOOL CEmrActionDlg::SourceTypeAllowsPerAnatomicLocation()
{
	switch(m_SourceType)
	{
		// (j.jones 2012-08-24 15:04) - PLID 52091 - if any other types are added here,
		// you must change the queries in CEmrActionDlg::OnCopyActionAnatomicLocations
		// to account for that type, and potentially some messageboxes

		case eaoSmartStamp:
			return TRUE;
			break;
	}

	return FALSE;
}

// (z.manning 2011-06-28 09:59) - PLID 44347
void CEmrActionDlg::OnEditActionAnatomicLocations()
{
	try
	{

		//(s.dhole 7/29/2014 8:22 AM ) - PLID 63083 check both pointers
		if (m_pdlLastRightClickedDatalist == NULL && m_pdlLastRightClickedDatalistDiag == NULL) {
			return;
		}
		//(s.dhole 7/29/2014 8:22 AM ) - PLID 63083 reset Lastselected Datalist2
		if (m_pdlLastRightClickedDatalistDiag)
		{
			OnEditDiagActionAnatomicLocations();
			//set null value
			
			m_pdlLastRightClickedDatalistDiag = NULL;
		}
		else
		{

			long nRow = m_pdlLastRightClickedDatalist->GetCurSel();
			if (nRow == -1) {
				return;
			}
			IRowSettingsPtr pRow = m_pdlLastRightClickedDatalist->GetRow(nRow);
			if (pRow == NULL) {
				return;
			}

			short nCol = GetAnatomicLocationColumnFromDatalist(m_pdlLastRightClickedDatalist);
			if (nCol == -1) {
				return;
			}

			// (z.manning 2011-06-29 08:43) - PLID 44347 - Pop up a multi select dialog with all active anatomic
			// locations and allow the user to select the ones that should be associated with this action.
			CString strLabAnatomyFilter = VarString(pRow->GetValue(nCol), "");

			// (a.walling 2014-08-06 16:35) - PLID 62687 - Launch new CEmrActionFilterAnatomyDlg with Emr::ActionFilter
			CEmrActionFilterAnatomyDlg dlg(this, Emr::ActionFilter::FromXml(strLabAnatomyFilter));

			int nResult = dlg.DoModal();
			if (nResult == IDOK)
			{
				pRow->PutValue(nCol, dlg.m_filter.ToXml().c_str());
				UpdateRowForAnatomyChange(pRow, nCol);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
	//set null value
	//(s.dhole 7/29/2014 8:22 AM ) - PLID 63083 reset Lastselected Datalist2
	m_pdlLastRightClickedDatalistDiag = NULL;
}

//(s.dhole 7/29/2014 8:22 AM ) - PLID 63083 based on OnEditActionAnatomicLocations , change function to support datalist 2 for Diagnosis code 
void CEmrActionDlg::OnEditDiagActionAnatomicLocations()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlLastRightClickedDatalistDiag->CurSel;
	if (!pRow) {
		return;
	}

	// (z.manning 2011-06-29 08:43) - PLID 44347 - Pop up a multi select dialog with all active anatomic
	// locations and allow the user to select the ones that should be associated with this action.
	CString strLabAnatomyFilter = VarString(pRow->GetValue(adcLabAnatomyIDs), "");

	// (a.walling 2014-08-06 16:35) - PLID 62687 - Launch new CEmrActionFilterAnatomyDlg with Emr::ActionFilter
	CEmrActionFilterAnatomyDlg dlg(this, Emr::ActionFilter::FromXml(strLabAnatomyFilter));

	int nResult = dlg.DoModal();
	if (nResult == IDOK)
	{
		pRow->PutValue(adcLabAnatomyIDs, dlg.m_filter.ToXml().c_str());
		UpdateDiagRowForAnatomyChange(pRow, adcLabAnatomyIDs);
	}
}

// (z.manning 2011-06-28 09:59) - PLID 44347
short CEmrActionDlg::GetAnatomicLocationColumnFromDestType(EmrActionObject eaoDestType)
{
	short nLabAnatomyColumn = -1;
	switch(eaoDestType)
	{
		case eaoCpt:
			nLabAnatomyColumn = clcLabAnatomyIDs;
			break;

		case eaoTodo:
			nLabAnatomyColumn = ectlLabAnatomyIDs;
			break;
			//(s.dhole 7/16/2014 3:08 PM ) - PLID 
		case eaoDiagnosis :
			nLabAnatomyColumn = adcLabAnatomyIDs;
			break;
		default:
			nLabAnatomyColumn = alcLabAnatomyIDs;
			break;
	}

	return nLabAnatomyColumn;
}

// (z.manning 2011-06-28 09:59) - PLID 44347
short CEmrActionDlg::GetAnatomicLocationColumnFromDatalist(_DNxDataListPtr pdl)
{
	if(pdl == NULL) {
		return -1;
	}
	else if(pdl == m_pChargeList) {
		return clcLabAnatomyIDs;
	}
	else if(pdl == m_pTodoList) {
		return ectlLabAnatomyIDs;
	}
	else {
		return alcLabAnatomyIDs;
	}
}

// (j.jones 2012-08-24 15:14) - PLID 52091 - Added so you could get the DestID column index
// from any given datalist. Not valid on todo lists.
short CEmrActionDlg::GetDestIDColumnFromDatalist(NXDATALISTLib::_DNxDataListPtr pdl)
{
	//these enums are all 1, but we do this for the proper enum, and also to fail on
	//todo lists
	if(pdl == NULL) {
		return -1;
	}
	else if(pdl == m_pChargeList) {
		return clcID;
	}
	else if(pdl == m_pTodoList) {
		//this is not supported, because todos do not have destIDs
		ASSERT(FALSE);
		ThrowNxException("CEmrActionDlg::GetDestIDColumnFromDatalist called on a Todo action!");
		return -1;
	}
	else {
		//generic column for all remaining datalists
		return alcID;
	}
}

// (z.manning 2011-06-28 09:59) - PLID 44347
void CEmrActionDlg::UpdateRowForAnatomyChange(IRowSettingsPtr pRow, const short nLabAnatomyCol)
{
	if(!SourceTypeAllowsPerAnatomicLocation()) {
		return;
	}

	// (z.manning 2011-06-28 17:22) - PLID 44347 - Let's shade action rows with an anatomic location filter
	// so that they stand out.
	Emr::ActionFilter filter = Emr::ActionFilter::FromXml(VarString(pRow->GetValue(nLabAnatomyCol), ""));
	if (filter.empty()) {
		pRow->PutBackColor(NXDATALISTLib::dlColorNotSet);
	}
	else {
		pRow->PutBackColor(RGB(180, 255, 180));
	}
}

//(s.dhole 7/18/2014 11:17 AM ) - PLID 62593 update to datalist2
void CEmrActionDlg::UpdateDiagRowForAnatomyChange(NXDATALIST2Lib::IRowSettingsPtr pRow, const short nLabAnatomyCol)
{
	if (!SourceTypeAllowsPerAnatomicLocation()) {
		return;
	}

	// (z.manning 2011-06-28 17:22) - PLID 44347 - Let's shade action rows with an anatomic location filter
	// so that they stand out.
	Emr::ActionFilter filter = Emr::ActionFilter::FromXml(VarString(pRow->GetValue(nLabAnatomyCol), ""));
	if (filter.empty()) {
		pRow->PutBackColor(NXDATALIST2Lib::dlColorNotSet);
	}
	else {
		pRow->PutBackColor(RGB(180, 255, 180));
	}
}

//(s.dhole 7/31/2014 10:48 AM ) - PLID 63119 Check is code is missing 
BOOL CEmrActionDlg::IsDiagCodeMissing()
{
	BOOL bResult = FALSE;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->GetCurSel();
	if (pRow != nullptr)
	{
		if (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD9_Search)
		{
			// In 9 mode ICD9 is missing
			if (VarLong(pRow->GetValue(adcICD9DiagCodeID), -1) == -1)
			{
				bResult = TRUE;
			}
		}
		else if (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD10_Search)
		{
			if (VarLong(pRow->GetValue(adcICD10DiagCodeID), -1) == -1)
			{
				bResult = TRUE;
			}
		}
	}
	return bResult;
}


// (z.manning 2011-06-28 09:59) - PLID 44347
void CEmrActionDlg::AppendCommonRightClickMenuOptions(CMenu *pMenu)
{
	if(SourceTypeAllowsPerAnatomicLocation())
	{
		if(pMenu->GetMenuItemCount() > 0) {
			pMenu->AppendMenu(MF_SEPARATOR);
		}
		//(s.dhole 7/31/2014 11:01 AM ) - PLID 63119 ser menu enable or desable based on selection
		DWORD dwDisabled = 0;
		if (IsDiagCodeMissing())
		{
			dwDisabled = MF_DISABLED | MF_GRAYED;
		}

		//(s.dhole 7/31/2014 11:01 AM ) - PLID  63119 Set menu 
		pMenu->AppendMenu(MF_ENABLED | dwDisabled, ID_EDIT_ACTION_ANATOMIC_LOCATIONS, "Edit Anatomic &Location Filter...");


		// (j.jones 2012-08-24 14:10) - PLID 52091 - added ability to copy the anatomic location filter from another action,
		// supported on all actions except for todos
		if(m_LastRightClickedAction != eaoTodo && m_LastRightClickedAction != eaoInvalid) {
			BOOL bIsLevel2 = (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
			CString strSourceType = GetEmrActionObjectName(m_SourceType, bIsLevel2);
			CString strDestType = GetEmrActionObjectName(m_LastRightClickedAction, bIsLevel2);
			CString strCopyThis, strCopyAny;
			//accelerator is &C
			strCopyThis.Format("&Copy Anatomic Location Filter from another %s spawning this %s...", strSourceType, strDestType);
			//accelerator is &Y
			strCopyAny.Format("Copy Anatomic Location Filter from an&y %s spawning any %s...", strSourceType, strDestType);
			//(s.dhole 7/31/2014 11:01 AM ) - PLID  63119 Set menu 
			pMenu->AppendMenu(MF_ENABLED | dwDisabled, ID_COPY_THIS_ACTION_ANATOMIC_LOCATIONS, strCopyThis);
			pMenu->AppendMenu(MF_ENABLED | dwDisabled, ID_COPY_ANY_ACTION_ANATOMIC_LOCATIONS, strCopyAny);
		}
	}
}

// (j.jones 2012-08-24 14:10) - PLID 52091 - added ability to copy the anatomic location filter from another action
void CEmrActionDlg::OnCopyThisActionAnatomicLocations()
{
	try {
		//(s.dhole 7/29/2014 9:50 AM ) - PLID 63083 based on datlist selection call selected function
		if (m_pdlLastRightClickedDatalistDiag)
		{
			CopyDiagAnatomicFilters(TRUE);
		}
		else
		{
			CopyAnatomicFilters(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEmrActionDlg::OnCopyAnyActionAnatomicLocations()
{
	try {
		//(s.dhole 7/29/2014 9:50 AM ) - PLID 63083 based on datlist selection call selected function
		if (m_pdlLastRightClickedDatalistDiag)
		{
			CopyDiagAnatomicFilters(FALSE);
		}
		else
		{
			CopyAnatomicFilters(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 8/14/2014 3:56 PM ) - PLID 63128
//validate which row we can add when try to load existing code list, since we will allow same code multiple time in ICD9 mode and icd10 mode due to NexTGem , we need to scan unsaved code
BOOL CEmrActionDlg::IsIncludeThisRow(BOOL bOnlyForThisAction, NXDATALIST2Lib::IRowSettingsPtr pCurrrentRow, NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	long nIcd9ID = VarLong(pCurrrentRow->GetValue(adcICD9DiagCodeID), -1);
	long nIcd10ID = VarLong(pCurrrentRow->GetValue(adcICD10DiagCodeID), -1);
	DiagCodeSearchStyle oStyle = DiagSearchUtils::GetPreferenceSearchStyle();
	//Execute if this value is true
	BOOL bResult = FALSE;
	if (bOnlyForThisAction == TRUE)
	{
		// if crosswalk mode nIcd10ID is-  1 then look for only ICD9 code in existing list 
		if (oStyle == eManagedICD9_Search || (oStyle == eICD9_10_Crosswalk && nIcd10ID == -1))
		{
			if (VarLong(pRow->GetValue(adcICD9DiagCodeID), -1) == nIcd9ID)
			{
				bResult = TRUE;
			}
		}
		// if crosswalk mode nIcd9ID is -1 then look for only ICDI10 code in existing list 
		else if (oStyle == eManagedICD10_Search || (oStyle == eICD9_10_Crosswalk && nIcd9ID == -1))
		{
			if (VarLong(pRow->GetValue(adcICD10DiagCodeID), -1) == nIcd10ID)
			{
				bResult = TRUE;
			}
		}
		else
		{
			//Crosswalk mode   we have nIcd9ID and nIcd10ID are non -1 , So will try to match any code code. ICD10  or ICD9 . 

			long nNewIcd9ID = VarLong(pRow->GetValue(adcICD9DiagCodeID), -1);
			long nNewIcd10ID = VarLong(pRow->GetValue(adcICD10DiagCodeID), -1);
			// Match any of code except -1
			if ((nNewIcd9ID == nIcd9ID && nIcd9ID != -1) || (nNewIcd10ID == nIcd10ID && nIcd10ID != -1))
			{
				bResult = TRUE;
			}

		}
	}
	else
	{
		bResult = TRUE;
	}
	return bResult;
}

//(s.dhole 7/31/2014 3:09 PM ) - PLID 63128 
//move out this code from CopyDiagAnatomicFilters, since it will use in multiple places
// this function will scan exiting list ,and handel unsaved records id
CSqlFragment  CEmrActionDlg::GetDiagCurrentList(BOOL bOnlyForThisAction, NXDATALIST2Lib::IRowSettingsPtr pCurRow)
{
	CSqlFragment sqlCurrentActions("");
	NXDATALIST2Lib::IRowSettingsPtr pCheckRow = m_pdlLastRightClickedDatalistDiag->GetFirstRow();
	while (pCheckRow)
	{
		long nOrder = (-VarLong(pCheckRow->GetValue(adcOrder))) - 100;
		CString strLabAnatomyIDs = VarString(pCheckRow->GetValue(adcLabAnatomyIDs), "");
		if (pCurRow != pCheckRow && (!strLabAnatomyIDs.IsEmpty()) && IsIncludeThisRow(bOnlyForThisAction, pCurRow, pCheckRow)) {
			//track the row index as an ID, but make it negative, subtract 100 so -1 still means invalid
			//track the name    
			CString strDisplayValue = GetDiagCodeDescription(pCheckRow);
			CString strItemName = m_strSourceActionName;
			if (strItemName == m_strSourceObjectName) {
				// (j.jones 2012-08-27 15:31) - This code currently can't be hit, because
				// this feature only exists for smart stamps and they are always saved
				// prior to being able to edit actions.
				//we don't know the item name, so instead say "this action" (ie. "This stamp")
				strItemName.Format("This %s", m_strSourceObjectName);
			}
			strDisplayValue = strItemName + " spawning " + strDisplayValue;
			sqlCurrentActions += CSqlFragment(" UNION SELECT {INT} AS ID, {STRING} AS DisplayValue ", nOrder, strDisplayValue);
		}
		pCheckRow = pCheckRow->GetNextRow();
	}
	return sqlCurrentActions;
}

//(s.dhole 7/31/2014 3:10 PM ) - PLID 63128
//return display text based on available data
CString  CEmrActionDlg::GetDiagCodeDescription(NXDATALIST2Lib::IRowSettingsPtr pRow)
{

	CString strDisplayValue;
	if ((VarLong(pRow->GetValue(adcICD9DiagCodeID), -1) != -1) && (VarLong(pRow->GetValue(adcICD10DiagCodeID), -1) != -1))
	{
		strDisplayValue.Format("%s - %s (%s) ", VarString(pRow->GetValue(adcICD10Code), ""),
			VarString(pRow->GetValue(adcICD10Desc), ""), VarString(pRow->GetValue(adcICD9Code), ""));
	}
	//(s.dhole 7/31/2014 3:10 PM ) - PLID 63126
	else  if (VarLong(pRow->GetValue(adcICD9DiagCodeID), -1) != -1)
	{
		strDisplayValue.Format("%s - %s ", VarString(pRow->GetValue(adcICD9Code), ""),
			VarString(pRow->GetValue(adcICD9Desc), ""));
	}
	//(s.dhole 7/31/2014 3:10 PM ) - PLID 63118
	else  if (VarLong(pRow->GetValue(adcICD10DiagCodeID), -1) != -1)
	{
		strDisplayValue.Format("%s - %s ", VarString(pRow->GetValue(adcICD10Code), ""),
			VarString(pRow->GetValue(adcICD10Desc), ""));
	}
	return strDisplayValue;
}

//(s.dhole 7/31/2014 3:10 PM ) - PLID 63118
//return sql where query based on data and search mode
CSqlFragment CEmrActionDlg::GetDiagFilterSQl(BOOL bOnlyForThisAction,  NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	CSqlFragment sqlWhere("");
	long nIcd9ID = VarLong(pRow->GetValue(adcICD9DiagCodeID), -1);
	long nIcd10ID = VarLong(pRow->GetValue(adcICD10DiagCodeID), -1);
	//Execute if this value is true
	BOOL bResult = FALSE;
	if (bOnlyForThisAction == TRUE)
	{
		if (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD9_Search )
		{
			//In ICD9 mode use only ICD10 code
			sqlWhere = CSqlFragment(" ISNULL(DiagCodeID_ICD9, -1) = { INT }  ", nIcd9ID);
		}
		//(s.dhole 7/31/2014 3:10 PM ) - PLID 63126
		else if (DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD10_Search )
		{
			//In ICD10 mode use only ICD10 code
			sqlWhere = CSqlFragment("  ISNULL(DiagCodeID_ICD10, -1) = { INT } ", nIcd10ID);
		}
		//(s.dhole 7/31/2014 3:10 PM ) - PLID 63128
		else if (DiagSearchUtils::GetPreferenceSearchStyle() == eICD9_10_Crosswalk)
		{
			// In Crosswalk mode we allow to search on any available code
			//(s.dhole 8/19/2014 11:45 AM ) - PLID 63128
			if (nIcd9ID > 0 && nIcd10ID > 0)
			{
				sqlWhere = CSqlFragment(" ( ISNULL(DiagCodeID_ICD9, -1) = { INT } OR ISNULL(DiagCodeID_ICD10, -1) = { INT } )", nIcd9ID, nIcd10ID);
			}
			else if (nIcd9ID > 0 )
			{
				sqlWhere = CSqlFragment(" ISNULL(DiagCodeID_ICD9, -1) = { INT } ", nIcd9ID);
			}
			else if ( nIcd10ID > 0)
			{
				sqlWhere = CSqlFragment("  ISNULL(DiagCodeID_ICD10, -1) = { INT } ", nIcd10ID);
			}
		}
	}
	return sqlWhere;
}

//(s.dhole 7/30/2014 9:48 AM ) - PLID 63083 based on CopyDiagAnatomicFilters, change function to support datalist2
void CEmrActionDlg::CopyDiagAnatomicFilters(BOOL bOnlyForThisAction)
{
	try
	{
		if (m_pdlLastRightClickedDatalistDiag == NULL) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}

		if (m_LastRightClickedAction == eaoInvalid) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}

		if (m_LastRightClickedAction != eaoDiagnosis) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}


		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pdlLastRightClickedDatalistDiag->CurSel;

		short nAnatomicLocationCol = adcLabAnatomyIDs;
		

		long nDestID = VarLong(pCurRow->GetValue(adcActionID));
		long nICD9ID = VarLong(pCurRow->GetValue(adcICD9DiagCodeID));
		long nICD10ID = VarLong(pCurRow->GetValue(adcICD10DiagCodeID));


		//Pop up a single-select dialog of all EMR item actions that have the same source type
		//and same action - so if this is a stamp spawning a diag code, list all stamps that spawn
		//the same diagnosis code, that already have anatomic locations entered.

		//Currently the only source type that allows anatomic locations is a stamp, but this
		//query can easily be changed for any other future type. The only trick is that the
		//DisplayValue text would need to change to properly calculate a description for
		//that action type.
		//So just add the new source type to this if statement, and update the DisplayValue
		//calculation accordingly. You might also need to change the messages for when no actions
		//are found, and the display text on the selection dialog, to be grammatically compatible
		//with the new action type.
		if (m_SourceType != eaoSmartStamp) {
			ThrowNxException("OnCopyActionAnatomicLocations called by an unsupported action type!");
		}

		//we have to filter on non-deleted actions that have active anatomic locations, 
		//and these statments are reused multiple times so I just made it a fragment
		CSqlFragment sqlValidAnatomicLocations("EMRActionAnatomicLocationsT "
			"INNER JOIN EMRActionsT ON EMRActionAnatomicLocationsT.EMRActionID = EMRActionsT.ID "
			"INNER JOIN LabAnatomyT ON EMRActionAnatomicLocationsT.LabAnatomyID = LabAnatomyT.ID "
			"WHERE EMRActionsT.Deleted = 0 AND LabAnatomyT.Inactive = 0");

		//find all actions that have the same source type, but not this source ID,
		//and have the same dest type, and whether we filter by dest ID is controlled
		//by bOnlyForThisAction
		CSqlFragment sqlFrom;
		if (bOnlyForThisAction) {
			//filter only on actions that spawn this same exact thing (ie. same diagnosis code)
			//(s.dhole 8/1/2014 9:50 AM ) -  Now we will show stamp information in dropdown since there can be duplicate code in same action
			sqlFrom = CSqlFragment("(SELECT EmrActionsT.ID, "
				"CASE WHEN EMRActionsT.SourceType = {INT} THEN {SQL} "
				"	ELSE '<Unknown Source>' END "
				"+ ' spawning ' + "
				"{SQL} AS DisplayValue "
				"FROM EmrActionsT "
				"INNER JOIN EmrActionDiagnosisDataT "
				"ON EmrActionDiagnosisDataT.EmrActionID = EmrActionsT.ID "
				"LEFT JOIN DiagCodes AS ICD9Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD9 = ICD9Q.ID "
				"LEFT JOIN DiagCodes AS ICD10Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD10 = ICD10Q.ID "
				"LEFT JOIN EMRImageStampsT ON EmrActionsT.SourceID = EMRImageStampsT.ID AND EMRActionsT.SourceType = {CONST} "
				"WHERE EMRActionsT.Deleted = 0 "
				"AND EmrActionsT.ID IN (SELECT EMRActionsT.ID FROM {SQL}) "
				"AND SourceType = {INT} AND SourceID <> {INT} AND DestType = {INT} AND DestID = -1 AND  {SQL}  "
				"{SQL}"
				") AS ActionsQ",
				eaoSmartStamp, CSqlFragment(GetEmrActionObjectSourceNameField(eaoSmartStamp)),
				CSqlFragment(GetEmrActionObjectDestNameField(eaoDiagnosis)),
				eaoSmartStamp,
				sqlValidAnatomicLocations,
				m_SourceType, m_nSourceID, (long)m_LastRightClickedAction,
				//(s.dhole 7/31/2014 3:12 PM ) - PLID 63118
				GetDiagFilterSQl(bOnlyForThisAction,  pCurRow),
				//(s.dhole 7/31/2014 3:12 PM ) - PLID 63128
				GetDiagCurrentList(bOnlyForThisAction, pCurRow)
				);
		}
		else {
			//include actions that spawn the same destination type, but all codes (ie. any diagnosis code),
			//which also means showing other codes this same source item spawns

			//get the current item actions, they may not yet be saved
			CSqlFragment sqlCurrentActions("");

			//(s.dhole 7/31/2014 3:09 PM ) - PLID 63128 load item sql from current datalist
			sqlCurrentActions = GetDiagCurrentList(bOnlyForThisAction, pCurRow);
			// (b.savon 2014-07-14 13:56) - PLID 62711 - Handle loading the new Diagnosis DestType action from EmrActionsT and EmrActionsDiagnosisDataT
			//(s.dhole 7/31/2014 3:12 PM ) - PLID 63118
			sqlFrom = CSqlFragment("(SELECT EmrActionsT.ID, "
				"CASE WHEN EMRActionsT.SourceType = {INT} THEN {SQL} "
				"	ELSE '<Unknown Source>' END "
				"+ ' spawning ' + "
				"{SQL} AS DisplayValue "
				"FROM EmrActionsT "
				"INNER JOIN EMRImageStampsT ON (EmrActionsT.SourceID = EMRImageStampsT.ID AND EMRActionsT.SourceType = {CONST}) "
				"INNER JOIN EMRActionDiagnosisDataT ON (EMRActionsT.ID = EMRActionDiagnosisDataT.EmrActionID AND EMRActionsT.DestType = {INT}) "
				"LEFT JOIN DiagCodes AS ICD9Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD9 = ICD9Q.ID "
				"LEFT JOIN DiagCodes AS ICD10Q ON EmrActionDiagnosisDataT.DiagCodeID_ICD10 = ICD10Q.ID "
				"WHERE EMRActionsT.Deleted = 0 "
				"AND EmrActionsT.ID IN (SELECT EMRActionsT.ID "
				"	FROM {SQL}) "
				"AND SourceType = {INT} AND SourceID <> {INT} AND DestType = {INT} "
				"{SQL} "
				") AS ActionsQ",
				eaoSmartStamp, CSqlFragment(GetEmrActionObjectSourceNameField(eaoSmartStamp)),
				m_LastRightClickedAction == eaoTodo ? "''" : CSqlFragment(GetEmrActionObjectDestNameField(m_LastRightClickedAction)),
				eaoSmartStamp, eaoDiagnosis,	 sqlValidAnatomicLocations,
				m_SourceType, m_nSourceID, (long)m_LastRightClickedAction,
				sqlCurrentActions);
		}
		
		BOOL bIsLevel2 = (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);

		//warn if no existing actions have anatomic locations
		_RecordsetPtr rsCheckActions = CreateParamRecordset("SELECT TOP 1 * FROM {SQL}", sqlFrom);
		if (rsCheckActions->eof) {
			CString strWarning;
			strWarning.Format("No existing %ss spawn %s%s%s with an anatomic location filter. "
				"There are no available actions to copy from.",
				GetEmrActionObjectName(m_SourceType, bIsLevel2),
				bOnlyForThisAction ? "this " : "",
				GetEmrActionObjectName(m_LastRightClickedAction, bIsLevel2),
				bOnlyForThisAction ? "" : "s");
			MessageBox(strWarning, "Practice", MB_ICONINFORMATION | MB_OK);
			return;
		}
		rsCheckActions->Close();

		//If we get here, we know there are valid items to copy from.
		//If the current action already has anatomic locations, ask the user if they want
		//to replace them or not.
		CString strCurrentLabAnatomyIDs = VarString(pCurRow->GetValue(adcLabAnatomyIDs), "");
		if (!strCurrentLabAnatomyIDs.IsEmpty() &&
			IDNO == MessageBox("This action already has an anatomic location filter. "
			"Copying anatomic locations from another action will replace the current filter.\n\n"
			"Are you sure you want to copy anatomic locations from another action?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
			return;
		}

		//convert the paramaterized query into something the single select list can handle
		CString strFrom = sqlFrom.Flatten();

		CString strLabel;
		//change the label to explain whether we filtered only on actions that spawn this same exact thing (ie. same diagnosis code),
		//or on any action that spawns the same object type (ie. any diagnosis code)
		strLabel.Format("The following %ss spawn %s%s%s with an anatomic location filter. Please select the existing action that anatomic locations should be copied from.",
			GetEmrActionObjectName(eaoDiagnosis, bIsLevel2),
			bOnlyForThisAction ? "this " : "",
			GetEmrActionObjectName(m_LastRightClickedAction, bIsLevel2),
			bOnlyForThisAction ? "" : "s");

		CSingleSelectDlg dlg(this);
		if (IDOK == dlg.Open(strFrom, "", "ID", "DisplayValue", strLabel)) {

			long nActionIDToCopy = dlg.GetSelectedID();
			if (nActionIDToCopy != -1) {
				//get the active anatomic locations tied to this action

				CString strLabAnatomyIDs;

				if (nActionIDToCopy < -1) {
					//this is a row ID, negative, with 100 subtracted
					nActionIDToCopy += 100;
					//this should now be 0 or negative row index
					if (nActionIDToCopy <= 0) {
						nActionIDToCopy = -nActionIDToCopy;
						//get this row's filter (will fail if it's an invalid row)
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlLastRightClickedDatalistDiag->FindByColumn(adcOrder, nActionIDToCopy, NULL, VARIANT_FALSE);
						if (pRow)
						{
							strLabAnatomyIDs = VarString(pRow->GetValue(adcLabAnatomyIDs), "");
						}
						else
						{
							//if it's positive, then this is an invalid Order
							ThrowNxException("Failed to find existing row %li!", nActionIDToCopy);
						}
					}
					else {
						//if it's positive, then this is an invalid order
						ThrowNxException("Failed to find existing row %li!", nActionIDToCopy);
					}
				}
				else {
					//this is an action ID, load its filter from data
					// (a.walling 2014-08-14 14:53) - PLID 62682 - Load the filter from the action via LoadActionInfo
					MFCArray<EmrAction> actions;
					::LoadActionInfo(
						{
							"EMRActionsT.ID IN (SELECT EMRActionsT.ID FROM {SQL} AND EMRActionsT.ID = {INT})"
							, sqlValidAnatomicLocations
							, nActionIDToCopy
						}
						, actions
					);

					//track as a string
					if (!actions.IsEmpty()) {
						strLabAnatomyIDs = actions[0].filter.ToXml().c_str();
					}
				}
				//this will replace any anatomic locations currently tracked,
				//we would have already warned earlier if this was going to happen
				pCurRow->PutValue(nAnatomicLocationCol, _bstr_t(strLabAnatomyIDs));
				UpdateDiagRowForAnatomyChange(pCurRow, nAnatomicLocationCol);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-08-24 14:10) - PLID 52091 - added ability to copy the anatomic location filter from another action
void CEmrActionDlg::CopyAnatomicFilters(BOOL bOnlyForThisAction)
{
	try
	{
		if(m_pdlLastRightClickedDatalist == NULL) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}

		if(m_LastRightClickedAction == eaoInvalid) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}

		//(s.dhole 7/30/2014 9:49 AM ) - PLID 63083 do not pass diagnosis changes
		if (m_LastRightClickedAction == eaoDiagnosis) {
			//shouldn't be possible 
			ASSERT(FALSE);
			return;
		}

		long nRow = m_pdlLastRightClickedDatalist->GetCurSel();
		if(nRow == -1) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}
		IRowSettingsPtr pCurRow = m_pdlLastRightClickedDatalist->GetRow(nRow);
		if(pCurRow == NULL) {
			//shouldn't be possible, how was this function called?
			ASSERT(FALSE);
			return;
		}

		short nAnatomicLocationCol = GetAnatomicLocationColumnFromDatalist(m_pdlLastRightClickedDatalist);
		if(nAnatomicLocationCol == -1) {
			//shouldn't be possible, why was this function even called?
			ASSERT(FALSE);
			return;
		}

		//(s.dhole 7/30/2014 8:05 AM ) - PLID 63083 should not happen
		if (m_LastRightClickedAction == eaoDiagnosis) {
			ASSERT(FALSE);
			ThrowNxException("CEmrActionDlg::OnCopyActionAnatomicLocations called on a Todo action!");
			}

		//todos aren't supported here, they don't have destIDs
		if(m_LastRightClickedAction == eaoTodo) {
			ASSERT(FALSE);
			ThrowNxException("CEmrActionDlg::OnCopyActionAnatomicLocations called on a Todo action!");
		}

		short nDestIDCol = GetDestIDColumnFromDatalist(m_pdlLastRightClickedDatalist);
		if(nDestIDCol == -1) {
			//shouldn't be possible
			ASSERT(FALSE);
			return;
		}
		long nDestID = VarLong(pCurRow->GetValue(nDestIDCol));

		//Pop up a single-select dialog of all EMR item actions that have the same source type
		//and same action - so if this is a stamp spawning a diag code, list all stamps that spawn
		//the same diagnosis code, that already have anatomic locations entered.
		
		//Currently the only source type that allows anatomic locations is a stamp, but this
		//query can easily be changed for any other future type. The only trick is that the
		//DisplayValue text would need to change to properly calculate a description for
		//that action type.
		//So just add the new source type to this if statement, and update the DisplayValue
		//calculation accordingly. You might also need to change the messages for when no actions
		//are found, and the display text on the selection dialog, to be grammatically compatible
		//with the new action type.
		if(m_SourceType != eaoSmartStamp) {
			ThrowNxException("OnCopyActionAnatomicLocations called by an unsupported action type!");
		}

		//we have to filter on non-deleted actions that have active anatomic locations, 
		//and these statments are reused multiple times so I just made it a fragment
		CSqlFragment sqlValidAnatomicLocations("EMRActionAnatomicLocationsT "
			"INNER JOIN EMRActionsT ON EMRActionAnatomicLocationsT.EMRActionID = EMRActionsT.ID "
			"INNER JOIN LabAnatomyT ON EMRActionAnatomicLocationsT.LabAnatomyID = LabAnatomyT.ID "
			"WHERE EMRActionsT.Deleted = 0 AND LabAnatomyT.Inactive = 0");
		
		//find all actions that have the same source type, but not this source ID,
		//and have the same dest type, and whether we filter by dest ID is controlled
		//by bOnlyForThisAction
		CSqlFragment sqlFrom;
		if(bOnlyForThisAction) {
			//filter only on actions that spawn this same exact thing (ie. same diagnosis code)
			sqlFrom = CSqlFragment("(SELECT EmrActionsT.ID, "
				"CASE WHEN EMRActionsT.SourceType = {INT} THEN {SQL} "
				"	ELSE '<Unknown Source>' END AS DisplayValue "
				"FROM EmrActionsT "
				"LEFT JOIN EMRImageStampsT ON EmrActionsT.SourceID = EMRImageStampsT.ID AND EMRActionsT.SourceType = {CONST} "
				"WHERE EMRActionsT.Deleted = 0 "
				"AND EmrActionsT.ID IN (SELECT EMRActionsT.ID FROM {SQL}) "
				"AND SourceType = {INT} AND SourceID <> {INT} AND DestType = {INT} AND DestID = {INT} "
				") AS ActionsQ",
				eaoSmartStamp, CSqlFragment(GetEmrActionObjectSourceNameField(eaoSmartStamp)),
				eaoSmartStamp,
				sqlValidAnatomicLocations,
				m_SourceType, m_nSourceID, (long)m_LastRightClickedAction, nDestID);
		}
		else {
			//include actions that spawn the same destination type, but all codes (ie. any diagnosis code),
			//which also means showing other codes this same source item spawns

			//get the current item actions, they may not yet be saved
			CSqlFragment sqlCurrentActions("");
			for(int i=0;i<m_pdlLastRightClickedDatalist->GetRowCount(); i++) {
				//skip this row
				if(i != nRow) {
					IRowSettingsPtr pCheckRow = m_pdlLastRightClickedDatalist->GetRow(i);
					CString strLabAnatomyIDs = VarString(pCheckRow->GetValue(nAnatomicLocationCol), "");
					if(!strLabAnatomyIDs.IsEmpty()) {
						//track the row index as an ID, but make it negative, subtract 100 so -1 still means invalid
						long nRowID = (-i) - 100;
						//track the name
						CString strDisplayValue;
						// (b.savon 2014-07-15 15:50) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
						if(m_LastRightClickedAction != eaoCpt && m_LastRightClickedAction != eaoDiagnosis/*eaoDiag*/) {
							strDisplayValue = VarString(pCheckRow->GetValue(alcName), "");
						}
						else if(m_LastRightClickedAction == eaoCpt) {
							strDisplayValue = VarString(pCheckRow->GetValue(clcName), "");
							if(!strDisplayValue.IsEmpty()) {
								strDisplayValue += " - ";
							}
							strDisplayValue += VarString(pCheckRow->GetValue(clcDescription), "");
						}
						// (b.savon 2014-07-15 15:50) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
						else if(m_LastRightClickedAction == eaoDiagnosis/*eaoDiag*/) {
							strDisplayValue = VarString(pCheckRow->GetValue(alcName), "");
							if(!strDisplayValue.IsEmpty()) {
								strDisplayValue += " - ";
							}
							strDisplayValue += VarString(pCheckRow->GetValue(alcName2), "");
						}
						CString strItemName = m_strSourceActionName;
						if(strItemName == m_strSourceObjectName) {
							// (j.jones 2012-08-27 15:31) - This code currently can't be hit, because
							// this feature only exists for smart stamps and they are always saved
							// prior to being able to edit actions.

							//we don't know the item name, so instead say "this action" (ie. "This stamp")
							strItemName.Format("This %s", m_strSourceObjectName);
						}
						strDisplayValue = strItemName + " spawning " + strDisplayValue;
						sqlCurrentActions += CSqlFragment(" UNION SELECT {INT} AS ID, {STRING} AS DisplayValue ", nRowID, strDisplayValue);
					}
				}
			}
			// (b.savon 2014-07-14 13:56) - PLID 62711 - Handle loading the new Diagnosis DestType action from EmrActionsT and EmrActionsDiagnosisDataT
			//(s.dhole 7/30/2014 9:49 AM ) - PLID 63083 do not need diagnosis changes
			sqlFrom = CSqlFragment("(SELECT EmrActionsT.ID, "
				"CASE WHEN EMRActionsT.SourceType = {INT} THEN {SQL} "
				"	ELSE '<Unknown Source>' END "
				"+ ' spawning ' + "
				"{SQL} AS DisplayValue "
				"FROM EmrActionsT "
				"LEFT JOIN EMRImageStampsT ON EmrActionsT.SourceID = EMRImageStampsT.ID AND EMRActionsT.SourceType = {CONST} "
				"LEFT JOIN ServiceT ON EMRActionsT.DestID = ServiceT.ID AND EMRActionsT.DestType = {CONST}  "
				"LEFT JOIN CptCodeT ON ServiceT.ID = CPTCodeT.ID "
				//"LEFT JOIN EMRActionDiagnosisDataT ON EMRActionsT.ID = EMRActionDiagnosisDataT.EmrActionID AND EMRActionsT.DestType = {CONST} "
				"LEFT JOIN EMRInfoMasterT ON EMRActionsT.DestID = EMRInfoMasterT.ID AND EMRActionsT.DestType = {CONST} "
				"LEFT JOIN EMRInfoT ON EMRInfoMasterT.ActiveEMRInfoID = EMRInfoT.ID "
				"LEFT JOIN ProcedureT ON EMRActionsT.DestID = ProcedureT.ID AND EMRActionsT.DestType = {CONST} "
				"LEFT JOIN EMRTemplateT ON EMRActionsT.DestID = EMRTemplateT.ID AND EMRActionsT.DestType IN ({CONST}, {CONST}) "
				"LEFT JOIN DrugList ON EMRActionsT.DestID = DrugList.ID AND EMRActionsT.DestType = {CONST} "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"LEFT JOIN LabProceduresT ON EMRActionsT.DestID = LabProceduresT.ID AND EMRActionsT.DestType = {CONST} "
				"WHERE EMRActionsT.Deleted = 0 "
				"AND EmrActionsT.ID IN (SELECT EMRActionsT.ID "
				"	FROM {SQL}) "
				"AND SourceType = {INT} AND SourceID <> {INT} AND DestType = {INT} "
				"{SQL} "
				") AS ActionsQ",
				eaoSmartStamp, CSqlFragment(GetEmrActionObjectSourceNameField(eaoSmartStamp)),
				m_LastRightClickedAction == eaoTodo ? "''" : CSqlFragment(GetEmrActionObjectDestNameField(m_LastRightClickedAction)),
				eaoSmartStamp,
				eaoCpt/*, eaoDiagnosis eaoDiag */ , eaoEmrItem, eaoProcedure, eaoMint, eaoMintItems, eaoMedication, eaoLab,
				sqlValidAnatomicLocations,
				m_SourceType, m_nSourceID, (long)m_LastRightClickedAction,
				sqlCurrentActions);
		}

		BOOL bIsLevel2 = (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
		//warn if no existing actions have anatomic locations
		_RecordsetPtr rsCheckActions = CreateParamRecordset("SELECT TOP 1 * FROM {SQL}", sqlFrom);
		if(rsCheckActions->eof) {
			CString strWarning;
			strWarning.Format("No existing %ss spawn %s%s%s with an anatomic location filter. "
				"There are no available actions to copy from.",
				GetEmrActionObjectName(m_SourceType, bIsLevel2),
				bOnlyForThisAction ? "this " : "",
				GetEmrActionObjectName(m_LastRightClickedAction, bIsLevel2),
				bOnlyForThisAction ? "" : "s");
			MessageBox(strWarning, "Practice", MB_ICONINFORMATION|MB_OK);
			return;
		}
		rsCheckActions->Close();

		//If we get here, we know there are valid items to copy from.
		//If the current action already has anatomic locations, ask the user if they want
		//to replace them or not.
		CString strCurrentLabAnatomyIDs = VarString(pCurRow->GetValue(nAnatomicLocationCol), "");
		if(!strCurrentLabAnatomyIDs.IsEmpty() &&
			IDNO == MessageBox("This action already has an anatomic location filter. "
				"Copying anatomic locations from another action will replace the current filter.\n\n"
				"Are you sure you want to copy anatomic locations from another action?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//convert the paramaterized query into something the single select list can handle
		CString strFrom = sqlFrom.Flatten();

		CString strLabel;
		//change the label to explain whether we filtered only on actions that spawn this same exact thing (ie. same diagnosis code),
		//or on any action that spawns the same object type (ie. any diagnosis code)
		strLabel.Format("The following %ss spawn %s%s%s with an anatomic location filter. Please select the existing action that anatomic locations should be copied from.",
			GetEmrActionObjectName(m_SourceType, bIsLevel2),
			bOnlyForThisAction ? "this " : "",
			GetEmrActionObjectName(m_LastRightClickedAction, bIsLevel2),
			bOnlyForThisAction ? "" : "s");

		CSingleSelectDlg dlg(this);
		if(IDOK == dlg.Open(strFrom, "", "ID", "DisplayValue", strLabel)) {

			long nActionIDToCopy = dlg.GetSelectedID();
			if(nActionIDToCopy != -1) {
				//get the active anatomic locations tied to this action

				CString strLabAnatomyIDs;

				if(nActionIDToCopy < -1) {
					//this is a row ID, negative, with 100 subtracted
					nActionIDToCopy += 100;
					//this should now be 0 or negative row index
					if(nActionIDToCopy <= 0) {
						nActionIDToCopy = -nActionIDToCopy;
						//get this row's filter (will fail if it's an invalid row)
						strLabAnatomyIDs = VarString(m_pdlLastRightClickedDatalist->GetValue(nActionIDToCopy, nAnatomicLocationCol), "");
					}
					else {
						//if it's positive, then this is an invalid index
						ThrowNxException("Failed to find existing row %li!", nActionIDToCopy);
					}
				}
				else {
					//this is an action ID, load its filter from data
					// (a.walling 2014-08-14 14:53) - PLID 62682 - Load the filter from the action via LoadActionInfo
					MFCArray<EmrAction> actions;
					::LoadActionInfo(
						{ 
							"EMRActionsT.ID IN (SELECT EMRActionsT.ID FROM {SQL} AND EMRActionsT.ID = {INT})"
							, sqlValidAnatomicLocations
							, nActionIDToCopy 
						}
						, actions
					);

					//track as a string
					if (!actions.IsEmpty()) {
						strLabAnatomyIDs = actions[0].filter.ToXml().c_str();
					}
				}
				//this will replace any anatomic locations currently tracked,
				//we would have already warned earlier if this was going to happen
				pCurRow->PutValue(nAnatomicLocationCol, _bstr_t(strLabAnatomyIDs));
				UpdateRowForAnatomyChange(pCurRow, nAnatomicLocationCol);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 07\08\2014 8:56) - PLID 62592  Code selection
void CEmrActionDlg::SelChosenActionDiagSearch(LPDISPATCH lpRow)
{
	try {
		if (lpRow == NULL) {
			return;
		}

		CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);
		//(s.dhole 7/17/2014 4:51 PM ) - PLID 62723 
		AddDiagCodeBySearchResults(results);
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 07\09\2014 8:56) - PLID 63114  set dispaly column
void CEmrActionDlg::UpdateDiagnosisListColumnSizes()
{
	SizeDiagnosisListColumnsBySearchPreference(m_pDiagList, adcICD9DiagCodeID , adcICD10DiagCodeID , adcICD9Code, adcICD10Code,
		50, 50, "", "", adcICD9Desc, adcICD10Desc, false, true, true, DiagSearchUtils::GetPreferenceSearchStyle());
	
	
}
//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114   search for duplicate row
BOOL CEmrActionDlg::IsDuplicateDiagnosisCode(short nColumn )
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->GetFirstRow();
	while (pRow != nullptr)
	{
		long  nICDID = VarLong(pRow->GetValue(nColumn));
		
		NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
		if (pNextRow != nullptr)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRowSearch = m_pDiagList->FindByColumn(nColumn, nICDID, pNextRow, VARIANT_FALSE);
			if (pRowSearch != nullptr && pRow != pRowSearch){
				// Found a row in the list that matches the original, other than the original
				return TRUE;
			}
		}
		// No duplicates of this row, loop back up and see if the next row has duplicates
		pRow = pNextRow;
	}
	return FALSE;
}





//(s.dhole 7/18/2014 9:33 AM ) - PLID 63114  This function will show both icd10 and icd9 if one of the code is missing and preference is  ICD9 or ICD10 mode
// this is based on DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference
void CEmrActionDlg::SizeDiagnosisListColumnsBySearchPreference(NXDATALIST2Lib::_DNxDataListPtr pDataList,
	short iColICD9CodeID, short iColICD10CodeID,
	short iColICD9Code, short iColICD10Code,
	long nMinICD9CodeWidth , long nMinICD10CodeWidth ,
	CString strICD9EmptyFieldText , CString strICD10EmptyFieldText ,
	short iColICD9Desc , short iColICD10Desc ,
	bool bShowDescriptionColumnsInCrosswalk ,
	bool bAllowCodeWidthAuto ,
	bool bRenameDescriptionColumns ,
	DiagCodeSearchStyle eSearchStyle
	)
{
	//If one empty field text was provided, why wasn't the other?
	//There is no situation where one would be filled and one would not be.
	//If you truly do need this, remove the assertion.
	ASSERT(strICD9EmptyFieldText.IsEmpty() == strICD10EmptyFieldText.IsEmpty());

	//get the desired search style, this is cached at startup in MainFrame
	

	try {

		//confirm this is a valid style - there are more style enumerations than are allowed
		//to be return values from GetPreferenceSearchStyle
		if (eSearchStyle != eICD9_10_Crosswalk && eSearchStyle != eManagedICD9_Search && eSearchStyle != eManagedICD10_Search) {
			//This function should never be called on lists that don't follow the preference search style,
			//which can never be any style but crosswalk, Managed ICD-9, or Managed ICD-10.
			//If it is ever anything else, this code may need to be changed to reflect the new style option.
			ASSERT(FALSE);

			//revert to Crosswalk if this happens
			eSearchStyle = eICD9_10_Crosswalk;
		}

		//If the style is not the crosswalk, we would normally show only ICD-9 or
		//ICD-10, based on their search style. But if this patient has a code in
		//the hidden column, we need to show both columns, just like in crosswalk mode.
		if (eSearchStyle != eICD9_10_Crosswalk) {
			bool bHasICD9Missing = false;
			bool bHasICD10Missing = false;
			NXDATALIST2Lib::IRowSettingsPtr pRow = pDataList->GetFirstRow();
			while (pRow != NULL && (!bHasICD9Missing || !bHasICD10Missing)) {
				long nICD9Code = VarLong(pRow->GetValue(iColICD9CodeID), -1);
				long nICD10Code = VarLong(pRow->GetValue(iColICD10CodeID), -1);

				//if the code field is not empty, and it's not the placeholder text
				//for an empty field, then we must display this column
				if (nICD9Code==-1) {
					bHasICD9Missing = true;
				}
				if (nICD10Code == -1) {
					bHasICD10Missing = true;
				}

				pRow = pRow->GetNextRow();
			}

			//if a code exists that is not in the desired search style,
			//revert to the column layout for the crosswalk search,
			//such that both ICD-9 and ICD-10 codes are displayed
			if ((eSearchStyle == eManagedICD10_Search  && bHasICD10Missing) || (eSearchStyle == eManagedICD9_Search && bHasICD9Missing)) {

				eSearchStyle = eICD9_10_Crosswalk;
			}
		}
		
		//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 Do not Collapse column if there is duplicate code in mutiple rows
		if ((eSearchStyle != eICD9_10_Crosswalk) && (IsDuplicateDiagnosisCode((eSearchStyle == eManagedICD9_Search) ? adcICD9DiagCodeID : adcICD10DiagCodeID)))
		{

			eSearchStyle = eICD9_10_Crosswalk;
		}

		//now ensure our columns are correct for the desired search style

		NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD9Code = pDataList->GetColumn(iColICD9Code);
		NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD10Code = pDataList->GetColumn(iColICD10Code);

		//get the description columns, only if indexes were provided

		NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD9Desc = NULL;
		NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD10Desc = NULL;

		if (iColICD9Desc != -1 && iColICD10Desc != -1) {
			pColDiagICD9Desc = pDataList->GetColumn(iColICD9Desc);
			pColDiagICD10Desc = pDataList->GetColumn(iColICD10Desc);
		}

		//handle the ICD-9 description column
		bool bIsShowingICD9Desc = false;
		if (pColDiagICD9Desc) {

			//rename the column, if requested
			if (bRenameDescriptionColumns) {
				if (eSearchStyle == eManagedICD9_Search) {
					pColDiagICD9Desc->PutColumnTitle("Description");
				}
				else {
					pColDiagICD9Desc->PutColumnTitle("ICD-9 Description");
				}
			}

			if (eSearchStyle == eManagedICD10_Search
				|| (eSearchStyle == eICD9_10_Crosswalk && !bShowDescriptionColumnsInCrosswalk)) {

				//hide the ICD-9 description column
				long nWidth = 0;
				long nStyle = csVisible | csFixedWidth;

				if (eSearchStyle == eICD9_10_Crosswalk) {
					//in crosswalk mode, allow it to be resizeable
					nStyle = csVisible;
				}

				//when hiding the column, set the style first
				pColDiagICD9Desc->PutColumnStyle(nStyle);
				pColDiagICD9Desc->PutStoredWidth(nWidth);

				bIsShowingICD9Desc = false;
			}
			else {
				//normal case, width auto
				pColDiagICD9Desc->PutColumnStyle(csVisible | csWidthAuto);

				bIsShowingICD9Desc = true;
			}
		}

		//handle the ICD-10 description column
		bool bIsShowingICD10Desc = false;
		if (pColDiagICD10Desc) {

			//rename the column, if requested
			if (bRenameDescriptionColumns) {
				if (eSearchStyle == eManagedICD10_Search) {
					pColDiagICD10Desc->PutColumnTitle("Description");
				}
				else {
					pColDiagICD10Desc->PutColumnTitle("ICD-10 Description");
				}
			}

			if (eSearchStyle == eManagedICD9_Search
				|| (eSearchStyle == eICD9_10_Crosswalk && !bShowDescriptionColumnsInCrosswalk)) {

				//hide the ICD-10 description column
				long nWidth = 0;
				long nStyle = csVisible | csFixedWidth;

				if (eSearchStyle == eICD9_10_Crosswalk) {
					//in crosswalk mode, allow it to be resizeable
					nStyle = csVisible;
				}

				//when hiding the column, set the style first
				pColDiagICD10Desc->PutColumnStyle(nStyle);
				pColDiagICD10Desc->PutStoredWidth(nWidth);

				bIsShowingICD10Desc = false;
			}
			else {
				//normal case, width auto
				pColDiagICD10Desc->PutColumnStyle(csVisible | csWidthAuto);

				bIsShowingICD10Desc = true;
			}
		}

		//handle the ICD-9 code column
		if (eSearchStyle == eManagedICD10_Search) {
			//hide the ICD-9 code column
			//when hiding the column, set the style first
			pColDiagICD9Code->PutColumnStyle(csVisible | csFixedWidth);
			pColDiagICD9Code->PutStoredWidth(0);
		}
		else if (bAllowCodeWidthAuto && !bIsShowingICD9Desc) {
			//if we are not showing the description column,
			//change the style to csWidthAuto
			pColDiagICD9Code->PutColumnStyle(csVisible | csWidthAuto);
		}
		else {
			//normal case, width data
			//when applying data-width, set the style last
			pColDiagICD9Code->PutStoredWidth(nMinICD9CodeWidth);
			pColDiagICD9Code->PutColumnStyle(csVisible | csWidthData);
		}

		//handle the ICD-10 code column
		if (eSearchStyle == eManagedICD9_Search) {
			//hide the ICD-10 code column
			//when hiding the column, set the style first
			pColDiagICD10Code->PutColumnStyle(csVisible | csFixedWidth);
			pColDiagICD10Code->PutStoredWidth(0);
		}
		else if (bAllowCodeWidthAuto && !bIsShowingICD10Desc) {
			//if we are not showing the description column,
			//change the style to csWidthAuto
			pColDiagICD10Code->PutColumnStyle(csVisible | csWidthAuto);
		}
		else {
			//normal case, width data
			//when applying data-width, set the style last
			pColDiagICD10Code->PutStoredWidth(nMinICD10CodeWidth);
			pColDiagICD10Code->PutColumnStyle(csVisible | csWidthData);
		}
	}NxCatchAll(__FUNCTION__);
}


//(s.dhole 7/15/2014 8:13 AM ) - PLID 62723
NXDATALIST2Lib::IRowSettingsPtr CEmrActionDlg::FindRowInDiagList(long nICD9ID, long nICD10ID)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->GetFirstRow();
	while (pRow) {
		if (VarLong(pRow->GetValue(adcICD9DiagCodeID), -1) == nICD9ID && VarLong(pRow->GetValue(adcICD10DiagCodeID), -1) == nICD10ID) {
			return pRow;
		}
		pRow = pRow->GetNextRow();
	}
	return NULL;
}




//(s.dhole 7/17/2014 4:51 PM ) - PLID 62723 Add Dignosis code result to datalist
void CEmrActionDlg::AddDiagCodeBySearchResultsData(CDiagSearchResults &results, NexGEMMatchType matchType)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->GetNewRow();
	DiagCodeList diagCodeList;
	diagCodeList.nActionID = -1;
	diagCodeList.nICD9 = VarLong(results.m_ICD9.m_nDiagCodesID, -1);
	diagCodeList.nICD10 = VarLong(results.m_ICD10.m_nDiagCodesID, -1);
	diagCodeList.strICD9Code = results.m_ICD9.m_strCode;
	diagCodeList.strICD10Code = results.m_ICD10.m_strCode;
	diagCodeList.strICD9Description  = results.m_ICD9.m_strDescription ;
	diagCodeList.strICD10Description = results.m_ICD10.m_strDescription;
	diagCodeList.matchType = matchType;
	HandelDatalistRowUpdate(pRow, diagCodeList);
	pRow->PutValue(adcOrder, m_pDiagList->GetRowCount() + 1);
	pRow->PutValue(adcPopup, VARIANT_FALSE);
	pRow->PutValue(adcProblemArray, (long)(new CProblemActionAry));
	UpdateDiagActionProblemRow(pRow, adcProblemBitmap, adcProblemArray);
	pRow->PutValue(adcLabAnatomyIDs, g_cvarNull);
	m_pDiagList->AddRowAtEnd(pRow, NULL);
}

//(s.dhole 7/17/2014 4:51 PM ) - PLID 62597 data row update and back  color
void CEmrActionDlg::HandelDatalistRowUpdate(NXDATALIST2Lib::IRowSettingsPtr pRow, DiagCodeList &diagCodeList)
{
	pRow->PutValue(adcActionID, diagCodeList.nActionID);
	pRow->PutValue(adcICD9DiagCodeID, diagCodeList.nICD9);
	pRow->PutValue(adcICD10DiagCodeID, diagCodeList.nICD10);
	pRow->PutValue(adcICD9Desc , _bstr_t(diagCodeList.strICD9Description ));
	pRow->PutValue(adcICD10Desc, _bstr_t(diagCodeList.strICD10Description));
	pRow->PutValue(adcMatchtype, diagCodeList.matchType);
	NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
	pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);
	if (diagCodeList.nICD9 == -1) {
		pRow->PutCellLinkStyle(adcICD9Code, NXDATALIST2Lib::dlLinkStyleTrue);
		pRow->PutRefCellFormatOverride(adcICD9Code, pHyperLink);
		pRow->PutValue(adcICD9Code, _bstr_t("<No Matching Code>"));
	}
	else{
		pRow->PutValue(adcICD9Code, _bstr_t(diagCodeList.strICD9Code ));
		pRow->PutCellLinkStyle(adcICD9Code, NXDATALIST2Lib::dlLinkStyleFalse);
	}

	if (diagCodeList.nICD10 == -1) {
		pRow->PutCellLinkStyle(adcICD10Code, NXDATALIST2Lib::dlLinkStyleTrue);
		pRow->PutRefCellFormatOverride(adcICD10Code, pHyperLink);
	}
	else{
		pRow->PutCellLinkStyle(adcICD10Code, NXDATALIST2Lib::dlLinkStyleFalse);
		NXDATALIST2Lib::IFormatSettingsPtr pNoHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pNoHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
		pRow->PutRefCellFormatOverride(adcICD10Code, pNoHyperLink);
	}
	pRow->PutValue(adcICD10Code, _bstr_t(GetNexGEMDisplayText(diagCodeList.nICD10, diagCodeList.strICD10Code, diagCodeList.matchType)));
	if (diagCodeList.matchType== nexgemtManyMatch)
	{
		//Change color only from multi match
		pRow->PutCellBackColor(adcICD10Code, GetDiagnosisNexGEMMatchColor(diagCodeList.nICD10, diagCodeList.matchType));
	}
	else if (pRow->GetValue(adcLabAnatomyIDs).vt == VT_BSTR)	
	{
		// update back color based on icd 9 colum
		pRow->PutCellBackColor(adcICD10Code, pRow->GetCellBackColor(adcICD9Code));
	
	}
	
}

// (s.dhole 07\09\2014 8:56) - PLID 62723  update datalist
BOOL  CEmrActionDlg::AddDiagCodeBySearchResults(CDiagSearchResults &results)
{
	//Return if nothing is selected
	if (results.m_ICD9.m_nDiagCodesID == -1 && results.m_ICD10.m_nDiagCodesID == -1) {
		return TRUE ;
	}

	//Update the interface
	// Make sure it's not already in the list
	//(s.dhole 7/16/2014 10:13 AM ) - PLID 62723 Utility function to search both IDs
	if (FindRowInDiagList(results.m_ICD9.m_nDiagCodesID, results.m_ICD10.m_nDiagCodesID) != NULL) {
		MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
		//clear the selection
		m_pDiagSearch->CurSel = NULL;
		return FALSE;
	}
	else {
		//it was not a duplicate, update the memory object.
		NexGEMMatchType matchType = nexgemtDone;
		if (results.m_ICD10.m_strCode.IsEmpty() && DiagSearchUtils::GetPreferenceSearchStyle() != eManagedICD9_Search)
		{
			matchType = nexgemtNoMatch;
		}
		//Add recore to datalist
		AddDiagCodeBySearchResultsData(results,  matchType);
	}
	// clear the selection
	m_pDiagSearch->CurSel = NULL;
	// (s.dhole 07\09\2014 8:56) - PLID 63114  Set dignosis column 
	UpdateDiagnosisListColumnSizes();
	return TRUE;
}

//(s.dhole 7/14/2014 2:17 PM ) - PLID 62593 update up/down arrow
void CEmrActionDlg::SelChangedActionDiagList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try	{ 
		UpdateDiagCodeArrows();
	}
	NxCatchAll(__FUNCTION__);
}


//(s.dhole 7/14/2014 2:17 PM ) - PLID 62593  update up/Down arrror stat
void CEmrActionDlg::UpdateDiagCodeArrows() {

	//otherwise, check the selection status
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pDiagList->CurSel;

	if (pCurSel == NULL) {
		GetDlgItem(IDC_BTN_DIAG_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_DIAG_DOWN)->EnableWindow(FALSE);
	}
	else {
		// Enable/Disable buttons according to what's possible based on the index
		if (pCurSel->GetPreviousRow() == NULL) {
			GetDlgItem(IDC_BTN_DIAG_UP)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_BTN_DIAG_UP)->EnableWindow(TRUE);
		}

		if (pCurSel->GetNextRow() == NULL) {
			GetDlgItem(IDC_BTN_DIAG_DOWN)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_BTN_DIAG_DOWN)->EnableWindow(TRUE);
		}
	}
}

//(s.dhole 7/17/2014 3:31 PM ) - PLID 62593
void CEmrActionDlg::LButtonDownActionDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if (!pRow)
	{
		return;
	}
	try {
		switch (nCol) {
			case adcProblemBitmap:
			{
				CProblemActionAry* pary = (CProblemActionAry*)VarLong(pRow->GetValue(adcProblemArray));
				CEmrProblemActionsDlg dlg(pary, m_SourceType, eaoDiagnosis/*eaoDiag*/, this);
				if (IDOK == dlg.DoModal()) {
					// Update the row appearance. The dialog will have already updated pary.
					UpdateDiagActionProblemRow(pRow, adcProblemBitmap, adcProblemArray);
				}
			}
			break;
			case adcICD9Code:
			{
				// Update/Select any missing ICD9 code 
				if (VarLong(pRow->GetValue(adcICD9DiagCodeID)) == -1)
				{
				
					if (VarLong(pRow->GetValue(adcICD9DiagCodeID)) == -1 && VarLong(pRow->GetValue(adcICD10DiagCodeID)) == -1) {
						return;
					}
					if (VarLong(pRow->GetValue(adcICD9DiagCodeID)) == -1) {	//we should only be here if this is -1
						LoadMissingICD9DiagCodeInfo(pRow);
					}

				}
			}
				break;
			case adcICD10Code:
			{
				// Update/Select any missing ICD10 code 
				switch (VarLong(pRow->GetValue(adcMatchtype))){
					case nexgemtManyMatch:
					{
						HandleDiagMatchColumn(pRow);
						//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 update column display
						UpdateDiagnosisListColumnSizes();
					}
					break;
					case nexgemtDone:
					{
						if (VarLong(pRow->GetValue(adcICD10DiagCodeID)) == -1)
						{
							UpdateDiagCode (pRow );
							//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 update column display
							UpdateDiagnosisListColumnSizes();
						}
					}
					break;
					
					case nexgemtNoMatch:
					{
						HandleDiagCodeColumn (pRow);
						//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 update column display
						UpdateDiagnosisListColumnSizes();
					}
						break;
				}
			}
				break;
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnLButtonDownActionDiagList");
}

//(s.dhole 7/14/2014 2:07 PM ) - PLID 62593 change To Datalist2
void CEmrActionDlg::RButtonUpActionDiagList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow)
		{
			return;
		}

		m_pDiagList->CurSel = pRow;
		//(s.dhole 7/29/2014 9:40 AM ) - PLID 63083 Set last selected datalist2
		m_pdlLastRightClickedDatalistDiag = m_pDiagList;
		//(s.dhole 7/29/2014 9:40 AM ) - PLID 63083 Reset last selected datalist1
		m_pdlLastRightClickedDatalist= NULL ;
		// (j.jones 2012-08-24 15:29) - PLID 52091 - track this EmrAction type
		// (b.savon 2014-07-15 15:50) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		m_LastRightClickedAction = eaoDiagnosis/*eaoDiag*/;
		if (pRow) {
			CMenu mPopup;
			mPopup.CreatePopupMenu();
			//(s.dhole 8/1/2014 10:36 AM ) - PLID 62597 show this option if icd 10 is missing and ICD 10 column is accessible
			if (VarLong(pRow->GetValue(adcICD10DiagCodeID)) == -1)
			{
				NXDATALIST2Lib::IColumnSettingsPtr pColDiagICD10Desc = m_pDiagList->GetColumn(adcICD10Code);
				long nStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthAuto;
				if (pColDiagICD10Desc->GetColumnStyle() == nStyle)
				{
					mPopup.AppendMenu(MF_BYPOSITION, ID_DIAG_SEARCH_FOR_ICD10, "Search for an ICD-10");
					mPopup.AppendMenu(MF_SEPARATOR);
				}

			}
			// (c.haag 2008-07-24 09:31) - PLID 30723 - Added right-click functionality for
			// consistency with other areas in the program
			mPopup.AppendMenu(MF_BYPOSITION, ID_CONFIGURE_DIAG_PROBLEMS, "Configure &Problems...");
			
			AppendCommonRightClickMenuOptions(&mPopup);
			mPopup.AppendMenu(MF_SEPARATOR);

			mPopup.AppendMenu(MF_BYPOSITION, ID_REMOVE_DIAG, "&Remove");
			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
		}
	}
	NxCatchAll("Error in CEmrActionDlg::OnRButtonUpActionDiagList");
}



//(s.dhole 7/17/2014 3:31 PM ) - PLID 62597
void CEmrActionDlg::UpdateDiagCode(NXDATALIST2Lib::IRowSettingsPtr pRow )
{
	//We are only ever looking at 1 ICD-9 code but we must use arrays to use the API functions
	Nx::SafeArray<BSTR> saICD9CodeIDs;
	saICD9CodeIDs.Add(AsString(pRow->GetValue(adcICD9DiagCodeID )));
	NexTech_Accessor::_NexGEMMatchResultsPtr codeMatchResults = GetAPI()->GetNexGEMMatchesFromICD9s(GetAPISubkey(), GetAPILoginToken(), saICD9CodeIDs);
	Nx::SafeArray<IUnknown*> codeResults = codeMatchResults->match;
	if (codeResults.GetSize() > 0)
	{
		NexTech_Accessor::_NexGEMMatchResultPtr matchedResult = codeResults.GetAt(0);
		if (matchedResult)
		{
			long nMatchStatus = MapMatchStatus(matchedResult->matchStatus);
			pRow->PutValue(adcMatchtype, nMatchStatus);
			DiagCodeList diagCodeList = GetDigCodeList(pRow);
			diagCodeList.matchType = (NexGEMMatchType)nMatchStatus;
			if (nMatchStatus == nexgemtDone) {
				// Make sure it's not already in the list
				if (FindRowInDiagList(AsLong(pRow->GetValue(adcICD9DiagCodeID)), AsLong(matchedResult->exactMatchedICD10->ID)) != NULL) {
					MessageBox("This diagnosis code is already in the list and will be automatically removed.", NULL, MB_OK | MB_ICONEXCLAMATION);
					m_pDiagList->PutCurSel(pRow);
					return;
				}
				diagCodeList.nICD10 = atol(matchedResult->exactMatchedICD10->ID);
				diagCodeList.strICD10Code = VarString(matchedResult->exactMatchedICD10->Code);
				diagCodeList.strICD10Description = VarString(matchedResult->exactMatchedICD10->description);
			}
			HandelDatalistRowUpdate(pRow, diagCodeList);
			// if a user adds codes in icd9 mode and another user loads in icd10 mode
			if (diagCodeList.matchType == nexgemtNoMatch || diagCodeList.matchType == nexgemtManyMatch){
				LButtonDownActionDiagList(pRow, adcICD10Code, -1, -1, -1);
			}
		}
	}
	else
	{
		MessageBox("No matches could be found for this code.", NULL);
	}
}

//(s.dhole 7/17/2014 3:31 PM ) - PLID 62597 load row data to struct
CEmrActionDlg::DiagCodeList CEmrActionDlg::GetDigCodeList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	DiagCodeList diagCodeList;
	diagCodeList.nActionID = pRow->GetValue(adcActionID);
	diagCodeList.nICD9 = pRow->GetValue(adcICD9DiagCodeID);
	diagCodeList.nICD10 = pRow->GetValue(adcICD10DiagCodeID);
	diagCodeList.strICD9Code = VarString(pRow->GetValue(adcICD9Code));
	diagCodeList.strICD10Code = AsString (pRow->GetValue(adcICD10Code));
	diagCodeList.strICD9Description = VarString(pRow->GetValue(adcICD9Desc));
	diagCodeList.strICD10Description = VarString(pRow->GetValue(adcICD10Desc));
	diagCodeList.matchType = (NexGEMMatchType)(VarLong(pRow->GetValue(adcMatchtype)));
	return  diagCodeList;

}

//(s.dhole 7/17/2014 3:31 PM ) - PLID 62597
void CEmrActionDlg::HandleDiagCodeColumn(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		CNexCodeDlg dlg(this);
		if (IDOK == dlg.DoModal()){
			long nDiagCodeID_ICD10 = dlg.GetDiagCodeID();
			////(s.dhole 7/18/2014 3:43 PM ) - PLID 62723 Make sure it's not already in the list
			if (FindRowInDiagList(AsLong(pRow->GetValue(adcICD9DiagCodeID)), nDiagCodeID_ICD10) != NULL) {
				MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
				return;
			}
			DiagCodeList diagCodeList = GetDigCodeList(pRow);
			// Update memory
			diagCodeList.nICD10  = nDiagCodeID_ICD10;
			diagCodeList.strICD10Code  = dlg.GetDiagCode();
			diagCodeList.strICD10Description  = dlg.GetDiagCodeDescription();
			diagCodeList.matchType= nexgemtDone;
			// Update the display
			HandelDatalistRowUpdate(pRow, diagCodeList);
		}

	}NxCatchAll(__FUNCTION__);
}



//(s.dhole 7/17/2014 3:31 PM ) - PLID 62597
void CEmrActionDlg::HandleDiagMatchColumn(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//Call the API to get the multi-matches
	CString strDiagCodeID;
	strDiagCodeID.Format("%li", AsLong(pRow->GetValue(adcICD9DiagCodeID)));
	NexTech_Accessor::_DiagnosisCodeCommitsPtr pCodes = GetAPI()->GetNexGEMMultiMatchesFromICD9(GetAPISubkey(), GetAPILoginToken(), AsBstr(strDiagCodeID));
	//Popup the single-select list
	//Populate input array
	Nx::SafeArray<IUnknown *> saryCodes = pCodes->Codes;

	// (c.haag 2015-07-14) - PLID 66023 - With the advent of custom crosswalks, what used to be multi-matches
	// can become single or even no matches. We will treat single matches as they were before (a dropdown will
	// appear with only one item in it); but if there are no matches then we will invite the user to build an ICD-10.
	if (0 == saryCodes.GetLength())
	{
		// If we get here, there are no longer any matches to choose from. Re-direct the user to build the code.
		if (IDYES == AfxMessageBox(R"(There are no longer any available matches to choose from.

Would you like to open NexCode to build an ICD-10 code?)", MB_ICONWARNING | MB_YESNO))
		{
			HandleDiagCodeColumn(pRow);
		}
	}
	else
	{
		// If we get here, there is at least one match to choose from. The only time there would ever be only one to choose
		// from is if there used to be multiple matching user-created custom crosswalks when the code was added...and only 
		// one crosswalk right now because the user deleted all but one of them. I don't think the user choosing from just one
		// dropdown option in this very rare case is an issue.
		int index = 0;
		CString strDropdown = "-1;< Select from possible matches >;";
		CString strCodeDrop;
		m_pDiagCodeDDMultiMatch->aryMultiMatchDD.RemoveAll();
		foreach(NexTech_Accessor::_DiagnosisCodeCommitPtr matchDiag, saryCodes){
			strCodeDrop.Format("%li;%s;", index++, AsString(matchDiag->Code) + " - " + AsString(matchDiag->description));
			strDropdown += strCodeDrop;
			m_pDiagCodeDDMultiMatch->aryMultiMatchDD.Add(matchDiag);
		}
		//Populate dropdown
		NXDATALIST2Lib::IFormatSettingsPtr pfsMultiMatch(__uuidof(NXDATALIST2Lib::FormatSettings));
		pfsMultiMatch->PutDataType(VT_BSTR);
		pfsMultiMatch->PutFieldType(NXDATALIST2Lib::cftComboSimple);
		pfsMultiMatch->PutConnection(_variant_t((LPDISPATCH)NULL));
		pfsMultiMatch->PutEditable(VARIANT_TRUE);
		pfsMultiMatch->PutComboSource(AsBstr(strDropdown));
		pRow->PutRefCellFormatOverride(adcICD10Code, pfsMultiMatch);
		pRow->PutValue(adcICD10Code, -1);
		pRow->PutValue(adcMatchtype, nexgemtManyMatch);
		m_pDiagList->StartEditing(pRow, adcICD10Code);
	}
}


//(s.dhole 7/17/2014 4:50 PM ) - PLID 62597 
void CEmrActionDlg::EditingFinishedActionDiagList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL){
			return;
		}
		if (nCol == adcICD10Code){
			if (bCommit == FALSE){
					DiagCodeList diagCodeList = GetDigCodeList(pRow);
					HandelDatalistRowUpdate(pRow, diagCodeList);
				return;
			}
			if (AsLong(varOldValue) == AsLong(varNewValue)){
				return;
			}

			NexTech_Accessor::_DiagnosisCodeCommitPtr selectedDiagCode = m_pDiagCodeDDMultiMatch->aryMultiMatchDD.GetAt(AsLong(varNewValue));
			//Save
			// Make sure the selected ICD10 is already imported into PracData
			CString strDiagCodeID_ICD10 = AsString((LPCTSTR)selectedDiagCode->ID);
			VARIANT_BOOL vbICD10 = selectedDiagCode->ICD10;
			if (strDiagCodeID_ICD10.IsEmpty() && vbICD10 != VARIANT_FALSE){
				//Need to Import
				Nx::SafeArray<IUnknown *> saryCodeToCreate;
				saryCodeToCreate.Add(selectedDiagCode);

				NexTech_Accessor::_DiagnosisCodesPtr pImportedDiagCode =
					GetAPI()->CreateDiagnosisCodes(
					GetAPISubkey(),
					GetAPILoginToken(),
					saryCodeToCreate
					);
				//(s.dhole 7/18/2014 3:43 PM ) - PLID 62724
				if (pImportedDiagCode == NULL){
					MessageBox("Practice was unable to save this diagnosis code.", "Practice", MB_OK | MB_ICONERROR);
					return;
				}

				Nx::SafeArray<IUnknown *> saryImportedCodes = pImportedDiagCode->Codes;
				if (saryImportedCodes.GetSize() != 1){
					MessageBox("Unexpected ICD-10 import value.", "Practice", MB_OK | MB_ICONERROR);
					return;
				}

				NexTech_Accessor::_DiagnosisCodePtr pImportedICD10 = saryImportedCodes.GetAt(0);
				selectedDiagCode->ID = pImportedICD10->ID;

			}
			else{
				// Make sure it's not already in the list
				if (FindRowInDiagList(AsLong(pRow->GetValue(adcICD9DiagCodeID)), AsLong(selectedDiagCode->ID)) != NULL) {
					MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
					DiagCodeList diagCodeList = GetDigCodeList(pRow);

					// Update the display
					HandelDatalistRowUpdate(pRow, diagCodeList);
					return;
				}
			}

			//it was not a duplicate

			//update the memory object.
			DiagCodeList diagCodeList = GetDigCodeList(pRow);
			diagCodeList.nICD10 = AsLong(selectedDiagCode->ID);
			
			diagCodeList.strICD10Code = AsString(selectedDiagCode->Code);
			diagCodeList.strICD10Description = AsString(selectedDiagCode->description);
			diagCodeList.matchType = nexgemtDone;
			// Update the display
			HandelDatalistRowUpdate(pRow, diagCodeList);
			//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114  Set dignosis column 
			UpdateDiagnosisListColumnSizes();
		}

	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 7/18/2014 3:39 PM ) - PLID 62597 load ICD9 code dialog to select matching code
void CEmrActionDlg::LoadMissingICD9DiagCodeInfo(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//first popup our selection dialog	
	CSelectDlg dlg(this);
	dlg.m_strTitle = "Select an ICD-9 Code";
	dlg.m_strCaption = "Select an ICD-9 Code.";
	dlg.m_strFromClause = "(SELECT ID, CodeNumber, CodeDesc From DiagCodes WHERE Active = 1 AND ICD10 = 0 ) Q";
	dlg.AddColumn("ID", "ID", FALSE, FALSE);
	dlg.AddColumn("CodeNumber", "Code", TRUE, FALSE, TRUE, -1, 1);
	dlg.AddColumn("CodeDesc", "Description", TRUE, FALSE, FALSE);
	if (IDOK == dlg.DoModal()) {
		long nDiagCodeID = VarLong(dlg.m_arSelectedValues[0], -1);

		if (nDiagCodeID != -1) {
			//we have to lookup the other information we need from data
			CString strCode, strDescription;
			_RecordsetPtr rsDiag = CreateParamRecordset("SELECT CodeNumber, CodeDesc FROM DiagCodes WHERE ID = {INT}", nDiagCodeID);
			if (!rsDiag->eof){
				strCode = AdoFldString(rsDiag->Fields, "CodeNumber");
				strDescription = AdoFldString(rsDiag->Fields, "CodeDesc");
			}
			else{
				MsgBox("The selected code is no longer available in your system. Please try again.");
				return;
			}

			//(s.dhole 7/18/2014 3:43 PM ) - PLID 62723 Look for duplicates
			if (FindRowInDiagList(nDiagCodeID, AsLong(pRow->GetValue(adcICD10DiagCodeID))) != NULL) {
				MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
				return;
			}

			DiagCodeList diagCodeList = GetDigCodeList(pRow);
			diagCodeList.nICD9 = nDiagCodeID;
			diagCodeList.strICD9Code = strCode;
			diagCodeList.strICD9Description = strDescription;
			diagCodeList.matchType = nexgemtDone;
			HandelDatalistRowUpdate(pRow, diagCodeList);
			//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114
			UpdateDiagnosisListColumnSizes();
		}
	}
}

//(s.dhole 8/1/2014 10:26 AM ) - PLID 62597 process 'Search for an ICD-10' action
void CEmrActionDlg::OnSearchMMatchingICD10()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagList->GetCurSel();
		if (pRow != nullptr)
		{
			switch (VarLong(pRow->GetValue(adcMatchtype))){
				case nexgemtManyMatch:
				{
					HandleDiagMatchColumn(pRow);
					//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 update column display
					UpdateDiagnosisListColumnSizes();
				}
					break;
				case nexgemtDone:
				{
					if (VarLong(pRow->GetValue(adcICD10DiagCodeID)) == -1)
					{
						UpdateDiagCode(pRow);
						//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 update column display
						UpdateDiagnosisListColumnSizes();
					}
				}
				break;
				case nexgemtNoMatch:
				{
					HandleDiagCodeColumn(pRow);
					//(s.dhole 7/30/2014 3:55 PM ) - PLID 63114 update column display
					UpdateDiagnosisListColumnSizes();
				}
				break;
				default:
				{ 
				}
				break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-30 10:39) - PLID 67758
void CEmrActionDlg::SelChosenNxdlAddMedAction(LPDISPATCH lpRow)
{
	try {

		AddMedicationFromSearch(lpRow);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2015-12-30 10:39) - PLID 67758
void CEmrActionDlg::AddMedicationFromSearch(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	if (pRow != NULL) {

		if (VarLong(pRow->GetValue(mrcMedicationID)) == NO_RESULTS_ROW) {
			return;
		}

		//Dont add dupes
		long nRowIdx = m_pMedicationList->FindByColumn(alcName, pRow->GetValue(mrcMedicationName), NULL, VARIANT_FALSE);
		if (nRowIdx > -1) {
			return;
		}

		//Pull Values from selection
		long nMedicationID = VarLong(pRow->GetValue(mrcMedicationID), -1);
		long nFDBID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);
		CString strMedname = VarString(pRow->GetValue(mrcMedicationName), "");
		BOOL bFirstDataBankOutOfDate = VarBool(pRow->GetValue(mrcFDBOutOfDate), FALSE) && nFDBID > 0;

		//If the medicationid is -1 (i.e. DrugList.ID), we need to import it from FDB
		long nNewDrugListID = nMedicationID;
		if (nMedicationID == -1) {
			ImportMedication(nFDBID, strMedname, nNewDrugListID);
		}

		// Update Med List
		{
			AddMedicationToList(nNewDrugListID, strMedname, nFDBID, bFirstDataBankOutOfDate);
			OnSelChangedMedicationList(m_pMedicationList->CurSel);
		}
	}
}

// (b.savon 2015-12-30 10:39) - PLID 67758
void CEmrActionDlg::AddMedicationToList(
	const long &nDrugListID, const CString& strMedName, const long &nFDBID, const BOOL &bFDBOutOfDate, 
	const long &nActionID /* = -1 */, const VARIANT_BOOL vbPopup /* = VARIANT_FALSE */, const CProblemActionAry* paryActionProblems /* = new CProblemActionAry */, const long &nSortOrder /* = -1*/)
{
	try {
		NXDATALISTLib::IRowSettingsPtr pRowToAdd;
		pRowToAdd = m_pMedicationList->GetRow(-1);
		pRowToAdd->PutValue(alcActionID, nActionID);
		pRowToAdd->PutValue(alcID, nDrugListID);
		pRowToAdd->PutValue(alcName, AsBstr(strMedName));
		pRowToAdd->PutValue(alcName2, _bstr_t(""));
		pRowToAdd->PutValue(alcSortOrder, nSortOrder == -1 ? (m_pMedicationList->GetRowCount() + 1) : nSortOrder);
		pRowToAdd->PutValue(alcPopup, vbPopup);
		pRowToAdd->PutValue(alcLabAnatomyIDs, _bstr_t(""));
		pRowToAdd->PutValue(alcProblemArray, (long)paryActionProblems);
		UpdateActionProblemRow(pRowToAdd, alcProblemBitmap, alcProblemArray);

		if (nFDBID != -1) {
			if (nFDBID > 0 && bFDBOutOfDate) {
				pRowToAdd->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
			}
			else {
				pRowToAdd->PutBackColor(ERX_IMPORTED_COLOR);
			}
		}

		m_pMedicationList->AddRow(pRowToAdd);
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-25 15:18) - PLID 67998 - added checkbox to include free text meds in the medication search
void CEmrActionDlg::OnCheckIncludeFreeTextMeds()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextMeds.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, false)) {
				//they changed their minds
				m_checkIncludeFreeTextMeds.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextMeds.SetFocus();
		}

		//reflect their choice in the search results
		ResetMedicationSearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-25 15:26) - PLID 67998 - resets the medication search provider based
// on the value of the 'include free text' checkbox
void CEmrActionDlg::ResetMedicationSearchProvider()
{
	try {

		bool bIncludeFDBMedsOnly = m_checkIncludeFreeTextMeds.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBMedsOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlMedicationSearch = BindMedicationSearchListCtrl(this, IDC_NXDL_ADD_MED_ACTION, GetRemoteData(), false, bIncludeFDBMedsOnly);

	}NxCatchAll(__FUNCTION__);
}