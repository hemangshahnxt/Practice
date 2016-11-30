// SureScriptsCommDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SureScriptsCommDlg.h"
#include "SureScripts.h"
#include "SOAPUtils.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "RefillRequestDlg.h"
#include "PatientsRc.h"
#include "MultiSelectDlg.h"
#include "MsgBox.h"
#include "PrescriptionEditDlg.h"
#include "PatientView.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// SureScriptsCommDlg dialog
//TES 4/7/2009 - PLID 33882 - Adam created this dialog a while back, but it was empty and unreferenced, 
// I'm now actually implementing this dialog.

IMPLEMENT_DYNAMIC(CSureScriptsCommDlg, CNxDialog)

CSureScriptsCommDlg::CSureScriptsCommDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSureScriptsCommDlg::IDD, pParent)
{
	m_nSelectedID = -1;
	m_bUsePreFilter = false;
	m_bUseProviderFilter = false;
	m_bReadOnlyMode = TRUE;
}

CSureScriptsCommDlg::~CSureScriptsCommDlg()
{
}

void CSureScriptsCommDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_MESSAGE_INFORMATION, m_nxsMessageInformation);
	DDX_Control(pDX, IDC_ACTION_REQUIRED, m_nxeActionRequired); //TES 11/18/2009 - PLID 36353 - Replaced static control with edit control
	DDX_Control(pDX, IDC_BTN_ACTION, m_btnAction);
	DDX_Control(pDX, IDC_VIEW_COMPLETE_MESSAGE, m_btnViewCompleteMessage);
	DDX_Control(pDX, IDC_ACTION_TAKEN, m_nxsActionTaken);
	DDX_Control(pDX, IDC_SHOW_ALL_MESSAGES, m_btnShowAll);
	DDX_Control(pDX, IDC_SHOW_NEEDING_ATTENTION, m_btnShowNeedingAttention);
	DDX_Control(pDX, IDC_FILTER_ALL_PROVIDERS, m_btnFilterAllProviders);
	DDX_Control(pDX, IDC_FILTER_SELECTED_PROVIDERS, m_btnFilterSelectedProviders);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcolor);

	DDX_Control(pDX, IDC_LBL_ACTION_REQUIRED, m_nxsActionRequiredLabel);
	DDX_Control(pDX, IDC_LBL_ACTION_TAKEN, m_nxsActionTakenLabel);
	DDX_Control(pDX, IDC_LBL_MESSAGE_INFO, m_nxsMessageInfoLabel);
	DDX_Control(pDX, IDC_MULTI_PROVIDER_FILTER, m_nxlMultiProviderFilter);
}


BEGIN_MESSAGE_MAP(CSureScriptsCommDlg, CNxDialog)
	ON_BN_CLICKED(IDC_VIEW_COMPLETE_MESSAGE, OnViewCompleteMessage)
	ON_BN_CLICKED(IDC_BTN_ACTION, OnBtnAction)
	ON_BN_CLICKED(IDC_SHOW_ALL_MESSAGES, OnShowAllMessages)
	ON_BN_CLICKED(IDC_SHOW_NEEDING_ATTENTION, OnShowNeedingAttention)
	ON_BN_CLICKED(IDC_FILTER_ALL_PROVIDERS, OnFilterAllProviders)
	ON_BN_CLICKED(IDC_FILTER_SELECTED_PROVIDERS, OnFilterSelectedProviders)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_UPDATEVIEW, OnUpdateView)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// (z.manning 2010-04-21 15:21) - PLID 38318 - Some of these values used to forcibly assign itself to
// a number but those numbers were not adjusted when the patient ID column was added so they were off.
// I removed those manual assignments.
enum SureScriptsMessageListColumns {
	ssmlcID = 0,
	ssmlcMessageID,
	ssmlcParentID,
	ssmlcType,
	ssmlcDescription,
	ssmlcPatientID, // (a.walling 2010-01-25 08:29) - PLID 37026 - Added patient ID column
	ssmlcPatientName,
	ssmlcSentBy,
	ssmlcReceivedBy,
	ssmlcSentReceivedOn,
	ssmlcMessage,
	ssmlcNeedsAttention,
	ssmlcSent,
};

// CSureScriptsCommDlg message handlers
BOOL CSureScriptsCommDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		extern CPracticeApp theApp;
		m_nxsActionRequiredLabel.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		m_nxsActionTakenLabel.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));
		m_nxsMessageInfoLabel.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnViewCompleteMessage.AutoSet(NXB_INSPECT);
		m_btnAction.AutoSet(NXB_MODIFY);

		m_pMessageList = BindNxDataList2Ctrl(IDC_SS_LIST, false);
		m_pProviderFilterList = BindNxDataList2Ctrl(IDC_PROVIDER_FILTER_LIST);

		m_nxlMultiProviderFilter.SetColor(0x00DEB05C);
		m_nxlMultiProviderFilter.SetText("");
		m_nxlMultiProviderFilter.SetType(dtsHyperlink);

		//TES 4/14/2009 - PLID 33888 - Set our filter options.
		//TES 4/21/2009 - PLID 33888 - We'll always default to showing the "Needs Attention" messages, the rest of
		// the filtering has been moved to OnShowWindow().
		CheckRadioButton(IDC_SHOW_ALL_MESSAGES, IDC_SHOW_NEEDING_ATTENTION, IDC_SHOW_NEEDING_ATTENTION);		

	}NxCatchAll("Error in CSureScriptsCommDlg::OnInitDialog()");

	return FALSE;
}

void CSureScriptsCommDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try {
		CNxDialog::OnShowWindow(bShow, nStatus);

		if (!bShow) return;

		// (e.lally 2009-07-10) PLID 34039 - Update the read only mode status for this user
		m_bReadOnlyMode = TRUE;
		//Use get permissions here
		if(GetCurrentUserPermissions(bioSureScriptsMessages) & (sptWrite|sptWriteWithPass) ){
			//They have write permissions
			m_bReadOnlyMode = FALSE;
		}
		
		//TES 4/16/2009 - PLID 33888 - Added provider filtering
		//TES 4/21/2009 - PLID 33888 - This will be the one time we reference our prefilter list.
		if(m_bUsePreFilter) {
			for(int i = 0; i < m_arPreFilterProviderIDs.GetSize(); i++) {
				m_arCurrentFilterProviderIDs.Add(m_arPreFilterProviderIDs[i]);
			}
			if(m_arCurrentFilterProviderIDs.GetSize()) {
				//TES 4/27/2009 - PLID 33888 - We were given providers to filter on, so set our variable to use
				// the provider filter.
				m_bUseProviderFilter = true;
			}
			//TES 4/21/2009 - PLID 33888 - Remember not to use this again (until PreFilter is called again, anyway).
			m_bUsePreFilter = false;
		}
		//TES 5/1/2009 - PLID 34141 - We now enough to determine which providers we want to be able to filter on
		// (specifically, providers with SPIs plus any providers we're already filtering on), so requery the list.
		m_strProvWhere.Format("COALESCE(SPI,'') <> '' OR ID IN (%s) OR ID = -2", ArrayAsString(m_arCurrentFilterProviderIDs, false));
		m_pProviderFilterList->WhereClause = _bstr_t(m_strProvWhere);
		m_pProviderFilterList->Requery();
		//TES 4/21/2009 - PLID 33888 - Now we have our list of providers to filter on, reflect that on screen.
		if(m_bUseProviderFilter) {
			CheckRadioButton(IDC_FILTER_ALL_PROVIDERS, IDC_FILTER_SELECTED_PROVIDERS, IDC_FILTER_SELECTED_PROVIDERS);
			OnFilterSelectedProviders();
		}
		else {
			CheckRadioButton(IDC_FILTER_ALL_PROVIDERS, IDC_FILTER_SELECTED_PROVIDERS, IDC_FILTER_ALL_PROVIDERS);
			OnFilterAllProviders();
		}
		RefreshList();
	}NxCatchAll("Error in CSureScriptsCommDlg::OnShowWindow()");
}

