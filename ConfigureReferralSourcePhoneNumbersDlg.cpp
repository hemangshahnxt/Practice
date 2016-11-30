// ConfigureReferralSourcePhoneNumbersDlg.cpp : implementation file
//
// (j.gruber 2010-01-12 11:11) - PLID 36647 - created for

#include "stdafx.h"
#include "Practice.h"
#include "ConfigureReferralSourcePhoneNumbersDlg.h"


enum ReferralSourceListColumn {
	rslcID = 0,
	rslcName,
	rslcNumber,
	rslcHomeNumber,
	rslcHasChanged
};
// CConfigureReferralSourcePhoneNumbersDlg dialog

IMPLEMENT_DYNAMIC(CConfigureReferralSourcePhoneNumbersDlg, CNxDialog)

CConfigureReferralSourcePhoneNumbersDlg::CConfigureReferralSourcePhoneNumbersDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureReferralSourcePhoneNumbersDlg::IDD, pParent)
{
	m_bFormatPhoneNums = FALSE;
	m_strPhoneFormat = "";
	m_nReferralID = -1;
	m_bFormattingField = FALSE;

}

CConfigureReferralSourcePhoneNumbersDlg::~CConfigureReferralSourcePhoneNumbersDlg()
{
}	

void CConfigureReferralSourcePhoneNumbersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_CRS_CALLERID_NUMBER, m_edtPhoneNumber);
	DDX_Control(pDX, IDC_CRS_HOME_PHONE_NUMBER, m_edtHomePhoneNumber);
	
}


BEGIN_MESSAGE_MAP(CConfigureReferralSourcePhoneNumbersDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CConfigureReferralSourcePhoneNumbersDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CConfigureReferralSourcePhoneNumbersDlg::OnBnClickedCancel)
	ON_EN_SETFOCUS(IDC_CRS_CALLERID_NUMBER, &CConfigureReferralSourcePhoneNumbersDlg::OnEnSetfocusCrsPhoneNumber)
	ON_EN_KILLFOCUS(IDC_CRS_CALLERID_NUMBER, &CConfigureReferralSourcePhoneNumbersDlg::OnEnKillfocusCrsPhoneNumber)
	ON_EN_CHANGE(IDC_CRS_CALLERID_NUMBER, &CConfigureReferralSourcePhoneNumbersDlg::OnEnChangeCrsPhoneNumber)
	ON_EN_CHANGE(IDC_CRS_HOME_PHONE_NUMBER, &CConfigureReferralSourcePhoneNumbersDlg::OnEnChangeCrsHomePhoneNumber)
	ON_EN_KILLFOCUS(IDC_CRS_HOME_PHONE_NUMBER, &CConfigureReferralSourcePhoneNumbersDlg::OnEnKillfocusCrsHomePhoneNumber)
	ON_EN_SETFOCUS(IDC_CRS_HOME_PHONE_NUMBER, &CConfigureReferralSourcePhoneNumbersDlg::OnEnSetfocusCrsHomePhoneNumber)
END_MESSAGE_MAP()


// CConfigureReferralSourcePhoneNumbersDlg message handlers
BOOL CConfigureReferralSourcePhoneNumbersDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		g_propManager.CachePropertiesInBulk("ConfigureReferralSourcePhoneNumbers", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'FormatPhoneNums' "				
				")",
				_Q(GetCurrentUserName()));
			
		g_propManager.CachePropertiesInBulk("ConfigureReferralSourcePhoneNumbers", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'PhoneFormatString' "				
				")",
				_Q(GetCurrentUserName()));


		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		m_pReferralSourceList = BindNxDataList2Ctrl(IDC_CRS_REFERRAL_SOURCE_LIST, true);		

		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pReferralSourceList->GetColumn(rslcName);
		if (pCol) {
			pCol->PutSortPriority(1);
		}
		m_pReferralSourceList->Sort();		

	}NxCatchAll(__FUNCTION__);

	return 0;
}

