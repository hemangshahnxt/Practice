// GlassesCatalogBillingSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InventoryRc.h"
#include "GlassesCatalogBillingSetupDlg.h"
#include <GlobalParamUtils.h>
#include "AuditTrail.h"
#include "MultiSelectDlg.h"
#include "GlobalStringUtils.h"
#include "InvSelectMultipleCPTDlg.h"
// CGlassesCatalogBillingSetupDlg dialog

IMPLEMENT_DYNAMIC(CGlassesCatalogBillingSetupDlg, CNxDialog)

CGlassesCatalogBillingSetupDlg::CGlassesCatalogBillingSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGlassesCatalogBillingSetupDlg::IDD, pParent)
{

}

CGlassesCatalogBillingSetupDlg::~CGlassesCatalogBillingSetupDlg()
{
}

void CGlassesCatalogBillingSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	
	DDX_Control(pDX, IDC_BILLING_SETUP_LABEL, m_nxsBillingSetupLabel);
	DDX_Control(pDX, IDC_DESIGNS, m_nxbDesigns);
	DDX_Control(pDX, IDC_MATERIALS, m_nxbMaterials);
	DDX_Control(pDX, IDC_TREATMENTS, m_nxbTreatments);
	
}


BEGIN_MESSAGE_MAP(CGlassesCatalogBillingSetupDlg, CNxDialog)
	
	
	ON_BN_CLICKED(IDC_DESIGNS, &CGlassesCatalogBillingSetupDlg::OnDesigns)
	ON_BN_CLICKED(IDC_MATERIALS, &CGlassesCatalogBillingSetupDlg::OnMaterials)
	ON_BN_CLICKED(IDC_TREATMENTS, &CGlassesCatalogBillingSetupDlg::OnTreatments)
	
END_MESSAGE_MAP()

//TES 5/20/2011 - PLID 43698 - NOTE: The code all depends on m_pDesignList, m_pMaterialList, and m_pTreatmentList having the same columns.
// (j.dinatale 2013-03-28 10:09) - PLID 55919 - enum additions for the prompt column
enum GlassesCatalogListColumns {
	gclcID = 0,
	gclcName = 1,
	gclcCpt = 2,
	gclcCptID = 3,
	gclcSavedCptID = 4,
	gclcSavedServiceCodes =5,
	gclcBillPerLens = 6,
	gclcSavedBillPerLens = 7,
	gclcPrompt = 8,
	gclcPromptSaved = 9,
};
// CGlassesCatalogBillingSetupDlg message handlers

BOOL CGlassesCatalogBillingSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//TES 5/20/2011 - PLID 43698 - Set the label explaining what this dialog does.
		// (s.dhole 2012-05-15 11:46) - PLID 48357 
		SetDlgItemText(IDC_BILLING_SETUP_LABEL, "This dialog allows you to associate service code(s) with a glasses item.  You may associate a service code by clicking the Service Code field,  Bill Per Lens determines whether a separate charge will be entered for each lens on the Glasses Order; if the box is unchecked, only one charge will be added for that item on each order.");

		//TES 5/20/2011 - PLID 43698 - NxIconify
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		
		

		//TES 5/20/2011 - PLID 43698 - Bind our datalists
		// (s.dhole 2012-05-01 18:14) - PLID 48357 set forecolor
		m_pDesignList = BindNxDataList2Ctrl(IDC_DESIGN_LIST);
		m_pMaterialList = BindNxDataList2Ctrl(IDC_MATERIAL_LIST);
		m_pTreatmentList = BindNxDataList2Ctrl(IDC_TREATMENT_LIST);
		m_pDesignList->GetColumn(gclcCpt)->PutForeColor(RGB(0,0,255));
		m_pMaterialList->GetColumn(gclcCpt)->PutForeColor (RGB(0,0,255));
		m_pTreatmentList->GetColumn(gclcCpt)->PutForeColor(RGB(0,0,255));
		//TES 5/20/2011 - PLID 43698 - Default to viewing Designs
		CheckDlgButton(IDC_DESIGNS, BST_CHECKED);
		OnDesigns();
	
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}



enum GlassesListType {
	lstDesign = 0,
	lstTreatment = 1,
	lstMaterial = 2,
};


enum CptListColumns {
	clcID = 0,
	clcCode = 1,
	clcSubCode = 2,
	clcName = 3,
	clcPrice = 4,
};

