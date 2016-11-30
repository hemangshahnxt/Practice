// CareCreditSetupDLG.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "CareCreditSetupDLG.h"

//CareCredit Enumeration
enum CareCreditFields
	{
		ccAccountName,
		ccCareCreditID,
		ccMerchantNumber,
		ccPassword,
		ccModified
	};



// CCareCreditSetupDlg dialog

IMPLEMENT_DYNAMIC(CCareCreditSetupDlg, CNxDialog)

CCareCreditSetupDlg::CCareCreditSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCareCreditSetupDlg::IDD, pParent)
{
	m_nPreviousAccountNum=0;

	//for new accounts
	m_nNextNewID=-1;
}

CCareCreditSetupDlg::~CCareCreditSetupDlg()
{
}

void CCareCreditSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CARECREDIT_ADD_ACCOUNT, m_btnAdd);
	DDX_Control(pDX, IDC_CARECREDIT_DELETE_ACCOUNT, m_btnDelete);
	DDX_Control(pDX, IDC_CARECREDIT_ACCOUNTNAME, m_nxeditAccountName);
	DDX_Control(pDX, IDC_CARECREDIT_MERCHANT_ID, m_nxeditMerchantID);
	DDX_Control(pDX, IDC_CARECREDIT_PASSWORD, m_nxeditPassword);
}


BEGIN_MESSAGE_MAP(CCareCreditSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CARECREDIT_ADD_ACCOUNT, &CCareCreditSetupDlg::OnBnClickedCarecreditAddAccount)
	ON_BN_CLICKED(IDC_CARECREDIT_DELETE_ACCOUNT, &CCareCreditSetupDlg::OnBnClickedCarecreditDeleteAccount)
	ON_EN_KILLFOCUS(IDC_CARECREDIT_ACCOUNTNAME, &CCareCreditSetupDlg::OnEnKillfocusCarecreditAccountname)
END_MESSAGE_MAP()


// CCareCreditSetupDlg message handlers
BEGIN_EVENTSINK_MAP(CCareCreditSetupDlg, CNxDialog)
	ON_EVENT(CCareCreditSetupDlg, IDC_CARECREDIT_ACCOUNT_LIST, 16, CCareCreditSetupDlg::SelChosenCarecreditAccountList, VTS_DISPATCH)
	ON_EVENT(CCareCreditSetupDlg, IDC_CARECREDIT_ACCOUNT_LIST, 1, CCareCreditSetupDlg::SelChangingCarecreditAccountList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (j.camacho 2013-07-31 16:50) - PLID 57518 - Load currently selected account info
void CCareCreditSetupDlg::SelChosenCarecreditAccountList(LPDISPATCH lpRow)
{
	try{
		if(SaveCurrentlyDisplayedAccount())
		{
			//successful save	
			LoadCurrentAccountInfo();
		}
		else
		{
			m_pCCAccountList->SetSelByColumn(ccCareCreditID,(long)m_nPreviousAccountNum);
			return;
		}

	}NxCatchAll(__FUNCTION__);

}
// (j.camacho 2013-08-05 17:00) - PLID 57518 - Checks for duplicate account names
bool CCareCreditSetupDlg::DoesNameExist(CString strAccount, long nIDToSkip)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetFirstRow();
	while(pRow!=NULL)
	{
		if(nIDToSkip == VarLong(pRow->GetValue(ccCareCreditID)))
		{
			//skip this one
		}
		else
		{
			if (VarString(pRow->GetValue(ccAccountName)).CompareNoCase(strAccount)==0)
			{
				//matches
				return true;
			}
		}
		pRow = pRow->GetNextRow();	
	}
	return false;
}

