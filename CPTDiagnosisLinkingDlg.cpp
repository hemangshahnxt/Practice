// CPTDiagnosisLinkingDlg.cpp : implementation file
// (r.gonet 02/20/2014) - PLID 60778 - Renamed the file to remove the reference to ICD9
//   I also renamed the class, several functions, and control macro identifiers here. I 
//   haven't put a PLID comment on all of them since that would just be clutter and frankly 
//   is a worthless comment.
//

#include "stdafx.h"
#include "CPTDiagnosisLinkingDlg.h"
#include "MultiSelectDlg.h"
#include "DiagSearchUtils.h"
#include "AdministratorRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_DELETE_CPT_DIAGNOSIS_LINK	47886

// (j.jones 2013-04-09 16:43) - PLID 56126 - added enum for the product combo
enum ProductComboColumn {

	pccID = 0,
	pccName,
	pccInsCode,
};

/////////////////////////////////////////////////////////////////////////////
// CCPTDiagnosisLinkingDlg dialog


CCPTDiagnosisLinkingDlg::CCPTDiagnosisLinkingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCPTDiagnosisLinkingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCPTDiagnosisLinkingDlg)
	//}}AFX_DATA_INIT
	m_bHasInventory = true;
}


void CCPTDiagnosisLinkingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCPTDiagnosisLinkingDlg)
	DDX_Control(pDX, IDC_RADIO_CPT_LINK, m_btnLinkCPT);
	DDX_Control(pDX, IDC_RADIO_DIAGNOSIS_LINK, m_btnLinkDiagnosis);
	DDX_Control(pDX, IDC_RADIO_VIEW_LINKED_CODES, m_btnViewLinked);
	DDX_Control(pDX, IDC_RADIO_VIEW_BLOCKED_CODES, m_btnViewBlocked);
	DDX_Control(pDX, IDC_LINKCODES, m_btnEnableLinking);
	DDX_Control(pDX, IDC_LABEL_CPT_DIAGNOSIS_DROPDOWN, m_nxstaticLabelCptDiagnosisDropdown);
	DDX_Control(pDX, IDC_RADIO_SHOW_ICD9_CODES, m_radioShowICD9Codes);
	DDX_Control(pDX, IDC_RADIO_SHOW_ICD10_CODES, m_radioShowICD10Codes);
	DDX_Control(pDX, IDC_LINKED_CODE_LIST_LABEL, m_nxstaticLinkedCodeListLabel);
	DDX_Control(pDX, IDC_BTN_ADD_NEW_CPT_DIAGNOSIS_LINK, m_btnAddNewCptDiagnosisLink);
	DDX_Control(pDX, IDC_BTN_DELETE_CPT_DIAGNOSIS_LINK, m_btnDeleteCptDiagnosisLink);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_CHECK_ALLOW_DYNAMIC_LINKING, m_checkAllowDynamicLinking);
	DDX_Control(pDX, IDC_RADIO_CPT_LINK_SELECT, m_radioSelectCPTCodes);
	DDX_Control(pDX, IDC_RADIO_PRODUCT_LINK_SELECT, m_radioSelectProducts);
	DDX_Control(pDX, IDC_PRODUCT_INFO_LABEL, m_staticProductInfoLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCPTDiagnosisLinkingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCPTDiagnosisLinkingDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_CPT_DIAGNOSIS_LINK, OnBtnAddNewCptDiagnosisLink)
	ON_BN_CLICKED(IDC_BTN_DELETE_CPT_DIAGNOSIS_LINK, OnBtnDeleteCptDiagnosisLink)
	ON_BN_CLICKED(IDC_LINKCODES, OnLinkcodes)
	ON_BN_CLICKED(IDC_RADIO_SHOW_ICD9_CODES, OnRadioShowICD9Codes)
	ON_BN_CLICKED(IDC_RADIO_SHOW_ICD10_CODES, OnRadioShowICD10Codes)
	ON_BN_CLICKED(IDC_RADIO_CPT_LINK, OnRadioCptLink)
	ON_BN_CLICKED(IDC_RADIO_DIAGNOSIS_LINK, OnRadioDiagnosisLink)
	ON_BN_CLICKED(IDC_RADIO_VIEW_LINKED_CODES, OnRadioViewLinkedCodes)
	ON_BN_CLICKED(IDC_RADIO_VIEW_BLOCKED_CODES, OnRadioViewBlockedCodes)
	ON_BN_CLICKED(IDC_CHECK_ALLOW_DYNAMIC_LINKING, OnCheckAllowDynamicLinking)
	ON_BN_CLICKED(IDC_RADIO_PRODUCT_LINK_SELECT, OnRadioProductLinkSelect)
	ON_BN_CLICKED(IDC_RADIO_CPT_LINK_SELECT, OnRadioCptLinkSelect)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCPTDiagnosisLinkingDlg message handlers

BOOL CCPTDiagnosisLinkingDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
	
		// (z.manning, 04/30/2008) - PLID 29852 - Set button styles
		m_btnAddNewCptDiagnosisLink.AutoSet(NXB_NEW);
		m_btnDeleteCptDiagnosisLink.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
	
		if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			m_bHasInventory = false;
		}
	
		m_CPTCombo = BindNxDataListCtrl(this,IDC_CPT_LINK_SELECT_COMBO,GetRemoteData(),true);
		// (j.jones 2013-04-09 16:43) - PLID 56126 - added product combo
		m_ProductCombo = BindNxDataList2Ctrl(this,IDC_PRODUCT_LINK_SELECT_COMBO,GetRemoteData(), m_bHasInventory);
		m_DiagnosisCombo = BindNxDataListCtrl(this,IDC_DIAGNOSIS_LINK_SELECT_COMBO,GetRemoteData(),true);
		m_ServiceList = BindNxDataListCtrl(this,IDC_LINKED_CPT_LIST,GetRemoteData(),false);
		m_DiagnosisList = BindNxDataListCtrl(this,IDC_LINKED_DIAGNOSIS_LIST,GetRemoteData(),false);
	
		// (j.jones 2012-12-12 14:46) - PLID 47773 - added a cache, it never had one before
		g_propManager.CachePropertiesInBulk("BillingModuleDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'LinkCPTDiagnosisCodes' "
			"OR Name = 'AllowDynamicCPTICDLinking' "
			")",
			_Q(GetCurrentUserName()));
		
		if(GetRemotePropertyInt("LinkCPTDiagnosisCodes",0,0,"<None>",TRUE) == 1) {
			CheckDlgButton(IDC_LINKCODES,TRUE);
		} else {
			CheckDlgButton(IDC_LINKCODES,FALSE);
		}
	
		// (j.jones 2012-12-12 15:52) - PLID 47773 - added ability to toggle the prompt in the bill
		m_checkAllowDynamicLinking.SetCheck(GetRemotePropertyInt("AllowDynamicCPTICDLinking", 1, 0, "<None>", true) == 1);
	
		CheckDlgButton(IDC_RADIO_CPT_LINK,TRUE);
		CheckDlgButton(IDC_RADIO_VIEW_LINKED_CODES,TRUE);
		m_radioSelectCPTCodes.SetCheck(TRUE);
		// (r.gonet 02/20/2014) - PLID 60778 - There are some radio buttons that control
		// what codes (ICD-9 or ICD-10) show up in the diagnosis combo box. The default is
		// controlled by the global preference on how to show diagnosis codes.
		DiagCodeSearchStyle eDiagCodeStyle = DiagSearchUtils::GetPreferenceSearchStyle();
		if(eDiagCodeStyle == eManagedICD9_Search) {
			m_radioShowICD9Codes.SetCheck(BST_CHECKED);
		} else {
			m_radioShowICD10Codes.SetCheck(BST_CHECKED);
		}
		// (r.gonet 02/20/2014) - PLID 60778 - Need to reload the datalist now since it depends on the radio buttons
		ReloadDiagnosisCombo();
		OnRadioCPTDiagnosisChange(TRUE);
	} NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 02/20/2014) - PLID 60778 - Reloads the content of the diagnosis combo box.
// Fills with ICD-10 codes or ICD-9 codes depending on which radio button is checked above it.
void CCPTDiagnosisLinkingDlg::ReloadDiagnosisCombo()
{
	CString strWhereClause = "DiagCodes.Active = 1";
	if(IsDlgButtonChecked(IDC_RADIO_SHOW_ICD9_CODES)) {
		strWhereClause += " AND DiagCodes.ICD10 = 0";
	} else {
		strWhereClause += " AND DiagCodes.ICD10 = 1";
	}
	m_DiagnosisCombo->WhereClause = _bstr_t(strWhereClause);
	m_DiagnosisCombo->Requery();
}

