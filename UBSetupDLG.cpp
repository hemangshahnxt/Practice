// CUBSetupDLG.cpp : implementation file
// (s.tullis 2014-04-29 14:51) - PLID 61893 - Remove the UB fields from Admin Billing and place them in a new UB dialog

#include "stdafx.h"
#include "practice.h"
#include "CPTCodes.h"
#include "UBSetupDLG.h"
#include "OHIPPremiumCodesDlg.h"
#include "MultipleRevCodesDlg.h"
#include "AdministratorRc.h"
#include "ICD9ProcedureSetupDlg.h"
#include "AdvRevCodeSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;






CUBSetupDLG::CUBSetupDLG(CWnd* pParent)
	: CNxDialog(CUBSetupDLG::IDD, pParent)
{

		
		
}




void CUBSetupDLG::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADV_REVCODE_SETUP, m_btnAdvRevCodeSetup);
	DDX_Control(pDX, IDC_BTN_SETUP_ICD9V3, m_btnSetupICD9v3);
	DDX_Control(pDX, IDC_UB_BOX4, m_editUBBox4);
	DDX_Control(pDX, IDC_BTN_EDIT_MULTIPLE_REV_CODES, m_btnEditMultipleRevCodes);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_CHECK_MULTIPLE_REV_CODES, m_checkMultipleRevCodes);
	DDX_Control(pDX, IDC_CHECK_SINGLE_REV_CODE, m_checkSingleRevCode);
}


BEGIN_MESSAGE_MAP(CUBSetupDLG, CNxDialog)
ON_BN_CLICKED(IDC_BTN_EDIT_MULTIPLE_REV_CODES, OnBtnEditMultipleRevCodes)
ON_BN_CLICKED(IDC_BTN_SETUP_ICD9V3, OnBtnSetupIcd9v3)
ON_BN_CLICKED(IDOK,OnOKClick)
ON_BN_CLICKED(IDC_CHECK_SINGLE_REV_CODE, OnCheckSingleRevCode)
ON_BN_CLICKED(IDC_CHECK_MULTIPLE_REV_CODES, OnCheckMultipleRevCodes)
ON_BN_CLICKED(IDC_ADV_REVCODE_SETUP, OnAdvRevcodeSetup)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CUBSetupDLG, CNxDialog)
END_EVENTSINK_MAP()

// (s.tullis 2014-05-20 17:42) - PLID 61893 - Init dialog , query the fields and load them
BOOL CUBSetupDLG::OnInitDialog(){

		CNxDialog::OnInitDialog();

		try{

			//set buttons
			m_btnOk.AutoSet(NXB_CLOSE);
			m_btnEditMultipleRevCodes.AutoSet(NXB_MODIFY);
			m_editUBBox4.SetLimitText(20);

			_RecordsetPtr	tmpRS;
			FieldsPtr Fields;
			NXDATALIST2Lib::IRowSettingsPtr pRow;


			m_UB92_Category = BindNxDataList2Ctrl(IDC_UB92_CPT_CATEGORIES, true);

			
			pRow = m_UB92_Category->GetNewRow();
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, "<None>");
			pRow->PutValue(2, "<No Category Selected>");
			m_UB92_Category->AddRowSorted(pRow, NULL);


			m_ICD9V3List = BindNxDataList2Ctrl(IDC_LIST_ICD9V3, GetRemoteData(), true);
			NXDATALIST2Lib::IRowSettingsPtr pNoRow = m_ICD9V3List->GetNewRow();
			if (pNoRow) {
				pNoRow->PutValue(0, _variant_t(long(-1)));
				pNoRow->PutValue(1, _variant_t(""));
				pNoRow->PutValue(2, _variant_t("<None>"));

				m_ICD9V3List->AddRowSorted(pNoRow, NULL);
			}




			tmpRS = CreateParamRecordset("Select UBBOX4, UB92Category, ICD9ProcedureCodeID, RevCodeUse FROM serviceT Inner Join CPTCodeT ON CPTCodeT.ID= serviceT.ID WHERE ServiceT.ID = {INT} ", m_nServiceID);
			if (!tmpRS->eof){
				Fields = tmpRS->Fields;


				strUB4 = VarString(Fields->GetItem("UBBox4")->Value, "");


				nUB92CatID = VarLong(Fields->GetItem("UB92Category")->Value, -1);

				nICD9ProcedureID = VarLong(Fields->GetItem("ICD9ProcedureCodeID")->Value, -1);


				nRevCode = VarLong(Fields->GetItem("RevCodeUse")->Value, 0);


				SetDlgItemText(IDC_UB_BOX4, strUB4);


				m_UB92_Category->SetSelByColumn(0, nUB92CatID);

				m_ICD9V3List->SetSelByColumn(0, nICD9ProcedureID);


				if (nRevCode == 1) {
					//single
					m_checkSingleRevCode.SetCheck(TRUE);
					m_checkMultipleRevCodes.SetCheck(FALSE);
					m_UB92_Category->Enabled = TRUE;
					m_btnEditMultipleRevCodes.EnableWindow(FALSE);
				}
				else if (nRevCode == 2) {
					//multiple
					m_checkSingleRevCode.SetCheck(FALSE);
					m_checkMultipleRevCodes.SetCheck(TRUE);
					m_UB92_Category->Enabled = FALSE;

					m_btnEditMultipleRevCodes.EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS);
				}
				else {
					//none
					m_checkSingleRevCode.SetCheck(FALSE);
					m_checkMultipleRevCodes.SetCheck(FALSE);
					m_UB92_Category->Enabled = FALSE;
					m_btnEditMultipleRevCodes.EnableWindow(FALSE);

				}


			}

			tmpRS->Close();
			if (!bCanWrite){//no writing privilidges need to secure the controls
				SecureControls();
			}

		}NxCatchAll("Error in UB Setup OnInitDialog");



		return TRUE;


}



