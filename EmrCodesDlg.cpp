// EmrCodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "GlobalFinancialUtils.h"
#include "EMRSelectServiceDlg.h"
#include "EmrCodesDlg.h"
#include "EmrTreeWnd.h"
#include "EMN.h"
#include "EMR.h"
#include "ChargeSplitDlg.h"
#include "EMRProblemChooserDlg.h"
#include "EMRProblemEditDlg.h"
#include "EmrFrameWnd.h"
#include "EmrColors.h"
#include "CPTCodes.h"
#include "HL7Utils.h"
#include "MedlinePlusUtils.h"
#include "InvEditDlg.h"
#include "NxModalParentDlg.h"
#include "EMRChargePromptDlg.h"
#include "EmrCodingGroupManager.h"
#include "EMNMedication.h"
#include "DiagSearchUtils.h"
#include "DiagSearchConfig.h"
#include "EMRProblemListDlg.h"
#include "DiagQuickListDlg.h"
#include <NxUILib\NxMenuCommandMap.h>
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <foreach.h>
#include "NexCodeDlg.h" // (b.savon 2014-03-07 12:13) - PLID 60987 - We need NexCode dialog
// (a.walling 2014-03-12 12:31) - PLID 61334 - #import of API in stdafx causes crash in cl.exe
#include "NexGEM.h"
#include "DiagQuickListUtils.h"
#include "NexCodeDlg.h" // (b.savon 2014-03-07 12:13) - PLID 60987 - We need NexCode dialog
#include "SelectDlg.h" // (r.farnworth 2014-04-01 09:29) - PLID 61608
#include "DiagCodeInfo.h"
#include "DiagCodeInfoFwd.h"
#include "ReplaceDiagCodeDlg.h"

// (s.dhole 2014-02-14 14:48) - PLID 60742 Move Code from <More Info> dialog
// (r.farnworth 2014-02-18 09:01) - PLID 60746 - Brought over codes used for diagnosis codes section
#define ID_REMOVE_ICD9	44928
#define ID_REMOVE_CHARGE 44929
#define ID_ADD_DEFAULT_ICD9S 44931
#define ID_ADD_ICD9_PROBLEM 44935
#define ID_EDIT_ICD9_PROBLEM 44936
#define ID_ADD_CHARGE_PROBLEM 44937
#define ID_EDIT_CHARGE_PROBLEM 44938
#define ID_LINK_ICD9_PROBLEMS 44941
#define ID_LINK_CHARGE_PROBLEMS 44942
// (c.haag 2010-10-06 09:21) - PLID 40384 - Menu commands for editing master lists
#define ID_EDIT_CPT_CODES	51250
#define ID_EDIT_INV_ITEMS	51251
#define ID_EDIT_CHARGE		51252 // (z.manning 2011-07-12 16:45) - PLID 44469
#define ID_RESEARCH_ICD10_CODE 51253 // (r.farnworth 2014-03-14 15:22) - PLID 61380 
#define	ID_REPLACE_DIAG_CODE	51254	// (j.jones 2014-12-23 10:48) - PLID 64491

// CEmrCodesDlg dialog
using namespace NXDATALIST2Lib ;
using namespace ADODB;

// (r.farnworth 2014-02-14 15:15) - PLID 60745 - Column enum for the Visit type Datalist
enum VisitTypeColumns {

	vtcID = 0,
	vtcName,
};


extern CPracticeApp theApp;
// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
	enum EChargeListColumns {
		clcID,
		clcServiceID,
		clcProblemIcon,	// (j.jones 2008-07-21 14:02) - PLID 30792
		clcCode,
		clcSubcode,
		clcCategory,// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		clcDescription,
		clcMod1,
		clcMod2,
		clcMod3,
		clcMod4,
		clcQuantity,
		clcCost,
		clcEMNChargePtr,	//DRT 1/11/2007 - PLID 24220
		clcWhichcodes,		//DRT 1/11/2007 - PLID 24177
		clcBillable,
	};

	// (r.farnworth 2014-02-20 11:33) - PLID 60746 - Brought over from MoreInfo
	// Labels for the columns of the IDC_DIAGS datalist referenced by m_DiagCodeList
	//TES 2/26/2014 - PLID 60807 - Added ICD10 columns, renamed the enums so they don't match an enum in cptcodes.cpp
	enum EEmnDiagCodeListColumns {
		edclcID,		// (j.jones 2008-07-23 10:42) - PLID 30819
		edclcICD9DiagCodeID,		// (j.jones 2008-07-23 10:42) - PLID 30819
		edclcICD10DiagCodeID,
		edclcInfoButton,		// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
		edclcProblemIcon,	// (j.jones 2008-07-21 14:02) - PLID 30792
		edclcICD9CodeNumber,
		edclcICD9CodeDesc,
		edclcICD10CodeNumber,
		edclcICD10CodeDesc,
		edclcQuickListStar, // (c.haag 2014-03-17) - PLID 60929
		edclcOrderIndex,
		edclcEMNDiagCodePtr,	// (j.jones 2008-07-22 12:14) - PLID 30792
	};

	// (r.farnworth 2014-02-20 11:33) - PLID 60746 - Brought over from MoreInfo
	// (j.jones 2008-08-01 11:51) - PLID 30819 - added diag dropdown enums
	enum EDiagCodeComboColumns2 {

		dcccID = 0,
		dcccCode,
		dcccDescription,
	};

	// (b.savon 2014-03-14 08:22) - PLID 60824 - Handle the selection of a multimatch
	struct CEmrCodesDlg::DiagnosisCodeCommit{
		CArray<NexTech_Accessor::_DiagnosisCodeCommitPtr, NexTech_Accessor::_DiagnosisCodeCommitPtr> aryMultiMatch;
	};


//TES 2/11/2014 - PLID 60740 - New dialog for just the coding-related parts of CEmrCodesDlg

// (a.walling 2014-03-11 12:57) - PLID 61313 - MFC base class must be CNxDialog
// otherwise RUNTIME_CLASS(CEmrCodesDlg) is an immediate subclass of CDialog
// and therefore not IsKindOf CNxDialog or CNexTechDialog
IMPLEMENT_DYNAMIC(CEmrCodesDlg, CNxDialog)

CEmrCodesDlg::CEmrCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrCodesDlg::IDD, pParent),
	m_checkCPTCodeT(NetUtils::CPTCodeT),
	m_checkDiagCodes(NetUtils::DiagCodes), 
	m_pDiagCodeCommitMultiMatch(new DiagnosisCodeCommit()) // (b.savon 2014-03-14 08:22) - PLID 60824 - Handle the selection of a multimatch
{
	m_pEMN = NULL;
	m_bReadOnly = FALSE;
	// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
	m_bIsTemplate = FALSE;
	m_pPrepareBillListThread = NULL;
		// (j.gruber 2009-12-24 10:41) - PLID 15329 - warning check
	m_bHadBillWarning = FALSE;
	m_bkgColor = 0x8000000F;

	m_hInfoButtonIcon = NULL;
	m_hStarFilledIcon = NULL;
	m_hStarUnfilledIcon = NULL;

	// (b.savon 2014-03-14 08:22) - PLID 60824 - Handle the selection of a multimatch
	m_pDiagCodeCommitMultiMatch->aryMultiMatch.RemoveAll();
}

CEmrCodesDlg::~CEmrCodesDlg()
{
	// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
	if(m_pPrepareBillListThread) {
		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pPrepareBillListThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so post a quit message to it and let it delete itself
			// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
			m_pPrepareBillListThread->m_bAutoDelete = TRUE;
			PostThreadMessage(m_pPrepareBillListThread->m_nThreadID, WM_QUIT, 0, 0);
		} else {
			// The thread is finished, so just delete it
			delete m_pPrepareBillListThread;
		}
	}
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
struct PrepareBillListInfo
{
	HWND hwndNotifyIn;
	CString strCptModifierListOut;
	CString strServiceCodeListOut;
};

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
UINT PrepareBillListCode(LPVOID p)
{
	// (c.haag 2007-05-03 17:31) - PLID 25902 - Added exception handling
	try {
		//Threads need to have their own connection, let's create one, based on the global connection.
		// (a.walling 2010-07-23 17:11) - PLID 39835 - Use GetThreadRemoteData to get a new connection using default values within a thread
		_ConnectionPtr pConn = GetThreadRemoteData();

		PrepareBillListInfo *pPbli = (PrepareBillListInfo*)p;
		// (z.manning, 05/01/2007) - PLID 16623 - Don't show inactive modifiers.
		_RecordsetPtr rsCPTModifiers = CreateRecordset(pConn, 
"\
SELECT \
	Number AS ID, \
	(Number + '   ' + Note) AS Text, \
	Active \
FROM CPTModifierT \
UNION \
SELECT \
	'-1' AS ID, \
	'     (None)' AS Text, \
	1 AS Active \
ORDER BY Text ASC \
");

		// (z.manning, 05/01/2007) - PLID 16623 - Tell the datalist we're going to be including the visible flag.
		// (a.walling 2012-07-05 17:28) - PLID 50892 - New list format, uses nonprintable chars as delimiters
		pPbli->strCptModifierListOut = ";;-1;;";

		// (a.walling 2012-06-15 09:15) - PLID 50892 - Need to escape ; in data -- this is a datalist1
		while(!rsCPTModifiers->eof) {
			// (a.walling 2012-07-05 17:28) - PLID 50892 - New list format, uses nonprintable chars as delimiters

			pPbli->strCptModifierListOut.AppendFormat("%s\x02%s\x02%s\x02\x03"
				, AdoFldString(rsCPTModifiers, "ID")
				, AdoFldString(rsCPTModifiers, "Text")
				, AsString(rsCPTModifiers->Fields->GetItem("Active")->Value)
			);

			rsCPTModifiers->MoveNext();
		}
		rsCPTModifiers->Close();

		if(::IsWindow(pPbli->hwndNotifyIn)) {
			::PostMessage(pPbli->hwndNotifyIn, NXM_PREPARE_BILL_LIST_COMPLETED, (WPARAM)pPbli, 0);
		}
		else {
			//We need to clean this up ourselves.
			delete pPbli;
		}
	}
	NxCatchAllThread("Error in PrepareBillListCode");
	return 0;
}


void CEmrCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CREATE_QUOTE_CODE, m_btnCreateQuote);
	DDX_Control(pDX, IDC_BTN_CREATE_BILL_CODE, m_btnCreateBill);
	DDX_Control(pDX, IDC_BTN_ADD_CHARGE_CODE, m_btnAddCharge);
	DDX_Control(pDX, IDC_BTN_CHARGE_SPLIT_CODE, m_btnAssignInsResp);
	DDX_Control(pDX, IDC_HIDE_UNBILLABLE_CPT_CODES_CODE, m_checkHideUnbillableCPTCodes);	
	DDX_Control(pDX, IDC_SEND_BILL_TO_HL7_CODE, m_checkSendBillToHL7);
	DDX_Control(pDX, IDC_BKG2, m_bkg2);
	DDX_Control(pDX, IDC_BKG_CODE4, m_bkg4);
	DDX_Control(pDX, IDC_BKG5, m_bkg5);
	DDX_Control(pDX, IDC_BTN_MOVE_DIAG_UP, m_btnMoveDiagUp);
	DDX_Control(pDX, IDC_BTN_MOVE_DIAG_DOWN, m_btnMoveDiagDown);
	DDX_Control(pDX, IDC_BTN_EDIT_SERVICES, m_btnEditServices);
	DDX_Control(pDX, IDC_EXISTING_QUOTE_BTN, m_btnExistingQuote); 
	DDX_Control(pDX, IDC_BTN_CODES_QUICKLIST, m_btnQuickList);
	DDX_Control(pDX, IDC_ADD_NEXCODE_BTN, m_btnNexCode);
	DDX_Control(pDX, IDC_LINK_CODES_TO_ALL_CHARGES, m_btnLinkCodesToAllCharges);
	DDX_Control(pDX, IDC_BTN_SEARCHQUEUE, m_btnSearchQueue);// (j.camacho 7/16/2014) - plid 62636
}


BEGIN_MESSAGE_MAP(CEmrCodesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_USE_EM_CHECKLIST, &CEmrCodesDlg::OnBnClickedBtnUseEmChecklist)
	ON_BN_CLICKED(IDC_BTN_CREATE_BILL_CODE, &CEmrCodesDlg::OnBnClickedBtnCreateBillCode)
	ON_BN_CLICKED(IDC_BTN_CREATE_QUOTE_CODE, &CEmrCodesDlg::OnBnClickedBtnCreateQuoteCode)
	ON_BN_CLICKED(IDC_BTN_ADD_CHARGE_CODE, &CEmrCodesDlg::OnBnClickedBtnAddChargeCode)
	ON_BN_CLICKED(IDC_BTN_CHARGE_SPLIT_CODE, &CEmrCodesDlg::OnBnClickedBtnChargeSplitCode)
	ON_BN_CLICKED(IDC_HIDE_UNBILLABLE_CPT_CODES_CODE, &CEmrCodesDlg::OnBnClickedHideUnbillableCptCodesCode)
	ON_BN_CLICKED(IDC_SEND_BILL_TO_HL7_CODE, &CEmrCodesDlg::OnBnClickedSendBillToHl7Code)
	ON_MESSAGE(NXM_PREPARE_BILL_LIST_COMPLETED, OnPrepareBillListCompleted)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_COMMAND(ID_EDIT_INV_ITEMS, OnEditInventory)
	ON_COMMAND(ID_EDIT_CPT_CODES, OnEditCPT)
	ON_MESSAGE(NXM_EMN_CHARGE_ADDED, OnEmnChargeAdded)
	ON_MESSAGE(NXM_EMN_CHARGE_CHANGED, OnEmnChargeChanged)
	ON_MESSAGE(NXM_EMR_SET_SEARCH_QUEUE, OnSaveSearchQueueList)
	ON_COMMAND(IDC_BTN_EDIT_SERVICES, OnEditServices)
	ON_BN_CLICKED(IDC_BTN_MOVE_DIAG_UP, OnBtnMoveDiagUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_DIAG_DOWN, OnBtnMoveDiagDown)
	ON_MESSAGE(NXM_EMN_DIAG_ADDED, OnEmnDiagAdded)
	ON_BN_CLICKED(IDC_EXISTING_QUOTE_BTN, &CEmrCodesDlg::OnBnClickedExistingQuote)
	ON_COMMAND(IDC_BTN_CODES_QUICKLIST, OnBtnQuickList)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ADD_NEXCODE_BTN, &CEmrCodesDlg::OnBnClickedAddNexcodeBtn)
	ON_BN_CLICKED(IDC_LINK_CODES_TO_ALL_CHARGES, &CEmrCodesDlg::OnLinkCodesToAllCharges)
	ON_BN_CLICKED(IDC_BTN_SEARCHQUEUE, &CEmrCodesDlg::OnBtnClickedSearchQueue) //plid 62638
	ON_WM_ACTIVATE()
	ON_COMMAND(ID_REPLACE_DIAG_CODE, OnReplaceDiagCode)
END_MESSAGE_MAP()


// CEmrCodesDlg message handlers

//TES 2/12/2014 - PLID 60740 - Load this dialog from the given EMN
void CEmrCodesDlg::Initialize(CEMN *pEMN)
{
	m_pEMN = pEMN;

	m_nPatientID = m_pEMN->GetParentEMR()->GetPatientID();
		// (r.farnworth 2014-02-14 15:15) - PLID 60745 - Carried over from MoreInfo for Control Enabling
	m_bIsTemplate = m_pEMN->IsTemplate();

	// (r.farnworth 2014-02-14 15:15) - PLID 60745 - Try and set the visit type using the EMN's predefinied data
	TrySetVisitType(m_pEMN->GetVisitTypeID(), m_pEMN->GetVisitTypeName());

	// (r.farnworth 2014-02-18 10:55) - PLID 60746 - Brought over from MoreInfo
	RefreshDiagCodeList();

// (s.dhole 2014-02-14 14:48) - PLID 60742 Move Code from <More Info> dialog 
	//TES 7/10/2009 - PLID 25154 - Set whether we're planning to export to HL7
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	//TES 6/22/2011 - PLID 44261 - New function to access HL7 Settings
	CArray<long,long> arHL7GroupIDs;
	GetHL7SettingsGroupsBySetting("ExportEmnBills", TRUE, arHL7GroupIDs);
	if(!m_bIsTemplate && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent) &&
		arHL7GroupIDs.GetSize() > 0) {
		m_checkSendBillToHL7.SetCheck(m_pEMN->GetSendBillToHL7());
	}

	//(j.camacho 2014-07-30) - plid 62641
	//SetSearchQueueList();

	// (j.jones 2012-02-17 10:33) - PLID 47886 - Added ability to hide unbillable service codes.
	// Remembers per user.
	m_checkHideUnbillableCPTCodes.SetCheck(GetRemotePropertyInt("EMN_HideUnbillableCPTCodes", 0, 0, GetCurrentUserName(), true) == 1);

	BOOL bLocked = (m_pEMN->GetStatus() == 2);
	BOOL bCanWriteToEMR = CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
	BOOL bIsReadOnly = !(m_pEMN->IsWritable());

	SetReadOnly((bLocked || !bCanWriteToEMR || bIsReadOnly) ? TRUE : FALSE);

	// (s.tullis 2015-07-01 10:36) - PLID 65539 - Should not be able to expand the column in templates
	if (m_bIsTemplate)
	{
		m_BillList->GetColumn(clcCategory)->PutColumnStyle(csVisible | csFixedWidth);
	}

	m_bInitialized = TRUE;
}

void CEmrCodesDlg::SetReadOnly(BOOL bReadOnly)
{
	m_bReadOnly = bReadOnly;
	EnableControls();
}

void CEmrCodesDlg::EnableControls()
{
	//TES 3/10/2014 - PLID 61313 - I moved a bunch of code that checks m_bIsTemplate here from OnInitDialog()

	//DRT 1/16/2007 - PLID 24177 - If we're in the template editor, never provide accessibility to
	//	the WhichCodes field.  Make it 0 width and not resizable.
	if(m_bIsTemplate) {
		IColumnSettingsPtr pCol = m_BillList->GetColumn(clcWhichcodes);
		pCol->PutStoredWidth(0);
		long nStyle = pCol->GetColumnStyle();
		if(nStyle & csVisible) {
			nStyle = nStyle & ~csVisible;
			pCol->PutColumnStyle(nStyle);
		}
	}

	//set the background color
	// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
	m_bkgColor = EmrColors::Topic::Background(!!m_bIsTemplate);

	if(m_brush.m_hObject) {
		m_brush.DeleteObject();
	}
	m_brush.CreateSolidBrush(m_bkgColor);
	m_bkg2.SetColor(m_bkgColor);
	m_bkg4.SetColor(m_bkgColor);
	m_bkg5.SetColor(m_bkgColor);

	m_pVisitTypeCombo->Enabled = !m_bReadOnly;

	// (j.jones 2007-08-27 09:51) - PLID 27056 - disable the E/M Checklist button
	// when locked or on a template
	// (j.jones 2013-04-22 16:32) - PLID 56372 - we now permit opening the checklist for a read-only EMN,
	// so that they can review decisions that were already made
	// (r.farnworth 2014-02-14 15:15) - PLID 60745 - Recreating Josh's functionality in the new dialog.
	// Will probably need to create an EnableControls function for things like this.
	GetDlgItem(IDC_BTN_USE_EM_CHECKLIST)->EnableWindow(!m_bIsTemplate);
	// (r.farnworth 2014-02-20 14:51) - PLID 60746 - Enable functions for Diagnosis Codes section
	//TES 2/26/2014 - PLID 60806 - Changed dropdown to search box
	GetDlgItem(IDC_EMN_DIAG_SEARCH)->EnableWindow(!m_bReadOnly);
	m_DiagCodeList->PutReadOnly(m_bReadOnly);
	// (c.haag 2014-03-07) - PLID 60930 - Update the QuickList button
	m_btnQuickList.EnableWindow(!m_bReadOnly);
	// (j.camacho 7/17/2014) - plid 62636 - Update Search Queue button
	//(j.camacho 7/31/2014) - plid 62637 - Keywords should exist otherwise disable the button
	m_btnSearchQueue.EnableWindow(!m_bReadOnly && !vSearchQueueKeywords.empty()); 
	//(j.camacho 7/17/2014) - plid 62639 - button should not be visible in emr template editor.
	if (m_bIsTemplate)
	{
		m_btnSearchQueue.ShowWindow(FALSE);
	}
	// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
	//Controls that aren't valid on templates, but ARE valid on locked EMNs.
	if(!m_bPreparingBillList) {//If the bill list is still loading, leave these disabled.
		GetDlgItem(IDC_BTN_CREATE_QUOTE_CODE)->EnableWindow(!m_bIsTemplate);
		GetDlgItem(IDC_BTN_CREATE_BILL_CODE)->EnableWindow(!m_bIsTemplate);
		// (s.dhole 2014-03-21 12:53) - PLID 60742
		GetDlgItem(IDC_BTN_ADD_CHARGE_CODE)->EnableWindow(!m_bReadOnly);
		m_BillList->PutReadOnly(m_bReadOnly);
	} 

	// (b.savon 2014-03-31 08:21) - PLID 60991 - Add a button to access NexCode from the <Codes> topic in the EMN and add the code to the datalist
	if( !m_bReadOnly && DiagSearchUtils::GetPreferenceSearchStyle() != eManagedICD9_Search ){
		m_btnNexCode.EnableWindow(TRUE);
		m_btnNexCode.ShowWindow(SW_SHOW);
	}else if(DiagSearchUtils::GetPreferenceSearchStyle() == eManagedICD9_Search){
		m_btnNexCode.EnableWindow(FALSE);
		m_btnNexCode.ShowWindow(SW_HIDE);
	}else{
		m_btnNexCode.EnableWindow(FALSE);
		m_btnNexCode.ShowWindow(SW_SHOW);
	}
	
	// (c.haag 2014-07-16) - PLID 54905 - Toggle the usability of the link codes to charges button
	m_btnLinkCodesToAllCharges.EnableWindow(!m_bReadOnly);

	UpdateDiagCodeArrows();

	CArray<long,long> arHL7GroupIDs;
	GetHL7SettingsGroupsBySetting("ExportEmnBills", TRUE, arHL7GroupIDs);
	if(!m_bIsTemplate && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent) &&
		arHL7GroupIDs.GetSize() > 0) {
			GetDlgItem(IDC_SEND_BILL_TO_HL7_CODE)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_SEND_BILL_TO_HL7_CODE)->EnableWindow(FALSE);
	}
}