void CCPTDiagnosisLinkingDlg::OnBtnAddNewCptDiagnosisLink() 
{
	try {

		BOOL bViewLinkedCodes = TRUE;
	
		//find out if we are looking at linked codes or blocked codes
		if(IsDlgButtonChecked(IDC_RADIO_VIEW_BLOCKED_CODES))
			bViewLinkedCodes = FALSE;

		if(IsDlgButtonChecked(IDC_RADIO_CPT_LINK)) {

			// (j.jones 2013-04-10 10:12) - PLID 56126 - supported products
			if(m_radioSelectCPTCodes.GetCheck()) {
				if(m_CPTCombo->CurSel == -1) {
					AfxMessageBox("Please select a service code from the list before continuing.");
					return;
				}
			}
			else {
				if(m_ProductCombo->CurSel == NULL) {
					AfxMessageBox("Please select an inventory item from the list before continuing.");
					return;
				}
			}

			if(bViewLinkedCodes) {
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "DiagCodes");
				for(int i=0;i<m_DiagnosisList->GetRowCount();i++) {
					dlg.PreSelect(VarLong(m_DiagnosisList->GetValue(i,0)));
				}

				// (r.gonet 02/20/2014) - PLID 60778 - 1: After selecting a service code or product in dropdown, 
				// the Add/Edit Links button should pop-up with a window of the managed ICD-9 and ICD-10 lists 
				// mingled together based on global search preference.
				DiagCodeSearchStyle eSearchStyle = DiagSearchUtils::GetPreferenceSearchStyle();
				CString strWhereClause = "Active = 1";
				switch(eSearchStyle) {
				case eICD9_10_Crosswalk:
					// (r.gonet 02/20/2014) - PLID 60778 - We are going to display both ICD-9 codes and ICD-10 codes in the same list.
					break;
				case eManagedICD9_Search:
					// (r.gonet 02/20/2014) - PLID 60778 - We are going to display only ICD-9 codes.
					strWhereClause += " AND ICD10 = 0";
					break;
				case eManagedICD10_Search:
					// (r.gonet 02/20/2014) - PLID 60778 - We are going to display only ICD-10 codes.
					strWhereClause += " AND ICD10 = 1";
					break;
				default:
					ThrowNxException("%s : Unhandled diagnosis code search style (%li)", __FUNCTION__, eSearchStyle);
					break;
				}
				int res = dlg.Open("DiagCodes", strWhereClause, "ID", "CodeNumber + ' - ' + CodeDesc", "Select Diag Codes.");

				if(res == IDCANCEL) {
					return;
				}

				CString strDiagIDs = dlg.GetMultiSelectIDString();

				long ServiceID = -1;
				// (j.jones 2013-04-10 10:12) - PLID 56126 - supported products
				if(m_radioSelectCPTCodes.GetCheck()) {
					ServiceID = VarLong(m_CPTCombo->GetValue(m_CPTCombo->CurSel,0));
				}
				else {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProductCombo->CurSel;
					ServiceID = VarLong(pRow->GetValue(pccID));
				}

				strDiagIDs = strDiagIDs + " ";	//add a space for parsing
				strDiagIDs.Remove(',');
				strDiagIDs.TrimLeft();	//it does not have to have diag codes

				ExecuteParamSql("DELETE FROM CPTDiagnosisGroupsT WHERE ServiceID = {INT}", ServiceID);

				if(!strDiagIDs.IsEmpty()) {
					ExecuteParamSql("INSERT INTO CPTDiagnosisGroupsT (ServiceID, DiagCodeID) "
						"SELECT {INT}, ID FROM DiagCodes WHERE ID IN ({INTSTRING})", ServiceID, strDiagIDs);
				}

				m_DiagnosisList->Requery();
			}
			else {
				// (j.jones 2013-04-10 10:18) - PLID 56126 - changed to support products too
				CMultiSelectDlg dlg(this, "CPTCodeT"); //this is a misnomer, it's just used to track the same window size for all like selections
				for(int i=0;i<m_ServiceList->GetRowCount();i++)
					dlg.PreSelect(VarLong(m_ServiceList->GetValue(i,0)));

				CString strWhere = "Active = 1 AND (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null)";
				CString strLabel = "Select Service Codes / Products";
				if(!m_bHasInventory) {
					strWhere = "Active = 1 AND CPTCodeT.ID Is Not Null";
					strLabel = "Select Service Codes";
				}

				int res = dlg.Open("ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID",
					strWhere, "ServiceT.ID",
					"CASE WHEN ProductT.ID Is Not Null THEN "
					"	ProductT.InsCode + CASE WHEN Coalesce(ProductT.InsCode,'') = '' THEN '' ELSE ' - ' END "
					"ELSE CPTCodeT.Code + ' - ' END "
					"+ ServiceT.Name", strLabel);

				if(res == IDCANCEL)
					return;

				CString strCPTIDs = dlg.GetMultiSelectIDString();

				// (j.jones 2013-04-10 10:12) - PLID 56126 - supported products
				long ServiceID = -1;
				if(m_radioSelectCPTCodes.GetCheck()) {
					ServiceID = VarLong(m_CPTCombo->GetValue(m_CPTCombo->CurSel,0));
				}
				else {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProductCombo->CurSel;
					ServiceID = VarLong(pRow->GetValue(pccID));
				}

				ExecuteParamSql("DELETE FROM BlockedCPTCodesT WHERE ServiceID1 = {INT} OR ServiceID2 = {INT}", ServiceID, ServiceID);

				if(!strCPTIDs.IsEmpty()) {
					ExecuteParamSql("INSERT INTO BlockedCPTCodesT (ServiceID1, ServiceID2) "
						"SELECT {INT}, ID FROM ServiceT WHERE ID IN ({INTSTRING}) AND ID <> {INT}", ServiceID, strCPTIDs, ServiceID);
				}

				m_ServiceList->Requery();
			}
		}
		else if(IsDlgButtonChecked(IDC_RADIO_DIAGNOSIS_LINK)) {

			if(m_DiagnosisCombo->CurSel == -1) {
				AfxMessageBox("Please select a diagnosis code from the list before continuing.");
				return;
			}

			if(bViewLinkedCodes) {
				// (j.jones 2013-04-10 10:18) - PLID 56126 - changed to support products too
				CMultiSelectDlg dlg(this, "CPTCodeT"); //this is a misnomer, it's just used to track the same window size for all like selections
				for(int i=0;i<m_ServiceList->GetRowCount();i++) {
					dlg.PreSelect(VarLong(m_ServiceList->GetValue(i,0)));
				}

				CString strWhere = "Active = 1 AND (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null)";
				CString strLabel = "Select Service Codes / Products";
				if(!m_bHasInventory) {
					strWhere = "Active = 1 AND CPTCodeT.ID Is Not Null";
					strLabel = "Select Service Codes";
				}

				int res = dlg.Open("ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID",
					strWhere, "ServiceT.ID",
					"CASE WHEN ProductT.ID Is Not Null THEN "
					"	ProductT.InsCode + CASE WHEN Coalesce(ProductT.InsCode,'') = '' THEN '' ELSE ' - ' END "
					"ELSE CPTCodeT.Code + ' - ' END "
					"+ ServiceT.Name", strLabel);

				if(res == IDCANCEL) {
					return;
				}

				CString strCPTIDs = dlg.GetMultiSelectIDString();

				long DiagCodeID = VarLong(m_DiagnosisCombo->GetValue(m_DiagnosisCombo->CurSel,0),-1);

				ExecuteParamSql("DELETE FROM CPTDiagnosisGroupsT WHERE DiagCodeID = {INT}", DiagCodeID);


				if(!strCPTIDs.IsEmpty()) {
					ExecuteParamSql("INSERT INTO CPTDiagnosisGroupsT (DiagCodeID, ServiceID) "
						"SELECT {INT}, ID FROM ServiceT WHERE ID IN ({INTSTRING})", DiagCodeID, strCPTIDs);
				}

				m_ServiceList->Requery();
			}
			else {
				// (r.gonet 02/20/2014) - PLID 60778 - Build the where clause based on whether they have
				// selected to show ICD9s or ICD10s
				CString strWhereClause = "Active = 1";
				if(m_radioShowICD9Codes.GetCheck() == BST_CHECKED) {
					// (r.gonet 02/20/2014) - PLID 60778 - 3: If only an ICD-9 is selected, the pop-up box 
					// should only display ICD-9 managed list when clicking on the Add/ Edit Blocks button.
					strWhereClause += " AND ICD10 = 0";
				} else {
					// (r.gonet 02/20/2014) - PLID 60778 - If only an ICD-10 is selected, the pop-up box should only display ICD-10 managed list when clicking on the Add/ Edit Blocks button.
					strWhereClause += " AND ICD10 = 1";
				}

				CMultiSelectDlg dlg(this, "DiagCodes");
				for(int i=0;i<m_DiagnosisList->GetRowCount();i++) {
					dlg.PreSelect(VarLong(m_DiagnosisList->GetValue(i,0)));
				}

				int res = dlg.Open("DiagCodes", strWhereClause, "ID", "CodeNumber + ' - ' + CodeDesc", "Select Diag Codes.");

				if(res == IDCANCEL) {
					return;
				}

				CString strDiagIDs = dlg.GetMultiSelectIDString();

				long DiagCodeID = VarLong(m_DiagnosisCombo->GetValue(m_DiagnosisCombo->CurSel,0),-1);

				ExecuteParamSql("DELETE FROM BlockedDiagnosisCodesT WHERE DiagCodeID1 = {INT} OR DiagCodeID2 = {INT}", DiagCodeID, DiagCodeID);
				
				if(!strDiagIDs.IsEmpty()) {
					ExecuteParamSql("INSERT INTO BlockedDiagnosisCodesT (DiagCodeID1, DiagCodeID2) "
						"SELECT {INT}, ID FROM DiagCodes WHERE ID IN ({INTSTRING}) AND ID <> {INT}", DiagCodeID, strDiagIDs, DiagCodeID);
				}

				m_DiagnosisList->Requery();
			}
		}		

	}NxCatchAll("Error adding/editing links.");	
}

