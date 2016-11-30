// AddComplexMedicationNameDlg.cpp : implementation file
//

// (d.thompson 2008-12-01) - PLID 32175 - Created

#include "stdafx.h"
#include "Practice.h"
#include "AddComplexMedicationNameDlg.h"
#include "PatientsRc.h"
#include "NxAPI.h"

#define NO_SEL_TEXT	"<No Selection>"
//Utility functions

// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added and enum for the Route List columns
enum RouteListColumns
{
	rlcID = 0,
	rlcRoute,
};

// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added a Dosage Unit List
enum DosageUnitListColumns
{
	dulcID = 0,
	dulcUnitDescription,
};
// (s.dhole 2013-02-27 11:44) - PLID 55345
CString GetDatalist2lText(NXDATALIST2Lib::_DNxDataListPtr pList, long nID)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->FindByColumn(0,nID, NULL, VARIANT_FALSE) ; 
	if(pRow == NULL) {
		return "";
	}

	return AsString(pRow->GetValue(1));
}


//Safely gets the text of the current selection in the given datalist2.  Returns an empty
//	string if no selection.
CString GetDatalist2CurSelText(NXDATALIST2Lib::_DNxDataListPtr pList, short nCol)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
	if(pRow == NULL) {
		return "";
	}

	return AsString(pRow->GetValue(nCol));
}

//Same as above, but for an ID.  Returns -1 for NULL value or no selection
long GetDatalist2CurSelID(NXDATALIST2Lib::_DNxDataListPtr pList, short nCol)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = pList->CurSel;
	if(pRow == NULL) {
		return -1;
	}

	return VarLong(pRow->GetValue(nCol), -1);
}

// (d.thompson 2008-11-25) - PLID 32175 - Given the 4 elements of a drug, calculates
//	an appropriate drug description.
CString CalculateFullDrugDescription(CString strDrugName, CString strStrength, 
									 CString strUnits, CString strDosageForm)
{
	//This is pretty simple, just slap 'em together.
	CString strResult;
	strResult += strDrugName;
	strResult.TrimRight();
	strResult += " ";

	strResult += strStrength;
	strResult.TrimRight();
	strResult += " ";

	strResult += strUnits;
	strResult.TrimRight();
	strResult += " ";

	strResult += strDosageForm;
	strResult.TrimRight();

	return strResult;
}

// CAddComplexMedicationNameDlg dialog

IMPLEMENT_DYNAMIC(CAddComplexMedicationNameDlg, CNxDialog)

CAddComplexMedicationNameDlg::CAddComplexMedicationNameDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAddComplexMedicationNameDlg::IDD, pParent)
{
	m_nStrengthUnitID = -1;
	m_nDosageFormID = -1;
	m_nDosageUnitID = -1;
	m_nRouteID = -1;
	m_bAutoCalcName = false;
	m_bAllowDrugDescriptionToChange = true;
	m_nMedicationID = -1;
	m_nQuntityUnitID = -1; 	// (s.dhole 2012-11-16 10:22) - PLID 53697 
	m_drugType = NexTech_Accessor::DrugType_NDC;	// (j.fouts 2012-11-27 16:47) - PLID 51889 - Default to NDC
	m_strDosageUnit = "";
	m_strDosageRoute = "";
	m_nFDBID=-1;
}

CAddComplexMedicationNameDlg::~CAddComplexMedicationNameDlg()
{
}

void CAddComplexMedicationNameDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DRUG_NAME, m_editDrugName);
	DDX_Control(pDX, IDC_DRUG_STRENGTH, m_editStrength);
	DDX_Control(pDX, IDC_DRUG_DESCRIPTION, m_editFullDesc);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_OPT1_TEXT, m_nxstaticOptionalText1);
	DDX_Control(pDX, IDC_OPT2_TEXT, m_nxstaticOptionalText2);
	DDX_Control(pDX, IDC_DRUG_PRESCRIPTION_BTN, m_radioSingleMed);
	DDX_Control(pDX, IDC_SUPPLY_PRESCRIPTION_BUTTON, m_radioSupply);
	DDX_Control(pDX, IDC_COMPOUND_PRESCRIPTION_BUTTON, m_radioCompound);
	DDX_Control(pDX, IDC_EDIT_DRUG_BKG, m_bkg);
}