void CSureScriptsCommDlg::RefreshList()
{
	//TES 4/13/2009 - PLID 33882 - Remember the selected ID so we can re-select it when the requery finishes.
	m_nSelectedID = -1;
	if(m_pMessageList->CurSel) {
		m_nSelectedID = VarLong(m_pMessageList->CurSel->GetValue(ssmlcID));
	}

	//TES 4/14/2009 - PLID 33888 - Calculate the where clause based on our filter options
	CString strWhere;
	if(IsDlgButtonChecked(IDC_SHOW_ALL_MESSAGES)) {
		strWhere = "(1=1)";
	}
	else {
		ASSERT(IsDlgButtonChecked(IDC_SHOW_NEEDING_ATTENTION));
		strWhere = "(dbo.GetRootSureScriptsMessage(SureScriptsMessagesT.ID) IN "
			"(SELECT dbo.GetRootSureScriptsMessage(ID) FROM SureScriptsMessagesT WHERE NeedsAttention = 1))";
	}

	//TES 4/16/2009 - PLID 33888 - Added provider filtering
	if(IsDlgButtonChecked(IDC_FILTER_SELECTED_PROVIDERS)) {
		ASSERT(m_arCurrentFilterProviderIDs.GetSize() > 0);
		strWhere += " AND (COALESCE(PersonProvTo.ID,PersonProvFrom.ID) IN (" + ArrayAsString(m_arCurrentFilterProviderIDs) + ") "
			"OR COALESCE(PersonProvTo.ID,PersonProvFrom.ID) Is Null)";
	}
	else {
		strWhere += " AND (1=1)";
	}
	m_pMessageList->WhereClause = _bstr_t(strWhere);
	m_pMessageList->Requery();
}

BEGIN_EVENTSINK_MAP(CSureScriptsCommDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSureScriptsCommDlg)
	ON_EVENT(CSureScriptsCommDlg, IDC_SS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSsList, VTS_I2)
	ON_EVENT(CSureScriptsCommDlg, IDC_SS_LIST, 6 /* RButtonDown */, OnRButtonDownSsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CSureScriptsCommDlg, IDC_SS_LIST, 2 /* SelChanged */, OnSelChangedSsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSureScriptsCommDlg, IDC_PROVIDER_FILTER_LIST, 1 /* SelChanging */, OnSelChangingProviderFilterList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CSureScriptsCommDlg, IDC_PROVIDER_FILTER_LIST, 16 /* SelChosen */, OnSelChosenProviderFilterList, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

#define ROW_NEEDS_ATTENTION_COLOR	RGB(255,127,127)
#define CHILD_ROW_NEEDS_ATTENTION_COLOR	RGB(255,200,200)
void CSureScriptsCommDlg::OnRequeryFinishedSsList(short nFlags)
{
	try {
		//TES 4/8/2009 - PLID 33376 - Go through and fill all the columns that need to be parsed out of the XML
		// (which, at the moment, is just the Description column)
		MSXML2::IXMLDOMDocumentPtr xmlMessage(__uuidof(MSXML2::DOMDocument60));
		MSXML2::IXMLDOMNodePtr xmlMessageBody, xmlMedication, xmlCode;
		CArray<LPDISPATCH,LPDISPATCH> arParentRowsNeedingAttention;
		FOR_ALL_ROWS(m_pMessageList) {
			//TES 4/13/2009 - PLID 33882 - If this is the message we were asked to re-select it, then do so.
			if(m_nSelectedID != -1 && VarLong(pRow->GetValue(ssmlcID)) == m_nSelectedID) {
				m_pMessageList->CurSel = pRow;
				//TES 4/20/2009 - PLID 33882 - Don't call OnSelChanged until we've finished filling in the row.
				//OnSelChangedSsList(NULL, pRow);
				m_nSelectedID = -1;
			}

			//TES 4/8/2009 - PLID 33376 - Get the type, we'll base our description on that.
			SureScripts::MessageType mt = (SureScripts::MessageType)VarLong(pRow->GetValue(ssmlcType));
			CString strDescription;
			switch(mt) {
				case SureScripts::mtPendingRx: // (a.walling 2009-04-23 11:01) - PLID 34032
				case SureScripts::mtNewRx:
				case SureScripts::mtRefillRequest:
				case SureScripts::mtRefillResponse:
				case SureScripts::mtRxChangeRequest:
				case SureScripts::mtRxChangeResponse:
				case SureScripts::mtRxFill:
				case SureScripts::mtCancelRx:
				case SureScripts::mtCancelRxResponse:
					{
						//TES 4/8/2009 - PLID 33376 - Pull the medication name.
						CString strMessage = VarString(pRow->GetValue(ssmlcMessage));
						xmlMessage->loadXML(_bstr_t(VarString(pRow->GetValue(ssmlcMessage))));
						if(xmlMessage) {
							xmlMessageBody = FindChildNode(FindChildNode(xmlMessage, "Message"), "Body");
							if (xmlMessageBody) {
								xmlMedication = FindChildNode(xmlMessageBody->GetfirstChild(), "MedicationPrescribed");
								if (xmlMedication) {
									strDescription = GetXMLNodeText(xmlMedication, "DrugDescription");
								}
							}
						}
					}
					break;
				
				case SureScripts::mtStatus:
					//TES 4/8/2009 - PLID 33376 - Pull the status code.
					{
						CString strMessage = VarString(pRow->GetValue(ssmlcMessage));
						xmlMessage->loadXML(_bstr_t(VarString(pRow->GetValue(ssmlcMessage))));
						if(xmlMessage) {
							xmlMessageBody = FindChildNode(FindChildNode(xmlMessage, "Message"), "Body");
							xmlCode = FindChildNode(xmlMessageBody->GetfirstChild(), "Code");
							CString strCode = (LPCTSTR)xmlCode->text;
							if(strCode == "000") {
								strDescription = "Transaction Pending";
							}
							else if(strCode == "010") {
								strDescription = "Transaction Accepted";
							}
							else {
								ASSERT(FALSE);
								strDescription = "Unknown Status Code " + strCode;
							}
						}
					}
					break;

				case SureScripts::mtError:
					//TES 4/8/2009 - PLID 33376 - Pull the status code.
					{
						CString strMessage = VarString(pRow->GetValue(ssmlcMessage));
						xmlMessage->loadXML(_bstr_t(VarString(pRow->GetValue(ssmlcMessage))));
						if(xmlMessage) {
							xmlMessageBody = FindChildNode(FindChildNode(xmlMessage, "Message"), "Body");
							xmlCode = FindChildNode(xmlMessageBody->GetfirstChild(), "Code");
							CString strCode = (LPCTSTR)xmlCode->text;
							if(strCode == "600") {
								strDescription = "Error: Communication problem - try again later";
							}
							else if(strCode == "601") {
								strDescription = "Error: Receiver unable to process - do not retry";
							}
							else if(strCode == "602") {
								strDescription = "Error: Receiver system error - try again later";
							}
							else if(strCode == "900") {
								strDescription = "Error: Transaction rejected - do not retry";
							}
							else {
								ASSERT(FALSE);
								strDescription = "Error: Unknown Error Code " + strCode;
							}
							CString strDescription1, strDescription2;
							MSXML2::IXMLDOMNodePtr xmlDescriptionCode = FindChildNode(xmlMessageBody->GetfirstChild(), "DescriptionCode");
							if(xmlDescriptionCode) {
								strDescription1 = SureScripts::GetErrorDescription((LPCTSTR)xmlDescriptionCode->text);
							}
							MSXML2::IXMLDOMNodePtr xmlDescription = FindChildNode(xmlMessageBody->GetfirstChild(), "Description");
							if(xmlDescription) {
								strDescription2 = CString((LPCTSTR)xmlDescription->text);;
							}
							strDescription += "\r\nDescription: " + strDescription1;
							if(strDescription1 != strDescription2) {
								strDescription += "\r\nExtended Description: " + strDescription2;
							}
						}
						strDescription.TrimRight("\r\n");
					}
					break;

				case SureScripts::mtVerify:
					//TES 4/8/2009 - PLID 33376 - There isn't really anything to this message, so just describe it.
					strDescription = "Message Receipt Verified";
					break;

				case SureScripts::mtInvalid:
					//TES 4/17/2009 - PLID 33376 - Try to pull the text of the invalid message type.  This will almost
					// certainly fail, because if the message type wasn't invalid, probably the whole message was invalid,
					// and therefore unparseable.  Still, we'll give it a shot, just in case.
					{
						CString strMessage = VarString(pRow->GetValue(ssmlcMessage));
						xmlMessage->loadXML(_bstr_t(VarString(pRow->GetValue(ssmlcMessage))));
						if(xmlMessage) {
							xmlMessageBody = FindChildNode(FindChildNode(xmlMessage, "Message"), "Body");
							if(xmlMessageBody) {
								MSXML2::IXMLDOMNodePtr xmlMessageType = xmlMessageBody->GetfirstChild();
								if(xmlMessageType) {
									strDescription = "Unrecognized Message Type '" + CString((LPCTSTR)xmlMessageType->text) + "'";
								}
							}
						}
						if(strDescription.IsEmpty()) {
							strDescription = "Message received with an unreadable Message Type";
						}
					}
					break;

			}
			pRow->PutValue(ssmlcDescription, _bstr_t(strDescription));

			//TES 4/10/2009 - PLID 33889 - Now, determine whether this row needs attention.
			BOOL bNeedsAttention = VarBool(pRow->GetValue(ssmlcNeedsAttention));
			if(bNeedsAttention) {
				//TES 4/10/2009 - PLID 33889 - Color this row appropriately.
				pRow->PutBackColor(ROW_NEEDS_ATTENTION_COLOR);
				//TES 4/10/2009 - PLID 33889 - Now, color any parents a slightly different color
				// (unless the row itself is already colored).
				NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
				while(pParent) {
					if(pParent->GetBackColor() != ROW_NEEDS_ATTENTION_COLOR) {
						pParent->PutBackColor(CHILD_ROW_NEEDS_ATTENTION_COLOR);
					}
					pParent = pParent->GetParentRow();
				}
			}

			//TES 4/28/2009 - PLID 33882 - Now, if this row doesn't have a patient name, and if any of its parent
			// rows DO have a patient name, then fill the parent's patient name in here.
			if(VarString(pRow->GetValue(ssmlcPatientName),"") == "<Unknown>") {
				CString strParentPatName = "<Unknown>";
				NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
				while(pParent && strParentPatName == "<Unknown>") {
					strParentPatName = VarString(pParent->GetValue(ssmlcPatientName),"");
					if(strParentPatName != "<Unknown>") {
						pRow->PutValue(ssmlcPatientName, _bstr_t(strParentPatName));
					}
					pParent = pParent->GetParentRow();
				}
			}
			
			if(m_pMessageList->CurSel) {
				//TES 4/20/2009 - PLID 33882 - This row was selected, and we've finished filling it in, so reflect
				// it on screen.
				OnSelChangedSsList(NULL, m_pMessageList->CurSel);
			}
			GET_NEXT_ROW(m_pMessageList)
		}

		if(m_pMessageList->CurSel == NULL) {
			//TES 4/14/2009 - PLID 33882 - Nothing was previously selected, or the previously selected row is no longer
			// in the list.  Let's go ahead and select the first row.
			m_pMessageList->CurSel = m_pMessageList->GetFirstRow();
			OnSelChangedSsList(NULL, m_pMessageList->CurSel);
			m_nSelectedID = -1;
		}

	} NxCatchAll("Error in CSureScriptsCommDlg::OnRequeryFinishedSsList()");
}

using namespace NXDATALIST2Lib;
using namespace ADODB;
void CSureScriptsCommDlg::OnRButtonDownSsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		//TES 4/27/2009 - PLID 33882 - Update the selection.
		IRowSettingsPtr pOldSel = m_pMessageList->CurSel;
		m_pMessageList->CurSel = pRow;
		OnSelChangedSsList(pOldSel, pRow);

		// (e.lally 2009-07-10) PLID 34039 - Use a simple check for read only mode to determine if they can attempt
		//	one of the modify actions.
		if(m_bReadOnlyMode){
			return;
		}

		//TES 4/13/2009 - PLID 33890 - Give them options based on the message type.
		SureScripts::MessageType mt = (SureScripts::MessageType)VarLong(pRow->GetValue(ssmlcType));
		switch(mt) {
			case SureScripts::mtError:
				{
					//TES 4/13/2009 - PLID 33890 - They have one option, to acknowledge the error.
					CMenu mnu;
					mnu.CreatePopupMenu();
					mnu.AppendMenu(MF_ENABLED, 1, "Acknowledge Error...");
					CPoint ptClicked(x, y);
					GetDlgItem(IDC_SS_LIST)->ClientToScreen(&ptClicked);
					int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
					if(nResult == 1) {
						//TES 4/13/2009 - PLID 33890 - They do want to acknowledge it, so do so.
						AcknowledgeError();
					}
				}
				break;
			case SureScripts::mtInvalid:
				{
					//TES 4/17/2009 - PLID 33890 - They have one option, to acknowledge the invalid message.
					CMenu mnu;
					mnu.CreatePopupMenu();
					mnu.AppendMenu(MF_ENABLED, 1, "Acknowledge Invalid Message...");
					CPoint ptClicked(x, y);
					GetDlgItem(IDC_SS_LIST)->ClientToScreen(&ptClicked);
					int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
					if(nResult == 1) {
						//TES 4/17/2009 - PLID 33890 - They do want to acknowledge it, so do so.
						AcknowledgeInvalidMessage();
					}
				}
				break;
			default:
				//TES 4/13/2009 - PLID 33890 - There's no action needed for any other message types.
				break;
		}

	}NxCatchAll("Error in CSureScriptsCommDlg::OnRButtonDownSsList()");
}

