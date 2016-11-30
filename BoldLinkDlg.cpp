// BoldLinkDlg.cpp : implementation file
//

// (j.gruber 2010-06-01 10:58) - PLID 38337 - created for

#include "stdafx.h"
#include "Practice.h"
#include "BoldLinkDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "BOLDFieldPickDlg.h"
#include "BoldSoapUtils.h"
#include "FirstDatabankutils.h"
#include "FinancialRc.h"

enum BOLDProviderColumns {
	provColID =0,
	provColName,
};

enum SearchListColumns {
	slcPatientID = 0,
	slcUserDefinedID,
	slcEMNID,
	slcSortOrder,
	//slcBitmap,
	slcSend,
	slcNameSort,		
	slcNameDesc,
	//slcDate,
	//slcDesc,
	slcBoldTypeID,
	slcBoldTypeName,
	slcSentDate,
	slcColor,
	slcMessages,
};

// CBoldLinkDlg dialog

IMPLEMENT_DYNAMIC(CBoldLinkDlg, CNxDialog)

CBoldLinkDlg::CBoldLinkDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBoldLinkDlg::IDD, pParent)
{

}

CBoldLinkDlg::~CBoldLinkDlg()
{
}

void CBoldLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOLD_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_BOLD_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_BOLD_SEARCH, m_btnSearch);
	DDX_Control(pDX, IDC_BOLD_SEND, m_btnSend);
	DDX_Control(pDX, IDC_BOLD_VALIDATE, m_btnValidate);
	DDX_Control(pDX, IDC_LABEL_BOLD_MULTI_PROV, m_nxlMultiProviders);
	DDX_Control(pDX, IDC_BOLD_USERNAME, m_nxeditUsername);
	DDX_Control(pDX, IDC_BOLD_PASSWORD, m_nxeditPassword);	
	DDX_Control(pDX, IDC_BOLD_RD_SENT, m_rdBoldSent);
	DDX_Control(pDX, IDC_BOLD_RD_NOT_SENT, m_rdBoldNotSent);
	DDX_Control(pDX, IDC_BOLD_RD_ALL, m_rdBoldAll);
	DDX_Control(pDX, IDC_BOLD_FILTER_DATES, m_chkFilterDates);
}


BEGIN_MESSAGE_MAP(CBoldLinkDlg, CNxDialog)	
	ON_BN_CLICKED(IDC_BOLD_SEARCH, &CBoldLinkDlg::OnBnClickedBoldSearch)
	ON_BN_CLICKED(IDC_BOLD_SEND, &CBoldLinkDlg::OnBnClickedBoldSend)
	ON_BN_CLICKED(IDC_BOLD_RD_SENT, &CBoldLinkDlg::OnBnClickedBoldRdSent)
	ON_BN_CLICKED(IDC_BOLD_RD_NOT_SENT, &CBoldLinkDlg::OnBnClickedBoldRdNotSent)
	ON_BN_CLICKED(IDC_BOLD_RD_ALL, &CBoldLinkDlg::OnBnClickedBoldRdAll)
	ON_BN_CLICKED(IDC_BOLD_FILTER_DATES, &CBoldLinkDlg::OnBnClickedBoldFilterDates)
END_MESSAGE_MAP()


// CBoldLinkDlg message handlers
 // (j.gruber 2010-06-01 10:58) - PLID 37337
BOOL CBoldLinkDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		//Set the icons on the NxIconButtons
		m_btnSend.AutoSet(NXB_EXPORT);
		m_btnSearch.AutoSet(NXB_INSPECT);		
		m_btnValidate.AutoSet(NXB_INSPECT);

	/*	m_nxlMultiProviders.SetText("");
		m_nxlMultiProviders.SetType(dtsHyperlink);
	*/	

		//uncheck the filter dates and set all for the sent filter
		m_chkFilterDates.SetCheck(0);
		m_dtFrom.EnableWindow(FALSE);
		m_dtTo.EnableWindow(FALSE);

		m_rdBoldAll.SetCheck(1);
		m_rdBoldSent.SetCheck(0);
		m_rdBoldNotSent.SetCheck(0);

		//Preferences
		g_propManager.CachePropertiesInBulk("BOLDLink", propText,
			"(Username = '<None>' OR Username = '%s') AND ("			
			" Name = 'BOLDUsername' "						
			")",
			_Q(GetCurrentUserName()));

		//Set up the datalists
		m_pSearchList = BindNxDataList2Ctrl(IDC_BOLD_SEND_LIST, false);
		m_pSearchList->AllowSort = FALSE;
	/*	m_pProviderFilter = BindNxDataList2Ctrl(IDC_BOLD_PROVIDER_LIST, false);
		RequeryProviderList();	
*/

/*		m_nxeditUsername.SetLimitText(255);		//Max length of ConfigRT.TextParam
		SetDlgItemText(IDC_BOLD_USERNAME, GetRemotePropertyText("BOLDUsername", "", 0, "<None>", true));		
*/
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.gruber 2010-06-01 10:58) - PLID 37337
void CBoldLinkDlg::OnBnClickedBoldSearch()
{
	try {
		LoadList();		
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-01 10:58) - PLID 37337
void CBoldLinkDlg::LoadList() {

	
	//first, clear the current list
	m_pSearchList->Clear();

	if (!EnsureValidSearchDateRange()) {
		return;
	}

	CString strWhere = " WHERE (1=1) ";
	if (IsDlgButtonChecked(IDC_BOLD_FILTER_DATES)) {
		strWhere.Format(" WHERE (1=1) AND Q.Date >= '%s' AND Q.Date < DateAdd(day, 1, '%s')", 
			FormatDateTimeForSql((COleDateTime(m_dtFrom.GetValue())), dtoDate), 
			FormatDateTimeForSql((COleDateTime(m_dtTo.GetValue())), dtoDate));
	}

	if (m_rdBoldSent.GetCheck()) {
		strWhere += " AND SentDate IS NOT NULL ";
	}
	else if (m_rdBoldNotSent.GetCheck()) {
		strWhere += " AND SentDate IS NULL ";
	}


	//first get all the patients that
	ADODB::_RecordsetPtr rsList = CreateRecordset("SELECT PatientID, UserDefinedID, ID, Date, Description, PatientName, BoldTypeID, BoldTypeName, PatientVisitDate, SentDate, SortOrder FROM (  "
		 " SELECT PatientID, UserDefinedID, EMRMasterT.ID, Date, Description, PersonT.Last + ', ' + PersonT.First as PatientName,   "
		 " BoldTypesT.ID as BoldTypeID, BoldTypesT.Name as BoldTypeName, BoldTypesT.SortOrder,  "
		 " (SELECT TOP 1 SentDate FROM BoldSentHistoryT WHERE PatientID = EMRMasterT.PatientID AND BoldTypeID = 1 ORDER BY SentDate DESC) as PatientVisitDate,  "
		 " (SELECT TOP 1 SentDate FROM BoldSentHistoryT WHERE PatientID = EMRMasterT.PatientID AND EMNID = EMRMasterT.ID ORDER BY SentDate DESC) as SentDate  "
		 " FROM EMRMasterT   "
		 " LEFT JOIN PatientsT ON EMRMasterT.PatientID = PatientsT.PersonID  "
		 " LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID 		  "
		 " LEFT JOIN EMRCollectionT ON EMRMasterT.EMRCollectionID = EMRCollectionT.ID  "
		 " LEFT JOIN BoldTypesT ON EMRCollectionT.BoldTypeID = BoldTypesT.ID  "
		 " WHERE (EMRCollectionT.BOLDTypeID IS NOT NULL) AND EMRMasterT.Deleted = 0  "
		/* " UNION   "
		 " SELECT PatientID, PatientsT.UserDefinedID, EMRMasterT.ID, EMRMasterT.Date, Description, PersonT.Last + ', ' + PersonT.First as PatientName,   "
		 " BoldTypesT.ID as BoldTypeID, BoldTypesT.Name as BoldTypeName, BoldTypesT.SortOrder,  "
		 " (SELECT SentDate FROM BoldSentHistoryT WHERE PatientID = EMRMasterT.PatientID AND BoldTypeID = 1) as PatientVisitDate,  "
		 " (SELECT SentDate FROM BoldSentHistoryT WHERE PatientID = EMRMasterT.PatientID AND BoldTypeID = BoldTypesT.ID) as SentDate  "
		 " FROM EMRMasterT   "
		 " LEFT JOIN PatientsT ON EMRMasterT.PatientID = PatientsT.PersonID  "
		 " LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID  "
		 " LEFT JOIN EMRTemplateT ON EMRMasterT.TemplateID = EMRTemplateT.ID  "
		 " LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID  "
		 " LEFT JOIN BoldTypesT ON EMRCollectionT.BOLDTypeID = BoldTypesT.ID  "
		 " WHERE EMRCollectionT.BOLDTypeID IS NOT NULL AND EMRMasterT.Deleted = 0  "*/
		 " ) Q %s ORDER BY PatientID, SortOrder ", strWhere); 
	
	long nOldPatientID = -1;
	long nCurrentPatientID = -1;

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	NXDATALIST2Lib::IRowSettingsPtr pParentRow = NULL;

	if (rsList->eof) {
		MessageBox("No Results to Display");
	}
	
	while (!rsList->eof) {

		nCurrentPatientID = AdoFldLong(rsList, "PatientID");

		if (nOldPatientID != nCurrentPatientID) {

			//we have to have a patient visit EMN
			long nBoldID = AdoFldLong(rsList, "BoldTypeID");
			long nUserDefinedID = AdoFldLong(rsList, "UserDefinedID");
			CString strPatientName = AdoFldString(rsList, "PatientName");						
			
			if (nBoldID == bvtPatient) {			
				COleDateTime dtSentDate = AdoFldDateTime(rsList, "PatientVisitDate", dtInvalid);
				long nEMNID = AdoFldLong(rsList, "ID");
				long nSortOrder = AdoFldLong(rsList, "SortOrder");
				COleDateTime dtEMN = AdoFldDateTime(rsList, "Date", dtInvalid);
				pParentRow = AddPatientRow(nCurrentPatientID, nEMNID, nUserDefinedID, strPatientName, dtSentDate, nSortOrder, dtEMN);				
			}
			else {
				//this means that they have no patient visit or their patient visit is not included in the filter
				//we need to add the parent row and have it not have a send box to send the message
				pParentRow = AddPatientRow(nCurrentPatientID, -1, nUserDefinedID, strPatientName, dtInvalid, -1, dtInvalid);
			}
			nOldPatientID = nCurrentPatientID;
		}

		if (pParentRow) {
			long nEMNID = AdoFldLong(rsList, "ID");
			long nSortOrder = AdoFldLong(rsList, "SortOrder");
			CString strDescription = AdoFldString(rsList, "Description");
			COleDateTime dtLastSent = AdoFldDateTime(rsList, "SentDate", dtInvalid);
			COleDateTime dtEMN = AdoFldDateTime(rsList, "Date", dtInvalid);
			CString strBoldTypeName = AdoFldString(rsList, "BoldTypeName", "");
			long nBoldID = AdoFldLong(rsList, "BoldTypeID");

			//don't show prev bariatric or insurance visits in the list since those are included in patient
			//also adverse events before discharge
			if (nBoldID != 1 && nBoldID != 2 && nBoldID != 3 && nBoldID != 6) {

				AddStepRow(pParentRow, nEMNID, dtEMN, strDescription, nBoldID, strBoldTypeName, dtLastSent, nSortOrder);
			}
		}

		rsList->MoveNext();
	}

	//now we have to go back through and take out any patient rows that can't be sent and have no children
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetFirstRow();
	while (pRow) {
		BOOL bRemove = FALSE;

		//see if the send field is null
		_variant_t varSend = pRow->GetValue(slcSend);
		if (varSend.vt == VT_NULL) {
			//see if this row has any children
			if (pRow->GetFirstChildRow() == NULL) {
				//we can remove the row
				bRemove = TRUE;
			}
		}
		NXDATALIST2Lib::IRowSettingsPtr pTempRow = pRow;
		pRow = pRow->GetNextRow();
		if (bRemove) {
			m_pSearchList->RemoveRow(pTempRow);
		}
	}
}

// (j.gruber 2010-06-01 10:58) - PLID 37337
NXDATALIST2Lib::IRowSettingsPtr CBoldLinkDlg::AddPatientRow(long nPatientID, long nEMNID, long nUserDefinedID, CString strPatientName, COleDateTime dtSentDate, long nSortOrder, COleDateTime dtEMN)
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetNewRow();
	if (pRow) {
		pRow->PutValue(slcPatientID, (long)nPatientID);
		pRow->PutValue(slcUserDefinedID, (long)nUserDefinedID);
		pRow->PutValue(slcEMNID, (long)nEMNID);
		//pRow->PutValue(slcBitmap, _variant_t("BITMAP:UPARROW"));
		pRow->PutValue(slcNameSort, _variant_t(strPatientName));
		CString strUserDefinedID = AsString(nUserDefinedID);
		CString strCell;
		CString strDate;
		if (dtEMN.GetStatus() != COleDateTime::invalid) {
			strDate = " - " + FormatDateTimeForInterface(dtEMN);
		}
		strCell.Format("%s%s - %s", strUserDefinedID, strDate, strPatientName);
		pRow->PutValue(slcNameDesc, _variant_t(strCell));		
		pRow->PutValue(slcBoldTypeID, (long)bvtPatient);
		pRow->PutValue(slcSortOrder, (long)nSortOrder);
		pRow->PutValue(slcBoldTypeName, _variant_t("Patient"));
		if (dtSentDate.GetStatus() != COleDateTime::invalid) {
			pRow->PutValue(slcSentDate, _variant_t(FormatDateTimeForInterface(dtSentDate)));			
		}
		if (nEMNID == -1) {
			//put a null value in, so they can't send it
			pRow->PutValue(slcSend, g_cvarNull);
		}
		else {
			pRow->PutValue(slcSend, g_cvarFalse);
		}
		

		m_pSearchList->AddRowAtEnd(pRow, NULL);
		return pRow;	
				
		
	}

	return NULL;
}

// (j.gruber 2010-06-01 10:58) - PLID 37337
void CBoldLinkDlg::AddStepRow(NXDATALIST2Lib::IRowSettingsPtr pParentRow, long nEMNID, COleDateTime dtEMN, CString strEMNDescription, long nBoldTypeID, CString strBoldTypeName, COleDateTime dtLastSent, long nSortOrder)
{

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetNewRow();
	if (pParentRow && pRow) {
		pRow->PutValue(slcPatientID, pParentRow->GetValue(slcPatientID));
		pRow->PutValue(slcUserDefinedID, pParentRow->GetValue(slcUserDefinedID));
		pRow->PutValue(slcEMNID, (long)nEMNID);
		//pRow->PutValue(slcBitmap, _variant_t("BITMAP:DETAILLINE"));
		//pRow->PutValue(slcDate, _variant_t(FormatDateTimeForInterface(dtEMN)));
		CString strCell;
		strCell.Format("%s - %s", FormatDateTimeForInterface(dtEMN), strEMNDescription);
		pRow->PutValue(slcNameDesc, _variant_t(strCell));
		pRow->PutValue(slcBoldTypeID, (long)nBoldTypeID);
		pRow->PutValue(slcBoldTypeName, _variant_t(strBoldTypeName));
		pRow->PutValue(slcSortOrder, (long)nSortOrder);
		if (dtLastSent.GetStatus() != COleDateTime::invalid) {
			pRow->PutValue(slcSentDate, _variant_t(FormatDateTimeForInterface(dtLastSent)));
			//pRow->PutValue(slcSend, g_cvarTrue);
		}
		else {
			//pRow->PutValue(slcSend, g_cvarFalse);
		}
		pRow->PutValue(slcSend, g_cvarFalse);

		m_pSearchList->AddRowAtEnd(pRow, pParentRow);
	}

}

/*
void CBoldLinkDlg::RequeryProviderList()
{
	CString strProvFrom;
	
	strProvFrom = "(SELECT Last + ', ' + First + ' ' + Middle AS Name, "
		"ProvidersT.PersonID as ID "
		"FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
		") SubQ ";
	
	m_pProviderFilter->FromClause = _bstr_t(strProvFrom);
	m_pProviderFilter->Requery();
}*/


// (j.gruber 2010-06-01 11:51) - PLID 38211
void CBoldLinkDlg::GetValuesFromArray(CArray<BOLDItem*, BOLDItem*> *aryItems, CStringArray *strAry, long nItemID) {

	for (int i = 0; i < aryItems->GetSize(); i++) {

		long nItemIDArray = aryItems->GetAt(i)->nItemID;

		if (nItemIDArray == nItemID) {

			for (int j = 0; j < aryItems->GetAt(i)->paryDetails->GetSize(); j++) {
				if (aryItems->GetAt(i)->paryDetails->GetAt(j)->strSelectCode.IsEmpty()) {
					strAry->Add(aryItems->GetAt(i)->paryDetails->GetAt(j)->strSelection);
				}
				else {
					strAry->Add(aryItems->GetAt(i)->paryDetails->GetAt(j)->strSelectCode);
				}
			}
		}
	}
}

// (j.gruber 2010-06-01 11:01) - PLID 38211
BOLDCodeInfo *CBoldLinkDlg::GetBOLDInfo(CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo) 
{
	//loop through the array and return the info that matches the code
	for (int i = 0; i < aryInfo->GetSize(); i++) {

		BOLDCodeInfo *pInfo = aryInfo->GetAt(i);

		if (pInfo) {			

			if (strCode == pInfo->strCode) {
				return pInfo;
			}
		}
	}
	return NULL;
}

// (j.gruber 2010-06-01 11:02) - PLID 38211
void CBoldLinkDlg::BuildStringArray(CStringArray *strary, CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) {

	BOLDCodeInfo *pInfo = GetBOLDInfo(strCode, aryInfo);

	if (pInfo) { 
		BOOL bAllowMultiples = pInfo->bAllowMultiples;	
		CArray<BOLDItem*, BOLDItem*> *aryItems = pInfo->aryItems;

		if (mapValuesFromDialog) {

			//see if our value is in the map
			CString strValue;
			if (mapValuesFromDialog->Lookup(strCode, strValue)) {

				if (bAllowMultiples) {
					//load the array
					long nItemID = atoi(strValue);
					GetValuesFromArray(aryItems, strary, nItemID);
				}
				else {
					//this shouldn't happen because if we couldn't allow multiples we shouldn't be filling an array
					ASSERT(FALSE);
					//strary->Add(strValue);
				}
			}
			else {

				if (aryItems->GetSize() == 1) {

					if (bAllowMultiples) {

						//we have multiple items for our one item
						long nItemID = aryItems->GetAt(0)->nItemID;

						GetValuesFromArray(aryItems, strary, nItemID);										
					}
					else {
						//this shouldn't happen because if we couldn't allow multiples we shouldn't be filling an array
						ASSERT(FALSE);

						/*if (aryItems->GetAt(0)->paryDetails->GetSize() == 1) {

							//we only have 1 item, 1 value 
							if (aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode.IsEmpty()) {
								strary->Add(aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelection);
							}
							else {
								strary->Add(aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode);
							}								
						}*/
					}
				}
			}
		}
		else {
			if (aryItems->GetSize() == 1) {

				if (bAllowMultiples) {

					//we have multiple items for our one item
					long nItemID = aryItems->GetAt(0)->nItemID;

					GetValuesFromArray(aryItems, strary, nItemID);										
				}
				else {
					//this shouldn't happen because if we couldn't allow multiples we shouldn't be filling an array
					ASSERT(FALSE);

					/*if (aryItems->GetAt(0)->paryDetails->GetSize() == 1) {

						//we only have 1 item, 1 value 
						if (aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode.IsEmpty()) {
							strary->Add(aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelection);
						}
						else {
							strary->Add(aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode);
						}								
					}*/
				}
			}
		}
	}
	else {
		TRACE (strCode + "\r\n");
		//ASSERT(FALSE);
	}
}

// (j.gruber 2010-06-01 11:02) - PLID 38211
void CBoldLinkDlg::GetSelectIDValuesFromArray(CArray<BOLDItem*, BOLDItem*> *aryItems, CStringArray *strAry, long nItemID) {

	for (int i = 0; i < aryItems->GetSize(); i++) {

		long nItemIDArray = aryItems->GetAt(i)->nItemID;

		if (nItemIDArray == nItemID) {

			for (int j = 0; j < aryItems->GetAt(i)->paryDetails->GetSize(); j++) {
				strAry->Add(AsString(aryItems->GetAt(i)->paryDetails->GetAt(j)->nSelectID));				
			}
		}
	}
}