// (s.tullis 2014-05-20 17:42) - PLID 61893 - Save the fields if they have changed
void CUBSetupDLG::OnOKClick(){


	try{
		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		CString strUBbuffer;
		long nRevCodeNew=0;
		long nUB92category;
		long nICD9ID;
		BOOL bFirst=TRUE;
		_variant_t vtUBcategory;
		_variant_t vtICD9ProcID;

		
		GetDlgItemText(IDC_UB_BOX4, strUBbuffer);

		if ( m_checkSingleRevCode.GetCheck() == TRUE ){
			nRevCodeNew = 1;

		}
		else if (m_checkMultipleRevCodes.GetCheck() == TRUE){
			nRevCodeNew = 2;
		}
		//row pointer to UB92 list
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_UB92_Category->GetCurSel());
		// get actual value for change comparison
		nUB92category = VarLong(pRow->GetValue(0), -1);
		//get variant value from list if default <none> need to update null
		vtUBcategory = (VarLong(pRow->GetValue(0), -1) == -1) ? g_cvarNull : pRow->GetValue(0); 
		//set row point to ICD( Procedures List
		pRow = m_ICD9V3List->GetCurSel();
		// get actual value for change comparison
		nICD9ID = VarLong(pRow->GetValue(0), -1);
		//get variant value from list if default <none> need to update null
		vtICD9ProcID = (VarLong(pRow->GetValue(0), -1) == -1) ? g_cvarNull : pRow->GetValue(0);


		
		//if a field has changed add it to the query to be updated
		if (nRevCodeNew != nRevCode){
				
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, " UPDATE ServiceT SET ServiceT.RevCodeUse = {INT} ", nRevCodeNew);
			bFirst = FALSE;
		}

		if (nUB92category != nUB92CatID){

			if (bFirst == TRUE){

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, " UPDATE ServiceT SET ServiceT.UB92Category = {VT_I4} ", vtUBcategory);
				bFirst = FALSE;
			}
			else{

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, ", ServiceT.UB92Category = {VT_I4} ", vtUBcategory);

			}
		}

		if (strUB4.Compare(strUBbuffer) != 0){


			if (bFirst == TRUE){

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, " UPDATE ServiceT SET ServiceT.UBBox4 = {STRING} ", strUBbuffer);
				bFirst = FALSE;
			}
			else{

				AddParamStatementToSqlBatch(strSqlBatch, aryParams, ", ServiceT.UBBox4 = {STRING} ", strUBbuffer);

			}

		}

		if (bFirst == FALSE){
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, " WHERE ServiceT.ID = {INT} ;", m_nServiceID);
		}
		


		if (nICD9ID != nICD9ProcedureID){


			AddParamStatementToSqlBatch(strSqlBatch, aryParams, " UPDATE CPTCodeT SET CPTCodeT.ICD9ProcedureCodeID = { VT_I4 }  WHERE CPTCodeT.ID = { INT } ", vtICD9ProcID, m_nServiceID);

		}

		

		if(!strSqlBatch.IsEmpty()) {
						ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

						//let everyone else know fields have changed
						CClient::RefreshTable(NetUtils::CPTCodeT, m_nServiceID);
					}

		

		CDialog::OnOK();

	}NxCatchAll("Error On saving UB setup");



}