BOOL CCareCreditSetupDlg::OnInitDialog()
{

	try{
		CNxDialog::OnInitDialog();

		//Interface niceties
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		m_nxeditAccountName.SetLimitText(80);
		m_nxeditAccountName.EnableWindow(0);
		m_nxeditMerchantID.SetLimitText(16);
		m_nxeditMerchantID.EnableWindow(0);
		m_nxeditPassword.SetLimitText(20);
		m_nxeditPassword.EnableWindow(0);

		//initialize drop down
		m_pCCAccountList= BindNxDataList2Ctrl(IDC_CARECREDIT_ACCOUNT_LIST,true);
		m_pCCAccountList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pCCAccountList->PutCurSel(m_pCCAccountList->GetFirstRow());
		if(m_pCCAccountList->GetCurSel()==NULL)
		{
			//no accounts exists
		}
		else
		{
			LoadCurrentAccountInfo();
			m_nxeditAccountName.EnableWindow(1);
			m_nxeditMerchantID.EnableWindow(1);
			m_nxeditPassword.EnableWindow(1);
		}
			
		//fill out information details for default option
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
// (j.camacho 2013-08-05 17:00) - PLID 57518 - saves currently viewed info when changing to another account
void CCareCreditSetupDlg::LoadCurrentAccountInfo()
{
	//initalize text boxes
	CString strAccountName,strMerchantNum, strPassword;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetCurSel();

	//keep track of last account we loaded
	if(pRow!=NULL)
	{
		strAccountName = VarString(pRow->GetValue(ccAccountName));
		strMerchantNum = VarString(pRow->GetValue(ccMerchantNumber));
		strPassword = VarString(pRow->GetValue(ccPassword)); 

		if(m_nPreviousAccountNum!=VarLong(pRow->GetValue(ccCareCreditID)))
		{
			m_nPreviousAccountNum=VarLong(pRow->GetValue(ccCareCreditID));
			//set dialog values
			SetDlgItemText(IDC_CARECREDIT_ACCOUNTNAME,strAccountName);
			SetDlgItemText(IDC_CARECREDIT_MERCHANT_ID,strMerchantNum);
			SetDlgItemText(IDC_CARECREDIT_PASSWORD,strPassword);
		}
		return;
	}
	else
	{
		strAccountName = "";
		strMerchantNum = "";
		strPassword = ""; 

	}
}

// (j.camacho 2013-08-05 16:55) - PLID 57518 save the three fields into the data list for saving later on submission
bool CCareCreditSetupDlg::SaveCurrentlyDisplayedAccount()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->FindByColumn(ccCareCreditID,m_nPreviousAccountNum,NULL,VARIANT_FALSE);

	//write dialogs over datalist entries
	CString strAccount, strMerchantID,strPassword;

	GetDlgItemText(IDC_CARECREDIT_ACCOUNTNAME, strAccount);
	GetDlgItemText(IDC_CARECREDIT_MERCHANT_ID,strMerchantID);
	GetDlgItemText(IDC_CARECREDIT_PASSWORD,strPassword);


	CString stroldAccount,stroldMerchantid,stroldPassword;
	if(pRow==NULL)
	{
		//empty row selected
		//successfully return since nothing needs to be saved
		return true;
	}
	else
	{
		strAccount.TrimRight();
		if(strAccount.IsEmpty())
		{
			AfxMessageBox("The account cannot have an empty name. Please enter in an account name.");
			return false;
		}
		stroldAccount = pRow->GetValue(ccAccountName);
		stroldMerchantid = pRow->GetValue(ccMerchantNumber);
		stroldPassword = pRow->GetValue(ccPassword);

		if(strAccount!=stroldAccount || strMerchantID != stroldMerchantid || strPassword != stroldPassword)
		{
			//marked as modified
			pRow->PutValue(ccModified, g_cvarTrue);
		}
		else
		{
			return true;
		}
		//Validate
		if(DoesNameExist(strAccount,pRow->GetValue(ccCareCreditID)))
		{
			//name exists, cannot save
			TRACE("Account name already exists. You can't use the same account name twice.");
			//create message
			AfxMessageBox("Account name already exists. You can't use the same account name twice.");
			return false;
		}

		if(strMerchantID.TrimRight().GetLength()<=0)
		{
			AfxMessageBox("Please enter a valid Merchant number.");
			return false;
		}

		if(strPassword.TrimRight().GetLength()<=0)
		{
			AfxMessageBox("Please enter a valid password.");
			return false;
		}
		//store changes
		pRow->PutValue(ccAccountName,_bstr_t(strAccount));
		pRow->PutValue(ccMerchantNumber,_bstr_t(strMerchantID));
		pRow->PutValue(ccPassword, _bstr_t(strPassword));

		return true; 
	}
}

// (j.camacho 2013-08-05 17:01) - PLID 57518 - Save all data in the datalist made. This is where all queries are made.
void CCareCreditSetupDlg::OnOK()
{


	try
	{
		//check for case where datalist is empty or removed the last item on the list
		if(m_pCCAccountList->GetFirstRow()!=NULL)
		{
			//save current data
			if(!SaveCurrentlyDisplayedAccount())
			{
				// data was not verified or couldn't be saved
				// make a message here to tell them so. Can I cancel an OnOK message?
				TRACE("Failure to save current CareCredit Data!");
				AfxMessageBox("Failure to save current CareCredit Data!");
				return;
			}
		}

		CParamSqlBatch batch;
		//1.)delete anything that shoulmy d be deleted
		for(int i=0; i<m_dwaryDeletedAccounts.GetSize();i++)
		{
			TRACE("DELETED a CareCredit account");
			batch.Add("DELETE FROM CarecreditusersT WHERE carecreditid={INT}",m_dwaryDeletedAccounts[i]);
		}

		//2.)Save any modified accounts in the list.
		NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pCCAccountList->GetFirstRow();
		while(pCurrentRow!=NULL)
		{
			if(VarBool(pCurrentRow->GetValue(ccModified)))
			{
				
				CString strAccount = VarString(pCurrentRow->GetValue(ccAccountName));
				CString strMerchantid = VarString(pCurrentRow->GetValue(ccMerchantNumber));
				CString strPassword = VarString(pCurrentRow->GetValue(ccPassword));
				long dUserId = VarLong(pCurrentRow->GetValue(ccCareCreditID));
				if(dUserId <0) //new
				{
					//3.)add new accounts if they have a negative ID
					TRACE("New account added to CareCredit\n");
					batch.Add("INSERT INTO CareCreditUserst"" (AccountName, Password, MerchantNumber)"
						" VALUES ({STRING},{STRING},{STRING})",strAccount, strPassword, strMerchantid);
				}
				else //modification
				{
						batch.Add("UPDATE CareCreditUserst"
						" SET AccountName = {STRING} , Password = {STRING}, MerchantNumber = {STRING}"
						" WHERE carecreditid={INT}", strAccount,strPassword,strMerchantid,dUserId);
					TRACE("Updated account for CareCredit\n");
				}
			}
			pCurrentRow = pCurrentRow->GetNextRow();
		}
		batch.Execute(GetRemoteData());
		CDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (j.camacho 2013-08-05 17:06) - PLID 57518 - add functionality makes new rows to save later
void CCareCreditSetupDlg::OnBnClickedCarecreditAddAccount(void)
{
	try{
		m_nxeditAccountName.EnableWindow(1);
		m_nxeditMerchantID.EnableWindow(1);
		m_nxeditPassword.EnableWindow(1);
		//if we have a currently viewing account, save any changes that may be there
		if(m_pCCAccountList->GetCurSel()!=NULL)
		{
			if(!SaveCurrentlyDisplayedAccount())
			{
				//failed to save this data, ignore the request to add a new one
				return;
			}
		}
		// Insert new Row in datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetNewRow();
		pRow->PutValue(ccAccountName,"");
		pRow->PutValue(ccCareCreditID,(long)m_nNextNewID);
		pRow->PutValue(ccMerchantNumber,"");
		pRow->PutValue(ccPassword,"");
		pRow->PutValue(ccModified,g_cvarTrue);
		// Set userid to a negative number (Do they need to be different?)
		m_pCCAccountList->AddRowSorted(pRow,NULL);

		m_nNextNewID--;

		// Set to that row as current info 
		m_pCCAccountList->PutCurSel(pRow);

		// Onselected handler should hit and load that datalist
		// Apparently it can't
		LoadCurrentAccountInfo();

		// Set focus to account name!
		GetDlgItem(IDC_CARECREDIT_ACCOUNTNAME)->SetFocus();
	}NxCatchAll(__FUNCTION__);
}

// (j.camacho 2013-08-05 17:06) - PLID 57518 - Deletes certain accounts and removes from datalist.
void CCareCreditSetupDlg::OnBnClickedCarecreditDeleteAccount()
{
	try
	{
		// Add carecreditID to a list 
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetCurSel();
		if(!pRow)
		{
			return;
		}
		long nID = VarLong(pRow->GetValue(ccCareCreditID));
		//pop up dialog to ask the customer if they are sure
		if(AfxMessageBox("Are you sure you want to delete",MB_YESNO)==IDYES)
		{
			if(nID>0)
			{
				m_dwaryDeletedAccounts.Add(nID);
			}
			else
			{
				// This is an unsaved new account. Just remove it from the datalist.
			}
			
			NXDATALIST2Lib::IRowSettingsPtr pNextRow=pRow->GetNextRow();
			m_pCCAccountList->RemoveRow(pRow);
			if(pNextRow == NULL)
			{
				pNextRow = m_pCCAccountList->GetFirstRow();
			}

			if(pNextRow!=NULL)
			{
				m_pCCAccountList->PutCurSel(pNextRow);
				// Load new selected item data
				LoadCurrentAccountInfo();
			}
			else
			{
				//no items left
				m_nxeditAccountName.EnableWindow(0);
				m_nxeditMerchantID.EnableWindow(0);
				m_nxeditPassword.EnableWindow(0);
				//clear information
				SetDlgItemText(IDC_CARECREDIT_ACCOUNTNAME,"");
				SetDlgItemText(IDC_CARECREDIT_MERCHANT_ID,"");
				SetDlgItemText(IDC_CARECREDIT_PASSWORD,"");
			}
		}
		else
		{
			return;
		}
	}NxCatchAll(__FUNCTION__);
}

void CCareCreditSetupDlg::SelChangingCarecreditAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);

}

void CCareCreditSetupDlg::OnEnKillfocusCarecreditAccountname()
{
	try
	{
		CString strAccount;
		GetDlgItemText(IDC_CARECREDIT_ACCOUNTNAME, strAccount);
		//Set the datalist when the account name is changed
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCAccountList->GetCurSel();
		if(pRow)
		{
			pRow->PutValue(ccAccountName,_bstr_t(strAccount));
		}
	}NxCatchAll(__FUNCTION__);

}