BEGIN_MESSAGE_MAP(CAddComplexMedicationNameDlg, CNxDialog)
	ON_EN_CHANGE(IDC_DRUG_NAME, &CAddComplexMedicationNameDlg::OnChangeDrugName)
	ON_EN_CHANGE(IDC_DRUG_STRENGTH, &CAddComplexMedicationNameDlg::OnChangeDrugStrength)
	ON_EN_CHANGE(IDC_DRUG_DESCRIPTION, &CAddComplexMedicationNameDlg::OnChangeDrugDescription)
	ON_BN_CLICKED(IDC_DRUG_PRESCRIPTION_BTN, &CAddComplexMedicationNameDlg::OnBnClickedDrugPrescriptionBtn)
	ON_BN_CLICKED(IDC_SUPPLY_PRESCRIPTION_BUTTON, &CAddComplexMedicationNameDlg::OnBnClickedSupplyPrescriptionButton)
	ON_BN_CLICKED(IDC_COMPOUND_PRESCRIPTION_BUTTON, &CAddComplexMedicationNameDlg::OnBnClickedCompoundPrescriptionButton)
END_MESSAGE_MAP()


// CAddComplexMedicationNameDlg message handlers
BOOL CAddComplexMedicationNameDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Bind datalists
		// (s.dhole 2012-11-16 10:22) - PLID 53697 Change binding to asynchronous
		m_pStrengthList = BindNxDataList2Ctrl(IDC_DRUG_STRENGTH_UNITS,false);
		m_pDosageList = BindNxDataList2Ctrl(IDC_DRUG_DOSAGE_FORM);
		m_pQuntityList= BindNxDataList2Ctrl(IDC_DRUG_QUNTITY_UNITS,false);
		// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added a Dosage Unit list
		m_pDosageUnitsList = BindNxDataList2Ctrl(IDC_DRUG_DOSAGE_UNITS, true);
		// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route list and a nxcolor
		m_pRouteList = BindNxDataList2Ctrl(IDC_DRUG_ROUTE, true);
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		// (s.dhole 2012-11-16 10:22) - PLID 53697 Load list with filter or old code to support historical data
		m_pQuntityList->WhereClause= _bstr_t(FormatString(" IsQuantityUnit = 1 OR ID = %li ",m_nQuntityUnitID )); 
		m_pStrengthList->WhereClause= _bstr_t(FormatString(" IsStrengthUnit = 1 OR ID = %li ",m_nStrengthUnitID )); 
		m_pQuntityList->Requery();
		m_pStrengthList->Requery();
		// (s.dhole 2012-11-16 10:22) - PLID 53697 Wait till finish loading since it is small amount of data
		m_pQuntityList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pStrengthList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		
		//Manually add "no selection" rows
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pStrengthList->GetNewRow();
		pRow->PutValue(0, g_cvarNull);
		pRow->PutValue(1, _bstr_t(NO_SEL_TEXT));
		m_pStrengthList->AddRowBefore(pRow, m_pStrengthList->GetFirstRow());

		pRow = m_pDosageList->GetNewRow();
		pRow->PutValue(0, g_cvarNull);
		pRow->PutValue(1, _bstr_t(NO_SEL_TEXT));
		m_pDosageList->AddRowBefore(pRow, m_pDosageList->GetFirstRow());
		
		// (s.dhole 2012-11-16 10:22) - PLID 53697 insert no selection
		pRow = m_pQuntityList->GetNewRow();
		pRow->PutValue(0, g_cvarNull);
		pRow->PutValue(1, _bstr_t(NO_SEL_TEXT));
		m_pQuntityList->AddRowBefore(pRow, m_pQuntityList->GetFirstRow());

		// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added Dosage Unit list
		pRow = m_pDosageUnitsList->GetNewRow();
		pRow->PutValue(0, g_cvarNull);
		pRow->PutValue(1, _bstr_t(NO_SEL_TEXT));
		m_pDosageUnitsList->AddRowBefore(pRow, m_pDosageUnitsList->GetFirstRow());

		// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route list
		pRow = m_pRouteList->GetNewRow();
		pRow->PutValue(0, g_cvarNull);
		pRow->PutValue(1, _bstr_t(NO_SEL_TEXT));
		m_pRouteList->AddRowBefore(pRow, m_pRouteList->GetFirstRow());

		//Setup controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//Text limits
		((CEdit*)GetDlgItem(IDC_DRUG_DESCRIPTION))->SetLimitText(255);
		((CEdit*)GetDlgItem(IDC_DRUG_NAME))->SetLimitText(255);
		((CEdit*)GetDlgItem(IDC_DRUG_STRENGTH))->SetLimitText(25);

		//Load the default values that were passed in
		SetDlgItemText(IDC_DRUG_NAME, m_strDrugName);
		// (j.fouts 2012-11-27 16:47) - PLID 51889 - Select the apporpriate radio button
		switch(m_drugType)
		{
		case NexTech_Accessor::DrugType_NDC:
			m_radioSingleMed.SetCheck(TRUE);
			OnBnClickedDrugPrescriptionBtn();
			break;
		case NexTech_Accessor::DrugType_Supply:
			m_radioSupply.SetCheck(TRUE);
			OnBnClickedSupplyPrescriptionButton();
			break;
		case NexTech_Accessor::DrugType_Compound:
			m_radioCompound.SetCheck(TRUE);
			OnBnClickedCompoundPrescriptionButton();
			break;
		}

		

		if(m_nDosageFormID == -1) {
			//default to "no selection"
			m_pDosageList->SetSelByColumn(0, g_cvarNull);
		}
		else {
			m_pDosageList->SetSelByColumn(0, m_nDosageFormID);
		}
		if(m_nQuntityUnitID == -1) {
			//default to "no selection"
			m_pQuntityList->SetSelByColumn(0, g_cvarNull);
		}
		else {
			m_pQuntityList->SetSelByColumn(0, m_nQuntityUnitID);
		}
		
		// (j.fouts 2013-02-01 15:05) - PLID 54985 - Select Dosage Unit
		if(m_nDosageUnitID == -1)
		{
			m_pDosageUnitsList->SetSelByColumn(dulcID, g_cvarNull);
		}
		else
		{
			m_pDosageUnitsList->SetSelByColumn(dulcID, m_nDosageUnitID);
		}

		// (j.fouts 2013-02-01 15:08) - PLID 54528 - Select Dosage Route
		if(m_nRouteID == -1)
		{
			m_pRouteList->SetSelByColumn(rlcID, g_cvarNull);
		}
		else
		{
			m_pRouteList->SetSelByColumn(rlcID, m_nRouteID);
		}

		if(!m_bAllowDrugDescriptionToChange) {
			GetDlgItem(IDC_DRUG_DESCRIPTION)->EnableWindow(FALSE);
			//Shouldn't happen, but just to be safe, ensure the auto calculate is off
			m_bAutoCalcName = false;
			// (j.fouts 2012-11-28 10:22) - PLID 51889 - Disable changing type as well
			m_radioSingleMed.EnableWindow(FALSE);
			m_radioSupply.EnableWindow(FALSE);
			m_radioCompound.EnableWindow(FALSE);
		}

		// (j.gruber 2010-11-02 13:57) - PLID 39048 - if there is an FDBID, disable some fields
		// (s.dhole 2012-11-16 10:22) - PLID 53697 For ndc drug allow to change Dosage unit and stregth unit
		// (s.dhole 2013-02-27 11:22) - PLID 55345 Allow to change Qty unit and strenght Unit
		if (m_nFDBID != -1) {
			// (s.dhole 2013-04-25 13:00) - PLID 55345 Allow strenght Unit
			m_pDosageList->PutEnabled(VARIANT_FALSE);
			GetDlgItem(IDC_DRUG_DESCRIPTION)->EnableWindow(FALSE);
			GetDlgItem(IDC_DRUG_NAME)->EnableWindow(FALSE);
			// (s.dhole 2013-04-25 13:00) - PLID 55345 not edit on strength 
			GetDlgItem(IDC_DRUG_STRENGTH)->EnableWindow(FALSE);
			// (j.fouts 2012-11-27 16:47) - PLID 51889 - Disable changing type as well
			m_radioSingleMed.EnableWindow(FALSE);
			m_radioSupply.EnableWindow(FALSE);
			m_radioCompound.EnableWindow(FALSE);
			// (j.fouts 2013-02-01 15:05) - PLID 54985 - Disallow changes
			m_pDosageUnitsList->PutEnabled(VARIANT_FALSE);
			// (j.fouts 2013-02-01 15:08) - PLID 54528 - Disallow changes
			m_pRouteList->PutEnabled(VARIANT_FALSE);
		}
		



		//And now force an update of the full description, if we're auto calculating (new med), otherwise
		//	use what was passed in
		if(m_bAutoCalcName) {
			UpdateDrugDescription();

			//Default focus should be the drug name in this case
			GetDlgItem(IDC_DRUG_NAME)->SetFocus();
		}
		else { 
			SetDlgItemText(IDC_DRUG_DESCRIPTION, m_strFullDrugDescription);

			//Default focus should be the drug description in this case
			GetDlgItem(IDC_DRUG_DESCRIPTION)->SetFocus();
		}

	} NxCatchAll("Error in OnInitDialog");

	return FALSE;
}