void CConfigureReferralSourcePhoneNumbersDlg::OnBnClickedOk()
{
	try {

		//make sure its saved
		SaveToMemory(IDC_CRS_CALLERID_NUMBER);
		SaveToMemory(IDC_CRS_HOME_PHONE_NUMBER);

		//run through our list and generate a save string
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReferralSourceList->GetFirstRow();
		CString strSql = BeginSqlBatch();
		BOOL bCommitBatch = FALSE;

		while (pRow) {

			if (VarBool(pRow->GetValue(rslcHasChanged), 0)) {

				bCommitBatch = TRUE;

				AddStatementToSqlBatch(strSql, "UPDATE PersonT SET WorkPhone = '%s', HomePhone = '%s' WHERE ID = %li", 
					_Q(VarString(pRow->GetValue(rslcNumber), "")),
					_Q(VarString(pRow->GetValue(rslcHomeNumber), "")),
					VarLong(pRow->GetValue(rslcID)));
			}

			pRow = pRow->GetNextRow();
		}

		if (bCommitBatch) {
			ExecuteSqlBatch(strSql);
		}	

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::OnBnClickedCancel()
{
	CNxDialog::OnCancel();
}


void CConfigureReferralSourcePhoneNumbersDlg::OnEnSetfocusCrsPhoneNumber()
{
	try {	
		EnSetFocus(IDC_CRS_CALLERID_NUMBER);
		
	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::OnEnKillfocusCrsPhoneNumber()
{
	try {
		EnKillFocus(IDC_CRS_CALLERID_NUMBER);
	}NxCatchAll(__FUNCTION__);

}

BEGIN_EVENTSINK_MAP(CConfigureReferralSourcePhoneNumbersDlg, CNxDialog)	
	ON_EVENT(CConfigureReferralSourcePhoneNumbersDlg, IDC_CRS_REFERRAL_SOURCE_LIST, 18, CConfigureReferralSourcePhoneNumbersDlg::RequeryFinishedCrsReferralSourceList, VTS_I2)	
	ON_EVENT(CConfigureReferralSourcePhoneNumbersDlg, IDC_CRS_REFERRAL_SOURCE_LIST, 16, CConfigureReferralSourcePhoneNumbersDlg::SelChosenCrsReferralSourceList, VTS_DISPATCH)
	ON_EVENT(CConfigureReferralSourcePhoneNumbersDlg, IDC_CRS_REFERRAL_SOURCE_LIST, 1, CConfigureReferralSourcePhoneNumbersDlg::SelChangingCrsReferralSourceList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()



// (j.gruber 2010-03-04 10:18) - PLID 37622 - added an ID field
void CConfigureReferralSourcePhoneNumbersDlg::SaveToMemory(long nDlgID) 
{
	try  {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReferralSourceList->FindByColumn(rslcID, m_nReferralID, NULL, FALSE);
		if (pRow) {

			short nColID;
			if (nDlgID == IDC_CRS_CALLERID_NUMBER) {
				nColID = rslcNumber;
			}
			else {
				nColID = rslcHomeNumber;
			}


			CString strPhone = VarString(pRow->GetValue(nColID), "");
			long nID = VarLong(pRow->GetValue(rslcID), -1);

			CString strNewPhone;
			GetDlgItemText(nDlgID, strNewPhone);

			//don't save the phone format
			if (strNewPhone == m_strPhoneFormat) {
				strNewPhone = "";
			}

			if (strPhone != strNewPhone) {

				// (j.gruber 2010-03-04 10:33) - PLID 37622 - don't check for the real phone number
				if (!strNewPhone.IsEmpty() && nDlgID == IDC_CRS_CALLERID_NUMBER) {
		
					//loop through the list and see if there is a duplicate
					NXDATALIST2Lib::IRowSettingsPtr pCheckRow = m_pReferralSourceList->GetFirstRow();
					BOOL bFound = FALSE;
					while (pCheckRow && !bFound) {
						CString strCheckPhone = VarString(pCheckRow->GetValue(nColID), "");
						long nCheckID = VarLong(pCheckRow->GetValue(rslcID), -1);

						if (strCheckPhone == strNewPhone) {
							if (nID != nCheckID) {
								bFound = TRUE;
							}
						}
						pCheckRow = pCheckRow->GetNextRow();
					}						

					if (bFound) {
						if (IDNO == MsgBox(MB_YESNO, "This phone number already exists for a referral source.\nWould you like to continue saving it?")) {
							//clear the box 
							SetDlgItemText(nDlgID, "");
							SaveToMemory(nDlgID);
							return;
						}
					}
				}
				pRow->PutValue(nColID, _variant_t(strNewPhone));
				pRow->PutValue(rslcHasChanged, g_cvarTrue);									
			}
		}
	}NxCatchAll(__FUNCTION__);

}

void CConfigureReferralSourcePhoneNumbersDlg::RequeryFinishedCrsReferralSourceList(short nFlags)
{
	try {

		//set the selection to be the first row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReferralSourceList->GetFirstRow();
				
		//load the referral
		if (pRow) {
			m_pReferralSourceList->CurSel = pRow;
			CString strPhone = VarString(pRow->GetValue(rslcNumber), "");
			// (j.gruber 2010-03-04 10:36) - PLID 37622 - added home phone
			CString strHomePhone = VarString(pRow->GetValue(rslcHomeNumber), "");
			SetDlgItemText(IDC_CRS_CALLERID_NUMBER, strPhone);
			SetDlgItemText(IDC_CRS_HOME_PHONE_NUMBER, strHomePhone);
			m_nReferralID = VarLong(pRow->GetValue(rslcID));			
		}	

	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::SelChosenCrsReferralSourceList(LPDISPATCH lpRow)
{
	try {

		//save the last value
		//SaveToMemory();
		
		//now let's set our current values
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			CString strPhone = VarString(pRow->GetValue(rslcNumber), "");
			SetDlgItemText(IDC_CRS_CALLERID_NUMBER, strPhone);
			// (j.gruber 2010-03-04 10:37) - PLID 37622
			strPhone = VarString(pRow->GetValue(rslcHomeNumber), "");
			SetDlgItemText(IDC_CRS_HOME_PHONE_NUMBER, strPhone);
			m_nReferralID = VarLong(pRow->GetValue(rslcID));			
		}					

	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::OnEnChangeCrsPhoneNumber()
{
	try {
		EnChange(IDC_CRS_CALLERID_NUMBER);
	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::EnChange(long nID) {
	

	if (m_bFormattingField) {
		return;
	}		

	CString str;
	GetDlgItemText(nID, str);
	str.TrimRight();
	if (str != "") {
		if(m_bFormatPhoneNums) {
			m_bFormattingField = true;
			FormatItem (nID, m_strPhoneFormat);
			m_bFormattingField = false;
		}
	}	

}


void CConfigureReferralSourcePhoneNumbersDlg::EnSetFocus(long nID) 
{

	if (m_bFormatPhoneNums) {
		FormatItem (nID, m_strPhoneFormat);
	}

}

void CConfigureReferralSourcePhoneNumbersDlg::EnKillFocus(long nID) {

	SaveToMemory(nID);
}

void CConfigureReferralSourcePhoneNumbersDlg::OnEnChangeCrsHomePhoneNumber()
{
	try {
		EnChange(IDC_CRS_HOME_PHONE_NUMBER);
	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::OnEnKillfocusCrsHomePhoneNumber()
{
	try {
		EnKillFocus(IDC_CRS_HOME_PHONE_NUMBER);	
	}NxCatchAll(__FUNCTION__);
}

void CConfigureReferralSourcePhoneNumbersDlg::OnEnSetfocusCrsHomePhoneNumber()
{
	try {
		EnSetFocus(IDC_CRS_HOME_PHONE_NUMBER);
	}NxCatchAll(__FUNCTION__);
	
}

void CConfigureReferralSourcePhoneNumbersDlg::SelChangingCrsReferralSourceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}