// (s.tullis 2014-05-20 17:42) - PLID 61893 - launch MultipleRevCodesDLG
void CUBSetupDLG::OnBtnEditMultipleRevCodes() 
{
	try{
		if(m_nServiceID == -1)
			return;

		CMultipleRevCodesDlg dlg(this);
		dlg.m_nServiceID = m_nServiceID;
		dlg.DoModal();


	}NxCatchAll("Error editing multiple revenue code setup");
}

// (s.tullis 2014-05-20 17:42) - PLID 61893 - Launch CICD9ProcedureSetupDlg and requery the datalist and update
void CUBSetupDLG::OnBtnSetupIcd9v3() 
{
	
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_ICD9V3List->GetCurSel());

		CICD9ProcedureSetupDlg dlg(this);
		dlg.DoModal();

		
		_RecordsetPtr prs = CreateParamRecordset("SELECT ICD9ProcedureCodeID FROM CPTCodeT WHERE ID = {INT}", m_nServiceID);

		long nID = -1;

		if (!prs->eof) {
			nID = AdoFldLong(prs, "ICD9ProcedureCodeID", -1);
		}

		m_ICD9V3List->Requery();
		NXDATALIST2Lib::IRowSettingsPtr pNoRow = m_ICD9V3List->GetNewRow();
		if (pNoRow) {
			pNoRow->PutValue(0, _variant_t(long(-1)));
			pNoRow->PutValue(1, _variant_t(""));
			pNoRow->PutValue(2, _variant_t("<None>"));

			m_ICD9V3List->AddRowSorted(pNoRow, NULL);
		}

		
		m_ICD9V3List->TrySetSelByColumn_Deprecated(0, _variant_t(nID));

	} NxCatchAll("Error setting up ICD9 Procedure codes");
}





// (s.tullis 2014-05-20 17:46) - PLID 61893 - Set Controls Enabled or Disabled based on if the Single Revenue Code check box is checked
void CUBSetupDLG::OnCheckSingleRevCode() 
{
	
	try	{

	if(m_checkSingleRevCode.GetCheck()) {
		//single
	
		m_checkMultipleRevCodes.SetCheck(FALSE);
		m_UB92_Category->Enabled = TRUE;
		m_btnEditMultipleRevCodes.EnableWindow(FALSE);

	}
	else {
		//none
		
		m_checkMultipleRevCodes.SetCheck(FALSE);
		m_UB92_Category->Enabled = FALSE;
		m_btnEditMultipleRevCodes.EnableWindow(FALSE);
	}

	
	}NxCatchAll(__FUNCTION__);
}
// (s.tullis 2014-05-20 17:46) - PLID 61893 - Set Controls Enabled or Disabled based on if the Multiple Revenue Code check box is checked
void CUBSetupDLG::OnCheckMultipleRevCodes() {
	
	try	{
		if(m_checkMultipleRevCodes.GetCheck()) {
			//multiple
			m_checkSingleRevCode.SetCheck(FALSE);
			m_UB92_Category->Enabled= FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(TRUE);
		}
		else {
			//none
			m_checkSingleRevCode.SetCheck(FALSE);
			m_UB92_Category->Enabled =FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2014-04-29 17:17) - PLID 61893 - Secure Controls if User can not write
void CUBSetupDLG::SecureControls()
{
	
	

	GetDlgItem(IDC_CHECK_SINGLE_REV_CODE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_UB92_CPT_CATEGORIES)->EnableWindow(bCanWrite); 
	GetDlgItem(IDC_CHECK_MULTIPLE_REV_CODES)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_BTN_EDIT_MULTIPLE_REV_CODES)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_ADV_REVCODE_SETUP)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_UB_BOX4)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_LIST_ICD9V3)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_BTN_SETUP_ICD9V3)->EnableWindow(bCanWrite);
	




}

void CUBSetupDLG::OnAdvRevcodeSetup() {
	try{
		CAdvRevCodeSetupDlg dlg(this);
		dlg.DoModal();

		//reflect any change that could have been made
		//Refresh();
	}NxCatchAll("Error managing advanced revenue code setup");
}



BOOL CUBSetupDLG::PreTranslateMessage(MSG* pMsg){


	if (pMsg->message == WM_KEYDOWN)
	{
		if ((pMsg->wParam == VK_RETURN) )
			pMsg->wParam = VK_TAB;
	}
	return CDialog::PreTranslateMessage(pMsg);


}