BOOL CEmrCodesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {

		// (s.dhole 2014-02-17 09:01) - PLID PLID 60742 Copy Code from <More Info> dialog 
		// (z.manning 2009-02-19 10:58) - PLID 33154 - Cached all the properties in CEmrCodesDlg::OnInitDialog
		g_propManager.CachePropertiesInBulk("EmrCodesDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ("
			"	'EMNChargesSortByBillable', " // (d.singleton 2011-11-01 11:26) - PLID 43548
			"	'EMN_HideUnbillableCPTCodes', "	// (j.jones 2012-02-17 10:37) - PLID 47886
			"	'ChargeAllowQtyIncrement', "	
			"	'EMNChargesAllowQtyIncrement', "	
			"	'RequireEMRChargeResps', "	
			"	'BillEMNTo', "	
			"	'DisplayChargeSplitDlgAlways',"	
			"	'EMNChargesAllowQtyIncrement', "
			"   'EMR_SkipLinkingDiagsToNonBillableCPTs' "
			")"
			, _Q(GetCurrentUserName()));

		// (r.farnworth 2014-02-17 15:43) - PLID 60745 - MOVE - Move the E/M coding section of <More Info> to the new <Codes> topic -Move
		m_pVisitTypeCombo = BindNxDataList2Ctrl(IDC_VISIT_TYPE, true);
		// (r.farnworth 2014-02-17 15:43) - PLID 60746 - MOVE - Move the diag codes section of <More Info> to the new <Codes> topic
		//TES 2/26/2014 - PLID 60806 - Changed dropdown to search box
		m_DiagSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_EMN_DIAG_SEARCH, GetRemoteData());
		m_DiagCodeList = BindNxDataList2Ctrl(IDC_DIAGS,false);
		// (s.dhole 2014-02-14 14:48) - PLID 60742 Move Code from <More Info> dialog 
		m_BillList  = BindNxDataList2Ctrl(IDC_BILL_CODE, false);
		m_bkg2.SetColor(m_bkgColor);
		m_bkg4.SetColor(m_bkgColor);
		m_bkg5.SetColor(m_bkgColor);
		m_btnAddCharge.AutoSet(NXB_NEW);
		// (b.savon 2014-02-26 10:42) - PLID 60805 - UPDATE - Add new button, "Existing Quote" in the procedure codes section of the <Codes> topic
		m_btnExistingQuote.AutoSet(NXB_NEW);
		m_btnCreateQuote.AutoSet(NXB_NEW);
		m_btnCreateBill.AutoSet(NXB_NEW);
		m_btnAssignInsResp.AutoSet(NXB_MODIFY);	// (j.dinatale 2012-01-09 11:39) - PLID 39451
		// (j.dinatale 2012-01-11 15:15) - PLID 39451 - disable assign ins resp button if they are in a template editor
		if(m_bIsTemplate){
			m_btnAssignInsResp.EnableWindow(FALSE);
		}

		// (r.farnworth 2014-02-17 15:03) - PLID 60746 - MOVE - Move the diag codes section of <More Info> to the new <Codes> topic
		m_btnMoveDiagUp.SetIcon(IDI_UARROW);
		m_btnMoveDiagDown.SetIcon(IDI_DARROW);

		//(j.camacho 7/16/2014) - PLID 62636
		m_btnSearchQueue.SetIcon(IDI_RARROW_SMALL);
		GetDlgItem(IDC_BTN_SEARCHQUEUE)->EnableWindow(!m_bReadOnly && !vSearchQueueKeywords.empty());

		// (r.farnworth 2014-02-17 17:17) - PLID 60746 - InfoButton Controls brought over from MoreInfo
		m_hInfoButtonIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INFO_ICON), IMAGE_ICON, 16,16, 0);
		// (c.haag 2014-03-17) - PLID 60929 - QuickList star icons
		m_hStarFilledIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_STAR_FILLED), IMAGE_ICON, 16,16, 0);
		m_hStarUnfilledIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_STAR_UNFILLED), IMAGE_ICON, 16,16, 0);

		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IColumnSettingsPtr pInfoButtonColumn = m_DiagCodeList->GetColumn(edclcInfoButton);
		if(bShowPatientEducationButton) {
			pInfoButtonColumn->ColumnStyle |= NXDATALIST2Lib::csVisible;
		} else {
			pInfoButtonColumn->ColumnStyle &= ~NXDATALIST2Lib::csVisible;
		}
		// (r.gonet 03/07/2014) - PLID 60756 - (This was missing a comment.) Make the ICD-9 code number field a hyperlink if we have
		// patient education links turned on.
		NXDATALIST2Lib::IColumnSettingsPtr pICD9CodeNumberColumn = m_DiagCodeList->GetColumn(edclcICD9CodeNumber);
		if(bShowPatientEducationLink) {
			pICD9CodeNumberColumn->FieldType = NXDATALIST2Lib::cftTextSingleLineLink;
		} else {
			pICD9CodeNumberColumn->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		}
		// (r.gonet 03/07/2014) - PLID 60756 - Make the ICD-10 code number field a hyperlink if we have
		// patient education links turned on.
		NXDATALIST2Lib::IColumnSettingsPtr pICD10CodeNumberColumn = m_DiagCodeList->GetColumn(edclcICD10CodeNumber);
		if(bShowPatientEducationLink) {
			pICD10CodeNumberColumn->FieldType = NXDATALIST2Lib::cftTextSingleLineLink;
		} else {
			pICD10CodeNumberColumn->FieldType = NXDATALIST2Lib::cftTextSingleLine;
		}

		m_BillList->WhereClause = _bstr_t("EMRChargesT.Deleted = 0 AND EMRChargesT.EMRID = -1");
		m_bPreparingBillList = TRUE;
		GetDlgItem(IDC_BTN_ADD_CHARGE_CODE)->EnableWindow(FALSE);
		m_BillList->PutReadOnly(TRUE);
		GetDlgItem(IDC_BTN_CREATE_QUOTE_CODE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CREATE_BILL_CODE)->EnableWindow(FALSE);
		PrepareBillListInfo *pPbli = new PrepareBillListInfo;
		pPbli->hwndNotifyIn = GetSafeHwnd();
		m_pPrepareBillListThread = AfxBeginThread(PrepareBillListCode, pPbli, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
		m_pPrepareBillListThread->m_bAutoDelete = false;
		m_pPrepareBillListThread->ResumeThread();

		//decide whether to show the subcode and modifier columns
		//DRT 1/16/2007 - PLID 24177 - Added Whichcodes field to this setup
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillCPTSubCode, BillMod1, BillMod2, BillMod3, BillMod4, BillDiagCs FROM ConfigBillColumnsT WHERE LocationID = {INT}",GetCurrentLocationID());
		if(!rs->eof) {
			BOOL bBillCPTSubCode = AdoFldBool(rs, "BillCPTSubCode",TRUE);
			BOOL bBillMod1 = AdoFldBool(rs, "BillMod1",TRUE);
			BOOL bBillMod2 = AdoFldBool(rs, "BillMod2",TRUE);
			BOOL bBillMod3 = AdoFldBool(rs, "BillMod3",FALSE);
			BOOL bBillMod4 = AdoFldBool(rs, "BillMod4",FALSE);
			BOOL bWhichCodes = AdoFldBool(rs, "BillDiagCs",FALSE);
			

			if(!bBillCPTSubCode) {
				m_BillList->GetColumn(clcSubcode)->PutStoredWidth(0);
			}
			if(!bBillMod1) {
				m_BillList->GetColumn(clcMod1)->PutStoredWidth(0);
			}
			if(!bBillMod2) {
				m_BillList->GetColumn(clcMod2)->PutStoredWidth(0);
			}
			if(!bBillMod3) {
				m_BillList->GetColumn(clcMod3)->PutStoredWidth(0);
			}
			if(!bBillMod4) {
				m_BillList->GetColumn(clcMod4)->PutStoredWidth(0);
			}
			if(!bWhichCodes) {
				m_BillList->GetColumn(clcWhichcodes)->PutStoredWidth(0);
			}

		}
		rs->Close();
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Charge Category defaults to not show
		m_BillList->GetColumn(clcCategory)->PutStoredWidth(0);

		//TES 3/10/2014 - PLID 61313 - Various decisions were being made here based on m_bIsTemplate, but that
		// variable hasn't actually been set yet. I moved that code to EnableControls()

		// (a.walling 2007-07-03 13:06) - PLID 23714 - Get messages from the mainframe
		GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());

		// (r.farnworth 2014-02-14 15:15) - PLID 60745 - Add the default selection to the datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pVisitTypeCombo->GetNewRow();
		pRow->PutValue(vtcID, (long)-1);
		pRow->PutValue(vtcName, _bstr_t("<No Type Selected>"));
		m_pVisitTypeCombo->AddRowSorted(pRow, NULL);

		EnableControls();

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
void CEmrCodesDlg::OnDestroy() 
{
	try {
		CNxDialog::OnDestroy();

		if(m_hInfoButtonIcon) {
			DestroyIcon(m_hInfoButtonIcon);
		}
		// (c.haag 2014-03-17) - PLID 60929 - Icon cleanup
		if (m_hStarFilledIcon) {
			DestroyIcon(m_hStarFilledIcon);
			m_hStarFilledIcon = NULL;
		}
		if (m_hStarUnfilledIcon) {
			DestroyIcon(m_hStarUnfilledIcon);
			m_hStarUnfilledIcon = NULL;
		}

		// (a.walling 2007-07-03 13:07) - PLID 23714
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-07-03 13:09) - PLID 23714
LRESULT CEmrCodesDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// If we are not visible, don't bother updating. It will be updated in OnShowWindow.
		// Otherwise try to update the lists.
		if (IsWindowVisible()) {
			UpdateChangedInfo();

		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}


// (r.farnworth 2014-02-14 15:15) - PLID 60745
void CEmrCodesDlg::OnBnClickedBtnUseEmChecklist()
{
	try {

		CEmrTreeWnd *pTree = GetEmrTreeWnd();
		if(pTree) {
			pTree->OpenEMChecklist(m_pEMN);
		}

	}NxCatchAll(__FUNCTION__);
}


// (r.farnworth 2014-02-14 15:15) - PLID 60745
void CEmrCodesDlg::OnSelChosenVisitType(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {

			long nVisitTypeID = VarLong(pRow->GetValue(vtcID),-1);
			CString strVisitTypeName = VarString(pRow->GetValue(vtcName),"");
			if(nVisitTypeID == -1) {
				//if the <None Selected> row, might as well clear out the selection
				m_pVisitTypeCombo->CurSel = NULL;
				strVisitTypeName = "";
			}

			//need to set the VisitTypeID in the m_pEMN pointer
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
			m_pEMN->SetVisitType(nVisitTypeID, strVisitTypeName);			
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-02-14 15:15) - PLID 60745
void CEmrCodesDlg::TrySetVisitType(long nVisitTypeID, CString strVisitTypeID)
{
	if(nVisitTypeID != -1) 
	{
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		long nSel = m_pVisitTypeCombo->TrySetSelByColumn_Deprecated(vtcID, nVisitTypeID);
		if(nSel == sriNoRow) {
			//it could be inactive
			m_pVisitTypeCombo->PutComboBoxText(_bstr_t(strVisitTypeID));
		}
	}
	else 
	{
		m_pVisitTypeCombo->CurSel = NULL;
		m_pVisitTypeCombo->PutComboBoxText(_bstr_t(""));
	}
}

// (r.farnworth 2014-02-14 15:15) - PLID 60745
void CEmrCodesDlg::OnTrySetSelFinishedVisitType(long nRowEnum, long nFlags) 
{
	if(nFlags == dlTrySetSelFinishedFailure) {
		m_pVisitTypeCombo->PutComboBoxText(_bstr_t(m_pEMN->GetVisitTypeName()));
	}
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
// (a.walling 2013-05-15 15:38) - PLID 56697 - Helper object to map arbitrary values to sequential generated command IDs for tracking popup menus
// (d.singleton 2014-02-27 16:49) - PLID 61072 - moved MenuCommandMap to its own header so everyone can use it

void CEmrCodesDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		
		CNxDialog::OnShowWindow(bShow, nStatus);

		if (bShow) {
			UpdateChangedInfo();
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-07-09 13:04) - PLID 23714 - Update any changed information based on the table checkers
void CEmrCodesDlg::UpdateChangedInfo()
{
	try
	{
		// (a.walling 2007-10-04 13:43) - PLID 23714 - Update the charge list
		if (m_checkCPTCodeT.Changed()) {
			// the CPT codes have changed, so refresh the list
			RefreshChargeList();
		}

		// (a.walling 2012-03-22 16:50) - PLID 49141 - Anything that changed should already notify via one of the functions above
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
void CEmrCodesDlg::OnBnClickedBtnCreateBillCode()
{
	// (j.jones 2005-11-23 16:19) - this shouldn't be called on a template
	if (m_bIsTemplate) {
		return;
	}

	try {
		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(!pTreeWnd) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(!CheckCurrentUserPermissions(bioBill,sptCreate))
			return;

		// (j.dinatale 2012-01-17 08:53) - PLID 47620 - check if we need to bill partials. If we do, skip the context menu
		long nEMNID =  m_pEMN->GetID();
		BOOL bBillPartials = FALSE;
		
		// (j.dinatale 2012-01-31 11:52) - PLID 47620 - if the row count is 0, we have no charges... so we should bill the EMN like we normally do when it has no charges
		if(m_BillList->GetRowCount() > 0){
			bBillPartials = GetRemotePropertyInt("RequireEMRChargeResps", 0, 0, "<None>", true);

			if(!bBillPartials){

				IRowSettingsPtr pRow=m_BillList->GetFirstRow();
				while(pRow)
				{
					EMNCharge *pCharge = (EMNCharge*)VarLong(pRow ->GetValue(clcEMNChargePtr));

					if(pCharge->bBillable && pCharge->nInsuredPartyID != -2){
						bBillPartials = TRUE;
					}
					pRow=pRow->GetNextRow();
				}
			}
		}

		// (j.jones 2006-02-03 11:14) - PLID 18735 - determined which responsibility to generate a bill for
		long nInsuredPartyID = -1;

		if(!bBillPartials){
		long nBillEMNTo = GetRemotePropertyInt("BillEMNTo", 0, 0, GetCurrentUserName(), true);
		//1 - patient responsibility
		//2 - primary responsibility if it exists
		//0 - pop-out a menu of options of available responsibilities

		// (j.jones 2011-12-16 15:39) - PLID 46289 - we now pop out the menu if they
		// select primary, but have more than one primary (medical and vision)
		BOOL bPopOutMenu = FALSE;

		if(nBillEMNTo == 1) {
			//patient responsibility
			nInsuredPartyID = -1;
		}
		else if(nBillEMNTo == 2) {
			//primary responsibility if it exists
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (j.jones 2011-12-16 16:43) - PLID 46289 - find all Primary resps, there could be Primary Medical or Vision, or both
			_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID FROM InsuredPartyT "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} AND RespTypeT.CategoryPlacement = 1", m_nPatientID);
			if(!rs->eof) {
				// (j.jones 2011-12-16 16:41) - PLID 46289 - if multiple records,
				// we will pop out the menu
				if(rs->GetRecordCount() > 1) {
					nInsuredPartyID = -1;
					bPopOutMenu = TRUE;
				}
				else {
					nInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
				}
			}
			else {
				nInsuredPartyID = -1;
			}
			rs->Close();
		}
		else {
			bPopOutMenu = TRUE;
		}

		if(bPopOutMenu) {
			//pop-out a menu of options of available responsibilities

			//if there is no insurance, use patient

			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (j.jones 2011-12-16 16:43) - PLID 46289 - find all active resps
			_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
				"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority", m_nPatientID);
			if(!rs->eof) {

				// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
				CNxMenu mnu;
				mnu.m_hMenu = CreatePopupMenu();
				//add a line for patient

				// (a.walling 2013-05-15 15:38) - PLID 56697
				// since CEmrCodesDlg lives within a frame now, the frame handles WM_INITMENUPOPUP and hence will attempt to update
				// the menu items via the command ID. EMR's commands are all >= 40000, and MFC and reserved IDs all higher than that
				// so we need to restrict the range. The most straightforward way to do this is with a map: enter Nx::MenuCommands
				Nx::MenuCommandMap<long> menuCmds;

				long nIndex = 0;
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, menuCmds(-1), "For Patient Responsibility");

				//add a line for each responsibility
				while(!rs->eof) {

					long nInsPartyID = AdoFldLong(rs, "PersonID",-1);
					CString strInsCoName = AdoFldString(rs, "Name","");
					CString strRespTypeName = AdoFldString(rs, "TypeName","");
					CString strLabel;
					strLabel.Format("For %s (%s)", strInsCoName, strRespTypeName);

					//(e.lally 2012-03-27) PLID 49246 - Apply the menu ID offset
					mnu.InsertMenu(nIndex++, MF_BYPOSITION, menuCmds(nInsPartyID), strLabel);
					rs->MoveNext();
				}

				CRect rc;
				CWnd *pWnd = GetDlgItem(IDC_BTN_CREATE_BILL);
				if (pWnd) {
					pWnd->GetWindowRect(&rc);
					nInsuredPartyID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , rc.right, rc.top, this, NULL);
					mnu.DestroyMenu();
				} else {
					CPoint pt;
					GetCursorPos(&pt);
					nInsuredPartyID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , pt.x, pt.y, this, NULL);
					mnu.DestroyMenu();
				}

				//(e.lally 2012-03-27) PLID 49246 - Leave as 0 for no selection
				if(nInsuredPartyID == 0) {
					return;
				}

				// (a.walling 2013-05-15 15:38) - PLID 56697 - translate the command to the real value
				boost::optional<long> choice = menuCmds.Translate(nInsuredPartyID);
				if (!choice) {
					ASSERT(FALSE);
					return;
				} else {
					nInsuredPartyID = *choice;
				}
			}
			else {
				nInsuredPartyID = -1;
			}
			rs->Close();
		}
		}else{
			// (j.dinatale 2012-01-17 09:00) - PLID 47620 - if we need to bill partials... then dont do anything with the context menu
		}

		CString strWarning = "Before creating a bill, the EMR will automatically be saved and closed.\n"
			"Are you sure you wish to create a bill at this time?";

		if(m_BillList->GetRowCount() == 0) {
			strWarning = "This EMN does not have any charges to be billed. Are you sure you wish to create the bill?\n"
				"If yes, then the EMR will be automatically saved and closed.";
		}

		if(IDNO == MessageBox(strWarning,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		//checks for any active global period, and warns accordingly
		// (a.walling 2008-07-07 18:03) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		if(!CheckWarnGlobalPeriod(m_nPatientID))
			return;

		// (j.dinatale 2012-01-18 17:43) - PLID 47620 - handle partial billing
		if(GetMainFrame()){
		if (!GetMainFrame()->IsBillingModuleOpen(true)) {
				if(!bBillPartials){
			pTreeWnd->GetParent()->SendMessage(NXM_EMR_SAVE_CREATE_BILL, (WPARAM)m_pEMN, (LPARAM)nInsuredPartyID);			
				}else{
					{
						// (a.walling 2012-04-25 15:17) - PLID 46071 - Need to use the tree wnd directly rather than GetParent now
						EmrSaveStatus essResult = (EmrSaveStatus)GetEmrTreeWnd()->SendMessage(NXM_EMR_SAVE_ALL, TRUE, NULL);
						if(FAILED(essResult)){
							//Do we need any special messages that it failed? We should return here to stop the report from running.
							ASSERT(FALSE); 
							return;
						}
					}

					// (j.dinatale 2012-01-30 11:14) - PLID 47620 - need to set our EMNID to the new EMNID after a save
					nEMNID =  m_pEMN->GetID();

					// (j.dinatale 2012-01-23 14:25) - PLID 47620 - if the EMR has been fully billed already, then dont let them bill it again
					if(GetMainFrame()->m_EMNBillController.HasBeenFullyBilled(nEMNID)){
						MessageBox("This EMN has been fully billed already. It cannot be billed again.");
						return;
					}

					bool bHasUnassigned = GetMainFrame()->m_EMNBillController.HasUnassignedCharges(nEMNID);
					bool bIsLocked = (m_pEMN->GetStatus() == 2);

					if(bIsLocked && bHasUnassigned){
						MessageBox("This EMN is locked and has charges that are not assigned to a responsibility. This EMN must be billed from Patients to be Billed.");
						return;
					}

					// (j.dinatale 2012-01-20 09:44) - PLID 47620 - need to ensure that our insurance is assigned correctly for charges that have yet to be billed
					if(GetMainFrame()->m_EMNBillController.HasInactiveInsuranceAssigned(nEMNID)){
						MessageBox("At least one unbilled EMN charge is assigned to a responsibility that is marked inactive. Please reassign those charges and then bill this EMN again.");
						bHasUnassigned = true; // if inactive insurance is assigned, its like they are unassigned
					}

					// if we need to, check and see if we have to display the charge split dialog and if we have any unassigned charges
					bool bAlwaysShowChrgSplitDlg = (!!GetRemotePropertyInt("DisplayChargeSplitDlgAlways", 0, 0, "<None>", true));
					if(bAlwaysShowChrgSplitDlg || bHasUnassigned){
						// if the user hit cancel, or we still have charges that are not assigned, fail
						long nResult = ShowChargeRespDlg();
						if(nResult == IDCANCEL){
							return;
						}

						{
							// (a.walling 2012-04-25 15:17) - PLID 46071 - Need to use the tree wnd directly rather than GetParent now
							EmrSaveStatus essResult = (EmrSaveStatus)GetEmrTreeWnd()->SendMessage(NXM_EMR_SAVE_ALL, TRUE, NULL);
							if(FAILED(essResult)){
								//Do we need any special messages that it failed? We should return here to stop the report from running.
								ASSERT(FALSE); 
								return;
							}
						}

						// (j.dinatale 2012-01-30 11:14) - PLID 47620 - need to set our EMNID to the new EMNID after a save
						nEMNID =  m_pEMN->GetID();

						// (j.dinatale 2012-01-20 09:44) - PLID 47620 - need to ensure that our insurance is assigned correctly for charges that have yet to be billed
						if(GetMainFrame()->m_EMNBillController.HasInactiveInsuranceAssigned(nEMNID)){
							MessageBox("At least one unbilled EMN charge is assigned to a responsibility that is marked inactive. Please reassign those charges and then bill this EMN again.");
							return;
						}

						if(GetMainFrame()->m_EMNBillController.HasUnassignedCharges(nEMNID)){
							MessageBox("This EMN could not be billed because all charges must be assigned to a responsibility before this EMN can be billed.");	
							return;
						}
					}

					pTreeWnd->GetParent()->SendMessage(NXM_EMR_SAVE_CREATE_PARTIAL_BILLS, (WPARAM)m_pEMN, NULL);

				}
			}
		}
	
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::OnBnClickedBtnCreateQuoteCode()
{
	// (j.jones 2005-11-23 16:19) - this shouldn't be called on a template
	if (m_bIsTemplate) {
		return;
	}

	try {

		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(!pTreeWnd) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}

		if(!CheckCurrentUserPermissions(bioPatientQuotes,sptCreate))
			return;

		CString strWarning = "Before creating a quote, the EMR will automatically be saved and closed.\n"
			"Are you sure you wish to create a quote at this time?";

		if(m_BillList->GetRowCount() == 0) {
			strWarning = "This EMN does not have any charges to be quoted. Are you sure you wish to create the quote?\n"
				"If yes, then the EMR will be automatically saved and closed.";
		}

		if(IDNO == MessageBox(strWarning,"Practice",MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		pTreeWnd->GetParent()->SendMessage(NXM_EMR_SAVE_CREATE_QUOTE, (WPARAM)m_pEMN);
	
		}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::OnBnClickedBtnAddChargeCode()
{
	
	try {

		// (j.jones 2007-08-13 09:46) - PLID 27050 - warn if they try to add default charges to the template
		if(m_bIsTemplate) {
			if(IDNO == MessageBox("Adding a charge to a template will default to using the charge on every patient EMN generated from this template.\n"
				"This is not recommended. Are you sure you wish to add a default charge?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		// (j.gruber 2009-12-24 10:05) - PLID 15329 - check to see if this emn has already been billed
		if (!WarnChargeAlreadyBilled()) {
			return;
		}

		CEMRSelectServiceDlg dlg(this);
		// (j.dinatale 2012-01-11 12:25) - PLID 47464 - show the resp list on the Select Service dlg is this is not a template
		// (j.jones 2008-06-03 10:28) - PLID 30062 - send a patient ID if not a template
		if(!m_bIsTemplate && m_nPatientID != -1) {
			dlg.m_nPatientID = m_nPatientID;
			dlg.m_bShowRespList = TRUE;
			// (j.jones 2012-08-22 09:23) - PLID 50486 - set the default insured party ID
			dlg.m_nAssignedInsuredPartyID = m_pEMN->GetParentEMR()->GetDefaultChargeInsuredPartyID();
		}
		if(dlg.DoModal() == IDOK) {

			//Update the interface
			if(dlg.m_nQuoteID != -1) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				AddQuoteToBillList(dlg.m_nQuoteID);
			}
			else if(dlg.m_ServiceID != -1) {
				// (j.dinatale 2012-01-05 10:12) - PLID 47464 - added Insured Party ID
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				// (s.tullis 2015-04-09 11:42) - PLID 64978 - Added Category and Category Count for showing/hiding /disabling category
				AddChargeToBillList(dlg.m_ServiceID, dlg.m_strCode, dlg.m_strSubCode, dlg.m_strDescription, dlg.m_nAssignedInsuredPartyID, dlg.m_bBillable, dlg.m_cyPrice,dlg.m_nCategory,dlg.m_nCategoryCount);
			}
			else {
				//nothing was selected, which should be impossible,
				//so assert and silently return
				ASSERT(FALSE);
				return;
			}
		}	
	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog 
void CEmrCodesDlg::OnBnClickedBtnChargeSplitCode()
{
	try{
		// (j.dinatale 2012-01-19 09:11) - PLID 47620 - moved the show charge split dialog logic into a function
		ShowChargeRespDlg();
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-13 17:43) - PLID 60742
void CEmrCodesDlg::OnBnClickedHideUnbillableCptCodesCode()
{
	try {

		//save this setting per user
		SetRemotePropertyInt("EMN_HideUnbillableCPTCodes", m_checkHideUnbillableCPTCodes.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		RefreshChargeList();

		//update the preview pane
		CEmrTreeWnd* pInterfaceWnd = m_pEMN->GetInterface();
		if(pInterfaceWnd) {
			pInterfaceWnd->UpdatePreviewMoreInfo(m_pEMN);
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-13 17:43) - PLID 60742
void CEmrCodesDlg::OnBnClickedSendBillToHl7Code()
{
		try {
		if(m_pEMN) {
			//TES 7/10/2009 - PLID 25154 - Tell the EMN.
			m_pEMN->SetSendBillToHL7(IsDlgButtonChecked(IDC_SEND_BILL_TO_HL7_CODE)?true:false);
		}
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CEmrCodesDlg, CNxDialog)
	ON_EVENT(CEmrCodesDlg, IDC_BILL_CODE, 10, CEmrCodesDlg::EditingFinishedBillCode, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrCodesDlg, IDC_BILL_CODE, 9, CEmrCodesDlg::EditingFinishingBillCode, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEmrCodesDlg, IDC_BILL_CODE, 8, CEmrCodesDlg::EditingStartingBillCode, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CEmrCodesDlg, IDC_BILL_CODE, 19, CEmrCodesDlg::LeftClickBillCode, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCodesDlg, IDC_BILL_CODE, 6, CEmrCodesDlg::RButtonDownBillCode, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCodesDlg, IDC_VISIT_TYPE, 16 /* SelChosen */, OnSelChosenVisitType, VTS_DISPATCH)
	ON_EVENT(CEmrCodesDlg, IDC_VISIT_TYPE, 20 /* TrySetSelFinished */, OnTrySetSelFinishedVisitType, VTS_I4 VTS_I4)
	ON_EVENT(CEmrCodesDlg, IDC_EMN_DIAG_SEARCH, 16, CEmrCodesDlg::OnSelChosenEmnDiagSearch, VTS_DISPATCH)
	ON_EVENT(CEmrCodesDlg, IDC_DIAGS, 14, CEmrCodesDlg::DragEndDiags, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CEmrCodesDlg, IDC_DIAGS, 2, CEmrCodesDlg::SelChangedDiags, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CEmrCodesDlg, IDC_DIAGS, 19, CEmrCodesDlg::LeftClickDiags, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCodesDlg, IDC_DIAGS, 6, CEmrCodesDlg::RButtonDownDiags, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCodesDlg, IDC_DIAGS, 10, CEmrCodesDlg::EditingFinishedDiags, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


// (s.dhole 2014-02-13 17:43) - PLID 60742
void CEmrCodesDlg::EditingFinishedBillCode(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow==NULL){
			return;
		}
		if(!bCommit) return;

		bool bEmnModified = false;
		switch(nCol) {
		//TES 8/2/2006 - PLID 21627 - This column is no longer editable.
		/*case clcServiceID:
			{
				//They have selected a new CPT Code.
				if(_variant_t(varOldValue) == _variant_t(varNewValue)) return;
				
				//Fill in the rest of the columns with their default values.
				_RecordsetPtr rsService = CreateRecordset("SELECT Code, SubCode, Name, Price FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID WHERE ServiceT.ID = %li", VarLong(varNewValue));
				if(rsService->eof) {
					ASSERT(FALSE);
					return;
				}
				IRowSettingsPtr pRow = m_BillList->GetRow(nRow);
				pRow->PutValue(clcSubcode, rsService->Fields->Item["SubCode"]->Value);
				pRow->PutValue(clcDescription, rsService->Fields->Item["Name"]->Value);
				pRow->PutValue(clcCost, rsService->Fields->Item["Price"]->Value);

				//update the CEMN object
				long nServiceID = VarLong(varOldValue);

				for(int i=0;i<m_pEMN->GetChargeCount();i++) {
					EMNCharge ec;
					m_pEMN->GetCharge(i, ec);
					if(ec.nServiceID == nServiceID) {
						ec.nServiceID = VarLong(varNewValue);
						ec.strDescription = AdoFldString(rsService, "Name","");
						ec.cyUnitCost = AdoFldCurrency(rsService, "Price",COleCurrency(0,0));
						ec.strSubCode = AdoFldString(rsService, "SubCode","");
						ec.strCode = AdoFldString(rsService, "Code","");
						m_pEMN->SetCharge(i, ec);
						bEmnModified = true;
					}
				}
			}
			break;*/
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		case clcCategory:
			{
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				long nCategoryID = VarLong(varNewValue, -1);
				pCharge->nCategoryID = nCategoryID;
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;
		case clcDescription:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->strDescription = VarString(varNewValue, "");
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;

		case clcMod1:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->strMod1 = (varNewValue.vt != VT_EMPTY ? VarString(varNewValue,"") : "");
				if(pCharge->strMod1 == "-1") { // ZM - PLID 19989 - They selected "(none)" so don't try to insert a modifier of -1.
					pCharge->strMod1 = "";
				}
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;

		case clcMod2:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->strMod2 = (varNewValue.vt != VT_EMPTY ? VarString(varNewValue,"") : "");
				if(pCharge->strMod2 == "-1") { // ZM - PLID 19989 - They selected "(none)" so don't try to insert a modifier of -1.
					pCharge->strMod2 = "";
				}
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;

		case clcMod3:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->strMod3 = (varNewValue.vt != VT_EMPTY ? VarString(varNewValue,"") : "");
				if(pCharge->strMod3 == "-1") { // ZM - PLID 19989 - They selected "(none)" so don't try to insert a modifier of -1.
					pCharge->strMod3 = "";
				}
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;

		case clcMod4:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->strMod4 = (varNewValue.vt != VT_EMPTY ? VarString(varNewValue,"") : "");
				if(pCharge->strMod4 == "-1") { // ZM - PLID 19989 - They selected "(none)" so don't try to insert a modifier of -1.
					pCharge->strMod4 = "";
				}
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;

		case clcQuantity:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->dblQuantity = VarDouble(varNewValue, 1.0);
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;

		case clcCost:
			{
				//update the CEMN object

				//DRT 1/11/2007 - PLID 24220 - We now do all EMNCharge work through pointers, and the pointer
				//	is saved in the datalist row.  Thus we can throw out all the code that was here and do it
				//	right out of the datalist.
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
				pCharge->cyUnitCost = VarCurrency(varNewValue, COleCurrency(0,0));
				pCharge->bChanged = TRUE;
				bEmnModified = true;
			}
			break;
		}
		if(bEmnModified) {
			//Tell our parent, so it can update its interface.
			// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface
			// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
			m_pEMN->SetCodesUnsaved();
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::EditingFinishingBillCode(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		// (j.gruber 2009-12-24 10:35) - PLID 15329 - check to see if they want to do this
		if (!WarnChargeAlreadyBilled()) {
			// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
			VariantClear(pvarNewValue);
			*pvarNewValue = _variant_t(varOldValue).Detach();
			*pbCommit = FALSE;
			return;
		}

		
		if(nCol == clcQuantity && pvarNewValue->vt == VT_R8) {

			if(pvarNewValue->dblVal <= 0.0) {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				AfxMessageBox("You must have a quantity greater than zero.");
				return;
			}
		}
		
	} NxCatchAll(__FUNCTION__);
}


// (c.haag 2010-10-06 09:21) - PLID 40384 - Handler for when the user edits inventory items
void CEmrCodesDlg::OnEditInventory()
{
	try {
		if (!CheckCurrentUserPermissions(bioInvItem,sptView))
			return;

		CInvEditDlg inv(this);
		inv.m_bPopup = true;
		CNxModalParentDlg dlg(this, &inv, CString("Inventory Items"), CRect(0, 0, 1024, 600)); // (r.goldschmidt 2014-06-23 17:24) - PLID 47280 - make dialog wider
		dlg.DoModal();

		// There are no product or service dropdowns; nothing to do here
	}
	NxCatchAll(__FUNCTION__);
}



// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::EditingStartingBillCode(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow==NULL){
			return;
		}
		
		EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

		switch(nCol)
		{
			case clcQuantity:
				// (z.manning 2011-07-12 16:26) - PLID 44469 - If this cpt code is part of a coding group then they can't edit quantity.
				if(m_pEMN->IsCptCodeInCodingGroup(pCharge->nServiceID)) {
					*pbContinue = FALSE;
					return;
				}
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::LeftClickBillCode(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		
		IRowSettingsPtr pRow(lpRow);
		if(pRow==NULL){
			return;
		}

		switch(nCol) {
		case clcWhichcodes:
			{
				//DRT 1/11/2007 - PLID 24177 - Clicking on the WhichCodes column launches
				//	the prompt to change what is selected.
				

				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

				//Find the right charge in our array
				//DRT 1/12/2007 - PLID 24178 - Moved Prompt to EMRUtils, we must provide our EMR.
				if(PromptToLinkDiagCodesToCharge(pCharge, m_pEMN, FALSE)) {
					//Update the interface
					// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
					// so just use that, or <None> if blank	
					//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
					pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));

					//Inform the parent that the more info has indeed changed
					// (a.walling 2012-03-22 16:50) - PLID 49141 - notify the interface
					// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
					m_pEMN->SetCodesUnsaved();
				}
			}
			break;

		// (j.jones 2008-07-22 12:02) - PLID 30792 - if they click on a problem icon, display the problems for this
		// charge in the problem list
		case clcProblemIcon:
			{
			EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

			if(pCharge) {
				
				// (c.haag 2009-05-19 13:04) - PLID 34312 - Use new problem structure
				if(pCharge->m_apEmrProblemLinks.GetSize() == 0) {
					AfxMessageBox("There are no problems currently on this charge.");
					return;
				}

				EditChargeProblems();
			}
			else {
				ASSERT(FALSE);
				ThrowNxException("Failed to get a valid charge object!");
			}
			}
			break;
		}


	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::RButtonDownBillCode(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow==NULL){
			return;
		}
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu pMenu;
			if(pRow) {
			m_BillList->PutCurSel(pRow);
			pMenu.CreatePopupMenu();
			EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

			// (z.manning 2011-07-12 16:41) - PLID 44469 - Added option to edit charge
			pMenu.AppendMenu(MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : MF_DISABLED|MF_GRAYED), ID_EDIT_CHARGE, "&Edit Charge");
			pMenu.AppendMenu(MF_SEPARATOR);

			// (j.jones 2008-07-22 12:45) - PLID 30792 - add ability to add problems, and view problem information
			if (!m_bIsTemplate) {
				// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
				// if the EMN is not writeable
				pMenu.AppendMenu(MF_STRING|MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), ID_ADD_CHARGE_PROBLEM, "Link with New &Problem");
				// (c.haag 2009-05-28 14:56) - PLID 34312 - Option for linking with existing problems
				CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
				if (NULL != pEMR) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), ID_LINK_CHARGE_PROBLEMS, "Link with Existing Problems");
				}
				if (pCharge->HasProblems()) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION, ID_EDIT_CHARGE_PROBLEM, "Update Problem &Information");
				}
				pMenu.AppendMenu(MF_SEPARATOR);
			}

			pMenu.AppendMenu(MF_BYPOSITION, ID_REMOVE_CHARGE, "Remove Charge");

			CPoint pt;
			GetCursorPos(&pt);
			pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
			pMenu.DestroyMenu();
		}
	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (j.jones 2008-07-25 09:00) - PLID 30792 - placed problem editing into their own functions
void CEmrCodesDlg::EditChargeProblems()
{
	try {
		IRowSettingsPtr pRow = m_BillList->CurSel;
		if(pRow ) {
			EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

			if(pCharge == NULL) {
				ThrowNxException("View Charge Problems - failed because no charge object was found!");
			}

			// (c.haag 2009-05-26 10:23) - PLID 34312 - Use the new problem link structure
			if(pCharge->m_apEmrProblemLinks.GetSize() == 0) {
				AfxMessageBox("This charge has no problems.");
				return;
			}

			//close the current problem list, if there is one
			CMainFrame *pFrame = GetMainFrame();
			if(pFrame) {
				pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
			}

			//now open filtered on this charge
			// (c.haag 2009-05-26 10:23) - PLID 34312 - Use the new problem link structure
			CEMRProblemListDlg dlg(this);
			dlg.SetDefaultFilter(m_pEMN->GetParentEMR()->GetPatientID(), eprtEmrCharge, pCharge->nID, pCharge->strDescription);
			dlg.LoadFromProblemList(GetEmrTreeWnd(), &pCharge->m_apEmrProblemLinks);
			dlg.DoModal();

			//try to update the icon
			_variant_t varProblemIcon = g_cvarNull;
			BOOL bChanged = FALSE;
			if(pCharge->HasProblems()) {
				if(pCharge->HasOnlyClosedProblems()) {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
					//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
					varProblemIcon = (long)hProblem;
				}
				else {
					//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
			}
			else {
				bChanged = TRUE;
			}
			pRow->PutValue(clcProblemIcon, varProblemIcon);

			ShowHideProblemIconColumn_ChargeList();

			//also see if any problem changed, if so, mark the charge as changed
			if(bChanged || pCharge->HasChangedProblemLinks()) {

				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);

				if(!m_pEMN->IsLockedAndSaved()) {

					pCharge->bChanged = TRUE;

					// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
					// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
					m_pEMN->SetCodesUnsaved();
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (c.haag 2009-05-28 15:47) - PLID 34312 - Link the selected charge with existing problems
void CEmrCodesDlg::LinkChargeProblems()
{
	try {
		IRowSettingsPtr pRow = m_BillList->CurSel;
		if(pRow ) {
			EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
			CArray<CEmrProblemLink*,CEmrProblemLink*> aNewProblemLinks;
			int i;

			if(pCharge == NULL) {
				ThrowNxException("View Charge Problems - failed because no charge object was found!");
			}

			for (i=0; i < pCharge->m_apEmrProblemLinks.GetSize(); i++) {
				pCharge->m_apEmrProblemLinks[i]->UpdatePointersWithCharge(m_pEMN, pCharge);
			}
			if (LinkProblems(pCharge->m_apEmrProblemLinks, eprtEmrCharge, pCharge->nID, aNewProblemLinks)) {
				for (i=0; i < aNewProblemLinks.GetSize(); i++) {
					aNewProblemLinks[i]->UpdatePointersWithCharge(m_pEMN, pCharge);
					pCharge->m_apEmrProblemLinks.Add(aNewProblemLinks[i]);
				}

				//try to update the icon
				_variant_t varProblemIcon = g_cvarNull;
				BOOL bChanged = FALSE;
				if(pCharge->HasProblems()) {
					if(pCharge->HasOnlyClosedProblems()) {
						HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
						//HICON hProblem =  theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
						varProblemIcon = (long)hProblem;
					}
					else {
						HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
						//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
						varProblemIcon = (long)hProblem;
					}
				}
				else {
					bChanged = TRUE;
				}
				pRow->PutValue(clcProblemIcon, varProblemIcon);

				ShowHideProblemIconColumn_ChargeList();

				//also see if any problem changed, if so, mark the charge as changed
				if(bChanged || pCharge->HasChangedProblemLinks()) {

					// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
					GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);

					if(!m_pEMN->IsLockedAndSaved()) {
						pCharge->bChanged = TRUE;
						// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
						// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
						m_pEMN->SetCodesUnsaved();
					}
				}
			} // if (LinkProblems(pCharge->m_apEmrProblemLinks, eprtEmrCharge, pCharge->nID, aNewProblemLinks)) {
		} // if(m_BillList->GetCurSel() != -1) {
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (j.jones 2008-07-22 09:05) - PLID 30792 - this function will show or hide
// the problem icon column in the charge list, based on whether any charges have the icon
void CEmrCodesDlg::ShowHideProblemIconColumn_ChargeList()
{
	try {

		//see if any row has an icon, and if so, show the column, otherwise hide it

		BOOL bHasIcon = FALSE;
		IRowSettingsPtr pRow =m_BillList->GetFirstRow(); 
		while(pRow && !bHasIcon)
		{
			long nIcon = VarLong(pRow->GetValue( clcProblemIcon), 0);
			if(nIcon != 0) {
				bHasIcon = TRUE;
			}
			pRow =pRow ->GetNextRow(); 
		}

		IColumnSettingsPtr pCol = m_BillList->GetColumn(clcProblemIcon);

		if(bHasIcon) {
			//show the column			
			pCol->PutStoredWidth(20);
		}
		else {
			//hide the column
			pCol->PutStoredWidth(0);
		}

	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info now in top level frame

// (c.haag 2009-05-28 15:22) - PLID 34312 - This function is called when the user wants to link something
// in the more info section with one or more existing problems. Returns TRUE if at least one problem was
// added to the output array
BOOL CEmrCodesDlg::LinkProblems(CArray<CEmrProblemLink*,CEmrProblemLink*>& aryEMRObjectProblemLinks, 
								  EMRProblemRegardingTypes eprtType, long nEMRRegardingID,
								  OUT CArray<CEmrProblemLink*,CEmrProblemLink*>& aNewProblemLinks)
{
	CArray<CEmrProblem*,CEmrProblem*> aryAllProblems;
	CArray<CEmrProblem*,CEmrProblem*> aryEMRObjectProblems;
	CArray<CEmrProblem*,CEmrProblem*> arySelectionsInMemory;
	CArray<long,long> arynSelectionsInData;
	int i;

	CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
	if (NULL == m_pEMN || NULL == pEMR) {
		ThrowNxException("Called CEmrCodesDlg::LinkProblems without a valid EMN or EMR");
	}

	// We require problem create permissions to create new links
	if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
		return FALSE;
	}

	// Pull ALL problems, including deleted ones, because the chooser dialog will run a query
	// that must be filtered by both deleted and non-deleted problems in memory
	pEMR->GetAllProblems(aryAllProblems, TRUE);
	for (i=0; i < aryEMRObjectProblemLinks.GetSize(); i++) {
		EnsureProblemInArray(aryEMRObjectProblems, aryEMRObjectProblemLinks[i]->GetProblem());
	}

	CEMRProblemChooserDlg dlg(aryAllProblems, aryEMRObjectProblems, pEMR->GetPatientID(), this);
	if (dlg.HasProblems()) {
		if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
			// User chose at least one problem
			
			// First, go through the problems that already exist in memory (and owned
			// by this EMR), and create problem links for them.
			for (i=0; i < arySelectionsInMemory.GetSize(); i++) {
				CEmrProblemLink* pNewLink = new CEmrProblemLink(arySelectionsInMemory[i], -1, eprtType, nEMRRegardingID, -1);
				aNewProblemLinks.Add(pNewLink);
			}
			// Next, go through the problems that exist in data and not in memory, create
			// problem objects for them, and then links for those. Here's the neat part:
			// If the problem is already in memory, but linked with nothing; then the EMR
			// is smart enough to just give you the problem already in memory when calling
			// the function to allocate it.
			// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
			// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID
			// (s.tullis 2015-02-23 15:44) - PLID 64723
			if (arynSelectionsInData.GetSize() > 0) {
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				// (s.dhole 2014-02-27 12:47) - PLID 60996  added DiagCodeID_ICD10  to sQl
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
				_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
					"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID,DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, PatientID, EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, "
					"EmrProblemsT.DoNotShowOnProblemPrompt "
					"FROM EMRProblemsT "
					"WHERE Deleted = 0 AND ID IN ({INTARRAY})", arynSelectionsInData);
				while (!prs->eof) {
					CEmrProblem* pNewProblem = pEMR->AllocateEmrProblem(prs->Fields);
					CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtType, nEMRRegardingID, -1);
					aNewProblemLinks.Add(pNewLink);
					pNewProblem->Release();
					prs->MoveNext();
				}
			}

			// (c.haag 2009-06-01 12:56) - PLID 34312 - Because we added or modified a problem or problem link, we must flag
			// the EMR as modified.
			pEMR->SetUnsaved();

		}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
		else {
			// User changed their mind
		}
	} else {
		// Dialog has no visible problems
		AfxMessageBox("There are no available problems to choose from.");
	}

	return (aNewProblemLinks.GetSize() > 0) ? TRUE : FALSE;
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (j.jones 2008-06-03 12:01) - PLID 30062 - added ability to add quote charges
BOOL CEmrCodesDlg::AddQuoteToBillList(long nQuoteID)
{
	BOOL bSomeAdded = FALSE;

	try {

		//don't bother handling packages in a special case,
		//just add their charges like a normal quote

		//do not include quote charges that are only outside fees
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		_RecordsetPtr rs = CreateParamRecordset("SELECT ChargesT.ID, ChargesT.ServiceID, ChargesT.ItemCode, ChargesT.ItemSubCode, "
			"LineItemT.Description, ChargesT.Quantity, LineItemT.Amount, CPTCodeT.Billable,ChargesT.Category ,COALESCE(CptCategoryCountQ.CPTCategoryCount, 0 ) AS CategoryCount "
			"FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
			"LEFT JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"LEFT JOIN ( Select ServiceID, COUNT( DISTINCT ServiceMultiCategoryT.CategoryID ) as CPTCategoryCount "
			"            FROM ServiceMultiCategoryT "
			"            Group BY ServiceID ) CptCategoryCountQ On  CptCategoryCountQ.ServiceID = ChargesT.ServiceID "
			"WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
			"AND BillsT.EntryType = 2 AND LineItemT.Type = 11 "
			"AND (LineItemT.Amount > Convert(money, 0) OR ChargesT.OthrBillFee = Convert(money, 0)) "
			"AND BillsT.ID = {INT} "
			"ORDER BY ChargesT.LineID", nQuoteID);

		while(!rs->eof) {

			long nServiceID = AdoFldLong(rs, "ServiceID", -1);
			CString strCode = AdoFldString(rs, "ItemCode", "");
			CString strSubCode = AdoFldString(rs, "ItemSubCode", "");
			CString strDescription = AdoFldString(rs, "Description", "");
			double dblQuantity = AdoFldDouble(rs, "Quantity", 1.0);
			COleCurrency cyPrice = AdoFldCurrency(rs, "Amount", COleCurrency(0,0));
			BOOL bBillable = AdoFldBool(rs, "Billable", TRUE);
			// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
			long nCategory = AdoFldLong(rs, "Category", -1);
			long nCategoryCount = AdoFldLong(rs, "CategoryCount", 0);

			// (j.jones 2008-06-04 16:21) - PLID 30255 - added nQuoteChargeID,
			// so the EMR can keep track of the charge from the quote
			long nQuoteChargeID = AdoFldLong(rs, "ID", -1);
			
			// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
			// (j.jones 2012-08-22 09:35) - PLID 50486 - added default insured party ID
			if (AddChargeToBillList(nServiceID, strCode, strSubCode, strDescription, m_pEMN->GetParentEMR()->GetDefaultChargeInsuredPartyID(), bBillable, cyPrice, nCategory, nCategoryCount, dblQuantity, NULL, nQuoteChargeID)) {
				bSomeAdded = TRUE;
			}
			else {
				//this can only happen if an exception occurred, so abort adding the quote
				return bSomeAdded;
			}

			rs->MoveNext();
		}
		rs->Close();

		return bSomeAdded;

	}NxCatchAll(__FUNCTION__);

	return bSomeAdded;
}


// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (j.jones 2007-08-30 11:26) - PLID 27221 - added optional parameter for pending E/M checklist audit information
// (j.jones 2008-06-03 12:14) - PLID 30062 - added quantity
// (j.jones 2008-06-04 16:20) - PLID 30255 - added nQuoteChargeID
// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
// (j.dinatale 2012-01-05 10:12) - PLID 47464 - added Insured Party ID
// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
// (j.jones 2012-08-22 09:38) - PLID 50486 - moved insured party ID to be a required parameter
BOOL CEmrCodesDlg::AddChargeToBillList(long nServiceID, CString strCode, CString strSubCode, CString strDescription, long nInsuredPartyID, BOOL bBillable, COleCurrency cyPrice, long nCategory, long nCategoryCount,double dblQuantity /*= 1.0*/, CArray<CPendingAuditInfo*, CPendingAuditInfo*> *paryPendingEMAuditInfo /*= NULL*/, long nQuoteChargeID /*= -1*/)
{
	try {
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Only Show the Category Column if they have multiple categories configured
		BOOL bShowChargeCategory = FALSE;
		CDC* pWndDC = GetDC();
		BOOL bIsHighColor = ::GetDeviceCaps(pWndDC->m_hDC, BITSPIXEL) > 8;
		ReleaseDC(pWndDC);

		// (j.jones 2011-07-07 15:48) - PLID 38366 - if this service is part of a coding group,
		// then TryUpdateCodingGroup will automatically handle adding the correct charge, otherwise
		// we need to add it ourselves
		if(!m_bIsTemplate) {
			if(m_pEMN->TryUpdateCodingGroupByServiceID(nServiceID, FALSE)) {
				//the addition has been handled per the coding group rules,
				//and we do not need to do anything further
				return TRUE;
			}
		}

		// (j.jones 2012-03-27 15:10) - PLID 44763 - warn if we're under a global period
		CEmrTreeWnd *pTreeWnd = GetEmrTreeWnd();
		if(pTreeWnd) {
			pTreeWnd->CheckWarnGlobalPeriod_EMR(m_pEMN->GetEMNDate());
		}

		// (m.hancock 2006-09-13 16:53) - PLID 19848 - Add the charge if it does not already exist.
		// We used to just prevent adding charges that already existed, but with PLID 19848 we're
		// going to increase the quantity of the charge when it is added again.

		// (j.jones 2010-09-22 08:13) - PLID 24221 - we now have a preference to decide whether
		// we increase the quantity or add duplicates, it defaults to the bill preference
		BOOL bAllowQtyIncrement = GetRemotePropertyInt("EMNChargesAllowQtyIncrement",
			GetRemotePropertyInt("ChargeAllowQtyIncrement", 0, 0, "<None>", false),
			0, "<None>", true);

		EMNCharge *pFoundCharge = NULL;
		//don't need to check that the charge exists if we aren't increasing quantity
		if(bAllowQtyIncrement) {
			// (j.jones 2012-02-20 09:25) - PLID 47886 - need to search our memory objects, not the datalist
			pFoundCharge = m_pEMN->FindByServiceID(nServiceID);
		}
		if(!bAllowQtyIncrement || pFoundCharge == NULL) {

			long nID = -1;

			//DRT 1/11/2007 - PLID 24220 - Setup the datalist so that a pointer to the EMNCharge object
			//	is maintained in the list.

			//create the memory object
			EMNCharge *pCharge = new EMNCharge;
			pCharge->nID = nID;
			pCharge->nServiceID = nServiceID;
			pCharge->strDescription = strDescription;
			pCharge->dblQuantity = dblQuantity;
			pCharge->cyUnitCost = cyPrice;
			pCharge->bChanged = TRUE;
			pCharge->strSubCode = strSubCode;
			pCharge->strCode = strCode;
			// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
			pCharge->nCategoryID = nCategory;
			pCharge->nCategoryCount = nCategoryCount;
			// (j.jones 2011-03-28 14:45) - PLID 42575 - added Billable flag
			pCharge->bBillable = bBillable;

			// (j.jones 2008-06-04 16:21) - PLID 30255 - added nQuoteChargeID
			pCharge->nQuoteChargeID = nQuoteChargeID;

			// (j.dinatale 2012-01-05 10:12) - PLID 39451 - added Insured Party ID
			pCharge->nInsuredPartyID = nInsuredPartyID;

			// (j.jones 2007-08-30 11:28) - PLID 27211 - if we have a list of audit items, add them to our object
			if(paryPendingEMAuditInfo) {
				for(int i=0; i < paryPendingEMAuditInfo->GetSize(); i++) {
					CPendingAuditInfo* pOldInfo = (CPendingAuditInfo*)(paryPendingEMAuditInfo->GetAt(i));
					CPendingAuditInfo* pNewInfo = new CPendingAuditInfo(pOldInfo);
					pCharge->aryPendingEMAuditInfo.Add(pNewInfo);
				}
			}

			//Add the charge
			m_pEMN->AddCharge(pCharge);

		
			// (j.jones 2012-02-17 10:39) - PLID 47886 - do not add to the list if the billable setting
			// says to hide unbillable charges
			// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
			if(bBillable || !m_checkHideUnbillableCPTCodes.GetCheck()) {
				//add to the datalist
				IRowSettingsPtr pRow = m_BillList->GetNewRow();
				SetCPTCategoryCombo(pRow, pCharge);
				// (s.tullis 2015-04-01 14:09) - PLID 64978 - Just need one charge with multi categories to show the column
				// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
				if (pCharge->nCategoryCount > 1 && m_bIsTemplate == FALSE){
					bShowChargeCategory = TRUE;
				}
				pRow->PutValue(clcID, nID);
				pRow->PutValue(clcServiceID, nServiceID);
				pRow->PutValue(clcCategory, nCategory);
				pRow->PutValue(clcProblemIcon, g_cvarNull);	// (j.jones 2008-07-21 14:02) - PLID 30792
				pRow->PutValue(clcCode, _bstr_t(strCode));
				pRow->PutValue(clcSubcode, _bstr_t(strSubCode));
				pRow->PutValue(clcDescription, _bstr_t(strDescription));
				pRow->PutValue(clcQuantity, dblQuantity);
				pRow->PutValue(clcCost, _variant_t(cyPrice));
				pRow->PutValue(clcEMNChargePtr, (long)pCharge);
				//DRT 1/11/2007 - PLID 24177 - Add WhichCodes field, initially blank
				// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
				// so just use that, or <None> if blank	
				//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
				pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));
				// (d.singleton 2011-11-01 10:34) - PLID 43548 - added billable column so if pref is enabled they can sort by billable status
				pRow->PutValue(clcBillable, bBillable);

				m_BillList->AddRowSorted(pRow,NULL);			

				// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
				ShowHideProblemIconColumn_ChargeList();

				// (j.jones 2011-03-28 15:03) - PLID 42575 - if the charge is not billable,
				// color it gray
				if(!pCharge->bBillable) {
					if(bIsHighColor) {
						pRow->PutBackColor(EMR_GHOSTLY_16_BIT_GRAY);
					}
					else {
						pRow->PutBackColor(EMR_GHOSTLY_8_BIT_GRAY);
					}
				}
			
				// (j.jones 2008-06-04 16:23) - PLID 30255 - if the charge is from a quote,
				// color the row accordingly
				else if(pCharge->nQuoteChargeID != -1) {
					pRow->PutBackColor(EMR_SOME_CHARGES_ON_QUOTES);
				}

				m_BillList->Sort();
			}

			// (s.tullis 2015-04-01 14:09) - PLID 64978 - Show the category column if there are multiple categories configured
			if (bShowChargeCategory)
			{
				ForceShowColumn(clcCategory, 10, 85);
			}

			return TRUE;
		}
		// (m.hancock 2006-09-13 16:53) - PLID 19848 - Increase the quantity for the charge if it already exists.
		else {
			// (j.jones 2012-02-20 09:43) - PLID 47886 - use the charge pointer, not a datalist row
			long nID = pFoundCharge->nID;
			long nServiceID = pFoundCharge->nServiceID;

			//Set the updated quantity
			pFoundCharge->dblQuantity += dblQuantity;

			//Mark the object as having been changed
			pFoundCharge->bChanged = TRUE;

			//update the datalist through OnEmnChargeChanged
			CWnd* pInterfaceWnd = m_pEMN->GetInterface();
			if(pInterfaceWnd) {
				pInterfaceWnd->SendMessage(NXM_EMN_CHARGE_CHANGED, (WPARAM)pFoundCharge, (LPARAM)m_pEMN);
			}
			// (j.jones 2012-06-18 11:02) - PLID 49141 - notify the interface
			// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
			m_pEMN->SetCodesUnsaved();

			return TRUE;
		}


	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
//DRT 1/11/2007 - PLID 24220 - Changed to accept an EMNCharge* pointer, not a serviceID
void CEmrCodesDlg::RemoveCharge(EMNCharge* pCharge)
{
	try {
		//remove from the datalist
		// (c.haag 2008-07-31 12:42) - PLID 30905 - Check if the index is -1 before trying to actually
		// remove it. Silently fail if it doesn't exist; there's nothing to do in that case.
		IRowSettingsPtr pRow  = m_BillList->FindByColumn(clcEMNChargePtr, (long)pCharge, 0, FALSE);
		if (pRow  ) {
			m_BillList->RemoveRow(pRow);

			// (j.jones 2008-07-22 09:26) - PLID 30792 - try to hide the problem icon column
			ShowHideProblemIconColumn_ChargeList();
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
// (j.dinatale 2012-01-19 09:11) - PLID 47620 - moved the show charge split dialog logic into a function, now returns the result of the dialog
long CEmrCodesDlg::ShowChargeRespDlg()
{
	// (j.dinatale 2012-01-03 17:52) - PLID 39451 - create a charge split dlg and fill it
	CArray<long, long> aryChargeObjs;
	IRowSettingsPtr pRow =m_BillList->GetFirstRow(); 
	while(pRow ){
		EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

		if(pCharge->bBillable){
			aryChargeObjs.Add((long)pCharge);
		}
		pRow=pRow->GetNextRow(); 
	}

	// (j.dinatale 2012-01-12 11:31) - PLID 47483 - need to pass in the new enum value to state we are using EMNCharges
	CChargeSplitDlg dlg(ChrgSptDlgEnums::EMNCharge, aryChargeObjs, m_nPatientID, m_pEMN->GetID(), m_bReadOnly);
	long nResult = dlg.DoModal();
	if(IDOK == nResult && dlg.ChargesChanged()){
		//Tell our parent, so it can update its interface.
		// (a.walling 2012-03-22 16:50) - PLID 49141 - notify the interface
		// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
		m_pEMN->SetCodesUnsaved();
	}

	return nResult;
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
BOOL CEmrCodesDlg::WarnChargeAlreadyBilled() {

	if (!m_bIsTemplate) {

		//check the id, if its -1 then its not saved
		if (m_pEMN->GetID() > 0) {

			//let's also only warn them once per open EMN if they say yes, so we don't bug them
			if (!m_bHadBillWarning) {

				//it's saved, so check if its been billed
				_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM BilledEMNsT WHERE BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1) AND EMNID = {INT}", m_pEMN->GetID());
				if (!rsCheck->eof) {
					//pop up the warning
					if (IDYES == MsgBox(MB_YESNO, "This EMN has already been billed, any changes to the charges will not be reflected in the bill.  Do you want to continue?")) {
						m_bHadBillWarning = TRUE;
						return TRUE;
					}
					else {
						return FALSE;
					}
				}
			}
		}
	}

	return TRUE;

}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from <More Info> dialog And updte to datalist2
void CEmrCodesDlg::RefreshChargeList()
{
	try {
		BOOL bShowChargeCategory = FALSE;
		CDC* pWndDC = GetDC();
		BOOL bIsHighColor = ::GetDeviceCaps(pWndDC->m_hDC, BITSPIXEL) > 8;
		ReleaseDC(pWndDC);

		//reload the charge list
		
		m_BillList->Clear();

		for(int i=0;i<m_pEMN->GetChargeCount(); i++) {
			//DRT 1/11/2007 - PLID 24220 - We now track all EMNCharge objects by pointer, and save
			//	that pointer in the datalist as well.
			EMNCharge *pCharge = m_pEMN->GetCharge(i);

			// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
			_variant_t varProblemIcon = g_cvarNull;			
			if(pCharge->HasProblems()) {
				if(pCharge->HasOnlyClosedProblems()) {
					//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
				else {
					//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
			}

			// (j.jones 2012-02-17 10:39) - PLID 47886 - do not add to the list if the billable setting
			// says to hide unbillable charges
			if(pCharge->bBillable || !m_checkHideUnbillableCPTCodes.GetCheck()) {
				IRowSettingsPtr pRow = m_BillList->GetNewRow() ;
				// (s.tullis 2015-04-01 14:09) - PLID 64978 - Only Show the column if the service code has multiple catagories configured
				// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
				if (pCharge->nCategoryCount > 1 && m_bIsTemplate == FALSE)
				{
					bShowChargeCategory = TRUE;
				}
				SetCPTCategoryCombo(pRow, pCharge);
				pRow->PutValue(clcID, pCharge->nID);
				pRow->PutValue(clcServiceID, pCharge->nServiceID);
				pRow->PutValue(clcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
				pRow->PutValue(clcCode, _bstr_t(pCharge->strCode));
				pRow->PutValue(clcSubcode, _bstr_t(pCharge->strSubCode));
				pRow->PutValue(clcDescription, _bstr_t(pCharge->strDescription));
				// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
				pRow->PutValue(clcCategory, pCharge->nCategoryID);
				pRow->PutValue(clcMod1, _bstr_t(pCharge->strMod1));
				pRow->PutValue(clcMod2, _bstr_t(pCharge->strMod2));
				pRow->PutValue(clcMod3, _bstr_t(pCharge->strMod3));
				pRow->PutValue(clcMod4, _bstr_t(pCharge->strMod4));
				pRow->PutValue(clcQuantity, pCharge->dblQuantity);
				pRow->PutValue(clcCost, _variant_t(pCharge->cyUnitCost));
				pRow->PutValue(clcEMNChargePtr, (long)pCharge);
				//DRT 1/11/2007 - PLID 24177 - Fill the whichcodes field
				// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
				// so just use that, or <None> if blank	
				//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
				pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));

				m_BillList->AddRowSorted(pRow,NULL);

				// (j.jones 2011-03-28 15:03) - PLID 42575 - if the charge is not billable,
				// color it gray
				if(!pCharge->bBillable) {
					if(bIsHighColor) {
						pRow->PutBackColor(EMR_GHOSTLY_16_BIT_GRAY);
					}
					else {
						pRow->PutBackColor(EMR_GHOSTLY_8_BIT_GRAY);
					}
				}
				// (j.jones 2008-06-04 16:23) - PLID 30255 - if the charge is from a quote,
				// color the row accordingly
				else if(pCharge->nQuoteChargeID != -1) {
					pRow->PutBackColor(EMR_SOME_CHARGES_ON_QUOTES);
				}
			}
		}
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Show the column 
		if (bShowChargeCategory)
		{
			ForceShowColumn(clcCategory, 10, 85);
		}
		// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
		ShowHideProblemIconColumn_ChargeList();

	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-02-14 14:48) - PLID 60742 Copy Code from More Info dialog
LRESULT CEmrCodesDlg::OnPrepareBillListCompleted(WPARAM wParam, LPARAM lParam)
{
	try {
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Show column if service code has multiple catagories configured
		BOOL bShowChargeCategory= FALSE;
		CDC* pWndDC = GetDC();
		BOOL bIsHighColor = ::GetDeviceCaps(pWndDC->m_hDC, BITSPIXEL) > 8;
		ReleaseDC(pWndDC);

		PrepareBillListInfo *pPbli = (PrepareBillListInfo*)wParam;
		m_BillList->GetColumn(clcMod1)->ComboSource = _bstr_t(pPbli->strCptModifierListOut);
		m_BillList->GetColumn(clcMod2)->ComboSource = _bstr_t(pPbli->strCptModifierListOut);
		m_BillList->GetColumn(clcMod3)->ComboSource = _bstr_t(pPbli->strCptModifierListOut);
		m_BillList->GetColumn(clcMod4)->ComboSource = _bstr_t(pPbli->strCptModifierListOut);
		m_BillList->Requery();
		delete pPbli;

		m_bPreparingBillList = FALSE;

		//charges
		for(int i=0;i<m_pEMN->GetChargeCount(); i++) {
			//DRT 1/11/2007 - PLID 24220 - We now handle all EMNCharge work by pointer, and additionally
			//	save the pointer in the datalist itself.
			EMNCharge *pCharge = m_pEMN->GetCharge(i);

			// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
			_variant_t varProblemIcon = g_cvarNull;			
			if(pCharge->HasProblems()) {
				if(pCharge->HasOnlyClosedProblems()) {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
				else {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
			}

			// (j.jones 2012-02-17 10:39) - PLID 47886 - do not add to the list if the billable setting
			// says to hide unbillable charges
			if(pCharge->bBillable || !m_checkHideUnbillableCPTCodes.GetCheck()) {
				IRowSettingsPtr pRow = m_BillList->GetNewRow();
				// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
				if (pCharge->nCategoryCount > 1 && m_bIsTemplate == FALSE)
				{
					bShowChargeCategory = TRUE;
				}
				SetCPTCategoryCombo(pRow, pCharge);
				pRow->PutValue(clcID, pCharge->nID);
				pRow->PutValue(clcServiceID, pCharge->nServiceID);
				pRow->PutValue(clcCategory, pCharge->nCategoryID);
				pRow->PutValue(clcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
				pRow->PutValue(clcCode, _bstr_t(pCharge->strCode));
				pRow->PutValue(clcSubcode, _bstr_t(pCharge->strSubCode));
				pRow->PutValue(clcDescription, _bstr_t(pCharge->strDescription));
				pRow->PutValue(clcMod1, _bstr_t(pCharge->strMod1));
				pRow->PutValue(clcMod2, _bstr_t(pCharge->strMod2));
				pRow->PutValue(clcMod3, _bstr_t(pCharge->strMod3));
				pRow->PutValue(clcMod4, _bstr_t(pCharge->strMod4));
				pRow->PutValue(clcQuantity, pCharge->dblQuantity);
				pRow->PutValue(clcCost, _variant_t(pCharge->cyUnitCost));
				pRow->PutValue(clcEMNChargePtr, (long)pCharge);
				//DRT 1/11/2007 - PLID 24177 - Fill the whichcodes field
				// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
				// so just use that, or <None> if blank	
				//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
				pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));
				// (d.singleton 2011-10-31 14:33) - PLID 43548 - added billable as a column so we can sort on it if pref is enabled
				pRow->PutValue(clcBillable, pCharge->bBillable);

				m_BillList->AddRowSorted(pRow,NULL);

				// (j.jones 2011-03-28 15:03) - PLID 42575 - if the charge is not billable,
				// color it gray
				if(!pCharge->bBillable) {
					if(bIsHighColor) {
						pRow->PutBackColor(EMR_GHOSTLY_16_BIT_GRAY);
					}
					else {
						pRow->PutBackColor(EMR_GHOSTLY_8_BIT_GRAY);
					}
				}
				// (j.jones 2008-06-04 16:23) - PLID 30255 - if the charge is from a quote,
				// color the row accordingly
				else if(pCharge->nQuoteChargeID != -1) {
					pRow->PutBackColor(EMR_SOME_CHARGES_ON_QUOTES);
				}
			}
		}
		
		// (d.singleton 2011-11-01 10:32) - PLID 43548 - if pref enabled then auto sort the charges list by billable status so all unbillable codes are at the bottom of list.
		if(GetRemotePropertyInt("EMNChargesSortByBillable", 0, 0, GetCurrentUserName(), true) == 1)
		{
			IColumnSettingsPtr pCol = m_BillList->GetColumn(clcBillable);

			pCol->PutSortPriority(0);
			pCol->PutSortAscending(FALSE);
			m_BillList->PutAllowSort(FALSE);
		}

		m_BillList->Sort();

		// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
		ShowHideProblemIconColumn_ChargeList();

		GetDlgItem(IDC_BTN_CREATE_QUOTE_CODE)->EnableWindow(!m_bIsTemplate);
		GetDlgItem(IDC_BTN_CREATE_BILL_CODE)->EnableWindow(!m_bIsTemplate);
		GetDlgItem(IDC_BTN_ADD_CHARGE_CODE)->EnableWindow(!m_bReadOnly);
		// (s.dhole 2014-03-21 12:53) - PLID 60742
		m_BillList->PutReadOnly(m_bReadOnly);
		// (b.savon 2014-02-28 08:07) - PLID 60805 - UPDATE - Add new button, "Existing Quote" in the procedure codes section of the <Codes> topic
		if( m_bIsTemplate || m_bReadOnly ){
			m_btnExistingQuote.EnableWindow(FALSE);
		}else{
			m_btnExistingQuote.EnableWindow(TRUE);
		}
		m_BillList->PutReadOnly(m_bReadOnly);
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		if (bShowChargeCategory)
		{
			ForceShowColumn(clcCategory, 10, 85);
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (s.dhole 2014-02-17 15:28) - PLID  60742 Copy Code from More Info dialog
BOOL CEmrCodesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(wParam == ID_REMOVE_CHARGE) {
		try {
			IRowSettingsPtr pRow = m_BillList->CurSel; 
			if(pRow ) {

				// (j.gruber 2009-12-24 10:33) - PLID 15329 - see if its billed and they want to do this
				if (!WarnChargeAlreadyBilled()) {
					return CNxDialog::OnCommand(wParam, lParam);			
				}

				//for the time being, all charges are deleted and re-saved from scratch, so nothing else is necessary for now

				//DRT 1/11/2007 - PLID 24220 - Changed to use EMNCharge* pointers in the datalist
				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow ->GetValue( clcEMNChargePtr));

				// (c.haag 2008-07-25 16:14) - PLID 30826 - Don't let the user delete the charge
				// if it is tied to saved problems and the user doesn't have permissions
				if (!CanCurrentUserDeleteEmrProblems()) {
					if (NULL != pCharge && m_pEMN->DoesChargeHaveSavedProblems(pCharge)) {
						MsgBox(MB_OK | MB_ICONERROR, "You may not remove this charge because it is associated with one or more saved problems.");
						return CNxDialog::OnCommand(wParam, lParam);			
					}
				}

				// (j.jones 2008-07-29 14:14) - PLID 30729 - see if there are any problems at all
				BOOL bHasProblems = pCharge->HasProblems();

				//remove from the CEMN object
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				m_pEMN->RemoveCharge(pCharge);

				//remove from the datalist
				m_BillList->RemoveRow(pRow);

				// (j.jones 2008-07-22 09:26) - PLID 30792 - try to hide the problem icon column
				ShowHideProblemIconColumn_ChargeList();

				// (j.jones 2008-07-29 14:15) - PLID 30729 - if we has problems, send a message
				// stating that they changed
				if(bHasProblems) {
					GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);
				}
			}
		} NxCatchAll("CEmrCodesDlg::OnCommand:ID_REMOVE_CHARGE");
	}
	else if(wParam == ID_EDIT_CHARGE)
	{
		IRowSettingsPtr pRow = m_BillList->CurSel; 
		// (z.manning 2011-07-12 16:46) - PLID 44469 - Added option to edit charge
		if(pRow)
		{
			EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue( clcEMNChargePtr));
			if(m_pEMN->IsCptCodeInCodingGroup(pCharge->nServiceID)) {
				// (z.manning 2011-07-12 16:58) - PLID 44469 - This is part of a coding group so we need to popup
				// a dialog for the whole group.
				CEmrCodingGroup *pCodingGroup = GetMainFrame()->GetEmrCodingGroupManager()->GetCodingGroupByCptID(pCharge->nServiceID);
				m_pEMN->UpdateCodingGroup(pCodingGroup, TRUE, 0);
			}
			else {
				// (z.manning 2011-07-12 16:55) - PLID 44469 - This is a normal charge row but let's go ahead and
				// pop up the charge prompt dialog for it.
				CEMNChargeArray arypCharge;
				arypCharge.Add(pCharge);
				// (j.jones 2012-01-26 11:19) - PLID 47700 - added patient ID, and an optional EMNID,
				// both are -1 if this is a template, EMNID is -1 if unsaved
				CEMRChargePromptDlg dlg(this, m_bIsTemplate ? -1 : m_nPatientID, &arypCharge, m_bIsTemplate ? -1 : m_pEMN->GetID());
				if(dlg.DoModal() == IDOK) {
					pCharge->bChanged = TRUE;
					// (a.walling 2012-03-22 16:50) - PLID 49141 - Notify the interface
					// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
					m_pEMN->SetCodesUnsaved();
					SyncBillListRowWithEMNCharge(pCharge, pRow);

				}
			}
		}
	}
	
	else if(wParam == ID_ADD_CHARGE_PROBLEM) {

		try {
			IRowSettingsPtr pRow = m_BillList->CurSel; 
			if(pRow) {

				EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

				if(pCharge == NULL) {
					ThrowNxException("Add Charge Problem - failed because no charge object was found!");
				}

				if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
				// this is independent of locking
				if(!m_pEMN->IsWritable()) {
					//prompt, but do not attempt to take access, let the user decide what to do
					AfxMessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				CEMRProblemEditDlg dlg(this);

				const BOOL bEMNIsLocked = (m_pEMN->GetStatus() == 2) ? TRUE : FALSE;
				if (bEMNIsLocked) {
					dlg.SetWriteToData(TRUE); // This will force the dialog to save the problem details to data
				}

				// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
				// (j.jones 2009-05-21 16:42) - PLID 34250 - renamed this function
				dlg.AddLinkedObjectInfo(-1, eprtEmrCharge, pCharge->strDescription, "", pCharge->nID);
				dlg.SetPatientID(m_pEMN->GetParentEMR()->GetPatientID());

				if (IDOK == dlg.DoModal()) {
					if (!dlg.ProblemWasDeleted()) {

						COleDateTime dtInvalid;
						dtInvalid.SetStatus(COleDateTime::invalid);

						// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
						// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
						// (c.haag 2009-05-16 14:21) - PLID 34312 - Create a new problem and problem link
						CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
						if (NULL != pEMR) {

							// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
							long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
							dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

							// (z.manning 2009-05-27 10:14) - PLID 34297 - Added patient ID
							// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
							// (s.tullis 2015-02-23 15:44) - PLID 64723
							// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
							CEmrProblem *pProblem = pEMR->AllocateEmrProblem(dlg.GetProblemID(), pEMR->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
								dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(),dlg.GetProblemDoNotShowOnCCDA(),
								dlg.GetProblemDoNotShowOnProblemPrompt());
							CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtEmrCharge, pCharge->nID, -1);
							pNewLink->UpdatePointersWithCharge(m_pEMN, pCharge);
							pCharge->m_apEmrProblemLinks.Add(pNewLink);
							// (c.haag 2009-05-16 14:23) - PLID 34312 - Release the reference that pProblem has
							// to the problem. Only the detail and the EMR (which tracks all the problems it created)
							// should own a reference.
							pProblem->Release();
						} else {
							ThrowNxException("Could not create a new problem because there is no valid EMR");
						}

						//update the icon
						_variant_t varProblemIcon = g_cvarNull;
						if(pCharge->HasOnlyClosedProblems()) {

							HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
							//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_CLOSED);
							varProblemIcon = (long)hProblem;
						}
						else {
							HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
							//HICON hProblem = theApp.LoadIcon(IDI_EMR_PROBLEM_FLAG);
							varProblemIcon = (long)hProblem;
						}
						pRow->PutValue(clcProblemIcon, varProblemIcon);

						ShowHideProblemIconColumn_ChargeList();

						if(!m_pEMN->IsLockedAndSaved()) {

							pCharge->bChanged = TRUE;

							// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
							// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
							m_pEMN->SetCodesUnsaved();
						}
						// (c.haag 2009-06-01 13:17) - PLID 34312 - We need to flag the EMR as unsaved because it is now the "manager" of problems
						pEMR->SetUnsaved();

						// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
						GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);
					}
				}
				
			}

		} NxCatchAll("CEmrCodesDlg::OnCommand:ID_ADD_CHARGE_PROBLEM");
	}
	else if(wParam == ID_EDIT_CHARGE_PROBLEM) {

		EditChargeProblems();
	}
	else if (wParam == ID_LINK_CHARGE_PROBLEMS) {
		// (c.haag 2009-05-28 16:11) - PLID 34312 - Link with existing problems
		LinkChargeProblems();
	}
	else if(wParam == ID_REMOVE_ICD9) {
		
		try {

			//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
			NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_DiagCodeList->CurSel;
			if (pDiagRow != NULL) {

				// (j.jones 2014-12-23 13:45) - PLID 64491 - moved the ability to remove a diag code to its own function
				RemoveDiagCode(pDiagRow);
			}

		} NxCatchAll("CEmrCodesDlg::OnCommand:ID_REMOVE_ICD9");
	}
	else if (wParam == ID_ADD_DEFAULT_ICD9S) {
		try {

			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			//TES 2/27/2014 - PLID 61079 - Added ICD10s. We also need to load the code and description, because 
			// we no longer have a combo to look them up in.
			_RecordsetPtr rs = CreateParamRecordset("SELECT "
				"DefaultDiagID1, DiagCodes9_1.CodeNumber as ICD9Code1, DiagCodes9_1.CodeDesc AS ICD9CodeDesc1, "
				"DefaultICD10DiagID1, DiagCodes10_1.CodeNumber AS ICD10Code1, DiagCodes10_1.CodeDesc AS ICD10CodeDesc1, "
				"DefaultDiagID2, DiagCodes9_2.CodeNumber as ICD9Code2, DiagCodes9_2.CodeDesc AS ICD9CodeDesc2, "
				"DefaultICD10DiagID2, DiagCodes10_2.CodeNumber AS ICD10Code2, DiagCodes10_2.CodeDesc AS ICD10CodeDesc2, "
				"DefaultDiagID3, DiagCodes9_3.CodeNumber as ICD9Code3, DiagCodes9_3.CodeDesc AS ICD9CodeDesc3, "
				"DefaultICD10DiagID3, DiagCodes10_3.CodeNumber AS ICD10Code3, DiagCodes10_3.CodeDesc AS ICD10CodeDesc3, "
				"DefaultDiagID4, DiagCodes9_4.CodeNumber as ICD9Code4, DiagCodes9_4.CodeDesc AS ICD9CodeDesc4, "
				"DefaultICD10DiagID4, DiagCodes10_4.CodeNumber AS ICD10Code4, DiagCodes10_4.CodeDesc AS ICD10CodeDesc4 "
				"FROM PatientsT "
				"LEFT JOIN DiagCodes DiagCodes9_1 ON PatientsT.DefaultDiagID1 = DiagCodes9_1.ID "
				"LEFT JOIN DiagCodes DiagCodes10_1 ON PatientsT.DefaultICD10DiagID1 = DiagCodes10_1.ID "
				"LEFT JOIN DiagCodes DiagCodes9_2 ON PatientsT.DefaultDiagID2 = DiagCodes9_2.ID "
				"LEFT JOIN DiagCodes DiagCodes10_2 ON PatientsT.DefaultICD10DiagID2 = DiagCodes10_2.ID "
				"LEFT JOIN DiagCodes DiagCodes9_3 ON PatientsT.DefaultDiagID3 = DiagCodes9_3.ID "
				"LEFT JOIN DiagCodes DiagCodes10_3 ON PatientsT.DefaultICD10DiagID3 = DiagCodes10_3.ID "
				"LEFT JOIN DiagCodes DiagCodes9_4 ON PatientsT.DefaultDiagID4 = DiagCodes9_4.ID "
				"LEFT JOIN DiagCodes DiagCodes10_4 ON PatientsT.DefaultICD10DiagID4 = DiagCodes10_4.ID "
				"WHERE PersonID = {INT}",m_nPatientID);
			if(!rs->eof) {
				//add their default diagnosis codes
				long nDefaultDiagID1, nDefaultDiagID2, nDefaultDiagID3, nDefaultDiagID4;
				nDefaultDiagID1 = AdoFldLong(rs, "DefaultDiagID1",-1);
				nDefaultDiagID2 = AdoFldLong(rs, "DefaultDiagID2",-1);
				nDefaultDiagID3 = AdoFldLong(rs, "DefaultDiagID3",-1);
				nDefaultDiagID4 = AdoFldLong(rs, "DefaultDiagID4",-1);
				long nDefaultICD10ID1, nDefaultICD10ID2, nDefaultICD10ID3, nDefaultICD10ID4;
				nDefaultICD10ID1 = AdoFldLong(rs, "DefaultICD10DiagID1",-1);
				nDefaultICD10ID2 = AdoFldLong(rs, "DefaultICD10DiagID2",-1);
				nDefaultICD10ID3 = AdoFldLong(rs, "DefaultICD10DiagID3",-1);
				nDefaultICD10ID4 = AdoFldLong(rs, "DefaultICD10DiagID4",-1);
				if (nDefaultDiagID1 != -1 || nDefaultDiagID2 != -1 || nDefaultDiagID3 != -1 || nDefaultDiagID4 != -1 
					|| nDefaultICD10ID1 != -1 || nDefaultICD10ID2 != -1 || nDefaultICD10ID3 != -1 || nDefaultICD10ID4 != -1) {
					// Try to add them each in turn
					BOOL bAdded = FALSE;
					//TES 2/26/2014 - PLID 60807 - Added new utility function to search both IDs
					if(FindRowInDiagList(nDefaultDiagID1, nDefaultICD10ID1) == NULL) {
						//TES 2/26/2014 - PLID 61079 - Simulate search results to pass into our function
						CDiagSearchResults::CDiagCode diag9;
						diag9.m_nDiagCodesID = nDefaultDiagID1;
						diag9.m_strCode = AdoFldString(rs, "ICD9Code1","");
						diag9.m_strDescription = AdoFldString(rs, "ICD9CodeDesc1","");
						CDiagSearchResults::CDiagCode diag10;
						diag10.m_nDiagCodesID = nDefaultICD10ID1;
						diag10.m_strCode = AdoFldString(rs, "ICD10Code1","");
						diag10.m_strDescription = AdoFldString(rs, "ICD10CodeDesc1","");
						CDiagSearchResults results;
						results.m_ICD9 = diag9;
						results.m_ICD10 = diag10;
						if(results.m_ICD9.m_nDiagCodesID != -1 || results.m_ICD10.m_nDiagCodesID != -1) {
							//TES 3/28/2014 - PLID 61079 - Specify nexgemtDone
							AddDiagCodeBySearchResults_Raw(results, nexgemtDone);
							bAdded = TRUE;
						}
					}
					if(FindRowInDiagList(nDefaultDiagID2, nDefaultICD10ID2) == NULL) {
						//TES 2/26/2014 - PLID 61079 - Simulate search results to pass into our function
						CDiagSearchResults::CDiagCode diag9;
						diag9.m_nDiagCodesID = nDefaultDiagID2;
						diag9.m_strCode = AdoFldString(rs, "ICD9Code2","");
						diag9.m_strDescription = AdoFldString(rs, "ICD9CodeDesc2","");
						CDiagSearchResults::CDiagCode diag10;
						diag10.m_nDiagCodesID = nDefaultICD10ID2;
						diag10.m_strCode = AdoFldString(rs, "ICD10Code2","");
						diag10.m_strDescription = AdoFldString(rs, "ICD10CodeDesc2","");
						CDiagSearchResults results;
						results.m_ICD9 = diag9;
						results.m_ICD10 = diag10;
						if(results.m_ICD9.m_nDiagCodesID != -1 || results.m_ICD10.m_nDiagCodesID != -1) {
							//TES 3/28/2014 - PLID 61079 - Specify nexgemtDone
							AddDiagCodeBySearchResults_Raw(results, nexgemtDone);
							bAdded = TRUE;
						}
					}
					if(FindRowInDiagList(nDefaultDiagID3, nDefaultICD10ID3) == NULL) {
						//TES 2/26/2014 - PLID 61079 - Simulate search results to pass into our function
						CDiagSearchResults::CDiagCode diag9;
						diag9.m_nDiagCodesID = nDefaultDiagID3;
						diag9.m_strCode = AdoFldString(rs, "ICD9Code3","");
						diag9.m_strDescription = AdoFldString(rs, "ICD9CodeDesc3","");
						CDiagSearchResults::CDiagCode diag10;
						diag10.m_nDiagCodesID = nDefaultICD10ID3;
						diag10.m_strCode = AdoFldString(rs, "ICD10Code3","");
						diag10.m_strDescription = AdoFldString(rs, "ICD10CodeDesc3","");
						CDiagSearchResults results;
						results.m_ICD9 = diag9;
						results.m_ICD10 = diag10;
						if(results.m_ICD9.m_nDiagCodesID != -1 || results.m_ICD10.m_nDiagCodesID != -1) {
							//TES 3/28/2014 - PLID 61079 - Specify nexgemtDone
							AddDiagCodeBySearchResults_Raw(results, nexgemtDone);
							bAdded = TRUE;
						}
					}
					if(FindRowInDiagList(nDefaultDiagID4, nDefaultICD10ID4) == NULL) {
						//TES 2/26/2014 - PLID 61079 - Simulate search results to pass into our function
						CDiagSearchResults::CDiagCode diag9;
						diag9.m_nDiagCodesID = nDefaultDiagID4;
						diag9.m_strCode = AdoFldString(rs, "ICD9Code4","");
						diag9.m_strDescription = AdoFldString(rs, "ICD9CodeDesc4","");
						CDiagSearchResults::CDiagCode diag10;
						diag10.m_nDiagCodesID = nDefaultICD10ID4;
						diag10.m_strCode = AdoFldString(rs, "ICD10Code4","");
						diag10.m_strDescription = AdoFldString(rs, "ICD10CodeDesc4","");
						CDiagSearchResults results;
						results.m_ICD9 = diag9;
						results.m_ICD10 = diag10;
						if(results.m_ICD9.m_nDiagCodesID != -1 || results.m_ICD10.m_nDiagCodesID != -1) {
							//TES 3/28/2014 - PLID 61079 - Specify nexgemtDone
							AddDiagCodeBySearchResults_Raw(results, nexgemtDone);
							bAdded = TRUE;
						}
					}

					// See if we added any
					if (!bAdded) {
						MessageBox("All of this patient's default Diagnosis Codes are already selected on this EMR.", NULL, MB_ICONINFORMATION|MB_OK);
					}
				} else {
					MessageBox("This patient does not have any default Diagnosis Codes.", NULL, MB_ICONINFORMATION|MB_OK);
				}
			}
			rs->Close();
		} NxCatchAll("CEmrCodesDlg::OnCommand:ID_ADD_DEFAULT_ICD9S");
	}
	else if(wParam == ID_ADD_ICD9_PROBLEM) {

		try {

			if(m_DiagCodeList->CurSel != NULL) {

				//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;
				EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

				if(pDiag == NULL) {
					ThrowNxException("Add Diagnosis Problem - failed because no diagnosis object was found!");
				}

				if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
					return CNxDialog::OnCommand(wParam, lParam);
				}

				// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
				// this is independent of locking
				if(!m_pEMN->IsWritable()) {
					//prompt, but do not attempt to take access, let the user decide what to do
					AfxMessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
					return CNxDialog::OnCommand(wParam, lParam);
				}

				CEMRProblemEditDlg dlg(this);

				const BOOL bEMNIsLocked = (m_pEMN->GetStatus() == 2) ? TRUE : FALSE;
				if (bEMNIsLocked) {
					dlg.SetWriteToData(TRUE); // This will force the dialog to save the problem details to data
				}

				// (j.jones 2014-04-07 12:24) - PLID 60781 - if a problem is linked to a diagnosis, show the 9 and 10 code if both exist
				CString strDetailName = "";
				if(pDiag) {
					if(pDiag->nDiagCodeID != -1 && pDiag->nDiagCodeID_ICD10 != -1) {
						strDetailName = pDiag->strCode_ICD10 + " - " + pDiag->strCodeDesc_ICD10 + " (" + pDiag->strCode + " - " + pDiag->strCodeDesc + ")";
					}
					else if(pDiag->nDiagCodeID_ICD10 != -1) {
						strDetailName = pDiag->strCode_ICD10 + " - " + pDiag->strCodeDesc_ICD10;
					}
					else if(pDiag->nDiagCodeID != -1) {
						strDetailName = pDiag->strCode + " - " + pDiag->strCodeDesc;
					}
					else {
						//this should be impossible
						ASSERT(FALSE);
						strDetailName = "<Unknown Diagnosis Code>";
					}
				}

				// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
				// (j.jones 2009-05-21 16:42) - PLID 34250 - renamed this function
				// (j.jones 2014-04-07 12:23) - PLID 60781  - this needs to format 
				dlg.AddLinkedObjectInfo(-1, eprtEmrDiag, strDetailName, "", pDiag->nID);
				dlg.SetPatientID(m_pEMN->GetParentEMR()->GetPatientID());

				if (IDOK == dlg.DoModal()) {
					if (!dlg.ProblemWasDeleted()) {

						COleDateTime dtInvalid;
						dtInvalid.SetStatus(COleDateTime::invalid);

						// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
						// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
						// (c.haag 2009-05-16 14:21) - PLID 34312 - Create a new problem and problem link
						CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
						if (NULL != pEMR) {

							// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
							long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
							dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

							// (z.manning 2009-05-27 10:14) - PLID 34297 - Added patient ID
							// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
							// (s.tullis 2015-02-23 15:44) - PLID 64723
							// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
							CEmrProblem *pProblem = pEMR->AllocateEmrProblem(dlg.GetProblemID(), pEMR->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
								dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), TRUE, dlg.GetProblemCodeID(),dlg.GetProblemDoNotShowOnCCDA(),
								dlg.GetProblemDoNotShowOnProblemPrompt());
							CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtEmrDiag, pDiag->nID, -1);
							pNewLink->UpdatePointersWithDiagCode(m_pEMN, pDiag);
							pDiag->m_apEmrProblemLinks.Add(pNewLink);
							// (c.haag 2009-05-16 14:23) - PLID 34312 - Release the reference that pProblem has
							// to the problem. Only the detail and the EMR (which tracks all the problems it created)
							// should own a reference.
							pProblem->Release();
						} else {
							ThrowNxException("Could not create a new problem because there is no valid EMR");
						}

						//update the icon
						_variant_t varProblemIcon = g_cvarNull;
						if(pDiag->HasOnlyClosedProblems()) {
							HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
							varProblemIcon = (long)hProblem;
						}
						else {
							HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
							varProblemIcon = (long)hProblem;
						}
						pRow->PutValue(edclcProblemIcon, varProblemIcon);

						ShowHideProblemIconColumn_DiagList();

						if(!m_pEMN->IsLockedAndSaved()) {
							pDiag->bChanged = TRUE;

							// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
							// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
							m_pEMN->SetCodesUnsaved();
						}
						// (c.haag 2009-06-01 13:17) - PLID 34312 - We need to flag the EMR as unsaved because it is now the "manager" of problems
						pEMR->SetUnsaved();

						// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
						GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);
					}
				}
			}

		} NxCatchAll("CEmrCodesDlg::OnCommand:ID_ADD_ICD9_PROBLEM");
	}
	else if(wParam == ID_LINK_ICD9_PROBLEMS) {
		// (c.haag 2009-05-28 16:11) - PLID 34312 - Link with existing problems
		LinkDiagnosisProblems();
	}
	else if(wParam == ID_EDIT_ICD9_PROBLEM) {
		EditDiagnosisProblems();
	}
	else if (wParam == ID_RESEARCH_ICD10_CODE) {
		// (r.farnworth 2014-03-14 15:25) - PLID 61380 - Add a right click menu option in the diagnosis datalist so that you can trigger the < Find ICD-10 Code > functionality.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;
		if(pRow)
		{
			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

			// (r.farnworth 2014-03-17 17:28) - PLID 61380 - Clear the data that existed for this ICD-10 Code
			pDiag->nDiagCodeID_ICD10 = -1;
			pDiag->strCode_ICD10 = "";
			pDiag->strCodeDesc_ICD10 = "";

			pRow->PutValue(edclcICD10DiagCodeID, -1);
			pRow->PutValue(edclcICD10CodeNumber, "");

			UpdateLegacyCode(pRow, FALSE);
		}
	}

	return CNxDialog::OnCommand(wParam, lParam);
}


// (s.dhole 2014-02-17 15:28) - PLID  60742 Copy Code from More Info dialog
HBRUSH CEmrCodesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-04-02 09:14) - PLID 29497 - Deprecated, use base class
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (s.dhole 2014-02-17 15:28) - PLID  60742 Copy Code from More Info dialog
//DRT 1/12/2007 - PLID 24234 - Added this helper function to sync a single row with an existing
//	EMNCharge pointer.
//TODO:  There are several instances where we do the same work to sync a pRow, all of that should
//	take advantage of this feature.  See OnEmnChargeAdded, RefreshChargeList, etc.
void CEmrCodesDlg::SyncBillListRowWithEMNCharge(EMNCharge *pCharge, IRowSettingsPtr pRow)
{
	// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
	_variant_t varProblemIcon = g_cvarNull;			
	if(pCharge->HasProblems()) {
		if(pCharge->HasOnlyClosedProblems()) {
			HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0); 
			
			varProblemIcon = (long)hProblem;
		}
		else {
			HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0); 
			varProblemIcon = (long)hProblem;
		}
	}

	pRow->PutValue(clcID, pCharge->nID);
	pRow->PutValue(clcServiceID, pCharge->nServiceID);
	pRow->PutValue(clcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
	pRow->PutValue(clcCode, _bstr_t(pCharge->strCode));
	pRow->PutValue(clcSubcode, _bstr_t(pCharge->strSubCode));
	// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
	pRow->PutValue(clcCategory, pCharge->nCategoryID);
	pRow->PutValue(clcDescription, _bstr_t(pCharge->strDescription));
	pRow->PutValue(clcQuantity, pCharge->dblQuantity);
	pRow->PutValue(clcCost, _variant_t(pCharge->cyUnitCost));
	pRow->PutValue(clcMod1, _bstr_t(pCharge->strMod1));
	pRow->PutValue(clcMod2, _bstr_t(pCharge->strMod2));
	pRow->PutValue(clcMod3, _bstr_t(pCharge->strMod3));
	pRow->PutValue(clcMod4, _bstr_t(pCharge->strMod4));
	pRow->PutValue(clcEMNChargePtr, (long)pCharge);
	// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
	// so just use that, or <None> if blank	
	//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
	pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));

	// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
	ShowHideProblemIconColumn_ChargeList();
}

//DRT 1/12/2007 - PLID 24234 - An EMNCharge pointer has changed.  We need to update the 
//	interface to reference this change.
LRESULT CEmrCodesDlg::OnEmnChargeChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		//wParam = EMNCharge*
		EMNCharge *pCharge = (EMNCharge*)wParam;

		//Our pCharge object has changed.  Update the interface appropriately.
		IRowSettingsPtr pRow  = m_BillList->FindByColumn(clcEMNChargePtr, (long)pCharge, 0, FALSE);
		// (j.jones 2012-02-20 09:25) - PLID 47886 - removed an assert if not found, because
		// we might be hiding unbillable charges
		if(pRow) {
			//Need to update the interface with all the new details of this charge
			//IRowSettingsPtr pRow = m_BillList->GetRow(nRow);
			SyncBillListRowWithEMNCharge(pCharge, pRow);
		}

	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (s.dhole 2014-02-17 15:28) - PLID  60742 Copy Code from More Info dialog
LRESULT CEmrCodesDlg::OnEmnChargeAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
		BOOL bShowChargeCategory = FALSE;
		CDC* pWndDC = GetDC();
		BOOL bIsHighColor = ::GetDeviceCaps(pWndDC->m_hDC, BITSPIXEL) > 8;
		ReleaseDC(pWndDC);

		EMNCharge *pCharge = (EMNCharge*)wParam;

		// (a.walling 2008-01-30 10:58) - PLID 28741 - Check to see if the charge is already in the list
		// (j.jones 2012-03-01 12:52) - PLID 47886 - this should not search our memory objects, as this code
		// is just adding an existing memory object to the list, provided something has not already added
		// the same EMNCharge pointer
		IRowSettingsPtr pRow = m_BillList->FindByColumn(clcEMNChargePtr, (long)pCharge, 0, FALSE);
		if (!pRow) {

			// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
			_variant_t varProblemIcon = g_cvarNull;			
			if(pCharge->HasProblems()) {
				if(pCharge->HasOnlyClosedProblems()) {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0); 
					varProblemIcon = (long)hProblem;
				}
				else {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0); 
					varProblemIcon = (long)hProblem;
				}
			}

			// (j.jones 2012-02-17 10:39) - PLID 47886 - do not add to the list if the billable setting
			// says to hide unbillable charges
			if(pCharge->bBillable || !m_checkHideUnbillableCPTCodes.GetCheck()) {
				//add to the datalist
				IRowSettingsPtr pRow = m_BillList->GetNewRow();
				// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category
				SetCPTCategoryCombo(pRow, pCharge);
				// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
				if(pCharge->nCategoryCount > 1 && m_bIsTemplate == FALSE)
				{
					bShowChargeCategory=TRUE;
				}
				pRow->PutValue(clcID, pCharge->nID);
				pRow->PutValue(clcServiceID, pCharge->nServiceID);
				pRow->PutValue(clcCategory, pCharge->nCategoryID);
				pRow->PutValue(clcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
				pRow->PutValue(clcCode, _bstr_t(pCharge->strCode));
				pRow->PutValue(clcSubcode, _bstr_t(pCharge->strSubCode));
				pRow->PutValue(clcDescription, _bstr_t(pCharge->strDescription));
				pRow->PutValue(clcQuantity, pCharge->dblQuantity);
				pRow->PutValue(clcCost, _variant_t(pCharge->cyUnitCost));
				//DRT 1/10/2007 - PLID 24182 - Now that we support default modifiers, we
				//	need to fill the values when charges are added.  Quantity was already done.
				pRow->PutValue(clcMod1, _bstr_t(pCharge->strMod1));
				pRow->PutValue(clcMod2, _bstr_t(pCharge->strMod2));
				pRow->PutValue(clcMod3, _bstr_t(pCharge->strMod3));
				pRow->PutValue(clcMod4, _bstr_t(pCharge->strMod4));
				//DRT 1/11/2007 - PLID 24220 - Add a pointer to the charge object to the datalist
				pRow->PutValue(clcEMNChargePtr, (long)pCharge);

				//DRT 1/11/2007 - PLID 24177 - Fill the WhichCodes field
				//DRT 1/11/2007 - PLID 24178 - This method is only called for new charges being added, thus the strDiagList will
				//	always be empty.  We want to prompt the user now to do any linking, to save them time having to
				//	manually come to the MoreInfo topic to do it themselves later on.
				//This function will change the strDiagList for us
				//DRT 1/12/2007 - PLID 24178 - Moved the Prompt function to EMRUtils to be at a higher level.  The more info
				//	may not exist when codes are spawned.  It is now called from the EMRTreeWnd, and when we reach this point, 
				//	the strDiagList is already filled for us.
				// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
				// so just use that, or <None> if blank	
				//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
				pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));
				m_BillList->AddRowSorted(pRow,NULL);

				// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
				ShowHideProblemIconColumn_ChargeList();

				// (j.jones 2011-03-28 15:03) - PLID 42575 - if the charge is not billable,
				// color it gray
				if(!pCharge->bBillable) {
					if(bIsHighColor) {
						pRow->PutBackColor(EMR_GHOSTLY_16_BIT_GRAY);
					}
					else {
						pRow->PutBackColor(EMR_GHOSTLY_8_BIT_GRAY);
					}
				}
				// (j.jones 2008-06-04 16:23) - PLID 30255 - if the charge is from a quote,
				// color the row accordingly
				else if(pCharge->nQuoteChargeID != -1) {
					pRow->PutBackColor(EMR_SOME_CHARGES_ON_QUOTES);
				}
			}

		} else {
			// (a.walling 2008-01-30 10:58) - PLID 28741
			// this charge already exists on the list. This can occur due to a race condition with PrepareBillList that ends
			// up refreshing the charge list, including this newly spawned charge, before we get this message to add the charge.
			// If we add a duplicate, then we can have an access violation if the underlying charge is removed, since only
			// one datalist row will be removed.
		}
		// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
		if (bShowChargeCategory)
		{
			ForceShowColumn(clcCategory, 10, 85);
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (s.dhole 2014-02-17 15:28) - PLID  60742 Copy Code from More Info dialog
void CEmrCodesDlg::HandleProblemChange(CEmrProblem *pChangedProblem)
{
	//TES 10/30/2008 - PLID 31269 - A problem has changed, so refresh all our problem icons.
	
	//TES 10/30/2008 - PLID 31269 - First, diag codes.
	int i = 0;
	//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
	NXDATALIST2Lib::IRowSettingsPtr pDiagRow = m_DiagCodeList->GetFirstRow();
	while(pDiagRow) {
		_variant_t varProblemIcon = g_cvarNull;
		EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pDiagRow->GetValue(edclcEMNDiagCodePtr));
		if(pDiag->HasProblems()) {
			if(pDiag->HasOnlyClosedProblems()) {
				HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
				varProblemIcon = (long)hProblem;
			}
			else {
				HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
				varProblemIcon = (long)hProblem;
			}
		}
		pDiagRow->PutValue(edclcProblemIcon, varProblemIcon);

		pDiagRow = pDiagRow->GetNextRow();
	}

	ShowHideProblemIconColumn_DiagList();

	IRowSettingsPtr pRow = m_BillList->GetFirstRow(); 
	while(pRow ){
	//TES 10/30/2008 - PLID 31269 - Next, charges.
	//for(i = 0; i < m_BillList->GetRowCount(); i++) {
		_variant_t varProblemIcon = g_cvarNull;
		EMNCharge *pCharge= (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));
		if(pCharge->HasProblems()) {
			if(pCharge->HasOnlyClosedProblems()) {
				HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0); 
				varProblemIcon = (long)hProblem;
			}
			else {
				HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0); 
				varProblemIcon = (long)hProblem;
			}
		}
		pRow->PutValue(clcProblemIcon, varProblemIcon);
		pRow = pRow->GetNextRow(); 
	}

	ShowHideProblemIconColumn_ChargeList();


}

// (r.farnworth 2014-02-17 16:15) - PLID 60746 - Brought over from More Info
// (c.haag 2014-03-24) - PLID 60930 - Return the new row
NXDATALIST2Lib::IRowSettingsPtr CEmrCodesDlg::AddDiagCode(EMNDiagCode *pDiag)
{
	// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
	_variant_t varProblemIcon = g_cvarNull;			
	if(pDiag->HasProblems()) {
		if(pDiag->HasOnlyClosedProblems()) {
			HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
			varProblemIcon = (long)hProblem;
		}
		else {
			HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
			varProblemIcon = (long)hProblem;
		}
	}

	// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
	_variant_t varInfoButtonIcon = (long)m_hInfoButtonIcon;

	//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
	NXDATALIST2Lib::IRowSettingsPtr pListRow = m_DiagCodeList->GetNewRow();
	// (j.jones 2008-07-23 10:20) - PLID 30819 - changed the original nID to nDiagCodeID,
	// then added a new nID for the actual record ID
	pListRow->PutValue(edclcID, pDiag->nID);
	pListRow->PutValue(edclcICD9DiagCodeID, pDiag->nDiagCodeID);
	//TES 2/26/2014 - PLID 60807 - Add ICD-10 columns
	pListRow->PutValue(edclcICD10DiagCodeID, pDiag->nDiagCodeID_ICD10);
	pListRow->PutValue(edclcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
	pListRow->PutValue(edclcICD9CodeNumber, _bstr_t(pDiag->strCode));
	//TES 2/28/2014 - PLID 60807 - Don't show the link or infobutton if there's no ICD-9 code
	if(pDiag->nDiagCodeID == -1) {
		pListRow->PutCellLinkStyle(edclcICD9CodeNumber, dlLinkStyleFalse);
	}
	// (r.gonet 03/07/2014) - PLID 60756 - Make the ICD-10 code number field a non-hyperlink if we have
	// no ICD-10 code
	if(pDiag->nDiagCodeID_ICD10 == -1) {
		pListRow->PutCellLinkStyle(edclcICD10CodeNumber, dlLinkStyleFalse);
	}
	//TES 2/26/2014 - PLID 60807 - Add ICD-10 columns
	pListRow->PutValue(edclcICD10CodeNumber, _bstr_t(pDiag->strCode_ICD10));
	// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
	//TES 2/28/2014 - PLID 60807 - Don't show the link or infobutton if there's no ICD-9 code
	// (r.gonet 03/17/2014) - PLID 60756 - Actually, show the infobutton column because there is an ICD-10 and ICD-10 codes can now function with patient education.
	pListRow->PutValue(edclcInfoButton, varInfoButtonIcon);
	//TES 2/28/2014 - PLID 60807 - For consistency, show <No Matching Code> if the ID is null
	pListRow->PutValue(edclcICD9CodeDesc, pDiag->nDiagCodeID == -1 ? _bstr_t("<No Matching Code>"):_bstr_t(pDiag->strCodeDesc));
	// (r.farnworth 2014-04-01 08:48) - PLID 61608 - If the ICD-9 column in the diag list in the <Codes> topic of the EMN is empty, the cell should have a hyperlink allowing the user to choose from the list of managed ICD-9 codes.
	if(pDiag->nDiagCodeID == -1) {
		NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);

		pListRow->PutCellLinkStyle(edclcICD9CodeDesc, dlLinkStyleTrue);
		pListRow->PutRefCellFormatOverride(edclcICD9CodeDesc, pHyperLink);
	}
	// (b.savon 2014-03-06 08:22) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
	AddICD10DisplayColumn(pListRow, pDiag);
	pListRow->PutValue(edclcOrderIndex, pDiag->nOrderIndex);
	pListRow->PutValue(edclcEMNDiagCodePtr, (long)pDiag);	// (j.jones 2008-07-22 12:14) - PLID 30792
	// (c.haag 2014-03-17) - PLID 60929 - Set the QuickList star icon
	UpdateRowQuickListIcon(pListRow);
	m_DiagCodeList->AddRowSorted(pListRow, NULL);

	// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
	ShowHideProblemIconColumn_DiagList();

	// (c.haag 2007-02-21 17:07) - PLID 24150 - Make sure the colors are right
	EnsureDiagCodeColors();

	//TES 2/27/2014 - PLID 60807 - Resize based on the user's preferences
	UpdateDiagnosisListColumnSizes();

	return pListRow;
}

// (r.farnworth 2014-02-17 16:15) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::EnsureDiagCodeColors()
{
	// (c.haag 2007-02-21 17:07) - PLID 24150 - This function ensures that the first
	// four diagnosis codes are colored in black, and the rest will be in gray.
	// (j.jones 2009-03-20 09:49) - PLID 9729 - now we don't color the first 8 rows,
	// even though all codes will now be billed, only 8 can send on an ANSI claim
	try {
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow();
		int nRowCount = 0;
		while(pRow) {
			if (nRowCount < 8) {
				// This is one of the first eight rows; color in black
				pRow->ForeColor = RGB(0,0,0);
			} else {
				// This is the 9th row or higher; color in gray
				pRow->ForeColor = RGB(128,128,128);
			}
			pRow = pRow->GetNextRow();
			nRowCount++;
		}
	} NxCatchAll("Error in EnsureDiagCodeColors");
}

// (r.farnworth 2014-02-17 16:39) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::ShowHideProblemIconColumn_DiagList()
{
	try {

		//see if any row has an icon, and if so, show the column, otherwise hide it
		BOOL bHasIcon = FALSE;
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow();
		while(pRow && !bHasIcon) {
			long nIcon = VarLong(pRow->GetValue(edclcProblemIcon), 0);
			if(nIcon != 0) {
				bHasIcon = TRUE;
			}
			pRow = pRow->GetNextRow();
		}

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_DiagCodeList->GetColumn(edclcProblemIcon);

		if(bHasIcon) {
			//show the column
			pCol->PutStoredWidth(20);
		}
		else {
			//hide the column
			pCol->PutStoredWidth(0);
		}

	}NxCatchAll("Error in CEmrCodesDlg::ShowHideProblemIconColumn_DiagList");
}

// (r.farnworth 2014-02-17 17:43) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::UpdateDiagCodeArrows() {

	//if read only, then always disable the arrows
	if(m_bReadOnly) {
		GetDlgItem(IDC_BTN_MOVE_DIAG_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MOVE_DIAG_DOWN)->EnableWindow(FALSE);
		return;
	}

	//otherwise, check the selection status
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_DiagCodeList->CurSel;

	if (pCurSel == NULL) {
		GetDlgItem(IDC_BTN_MOVE_DIAG_UP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_MOVE_DIAG_DOWN)->EnableWindow(FALSE);
	} else {
		// Enable/Disable buttons according to what's possible based on the index
		if (pCurSel->GetPreviousRow() == NULL) {
			GetDlgItem(IDC_BTN_MOVE_DIAG_UP)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_BTN_MOVE_DIAG_UP)->EnableWindow(TRUE);
		}

		if (pCurSel->GetNextRow() == NULL) {
			GetDlgItem(IDC_BTN_MOVE_DIAG_DOWN)->EnableWindow(FALSE);
		} else {
			GetDlgItem(IDC_BTN_MOVE_DIAG_DOWN)->EnableWindow(TRUE);
		}
	}
}

// (r.farnworth 2014-02-17 17:58) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::EditDiagnosisProblems()
{
	try {

		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;
		if(pRow) {

			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

			if(pDiag == NULL) {
				ThrowNxException("View Diagnosis Problems - failed because no diagnosis object was found!");
			}

			// (c.haag 2009-05-26 10:23) - PLID 34312 - Use the new problem link structure
			if(pDiag->m_apEmrProblemLinks.GetSize() == 0) {
				AfxMessageBox("This diagnosis code has no problems.");
				return;
			}

			//close the current problem list, if there is one
			CMainFrame *pFrame = GetMainFrame();
			if(pFrame) {
				pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
			}

			//now open filtered on this diagnosis
			// (c.haag 2009-05-26 10:23) - PLID 34312 - Use the new problem link structure
			CEMRProblemListDlg dlg(this);
			dlg.SetDefaultFilter(m_pEMN->GetParentEMR()->GetPatientID(), eprtEmrDiag, pDiag->nID, pDiag->strCode);
			dlg.LoadFromProblemList(GetEmrTreeWnd(), &pDiag->m_apEmrProblemLinks);
			dlg.DoModal();

			//try to update the icon
			BOOL bChanged = FALSE;
			_variant_t varProblemIcon = g_cvarNull;
			if(pDiag->HasProblems()) {
				if(pDiag->HasOnlyClosedProblems()) {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
				else {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
			}
			else {
				bChanged = TRUE;
			}
			pRow->PutValue(edclcProblemIcon, varProblemIcon);

			ShowHideProblemIconColumn_DiagList();

			//also see if any problem changed, if so, mark the diagnosis as changed
			if(bChanged || pDiag->HasChangedProblems()) {

				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);

				if(!m_pEMN->IsLockedAndSaved()) {
					pDiag->bChanged = TRUE;
					// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
					// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
					m_pEMN->SetCodesUnsaved();
				}
			}
		}

	} NxCatchAll("Error in CEmrCodesDlg::EditDiagnosisProblems");
}

// (r.farnworth 2014-02-18 09:06) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::LinkDiagnosisProblems()
{
	try {
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;
		if(pRow)
		{
			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
			CArray<CEmrProblemLink*,CEmrProblemLink*> aNewProblemLinks;
			int i;

			if(pDiag == NULL) {
				ThrowNxException("View Diagnosis Problems - failed because no diagnosis object was found!");
			}

			for (i=0; i < pDiag->m_apEmrProblemLinks.GetSize(); i++) {
				pDiag->m_apEmrProblemLinks[i]->UpdatePointersWithDiagCode(m_pEMN, pDiag);
			}
			if (LinkProblems(pDiag->m_apEmrProblemLinks, eprtEmrDiag, pDiag->nID, aNewProblemLinks)) {
				for (i=0; i < aNewProblemLinks.GetSize(); i++) {
					aNewProblemLinks[i]->UpdatePointersWithDiagCode(m_pEMN, pDiag);
					pDiag->m_apEmrProblemLinks.Add(aNewProblemLinks[i]);
				}

				//try to update the icon
				_variant_t varProblemIcon = g_cvarNull;
				BOOL bChanged = FALSE;
				if(pDiag->HasProblems()) {
					if(pDiag->HasOnlyClosedProblems()) {
						HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
						varProblemIcon = (long)hProblem;
					}
					else {
						HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
						varProblemIcon = (long)hProblem;
					}
				}
				else {
					bChanged = TRUE;
				}
				pRow->PutValue(edclcProblemIcon, varProblemIcon);

				ShowHideProblemIconColumn_DiagList();

				//also see if any problem changed, if so, mark the diagnosis as changed
				// (r.gonet 2015-01-06 11:16) - PLID 64509 - Check if any links changed rather than the problems, which won't change if we are linking existing problems.
				if(bChanged || pDiag->HasChangedProblemLinks()) {

					// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
					GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);

					if(!m_pEMN->IsLockedAndSaved()) {
						pDiag->bChanged = TRUE;
						// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
						// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
						m_pEMN->SetCodesUnsaved();
					}
				}
			} // if (LinkProblems(pDiag->m_apEmrProblemLinks, eprtEMRDiagCode, pDiag->nID, aNewProblemLinks)) {
		} // if(m_DiagCodeList->GetCurSel() != -1)
	}
	NxCatchAll("Error in CEmrCodesDlg::LinkDiagnosisProblems");
}

// (r.farnworth 2014-02-18 09:08) - PLID 60746 - Brought over from MoreInfo
//TES 2/28/2014 - PLID 61080 - Added ICD-10 fields
void CEmrCodesDlg::RemoveDiagCodeFromWhichCodes(long nID, CString strCode, long nICD10ID, CString strICD10Code)
{
	//We must loop through every charge and remove this diagnosis code
	IRowSettingsPtr pRow = m_BillList->GetFirstRow();

	while(pRow) {

		EMNCharge *pCharge = (EMNCharge*)VarLong(pRow->GetValue(clcEMNChargePtr));

		// (j.jones 2009-01-02 09:40) - PLID 32601 - we have to handle both the ID list and the Code list
		CString strCurrentCodeList = pCharge->strDiagCodeList;

		//TES 2/28/2014 - PLID 61080 - Use the audit value that checks both codes
		CString strCodeForAudit = GenerateEMRDiagCodesTAuditValue(strCode, strICD10Code);

		long nFindCode = strCurrentCodeList.Find(strCodeForAudit);
		if(nFindCode > -1) {
			//This code exists, must remove

			//Get everything before the code we're removing
			CString strNewCodeList = strCurrentCodeList.Left(nFindCode);

			//If it's the last code, we can just quit
			if(nFindCode + strCodeForAudit.GetLength() + 2 > strCurrentCodeList.GetLength()) {
				//There is no delimiter afterward, so it's the end.  We must
				//	now trim the delim off the end of the new
				strNewCodeList = strNewCodeList.Left(strNewCodeList.GetLength() - 2);
			}
			else {
				//There are other codes after our removal
				strNewCodeList += strCurrentCodeList.Mid(nFindCode + strCodeForAudit.GetLength() + 2);
			}

			//Now that we have the new code update the EMNCharge*
			pCharge->strDiagCodeList = strNewCodeList;
		}

		int i=0;
		//TES 2/28/2014 - PLID 61080 - Renamed to aryDiagIndexes, pull the actual EMNDiagCode pointer and check both IDs
		for(i=pCharge->aryDiagIndexes.GetSize() - 1; i>=0; i--) {
			EMNDiagCode* pDiag = m_pEMN->GetDiagCode(pCharge->aryDiagIndexes.GetAt(i));
			if(pDiag->nDiagCodeID == nID && pDiag->nDiagCodeID_ICD10 == nICD10ID) {
				pCharge->aryDiagIndexes.RemoveAt(i);
			}
		}

		// (j.jones 2009-01-02 09:24) - PLID 32601 - the strDiagCodeList is already comma-delimited,
		// so just use that, or <None> if blank	
		//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
		pRow->PutValue(clcWhichcodes, _bstr_t(pCharge->aryDiagIndexes.IsEmpty() ? "<None>" : GetDiagCodeListForInterface(pCharge)));

		pRow = pRow->GetNextRow();
	}

	//TES 3/25/2009 - PLID 33262 - Do the same thing for medications, but prompt first, once we've
	// found one eligible for removal.
	bool bPrompted = false;

	// (r.farnworth 2014-02-18 14:11) - PLID 60746 - Need to call the EMN object here since we cannot reference the medication list directly
	long medCount = m_pEMN->GetMedicationCount();
	long medIndex = 0;

	while(medIndex < medCount) {
	
		EMNMedication *pMed = m_pEMN->GetMedicationPtr(medIndex);

		int i=0;
		for(i=pMed->aryDiagnoses.GetSize() - 1; i>=0; i--) {
			if((long)pMed->aryDiagnoses.GetAt(i).nID == nID) {
				if(!bPrompted) {
					if(IDYES != MsgBox(MB_YESNO, "This diagnosis code is also selected on one or more prescriptions.  Would you like to remove it from them?")) {
						return;
					}
					bPrompted = true;
				}
				pMed->aryDiagnoses.RemoveAt(i);
				// (a.walling 2009-04-22 13:38) - PLID 34044
				pMed->SetChanged_Deprecated();
				// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
				m_pEMN->SetCodesUnsaved();
			}
		}
		medIndex++;
	}
}

// (r.farnworth 2014-02-18 09:47) - PLID 60746 - Moved over from MoreInfo
void CEmrCodesDlg::OnEditServices()
{
	try {
		//create a menu

		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_EDIT_CPT_CODES, "Edit &Service/Diag Codes");

		if (g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_EDIT_INV_ITEMS, "Edit &Inventory Items");
		}

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_EDIT_CODES);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-02-18 10:30) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::OnEditCPT()
{
	try {
		if (!CheckCurrentUserPermissions(bioAdminBilling,sptRead))
			return;

		CCPTCodes cpt(this);
		cpt.m_IsModal = TRUE;
		CNxModalParentDlg dlg(this, &cpt, CString("Billing Codes"));
		dlg.DoModal();

		// Requery the diagnosis dropdown in all cases. I tried checking
		// m_checkDiagCodes.Changed() but it was not set even when I
		// added a diagnosis code. Since it's a select-only dropdown, we
		// don't need to recall the previous selection.
		//TES 2/26/2014 - PLID 60806 - Replaced combo with search box
		//m_DiagCodeCombo->Requery();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-02-18 10:28) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::OnBtnMoveDiagUp() 
{
	try {
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;
		if(pRow == NULL) {
			//neither of these should be possible
			ASSERT(FALSE);
			UpdateDiagCodeArrows();
			return;
		}

		//swap orders
		NXDATALIST2Lib::IRowSettingsPtr pMovedUpRow = pRow;
		NXDATALIST2Lib::IRowSettingsPtr pSwappedRow = pRow->GetPreviousRow();

		//swap in the datalist
		// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID
		//TES 2/26/2014 - PLID 60807 - Added ICD10
		long nMovedICD9ID = VarLong(pMovedUpRow->GetValue(edclcICD9DiagCodeID),-1);
		long nMovedICD10ID = VarLong(pMovedUpRow->GetValue(edclcICD10DiagCodeID),-1);
		long nMovedOrderIndex = VarLong(pMovedUpRow->GetValue(edclcOrderIndex));
		nMovedOrderIndex--;
		pMovedUpRow->PutValue(edclcOrderIndex, nMovedOrderIndex);

		//TES 2/26/2014 - PLID 60807 - Added ICD10
		long nSwappedICD9ID = VarLong(pSwappedRow->GetValue(edclcICD9DiagCodeID),-1);
		long nSwappedICD10ID = VarLong(pSwappedRow->GetValue(edclcICD10DiagCodeID),-1);
		long nSwappedOrderIndex = VarLong(pSwappedRow->GetValue(edclcOrderIndex));
		nSwappedOrderIndex++;
		pSwappedRow->PutValue(edclcOrderIndex, nSwappedOrderIndex);

		m_DiagCodeList->Sort();

		// (c.haag 2007-02-21 17:09) - PLID 24150 - Make sure the colors are right
		EnsureDiagCodeColors();

		//now swap in the EMN list
		for(int i=0;i<m_pEMN->GetDiagCodeCount(); i++) {

			EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			if(pDiag->nDiagCodeID == nMovedICD9ID && pDiag->nDiagCodeID_ICD10 == nMovedICD10ID) {
				pDiag->nOrderIndex = nMovedOrderIndex;
				//denote that the user moved this item, later used in auditing
				pDiag->bHasMoved = TRUE;
				pDiag->bChanged = TRUE;
			}
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			else if(pDiag->nDiagCodeID == nSwappedICD9ID && pDiag->nDiagCodeID_ICD10 == nSwappedICD10ID) {
				pDiag->nOrderIndex = nSwappedOrderIndex;
				pDiag->bChanged = TRUE;
			}
		}

		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
		m_pEMN->SetCodesUnsaved();

		UpdateDiagCodeArrows();

	}NxCatchAll("Error in CEmrCodesDlg::OnBtnMoveDiagUp");
}

// (r.farnworth 2014-02-18 10:28) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::OnBtnMoveDiagDown() 
{
	try {

		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;
		if(pRow == NULL) {
			//neither of these should be possible
			ASSERT(FALSE);
			UpdateDiagCodeArrows();
			return;
		}

		//swap orders
		NXDATALIST2Lib::IRowSettingsPtr pMovedDownRow = pRow;
		NXDATALIST2Lib::IRowSettingsPtr pSwappedRow = pRow->GetNextRow();

		//swap in the datalist
		// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID
		//TES 2/26/2014 - PLID 60807 - Added ICD10
		long nMovedICD9ID = VarLong(pMovedDownRow->GetValue(edclcICD9DiagCodeID),-1);
		long nMovedICD10ID = VarLong(pMovedDownRow->GetValue(edclcICD10DiagCodeID),-1);
		long nMovedOrderIndex = VarLong(pMovedDownRow->GetValue(edclcOrderIndex));
		nMovedOrderIndex++;
		pMovedDownRow->PutValue(edclcOrderIndex, nMovedOrderIndex);

		long nSwappedICD9ID = VarLong(pSwappedRow->GetValue(edclcICD9DiagCodeID),-1);
		long nSwappedICD10ID = VarLong(pSwappedRow->GetValue(edclcICD10DiagCodeID),-1);
		long nSwappedOrderIndex = VarLong(pSwappedRow->GetValue(edclcOrderIndex));
		nSwappedOrderIndex--;
		pSwappedRow->PutValue(edclcOrderIndex, nSwappedOrderIndex);

		m_DiagCodeList->Sort();

		// (c.haag 2007-02-21 17:09) - PLID 24150 - Make sure the colors are right
		EnsureDiagCodeColors();

		//now swap in the EMN list
		for(int i=0;i<m_pEMN->GetDiagCodeCount(); i++) {

			EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			if(pDiag->nDiagCodeID == nMovedICD9ID && pDiag->nDiagCodeID_ICD10 == nMovedICD10ID) {
				pDiag->nOrderIndex = nMovedOrderIndex;
				//denote that the user moved this item, later used in auditing
				pDiag->bHasMoved = TRUE;
				pDiag->bChanged = TRUE;
			}
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			else if(pDiag->nDiagCodeID == nSwappedICD9ID && pDiag->nDiagCodeID_ICD10 == nSwappedICD10ID) {
				pDiag->nOrderIndex = nSwappedOrderIndex;
				pDiag->bChanged = TRUE;
			}
		}

		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
		m_pEMN->SetCodesUnsaved();

		UpdateDiagCodeArrows();

	}NxCatchAll("Error in CEmrCodesDlg::OnBtnMoveDiagDown");
}

// (c.haag 2014-03-05) - PLID 60930 - Added the ability to add codes from the QuickList
void CEmrCodesDlg::OnBtnQuickList()
{
	try
	{
		CDiagQuickListDlg dlg;
		dlg.m_bIsEMRTemplate = m_bIsTemplate;

		// Open the dialog
		int result = dlg.DoModal();

		// (c.haag 2014-03-18) - PLID 60929 - The QuickList dialog can change data, regardless of whether
		// the user clicks Add Codes To (visit/template) or Close. Update the existing QuickList states for all
		// existing diagnosis codes.
		UpdateAllQuickListIcons();

		// (c.haag 2014-03-05) - PLID 60930 - Now handle adding new codes from the QuickList
		if (IDOK == result)
		{
			int nCodes = dlg.m_aSelectedQuickListItems.GetCount();
			if (nCodes > 0)
			{
				// This is set to true after a new code is added
				BOOL bNewCodesAdded = FALSE;

				// Warn if this is a template
				if(m_bIsTemplate) 
				{
					if (IDNO == MessageBox("Adding diagnosis codes to a template will default to using the codes on every patient EMN generated from this template.\n\n"
						"This is not recommended. Are you sure you wish to add the selected diagnosis codes from your QuickList?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) 
					{
						return;
					}
				}

				// Do for all codes
				CStringArray astrDuplicateCodes;
				for (int i=0; i < nCodes; i++)
				{
					CDiagSearchResults results;

					NexTech_Accessor::_DiagQuickListItemPtr pItem(dlg.m_aSelectedQuickListItems[i]);
					results.m_ICD9.m_nDiagCodesID = (NULL != pItem->ICD9) ? atol( VarString(pItem->ICD9->ID) ) : -1;
					results.m_ICD9.m_strCode = (NULL != pItem->ICD9) ? VarString(pItem->ICD9->Code) : "";
					results.m_ICD9.m_strDescription = (NULL != pItem->ICD9) ? VarString(pItem->ICD9->description) : "";
					results.m_ICD10.m_nDiagCodesID = (NULL != pItem->ICD10) ? atol( VarString(pItem->ICD10->ID) ) : -1;
					results.m_ICD10.m_strCode = (NULL != pItem->ICD10) ? VarString(pItem->ICD10->Code) : "";
					results.m_ICD10.m_strDescription = (NULL != pItem->ICD10) ? VarString(pItem->ICD10->description) : "";
					results.m_bIsQuicklist = true;

					// Check for duplicates
					if (FindRowInDiagList(results.m_ICD9.m_nDiagCodesID, results.m_ICD10.m_nDiagCodesID) != NULL) 
					{
						if (results.m_ICD9.m_nDiagCodesID != -1 && results.m_ICD10.m_nDiagCodesID != -1) 
						{
							astrDuplicateCodes.Add(FormatString("ICD-10: %s  ICD-9: %s", results.m_ICD10.m_strCode, results.m_ICD9.m_strCode));
						}
						else if (results.m_ICD9.m_nDiagCodesID != -1)
						{
							astrDuplicateCodes.Add("ICD-9: " + results.m_ICD9.m_strCode);
						}
						else if (results.m_ICD10.m_nDiagCodesID != -1)
						{
							astrDuplicateCodes.Add("ICD-10: " + results.m_ICD10.m_strCode);
						}
					}
					else
					{
						// Add the code. We have to calculate the match type here.
						NexGEMMatchType matchType = nexgemtDone;
						if(results.m_ICD10.m_strCode.IsEmpty() && DiagSearchUtils::GetPreferenceSearchStyle() != eManagedICD9_Search)
						{
							// No match type because there's no 10 code and we're not in 9-mode.
							matchType = nexgemtNoMatch;
						}
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = AddDiagCodeBySearchResults_Raw(results, matchType);

						// If there's no match, then we need to assign one.
						if (matchType == nexgemtNoMatch)
						{
							EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pNewRow->GetValue(edclcEMNDiagCodePtr));
							// Use this existing function to search for a matching code
							UpdateLegacyCode(pNewRow, TRUE);
							// If there is still no match, then set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
							// If there was a match, AddICD10DisplayColumn should already have been called by now.
							if (pDiag->MatchType == nexgemtNoMatch)
							{
								AddICD10DisplayColumn(pNewRow, pDiag);
							}
						}
						bNewCodesAdded = TRUE;
					}
				} // for (int i=0; i < nCodes; i++)

				// Notify the user of duplicate codes
				if (astrDuplicateCodes.GetCount() > 0)
				{
					CString strMsg = "The following diagnosis codes have already been selected:\n\n";
					for (int i=0; i < astrDuplicateCodes.GetCount(); i++)
					{
						strMsg += astrDuplicateCodes[i] + "\n";
						if (19 == i && astrDuplicateCodes.GetCount() > 20)
						{
							strMsg += "<more omitted>";
							break;
						}
					}
					AfxMessageBox(strMsg);
				}

				// Flag the fact the codes are unsaved
				if (bNewCodesAdded)
				{
					m_pEMN->SetCodesUnsaved();
				}

				// This function will check their preference to save the EMN and warn about drug interactions
				CEmrTreeWnd *pTree = GetEmrTreeWnd();
				if(!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
					// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
					m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
				}

			} // if (nCodes > 0)
		} // if (IDOK == dlg.DoModal())
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2014-04-09) - PLID 60929 - Update the QuickList icon for every row.
void CEmrCodesDlg::UpdateAllQuickListIcons()
{
		// First, go through every row in the codes list, gather diagnosis ID's and reset their QuickList ID's
		Nx::SafeArray<IUnknown *> saCommits;
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
		{
			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
			// Add this diagnosis to our crosswalk list
			NexTech_Accessor::_DiagnosisCrosswalkCommitPtr pCommit(__uuidof(NexTech_Accessor::DiagnosisCrosswalkCommit));
			NexTech_Accessor::_DiagnosisCodeCommitPtr pICD9(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
			NexTech_Accessor::_DiagnosisCodeCommitPtr pICD10(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
			if (pDiag->nDiagCodeID != -1) pICD9->ID = _bstr_t(AsString(pDiag->nDiagCodeID));
			if (pDiag->nDiagCodeID_ICD10 != -1) pICD10->ID = _bstr_t(AsString(pDiag->nDiagCodeID_ICD10));
			pCommit->ICD9 = pICD9;
			pCommit->ICD10 = pICD10;
			saCommits.Add(pCommit);
			// Reset the QuickList ID. If this diagnosis is still in the QuickList, we'll re-assign it in the next code block.
			pDiag->nQuickListID = -1;
			UpdateRowQuickListIcon(pRow);
		}

		// Now query the API to get the subset of rows that are still in the QuickList
		NexTech_Accessor::_DiagnosisCrosswalksPtr pFoundCrosswalks = GetAPI()->GetCrosswalksInQuickListByDisplayPreference(GetAPISubkey(), GetAPILoginToken(), saCommits);

		// Now for each API result, go through the list looking for matches and assign those rows
		// QuickList ID's as we find them.
		Nx::SafeArray<IUnknown *> saFoundCrosswalks(pFoundCrosswalks->Items);
		foreach(NexTech_Accessor::_DiagnosisCrosswalkPtr pCrosswalk, saFoundCrosswalks)
		{
			NexTech_Accessor::_DiagnosisCodePtr pICD9(pCrosswalk->ICD9);
			NexTech_Accessor::_DiagnosisCodePtr pICD10(pCrosswalk->ICD10);
			int nID9 = (NULL != pCrosswalk->ICD9) ? atol( VarString(pCrosswalk->ICD9->ID) ) : -1;
			int nID10 = (NULL != pCrosswalk->ICD10) ? atol( VarString(pCrosswalk->ICD10->ID) ) : -1;
			for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
			{
				EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
				BOOL bMatches = FALSE;
				switch (DiagQuickListUtils::GetAPIDiagDisplayType())
				{
				case NexTech_Accessor::DiagDisplayType_Crosswalk:
					bMatches = pDiag->nDiagCodeID == nID9 && pDiag->nDiagCodeID_ICD10 == nID10;
					break;
				case NexTech_Accessor::DiagDisplayType_ICD10:
					bMatches = pDiag->nDiagCodeID_ICD10 == nID10;
					break;
				default:
					bMatches = pDiag->nDiagCodeID == nID9;
					break;
				}

				if (bMatches)
				{
					pDiag->nQuickListID = atol( VarString(pCrosswalk->DiagQuickListID) );
					UpdateRowQuickListIcon(pRow);
				}

			} // For all diag list rows
		} // For all API results
}

// (r.farnworth 2014-02-18 14:11) - PLID 60746 - Brought over from MoreInfo
LRESULT CEmrCodesDlg::OnEmnDiagAdded(WPARAM wParam, LPARAM lParam)
{
	try {
		EMNDiagCode *pDiag = (EMNDiagCode*)wParam;
		//TES 2/26/2014 - PLID 60806 - We don't search in the combo any more (since it doesn't exist)

		// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
		_variant_t varProblemIcon = g_cvarNull;			
		if(pDiag->HasProblems()) {
			if(pDiag->HasOnlyClosedProblems()) {
				HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
				varProblemIcon = (long)hProblem;
			}
			else {
				HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
				varProblemIcon = (long)hProblem;
			}
		}

		// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
		_variant_t varInfoButtonIcon = (long)m_hInfoButtonIcon;

		NXDATALIST2Lib::IRowSettingsPtr pListRow = m_DiagCodeList->GetNewRow();
		// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID and dclcID
		pListRow->PutValue(edclcID, pDiag->nID);
		pListRow->PutValue(edclcICD9DiagCodeID, pDiag->nDiagCodeID);
		//TES 2/26/2014 - PLID 60807 - Added ICD10
		pListRow->PutValue(edclcICD10DiagCodeID, pDiag->nDiagCodeID_ICD10);
		pListRow->PutValue(edclcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
		pListRow->PutValue(edclcICD9CodeNumber, _bstr_t(pDiag->strCode));
		//TES 2/28/2014 - PLID 60807 - Don't show the link or infobutton if there's no ICD-9 code
		if(pDiag->strCode.IsEmpty()) {
			pListRow->PutCellLinkStyle(edclcICD9CodeNumber, dlLinkStyleFalse);
		}
		// (r.gonet 03/07/2014) - PLID 60756 - Make the ICD-10 code number field a non-hyperlink if we have
		// no ICD-10 code
		if(pDiag->strCode_ICD10.IsEmpty()) {
			pListRow->PutCellLinkStyle(edclcICD10CodeNumber, dlLinkStyleFalse);
		}
		//TES 2/26/2014 - PLID 60807 - Added ICD10
		pListRow->PutValue(edclcICD10CodeNumber, _bstr_t(pDiag->strCode_ICD10));
		// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
		//TES 2/28/2014 - PLID 60807 - Don't show the link or infobutton if there's no ICD-9 code
		// (r.gonet 03/17/2014) - PLID 60756 - Actually, show the infobutton column because there is an ICD-10 and ICD-10 codes can now function with patient education.
		pListRow->PutValue(edclcInfoButton, varInfoButtonIcon);
		//TES 2/28/2014 - PLID 60807 - For consistency, show <No Matching Code> if the ID is null
		pListRow->PutValue(edclcICD9CodeDesc, pDiag->nDiagCodeID == -1 ? _bstr_t("<No Matching Code>"):_bstr_t(pDiag->strCodeDesc));
		// (r.farnworth 2014-04-01 08:48) - PLID 61608 - If the ICD-9 column in the diag list in the <Codes> topic of the EMN is empty, the cell should have a hyperlink allowing the user to choose from the list of managed ICD-9 codes.
		if(pDiag->nDiagCodeID == -1) {
			NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
			pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);

			pListRow->PutCellLinkStyle(edclcICD9CodeDesc, dlLinkStyleTrue);
			pListRow->PutRefCellFormatOverride(edclcICD9CodeDesc, pHyperLink);
		}
		// (b.savon 2014-03-07 07:24) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
		AddICD10DisplayColumn(pListRow, pDiag);
		pListRow->PutValue(edclcOrderIndex, (long)(m_DiagCodeList->GetRowCount() + 1));
		pListRow->PutValue(edclcEMNDiagCodePtr, (long)pDiag);	// (j.jones 2008-07-22 12:14) - PLID 30792
		// (c.haag 2014-03-17) - PLID 60929 - Set the QuickList star icon
		UpdateRowQuickListIcon(pListRow);
		m_DiagCodeList->AddRowSorted(pListRow, NULL);

		// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
		ShowHideProblemIconColumn_DiagList();

		// (c.haag 2007-02-21 17:09) - PLID 24150 - Make sure the colors are right
		EnsureDiagCodeColors();

		//TES 2/27/2014 - PLID 60807 - Resize based on the user's preferences
		UpdateDiagnosisListColumnSizes();

	} NxCatchAll("Error in OnEmnDiagAdded");

	return 0;
}

// (r.farnworth 2014-02-18 10:52) - PLID 60746 - Brought over from MoreInfo
void CEmrCodesDlg::RefreshDiagCodeList()
{
	try {
		m_DiagCodeList->Clear();

		// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
		_variant_t varInfoButtonIcon = (long)m_hInfoButtonIcon;

		for(int i=0;i<m_pEMN->GetDiagCodeCount(); i++) {

			EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
			
			// (j.jones 2008-07-22 11:04) - PLID 30792 - determine the problem flag icon
			_variant_t varProblemIcon = g_cvarNull;			
			if(pDiag->HasProblems()) {
				if(pDiag->HasOnlyClosedProblems()) {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
				else {
					HICON hProblem = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
					varProblemIcon = (long)hProblem;
				}
			}

			// (a.walling 2007-09-04 12:50) - PLID 23714 - What was I thinking? this is already in the EMN!
			// this was copied from Initialize() verbatim, which did not need this CreateRecordset at all to
			// begin with!
			//_RecordsetPtr rs = CreateRecordset("SELECT CodeNumber, CodeDesc FROM DiagCodes WHERE ID = %li", DiagID);
			//if(!rs->eof) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetNewRow();
			// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID and dclcID
			pRow->PutValue(edclcID, (long)pDiag->nID);
			pRow->PutValue(edclcICD9DiagCodeID, (long)pDiag->nDiagCodeID);
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			pRow->PutValue(edclcICD10DiagCodeID, (long)pDiag->nDiagCodeID_ICD10);
			pRow->PutValue(edclcProblemIcon, varProblemIcon);	// (j.jones 2008-07-21 14:02) - PLID 30792
			pRow->PutValue(edclcICD9CodeNumber, (LPCTSTR)pDiag->strCode);
			//TES 2/28/2014 - PLID 60807 - Don't show the link or infobutton if there's no ICD-9 code
			if(pDiag->strCode.IsEmpty()) {
				pRow->PutCellLinkStyle(edclcICD9CodeNumber, dlLinkStyleFalse);
			}
			// (r.gonet 03/07/2014) - PLID 60756 - Make the ICD-10 code number field a non-hyperlink if we have
			// no ICD-10 code
			if(pDiag->strCode_ICD10.IsEmpty()) {
				pRow->PutCellLinkStyle(edclcICD10CodeNumber, dlLinkStyleFalse);
			}
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			pRow->PutValue(edclcICD10CodeNumber, (LPCTSTR)pDiag->strCode_ICD10);
			// (j.jones 2013-10-17 12:05) - PLID 58981 - added infobutton column
			//TES 2/28/2014 - PLID 60807 - Don't show the link or infobutton if there's no ICD-9 code
			// (r.gonet 03/17/2014) - PLID 60756 - Actually, show the infobutton column because there is an ICD-10 and ICD-10 codes can now function with patient education.
			pRow->PutValue(edclcInfoButton, varInfoButtonIcon);
			//TES 2/28/2014 - PLID 60807 - For consistency, show <No Matching Code> if the ID is null
			pRow->PutValue(edclcICD9CodeDesc, pDiag->nDiagCodeID == -1 ? (LPCTSTR)"<No Matching Code>":(LPCTSTR)pDiag->strCodeDesc);
			// (r.farnworth 2014-04-01 08:48) - PLID 61608 - If the ICD-9 column in the diag list in the <Codes> topic of the EMN is empty, the cell should have a hyperlink allowing the user to choose from the list of managed ICD-9 codes.
			if(pDiag->nDiagCodeID == -1) {
				NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
				pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);

				pRow->PutCellLinkStyle(edclcICD9CodeDesc, dlLinkStyleTrue);
				pRow->PutRefCellFormatOverride(edclcICD9CodeDesc, pHyperLink);
			}
			// (b.savon 2014-03-06 08:22) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
			AddICD10DisplayColumn(pRow, pDiag);
			pRow->PutValue(edclcOrderIndex, pDiag->nOrderIndex);
			pRow->PutValue(edclcEMNDiagCodePtr, (long)pDiag);	// (j.jones 2008-07-22 12:14) - PLID 30792
			// (c.haag 2014-03-17) - PLID 60929 - Set the QuickList star icon
			UpdateRowQuickListIcon(pRow);
			m_DiagCodeList->AddRowSorted(pRow, NULL);
			//}
			//rs->Close();
		}

		// (j.jones 2008-07-22 09:26) - PLID 30792 - try to show the problem icon column
		ShowHideProblemIconColumn_DiagList();

		// (c.haag 2007-02-21 17:09) - PLID 24150 - Make sure the colors are right
		EnsureDiagCodeColors();

		//TES 2/27/2014 - PLID 60807 - Resize based on the user's preferences
		UpdateDiagnosisListColumnSizes();

	} NxCatchAll("Error refreshing diag code list");
}

// (r.farnworth 2014-02-18 11:35) - PLID 60746 - Brought over from MoreInfo
//TES 2/26/2014 - PLID 60807 - Added ICD10
void CEmrCodesDlg::RemoveDiagCode(long nICD9DiagCodeID, long nICD10DiagCodeID)
{
	try {
		//Remove from the screen.
		// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID
		//TES 2/26/2014 - PLID 60807 - Utility function to search by both IDs
		NXDATALIST2Lib::IRowSettingsPtr pRow = FindRowInDiagList(nICD9DiagCodeID, nICD10DiagCodeID);

		//DRT 1/11/2007 - PLID 24177 - Need to remove this code from any associated CPT codes
		// (c.haag 2008-07-31 12:43) - PLID 30905 - Do nothing if the index is invalid
		if(pRow) {
			CString strCode = VarString(pRow->GetValue(edclcICD9CodeNumber), "");
			// (j.jones 2009-01-02 09:48) - PLID 32601 - also need to pass in the DiagCodeID
			long nID = VarLong(pRow->GetValue(edclcICD9DiagCodeID),-1);
			//TES 2/26/2014 - PLID 60807 - Added ICD10, though the WhichCodes haven't been updated to use it yet
			CString strICD10Code = VarString(pRow->GetValue(edclcICD10CodeNumber), "");
			long nICD10ID = VarLong(pRow->GetValue(edclcICD10DiagCodeID),-1);
			//TES 2/28/2014 - PLID 61080 - This now supports ICD-10 codes
			RemoveDiagCodeFromWhichCodes(nID, strCode, nICD10ID, strICD10Code);

			//Now remove from interface
			//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			m_DiagCodeList->RemoveRow(pRow);

			// (j.jones 2008-07-22 09:26) - PLID 30792 - try to hide the problem icon column
			ShowHideProblemIconColumn_DiagList();

			// (j.jones 2007-01-05 11:52) - PLID 24070 - update remaining order indices
			//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
			while(pNextRow) {
				long nOrderIndex = VarLong(pNextRow->GetValue(edclcOrderIndex),-1);
				nOrderIndex--;
				pNextRow->PutValue(edclcOrderIndex,(long)nOrderIndex);
				pNextRow = pNextRow->GetNextRow();
			}

			// (c.haag 2007-02-21 17:07) - PLID 24150 - Make sure the colors are right
			EnsureDiagCodeColors();

			//TES 2/27/2014 - PLID 60807 - Resize based on the user's preferences
			UpdateDiagnosisListColumnSizes();
		}

	} NxCatchAll("Error in RemoveDiagCode");
}

//TES 2/20/2014 - PLID 60748 - For functionality moved from CEMNMoreInfoDlg::OnMoreInfoChanged()
void CEmrCodesDlg::OnCodesChanged()
{
	// (r.farnworth 2014-02-20 11:55) - PLID 60745 - Moved from the OnMoreInfoChanged
	TrySetVisitType(m_pEMN->GetVisitTypeID(), m_pEMN->GetVisitTypeName());
}

// (b.savon 2014-02-26 10:19) - PLID 60805 - UPDATE - Add new button, "Existing Quote" in the procedure codes section of the <Codes> topic
void CEmrCodesDlg::OnBnClickedExistingQuote()
{
	try{
		//Following Adding a quote; this shouldn't be on a template
		if (m_bIsTemplate) {
			return;
		}

		// (j.gruber 2009-12-24 10:05) - PLID 15329 - check to see if this emn has already been billed
		if (!WarnChargeAlreadyBilled()) {
			return;
		}

		// (b.savon 2014-02-26 10:24) - PLID 60805 - UPDATE - Add new button, "Existing Quote" in the procedure codes section of the <Codes> topic
		CEMRSelectServiceDlg dlg(this, drsQuote);
		// (j.dinatale 2012-01-11 12:25) - PLID 47464 - show the resp list on the Select Service dlg is this is not a template
		// (j.jones 2008-06-03 10:28) - PLID 30062 - send a patient ID if not a template
		if(!m_bIsTemplate && m_nPatientID != -1) {
			dlg.m_nPatientID = m_nPatientID;
			dlg.m_bShowRespList = TRUE;
			// (j.jones 2012-08-22 09:23) - PLID 50486 - set the default insured party ID
			dlg.m_nAssignedInsuredPartyID = m_pEMN->GetParentEMR()->GetDefaultChargeInsuredPartyID();
		}
		if(dlg.DoModal() == IDOK) {

			//Update the interface
			if(dlg.m_nQuoteID != -1) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				AddQuoteToBillList(dlg.m_nQuoteID);
			}
			else if(dlg.m_ServiceID != -1) {
				// (j.dinatale 2012-01-05 10:12) - PLID 47464 - added Insured Party ID
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
				AddChargeToBillList(dlg.m_ServiceID, dlg.m_strCode, dlg.m_strSubCode, dlg.m_strDescription, dlg.m_nAssignedInsuredPartyID, dlg.m_bBillable, dlg.m_cyPrice,dlg.m_nCategory,dlg.m_nCategoryCount);
			}
			else {
				//nothing was selected, which should be impossible,
				//so assert and silently return
				ASSERT(FALSE);
				return;
			}
		}	

	}NxCatchAll(__FUNCTION__);
}

//TES 2/26/2014 - PLID 60806 - Replaced dropdown with search box
void CEmrCodesDlg::OnSelChosenEmnDiagSearch(LPDISPATCH lpRow)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		//TES 2/26/2014 - PLID 60807 - Call the utility function
		CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);

		//TES 2/26/2014 - PLID 61079 - Broke the rest of the code out into its own function
		AddDiagCodeBySearchResults(results);
		

	}NxCatchAll(__FUNCTION__);
}

//TES 2/27/2014 - PLID 61079 - Broke this out into its own function
bool CEmrCodesDlg::AddDiagCodeBySearchResults(CDiagSearchResults &results)
{
	if(results.m_ICD9.m_nDiagCodesID == -1 && results.m_ICD10.m_nDiagCodesID == -1) {
		return false;
	}

	// (j.jones 2007-08-13 09:46) - PLID 27050 - warn if they try to add default diags to the template
	if(m_bIsTemplate) {
		if(IDNO == MessageBox("Adding a diagnosis code to a template will default to using the code on every patient EMN generated from this template.\n"
			"This is not recommended. Are you sure you wish to add this default diagnosis?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
			// (j.jones 2009-12-21 10:50) - PLID 17521 - clear the selection
			m_DiagSearch->CurSel = NULL;
			return false;
		}
	}

	//Update the interface

	// Make sure it's not already in the list
	//TES 2/26/2014 - PLID 60807 - Utility function to search both IDs
	if (FindRowInDiagList(results.m_ICD9.m_nDiagCodesID, results.m_ICD10.m_nDiagCodesID) != NULL) {
		MessageBox("This diagnosis code has already been selected.", NULL, MB_OK|MB_ICONEXCLAMATION);
		// (j.jones 2009-12-21 10:50) - PLID 17521 - clear the selection
		m_DiagSearch->CurSel = NULL;
		return false;
	}
	else {
		//it was not a duplicate

		//update the memory object.
		// (r.farnworth 2014-03-06 14:24) - PLID 60820 - Need to populate the MatchResult flag here as well
		// (r.farnworth 2014-03-17 10:18) - PLID 60820 - Should not default to NoMatch when in ICD-9 Mode
		// (c.haag 2014-03-06) - PLID 60930 - Refactored r.farnworth's code
		NexGEMMatchType matchType = nexgemtDone;
		if(results.m_ICD10.m_strCode.IsEmpty() && DiagSearchUtils::GetPreferenceSearchStyle() != eManagedICD9_Search)
		{
			matchType = nexgemtNoMatch;
		}

		// (c.haag 2014-03-06) - PLID 60930 - Use this utility function
		AddDiagCodeBySearchResults_Raw(results, matchType);

		// (j.jones 2012-10-01 15:44) - PLID 52869 - this function will check their preference
		// to save the EMN and warn about drug interactions
		CEmrTreeWnd *pTree = GetEmrTreeWnd();
		if(!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
			// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
			m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
		}
	}

	// (j.jones 2009-12-21 10:50) - PLID 17521 - clear the selection
	m_DiagSearch->CurSel = NULL;

	return true;
}

// (c.haag 2014-03-06) - PLID 60930 - Adds a CDiagSearchResults object to the EMN diagnosis codes list
NXDATALIST2Lib::IRowSettingsPtr CEmrCodesDlg::AddDiagCodeBySearchResults_Raw(CDiagSearchResults &results, NexGEMMatchType matchType)
{
	//update the memory object.
	EMNDiagCode *pDiag = new EMNDiagCode;
	// (j.jones 2008-07-23 10:20) - PLID 30819 - changed the original nID to nDiagCodeID,
	// then added a new nID for the actual record ID
	//TES 2/26/2014 - PLID 60807 - added ICD10 fields
	pDiag->nID = -1;
	pDiag->nDiagCodeID = results.m_ICD9.m_nDiagCodesID;
	pDiag->nDiagCodeID_ICD10 = results.m_ICD10.m_nDiagCodesID;
	pDiag->bIsNew = TRUE;
	pDiag->strCode = results.m_ICD9.m_strCode;
	pDiag->strCodeDesc = results.m_ICD9.m_strDescription;
	pDiag->strCode_ICD10 = results.m_ICD10.m_strCode;
	pDiag->strCodeDesc_ICD10 = results.m_ICD10.m_strDescription;
	pDiag->MatchType = matchType;

	// (c.haag 2014-03-17) - PLID 60929 - We have to calculate the QuickList ID. We can't
	// pull it from results because they also count shared users; and we're only interested in
	// the QuickList ID in -our- QuickList.
	UpdateDiagQuickListID(pDiag);

	// (j.jones 2007-01-05 10:33) - PLID 24070 - generate the next order index
	// (c.haag 2014-03-06) - PLID 60930 - Moved here
	pDiag->nOrderIndex = m_DiagCodeList->GetRowCount() + 1;
	pDiag->bHasMoved = FALSE;
	pDiag->bChanged = FALSE;
	// (j.jones 2014-12-23 15:07) - PLID 64491 - added bReplaced
	pDiag->bReplaced = FALSE;
	m_pEMN->AddDiagCode(pDiag);

	// (j.jones 2008-07-22 12:19) - PLID 30792 - now add to the list
	// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
	return AddDiagCode(pDiag);
}

// (c.haag 2014-03-17) - PLID 60929 - Updates our QuickList ID for a given diagnosis code
void CEmrCodesDlg::UpdateDiagQuickListID(EMNDiagCode *pDiag)
{
	NexTech_Accessor::_DiagnosisCrosswalkCommitPtr pCommit(__uuidof(NexTech_Accessor::DiagnosisCrosswalkCommit));
	NexTech_Accessor::_DiagnosisCodeCommitPtr pICD9(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
	NexTech_Accessor::_DiagnosisCodeCommitPtr pICD10(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
	if (pDiag->nDiagCodeID != -1) pICD9->ID = _bstr_t(AsString(pDiag->nDiagCodeID));
	if (pDiag->nDiagCodeID_ICD10 != -1) pICD10->ID = _bstr_t(AsString(pDiag->nDiagCodeID_ICD10));
	pCommit->ICD9 = pICD9;
	pCommit->ICD10 = pICD10;
	Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
	NexTech_Accessor::_DiagnosisCrosswalksPtr pFoundCrosswalks = GetAPI()->GetCrosswalksInQuickListByDisplayPreference(GetAPISubkey(), GetAPILoginToken(), saCommits);		
	Nx::SafeArray<IUnknown *> saResults(pFoundCrosswalks->Items);
	if (1 == saResults.GetCount())
	{
		// We found a match in the QuickList
		NexTech_Accessor::_DiagnosisCrosswalkPtr pFoundCrosswalk = saResults.GetAt( saResults.GetLowerBound() );
		pDiag->nQuickListID = atol((LPCTSTR)pFoundCrosswalk->DiagQuickListID);
	}
	else
	{
		// No match
		pDiag->nQuickListID = -1;
	}
}

// (c.haag 2014-03-17) - PLID 60929 - Updates the QuickList icon for a given row
void CEmrCodesDlg::UpdateRowQuickListIcon(LPDISPATCH lpRow)
{
	_variant_t varStarFilledIcon = (long)m_hStarFilledIcon;
	_variant_t varStarUnfilledIcon = (long)m_hStarUnfilledIcon;
	_variant_t vNewValue;
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
	NexTech_Accessor::DiagDisplayType displayType = DiagQuickListUtils::GetAPIDiagDisplayType();

	// Change the icon's appearance depending on the global Practice preference
	if (-1 == pDiag->nDiagCodeID && NexTech_Accessor::DiagDisplayType_ICD9 == displayType)
	{
		// If this code has no 9, and we're in 9-only mode, then the star should not exist
		vNewValue = g_cvarNull;
	}
	else if (-1 == pDiag->nDiagCodeID_ICD10 && NexTech_Accessor::DiagDisplayType_ICD10 == displayType)
	{
		// If this code has no 10, and we're in 10-only mode, then the star should not exist
		vNewValue = g_cvarNull;
	}
	else if (-1 == pDiag->nDiagCodeID_ICD10 && NexTech_Accessor::DiagDisplayType_Crosswalk == displayType)
	{
		// If this code has no 10, and we're in crosswalk mode, then the star should not exist. We want
		// to prevent users from adding 9-only codes to their QuickList in crosswalk mode.
		vNewValue = g_cvarNull;
	}
	else if (-1 == pDiag->nDiagCodeID && -1 == pDiag->nDiagCodeID_ICD10)
	{
		// A code with neither a 9 nor 10 should never happen, but check for completeness
		vNewValue = g_cvarNull;
	}
	else
	{
		// Assign the QuickList star
		vNewValue = pDiag->nQuickListID == -1 ? varStarUnfilledIcon : varStarFilledIcon;
	}

	pRow->PutValue(edclcQuickListStar, vNewValue);
}

//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
void CEmrCodesDlg::DragEndDiags(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pToRow(lpRow);
		NXDATALIST2Lib::IRowSettingsPtr pFromRow(lpFromRow);

		//the from row cannot be -1 or outside of the list bounds
		if(pFromRow == NULL) {
			//should be impossible
			ASSERT(FALSE);
			return;
		}


		//this means you are dragging a row to the bottom of the list
		if(pToRow == NULL) {
			//take the dragged item, make it the last in the list, and advance the rest of the list up.

			// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID
			long nFromICD9ID = VarLong(pFromRow->GetValue(edclcICD9DiagCodeID),-1);
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			long nFromICD10ID = VarLong(pFromRow->GetValue(edclcICD10DiagCodeID),-1);
			long nOldIndex = VarLong(pFromRow->GetValue(edclcOrderIndex));

			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;
			
			//move up every successive row
			NXDATALIST2Lib::IRowSettingsPtr p = pFromRow->GetNextRow();
			while(p) {

				long nOrderIndex = VarLong(p->GetValue(edclcOrderIndex));
				nOrderIndex--;
				p->PutValue(edclcOrderIndex, nOrderIndex);

				p = p->GetNextRow();
			}

			//move this row to the end
			long nOrderIndex = m_DiagCodeList->GetRowCount();
			pFromRow->PutValue(edclcOrderIndex, nOrderIndex);

			//now make the same changes to the EMN list
			for(i=0;i<m_pEMN->GetDiagCodeCount(); i++) {

				EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
				
				//TES 2/26/2014 - PLID 60807 - Added ICD10
				if(pDiag->nDiagCodeID == nFromICD9ID && pDiag->nDiagCodeID_ICD10 == nFromICD10ID) {
					//this row needs to be given the final index
					pDiag->nOrderIndex = nOrderIndex;
					//denote that the user moved this item, later used in auditing
					pDiag->bHasMoved = TRUE;
					pDiag->bChanged = TRUE;
				}
				else if(pDiag->nOrderIndex > nOldIndex) {
					//if there are any rows after our original row, promote them
					pDiag->nOrderIndex--;
					pDiag->bChanged = TRUE;
				}
			}

			//all done, the code at the end of this function will sort
			//and tell the parent it is unsaved
		}
		else {

			//must determine which direction we are moving

			// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID
			//TES 2/26/2014 - PLID 60807 - Added ICD10
			long nFromICD9ID = VarLong(pFromRow->GetValue(edclcICD9DiagCodeID),-1);
			long nFromICD10ID = VarLong(pFromRow->GetValue(edclcICD10DiagCodeID),-1);
			long nFromIndex = VarLong(pFromRow->GetValue(edclcOrderIndex));

			long nToICD9ID = VarLong(pToRow->GetValue(edclcICD9DiagCodeID),-1);
			long nToICD10ID = VarLong(pToRow->GetValue(edclcICD10DiagCodeID),-1);
			long nToIndex = VarLong(pToRow->GetValue(edclcOrderIndex));

			//if you are dragging a row up
			if(nFromIndex > nToIndex) {

				// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
				int i = 0;

				//more down every row in between, counting the destination row
				NXDATALIST2Lib::IRowSettingsPtr p = pFromRow->GetPreviousRow();
				bool bFoundToRow = false;
				while(p && !bFoundToRow) {
					if(p == pToRow) {
						bFoundToRow = true;
					}

					long nOrderIndex = VarLong(p->GetValue(edclcOrderIndex));
					nOrderIndex++;
					p->PutValue(edclcOrderIndex, nOrderIndex);

					p = p->GetPreviousRow();
				}
				

				//now set the index for the moved row
				pFromRow->PutValue(edclcOrderIndex, nToIndex);

				//now make the same changes to the EMN list
				for(i=0;i<m_pEMN->GetDiagCodeCount(); i++) {

					EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
					
					//TES 2/26/2014 - PLID 60807 - Added ICD10
					if(pDiag->nDiagCodeID == nFromICD9ID && pDiag->nDiagCodeID_ICD10 == nFromICD10ID) {
						//this row needs to be given the final index
						pDiag->nOrderIndex = nToIndex;
						//denote that the user moved this item, later used in auditing
						pDiag->bHasMoved = TRUE;
						pDiag->bChanged = TRUE;
					}
					else if(pDiag->nOrderIndex < nFromIndex && pDiag->nOrderIndex >= nToIndex) {
						//more down every row in between, counting the destination row
						pDiag->nOrderIndex++;
						pDiag->bChanged = TRUE;
					}
				}

				//all done, the code at the end of this function will sort
				//and tell the parent it is unsaved

			}
			//if you are dragging a row down
			else if(nFromIndex < nToIndex) {
				// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
				int i = 0;

				//more up every row in between, except the destination row
				NXDATALIST2Lib::IRowSettingsPtr p = pFromRow->GetNextRow();
				while(p && p != pToRow) {

					long nOrderIndex = VarLong(p->GetValue(edclcOrderIndex));
					nOrderIndex--;
					p->PutValue(edclcOrderIndex, nOrderIndex);

					p = p->GetNextRow();
				}

				//now set the index for the moved row
				pFromRow->PutValue(edclcOrderIndex, (long)(nToIndex-1));

				//now make the same changes to the EMN list
				for(i=0;i<m_pEMN->GetDiagCodeCount(); i++) {

					EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
					
					//TES 2/26/2014 - PLID 60807 - Added ICD10
					if(pDiag->nDiagCodeID == nFromICD9ID && pDiag->nDiagCodeID_ICD10 == nFromICD10ID) {
						//this row needs to be given the final index
						pDiag->nOrderIndex = nToIndex-1;
						//denote that the user moved this item, later used in auditing
						pDiag->bHasMoved = TRUE;
						pDiag->bChanged = TRUE;
					}
					else if(pDiag->nOrderIndex > nFromIndex && pDiag->nOrderIndex < nToIndex) {
						//more up every row in between, except the destination row
						pDiag->nOrderIndex--;
						pDiag->bChanged = TRUE;
					}
				}

				//all done, the code at the end of this function will sort
				//and tell the parent it is unsaved
			}
		}

		//sort the list
		m_DiagCodeList->Sort();

		// (c.haag 2007-02-21 17:09) - PLID 24150 - Make sure the colors are right
		EnsureDiagCodeColors();

		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		// (s.dhole 2014-02-20 12:44) - PLID  60742 Change call from SetMoreInfoUnsaved() to  SetCodesUnsaved
		m_pEMN->SetCodesUnsaved();

		UpdateDiagCodeArrows();

	}NxCatchAll(__FUNCTION__);
}

//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
void CEmrCodesDlg::SelChangedDiags(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//enable/disable up/down arrows
		UpdateDiagCodeArrows();

	}NxCatchAll(__FUNCTION__);
}

//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
void CEmrCodesDlg::LeftClickDiags(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {

		// (j.jones 2008-07-22 12:02) - PLID 30792 - if they click on a problem icon, display the problems for this
		// diagnosis code in the problem list
		case edclcProblemIcon:
			{

			m_DiagCodeList->CurSel = pRow;
			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

			if(pDiag) {
				// (c.haag 2009-05-20 10:55) - PLID 34312 - Use new problem structure
				if(pDiag->m_apEmrProblemLinks.GetSize() == 0) {
					AfxMessageBox("There are no problems currently on this diagnosis code.");
					return;
				}

				EditDiagnosisProblems();
			}
			else {
				ASSERT(FALSE);
				ThrowNxException("Failed to get a valid diagnosis code object!");
			}
			}
			break;

		// (j.jones 2013-10-17 12:05) - PLID 58981 - added Patient Education support
		// (r.gonet 03/07/2014) - PLID 60756 - Also allow lookup of pt. edu. by clicking the ICD-10 Code Number column
		case edclcICD9CodeNumber:
		case edclcICD10CodeNumber:
		case edclcInfoButton:
			{
			long nICD9DiagCodeID = VarLong(pRow->GetValue(edclcICD9DiagCodeID), -1);
			long nICD10DiagCodeID = VarLong(pRow->GetValue(edclcICD10DiagCodeID), -1);

			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Don't search MedLinePlus if they click the column
			// but we aren't showing patient education links.
			// (r.gonet 03/07/2014) - PLID 60756 - Or if there is no diagnosis code.
			bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
			if(nCol == edclcICD9CodeNumber) {
				NXDATALIST2Lib::IColumnSettingsPtr pCodeNumberCol = m_DiagCodeList->GetColumn(edclcICD9CodeNumber);
				if(!bShowPatientEducationLink || pCodeNumberCol->FieldType != NXDATALIST2Lib::cftTextSingleLineLink || nICD9DiagCodeID == -1) {
					break;
				}
			} else if(nCol == edclcICD10CodeNumber) {
				NXDATALIST2Lib::IColumnSettingsPtr pCodeNumberCol = m_DiagCodeList->GetColumn(edclcICD10CodeNumber);
				if(!bShowPatientEducationLink || pCodeNumberCol->FieldType != NXDATALIST2Lib::cftTextSingleLineLink || nICD10DiagCodeID == -1) {
					break;
				}
			}

			if(nICD9DiagCodeID == -1 && nICD10DiagCodeID == -1) {
				//should not be possible
				ASSERT(FALSE);
				ThrowNxException("No valid diagnosis code was found.");
			}
			m_DiagCodeList->CurSel = pRow;

			if(nCol == edclcICD9CodeNumber) {
				// (r.gonet 03/07/2014) - PLID 60756 - Make the ICD-9 code the primary record to lookup, since they clicked the ICD-9 column,
				// but make the ICD-10 code an alternative if it exists.
				long nPrimaryRecordID = -1;
				CArray<long, long> aryAlternateRecordIDs;
				if(nICD9DiagCodeID != -1) {
					nPrimaryRecordID = nICD9DiagCodeID;
					if(nICD10DiagCodeID != -1) {
						aryAlternateRecordIDs.Add(nICD10DiagCodeID);
					}
				} else {
					nPrimaryRecordID = nICD10DiagCodeID;
				}

				// (r.gonet 10/30/2013) - PLID 58980 - the code number hyperlink goes to the MedlinePlus website
				// (r.gonet 03/07/2014) - PLID 60756 - Added aryAlternateRecordIDs, possibly empty.
				LookupMedlinePlusInformationViaSearch(this, mlpDiagCodeID, nPrimaryRecordID, aryAlternateRecordIDs);
			}
			else if(nCol == edclcICD10CodeNumber) {
				// (r.gonet 03/07/2014) - PLID 60756 - Make the ICD-10 code the primary record to lookup, since they clicked the ICD-10 column,
				// but make the ICD-9 code an alternative if it exists.
				long nPrimaryRecordID = -1;
				CArray<long, long> aryAlternateRecordIDs;
				if(nICD10DiagCodeID != -1) {
					nPrimaryRecordID = nICD10DiagCodeID;
					if(nICD9DiagCodeID != -1) {
						aryAlternateRecordIDs.Add(nICD9DiagCodeID);
					}
				} else {
					nPrimaryRecordID = nICD9DiagCodeID;
				}

				// (r.gonet 10/30/2013) - PLID 58980 - the code number hyperlink goes to the MedlinePlus website
				// (r.gonet 03/07/2014) - PLID 60756 - Added aryAlternateRecordIDs, possibly empty.
				LookupMedlinePlusInformationViaSearch(this, mlpDiagCodeID, nPrimaryRecordID, aryAlternateRecordIDs);
			}
			else if(nCol == edclcInfoButton) {
				// (r.gonet 03/04/2014) - PLID 60756 - Support sending ICD-9 and 10 codes to MedlinePlus.
				long nPrimaryRecordID = -1;
				CArray<long, long> aryAlternateRecordIDs;
				// (r.gonet 03/07/2014) - PLID 60756 - Pt education buttons should be based on ICD-10. If an ICD-10 is not available, 
				// then base it on an ICD-9.
				if(nICD10DiagCodeID != -1) {
					nPrimaryRecordID = nICD10DiagCodeID;
					if(nICD9DiagCodeID != -1) {
						aryAlternateRecordIDs.Add(nICD9DiagCodeID);
					}
				} else {
					nPrimaryRecordID = nICD9DiagCodeID;
				}

				// The info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
				// (r.gonet 03/07/2014) - PLID 60756 - Added aryAlternateRecordIDs, possibly empty.
				LookupMedlinePlusInformationViaURL(this, mlpDiagCodeID, nPrimaryRecordID, aryAlternateRecordIDs);
			}
			break;
			}
		case edclcICD10CodeDesc:
			{
				EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
				switch(pDiag->MatchType){
					case nexgemtManyMatch:
						{
							// (b.savon 2014-03-10 07:13) - PLID 60824 - Pop-up the list of multimatch codes
							HandleNexGEMMultiMatchColumn(pRow);
						}
						break;
					// (r.farnworth 2014-03-10 09:16) - PLID 61246 - Check if clicking the hyperlink, then update the code
					case nexgemtDone:
						{
							if(pDiag->nDiagCodeID_ICD10 == -1)
							{
								UpdateLegacyCode(pRow, FALSE);
							}
						}
						break;
					// (b.savon 2014-03-12 07:26) - PLID 61310 - Handle adding a NexCode from the <Codes> pane datalist
					case nexgemtNoMatch:
						{
							HandleNexCodeColumn(pRow);
						}
						break;
				}
			}
			break;
			// (r.farnworth 2014-04-01 08:34) - PLID 61608 - If the ICD-9 column in the diag list in the <Codes> topic of the EMN is empty, 
			// the cell should have a hyperlink allowing the user to choose from the list of managed ICD-9 codes.
		case edclcICD9CodeDesc:
			{
				EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
				if (VarLong(pRow->GetValue(edclcICD9DiagCodeID)) == -1 && VarLong(pRow->GetValue(edclcICD10DiagCodeID)) == -1) {
					return;
				}
				if (VarLong(pRow->GetValue(edclcICD9DiagCodeID)) == -1) {	//we should only be here if this is -1
					SetICD9ForDiagCodeInfo(pRow);
				}						
			}				
			break;
		// (c.haag 2014-03-17) - PLID 61397 - Handle toggling the QuickList presence of the code
		// (c.haag 2014-03-17) - PLID 60929 - The QuickList star is now null if the code has no ICD-10.
		case edclcQuickListStar:
			if (!m_bReadOnly && pRow->GetValue(edclcQuickListStar).vt != VT_NULL)
			{
				EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));
				if (NULL != pDiag)
				{
					if (-1 == pDiag->nQuickListID)
					{
						// If the current QuickList ID is -1, then the code is not in this user's QuickList and they want to add it.
						NexTech_Accessor::_QuickListDiagCommitPtr pCommit(__uuidof(NexTech_Accessor::QuickListDiagCommit));
						if (-1 != pDiag->nDiagCodeID)
						{
							pCommit->ICD9DiagID = _bstr_t(AsString(pDiag->nDiagCodeID));
						}
						if (-1 != pDiag->nDiagCodeID_ICD10)
						{
							pCommit->ICD10DiagID = _bstr_t(AsString(pDiag->nDiagCodeID_ICD10));
						}
						Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
						NexTech_Accessor::_AddDiagCodesToQuickListResultPtr pResult = GetAPI()->AddDiagCodesToQuickList(GetAPISubkey(), GetAPILoginToken(), saCommits);
						Nx::SafeArray<IUnknown *> saryNewQuickListItems(pResult->NewItems);
						foreach(NexTech_Accessor::_DiagQuickListItemPtr pItem, saryNewQuickListItems)
						{
							long n9ID = (NULL == pItem->ICD9) ? -1 : atol((LPCTSTR)pItem->ICD9->ID);
							long n10ID = (NULL == pItem->ICD10) ? -1 : atol((LPCTSTR)pItem->ICD10->ID);	
							if (n9ID == pDiag->nDiagCodeID && n10ID == pDiag->nDiagCodeID_ICD10)
							{
								pDiag->nQuickListID = atol((LPCTSTR)pItem->ID);
								break;
							}
						}
					}
					else
					{
						// If we get here, then the code is in the QuickList is in the user's QuickList and they want to remove it.
						Nx::SafeArray<BSTR> quickListIDsToDelete = Nx::SafeArray<BSTR>::FromValue( _bstr_t(AsString(pDiag->nQuickListID)) );
						GetAPI()->RemoveDiagQuickListItemsAndRelatedBasedOnDisplayType(GetAPISubkey(), GetAPILoginToken(), quickListIDsToDelete, DiagQuickListUtils::GetAPIDiagDisplayType());
						pDiag->nQuickListID = -1;
					}
					// Update all of the QuickList icons. We can have, for instance, multiple of the same 10 code with
					// different 9 codes. If we add or remove the 10 from our QuickList, all of those codes are subject
					// to change.
					UpdateAllQuickListIcons();

				} // if (NULL != pDiag)
			}
			break;
		}

	} NxCatchAll(__FUNCTION__);
}

//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
void CEmrCodesDlg::RButtonDownDiags(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu pMenu;
		m_DiagCodeList->CurSel = pRow;
		// (r.farnworth 2014-03-25 15:51) - PLID 61380
		bool bIncludeResearch = true;

		// (j.jones 2007-01-05 12:00) - PLID 24070 - update the arrows
		UpdateDiagCodeArrows();
		
		pMenu.CreatePopupMenu();

		// (c.haag 2007-02-22 08:31) - PLID 24868 - Adding Default ICD-9 Codes has no context
		// in an EMR template, so don't show the option if we are a template
		if (!m_bIsTemplate) {
			//TES 2/26/2014 - PLID 60807 - Removed reference to "ICD-9 Codes"
			pMenu.AppendMenu(MF_ENABLED, ID_ADD_DEFAULT_ICD9S, "Add Default Diagnosis Codes");

			if(pRow != NULL) {

				EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

				// (j.jones 2008-07-22 12:45) - PLID 30792 - add ability to add problems, and view problem information
				// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
				// if the EMN is not writeable
				pMenu.AppendMenu(MF_STRING|MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), ID_ADD_ICD9_PROBLEM, "Link with New &Problem");
				// (c.haag 2009-05-28 14:56) - PLID 34312 - Option for linking with existing problems
				CEMR* pEMR = (m_pEMN) ? m_pEMN->GetParentEMR() : NULL;
				if (NULL != pEMR) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION|(m_pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), ID_LINK_ICD9_PROBLEMS, "Link with Existing Problems");
				}
				if (pDiag->HasProblems()) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION, ID_EDIT_ICD9_PROBLEM, "Update Problem &Information");
				}

				// (r.farnworth 2014-03-25 15:39) - PLID 61380 - Look through the datalist and see if we have the currently selected row's ICD-9 listed more than once
				// and the other occurence isn't linked to an ICD-10, dont show the Re-search option.
				NXDATALIST2Lib::IRowSettingsPtr tRow = m_DiagCodeList->GetFirstRow();
				while(tRow) {
					if(VarLong(tRow->GetValue(edclcICD9DiagCodeID),-1) == VarLong(pRow->GetValue(edclcICD9DiagCodeID),-1)
						&& VarLong(tRow->GetValue(edclcICD10DiagCodeID),-1) == -1 && tRow != pRow) {
						bIncludeResearch = false;
						break; //Infinite loops are bad
					}
					tRow = tRow->GetNextRow();
				}

				// (r.farnworth 2014-03-14 14:54) - PLID 61380 - Add a right click menu option in the diagnosis datalist so that you can trigger the < Find ICD-10 Code > functionality.
				if(DiagSearchUtils::GetPreferenceSearchStyle() != eManagedICD9_Search && pDiag->MatchType == nexgemtDone 
					&& pDiag->nDiagCodeID_ICD10 != -1 && pDiag->nDiagCodeID != -1 && bIncludeResearch) {
					pMenu.AppendMenu(MF_STRING|MF_BYPOSITION, ID_RESEARCH_ICD10_CODE, "&Search for another ICD-10 Code");
				}
			}

			pMenu.AppendMenu(MF_SEPARATOR);
		}

		if (pRow) {
			// (j.jones 2014-12-23 10:48) - PLID 64491 - added ability to replace a code
			pMenu.AppendMenu((pRow == NULL) ? MF_DISABLED | MF_GRAYED : MF_ENABLED, ID_REPLACE_DIAG_CODE, "&Replace Diagnosis Code");
			//TES 2/26/2014 - PLID 60807 - Removed reference to "ICD-9 Codes"
			pMenu.AppendMenu((pRow == NULL) ? MF_DISABLED | MF_GRAYED : MF_ENABLED, ID_REMOVE_ICD9, "Remo&ve Diagnosis Code");
		}

		CPoint pt;
		GetCursorPos(&pt);
		pMenu.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);

		//pMenu.DestroyMenu();

	} NxCatchAll(__FUNCTION__);
}

