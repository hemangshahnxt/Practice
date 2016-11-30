// NexERxRegisterPrescriberDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexERxRegisterPrescriberDlg.h"

// (b.savon 2013-06-06 15:51) - PLID 56840 - Created

// CNexERxRegisterPrescriberDlg dialog
enum RegisterPrescriberLocationServiceLevelColumn{
	rplCheck = 0,
	rplID,
	rplLocation,
	rplServiceLevel,
	rplValidationResults,
	rplRegistrationResults,
};

enum RegisterPresciberServiceLevelColumn{
	rpsID = 0,
	rpsText = 1,
};

enum RegisterPrescriberColors{
	rpcRed = RGB(248, 167, 171),
	rpcGreen = RGB(152,251,152),
	rpcBlack = RGB(0, 0, 0),
	prcAlternateRow = RGB(232, 235, 249),
};

IMPLEMENT_DYNAMIC(CNexERxRegisterPrescriberDlg, CNxDialog)

CNexERxRegisterPrescriberDlg::CNexERxRegisterPrescriberDlg(const NexERxPrescriber &prescriber, CWnd* pParent /*=NULL*/)
: CNxDialog(CNexERxRegisterPrescriberDlg::IDD, pParent), m_prescriber(prescriber)
{
	m_nCountSelected = 0;
	m_bRegistered = FALSE;
}

BOOL CNexERxRegisterPrescriberDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		PrepareControls();

		LoadPrescriberInfo();

		return TRUE;
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

CNexERxRegisterPrescriberDlg::~CNexERxRegisterPrescriberDlg()
{
}

void CNexERxRegisterPrescriberDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NXC_REGISTER_PRESCRIBER_TOP, m_nxcTop);
	DDX_Control(pDX, IDC_NXC_REGISTER_PRESCRIBER_BOTTOM, m_nxcBottom);
	DDX_Control(pDX, IDOK, m_btnRegister);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_PRESCRIBER_REGISTRATION_INFO, m_nxstaticInfo);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX_REG, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX_REG, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX_REG, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_WORK_PHONE_BOX_REG, m_nxeditWorkPhoneBox);
	DDX_Control(pDX, IDC_EXT_PHONE_BOX_REG, m_nxeditExtPhoneBox);
	DDX_Control(pDX, IDC_EMAIL_BOX_REG, m_nxeditEmailBox);
	DDX_Control(pDX, IDC_FAX_BOX_REG, m_nxeditFaxBox);
	DDX_Control(pDX, IDC_NPI_BOX_REG, m_nxeditNpiBox);
	DDX_Control(pDX, IDC_LICENSE_BOX_REG, m_nxeditLicenseBox);
	DDX_Control(pDX, IDC_DEA_BOX_REG, m_nxeditDEA);
	DDX_Control(pDX, IDC_CHECK_ALL_RX_LOCATIONS, m_chkSelectAllLocations);
}


BEGIN_MESSAGE_MAP(CNexERxRegisterPrescriberDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNexERxRegisterPrescriberDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_ALL_RX_LOCATIONS, &CNexERxRegisterPrescriberDlg::OnBnClickedCheckAllRxLocations)
END_MESSAGE_MAP()