void CSureScriptsCommDlg::OnSelChangedSsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//TES 4/13/2009 - PLID 33882 - Fill in the information on the bottom half of the dialog, based on the currently
		// selected message.
		IRowSettingsPtr pNewSel(lpNewSel);
		if(pNewSel == NULL) {
			//TES 4/13/2009 - PLID 33882 - Clear everything out
			SetDlgItemText(IDC_MESSAGE_INFORMATION, "");
			SetActionRequired("");
			SetDlgItemText(IDC_ACTION_TAKEN, "");
			m_btnAction.EnableWindow(FALSE);
			m_btnAction.ShowWindow(SW_HIDE);
			m_btnViewCompleteMessage.EnableWindow(FALSE);
			m_btnViewCompleteMessage.ShowWindow(SW_HIDE);
		}
		else {
			//TES 4/13/2009 - PLID 33882 - Update the information area.
			_variant_t varSentReceived = pNewSel->GetValue(ssmlcSentReceivedOn);
			CString strSentReceived;
			if (varSentReceived.vt == VT_DATE) {
				strSentReceived = FormatDateTimeForInterface(VarDateTime(varSentReceived));
			} else {
				strSentReceived = "Pending";
			}

			CString strInfo;
			SureScripts::MessageType mt = (SureScripts::MessageType)VarLong(pNewSel->GetValue(ssmlcType));
			strInfo.Format("From: %s\r\n"
				"To: %s\r\n"
				"Delivered: %s\r\n"
				"Type: %s\r\n"
				"Description:\r\n%s\r\n",
				VarString(pNewSel->GetValue(ssmlcSentBy)), VarString(pNewSel->GetValue(ssmlcReceivedBy)),
				strSentReceived,
				SureScripts::GetMessageTypeDescription(mt),
				VarString(pNewSel->GetValue(ssmlcDescription)));
			SetDlgItemText(IDC_MESSAGE_INFORMATION, strInfo);

			m_btnViewCompleteMessage.EnableWindow(TRUE);
			m_btnViewCompleteMessage.ShowWindow(SW_SHOWNA);

			//TES 4/13/2009 - PLID 33882 - Set any actions that are needed
			BOOL bNeedsAttention = VarBool(pNewSel->GetValue(ssmlcNeedsAttention));
			switch(mt) {
				case SureScripts::mtPendingRx: // (a.walling 2009-04-23 11:01) - PLID 34032
				case SureScripts::mtNewRx:
					if (bNeedsAttention) {
						// (a.walling 2009-04-21 15:27) - PLID 34032 - A new prescription will only be here
						// if the message failed to generate. So basically, the only action is to edit, and they
						// can either try it again or remove it from being e-prescribed.
						if (mt == SureScripts::mtPendingRx) {
							try {
								MSXML2::IXMLDOMDocumentPtr xmlMessage(__uuidof(MSXML2::DOMDocument60));
								xmlMessage->loadXML(_bstr_t(pNewSel->GetValue(ssmlcMessage)));

								CString strErrorMessage = GetXMLNodeText(FindChildNode(FindChildNode(FindChildNode(xmlMessage, "Message"), "Body"), "NewRx"), "ErrorDescription");

								if (!strErrorMessage.IsEmpty()) {									
									SetActionRequired(FormatString(
										"Edit and Resubmit Prescription to resolve errors:\r\n%s", strErrorMessage));
								}
							} catch(_com_error e) {
								SetActionRequired("Edit and Resubmit Prescription");
							}
						} else {
							SetActionRequired("Edit and Resubmit Prescription");
						}

						m_btnAction.SetWindowText("Edit...");
						// (e.lally 2009-07-10) PLID 34039 - Use a simple check for read only mode to determine if they can attempt 
						//	the Edit action.
						m_btnAction.EnableWindow(!m_bReadOnlyMode);
						m_btnAction.ShowWindow(SW_SHOWNA);
					} else {
						SetActionRequired("<None>");

						m_btnAction.EnableWindow(FALSE);
						m_btnAction.ShowWindow(SW_HIDE);
					}

					{
						// (a.walling 2009-04-21 17:43) - PLID 34032 - We're going to need to make a quick trip to the database to get
						// info about who acknowledged the newrx previously.
						_RecordsetPtr rsAction = CreateParamRecordset("SELECT TOP 1 ActionType, UsersT.UserName, Date "
							"FROM SureScriptsActionsT INNER JOIN UsersT ON SureScriptsActionsT.UserID = UsersT.PersonID "
							"WHERE MessageID = {INT} ORDER BY Date DESC",
							VarLong(pNewSel->GetValue(ssmlcID)));
						if(rsAction->eof) {
							SetDlgItemText(IDC_ACTION_TAKEN, "<None>");
						}
						else {
							CString strAction;
							strAction.Format("%s by %s on %s",
								SureScripts::GetActionTypeDescription((SureScripts::ActionType)AdoFldLong(rsAction, "ActionType")),
								AdoFldString(rsAction, "UserName"), 
								FormatDateTimeForInterface(AdoFldDateTime(rsAction, "Date")));
							SetDlgItemText(IDC_ACTION_TAKEN, strAction);
						}						
					}
					break;
				case SureScripts::mtError:
					//TES 4/13/2009 - PLID 33890 - Give them the option to acknowledge the error.
					if(bNeedsAttention) {
						//TES 4/13/2009 - PLID 33890 - It hasn't been acknowledged yet, they need to.
						SetActionRequired("Acknowledge Error");
						SetDlgItemText(IDC_ACTION_TAKEN, "<None>");
					}
					else {
						//TES 4/13/2009 - PLID 33890 - It's been acknowledged, so they don't need to do anything.
						SetActionRequired("<None>");

						//TES 4/13/2009 - PLID 33890 - We're going to need to make a quick trip to the database to get
						// info about who acknowledged the error previously.
						_RecordsetPtr rsAction = CreateParamRecordset("SELECT TOP 1 ActionType, UsersT.UserName, Date "
							"FROM SureScriptsActionsT INNER JOIN UsersT ON SureScriptsActionsT.UserID = UsersT.PersonID "
							"WHERE MessageID = {INT} ORDER BY Date DESC",
							VarLong(pNewSel->GetValue(ssmlcID)));
						if(rsAction->eof) {
							SetDlgItemText(IDC_ACTION_TAKEN, "<None>");
						}
						else {
							CString strAction;
							strAction.Format("%s by %s on %s",
								SureScripts::GetActionTypeDescription((SureScripts::ActionType)AdoFldLong(rsAction, "ActionType")),
								AdoFldString(rsAction, "UserName"), 
								FormatDateTimeForInterface(AdoFldDateTime(rsAction, "Date")));
							SetDlgItemText(IDC_ACTION_TAKEN, strAction);
						}
					}
					m_btnAction.SetWindowText("Acknowledge...");
					// (e.lally 2009-07-10) PLID 34039 - Use a simple check for read only mode to determine if they can attempt
					//	the Acknowledge error message action.
					m_btnAction.EnableWindow(!m_bReadOnlyMode);
					m_btnAction.ShowWindow(SW_SHOWNA);
					break;
				case SureScripts::mtRefillRequest:
					// (a.walling 2009-04-16 16:35) - PLID 33951 - Handle refill requests
					//TES 4/13/2009 - PLID 33890 - Give them the option to acknowledge the error.
					if(bNeedsAttention) {
						//TES 4/13/2009 - PLID 33890 - It hasn't been acknowledged yet, they need to.
						SetActionRequired("Respond to Refill Request");
						SetDlgItemText(IDC_ACTION_TAKEN, "<None>");

						m_btnAction.SetWindowText("Respond...");
						// (e.lally 2009-07-10) PLID 34039 - Use a simple check for read only mode to determine if they can attempt
						//	the Respond action
						m_btnAction.EnableWindow(!m_bReadOnlyMode);
						m_btnAction.ShowWindow(SW_SHOWNA);
					}
					else {
						//TES 4/13/2009 - PLID 33890 - It's been acknowledged, so they don't need to do anything.
						SetActionRequired("<None>");

						//TES 4/13/2009 - PLID 33890 - We're going to need to make a quick trip to the database to get
						// info about who acknowledged the error previously.
						_RecordsetPtr rsAction = CreateParamRecordset("SELECT TOP 1 ActionType, UsersT.UserName, Date "
							"FROM SureScriptsActionsT INNER JOIN UsersT ON SureScriptsActionsT.UserID = UsersT.PersonID "
							"WHERE MessageID = {INT} ORDER BY Date DESC",
							VarLong(pNewSel->GetValue(ssmlcID)));
						if(rsAction->eof) {
							SetDlgItemText(IDC_ACTION_TAKEN, "<None>");
						}
						else {
							CString strAction;
							strAction.Format("%s by %s on %s",
								SureScripts::GetActionTypeDescription((SureScripts::ActionType)AdoFldLong(rsAction, "ActionType")),
								AdoFldString(rsAction, "UserName"), 
								FormatDateTimeForInterface(AdoFldDateTime(rsAction, "Date")));
							SetDlgItemText(IDC_ACTION_TAKEN, strAction);
						}
						
						m_btnAction.EnableWindow(FALSE);
						m_btnAction.ShowWindow(SW_HIDE);
					}

					break;
				case SureScripts::mtInvalid:
					//TES 4/17/2009 - PLID 33890 - Give them the option to acknowledge the invalid message
					if(bNeedsAttention) {
						//TES 4/17/2009 - PLID 33890 - It hasn't been acknowledged yet, they need to.
						SetActionRequired("Acknowledge Invalid Message");
						SetDlgItemText(IDC_ACTION_TAKEN, "<None>");
					}
					else {
						//TES 4/17/2009 - PLID 33890 - It's been acknowledged, so they don't need to do anything.
						SetActionRequired("<None>");

						//TES 4/17/2009 - PLID 33890 - We're going to need to make a quick trip to the database to get
						// info about who acknowledged the message previously.
						_RecordsetPtr rsAction = CreateParamRecordset("SELECT TOP 1 ActionType, UsersT.UserName, Date "
							"FROM SureScriptsActionsT INNER JOIN UsersT ON SureScriptsActionsT.UserID = UsersT.PersonID "
							"WHERE MessageID = {INT} ORDER BY Date DESC",
							VarLong(pNewSel->GetValue(ssmlcID)));
						if(rsAction->eof) {
							SetDlgItemText(IDC_ACTION_TAKEN, "<None>");
						}
						else {
							CString strAction;
							strAction.Format("%s by %s on %s",
								SureScripts::GetActionTypeDescription((SureScripts::ActionType)AdoFldLong(rsAction, "ActionType")),
								AdoFldString(rsAction, "UserName"), 
								FormatDateTimeForInterface(AdoFldDateTime(rsAction, "Date")));
							SetDlgItemText(IDC_ACTION_TAKEN, strAction);
						}
					}
					m_btnAction.SetWindowText("Acknowledge...");
					// (e.lally 2009-07-10) PLID 34039 - Use a simple check for read only mode to determine if they can attempt
					//	the Acknowledge invalid message action.
					m_btnAction.EnableWindow(!m_bReadOnlyMode);
					m_btnAction.ShowWindow(SW_SHOWNA);
					break;
				default:
					//TES 4/13/2009 - PLID 33882 - No other message types require an action.
					SetActionRequired("<None>");
					SetDlgItemText(IDC_ACTION_TAKEN, "");
					m_btnAction.EnableWindow(FALSE);
					m_btnAction.ShowWindow(SW_HIDE);
					break;
			}
		}	
	}NxCatchAll("Error in CSureScriptsCommDlg::OnSelChangedSsList()");
}