//TES 2/26/2014 - PLID 60807 - Utility function to search by both IDs
NXDATALIST2Lib::IRowSettingsPtr CEmrCodesDlg::FindRowInDiagList(long nICD9ID, long nICD10ID)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->GetFirstRow();
	while(pRow) {
		if(VarLong(pRow->GetValue(edclcICD9DiagCodeID),-1) == nICD9ID && VarLong(pRow->GetValue(edclcICD10DiagCodeID),-1) == nICD10ID) {
			return pRow;
		}
		pRow = pRow->GetNextRow();
	}
	return NULL;
}

//TES 2/27/2014 - PLID 60807 - Resize based on the user's preferences
void CEmrCodesDlg::UpdateDiagnosisListColumnSizes()
{
	DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_DiagCodeList, edclcICD9CodeNumber, edclcICD10CodeNumber,
			50, 50, "", "", edclcICD9CodeDesc, edclcICD10CodeDesc, true, true, true);
}

//TES 3/3/2014 - PLID 61080 - strDiagCodeList has some extra auditing information in it, made a new function for a better-looking list here
CString CEmrCodesDlg::GetDiagCodeListForInterface(EMNCharge *pCharge)
{
	//TES 3/3/2014 - PLID 61080 - Similar to the preview pane, we will show 10(9) if both exist, otherwise just the one that exists
	CString str;
	for(int i = 0; i < pCharge->aryDiagIndexes.GetSize(); i++) {
		EMNDiagCode* pDiag = m_pEMN->GetDiagCode(pCharge->aryDiagIndexes[i]);
		if(pDiag->nDiagCodeID_ICD10 != -1) {
			str += pDiag->strCode_ICD10;
			// (s.dhole 2014-03-05 09:39) - PLID 60916
			if(pDiag->nDiagCodeID != -1 && !pDiag->strCode.IsEmpty()) {
				str += "(" + pDiag->strCode + ")";
			}
		}
		else {
			ASSERT(pDiag->nDiagCodeID != -1);
			str += pDiag->strCode;
		}
		str += ";";
	}
	if(str.IsEmpty()) {
		return "<None>";
	}
	else {
		str.TrimRight(";");
	}
	return str;
}