UINT CAddComplexMedicationNameDlg::BeginAddNew()
{
	// (j.gruber 2010-11-02 15:54) - PLID 39048
	m_nFDBID = -1;
	m_bAutoCalcName = true;
	return DoModal();
}
// (s.dhole 2013-04-04 11:20) - PLID 55345 
CString GetNotSelected(CString stVal)
{
	return  (stVal.IsEmpty()?NO_SEL_TEXT:stVal)  ;

}

void CAddComplexMedicationNameDlg::OnOK()
{
	try {
		CString strStrength="";// (s.dhole 2013-02-27 11:45) - PLID 55345
		long nStrengthUnitID = -1 ;
		//sync the controls back to their in / out variables
		GetDlgItemText(IDC_DRUG_NAME, m_strDrugName);

		// (j.fouts 2012-11-27 16:47) - PLID 51889 - Sync according to the checked button
		if(m_radioSingleMed.GetCheck())
		{
			//Use strength and units
			// (s.dhole 2013-02-27 11:45) - PLID 55345
			GetDlgItemText(IDC_DRUG_STRENGTH, strStrength);
			nStrengthUnitID = GetDatalist2CurSelID(m_pStrengthList, 0);
			m_drugType = NexTech_Accessor::DrugType_NDC;
		}
		else
		{
			if(m_radioSupply.GetCheck())
			{
				m_drugType = NexTech_Accessor::DrugType_Supply;
			}
			else if(m_radioCompound.GetCheck())
			{
				m_drugType = NexTech_Accessor::DrugType_Compound;
			}

			//Use Note to pharm and clear strength and units
			m_editStrength.GetWindowText(m_strNotes);
			// (s.dhole 2013-02-27 11:45) - PLID 55345
			strStrength = "";
			nStrengthUnitID = -1;
		}
		GetDlgItemText(IDC_DRUG_DESCRIPTION, m_strFullDrugDescription);

		//We attempt to limit this to 255 characters, but the auto generation will let you go over that limit, so check
		//	again on save.
		if(m_strFullDrugDescription.GetLength() > 255) {
			AfxMessageBox("You may not save a description longer than 255 characters.  Please shorten your description and try saving again.");
			return;
		}
		else if(m_strFullDrugDescription.GetLength() == 0) {
			AfxMessageBox("You may not save a blank description.  Please edit your description and try saving again.");
			return;
		}
		
		// (j.jones 2009-06-12 15:15) - PLID 34613 - disallow saving a description that already exists (if new,
		// m_nMedicationID will be -1 which is fine here, and if existing, we do want to allow changing the
		// case of the same medication description)
		ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT EmrDataT.Data FROM DrugList "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE DrugList.ID <> {INT} AND EmrDataT.Data = {STRING}", m_nMedicationID, m_strFullDrugDescription);
		if(!rs->eof) {
			AfxMessageBox("The description you entered already exists for another medication in the list.  Please change your description and try saving again.");
			return;
		}
		rs->Close();
		
		m_nDosageFormID = GetDatalist2CurSelID(m_pDosageList, 0);
		// (s.dhole 2013-02-27 11:45) - PLID 55345
		long nQuntityUnitID= GetDatalist2CurSelID(m_pQuntityList, 0);	// (s.dhole 2012-11-16 10:22) - PLID 53697 
		// (j.fouts 2013-02-01 15:05) - PLID 54985 - Save the ID
		m_nDosageUnitID = GetDatalist2CurSelID(m_pDosageUnitsList, dulcID);
		// (j.fouts 2013-02-01 15:08) - PLID 54528 - Save the ID
		m_nRouteID = GetDatalist2CurSelID(m_pRouteList, rlcID);

		
		//Do a final calculation of the data and put it in our output variable
		//We cannot have <No Selection> in the name, so look for that
		//	and knock it down to nothing.
		CString strUnits = GetDatalist2CurSelText(m_pStrengthList, 1);
		if(strUnits == NO_SEL_TEXT) {
			strUnits = "";
		}
		CString strDosage = GetDatalist2CurSelText(m_pDosageList, 1);
		if(strDosage == NO_SEL_TEXT) {
			strDosage = "";
		}

		// (j.fouts 2013-02-01 15:06) - PLID 54985 - Save the text
		m_strDosageUnit = GetDatalist2CurSelText(m_pDosageUnitsList, dulcUnitDescription);
		if(m_strDosageUnit == NO_SEL_TEXT) {
			m_strDosageUnit = "";
		}

		// (j.fouts 2013-02-01 15:08) - PLID 54528 - Save the text
		m_strDosageRoute = GetDatalist2CurSelText(m_pRouteList, rlcRoute);
		if(m_strDosageRoute == NO_SEL_TEXT) {
			m_strDosageRoute = "";
		}
		// (s.dhole 2013-02-27 08:42) - PLID 55345 check Quntity unit and streght if medication has FDBid 
		if (m_nFDBID>0)
		{
			NexTech_Accessor::_CheckDrugUnitCodePtr  pResults = GetAPI()->GetDrugUnitCode(GetAPISubkey(), GetAPILoginToken(),m_nFDBID);
			if (pResults)
			{	
				 long nCount = 1;
				 long QuantityUnitID = AsLong(pResults->QuantityUnitID);
				 CString oldValue,newValue;
				 CString strMsg ="";
				 oldValue =GetDatalist2lText(m_pQuntityList,m_nQuntityUnitID);
				 newValue =GetDatalist2CurSelText(m_pQuntityList,1);
				 // if saved value and new value is not same and new value and not matching with FDB value
				 if (m_nQuntityUnitID !=nQuntityUnitID &&  nQuntityUnitID!=QuantityUnitID && oldValue!=newValue )
				 {
					 strMsg += FormatString("Do you want to change Qty Units from [%s] to [%s]?\r\n\t\tSuggested Qty Units is [%s].\r\n\r\n",GetNotSelected(oldValue) ,GetNotSelected(newValue),GetNotSelected(GetDatalist2lText(m_pQuntityList,QuantityUnitID))  );
				 }
				 else if (nQuntityUnitID!=QuantityUnitID){
					 strMsg += FormatString("Please check Qty Units [%s].\r\n\t\tSuggested Qty Units is [%s]\r\n\r\n",GetNotSelected(oldValue) ,GetNotSelected(GetDatalist2lText(m_pQuntityList,QuantityUnitID)));
				 }

				 oldValue =GetDatalist2lText(m_pStrengthList,m_nStrengthUnitID);
				 newValue =GetDatalist2CurSelText(m_pStrengthList,1);
				 // (s.dhole 2013-04-25 12:27) -  Defult FDB StrengthUnit value is always null
				 long StrengthUnitID = -1;
				// if saved value and new value is not same and new valuedid not match with FDB
				if (m_nStrengthUnitID !=nStrengthUnitID &&  nStrengthUnitID!=StrengthUnitID && oldValue!=newValue )
				{
					 strMsg += FormatString("Do you want to change of Strength Units from [%s] to [%s]?\r\n\t\tSuggested Strength Units is [%s]?\r\n\r\n",GetNotSelected(oldValue),GetNotSelected(newValue),NO_SEL_TEXT  );
					 nCount ++;
				} 
				else if (nStrengthUnitID!=StrengthUnitID ){
					 strMsg += FormatString("Please check Strength Units [%s]\r\n\t\tSuggested Strength Units is [%s]\r\n\r\n",GetNotSelected(oldValue),NO_SEL_TEXT );
					 nCount ++;
				}
				if  (strMsg!="")
				{
					CString strMsgDisplay;
					if(g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts) {
						strMsgDisplay = FormatString("Following %s will post an incorect prescription to SureScript.\r\n\r\n%s\r\n\r\n",nCount==1?"field":"fields" ,strMsg);
					}
					else{
						strMsgDisplay = FormatString("Please verify following %s.\r\n\r\n%s\r\n\r\n",nCount==1?"change":"changes" ,strMsg);
					}
					
					if ( IDNO== MsgBox(MB_YESNO, FormatString("%sAre you sure you wish to continue?",strMsgDisplay )  ))
					{
					   return; 
					}
				}
				
			}
		}
		// (s.dhole 2013-02-27 08:42) - PLID 55345 
		m_strStrength = strStrength;
		m_nQuntityUnitID = nQuntityUnitID;
		m_nStrengthUnitID =nStrengthUnitID ;
		

		CDialog::OnOK();
	} NxCatchAll("Error in OnOK");
}

