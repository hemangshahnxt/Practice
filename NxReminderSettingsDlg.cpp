// NxReminderSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "globalutils.h"
#include "Practice.h"
#include "PatientsRc.h"
#include "NxReminderSettingsDlg.h"
#include "NxReminderTiersDlg.h"
#include "NxException.h"

// (f.dinatale 2010-10-26) - PLID 40827 - dialog created
// NxReminderSettingsDlg dialog

#define NXM_NXREMINDERLOADED WM_APP + 0x40

IMPLEMENT_DYNAMIC(CNxReminderSettingsDlg, CNxDialog)

CNxReminderSettingsDlg::CNxReminderSettingsDlg(const CString& strLicenseKey, CWnd* pParent /*=NULL*/)
	: CNxDialog(CNxReminderSettingsDlg::IDD, pParent), m_nxremDlgInfo(strLicenseKey)
{
}

CNxReminderSettingsDlg::~CNxReminderSettingsDlg()
{
}

void CNxReminderSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNxReminderSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_EDIT_TIERS, &CNxReminderSettingsDlg::OnEditTiers)
	ON_BN_CLICKED(IDC_CELLTRUST_OK, &CNxReminderSettingsDlg::OnCelltrustOk)
	ON_MESSAGE(NXM_NXREMINDERLOADED, &CNxReminderSettingsDlg::OnDialogLoaded)
END_MESSAGE_MAP()


BOOL CNxReminderSettingsDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		((CNxColor*)GetDlgItem(IDC_NXREMINDER_COLOR))->SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_CELLTRUST_OK))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);
		m_lTiers = BindNxDataList2Ctrl(IDC_CLIENT_TIER_LIST);

		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_EDIT_TIERS))->EnableWindow(FALSE);
		GetDlgItem(IDC_CLIENT_TIER_LIST)->EnableWindow(FALSE);
		//GetDlgItem(IDC_CELLTRUST_NICKNAME)->EnableWindow(FALSE);


		// Post message
		 PostMessage(NXM_NXREMINDERLOADED, (WPARAM)0, (LPARAM)0);
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

// NxReminderSettingsDlg message handlers