void CCPTDiagnosisLinkingDlg::OnBtnDeleteCptDiagnosisLink() 
{
	try {

		CString str;

		BOOL bViewLinkedCodes = TRUE;
	
		//find out if we are looking at linked codes or blocked codes
		if(IsDlgButtonChecked(IDC_RADIO_VIEW_BLOCKED_CODES))
			bViewLinkedCodes = FALSE;

		if(IsDlgButtonChecked(IDC_RADIO_CPT_LINK)) {
			if(bViewLinkedCodes) {
				if(m_DiagnosisList->CurSel == -1) {
					AfxMessageBox("You must first select a diagnosis code from the list in order to remove the link.");
					return;
				}
			}
			else {
				if(m_ServiceList->CurSel == -1) {
					AfxMessageBox("You must first select a service code from the list in order to remove the block.");
					return;
				}
			}
			
			if(bViewLinkedCodes) {
				str = "Are you sure you want to remove the link between\n"
					"the selected service code and diagnosis code?";
			} else {
				str = "Are you sure you want to remove the block between the selected service codes?";
			}

			if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			if(bViewLinkedCodes) {
				// (j.jones 2013-04-10 10:12) - PLID 56126 - supported products
				long ServiceID = -1;
				if(m_radioSelectCPTCodes.GetCheck()) {
					ServiceID = VarLong(m_CPTCombo->GetValue(m_CPTCombo->CurSel,0));
				}
				else {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProductCombo->CurSel;
					ServiceID = VarLong(pRow->GetValue(pccID));
				}
				long DiagCodeID = VarLong(m_DiagnosisList->GetValue(m_DiagnosisList->CurSel,0),-1);

				ExecuteParamSql("DELETE FROM CPTDiagnosisGroupsT WHERE ServiceID = {INT} AND DiagCodeID = {INT}", ServiceID, DiagCodeID);
				m_DiagnosisList->RemoveRow(m_DiagnosisList->CurSel);
			}
			else {
				// (j.jones 2013-04-10 10:12) - PLID 56126 - supported products
				long ServiceID1 = -1;
				if(m_radioSelectCPTCodes.GetCheck()) {
					ServiceID1 = VarLong(m_CPTCombo->GetValue(m_CPTCombo->CurSel,0));
				}
				else {
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProductCombo->CurSel;
					ServiceID1 = VarLong(pRow->GetValue(pccID));
				}
				long ServiceID2 = VarLong(m_ServiceList->GetValue(m_ServiceList->CurSel,0),-1);
	
				ExecuteParamSql("DELETE FROM BlockedCPTCodesT WHERE (ServiceID1 = {INT} AND ServiceID2 = {INT}) OR (ServiceID1 = {INT} AND ServiceID2 = {INT})", ServiceID1, ServiceID2, ServiceID2, ServiceID1);
				m_ServiceList->RemoveRow(m_ServiceList->CurSel);
			}
		}
		else if(IsDlgButtonChecked(IDC_RADIO_DIAGNOSIS_LINK)) {
			if(bViewLinkedCodes) {
				if(m_ServiceList->CurSel == -1) {				
					AfxMessageBox("You must first select a service code from the list in order to remove the link.");
					return;
				}
			}
			else {
				if(m_DiagnosisList->CurSel == -1) {
					AfxMessageBox("You must first select a diagnosis code from the list in order to remove the block.");
					return;
				}
			}

			if(bViewLinkedCodes) {
				str = "Are you sure you want to remove the link between\n"
					"the selected Diagnosis Code and Service Code?";
			} else {
				str = "Are you sure you want to remove the block between the selected diagnosis codes?";
			}

			if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			if(m_DiagnosisCombo->CurSel == -1) {
				ThrowNxException("%s : Attempted to delete a linked code but no diagnosis code was selected from the Diagnosis Codes combo box!", __FUNCTION__);
			}
			if(bViewLinkedCodes) {
				long DiagCodeID = VarLong(m_DiagnosisCombo->GetValue(m_DiagnosisCombo->CurSel,0),-1);
				long ServiceID = VarLong(m_ServiceList->GetValue(m_ServiceList->CurSel,0),-1);

				ExecuteParamSql("DELETE FROM CPTDiagnosisGroupsT WHERE ServiceID = {INT} AND DiagCodeID = {INT}", ServiceID, DiagCodeID);
				m_ServiceList->RemoveRow(m_ServiceList->CurSel);
			}
			else {
				long DiagCodeID1 = VarLong(m_DiagnosisCombo->GetValue(m_DiagnosisCombo->CurSel,0),-1);
				long DiagCodeID2 = VarLong(m_DiagnosisList->GetValue(m_DiagnosisList->CurSel,0),-1);
	
				ExecuteParamSql("DELETE FROM BlockedDiagnosisCodesT WHERE (DiagCodeID1 = {INT} AND DiagCodeID2 = {INT}) OR (DiagCodeID1 = {INT} AND DiagCodeID2 = {INT})", DiagCodeID1, DiagCodeID2, DiagCodeID2, DiagCodeID1);
				m_DiagnosisList->RemoveRow(m_DiagnosisList->CurSel);
			}
		}

	}NxCatchAll("Error deleting Service/Diagnosis link.");	
}