using namespace NXDATALIST2Lib;
using namespace ADODB;
// (s.dhole 2012-03-06 09:20) - PLID 48357 Change function to support multiple service code selection
void CGlassesCatalogBillingSetupDlg::OnOK()
{
	try {
		//TES 5/20/2011 - PLID 43698 - Go through each list, find everything that changed.
		CString strSql;
		CNxParamSqlArray aryParams;
		long nAuditID = -1;
		
		//TES 5/20/2011 - PLID 43698 - Designs
		IRowSettingsPtr pRow = m_pDesignList->GetFirstRow();
		while(pRow) {
			long nDesignID = VarLong(pRow->GetValue(gclcID));
			CString strDesign = VarString(pRow->GetValue(gclcName));
			CString  strCptID = VarString(pRow->GetValue(gclcCptID),"");
			CString   strSavedCptID = VarString(pRow->GetValue(gclcSavedCptID),"");
			CString   strSavedServiceCodes = VarString(pRow->GetValue(gclcSavedServiceCodes ),"");
			CString   strServiceCodes = VarString(pRow->GetValue(gclcCpt  ),"");
			if (strServiceCodes.CollateNoCase("      <None>")  == 0 ){
				strServiceCodes ="";
			}
			if(IsServiceCodeChange(strCptID,strSavedCptID) == TRUE) {
				CStringArray strIDArray;
				SplitString(strSavedCptID,"," ,&strIDArray);
				for(int i = 0; i < strIDArray.GetSize(); i++) {
					AddParamStatementToSqlBatch(strSql, aryParams,
						"Delete  From GlassesCatalogDesignsCptT WHERE  GlassesCatalogDesignsID = {INT}  AND cptID = {INT}   ",
						nDesignID,atol( strIDArray[i]));
					}
				strIDArray.RemoveAll();
				SplitString(strCptID,"," ,&strIDArray);
				for(int i = 0; i < strIDArray.GetSize(); i++) {

				AddParamStatementToSqlBatch(strSql, aryParams, 
				
						" IF NOT exists (Select cptID FROM GlassesCatalogDesignsCptT Where cptID = {INT} AND GlassesCatalogDesignsID = {INT} ) " 		 
							" BEGIN "
									" INSERT INTO GlassesCatalogDesignsCptT (cptID , GlassesCatalogDesignsID)  VALUES  ( {INT},{INT} ) " 		 
							" END"
					,atol( strIDArray[i]), nDesignID,atol( strIDArray[i]),nDesignID);

				}	
				if(nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}
				AuditEvent(-1, "", nAuditID, aeiGlassesDesignCpt, nDesignID, strDesign + ": " + strSavedServiceCodes , strServiceCodes , aepMedium);
			}
			//TES 5/20/2011 - PLID 43698 - Bill Per Lens is only valid if there's a CPT selected.
			// (j.dinatale 2013-03-27 17:26) - PLID 55919 - save whether the user wants a prompt or not
			if (!strCptID.IsEmpty()) {
				BOOL bBillPerLens = VarBool( pRow->GetValue(gclcBillPerLens));
				BOOL bSavedBillPerLens = VarBool(pRow->GetValue(gclcSavedBillPerLens));
				BOOL bPrompt = VarBool(pRow->GetValue(gclcPrompt));
				BOOL bSavedPrompt = VarBool(pRow->GetValue(gclcPromptSaved));

				if(bBillPerLens != bSavedBillPerLens || bPrompt != bSavedPrompt) {
					AddParamStatementToSqlBatch(strSql, aryParams, "UPDATE GlassesCatalogDesignsT SET BillPerLens = {BOOL}, ShowPrompt = {BOOL} WHERE ID = {INT}",
						bBillPerLens, bPrompt, nDesignID);
					if(nAuditID == -1) {
						nAuditID = BeginNewAuditEvent();
					}

					if(bBillPerLens != bSavedBillPerLens){
						AuditEvent(-1, "", nAuditID, aeiGlassesDesignBillPerLens, nDesignID, strDesign + ": " + CString(bSavedBillPerLens?"Yes":"No"), bBillPerLens?"Yes":"No", aepMedium);
					}

					if(bPrompt != bSavedPrompt){
						AuditEvent(-1, "", nAuditID, aeiGlassesDesignPrompt, nDesignID, strDesign + ": " + CString(bSavedPrompt?"Yes":"No"), bPrompt?"Yes":"No", aepMedium);
					}
				}
			}

			pRow = pRow->GetNextRow();
		}

		
		pRow = m_pMaterialList->GetFirstRow();
		while(pRow) {
			long nMaterialID = VarLong(pRow->GetValue(gclcID));
			CString strMaterial = VarString(pRow->GetValue(gclcName));
			CString  strCptID = VarString(pRow->GetValue(gclcCptID),"");
			CString   strSavedCptID = VarString(pRow->GetValue(gclcSavedCptID),"");
			CString   strSavedServiceCodes = VarString(pRow->GetValue(gclcSavedServiceCodes ),"");
			CString   strServiceCodes = VarString(pRow->GetValue(gclcCpt  ),"");
			if (strServiceCodes.CollateNoCase("      <None>")  == 0 ){
				strServiceCodes ="";
			}
			if(IsServiceCodeChange(strCptID,strSavedCptID) == TRUE) {
				CStringArray strIDArray;
				SplitString(strSavedCptID,"," ,&strIDArray);
				for(int i = 0; i < strIDArray.GetSize(); i++) {
					AddParamStatementToSqlBatch(strSql, aryParams,
						"Delete  From GlassesCatalogMaterialsCptT WHERE  GlassesCatalogMaterialsID = {INT}  AND cptID = {INT}   ",
						nMaterialID,atol( strIDArray[i]));
					}
				strIDArray.RemoveAll();
				SplitString(strCptID,"," ,&strIDArray);
				for(int i = 0; i < strIDArray.GetSize(); i++) {

				AddParamStatementToSqlBatch(strSql, aryParams, 
				
						" IF NOT exists (Select cptID FROM GlassesCatalogMaterialsCptT Where cptID = {INT} AND GlassesCatalogMaterialsID = {INT} ) " 		 
							" BEGIN "
									" INSERT INTO GlassesCatalogMaterialsCptT (cptID , GlassesCatalogMaterialsID)  VALUES  ( {INT},{INT} ) " 		 
							" END"
					,atol( strIDArray[i]), nMaterialID,atol( strIDArray[i]),nMaterialID);
				}	
				if(nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}
				AuditEvent(-1, "", nAuditID, aeiGlassesMaterialCpt, nMaterialID, strMaterial + ": " + strSavedServiceCodes , strServiceCodes , aepMedium);
			}

			// (j.dinatale 2013-03-27 17:26) - PLID 55919 - save whether the user wants a prompt or not
			if (!strCptID.IsEmpty()) {
				BOOL bBillPerLens = VarBool( pRow->GetValue(gclcBillPerLens));
				BOOL bSavedBillPerLens = VarBool(pRow->GetValue(gclcSavedBillPerLens));
				BOOL bPrompt = VarBool(pRow->GetValue(gclcPrompt));
				BOOL bSavedPrompt = VarBool(pRow->GetValue(gclcPromptSaved));

				if(bBillPerLens != bSavedBillPerLens || bPrompt != bSavedPrompt) {
					AddParamStatementToSqlBatch(strSql, aryParams, "UPDATE GlassesCatalogMaterialsT SET BillPerLens = {BOOL}, ShowPrompt = {BOOL} WHERE ID = {INT}",
						bBillPerLens, bPrompt, nMaterialID);
					if(nAuditID == -1) {
						nAuditID = BeginNewAuditEvent();
					}

					if(bBillPerLens != bSavedBillPerLens){
						AuditEvent(-1, "", nAuditID, aeiGlassesMaterialBillPerLens, nMaterialID, strMaterial + ": " + CString(bSavedBillPerLens?"Yes":"No"), bBillPerLens?"Yes":"No", aepMedium);
					}

					if(bPrompt != bSavedPrompt){
						AuditEvent(-1, "", nAuditID, aeiGlassesMaterialPrompt, nMaterialID, strMaterial + ": " + CString(bSavedPrompt?"Yes":"No"), bPrompt?"Yes":"No", aepMedium);
					}
				}
			}

			pRow = pRow->GetNextRow();
		}

		//TES 5/20/2011 - PLID 43698 - Treatments
		pRow = m_pTreatmentList->GetFirstRow();
		while(pRow) {
			long nTreatmentID = VarLong(pRow->GetValue(gclcID));
			CString strTreatment = VarString(pRow->GetValue(gclcName));
			CString  strCptID = VarString(pRow->GetValue(gclcCptID),"");
			CString   strSavedCptID = VarString(pRow->GetValue(gclcSavedCptID),"");
			CString   strSavedServiceCodes = VarString(pRow->GetValue(gclcSavedServiceCodes ),"");
			CString   strServiceCodes = VarString(pRow->GetValue(gclcCpt  ),"");
			if (strServiceCodes.CollateNoCase("      <None>")  == 0 ){
				strServiceCodes ="";
			}
			if(IsServiceCodeChange(strCptID,strSavedCptID) == TRUE) {
				CStringArray strIDArray;
				SplitString(strSavedCptID,"," ,&strIDArray);
				for(int i = 0; i < strIDArray.GetSize(); i++) {
					AddParamStatementToSqlBatch(strSql, aryParams,
						"Delete  From GlassesCatalogTreatmentsCptT WHERE  GlassesCatalogTreatmentsID = {INT}  AND cptID = {INT}   ",
						nTreatmentID,atol( strIDArray[i]));
					}
				strIDArray.RemoveAll();
				SplitString(strCptID,"," ,&strIDArray);
				for(int i = 0; i < strIDArray.GetSize(); i++) {

				AddParamStatementToSqlBatch(strSql, aryParams, 
						" IF NOT exists (Select cptID FROM GlassesCatalogTreatmentsCptT Where cptID = {INT} AND GlassesCatalogTreatmentsID  = {INT} ) " 		 
							" BEGIN "
									" INSERT INTO GlassesCatalogTreatmentsCptT (cptID , GlassesCatalogTreatmentsID )  VALUES  ( {INT},{INT} ) " 		 
							" END"
					,atol( strIDArray[i]), nTreatmentID,atol( strIDArray[i]),nTreatmentID);
				}	
				if(nAuditID == -1) {
					nAuditID = BeginNewAuditEvent();
				}
				AuditEvent(-1, "", nAuditID, aeiGlassesTreatmentCpt, nTreatmentID, strTreatment + ": " + strSavedServiceCodes , strServiceCodes , aepMedium);
			}

			if (!strCptID.IsEmpty()) {
				BOOL bBillPerLens = VarBool( pRow->GetValue(gclcBillPerLens));
				BOOL bSavedBillPerLens = VarBool(pRow->GetValue(gclcSavedBillPerLens));

				if(bBillPerLens != bSavedBillPerLens) {
					AddParamStatementToSqlBatch(strSql, aryParams, "UPDATE GlassesCatalogTreatmentsT SET BillPerLens = {BOOL} WHERE ID = {INT}",
						bBillPerLens, nTreatmentID);

					if(nAuditID == -1) {
						nAuditID = BeginNewAuditEvent();
					}

					if(bBillPerLens != bSavedBillPerLens){
						AuditEvent(-1, "", nAuditID, aeiGlassesTreatmentBillPerLens, nTreatmentID, strTreatment + ": " + CString(bSavedBillPerLens?"Yes":"No"), bBillPerLens?"Yes":"No", aepMedium);
					}
				}
			}
			
			pRow = pRow->GetNextRow();
		}

		//TES 5/20/2011 - PLID 43698 - If we found any changes, commit them here.
		if(!strSql.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSql, aryParams);
			CommitAuditTransaction(nAuditID);
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CGlassesCatalogBillingSetupDlg, CNxDialog)
	
	ON_EVENT(CGlassesCatalogBillingSetupDlg, IDC_DESIGN_LIST, 19, CGlassesCatalogBillingSetupDlg::LeftClickDesignList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGlassesCatalogBillingSetupDlg, IDC_TREATMENT_LIST, 19, CGlassesCatalogBillingSetupDlg::LeftClickTreatmentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGlassesCatalogBillingSetupDlg, IDC_MATERIAL_LIST, 19, CGlassesCatalogBillingSetupDlg::LeftClickMaterialList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (s.dhole 2012-03-06 09:20) - PLID 48357 Check if there is change in code selection
BOOL CGlassesCatalogBillingSetupDlg::IsServiceCodeChange(CString  strCptIds,  CString  strSavedCptIds)
{
	BOOL bResult = FALSE;
	strCptIds.Replace(" ","");
	strSavedCptIds.Replace(" ",""); 
	// Check string value and lenght. if those does not match thne return True
	if (strCptIds != strSavedCptIds|| strCptIds.GetLength() != strSavedCptIds.GetLength()){
		return TRUE;
	}
	else{
		CStringArray strIDsArray;
		SplitString(strCptIds,"," ,&strIDsArray);
		CStringArray strSavedIDsArray;
		SplitString(strSavedCptIds,"," ,&strSavedIDsArray);
		for(int i = 0; i < strIDsArray.GetSize(); i++) {
			  BOOL bExist = FALSE; 
			  // Loop through code and check ecah code exist els return true
				for(int nCount = 0; nCount < strSavedIDsArray.GetSize(); nCount++) {
					//long a =  atol(strIDsArray[i]);
					//long b=  atol(strSavedIDsArray[nCount]);
					if (atol(strIDsArray[i]) == atol(strSavedIDsArray[nCount])) {
						bExist =TRUE;
						break;
					}
					else{
					// continue
					}
				}
				if (bExist == FALSE){
					return TRUE;
					break;
				}
				else{
					// continue
				}
		}
	}
	return bResult; 
}	

void CGlassesCatalogBillingSetupDlg::OnDesigns()
{
	try {
		//TES 5/20/2011 - PLID 43698 - Show the Designs list, hide the others

		GetDlgItem(IDC_DESIGN_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_MATERIAL_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TREATMENT_LIST)->ShowWindow(SW_HIDE);
		
	}NxCatchAll(__FUNCTION__);
}

void CGlassesCatalogBillingSetupDlg::OnMaterials()
{
	try {
		//TES 5/20/2011 - PLID 43698 - Show the Materials list, hide the others

		GetDlgItem(IDC_DESIGN_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MATERIAL_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TREATMENT_LIST)->ShowWindow(SW_HIDE);
		
	}NxCatchAll(__FUNCTION__);
}

void CGlassesCatalogBillingSetupDlg::OnTreatments()
{
	try {
		//TES 5/20/2011 - PLID 43698 - Show the Treatments list, hide the others
		GetDlgItem(IDC_DESIGN_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MATERIAL_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TREATMENT_LIST)->ShowWindow(SW_SHOW);
		
	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2012-03-06 09:17) - PLID 48357

void CGlassesCatalogBillingSetupDlg::LeftClickDesignList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	if ( gclcCpt==nCol)
		OnMultiSelectCPT(lpRow,lstDesign);
}

// (s.dhole 2012-03-06 09:17) - PLID 48357
void CGlassesCatalogBillingSetupDlg::LeftClickTreatmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	if ( gclcCpt==nCol)
		OnMultiSelectCPT(lpRow,lstTreatment );
}

// (s.dhole 2012-03-06 09:17) - PLID 48357
void CGlassesCatalogBillingSetupDlg::LeftClickMaterialList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	if ( gclcCpt==nCol)
		OnMultiSelectCPT(lpRow,lstMaterial  );
}