// This dialog is incomplete after the business cycle was decided mid-project.  The button to it is always disabled.
void CNxReminderSettingsDlg::OnEditTiers()
{
	try {
		CNxReminderTiersDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

void CNxReminderSettingsDlg::OnCelltrustOk()
{
	try {
		//CString strNickname;
		//GetDlgItemText(IDC_CELLTRUST_NICKNAME, strNickname);
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_lTiers->GetCurSel();

		if(!pRow){
			MessageBox("A text message tier must be selected for this client.", "Invalid Tier Selected", MB_OK);
			return;
		} else {
			int nTier = VarLong(pRow->GetValue(dlSettingsID));
			int nMessages;
			CString strMessages;
			GetDlgItemText(IDC_NXREMINDER_MESSAGES, strMessages);
			nMessages = atoi(strMessages);
	
			if(nMessages < 0) {
				MessageBox("Please enter a positive number of allotted messages.", "Negative Value for allotted Messages", MB_OK);
				return;
			}

			int nLicenseStatus = m_nxremDlgInfo.ModifyClientLicense((long)nTier, (long)nMessages);

			if(nLicenseStatus < 0) {
				MessageBox("Error when modifying client license information on NexReminder Server", "Error Modifying NxReminder Settings", MB_OK);
			} else {
				int nResult = MessageBox("The client's NexReminder settings have been saved, would you like changes to be applied immediately?  If No, changes"
					" will take effect next billing cycle.", 
					"Client Information Modified", MB_YESNO);

				// (f.dinatale 2011-01-18) - PLID 40827 - We now ask if we want to immediately apply the settings or wait until the next billing cycle.
				switch(nResult) {
					case IDYES:
						{
							int nActivationStatus = m_nxremDlgInfo.ModifyClientActiveStatus(false);

							if(nActivationStatus < 0) {
								MessageBox("There was an issue activating the new settings.  Please check your connection and then recommit your changes.", 
									"Activation Error", MB_OK);
								return;
							}

							nActivationStatus = m_nxremDlgInfo.ModifyClientActiveStatus(true);

							if(nActivationStatus < 0) {
								MessageBox("There was an issue activating the new settings.  Please check your connection and then recommit your changes.", 
									"Activation Error", MB_OK);
								return;
							}
						}
						break;
					case IDNO:
						break;
				}

				m_nxremDlgInfo.SetTierID(nTier);
				m_nxremDlgInfo.SetAllotedMessages(nMessages);
				OnOK();
			}
		}

		// (f.dinatale 2010-11-30) - PLID 40829 - We no longer need to worry about the nickname.
		/*if(strNickname != m_nxremDlgInfo.GetNickname()) {

			if(strNickname == ""){
				MessageBox("A unique nickname must be provided for this client.", "Invalid Nickname", MB_OK);
				return;
			}

			int nNicknameStatus = m_nxremDlgInfo.ModifyClientNickname(strNickname);

			if(nNicknameStatus > 0) {
				if(nNicknameStatus == 1) {
					MessageBox("The nickname entered is already in use.  Provide a unique nickname for this client.", "Invalid Nickname", MB_OK);
					return;
				} else {
					ThrowNxException("Error when modifying client nickname information on NxReminder Server");
					return;
				}
			}
		}*/
	} NxCatchAll(__FUNCTION__);
}

LRESULT CNxReminderSettingsDlg::OnDialogLoaded(WPARAM wParam, LPARAM lParam)
{
	try {
		int nStatus = m_nxremDlgInfo.GetClientInfo();
		
		if(nStatus < 0) {
			MessageBox("Error retrieving client information from NexReminder Server.");
			OnCancel();
			return -1;
		}

		// (f.dinatale 2011-01-13) - PLID 40829 - Practice now notifies the user that the client has not been set that they have bought NexReminder.
		if(nStatus == 1) {
			MessageBox("Make sure that the client has bought NexReminder.");
			OnCancel();
			return -1;
		}
		
		if(!m_nxremDlgInfo.IsActive()) {
			MessageBox("Before setting NexReminder settings please be sure that the client is set to \"Bought\" or \"In Use\" for the NexReminder service.", "Inactive Client", MB_OK);
			OnCancel();
			return -1;
		} else {
			// (f.dinatale 2010-11-30) - PLID 40829 - Nickname is no longer needed.
			//GetDlgItem(IDC_CELLTRUST_NICKNAME)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_TIERS)->EnableWindow(FALSE);
			GetDlgItem(IDC_CLIENT_TIER_LIST)->EnableWindow(TRUE);

			if(m_nxremDlgInfo.GetTierID() != -1) {
				m_lTiers->SetSelByColumn(dlSettingsID, _variant_t(((long)m_nxremDlgInfo.GetTierID())));
				//SetDlgItemTextA(IDC_CELLTRUST_NICKNAME, m_nxremDlgInfo.GetNickname());
			}

			// (f.dinatale 2011-01-18) - PLID 40827 - Added remaining messages to the dialog.  If they have no allotted messages, they won't have any remaining either.
			if(m_nxremDlgInfo.GetAllotedMessages() != -1) {
				CString strMessages;
				strMessages.Format("%li", m_nxremDlgInfo.GetAllotedMessages());
				SetDlgItemText(IDC_NXREMINDER_MESSAGES, strMessages);
				strMessages.Format("%li", m_nxremDlgInfo.GetRemainingMessages());
				SetDlgItemText(IDC_NXREMINDER_REMAINING, strMessages);
			} else {
				SetDlgItemText(IDC_NXREMINDER_MESSAGES, "None");
				SetDlgItemText(IDC_NXREMINDER_REMAINING, "None");
			}
			return 0;
		}

	} NxCatchAll(__FUNCTION__);

	MessageBox("Before setting NexReminder settings please be sure that the client is set to \"Bought\" or \"In Use\" for the NexReminder service.", "Inactive Client", MB_OK);
	OnCancel();
	return -1;
}

BEGIN_EVENTSINK_MAP(CNxReminderSettingsDlg, CNxDialog)
ON_EVENT(CNxReminderSettingsDlg, IDC_CLIENT_TIER_LIST, 1, CNxReminderSettingsDlg::OnSelChangingClientTierList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CNxReminderSettingsDlg::OnSelChangingClientTierList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel);

		if(pRow) {
			int nMessages = VarLong(pRow->GetValue(dlSettingsMessages));
			CString strMessages;
			strMessages.Format("%li", nMessages);
			SetDlgItemText(IDC_NXREMINDER_MESSAGES, strMessages);
		} else {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpOldSel);
			m_lTiers->PutCurSel(pRow);
		}
	} NxCatchAll(__FUNCTION__);
}
