// LinksSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LinksSetupDlg.h"
#include "AdministratorRc.h"
#include "ApptPrototypesDlg.h"
#include "GetNewIDName.h"
#include "ConfigureNexWebPasswordComplexityRequirementsDlg.h" // (b.savon 2012-07-25 22:47) - PLID 50585
#include "AuditTrail.h"
#include "NexwebTextContentOverrideDlg.h" // (d.singleton 2013-02-15 08:57) - PLID 54979
#include "HtmlEditorDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

//(e.lally 2010-10-25) PLID 40994 - Created
// CLinksSetupDlg dialog

IMPLEMENT_DYNAMIC(CLinksSetupDlg, CNxDialog)

// (d.singleton 2013-02-01 11:47) - PLID 54979 hardcoded values for the content types of nexweb content settings
#define CONTENT_TYPE_BOOLEAN	"{7F9C8999-287A-431B-8454-28103B682566}"
#define CONTENT_TYPE_PLUGIN_SECTION_TEXT	"{9DDDC05A-5153-40EE-9335-76BA43C39FC1}"
// (b.cardillo 2013-02-15 21:30) - PLID 55209 - add support for integer type settings
#define CONTENT_TYPE_INTEGER	"{56681C22-08C5-48E8-BBFE-B5FE0AE7843C}"
// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
#define CONTENT_TYPE_PLAIN_TEXT	"{49A2F944-A0BE-4D9D-9B3A-927B916F5F82}"
#define CONTENT_TYPE_HTML_TEXT	"{32C2EAFE-C2CD-4F48-907D-9ED015106474}"


enum ToDoPriority {
	tpHigh = 1,
	tpMedium = 2,
	tpLow = 3,
};

enum ToDoMethod {
	tmPhone = 0,
	tmEmail = 1,
	tmFax = 2,
	tmLetter = 3,
	tmOther = 4
};

enum ToDoEventListColumns{
	telcValue = 0,
	telcDescription,
};

enum ToDoPriorityColumns{
	tpcValue = 0,
	tpcDescription,
};

enum ToDoMethodColumns {
	tmcValue = 0,
	tmcDescription,
};

enum ToDoCategoryColumns {
	tccID = 0,
	tccDescription,
};

enum ToDoAssignToColumns {
	tacSelected = 0,
	tacUserID,
	tacUsername,
};

//(e.lally 2011-04-26) PLID 41445
enum SubdomainColumns {
	scID = 0,
	scName,
	scDefault,
};


enum NexWebDisplaylistTypeColumns
{
	dtcTypeID=0,
	dtcTypeName,
};

enum NexWebDisplaylistDetailColumns
{
	ddcFKID=0,
	ddcVisible,
	ddcDataName,
	ddcFriendlyName,
	ddcAdditionalInfo1, //(e.lally 2010-12-06) PLID 41703
};

// (d.singleton 2013-02-01 11:48) - PLID 54979
enum NexWebContentDetailColumns
{
	cdcID=0,
	cdcMasterUID,
	cdcText,
	cdcDescription,
	cdcIntParam, 
	cdcContentTypeUID,
	// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Enforce min/max
	cdcIntParamMin,
	cdcIntParamMax,
};

const long cnNoCategory = -1;
//This value is arbitrary, but we used the same value as tssaPatientCoordinator
const long cnPatientCoordinator  = -10;

CLinksSetupDlg::CLinksSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLinksSetupDlg::IDD, pParent)
{
	m_bAssignToPatCood = FALSE;
	//(e.lally 2011-04-26) PLID 41445
	m_nSubdomainID = -1;
	//(e.lally 2011-04-27) PLID 41445
	m_eEvent = nteInvalid;

	m_nDisplaylistTypeID = -1;
	m_bDisplaylistSetupIsValid = false;
}

CLinksSetupDlg::~CLinksSetupDlg()
{
}

void CLinksSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEXWEB_SETUP_TODO_ENABLED, m_checkToDoEnabled);
	DDX_Control(pDX, IDC_NEXWEB_SETUP_ACTION_ENABLED, m_checkActionEnabled);
	DDX_Control(pDX, IDC_NEXWEB_SETUP_TODO_NOTE, m_nxeditToDoNote);
	DDX_Control(pDX, IDC_NEXWEB_SETUP_ACTION_NOTE, m_nxeditActionNote);
	DDX_Control(pDX, IDC_BTN_NEXWEB_TODO_EVENT_NEXT, m_btnNexWebToDoEventNext);
	DDX_Control(pDX, IDC_BTN_NEXWEB_TODO_EVENT_PREV, m_btnNexWebToDoEventPrev);
	DDX_Control(pDX, IDC_BTN_NEXWEB_SUBDOMAIN_NEXT, m_btnNexWebSubdomainNext);
	DDX_Control(pDX, IDC_BTN_NEXWEB_SUBDOMAIN_PREV, m_btnNexWebSubdomainPrev);
	DDX_Control(pDX, IDC_BTN_NEXWEB_SUBDOMAIN_NEW, m_btnNexWebSubdomainNew);
	DDX_Control(pDX, IDC_BTN_NEXWEB_SUBDOMAIN_RENAME, m_btnNexWebSubdomainRename);
	DDX_Control(pDX, IDC_BTN_NEXWEB_SUBDOMAIN_DELETE, m_btnNexWebSubdomainDelete);
	DDX_Control(pDX, IDC_BTN_NEXWEB_SUBDOMAIN_MAKE_DEFAULT, m_btnNexWebSubdomainMakeDefault);
	DDX_Control(pDX, IDC_BTN_NEXWEB_PASSWORD_REQ, m_btnNexWebPasswordRequirements);
}


BEGIN_MESSAGE_MAP(CLinksSetupDlg, CNxDialog)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_NEXWEB_SETUP_TODO_ENABLED, OnEnableToDo)
	ON_BN_CLICKED(IDC_NEXWEB_SETUP_ACTION_ENABLED, OnEnableAction)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_TODO_EVENT_PREV, OnPrevToDoEvent)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_TODO_EVENT_NEXT, OnNextToDoEvent)
	ON_BN_CLICKED(IDC_APPTPROTOTYPE_SETUP_BTN, &CLinksSetupDlg::OnBnClickedApptprototypeSetupBtn)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_SUBDOMAIN_PREV, OnPrevSubdomain)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_SUBDOMAIN_NEXT, OnNextSubdomain)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_SUBDOMAIN_NEW, OnNewSubdomain)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_SUBDOMAIN_RENAME, OnRenameSubdomain)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_SUBDOMAIN_DELETE, OnDeleteSubdomain)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_SUBDOMAIN_MAKE_DEFAULT, OnMakeDefaultSubdomain)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_DISPLAY_SELECT_ALL, OnSelectallDisplaylistBtn)
	ON_BN_CLICKED(IDC_BTN_NEXWEB_PASSWORD_REQ, &CLinksSetupDlg::OnBnClickedBtnNexwebPasswordReq)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CLinksSetupDlg, CNxDialog)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_EVENT_LIST, 16, OnSelChosenToDoEvent, VTS_DISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_PRIORITY, 16, OnSelChosenToDoPriority, VTS_DISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_METHOD, 16, OnSelChosenToDoMethod, VTS_DISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_CATEGORY, 16, OnSelChosenToDoCategory, VTS_DISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_ASSIGNTO, 10, OnEditingFinishedToDoAssignToUsers, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_CATEGORY, 18, OnRequeryFinishedToDoCategory, VTS_I2)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_TODO_ASSIGNTO, 18, OnRequeryFinishedToDoAssignToUsers, VTS_I2)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SUBDOMAIN_LIST, 1, OnSelChangingSubdomainList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SUBDOMAIN_LIST, 16, OnSelChosenSubdomainList, VTS_DISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_DISPLAY_LIST, 16, OnSelChosenDisplaylistType, VTS_DISPATCH)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_DISPLAY_DETAIL_LIST, 10, OnEditingFinishedNexWebListDisplay, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_DISPLAY_DETAIL_LIST, 9, OnEditingFinishingNexWebListDisplay, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_CONTENT_SETTING_DETAIL_TABLE, 10, OnEditingFinishedContentSettingDetailTable, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_CONTENT_SETTING_DETAIL_TABLE, 9, OnEditingFinishingContentSettingDetailTable, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CLinksSetupDlg, IDC_NEXWEB_SETUP_CONTENT_SETTING_DETAIL_TABLE, 19, CLinksSetupDlg::LeftClickNexwebSetupContentSettingDetailTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// CLinksSetupDlg message handlers


LRESULT CLinksSetupDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
			case NetUtils::Coordinators:

				// (j.jones 2014-08-08 09:51) - PLID 63232 - if the Admin module is not active,
				// we would not receive this TC, because coming back will still refresh the todo user list,
				// even though this code does not explicitly call UpdateView()

				if(m_pToDoAssignTo != NULL){
					//(e.lally 2010-11-22) PLID 35819 - Refresh the list of users if we get a table checker for it.
					m_pToDoAssignTo->Requery();
				}
				break;
		}

	} NxCatchAll(__FUNCTION__);
	return 0;
}



BOOL CLinksSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//(e.lally 2011-04-26) PLID 41445
		m_nSubdomainID = -1;
		//(e.lally 2011-04-27) PLID 41445 - Track the current ToDo Event
		m_eEvent = nteInvalid;
		m_bDisplaylistSetupIsValid = false;

		EnsureAllControls();

		m_btnNexWebToDoEventNext.AutoSet(NXB_RIGHT);
		m_btnNexWebToDoEventPrev.AutoSet(NXB_LEFT);

		//(e.lally 2011-04-27) PLID 41445 - subdomain buttons
		m_btnNexWebSubdomainNext.AutoSet(NXB_RIGHT);
		m_btnNexWebSubdomainPrev.AutoSet(NXB_LEFT);
		m_btnNexWebSubdomainNew.AutoSet(NXB_NEW);
		m_btnNexWebSubdomainRename.AutoSet(NXB_MODIFY);
		m_btnNexWebSubdomainDelete.AutoSet(NXB_DELETE);
		m_btnNexWebSubdomainMakeDefault.AutoSet(NXB_MODIFY);

		m_btnNexWebContentSettingUp.AutoSet(NXB_UP);
		m_btnNexWebContentSettingDown.AutoSet(NXB_DOWN);
		m_btnNexWebContentSettingAdd.AutoSet(NXB_NEW);
		m_btnNexWebContentSettingRemove.AutoSet(NXB_DELETE);

		// (b.savon 2012-07-26 08:09) - PLID 50585
		m_btnNexWebPasswordRequirements.AutoSet(NXB_LOCK);

		//(e.lally 2011-04-26) PLID 41445 - Initialize the subdomain list
		m_pSubdomainList = BindNxDataList2Ctrl(IDC_NEXWEB_SUBDOMAIN_LIST, false);
		InitializeEventControls();
		InitializeDisplaylistControls();
		InitializeContentSettingControls();

		//Load ToDo event
		m_pEventList->PutCurSel(m_pEventList->GetFirstRow());
		IRowSettingsPtr pSelRow = m_pEventList->GetCurSel();
		if(pSelRow != NULL){
			//(e.lally 2011-04-27) PLID 41445 - Track the current ToDo Event
			m_eEvent = (NexWebEvent)VarLong(pSelRow->GetValue(telcValue));
		}		

		// (d.singleton 2013-02-15 09:09) - PLID 54979 row settings for a text content type
		if(m_pfsText == NULL) {		
			NXDATALIST2Lib::IFormatSettingsPtr pfsText(__uuidof(NXDATALIST2Lib::FormatSettings));
			pfsText->PutFieldType(cftTextSingleLineLink);
			pfsText->PutDataType(VT_BSTR);
			pfsText->PutEditable(FALSE);
			m_pfsText = pfsText;
		}		
		if(m_pfsBoolean == NULL) {
		// (d.singleton 2013-02-15 09:09) - PLID 54979 row settings for a boolean content type	
			NXDATALIST2Lib::IFormatSettingsPtr pfsBoolean(__uuidof(NXDATALIST2Lib::FormatSettings));
			pfsBoolean->PutFieldType(cftBoolCheckbox);
			pfsBoolean->PutDataType(VT_BOOL);	
			pfsBoolean->PutEditable(TRUE);
			m_pfsBoolean = pfsBoolean;
		}
		// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Handle actual integer values
		if (m_pfsInteger == NULL) {
			NXDATALIST2Lib::IFormatSettingsPtr pfs(__uuidof(NXDATALIST2Lib::FormatSettings));
			pfs->PutFieldType(cftTextSingleLine);
			pfs->PutDataType(VT_BSTR);	
			pfs->PutEditable(TRUE);
			m_pfsInteger = pfs;
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//(e.lally 2011-04-27) PLID 41445
void CLinksSetupDlg::Refresh()
{
	try{
		m_pSubdomainList->Requery();
		m_pSubdomainList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		//Try to reselect our row
		m_pSubdomainList->SetSelByColumn(scID, m_nSubdomainID);

		if(m_pSubdomainList->CurSel == NULL){
			m_nSubdomainID = -1;
			//(e.lally 2011-05-05) PLID 41445 - Now try to select the default
			IRowSettingsPtr pCurSubdomainRow = m_pSubdomainList->GetFirstRow();
			while(pCurSubdomainRow != NULL && m_nSubdomainID == -1){
				if(AsBool(pCurSubdomainRow->GetValue(scDefault))){
					m_nSubdomainID = VarLong(pCurSubdomainRow->GetValue(scID));
					m_pSubdomainList->PutCurSel(pCurSubdomainRow);
				}
				pCurSubdomainRow = pCurSubdomainRow->GetNextRow();
			}
			if(m_pSubdomainList->CurSel == NULL){
				//All else fails, select the first one.
				pCurSubdomainRow = m_pSubdomainList->GetFirstRow();
				m_pSubdomainList->PutCurSel(pCurSubdomainRow);
				if(pCurSubdomainRow != NULL){
					m_nSubdomainID = VarLong(pCurSubdomainRow->GetValue(scID));
				}
			}
		}

		LoadSubdomain();

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-26) PLID 43445 - Moved to its own function
// (j.armen 2011-11-22 10:09) - PLID 40420 - Renamed to generalize events
void CLinksSetupDlg::InitializeEventControls()
{
	m_bAssignToPatCood = FALSE;
	//(e.lally 2010-11-17) PLID 35819 - Set up the datalists for NexWeb ToDo templates

	//Users assigned to
	m_pToDoAssignTo = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_TODO_ASSIGNTO, false);

	//Events
	{
		//(e.lally 2011-02-28) PLID 42603 - Made the events sorted alphabetically.

		m_pEventList = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_TODO_EVENT_LIST, false);
		//Load static list of events
		IRowSettingsPtr pRow = m_pEventList->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(telcValue, (long)nteNewProspectEntered);
			pRow->PutValue(telcDescription, _bstr_t("New Leads Entered"));
			m_pEventList->AddRowSorted(pRow, NULL);
		}
		pRow = m_pEventList->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(telcValue, (long)ntePatientMessageReceived);
			pRow->PutValue(telcDescription, _bstr_t("New Patient Message Received"));
			m_pEventList->AddRowSorted(pRow, NULL);
		}
		//(e.lally 2011-02-28) PLID 42603 - Added new event entry
		pRow = m_pEventList->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(telcValue, (long)nteAppointmentRequestReceived);
			pRow->PutValue(telcDescription, _bstr_t("Appointment Request Received"));
			m_pEventList->AddRowSorted(pRow, NULL);
		}

		// (j.armen 2011-07-20 11:49) - PLID 44208 - Added Appointment Change Request entry
		pRow = m_pEventList->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(telcValue, (long)nteAppointmentChangeRequestReceived);
			pRow->PutValue(telcDescription, _bstr_t("Appointment Change Request Received"));
			m_pEventList->AddRowSorted(pRow, NULL);
		}

		// (j.armen 2011-07-20 11:49) - PLID 44208 - Added Appointment Cancelation Request entry
		pRow = m_pEventList->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(telcValue, (long)nteAppointmentCancelationRequestReceived);
			pRow->PutValue(telcDescription, _bstr_t("Appointment Cancellation Request Received"));
			m_pEventList->AddRowSorted(pRow, NULL);
		}
	}
	//Priority
	{
		m_pToDoPriority = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_TODO_PRIORITY, false);
		//Load static list of events
		IRowSettingsPtr pRow = m_pToDoPriority->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tpcValue, (long)tpHigh);
			pRow->PutValue(tpcDescription, _bstr_t("High"));
			m_pToDoPriority->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pToDoPriority->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tpcValue, (long)tpMedium);
			pRow->PutValue(tpcDescription, _bstr_t("Medium"));
			m_pToDoPriority->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pToDoPriority->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tpcValue, (long)tpLow);
			pRow->PutValue(tpcDescription, _bstr_t("Low"));
			m_pToDoPriority->AddRowAtEnd(pRow, NULL);
		}
	}
	//Method of Contact
	{
		m_pToDoMethod = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_TODO_METHOD, false);
		//Load static list of events
		IRowSettingsPtr pRow = m_pToDoMethod->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tmcValue, (long)tpHigh);
			pRow->PutValue(tmcDescription, _bstr_t("Phone"));
			m_pToDoMethod->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pToDoMethod->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tmcValue, (long)tpMedium);
			pRow->PutValue(tmcDescription, _bstr_t("E-Mail"));
			m_pToDoMethod->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pToDoMethod->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tmcValue, (long)tpLow);
			pRow->PutValue(tmcDescription, _bstr_t("Fax"));
			m_pToDoMethod->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pToDoMethod->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tmcValue, (long)tpLow);
			pRow->PutValue(tmcDescription, _bstr_t("Letter"));
			m_pToDoMethod->AddRowAtEnd(pRow, NULL);
		}
		pRow = m_pToDoMethod->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tmcValue, (long)tpLow);
			pRow->PutValue(tmcDescription, _bstr_t("Other"));
			m_pToDoMethod->AddRowAtEnd(pRow, NULL);
		}
	}
	//Category
	{
		m_pToDoCategory = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_TODO_CATEGORY, true);
	}

	m_nxeditToDoNote.SetLimitText(2000);
	m_nxeditActionNote.SetLimitText(2000);	// (j.armen 2011-11-22 10:10) - PLID 40420 - Added Action Note
}