void CSureScriptsCommDlg::OnViewCompleteMessage()
{
	try {
		IRowSettingsPtr pRow = m_pMessageList->CurSel;
		if(pRow == NULL) {
			return;
		}

		CMsgBox msgbox(this);
		CString strXML = VarString(pRow->GetValue(ssmlcMessage));

		// (a.walling 2009-04-20 14:18) - PLID 33951 - Pretty print the XML so it's more legible
		try {
			MSXML2::IXMLDOMDocumentPtr pDoc(__uuidof(MSXML2::DOMDocument60));
			pDoc->loadXML(AsBstr(strXML));
			msgbox.msg = PrettyPrintXML(pDoc);
			msgbox.DoModal();
			return;
		} NxCatchAll("Error formatting XML");

		// if we got an error, fall back here.

		//TES 4/13/2009 - PLID 33882 - Just pop up the full text of the message, this may be helpful at times.		
		// (a.walling 2009-04-20 14:18) - PLID 33951 - Made this a CMsgBox so it's easier to copy and paste
		msgbox.msg = strXML;
		msgbox.DoModal();

	}NxCatchAll("Error in CSureScriptsCommDlg::OnViewCompleteMessage()");
}

void CSureScriptsCommDlg::OnBtnAction()
{
	try {
		IRowSettingsPtr pRow = m_pMessageList->CurSel;
		if(pRow == NULL) {
			return;
		}
		
		//TES 4/14/2009 - PLID 33890 - Determine the action based on the message type.
		SureScripts::MessageType mt = (SureScripts::MessageType)VarLong(pRow->GetValue(ssmlcType));
		switch(mt) {
			case SureScripts::mtPendingRx:
			case SureScripts::mtNewRx:
				EditPrescription();
				break;
			case SureScripts::mtError:
				AcknowledgeError();
				break;
			case SureScripts::mtRefillRequest:
				RespondToRefillRequest();
				break;
			case SureScripts::mtInvalid: //TES 4/17/2009 - PLID 33890
				AcknowledgeInvalidMessage();
				break;
			default:
				//No other types have actions.
				break;
		}
	}NxCatchAll("Error in CSureScriptsCommDlg::OnBtnAction()");
}