void CAddComplexMedicationNameDlg::OnCancel()
{
	try {
		CDialog::OnCancel();
	} NxCatchAll("Error in OnCancel");
}

BEGIN_EVENTSINK_MAP(CAddComplexMedicationNameDlg, CNxDialog)
	ON_EVENT(CAddComplexMedicationNameDlg, IDC_DRUG_STRENGTH_UNITS, 16, CAddComplexMedicationNameDlg::OnSelChosenUnitsList, VTS_DISPATCH)
	ON_EVENT(CAddComplexMedicationNameDlg, IDC_DRUG_DOSAGE_FORM, 16, CAddComplexMedicationNameDlg::OnSelChosenDosageList, VTS_DISPATCH)
	ON_EVENT(CAddComplexMedicationNameDlg, IDC_DRUG_STRENGTH_UNITS, 1, CAddComplexMedicationNameDlg::OnSelChangingUnitsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAddComplexMedicationNameDlg, IDC_DRUG_DOSAGE_FORM, 1, CAddComplexMedicationNameDlg::OnSelChangingDosageList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAddComplexMedicationNameDlg, IDC_DRUG_QUNTITY_UNITS, 1, CAddComplexMedicationNameDlg::OnSelChangingQuntityUnitsList, VTS_DISPATCH VTS_PDISPATCH)