// (s.dhole 2012-03-06 09:12) - PLID 48638 To select/edit multiple Service code , common function
void CGlassesCatalogBillingSetupDlg::OnMultiSelectCPT(LPDISPATCH lpRow,GlassesListType typ)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (pRow== NULL)
			return ;
		CString  strSelectedID =   VarString(pRow->GetValue(gclcCptID) ,"");
		CStringArray strIDArray;
		SplitString(strSelectedID,"," ,&strIDArray);
		CInvSelectMultipleCPTDlg dlg(this);
		for(int i = 0; i < strIDArray.GetSize(); i++) {
			dlg.m_arPreSelectedID.Add(atol( strIDArray[i]));
		}
		
		switch(typ) {
		case lstDesign :
				dlg.m_strType = "Design";  
				dlg.m_strItem= VarString(pRow->GetValue(gclcName ) ,"");
			break;
			case lstTreatment :
				dlg.m_strType = "Treatment";  
				dlg.m_strItem= VarString(pRow->GetValue(gclcName ) ,"");
				break;
			case lstMaterial :
				dlg.m_strType = "Material";
				dlg.m_strItem= VarString(pRow->GetValue(gclcName ) ,"");
				break;
		}
		
		if(IDOK == dlg.DoModal() ) {
			CString  strNewVal = dlg.m_strSelectedCodes;
			if (strNewVal.IsEmpty()){
				strNewVal ="      <None>";
			}else
			{
				// nothing
			}

			// (j.dinatale 2013-03-27 17:26) - PLID 55919 - handle our new prompt column on our lists
			pRow->PutValue( gclcCptID,_variant_t(dlg.m_strSelectedIDs ));
			pRow->PutValue( gclcCpt, _variant_t( strNewVal));
			if(strNewVal.CompareNoCase("      <None>") == 0    ) {
				pRow->PutValue(gclcBillPerLens, g_cvarNull);
				pRow->PutValue(gclcPrompt, g_cvarNull);
			}
			else {//(s.dhole 2012-06-20 14:12)  Should check current changes not saved one 
				_variant_t vVal =pRow->GetValue(gclcBillPerLens); 
				bool bBillPerLens=false;
				if (vVal.vt != VT_NULL &&  vVal.lVal != FALSE ){
					bBillPerLens=true;
				}
				pRow->PutValue(gclcBillPerLens, bBillPerLens);	

				if(typ != lstTreatment){
					vVal = pRow->GetValue(gclcPrompt);
					bool bPrompt = false;
					if (vVal.vt != VT_NULL &&  vVal.lVal != FALSE ){
						bPrompt = true;
					}
					pRow->PutValue(gclcPrompt, bPrompt);	
				}
			}
		}
		else {
			// nothing
		}
	}NxCatchAll(__FUNCTION__);

}