// (a.walling 2009-04-21 15:31) - PLID 34032 - Edit the prescription
void CSureScriptsCommDlg::EditPrescription()
{
	IRowSettingsPtr pRow = m_pMessageList->CurSel;
	if(pRow == NULL) {
		//How did this function get called?
		ASSERT(FALSE);
		return;
	}
	// (e.lally 2009-07-10) PLID 34039 - Check permissions
	CPermissions permsPatientMeds = GetCurrentUserPermissions(bioPatientMedication);
	CPermissions permsSureScriptsMessage = GetCurrentUserPermissions(bioSureScriptsMessages);
	// (e.lally 2009-07-10) PLID 34039 - Make sure we have one of the read/write permissions for meds and one of the write permissions for this dlg
	if( !(permsPatientMeds & (sptRead|sptReadWithPass|sptWrite|sptWriteWithPass)) ||
		!(permsSureScriptsMessage & (sptWrite|sptWriteWithPass)) ){
		//They are missing a permission so we cannot continue
		// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
		// messageboxes with PermissionsFailedMessageBox
		PermissionsFailedMessageBox();
		return;
	}

	// (e.lally 2009-07-10) PLID 34039 - See if any of this user's permissions require a password
	BOOL bAssumePassKnown = FALSE;
	if( (permsPatientMeds & (sptReadWithPass|sptWriteWithPass)) ||
		(permsSureScriptsMessage & (sptWriteWithPass)) ){
		//We need a password for one of these permissions
		if(!CheckCurrentUserPassword()){
			return;
		}
		else {
			bAssumePassKnown = TRUE;
		}
	}

	// (e.lally 2009-07-10) PLID 34039 - Does the user have read permissions
	if(!CheckCurrentUserPermissions(bioPatientMedication, sptRead, 0, 0, 0, bAssumePassKnown) ){
		//This is only possible if the user has write but not read permission.
		return;
	}
	//We know we can read medications if we get this far.

	// (e.lally 2009-07-10) PLID 34039 - See if they have the write permission for medications too
	BOOL bCanEditMeds = FALSE;
	if( (permsPatientMeds & (sptWrite|sptWriteWithPass)) ){
		bCanEditMeds = TRUE;
	}

	long nMessageID = VarLong(pRow->GetValue(ssmlcID), -1);

	if (nMessageID != -1) {
		_RecordsetPtr prs = CreateParamRecordset("SELECT PatientID, PatientMedicationID FROM SureScriptsMessagesT WHERE ID = {INT}", nMessageID);
		
		if (!prs->eof) {
			long nPatientID = AdoFldLong(prs, "PatientID", -1);
			long nPrescriptionID = AdoFldLong(prs, "PatientMedicationID", -1);

			if (nPatientID == -1 || nPrescriptionID == -1) {
				ThrowNxException("Cannot edit prescription: cannot find patient or prescription");
			}

			CPrescriptionEditDlg dlg(this);

			// (e.lally 2009-07-10) PLID 34039 - Send in a read only flag if they can't edit.
			// (j.jones 2012-11-16 11:34) - PLID 53765 - this now returns an enum of whether we edited or deleted a prescription
			// (j.fouts 2013-03-12 10:18) - PLID 52973 - Seperated Loading a prescription out from the prescription Edit Dlg
			// (j.jones 2013-11-25 09:55) - PLID 59772 - this does not need the drug interaction info
			// (j.jones 2016-01-06 15:40) - PLID 67823 - filled bIsNewRx
			PrescriptionDialogReturnValue epdrvReturn = (PrescriptionDialogReturnValue)dlg.EditPrescription(false, LoadFullPrescription(nPrescriptionID), NULL, !bCanEditMeds);
			//check whether we deleted, or really made a change
			if(epdrvReturn == epdrvDeleteRx || dlg.GetChangesMade()) {
				//TES 4/13/2009 - PLID 33890 - Refresh the screen
				RefreshList();

				//TES 4/13/2009 - PLID 33890 - Refresh everyone.
				CClient::RefreshTable(NetUtils::SureScriptsMessagesT);

				// (a.walling 2009-04-28 16:06) - PLID 34032 - Refresh the patient medications tab if necessary
				try {
					if (nPatientID == GetActivePatientID()) {
						CNxView* pView = GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
						if (pView) {
							((CPatientView*)pView)->UpdateView();
						}
					}
				} NxCatchAll("Error refreshing patient medications");
			}
		}
	}
}