END_EVENTSINK_MAP()

void CAddComplexMedicationNameDlg::OnSelChosenUnitsList(LPDISPATCH lpRow)
{
	try {
		//We must now update the full description
		UpdateDrugDescription();
	} NxCatchAll("Error in OnSelChosenUnitsList");
}

void CAddComplexMedicationNameDlg::OnSelChosenDosageList(LPDISPATCH lpRow)
{
	try {
		//We must now update the full description
		UpdateDrugDescription();
	} NxCatchAll("Error in OnSelChosenDosageList");
}

void CAddComplexMedicationNameDlg::OnChangeDrugName()
{
	try {
		//We must now update the full description
		UpdateDrugDescription();
	} NxCatchAll("Error in OnChangeDrugName");
}

void CAddComplexMedicationNameDlg::OnChangeDrugStrength()
{
	try {
		//We must now update the full description
		UpdateDrugDescription();
	} NxCatchAll("Error in OnChangeDrugStrength");
}

void CAddComplexMedicationNameDlg::UpdateDrugDescription()
{
	if(m_bAutoCalcName) {
		//We need to update the full description
		CString strDrugName, strStrength;
		GetDlgItemText(IDC_DRUG_NAME, strDrugName);
		GetDlgItemText(IDC_DRUG_STRENGTH, strStrength);

		//We cannot have <No Selection> in the name, so look for that
		//	and knock it down to nothing.
		CString strUnits = GetDatalist2CurSelText(m_pStrengthList, 1);
		if(strUnits == NO_SEL_TEXT) {
			strUnits = "";
		}
		CString strDosage = GetDatalist2CurSelText(m_pDosageList, 1);
		if(strDosage == NO_SEL_TEXT) {
			strDosage = "";
		}

		SetDlgItemText(IDC_DRUG_DESCRIPTION, 
			CalculateFullDrugDescription(strDrugName, strStrength, 
			strUnits, strDosage));

		//The act of setting the description will fire the OnChange... function which
		//	will turn off auto-updating.  Turn it back on.
		m_bAutoCalcName = true;
	}
}

