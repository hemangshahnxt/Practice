// CareCreditSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "CareCreditSelectDlg.h"

//CareCreditselect Fields
enum CareCreditSelect
{
	ccsAccount,
	ccsCareCreditID,
	ccsMerchantNum,
	ccsMerchantPass
};



// CareCreditSelectDlg dialog

IMPLEMENT_DYNAMIC(CareCreditSelectDlg, CNxDialog)

CareCreditSelectDlg::CareCreditSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CareCreditSelectDlg::IDD, pParent)
{
	m_strLoginMerchantID = "";
	m_strLoginMerchantPassword = "";

}

CareCreditSelectDlg::~CareCreditSelectDlg()
{
}



void CareCreditSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BOOL CareCreditSelectDlg::OnInitDialog()
{
	try
	{
		//interface niceties
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		CNxDialog::OnInitDialog();

		m_pCCAccountList= BindNxDataList2Ctrl(IDC_CARECREDIT_ACCOUNT,true);
		m_pCCAccountList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pCCAccountList->PutCurSel(m_pCCAccountList->GetFirstRow());

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetCurSel();
		if(pRow)
		{
			m_strLoginMerchantPassword = VarString(pRow->GetValue(ccsMerchantPass));
			m_strLoginMerchantID = VarString(pRow->GetValue(ccsMerchantNum));
		}
	}NxCatchAll(__FUNCTION__);

	return true;
}


BEGIN_MESSAGE_MAP(CareCreditSelectDlg, CNxDialog)
END_MESSAGE_MAP()

// (j.camacho 2013-08-14 12:10) - PLID 58024
// Public function to access the memberid of the account selected
CString CareCreditSelectDlg::GetMerchantNumber()
{
	return m_strLoginMerchantID;
}

// (j.camacho 2013-08-14 12:10) - PLID 58024
// Public function to access the password of the account select
CString CareCreditSelectDlg::GetMerchantPassword()
{
	return m_strLoginMerchantPassword;
}

// CareCreditSelectDlg message handlers
BEGIN_EVENTSINK_MAP(CareCreditSelectDlg, CNxDialog)
	ON_EVENT(CareCreditSelectDlg, IDC_CARECREDIT_ACCOUNT, 1, CareCreditSelectDlg::SelChangingCarecreditAccount, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CareCreditSelectDlg, IDC_CARECREDIT_ACCOUNT, 2, CareCreditSelectDlg::SelChangedCarecreditAccount, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CareCreditSelectDlg::SelChangingCarecreditAccount(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.camacho 2013-08-19 11:30) - PLID 58024
void CareCreditSelectDlg::SelChangedCarecreditAccount(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	//Check if null and warn
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetCurSel();
	if(pRow==NULL)
	{
		AfxMessageBox("Your selection is not valid. Please select a valid option from the drop down menu.");
	}
	else
	{
		//not null row, let it succeed and store those values in our dialog.
		m_strLoginMerchantPassword = pRow->GetValue(ccsMerchantPass);
		m_strLoginMerchantID =  pRow->GetValue(ccsMerchantNum);
	}
	
}
