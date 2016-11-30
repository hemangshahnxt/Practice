#include "stdafx.h"
#include "OHIPSetupCPTDLG.h"
#include "AssistingCodesSetupDlg.h"
#include "OHIPPremiumCodesDlg.h"



// (s.tullis 2014-05-22 09:52) - PLID 62120 - Make a new OHIP setup dialog to be called from the additional service code setup menu.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

static _variant_t NullOrIntStringToVariant(const CString & str)//s.tullis
{
	if (str == "NULL" || str.IsEmpty())
		return g_cvarNull;
	else
		return atol(str);
}

void COHIPCPTSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_CHECK_ASSISTING_CODE, m_checkAssistingCode);
	DDX_Control(pDX, IDC_BTN_OHIP_PREMIUM_CODES, m_btnOHIPPremiumCodes);
	DDX_Control(pDX, IDC_ASSISTING_BASE_UNITS, m_editAssistingBaseUnits);
	
}



COHIPCPTSetupDlg::COHIPCPTSetupDlg(CWnd* pParent)
: CNxDialog(COHIPCPTSetupDlg::IDD, pParent)
{



}



BEGIN_MESSAGE_MAP(COHIPCPTSetupDlg, CNxDialog)


	ON_BN_CLICKED(IDOK, OnCloseClick)
	ON_BN_CLICKED(IDC_BTN_OHIP_PREMIUM_CODES, OnBtnOhipPremiumCodes)
	ON_BN_CLICKED(IDC_CHECK_ASSISTING_CODE, OnCheckAssisting)
	

END_MESSAGE_MAP()
// (s.tullis 2014-05-22 09:52) - PLID 62120 - Init , load values
BOOL COHIPCPTSetupDlg::OnInitDialog(){

	CNxDialog::OnInitDialog();




	try{

		_variant_t tmpVar;
		CString str;
		m_btnOk.AutoSet(NXB_CLOSE);
		m_editAssistingBaseUnits.SetLimitText(5);


		_RecordsetPtr rs;
		FieldsPtr fields;

		rs = CreateParamRecordset(" Select AssistingCode, AssistingBaseUnits FROM CPTCODET WHERE ID ={INT}	", m_nServiceID);

		if (!rs->eof){
			fields = rs->Fields;

			m_checkAssistingCode.SetCheck(VarBool(fields->GetItem("AssistingCode")->Value));
			tmpVar = fields->GetItem("AssistingBaseUnits")->Value;
			str = "";
			if (tmpVar.vt == VT_I4)	{
				str.Format("%li", VarLong(tmpVar));
			}
			SetDlgItemText(IDC_ASSISTING_BASE_UNITS, str);

			rs->Close();

			if (!bCanWrite){
				SecureControls();
			}


			if (m_checkAssistingCode.GetCheck() == TRUE)
			{
				GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(TRUE);
			}
			else{
				GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(FALSE);
			}
		}
	}NxCatchAll(__FUNCTION__)


		return TRUE;

}



// (s.tullis 2014-05-22 09:52) - PLID 62120 - save values
void COHIPCPTSetupDlg::OnCloseClick(){

	try{


		long nAssistingCode = 0;
		CString strAssistingBaseUnits = "";

		if (m_checkAssistingCode.GetCheck()) {
		nAssistingCode = 1;
		}
		else {
		nAssistingCode = 0;
		}
		
		GetDlgItemText(IDC_ASSISTING_BASE_UNITS, strAssistingBaseUnits);
		//make sure it's numeric only
		strAssistingBaseUnits = strAssistingBaseUnits.SpanIncluding("0123456789");
		SetDlgItemText(IDC_ASSISTING_BASE_UNITS, strAssistingBaseUnits);
	
		
		ExecuteParamSql(" UPDATE CPTCODET SET AssistingCode = {INT} , AssistingBaseUnits = {VT_I4} WHERE ID ={INT} ", nAssistingCode, NullOrIntStringToVariant(strAssistingBaseUnits), m_nServiceID);
		
		//let everyone else know fields have changed
		CClient::RefreshTable(NetUtils::CPTCodeT, m_nServiceID);
	
	}NxCatchAll(__FUNCTION__)


		CNxDialog::OnOK();

}

// (s.tullis 2014-05-22 09:52) - PLID 62120 - secure controls
void COHIPCPTSetupDlg::SecureControls(){


	try{
		GetDlgItem(IDC_CHECK_ASSISTING_CODE)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(bCanWrite);


	}
	NxCatchAll(__FUNCTION__)


}

void COHIPCPTSetupDlg::OnCheckAssisting(){

	try{

		if (m_checkAssistingCode.GetCheck() == TRUE)
		{
			GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(TRUE);
		}
		else{
			GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(FALSE);
		}
	}
	NxCatchAll(__FUNCTION__)
}

void COHIPCPTSetupDlg::OnBtnOhipPremiumCodes()
{
	try {


		COHIPPremiumCodesDlg dlg(this);
		dlg.m_nDefaultServiceID = m_nServiceID;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

BOOL COHIPCPTSetupDlg::PreTranslateMessage(MSG* pMsg){

	try{
		if (pMsg->message == WM_KEYDOWN)
		{
			if ((pMsg->wParam == VK_RETURN) )
				pMsg->wParam = VK_TAB;
		}



	}NxCatchAll(__FUNCTION__)
		return CDialog::PreTranslateMessage(pMsg);


}