void CSureScriptsCommDlg::AcknowledgeError()
{
	IRowSettingsPtr pRow = m_pMessageList->CurSel;
	if(pRow == NULL) {
		//How did this function get called?
		ASSERT(FALSE);
		return;
	}
	// (e.lally 2009-07-10) PLID 34039 - Check the SureScripts edit permission
	if(!CheckCurrentUserPermissions(bioSureScriptsMessages, sptWrite)){
		return;
	}


	//TES 4/13/2009 - PLID 33890 - Confirm that they know what they're doing.
	CString strDate = "<Pending>";
	_variant_t varDate = pRow->GetValue(ssmlcSentReceivedOn);
	if(varDate.vt == VT_DATE) {
		strDate = FormatDateTimeForInterface(VarDateTime(varDate));
	}
	CString strFrom = VarString(pRow->GetValue(ssmlcSentBy));
	CString strTo = VarString(pRow->GetValue(ssmlcReceivedBy));
	CString strContact = VarBool(pRow->GetValue(ssmlcSent))?strTo:strFrom;
	if(!strContact.IsEmpty()) {
		strContact = "  If you are not sure what this message means, please call " + strContact + ", or NexTech Technical Support.";
	}
	if(IDYES == MsgBox(MB_YESNO, "The following SureScripts error message was sent from %s to %s on %s:\r\n"
		"\r\n"
		"%s\r\n"
		"\r\nDo you wish to acknowledge this message, indicating that you have read it and taken any "
		"necessary action?%s", strFrom.IsEmpty()?"<None>":strFrom, strTo.IsEmpty()?"<None>":strTo,
		strDate, VarString(pRow->GetValue(ssmlcDescription)), 
		strContact)) {
			//TES 4/13/2009 - PLID 33890 - Update the data
			long nID = VarLong(pRow->GetValue(ssmlcID));
			// (a.walling 2010-01-25 08:33) - PLID 37026 - Pass in the patient ID
			SureScripts::CreateAction(nID, SureScripts::atErrorAcknowledged, SureScripts::GetActionTypeDescription(SureScripts::atErrorAcknowledged) + " for '" + VarString(pRow->GetValue(ssmlcDescription)) + "'", VarLong(pRow->GetValue(ssmlcPatientID)), VarString(pRow->GetValue(ssmlcPatientName), ""), TRUE);
			
			//TES 4/13/2009 - PLID 33890 - Refresh the screen
			RefreshList();

			//TES 4/13/2009 - PLID 33890 - Refresh everyone.
			CClient::RefreshTable(NetUtils::SureScriptsMessagesT);
	}
}

void CSureScriptsCommDlg::AcknowledgeInvalidMessage()
{
	IRowSettingsPtr pRow = m_pMessageList->CurSel;
	if(pRow == NULL) {
		//How did this function get called?
		ASSERT(FALSE);
		return;
	}

	// (e.lally 2009-07-10) PLID 34039 - Check the SureScripts edit permission
	if(!CheckCurrentUserPermissions(bioSureScriptsMessages, sptWrite)){
		return;
	}

	//TES 4/17/2009 - PLID 33890 - Confirm that they know what they're doing.
	CString strDate = "<Pending>";
	_variant_t varDate = pRow->GetValue(ssmlcSentReceivedOn);
	if(varDate.vt == VT_DATE) {
		strDate = FormatDateTimeForInterface(VarDateTime(varDate));
	}
	CString strFrom = VarString(pRow->GetValue(ssmlcSentBy));
	CString strTo = VarString(pRow->GetValue(ssmlcReceivedBy));
	CString strContact = VarBool(pRow->GetValue(ssmlcSent))?strTo:strFrom;
	if(!strContact.IsEmpty()) {
		strContact = "  If you are not sure why this invalid message mean was sent, please call " + strContact + ", or NexTech Technical Support.";
	}
	if(IDYES == MsgBox(MB_YESNO, "The following invalid SureScripts message was sent from %s to %s on %s:\r\n"
		"\r\n"
		"%s\r\n"
		"\r\nDo you wish to acknowledge this message, indicating that you have seen it and taken any "
		"necessary action?%s", strFrom.IsEmpty()?"<None>":strFrom, strTo.IsEmpty()?"<None>":strTo,
		strDate, VarString(pRow->GetValue(ssmlcMessage)), 
		strContact)) {
			//TES 4/17/2009 - PLID 33890 - Update the data
			long nID = VarLong(pRow->GetValue(ssmlcID));
			// (a.walling 2010-01-25 08:33) - PLID 37026 - Pass in the patient ID
			SureScripts::CreateAction(nID, SureScripts::atInvalidMessageAcknowledged, SureScripts::GetActionTypeDescription(SureScripts::atInvalidMessageAcknowledged) + " for '" + VarString(pRow->GetValue(ssmlcDescription)) + "'", VarLong(pRow->GetValue(ssmlcPatientID)), VarString(pRow->GetValue(ssmlcPatientName), ""), TRUE);
			
			//TES 4/13/2009 - PLID 33890 - Refresh the screen
			RefreshList();

			//TES 4/13/2009 - PLID 33890 - Refresh everyone.
			CClient::RefreshTable(NetUtils::SureScriptsMessagesT);
	}
}

// (a.walling 2009-04-16 17:04) - PLID 33951
void CSureScriptsCommDlg::RespondToRefillRequest()
{
	try {
		IRowSettingsPtr pRow = m_pMessageList->CurSel;
		if(pRow == NULL) {
			//How did this function get called?
			ASSERT(FALSE);
			return;
		}

		// (e.lally 2009-07-10) PLID 34039 - Check permissions
		CPermissions permsPatientMeds = GetCurrentUserPermissions(bioPatientMedication);
		CPermissions permsSureScriptsMessage = GetCurrentUserPermissions(bioSureScriptsMessages);
		// (e.lally 2009-07-10) PLID 34039 - Make sure we have one of the create permissions for meds 
		//	and one of the write permissions for the SureScripts messages
		if( !(permsPatientMeds & (sptCreate|sptCreateWithPass)) ||
			!(permsSureScriptsMessage & (sptWrite|sptWriteWithPass)) ){
			//They are missing a permission
			// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
			// messageboxes with PermissionsFailedMessageBox
			PermissionsFailedMessageBox();
			return;
		}

		// (e.lally 2009-07-10) PLID 34039 - Once we get this far, we know they have the proper permissions,
		//	but we may need to verify their password first.

		// (e.lally 2009-07-10) PLID 34039 - Do we need to prompt for a password? Only prompt once.
		if( (permsPatientMeds & (sptCreateWithPass)) ||
			(permsSureScriptsMessage & (sptWriteWithPass)) ){
			//We need a password for one of these permissions
			if(!CheckCurrentUserPassword()){
				return;
			}
		}


		long nID = VarLong(pRow->GetValue(ssmlcID));

		MSXML2::IXMLDOMDocument2Ptr pMessage(__uuidof(MSXML2::DOMDocument60));
		pMessage->loadXML(_bstr_t(pRow->GetValue(ssmlcMessage)));

		CRefillRequestDlg dlg(this);
		dlg.m_pMessage = pMessage;

		if (IDOK == dlg.DoModal()) {
			// (a.walling 2010-01-25 08:33) - PLID 37026 - Pass in the patient ID
			SureScripts::CreateAction(nID, dlg.m_atAction, GetActionTypeDescription(dlg.m_atAction) + " for '" + VarString(pRow->GetValue(ssmlcDescription)) + "'", VarLong(pRow->GetValue(ssmlcPatientID)), VarString(pRow->GetValue(ssmlcPatientName), ""), TRUE);
			// (a.walling 2009-04-28 14:47) - PLID 33951 - Need to refresh afterwards!
			RefreshList();
		}
	} NxCatchAll("Error responding to refill request");
}