//this is similiar to the standard, but we need to get the selectIDs which we typically wouldn't want
// (j.gruber 2010-06-01 11:02) - PLID 38211
void CBoldLinkDlg::BuildSelectIDArray(CStringArray *strary, CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) {

	BOLDCodeInfo *pInfo = GetBOLDInfo(strCode, aryInfo);

	if (pInfo) { 
		BOOL bAllowMultiples = pInfo->bAllowMultiples;	
		CArray<BOLDItem*, BOLDItem*> *aryItems = pInfo->aryItems;

		if (mapValuesFromDialog) {

			//see if our value is in the map
			CString strValue;
			if (mapValuesFromDialog->Lookup(strCode, strValue)) {

				if (bAllowMultiples) {
					//load the array
					long nItemID = atoi(strValue);
					GetSelectIDValuesFromArray(aryItems, strary, nItemID);
				}
				else {
					//this shouldn't happen because if we couldn't allow multiples we shouldn't be filling an array
					ASSERT(FALSE);
					//strary->Add(strValue);
				}
			}
			else {

				if (aryItems->GetSize() == 1) {

					if (bAllowMultiples) {

						//we have multiple items for our one item
						long nItemID = aryItems->GetAt(0)->nItemID;

						GetSelectIDValuesFromArray(aryItems, strary, nItemID);										
					}
					else {
						//this shouldn't happen because if we couldn't allow multiples we shouldn't be filling an array
						ASSERT(FALSE);

						/*if (aryItems->GetAt(0)->paryDetails->GetSize() == 1) {

							//we only have 1 item, 1 value 
							strary->Add(AsString(aryItems->GetAt(0)->paryDetails->GetAt(0)->nSelectID));
						}*/
					}
				}
			}
		}
		else {
			if (aryItems->GetSize() == 1) {

				if (bAllowMultiples) {

					//we have multiple items for our one item
					long nItemID = aryItems->GetAt(0)->nItemID;

					GetSelectIDValuesFromArray(aryItems, strary, nItemID);										
				}
				else {
					//this shouldn't happen because if we couldn't allow multiples we shouldn't be filling an array
					ASSERT(FALSE);

					/*if (aryItems->GetAt(0)->paryDetails->GetSize() == 1) {

						//we only have 1 item, 1 value 
						if (aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode.IsEmpty()) {
							strary->Add(aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelection);
						}
						else {
							strary->Add(aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode);
						}								
					}*/
				}
			}
		}
	}
	else {
		TRACE (strCode + "\r\n");
		//ASSERT(FALSE);
	}
}


// (j.gruber 2010-06-01 11:02) - PLID 38211
CString CBoldLinkDlg::GetBOLDField(CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) {


	BOLDCodeInfo *pInfo = GetBOLDInfo(strCode, aryInfo);

	if (pInfo) {
		BOOL bAllowMultiples = pInfo->bAllowMultiples;
		ASSERT(!bAllowMultiples);
		CArray<BOLDItem*, BOLDItem*> *aryItems = pInfo->aryItems;

		if (mapValuesFromDialog) {

			//see if our value is in the map
			CString strValue;
			if (mapValuesFromDialog->Lookup(strCode, strValue)) {

				//there is a value
				return strValue;
			}
			else {

				if (aryItems->GetSize() == 1) {

					//get the value from the array
					if (aryItems->GetAt(0)->paryDetails->GetSize() == 1) {

						//we only have 1 item, 1 value 
						if (!aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode.IsEmpty()) {
							return aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode;
						}
						else {
							return aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelection;
						}
					}
				}
			}
		}
		else {
			if (aryItems->GetSize() == 1) {
				//get the value from the array
				if (aryItems->GetAt(0)->paryDetails->GetSize() == 1) {

					//we only have 1 item, 1 value 
					if (!aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode.IsEmpty()) {
						return aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelectCode;
					}
					else {
						return aryItems->GetAt(0)->paryDetails->GetAt(0)->strSelection;
					}
				}
			}
		}
	}
	else {
		TRACE (strCode + "\r\n");
		//ASSERT(FALSE);
	}
	return "";
}

// (j.gruber 2010-06-01 11:02) - PLID 38949
CString CBoldLinkDlg::GetNameFromCode(CString strCode) {

	if (strCode == "PAA1058") { return "Gastric band, adjustable";}
	else if (strCode == "PAA1059") { return "Biliopancreatic diversion (BPD)";}
	else if (strCode == "PAA1060") { return "BPD with duodenal switch";}
	else if (strCode == "PAA1061") { return "Gastrectomy";}
	else if (strCode == "PAA1062") { return "Gastric bypass (Roux-en-Y), laparoscopic";}
	else if (strCode == "PAA1063") { return "Gastric bypass (Roux-en-Y), open";}
	else if (strCode == "PAA1064") { return "Gastric bypass, banded";}
	else if (strCode == "PAA1065") { return "Gastric pacing";}
	else if (strCode == "PAA1066") { return "Intestinal bypass";}
	else if (strCode == "PAA1067") { return "Gastric bypass, mini loop";}
	else if (strCode == "PAA1068") { return "Gastric band, non-adjustable";}
	else if (strCode == "PAA1069") { return "Sleeve gastrectomy";}
	else if (strCode == "PAA1070") { return "Vertical banded gastroplasty";}
	else if (strCode == "PAA1071") { return "Other";}
	else if (strCode == "PAA1072") { return "Gastric bypass (Roux-en-Y) with distal Gastrectomy, laparoscopic";}
	else if (strCode == "PAA1073") { return "Gastric bypass (Roux-en-Y) with distal Gastrectomy, open";}
	else return "";
}