// CNexERxRegisterPrescriberDlg message handlers
BEGIN_EVENTSINK_MAP(CNexERxRegisterPrescriberDlg, CNxDialog)
	ON_EVENT(CNexERxRegisterPrescriberDlg, IDC_NXDL_REGISTER_PRESCRIBER_LOCATIONS, 10, CNexERxRegisterPrescriberDlg::EditingFinishedNxdlRegisterPrescriberLocations, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexERxRegisterPrescriberDlg, IDC_NXDL_APPLY_SERVICELEVEL, 16, CNexERxRegisterPrescriberDlg::SelChosenNxdlApplyServicelevel, VTS_DISPATCH)
	ON_EVENT(CNexERxRegisterPrescriberDlg, IDC_NXDL_APPLY_SERVICELEVEL, 1, CNexERxRegisterPrescriberDlg::SelChangingNxdlApplyServicelevel, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CNexERxRegisterPrescriberDlg::PrepareControls()
{
	try{
		m_nxstaticInfo.SetWindowTextA(
			"Please verify the prescriber information below.  If it is incorrect, please cancel "
			"this window and correct the values on the previous screen.  If the "
			"prescriber fields are correct, please proceed to select the location(s) and the service "
			"level(s) at each location for this prescriber. \r\n\r\n**Note** Fields with a red background are required "
			"when registering."
		);
		m_nxcTop.SetColor(GetNxColor(GNC_CONTACT, 0));
		m_nxcBottom.SetColor(GetNxColor(GNC_CONTACT, 0));
		m_btnRegister.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxdlLocationServiceLevel = BindNxDataList2Ctrl(IDC_NXDL_REGISTER_PRESCRIBER_LOCATIONS, false);
		m_nxdlServiceLevelDD = BindNxDataList2Ctrl(IDC_NXDL_APPLY_SERVICELEVEL, false);
		LoadDatalist();

		EnableRegistration();
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-06 15:52) - PLID 56867 - Load the data for the datalists
void CNexERxRegisterPrescriberDlg::LoadDatalist()
{
	try{
		/* Registration Datalist */
		long nPrescriberID = atol(m_prescriber.strID);

		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT	CASE WHEN EXISTS(SELECT TOP 1 1 FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID AND RegistrationResult = 1) THEN NULL ELSE 0 END AS CheckBox, \r\n"
			"		LocationsT.ID, \r\n"
			"		Name + CHAR(13) + CHAR(10) + Address1 + (CASE WHEN Address2 <> '' THEN CHAR(13) + CHAR(10) + Address2 ELSE '' END) + CHAR(13) + CHAR(10) + City + ', ' + State + ' ' + Zip AS Location, \r\n"
			"		CASE WHEN EXISTS(SELECT TOP 1 1 FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID) THEN (SELECT ServiceLevel FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID) ELSE 0 END AS ServiceLevel, \r\n"
			"		NULL AS ValidationResults, \r\n"
			"		CASE WHEN EXISTS(SELECT TOP 1 1 FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID) THEN (SELECT RegistrationText FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID) ELSE NULL END AS RegistrationText, \r\n"
			"		CONVERT(BIT, CASE WHEN EXISTS(SELECT TOP 1 1 FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID) THEN (SELECT RegistrationResult FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT} AND LocationID = LocationsT.ID) ELSE 0 END) AS RegistrationResult \r\n"
			"FROM	LocationsT \r\n"
			"WHERE	Managed = 1 AND Active = 1 \r\n"
			"ORDER BY LocationsT.ID \r\n",
			nPrescriberID, nPrescriberID, nPrescriberID, nPrescriberID, nPrescriberID, nPrescriberID, nPrescriberID
		);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlLocationServiceLevel->GetNewRow();
		int row = 0;
		while(!prs->eof){
			if( row++ % 2 == 0 ){
				pRow->PutBackColor(prcAlternateRow);
			}

			pRow->PutValue(rplCheck, prs->Fields->GetItem("CheckBox")->GetValue());
			pRow->PutValue(rplID, AdoFldLong(prs->Fields, "ID"));
			pRow->PutValue(rplLocation, AsBstr(AdoFldString(prs->Fields, "Location")));
			pRow->PutValue(rplServiceLevel, AdoFldLong(prs->Fields, "ServiceLevel"));
			pRow->PutValue(rplValidationResults, prs->Fields->GetItem("ValidationResults")->GetValue());
			CString strRegistrationText = AdoFldString(prs->Fields, "RegistrationText", "");
			pRow->PutValue(rplRegistrationResults, AsBstr(strRegistrationText));

			BOOL bRegistrationResults = AdoFldBool(prs->Fields, "RegistrationResult", FALSE);
			if( bRegistrationResults ){
				pRow->PutCellBackColor(rplRegistrationResults, rpcGreen);
				pRow->PutCellBackColorSel(rplRegistrationResults, rpcGreen);
				pRow->PutCellForeColorSel(rplRegistrationResults, rpcBlack);
				pRow->PutRefCellFormatOverride(rplServiceLevel, GetReadOnlyServiceLevelFormat());
			}else if ( !strRegistrationText.IsEmpty() && !bRegistrationResults ){
				pRow->PutCellBackColor(rplRegistrationResults, rpcRed);
				pRow->PutCellBackColorSel(rplRegistrationResults, rpcRed);
				pRow->PutCellForeColorSel(rplRegistrationResults, rpcBlack);
			}

			m_nxdlLocationServiceLevel->AddRowSorted(pRow, NULL);
			pRow = m_nxdlLocationServiceLevel->GetNewRow();
			prs->MoveNext();
		}

		/*Service Level Datalist*/
		row = 0;
		pRow = m_nxdlServiceLevelDD->GetNewRow();
		while(pRow && row < 4){
			switch( row ){
				case 0:
					{
						pRow->PutValue(rpsID, AsBstr("0"));
						pRow->PutValue(rpsText, AsBstr("Deactivated"));
						++row;
					}
					break;
				case 1:
					{
						pRow->PutValue(rpsID, AsBstr("1"));
						pRow->PutValue(rpsText, AsBstr("NewRx"));
						++row;
					}
					break;
				case 2:
					{
						pRow->PutValue(rpsID, AsBstr("2"));
						pRow->PutValue(rpsText, AsBstr("Refill Requests"));
						++row;
					}
					break;
				case 3:
					{
						pRow->PutValue(rpsID, AsBstr("3"));
						pRow->PutValue(rpsText, AsBstr("NewRx & Refill Requests"));
						++row;
					}
					break;
				default:
					ThrowNxException("Invalid Row!");
			}
			m_nxdlServiceLevelDD->AddRowAtEnd(pRow, NULL);
			pRow = m_nxdlServiceLevelDD->GetNewRow();
		}

		m_nxdlServiceLevelDD->PutCurSel(m_nxdlServiceLevelDD->FindByColumn(rpsID, AsBstr("0"), NULL, VARIANT_TRUE));

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-06 15:51) - PLID 56867 - Load the prescriber info
void CNexERxRegisterPrescriberDlg::LoadPrescriberInfo()
{
	try{
		m_nxeditFirstNameBox.SetWindowTextA(m_prescriber.strFirst);
		m_nxeditMiddleNameBox.SetWindowTextA(m_prescriber.strMiddle);
		m_nxeditLastNameBox.SetWindowTextA(m_prescriber.strLast);
		m_nxeditNpiBox.SetWindowTextA(m_prescriber.strNPI);
		m_nxeditLicenseBox.SetWindowTextA(m_prescriber.strStateLicense);
		// (b.savon 2013-08-02 14:21) - PLID 57747 - DEA
		m_nxeditDEA.SetWindowTextA(m_prescriber.strDEA);
		m_nxeditWorkPhoneBox.SetWindowTextA(m_prescriber.strWork);
		m_nxeditExtPhoneBox.SetWindowTextA(m_prescriber.strExt);
		m_nxeditFaxBox.SetWindowTextA(m_prescriber.strFax);
		m_nxeditEmailBox.SetWindowTextA(m_prescriber.strEmail);

		SetTextBoxColors();
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-06 15:53) - PLID 56867 - Set the textbox colors for the prescriber info
void CNexERxRegisterPrescriberDlg::SetTextBoxColors()
{
	try{
		/* Standard */
		if( m_prescriber.strFirst.IsEmpty() ){
			m_nxeditFirstNameBox.SetBackgroundColorStandard(rpcRed, true, true);
		}
		if( m_prescriber.strLast.IsEmpty() ){
			m_nxeditLastNameBox.SetBackgroundColorStandard(rpcRed, true, true);
		}
		if( m_prescriber.strNPI.IsEmpty() ){
			m_nxeditNpiBox.SetBackgroundColorStandard(rpcRed, true, true);
		}
		if( m_prescriber.strStateLicense.IsEmpty() ){
			m_nxeditLicenseBox.SetBackgroundColorStandard(rpcRed, true, true);
		}
		if( m_prescriber.strWork.IsEmpty() ){
			m_nxeditWorkPhoneBox.SetBackgroundColorStandard(rpcRed, true, true);
		}
		if( m_prescriber.strFax.IsEmpty() ){
			m_nxeditFaxBox.SetBackgroundColorStandard(rpcRed, true, true);
		}
		/* Focus */
		if( m_prescriber.strFirst.IsEmpty() ){
			m_nxeditFirstNameBox.SetBackgroundColorFocus(rpcRed, true, true);
		}
		if( m_prescriber.strLast.IsEmpty() ){
			m_nxeditLastNameBox.SetBackgroundColorFocus(rpcRed, true, true);
		}
		if( m_prescriber.strNPI.IsEmpty() ){
			m_nxeditNpiBox.SetBackgroundColorFocus(rpcRed, true, true);
		}
		if( m_prescriber.strStateLicense.IsEmpty() ){
			m_nxeditLicenseBox.SetBackgroundColorFocus(rpcRed, true, true);
		}
		if( m_prescriber.strWork.IsEmpty() ){
			m_nxeditWorkPhoneBox.SetBackgroundColorFocus(rpcRed, true, true);
		}
		if( m_prescriber.strFax.IsEmpty() ){
			m_nxeditFaxBox.SetBackgroundColorFocus(rpcRed, true, true);
		}
		/* Hovered */
		if( m_prescriber.strFirst.IsEmpty() ){
			m_nxeditFirstNameBox.SetBackgroundColorHovered(rpcRed, true, true);
		}
		if( m_prescriber.strLast.IsEmpty() ){
			m_nxeditLastNameBox.SetBackgroundColorHovered(rpcRed, true, true);
		}
		if( m_prescriber.strNPI.IsEmpty() ){
			m_nxeditNpiBox.SetBackgroundColorHovered(rpcRed, true, true);
		}
		if( m_prescriber.strStateLicense.IsEmpty() ){
			m_nxeditLicenseBox.SetBackgroundColorHovered(rpcRed, true, true);
		}
		if( m_prescriber.strWork.IsEmpty() ){
			m_nxeditWorkPhoneBox.SetBackgroundColorHovered(rpcRed, true, true);
		}
		if( m_prescriber.strFax.IsEmpty() ){
			m_nxeditFaxBox.SetBackgroundColorHovered(rpcRed, true, true);
		}
		/* Hovered Focus */
		if( m_prescriber.strFirst.IsEmpty() ){
			m_nxeditFirstNameBox.SetBackgroundColorHoveredFocus(rpcRed, true, true);
		}
		if( m_prescriber.strLast.IsEmpty() ){
			m_nxeditLastNameBox.SetBackgroundColorHoveredFocus(rpcRed, true, true);
		}
		if( m_prescriber.strNPI.IsEmpty() ){
			m_nxeditNpiBox.SetBackgroundColorHoveredFocus(rpcRed, true, true);
		}
		if( m_prescriber.strStateLicense.IsEmpty() ){
			m_nxeditLicenseBox.SetBackgroundColorHoveredFocus(rpcRed, true, true);
		}
		if( m_prescriber.strWork.IsEmpty() ){
			m_nxeditWorkPhoneBox.SetBackgroundColorHoveredFocus(rpcRed, true, true);
		}
		if( m_prescriber.strFax.IsEmpty() ){
			m_nxeditFaxBox.SetBackgroundColorHoveredFocus(rpcRed, true, true);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-06 15:52) - PLID 56867 - Begin the registration process
void CNexERxRegisterPrescriberDlg::OnBnClickedOk()
{
	try{
		NexTech_Accessor::_ERxPrescriberRegistrationPtr prescriber(__uuidof(NexTech_Accessor::ERxPrescriberRegistration));
		prescriber->PrescriberID = AsBstr(m_prescriber.strID);

		CArray<NexTech_Accessor::_ERxPrescriberRegistrationParamsPtr, NexTech_Accessor::_ERxPrescriberRegistrationParamsPtr> aryParams;

		CString strPrescribersAsDeactive;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlLocationServiceLevel->GetFirstRow();
		while(pRow){

			if( AsBool(pRow->GetValue(rplCheck)) ){
				// (b.savon 2015-03-12 11:27) - PLID 60116 - Only prepare our list of prescribers to register if they have a non-deactivated service level
				if (AsLong(pRow->GetValue(rplServiceLevel)) > 0){
					NexTech_Accessor::_ERxPrescriberRegistrationParamsPtr params(__uuidof(NexTech_Accessor::ERxPrescriberRegistrationParams));
					long locationID = AsLong(pRow->GetValue(rplID));
					CString strLocationID;
					strLocationID.Format("%li", locationID);
					params->PutlocationID(AsBstr(strLocationID));
					params->PutServiceLevel(AsLong(pRow->GetValue(rplServiceLevel)));
					aryParams.Add(params);
				}
				else{ // (b.savon 2015-03-12 11:27) - PLID 60116 - Throw them into a bucket if they have a deactivated service level
					strPrescribersAsDeactive += ((strPrescribersAsDeactive.IsEmpty()) ? "" : "\r\n\r\n") + (CString)(pRow->GetValue(rplLocation));
				}
			}
			pRow = pRow->GetNextRow();
		}

		// (b.savon 2015-03-12 11:27) - PLID 60116 - Don't let NexERx prescribers be registered as Deactivated.
		if (!strPrescribersAsDeactive.IsEmpty()){
			MessageBox(
				FormatString("The prescriber cannot be registered as Deactivated.  Please fix the service level for the following location(s) and try again.\r\n\r\n%s", strPrescribersAsDeactive), 
				"Practice", 
				MB_ICONWARNING
			);
			return;
		}

		Nx::SafeArray<IUnknown *> saryPrescriberParams = Nx::SafeArray<IUnknown *>::From(aryParams);

		prescriber->PutPrescriberParams(saryPrescriberParams);

		CWaitCursor cwait;

		NexTech_Accessor::_ERxPrescriberRegistrationPtr results = GetAPI()->RegisterPrescriber(GetAPISubkey(), GetAPILoginToken(), prescriber);

		HandleRegistrationResults(results);

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-06 15:52) - PLID 56867 - Handle the results from the registration
void CNexERxRegisterPrescriberDlg::HandleRegistrationResults(NexTech_Accessor::_ERxPrescriberRegistrationPtr results)
{
	try{
		Nx::SafeArray<BSTR> saryRegisterResults(results->Validation);
		Nx::SafeArray<IUnknown*> saryPrescriberParams(results->PrescriberParams);

		BOOL bResponses = FALSE;
		Nx::SafeArray<BSTR> sarySureScriptsResponse;
		Nx::SafeArray<LONG> saryRegistrationResults;
		if( results->SureScriptsResponse != NULL && results->RegistrationResults != NULL ){
			sarySureScriptsResponse = Nx::SafeArray<BSTR>(results->SureScriptsResponse);
			saryRegistrationResults = Nx::SafeArray<LONG>(results->RegistrationResults);
			bResponses = TRUE;
		}

		BOOL bShowValidationResults = FALSE;
		for( int i = 0; i < (int)saryRegisterResults.GetCount(); i++ ){
			NexTech_Accessor::_ERxPrescriberRegistrationParamsPtr pParams = saryPrescriberParams.GetAt(i); 
			NexTech_Accessor::ERxPrescriberRegistrationResults pregResults;
			if( bResponses ){
				pregResults = (NexTech_Accessor::ERxPrescriberRegistrationResults)saryRegistrationResults.GetAt(i);
			}
			CString strLocationID = (LPCTSTR)pParams->locationID;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlLocationServiceLevel->FindByColumn(rplID, atoi(strLocationID), NULL, VARIANT_FALSE);
			if( pRow == NULL ){
				ThrowNxException("Invalid LocationID!");
			}

			CString strValidationResults;
			strValidationResults.Format("%s", saryRegisterResults.GetAt(i));
			pRow->PutCellBackColor(rplValidationResults, strValidationResults.IsEmpty() ? rpcGreen : rpcRed);
			pRow->PutCellBackColorSel(rplValidationResults, strValidationResults.IsEmpty() ? rpcGreen : rpcRed);
			pRow->PutCellForeColorSel(rplValidationResults, rpcBlack);
			pRow->PutValue(rplValidationResults, strValidationResults.IsEmpty() ? AsBstr("Valid") : saryRegisterResults.GetAt(i));
			if( !strValidationResults.IsEmpty() ){
				bShowValidationResults = TRUE;
			}

			if( bResponses ){
				if( pregResults == NexTech_Accessor::ERxPrescriberRegistrationResults_Success ){
					pRow->PutCellBackColor(rplRegistrationResults, rpcGreen);
					pRow->PutCellBackColorSel(rplRegistrationResults, rpcGreen);
					pRow->PutCellForeColorSel(rplRegistrationResults, rpcBlack);
					pRow->PutValue(rplCheck, g_cvarNull);
					pRow->PutRefCellFormatOverride(rplServiceLevel, GetReadOnlyServiceLevelFormat());
					--m_nCountSelected;
					m_bRegistered = TRUE;
				}else{
					pRow->PutCellBackColor(rplRegistrationResults, rpcRed);
					pRow->PutCellBackColorSel(rplRegistrationResults, rpcRed);
					pRow->PutCellForeColorSel(rplRegistrationResults, rpcBlack);
				}
				pRow->PutValue(rplRegistrationResults, sarySureScriptsResponse.GetAt(i));
			}
		}

		if( bShowValidationResults ){
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_nxdlLocationServiceLevel->GetColumn(rplValidationResults);
			pCol->PutStoredWidth(50);
			pCol = m_nxdlLocationServiceLevel->GetColumn(rplRegistrationResults);
			pCol->PutStoredWidth(0);
		}else{
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_nxdlLocationServiceLevel->GetColumn(rplValidationResults);
			pCol->PutStoredWidth(0);
			pCol = m_nxdlLocationServiceLevel->GetColumn(rplRegistrationResults);
			pCol->PutStoredWidth(50);
		}
		EnableRegistration();

	}NxCatchAll(__FUNCTION__);
}

void CNexERxRegisterPrescriberDlg::EditingFinishedNxdlRegisterPrescriberLocations(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		if( !bCommit ){
			return;
		}

		switch(nCol){
			case rplCheck:
				{
					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
					if( (pRow->GetValue(rplCheck)).vt != VT_NULL ){
						AsBool(varNewValue) ? ++m_nCountSelected : --m_nCountSelected;
						EnableRegistration();
					}
				}
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxRegisterPrescriberDlg::EnableRegistration()
{
	m_btnRegister.EnableWindow(m_nCountSelected > 0);
}

void CNexERxRegisterPrescriberDlg::OnBnClickedCheckAllRxLocations()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlLocationServiceLevel->GetFirstRow();
		while(pRow){
			if( (pRow->GetValue(rplCheck)).vt != VT_NULL ){
				if( AsBool(pRow->GetValue(rplCheck)) != m_chkSelectAllLocations.GetCheck() ){
					pRow->PutValue(rplCheck, m_chkSelectAllLocations.GetCheck() == BST_CHECKED ? TRUE : FALSE);
					m_chkSelectAllLocations.GetCheck() == BST_CHECKED ? ++m_nCountSelected : --m_nCountSelected;
				}
			}

			pRow = pRow->GetNextRow();
		}
		EnableRegistration();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxRegisterPrescriberDlg::SelChosenNxdlApplyServicelevel(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			NXDATALIST2Lib::IRowSettingsPtr pRegRow = m_nxdlLocationServiceLevel->GetFirstRow();
			while(pRegRow){
				if( (pRegRow->GetValue(rplCheck)).vt != VT_NULL ){
					pRegRow->PutValue(rplServiceLevel, pRow->GetValue(rpsID));
				}
				pRegRow = pRegRow->GetNextRow();
			}			
		}
	}NxCatchAll(__FUNCTION__);
}

void CNexERxRegisterPrescriberDlg::SelChangingNxdlApplyServicelevel(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		if (NULL == *lppNewSel) {
			// Undo NULL selections (unless lpOldSel is NULL too, in that case we don't have any choice)
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		} 	
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-06-09 13:39) - PLID 56867
NXDATALIST2Lib::IFormatSettingsPtr CNexERxRegisterPrescriberDlg::GetReadOnlyServiceLevelFormat()
{
	NXDATALIST2Lib::IFormatSettingsPtr pFormat(__uuidof(NXDATALIST2Lib::FormatSettings));
	pFormat->PutComboSource(AsBstr("0;Deactivated;1;NewRx;2;Renewal Requests;3;NewRx & Renewal Requests"));
	pFormat->PutEditable(VARIANT_FALSE);
	pFormat->PutFieldType(NXDATALIST2Lib::cftComboSimple);
	pFormat->PutDataType(VT_BSTR);
	pFormat->PutConnection(_variant_t((LPDISPATCH)NULL));
	return pFormat;
}