// (a.walling 2009-04-21 16:32) - PLID 34032 - Moved to SureScripts namespace in SureScriptsPractice
/*
void CSureScriptsCommDlg::CreateAction(long nID, SureScripts::ActionType actionType, const CString& strDescription, const CString &strPatientName)
{
	AuditEventItems aeiItem = SureScripts::GetAuditItemForAction(actionType);

//TES 4/13/2009 - PLID 33890 - Update the data
	ExecuteParamSql("UPDATE SureScriptsMessagesT SET NeedsAttention = 0 WHERE ID = {INT};"
		"INSERT INTO SureScriptsActionsT (MessageID, ActionType, UserID, Date) "
		"VALUES ({INT}, {INT}, {INT}, getdate())",
		nID, nID, (int)actionType, GetCurrentUserID());
	//TES 4/13/2009 - PLID 33890 - Audit
	AuditEvent(strPatientName, BeginNewAuditEvent(), aeiItem, nID, "", 
		strDescription, aepMedium);
}
*/

void CSureScriptsCommDlg::OnShowAllMessages()
{
	try {
		//TES 4/14/2009 - PLID 33888 - We just need to refresh the list.
		RefreshList();
	}NxCatchAll("Error in CSureScriptsCommDlg::OnShowAllMessages()");
}

void CSureScriptsCommDlg::OnShowNeedingAttention()
{
	try {
		//TES 4/14/2009 - PLID 33888 - We just need to refresh the list.
		RefreshList();
	}NxCatchAll("Error in CSureScriptsCommDlg::OnShowNeedingAttention()");
}

void CSureScriptsCommDlg::OnFilterAllProviders()
{
	try {
		//TES 4/16/2009 - PLID 33888 - Disable the provider filtering options.
		GetDlgItem(IDC_PROVIDER_FILTER_LIST)->EnableWindow(FALSE);
		GetDlgItem(IDC_MULTI_PROVIDER_FILTER)->EnableWindow(FALSE);
		m_nxlMultiProviderFilter.SetType(dtsDisabledHyperlink);
		m_nxlMultiProviderFilter.Invalidate();

		//TES 4/27/2009 - PLID 33888 - We're no longer filtering on providers (even though there still may be IDs
		// in our array).
		m_bUseProviderFilter = false;

		//TES 4/16/2009 - PLID 33888 - Refresh
		RefreshList();
	} NxCatchAll("Error in CSureScriptsCommDlg::OnFilterAllProviders()");
}

enum ProviderFilterListColumns
{
	pflcID = 0,
	pflcName = 1,
	pflcSPI = 2,
};

void CSureScriptsCommDlg::OnFilterSelectedProviders()
{
	try {
		//TES 4/16/2009 - PLID 33888 - Enable our provider filtering options
		GetDlgItem(IDC_PROVIDER_FILTER_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_MULTI_PROVIDER_FILTER)->EnableWindow(TRUE);

		//TES 4/27/2009 - PLID 33888 - We're now filtering on providers.
		m_bUseProviderFilter = true;

		//TES 5/1/2009 - PLID 34141 - Make sure the list is requeried.
		m_pProviderFilterList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		//TES 5/1/2009 - PLID 34141 - Check whether there aren't any providers (much more likely now that we only
		// show providers with SPIs).
		if(m_pProviderFilterList->GetRowCount() == 1) {
			//TES 5/1/2009 - PLID 34141 - There's only one row, and that's the <Multiple> row, so set them back
			// to All Providers.
			MsgBox("You do not have any providers with valid SPIs.  In order to use SureScripts Integration, please fill in the SPI field on one or more providers.");
			m_bUseProviderFilter = false;
			CheckRadioButton(IDC_FILTER_ALL_PROVIDERS, IDC_FILTER_SELECTED_PROVIDERS, IDC_FILTER_ALL_PROVIDERS);
			OnFilterAllProviders();
			return;
		}
		//TES 4/16/2009 - PLID 33888 - Check what's currently selected
		IRowSettingsPtr pCurrentProvRow = m_pProviderFilterList->CurSel;
		//TES 4/16/2009 - PLID 33888 - If nothing's selected in the dropdown, select an appropriate provider.
		if (pCurrentProvRow == NULL) {
			if(m_arCurrentFilterProviderIDs.GetSize()) {
				//TES 4/17/2009 - PLID 33888 - They've already selected one or more providers (we may have been pre-filtered).
				// Select accordingly.
				if(m_arCurrentFilterProviderIDs.GetSize() > 1) {
					//TES 4/17/2009 - PLID 33888 - Select the <Multiple> row, code farther down will handle the rest.
					m_pProviderFilterList->SetSelByColumn(pflcID, (long)-2);
				}
				else {
					//TES 4/17/2009 - PLID 33888 - They're filtering on one provider, select it.
					m_pProviderFilterList->SetSelByColumn(pflcID, m_arCurrentFilterProviderIDs[0]);
				}
			}
			else {
				//TES 4/16/2009 - PLID 33888 - Select the first row that has a non-negative ID and, if possible, a non-blank SPI.
				bool bSPIFound = false;
				IRowSettingsPtr pRow = m_pProviderFilterList->GetFirstRow();
				IRowSettingsPtr pNewRow = NULL;
				while(pRow && !bSPIFound) {
					long nProvID = VarLong(pRow->GetValue(pflcID));
					if(nProvID > 0) {
						//TES 4/16/2009 - PLID 33888 - This is a candidate, does it have an SPI?
						CString strSPI = VarString(pRow->GetValue(pflcSPI),"");
						if(!strSPI.IsEmpty()) {
							//Yes, this is it!
							bSPIFound = true;
							pNewRow = pRow;
						}
						else {
							//No, but it's still a candidate if we haven't found any others yet.
							if(pNewRow == NULL) pNewRow = pRow;
						}
					}
					pRow = pRow->GetNextRow();
				}
				m_pProviderFilterList->CurSel = pNewRow;
			}
		}
		//TES 4/16/2009 - PLID 33888 - Handle the selection change.
		if(m_pProviderFilterList->CurSel == NULL) {
			//TES 4/16/2009 - PLID 33888 - No selection, that shouldn't be possible!
			ASSERT(FALSE);
			return;
		}
		long nID = VarLong(m_pProviderFilterList->CurSel->GetValue(pflcID));
		if(nID == -2) {
			//TES 4/16/2009 - PLID 33888 - They selected "<Multiple Providers...>", so make sure
			// the label is displayed (and datalist hidden) correctly.
			ShowDlgItem(IDC_PROVIDER_FILTER_LIST, SW_HIDE);
			CString strIDList, strID, strProviderList;
			for(int i=0; i < m_arCurrentFilterProviderIDs.GetSize(); i++) {
				strID.Format("%li, ", m_arCurrentFilterProviderIDs.GetAt(i));
				strIDList += strID;
			}
			strIDList = strIDList.Left(strIDList.GetLength()-2);
			_RecordsetPtr rsProviders = CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name "
				"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
				"WHERE PersonT.ID IN (%s) ORDER BY Name ASC", strIDList);
			while(!rsProviders->eof) {
				strProviderList += AdoFldString(rsProviders, "Name") + ", ";
				rsProviders->MoveNext();
			}
			rsProviders->Close();
			strProviderList = strProviderList.Left(strProviderList.GetLength()-2);
			m_nxlMultiProviderFilter.SetText(strProviderList);
			m_nxlMultiProviderFilter.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MULTI_PROVIDER_FILTER, SW_SHOWNA);
			InvalidateDlgItem(IDC_MULTI_PROVIDER_FILTER);
		}
		else {
			//TES 4/16/2009 - PLID 33888 - They selected a provider, ensure our member variable is
			// up to date.
			m_arCurrentFilterProviderIDs.RemoveAll();
			m_arCurrentFilterProviderIDs.Add(nID);
			m_nxlMultiProviderFilter.SetType(dtsDisabledHyperlink);
			ShowDlgItem(IDC_MULTI_PROVIDER_FILTER, SW_HIDE);
			ShowDlgItem(IDC_PROVIDER_FILTER_LIST, SW_SHOWNA);
		}

		//TES 4/16/2009 - PLID 33888 - Refresh the list
		RefreshList();

	} NxCatchAll("Error in CSureScriptsCommDlg::OnFilterSelectedProviders()");
}

