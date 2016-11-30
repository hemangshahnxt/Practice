// (f.dinatale 2010-10-22) - PLID 40827 - Dialog created
// (f.dinatale 2011-03-02) - PLID 42635 - Created PL item that documents this dialog's abandonment.
// NxReminderTiersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NxReminderTiersDlg.h"


// CNxReminderTiersDlg dialog

IMPLEMENT_DYNAMIC(CNxReminderTiersDlg, CNxDialog)

CNxReminderTiersDlg::CNxReminderTiersDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNxReminderTiersDlg::IDD, pParent),
	m_uifctTiers("CellTrustTiersT")
{
}

CNxReminderTiersDlg::~CNxReminderTiersDlg()
{
}

void CNxReminderTiersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNxReminderTiersDlg, CNxDialog)
	ON_BN_CLICKED(IDC_TIERS_SAVECLOSE, &CNxReminderTiersDlg::OnTiersSaveclose)
	ON_BN_CLICKED(IDC_TIERS_ADD, &CNxReminderTiersDlg::OnTiersAdd)
END_MESSAGE_MAP()


// CNxReminderTiersDlg message handlers
// (f.dinatale 2010-10-25) - PLID 40827
BOOL CNxReminderTiersDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// Bind the data Tiers datalist and set all NxIconButtons to the proper icons.
		m_dlTiers = BindNxDataList2Ctrl(IDC_TIERS_LIST);
		((CNxIconButton*)SafeGetDlgItem<CNxIconButton>(IDC_TIERS_ADD))->AutoSet(NXB_NEW);
		((CNxIconButton*)SafeGetDlgItem<CNxIconButton>(IDC_TIERS_SAVECLOSE))->AutoSet(NXB_CLOSE);
		((CNxColor*)GetDlgItem(IDC_TIERS_COLOR))->SetColor(GetNxColor(GNC_PATIENT_STATUS, 0));
		((CNxColor*)GetDlgItem(IDC_TIERS_COLOR2))->SetColor(GetNxColor(GNC_PATIENT_STATUS, 0));

		m_strTierName = "";
		m_strMessages = "";
		m_curPrice.SetCurrency(0, 0);

	} NxCatchAll(__FUNCTION__);

	return FALSE;
}


void CNxReminderTiersDlg::OnTiersSaveclose()
{
	// TODO: Add your control notification handler code here
}

BEGIN_EVENTSINK_MAP(CNxReminderTiersDlg, CDialog)
	ON_EVENT(CNxReminderTiersDlg, IDC_TIERS_LIST, 2, CNxReminderTiersDlg::OnSelChangedTiersList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CNxReminderTiersDlg::OnSelChangedTiersList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		if(lpNewSel == NULL) {
			return;
		}

		CString strTier;
		CString strMessages;
		CString strCost;

		GetDlgItemText(IDC_TIERS_NAME, strTier);
		GetDlgItemText(IDC_TIERS_MESSAGE_COUNT, strMessages);
		GetDlgItemText(IDC_TIERS_PRICE, strCost);

		if(m_strTierName != strTier) {
			// add to changes
		}

		if(m_strMessages != strMessages) {
			// add to changes
		}

		if(m_curPrice.Format() != strCost) {
			// add to changes
		}
	} NxCatchAll(__FUNCTION__);
}

void CNxReminderTiersDlg::OnTiersAdd()
{
	try {
		CString strTierName, strMessages, strPrice;

		GetDlgItemText(IDC_TIERS_NAME, strTierName);
		GetDlgItemText(IDC_TIERS_MESSAGE_COUNT, strMessages);
		GetDlgItemText(IDC_TIERS_PRICE, strPrice);

		if(strTierName == "" || strMessages == "" || strPrice == "") {
			MessageBox("All fields must be filled in before adding a new Tier.", "Invalid Fields", MB_OK);
			return;
		} else {
			// add to changes
		}
	} NxCatchAll(__FUNCTION__);
}