BEGIN_EVENTSINK_MAP(CCPTDiagnosisLinkingDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCPTDiagnosisLinkingDlg)
	ON_EVENT(CCPTDiagnosisLinkingDlg, IDC_LINKED_CPT_LIST, 6 /* RButtonDown */, OnRButtonDownLinkedCptList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCPTDiagnosisLinkingDlg, IDC_LINKED_DIAGNOSIS_LIST, 6 /* RButtonDown */, OnRButtonDownLinkedDiagnosisList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCPTDiagnosisLinkingDlg, IDC_CPT_LINK_SELECT_COMBO, 16 /* SelChosen */, OnSelChosenCptLinkSelectCombo, VTS_I4)
	ON_EVENT(CCPTDiagnosisLinkingDlg, IDC_DIAGNOSIS_LINK_SELECT_COMBO, 16 /* SelChosen */, OnSelChosenDiagnosisLinkSelectCombo, VTS_I4)
	ON_EVENT(CCPTDiagnosisLinkingDlg, IDC_PRODUCT_LINK_SELECT_COMBO, 16 /* SelChosen */, OnSelChosenProductLinkSelectCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CCPTDiagnosisLinkingDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch(wParam) {
			case ID_DELETE_CPT_DIAGNOSIS_LINK:
				OnBtnDeleteCptDiagnosisLink();
			break;
		}
	} NxCatchAll(__FUNCTION__);
	
	return CDialog::OnCommand(wParam, lParam);
}