// (b.savon 2014-03-06 08:21) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
void CEmrCodesDlg::AddICD10DisplayColumn(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNDiagCode *pDiag)
{
	pRow->PutValue(edclcICD10CodeDesc, AsBstr(GetNexGEMDisplayText(pDiag->nDiagCodeID_ICD10, pDiag->strCodeDesc_ICD10, pDiag->MatchType)));
	SetNexGEMMatchColumnFormat(pRow, pDiag);
}

// (b.savon 2014-03-06 09:36) - PLID 60824 - Set the column format
void CEmrCodesDlg::SetNexGEMMatchColumnFormat(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNDiagCode *pDiag)
{	
	if( pDiag->nDiagCodeID_ICD10 == -1 ){
		NXDATALIST2Lib::IFormatSettingsPtr pHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLineLink);

		// (r.gonet 03/17/2014) - PLID 60756 - There is no ICD-10 code. So there is no ICD-10 patient education lookup by clicking the code cell.
		pRow->PutCellLinkStyle(edclcICD10CodeNumber, dlLinkStyleFalse);
		pRow->PutRefCellFormatOverride(edclcICD10CodeDesc, pHyperLink);
	}else{
		NXDATALIST2Lib::IFormatSettingsPtr pNoHyperLink(__uuidof(NXDATALIST2Lib::FormatSettings));
		pNoHyperLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);

		// (r.gonet 03/17/2014) - PLID 60756 - There is an ICD-10 code. So there is ICD-10 patient education lookup by clicking the code cell (if the preference is set).
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		if(bShowPatientEducationLink) {
			pRow->PutCellLinkStyle(edclcICD10CodeNumber, dlLinkStyleTrue);
		} else {
			pRow->PutCellLinkStyle(edclcICD10CodeNumber, dlLinkStyleFalse);
		}
		pRow->PutRefCellFormatOverride(edclcICD10CodeDesc, pNoHyperLink);
	}
	
	pRow->PutCellBackColor(edclcICD10CodeDesc, GetDiagnosisNexGEMMatchColor(pDiag->nDiagCodeID_ICD10, pDiag->MatchType));
}