// (j.gruber 2010-06-01 11:03) - PLID 38949
void CBoldLinkDlg::FillPrevBariatricSurgeries(CPtrArray *pPrevSurgs, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog){

	//we already have our array setup, just not filled so we just need to fill it here
	for (int i = 0; i < pPrevSurgs->GetSize(); i++) {

		BOLDPrevBarSurg *pSurg = ((BOLDPrevBarSurg*)pPrevSurgs->GetAt(i));
		//pop it off the array
		pPrevSurgs->RemoveAt(i);

		long nEMNID = pSurg->nInternalID;
		CString strEMNID = AsString(nEMNID);

		CString strSurgCode = GetBOLDField("BOLD_SurgName_"+strEMNID, aryInfo, mapValuesFromDialog);
		pSurg->strCode = strSurgCode;
		pSurg->strName = GetNameFromCode(strSurgCode);
		pSurg->bmuOriginalWt.dblValue = _DBL(GetBOLDField("BOLD_OrigWt_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->bmuOriginalWt.bEstimated = _BOOL(GetBOLDField("BOLD_OrigWtEst_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->bmuOriginalWt.bmuType = _BMU(GetBOLDField("BOLD_OrigWtType_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->bmuLowestWt.dblValue = _DBL(GetBOLDField("BOLD_LowestWt_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->bmuLowestWt.bEstimated = _BOOL(GetBOLDField("BOLD_LowestWtEst_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->bmuLowestWt.bmuType = _BMU(GetBOLDField("BOLD_LowestWtType_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->nYear = _LONG(GetBOLDField("BOLD_SurgYear_"+strEMNID, aryInfo, mapValuesFromDialog));
		pSurg->strSurgeonID = GetBOLDField("BOLD_Surgeon_"+strEMNID, aryInfo, mapValuesFromDialog);
		BuildStringArray(&pSurg->straryAdverseEventCodes, "BOLD_AdverseEvent_"+strEMNID, aryInfo, mapValuesFromDialog);		

		//add it back to the array
		pPrevSurgs->InsertAt(i,pSurg);

	}

}

// (j.gruber 2010-06-01 11:03) - PLID 38211
BOLDBOOL CBoldLinkDlg::_BOOL(CString str) {
	if (str == "1" || str.MakeLower() == "true") {
		return bTrue;
	}
	else if (str == "0" || str.MakeLower() == "false") {
		return bFalse;
	}
	else {
		return bNone;
	}
}

// (j.gruber 2010-06-01 11:03) - PLID 38211
CString CBoldLinkDlg::GetBOLDGenderCode(long nGender) {

	if (nGender == 1) {
		return "GAA10001";
	}
	else if (nGender == 2) {
		return "GAA10002";
	}
	else {
		return "GAA10003";
	}
}

// (j.gruber 2010-06-01 11:03) - PLID 38211
BOLDMetricUnitType CBoldLinkDlg::_BMU(CString str) {

	if (str == "0" || str.MakeLower() == "standard") {
		return bmutStandard;
	}
	else if (str == "1" || str.MakeLower() == "metric") {
		return bmutMetric;
	}
	else {
		return bmutNone;
	}
}

// (j.gruber 2010-06-01 11:03) - PLID 38211
BOLDTimeUnit CBoldLinkDlg::_BTU(CString str) {

	if (str == "0" || str.MakeLower() == "hours") {
		return btuHours;
	}
	else if (str == "1" || str.MakeLower() == "days") {
		return btuDays;
	}
	else {
		return btuNone;
	}
}

// (j.gruber 2010-06-01 11:03) - PLID 38211
double CBoldLinkDlg::_DBL(CString str) {
	//check to see if it is a valid double
	if (IsValidDouble(str)) {
		return AsDouble(_variant_t(str));
	}
	else {
		return 0;
	}
}

// (j.gruber 2010-06-01 11:03) - PLID 38211
long CBoldLinkDlg::_LONG(CString str) {	
	return AsLong(_variant_t(str));
}

// (j.gruber 2010-06-01 11:03) - PLID 38211
COleDateTime CBoldLinkDlg::_DATE(CString str) {
	return AsDateTime(_variant_t(str));	
}

// (j.gruber 2010-06-01 11:03) - PLID 38949
void CBoldLinkDlg::FillPatientVisit(BOLDPatientVisitInfo *pPatVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) 
{
	
	//consent
	pPatVisit->bConsentReceived = _BOOL(GetBOLDField("BOLD_ConsentSRC", aryInfo, mapValuesFromDialog));
	pPatVisit->strEmploymentCode = (GetBOLDField("BOLD_Employment", aryInfo, mapValuesFromDialog));
	

	BuildStringArray(&pPatVisit->straryRaceCodes, "BOLD_Race", aryInfo, mapValuesFromDialog);
	
	//previous non bariatric surgeries
	BuildStringArray(&pPatVisit->straryPrevNonBarCodes, "BOLD_PrevNonBarSurg", aryInfo, mapValuesFromDialog);

	BOLDInsurance *pIns = pPatVisit->pboldPatIns;
	if (pIns == NULL) {
		pIns = new BOLDInsurance();
	}
	pPatVisit->pboldPatIns->bPreCertMentalHealth = _BOOL(GetBOLDField("BOLD_PreCertMentHealth", aryInfo, mapValuesFromDialog));
	BuildStringArray(&pPatVisit->pboldPatIns->straryPaymentCodes, "BOLD_PayCodes", aryInfo, mapValuesFromDialog);
	pPatVisit->pboldPatIns->strPrecertCode = GetBOLDField("BOLD_PreCertProg", aryInfo, mapValuesFromDialog);
	pPatVisit->pboldPatIns->bmuWeightLossAmt.bEstimated = _BOOL(GetBOLDField("BOLD_WtEst", aryInfo, mapValuesFromDialog));
	pPatVisit->pboldPatIns->bmuWeightLossAmt.bmuType = _BMU(GetBOLDField("BOLD_UnitType", aryInfo, mapValuesFromDialog));
	pPatVisit->pboldPatIns->bmuWeightLossAmt.dblValue = _DBL(GetBOLDField("BOLD_WtLossAmt", aryInfo, mapValuesFromDialog));

	FillPrevBariatricSurgeries(&pPatVisit->prevBariatricSurgeries, aryInfo, mapValuesFromDialog);
}

// (j.gruber 2010-06-01 11:04) - PLID 38949
BOOL CBoldLinkDlg::DoesPrevSurgExist(CPtrArray *prevSurgeries, long nEMNID) {

	//loop through the previous surgery array and see if we can find this EMNID
	for (int i = 0; i < prevSurgeries->GetSize(); i++) {
		BOLDPrevBarSurg *pSurg = ((BOLDPrevBarSurg*)prevSurgeries->GetAt(i));
		if (pSurg->nInternalID == nEMNID) {
			return TRUE;
		}
	}
	return FALSE;
}

// (j.gruber 2010-06-01 11:04) - PLID 38949
BOOL CBoldLinkDlg::GetPreviousBariatricProcedures(ADODB::_RecordsetPtr rsPat, CPtrArray *prevSurgeries, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfos)
{

	BOOL bShowDialog = FALSE;
	BOOL bAllowMultiples;

	BOLDCodeInfo *bldInfo = NULL; 

	ADODB::FieldsPtr flds = rsPat->Fields;
				
	CString strOldID = "";

	BOLDItem *bldItem = NULL;
	CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 	

	CString strCurrentEMNID, strOldEMNID;

	CString strCurrentID;				
	CString strRow;
	CString strSelection;

	while (! rsPat->eof) {	

		if (bldInfo == NULL ) {
			bldInfo = new BOLDCodeInfo();
		}

		if (aryItems == NULL) {
			aryItems = new CArray<BOLDItem*,BOLDItem*>();
		}

		if (bldItem == NULL) {
			bldItem = new BOLDItem();
			bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
		}

		CString strCode = AdoFldString(flds, "BOLDCode");
		CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
		long nItemID = AdoFldLong(flds, "ItemID");
		CString strItemName = AdoFldString(flds, "ItemName");
		CString strEMNName = AdoFldString(flds, "EMNName");
		COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
		CString strValue = AdoFldString(flds, "Value");				
		bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
		long nSelectID = AdoFldLong(flds, "SelectID");
		CString strSelectCode = AdoFldString(flds,"BoldDataCode");
		CString strEMNID = AsString(AdoFldLong(flds, "EMNID"));

		strCode += "_" + strEMNID;

		strCurrentEMNID = strEMNID;

		if (strCurrentEMNID != strOldEMNID) {

			//add one to our info array
			if (strOldEMNID != "") {	
				aryItems->Add(bldItem);
				bldInfo->aryItems = aryItems;

				if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
					bShowDialog = TRUE;
				}

				if (aryItems->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				aryInfos->Add(bldInfo);

				bldInfo = new BOLDCodeInfo();
				aryItems = new CArray<BOLDItem*,BOLDItem*>();
				bldItem = new BOLDItem();
				bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();

				strOldID = "";
			}

			//try to add it to our array of Previous visits
			if (!DoesPrevSurgExist(prevSurgeries, atoi(strEMNID))) {
				//make a new pointer
				BOLDPrevBarSurg *pSurg = new BOLDPrevBarSurg();
				pSurg->nInternalID = atoi(strEMNID);
				prevSurgeries->Add(pSurg);
			}

			strOldEMNID = strCurrentEMNID;
			

		}

		
		strCurrentID = strCode + "-" + AsString(nItemID);

		bldInfo->strCode = strCode;
		//bldInfo->strCodeDesc = strCodeDesc;
		strCodeDesc.Format("EMN Date: %s EMN Name: %s ", FormatDateTimeForInterface(dtEMN), strEMNName);
		bldInfo->strCodeDesc = strCodeDesc;
		bldInfo->bAllowMultiples = bAllowMultiples;					
		bldInfo->bContainsEMNID = TRUE;

		if (strOldID != strCurrentID) {

			//add one to our array
			if (strOldID != "") {						
				aryItems->Add(bldItem);
				bldItem = new BOLDItem();
				bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
				if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
					bShowDialog = TRUE;
				}
			}
		
			strOldID = strCurrentID;					
			bldItem->nItemID = nItemID;
			bldItem->strRow.Format("Item Name: %s", strItemName);
		}

	
		BOLDDetail *pDetail = new BOLDDetail();
		pDetail->strSelection = strValue;
		pDetail->nSelectID = nSelectID;
		pDetail->strSelectCode = strSelectCode;
		bldItem->paryDetails->Add(pDetail);

		rsPat->MoveNext();
	}		
	
	if (bldItem) {
		if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
			bShowDialog = TRUE;
		}
		aryItems->Add(bldItem);
	}

	if (aryItems) {
		if (aryItems->GetSize() > 1) {
			bShowDialog = TRUE;
		}
		bldInfo->aryItems = aryItems;
	}

	//add it to the top most array
	if (bldInfo) {
		aryInfos->Add(bldInfo);
	}
	return bShowDialog;
}

// (j.gruber 2010-06-01 11:04) - PLID 38949
void CBoldLinkDlg::FillPatientVisitInfo(long nPatientID, long nEMNID, BOLDPatientVisitInfo *pPatVisit, CString strPatientName) {

	//first get our vitals
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset("SELECT First, Middle, Last, Title, BirthDate, Gender, Employment, State, CountriesT.ISO3Code, "
		" Company FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
		" LEFT JOIN CountriesT ON PersonT.Country = CountriesT.CountryName "
		" WHERE PersonT.ID = {INT}"
		"\r\n"
		"\r\n"
		//Previous bariatric recordset

		"SET NOCOUNT ON; \r\n " 
			" DECLARE @nPatientID int; \r\n"
			" SET @nPatientID = {INT}; \r\n "
			" DECLARE @nEMNID int; \r\n"
			" SET @nEMNID = {INT}; \r\n "
			"\r\n"		
			
			" DECLARE @tResults3 TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255), EMNID INT) \r\n "
			
			" DECLARE rsItems3 CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_SurgName','BOLD_OrigWt', 'BOLD_OrigWtType', 'BOLD_OrigWtEst', "
			"	 'BOLD_LowestWt', 'BOLD_LowestWtType', 'BOLD_LowestWtEst', "
			"	 'BOLD_SurgYear', 'BOLD_Surgeon', 'BOLD_AdverseEvent'		"
				") \r\n "

			" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "
		
			"	Open rsItems3   \r\n "
			"	FETCH FROM rsItems3 INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults3 (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode, EMNID) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode, EMRMasterT.ID as EMNID    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.EMRCollectionID IN (SELECT ID FROM EMRCollectionT WHERE BoldTypeID = 2) AND EMRMasterT.Date <= (SELECT EMRMasterInnerT.Date FROM EMRMasterT EMRMasterInnerT WHERE EMRMasterInnerT.ID = @nEMNID) \r\n"
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults3 (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode, EMNID) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode, EMRMasterT.ID as EMNID	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.EMRCollectionID IN (SELECT ID FROM EMRCollectionT WHERE BoldTypeID = 2) AND EMRMasterT.Date <= (SELECT EMRMasterInnerT.Date FROM EMRMasterT EMRMasterInnerT WHERE EMRMasterInnerT.ID = @nEMNID) \r\n"
			"		END \r\n "
	
			"	FETCH FROM rsItems3 INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems3; \r\n "
			"	DEALLOCATE rsItems3; \r\n "
		"	SET NOCOUNT OFF; \r\n "
		
		//" SELECT * FROM ( \r\n "
		" SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  \r\n "
		" FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_SurgName' and Value <> ''  \r\n"
		//" UNION ALL \r\n "
		"\r\n"
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_OrigWt' and Value <> ''  \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_OrigWtType' and Value <> ''  \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_OrigWtEst' and Value <> ''  \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_LowestWt' and Value <> ''  \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_LowestWtType' and Value <> ''  \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_LowestWtEst' and Value <> ''  \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_SurgYear' and Value <> '' \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_Surgeon' and Value <> '' \r\n "
		//"    UNION ALL \r\n "
		"	SELECT TResults.*, CONVERT(bit, 1) as AllowMultiples  			\r\n "
		"	FROM @tResults3 TResults WHERE BOLDCode = 'BOLD_AdverseEvent' and Value <> '' \r\n "
		//"    ) Q Order by BOLDCode, EMNID \r\n "
		"\r\n"
		"\r\n"
		//Extras
		
		"SET NOCOUNT ON; \r\n " 			
			"\r\n"		
			
			" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_ConsentSRC', 'BOLD_Employment', 'BOLD_PrevNonBarSurg', 'BOLD_Race' "
				") \r\n "

			/*" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "*/
		
			"	Open rsItems   \r\n "
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID  \r\n"
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID  \r\n"
			"		END \r\n "
	
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems; \r\n "
			"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "
		
		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults TResults WHERE BOLDCode = 'BOLD_ConsentSRC' and Value <> '';\r\n "

		"	SELECT TResults.*, CONVERT(bit, 1) as AllowMultiples \r\n "			
		"	FROM @tResults TResults WHERE BOLDCode = 'BOLD_PrevNonBarSurg' and Value <> ''; "

		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults TResults WHERE BOLDCode = 'BOLD_Employment' and Value <> ''; "

		"	SELECT TResults.*, CONVERT(bit, 1) as AllowMultiples \r\n "			
		"	FROM @tResults TResults WHERE BOLDCode = 'BOLD_Race' and Value <> ''; "
		"\r\n"
		"\r\n"
		"SET NOCOUNT ON; \r\n " 
								
			" DECLARE @tResults2 TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int, BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems2 CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_PayCodes','BOLD_PreCertProg', 'BOLD_PreCertMentHealth', 'BOLD_WtLossAmt', 'BOLD_WtEst', 'BOLD_UnitType' "			
				") \r\n "

			/*" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "*/
					
			"	Open rsItems2   \r\n "
			"	FETCH FROM rsItems2 INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults2 (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BOLDCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0  \r\n "
			"			AND EMRMasterT.EMRCollectionID IN (SELECT ID FROM EMRCollectionT WHERE BoldTypeID = 3) \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults2 (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0  \r\n "
			"			AND EMRMasterT.EMRCollectionID IN (SELECT ID FROM EMRCollectionT WHERE BoldTypeID = 3) \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems2 INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems2; \r\n "
			"	DEALLOCATE rsItems2; \r\n "
		"	SET NOCOUNT OFF; \r\n "
		
		"	SELECT TResults.*, CONVERT(bit, 1) as AllowMultiples \r\n "			
		"	FROM @tResults2 TResults WHERE BOLDCode = 'BOLD_PayCodes' and Value <> '';\r\n "

		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults2 TResults WHERE BOLDCode = 'BOLD_PreCertProg' and Value <> '';\r\n  "

		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults2 TResults WHERE BOLDCode = 'BOLD_WtLossAmt' and Value <> '';\r\n  "

		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults2 TResults WHERE BOLDCode = 'BOLD_WtEst' and Value <> '';\r\n  "

		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults2 TResults WHERE BOLDCode = 'BOLD_UnitType' and Value <> ''; \r\n "

		"	SELECT TResults.*, CONVERT(bit, 0) as AllowMultiples \r\n "			
		"	FROM @tResults2 TResults WHERE BOLDCode = 'BOLD_PreCertMentHealth' and Value <> ''; \r\n "
		"\r\n"
		"\r\n",

		nPatientID, nPatientID, nEMNID);

	if (rsPat) {

		//the first recordset is patient info
		if (!rsPat->eof) {

			pPatVisit->strFirst = AdoFldString(rsPat, "First");
			pPatVisit->strLast = AdoFldString(rsPat, "Last");
			pPatVisit->strMiddleInit = AdoFldString(rsPat, "Middle").Left(1);

			//strPatientName = pPatVisit->strFirst + ' ' + pPatVisit->strMiddleInit  + ' ' + pPatVisit->strLast;

			pPatVisit->strSuffix = AdoFldString(rsPat, "Title");
			COleDateTime dtNull;
			dtNull.SetDate(1800,1,1);
			COleDateTime dtBirthDate = AdoFldDateTime(rsPat, "Birthdate", dtNull);
			if (dtBirthDate.GetStatus() == COleDateTime::valid && dtBirthDate > dtNull) {
				pPatVisit->nYearOfBirth = atoi(FormatDateTimeForSql(dtBirthDate, dtoDate).Left(4));
			}
			pPatVisit->strGenderCode = GetBOLDGenderCode(AdoFldByte(rsPat, "Gender", 0));			
			pPatVisit->strStateCode = AdoFldString(rsPat, "State", "");
			pPatVisit->strCountryCode = AdoFldString(rsPat, "ISO3Code", "");						
		}


		long nRecordsetSetCount = 0;

		CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;
		BOOL bShowDialog = FALSE;

		while (nRecordsetSetCount < 10) {
			
			rsPat = rsPat->NextRecordset(NULL);
			BOOL bTempShowDialog = FALSE;
			if (!rsPat->eof) {

				//this recordset is for the previous bariatric surgeries
				bTempShowDialog = GetPreviousBariatricProcedures(rsPat, &pPatVisit->prevBariatricSurgeries, &aryInfos);
			}
			if (bTempShowDialog) {
				bShowDialog = TRUE;
			}
			nRecordsetSetCount++;
		}

		rsPat = rsPat->NextRecordset(NULL);

		//the rest are codes that we need to loop through to check for dups		
		BOOL bAllowMultiples = FALSE;		

		if (rsPat) {
			
			while (rsPat) {

				BOLDCodeInfo *bldInfo = NULL; 

				ADODB::FieldsPtr flds = rsPat->Fields;
							
				CString strOldID = "";

				BOLDItem *bldItem = NULL;
				CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

				while (! rsPat->eof) {				

					if (bldInfo == NULL ) {
						bldInfo = new BOLDCodeInfo();
					}

					if (aryItems == NULL) {
						aryItems = new CArray<BOLDItem*,BOLDItem*>();
					}

					if (bldItem == NULL) {
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
					}

					CString strCode = AdoFldString(flds, "BOLDCode");
					CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
					long nItemID = AdoFldLong(flds, "ItemID");
					CString strItemName = AdoFldString(flds, "ItemName");
					CString strEMNName = AdoFldString(flds, "EMNName");
					COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
					CString strValue = AdoFldString(flds, "Value");				
					bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
					long nSelectID = AdoFldLong(flds, "SelectID");
					CString strSelectCode = AdoFldString(flds,"BoldDataCode");
					CString strCurrentID;				

					CString strRow;
					CString strSelection;

					strCurrentID = strCode + "-" + AsString(nItemID);

					bldInfo->strCode = strCode;
					bldInfo->strCodeDesc = strCodeDesc;
					bldInfo->bAllowMultiples = bAllowMultiples;					

					if (strOldID != strCurrentID) {

						//add one to our array
						if (strOldID != "") {						
							aryItems->Add(bldItem);
							bldItem = new BOLDItem();
							bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
							if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
								bShowDialog = TRUE;
							}
						}
						
						strOldID = strCurrentID;					
						bldItem->nItemID = nItemID;
						bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
					}

					
					BOLDDetail *pDetail = new BOLDDetail();
					pDetail->strSelection = strValue;
					pDetail->nSelectID = nSelectID;
					pDetail->strSelectCode = strSelectCode;
					bldItem->paryDetails->Add(pDetail);

					rsPat->MoveNext();
				}		

				//add the last value
				if (bldItem) {
					if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					aryItems->Add(bldItem);
				}

				if (aryItems) {
					if (aryItems->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					bldInfo->aryItems = aryItems;
				}

				//add it to the top most array
				if (bldInfo) {
					aryInfos.Add(bldInfo);
				}

				rsPat = rsPat->NextRecordset(NULL);
			}		
			
			//now we have to send everything to our dialog if necessary
			if (aryInfos.GetSize() != 0) {
				if (bShowDialog) {
					// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
					CBoldFieldPickDlg dlg(this);
					CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
					dlg.m_paryBoldList = &aryInfos;			
					dlg.m_pmapReturnValues = &mapValuesFromDialog;
					dlg.m_strPatientName = strPatientName;
					dlg.m_strVisitType = "Patient Visit";

					if (dlg.DoModal() == IDCANCEL) {
						FillPatientVisit(pPatVisit, &aryInfos, NULL);
					}
					else {
						//we now should have an map of values			
						FillPatientVisit(pPatVisit, &aryInfos, &mapValuesFromDialog);
					}
				}
				else {
					FillPatientVisit(pPatVisit, &aryInfos, NULL);				
				}
			}

			//now clear out all our memory
			for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
				BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

				for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

					BOLDItem *pItem = pInfo->aryItems->GetAt(j);

					for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
						
						BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
						delete pDetail;
					}				
					delete pItem->paryDetails;
					delete pItem;
				}

				delete pInfo->aryItems;

				delete pInfo;
			}

		}
	}		
}

// (j.gruber 2010-06-01 11:04) - PLID 38211
CString CBoldLinkDlg::GetBOLDMedCode(CString strNDC, long nFormatIndicator) {

	//the NDC has to be 11 digits, which should be all that FDB stores
	ASSERT(strNDC.GetLength() == 11);

	CString strFormattedNDC; 

	switch (nFormatIndicator) {
		case 0:
			//this is PIN, shouldn't be and NDC
			ASSERT(FALSE);
			strFormattedNDC.Format("%s-%s-%s", strNDC.Left(5), strNDC.Mid(5, 2), strNDC.Right(2));
		break;

		case 1:
			//4-4-2 with leading 0
			strFormattedNDC.Format("%s-%s-%s", "00" + strNDC.Mid(1,4), strNDC.Mid(5, 4), strNDC.Right(2));
		break;

		case 2:
			//5-3-2 with 0 in the 6th position
			strFormattedNDC.Format("%s-%s-%s", "0" + strNDC.Left(5), "*" + strNDC.Mid(6, 3), strNDC.Right(2));
		break;

		case 3:
			//5-4-1 with 0 in second to last
			strFormattedNDC.Format("%s-%s-%s", "0" + strNDC.Left(5), strNDC.Mid(5, 4), strNDC.Right(1));
		break;

		default:
			ASSERT(FALSE);
			return "";
		break;
	}

	if (!strFormattedNDC.IsEmpty()) {
		//bold wants the first 2 parts of the NDC for their code
		long nResult = strFormattedNDC.ReverseFind('-');
		CString strBOLDCode = strFormattedNDC.Left(nResult);
		strBOLDCode.Replace("-", "");
		return strBOLDCode;
	}

	return "";
}

// (j.gruber 2010-06-01 11:05) - PLID 38211
void CBoldLinkDlg::ConvertToMedicationCodeArray(CStringArray *strarySelectIDs) {

	
	CStringArray strAryCodes;
	BOOL bCheckFDB = TRUE;

	//only do this if there is something in the array
	if (strarySelectIDs->GetSize() != 0) {
		//now get the list NPIs and corresponding codes from firstdatabank
		if (!FirstDataBank::EnsureDatabase(NULL, true)) { //don't give a message, but notify nxserver
			//add one to our array so we can fail validation later
			strAryCodes.Add("");			
			bCheckFDB = FALSE;
		}

		if (bCheckFDB) {
			//the array is filled with FDBIds, we need to convert those to BOLD Medication Codes	
			CString strSql = BeginSqlBatch();
			for (int i = 0; i < strarySelectIDs->GetSize(); i++) {
				//taken from ChooseNDCNumberFromFirstDataBank Function
				AddStatementToSqlBatch(strSql, "SELECT TOP 1 FirstDataBank..RMINDC1_NDC_MEDID.NDC, FirstDataBank..RNDC14.NDCFI as FormatIndicator "
						"FROM FirstDataBank..RMINDC1_NDC_MEDID "
						"INNER JOIN FirstDataBank..RMIID1_MED ON FirstDataBank..RMINDC1_NDC_MEDID.MEDID = FirstDataBank..RMIID1_MED.MEDID "
						"INNER JOIN FirstDataBank..RNDC14 ON FirstDataBank..RMINDC1_NDC_MEDID.NDC = FirstDataBank..RNDC14.NDC "
						"WHERE FirstDataBank..RMINDC1_NDC_MEDID.MEDID = %li "
						//we want to order such that non-replaced, non-obsolete NDCs come first,
						//then ordered by newest NDC to oldest
						"ORDER BY "
						//REPNDC identifies a replacement NDC, we prefer those that have no replacements
						"Convert(bit, CASE WHEN FirstDataBank..RNDC14.REPNDC = '' OR FirstDataBank..RNDC14.REPNDC Is Null THEN 0 ELSE 1 END) ASC, "
						//OBSDTEC identifies when this NDC became/becomes obsolete
						"Convert(bit, CASE WHEN FirstDataBank..RNDC14.OBSDTEC = '00000000' OR Convert(datetime, FirstDataBank..RNDC14.OBSDTEC) >= GetDate() THEN 0 ELSE 1 END) ASC, "
						//identifies the 200 drug products most frequently dispensed in community pharmacies
						" Convert(bit, CASE WHEN FirstDataBank..RNDC14.TOP200 = 0 THEN 1 ELSE 0 END) ASC,  "
						//identifies those products most likely to be found in community pharmacies. 
						" Convert(bit, FirstDataBank..RNDC14.MINI) DESC,  "
						//identifies those products that are most likely to be found in hospital pharmacies
						" Convert(bit, FirstDataBank..RNDC14.HOSP) DESC,  "
						//identifies the 50 most frequently dispensed generic drugs
						" Convert(bit, CASE WHEN FirstDataBank..RNDC14.TOP50Gen = 0 THEN 1 ELSE 0 END) ASC,  "
						//DADDNDC is the date the NDC was added
						"Convert(datetime, FirstDataBank..RNDC14.DADDNC) DESC", atoi(strarySelectIDs->GetAt(i)));
			}

			ADODB::_RecordsetPtr rsNDC = CreateRecordsetStd(strSql);		

			while (rsNDC) {

				if (rsNDC) {

					if (rsNDC->eof) {

						//there wasn't an NDC, fail
						//put in a blank so we can fail validation later
						strAryCodes.Add("");
					}
					else {
						long nFI = atoi(AdoFldString(rsNDC, "FormatIndicator", ""));
						CString strNDC = AdoFldString(rsNDC, "NDC", "");

						if (strNDC.IsEmpty()) {
							//I don't think this is possible
							ASSERT(FALSE);
							//put in a blank so we can fail validation later
							strAryCodes.Add("");
						}

						strAryCodes.Add(GetBOLDMedCode(strNDC, nFI));
					}

				}
				rsNDC = rsNDC->NextRecordset(NULL);
			}
		}


		//now switch the arrays
		strarySelectIDs->RemoveAll();

		for (int i = 0; i < strAryCodes.GetSize(); i++) {
			strarySelectIDs->Add(strAryCodes.GetAt(i));
		}

		strAryCodes.RemoveAll();
		
	}
}

// (j.gruber 2010-06-01 11:05) - PLID 38950
void CBoldLinkDlg::FillPreOpVisit(long nEMNID, BOLDPreOpVisitInfo *pPreOpVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) 
{
	
	//ID
	//pPreOpVisit->nID = nEMNID;
	pPreOpVisit->dtVisitDate = _DATE(GetBOLDField("BOLD_VisitDate", aryInfo, mapValuesFromDialog));
	
	pPreOpVisit->bmuWeight.dblValue = _DBL(GetBOLDField("BOLD_Wt", aryInfo, mapValuesFromDialog));
	pPreOpVisit->bmuWeight.bEstimated = _BOOL(GetBOLDField("BOLD_WtEst", aryInfo, mapValuesFromDialog));
	pPreOpVisit->bmuWeight.bmuType = _BMU(GetBOLDField("BOLD_WtType", aryInfo, mapValuesFromDialog));

	pPreOpVisit->bmuHeight.dblValue = _DBL(GetBOLDField("BOLD_Ht", aryInfo, mapValuesFromDialog));
	pPreOpVisit->bmuHeight.bEstimated = _BOOL(GetBOLDField("BOLD_HtEst", aryInfo, mapValuesFromDialog));
	pPreOpVisit->bmuHeight.bmuType = _BMU(GetBOLDField("BOLD_HtType", aryInfo, mapValuesFromDialog));

	BuildStringArray(&pPreOpVisit->straryVitamins, "BOLD_Vit", aryInfo, mapValuesFromDialog);
	BuildSelectIDArray(&pPreOpVisit->straryMedications, "BOLD_Med", aryInfo, mapValuesFromDialog);

	//now convert those selectIDs to medication Codes
	ConvertToMedicationCodeArray(&pPreOpVisit->straryMedications);
	
	pPreOpVisit->strHYPERT = GetBOLDField("BOLD_PreHyp", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strCONGHF = GetBOLDField("BOLD_PreConHrt", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strISCHHD = GetBOLDField("BOLD_PreIsc", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strANGASM = GetBOLDField("BOLD_PreAng", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strPEVASD = GetBOLDField("BOLD_PrePevas", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strLOEXED = GetBOLDField("BOLD_PreLoExd", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strDVTPE = GetBOLDField("BOLD_PreDvType", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strGLUMET = GetBOLDField("BOLD_PreGlu", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strLIPDYH = GetBOLDField("BOLD_PreLipD", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strGOUHYP = GetBOLDField("BOLD_PreGouh", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strOBSSYN = GetBOLDField("BOLD_PreObssyn", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strOBHSYN = GetBOLDField("BOLD_PreObHsyn", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strPULHYP = GetBOLDField("BOLD_PrePulHyp", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strASTHMA = GetBOLDField("BOLD_PreAst", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strGERD = GetBOLDField("BOLD_PreGerd", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strCHOLEL = GetBOLDField("BOLD_PreCho", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strLVRDIS = GetBOLDField("BOLD_PreLvr", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strBCKPAIN = GetBOLDField("BOLD_PreBkP", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strMUSDIS = GetBOLDField("BOLD_PreMusDis", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strFBMGIA = GetBOLDField("BOLD_PreFbMg", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strPLOVSYN = GetBOLDField("BOLD_PrePlo", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strMENIRG = GetBOLDField("BOLD_PreMenIrg", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strPSYIMP = GetBOLDField("BOLD_PrePsyImp", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strDEPRSN = GetBOLDField("BOLD_PreDep", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strCONMEN = GetBOLDField("BOLD_PreConMen", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strALCUSE = GetBOLDField("BOLD_PreAlc", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strTOBUSE = GetBOLDField("BOLD_PreTob", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strSUBUSE = GetBOLDField("BOLD_PreSub", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strSTURIN = GetBOLDField("BOLD_PreStu", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strPSCRBR = GetBOLDField("BOLD_PrePsc", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strADBHER = GetBOLDField("BOLD_PreAbHer", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strFUNSTAT = GetBOLDField("BOLD_PreFunSt", aryInfo, mapValuesFromDialog);
	pPreOpVisit->strABDSKN = GetBOLDField("BOLD_PreAdbS", aryInfo, mapValuesFromDialog);
}

// (j.gruber 2010-06-01 11:05) - PLID 38950
void CBoldLinkDlg::FillPreOpVisitInfo(long nPatientID, long nEMNID, BOLDPreOpVisitInfo *pPreOpVisit, CString strPatientName) 
{
	// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset(""
		"SET NOCOUNT ON; \r\n " 	
		"	DECLARE @nPatientID INT; \r\n "
		"	SET @nPatientID = {INT}; \r\n "
		"	DECLARE @nEMNID INT; \r\n "
		"	SET @nEMNID = {INT}; \r\n "

			"\r\n"		
			
			" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_VisitDate', 'BOLD_WtType', 'BOLD_Wt', 'BOLD_WtEst', 'BOLD_Ht', 'BOLD_HtType', 'BOLD_HtEst', "
			" 'BOLD_Vit', 'BOLD_Med', 'BOLD_PreHyp', 'BOLD_PreConHrt', 'BOLD_PreIsc', 'BOLD_PreAng', "
			" 'BOLD_PrePevas', 'BOLD_PreLoExd', 'BOLD_PreDvType', 'BOLD_PreGlu', 'BOLD_PreLipD', 'BOLD_PreGouh', "
			" 'BOLD_PreObssyn', 'BOLD_PreObHsyn', 'BOLD_PrePulHyp', 'BOLD_PreAst', 'BOLD_PreGerd', 'BOLD_PreCho', "
			" 'BOLD_PreLvr', 'BOLD_PreBkP', 'BOLD_PreMusDis', 'BOLD_PreFbMg', 'BOLD_PrePlo', 'BOLD_PreMenIrg', 'BOLD_PrePsyImp', "
			" 'BOLD_PreDep', 'BOLD_PreConMen', 'BOLD_PreAlc', 'BOLD_PreTob', 'BOLD_PreSub', 'BOLD_PreStu', 'BOLD_PrePsc', 'BOLD_PreAbHer', "
			" 'BOLD_PreFunSt', 'BOLD_PreAdbS' \r\n"
			") \r\n "

			" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "
		
			"	Open rsItems   \r\n "
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems; \r\n "
			"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "

		"   SELECT Top 1 VisitID FROM BoldSentHistoryT WHERE EMNID = @nEMNID ORDER BY SentDate DESC; \r\n "
		
		" SELECT 'BOLD_Med' as BOLDCode, 'Medications' as CodeDesc, 'Current Medications' as ItemName,  \r\n "
		" EMRDataT.Data as Value, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMNName, \r\n "
		" EMRDetailsT.ID as ItemID, DrugList.FDBID as SelectID, EMRDataT.BoldCode as BoldDataCode, CONVERT(bit, 1) as AllowMultiples  \r\n "
		" FROM \r\n "
		" EMRDetailTableDataT INNER JOIN (SELECT ID FROM EMRDataT WHERE EMRInfoID IN ( \r\n "
		" SELECT ID FROM EMRInfoT WHERE DataSubType = 1) \r\n "
		" AND SortOrder = 1 AND ListType = 5) RxDataT ON EMRDetailTableDataT.EMRDataID_Y = RxDataT.ID \r\n "
		" LEFT JOIN EMRDataT ON EMRDetailTableDataT.EMRDataID_X = EMRDataT.ID \r\n "
		" LEFT JOIN EMRDetailsT ON EMRDetailTableDataT.EMRDetailID = EMRDetailsT.ID \r\n "
		" LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
		" LEFT JOIN (SELECT ID as EMRDataID, EMRDataGroupID FROM EMRDataT WHERE EMRInfoID IN (  \r\n "
		" SELECT ID FROM EMRInfoT WHERE DataSubType = 1 AND ID IN (SELECT ActiveEMRInfoID FROM EMrInfoMAsterT))) ActiveCurMedGroupsT \r\n "
		" ON EMRDataT.EMRDataGroupID = ActiveCurMedGroupsT.EMRDataGroupID		  \r\n "
		" LEFT JOIN DrugList ON ActiveCurMedGroupsT.EMRDataID  = DrugList.EmrDataID \r\n "		
		" WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID IN (SELECT ID FROM EMRInfoT WHERE DataSubType = 1) \r\n "
		" AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
		" AND EMRMasterT.ID = @nEMNID AND "
		//TES 11/1/2011 - PLID 46225 - This is in a WHERE clause, and so it's going to run this formula on the entire column.  
		// Therefore, if we don't have the the "Convert(float," here, it will attempt to convert all numeric text data to an integer 
		// (to compare it against 1), so if you have any numeric non-integer text (such as "3.5") this will throw an exception.
		" (CASE WHEN IsNumeric(CONVERT(NVARCHAR(3000), EMRDetailTableDataT.Data) + 'e0') = 1 THEN \r\n "
		" 	Convert(float,EMRDetailTableDataT.Data)  ELSE 0 END) = 1 \r\n "		
		
		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_VisitDate' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_WtType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Wt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_WtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Ht' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_HtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_HtType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Vit' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreHyp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreConHrt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreIsc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreAng' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PrePevas' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreLoExd' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreDvType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreGlu' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreLipD' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreGouh' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreObssyn' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreObHsyn' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PrePulHyp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreAst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreGerd' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreCho' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreLvr' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreBkP' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreMusDis' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreFbMg' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PrePlo' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreMenIrg' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PrePsyImp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreDep' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreConMen' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreAlc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreTob' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreSub' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreStu' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PrePsc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreAbHer' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreFunSt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PreAdbS' AND Value <> ''; \r\n "

		"\r\n"
		"\r\n",
		nPatientID, nEMNID);

	BOOL bShowDialog = FALSE;
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;

	if (rsPat) {		
		
		BOOL bAllowMultiples = FALSE;		

		if (rsPat) {

			//first recordset is the visitID
			if (!rsPat->eof) {
				pPreOpVisit->strVisitID = AdoFldString(rsPat, "VisitID", "");
			}
			rsPat = rsPat->NextRecordset(NULL);
			
			while (rsPat) {

				BOLDCodeInfo *bldInfo = NULL; 

				ADODB::FieldsPtr flds = rsPat->Fields;
							
				CString strOldID = "";

				BOLDItem *bldItem = NULL;
				CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

				while (! rsPat->eof) {				

					if (bldInfo == NULL ) {
						bldInfo = new BOLDCodeInfo();
					}

					if (aryItems == NULL) {
						aryItems = new CArray<BOLDItem*,BOLDItem*>();
					}

					if (bldItem == NULL) {
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
					}

					CString strCode = AdoFldString(flds, "BOLDCode");
					CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
					long nItemID = AdoFldLong(flds, "ItemID");
					CString strItemName = AdoFldString(flds, "ItemName");
					CString strEMNName = AdoFldString(flds, "EMNName");
					COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
					CString strValue = AdoFldString(flds, "Value");				
					bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
					long nSelectID = AdoFldLong(flds, "SelectID");
					CString strSelectCode = AdoFldString(flds,"BoldDataCode");
					CString strCurrentID;				

					CString strRow;
					CString strSelection;

					strCurrentID = strCode + "-" + AsString(nItemID);

					bldInfo->strCode = strCode;
					bldInfo->strCodeDesc = strCodeDesc;
					bldInfo->bAllowMultiples = bAllowMultiples;					

					if (strOldID != strCurrentID) {

						//add one to our array
						if (strOldID != "") {						
							aryItems->Add(bldItem);
							bldItem = new BOLDItem();
							bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
							if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
								bShowDialog = TRUE;
							}
						}
						
						strOldID = strCurrentID;					
						bldItem->nItemID = nItemID;
						bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
					}

					
					BOLDDetail *pDetail = new BOLDDetail();
					pDetail->strSelection = strValue;
					pDetail->nSelectID = nSelectID;
					pDetail->strSelectCode = strSelectCode;
					bldItem->paryDetails->Add(pDetail);

					rsPat->MoveNext();
				}		

				//add the last value
				if (bldItem) {
					if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					aryItems->Add(bldItem);
				}

				if (aryItems) {
					if (aryItems->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					bldInfo->aryItems = aryItems;
				}

				//add it to the top most array
				if (bldInfo) {
					aryInfos.Add(bldInfo);
				}

				rsPat = rsPat->NextRecordset(NULL);
			}		
			
			//now we have to send everything to our dialog if necessary
			if (aryInfos.GetSize() != 0) {
				if (bShowDialog) {
					// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
					CBoldFieldPickDlg dlg(this);
					CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
					dlg.m_paryBoldList = &aryInfos;			
					dlg.m_pmapReturnValues = &mapValuesFromDialog;
					dlg.m_strPatientName = strPatientName;
					dlg.m_strVisitType = "Preoperative Visit";

					if (dlg.DoModal() == IDCANCEL) {
						FillPreOpVisit(nEMNID, pPreOpVisit, &aryInfos, NULL);
					}
					else {
						//we now should have an map of values			
						FillPreOpVisit(nEMNID, pPreOpVisit, &aryInfos, &mapValuesFromDialog);
					}
				}
				else {
					FillPreOpVisit(nEMNID, pPreOpVisit, &aryInfos, NULL);				
				}
			}

			//now clear out all our memory
			for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
				BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

				for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

					BOLDItem *pItem = pInfo->aryItems->GetAt(j);

					for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
						
						BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
						delete pDetail;
					}				
					delete pItem->paryDetails;
					delete pItem;
				}

				delete pInfo->aryItems;

				delete pInfo;
			}

		}
	}		
}

// (j.gruber 2010-06-01 11:05) - PLID 38951
void CBoldLinkDlg::FillHospVisitInfo(long nPatientID, long nEMNID, BOLDHospitalVisit *pHospVisit, CString strPatientName)
{

	
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset(""
		"SET NOCOUNT ON; \r\n " 	
		"	DECLARE @nPatientID INT; \r\n "
		"	SET @nPatientID = {INT}; \r\n "
		"	DECLARE @nEMNID INT; \r\n "
		"	SET @nEMNID = {INT}; \r\n "
		
		"\r\n"		
		" SET NOCOUNT OFF \r\n " 
		" SELECT Top 1 VisitID FROM BoldSentHistoryT WHERE EMNID = @nEMNID ORDER BY SentDate DESC; \r\n "
		"SET NOCOUNT ON \r\n "
			
		" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255), EMNID int) \r\n "
		
		" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
		" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
		"	--bold codes \r\n "
		"	('BOLD_AdvEv', 'BOLD_TmAfSurg', 'BOLD_TmAfMs', 'BOLD_Surgeon', 'BOLD_AESurg' \r\n"		
		") \r\n "

		" DECLARE @nEMRInfoID INT; \r\n "
		" DECLARE @nDataType INT; \r\n "
		" DECLARE @strDataCode nvarChar(50); \r\n "
		" DECLARE @strEMRInfoName nVarChar(255); \r\n "
		" DECLARE @strCodeDesc nVarChar(255); \r\n "
	
		"	Open rsItems   \r\n "
		"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
		"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "

		"	/*get all the information we'll need for this patient */ \r\n "
		"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
		"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode, EMNID) \r\n "
		"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode, EMRMasterT.ID as EMNID    \r\n "
		"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
		"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
		"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
		"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
		"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
		"			AND EMRMasterT.EMRCollectionID IN (SELECT ID FROM EMRCollectionT WHERE BoldTypeID = 6) \r\n "
		"		END \r\n "
		"		ELSE BEGIN \r\n "
		"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode, EMNID) \r\n "
		"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
		"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
		"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
		"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode, EMRMasterT.ID AS EMNID	  \r\n "
		"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
		"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
		"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
		"			AND EMRMasterT.EMRCollectionID IN (SELECT ID FROM EMRCollectionT WHERE BoldTypeID = 6) \r\n "
		"		END \r\n "

		"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
		
		"	END \r\n "

		"	CLOSE rsItems; \r\n "
		"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "
		
		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_AdvEv' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_TmAfSurg' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_TmAfMs' AND Value <> ''; \r\n "		

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Surgeon' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_AESurg' AND Value <> ''; \r\n "

		"\r\n"
		"\r\n"
			//Hospital Visit
			"\r\n"		
			"SET NOCOUNT ON; \r\n " 	
			" DECLARE @tResults2 TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems2 CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_SurgDt', 'BOLD_AdmDt', 'BOLD_LstWtDt', 'BOLD_Rev', 'BOLD_SurgDur', 'BOLD_Fac', \r\n"
			"	 'BOLD_Ht', 'BOLD_HtEst', 'BOLD_HtType', 'BOLD_Surgeon', \r\n "
			"    'BOLD_AnesDur', 'BOLD_EstBlLos', 'BOLD_BldTrans', 'BOLD_LtWtBfSurg', 'BOLD_LtWtBfSurgType', 'BOLD_LtWtBfSurgEst', \r\n "
			"	 'BOLD_SurgResPt', 'BOLD_SurgFelPt', 'BOLD_DiscDt', 'BOLD_DiscLc', 'BOLD_ASACls', 'BOLD_BarPrc', 'BOLD_BarTech', \r\n "
			"    'BOLD_DVTProp', 'BOLD_ConPrc', 'BOLD_InOpEv' \r\n "
			") \r\n "

		/*	" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "*/
		
			"	Open rsItems2   \r\n "
			"	FETCH FROM rsItems2 INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults2 (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults2 (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems2 INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems2; \r\n "
			"	DEALLOCATE rsItems2; \r\n "
		"	SET NOCOUNT OFF; \r\n "
		
		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_SurgDt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_AdmDt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_Fac' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_LstWtDt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_Ht' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_HtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_HtType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_Surgeon' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_Rev' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_SurgDur' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_AnesDur' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_EstBlLos' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_BldTrans' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_LtWtBfSurg' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_LtWtBfSurgType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_LtWtBfSurgEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_SurgResPt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_SurgFelPt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_DiscDt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_DiscLc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_ASACls' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_BarPrc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_BarTech' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_DVTProp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_ConPrc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults2 TResults WHERE BoldCode = 'BOLD_InOpEv' AND Value <> ''; \r\n "
		"\r\n"
		"\r\n",
		nPatientID, nEMNID);

	BOOL bShowDialog = FALSE;
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;

	if (rsPat) {		

		//first recordset is past visit
		if (!rsPat->eof) {
			pHospVisit->strHospitalID = AdoFldString(rsPat, "VisitID", "");
		}
		rsPat = rsPat->NextRecordset(NULL);
		
		BOOL bAllowMultiples = FALSE;				
		long nRecordsetSetCount = 0;		
		
		while (nRecordsetSetCount < 5) {
			
			if (nRecordsetSetCount != 0) {
				rsPat = rsPat->NextRecordset(NULL);
			}
			BOOL bTempShowDialog = FALSE;
			if (!rsPat->eof) {

				//this recordset is for the previous bariatric surgeries
				bTempShowDialog = GetAdverseEvents(rsPat, &pHospVisit->aryAdverseEventsBeforeDischarge, &aryInfos);
			}
			if (bTempShowDialog) {
				bShowDialog = TRUE;
			}
			nRecordsetSetCount++;
		}

		rsPat = rsPat->NextRecordset(NULL);
			
		while (rsPat) {

			BOLDCodeInfo *bldInfo = NULL; 

			ADODB::FieldsPtr flds = rsPat->Fields;
						
			CString strOldID = "";

			BOLDItem *bldItem = NULL;
			CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

			while (! rsPat->eof) {				

				if (bldInfo == NULL ) {
					bldInfo = new BOLDCodeInfo();
				}

				if (aryItems == NULL) {
					aryItems = new CArray<BOLDItem*,BOLDItem*>();
				}

				if (bldItem == NULL) {
					bldItem = new BOLDItem();
					bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
				}

				CString strCode = AdoFldString(flds, "BOLDCode");
				CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
				long nItemID = AdoFldLong(flds, "ItemID");
				CString strItemName = AdoFldString(flds, "ItemName");
				CString strEMNName = AdoFldString(flds, "EMNName");
				COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
				CString strValue = AdoFldString(flds, "Value");				
				bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
				long nSelectID = AdoFldLong(flds, "SelectID");
				CString strSelectCode = AdoFldString(flds,"BoldDataCode");
				CString strCurrentID;				

				CString strRow;
				CString strSelection;

				strCurrentID = strCode + "-" + AsString(nItemID);

				bldInfo->strCode = strCode;
				bldInfo->strCodeDesc = strCodeDesc;
				bldInfo->bAllowMultiples = bAllowMultiples;					

				if (strOldID != strCurrentID) {

					//add one to our array
					if (strOldID != "") {						
						aryItems->Add(bldItem);
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
						if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
							bShowDialog = TRUE;
						}
					}
					
					strOldID = strCurrentID;					
					bldItem->nItemID = nItemID;
					bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
				}

				
				BOLDDetail *pDetail = new BOLDDetail();
				pDetail->strSelection = strValue;
				pDetail->nSelectID = nSelectID;
				pDetail->strSelectCode = strSelectCode;
				bldItem->paryDetails->Add(pDetail);

				rsPat->MoveNext();
			}		

			//add the last value
			if (bldItem) {
				if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				aryItems->Add(bldItem);
			}

			if (aryItems) {
				if (aryItems->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				bldInfo->aryItems = aryItems;
			}

			//add it to the top most array
			if (bldInfo) {
				aryInfos.Add(bldInfo);
			}

			rsPat = rsPat->NextRecordset(NULL);
		}		
		
		//now we have to send everything to our dialog if necessary
		if (aryInfos.GetSize() != 0) {
			if (bShowDialog) {
				// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
				CBoldFieldPickDlg dlg(this);
				CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
				dlg.m_paryBoldList = &aryInfos;			
				dlg.m_pmapReturnValues = &mapValuesFromDialog;
				dlg.m_strPatientName = strPatientName;
				dlg.m_strVisitType = "Hospital Visit";

				if (dlg.DoModal() == IDCANCEL) {
					FillHospVisit(nEMNID, pHospVisit, &aryInfos, NULL);
				}
				else {
					//we now should have an map of values			
					FillHospVisit(nEMNID, pHospVisit, &aryInfos, &mapValuesFromDialog);
				}
			}
			else {
				FillHospVisit(nEMNID, pHospVisit, &aryInfos, NULL);				
			}
		}

		//now clear out all our memory
		for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
			BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

			for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

				BOLDItem *pItem = pInfo->aryItems->GetAt(j);

				for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
					
					BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
					delete pDetail;
				}				
				delete pItem->paryDetails;
				delete pItem;
			}

			delete pInfo->aryItems;

			delete pInfo;
		}

		
	}		

}

// (j.gruber 2010-06-01 11:05) - PLID 38951
void CBoldLinkDlg::FillHospVisit(long nEMNID, BOLDHospitalVisit *pHospVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog)
{

	pHospVisit->dtSurgery = _DATE(GetBOLDField("BOLD_SurgDt", aryInfo, mapValuesFromDialog));
	pHospVisit->dtAdmission = _DATE(GetBOLDField("BOLD_AdmDt", aryInfo, mapValuesFromDialog));
	pHospVisit->dtLastWeight = _DATE(GetBOLDField("BOLD_LstWtDt", aryInfo, mapValuesFromDialog));
	
	pHospVisit->bmuHeight.dblValue = _DBL(GetBOLDField("BOLD_Ht", aryInfo, mapValuesFromDialog));
	pHospVisit->bmuHeight.bEstimated = _BOOL(GetBOLDField("BOLD_HtEst", aryInfo, mapValuesFromDialog));
	pHospVisit->bmuHeight.bmuType = _BMU(GetBOLDField("BOLD_HtType", aryInfo, mapValuesFromDialog));

	pHospVisit->bRevision  = _BOOL(GetBOLDField("BOLD_Rev", aryInfo, mapValuesFromDialog));
	pHospVisit->strFacilityID = GetBOLDField("BOLD_Fac", aryInfo, mapValuesFromDialog);
	pHospVisit->strSurgeonID = GetBOLDField("BOLD_Surgeon", aryInfo, mapValuesFromDialog);

	pHospVisit->dblDurationSurgery = _DBL(GetBOLDField("BOLD_SurgDur", aryInfo, mapValuesFromDialog));
	pHospVisit->dblDurationAnesthesia = _DBL(GetBOLDField("BOLD_AnesDur", aryInfo, mapValuesFromDialog));
	pHospVisit->sdblEstBloodLoss = AsString(_DBL(GetBOLDField("BOLD_EstBlLos", aryInfo, mapValuesFromDialog)));
	pHospVisit->sdblBloodTransfusionUnits = AsString(_DBL(GetBOLDField("BOLD_BldTrans", aryInfo, mapValuesFromDialog)));

	pHospVisit->bmuLastWeightBeforeSurgery.dblValue = _DBL(GetBOLDField("BOLD_LtWtBfSurg", aryInfo, mapValuesFromDialog));
	pHospVisit->bmuLastWeightBeforeSurgery.bEstimated = _BOOL(GetBOLDField("BOLD_LtWtBfSurgEst", aryInfo, mapValuesFromDialog));
	pHospVisit->bmuLastWeightBeforeSurgery.bmuType = _BMU(GetBOLDField("BOLD_LtWtBfSurgType", aryInfo, mapValuesFromDialog));

	pHospVisit->bSurgicalResidentParticipated = _BOOL(GetBOLDField("BOLD_SurgResPt", aryInfo, mapValuesFromDialog));
	pHospVisit->bSurgicalFellowParticipated = _BOOL(GetBOLDField("BOLD_SurgFelPt", aryInfo, mapValuesFromDialog));

	pHospVisit->dtDischargeDate = _DATE(GetBOLDField("BOLD_DiscDt", aryInfo, mapValuesFromDialog));
	pHospVisit->strDischargeLocation = GetBOLDField("BOLD_DiscLc", aryInfo, mapValuesFromDialog);
	pHospVisit->strASAClassificationCode = GetBOLDField("BOLD_ASACls", aryInfo, mapValuesFromDialog);
	pHospVisit->strBariatricProcedureCode = GetBOLDField("BOLD_BarPrc", aryInfo, mapValuesFromDialog);
	pHospVisit->strBariatricTechniqueCode = GetBOLDField("BOLD_BarTech", aryInfo, mapValuesFromDialog);

	BuildStringArray(&pHospVisit->straryDVTTherapies, "BOLD_DVTProp", aryInfo, mapValuesFromDialog);
	BuildStringArray(&pHospVisit->straryConcurrentProcs, "BOLD_ConPrc", aryInfo, mapValuesFromDialog);
	BuildStringArray(&pHospVisit->straryIntraOpAdverseEvents, "BOLD_InOpEv", aryInfo, mapValuesFromDialog);

	FillAdverseEvents(&pHospVisit->aryAdverseEventsBeforeDischarge, aryInfo, mapValuesFromDialog);

}

// (j.gruber 2010-06-01 11:06) - PLID 38951
void CBoldLinkDlg::FillAdverseEvents(CPtrArray *paryAdverseEvents, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog)
{

	//we already have our array setup, just not filled so we just need to fill it here
	for (int i = 0; i < paryAdverseEvents->GetSize(); i++) {

		BOLDAdverseEvents *pAE = ((BOLDAdverseEvents*)paryAdverseEvents->GetAt(i));
		//pop it off the array
		paryAdverseEvents->RemoveAt(i);

		long nEMNID = pAE->nInternalID;
		CString strEMNID = AsString(nEMNID);

		pAE->strCode = GetBOLDField("BOLD_AdvEv_"+strEMNID, aryInfo, mapValuesFromDialog);
		pAE->nTimeAfterSurgery = _LONG(GetBOLDField("BOLD_TmAfSurg_"+strEMNID, aryInfo, mapValuesFromDialog));
		pAE->btuTimeAfterMeasurement= _BTU(GetBOLDField("BOLD_TmAfMs_"+strEMNID, aryInfo, mapValuesFromDialog));		
		pAE->strSurgeonID = GetBOLDField("BOLD_Surgeon_"+strEMNID, aryInfo, mapValuesFromDialog);
		BuildStringArray(&pAE->strarySurgeryCodes, "BOLD_AESurg_"+strEMNID, aryInfo, mapValuesFromDialog);		

		//add it back to the array
		paryAdverseEvents->InsertAt(i,pAE);

	}
}

// (j.gruber 2010-06-01 11:06) - PLID 38951
BOOL CBoldLinkDlg::DoesAdverseEventExist(CPtrArray *paryAE, long nEMNID) 
{
	//loop through the previous surgery array and see if we can find this EMNID
	for (int i = 0; i < paryAE->GetSize(); i++) {
		BOLDAdverseEvents *pAE = ((BOLDAdverseEvents*)paryAE->GetAt(i));
		if (pAE->nInternalID == nEMNID) {
			return TRUE;
		}
	}
	return FALSE;

}

// (j.gruber 2010-06-01 11:06) - PLID 38951
BOOL CBoldLinkDlg::GetAdverseEvents(ADODB::_RecordsetPtr rsAE, CPtrArray *paryAdverseEvents, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfos)
{

	BOOL bShowDialog = FALSE;
	BOOL bAllowMultiples;

	BOLDCodeInfo *bldInfo = NULL; 

	ADODB::FieldsPtr flds = rsAE->Fields;
				
	CString strOldID = "";

	BOLDItem *bldItem = NULL;
	CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 	

	CString strCurrentEMNID, strOldEMNID;

	CString strCurrentID;				
	CString strRow;
	CString strSelection;

	while (! rsAE->eof) {	

		if (bldInfo == NULL ) {
			bldInfo = new BOLDCodeInfo();
		}

		if (aryItems == NULL) {
			aryItems = new CArray<BOLDItem*,BOLDItem*>();
		}

		if (bldItem == NULL) {
			bldItem = new BOLDItem();
			bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
		}

		CString strCode = AdoFldString(flds, "BOLDCode");
		CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
		long nItemID = AdoFldLong(flds, "ItemID");
		CString strItemName = AdoFldString(flds, "ItemName");
		CString strEMNName = AdoFldString(flds, "EMNName");
		COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
		CString strValue = AdoFldString(flds, "Value");				
		bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
		long nSelectID = AdoFldLong(flds, "SelectID");
		CString strSelectCode = AdoFldString(flds,"BoldDataCode");
		CString strEMNID = AsString(AdoFldLong(flds, "EMNID"));

		strCode += "_" + strEMNID;

		strCurrentEMNID = strEMNID;

		if (strCurrentEMNID != strOldEMNID) {

			//add one to our info array
			if (strOldEMNID != "") {	
				aryItems->Add(bldItem);
				bldInfo->aryItems = aryItems;

				if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
					bShowDialog = TRUE;
				}

				if (aryItems->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				aryInfos->Add(bldInfo);

				bldInfo = new BOLDCodeInfo();
				aryItems = new CArray<BOLDItem*,BOLDItem*>();
				bldItem = new BOLDItem();
				bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();

				strOldID = "";
			}

			//try to add it to our array of Previous visits
			if (!DoesAdverseEventExist(paryAdverseEvents, atoi(strEMNID))) {
				//make a new pointer
				BOLDAdverseEvents *pAE = new BOLDAdverseEvents();
				pAE->nInternalID = atoi(strEMNID);
				paryAdverseEvents->Add(pAE);
			}

			strOldEMNID = strCurrentEMNID;
			

		}

		
		strCurrentID = strCode + "-" + AsString(nItemID);

		bldInfo->strCode = strCode;
		//bldInfo->strCodeDesc = strCodeDesc;
		strCodeDesc.Format("EMN Date: %s EMN Name: %s ", FormatDateTimeForInterface(dtEMN), strEMNName);
		bldInfo->strCodeDesc = strCodeDesc;
		bldInfo->bAllowMultiples = bAllowMultiples;					
		bldInfo->bContainsEMNID = TRUE;

		if (strOldID != strCurrentID) {

			//add one to our array
			if (strOldID != "") {						
				aryItems->Add(bldItem);
				bldItem = new BOLDItem();
				bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
				if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
					bShowDialog = TRUE;
				}
			}
		
			strOldID = strCurrentID;					
			bldItem->nItemID = nItemID;
			bldItem->strRow.Format("Item Name: %s", strItemName);
		}

	
		BOLDDetail *pDetail = new BOLDDetail();
		pDetail->strSelection = strValue;
		pDetail->nSelectID = nSelectID;
		pDetail->strSelectCode = strSelectCode;
		bldItem->paryDetails->Add(pDetail);

		rsAE->MoveNext();
	}		
	
	if (bldItem) {
		if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
			bShowDialog = TRUE;
		}
		aryItems->Add(bldItem);
	}

	if (aryItems) {
		if (aryItems->GetSize() > 1) {
			bShowDialog = TRUE;
		}
		bldInfo->aryItems = aryItems;
	}

	//add it to the top most array
	if (bldInfo) {
		aryInfos->Add(bldInfo);
	}
	return bShowDialog;
}


// (j.gruber 2010-06-01 11:06) - PLID 38953
void CBoldLinkDlg::FillPostOpVisit(long nEMNID, BOLDPostOpVisitInfo *pPostOpVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) 
{
	
	//ID
	//pPostOpVisit->nID = nEMNID;
	pPostOpVisit->dtVisitDate = _DATE(GetBOLDField("BOLD_VisitDate", aryInfo, mapValuesFromDialog));
	
	pPostOpVisit->bmuWeight.dblValue = _DBL(GetBOLDField("BOLD_Wt", aryInfo, mapValuesFromDialog));
	pPostOpVisit->bmuWeight.bEstimated = _BOOL(GetBOLDField("BOLD_WtEst", aryInfo, mapValuesFromDialog));
	pPostOpVisit->bmuWeight.bmuType = _BMU(GetBOLDField("BOLD_WtType", aryInfo, mapValuesFromDialog));

	pPostOpVisit->bmuHeight.dblValue = _DBL(GetBOLDField("BOLD_Ht", aryInfo, mapValuesFromDialog));
	pPostOpVisit->bmuHeight.bEstimated = _BOOL(GetBOLDField("BOLD_HtEst", aryInfo, mapValuesFromDialog));
	pPostOpVisit->bmuHeight.bmuType = _BMU(GetBOLDField("BOLD_HtType", aryInfo, mapValuesFromDialog));

	BuildStringArray(&pPostOpVisit->straryVitamins, "BOLD_Vit", aryInfo, mapValuesFromDialog);
	BuildSelectIDArray(&pPostOpVisit->straryMedications, "BOLD_Med", aryInfo, mapValuesFromDialog);

	//now convert those selectIDs to medication Codes
	ConvertToMedicationCodeArray(&pPostOpVisit->straryMedications);

	pPostOpVisit->strSupportGroup = GetBOLDField("BOLD_SpGrpFq", aryInfo, mapValuesFromDialog);

	pPostOpVisit->strHYPERT = GetBOLDField("BOLD_PstHyp", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strCONGHF = GetBOLDField("BOLD_PstConHrt", aryInfo, mapValuesFromDialog);
	//pPostOpVisit->strISCHHD = GetBOLDField("BOLD_PstIsc", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strANGASM = GetBOLDField("BOLD_PstAng", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strPEVASD = GetBOLDField("BOLD_PstPevas", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strLOEXED = GetBOLDField("BOLD_PstLoExd", aryInfo, mapValuesFromDialog);
	//pPostOpVisit->strDVTPE = GetBOLDField("BOLD_PstDvType", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strGLUMET = GetBOLDField("BOLD_PstGlu", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strLIPDYH = GetBOLDField("BOLD_PstLipD", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strGOUHYP = GetBOLDField("BOLD_PstGouh", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strOBSSYN = GetBOLDField("BOLD_PstObssyn", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strOBHSYN = GetBOLDField("BOLD_PstObHsyn", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strPULHYP = GetBOLDField("BOLD_PstPulHyp", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strASTHMA = GetBOLDField("BOLD_PstAst", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strGERD = GetBOLDField("BOLD_PstGerd", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strCHOLEL = GetBOLDField("BOLD_PstCho", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strLVRDIS = GetBOLDField("BOLD_PstLvr", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strBCKPAIN = GetBOLDField("BOLD_PstBkP", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strMUSDIS = GetBOLDField("BOLD_PstMusDis", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strFBMGIA = GetBOLDField("BOLD_PstFbMg", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strPLOVSYN = GetBOLDField("BOLD_PstPlo", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strMENIRG = GetBOLDField("BOLD_PstMenIrg", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strPSYIMP = GetBOLDField("BOLD_PstPsyImp", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strDEPRSN = GetBOLDField("BOLD_PstDep", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strCONMEN = GetBOLDField("BOLD_PstConMen", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strALCUSE = GetBOLDField("BOLD_PstAlc", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strTOBUSE = GetBOLDField("BOLD_PstTob", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strSUBUSE = GetBOLDField("BOLD_PstSub", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strSTURIN = GetBOLDField("BOLD_PstStu", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strPSCRBR = GetBOLDField("BOLD_PstPsc", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strADBHER = GetBOLDField("BOLD_PstAbHer", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strFUNSTAT = GetBOLDField("BOLD_PstFunSt", aryInfo, mapValuesFromDialog);
	pPostOpVisit->strABDSKN = GetBOLDField("BOLD_PstAdbS", aryInfo, mapValuesFromDialog);
}

// (j.gruber 2010-06-01 11:06) - PLID 38953
void CBoldLinkDlg::FillPostOpVisitInfo(long nPatientID, long nEMNID, BOLDPostOpVisitInfo *pPostOpVisit, CString strPatientName) 
{
	// (j.gruber 2010-06-08 13:08) - PLID 39047 - changed NewCropID TO FDBID
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset(""
		"SET NOCOUNT ON; \r\n " 	
		"	DECLARE @nPatientID INT; \r\n "
		"	SET @nPatientID = {INT}; \r\n "
		"	DECLARE @nEMNID INT; \r\n "
		"	SET @nEMNID = {INT}; \r\n "

			"\r\n"		
			
			" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_VisitDate', 'BOLD_WtType', 'BOLD_Wt', 'BOLD_WtEst', 'BOLD_Ht', 'BOLD_HtType', 'BOLD_HtEst', "
			" 'BOLD_Vit', 'BOLD_PstHyp', 'BOLD_PstConHrt', 'BOLD_PstIsc', 'BOLD_PstAng', "
			" 'BOLD_PstPevas', 'BOLD_PstLoExd', 'BOLD_PstDvType', 'BOLD_PstGlu', 'BOLD_PstLipD', 'BOLD_PstGouh', "
			" 'BOLD_PstObssyn', 'BOLD_PstObHsyn', 'BOLD_PstPulHyp', 'BOLD_PstAst', 'BOLD_PstGerd', 'BOLD_PstCho', "
			" 'BOLD_PstLvr', 'BOLD_PstBkP', 'BOLD_PstMusDis', 'BOLD_PstFbMg', 'BOLD_PstPlo', 'BOLD_PstMenIrg', 'BOLD_PstPsyImp', "
			" 'BOLD_PstDep', 'BOLD_PstConMen', 'BOLD_PstAlc', 'BOLD_PstTob', 'BOLD_PstSub', 'BOLD_PstStu', 'BOLD_PstPsc', 'BOLD_PstAbHer', "
			" 'BOLD_PstFunSt', 'BOLD_PstAdbS', 'BOLD_SpGrpFq' \r\n"
			") \r\n "

			" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "
		
			"	Open rsItems   \r\n "
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems; \r\n "
			"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "
		
		"   SELECT Top 1 VisitID FROM BoldSentHistoryT WHERE EMNID = @nEMNID ORDER BY SentDate DESC; \r\n "

		" SELECT 'BOLD_Med' as BOLDCode, 'Medications' as CodeDesc, 'Current Medications' as ItemName,  \r\n "
		" EMRDataT.Data as Value, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMNName, \r\n "
		" EMRDetailsT.ID as ItemID, DrugList.FDBID as SelectID, EMRDataT.BoldCode as BoldDataCode, CONVERT(bit, 1) as AllowMultiples  \r\n "
		" FROM \r\n "
		" EMRDetailTableDataT INNER JOIN (SELECT ID FROM EMRDataT WHERE EMRInfoID IN ( \r\n "
		" SELECT ID FROM EMRInfoT WHERE DataSubType = 1) \r\n "
		" AND SortOrder = 1 AND ListType = 5) RxDataT ON EMRDetailTableDataT.EMRDataID_Y = RxDataT.ID \r\n "
		" LEFT JOIN EMRDataT ON EMRDetailTableDataT.EMRDataID_X = EMRDataT.ID \r\n "
		" LEFT JOIN EMRDetailsT ON EMRDetailTableDataT.EMRDetailID = EMRDetailsT.ID \r\n "
		" LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
		" LEFT JOIN (SELECT ID as EMRDataID, EMRDataGroupID FROM EMRDataT WHERE EMRInfoID IN (  \r\n "
		" SELECT ID FROM EMRInfoT WHERE DataSubType = 1 AND ID IN (SELECT ActiveEMRInfoID FROM EMrInfoMAsterT))) ActiveCurMedGroupsT \r\n "
		" ON EMRDataT.EMRDataGroupID = ActiveCurMedGroupsT.EMRDataGroupID		  \r\n "
		" LEFT JOIN DrugList ON ActiveCurMedGroupsT.EMRDataID  = DrugList.EmrDataID \r\n "		
		" WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID IN (SELECT ID FROM EMRInfoT WHERE DataSubType = 1) \r\n "
		" AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
		" AND EMRMasterT.ID = @nEMNID AND \r\n "
		//TES 11/1/2011 - PLID 46225 - This is in a WHERE clause, and so it's going to run this formula on the entire column.  
		// Therefore, if we don't have the the "Convert(float," here, it will attempt to convert all numeric text data to an integer 
		// (to compare it against 1), so if you have any numeric non-integer text (such as "3.5") this will throw an exception.
		" (CASE WHEN IsNumeric(CONVERT(NVARCHAR(3000), EMRDetailTableDataT.Data) + 'e0') = 1 THEN \r\n "
		" 	Convert(float,EMRDetailTableDataT.Data)  ELSE 0 END) = 1 \r\n "	

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_VisitDate' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_WtType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Wt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_WtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Ht' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_HtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_HtType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Vit' AND Value <> ''; \r\n "		

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_SpGrpFq' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstHyp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstConHrt' AND Value <> ''; \r\n "

		/*"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstIsc' AND Value <> ''; \r\n "*/

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstAng' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstPevas' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstLoExd' AND Value <> ''; \r\n "

		/*"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstDvType' AND Value <> ''; \r\n "*/

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstGlu' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstLipD' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstGouh' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstObssyn' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstObHsyn' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstPulHyp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstAst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstGerd' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstCho' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstLvr' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstBkP' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstMusDis' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstFbMg' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstPlo' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstMenIrg' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstPsyImp' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstDep' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstConMen' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstAlc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstTob' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstSub' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstStu' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstPsc' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstAbHer' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstFunSt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_PstAdbS' AND Value <> ''; \r\n "

		"\r\n"
		"\r\n",
		nPatientID, nEMNID);

	BOOL bShowDialog = FALSE;
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;

	if (rsPat) {		

		if (!rsPat->eof) {
			pPostOpVisit->strVisitID = AdoFldString(rsPat, "VisitID", "");
		}
		rsPat = rsPat->NextRecordset(NULL);
		
		BOOL bAllowMultiples = FALSE;		

		if (rsPat) {
			
			while (rsPat) {

				BOLDCodeInfo *bldInfo = NULL; 

				ADODB::FieldsPtr flds = rsPat->Fields;
							
				CString strOldID = "";

				BOLDItem *bldItem = NULL;
				CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

				while (! rsPat->eof) {				

					if (bldInfo == NULL ) {
						bldInfo = new BOLDCodeInfo();
					}

					if (aryItems == NULL) {
						aryItems = new CArray<BOLDItem*,BOLDItem*>();
					}

					if (bldItem == NULL) {
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
					}

					CString strCode = AdoFldString(flds, "BOLDCode");
					CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
					long nItemID = AdoFldLong(flds, "ItemID");
					CString strItemName = AdoFldString(flds, "ItemName");
					CString strEMNName = AdoFldString(flds, "EMNName");
					COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
					CString strValue = AdoFldString(flds, "Value");				
					bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
					long nSelectID = AdoFldLong(flds, "SelectID");
					CString strSelectCode = AdoFldString(flds,"BoldDataCode");
					CString strCurrentID;				

					CString strRow;
					CString strSelection;

					strCurrentID = strCode + "-" + AsString(nItemID);

					bldInfo->strCode = strCode;
					bldInfo->strCodeDesc = strCodeDesc;
					bldInfo->bAllowMultiples = bAllowMultiples;					

					if (strOldID != strCurrentID) {

						//add one to our array
						if (strOldID != "") {						
							aryItems->Add(bldItem);
							bldItem = new BOLDItem();
							bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
							if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
								bShowDialog = TRUE;
							}
						}
						
						strOldID = strCurrentID;					
						bldItem->nItemID = nItemID;
						bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
					}

					
					BOLDDetail *pDetail = new BOLDDetail();
					pDetail->strSelection = strValue;
					pDetail->nSelectID = nSelectID;
					pDetail->strSelectCode = strSelectCode;
					bldItem->paryDetails->Add(pDetail);

					rsPat->MoveNext();
				}		

				//add the last value
				if (bldItem) {
					if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					aryItems->Add(bldItem);
				}

				if (aryItems) {
					if (aryItems->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					bldInfo->aryItems = aryItems;
				}

				//add it to the top most array
				if (bldInfo) {
					aryInfos.Add(bldInfo);
				}

				rsPat = rsPat->NextRecordset(NULL);
			}		
			
			//now we have to send everything to our dialog if necessary
			if (aryInfos.GetSize() != 0) {
				if (bShowDialog) {
					// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
					CBoldFieldPickDlg dlg(this);
					CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
					dlg.m_paryBoldList = &aryInfos;			
					dlg.m_pmapReturnValues = &mapValuesFromDialog;
					dlg.m_strPatientName = strPatientName;
					dlg.m_strVisitType = "Post Operative Visit";

					if (dlg.DoModal() == IDCANCEL) {
						FillPostOpVisit(nEMNID, pPostOpVisit, &aryInfos, NULL);
					}
					else {
						//we now should have an map of values			
						FillPostOpVisit(nEMNID, pPostOpVisit, &aryInfos, &mapValuesFromDialog);
					}
				}
				else {
					FillPostOpVisit(nEMNID, pPostOpVisit, &aryInfos, NULL);				
				}
			}

			//now clear out all our memory
			for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
				BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

				for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

					BOLDItem *pItem = pInfo->aryItems->GetAt(j);

					for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
						
						BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
						delete pDetail;
					}				
					delete pItem->paryDetails;
					delete pItem;
				}

				delete pInfo->aryItems;

				delete pInfo;
			}

		}
	}		
}

// (j.gruber 2010-06-01 11:06) - PLID 38337
void CBoldLinkDlg::SetErrorMessages(NXDATALIST2Lib::IRowSettingsPtr pRow, CStringArray *staryMessages)
{

	CString strMessage;
	for (int i = 0; i < staryMessages->GetSize(); i++) {
		strMessage += staryMessages->GetAt(i) + "\r\n";
	}

	pRow->PutValue(slcMessages, _variant_t(strMessage));
	
	//make the row red
	pRow->PutBackColor(RGB(255,0,0));
}


// (j.gruber 2010-06-01 11:06) - PLID 38955
void CBoldLinkDlg::FillGeneralVisit(long nEMNID, BOLDGeneralVisitInfo *pGenVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) 
{
	
	//ID
	//pGenVisit->strID = nEMNID;
	pGenVisit->dtVisit = _DATE(GetBOLDField("BOLD_VisitDate", aryInfo, mapValuesFromDialog));
	
	pGenVisit->bmuWeight.dblValue = _DBL(GetBOLDField("BOLD_Wt", aryInfo, mapValuesFromDialog));
	pGenVisit->bmuWeight.bEstimated = _BOOL(GetBOLDField("BOLD_WtEst", aryInfo, mapValuesFromDialog));
	pGenVisit->bmuWeight.bmuType = _BMU(GetBOLDField("BOLD_WtType", aryInfo, mapValuesFromDialog));

	pGenVisit->bmuHeight.dblValue = _DBL(GetBOLDField("BOLD_Ht", aryInfo, mapValuesFromDialog));
	pGenVisit->bmuHeight.bEstimated = _BOOL(GetBOLDField("BOLD_HtEst", aryInfo, mapValuesFromDialog));
	pGenVisit->bmuHeight.bmuType = _BMU(GetBOLDField("BOLD_HtType", aryInfo, mapValuesFromDialog));	
}

// (j.gruber 2010-06-01 11:07) - PLID 38955
void CBoldLinkDlg::FillGeneralVisitInfo(long nPatientID, long nEMNID, BOLDGeneralVisitInfo *pGenVisit, long nVisitType, CString strPatientName) 
{
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset(""
		"SET NOCOUNT ON; \r\n " 	
		"	DECLARE @nPatientID INT; \r\n "
		"	SET @nPatientID = {INT}; \r\n "
		"	DECLARE @nVisitType INT; \r\n "
		"	SET @nVisitType = {INT}; \r\n "
		"	DECLARE @nEMNID INT; \r\n "
		"	SET @nEMNID = {INT}; \r\n "

			"\r\n"		
			
			" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_VisitDate', 'BOLD_WtType', 'BOLD_Wt', 'BOLD_WtEst', 'BOLD_Ht', 'BOLD_HtType', 'BOLD_HtEst' "			
			") \r\n "

			" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "
		
			"	Open rsItems   \r\n "
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID  = @nEMNID \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "			
			"			AND EMRMasterT.ID  = @nEMNID \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems; \r\n "
			"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "

		"   SELECT Top 1 VisitID FROM BoldSentHistoryT WHERE EMNID = @nEMNID ORDER BY SentDate DESC; \r\n "
		
		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_VisitDate' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_WtType' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Wt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_WtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Ht' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_HtEst' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_HtType' AND Value <> ''; \r\n "

		"\r\n"
		"\r\n",
		nPatientID, nVisitType, nEMNID);

	BOOL bShowDialog = FALSE;
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;

	if (rsPat) {		

		if (!rsPat->eof) {
			pGenVisit->strVisitID = AdoFldString(rsPat, "VisitID", "");
		}
		rsPat = rsPat->NextRecordset(NULL);
		
		BOOL bAllowMultiples = FALSE;		

		if (rsPat) {
			
			while (rsPat) {

				BOLDCodeInfo *bldInfo = NULL; 

				ADODB::FieldsPtr flds = rsPat->Fields;
							
				CString strOldID = "";

				BOLDItem *bldItem = NULL;
				CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

				while (! rsPat->eof) {				

					if (bldInfo == NULL ) {
						bldInfo = new BOLDCodeInfo();
					}

					if (aryItems == NULL) {
						aryItems = new CArray<BOLDItem*,BOLDItem*>();
					}

					if (bldItem == NULL) {
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
					}

					CString strCode = AdoFldString(flds, "BOLDCode");
					CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
					long nItemID = AdoFldLong(flds, "ItemID");
					CString strItemName = AdoFldString(flds, "ItemName");
					CString strEMNName = AdoFldString(flds, "EMNName");
					COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
					CString strValue = AdoFldString(flds, "Value");				
					bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
					long nSelectID = AdoFldLong(flds, "SelectID");
					CString strSelectCode = AdoFldString(flds,"BoldDataCode");
					CString strCurrentID;				

					CString strRow;
					CString strSelection;

					strCurrentID = strCode + "-" + AsString(nItemID);

					bldInfo->strCode = strCode;
					bldInfo->strCodeDesc = strCodeDesc;
					bldInfo->bAllowMultiples = bAllowMultiples;					

					if (strOldID != strCurrentID) {

						//add one to our array
						if (strOldID != "") {						
							aryItems->Add(bldItem);
							bldItem = new BOLDItem();
							bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
							if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
								bShowDialog = TRUE;
							}
						}
						
						strOldID = strCurrentID;					
						bldItem->nItemID = nItemID;
						bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
					}

					
					BOLDDetail *pDetail = new BOLDDetail();
					pDetail->strSelection = strValue;
					pDetail->nSelectID = nSelectID;
					pDetail->strSelectCode = strSelectCode;
					bldItem->paryDetails->Add(pDetail);

					rsPat->MoveNext();
				}		

				//add the last value
				if (bldItem) {
					if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					aryItems->Add(bldItem);
				}

				if (aryItems) {
					if (aryItems->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					bldInfo->aryItems = aryItems;
				}

				//add it to the top most array
				if (bldInfo) {
					aryInfos.Add(bldInfo);
				}

				rsPat = rsPat->NextRecordset(NULL);
			}		
			
			//now we have to send everything to our dialog if necessary
			if (aryInfos.GetSize() != 0) {
				if (bShowDialog) {
					// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
					CBoldFieldPickDlg dlg(this);
					CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
					dlg.m_paryBoldList = &aryInfos;			
					dlg.m_pmapReturnValues = &mapValuesFromDialog;					
					dlg.m_strPatientName = strPatientName;
					if (nVisitType == bvtSelfReported) {
						dlg.m_strVisitType = "Self-Reported Visit";
					}
					else if (nVisitType == bvtWeightCheck) {
						dlg.m_strVisitType = "Weight Check Visit";
					}

					if (dlg.DoModal() == IDCANCEL) {
						FillGeneralVisit(nEMNID, pGenVisit, &aryInfos, NULL);
					}
					else {
						//we now should have an map of values			
						FillGeneralVisit(nEMNID, pGenVisit, &aryInfos, &mapValuesFromDialog);
					}
				}
				else {
					FillGeneralVisit(nEMNID, pGenVisit, &aryInfos, NULL);				
				}
			}

			//now clear out all our memory
			for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
				BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

				for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

					BOLDItem *pItem = pInfo->aryItems->GetAt(j);

					for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
						
						BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
						delete pDetail;
					}				
					delete pItem->paryDetails;
					delete pItem;
				}

				delete pInfo->aryItems;

				delete pInfo;
			}

		}
	}		
}

// (j.gruber 2010-06-01 11:07) - PLID 38955
void CBoldLinkDlg::FillOtherVisit(long nEMNID, BOLDOtherVisitInfo *pOthVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog) 
{	
	//ID
	//pOthVisit->strID = nEMNID;
	pOthVisit->dtVisit = _DATE(GetBOLDField("BOLD_VisitDate", aryInfo, mapValuesFromDialog));
	
}

// (j.gruber 2010-06-01 11:07) - PLID 38955
void CBoldLinkDlg::FillOtherVisitInfo(long nPatientID, long nEMNID, BOLDOtherVisitInfo *pOthVisit, CString strPatientName) 
{
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset(""
		"SET NOCOUNT ON; \r\n " 	
		"	DECLARE @nPatientID INT; \r\n "
		"	SET @nPatientID = {INT}; \r\n "
		"	DECLARE @nEMNID INT; \r\n "
		"	SET @nEMNID = {INT}; \r\n "
		
			"\r\n"		
			
			" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255)) \r\n "
			
			" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
			" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
			"	--bold codes \r\n "
			"	('BOLD_VisitDate' "			
			") \r\n "

			" DECLARE @nEMRInfoID INT; \r\n "
			" DECLARE @nDataType INT; \r\n "
			" DECLARE @strDataCode nvarChar(50); \r\n "
			" DECLARE @strEMRInfoName nVarChar(255); \r\n "
			" DECLARE @strCodeDesc nVarChar(255); \r\n "
		
			"	Open rsItems   \r\n "
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "
	
			"	/*get all the information we'll need for this patient */ \r\n "
			"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode    \r\n "
			"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
			"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
			"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
			"		ELSE BEGIN \r\n "
			"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode) \r\n "
			"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
			"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
			"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
			"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode	  \r\n "
			"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
			"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
			"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
			"			AND EMRMasterT.ID = @nEMNID \r\n "
			"		END \r\n "
	
			"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
			
			"	END \r\n "

			"	CLOSE rsItems; \r\n "
			"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "

		"   SELECT Top 1 VisitID FROM BoldSentHistoryT WHERE EMNID = @nEMNID ORDER BY SentDate DESC; \r\n "
		
		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_VisitDate' AND Value <> ''; \r\n "
		
		"\r\n"
		"\r\n",
		nPatientID, nEMNID);

	BOOL bShowDialog = FALSE;
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;

	if (rsPat) {		

		
		if (!rsPat->eof) {
			pOthVisit->strVisitID = AdoFldString(rsPat, "VisitID", "");
		}
		rsPat = rsPat->NextRecordset(NULL);
		
		BOOL bAllowMultiples = FALSE;		

		if (rsPat) {
			
			while (rsPat) {

				BOLDCodeInfo *bldInfo = NULL; 

				ADODB::FieldsPtr flds = rsPat->Fields;
							
				CString strOldID = "";

				BOLDItem *bldItem = NULL;
				CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

				while (! rsPat->eof) {				

					if (bldInfo == NULL ) {
						bldInfo = new BOLDCodeInfo();
					}

					if (aryItems == NULL) {
						aryItems = new CArray<BOLDItem*,BOLDItem*>();
					}

					if (bldItem == NULL) {
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
					}

					CString strCode = AdoFldString(flds, "BOLDCode");
					CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
					long nItemID = AdoFldLong(flds, "ItemID");
					CString strItemName = AdoFldString(flds, "ItemName");
					CString strEMNName = AdoFldString(flds, "EMNName");
					COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
					CString strValue = AdoFldString(flds, "Value");				
					bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
					long nSelectID = AdoFldLong(flds, "SelectID");
					CString strSelectCode = AdoFldString(flds,"BoldDataCode");
					CString strCurrentID;				

					CString strRow;
					CString strSelection;

					strCurrentID = strCode + "-" + AsString(nItemID);

					bldInfo->strCode = strCode;
					bldInfo->strCodeDesc = strCodeDesc;
					bldInfo->bAllowMultiples = bAllowMultiples;					

					if (strOldID != strCurrentID) {

						//add one to our array
						if (strOldID != "") {						
							aryItems->Add(bldItem);
							bldItem = new BOLDItem();
							bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
							if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
								bShowDialog = TRUE;
							}
						}
						
						strOldID = strCurrentID;					
						bldItem->nItemID = nItemID;
						bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
					}

					
					BOLDDetail *pDetail = new BOLDDetail();
					pDetail->strSelection = strValue;
					pDetail->nSelectID = nSelectID;
					pDetail->strSelectCode = strSelectCode;
					bldItem->paryDetails->Add(pDetail);

					rsPat->MoveNext();
				}		

				//add the last value
				if (bldItem) {
					if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					aryItems->Add(bldItem);
				}

				if (aryItems) {
					if (aryItems->GetSize() > 1) {
						bShowDialog = TRUE;
					}
					bldInfo->aryItems = aryItems;
				}

				//add it to the top most array
				if (bldInfo) {
					aryInfos.Add(bldInfo);
				}

				rsPat = rsPat->NextRecordset(NULL);
			}		
			
			//now we have to send everything to our dialog if necessary
			if (aryInfos.GetSize() != 0) {
				if (bShowDialog) {
					// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
					CBoldFieldPickDlg dlg(this);
					CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
					dlg.m_paryBoldList = &aryInfos;			
					dlg.m_pmapReturnValues = &mapValuesFromDialog;
					dlg.m_strPatientName = strPatientName;
					dlg.m_strVisitType = "Other Visit";

					if (dlg.DoModal() == IDCANCEL) {
						FillOtherVisit(nEMNID, pOthVisit, &aryInfos, NULL);
					}
					else {
						//we now should have an map of values			
						FillOtherVisit(nEMNID, pOthVisit, &aryInfos, &mapValuesFromDialog);
					}
				}
				else {
					FillOtherVisit(nEMNID, pOthVisit, &aryInfos, NULL);				
				}
			}

			//now clear out all our memory
			for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
				BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

				for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

					BOLDItem *pItem = pInfo->aryItems->GetAt(j);

					for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
						
						BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
						delete pDetail;
					}				
					delete pItem->paryDetails;
					delete pItem;
				}

				delete pInfo->aryItems;

				delete pInfo;
			}

		}
	}		
}

// (j.gruber 2010-06-01 11:07) - PLID 38954
void CBoldLinkDlg::FillPostOpAEVisit(long nEMNID, BOLDPostOpAEVisitInfo *pPOAEVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog)
{

	pPOAEVisit->strCode = GetBOLDField("BOLD_AdvEv", aryInfo, mapValuesFromDialog);
	pPOAEVisit->dtEvent = _DATE(GetBOLDField("BOLD_AEDt", aryInfo, mapValuesFromDialog));	
	
	pPOAEVisit->strFacilityID = GetBOLDField("BOLD_Fac", aryInfo, mapValuesFromDialog);
	pPOAEVisit->strSurgeonID = GetBOLDField("BOLD_Surgeon", aryInfo, mapValuesFromDialog);

	BuildStringArray(&pPOAEVisit->strarySurgeryCodes, "BOLD_AESurg", aryInfo, mapValuesFromDialog);
}

// (j.gruber 2010-06-01 11:07) - PLID 38954
void CBoldLinkDlg::FillPostOpAEVisitInfo(long nPatientID, long nEMNID, BOLDPostOpAEVisitInfo *pPOAEVisit, CString strPatientName)
{

	
	ADODB::_RecordsetPtr rsPat = CreateParamRecordset(""
		"SET NOCOUNT ON; \r\n " 	
		"	DECLARE @nPatientID INT; \r\n "
		"	SET @nPatientID = {INT}; \r\n "
		"	DECLARE @nEMNID INT; \r\n "
		"	SET @nEMNID = {INT}; \r\n "

		"\r\n"		
			
		" DECLARE @tResults TABLE (BOLDCode nVarChar(50), CodeDesc nVarChar(255), ItemID int, ItemName nVarChar(255), EMNName nVarChar(255), EMNDate dateTime, Value nVarChar(255), SelectID int,  BOLDDataCode nVarChar(255), EMNID int) \r\n "
		
		" DECLARE rsItems CURSOR LOCAL FORWARD_ONLY READ_ONLY FOR \r\n "
		" SELECT EMRInfoT.ID, Code, DataType, EMRInfoT.Name, EMRDataCodesT.Description as CodeDesc FROM EMRInfoT LEFT JOIN EMRDataCodesT ON EMRInfoT.DataCodeID = EMRDataCodesT.ID WHERE EMRDataCodesT.Code IN \r\n "
		"	--bold codes \r\n "
		"	('BOLD_AdvEv', 'BOLD_Fac', 'BOLD_Surgeon', 'BOLD_AESurg', 'BOLD_AEDt' \r\n"		
		") \r\n "

		" DECLARE @nEMRInfoID INT; \r\n "
		" DECLARE @nDataType INT; \r\n "
		" DECLARE @strDataCode nvarChar(50); \r\n "
		" DECLARE @strEMRInfoName nVarChar(255); \r\n "
		" DECLARE @strCodeDesc nVarChar(255); \r\n "
	
		"	Open rsItems   \r\n "
		"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
		"	WHILE @@FETCH_STATUS = 0 BEGIN \r\n "

		"	/*get all the information we'll need for this patient */ \r\n "
		"		IF @nDataType = 2 OR @nDataType = 3 BEGIN /*multi-select and single-select*/ \r\n "
		"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BOLDDataCode, EMNID) \r\n "
		"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName, EMRDataT.Data, EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, EMRSelectT.ID as SelectID, EMRDataT.BoldCode as BoldDataCode, EMRMasterT.ID as EMNID    \r\n "
		"			FROM EMRDataT LEFT JOIN EMRSelectT ON EMRDataT.ID = EMRSelectT.EMRDataID  \r\n "
		"			LEFT JOIN EMRDetailsT ON EMRSelectT.EMRDetailID = EMRDetailsT.ID  \r\n "
		"			LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
		"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDataT.EMRInfoID = @nEMRInfoID \r\n "
		"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
		"			AND EMRMasterT.ID = @nEMNID \r\n "
		"		END \r\n "
		"		ELSE BEGIN \r\n "
		"			INSERT INTO @tResults (BOLDCode, CodeDesc, ItemName, Value, EMNDate, EMNName, ItemID, SelectID, BoldDataCode, EMNID) \r\n "
		"			SELECT @strDataCode as DataCode, @strCodeDesc as CodeDesc, @strEMRInfoName as ItemName,  \r\n "
		"			CASE WHEN @nDataType = 1 then EMRDetailsT.Text \r\n "
		"			ELSE CASE WHEN @nDataType = 5 then Convert(nVarChar, EmrDetailsT.SliderValue) END END as Value, \r\n "
		"			EMRMasterT.Date as EMNDate, EMRMasterT.Description as EMRName, EMRDetailsT.ID as ItemID, -2 as SelectID, '' as BoldDataCode, EMRMasterT.ID AS EMNID	  \r\n "
		"			FROM EMRDetailsT LEFT JOIN EMRMasterT ON EMRDetailsT.EMRID = EMRMasterT.ID \r\n "
		"			WHERE EMRMasterT.PatientID = @nPatientID AND EMRDetailsT.EMRInfoID = @nEMRInfoID \r\n "
		"			AND EMRMasterT.Deleted = 0 and EMRDetailsT.Deleted = 0 \r\n "
		"			AND EMRMasterT.ID = @nEMNID \r\n "
		"		END \r\n "

		"	FETCH FROM rsItems INTO @nEMRInfoID, @strDataCode, @nDataType, @strEMRInfoName, @strCodeDesc \r\n "
		
		"	END \r\n "

		"	CLOSE rsItems; \r\n "
		"	DEALLOCATE rsItems; \r\n "
		"	SET NOCOUNT OFF; \r\n "

		"   SELECT Top 1 VisitID FROM BoldSentHistoryT WHERE EMNID = @nEMNID ORDER BY SentDate DESC; \r\n "
		
		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_AdvEv' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Surgeon' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_Fac' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 0) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_AEDt' AND Value <> ''; \r\n "

		"   SELECT TResults.*, CONVERT(bit, 1) AS AllowMultiples \r\n "
		"	FROM @tResults TResults WHERE BoldCode = 'BOLD_AESurg' AND Value <> ''; \r\n "

		"\r\n"
		"\r\n",
		nPatientID, nEMNID);

	BOOL bShowDialog = FALSE;
	CArray<BOLDCodeInfo*, BOLDCodeInfo*> aryInfos;

	if (rsPat) {		

		if (!rsPat->eof) {
			pPOAEVisit->strVisitID = AdoFldString(rsPat, "VisitID", "");
		}
		rsPat = rsPat->NextRecordset(NULL);
		
		BOOL bAllowMultiples = FALSE;				
		
		while (rsPat) {

			BOLDCodeInfo *bldInfo = NULL; 

			ADODB::FieldsPtr flds = rsPat->Fields;
						
			CString strOldID = "";

			BOLDItem *bldItem = NULL;
			CArray<BOLDItem*,BOLDItem*> *aryItems = NULL; 			

			while (! rsPat->eof) {				

				if (bldInfo == NULL ) {
					bldInfo = new BOLDCodeInfo();
				}

				if (aryItems == NULL) {
					aryItems = new CArray<BOLDItem*,BOLDItem*>();
				}

				if (bldItem == NULL) {
					bldItem = new BOLDItem();
					bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
				}

				CString strCode = AdoFldString(flds, "BOLDCode");
				CString strCodeDesc = AdoFldString(flds, "CodeDesc", "");				
				long nItemID = AdoFldLong(flds, "ItemID");
				CString strItemName = AdoFldString(flds, "ItemName");
				CString strEMNName = AdoFldString(flds, "EMNName");
				COleDateTime dtEMN = AdoFldDateTime(flds, "EMNDate");
				CString strValue = AdoFldString(flds, "Value");				
				bAllowMultiples = AdoFldBool(flds, "AllowMultiples");					
				long nSelectID = AdoFldLong(flds, "SelectID");
				CString strSelectCode = AdoFldString(flds,"BoldDataCode");
				CString strCurrentID;				

				CString strRow;
				CString strSelection;

				strCurrentID = strCode + "-" + AsString(nItemID);

				bldInfo->strCode = strCode;
				bldInfo->strCodeDesc = strCodeDesc;
				bldInfo->bAllowMultiples = bAllowMultiples;					

				if (strOldID != strCurrentID) {

					//add one to our array
					if (strOldID != "") {						
						aryItems->Add(bldItem);
						bldItem = new BOLDItem();
						bldItem->paryDetails = new CArray<BOLDDetail*, BOLDDetail*>();
						if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
							bShowDialog = TRUE;
						}
					}
					
					strOldID = strCurrentID;					
					bldItem->nItemID = nItemID;
					bldItem->strRow.Format("EMN Date: %s EMN Name: %s Item Name: %s", FormatDateTimeForInterface(dtEMN), strEMNName, strItemName);
				}

				
				BOLDDetail *pDetail = new BOLDDetail();
				pDetail->strSelection = strValue;
				pDetail->nSelectID = nSelectID;
				pDetail->strSelectCode = strSelectCode;
				bldItem->paryDetails->Add(pDetail);

				rsPat->MoveNext();
			}		

			//add the last value
			if (bldItem) {
				if ((!bAllowMultiples) && bldItem->paryDetails->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				aryItems->Add(bldItem);
			}

			if (aryItems) {
				if (aryItems->GetSize() > 1) {
					bShowDialog = TRUE;
				}
				bldInfo->aryItems = aryItems;
			}

			//add it to the top most array
			if (bldInfo) {
				aryInfos.Add(bldInfo);
			}

			rsPat = rsPat->NextRecordset(NULL);
		}		
		
		//now we have to send everything to our dialog if necessary
		if (aryInfos.GetSize() != 0) {
			if (bShowDialog) {
				// (j.gruber 2010-06-04 17:04) - PLID 38538 - dialog to pick duplicates
				CBoldFieldPickDlg dlg(this);
				CMap<CString, LPCTSTR, CString, LPCTSTR> mapValuesFromDialog;
				dlg.m_paryBoldList = &aryInfos;			
				dlg.m_pmapReturnValues = &mapValuesFromDialog;
				dlg.m_strPatientName = strPatientName;
				dlg.m_strVisitType = "Post Operative Adverse Event";

				if (dlg.DoModal() == IDCANCEL) {
					FillPostOpAEVisit(nEMNID, pPOAEVisit, &aryInfos, NULL);
				}
				else {
					//we now should have an map of values			
					FillPostOpAEVisit(nEMNID, pPOAEVisit, &aryInfos, &mapValuesFromDialog);
				}
			}
			else {
				FillPostOpAEVisit(nEMNID, pPOAEVisit, &aryInfos, NULL);				
			}
		}

		//now clear out all our memory
		for (int i = aryInfos.GetSize()-1; i >= 0; i--) {
			BOLDCodeInfo *pInfo = aryInfos.GetAt(i);

			for (int j = pInfo->aryItems->GetSize() - 1; j >= 0; j--) {

				BOLDItem *pItem = pInfo->aryItems->GetAt(j);

				for (int k = pItem->paryDetails->GetSize() - 1; k >= 0; k--) {
					
					BOLDDetail *pDetail = pItem->paryDetails->GetAt(k);
					delete pDetail;
				}				
				delete pItem->paryDetails;
				delete pItem;
			}

			delete pInfo->aryItems;

			delete pInfo;
		}		
	}		

}

// (j.gruber 2010-06-01 11:07) - PLID 38337
void CBoldLinkDlg::SaveVisit(NXDATALIST2Lib::IRowSettingsPtr pRow, long nEMNID, long nPatientID, long nBoldTypeID, CString strVisitID)
{
	ExecuteParamSql(""		
		" DECLARE @nNewID INT; \r\n "
		" SET @nNewID = (SELECT COALESCE(Max(ID), 0) + 1 FROM BoldSentHistoryT); \r\n "
		" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n "
		" INSERT INTO BoldSentHistoryT (ID, PatientID, EMNID, SentDate, BoldTypeID, VisitID) \r\n "
		" VALUES (@nNewID, {INT}, {INT}, getDate(), {INT}, {STRING})  \r\n ",
		nPatientID, nEMNID, nBoldTypeID, strVisitID);


		pRow->PutBackColor(RGB(0,255,0));
		//clear any errors
		pRow->PutValue(slcMessages, g_cvarNull);
		//uncheck it
		pRow->PutValue(slcSend, g_cvarFalse);
		//put the date
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		pRow->PutValue(slcSentDate, _variant_t(FormatDateTimeForInterface(dtNow)));
}

// (j.gruber 2010-06-01 11:08) - PLID 38337
BOOL CBoldLinkDlg::FailedMedicationCodesOnly(CStringArray *arystr){
	
	for(int i = 0; i < arystr->GetSize(); i++) {

		CString str = arystr->GetAt(i);

		//yup, the error its spelled wrong
		if (str.Find("The Medication  with Code") == -1 || str.Find("does not exists") == -1) {					  
			return FALSE;
		}
	}

	return TRUE;
}

// (j.gruber 2010-06-01 11:08) - PLID 38337
void CBoldLinkDlg::RefillMedicationCodes(CStringArray *arystrMedicationCodes, CStringArray *arystrMessages) {

	for (int i = 0; i < arystrMessages->GetSize(); i++) {
		CString strMessage = arystrMessages->GetAt(i);

		BOOL bFound = FALSE;
		for (int j = 0; j < arystrMedicationCodes->GetSize(); j++) {
			CString strMedCode = arystrMedicationCodes->GetAt(j);

			if (strMessage.Find(strMedCode) != -1) {

				//we found this medication code in this message
				//replace this medication code with 0's
				arystrMedicationCodes->RemoveAt(j);

				arystrMedicationCodes->InsertAt(j, "0000000000");

				bFound = TRUE;
			}		
		}
		ASSERT(bFound);
	}
}

// (j.gruber 2010-06-01 11:08) - PLID 38337
void CBoldLinkDlg::OnBnClickedBoldSend()
{
	try {
		CWaitCursor wait;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSearchList->GetFirstRow();

		//get the username and password
		CString strUserName, strPassword;
		GetDlgItemText(IDC_BOLD_USERNAME, strUserName);
		GetDlgItemText(IDC_BOLD_PASSWORD, strPassword);

		/*if (strUserName.IsEmpty() || strPassword.IsEmpty()) {
			MessageBox("Please enter a username and password for connecting to BOLD");
			return;
		}*/

		//SetRemotePropertyText("BOLDUsername", strUserName, 0, "<None>");

		//let's make sure that the rows aren't selected so that they can see the color change
		m_pSearchList->PutCurSel(NULL);
		
		while (pRow) {

			long nPatientID = VarLong(pRow->GetValue(slcPatientID));
			long nUserDefinedID = VarLong(pRow->GetValue(slcUserDefinedID));
			CString strPatientName = VarString(pRow->GetValue(slcNameSort), "");
			
			//is it checked?
			if (VarBool(pRow->GetValue(slcSend), FALSE) /*&& !IsAlreadySent(pRow)*/) {

				//we have to send it
				//we know we are on a parent row, so generate the patient Information
				BOLDInsurance *pIns = new BOLDInsurance();
				BOLDPatientVisitInfo *pPatVisit = new BOLDPatientVisitInfo();
				pPatVisit->pboldPatIns = pIns;				
				long nEMNID = VarLong(pRow->GetValue(slcEMNID));
				FillPatientVisitInfo(nPatientID, nEMNID, pPatVisit, strPatientName);					
					CStringArray aryMessages;
					if (ValidatePatientVisit(pPatVisit, &aryMessages)) {
						//if the patient doesn't already exist
						// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
						if (SendBoldPatient(nUserDefinedID, pPatVisit, &aryMessages)) {
							//update the data with our visitID
							SaveVisit(pRow, nEMNID, nPatientID, bvtPatient, "");							
						}
						else {
							SetErrorMessages(pRow, &aryMessages);
						}
					}
					else {
						SetErrorMessages(pRow, &aryMessages);
					}
				if (pPatVisit) {
					pPatVisit->pboldPatIns = NULL;

					if (pPatVisit->prevBariatricSurgeries.GetSize() > 0) {
						for (int i = pPatVisit->prevBariatricSurgeries.GetSize() - 1; i >= 0; i--) {
							BOLDPrevBarSurg *pBS = ((BOLDPrevBarSurg*)pPatVisit->prevBariatricSurgeries.GetAt(i));
							pPatVisit->prevBariatricSurgeries.RemoveAt(i);
							pBS->straryAdverseEventCodes.RemoveAll();						

							delete pBS;
						}
					}
					//DeletePatVisitContents(pPatVisit);											
					delete pPatVisit;
				}				
				if (pIns) {
					delete pIns;
				}
			}

			//now get this rows children
			NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
			while (pChildRow) {

				if (VarBool(pChildRow->GetValue(slcSend), FALSE) /*&& !IsAlreadySent(nEMNID)*/) {

					//see what type it is
					long nVisitType = VarLong(pChildRow->GetValue(slcBoldTypeID));
					long nEMNID = VarLong(pChildRow->GetValue(slcEMNID));
					switch (nVisitType) {
						case bvtPreOp:  //PreOp							
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDPreOpVisitInfo *pPreOpVisit = new BOLDPreOpVisitInfo();
								FillPreOpVisitInfo(nPatientID, nEMNID, pPreOpVisit, strPatientName);								
								CStringArray aryMessages;
								CString strVisitID;								
								if (ValidatePreOpVisit(pPreOpVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldPreOpVisit(nUserDefinedID, pPreOpVisit, strVisitID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtPreOp, strVisitID);										
									}
									else {	
										//check to see if we only failed because of medication codes, and if so, then resend with 0's instead of the failed codes
										if (FailedMedicationCodesOnly(&aryMessages)) {
											RefillMedicationCodes(&pPreOpVisit->straryMedications, &aryMessages);
											// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
											if (SendBoldPreOpVisit(nUserDefinedID, pPreOpVisit, strVisitID, &aryMessages)) {
												SaveVisit(pChildRow, nEMNID, nPatientID, bvtPreOp, strVisitID);
											}
											else {
												//wha??
												SetErrorMessages(pChildRow, &aryMessages);
											}
										}
										else {
											SetErrorMessages(pChildRow, &aryMessages);
										}
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}

								//update the row
								//delete the preop Visit Structure
								if (pPreOpVisit) {
									pPreOpVisit->straryMedications.RemoveAll();
									pPreOpVisit->straryVitamins.RemoveAll();
									delete pPreOpVisit;
								}
							}

						break;
						case bvtHospital:  //Hopital Visit
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDHospitalVisit *pHospVisit = new BOLDHospitalVisit();
								FillHospVisitInfo(nPatientID, nEMNID, pHospVisit, strPatientName);
								CStringArray aryMessages;
								CString strHospitalID;
								if (ValidateHospitalVisit(pHospVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldHospVisit(nUserDefinedID, pHospVisit, strHospitalID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtHospital, strHospitalID);										
									}
									else {																			
										SetErrorMessages(pChildRow, &aryMessages);									
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}									

								//update the row
								//delete the preop Visit Structure
								if (pHospVisit) {
									pHospVisit->straryConcurrentProcs.RemoveAll();
									pHospVisit->straryDVTTherapies.RemoveAll();
									pHospVisit->straryIntraOpAdverseEvents.RemoveAll();

									
									for (int i = pHospVisit->aryAdverseEventsBeforeDischarge.GetSize() - 1; i >= 0; i--) {
										BOLDAdverseEvents *pAE = ((BOLDAdverseEvents*)pHospVisit->aryAdverseEventsBeforeDischarge.GetAt(i));
										pHospVisit->aryAdverseEventsBeforeDischarge.RemoveAt(i);
										pAE->strarySurgeryCodes.RemoveAll();

										delete pAE;
									}

									
									delete pHospVisit;
								}
							}
						break;
						case bvtPostOp:  //PostOp
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDPostOpVisitInfo *pPostOpVisit = new BOLDPostOpVisitInfo();
								FillPostOpVisitInfo(nPatientID, nEMNID, pPostOpVisit, strPatientName);
								CStringArray aryMessages;
								CString strVisitID;								
								if (ValidatePostOpVisit(pPostOpVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldPostOpVisit(nUserDefinedID, pPostOpVisit, strVisitID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtPostOp, strVisitID);										
									}
									else {
										if (FailedMedicationCodesOnly(&aryMessages)) {
											RefillMedicationCodes(&pPostOpVisit->straryMedications, &aryMessages);

											//try and resend
											// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
											if (SendBoldPostOpVisit(nUserDefinedID, pPostOpVisit, strVisitID, &aryMessages)) {
												//update the data with our visitID
												SaveVisit(pChildRow, nEMNID, nPatientID, bvtPostOp, strVisitID);										
											}
											else {
												SetErrorMessages(pChildRow, &aryMessages);
											}
										}
										else {
											SetErrorMessages(pChildRow, &aryMessages);
										}
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}

								//update the row
								//delete the preop Visit Structure
								if (pPostOpVisit) {
									pPostOpVisit->straryMedications.RemoveAll();
									pPostOpVisit->straryVitamins.RemoveAll();
									delete pPostOpVisit;
								}
							}
						break;
						case bvtPostOpAE:  //PostOp Adverse Event
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDPostOpAEVisitInfo *pPOAEVisit = new BOLDPostOpAEVisitInfo();
								FillPostOpAEVisitInfo(nPatientID, nEMNID, pPOAEVisit, strPatientName);
								CStringArray aryMessages;
								CString strVisitID;
								if (ValidatePostOpAEVisit(pPOAEVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldPostOpAdverseEventVisit(nUserDefinedID, pPOAEVisit, strVisitID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtPostOpAE, strVisitID);										
									}
									else {									
										SetErrorMessages(pChildRow, &aryMessages);
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}
								
								//delete the preop Visit Structure
								if (pPOAEVisit) {
									pPOAEVisit->strarySurgeryCodes.RemoveAll();
									delete pPOAEVisit;
								}
							}
						break;
						case bvtSelfReported:  //Self REported
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDGeneralVisitInfo *pGenVisit = new BOLDGeneralVisitInfo();
								FillGeneralVisitInfo(nPatientID, nEMNID, pGenVisit, bvtSelfReported, strPatientName);
								CStringArray aryMessages;
								CString strVisitID;
								if (ValidateGeneralVisit(pGenVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldGeneralVisit("SelfReportedVisit", "SaveSelfReportedVisit", nUserDefinedID, pGenVisit, strVisitID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtSelfReported, strVisitID);									
									}
									else {																		
										SetErrorMessages(pChildRow, &aryMessages);
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}

								//delete the preop Visit Structure
								if (pGenVisit) {									
									delete pGenVisit;
								}
							}
						break;
						case bvtWeightCheck:  //Weight Check
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDGeneralVisitInfo *pGenVisit = new BOLDGeneralVisitInfo();
								FillGeneralVisitInfo(nPatientID, nEMNID, pGenVisit, bvtWeightCheck, strPatientName);
								CStringArray aryMessages;
								CString strVisitID;
								if (ValidateGeneralVisit(pGenVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldGeneralVisit("WeightCheckOnlyVisit", "SaveWeightCheckOnlyVisit", nUserDefinedID, pGenVisit, strVisitID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtWeightCheck, strVisitID);										
									}
									else {																	
										SetErrorMessages(pChildRow, &aryMessages);
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}

								//delete the preop Visit Structure
								if (pGenVisit) {									
									delete pGenVisit;
								}
							}
						break;
						case bvtOther:  //Other 
							if (CheckPriorVisitTypes(pChildRow)) {
								BOLDOtherVisitInfo *pOthVisit = new BOLDOtherVisitInfo();
								FillOtherVisitInfo(nPatientID, nEMNID, pOthVisit, strPatientName);
								CStringArray aryMessages;
								CString strVisitID;
								if (ValidateOtherVisit(pOthVisit, &aryMessages)) {
									// (j.gruber 2012-09-14 11:22) - PLID 52652 - take out password param
									if (SendBoldOtherVisit(nUserDefinedID, pOthVisit, strVisitID, &aryMessages)) {
										//update the data with our visitID
										SaveVisit(pChildRow, nEMNID, nPatientID, bvtOther, strVisitID);										
									}
									else {																	
										SetErrorMessages(pChildRow, &aryMessages);
									}									
								}
								else {
									SetErrorMessages(pChildRow, &aryMessages);
								}

								//delete the preop Visit Structure
								if (pOthVisit) {									
									delete pOthVisit;
								}
							}
						break;
					}
				}
				pChildRow = pChildRow->GetNextRow();
			}			
			pRow = pRow->GetNextRow();		
		}			

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-01 11:08) - PLID 38337
BOOL CBoldLinkDlg::CheckPriorVisitTypes(NXDATALIST2Lib::IRowSettingsPtr pRow){

	if (pRow) {

		long nBoldTypeID = (BoldVisitTypes)VarLong(pRow->GetValue(slcBoldTypeID));
		long nPatientID = VarLong(pRow->GetValue(slcPatientID));
		//BoldVisitTypes bvtPriorSent;
		CString strType;

		// (j.gruber 2012-01-06 14:13) - PLID 47355 - use an array
		CArray<BoldVisitTypes, BoldVisitTypes> arybvtTypes;

		switch (nBoldTypeID) {

			
			case bvtPatient:
				return TRUE;
			break;
			
			case bvtPreOp:
				//there has to be a patient visit				
				arybvtTypes.Add(bvtPatient);
				strType = "Patient Visit";
			break;
			
			case bvtHospital:				
				arybvtTypes.Add(bvtPreOp);
				strType = "Preop Visit";
			break;

			case bvtPostOp:				
				arybvtTypes.Add(bvtHospital);
				strType = "Hospital Visit";
			break;

			case bvtPostOpAE:		
				// (j.gruber 2012-01-06 14:13) - PLID 47355 - can be hopital visit or post op visit
				arybvtTypes.Add(bvtPostOp);
				arybvtTypes.Add(bvtHospital);
				strType = "Hospital Visit or Post Op Visit";
			break;

			case bvtSelfReported:
			case bvtWeightCheck:
			case bvtOther:
				arybvtTypes.Add(bvtHospital);				
				strType = "Hospital Visit";
			break;

			default:
				//can't send anything but the patient visit without the patient visit				
				arybvtTypes.Add(bvtPatient);
				strType = "Patient Visit";
			break;
		}

		ADODB::_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM BoldSentHistoryT WHERE PatientID = {INT} AND BoldTypeID IN ({INTARRAY})",
			nPatientID, arybvtTypes);
		if (rsCheck->eof) {
			CString strMessage;
			strMessage.Format("This message cannot be sent until a %s is sent for this patient", strType);
			pRow->PutValue(slcMessages, _variant_t(strMessage));
			pRow->PutBackColor(RGB(255,0,0));
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
	return FALSE;
}


BEGIN_EVENTSINK_MAP(CBoldLinkDlg, CNxDialog)
	ON_EVENT(CBoldLinkDlg, IDC_BOLD_SEND_LIST, 19, CBoldLinkDlg::LeftClickBoldSendList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (j.gruber 2010-06-01 11:47) - PLID 38337
void CBoldLinkDlg::LeftClickBoldSendList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		if (nCol == slcMessages) {
			
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {
				
				_variant_t varValue = pRow->GetValue(slcMessages);
				if (varValue.vt == VT_BSTR) {
					//they clicked on the messages, so pop them up in a readable format
					CString strMessages = VarString(varValue);
					MessageBox(strMessages);
				}
			}
		}


	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2010-06-01 11:47) - PLID 38211
BOOL CBoldLinkDlg::CheckDate(COleDateTime dt, CStringArray *aryMessages, CString strField) {
	//make sure the dates are greater than 1900
	COleDateTime dtMin;
	dtMin.SetDate(1900,1,1);
	CString strTest = FormatDateTimeForInterface(dt);
	if (dt.GetStatus() == COleDateTime::valid && dt >= dtMin) {
		return TRUE;
	}
	else {
		CString strMessage;
		strMessage.Format("Please enter a valid %s", strField);
		aryMessages->Add(strMessage);		
		return FALSE;
	}
}

// (j.gruber 2010-06-01 11:48) - PLID 38211
BOOL CBoldLinkDlg::CheckBool(BOLDBOOL bBool, CStringArray *aryMessages, CString strField) {
	if (bBool == bTrue || bBool == bFalse) {
		return TRUE;
	}else if (bBool == bNone) {
		CString strMessage;
		strMessage.Format("Please enter a value for %s", strField);
		aryMessages->Add(strMessage);		
		return FALSE;
	}
	else {
		//the recordset must of been eof, so it never got filled
		CString strMessage;
		strMessage.Format("Please enter a value for %s", strField);
		aryMessages->Add(strMessage);		
		return FALSE;
	}

}

// (j.gruber 2010-06-01 11:48) - PLID 38211
BOOL CBoldLinkDlg::CheckBMU(BOLDMetricUnitType bmut, CStringArray *aryMessages, CString strField) {
	if (bmut == bmutMetric || bmut == bmutStandard) {
		return TRUE;
	}else {
		CString strMessage;
		strMessage.Format("Please enter a value for %s", strField);
		aryMessages->Add(strMessage);		
		return FALSE;
	}	
}
// (j.gruber 2010-06-01 11:48) - PLID 38211
BOOL CBoldLinkDlg::CheckString(CString str, CStringArray *aryMessages, CString strField) {

	str.TrimLeft();
	str.TrimRight();

	if (str.IsEmpty()) {
		CString strMessage;
		strMessage.Format("Please enter a value for %s", strField);
		aryMessages->Add(strMessage);		
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (j.gruber 2010-06-01 11:48) - PLID 38211
BOOL CBoldLinkDlg::CheckDouble(double dbl, CStringArray *aryMessages, CString strField) {

	//returns 1 if dblValue > 0 (double-wise)
	if (LooseCompareDouble(dbl, 0.0, 0.0000001) != 1) {	
		CString strMessage;
		strMessage.Format("Please enter a positive value for %s", strField);
		aryMessages->Add(strMessage);		
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (j.gruber 2010-06-01 11:48) - PLID 38211
BOOL CBoldLinkDlg::CheckSurgeon(CString str, CStringArray *aryMessages, BoldVisitTypes bvType, CString strField){	

	str.TrimLeft();
	str.TrimRight();

	if (bvType == bvtHospital) {
		if (str.IsEmpty() || str.MakeUpper() == "OTHER") {
			CString strMessage;
			strMessage.Format("Please enter a valid value for %s", strField);
			aryMessages->Add(strMessage);		
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
	else {
		if (str.IsEmpty()) {
			CString strMessage;
			strMessage.Format("Please enter a value for %s", strField);
			aryMessages->Add(strMessage);		
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
}

// (j.gruber 2010-06-01 11:48) - PLID 38211
BOOL CBoldLinkDlg::CheckFacility(CString str, CStringArray *aryMessages, BoldVisitTypes bvType, CString strField){

	if (bvType == bvtHospital) {
		if (str.IsEmpty() || str.MakeUpper() == "OTHER") {
			CString strMessage;
			strMessage.Format("Please enter a valid value for %s", strField);
			aryMessages->Add(strMessage);		
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
	else {
		if (str.IsEmpty()) {
			CString strMessage;
			strMessage.Format("Please enter a value for %s", strField);
			aryMessages->Add(strMessage);		
			return FALSE;
		}
		else {
			return TRUE;
		}
	}

}

// (j.gruber 2010-06-01 11:48) - PLID 38949
BOOL CBoldLinkDlg::ValidatePatientVisit(BOLDPatientVisitInfo *pPatVisit, CStringArray *paryMessages){

	BOOL bReturn = TRUE;

	
	if (pPatVisit->nYearOfBirth <= 0) {
		paryMessages->Add("Please enter a valid Birthdate");
		bReturn = FALSE;
	}	

	if (! CheckString(pPatVisit->strGenderCode, paryMessages, "Gender")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPatVisit->strEmploymentCode, paryMessages, "Employment")) {
		bReturn = FALSE;
	}

	/*if (! CheckString(pPatVisit->strStateCode, paryMessages, "State")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPatVisit->strCountryCode, paryMessages, "Country")) {
		bReturn = FALSE;
	}*/
		
	if (!CheckBool(pPatVisit->bConsentReceived, paryMessages, "Consent Received")) {
		bReturn = FALSE;
	}

	if (pPatVisit->straryRaceCodes.GetSize() == 0) {
		paryMessages->Add("Please enter a value for Race");
		bReturn = FALSE;
	}

	if (pPatVisit->pboldPatIns->straryPaymentCodes.GetSize() == 0) {
		paryMessages->Add("Please enter a value for Insurance Payment Code");
		bReturn = FALSE;
	}

	for (int i = 0; i < pPatVisit->prevBariatricSurgeries.GetSize(); i++) {
		BOLDPrevBarSurg *pSurg  = ((BOLDPrevBarSurg*) pPatVisit->prevBariatricSurgeries.GetAt(i));

		if (pSurg) {

			if (! CheckString(pSurg->strCode, paryMessages, "Previous Bariatric Surgery Code")) {
				bReturn = FALSE;
			}

			if (!CheckDouble(pSurg->bmuOriginalWt.dblValue, paryMessages, "Previous Bariatric Surgery Original Weight Value")) {
				bReturn = FALSE;
			}

			if (!CheckBool(pSurg->bmuOriginalWt.bEstimated, paryMessages, "Previous Bariatric Surgery Original Weight Estimated")) {
				bReturn = FALSE;
			}

			if (!CheckBMU(pSurg->bmuOriginalWt.bmuType, paryMessages, "Previous Bariatric Surgery Original Weight Type")) {
				bReturn = FALSE;
			}

			if (!CheckDouble(pSurg->bmuLowestWt.dblValue, paryMessages, "Previous Bariatric Surgery Lowest Weight Value")) {
				bReturn = FALSE;
			}

			if (!CheckBool(pSurg->bmuLowestWt.bEstimated, paryMessages, "Previous Bariatric Surgery Lowest Weight Estimated")) {
				bReturn = FALSE;
			}

			if (!CheckBMU(pSurg->bmuLowestWt.bmuType, paryMessages, "Previous Bariatric Surgery Lowest Weight Type")) {
				bReturn = FALSE;
			}

			if (pSurg->nYear <= 0) {
				paryMessages->Add("Please enter a valid Previous Bariatric Surgery Year");
				bReturn = FALSE;
			}

			if (!CheckSurgeon(pSurg->strSurgeonID, paryMessages, bvtPrevBar, "Previous Bariatric Surgery Surgeon")) {
				bReturn = FALSE;
			}
		}

	}

	//if there is an insurance weight, make sure the estimated and standard are filled in too
	if (pPatVisit->pboldPatIns) {
		BOLDMetricUnit bmuWt = pPatVisit->pboldPatIns->bmuWeightLossAmt;
		//returns 1 if dblValue > 0 (double-wise)
		if (LooseCompareDouble(bmuWt.dblValue, 0.0, 0.0000001) == 1) {
			//check that they entered type and estimated
			if (!CheckBMU(bmuWt.bmuType, paryMessages, "Insurance Weight Loss Amount Type")) {
				bReturn = FALSE;
			}

			if (!CheckBool(bmuWt.bEstimated, paryMessages, "Insurance Weight Loss Amount Estimated")) {
				bReturn = FALSE;
			}
		}			
	}

	return bReturn;
}

// (j.gruber 2010-06-01 11:48) - PLID 38950
BOOL CBoldLinkDlg::ValidatePreOpVisit(BOLDPreOpVisitInfo *pPreOpVisit, CStringArray *paryMessages){

	BOOL bReturn = TRUE;

	if (!CheckDouble(pPreOpVisit->bmuHeight.dblValue, paryMessages, "Height Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pPreOpVisit->bmuHeight.bEstimated, paryMessages, "Height Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pPreOpVisit->bmuHeight.bmuType, paryMessages, "Height Type")) {
		bReturn = FALSE;
	}

	if (!CheckDouble(pPreOpVisit->bmuWeight.dblValue, paryMessages, "Weight Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pPreOpVisit->bmuWeight.bEstimated, paryMessages, "Weight Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pPreOpVisit->bmuWeight.bmuType, paryMessages, "Weight Type")) {
		bReturn = FALSE;
	}

	if (!CheckDate(pPreOpVisit->dtVisitDate, paryMessages, "Visit Date")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strHYPERT, paryMessages, "Hypertension")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strCONGHF, paryMessages, "Congestive Heart Failure")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strISCHHD, paryMessages, "Ischemic Heart Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strDVTPE, paryMessages, "DVT/PE")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strANGASM, paryMessages, "Angina Assessment")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strPEVASD, paryMessages, "Peripheral Vascular Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strLOEXED, paryMessages, "Lower Extremity Edema")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strGLUMET, paryMessages, "Glucose Metabolism")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strLIPDYH, paryMessages, "Lipids (Dyslipidemia or Hyperlipidemia)")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strGOUHYP, paryMessages, "GOUT/Hyperuricemia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strOBSSYN, paryMessages, "Obstructive Sleep Apnea Syndrome")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strOBHSYN, paryMessages, "Obesity Hypoventilation Syndrome")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strPULHYP, paryMessages, "Pulmonary Hypertension")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strASTHMA, paryMessages, "Asthma")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strGERD, paryMessages, "GERD")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strCHOLEL, paryMessages, "Cholelithiasis")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strLVRDIS, paryMessages, "Liver Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strBCKPAIN, paryMessages, "Back Pain")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strMUSDIS, paryMessages, "Musculoskeletal Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strFBMGIA, paryMessages, "Fibromyalgia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strPLOVSYN, paryMessages, "Polycystic Ovarian Syndrome")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strMENIRG, paryMessages, "Menstrual Irregularities (not PCOS)")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strPSYIMP, paryMessages, "Psychosocial Impairment")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strDEPRSN, paryMessages, "Depression")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strCONMEN, paryMessages, "Confirmed Mental Health Diagnosis")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strALCUSE, paryMessages, "Alcohol Use")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strTOBUSE, paryMessages, "Tobacco Use")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strSUBUSE, paryMessages, "Substance Abuse")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strSTURIN, paryMessages, "Stress Urinary Incontinence")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strPSCRBR, paryMessages, "Pseudotumor Cerebri")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strADBHER, paryMessages, "Abdominal Hernia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strFUNSTAT, paryMessages, "Functional Status")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPreOpVisit->strABDSKN, paryMessages, "Abdominal Skin/Pannus")) {
		bReturn = FALSE;
	}

	//check to see if the gender is male, then they must answer No history in MENIRG or PLOVSYN
//#pragma TODO("Do we want to check the data for this or let them handle it?")
	/*if (nGender == 1) {
		if (pPreOpVisit->strMENIRG != "SAA1188") {
			paryMessages->Add("If the patient is male, you must enter 'No history of menstrual irregularities'");
			bReturn = FALSE;
		}

		if (pPreOpVisit->strPLOVSYN != "SAA1181") {
			paryMessages->Add("If the patient is male, you must enter 'No history of polycystic ovarian syndrome'");
			bReturn = FALSE;
		}
	}*/

	//user may NOT select ‘No Impairment’ for the ‘Psychosocial Impairment’ question if the answer to the ‘Confirmed Mental Health Diagnosis’ question is anything but ‘None’
	if (!pPreOpVisit->strCONMEN.IsEmpty() && !pPreOpVisit->strPSYIMP.IsEmpty()) {
		if (pPreOpVisit->strCONMEN != "SAA1206") {
			if (pPreOpVisit->strPSYIMP == "SAA1194") {
				paryMessages->Add("You may not select 'No Impairment' for Psychosocial Impairment if the answer to Confirmed Mental Health Diagnosis is anything but 'None'");
				bReturn = FALSE;
			}
		}
	}

	//make sure we have no empty medication codes
	BOOL bContinue = TRUE;
	for (int i = 0; i < pPreOpVisit->straryMedications.GetSize() && bContinue; i++) {
		CString strItem = pPreOpVisit->straryMedications.GetAt(i);
		if (strItem.IsEmpty()) {
			//we failed
			paryMessages->Add("There is at least one medication code that could not be filled in.  This could be because your First Data Bank database is not ready to use, you are using a free text medication, or because Practice could not resolve the NDC for a particular medication.  Please review the medications and ensure that all medications are not free text, otherwise please call Technical Support.");
			bContinue = FALSE;
		}
	}

	return bReturn;
}

// (j.gruber 2010-06-01 11:49) - PLID 38951
BOOL CBoldLinkDlg::ValidateHospitalVisit(BOLDHospitalVisit *pHospVisit, CStringArray *paryMessages){

	BOOL bReturn = TRUE;

	if (!CheckDate(pHospVisit->dtSurgery, paryMessages, "Surgery Date")) {
		bReturn = FALSE;
	}

	if (!CheckDate(pHospVisit->dtAdmission, paryMessages, "Admission Date")) {
		bReturn = FALSE;
	}
	
	if (!CheckDate(pHospVisit->dtLastWeight, paryMessages, "Last Weight Date")) {
		bReturn = FALSE;
	}
	
	if (!CheckDouble(pHospVisit->bmuHeight.dblValue, paryMessages, "Height Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pHospVisit->bmuHeight.bEstimated, paryMessages, "Height Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pHospVisit->bmuHeight.bmuType, paryMessages, "Height Type")) {
		bReturn = FALSE;
	}

	if (!CheckDouble(pHospVisit->bmuLastWeightBeforeSurgery.dblValue, paryMessages, "Last Weight Before Surgery Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pHospVisit->bmuLastWeightBeforeSurgery.bEstimated, paryMessages, "Last Weight Before Surgery Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pHospVisit->bmuLastWeightBeforeSurgery.bmuType, paryMessages, "Last Weight Before Surgery Type")) {
		bReturn = FALSE;
	}	

	if (!CheckBool(pHospVisit->bRevision, paryMessages, "Revision")) {
		bReturn = FALSE;
	}

	if (! CheckFacility(pHospVisit->strFacilityID, paryMessages, bvtHospital, "Facility Code")) {
		bReturn = FALSE;
	}

	if (! CheckFacility(pHospVisit->strSurgeonID, paryMessages, bvtHospital, "Surgeon Code")) {
		bReturn = FALSE;
	}	

	if (! CheckDouble(pHospVisit->dblDurationSurgery, paryMessages, "Duration of Surgery")) {
		bReturn = FALSE;
	}

	if (! CheckDouble(pHospVisit->dblDurationAnesthesia, paryMessages, "Duration of Anesthesia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pHospVisit->sdblEstBloodLoss, paryMessages, "Estimated Blood Loss")) {
		bReturn = FALSE;
	}
	
	if (! CheckString(pHospVisit->sdblBloodTransfusionUnits, paryMessages, "Blood Transfusion Units")) {
		bReturn = FALSE;
	}	

	if(!CheckBool(pHospVisit->bSurgicalResidentParticipated, paryMessages, "Surgical Resident Participated")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pHospVisit->bSurgicalFellowParticipated, paryMessages, "Surgical Fellow Participated")) {
		bReturn = FALSE;
	}

	if (!CheckDate(pHospVisit->dtDischargeDate, paryMessages, "Discharge Date")) {
		bReturn = FALSE;
	}

	if (! CheckString(pHospVisit->strDischargeLocation, paryMessages, "Discharge Location")) {
		bReturn = FALSE;
	}		

	if (! CheckString(pHospVisit->strASAClassificationCode, paryMessages, "ASA Classification")) {
		bReturn = FALSE;
	}		

	if (! CheckString(pHospVisit->strBariatricProcedureCode, paryMessages, "Bariatric Procedure")) {
		bReturn = FALSE;
	}		

	if (! CheckString(pHospVisit->strBariatricTechniqueCode, paryMessages, "Bariatric Technique")) {
		bReturn = FALSE;
	}		

	//date of surgery must be equal to or after date of admission
	if (pHospVisit->dtAdmission.GetStatus() == COleDateTime::valid && pHospVisit->dtSurgery.GetStatus() == COleDateTime::valid) {
		if (pHospVisit->dtSurgery < pHospVisit->dtAdmission) {
			paryMessages->Add("The date of surgery must be equal to or after than the date of admission");
			bReturn = FALSE;
		}
	}

	//duration of anesthesia must be equal to or greater than duration of surgery
	if (pHospVisit->dblDurationAnesthesia < pHospVisit->dblDurationSurgery) {
		paryMessages->Add("The duration of anesthesia must be equal to or greater than the duration of surgery");
		bReturn = FALSE;
	}
	

	//discharge date must be greater than or equal to date of surgery
	if (pHospVisit->dtDischargeDate.GetStatus() == COleDateTime::valid && pHospVisit->dtSurgery.GetStatus() == COleDateTime::valid) {
		if (pHospVisit->dtDischargeDate < pHospVisit->dtSurgery) {
			paryMessages->Add("The date of discharge must be equal to or after the date of surgery");
			bReturn = FALSE;
		}
	}

	//date of last weight cannot be after hospital visit date
	//there is no hospital visit date, so I'm assuming surgery date
	if (pHospVisit->dtLastWeight.GetStatus() == COleDateTime::valid && pHospVisit->dtSurgery.GetStatus() == COleDateTime::valid) {
		if (pHospVisit->dtLastWeight > pHospVisit->dtSurgery) {
			paryMessages->Add("The date of last weight cannot be after the surgery date");
			bReturn = FALSE;
		}
	}

	for (int i = 0; i < pHospVisit->aryAdverseEventsBeforeDischarge.GetSize(); i++){
		BOLDAdverseEvents *pAE = ((BOLDAdverseEvents*)pHospVisit->aryAdverseEventsBeforeDischarge.GetAt(i));
		if (pAE) {			

			if (!CheckString(pAE->strCode, paryMessages, "Adverse Event Before Discharge Code")) {
				bReturn = FALSE;
			}			

			/*if (!CheckSurgeon(pAE->strSurgeonID, paryMessages, bvtPostOpAE, "Adverse Event Before Discharge Surgeon")) {
				bReturn = FALSE;
			}*/

			if (pAE->nTimeAfterSurgery <= 0) {
				paryMessages->Add("Please enter a valid value for Adverse Event Before Discharge Time After Surgery");
				bReturn = FALSE;
			}

			if (pAE->btuTimeAfterMeasurement == btuNone) {
				paryMessages->Add("Please enter a valid value for Adverse Event Before Discharge Time After Surgery Measurement");
				bReturn = FALSE;
			}		
		}
	}

	return bReturn;
}

// (j.gruber 2010-06-01 11:49) - PLID 38953
BOOL CBoldLinkDlg::ValidatePostOpVisit(BOLDPostOpVisitInfo *pPostOpVisit, CStringArray *paryMessages){

	BOOL bReturn = TRUE;

	if (!CheckDouble(pPostOpVisit->bmuHeight.dblValue, paryMessages, "Height Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pPostOpVisit->bmuHeight.bEstimated, paryMessages, "Height Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pPostOpVisit->bmuHeight.bmuType, paryMessages, "Height Type")) {
		bReturn = FALSE;
	}

	if (!CheckDouble(pPostOpVisit->bmuWeight.dblValue, paryMessages, "Weight Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pPostOpVisit->bmuWeight.bEstimated, paryMessages, "Weight Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pPostOpVisit->bmuWeight.bmuType, paryMessages, "Weight Type")) {
		bReturn = FALSE;
	}

	if (!CheckDate(pPostOpVisit->dtVisitDate, paryMessages, "Visit Date")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strSupportGroup, paryMessages, "Support Group Frequency")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strHYPERT, paryMessages, "Hypertension")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strCONGHF, paryMessages, "Congestive Heart Failure")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strANGASM, paryMessages, "Angina Assessment")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strPEVASD, paryMessages, "Peripheral Vascular Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strLOEXED, paryMessages, "Lower Extremity Edema")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strGLUMET, paryMessages, "Glucose Metabolism")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strLIPDYH, paryMessages, "Lipids (Dyslipidemia or Hyperlipidemia)")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strGOUHYP, paryMessages, "GOUT/Hyperuricemia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strOBSSYN, paryMessages, "Obstructive Sleep Apnea Syndrome")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strOBHSYN, paryMessages, "Obesity Hypoventilation Syndrome")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strPULHYP, paryMessages, "Pulmonary Hypertension")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strASTHMA, paryMessages, "Asthma")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strGERD, paryMessages, "GERD")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strCHOLEL, paryMessages, "Cholelithiasis")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strLVRDIS, paryMessages, "Liver Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strBCKPAIN, paryMessages, "Back Pain")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strMUSDIS, paryMessages, "Musculoskeletal Disease")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strFBMGIA, paryMessages, "Fibromyalgia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strPLOVSYN, paryMessages, "Polycystic Ovarian Syndrome")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strMENIRG, paryMessages, "Menstrual Irregularities (not PCOS)")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strPSYIMP, paryMessages, "Psychosocial Impairment")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strDEPRSN, paryMessages, "Depression")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strCONMEN, paryMessages, "Confirmed Mental Health Diagnosis")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strALCUSE, paryMessages, "Alcohol Use")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strTOBUSE, paryMessages, "Tobacco Use")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strSUBUSE, paryMessages, "Substance Abuse")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strSTURIN, paryMessages, "Stress Urinary Incontinence")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strPSCRBR, paryMessages, "Pseudotumor Cerebri")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strADBHER, paryMessages, "Abdominal Hernia")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strFUNSTAT, paryMessages, "Functional Status")) {
		bReturn = FALSE;
	}

	if (! CheckString(pPostOpVisit->strABDSKN, paryMessages, "Abdominal Skin/Pannus")) {
		bReturn = FALSE;
	}

	//check to see if the gender is male, then they must answer No history in MENIRG or PLOVSYN