void CCPTDiagnosisLinkingDlg::OnRButtonDownLinkedCptList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_ServiceList->CurSel = nRow;
	
		CMenu pMenu;
	
		if(nRow != -1) {
			pMenu.CreatePopupMenu();
			pMenu.InsertMenu(0, MF_BYPOSITION, ID_DELETE_CPT_DIAGNOSIS_LINK, "Delete Link");
			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	} NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnRButtonDownLinkedDiagnosisList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		m_DiagnosisList->CurSel = nRow;
	
		CMenu pMenu;
	
		if(nRow != -1) {
			pMenu.CreatePopupMenu();
			pMenu.InsertMenu(0, MF_BYPOSITION, ID_DELETE_CPT_DIAGNOSIS_LINK, "Delete Link");
			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	} NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnSelChosenCptLinkSelectCombo(long nRow) 
{
	try {

		BOOL bViewLinkedCodes = TRUE;
	
		//find out if we are looking at linked codes or blocked codes
		if(IsDlgButtonChecked(IDC_RADIO_VIEW_BLOCKED_CODES))
			bViewLinkedCodes = FALSE;

		if(nRow == -1) {
			if(bViewLinkedCodes)
				m_DiagnosisList->Clear();
			return;
		}
		
		long ServiceID = VarLong(m_CPTCombo->GetValue(m_CPTCombo->CurSel,0));
		CString str;

		if(bViewLinkedCodes) {			
			m_DiagnosisList->FromClause = _bstr_t("CPTDiagnosisGroupsT INNER JOIN DiagCodes ON CPTDiagnosisGroupsT.DiagCodeID = DiagCodes.ID");			
			str.Format("Active = 1 AND ServiceID = %li",ServiceID);
			m_DiagnosisList->WhereClause = _bstr_t(str);
			m_DiagnosisList->Requery();
		}
		else {
			m_ServiceList->FromClause = _bstr_t("ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID");
			str.Format("Active = 1 AND (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null) AND (ServiceT.ID IN (SELECT ServiceID1 FROM BlockedCPTCodesT WHERE ServiceID2 = %li) OR ServiceT.ID IN (SELECT ServiceID2 FROM BlockedCPTCodesT WHERE ServiceID1 = %li))",ServiceID,ServiceID);
			m_ServiceList->WhereClause = _bstr_t(str);
			m_ServiceList->Requery();
		}

	}NxCatchAll("Error listing diagnosis codes by service code.");
}

void CCPTDiagnosisLinkingDlg::OnSelChosenDiagnosisLinkSelectCombo(long nRow) 
{
	try {

		BOOL bViewLinkedCodes = TRUE;
	
		//find out if we are looking at linked codes or blocked codes
		if(IsDlgButtonChecked(IDC_RADIO_VIEW_BLOCKED_CODES))
			bViewLinkedCodes = FALSE;

		if(nRow == -1) {
			if(bViewLinkedCodes)
				m_ServiceList->Clear();
			return;
		}

		long DiagCodeID = VarLong(m_DiagnosisCombo->GetValue(nRow,0),-1);
		CString str;

		if(bViewLinkedCodes) {
			m_ServiceList->FromClause = _bstr_t("CPTDiagnosisGroupsT INNER JOIN ServiceT ON CPTDiagnosisGroupsT.ServiceID = ServiceT.ID LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID");
			str.Format("Active = 1 AND (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null) AND DiagCodeID = %li",DiagCodeID);
			m_ServiceList->WhereClause = _bstr_t(str);
			m_ServiceList->Requery();
		}
		else {
			m_DiagnosisList->FromClause = _bstr_t("DiagCodes");			
			str.Format("Active = 1 AND (DiagCodes.ID IN (SELECT DiagCodeID1 FROM BlockedDiagnosisCodesT WHERE DiagCodeID2 = %li) OR DiagCodes.ID IN (SELECT DiagCodeID2 FROM BlockedDiagnosisCodesT WHERE DiagCodeID1 = %li))",DiagCodeID,DiagCodeID);
			m_DiagnosisList->WhereClause = _bstr_t(str);
			m_DiagnosisList->Requery();
		}

	}NxCatchAll("Error listing service codes by diagnosis.");
}