void CLinksSetupDlg::InitializeDisplaylistControls()
{
	//Set up the datalists
	m_pDisplaylistType = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_DISPLAY_LIST, false);
	m_pDisplaylistDetails = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_DISPLAY_DETAIL_LIST, false);

	m_displaylistSetup.SetSize(nwltCount_DoNotUse, -1);

	//Build list type
	IRowSettingsPtr pRow;
	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltCustomFields);
		pRow->PutValue(dtcTypeName, "Custom Fields");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltCustomFields;
		detail.strTableName = "CustomFieldsT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "CustomFieldID";
		m_displaylistSetup.SetAt(nwltCustomFields, detail);
	}

	//(e.lally 2009-11-18) PLID 35805 - Only show EMR templates if they are licensed for it.
	//(e.lally 2011-10-21) PLID 46065 - We need to check EMR licensing when it gets selected and we are ready to requery,
	//	not here or it causes a validation exception
	//if(g_pLicense && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) 
	{
		pRow = m_pDisplaylistType->GetNewRow();
		if(pRow){
			pRow->PutValue(dtcTypeID, (long)nwltEMRTemplates);
			pRow->PutValue(dtcTypeName, "EMR Templates");
			m_pDisplaylistType->AddRowSorted(pRow, NULL);

			NexWebDisplaylistDetail detail;
			detail.enumValue = nwltEMRTemplates;
			detail.strTableName = "EmrTemplateT";
			detail.strPKIDFieldName = "ID";
			detail.strFKIDFieldName = "EmrTemplateID";
			m_displaylistSetup.SetAt(nwltEMRTemplates, detail);
		}
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltInsuranceCos);
		pRow->PutValue(dtcTypeName, "Insurance Companies");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltInsuranceCos;
		detail.strTableName = "InsuranceCoT";
		detail.strPKIDFieldName = "PersonID";
		detail.strFKIDFieldName = "InsuranceCoID";
		m_displaylistSetup.SetAt(nwltInsuranceCos, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltProcedures);
		pRow->PutValue(dtcTypeName, "Procedures");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltProcedures;
		detail.strTableName = "ProcedureT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "ProcedureID";
		m_displaylistSetup.SetAt(nwltProcedures, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltReferralSources);
		pRow->PutValue(dtcTypeName, "Referral Sources");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltReferralSources;
		detail.strTableName = "ReferralSourceT";
		detail.strPKIDFieldName = "PersonID";
		detail.strFKIDFieldName = "ReferralSourceID";
		m_displaylistSetup.SetAt(nwltReferralSources, detail);
	}

	//(e.lally 2011-05-04) PLID 42209 - Add more display lists
	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltCountries);
		pRow->PutValue(dtcTypeName, "Countries");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltCountries;
		detail.strTableName = "CountriesT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "CountryID";
		m_displaylistSetup.SetAt(nwltCountries, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltLanguages);
		pRow->PutValue(dtcTypeName, "Languages");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltLanguages;
		detail.strTableName = "LanguageT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "LanguageID";
		m_displaylistSetup.SetAt(nwltLanguages, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltRaces);
		pRow->PutValue(dtcTypeName, "Races");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltRaces;
		detail.strTableName = "RaceT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "RaceID";
		m_displaylistSetup.SetAt(nwltRaces, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltEthnicities);
		pRow->PutValue(dtcTypeName, "Ethnicities");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltEthnicities;
		// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
		detail.strTableName = "EthnicityT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "Ethnicity";
		m_displaylistSetup.SetAt(nwltEthnicities, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltPrefixes);
		pRow->PutValue(dtcTypeName, "Prefixes");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltPrefixes;
		detail.strTableName = "PrefixT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "PrefixID";
		m_displaylistSetup.SetAt(nwltPrefixes, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltReferringPhysicians);
		pRow->PutValue(dtcTypeName, "Referring Physicians");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltReferringPhysicians;
		detail.strTableName = "ReferringPhysT";
		detail.strPKIDFieldName = "PersonID";
		detail.strFKIDFieldName = "ReferringPhysicianID";
		m_displaylistSetup.SetAt(nwltReferringPhysicians, detail);
	}

	pRow = m_pDisplaylistType->GetNewRow();
	if(pRow){
		pRow->PutValue(dtcTypeID, (long)nwltPrimaryCareProviders);
		pRow->PutValue(dtcTypeName, "Primary Care Doctors (PCP)");
		m_pDisplaylistType->AddRowSorted(pRow, NULL);

		NexWebDisplaylistDetail detail;
		detail.enumValue = nwltPrimaryCareProviders;
		detail.strTableName = "ReferringPhysT";
		detail.strPKIDFieldName = "PersonID";
		detail.strFKIDFieldName = "PriCarePhysID";
		m_displaylistSetup.SetAt(nwltPrimaryCareProviders, detail);
	}

	{
		//(e.lally 2011-05-09) PLID 42209 - Split out custom list items into 6 separate entries.
		//Custom Lists 1-6
		NexWebDisplaylistDetail detail;
		detail.strTableName = "CustomListItemsT";
		detail.strPKIDFieldName = "ID";
		detail.strFKIDFieldName = "CustomListItemID";

		long nCustomListEnum = nwltCustomDropdownList1;
		for(int i =1; i<=6; i++){
			pRow = m_pDisplaylistType->GetNewRow();
			if(pRow){
				pRow->PutValue(dtcTypeID, (long)nCustomListEnum);
				pRow->PutValue(dtcTypeName, _bstr_t(FormatString("Custom List %li Items", i)));
				m_pDisplaylistType->AddRowSorted(pRow, NULL);

				detail.enumValue = (NexWebDisplaylistType)nCustomListEnum;
				m_displaylistSetup.SetAt(nCustomListEnum, detail);
			}
			nCustomListEnum++;
		}
	}

	//Double check the setup is correct. The index should match the enum value
	for(int i =0; i< m_displaylistSetup.GetCount(); i++){
		if((long)m_displaylistSetup.GetAt(i).enumValue != i){
			//There was a problem loading the setup!
			AfxThrowNxException("Invalid NexWeb Display List setup detected at index (%li)!", i);
		}
	}

	m_bDisplaylistSetupIsValid = true;

	// (b.cardillo 2010-08-18 14:16) - PLID 40172 - Initialize the enabled state of the "select/unselect all" button
	EnableDlgItem(IDC_BTN_NEXWEB_DISPLAY_SELECT_ALL, (m_nDisplaylistTypeID == nwltEMRTemplates || m_nDisplaylistTypeID == -1) ? FALSE : TRUE);

}

//(e.lally 2010-11-09) PLID 35819 - Load the selections for the given event, set the interface values
//(e.lally 2011-04-26) PLID 43445 - Renamed to LoadToDoEvent
//(e.lally 2011-04-27) PLID 41445 - Removed nEvent, load based on the cached current event
// (j.armen 2011-11-22 10:11) - PLID 40420 - Renamed to LoadEvent to generalize events.
void CLinksSetupDlg::LoadEvent()
{
	//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
	// (j.armen 2011-11-22 10:11) - PLID 40420 - Renamed to be event based
	CString strFrom;
	strFrom.Format("UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID "
	"LEFT JOIN "
	"(SELECT NexWebEventToDoAssignToT.UserID, NexWebEventT.EventID "
		"FROM NexWebEventToDoAssignToT "
		"LEFT JOIN NexWebEventT ON NexWebEventToDoAssignToT.NexWebEventID = NexWebEventT.ID "
		"WHERE NexWebEventT.EventID = %li AND NexWebEventT.SubdomainID = %li) "
	"NexWebEventToDoAssignToQ ON UsersT.PersonID = NexWebEventToDoAssignToQ.UserID ", m_eEvent, m_nSubdomainID);
	m_pToDoAssignTo->FromClause = _bstr_t(strFrom);

	// (j.armen 2011-11-22 10:11) - PLID 40420 - Using ToDo and Action fields
	_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM NexWebEventT WHERE SubdomainID = {INT} AND EventID = {INT} ", m_nSubdomainID, m_eEvent);
	if(!rs->eof){
		BOOL bToDoEnabled = AsBool(rs->Fields->Item["ToDoEnabled"]->Value);
		m_checkToDoEnabled.SetCheck(bToDoEnabled);

		BOOL bActionEnabled = AsBool(rs->Fields->Item["ActionEnabled"]->Value);
		m_checkActionEnabled.SetCheck(bActionEnabled);

		CString strNotes = VarString(rs->Fields->Item["ToDoNotes"]->Value);
		SetDlgItemText(IDC_NEXWEB_SETUP_TODO_NOTE, strNotes);

		CString strActionNotes = VarString(rs->Fields->Item["ActionNotes"]->Value);
		SetDlgItemText(IDC_NEXWEB_SETUP_ACTION_NOTE, strActionNotes);

		BYTE iPriority = VarByte(rs->Fields->Item["ToDoPriority"]->Value);
		m_pToDoPriority->SetSelByColumn(tpcValue, (long)iPriority);

		CString strTask = VarString(rs->Fields->Item["ToDoTask"]->Value);
		m_pToDoMethod->SetSelByColumn(tmcDescription, _bstr_t(strTask));

		long nCategoryID = VarLong(rs->Fields->Item["ToDoCategoryID"]->Value, 0);
		m_pToDoCategory->SetSelByColumn(tccID, (long)nCategoryID);

		//(e.lally 2010-11-29) PLID 41577 - set the pat coord value
		m_bAssignToPatCood = AsBool(rs->Fields->Item["ToDoAssignToPatCoord"]->Value);
	}
	else {
		ResetEventControls();
		//(e.lally 2011-04-26) PLID 43445 - Make sure we have a current subdomain
		if(m_eEvent != nteInvalid && m_nSubdomainID != -1){
			//This should only be possible if somehow the data was not auto-created already by a mod
			//Auto create the record
			ToDoPriority eDefaultPriority = tpMedium;
			CString strDefaultMethod = "Other";
			//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
			// (j.armen 2011-11-22 10:12) - PLID 40420 - Renamed tables to be event driven.
			ExecuteParamSql("INSERT INTO NexWebEventT "
				"(EventID, SubdomainID, ToDoPriority, ToDoTask) "
				"VALUES({INT}, {INT}, {INT}, {STRING}) ", 
				m_eEvent, m_nSubdomainID, eDefaultPriority, strDefaultMethod);
			//We only set priority and task, so show the selections for those
			m_pToDoPriority->SetSelByColumn(tpcValue, (long)eDefaultPriority);
			m_pToDoMethod->SetSelByColumn(tmcDescription, _bstr_t(strDefaultMethod));
		}
	}
	rs->Close();

	//(e.lally 2010-11-29) PLID 41577 - Requery the user list after we load the pat coord value
	m_pToDoAssignTo->Requery();
	EnsureEventControls();
}