// (b.savon 2014-03-06 08:21) - PLID 60824 - SPAWN - Set the colors and behavior of the ICD-10 columns for spawned diagnosis codes in the diag codes list in the <Codes> topic.
void CEmrCodesDlg::HandleNexGEMMultiMatchColumn(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	//Call the API to get the multi-matches
	CString strDiagCodeID;
	strDiagCodeID.Format("%li", AsLong(pRow->GetValue(edclcICD9DiagCodeID)));
	//TES 3/12/2014 - PLID 60819 - I changed this code to return a DiagnosisCodeCommits object instead of a DiagnosisCodes object, updated this function accordingly
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
			HandleNexCodeColumn(pRow);
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
		m_pDiagCodeCommitMultiMatch->aryMultiMatch.RemoveAll();
		foreach(NexTech_Accessor::_DiagnosisCodeCommitPtr matchDiag, saryCodes){
			strCodeDrop.Format("%li;%s;", index++, AsString(matchDiag->Code) + " - " + AsString(matchDiag->description));
			strDropdown += strCodeDrop;
			m_pDiagCodeCommitMultiMatch->aryMultiMatch.Add(matchDiag);
		}

		//Populate dropdown
		NXDATALIST2Lib::IFormatSettingsPtr pfsMultiMatch(__uuidof(NXDATALIST2Lib::FormatSettings));
		pfsMultiMatch->PutDataType(VT_BSTR);
		pfsMultiMatch->PutFieldType(NXDATALIST2Lib::cftComboSimple);
		pfsMultiMatch->PutConnection(_variant_t((LPDISPATCH)NULL));
		pfsMultiMatch->PutEditable(VARIANT_TRUE);
		pfsMultiMatch->PutComboSource(AsBstr(strDropdown));
		pRow->PutRefCellFormatOverride(edclcICD10CodeDesc, pfsMultiMatch);
		pRow->PutValue(edclcICD10CodeDesc, -1);
		m_DiagCodeList->StartEditing(pRow, edclcICD10CodeDesc);
	}
}


