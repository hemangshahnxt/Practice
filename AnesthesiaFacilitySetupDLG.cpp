#include "stdafx.h"
#include "AdditionalServiceCodesSetup.h"
#include "practice.h"
#include "CPTCodes.h"
#include "AnesthesiaFacilitySetupDLG.h"
#include "AnesthesiaSetupDlg.h"
#include "FacilityFeeConfigDlg.h"
#include "AdministratorRc.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (s.tullis 2014-05-21 09:09) - PLID 62023 - Remove the Anesthesia/Facility Setup fuctionality from the Admin Billing dialog to a new dialog that launchs from the Additional Service Code Setup Menu

void CAnesthesiaFacilitySetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_FACILITY_FEE_SETUP, m_btnFacilityFeeSetup);
	DDX_Control(pDX, IDC_BTN_ANESTHESIA_SETUP, m_btnAnesthesiaSetup);
	DDX_Control(pDX, IDC_CHECK_USE_FACILITY_BILLING, m_checkUseFacilityBilling);//s.tullis
	DDX_Control(pDX, IDC_CHECK_USE_ANESTHESIA_BILLING, m_checkUseAnesthesiaBilling);//s.tullis
	DDX_Control(pDX, IDC_CHECK_FACILITY, m_checkFacility);//s.tullis
	DDX_Control(pDX, IDC_CHECK_ANESTHESIA, m_checkAnesthesia);//s.tullis
	DDX_Control(pDX, IDC_ANESTH_BASE_UNITS, m_editAnesthBaseUnits);
	DDX_Control(pDX, IDOK, m_btnOk);


}



CAnesthesiaFacilitySetupDlg::CAnesthesiaFacilitySetupDlg(CWnd* pParent)
: CNxDialog(CAnesthesiaFacilitySetupDlg::IDD, pParent)
{



}



BEGIN_MESSAGE_MAP(CAnesthesiaFacilitySetupDlg, CNxDialog)
	
	
	
	ON_BN_CLICKED(IDC_CHECK_ANESTHESIA, OnCheckAnesthesia)
	//ON_BN_CLICKED(IDC_CHECK_USE_ANESTHESIA_BILLING, OnCheckUseAnesthesiaBilling)
	ON_BN_CLICKED(IDC_BTN_FACILITY_FEE_SETUP, OnBtnFacilityFeeSetup)
	//ON_BN_CLICKED(IDC_CHECK_USE_FACILITY_BILLING, OnCheckUseFacilityBilling)
	ON_BN_CLICKED(IDC_BTN_ANESTHESIA_SETUP, OnBtnAnesthesiaSetup)
	ON_BN_CLICKED(IDC_CHECK_FACILITY, OnCheckFacility)
	ON_BN_CLICKED(IDOK, OnCloseClick)
	

END_MESSAGE_MAP()




// (s.tullis 2014-05-21 09:09) - PLID 62023 - init dialog, load fields 

