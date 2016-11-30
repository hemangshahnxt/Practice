#include "stdafx.h"
#include "AdditionalServiceCodesSetup.h"
#include "practice.h"
#include "CPTCodes.h"
#include "UBSetupDLG.h"
#include "AdministratorRc.h"
#include "AnesthesiaFacilitySetupDLG.h"


// (s.tullis 2014-05-02 10:17) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;



void CAdditionServiceCodesSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_CHECK_UB ,  m_checkUBSetup);
	DDX_Control(pDX, IDC_CHECK_ANESTHESIA, m_checkAnesthesiaSetup);
	
}



CAdditionServiceCodesSetup::CAdditionServiceCodesSetup(CWnd* pParent)
: CNxDialog(CAdditionServiceCodesSetup::IDD, pParent)
{



}



BEGIN_MESSAGE_MAP(CAdditionServiceCodesSetup, CNxDialog)
	
	ON_BN_CLICKED(IDC_UB_SETUP, OnBtnUBSetup)
	ON_BN_CLICKED(IDC_ANESTHESIA_SETUP, OnBtnAnesethiaSetup)

END_MESSAGE_MAP()




BOOL CAdditionServiceCodesSetup::OnInitDialog(){

	CNxDialog::OnInitDialog();
	try{

		m_btnOk.AutoSet(NXB_CLOSE);

		m_checkUBSetup.EnableWindow(FALSE);

		_RecordsetPtr	tmpRS;
		FieldsPtr Fields;

		CString UB4;
		long UB92CatID;
		long ICD9ProcedureID;

		// check and see if their is a UBsetup
		tmpRS = CreateParamRecordset("Select UBBOX4, UB92Category, ICD9ProcedureCodeID, RevCodeUse FROM serviceT Inner Join CPTCodeT ON CPTCodeT.ID= serviceT.ID WHERE ServiceT.ID = {INT} ", m_nServiceID);

		Fields = tmpRS->Fields;


		UB4 = VarString(Fields->GetItem("UBBox4")->Value, "");


		UB92CatID = VarLong(Fields->GetItem("UB92Category")->Value, -1);

		ICD9ProcedureID = VarLong(Fields->GetItem("ICD9ProcedureCodeID")->Value, -1);





		// need to add anesthesia setup check


		if (UB4.IsEmpty() && UB92CatID == -1 && ICD9ProcedureID == -1)
		{
			m_checkUBSetup.EnableWindow(TRUE);
			m_checkUBSetup.SetCheck(FALSE);
			m_checkUBSetup.EnableWindow(FALSE);

		}
		else{

			m_checkUBSetup.EnableWindow(TRUE);
			m_checkUBSetup.SetCheck(TRUE);
			m_checkUBSetup.EnableWindow(FALSE);
		}
	}NxCatchAll(" Error in Additional Service Codes Setup OnInitDialog")

	return TRUE;

}


void CAdditionServiceCodesSetup::OnBtnUBSetup(){



	try{
		if ( m_nServiceID == -1)
			return;

		BOOL bHasSetup;
		CAnethesiaFacilitySetup dlg(this);
		dlg.m_nServiceID = m_nServiceID;
		bHasSetup = dlg.DoModal();

		
		if (bHasSetup)
		{
			m_checkUBSetup.EnableWindow(TRUE);
			m_checkUBSetup.SetCheck(TRUE);
			m_checkUBSetup.EnableWindow(FALSE);

		}
		else{

			m_checkUBSetup.EnableWindow(TRUE);
			m_checkUBSetup.SetCheck(FALSE);
			m_checkUBSetup.EnableWindow(FALSE);
		}
	

		//reflect any change that could have been made
		
	}NxCatchAll("Error OnUBSetup ");





} 


void CAdditionServiceCodesSetup::OnBtnAnesethiaSetup(){






	try{
		if (m_nServiceID == -1)
			return;

		BOOL bHasSetup;
		CAnethesiaFacilitySetup dlg(this);
		dlg.m_nServiceID = m_nServiceID;
		bHasSetup = dlg.DoModal();


		if (bHasSetup)
		{
			m_checkAnesthesiaSetup.EnableWindow(TRUE);
			m_checkAnesthesiaSetup.SetCheck(TRUE);
			m_checkAnesthesiaSetup.EnableWindow(FALSE);

		}
		else{

			m_checkAnesthesiaSetup.EnableWindow(TRUE);
			m_checkAnesthesiaSetup.SetCheck(FALSE);
			m_checkAnesthesiaSetup.EnableWindow(FALSE);
		}


	}NxCatchAll("Error in OnAnesthesia/Facility Setup")





}


void CAdditionServiceCodesSetup::SetChecks(int nRet){











}