// (r.farnworth 2014-03-10 09:16) - PLID 61246 - Add a button, new link style, or auto load if a user adds codes in icd9 mode and another user loads in icd10 mode
// (c.haag 2014-03-24) - PLID 60930 - Added a silent mode
void CEmrCodesDlg::UpdateLegacyCode(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bSilent)
{
	EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

	//We are only ever looking at 1 ICD-9 code but we must use arrays to use the API functions
	Nx::SafeArray<BSTR> saICD9CodeIDs;
	saICD9CodeIDs.Add(AsString(pDiag->nDiagCodeID));
	NexTech_Accessor::_NexGEMMatchResultsPtr codeMatchResults = GetAPI()->GetNexGEMMatchesFromICD9s(GetAPISubkey(), GetAPILoginToken(), saICD9CodeIDs);
	Nx::SafeArray<IUnknown*> codeResults = codeMatchResults->match;

	// (r.farnworth 2014-03-12 15:03) - PLID 61246 - Need error handling for an empty array which should never happen
	if(codeResults.GetSize() > 0)
	{
		NexTech_Accessor::_NexGEMMatchResultPtr matchedResult = codeResults.GetAt(0); 

		if(matchedResult)
		{
			pDiag->MatchType = MapMatchStatus(matchedResult->matchStatus);
			pDiag->bChanged = TRUE;

			if(pDiag->MatchType == nexgemtDone) {
				// Make sure it's not already in the list
				if (FindRowInDiagList(AsLong(pRow->GetValue(edclcICD9DiagCodeID)), AsLong(matchedResult->exactMatchedICD10->ID)) != NULL) {
					MessageBox("This diagnosis code is already in the list and will be automatically removed.", NULL, MB_OK|MB_ICONEXCLAMATION);
					m_DiagCodeList->PutCurSel(pRow);
					OnCommand(ID_REMOVE_ICD9, 0);
					return;
				}

				pDiag->nDiagCodeID_ICD10 = atol(matchedResult->exactMatchedICD10->ID);
				pDiag->strCode_ICD10 = AsString(matchedResult->exactMatchedICD10->Code);
				pDiag->strCodeDesc_ICD10 = AsString(matchedResult->exactMatchedICD10->description);

				pRow->PutValue(edclcICD10DiagCodeID, atol(matchedResult->exactMatchedICD10->ID));
				pRow->PutValue(edclcICD10CodeNumber, matchedResult->exactMatchedICD10->Code);
			}
		
			AddICD10DisplayColumn(pRow, pDiag);
			m_pEMN->SetCodesUnsaved();

			// (b.savon 2014-03-12 12:43) - PLID 61330 - Ensure the diag (which codes) match the charge if its attached
			EnsureEMRChargesToDiagCodes();

			// (c.haag 2014-03-18) - PLID 60929 - Update the QuickList
			UpdateDiagQuickListID(pDiag);
			UpdateRowQuickListIcon(pRow);

			// (r.farnworth 2014-10-01 14:13) - PLID 63425 - Repicking a 10 but not if its an exact match
			if (!pDiag->MatchType == nexgemtDone) {
				LinkDiagnosisCodeToProblem(pDiag);
			}

			// (j.jones 2012-10-01 15:44) - PLID 52869 - this function will check their preference
			// to save the EMN and warn about drug interactions
			CEmrTreeWnd *pTree = GetEmrTreeWnd();
			if(!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
				// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
				m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
			}

			// (b.savon 2014-04-01 14:14) - PLID 61246 - Add a button, new link style, or auto load
			// if a user adds codes in icd9 mode and another user loads in icd10 mode
			if( pDiag->MatchType == nexgemtNoMatch || pDiag->MatchType == nexgemtManyMatch ){
				LeftClickDiags(pRow, edclcICD10CodeDesc, -1, -1, -1);
			}
		}
	}
	else
	{
		if (!bSilent)
		{
			MessageBox("No matches could be found for this code.", NULL);
		}
	}
}