void CSureScriptsCommDlg::OnSelChangingProviderFilterList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			//TES 4/16/2009 - PLID 33888 - Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CSureScriptsCommDlg::OnSelChangingProviderFilterList()");
}

void CSureScriptsCommDlg::OnSelChosenProviderFilterList(LPDISPATCH lpRow) 
{
	try {
		IRowSettingsPtr pNewSel(lpRow);
		if(pNewSel == NULL) {
			//No selection, that shouldn't be possible!
			ASSERT(FALSE);
			return;
		}
		long nID = VarLong(pNewSel->GetValue(pflcID));
		if(nID == -2) {
			//TES 4/16/2009 - PLID 33888 - They selected "<Multiple Providers...>", so handle that.
			// Don't call OnMultiProviderFilter() directly, we need to let this SelChanged event finish happening.
			PostMessage(NXM_NXLABEL_LBUTTONDOWN, (WPARAM)IDC_MULTI_PROVIDER_FILTER);
		}
		else {
			//TES 4/16/2009 - PLID 33888 - They selected a provider, update our member variable.
			m_arCurrentFilterProviderIDs.RemoveAll();
			m_arCurrentFilterProviderIDs.Add(nID);

			//TES 4/16/2009 - PLID 33888 - Refresh the list().
			RefreshList();
		}

	}NxCatchAll("Error in CSureScriptsCommDlg::OnSelChangedProviderFilterList()");
}

void CSureScriptsCommDlg::OnMultiProviderFilter()
{
	try {
		//TES 4/16/2009 - PLID 33888 - Prompt them for which providers to use.
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
		dlg.PreSelect(m_arCurrentFilterProviderIDs);
		//TES 5/1/2009 - PLID 34141 - Pass in our current filter for providers.
		if(IDOK == dlg.Open("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID", m_strProvWhere, "PersonT.ID", 
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle", "Select one or more providers to filter on:", 1)) {
			
			//TES 4/16/2009 - PLID 33888 - Grab the IDs
			dlg.FillArrayWithIDs(m_arCurrentFilterProviderIDs);
		
			if(m_arCurrentFilterProviderIDs.GetSize() > 1) {
				//TES 4/16/2009 - PLID 33888 - They chose more than one, update our hyperlink.
				ShowDlgItem(IDC_PROVIDER_FILTER_LIST, SW_HIDE);
				m_nxlMultiProviderFilter.SetText(dlg.GetMultiSelectString());
				m_nxlMultiProviderFilter.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_PROVIDER_FILTER, SW_SHOWNA);
				InvalidateDlgItem(IDC_MULTI_PROVIDER_FILTER);
			}
			else if(m_arCurrentFilterProviderIDs.GetSize() == 1) {
				//TES 4/16/2009 - PLID 33888 - They selected exactly one, hide the hyperlink.
				ShowDlgItem(IDC_MULTI_PROVIDER_FILTER, SW_HIDE);
				ShowDlgItem(IDC_PROVIDER_FILTER_LIST, SW_SHOWNA);
				m_pProviderFilterList->SetSelByColumn(pflcID, m_arCurrentFilterProviderIDs[0]);
			}
			else {
				//TES 4/16/2009 - PLID 33888 - They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
		}
		else {
			//TES 4/16/2009 - PLID 33888 - Check if they have "multiple" selected
			if(m_arCurrentFilterProviderIDs.GetSize() > 1) {
				//TES 4/16/2009 - PLID 33888 - They do, make sure the screen is up to date.
				ShowDlgItem(IDC_PROVIDER_FILTER_LIST, SW_HIDE);
				CString strIDList, strID, strProviderList;
				for(int i=0; i < m_arCurrentFilterProviderIDs.GetSize(); i++) {
					strID.Format("%li, ", m_arCurrentFilterProviderIDs.GetAt(i));
					strIDList += strID;
				}
				strIDList = strIDList.Left(strIDList.GetLength()-2);
				_RecordsetPtr rsProviders = CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name "
					"FROM PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
					"WHERE PersonT.ID IN (%s) ORDER BY Name ASC", strIDList);
				while(!rsProviders->eof) {
					strProviderList += AdoFldString(rsProviders, "Name") + ", ";
					rsProviders->MoveNext();
				}
				rsProviders->Close();
				strProviderList = strProviderList.Left(strProviderList.GetLength()-2);
				m_nxlMultiProviderFilter.SetText(strProviderList);
				m_nxlMultiProviderFilter.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_PROVIDER_FILTER, SW_SHOWNA);
				InvalidateDlgItem(IDC_MULTI_PROVIDER_FILTER);
			}
			else {
				//TES 4/16/2009 - PLID 33888 - They selected exactly one, make sure the screen is up to date.
				ShowDlgItem(IDC_MULTI_PROVIDER_FILTER, SW_HIDE);
				ShowDlgItem(IDC_PROVIDER_FILTER_LIST, SW_SHOWNA);
				ASSERT(m_arCurrentFilterProviderIDs.GetSize() == 1);
				m_pProviderFilterList->SetSelByColumn(pflcID, m_arCurrentFilterProviderIDs[0]);
			}
		}

		//TES 4/16/2009 - PLID 33888 - Refresh the list
		RefreshList();
	}NxCatchAll("Error in CSureScriptsCommDlg::OnMultiProviderFilter()");
}

LRESULT CSureScriptsCommDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MULTI_PROVIDER_FILTER:
			OnMultiProviderFilter();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CSureScriptsCommDlg::OnLabelClick()");
	return 0;
}

BOOL CSureScriptsCommDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		//TES 4/16/2009 - PLID 33888 - If our multi provider filter is visible and enabled, and the mouse is over it,
		// set the link cursor.
		if(m_nxlMultiProviderFilter.IsWindowVisible() && m_nxlMultiProviderFilter.IsWindowEnabled()) {
			m_nxlMultiProviderFilter.GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll("Error in CSureScriptsCommDlg::OnSetCursor()");
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

LRESULT CSureScriptsCommDlg::OnUpdateView(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 4/21/2009 - PLID 33888 - Just refresh.
		RefreshList();
	}NxCatchAll("Error in CSureScriptsCommDlg::OnUpdateView()");
	return 0;
}

void CSureScriptsCommDlg::PrefilterProviders(const CArray<int,int> &arProviderIDs)
{
	//TES 4/21/2009 - PLID 33888 - Copy the list to our Pre-Filter, which is only checked in OnInitDialog().
	m_arPreFilterProviderIDs.RemoveAll();
	for(int i = 0; i < arProviderIDs.GetSize(); i++) {
		m_arPreFilterProviderIDs.Add(arProviderIDs[i]);
	}
	//TES 4/21/2009 - PLID 33888 - Remember to use this list.
	m_bUsePreFilter = true;
}

void CSureScriptsCommDlg::OnCancel()
{
	try {
		//TES 4/21/2009 - PLID 33888 - Next time we start up, we don't want any of our old filter variables confusing.
		m_arPreFilterProviderIDs.RemoveAll();
		m_bUsePreFilter = false;
	}NxCatchAll("Error in CSureScriptsCommDlg::OnCancel()");
	
	CNxDialog::OnCancel();

}

void CSureScriptsCommDlg::SetActionRequired(const CString &strRequiredAction)
{
	//TES 11/18/2009 - PLID 36353 - When it's stored as XML, the newlines are \ns, so we need to replace them with \r\ns (making sure any
	// existing \r\ns don't get cut off).
	CString strAction = strRequiredAction;
	strAction.Replace("\n","\r\n");
	strAction.Replace("\r\r\n", "\r\n");
	SetDlgItemText(IDC_ACTION_REQUIRED, strAction);
}