BOOL CAnesthesiaFacilitySetupDlg::OnInitDialog(){

	CNxDialog::OnInitDialog();
	try{

		m_editAnesthBaseUnits.SetLimitText(5);
		m_btnOk.AutoSet(NXB_CLOSE);



		_RecordsetPtr	tmpRS;
		FieldsPtr Fields;
		_variant_t		tmpVar;
		CString			tmpStr;



		tmpRS = CreateParamRecordset("Select Anesthesia, UseAnesthesiaBilling, FacilityFee,UseFacilityBilling, AnesthBaseUnits FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHere ServiceT.ID = {INT} ", m_nServiceID);

		if (!tmpRS->eof){
			Fields = tmpRS->Fields;


			tmpVar = Fields->GetItem("Anesthesia")->Value;
			if (tmpVar.vt == VT_BOOL && tmpVar.boolVal) {
				m_checkAnesthesia.SetCheck(TRUE);
			}
			else {
				m_checkAnesthesia.SetCheck(FALSE);
			}

			tmpVar = Fields->GetItem("UseAnesthesiaBilling")->Value;
			if (tmpVar.vt == VT_BOOL && tmpVar.boolVal) {
				m_checkUseAnesthesiaBilling.SetCheck(TRUE);
			}
			else {
				m_checkUseAnesthesiaBilling.SetCheck(FALSE);
			}

			tmpVar = Fields->GetItem("FacilityFee")->Value;
			if (tmpVar.vt == VT_BOOL && tmpVar.boolVal) {
				m_checkFacility.SetCheck(TRUE);
			}
			else {
				m_checkFacility.SetCheck(FALSE);
			}

			tmpVar = Fields->GetItem("UseFacilityBilling")->Value;
			if (tmpVar.vt == VT_BOOL && tmpVar.boolVal) {
				m_checkUseFacilityBilling.SetCheck(TRUE);
			}
			else {
				m_checkUseFacilityBilling.SetCheck(FALSE);
			}

			CString str;
			tmpVar = Fields->GetItem("AnesthBaseUnits")->Value;
			if (tmpVar.vt == VT_I4)
				str.Format("%li", tmpVar.lVal);
			else str = "";
			SetDlgItemText(IDC_ANESTH_BASE_UNITS, str);
		}
		CheckEnableAnesthesiaFacilityAssistingControls();
		

		
		tmpRS->Close();
		if (!bCanWrite){// if user is not able to write secure controls
			SecureControls();
		}

	}NxCatchAll("Error in Oninit Anesthesia/Facility Setup")

	
		
	return TRUE;
}


// (s.tullis 2014-05-21 09:09) - PLID 62023 - enable anesthesia controls and disable facility controls
void CAnesthesiaFacilitySetupDlg::OnCheckAnesthesia(){


	try {

		//only one can be toggled at a time
		if (m_checkAnesthesia.GetCheck() && m_checkFacility.GetCheck()) {
			m_checkFacility.SetCheck(FALSE);
			m_checkUseFacilityBilling.SetCheck(FALSE);
		}
	
	
		if (!m_checkAnesthesia.GetCheck()) {
			m_checkUseAnesthesiaBilling.SetCheck(FALSE);
		}

		
		CheckEnableAnesthesiaFacilityAssistingControls();

	}NxCatchAll("Error updating Anesthesia status.");




}

// (s.tullis 2014-05-21 09:09) - PLID 62023 - fuction that enables and disables controls based on permissions and checks
void CAnesthesiaFacilitySetupDlg::CheckEnableAnesthesiaFacilityAssistingControls(){


	try {

		BOOL bHasPermission = (GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS);
		BOOL bAnesthesiaIsChecked = FALSE;
		BOOL bFacilityIsChecked = FALSE;
		

		//these are mutually exclusive
		if (m_checkAnesthesia.GetCheck()) {
			bAnesthesiaIsChecked = TRUE;
		}
		else if (m_checkFacility.GetCheck()) {
			bFacilityIsChecked = TRUE;
		}

		//now update each field
		GetDlgItem(IDC_CHECK_USE_ANESTHESIA_BILLING)->EnableWindow(bHasPermission && bAnesthesiaIsChecked);
		GetDlgItem(IDC_CHECK_USE_FACILITY_BILLING)->EnableWindow(bHasPermission && bFacilityIsChecked);

		GetDlgItem(IDC_ANESTH_BASE_UNITS)->EnableWindow(bHasPermission && bAnesthesiaIsChecked);
		
		GetDlgItem(IDC_BTN_ANESTHESIA_SETUP)->EnableWindow(bHasPermission && (bAnesthesiaIsChecked ));//bAnesthesiaIsChecked|| bAssistingCodeIsChecked
		GetDlgItem(IDC_BTN_FACILITY_FEE_SETUP)->EnableWindow(bHasPermission && bFacilityIsChecked);

	}NxCatchAll(__FUNCTION__);


}