void CCPTDiagnosisLinkingDlg::OnLinkcodes() 
{
	try {
		if (IsDlgButtonChecked(IDC_LINKCODES)) {
			SetRemotePropertyInt("LinkCPTDiagnosisCodes",1,0,"<None>");
		} else {
			SetRemotePropertyInt("LinkCPTDiagnosisCodes",0,0,"<None>");
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 02/20/2014) - PLID 60778 - Handler for when the user clicks the ICD-9 radio
// button above the diagnosis combo. Forces the diagnosis combo to load only ICD-9 codes
void CCPTDiagnosisLinkingDlg::OnRadioShowICD9Codes() 
{
	try {
		ReloadDiagnosisCombo();
		m_DiagnosisList->Clear();
		m_ServiceList->Clear();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 02/20/2014) - PLID 60778 - Handler for when the user clicks the ICD-10 radio
// button above the diagnosis combo. Forces the diagnosis combo to load only ICD-10 codes
void CCPTDiagnosisLinkingDlg::OnRadioShowICD10Codes() 
{
	try {
		ReloadDiagnosisCombo();
		m_DiagnosisList->Clear();
		m_ServiceList->Clear();
	} NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnRadioCptLink() 
{
	try {
		OnRadioCPTDiagnosisChange(TRUE);
	} NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnRadioDiagnosisLink() 
{
	try {
		OnRadioCPTDiagnosisChange(TRUE);
	} NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnRadioCPTDiagnosisChange(BOOL bChangedCPTDiagnosis)
{
	try {

		BOOL bViewLinkedCodes = TRUE;
		
		//find out if we are looking at linked codes or blocked codes
		if(IsDlgButtonChecked(IDC_RADIO_VIEW_BLOCKED_CODES))
			bViewLinkedCodes = FALSE;

		m_ServiceList->Clear();
		m_DiagnosisList->Clear();	

		if(IsDlgButtonChecked(IDC_RADIO_CPT_LINK)) {
			// (j.jones 2013-04-09 17:27) - PLID 56126 - we now might show service codes or products
			if(m_bHasInventory) {
				//show the radio buttons for services / inventory
				GetDlgItem(IDC_RADIO_PRODUCT_LINK_SELECT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_RADIO_CPT_LINK_SELECT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_PRODUCT_LINK_SELECT_COMBO)->ShowWindow(m_radioSelectCPTCodes.GetCheck() ? SW_HIDE : SW_SHOW);
				GetDlgItem(IDC_CPT_LINK_SELECT_COMBO)->ShowWindow(m_radioSelectCPTCodes.GetCheck() ? SW_SHOW : SW_HIDE);
				GetDlgItem(IDC_LABEL_CPT_DIAGNOSIS_DROPDOWN)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_PRODUCT_INFO_LABEL)->ShowWindow(SW_SHOW);
			}
			else {
				//hide the inventory controls
				SetDlgItemText(IDC_LABEL_CPT_DIAGNOSIS_DROPDOWN, "Service Codes");
				GetDlgItem(IDC_LABEL_CPT_DIAGNOSIS_DROPDOWN)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CPT_LINK_SELECT_COMBO)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_PRODUCT_LINK_SELECT_COMBO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_PRODUCT_INFO_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_RADIO_PRODUCT_LINK_SELECT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_RADIO_CPT_LINK_SELECT)->ShowWindow(SW_HIDE);
			}
			GetDlgItem(IDC_DIAGNOSIS_LINK_SELECT_COMBO)->ShowWindow(SW_HIDE);
			// (r.gonet 02/20/2014) - PLID 60778 - Hide the diagnosis code system radio buttons since we are not in diagnosis mode.
			GetDlgItem(IDC_RADIO_SHOW_ICD9_CODES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_SHOW_ICD10_CODES)->ShowWindow(SW_HIDE);
			
			if(bViewLinkedCodes) {
				SetDlgItemText(IDC_LINKED_CODE_LIST_LABEL, m_bHasInventory ? "Linked Codes / Products" : "Linked Codes");
				GetDlgItem(IDC_LINKED_CPT_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_LINKED_DIAGNOSIS_LIST)->ShowWindow(SW_SHOW);
				SetDlgItemText(IDC_BTN_ADD_NEW_CPT_DIAGNOSIS_LINK,"Add/Edit Links");
				SetDlgItemText(IDC_BTN_DELETE_CPT_DIAGNOSIS_LINK,"Delete Link");
			}
			else {
				SetDlgItemText(IDC_LINKED_CODE_LIST_LABEL, m_bHasInventory ? "Blocked Codes / Products" : "Blocked Codes");
				GetDlgItem(IDC_LINKED_CPT_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_LINKED_DIAGNOSIS_LIST)->ShowWindow(SW_HIDE);
				SetDlgItemText(IDC_BTN_ADD_NEW_CPT_DIAGNOSIS_LINK,"Add/Edit Blocks");
				SetDlgItemText(IDC_BTN_DELETE_CPT_DIAGNOSIS_LINK,"Delete Block");
			}
		}
		else if(IsDlgButtonChecked(IDC_RADIO_DIAGNOSIS_LINK)) {
			GetDlgItem(IDC_LABEL_CPT_DIAGNOSIS_DROPDOWN)->ShowWindow(SW_SHOW);
			SetDlgItemText(IDC_LABEL_CPT_DIAGNOSIS_DROPDOWN, "Diagnosis Codes:");
			// (r.gonet 02/20/2014) - PLID 60778 - Show the diagnosis code system radio buttons since we are in diagnosis mode.
			GetDlgItem(IDC_RADIO_SHOW_ICD9_CODES)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_SHOW_ICD10_CODES)->ShowWindow(SW_SHOW);
			// (j.jones 2013-04-09 17:27) - PLID 56126 - hide product combo and radio buttons
			GetDlgItem(IDC_RADIO_PRODUCT_LINK_SELECT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_CPT_LINK_SELECT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PRODUCT_LINK_SELECT_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CPT_LINK_SELECT_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DIAGNOSIS_LINK_SELECT_COMBO)->ShowWindow(SW_SHOW);
			if(bViewLinkedCodes) {
				SetDlgItemText(IDC_LINKED_CODE_LIST_LABEL, "Linked Codes");
				GetDlgItem(IDC_LINKED_CPT_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_LINKED_DIAGNOSIS_LIST)->ShowWindow(SW_HIDE);
				SetDlgItemText(IDC_BTN_ADD_NEW_CPT_DIAGNOSIS_LINK,"Add/Edit Links");
				SetDlgItemText(IDC_BTN_DELETE_CPT_DIAGNOSIS_LINK,"Delete Link");
			}
			else {
				SetDlgItemText(IDC_LINKED_CODE_LIST_LABEL, "Blocked Codes");
				GetDlgItem(IDC_LINKED_CPT_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_LINKED_DIAGNOSIS_LIST)->ShowWindow(SW_SHOW);
				SetDlgItemText(IDC_BTN_ADD_NEW_CPT_DIAGNOSIS_LINK,"Add/Edit Blocks");
				SetDlgItemText(IDC_BTN_DELETE_CPT_DIAGNOSIS_LINK,"Delete Block");
			}
		}

		// (j.jones 2012-12-12 15:52) - PLID 47773 - the ability to toggle the prompt in the bill
		// only applies to linked codes, not blocked codes, so hide the checkbox when viewing
		// blocked codes
		if(bViewLinkedCodes) {
			m_checkAllowDynamicLinking.ShowWindow(SW_SHOW);
		}
		else {
			m_checkAllowDynamicLinking.ShowWindow(SW_HIDE);
		}
		
		if(bViewLinkedCodes && bChangedCPTDiagnosis) {
			//if we are viewing linked codes and changed the CPT/Diagnosis radio,
			//we want to clear out the selection in the combo
			m_CPTCombo->CurSel = -1;
			m_ProductCombo->CurSel = NULL;
			m_DiagnosisCombo->CurSel = -1;
		}
		else {
			if(m_radioSelectCPTCodes.GetCheck()) {
				OnSelChosenCptLinkSelectCombo(m_CPTCombo->CurSel);
			}
			else {
				OnSelChosenProductLinkSelectCombo(m_ProductCombo->CurSel);
			}
			OnSelChosenDiagnosisLinkSelectCombo(m_DiagnosisCombo->CurSel);
		}

	}NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnRadioViewLinkedCodes() 
{
	try {
		OnRadioCPTDiagnosisChange(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CCPTDiagnosisLinkingDlg::OnRadioViewBlockedCodes() 
{
	try {
		OnRadioCPTDiagnosisChange(FALSE);
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-12-12 15:52) - PLID 47773 - added ability to toggle the prompt in the bill
void CCPTDiagnosisLinkingDlg::OnCheckAllowDynamicLinking()
{
	try {

		SetRemotePropertyInt("AllowDynamicCPTICDLinking", m_checkAllowDynamicLinking.GetCheck() ? 1 : 0, 0, "<None>");

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-09 16:43) - PLID 56126 - added product combo
void CCPTDiagnosisLinkingDlg::OnSelChosenProductLinkSelectCombo(LPDISPATCH lpRow)
{
	try {

		BOOL bViewLinkedCodes = TRUE;
	
		//find out if we are looking at linked codes or blocked codes
		if(IsDlgButtonChecked(IDC_RADIO_VIEW_BLOCKED_CODES))
			bViewLinkedCodes = FALSE;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			if(bViewLinkedCodes) {
				m_DiagnosisList->Clear();
			}
			return;
		}
		
		long nServiceID = VarLong(pRow->GetValue(pccID));
		CString str;

		if(bViewLinkedCodes) {	
			m_DiagnosisList->FromClause = _bstr_t("CPTDiagnosisGroupsT INNER JOIN DiagCodes ON CPTDiagnosisGroupsT.DiagCodeID = DiagCodes.ID");			
			str.Format("Active = 1 AND ServiceID = %li", nServiceID);
			m_DiagnosisList->WhereClause = _bstr_t(str);
			m_DiagnosisList->Requery();
		}
		else {
			m_ServiceList->FromClause = _bstr_t("ServiceT LEFT JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID");
			str.Format("Active = 1 AND (CPTCodeT.ID Is Not Null OR ProductT.ID Is Not Null) AND (ServiceT.ID IN (SELECT ServiceID1 FROM BlockedCPTCodesT WHERE ServiceID2 = %li) OR ServiceT.ID IN (SELECT ServiceID2 FROM BlockedCPTCodesT WHERE ServiceID1 = %li))", nServiceID, nServiceID);
			m_ServiceList->WhereClause = _bstr_t(str);
			m_ServiceList->Requery();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-09 17:24) - PLID 56126 - added radio buttons for services/products
void CCPTDiagnosisLinkingDlg::OnRadioProductLinkSelect()
{
	try {

		OnRadioCPTDiagnosisChange(FALSE);
	
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-09 17:24) - PLID 56126 - added radio buttons for services/products
void CCPTDiagnosisLinkingDlg::OnRadioCptLinkSelect()
{
	try {

		OnRadioCPTDiagnosisChange(FALSE);
	
	}NxCatchAll(__FUNCTION__);
}