// (b.savon 2014-03-12 07:34) - PLID 61310 - Handle adding a NexCode from the <Codes> pane datalist
void CEmrCodesDlg::HandleNexCodeColumn(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{

		CNexCodeDlg dlg(this);
		if( IDOK == dlg.DoModal() ){
			long nDiagCodeID_ICD10 = dlg.GetDiagCodeID();
			// Make sure it's not already in the list
			if (FindRowInDiagList(AsLong(pRow->GetValue(edclcICD9DiagCodeID)), nDiagCodeID_ICD10) != NULL) {
				MessageBox("This diagnosis code has already been selected.", NULL, MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			// Update memory
			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

			pDiag->nDiagCodeID_ICD10 = nDiagCodeID_ICD10;
			pDiag->strCode_ICD10 = dlg.GetDiagCode();
			pDiag->strCodeDesc_ICD10 = dlg.GetDiagCodeDescription();
			pDiag->MatchType = nexgemtDone;
			pDiag->bChanged = TRUE;

			// Update the display
			pRow->PutValue(edclcICD10DiagCodeID, pDiag->nDiagCodeID_ICD10);
			pRow->PutValue(edclcICD10CodeNumber, AsBstr(pDiag->strCode_ICD10));
			AddICD10DisplayColumn(pRow, pDiag);

			// (c.haag 2014-03-20) - PLID 60929 - Update the QuickList
			UpdateDiagQuickListID(pDiag);
			UpdateRowQuickListIcon(pRow);
			m_pEMN->SetCodesUnsaved();

			// (b.savon 2014-03-12 12:43) - PLID 61330 - Ensure the diag (which codes) match the charge if its attached
			EnsureEMRChargesToDiagCodes();

			// (r.farnworth 2014-08-27 11:52) - PLID 63425 - NexCode
			LinkDiagnosisCodeToProblem(pDiag);

			// (j.jones 2012-10-01 15:44) - PLID 52869 - this function will check their preference
			// to save the EMN and warn about drug interactions
			CEmrTreeWnd *pTree = GetEmrTreeWnd();
			if(!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
				// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
				m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-03-12 12:43) - PLID 61330 - When an ICD10 code is matched from Multimatch or NexCode and the 
// ICD9 is tied to a charge, update the which codes to reflect the matched ICD10
void CEmrCodesDlg::EnsureEMRChargesToDiagCodes()
{
	for(int iCharge = 0; iCharge < m_pEMN->GetChargeCount(); iCharge++) {
		//Grab the charge from memory
		EMNCharge *pCharge = m_pEMN->GetCharge(iCharge);
		//If there are some diag (which codes) attached to it..
		if( !pCharge->aryDiagIndexes.IsEmpty() ){
			//Clear the interface text for this charge
			pCharge->strDiagCodeList = "";
			// Update their interface text in memory
			for(int iDiag = 0; iDiag < pCharge->aryDiagIndexes.GetCount(); iDiag++ ){
				//Go through all of the diag (which codes) from memory for this charge
				EMNDiagCode* pDiag = m_pEMN->GetDiagCode(pCharge->aryDiagIndexes[iDiag]);
				pCharge->strDiagCodeList += GenerateEMRDiagCodesTAuditValue(pDiag->strCode, pDiag->strCode_ICD10);;
			}
		}
	}

	RefreshChargeList();
}

// (b.savon 2014-03-14 08:22) - PLID 60824 - Handle the selection of a multimatch
void CEmrCodesDlg::EditingFinishedDiags(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if( pRow == NULL ){
			return;
		}

		if( bCommit == FALSE ){
			if( nCol == edclcICD10CodeDesc ){
				AddICD10DisplayColumn(pRow, (EMNDiagCode*)AsLong(pRow->GetValue(edclcEMNDiagCodePtr)));
			}
			return;
		}

		switch(nCol){
			case edclcICD10CodeDesc:
				{
					if( AsLong(varOldValue) == AsLong(varNewValue) ){
						return;
					}

					NexTech_Accessor::_DiagnosisCodeCommitPtr selectedDiagCode = m_pDiagCodeCommitMultiMatch->aryMultiMatch.GetAt(AsLong(varNewValue));

					//Save
						// Make sure the selected ICD10 is already imported into PracData
					CString strDiagCodeID_ICD10 = AsString((LPCTSTR)selectedDiagCode->ID);
					VARIANT_BOOL vbICD10 = selectedDiagCode->ICD10;
					if( strDiagCodeID_ICD10.IsEmpty() && vbICD10 != VARIANT_FALSE ){
						//Need to Import
						Nx::SafeArray<IUnknown *> saryCodeToCreate;
						saryCodeToCreate.Add(selectedDiagCode);

						NexTech_Accessor::_DiagnosisCodesPtr pImportedDiagCode = 
							GetAPI()->CreateDiagnosisCodes(
										GetAPISubkey(),
										GetAPILoginToken(),
										saryCodeToCreate
									);

						if( pImportedDiagCode == NULL ){
							MessageBox("Practice was unable to save this diagnosis code.", "Practice", MB_OK|MB_ICONERROR);
							return;
						}

						Nx::SafeArray<IUnknown *> saryImportedCodes = pImportedDiagCode->Codes;
						if( saryImportedCodes.GetSize() != 1 ){
							MessageBox("Unexpected ICD-10 import value.", "Practice", MB_OK|MB_ICONERROR);
							return;
						}

						NexTech_Accessor::_DiagnosisCodePtr pImportedICD10 = saryImportedCodes.GetAt(0);
						selectedDiagCode->ID = pImportedICD10->ID;
																			
					}else{
						// Make sure it's not already in the list
						if (FindRowInDiagList(AsLong(pRow->GetValue(edclcICD9DiagCodeID)), AsLong(selectedDiagCode->ID)) != NULL) {
							MessageBox("This diagnosis code has already been selected.", NULL, MB_OK|MB_ICONEXCLAMATION);
							AddICD10DisplayColumn(pRow, (EMNDiagCode*)AsLong(pRow->GetValue(edclcEMNDiagCodePtr)));
							return;
						}
					}

					//it was not a duplicate

					//update the memory object.
					EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

					pDiag->nDiagCodeID_ICD10 = AsLong(selectedDiagCode->ID);
					pDiag->strCode_ICD10 = AsString(selectedDiagCode->Code);
					pDiag->strCodeDesc_ICD10 = AsString(selectedDiagCode->description);
					pDiag->MatchType = nexgemtDone;
					pDiag->bChanged = TRUE;

					// Update the display
					pRow->PutValue(edclcICD10DiagCodeID, pDiag->nDiagCodeID_ICD10);
					pRow->PutValue(edclcICD10CodeNumber, AsBstr(pDiag->strCode_ICD10));
					AddICD10DisplayColumn(pRow, pDiag);

					// (c.haag 2014-03-20) - PLID 60929 - Update the QuickList
					UpdateDiagQuickListID(pDiag);
					UpdateRowQuickListIcon(pRow);

					// (r.farnworth 2014-08-27 11:52) - PLID 63425 - MultiMatch
					LinkDiagnosisCodeToProblem(pDiag);

					m_pEMN->SetCodesUnsaved();

					// (b.savon 2014-03-12 12:43) - PLID 61330 - Ensure the diag (which codes) match the charge if its attached
					EnsureEMRChargesToDiagCodes();

					// (j.jones 2012-10-01 15:44) - PLID 52869 - this function will check their preference
					// to save the EMN and warn about drug interactions
					CEmrTreeWnd *pTree = GetEmrTreeWnd();
					if(!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
						// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
						m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
					}
				}
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-03-10 15:10) - PLID 60991 - Add a button to access NexCode from the <Codes> topic in the EMN and add the code to the datalist
void CEmrCodesDlg::OnBnClickedAddNexcodeBtn()
{
	try{

		CNexCodeDlg dlg(this);
		if( IDOK == dlg.DoModal() ){
			CDiagSearchResults diagCode;
			//Ensure 9 is empty
			diagCode.m_ICD9.m_strCode = "";
			diagCode.m_ICD9.m_strDescription = "";

			//Populate 10
			diagCode.m_ICD10.m_nDiagCodesID = dlg.GetDiagCodeID();
			diagCode.m_ICD10.m_strCode = dlg.GetDiagCode();
			diagCode.m_ICD10.m_strDescription = dlg.GetDiagCodeDescription();
	
			AddDiagCodeBySearchResults(diagCode);
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.farnworth 2014-04-01 08:36) - PLID 61608 - If the ICD-9 column in the diag list in the <Codes> topic of the EMN is empty, 
// the cell should have a hyperlink allowing the user to choose from the list of managed ICD-9 codes.
void CEmrCodesDlg::SetICD9ForDiagCodeInfo(NXDATALIST2Lib::IRowSettingsPtr pRow)
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
			if (!rsDiag->eof)
			{
				strCode = AdoFldString(rsDiag->Fields, "CodeNumber");
				strDescription = AdoFldString(rsDiag->Fields, "CodeDesc");
			}
			else
			{
				// (r.farnworth 2014-04-01 08:36) - PLID 61608 - This may occur if one user deletes the code while another user is making a selection
				MsgBox("The selected code is no longer available in your system. Please try again.");
				return;
			}

			//Look for duplicates
			if (FindRowInDiagList(nDiagCodeID, AsLong(pRow->GetValue(edclcICD10DiagCodeID))) != NULL) {
				MessageBox("This diagnosis code has already been selected.", NULL, MB_OK|MB_ICONEXCLAMATION);
				return;
			}

			EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pRow->GetValue(edclcEMNDiagCodePtr));

			//add the 9 information
			pDiag->nDiagCodeID = nDiagCodeID;
			pDiag->strCode = strCode;
			pDiag->strCodeDesc = strDescription;
			pDiag->MatchType = nexgemtDone;
			pDiag->bChanged = TRUE;

			pRow->PutValue(edclcICD9DiagCodeID, pDiag->nDiagCodeID);
			pRow->PutValue(edclcICD9CodeNumber, AsBstr(pDiag->strCode));
			pRow->PutValue(edclcICD9CodeDesc, AsBstr(pDiag->strCodeDesc));
			pRow->PutCellLinkStyle(edclcICD9CodeDesc, dlLinkStyleFalse);

			UpdateDiagQuickListID(pDiag);
			UpdateRowQuickListIcon(pRow);
			m_pEMN->SetCodesUnsaved();
			EnsureEMRChargesToDiagCodes();

			// (r.farnworth 2014-08-27 11:52) - PLID 63425 - Picking a 9
			LinkDiagnosisCodeToProblem(pDiag);

			// to save the EMN and warn about drug interactions
			CEmrTreeWnd *pTree = GetEmrTreeWnd();
			if(!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
				m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
			}
		}
	}
}

// (c.haag 2014-07-16) - PLID 54905 - This function is called when the user presses a button with the intent
// to link all EMN diagnosis codes to all EMN charges.
void CEmrCodesDlg::OnLinkCodesToAllCharges()
{
	try
	{
		// Abort if there are no codes
		if (0 == m_DiagCodeList->GetRowCount())
		{
			MessageBox("There are no diagnosis codes to link charges with.", NULL, MB_ICONINFORMATION);
			return;
		}

		// Abort if there are no charges
		if (0 == m_BillList->GetRowCount())
		{
			MessageBox("There are no charges to link diagnosis codes with.", NULL, MB_ICONINFORMATION);
			return;
		}
		
		// Confirm that the user wants to do this. We can already link diagnosis codes with charges automatically through spawning, 
		// and in that case Practice respects a preference that ignores non-billable charges. The preference also applies here, so we should
		// format the message so that the user understands what will happen without wondering why only some charges got linked.
		BOOL bSkipLinkingNonBillableCPTs = (GetRemotePropertyInt("EMR_SkipLinkingDiagsToNonBillableCPTs", 1, 0, GetCurrentUserName(), true) != 0) ? TRUE : FALSE;
		if (bSkipLinkingNonBillableCPTs)
		{
			if (IDNO == MessageBox("Do you wish to save this EMN and update the 'WhichCodes' field "
				"on every billable charge to point to every diagnosis code for this EMN?\r\n", NULL, MB_YESNO | MB_ICONQUESTION))
			{
				return;
			}
		}
		else
		{
			if (IDNO == MessageBox("Do you wish to save this EMN and update the 'WhichCodes' field "
				"on every charge to point to every diagnosis code for this EMN?\r\n", NULL, MB_YESNO | MB_ICONQUESTION))
			{
				return;
			}
		}

		// Save the EMR
		CWaitCursor wc;
		EmrSaveStatus essResult = (EmrSaveStatus)GetEmrTreeWnd()->SendMessage(NXM_EMR_SAVE_ALL, TRUE, NULL);
		if (FAILED(essResult)){
			// The user cancelled the save so we can't use the API logic
			return;
		}

		// Now link the codes
		GetAPI()->LinkAllEMNDiagCodesToCPTCodes(GetAPISubkey(), GetAPILoginToken(), _bstr_t(m_pEMN->GetID()));

		// Now read the charges from data and update the EMN object with them
		m_pEMN->ReloadChargesFromData();

		// Display updated data
		RefreshChargeList();
	}
	NxCatchAll(__FUNCTION__);
}
//(J.Camacho 07/21/2014) - PLID 62638 - Create the dropdown list for the Keyword items.
void CEmrCodesDlg::OnBtnClickedSearchQueue()
{
	try {
		//if keywords exist make a list of all them with the menu object
		//create a menu
		CNxMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0, EmObjectID=0;
		//create menu id mapping (thanks to Adam and David)
		Nx::MenuCommandMap<long> menuCmds;
		
		for (CString var : vSearchQueueKeywords)
		{
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, menuCmds(EmObjectID++), var);
		}

		int nCmdID = 0;
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_EDIT_CODES);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		}
		else {
			CPoint pt;
			GetCursorPos(&pt);
			nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}

		if (nCmdID == 0) {
			return;
		}
		else
		{
			
			//pull the data.
			CString text;
			mnu.GetMenuStringA(nCmdID, text, NULL);
			m_DiagSearch->SearchStringText = _bstr_t(text);			
		}
	}
	NxCatchAll(__FUNCTION__);
}


//(J.Camacho 8/11/2014) - PLID 62641 - EMNkeywords are being picked up in the search queue list.
void CEmrCodesDlg::SetSearchQueueList()
{
	try
	{
		//view when the last time the EMN was modified was. If it just saved then that would have been now.
		COleDateTime dtLastSaved = m_pEMN->GetEMNModifiedDate();
		//(j.camacho 2014-08-15) - plid 62641 - Check to see if API needs to update the searchqueue list. Check last time it updated the search queue.
		if (dtLastSaved != this->m_dtLastSearchUpdate )
		{	
			
			//(j.camacho 7/23/2014 4:34) - plid 62641 - get all Search Queue keywords associated with the EMN
			ASSERT(m_pEMN);
			NexTech_Accessor::_GetEMNMoreInfoRequestDataPtr pmInfo(__uuidof(NexTech_Accessor::GetEMNMoreInfoRequestData));
			pmInfo->put_emnID(_bstr_t(m_pEMN->GetID()));
			//IncludeSearchQueue was made nullable so create nullable bool ptr.
			NexTech_Accessor::_NullableBoolPtr pIncludeSearchQueue(__uuidof(NexTech_Accessor::NullableBool));
			pIncludeSearchQueue->SetBool(VARIANT_TRUE);
			pmInfo->IncludeSearchQueue = pIncludeSearchQueue;
			NexTech_Accessor::_EMNMoreInfoPtr pMoreInfo(__uuidof(NexTech_Accessor::EMNMoreInfo));
			pMoreInfo = GetAPI()->GetEMNMoreInfo(GetAPISubkey(), GetAPILoginToken(), pmInfo);
			//store all Keywords pulled from API call
			NexTech_Accessor::_EMNSearchQueueKeywordsPtr EMNKeywords = pMoreInfo->SearchQueueKeywords;

			//skip this if there are no keywords.
			if (EMNKeywords)
			{
				//Fill out search queue member. 
				vSearchQueueKeywords.clear();
				Nx::SafeArray<IUnknown*> saEMNKeywords(EMNKeywords->SearchQueueKeywords);
				//Next
				CArray<NexTech_Accessor::_EMNSearchQueueKeywordPtr> aryEMNKeywords;
				saEMNKeywords.ToArray(aryEMNKeywords);
				foreach(NexTech_Accessor::_EMNSearchQueueKeywordPtr pKeyword, aryEMNKeywords)
				{
					vSearchQueueKeywords.push_back((LPCTSTR)pKeyword->SearchQueueKeyword);
				}
			}
			//update last saved
			m_dtLastSearchUpdate = dtLastSaved;
		}
	}
	NxCatchAll(__FUNCTION__);

}


//(J.Camacho 8/11/2014) - plid 62641 
LRESULT CEmrCodesDlg::OnSaveSearchQueueList(WPARAM wParam, LPARAM lParam)
{
	try
	{

		if (!m_pEMN || m_bIsTemplate)
		{
			return 0;
		}
		CEmrTreeWnd* pTreeWnd = m_pEMN->GetInterface();
		if (pTreeWnd && (m_pEMN->IsUnsaved() || m_pEMN->GetID() == -1) && m_pEMN->IsWritable() && (!m_bIsSaving))
		{
			
			CGuardLock lock(this->m_bIsSaving);
			GetEmrTreeWnd()->SaveEMR(esotEMN, (long)m_pEMN, FALSE, FALSE);
			if (m_pEMN->IsUnsaved())
			{
				//either they said no to saving or something is amiss, tell them the search queue won't update
				MsgBox("Saving was either cancelled or failed, the Search Queue will not be updated.", MB_OK);
			}
			
		}
		//(J.Camacho 8/11/2014) - plid 62641 
		SetSearchQueueList();
		EnableControls();
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2014-12-23 13:45) - PLID 64491 - moved the ability to remove a diag code to its own function
void CEmrCodesDlg::RemoveDiagCode(NXDATALIST2Lib::IRowSettingsPtr pDiagRow)
{
	try {

		if (pDiagRow == NULL) {
			//how did this happen?
			ASSERT(FALSE);
			return;
		}

		// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to use dclcDiagCodeID
		long nICD9DiagID = VarLong(pDiagRow->GetValue(edclcICD9DiagCodeID), -1);
		//TES 2/26/2014 - PLID 60807 - Added ICD-10
		long nICD10DiagID = VarLong(pDiagRow->GetValue(edclcICD10DiagCodeID), -1);
		EMNDiagCode *pDiag = (EMNDiagCode*)VarLong(pDiagRow->GetValue(edclcEMNDiagCodePtr));

		// (c.haag 2008-07-25 16:14) - PLID 30826 - Don't let the user delete the diagnosis
		// code if it is tied to saved problems and the user doesn't have permissions
		if (!CanCurrentUserDeleteEmrProblems()) {
			EMNDiagCode* pCode = m_pEMN->GetDiagCodeByDiagID(nICD9DiagID, nICD10DiagID);
			if (NULL != pCode && m_pEMN->DoesDiagCodeHaveSavedProblems(pCode)) {
				// (r.gonet 03/06/2014) - PLID 61191 - Renamed reference to ICD-9 to diagnosis since we can now have ICD-9s or ICD-10s
				MsgBox(MB_OK | MB_ICONERROR, "You may not remove this diagnosis code because it is associated with one or more saved problems.");
				return;
			}
		}

		// (j.jones 2008-07-29 14:14) - PLID 30729 - see if there are any problems at all
		BOOL bHasProblems = pDiag->HasProblems();

		//remove from the CEMN object
		// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
		m_pEMN->RemoveDiagCode(nICD9DiagID, nICD10DiagID);

		//remove from the datalist
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_DiagCodeList->CurSel;

		//DRT 1/11/2007 - PLID 24177 - Need to remove this code from any associated CPT codes
		NXDATALIST2Lib::IRowSettingsPtr pNextRow = NULL;
		if (pRow != NULL) {
			CString strICD9Code = VarString(pRow->GetValue(edclcICD9CodeNumber), "");
			// (j.jones 2009-01-02 09:48) - PLID 32601 - also need to pass in the DiagCodeID
			long nICD9ID = VarLong(pRow->GetValue(edclcICD9DiagCodeID), -1);
			//TES 2/26/2014 - PLID 60807 - Added ICD-10
			long nICD10ID = VarLong(pDiagRow->GetValue(edclcICD10DiagCodeID), -1);
			CString strICD10Code = VarString(pRow->GetValue(edclcICD10CodeNumber), "");
			//TES 2/28/2014 - PLID 61080 - This now supports ICD-10 fields
			RemoveDiagCodeFromWhichCodes(nICD9ID, strICD9Code, nICD10ID, strICD10Code);

			pNextRow = pRow->GetNextRow();

			//Now remove from interface
			m_DiagCodeList->RemoveRow(pRow);
		}

		// (j.jones 2008-07-22 09:26) - PLID 30792 - try to hide the problem icon column
		ShowHideProblemIconColumn_DiagList();

		// (j.jones 2008-07-29 14:15) - PLID 30729 - if we has problems, send a message
		// stating that they changed
		if (bHasProblems) {
			GetEmrTreeWnd()->SendMessage(NXM_EMR_PROBLEM_CHANGED);
		}

		// (j.jones 2007-01-05 11:52) - PLID 24070 - update remaining order indices
		//TES 2/26/2014 - PLID 60807 - m_DiagCodeList is an NxDataList2 now
		while (pNextRow) {
			long nOrderIndex = VarLong(pNextRow->GetValue(edclcOrderIndex), -1);
			nOrderIndex--;

			// (r.farnworth 2014-03-27 07:47) - PLID 61554 - OrderIndex in EMRDiagCodesT was not properly updating resulting in bad data
			EMNDiagCode *ptDiag = (EMNDiagCode*)VarLong(pNextRow->GetValue(edclcEMNDiagCodePtr));
			ptDiag->bChanged = TRUE;

			pNextRow->PutValue(edclcOrderIndex, (long)nOrderIndex);

			pNextRow = pNextRow->GetNextRow();
		}

		// (c.haag 2007-02-21 17:09) - PLID 24150 - Make sure the colors are right
		EnsureDiagCodeColors();
		UpdateDiagCodeArrows();
		//TES 3/31/2014 - PLID 60807 - We may need to resize the columns now
		UpdateDiagnosisListColumnSizes();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-12-23 13:20) - PLID 64491 - added ability to replace a diagnosis code
void CEmrCodesDlg::OnReplaceDiagCode()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_DiagCodeList->CurSel;
		if (pCurRow == NULL) {
			return;
		}

		DiagCodeInfoPtr oldDiagCode = make_shared<DiagCodeInfo>();

		oldDiagCode->nDiagCode9ID = VarLong(pCurRow->GetValue(edclcICD9DiagCodeID), -1);
		if (oldDiagCode->nDiagCode9ID != -1) {
			oldDiagCode->strDiagCode9Code = VarString(pCurRow->GetValue(edclcICD9CodeNumber), "");
			oldDiagCode->strDiagCode9Desc = VarString(pCurRow->GetValue(edclcICD9CodeDesc), "");
		}
		oldDiagCode->nDiagCode10ID = VarLong(pCurRow->GetValue(edclcICD10DiagCodeID), -1);
		if (oldDiagCode->nDiagCode10ID != -1) {
			oldDiagCode->strDiagCode10Code = VarString(pCurRow->GetValue(edclcICD10CodeNumber), "");
			oldDiagCode->strDiagCode10Desc = VarString(pCurRow->GetValue(edclcICD10CodeDesc), "");
		}

		long nNewDiag9CodeID = -1;
		long nNewDiag10CodeID = -1;

		CReplaceDiagCodeDlg dlg(this);
		if (IDOK == dlg.DoModal(m_bkgColor, oldDiagCode)) {
			
			if (dlg.m_newDiagCode->nDiagCode9ID == -1 && dlg.m_newDiagCode->nDiagCode10ID == -1) {
				//the dialog should not have allowed this
				ASSERT(FALSE);
				return;
			}

			//ensure the code doesn't already exist
			if (FindRowInDiagList(dlg.m_newDiagCode->nDiagCode9ID, dlg.m_newDiagCode->nDiagCode10ID) != NULL) {
				MessageBox("This diagnosis code has already been selected.", NULL, MB_OK | MB_ICONEXCLAMATION);
				return;
			}

			NexGEMMatchType matchType = nexgemtDone;
			if (dlg.m_newDiagCode->nDiagCode10ID == -1 && DiagSearchUtils::GetPreferenceSearchStyle() != eManagedICD9_Search)
			{
				// No match type because there's no 10 code and we're not in 9-mode.
				matchType = nexgemtNoMatch;
			}

			//update the memory object
			for (int i = 0; i<m_pEMN->GetDiagCodeCount(); i++) {

				EMNDiagCode* pDiag = m_pEMN->GetDiagCode(i);
				if (pDiag->nDiagCodeID == oldDiagCode->nDiagCode9ID && pDiag->nDiagCodeID_ICD10 == oldDiagCode->nDiagCode10ID) {

					//templates remove and re-add the code, regular EMNs replace the code
					if (m_bIsTemplate && !pDiag->bIsNew) {
						//make a copy of the old code and add it to the deleted list
						EMNDiagCode *pDeletedCode = new EMNDiagCode();
						*pDeletedCode = *pDiag;
						pDeletedCode->bReplaced = TRUE;
						m_pEMN->AddDeletedDiagCode(pDeletedCode);

						//mark the code as new
						pDiag->nID = -1;
						pDiag->bIsNew = TRUE;
					}

					pDiag->nDiagCodeID = dlg.m_newDiagCode->nDiagCode9ID;
					pDiag->nDiagCodeID_ICD10 = dlg.m_newDiagCode->nDiagCode10ID;
					pDiag->strCode = dlg.m_newDiagCode->strDiagCode9Code;
					pDiag->strCodeDesc = dlg.m_newDiagCode->strDiagCode9Desc;
					pDiag->strCode_ICD10 = dlg.m_newDiagCode->strDiagCode10Code;
					pDiag->strCodeDesc_ICD10 = dlg.m_newDiagCode->strDiagCode10Desc;
					pDiag->MatchType = matchType;
					pDiag->bChanged = TRUE;
					pDiag->bReplaced = TRUE;

					UpdateDiagQuickListID(pDiag);
					break;
				}
			}

			//if any charges link to this index, mark the charge as changed
			EnsureEMRChargesToDiagCodes();

			m_pEMN->SetCodesUnsaved();

			//refresh the screen
			RefreshDiagCodeList();
			RefreshChargeList();

			//check drug interactions
			CEmrTreeWnd *pTree = GetEmrTreeWnd();
			if (!m_bIsTemplate && pTree != NULL && !m_pEMN->IsLoading()) {
				m_pEMN->CheckSaveEMNForDrugInteractions(FALSE);
			}
		}

	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2015-04-01 14:09) - PLID 64978 - Added Charge Category 
void CEmrCodesDlg::SetCPTCategoryCombo(NXDATALIST2Lib::IRowSettingsPtr pRow, EMNCharge* pCharge)
{
	try{
		NXDATALIST2Lib::IFormatSettingsPtr pfsLookup(__uuidof(NXDATALIST2Lib::FormatSettings));
		if (pRow){
			pfsLookup = GetCPTMultiCategoryCombo(pCharge);

			if (pfsLookup != NULL){
				pRow->PutRefCellFormatOverride(clcCategory, pfsLookup);
			}
			else{
				ASSERT(FALSE);
			}
		}

	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2015-04-01 14:09) - PLID 64978 -  Gets the Format Settings pointer 
NXDATALIST2Lib::IFormatSettingsPtr CEmrCodesDlg::GetCPTMultiCategoryCombo(EMNCharge* pCharge)
{
	IFormatSettingsPtr pfs(__uuidof(FormatSettings));
	// (s.tullis 2015-04-14 11:36) - PLID 65539 - Templates will never display the category column in the Codes topic.
	//templates do not support saving charge categories.. disable the drop down
	long nCategoryCount = m_bIsTemplate ? 0 : pCharge->nCategoryCount;
	// (s.tullis 2015-04-14 12:01) - PLID 65538 - 
	//If an EMN charge has no categories, say (None) for the category. 
	//If an EMN charge has multiple categories, the ‘no selection’ row should say <Select>, 
	//but the column should never color red, because a selection is not required on EMNs.
	CString strComboSource = FormatString("Select CategoriesT.ID AS ID , CategoriesT.Name AS Text "
		"FROM CategoriesT  "
		"LEFT JOIN ServiceMultiCategoryT ON CategoriesT.ID = ServiceMultiCategoryT.CategoryID "
		"Where ServiceMultiCategoryT.ServiceID = %li OR CategoriesT.ID = %li "
		"UNION "
		"SELECT '-1' AS ID, CASE WHEN %li = 1 THEN  '     <Select>' ELSE  '     (None)' END AS Text "
		"ORDER BY Text ASC", pCharge->nServiceID, pCharge->nCategoryID, long(nCategoryCount > 1 ? TRUE:FALSE));

	pfs->PutDataType(VT_I4);
	pfs->PutFieldType(cftComboSimple);
	pfs->PutEditable(nCategoryCount > 1 ? VARIANT_TRUE : VARIANT_FALSE);
	pfs->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
	pfs->EmbeddedComboDropDownMaxHeight = 300;
	pfs->EmbeddedComboDropDownWidth = 200;
	pfs->PutComboSource(_bstr_t(strComboSource));

	return pfs;
}

// (s.tullis 2015-04-01 14:09) - PLID 64978 - Show charge category if it has multiple catagories configured
void CEmrCodesDlg::ForceShowColumn(short iColumnIndex, long nPercentWidth, long nPixelWidth)
{
	if (nPercentWidth <= 0 || nPixelWidth <= 0 || nPercentWidth >= 100) {
		//You clearly have missed the point of this function.
		//All callers must provide nonzero numbers in both fields,
		//and percent width cannot be > 100.
		ASSERT(FALSE);
		ThrowNxException("CEmrCodesDlg::ForceShowColumn could not size column %li appropriately.", iColumnIndex);
	}

	if (nPercentWidth >= nPixelWidth) {
		//are you sure you didn't enter these numbers backwards?
		ASSERT(FALSE);
	}

	
	IColumnSettingsPtr pCol = m_BillList->GetColumn(iColumnIndex);
	if (pCol == NULL) {
		//should be impossible
		ThrowNxException("CEmrCodesDlg::ForceShowColumn could not find column index %li.", iColumnIndex);
	}

	if (pCol->GetStoredWidth() == 0) {
		//the column is not shown, so show it

		long nStyle = pCol->ColumnStyle;
		if (nStyle & csWidthPercent) {
			//percent width
			pCol->PutStoredWidth(nPercentWidth);
		}
		else {
			//pixel width
			pCol->PutStoredWidth(nPixelWidth);
		}
	}
}