//#pragma TODO("Check data for this or let them do it")
	/*if (nGender == 1) {
		if (pPostOpVisit->strMENIRG != "SAA1189") {
			paryMessages->Add("If the patient is male, you must enter 'No indication of menstrual irregularities'");
			bReturn = FALSE;
		}

		if (pPostOpVisit->strPLOVSYN != "SAA1182") {
			paryMessages->Add("If the patient is male, you must enter 'No indication of polycystic ovarian syndrome'");
			bReturn = FALSE;
		}
	}*/

	if (!pPostOpVisit->strCONMEN.IsEmpty() && !pPostOpVisit->strPSYIMP.IsEmpty()) {
		if (pPostOpVisit->strCONMEN != "SAA1206") {
			if (pPostOpVisit->strPSYIMP == "SAA1194") {
				paryMessages->Add("You may not select 'No Impairment' for Psychosocial Impairment if the answer to Confirmed Mental Health Diagnosis is anything but 'None'");
				bReturn = FALSE;
			}
		}
	}


	//make sure we have no empty medication codes
	BOOL bContinue = TRUE;
	for (int i = 0; i < pPostOpVisit->straryMedications.GetSize() && bContinue; i++) {
		CString strItem = pPostOpVisit->straryMedications.GetAt(i);
		if (strItem.IsEmpty()) {
			//we failed
			paryMessages->Add("There is at least one medication code that could not be filled in.  This could be because your First Data Bank database is not ready to use, you are using a free text medication, or because Practice could not resolve the NDC for a particular medication.  Please review the medications and ensure that all medications are not free text, otherwise please call Technical Support.");
			bContinue = FALSE;
		}
	}
	
	return bReturn;
}