//(e.lally 2010-11-09) PLID 35819 - ensure the proper controls are enabled/disabled
//(e.lally 2011-04-26) PLID 43445 - Renamed to EnsureToDoControls
// (j.armen 2011-11-22 10:13) - PLID 40420 - Renamed to EnsureEventControls to generalize for Events
void CLinksSetupDlg::EnsureEventControls()
{
	BOOL bEnableToDoDetails = FALSE;
	BOOL bEnableActionDetails = FALSE;
	// (j.armen 2011-11-22 10:14) - PLID 40420 - Hide ToDoDetails/ActionDetails if the event does not use them
	int nSWToDoDetails = SW_SHOWNA;
	int nSWActionDetails = SW_SHOWNA;
	BOOL bEnablePrevEvent = FALSE;
	BOOL bEnableNextEvent = FALSE;

	//(e.lally 2011-04-26) PLID 43445 - Make sure we have a current subdomain
	if(m_pEventList != NULL && m_nSubdomainID != -1){
		IRowSettingsPtr pCurEvent = m_pEventList->GetCurSel();
		if(pCurEvent != NULL){
			// (j.armen 2011-11-22 10:15) - PLID 40420 - if an event has the ability to create a todo, 
			//then show the todo segment, else hide it
			if(HasToDoEvent())
			{
				bEnableToDoDetails = m_checkToDoEnabled.GetCheck()?TRUE:FALSE;
				m_checkToDoEnabled.EnableWindow(TRUE);
			}
			else
			{
				nSWToDoDetails = SW_HIDE;
				m_checkToDoEnabled.EnableWindow(FALSE);
				// (j.armen 2011-11-22 10:16) - PLID 40420 - only need to disable the checkbox here, other components will be disabled
				// since bEnableToDoDetails is FALSE
			}

			// (j.armen 2011-11-22 10:15) - PLID 40420 - if an event has the ablility to create an action,
			//then show the action segment, else hide it
			if(HasActionEvent())
			{
				bEnableActionDetails = m_checkActionEnabled.GetCheck()?TRUE:FALSE;
				m_checkActionEnabled.EnableWindow(TRUE);
				m_checkActionEnabled.SetWindowTextA(GetActionEventName());
			}
			else
			{
				nSWActionDetails = SW_HIDE;
				m_checkActionEnabled.EnableWindow(FALSE);
			}

			bEnablePrevEvent = (pCurEvent->GetPreviousRow() != NULL)?TRUE:FALSE;
			bEnableNextEvent = (pCurEvent->GetNextRow() != NULL)?TRUE:FALSE;
		}
		else{
			m_checkToDoEnabled.EnableWindow(FALSE);
			m_checkActionEnabled.EnableWindow(FALSE);
		}
	}
	else {
		m_checkToDoEnabled.EnableWindow(FALSE);
		m_checkActionEnabled.EnableWindow(FALSE);
	}

	// (j.armen 2011-11-22 10:17) - PLID 40420 - Show ToDo fields based on whether the action has the ability to perform a ToDo
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_PRIORITY)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_METHOD)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_CATEGORY)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_ASSIGNTO)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_NOTE)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_LABEL_PRIORITY)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_LABEL_METHOD)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_LABEL_CATEGORY)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_LABEL_ASSIGN_TO_USERS)->ShowWindow(nSWToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_LABEL_NOTE)->ShowWindow(nSWToDoDetails);
	m_checkToDoEnabled.ShowWindow(nSWToDoDetails);

	// (j.armen 2011-11-22 10:18) - PLID 40420 - Show Action fields based on whether the action has the ability to perform an Action
	m_checkActionEnabled.ShowWindow(nSWActionDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_ACTION_LABEL_NOTE)->ShowWindow(nSWActionDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_ACTION_NOTE)->ShowWindow(nSWActionDetails);

	GetDlgItem(IDC_NEXWEB_SETUP_TODO_PRIORITY)->EnableWindow(bEnableToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_METHOD)->EnableWindow(bEnableToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_TODO_CATEGORY)->EnableWindow(bEnableToDoDetails);
	GetDlgItem(IDC_NEXWEB_SETUP_ACTION_NOTE)->EnableWindow(bEnableActionDetails);

	// (j.armen 2011-10-10 11:43) - PLID 44208 - for Appointment Change and Cancelation Requests, we have a specific message that is being controlled by Nextech.Utilities
	switch(m_eEvent)
	{
		case nteAppointmentChangeRequestReceived:
			SetDlgItemText(IDC_NEXWEB_SETUP_TODO_NOTE, "Patient has requested to change the date/time of their {Purpose} appointment with {Resource} from {Original Date} to {Requested Date} at {Location}.");
			GetDlgItem(IDC_NEXWEB_SETUP_TODO_NOTE)->EnableWindow(FALSE);
			break;
		case nteAppointmentCancelationRequestReceived:
			SetDlgItemText(IDC_NEXWEB_SETUP_TODO_NOTE, "Patient has requested to cancel their {Purpose} appointment with {Resource} scheduled for {Date} at {Location}.");
			GetDlgItem(IDC_NEXWEB_SETUP_TODO_NOTE)->EnableWindow(FALSE);
			break;
		default:
			GetDlgItem(IDC_NEXWEB_SETUP_TODO_NOTE)->EnableWindow(bEnableToDoDetails);
			break;
	}

	//(e.lally 2011-04-27) PLID 41445 - Make the Assign To users readonly or disabled
	if(m_pToDoAssignTo != NULL){
		GetDlgItem(IDC_NEXWEB_SETUP_TODO_ASSIGNTO)->EnableWindow(TRUE);
		m_pToDoAssignTo->PutReadOnly(_variant_t((long)(bEnableToDoDetails ? FALSE : TRUE), VT_BOOL));
	}
	else {
		GetDlgItem(IDC_NEXWEB_SETUP_TODO_ASSIGNTO)->EnableWindow(FALSE);
	}

	//Previous, Next Event
	{
		GetDlgItem(IDC_BTN_NEXWEB_TODO_EVENT_PREV)->EnableWindow(bEnablePrevEvent);
		GetDlgItem(IDC_BTN_NEXWEB_TODO_EVENT_NEXT)->EnableWindow(bEnableNextEvent);
	}

	//(e.lally 2011-04-27) PLID 41445 - Make the ToDo Events readonly or disabled
	if(m_pEventList != NULL){
		GetDlgItem(IDC_NEXWEB_SETUP_TODO_EVENT_LIST)->EnableWindow(TRUE);
		if(m_nSubdomainID == -1){
			m_pEventList->PutReadOnly(VARIANT_TRUE);
		}
		else{
			m_pEventList->PutReadOnly(VARIANT_FALSE);
		}
	}
	else{
		GetDlgItem(IDC_NEXWEB_SETUP_TODO_EVENT_LIST)->EnableWindow(FALSE);
	}
}

//(e.lally 2010-11-17) PLID 35819 - Clear out the controls' values
// (j.armen 2011-11-22 10:19) - PLID 40420 - Renamed to ResetEventControls and added Action fields
void CLinksSetupDlg::ResetEventControls()
{
	m_checkToDoEnabled.SetCheck(FALSE);
	m_checkActionEnabled.SetCheck(FALSE);

	SetDlgItemText(IDC_NEXWEB_SETUP_TODO_NOTE, "");
	SetDlgItemText(IDC_NEXWEB_SETUP_ACTION_NOTE, "");

	m_pToDoPriority->CurSel = NULL;
	m_pToDoMethod->CurSel = NULL;
	m_pToDoCategory->CurSel = NULL;
}

//(e.lally 2010-11-17) PLID 35819 - Enables/Disables the ToDo template as being active
//(e.lally 2011-04-27) PLID 41445 - Renamed. Use the cached ToDo Event
void CLinksSetupDlg::OnEnableToDo()
{
	try {
		if(m_eEvent != nteInvalid){
			BOOL bEnabled = m_checkToDoEnabled.GetCheck();
			if(!bEnabled){
				//(e.lally 2010-11-22) PLID 41584 - Warn when the user is disabling a ToDo template
				CString strEventDescription = "for this event.";
				switch(m_eEvent){
					case nteNewProspectEntered:
						strEventDescription = "when a new lead or prospect is entered through NexWeb";
						break;
					case ntePatientMessageReceived:
						strEventDescription = "when a patient question or reply is received through NexWeb";
						break;
					//(e.lally 2011-02-28) PLID 42603
					case nteAppointmentRequestReceived:
						strEventDescription = "when a new appointment request is received through NexWeb";
						break;
					// (j.armen 2011-10-10 10:14) - PLID 44208
					case nteAppointmentChangeRequestReceived:
						strEventDescription = "when an existing appointment change is received through NexWeb";
						break;
					// (j.armen 2011-10-10 10:14) - PLID 44208
					case nteAppointmentCancelationRequestReceived:
						strEventDescription = "when an appointment cancellation request is received through NexWeb";
						break;
					default:
						//New events should have a custom message
						ASSERT(FALSE);
						break;
				}
				if (IDYES != MessageBox(FormatString("If you disable this ToDo configuration, "
					"you will no longer receive notifications %s.\r\n"
					"Are you sure you want to disable this ToDo Alarm?", strEventDescription), NULL, MB_YESNO|MB_ICONWARNING)) {
					m_checkToDoEnabled.SetCheck(TRUE);
					EnsureEventControls();
					return;
				}
			}
			//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
			// (j.armen 2011-11-22 10:20) - PLID 40420 - Changed to use NexWebEventsT
			ExecuteParamSql("UPDATE NexWebEventT SET ToDoEnabled = {BIT} WHERE SubdomainID = {INT} AND EventID = {INT} ", bEnabled, m_nSubdomainID, m_eEvent);
		}
		else{
			BOOL bEnabled = m_checkToDoEnabled.GetCheck();
			m_checkToDoEnabled.SetCheck(!bEnabled);
		}
		EnsureEventControls();
	}NxCatchAll(__FUNCTION__);

}

// (j.armen 2011-11-22 10:20) - PLID 40420 - Added action handler for OnEnableAction()
void CLinksSetupDlg::OnEnableAction()
{
	try {
		if(m_eEvent != nteInvalid){
			BOOL bEnabled = m_checkActionEnabled.GetCheck();
			if(!bEnabled){
				// (j.armen 2011-11-22 10:20) - PLID 40420 - Warn when disabling an action template
				CString strEventDescription;
				switch(m_eEvent){
					case nteAppointmentRequestReceived:
						strEventDescription = "appointments will no longer be created when a request is received through NexWeb";
						break;
					default:
						//New events should have a custom message
						// (j.armen 2011-11-22 10:20) - PLID 40420 - set the check to false and throw an exception
						m_checkActionEnabled.SetCheck(FALSE);
						AfxThrowNxException("Error: Unknown NexWeb Event");
						break;
				}
				if (IDYES != MessageBox(FormatString("If you disable this event action, "
					"%s.\r\n"
					"Are you sure you want to disable this action?", strEventDescription), NULL, MB_YESNO|MB_ICONWARNING)) {
						m_checkActionEnabled.SetCheck(TRUE);
						EnsureEventControls();
						return;
				}
			}
			// (j.armen 2011-11-22 10:22) - PLID 40420 - Update to use NexWebEventT
			ExecuteParamSql("UPDATE NexWebEventT SET ActionEnabled = {BIT} WHERE SubdomainID = {INT} AND EventID = {INT} ", bEnabled, m_nSubdomainID, m_eEvent);
		}
		else{
			BOOL bEnabled = m_checkActionEnabled.GetCheck();
			m_checkActionEnabled.SetCheck(!bEnabled);
		}
		EnsureEventControls();
	}NxCatchAll(__FUNCTION__);

}