void CAddComplexMedicationNameDlg::OnChangeDrugDescription()
{
	try {
		//Once the name has been manually edited, we do not want to overwrite it anymore
		m_bAutoCalcName = false;
	} NxCatchAll("Error in OnChangeDrugDescription");
}



	// (s.dhole 2012-11-16 10:22) - PLID 53697 
void CAddComplexMedicationNameDlg::OnSelChangingUnitsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
				SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

	// (s.dhole 2012-11-16 10:22) - PLID 53697 
void CAddComplexMedicationNameDlg::OnSelChangingDosageList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
				SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

	// (s.dhole 2012-11-16 10:22) - PLID 53697 
void CAddComplexMedicationNameDlg::OnSelChangingQuntityUnitsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
				SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-27 16:47) - PLID 51889 - Switch to medication controls
void CAddComplexMedicationNameDlg::OnBnClickedDrugPrescriptionBtn()
{
	try
	{
		m_editStrength.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DRUG_STRENGTH_UNITS)->ShowWindow(SW_SHOW);
		m_nxstaticOptionalText1.ShowWindow(SW_SHOW);
		m_nxstaticOptionalText2.ShowWindow(SW_SHOW);
		m_editStrength.EnableWindow(TRUE);
		m_pStrengthList->PutEnabled(VARIANT_TRUE);	
		m_nxstaticOptionalText1.SetWindowText("Strength:");
		m_nxstaticOptionalText2.SetWindowText("Strength Units:");
		m_editStrength.SetLimitText(25);

		SetDlgItemText(IDC_DRUG_STRENGTH, m_strStrength);
		if(m_nStrengthUnitID == -1) {
			//default to "no selection"
			m_pStrengthList->SetSelByColumn(0, g_cvarNull);
		}
		else {
			m_pStrengthList->SetSelByColumn(0, m_nStrengthUnitID);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-27 16:47) - PLID 51889 - Switch to supply controls
void CAddComplexMedicationNameDlg::OnBnClickedSupplyPrescriptionButton()
{
	try
	{
		m_editStrength.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DRUG_STRENGTH_UNITS)->ShowWindow(SW_HIDE);
		m_nxstaticOptionalText1.ShowWindow(SW_SHOW);
		m_nxstaticOptionalText2.ShowWindow(SW_HIDE);
		m_editStrength.EnableWindow(TRUE);
		m_pStrengthList->PutEnabled(VARIANT_FALSE);	
		m_nxstaticOptionalText1.SetWindowText("Supply Description:");
		m_editStrength.SetLimitText(255);

		SetDlgItemText(IDC_DRUG_STRENGTH, m_strNotes);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-27 16:47) - PLID 51889 - Switch to compound controls
void CAddComplexMedicationNameDlg::OnBnClickedCompoundPrescriptionButton()
{
	try
	{
		m_editStrength.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DRUG_STRENGTH_UNITS)->ShowWindow(SW_HIDE);
		m_nxstaticOptionalText1.ShowWindow(SW_SHOW);
		m_nxstaticOptionalText2.ShowWindow(SW_HIDE);
		m_editStrength.EnableWindow(TRUE);
		m_pStrengthList->PutEnabled(VARIANT_FALSE);	
		m_nxstaticOptionalText1.SetWindowText("Compound Description:");
		m_editStrength.SetLimitText(255);

		SetDlgItemText(IDC_DRUG_STRENGTH, m_strNotes);
	}
	NxCatchAll(__FUNCTION__);
}