// (j.gruber 2010-06-01 11:49) - PLID 38954
BOOL CBoldLinkDlg::ValidatePostOpAEVisit(BOLDPostOpAEVisitInfo *pPOAEVisit, CStringArray *paryMessages){

	BOOL bReturn = TRUE;
	
	if (!CheckDate(pPOAEVisit->dtEvent, paryMessages, "Adverse Event Date")) {
		bReturn = FALSE;
	}

	if (!CheckString(pPOAEVisit->strCode, paryMessages, "Adverse Event Code")) {
		bReturn = FALSE;
	}

	/*if (!CheckFacility(pPOAEVisit->strFacilityID, paryMessages, bvtPostOpAE, "Facility COEID")) {
		bReturn = FALSE;
	}*/

	/*if (!CheckSurgeon(pPOAEVisit->strSurgeonID, paryMessages, bvtPostOpAE, "Surgeon COEID")) {
		bReturn = FALSE;
	}*/
	
	return bReturn;

}

// (j.gruber 2010-06-01 11:49) - PLID 38955
BOOL CBoldLinkDlg::ValidateGeneralVisit(BOLDGeneralVisitInfo *pGenVisit, CStringArray *paryMessages){

	BOOL bReturn = TRUE;

	/*if (!CheckDouble(pGenVisit->bmuHeight.dblValue, paryMessages, "Height Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pGenVisit->bmuHeight.bEstimated, paryMessages, "Height Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pGenVisit->bmuHeight.bmuType, paryMessages, "Height Type")) {
		bReturn = FALSE;
	}

	if (!CheckDouble(pGenVisit->bmuWeight.dblValue, paryMessages, "Weight Value")) {
		bReturn = FALSE;
	}

	if (!CheckBool(pGenVisit->bmuWeight.bEstimated, paryMessages, "Weight Estimated")) {
		bReturn = FALSE;
	}

	if (!CheckBMU(pGenVisit->bmuWeight.bmuType, paryMessages, "Weight Type")) {
		bReturn = FALSE;
	}	*/

	if (!CheckDate(pGenVisit->dtVisit, paryMessages, "Visit Date")) {
		bReturn = FALSE;
	}
	
	return bReturn;

}