//(e.lally 2010-11-17) PLID 35819
void CLinksSetupDlg::OnSelChosenToDoEvent(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//(e.lally 2011-04-27) PLID 41445 - Track the current ToDo Event
			m_eEvent = nteInvalid;
			EnsureEventControls();
			return;
		}
		//Get the selected Event value
		m_eEvent = (NexWebEvent)VarLong(pRow->GetValue(telcValue));
		
		LoadEvent();
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819
void CLinksSetupDlg::OnSelChosenToDoPriority(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(m_eEvent != nteInvalid){
			long nPriority = VarLong(pRow->GetValue(tpcValue));
			//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
			//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
			// (j.armen 2011-11-22 10:23) - PLID 40420 - changed to use NexWebEventT
			ExecuteParamSql("UPDATE NexWebEventT SET ToDoPriority = {INT} WHERE SubdomainID = {INT} AND EventID = {INT} ", nPriority, m_nSubdomainID, m_eEvent);
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819
void CLinksSetupDlg::OnSelChosenToDoMethod(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(m_eEvent != nteInvalid){
			CString strMethod = VarString(pRow->GetValue(tmcDescription));
			//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
			//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
			// (j.armen 2011-11-22 10:23) - PLID 40420 - changed to use NexWebEventT
			ExecuteParamSql("UPDATE NexWebEventT SET ToDoTask = {STRING} WHERE SubdomainID = {INT} AND EventID = {INT} ", strMethod, m_nSubdomainID, m_eEvent);
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819
void CLinksSetupDlg::OnSelChosenToDoCategory(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(m_eEvent != nteInvalid){
			long nCategoryID = VarLong(pRow->GetValue(tccID));
			_variant_t varCategoryID = g_cvarNull;
			if(nCategoryID != cnNoCategory){
				varCategoryID = (long)nCategoryID;
			}
			//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
			// (j.armen 2011-11-22 10:23) - PLID 40420 - changed to use NexWebEventT
			ExecuteParamSql("UPDATE NexWebEventT SET ToDoCategoryID = {VT_I4} WHERE SubdomainID = {INT} AND EventID = {INT} ", varCategoryID, m_nSubdomainID, m_eEvent);
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819
BOOL CLinksSetupDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID = LOWORD(wParam);
	
	CString strNewName;

	switch (HIWORD(wParam))
	{	
		default:
			break;
		case EN_KILLFOCUS:
			switch (nID)
			{	
				case IDC_NEXWEB_SETUP_TODO_NOTE:
					try {
						//Find the current record
						IRowSettingsPtr pSelEventRow = m_pEventList->GetCurSel();
						if(m_eEvent == nteInvalid){
							return CNxDialog::OnCommand(wParam, lParam);
						}

						CString strNewNote;
						GetDlgItemText(IDC_NEXWEB_SETUP_TODO_NOTE, strNewNote);
						//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
						//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
						// (j.armen 2011-11-22 10:23) - PLID 40420 - changed to use NexWebEventT
						ExecuteParamSql("UPDATE NexWebEventT SET ToDoNotes = {STRING} WHERE SubdomainID = {INT} AND EventID = {INT} ", strNewNote, m_nSubdomainID, m_eEvent);

					}NxCatchAll("CLinksSetupDlg::OnCommad - EN_KILLFOCUS:IDC_NEXWEB_SETUP_TODO_NOTE");
					break;
				case IDC_NEXWEB_SETUP_ACTION_NOTE: // (j.armen 2011-11-22 10:26) - PLID 40420 - Added handler for action note
					try {
						IRowSettingsPtr pSelEventRow = m_pEventList->GetCurSel();
						if(m_eEvent == nteInvalid){
							return CNxDialog::OnCommand(wParam, lParam);
						}

						CString strNewNote;
						GetDlgItemText(IDC_NEXWEB_SETUP_ACTION_NOTE, strNewNote);
						ExecuteParamSql("UPDATE NexWebEventT SET ActionNotes = {STRING} WHERE SubdomainID = {INT} AND EventID = {INT} ", strNewNote, m_nSubdomainID, m_eEvent);
					}NxCatchAll("CLinksSetupDlg::OnCommand - EN_KILLFOCUS:IDC_NEXWEB_SETUP_ACTION_NOTE");
					break;
				default:
					break;
			}//end killfocus
		break;
	}//	end wParam
	return CNxDialog::OnCommand(wParam, lParam);
}

//(e.lally 2010-11-17) PLID 35819
void CLinksSetupDlg::OnEditingFinishedToDoAssignToUsers(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		//We can quit if they didn't commit the change
		//(e.lally 2011-04-27) PLID 41445 - Quit for invalid subdomain ID too
		if(pRow == NULL || bCommit == FALSE || m_nSubdomainID == -1){
			return;
		}
		switch(nCol){
			case tacSelected: {
				if(m_eEvent != nteInvalid){
					BOOL bSelected = VarBool(varNewValue);
					long nUserID = VarLong(pRow->GetValue(tacUserID));
					if(bSelected == FALSE){
						//(e.lally 2010-11-22) PLID 41584 - Warn if this is the last selected user.
						//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
						//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
						// (j.armen 2011-11-22 10:26) - PLID 40420 - Renamed to use NexWebEventT
						_RecordsetPtr rs = CreateParamRecordset("SELECT COUNT(*) AS UserCount FROM NexWebEventToDoAssignToT "
							"LEFT JOIN NexWebEventT ON NexWebEventToDoAssignToT.NexWebEventID = NexWebEventT.ID "
							"WHERE NexWebEventT.SubdomainID = {INT} AND NexWebEventT.EventID = {INT} "
							"AND NexWebEventToDoAssignToT.UserID <> {INT} ", m_nSubdomainID, m_eEvent, nUserID);
						if(!rs->eof){
							long nOtherAssignedUsers = VarLong(rs->Fields->Item["UserCount"]->Value);
							if(nOtherAssignedUsers == 0){
								CString strWarning = "This is the last assigned user, and ToDo Alarms cannot be created without an active user assigned which will cause an error in NexWeb.\r\n"
									"Are you sure to want to continue unselecting this user?";
								//(e.lally 2010-11-29) PLID 41577 - Give a separate warning if the patient coordinator is the last selected entry.
								if(m_bAssignToPatCood && nUserID != cnPatientCoordinator){
									strWarning = "This is the last assigned user aside from the Patient Coordinator. "
										"If there is no Patient Coordinator selected for the patient, a ToDo Alarm will not be generated, and the patient will receive an error.\r\n"
										"Are you sure to want to continue unselecting this user?";
								}
								if(IDYES != MessageBox(strWarning, NULL, MB_YESNO|MB_ICONWARNING)){
										pRow->PutValue(tacSelected, VARIANT_TRUE);
										return;
								}
							}
						}
					}
					//(e.lally 2010-11-29) PLID 41577 - Handle the patient coordinator entry specially
					if(nUserID == cnPatientCoordinator){
						//(e.lally 2010-11-29) PLID 41577 - Be sure to update the patient coord selection value
						m_bAssignToPatCood = bSelected;
						//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
						//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
						// (j.armen 2011-11-22 10:26) - PLID 40420 - Renamed to use NexWebEventT
						ExecuteParamSql("UPDATE NexWebEventT SET ToDoAssignToPatCoord = {BIT} WHERE SubdomainID = {INT} AND EventID = {INT} ", bSelected, m_nSubdomainID, m_eEvent);
					}
					else {
						if(bSelected != FALSE){
							//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
							//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
							// (j.armen 2011-11-22 10:26) - PLID 40420 - Renamed to use NexWebEventT
							ExecuteParamSql("INSERT INTO NexWebEventToDoAssignToT (NexWebEventID, UserID) "
								"SELECT (SELECT ID FROM NexWebEventT WHERE SubdomainID = {INT} AND EventID = {INT}), {INT} "
								"FROM UsersT WHERE UsersT.PersonID = {INT} "
								"AND UsersT.PersonID NOT IN(SELECT UserID FROM NexWebEventToDoAssignToT "
								"WHERE NexWebEventID = (SELECT ID FROM NexWebEventT WHERE SubdomainID = {INT} AND EventID = {INT}))"
								,m_nSubdomainID, m_eEvent, nUserID, nUserID, m_nSubdomainID, m_eEvent);
						}
						else{
							//(e.lally 2011-04-26) PLID 43445 - Changed to be per subdomain
							//(e.lally 2011-04-27) PLID 41445 - Use the cached ToDo Event
							// (j.armen 2011-11-22 10:26) - PLID 40420 - Renamed to use NexWebEventT
							ExecuteParamSql("DELETE NexWebEventToDoAssignToT "
								"FROM NexWebEventToDoAssignToT "
								"LEFT JOIN NexWebEventT ON NexWebEventToDoAssignToT.NexWebEventID = NexWebEventT.ID "
								"WHERE NexWebEventT.SubdomainID = {INT} AND NexWebEventT.EventID = {INT} "
								"AND NexWebEventToDoAssignToT.UserID = {INT} ",m_nSubdomainID, m_eEvent, nUserID);
						}
					}
				}
				break;
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819
void CLinksSetupDlg::OnRequeryFinishedToDoCategory(short nFlags)
{
	try{
		IRowSettingsPtr pRow = m_pToDoCategory->GetNewRow();
		if(pRow != NULL){
			pRow->PutValue(tccID, (long)cnNoCategory);
			pRow->PutValue(tccDescription, _bstr_t("<No Category>"));
			m_pToDoCategory->AddRowBefore(pRow, m_pToDoCategory->GetFirstRow());
		}
	}NxCatchAll(__FUNCTION__);
}

void CLinksSetupDlg::OnRequeryFinishedToDoAssignToUsers(short nFlags)
{
	try{
		IRowSettingsPtr pRow = m_pToDoAssignTo->GetNewRow();
		if(pRow != NULL){
			//(e.lally 2010-11-29) PLID 41577 - Set the checkbox based on the pat coord value that should have been loaded before now.
			pRow->PutValue(tacSelected, m_bAssignToPatCood ? VARIANT_TRUE : VARIANT_FALSE);
			pRow->PutValue(tacUserID, (long)cnPatientCoordinator);
			pRow->PutValue(tacUsername, _bstr_t("<Patient Coordinator>"));
			m_pToDoAssignTo->AddRowBefore(pRow, m_pToDoAssignTo->GetFirstRow());
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819 - Previous Event
//(e.lally 2011-04-27) PLID 41445 - Renamed. Use the cached ToDo Event
void CLinksSetupDlg::OnPrevToDoEvent()
{
	try{
		IRowSettingsPtr pCurEvent = m_pEventList->GetCurSel();
		if(pCurEvent != NULL){
			IRowSettingsPtr pPrevRow = pCurEvent->GetPreviousRow();
			if(pPrevRow != NULL){
				m_pEventList->CurSel = pPrevRow;
				m_eEvent = (NexWebEvent)VarLong(pPrevRow->GetValue(telcValue));
				LoadEvent();
				GetDlgItem(IDC_BTN_NEXWEB_TODO_EVENT_PREV)->SetFocus();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-11-17) PLID 35819 - Next Event
//(e.lally 2011-04-27) PLID 41445 - Renamed. Use the cached ToDo Event
void CLinksSetupDlg::OnNextToDoEvent()
{
	try{
		IRowSettingsPtr pCurEvent = m_pEventList->GetCurSel();
		if(pCurEvent != NULL){
			IRowSettingsPtr pNextRow = pCurEvent->GetNextRow();
			if(pNextRow != NULL){
				m_pEventList->CurSel = pNextRow;
				m_eEvent = (NexWebEvent)VarLong(pNextRow->GetValue(telcValue));
				LoadEvent();
				GetDlgItem(IDC_BTN_NEXWEB_TODO_EVENT_NEXT)->SetFocus();
			}
		}
	}NxCatchAll(__FUNCTION__);
}



void CLinksSetupDlg::OnBnClickedApptprototypeSetupBtn()
{
	// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes
	try {
		if (g_pLicense->CheckForLicense(CLicense::lcNexWebLeads, CLicense::cflrUse) || g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrUse)) {
			// For now this is considered NexWeb administration, so we restrict it to users who 
			// have that permission.  However, because it involves scheduler info (like resource 
			// names, appointment types, etc.) we should also only allow users who are allowed 
			// to see that stuff, i.e. at least read-only access to the scheduler, to go in here.
			if (CheckCurrentUserPermissions(bioNexwebObjects, sptWrite) && CheckCurrentUserPermissions(bioSchedulerModule, sptView)) {
				CApptPrototypesDlg dlg(this);
				dlg.DoModal();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-27) PLID 41445
void CLinksSetupDlg::EnsureAllControls()
{
	EnsureSubdomainControls();
	EnsureEventControls();
	EnsureDisplaylistControls();
	EnsureContentSettingControls();
}

//(e.lally 2011-04-27) PLID 41445
void CLinksSetupDlg::EnsureSubdomainControls()
{
	BOOL bEnablePrev = FALSE;
	BOOL bEnableNext = FALSE;

	//(e.lally 2011-04-27) PLID 41445 - Make sure we have a current subdomain
	if(m_pSubdomainList != NULL && m_nSubdomainID != -1){
		IRowSettingsPtr pCurRow = m_pSubdomainList->GetCurSel();
		if(pCurRow != NULL){
			if(pCurRow->GetPreviousRow() != NULL){
				bEnablePrev = TRUE;
			}
			if(pCurRow->GetNextRow() != NULL){
				bEnableNext = TRUE;
			}
		}
	}

	//Previous, Next
	{
		GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_PREV)->EnableWindow(bEnablePrev);
		GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_NEXT)->EnableWindow(bEnableNext);
	}

	BOOL bEnableDetails = FALSE;
	if(m_nSubdomainID != -1){
		bEnableDetails = TRUE;
	}

	GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_RENAME)->EnableWindow(bEnableDetails);
	GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_DELETE)->EnableWindow(bEnableDetails);
	GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_MAKE_DEFAULT)->EnableWindow(bEnableDetails);

}

//(e.lally 2011-04-27) PLID 41445
void CLinksSetupDlg::OnSelChangingSubdomainList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOld(lpOldSel);

		//Do not let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-27) PLID 41445
void CLinksSetupDlg::LoadSubdomain()
{
	//Load the ToDo event data
	LoadEvent();

	LoadDisplaylistDetails();

	// (d.singleton 2013-02-15 09:00) - PLID 54979 need to load the data for the content setttings datalist
	LoadContentSettingData();

	EnsureSubdomainControls();
}

//(e.lally 2011-04-27) PLID 41445
void CLinksSetupDlg::OnSelChosenSubdomainList(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_nSubdomainID = -1;
			EnsureAllControls();
			return;
		}
		//Get the selected subdomain ID
		m_nSubdomainID = VarLong(pRow->GetValue(scID));
		
		LoadSubdomain();

	}NxCatchAll(__FUNCTION__);
}


//(e.lally 2011-04-27) PLID 41445 - Previous Subdomain
void CLinksSetupDlg::OnPrevSubdomain()
{
	try{
		IRowSettingsPtr pCurRow = m_pSubdomainList->GetCurSel();
		if(pCurRow != NULL){
			IRowSettingsPtr pPrevRow = pCurRow->GetPreviousRow();
			if(pPrevRow != NULL){
				m_pSubdomainList->CurSel = pPrevRow;
				m_nSubdomainID = VarLong(pPrevRow->GetValue(scID));
				LoadSubdomain();
				GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_PREV)->SetFocus();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-27) PLID 41445 - Next Subdomain
void CLinksSetupDlg::OnNextSubdomain()
{
	try {
		IRowSettingsPtr pCurRow = m_pSubdomainList->GetCurSel();
		if(pCurRow != NULL){
			IRowSettingsPtr pNextRow = pCurRow->GetNextRow();
			if(pNextRow != NULL){
				m_pSubdomainList->CurSel = pNextRow;
				m_nSubdomainID = VarLong(pNextRow->GetValue(scID));
				LoadSubdomain();
				GetDlgItem(IDC_BTN_NEXWEB_SUBDOMAIN_NEXT)->SetFocus();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-27) PLID 41445 - New Subdomain
void CLinksSetupDlg::OnNewSubdomain()
{
	try{
		//Prevent creating a new subdomain if the current one is still named "Default"
		{
			IRowSettingsPtr pCurRow = m_pSubdomainList->GetCurSel();
			if(pCurRow != NULL){
				//Get the selected subdomain name
				CString strName = VarString(pCurRow->GetValue(scName));
				if(strName.CompareNoCase("default") ==0){
					AfxMessageBox("In order to manage more than one subdomain, they should all be named something other than 'Default'. Please rename the current subdomain before creating a new one.", MB_OK|MB_ICONERROR);
					return;
				}
			}
		}

		CString strName;
		CGetNewIDName dlg(this);
		dlg.m_nMaxLength = 100;
		dlg.m_pNewName = &strName;
		dlg.m_strCaption = "Enter a new Subdomain name";

		bool bValid = false;
		while(!bValid){
			if (IDOK != dlg.DoModal()){
				return;
			}
			strName.TrimRight();
			if(strName.IsEmpty()){
				AfxMessageBox("You must enter a name for this Subdomain.");
				continue;
			}

			//Are all the characters valid for a subdomain?
			//We're only going to allow the traditional ASCII letters, numbers and hyphens,
			//but we're not going to check the leading or trailing character rules.
			if(strName != strName.SpanIncluding("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-")){
				AfxMessageBox("Subdomain names may only contain letters, numbers and hyphens.");
				continue;
			}

			//Make sure it is unique
			if(ReturnsRecordsParam("SELECT ID FROM NexWebSubdomainT WHERE Name = {STRING}", strName)){
				AfxMessageBox("This Subdomain name already exists. Please enter a unique Subdomain name.");
				continue;
			}

			bValid = true;
		}

		//Create it
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON \r\n"
			"DECLARE @isDefault BIT \r\n"
			"SET @isDefault = (SELECT CASE WHEN EXISTS(SELECT ID FROM NexWebSubdomainT WHERE [Default] = 1) THEN 0 ELSE 1 END)\r\n"
			"INSERT INTO NexWebSubdomainT (Name, [Default]) "
				"SELECT {STRING}, @isDefault \r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT CONVERT(INT, @@identity) AS ID, @isDefault AS IsDefault", strName);
		if(!rs->eof){
			m_nSubdomainID = VarLong(rs->Fields->Item["ID"]->Value);
			BOOL bDefault = AsBool(rs->Fields->Item["isDefault"]->Value);

			IRowSettingsPtr pNewRow = m_pSubdomainList->GetNewRow();
			if(pNewRow == NULL){
				ASSERT(FALSE);
				//What just happened?
				m_nSubdomainID = -1;
				m_pSubdomainList->Requery();
			}
			else {
				pNewRow->PutValue(scID, (long)m_nSubdomainID);
				pNewRow->PutValue(scName, _bstr_t(strName));
				pNewRow->PutValue(scDefault, _variant_t((long)bDefault, VT_BOOL));

				m_pSubdomainList->AddRowSorted(pNewRow, NULL);
				m_pSubdomainList->CurSel = pNewRow;
			}

			LoadSubdomain();

		}
		rs->Close();
		
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-27) PLID 41445 - Rename Subdomain
void CLinksSetupDlg::OnRenameSubdomain()
{
	try{
		if(m_nSubdomainID == -1 || m_pSubdomainList == NULL){
			return;
		}
		IRowSettingsPtr pCurSubdomain = m_pSubdomainList->CurSel;
		if(pCurSubdomain == NULL){
			return;
		}
		CString strOldName = VarString(pCurSubdomain->GetValue(scName),"");
		CString strNewName = strOldName;
		CGetNewIDName dlg(this);
		dlg.m_nMaxLength = 100;
		dlg.m_pNewName = &strNewName;
		dlg.m_strCaption = "Rename Subdomain";

		bool bValid = false;
		while(!bValid){
			if (IDOK != dlg.DoModal()){
				return;
			}
			strNewName.TrimRight();

			if(strOldName.CompareNoCase(strNewName) == 0) {
				//case may have changed is all
				bValid = true;
				continue;
			}
		
			if(strNewName.IsEmpty()){
				AfxMessageBox("You must enter a name for this Subdomain.");
				continue;
			}

			//Are all the characters valid for a subdomain?
			//We're only going to allow the traditional ASCII letters, numbers and hyphens,
			//but we're not going to check the leading or trailing character rules.
			if(strNewName != strNewName.SpanIncluding("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-")){
				AfxMessageBox("Subdomain names may only contain letters, numbers and hyphens.");
				continue;
			}

			//Make sure it is unique
			if(ReturnsRecordsParam("SELECT ID FROM NexWebSubdomainT WHERE Name = {STRING} AND ID <> {INT} ", strNewName, m_nSubdomainID)){
				AfxMessageBox("This Subdomain name already exists. Please enter a unique Subdomain name.");
				continue;
			}

			bValid = true;
		}

		//update it
		ExecuteParamSql("UPDATE NexWebSubdomainT SET Name = {STRING} WHERE ID = {INT} ", strNewName, m_nSubdomainID);
		pCurSubdomain->PutValue(scName, _bstr_t(strNewName));

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-05-12) PLID 41445 - Deletes the current subdomain and all its settings.
void CLinksSetupDlg::OnDeleteSubdomain()
{
	try{
		IRowSettingsPtr pCurRow = m_pSubdomainList->GetCurSel();
		if(pCurRow == NULL || m_nSubdomainID == -1){
			return;
		}

		if(AsBool(pCurRow->GetValue(scDefault))){
			//We're going to prevent deleting the one flagged as the default
			AfxMessageBox("You may not delete the subdomain that is marked as the default. Please set a different subdomain as the default and try again.", MB_OK|MB_ICONWARNING);
			return;
		}

				
		if(IDYES != AfxMessageBox("This will permanently delete all the settings associated with this subdomain and cannot be undone.\r\n"
			"Are you sure you wish to continue?", MB_YESNO|MB_ICONWARNING)){
				return;
		}

		//Delete all the records associated with this subdomain
		// (j.armen 2011-11-22 10:26) - PLID 40420 - Renamed to use NexWebEventT
		{
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(2000);
			ExecuteParamSql("SET XACT_ABORT ON \r\n"
				"BEGIN TRAN \r\n"
				"DECLARE @SubdomainID INT \r\n"
				"SET @SubdomainID = {INT} \r\n"
				
				"DELETE FROM NexWebEventToDoAssignToT WHERE NexWebEventID IN("
					"SELECT ID FROM NexWebEventT WHERE SubdomainID = @SubdomainID) \r\n"
				"DELETE FROM NexWebEventT WHERE SubdomainID = @SubdomainID \r\n"
				"DELETE FROM NexWebDisplayT WHERE SubdomainID = @SubdomainID \r\n"
				"DELETE FROM NexWebSubdomainContentT WHERE SubdomainID = @SubdomainID \r\n"

				"DELETE FROM NexWebSubdomainT WHERE ID = @SubdomainID \r\n"
				"COMMIT TRAN ", m_nSubdomainID);
		}

		IRowSettingsPtr pPrevRow = pCurRow->GetPreviousRow();
		if(pPrevRow == NULL){
			m_nSubdomainID = -1;
		}
		else {
			m_nSubdomainID = VarLong(pPrevRow->GetValue(scID));
		}
		m_pSubdomainList->PutCurSel(pPrevRow);
		m_pSubdomainList->RemoveRow(pCurRow);
		LoadSubdomain();
		
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-27) PLID 41445 - Makes Subdomain the default
void CLinksSetupDlg::OnMakeDefaultSubdomain()
{
	try{
		if(m_nSubdomainID == -1 || m_pSubdomainList == NULL){
			return;
		}
		IRowSettingsPtr pCurSubdomain = m_pSubdomainList->CurSel;
		if(pCurSubdomain == NULL){
			return;
		}

		//update it
		ExecuteParamSql("UPDATE NexWebSubdomainT SET [Default] = CASE WHEN ID = {INT} THEN 1 ELSE 0 END ", m_nSubdomainID);
		
		IRowSettingsPtr pRow = m_pSubdomainList->GetFirstRow();
		while(pRow){
			if(pRow->IsSameRow(pCurSubdomain) == VARIANT_FALSE){
				pRow->PutValue(scDefault, VARIANT_FALSE);
			}
			else {
				pRow->PutValue(scDefault, VARIANT_TRUE);
			}
			pRow = pRow->GetNextRow();
		}
		
	}NxCatchAll(__FUNCTION__);
}


//(e.lally 2011-04-29) PLID 43480 - Ensures proper displaylist related controls are enabled
void CLinksSetupDlg::EnsureDisplaylistControls()
{
	BOOL bEnableDisplaylist = FALSE;
	BOOL bEnableDetails = FALSE;
	BOOL bEnableSelectAll = FALSE;
	
	if(m_nSubdomainID != -1 && m_bDisplaylistSetupIsValid){
		bEnableDisplaylist = TRUE;

		if(m_nDisplaylistTypeID != -1){
			bEnableSelectAll = (m_nDisplaylistTypeID == nwltEMRTemplates) ? FALSE : TRUE;
			bEnableDetails = TRUE;
		}
	}
	EnableDlgItem(IDC_BTN_NEXWEB_DISPLAY_SELECT_ALL, bEnableSelectAll);

	if(m_pDisplaylistType != NULL){
		GetDlgItem(IDC_NEXWEB_SETUP_DISPLAY_LIST)->EnableWindow(TRUE);
		m_pDisplaylistType->PutReadOnly(_variant_t((long)(bEnableDisplaylist ? FALSE : TRUE), VT_BOOL));
	}
	else {
		GetDlgItem(IDC_NEXWEB_SETUP_DISPLAY_LIST)->EnableWindow(FALSE);
	}

	if(m_pDisplaylistDetails != NULL){
		GetDlgItem(IDC_NEXWEB_SETUP_DISPLAY_DETAIL_LIST)->EnableWindow(TRUE);
		m_pDisplaylistDetails->PutReadOnly(_variant_t((long)(bEnableDetails ? FALSE : TRUE), VT_BOOL));
	}
	else {
		GetDlgItem(IDC_NEXWEB_SETUP_DISPLAY_DETAIL_LIST)->EnableWindow(FALSE);
	}
}

//(e.lally 2011-04-29) PLID 43480 - Refreshes the displaylist details table
void CLinksSetupDlg::LoadDisplaylistDetails()
{
	if(m_nDisplaylistTypeID == nwltEMRTemplates){
		//(e.lally 2011-10-21) PLID 46065 - Don't requery if they aren't licensed for EMR
		if(g_pLicense == NULL || g_pLicense->HasEMROrExpired(CLicense::cflrSilent) != 2) {
			if(m_pDisplaylistDetails!= NULL) {
				m_pDisplaylistDetails->Clear();
				EnsureDisplaylistControls();
			}
			return;
		}
	}

	if(m_pDisplaylistDetails != NULL && m_nDisplaylistTypeID != -1){
		m_pDisplaylistDetails->FromClause = _bstr_t(GetDisplaylistDetailsFromClause((NexWebDisplaylistType) m_nDisplaylistTypeID));
		EnsureDisplaylistColumns();
		m_pDisplaylistDetails->Requery();
	}
	else {
		m_pDisplaylistDetails->Clear();
	}

	EnsureDisplaylistControls();
}

//(e.lally 2011-04-29) PLID 43480 - Ensures the proper columns are showing depending on the displaylist type selected
void CLinksSetupDlg::EnsureDisplaylistColumns()
{
	//For speed of drawing
	m_pDisplaylistDetails->SetRedraw(VARIANT_FALSE);
	//(e.lally 2010-12-06) PLID 41703 - Use an additional information column for EMR and Insurance
	IColumnSettingsPtr pColName = m_pDisplaylistDetails->GetColumn(ddcDataName);
	IColumnSettingsPtr pColFriendly = m_pDisplaylistDetails->GetColumn(ddcFriendlyName);
	IColumnSettingsPtr pColAddInfo1 = m_pDisplaylistDetails->GetColumn(ddcAdditionalInfo1);
	//(e.lally 2009-11-18) PLID 35805 - We aren't allowing users to change the visibility of EMR templates right now.
	//(e.lally 2010-12-06) PLID 41703 - We can now allow user to change the visibility of all EMR templates.
	//(e.lally 2011-05-04) PLID 42209 - Update the sort order
	CString strTitle;
	CString strAdditionalField1 = "AdditionalInfo1";
	switch(m_nDisplaylistTypeID){
		case nwltEMRTemplates:
			pColName->PutStoredWidth(200);
			pColName->ColumnStyle = csWidthData|csVisible;
			//show the additional column
			pColAddInfo1->PutStoredWidth(140);
			pColAddInfo1->ColumnStyle = csWidthData|csVisible;
			pColFriendly->PutStoredWidth(200);
			pColFriendly->ColumnStyle = csVisible|csEditable;
			strTitle = "Collection";
			break;
		case nwltInsuranceCos:
			//show the additional column
			pColName->PutStoredWidth(100);
			pColName->ColumnStyle = csWidthData|csVisible;
			pColAddInfo1->PutStoredWidth(240);
			pColAddInfo1->ColumnStyle = csWidthData|csVisible;
			pColFriendly->PutStoredWidth(200);
			pColFriendly->ColumnStyle = csVisible|csEditable;
			strTitle = "Address";
			break;
		//(e.lally 2011-05-04) PLID 42209 - Display the list name for the details
		case nwltCustomDropdownList1: 
		case nwltCustomDropdownList2:
		case nwltCustomDropdownList3:
		case nwltCustomDropdownList4:
		case nwltCustomDropdownList5:
		case nwltCustomDropdownList6:
			pColName->PutStoredWidth(140);
			pColName->ColumnStyle = csWidthData|csVisible;
			//show the additional column
			pColAddInfo1->PutStoredWidth(140);
			pColAddInfo1->ColumnStyle = csWidthData|csVisible;
			pColFriendly->PutStoredWidth(200);
			pColFriendly->ColumnStyle = csVisible|csEditable;
			strTitle = "Custom List";
			break;
		default:
			//hide the additional column
			pColName->ColumnStyle = csWidthAuto|csVisible;
			pColAddInfo1->PutStoredWidth(0);
			pColAddInfo1->ColumnStyle = csFixedWidth|csVisible;
			pColFriendly->ColumnStyle = csWidthAuto|csVisible|csEditable;
			strAdditionalField1 = "''";
			break;
	}
	pColAddInfo1->ColumnTitle = _bstr_t(strTitle);
	pColAddInfo1->FieldName = _bstr_t(strAdditionalField1);

	//For speed of drawing
	m_pDisplaylistDetails->SetRedraw(VARIANT_TRUE);
}

//(e.lally 2011-04-29) PLID 43480 - Handles selection of new displaylist type
void CLinksSetupDlg::OnSelChosenDisplaylistType(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL){
			m_pDisplaylistDetails->Clear();
			// (b.cardillo 2010-08-18 14:16) - PLID 40172 - Update the enabled state of the "select/unselect all" button
			EnableDlgItem(IDC_BTN_NEXWEB_DISPLAY_SELECT_ALL, FALSE);
			return;
		}
		m_nDisplaylistTypeID = VarLong(pRow->GetValue(dtcTypeID));

		LoadDisplaylistDetails();
		
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-29) PLID 43480 - Ensures edits of displaylist detail data is valid
void CLinksSetupDlg::OnEditingFinishingNexWebListDisplay(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		//(e.lally 2009-12-04) PLID 35805 - We can quit if they didn't commit the change
		if(*pbCommit == FALSE){
			return;
		}
		//(e.lally 2009-11-18) PLID 35805 - We aren't allowing users to change EMR template visibility from this dlg right now.
		//(e.lally 2010-12-06) PLID 41703 - We can now allow user to change the visibility of all EMR templates.
		/*if(m_nDisplaylistTypeID == nwltEMRTemplates && nCol == ldcVisible){
			//don't save					
			*pbCommit = FALSE;
			return;
		}
		else */ 
		if(nCol == ddcFriendlyName){
			//(e.lally 2009-12-04) PLID 35805 - Ensure we are under the max field limit size
			if (pvarNewValue->vt == VT_BSTR) {
				CString str = VarString(*pvarNewValue);
				if(str.GetLength()>255) {
					str = str.Left(255);
					::SetVariantString(*pvarNewValue, str);
					AfxMessageBox("Your description is longer then the maximum amount (255) and has been shortened.\n"
						"Please double-check the description and make changes as needed.");
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-29) PLID 43480 - Saves the changes to the displaylist details
void CLinksSetupDlg::OnEditingFinishedNexWebListDisplay(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		//(e.lally 2009-12-04) PLID 35805 - We can quit if they didn't commit the change
		if(pRow == NULL || bCommit == FALSE){
			return;
		}
		//(e.lally 2009-11-18) PLID 35805 - We just need to know the table name and ID field for our update statements.
		if(m_displaylistSetup.GetSize() < nwltCount_DoNotUse){
			ASSERT(FALSE);
			return;
		}

		
		long nFKID = VarLong(pRow->GetValue(ddcFKID));
		CString strFriendlyName = "";
		BOOL bVisible = FALSE;
		switch(nCol){
			case ddcVisible:
			case ddcFriendlyName:
			{
				bVisible = VarBool(pRow->GetValue(ddcVisible));
 				strFriendlyName = VarString(pRow->GetValue(ddcFriendlyName),"");
				break;
			}
			default:
				ASSERT(FALSE);
				return;
		}
		
		CString strFKIDFieldName = m_displaylistSetup.GetAt(m_nDisplaylistTypeID).strFKIDFieldName;
		CString strSql;
		strSql.Format(
		"IF NOT EXISTS("
			"SELECT NexWebDisplayT.ID FROM NexWebDisplayT "
			"WHERE NexWebDisplayT.SubdomainID = {INT} "
				"AND NexWebDisplayT.%s = {INT}) \r\n"
		"BEGIN \r\n"
			"INSERT INTO NexWebDisplayT (SubdomainID, %s, Visible, FriendlyName) "
			"SELECT {INT}, {INT}, {BIT}, {STRING} \r\n"
		"END "
		"ELSE BEGIN \r\n"
			"UPDATE NexWebDisplayT SET Visible = {BIT}, FriendlyName = {STRING} WHERE SubdomainID = {INT} AND %s = {INT} \r\n"
		"END ", strFKIDFieldName, strFKIDFieldName, strFKIDFieldName);

		ExecuteParamSql(strSql, m_nSubdomainID, nFKID,
			m_nSubdomainID, nFKID, bVisible, strFriendlyName,
			bVisible, strFriendlyName, m_nSubdomainID, nFKID);
		
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-04-29) PLID 43480 - Dynamic From clause for the displaylist details table based on subdomain ID
CString CLinksSetupDlg::GetDisplaylistDetailsFromClause(NexWebDisplaylistType nwListType)
{

	CString strFrom;
	switch(nwListType){
		case nwltReferralSources:
			// (a.wilson 2012-5-10) PLID 50378 - need to incorporate inactive referral sources.
			strFrom.Format("(SELECT ReferralSourceT.PersonID AS ID, ReferralSourceT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
				"FROM ReferralSourceT "
				"INNER JOIN PersonT ON ReferralSourceT.PersonID = PersonT.ID "
				"LEFT JOIN NexWebDisplayT ON ReferralSourceT.PersonID = NexWebDisplayT.ReferralSourceID AND NexWebDisplayT.SubdomainID = %li "
				"WHERE PersonT.Archived = 0 OR NexWebDisplayT.Visible = 1) "
				, m_nSubdomainID);
			break;
		case nwltInsuranceCos:
			//(e.lally 2010-08-27) PLID 40283 - Filter out inactive, but always allow the ones that are marked as visible
			strFrom.Format("(SELECT InsuranceCoT.PersonID AS ID,InsuranceCoT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName, "
				//(e.lally 2010-12-06) PLID 41703 - Add full address for additional info
				"LTRIM(RTRIM(PersonT.Address1 + ' ' + PersonT.Address2 + ' ' + "
				" PersonT.City + CASE WHEN PersonT.City <> '' THEN ', ' ELSE '' END + PersonT.State +  ' ' + PersonT.Zip)) AS AdditionalInfo1 "
			"FROM InsuranceCoT "
			"INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID "
			"LEFT JOIN NexWebDisplayT ON InsuranceCoT.PersonID = NexWebDisplayT.InsuranceCoID AND NexWebDisplayT.SubdomainID = %li "
			"WHERE PersonT.Archived = 0 OR NexWebDisplayT.Visible = 1) "
			, m_nSubdomainID);
			break;
		case nwltEMRTemplates:
			//(e.lally 2010-12-06) PLID 41703 - Add collection for additional info
			strFrom.Format("(SELECT EmrTemplateT.ID, EmrTemplateT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName, "
				"EmrCollectionT.Name as AdditionalInfo1 "
			"FROM EmrTemplateT "
			"LEFT JOIN EmrCollectionT ON EmrTemplateT.CollectionID = EmrCollectionT.ID "
			"LEFT JOIN NexWebDisplayT ON EmrTemplateT.ID = NexWebDisplayT.EmrTemplateID AND NexWebDisplayT.SubdomainID = %li "
			"WHERE EmrTemplateT.Deleted = 0 "
			//(e.lally 2009-11-18) PLID 35805 - Only show the template that is already visible since we only allow one right now
			"AND (NexWebDisplayT.Visible = 1 OR COALESCE(EmrCollectionT.Inactive, 0) = 0)) "
			, m_nSubdomainID);
			break;
		case nwltProcedures:
			//(e.lally 2010-08-27) PLID 40283 - Filter out inactive, but always allow the ones that are marked as visible
			strFrom.Format("(SELECT ProcedureT.ID, ProcedureT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM ProcedureT "
			"LEFT JOIN ProcedureLadderTemplateT ON ProcedureT.ID =ProcedureLadderTemplateT.ProcedureID "
			"LEFT JOIN NexWebDisplayT ON ProcedureT.ID = NexWebDisplayT.ProcedureID AND NexWebDisplayT.SubdomainID = %li "
			"WHERE NexWebDisplayT.Visible = 1 OR ProcedureT.Inactive = 0) "
			, m_nSubdomainID);
			break;
		case nwltCustomFields:
			strFrom.Format("(SELECT CustomFieldsT.ID, CustomFieldsT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM CustomFieldsT "
			"LEFT JOIN NexWebDisplayT ON CustomFieldsT.ID = NexWebDisplayT.CustomFieldID AND NexWebDisplayT.SubdomainID = %li "
			//(e.lally 2010-08-27) PLID 40283 - Restrict to the custom fields NexWeb can currently support
			"WHERE CustomFieldsT.ID IN(1, 2, 3, 4, 11, 12, 13, 14, 15, 16, "
			//(e.lally 2011-05-04) PLID 42209 - Added custom dropdown lists
				"21, 22, 23, 24, 25, 26, "
				"90, 91, 92, 93, 94, 95, 41, 42, 43, 44, 45, 46, 51, 52, 53, 54, 17) "
			//(e.lally 2010-08-27) PLID 40283 - Allow any that are visible
				"OR NexWebDisplayT.Visible = 1) "
			, m_nSubdomainID);
			break;

		//(e.lally 2011-05-04) PLID 42209 - Add more display lists
		case nwltCountries:
			strFrom.Format("(SELECT CountriesT.ID, CountriesT.CountryName AS Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM CountriesT "
			"LEFT JOIN NexWebDisplayT ON CountriesT.ID = NexWebDisplayT.CountryID AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		case nwltLanguages:
			strFrom.Format("(SELECT LanguageT.ID, LanguageT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM LanguageT "
			"LEFT JOIN NexWebDisplayT ON LanguageT.ID = NexWebDisplayT.LanguageID AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		case nwltRaces:
			strFrom.Format("(SELECT RaceT.ID, RaceT.Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM RaceT "
			"LEFT JOIN NexWebDisplayT ON RaceT.ID = NexWebDisplayT.RaceID AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		case nwltEthnicities:
			// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity structure, use the practice name
			strFrom.Format("(SELECT EthnicityT.ID, EthnicityT.Name AS Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM EthnicityT "
			"LEFT JOIN NexWebDisplayT ON EthnicityT.ID = NexWebDisplayT.Ethnicity AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		case nwltPrefixes:
			strFrom.Format("(SELECT PrefixT.ID, PrefixT.Prefix AS Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM PrefixT "
			"LEFT JOIN NexWebDisplayT ON PrefixT.ID = NexWebDisplayT.PrefixID AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		case nwltReferringPhysicians:
			strFrom.Format("(SELECT ReferringPhysT.PersonID AS ID, "
				"(PersonT.Last + ', ' + PersonT.First) AS Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM ReferringPhysT "
			"INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID "
			"LEFT JOIN NexWebDisplayT ON ReferringPhysT.PersonID = NexWebDisplayT.ReferringPhysicianID AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		case nwltPrimaryCareProviders:
			strFrom.Format("(SELECT ReferringPhysT.PersonID AS ID, "
				"(PersonT.Last + ', ' + PersonT.First) AS Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName "
			"FROM ReferringPhysT "
			"INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID "
			"LEFT JOIN NexWebDisplayT ON ReferringPhysT.PersonID = NexWebDisplayT.PriCarePhysID AND NexWebDisplayT.SubdomainID = %li "
			") "
			, m_nSubdomainID);
			break;

		//(e.lally 2011-05-09) PLID 42209 - Split out custom list items into 6 separate entries.
		case nwltCustomDropdownList1: 
		case nwltCustomDropdownList2:
		case nwltCustomDropdownList3:
		case nwltCustomDropdownList4:
		case nwltCustomDropdownList5:
		case nwltCustomDropdownList6:
		{
			long nCustomFieldID = -1;
			switch(nwListType){
				case nwltCustomDropdownList1: nCustomFieldID = 21; break;
				case nwltCustomDropdownList2: nCustomFieldID = 22; break;
				case nwltCustomDropdownList3: nCustomFieldID = 23; break;
				case nwltCustomDropdownList4: nCustomFieldID = 24; break;
				case nwltCustomDropdownList5: nCustomFieldID = 25; break;
				case nwltCustomDropdownList6: nCustomFieldID = 26; break;
			}
			strFrom.Format("(SELECT CustomListItemsT.ID AS ID, CustomListItemsT.Text AS Name, "
				"NexWebDisplayT.Visible, NexWebDisplayT.FriendlyName, "
				"CustomFieldsT.Name AS AdditionalInfo1 "
			"FROM CustomListItemsT "
			"INNER JOIN CustomFieldsT ON CustomListItemsT.CustomFieldID = CustomFieldsT.ID "
			"LEFT JOIN NexWebDisplayT ON CustomListItemsT.ID = NexWebDisplayT.CustomListItemID AND NexWebDisplayT.SubdomainID = %li "
			"WHERE CustomListItemsT.CustomFieldID = %li "
			")"
			, m_nSubdomainID, nCustomFieldID);
		}
		break;

		default:
			AfxThrowNxException("Could not find NexWeb List Display lookup for type number (%li)", (long)nwListType);


	}
	if(!strFrom.IsEmpty()){
		strFrom += " SubQ";
	}
	return strFrom;
}

// (b.cardillo 2010-08-18 14:16) - PLID 40172 - Let the user mark all/none visible in a single click
//(e.lally 2011-04-29) PLID 43480
void CLinksSetupDlg::OnSelectallDisplaylistBtn()
{
	try {
		// We aren't allowing users to change EMR template visibility from this dlg right now.
		if (m_nDisplaylistTypeID == nwltEMRTemplates) {
			return;
		}

		// Get the list of IDs, if any
		CArray<long, long> arListOfIDs;
		bool bSelectAll;
		bool bHasSomeSelected = false;
		{
			bSelectAll = false;
			for (IRowSettingsPtr pRow = m_pDisplaylistDetails->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
				long nID = VarLong(pRow->GetValue(ddcFKID));
				arListOfIDs.Add(nID);
				BOOL bVisible = VarBool(pRow->GetValue(ddcVisible));
				if (!bVisible) {
					// If even one is not selected, then we are in select-all mode, not unselect-all
					bSelectAll = true;
				}
				else {
					bHasSomeSelected = true;
				}
			}
		}

		//(e.lally 2011-05-05) PLID 43480 - It would be a pain to redo a specific set of visible records if only some of the rows were selected previously.
		//	And because we only unselect entries if all are selected, we will just warn when selecting all and some are already visible.	
		if(bSelectAll && bHasSomeSelected){
			if(IDYES != AfxMessageBox("Are you sure you wish to select ALL entries in the list?", MB_YESNO|MB_ICONWARNING)){
				return;
			}
		}
		
		// If there were any entries, then apply our change and then reflect it on screen
		if (arListOfIDs.GetCount() > 0) {
			// Apply the change to data
			{
				CString strTableName = m_displaylistSetup.GetAt(m_nDisplaylistTypeID).strTableName;
				CString strFKIDFieldName = m_displaylistSetup.GetAt(m_nDisplaylistTypeID).strFKIDFieldName;
				CString strPKIDFieldName = m_displaylistSetup.GetAt(m_nDisplaylistTypeID).strPKIDFieldName;
				//Template out our save query then replace the placeholders with actual table and field names
				CString strSql=
					"BEGIN TRAN\r\n"
					"UPDATE NexWebDisplayT SET Visible = {BIT} "
					"FROM <NW_TABLE> "
					"INNER JOIN NexWebDisplayT ON <NW_TABLE>.<NW_PKID> = NexWebDisplayT.<NW_FKID> "
					"WHERE NexWebDisplayT.SubdomainID = {INT} "
						"AND <NW_TABLE>.<NW_PKID> IN({INTARRAY}) "
					"\r\n"
					"IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
					"\r\n"
					"INSERT INTO NexWebDisplayT (SubdomainID, <NW_FKID>, Visible, FriendlyName) "
					"SELECT {INT}, <NW_TABLE>.<NW_PKID>, {BIT}, '' "
					"FROM <NW_TABLE> "
					"LEFT JOIN NexWebDisplayT ON <NW_TABLE>.<NW_PKID> = NexWebDisplayT.<NW_FKID> "
						"AND NexWebDisplayT.SubdomainID = {INT} "
					"WHERE NexWebDisplayT.<NW_FKID> IS NULL "
						"AND <NW_TABLE>.<NW_PKID> IN({INTARRAY}) \r\n"
					"IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n"
					"COMMIT TRAN \r\n";

				strSql.Replace("<NW_TABLE>", strTableName);
				strSql.Replace("<NW_PKID>" , strPKIDFieldName);
				strSql.Replace("<NW_FKID>", strFKIDFieldName);

				//execute it!
				
				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				NxAdo::PushMaxRecordsWarningLimit pmr(500);
				ExecuteParamSql(strSql, 
					bSelectAll, m_nSubdomainID, arListOfIDs,
					m_nSubdomainID, bSelectAll, m_nSubdomainID, arListOfIDs);
			}
			// Reflect the change on screen
			{
				_variant_t varIsVisible = (bSelectAll ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL));

				m_pDisplaylistDetails->SetRedraw(VARIANT_FALSE);
				for (IRowSettingsPtr pRow = m_pDisplaylistDetails->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
					pRow->PutValue(ddcVisible, varIsVisible);
				}
				m_pDisplaylistDetails->SetRedraw(VARIANT_TRUE);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-05-19) PLID 43333 - Initializes the content setting controls
void CLinksSetupDlg::InitializeContentSettingControls()
{
	// (d.singleton 2013-02-15 09:01) - PLID 54979 there is no longer a combo datalist so only one control here
	if(m_pContentSettingDetailTable == NULL){
		m_pContentSettingDetailTable = BindNxDataList2Ctrl(IDC_NEXWEB_SETUP_CONTENT_SETTING_DETAIL_TABLE, false);
	}
}

//(e.lally 2011-05-19) PLID 43333 - Ensures the content setting controls are properly enabled/disabled/read-only
void CLinksSetupDlg::EnsureContentSettingControls()
{
	// (d.singleton 2013-02-15 09:01) - PLID 54979 there is no longer a combo datalist so only one control here
	BOOL bEnableContentSettings = FALSE;
	
	if(m_nSubdomainID != -1){
		bEnableContentSettings = TRUE;
	}

	if(m_pContentSettingDetailTable != NULL){
		GetDlgItem(IDC_NEXWEB_SETUP_CONTENT_SETTING_DETAIL_TABLE)->EnableWindow(TRUE);
		m_pContentSettingDetailTable->PutReadOnly(_variant_t((long)(bEnableContentSettings ? FALSE : TRUE), VT_BOOL));
	}
	else {
		GetDlgItem(IDC_NEXWEB_SETUP_CONTENT_SETTING_DETAIL_TABLE)->EnableWindow(FALSE);
	}
}

// (d.singleton 2013-02-15 09:05) - PLID 54979 took out alot of code here,  moved it all to NexwebTextContentOverrideDlg.cpp

// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Modularized for different kinds of intparam usage
// Writes the given value to the specified setting of this domain. Any failure or misbehavior of 
// any kind results in an exception and nothing is written.
// Returns a recordset with exactly 1 row, with fields "ID" and "OldValue".  If "ID" is NULL it 
// means nothing the value in data already matched nNewValue so nothing needed to be written (and 
// nothing needs to be audited).  If ID is non-null, then a value was inserted or updated so the 
// caller is responsible for auditing the change.  Whether or not ID is null, OldValue is set to 
// the value of the setting in data before this function was called.
_RecordsetPtr SetNexWebContentSetting_IntParam(const CString &strMasterUID, int nSubdomainID, const _variant_t &varNewValue)
{
	return CreateParamRecordset(
		"SET XACT_ABORT ON \r\n"
		"SET NOCOUNT ON \r\n"
		"DECLARE @contentMasterUID UNIQUEIDENTIFIER SET @contentMasterUID = {STRING} \r\n"
		"DECLARE @subdomainID INT SET @subdomainID = {INT} \r\n"
		"DECLARE @newValue INT SET @newValue = {VT_I4} \r\n"
		"DECLARE @countAffected INT \r\n"
		"BEGIN TRAN \r\n"
		"  DECLARE @id INT, @oldValue INT \r\n"
		"  SELECT @id = ID, @oldValue = IntParam FROM NexwebSubdomainContentT WITH (UPDLOCK, HOLDLOCK) WHERE ContentMasterUID = @contentMasterUID AND SubdomainID = @subdomainID \r\n"
		"  IF (@id IS NOT NULL) BEGIN \r\n"
		"    IF ((@oldValue IS NULL AND @newValue IS NOT NULL) OR (@oldValue IS NOT NULL AND @newValue IS NULL) OR (@oldValue <> @newValue)) BEGIN \r\n"
		"      UPDATE NexWebSubdomainContentT SET IntParam = @newValue WHERE ID = @id \r\n"
		"      SET @countAffected = @@ROWCOUNT \r\n"
		"    END ELSE BEGIN \r\n"
		"      SET @countAffected = 1 \r\n"
		"      SET @id = NULL \r\n"
		"    END \r\n"
		"  END ELSE BEGIN \r\n"
		"    INSERT INTO NexWebSubdomainContentT (ContentMasterUID, SubdomainID, IntParam) \r\n"
		"     VALUES (@contentMasterUID, @subdomainID, @newValue) \r\n"
		"    SET @countAffected = @@ROWCOUNT \r\n"
		"    SET @id = SCOPE_IDENTITY() \r\n"
		"  END \r\n"
		"  IF (@countAffected <> 1) BEGIN \r\n"
		"    RAISERROR('Error saving data in NexWebSubdomainContentT', 16, 1) \r\n" 
		"    ROLLBACK TRAN \r\n"
		"    RETURN \r\n"
		"  END \r\n"
		"COMMIT TRAN \r\n"
		" \r\n"
		"SELECT @id AS ID, @oldValue AS OldValue \r\n"
		""
		, strMasterUID
		, nSubdomainID
		, varNewValue
		);
}

// (b.cardillo 2013-03-12 15:55) - PLID 55536 - Separated this out to be more modular
void AuditNexWebContentSetting_(const CString &strDescription, const CString &strSubdomainName, const CString &strOldValue, const CString &strNewValue)
{
	// Calculate the subdomain part of the audit text
	CString strSubdomain = (!strSubdomainName.IsEmpty() ? (" -(" + strSubdomainName + ")") : "");

	// Audit "description: value -(subdomain)" for old and new text
	AuditEvent(-1
		, GetCurrentUserName()
		, BeginNewAuditEvent()
		, aeiNexWebContentSetting
		, -1
		, strDescription + ": " + strOldValue + strSubdomain
		, strDescription + ": " + strNewValue + strSubdomain
		, 3
		);
}

// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
void AuditNexWebContentSetting_Text(const CString &strDescription, const CString &strSubdomainName, const _variant_t &varOldValue, const _variant_t &varNewValue)
{
	AuditNexWebContentSetting_(strDescription, strSubdomainName, AsString(varOldValue), AsString(varNewValue));
}

// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Modularized for different kinds of intparam usage
void AuditNexWebContentSetting_IntParam(const CString &strDescription, const CString &strSubdomainName, const _variant_t &varOldValue, const _variant_t &varNewValue, bool bAuditAsBoolean)
{
	// (d.singleton 2013-02-07 15:41) - PLID 41435, 53986 need to audit changes

	// Calculate the old and new value text as appropriate
	CString strNew, strOld;
	if (bAuditAsBoolean) {
		strNew = varNewValue.vt == VT_NULL ? "" : (AsBool(varNewValue) ? "True" : "False");
		strOld = varOldValue.vt == VT_NULL ? "" : (AsBool(varOldValue) ? "True" : "False");
	} else {
		strNew = varNewValue.vt == VT_NULL ? "" : AsString(varNewValue);
		strOld = varOldValue.vt == VT_NULL ? "" : AsString(varOldValue);
	}

	AuditNexWebContentSetting_(strDescription, strSubdomainName, strOld, strNew);
}

// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Modularized for different kinds of intparam usage
void CLinksSetupDlg::ChangeContentSettingValue_IntParam(LPDISPATCH lpRow, const VARIANT FAR& varNewValue, bool bTreatAsBoolean)
{
	IRowSettingsPtr pRow(lpRow);
	_RecordsetPtr prs = SetNexWebContentSetting_IntParam(VarString(pRow->GetValue(cdcMasterUID))
		, m_nSubdomainID
		, varNewValue
		);
	long nID = AdoFldLong(prs, _T("ID"), -1);
	if (nID != -1) {
		// It changed, need to audit
		_variant_t varOldValue = prs->GetCollect(_T("OldValue"));
		CString strSubdomainName;
		IRowSettingsPtr pSubdomainRow = m_pSubdomainList->GetCurSel();
		if (pSubdomainRow) {
			strSubdomainName = VarString(pSubdomainRow->GetValue(scName), "");
		}					
		AuditNexWebContentSetting_IntParam(VarString(pRow->GetValue(cdcText), ""), strSubdomainName, varOldValue, varNewValue, bTreatAsBoolean);
		// Also make sure our ID column in the datalist2 is set, since this may have inserted a row in data
		pRow->PutValue(cdcID, nID);
		// Also write our new value, since it could be slightly different from what the user entered (e.g. the user typed "-0", but from data it would be "0")
		pRow->PutValue(cdcIntParam, varNewValue);
	} else {
		// Put the old value on screen, because that's what's in data (this should never be different from what's already on screen, but we do it for completeness)
		pRow->PutValue(cdcIntParam, prs->GetCollect(_T("OldValue")));
	}
}

//(e.lally 2011-05-19) PLID 43333 - Saves any changes to the edited line
void CLinksSetupDlg::OnEditingFinishedContentSettingDetailTable(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		if(bCommit == FALSE || lpRow == NULL){
			return;
		}
		switch(nCol) {			
			// (d.singleton 2013-02-01 11:33) - PLID 41435, 53986 save the value if it changed
			case cdcIntParam:
			{
				IRowSettingsPtr pRow(lpRow);
				try {
					CString strContentType = VarString(pRow->GetValue(cdcContentTypeUID));
					if (strContentType.CompareNoCase(CONTENT_TYPE_BOOLEAN) == 0) {
						// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Call the function to do all this work
						ChangeContentSettingValue_IntParam(pRow, VarBool(varNewValue, FALSE) ? _variant_t((long)1) : _variant_t((long)0), true);
					} else if (strContentType.CompareNoCase(CONTENT_TYPE_INTEGER) == 0) {
						// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Handle actual integer values (null allowed currently)
						_variant_t varNewIntVal;
						{
							CString val = (LPCTSTR)VarString(varNewValue);
							val.Trim();
							if (!val.IsEmpty()) {
								varNewIntVal = _variant_t(_ttol(val));
							} else {
								varNewIntVal = g_cvarNull;
							}
						}
						// Call the standard mechanism for writing/auditing an intparam change
						ChangeContentSettingValue_IntParam(pRow, varNewIntVal, false);
					} else {
						ThrowNxException(_T("Unknown content type '" + strContentType + "'."));
					}
				} NxCatchAllCall(__FUNCTION__, {
					// Failure.  Revert to old value on screen
					pRow->PutValue(cdcIntParam, varOldValue);
				});
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

void CLinksSetupDlg::OnEditingFinishingContentSettingDetailTable(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if(*pbCommit == FALSE){
			return;
		}

		switch(nCol) {			
		case cdcIntParam:
			{
				BOOL bValueAllowed;
				CString strAllowedRange;
				IRowSettingsPtr pRow(lpRow);
				CString strContentType = VarString(pRow->GetValue(cdcContentTypeUID));
				if (strContentType.CompareNoCase(CONTENT_TYPE_BOOLEAN) == 0) {
					// No validation necessary for checkboxes
					bValueAllowed = TRUE;
				} else if (strContentType.CompareNoCase(CONTENT_TYPE_INTEGER) == 0) {
					// Integers can be limited by min and max, but null is always allowed (currently).  Get 
					// the value and then if it's not null then check if we have a min/max to look for.  
					// Currently we don't bother making sure the user didn't type alpha characters, as we'll 
					// just treat that as a 0.  But we could add better warnings in the future.
					{
						// Get the text the user typed and convert it to null or long
						_variant_t varNewIntVal;
						{
							CString val = (LPCTSTR)VarString(*pvarNewValue);
							val.Trim();
							if (!val.IsEmpty()) {
								varNewIntVal = _variant_t(_ttol(val));
							} else {
								varNewIntVal = g_cvarNull;
							}
						}
						// If not null, then it'll be long and we need to validate
						if (varNewIntVal.vt != VT_NULL) {
							long nNewVal = VarLong(varNewIntVal);
							_variant_t varMin = pRow->GetValue(cdcIntParamMin);
							_variant_t varMax = pRow->GetValue(cdcIntParamMax);
							if (varMin.vt != VT_NULL && varMax.vt != VT_NULL) {
								long nMin = VarLong(varMin);
								long nMax = VarLong(varMax);
								if (nNewVal < nMin || nNewVal > nMax) {
									// Out of range
									bValueAllowed = FALSE;
									strAllowedRange.Format(_T("between %li and %li, inclusive"), nMin, nMax);
								} else {
									bValueAllowed = TRUE;
								}
							} else if (varMin.vt != VT_NULL) {
								long nMin = VarLong(varMin);
								if (nNewVal < nMin) {
									// Out of range
									bValueAllowed = FALSE;
									strAllowedRange.Format(_T("no smaller than %li"), nMin);
								} else {
									bValueAllowed = TRUE;
								}
							} else if (varMax.vt != VT_NULL) {
								long nMax = VarLong(varMax);
								if (nNewVal > nMax) {
									// Out of range
									bValueAllowed = FALSE;
									strAllowedRange.Format(_T("no larger than %li"), nMax);
								} else {
									bValueAllowed = TRUE;
								}
							} else {
								bValueAllowed = TRUE;
							}
						} else {
							// It was null so it's allowed
							bValueAllowed = TRUE;
						}
					}
				}

				// If invalid, warn the user and don't let him save
				if (!bValueAllowed) {
					MessageBox(FormatString("Invalid value.  Please enter a value %s.", strAllowedRange), NULL, MB_OK|MB_ICONWARNING);
					*pbCommit = FALSE;
					*pbContinue = FALSE;
					return;
				}
			}
			break;
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2011-11-22 10:26) - PLID 40420 - returns true when the cached event can create a ToDo
bool CLinksSetupDlg::HasToDoEvent()
{
	try 
	{
		switch(m_eEvent)
		{
			//Actions that can Create ToDo's
			case nteNewProspectEntered:
			case ntePatientMessageReceived:
			case nteAppointmentRequestReceived:
			case nteAppointmentChangeRequestReceived:
			case nteAppointmentCancelationRequestReceived:
				return true;
			//Actions that can not Create ToDo's
				//<none>
			//It should be impossible for this to be Invalid, but just in case throw an exception.
			//We will catch it to warn the user, and then return false.  New Events should be added
			//to return either true or false
			case nteInvalid:
			default:
				AfxThrowNxException("Error: HasToDoEvent::Invalid NexWeb Event");
				break;
		}
	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.armen 2011-11-22 10:26) - PLID 40420 - returns true when the cached event can do an Action
bool CLinksSetupDlg::HasActionEvent()
{
	try 
	{
		switch(m_eEvent)
		{
			//Actions that can Create Actions
			case nteAppointmentRequestReceived:
				return true;
			//Actions that can not Create Actions
			case nteNewProspectEntered:
			case ntePatientMessageReceived:
			case nteAppointmentChangeRequestReceived:
			case nteAppointmentCancelationRequestReceived:
				return false;
			//It should be impossible for this to be Invalid, but just in case throw an exception.
			//We will catch it to warn the user, and then return false.  New Events should be added
			//to return either true or false
			case nteInvalid:
			default:
				AfxThrowNxException("Error: HasActionEvent::Invalid NexWeb Event");
				break;
		}
	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.armen 2011-11-22 10:26) - PLID 40420 - returns the name of the action that an event will perform.
// when HasActionEvent() returns true, then an entry must be added here as well to return a string that
// will fill the check box label
CString CLinksSetupDlg::GetActionEventName()
{
	try 
	{
		switch(m_eEvent)
		{
			case nteAppointmentRequestReceived:
				return "Create Appointment";
			case nteNewProspectEntered:
			case ntePatientMessageReceived:
			case nteAppointmentChangeRequestReceived:
			case nteAppointmentCancelationRequestReceived:
			case nteInvalid:
			default:
				AfxThrowNxException("Error: GetActionEventName::Invalid NexWeb Event");
				break;
		}
	}NxCatchAll(__FUNCTION__);

	return "Error: Invalid NexWeb Event";
}

// (b.savon 2012-07-26 11:20) - PLID 50585 - Show the dialog
void CLinksSetupDlg::OnBnClickedBtnNexwebPasswordReq()
{
	try{

		CConfigureNexWebPasswordComplexityRequirementsDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-02-15 09:09) - PLID 54979 manually build the content settings datalist
void CLinksSetupDlg::LoadContentSettingData()
{	
	m_pContentSettingDetailTable->Clear();

	// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Handle actual integer type values, including min/max enforcement
	//get settings and data
	_RecordsetPtr prs = CreateParamRecordset( 
		"SELECT M.UID AS ContentMasterUID, M.Name, M.ContentTypeUID, M.Description, M.IntParamMin, M.IntParamMax, C.ID, C.IntParam \r\n "
		"FROM NexWebContentMasterT M \r\n"
		"LEFT JOIN NexwebSubdomainContentT C ON C.ContentMasterUID = M.UID AND C.SubdomainID = {INT} \r\n"
		"WHERE M.ContentTypeUID <> '{9DDDC05A-5153-40EE-9335-76BA43C39FC1}' \r\n"
		"UNION ALL \r\n"
		"SELECT M.UID AS ContentMasterUID, M.Name, M.ContentTypeUID, M.Description, M.IntParamMin, M.IntParamMax, NULL AS ID, NULL AS IntParam \r\n"
		"FROM NexWebContentMasterT M WHERE M.ContentTypeUID = '{9DDDC05A-5153-40EE-9335-76BA43C39FC1}'", m_nSubdomainID);

	CString strUID, strName, strContentTypeUID, strDescription;
	long nID;
	IRowSettingsPtr pRow;
	// (d.singleton 2013-02-15 09:09) - PLID 54979 add the settings to the datalist
	while(!prs->eof) {
		strUID = AdoFldString(prs, "ContentMasterUID", "");
		strName = AdoFldString(prs, "Name", "");
		strDescription = AdoFldString(prs, "Description", "");
		strContentTypeUID = AdoFldString(prs, "ContentTypeUID", "");
		_variant_t varIntParam = prs->GetCollect(_T("IntParam"));
		nID = AdoFldLong(prs, "ID", -1);

		pRow = m_pContentSettingDetailTable->GetNewRow();
		pRow->PutValue(cdcID, g_cvarNull);
		pRow->PutValue(cdcMasterUID, _bstr_t(strUID));
		pRow->PutValue(cdcText, _bstr_t(strName));
		pRow->PutValue(cdcDescription, _bstr_t(strDescription));
		// (d.singleton 2013-02-15 09:09) - PLID 54979 Branch depending on content type and load values
		if(strContentTypeUID.CompareNoCase(CONTENT_TYPE_BOOLEAN) == 0) {
			pRow->PutRefCellFormatOverride(cdcIntParam, m_pfsBoolean);
			pRow->PutValue(cdcIntParam, VarLong(varIntParam, 0) ? g_cvarTrue : g_cvarFalse);
			pRow->PutValue(cdcIntParamMin, g_cvarNull);
			pRow->PutValue(cdcIntParamMax, g_cvarNull);
			if(nID != -1) {
				pRow->PutValue(cdcID, nID);
		}
		}
		else if(strContentTypeUID.CompareNoCase(CONTENT_TYPE_PLUGIN_SECTION_TEXT) == 0) {
			pRow->PutRefCellFormatOverride(cdcIntParam, m_pfsText);
			pRow->PutValue(cdcIntParam, _bstr_t("<details>"));
			pRow->PutValue(cdcIntParamMin, g_cvarNull);
			pRow->PutValue(cdcIntParamMax, g_cvarNull);
		} else if (strContentTypeUID.CompareNoCase(CONTENT_TYPE_INTEGER) == 0) {
			// (b.cardillo 2013-02-19 14:51) - PLID 55209 - Handle actual integer type values, including min/max enforcement
			pRow->PutRefCellFormatOverride(cdcIntParam, m_pfsInteger);
			pRow->PutValue(cdcIntParam, varIntParam.vt == VT_NULL ? g_cvarNull : _variant_t(VarLong(varIntParam)));
			pRow->PutValue(cdcIntParamMin, prs->GetCollect(_T("IntParamMin")));
			pRow->PutValue(cdcIntParamMax, prs->GetCollect(_T("IntParamMax")));
			if (nID != -1) {
				pRow->PutValue(cdcID, nID);
			}
		} else if (strContentTypeUID.CompareNoCase(CONTENT_TYPE_PLAIN_TEXT) == 0) {
			// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
			pRow->PutRefCellFormatOverride(cdcIntParam, m_pfsText);
			pRow->PutValue(cdcIntParam, _bstr_t("<details>"));
			pRow->PutValue(cdcIntParamMin, g_cvarNull);
			pRow->PutValue(cdcIntParamMax, g_cvarNull);
		} else if (strContentTypeUID.CompareNoCase(CONTENT_TYPE_HTML_TEXT) == 0) {
			// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
			pRow->PutRefCellFormatOverride(cdcIntParam, m_pfsText);
			pRow->PutValue(cdcIntParam, _bstr_t("<details>"));
			pRow->PutValue(cdcIntParamMin, g_cvarNull);
			pRow->PutValue(cdcIntParamMax, g_cvarNull);
		} else {
			ThrowNxException(_T("Unknown content type uid '%s'."), strContentTypeUID);
		}
		pRow->PutValue(cdcContentTypeUID, _bstr_t(strContentTypeUID)); 
		m_pContentSettingDetailTable->AddRowSorted(pRow, NULL);

		prs->MoveNext();
	}
	EnsureContentSettingControls();
}

// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
// Returns a recordset with exactly 1 row, with fields "ID" and "OldValue".  If "ID" is NULL it 
// means the value in data already matched varNewValue so nothing needed to be written (and 
// nothing needs to be audited).  If ID is non-null, then a value was inserted or updated so the 
// caller is responsible for auditing the change.  Whether or not ID is null, OldValue lets you 
// know the value of the setting in data before this function was called.
_RecordsetPtr SetNexWebContentSetting_Text(const CString &strMasterUID, int nSubdomainID, const _variant_t &varNewValue)
{
	return CreateParamRecordset(
		"SET XACT_ABORT ON \r\n"
		"SET NOCOUNT ON \r\n"
		"DECLARE @contentMasterUID UNIQUEIDENTIFIER SET @contentMasterUID = {STRING} \r\n"
		"DECLARE @subdomainID INT SET @subdomainID = {INT} \r\n"
		"DECLARE @newValue NVARCHAR(MAX) SET @newValue = {VT_BSTR} \r\n"
		"DECLARE @countAffected INT \r\n"
		"BEGIN TRAN \r\n"
		"  DECLARE @id INT, @oldValue NVARCHAR(MAX) \r\n"
		"  SELECT @id = ID, @oldValue = [Text] COLLATE SQL_Latin1_General_Cp1251_CS_AS FROM NexwebSubdomainContentT WITH (UPDLOCK, HOLDLOCK) WHERE ContentMasterUID = @contentMasterUID AND SubdomainID = @subdomainID \r\n"
		"  IF (@id IS NOT NULL) BEGIN \r\n"
		"    IF ((@oldValue IS NULL AND @newValue IS NOT NULL) OR (@oldValue IS NOT NULL AND @newValue IS NULL) OR (@oldValue <> @newValue COLLATE SQL_Latin1_General_Cp1251_CS_AS)) BEGIN \r\n"
		"      UPDATE NexWebSubdomainContentT SET [Text] = @newValue WHERE ID = @id \r\n"
		"      SET @countAffected = @@ROWCOUNT \r\n"
		"    END ELSE BEGIN \r\n"
		"      SET @countAffected = 1 \r\n"
		"      SET @id = NULL \r\n"
		"    END \r\n"
		"  END ELSE BEGIN \r\n"
		"    INSERT INTO NexWebSubdomainContentT (ContentMasterUID, SubdomainID, [Text]) \r\n"
		"     VALUES (@contentMasterUID, @subdomainID, @newValue) \r\n"
		"    SET @countAffected = @@ROWCOUNT \r\n"
		"    SET @id = SCOPE_IDENTITY() \r\n"
		"  END \r\n"
		"  IF (@countAffected <> 1) BEGIN \r\n"
		"    RAISERROR('Error saving data in NexWebSubdomainContentT', 16, 1) \r\n" 
		"    ROLLBACK TRAN \r\n"
		"    RETURN \r\n"
		"  END \r\n"
		"COMMIT TRAN \r\n"
		" \r\n"
		"SELECT @id AS ID, @oldValue AS OldValue \r\n"
		""
		, strMasterUID
		, nSubdomainID
		, varNewValue
		);
}

// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
void CLinksSetupDlg::ChangeContentSettingValue_Text(LPDISPATCH lpRow, const VARIANT FAR& varNewValue)
{
	IRowSettingsPtr pRow(lpRow);
	_RecordsetPtr prs = SetNexWebContentSetting_Text(
		  VarString(pRow->GetValue(cdcMasterUID))
		, m_nSubdomainID
		, varNewValue
		);
	long nID = AdoFldLong(prs, _T("ID"), -1);
	if (nID != -1) {
		// It changed, need to audit
		_variant_t varOldValue = prs->GetCollect(_T("OldValue"));
		CString strSubdomainName;
		IRowSettingsPtr pSubdomainRow = m_pSubdomainList->GetCurSel();
		if (pSubdomainRow) {
			strSubdomainName = VarString(pSubdomainRow->GetValue(scName), "");
		}					
		AuditNexWebContentSetting_Text(VarString(pRow->GetValue(cdcText), ""), strSubdomainName, varOldValue, varNewValue);
		// Also make sure our ID column in the datalist2 is set, since this may have inserted a row in data
		pRow->PutValue(cdcID, nID);
		/* // For now all text type settings are edited in a popup dialog, not inline, so we don't want to update any cell in the dl right now
		// Also write our new value, since it could be slightly different from what the user entered (e.g. the user typed "-0", but from data it would be "0")
		//pRow->PutValue(cdcIntParam, varNewValue);
		*/
	} else {
		/* // For now all text type settings are edited in a popup dialog, not inline, so we don't want to update any cell in the dl right now
		// Put the old value on screen, because that's what's in data (this should never be different from what's already on screen, but we do it for completeness)
		pRow->PutValue(cdcIntParam, prs->GetCollect(_T("OldValue")));
		*/
	}
}

// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
CString GetNexWebContentSettingValue_Text(INT nSubdomainID, const CString &strMasterUID)
{
	_RecordsetPtr prs = CreateParamRecordset(_T("SELECT ISNULL((")
						_T("SELECT [Text] FROM NexWebSubdomainContentT WHERE SubdomainID = {INT} AND ContentMasterUID = {STRING}")
						_T("), '')"), nSubdomainID, strMasterUID);
	return prs->GetCollect((long)0);
}

// (d.singleton 2013-02-14 17:56) - PLID 55199 open seperate dialog for the text content setting values
void CLinksSetupDlg::LeftClickNexwebSetupContentSettingDetailTable(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		
		if(pRow) {
			// (b.cardillo 2013-03-12 15:55) - PLID 55536 - Reorganized this implementation slightly to avoid 
			// duplication and so variables would be local to the areas of code that need them
			if (nCol == cdcIntParam) {
				//make sure we have a valid subdomain selected
				if(m_nSubdomainID == NULL || m_nSubdomainID == -1) {
					AfxMessageBox("Please select a valid subdomain");
					return;
				}

				// Respond appropriately depending on the kind of setting we're on
				CString strContentType = VarString(pRow->GetValue(cdcContentTypeUID));
				if (strContentType.CompareNoCase(CONTENT_TYPE_PLUGIN_SECTION_TEXT) == 0) {
					CString strMasterUID = VarString(pRow->GetValue(cdcMasterUID));				
					CString strSettingName = VarString(pRow->GetValue(cdcText));

					CNexwebTextContentOverrideDlg dlg(m_nSubdomainID, strMasterUID, strSettingName, this);								
					dlg.DoModal();
				} else if (strContentType.CompareNoCase(CONTENT_TYPE_HTML_TEXT) == 0) {
					// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
					CString strValue = GetNexWebContentSettingValue_Text(m_nSubdomainID, VarString(pRow->GetValue(cdcMasterUID)));
					if (CHtmlEditorDlg::InputHtmlBox(this, VarString(pRow->GetValue(cdcText)), _T("NexWebSubdomainSettings"), strValue) == IDOK) {
						ChangeContentSettingValue_Text(pRow, AsVariant(strValue));
					}
				} else if (strContentType.CompareNoCase(CONTENT_TYPE_PLAIN_TEXT) == 0) {
					// (b.cardillo 2013-03-12 15:55) - PLID 55536 - add support for plain text and html settings
					
					CString strValue = GetNexWebContentSettingValue_Text(m_nSubdomainID, VarString(pRow->GetValue(cdcMasterUID)));
					// (s.dhole 2014-05-03-12 15:55) - PLID 61844 - add Direct Messaging Disclaimer
					if (VarString(pRow->GetValue(cdcMasterUID)) == "{EED57EFD-7B23-4D57-B25C-2895603059AE}")
					{
						// allow only 250 characters
						if (InputBoxLimited(this, VarString(pRow->GetValue(cdcText)) + ":", strValue, "", 250, false, false, "Cancel") == IDOK) {
							ChangeContentSettingValue_Text(pRow, AsVariant(strValue));
						}
					}
					else  if (InputBox(this, VarString(pRow->GetValue(cdcText)) + ":", strValue, "") == IDOK) {
						ChangeContentSettingValue_Text(pRow, AsVariant(strValue));
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}