// (s.tullis 2014-05-21 09:09) - PLID 62023 - Launch Facility Fee Setup
void  CAnesthesiaFacilitySetupDlg::OnBtnFacilityFeeSetup(){


	try{

		CFacilityFeeConfigDlg dlg(this);
		dlg.m_nServiceID = m_nServiceID;
		dlg.m_strServiceCode = m_strServCode;
		dlg.DoModal();


	}NxCatchAll("Error in facility fee setup");

}



// (s.tullis 2014-05-21 09:09) - PLID 62023 - Launch Anesthesia Setup
void CAnesthesiaFacilitySetupDlg::OnBtnAnesthesiaSetup(){



	try{

			CAnesthesiaSetupDlg dlg(this);
			dlg.m_nServiceID = m_nServiceID;
			dlg.m_strServiceCode = m_strServCode;
			dlg.DoModal();
		
		

	}NxCatchAll(__FUNCTION__);
	


}
// (s.tullis 2014-05-21 09:09) - PLID 62023 - Enable Facility and disable Anesthesia
void CAnesthesiaFacilitySetupDlg::OnCheckFacility(){
	try{
		if (m_checkAnesthesia.GetCheck() && m_checkFacility.GetCheck()) {

			m_checkAnesthesia.SetCheck(FALSE);
			m_checkUseAnesthesiaBilling.SetCheck(FALSE);

		}
		if (!m_checkFacility.GetCheck()){

			m_checkUseFacilityBilling.SetCheck(FALSE);

		}

		CheckEnableAnesthesiaFacilityAssistingControls();
	}
	NxCatchAll(__FUNCTION__)

}

// (s.tullis 2014-05-21 09:22) - PLID 62023 - save and close 
void CAnesthesiaFacilitySetupDlg::OnCloseClick(){


	try{
		CString	 anesthbaseunits;
		CString strSqlBatch = "";
		CNxParamSqlArray aryParams;


		
		long  anesthesia, useanesthesia, facilityfee, usefacilityfee;

		GetDlgItemText(IDC_ANESTH_BASE_UNITS, anesthbaseunits);
		anesthbaseunits = anesthbaseunits.SpanIncluding("0123456789");


		if (m_checkAnesthesia.GetCheck())
			anesthesia = 1;
		else
			anesthesia = 0;

		if (m_checkUseAnesthesiaBilling.GetCheck())
			useanesthesia = 1;
		else
			useanesthesia = 0;

		if (m_checkFacility.GetCheck())
			facilityfee = 1;
		else
			facilityfee = 0;

		if (m_checkUseFacilityBilling.GetCheck())
			usefacilityfee = 1;
		else
			usefacilityfee = 0;


		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ServiceT Set Anesthesia = {INT}, UseAnesthesiaBilling = {INT}, FacilityFee = {INT}, UseFacilityBilling = {INT} WHERE ServiceT.ID = {INT} ", anesthesia, useanesthesia, facilityfee, usefacilityfee, m_nServiceID);

		AddParamStatementToSqlBatch(strSqlBatch, aryParams, " UPDATE CPTCODET SET AnesthBaseUnits= {STRING} WHERE ID ={INT} ", anesthbaseunits, m_nServiceID);


		if (!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			//let everyone else know fields have changed
			CClient::RefreshTable(NetUtils::CPTCodeT, m_nServiceID);
		}



		

	}NxCatchAll("Error closing Anesthesia/Facility Setup")


		CNxDialog::OnOK();
}

// (s.tullis 2014-05-21 09:22) - PLID 62023 - secure controls 
void  CAnesthesiaFacilitySetupDlg::SecureControls(){


	try{
		GetDlgItem(IDC_CHECK_ANESTHESIA)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_ANESTH_BASE_UNITS)->EnableWindow(bCanWrite);
		GetDlgItem(IDC_BTN_ANESTHESIA_SETUP)->EnableWindow(bCanWrite);
	}
	NxCatchAll(__FUNCTION__)

}