// (j.gruber 2010-06-01 11:49) - PLID 38955
BOOL CBoldLinkDlg::ValidateOtherVisit(BOLDOtherVisitInfo *pOthVisit, CStringArray *paryMessages){

	//all we need to check here is if the date is valid
	return CheckDate(pOthVisit->dtVisit, paryMessages, "Visit Date");		
}

// (j.gruber 2010-06-01 11:49) - PLID 38337
void CBoldLinkDlg::OnBnClickedBoldRdSent()
{
	try {
		if (IsDlgButtonChecked(IDC_BOLD_RD_SENT)) {
			m_rdBoldSent.SetCheck(1);
			m_rdBoldNotSent.SetCheck(0);
			m_rdBoldAll.SetCheck(0);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-01 11:49) - PLID 38337
void CBoldLinkDlg::OnBnClickedBoldRdNotSent()
{
	try {
		if (IsDlgButtonChecked(IDC_BOLD_RD_NOT_SENT)) {
			m_rdBoldSent.SetCheck(0);
			m_rdBoldNotSent.SetCheck(1);
			m_rdBoldAll.SetCheck(0);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-01 11:49) - PLID 38337
void CBoldLinkDlg::OnBnClickedBoldRdAll()
{
	try {
		if (IsDlgButtonChecked(IDC_BOLD_RD_ALL)) {
			m_rdBoldSent.SetCheck(0);
			m_rdBoldNotSent.SetCheck(0);
			m_rdBoldAll.SetCheck(1);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-01 11:49) - PLID 38337
void CBoldLinkDlg::OnBnClickedBoldFilterDates()
{
	try {
		if (IsDlgButtonChecked(IDC_BOLD_FILTER_DATES)) {
			
			//set the date fields
			m_dtFrom.EnableWindow(TRUE);
			m_dtTo.EnableWindow(TRUE);
			
			m_dtFrom.SetValue(COleDateTime::GetCurrentTime());
			m_dtTo.SetValue(COleDateTime::GetCurrentTime());
		}
		else {
			m_dtFrom.EnableWindow(FALSE);
			m_dtTo.EnableWindow(FALSE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-01 11:49) - PLID 38337
BOOL CBoldLinkDlg::EnsureValidSearchDateRange()
{
	if (IsDlgButtonChecked(IDC_BOLD_FILTER_DATES)) {
		COleDateTime dtFrom, dtTo;
		dtFrom = COleDateTime(m_dtFrom.GetValue());
		dtTo = COleDateTime(m_dtTo.GetValue());
		if(dtFrom.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("You have entered an invalid 'From' date. Please correct this date.");
			m_dtFrom.SetValue(_variant_t(dtTo));
			return FALSE;
		}
		else if(dtTo.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("You have entered an invalid 'To' date. Please correct this date.");
			m_dtTo.SetValue(_variant_t(dtFrom));
			return FALSE;
		}
		else {
			//if dtFrom > dtTo, update dtTo
			if(dtFrom > dtTo) {
				dtTo = dtFrom;
				AfxMessageBox("You have entered an 'From' date after the 'To' date. The 'To' Date has been set to the 'From' date value.");
				m_dtTo.SetValue(_variant_t(dtTo));
			}
		}
	}
	return TRUE;
}