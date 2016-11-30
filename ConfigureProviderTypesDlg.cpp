// ConfigureProviderTypesDlg.cpp : implementation file
//(r.wilson 4/22/2014) PLID 61826 - Made Dialog

#include "stdafx.h"
#include "Practice.h"
#include "ConfigureProviderTypesDlg.h"
#include "AuditTrail.h"


using namespace NXDATALIST2Lib;

using namespace ADODB;

// CConfigureProviderTypesDlg dialog

IMPLEMENT_DYNAMIC(CConfigureProviderTypesDlg, CNxDialog)

CConfigureProviderTypesDlg::CConfigureProviderTypesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureProviderTypesDlg::IDD, pParent)
{

}

CConfigureProviderTypesDlg::~CConfigureProviderTypesDlg()
{
}

void CConfigureProviderTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CB_AFF_PHYSICIAN_OPTICIAN,	m_cbAffPhy_Optician);
	DDX_Control(pDX, IDC_CB_REF_PROVIDER,	m_cbReferringProvider);
	DDX_Control(pDX, IDC_CB_ORD_PROVIDER,	m_cbOrderingProvider);
	DDX_Control(pDX, IDC_CB_SUP_PROVIDER,	m_cbSupervisingProvider);

	DDX_Control(pDX, IDC_BTN_SEL_ALL,	m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_SEL_ONE,	m_btnSelectOne);
	DDX_Control(pDX, IDC_BTN_UNSEL_ONE,	m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_UNSEL_ALL,	m_btnUnselectAll);

	DDX_Control(pDX, IDC_BTN_APPLY_CHANGES, m_btnApply);
	DDX_Control(pDX, IDC_BTN_CLOSE_CPT_DLG, m_btnClose);
	DDX_Control(pDX, IDC_BTN_CLEAR_TYPES, m_btnClearTypes);
	DDX_Control(pDX, IDC_CONFIG_PROVIDER_TYPES_INFO , m_icoConfigProvTypeInfo); //(r.wilson 4/22/2014) PLID 61828 - Map Question Mark Icon to static text control
}


BEGIN_MESSAGE_MAP(CConfigureProviderTypesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SEL_ONE, &CConfigureProviderTypesDlg::OnBnClickedBtnSelOne)
	ON_BN_CLICKED(IDC_BTN_SEL_ALL, &CConfigureProviderTypesDlg::OnBnClickedBtnSelAll)
	ON_BN_CLICKED(IDC_BTN_UNSEL_ONE, &CConfigureProviderTypesDlg::OnBnClickedBtnUnselOne)
	ON_BN_CLICKED(IDC_BTN_UNSEL_ALL, &CConfigureProviderTypesDlg::OnBnClickedBtnUnselAll)
	ON_BN_CLICKED(IDC_BTN_CLOSE_CPT_DLG, &CConfigureProviderTypesDlg::OnBnClickedBtnCloseCptDlg)
	ON_BN_CLICKED(IDC_BTN_CLEAR_TYPES, &CConfigureProviderTypesDlg::OnBnClickedBtnClearTypes)
	ON_BN_CLICKED(IDC_BTN_APPLY_CHANGES, &CConfigureProviderTypesDlg::OnBnClickedBtnApplyChanges)
END_MESSAGE_MAP()

BOOL CConfigureProviderTypesDlg::OnInitDialog()
{
	try{

		CNxDialog::OnInitDialog();
		m_DL_ProviderTypes = BindNxDataList2Ctrl(IDC_DL_PROVIDER_TYPE, false);
		m_DL_SelectedProviders = BindNxDataList2Ctrl( IDC_DL_SELECTED_PROVIDERS, false);
		m_DL_UnselectedProviders = BindNxDataList2Ctrl( IDC_DL_UNSELECTED_PROVIDERS, false);

		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);

		m_btnClearTypes.AutoSet(NXB_DELETE);
		m_btnApply.AutoSet(NXB_OK);
		m_btnClose.AutoSet(NXB_CLOSE);

		PopulateDataLists();

		//(r.wilson 4/22/2014) PLID 61828 - Set the Question mark hover text
		CreateAndSetToolTipText();
						
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}


//(r.wilson 4/22/2014) PLID 61826 - Move selected provider from unselected to the selected provider list
void CConfigureProviderTypesDlg::OnBnClickedBtnSelOne()
{
	try{
		//IRowSettingsPtr pRow = m_DL_UnselectedProviders->GetCurSel();		
		IRowSettingsPtr pRow = m_DL_UnselectedProviders->GetFirstSelRow();
		IRowSettingsPtr pRowNext;
		if(!pRow){
			return;
		}
		while (pRow)
		{
			pRowNext = pRow->GetNextSelRow();
			m_DL_UnselectedProviders->RemoveRow(pRow);
			m_DL_SelectedProviders->AddRowAtEnd(pRow, NULL);
			pRow = pRowNext;

		}

		m_DL_SelectedProviders->Sort();
	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/22/2014) PLID 61826
void CConfigureProviderTypesDlg::PopulateDataLists()
{
	//First Manually add the Provider and Referring Physician rows to the provider type drop down --------------------------
	IRowSettingsPtr pNewRow = m_DL_ProviderTypes->GetNewRow();
	pNewRow->PutValue(0,0);
	pNewRow->PutValue(1,"Provider");
	m_DL_ProviderTypes->AddRowAtEnd(pNewRow,NULL);
	pNewRow = m_DL_ProviderTypes->GetNewRow();
	pNewRow->PutValue(0,1);
	pNewRow->PutValue(1,"Referring Physician");
	m_DL_ProviderTypes->AddRowAtEnd(pNewRow,NULL);
	// ----------------------------------------------------------------------------------------------------------------------
	// Select column based on Selected contact is a provider or ref physician 
	IRowSettingsPtr pRow = m_DL_ProviderTypes->SearchByColumn(1, _bstr_t (strCurSelContactType), m_DL_ProviderTypes->GetFirstRow(), TRUE);
	m_nProvTypeCurentId = 0;
	
	// ------ 
	CWnd* pWndSel;
	CWnd* pWndUnsel;
	long nSelection = VarLong(pRow->GetValue(0));
	// We change the checkbox based on which provider type is selected
	//0 -> "Provider"   1 -> "Referring Physician"
	if (nSelection == 0)
	{
		m_cbAffPhy_Optician.SetWindowTextA("Optician");

		//Update the labels under the  selected/unselected datalists
		pWndSel = GetDlgItem(IDC_STATIC_CPT_UNSEL_PROVS);
		pWndSel->SetWindowTextA("Unselected Providers");
		pWndUnsel = GetDlgItem(IDC_STATIC_CPT_SEL_PROVS);
		pWndUnsel->SetWindowTextA("Selected Providers");
	}
	else if (nSelection == 1)
	{
		m_cbAffPhy_Optician.SetWindowTextA("Affiliate Physician");

		//Update the labels under the  selected/unselected datalists
		pWndSel = GetDlgItem(IDC_STATIC_CPT_UNSEL_PROVS);
		pWndSel->SetWindowTextA("Unselected Referring Physicians");
		pWndUnsel = GetDlgItem(IDC_STATIC_CPT_SEL_PROVS);
		pWndUnsel->SetWindowTextA("Selected Referring Physicians");
	}

	m_nProvTypeCurentId = VarLong(pRow->GetValue(0));

	//-------------------------------------------------------------------------


	//Populate the unselected datalist
	PopulateUnselectedDatalist();
	ClearSelectedProvidersDatalist();  // clear any selected providers out of that list
	HideOrShowOpticianCB();
}



void CConfigureProviderTypesDlg::UpdateSelectedDatalist(){



	_RecordsetPtr rs;
	std::map<long, ProvidersSetTypes>::iterator it;
	
	CString strSelect, strFrom, strWhere, strQuery;
	if (GetSelectedProviderType() == eptProvider){// (s.tullis 2014-05-28 14:35) - PLID 61826 - Need a mass configuration utility for configuring provider types. (Master Item)
		strSelect =
			" SELECT PersonID AS ID, Last + ', ' + First AS Name,  "
			" CASE WHEN ReferringProvider = 1 THEN 'Referring Provider' ELSE ''END AS ReferringProvider , "
			" CASE WHEN OrderingProvider = 1 THEN 'Ordering Provider'  ELSE '' END AS OrderingProvider,  "
			" CASE WHEN SupervisingProvider = 1 THEN 'Supervising Provider' ELSE '' END AS SupervisingProvider,  "
			" CASE WHEN Optician = 1 THEN 'Optician' ELSE '' END AS Optician, "
			" '' AS AffiliatePhysician ";
		strFrom = "FROM ProvidersT INNER JOIN PersonT ON PersonT.ID = ProvidersT.PersonID";

	}
	// (s.tullis 2014-05-28 12:45) - PLID 61827 - If Referring Physician then SELECT and FROM pull the following ..
	else if (GetSelectedProviderType() == eptReferringPhysician){
		strSelect =
			" SELECT PersonID AS ID, Last + ', ' + First AS Name,  "
			"CASE WHEN ReferringProvider = 1 THEN 'Referring Provider' ELSE ''END AS ReferringProvider , "
			"CASE WHEN OrderingProvider = 1 THEN 'Ordering Provider'  ELSE '' END AS OrderingProvider,  "
			"CASE WHEN SupervisingProvider = 1 THEN 'Supervising Provider' ELSE '' END AS SupervisingProvider,  "
			"CASE WHEN AffiliatePhysician = 1 THEN 'Affiliate Physician' ELSE '' END AS AffiliatePhysician,"
			" '' AS Optician ";
		strFrom = "FROM ReferringPhysT INNER JOIN dbo.PersonT ON PersonT.ID = ReferringPhysT.PersonID";
	}
	else{

		ASSERT(FALSE);
	}

	strWhere = " WHERE Archived = 0 AND PersonID IN ({ INTARRAY })";

	strQuery.Format("%s\r\n%s\r\n%s", strSelect, strFrom, strWhere);


	rs = CreateParamRecordset(strQuery, arrID);

	m_DL_SelectedProviders->Clear();

	while (!rs->eof)
	{
		std::list<CString> listProvTypes;
		CString strTypeCol, strRefProv, strOrderProv, strSuperProv, strOpt, strAffPhy;
		//listType.push_back()
		long nPersonId = VarLong(rs->Fields->Item["ID"]->Value);
		CString strName = VarString(rs->Fields->Item["Name"]->Value, "");
		//listProvTypes is a list of CStrings . If the column vaalue is not empty then it is put into the list. Used later to build type colum
		//
		strRefProv = AdoFldString(rs, "ReferringProvider");
		if (!strRefProv.IsEmpty()){ listProvTypes.push_back(strRefProv); }
		//
		strOrderProv = AdoFldString(rs, "OrderingProvider");
		if (!strOrderProv.IsEmpty()){ listProvTypes.push_back(strOrderProv); }
		//
		strSuperProv = AdoFldString(rs, "SupervisingProvider");
		if (!strSuperProv.IsEmpty()){ listProvTypes.push_back(strSuperProv); }
		//
		strOpt = AdoFldString(rs, "Optician");
		if (!strOpt.IsEmpty()){ listProvTypes.push_back(strOpt); }
		//
		strAffPhy = AdoFldString(rs, "AffiliatePhysician");
		if (!strAffPhy.IsEmpty()){ listProvTypes.push_back(strAffPhy); }

		BOOL bRefProv = (!strRefProv.IsEmpty()) ? TRUE : FALSE;
		BOOL bOrderingProv = (!strOrderProv.IsEmpty()) ? TRUE : FALSE;
		BOOL bSuperProv = (!strSuperProv.IsEmpty()) ? TRUE : FALSE;
		BOOL bOpt = (!strOpt.IsEmpty()) ? TRUE : FALSE;
		BOOL bAffPhy = (!strAffPhy.IsEmpty()) ? TRUE : FALSE;


		// (s.tullis 2014-05-28 12:46) - PLID 61826 - update the Provider list with the changed infomation
		if (GetSelectedProviderType() == eptProvider)
		{
			it = m_listOldProviderTypes.find(nPersonId);
			it->second.Optician = bOpt;
			it->second.ReferringProvider = bRefProv;
			it->second.OrderingProvider = bOrderingProv;
			it->second.SupervisingProvider = bSuperProv;


		}// (s.tullis 2014-05-28 12:45) - PLID 61827 -  Update the Ref Phys List with the Changed infomation
		else if (GetSelectedProviderType()==eptReferringPhysician)
		{
			it = m_listOldReferringPhysicianTypes.find(nPersonId);
			it->second.AffiliateProvider = bAffPhy;
			it->second.ReferringProvider = bRefProv;
			it->second.OrderingProvider = bOrderingProv;
			it->second.SupervisingProvider = bSuperProv;

		}
		else{
			ASSERT(FALSE);
		}
	

		//Go through our list and concat values with commas between them
		for (std::list<CString>::iterator it = listProvTypes.begin(); it != listProvTypes.end(); it++)
		{
			strTypeCol += *it;
			strTypeCol += ", ";
		}
		strTypeCol.TrimRight(", ");  // Trim right gets rid of the last trailing comma


		
		//Make a new row with values from the current DB returned row
		IRowSettingsPtr pNewRow = m_DL_SelectedProviders->GetNewRow();
		pNewRow->PutValue(0, nPersonId);
		pNewRow->PutValue(1, _variant_t(strName));
		pNewRow->PutValue(2, _variant_t(strTypeCol));

		//Add Row to unselected provider datalist
		m_DL_SelectedProviders->AddRowAtEnd(pNewRow, NULL);


		rs->MoveNext();

	}

	arrID.RemoveAll();
	m_DL_SelectedProviders->Sort();

	rs->Close();
	
}
//(r.wilson 4/22/2014) PLID 61826 - Put all valid providers or referring physicians into the unselected datalist based on the selected provider type
void CConfigureProviderTypesDlg::PopulateUnselectedDatalist()
{
	try{
		//Clear out lists
		m_listOldProviderTypes.clear();
		m_listOldReferringPhysicianTypes.clear();

		CString strSelect, strFrom, strWhere, strFinalQuery;
		m_DL_UnselectedProviders->Clear();

		// (s.tullis 2014-05-28 12:46) - PLID 61826 - If Provider then SELECT and FROM pull the following ..
		if (GetSelectedProviderType() == eptProvider){
			strSelect =
				" SELECT PersonID AS ID, Last + ', ' + First AS Name,  "
				" CASE WHEN ReferringProvider = 1 THEN 'Referring Provider' ELSE ''END AS ReferringProvider , "
				" CASE WHEN OrderingProvider = 1 THEN 'Ordering Provider'  ELSE '' END AS OrderingProvider,  "
				" CASE WHEN SupervisingProvider = 1 THEN 'Supervising Provider' ELSE '' END AS SupervisingProvider,  "
				" CASE WHEN Optician = 1 THEN 'Optician' ELSE '' END AS Optician, "
				" '' AS AffiliatePhysician ";
			strFrom = "FROM ProvidersT INNER JOIN PersonT ON PersonT.ID = ProvidersT.PersonID";

		}
		//// (s.tullis 2014-05-28 12:45) - PLID 61827 - If Referring Physician then SELECT and FROM pull the following ..
		else if (GetSelectedProviderType() == eptReferringPhysician){
			strSelect =
				" SELECT PersonID AS ID, Last + ', ' + First AS Name,  "
				"CASE WHEN ReferringProvider = 1 THEN 'Referring Provider' ELSE ''END AS ReferringProvider , "
				"CASE WHEN OrderingProvider = 1 THEN 'Ordering Provider'  ELSE '' END AS OrderingProvider,  "
				"CASE WHEN SupervisingProvider = 1 THEN 'Supervising Provider' ELSE '' END AS SupervisingProvider,  "
				"CASE WHEN AffiliatePhysician = 1 THEN 'Affiliate Physician' ELSE '' END AS AffiliatePhysician,"
				" '' AS Optician ";
			strFrom = "FROM ReferringPhysT INNER JOIN dbo.PersonT ON PersonT.ID = ReferringPhysT.PersonID";
		}
		else
		{
			ASSERT(FALSE);
		}

		// For no we want active providers
		strWhere = " WHERE Archived = 0";

		//Put all the query pieces together
		strFinalQuery.Format("%s\r\n%s\r\n%s", strSelect, strFrom, strWhere);


		//Run the query 
		ADODB::_RecordsetPtr rs = CreateRecordset(strFinalQuery);

		while (!rs->eof)
		{
			std::list<CString> listProvTypes;
			CString strTypeCol, strRefProv, strOrderProv, strSuperProv, strOpt, strAffPhy;
			//listType.push_back()
			long nPersonId = VarLong(rs->Fields->Item["ID"]->Value);
			CString strName = VarString(rs->Fields->Item["Name"]->Value, "");
			//listProvTypes is a list of CStrings . If the column vaalue is not empty then it is put into the list. Used later to build type colum
			//
			strRefProv = AdoFldString(rs, "ReferringProvider");
			if (!strRefProv.IsEmpty()){ listProvTypes.push_back(strRefProv); }
			//
			strOrderProv = AdoFldString(rs, "OrderingProvider");
			if (!strOrderProv.IsEmpty()){ listProvTypes.push_back(strOrderProv); }
			//
			strSuperProv = AdoFldString(rs, "SupervisingProvider");
			if (!strSuperProv.IsEmpty()){ listProvTypes.push_back(strSuperProv); }
			//
			strOpt = AdoFldString(rs, "Optician");
			if (!strOpt.IsEmpty()){ listProvTypes.push_back(strOpt); }
			//
			strAffPhy = AdoFldString(rs, "AffiliatePhysician");
			if (!strAffPhy.IsEmpty()){ listProvTypes.push_back(strAffPhy); }

			BOOL bRefProv = (!strRefProv.IsEmpty()) ? TRUE : FALSE;
			BOOL bOrderingProv = (!strOrderProv.IsEmpty()) ? TRUE : FALSE;
			BOOL bSuperProv = (!strSuperProv.IsEmpty()) ? TRUE : FALSE;
			BOOL bOpt = (!strOpt.IsEmpty()) ? TRUE : FALSE;
			BOOL bAffPhy = (!strAffPhy.IsEmpty()) ? TRUE : FALSE;

			//Keep track of this provider's types so when they change we can compare them
			ProvidersSetTypes provSetTypes(bOpt, bAffPhy, bRefProv, bOrderingProv, bSuperProv);
			if (GetSelectedProviderType() == eptProvider){// insert providers in the list
				m_listOldProviderTypes.insert(std::pair<long, ProvidersSetTypes>(nPersonId, provSetTypes));
			}
			else if (GetSelectedProviderType() == eptReferringPhysician){// (s.tullis 2014-05-28 15:09) - PLID 61827 - The mass configuration utility for configuring provider types needs to also support referring physicians.
				m_listOldReferringPhysicianTypes.insert(std::pair<long, ProvidersSetTypes>(nPersonId, provSetTypes));
			}
			else{

				ASSERT(FALSE);
				// should never happen
			}

			//Go through our list and concat values with commas between them
			for (std::list<CString>::iterator it = listProvTypes.begin(); it != listProvTypes.end(); it++)
			{
				strTypeCol += *it;
				strTypeCol += ", ";
			}
			strTypeCol.TrimRight(", ");  // Trim right gets rid of the last trailing comma

			//Make a new row with values from the current DB returned row
			IRowSettingsPtr pNewRow = m_DL_UnselectedProviders->GetNewRow();
			pNewRow->PutValue(0, nPersonId);
			pNewRow->PutValue(1, _variant_t(strName));
			pNewRow->PutValue(2, _variant_t(strTypeCol));

			//Add Row to unselected provider datalist
			m_DL_UnselectedProviders->AddRowAtEnd(pNewRow, NULL);


			rs->MoveNext();

		}

		m_DL_UnselectedProviders->Sort();

		rs->Close();
	}NxCatchAll(__FUNCTION__)

}

//(r.wilson 4/22/2014) PLID 61826 - Return an enum val of the currently selected provider type
CConfigureProviderTypesDlg::ProviderTypes CConfigureProviderTypesDlg::GetSelectedProviderType()
{
	
		IRowSettingsPtr pRow = m_DL_ProviderTypes->CurSel;
		CConfigureProviderTypesDlg::ProviderTypes eProvType =  (CConfigureProviderTypesDlg::ProviderTypes)VarLong(pRow->GetValue(0));



		return eProvType;
	
}

//(r.wilson 4/22/2014) PLID 61826 - Clear out selected providers datalist 
void CConfigureProviderTypesDlg::ClearSelectedProvidersDatalist()
{
	m_DL_SelectedProviders->Clear();
}

//(r.wilson 4/22/2014) PLID 61826 - Move all unselected providers to the selected datalist
void CConfigureProviderTypesDlg::OnBnClickedBtnSelAll()
{
	try{
		IRowSettingsPtr pRow = m_DL_UnselectedProviders->GetFirstRow();

		while(pRow)
		{
			IRowSettingsPtr pRowNext = pRow->GetNextRow();
			m_DL_UnselectedProviders->RemoveRow(pRow);
			m_DL_SelectedProviders->AddRowAtEnd(pRow, NULL);

			pRow = pRowNext;
		}

		m_DL_SelectedProviders->Sort();
	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/22/2014) PLID 61826 - Move selected provider from selected to unselected datalist
void CConfigureProviderTypesDlg::OnBnClickedBtnUnselOne()
{
	try{
		IRowSettingsPtr pRow = m_DL_SelectedProviders->GetFirstSelRow();
		IRowSettingsPtr pRowNext;		
		
		if(!pRow){
			return;
		}

		//m_DL_SelectedProviders->TakeRowAddSorted(pRow);

		
		while (pRow)
		{
			pRowNext = pRow->GetNextSelRow();
			//m_DL_SelectedProviders->RemoveRow(pRow);
			//m_DL_UnselectedProviders->AddRowAtEnd(pRow, NULL);			
			m_DL_UnselectedProviders->TakeRowAddSorted(pRow);
			pRow = pRowNext;
		}

		m_DL_UnselectedProviders->Sort();

	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/22/2014) PLID 61826 - Unselect All Providers (Move them to the left datalist)
void CConfigureProviderTypesDlg::OnBnClickedBtnUnselAll()
{
	try{
		IRowSettingsPtr pRow = m_DL_SelectedProviders->GetFirstRow();
		IRowSettingsPtr pRowNext;

		while(pRow)
		{
			pRowNext = pRow->GetNextRow();
			m_DL_SelectedProviders->RemoveRow(pRow);
			m_DL_UnselectedProviders->AddRowAtEnd(pRow, NULL);
			pRow = pRowNext;
		}
		m_DL_UnselectedProviders->Sort();
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CConfigureProviderTypesDlg, CNxDialog)
	ON_EVENT(CConfigureProviderTypesDlg, IDC_DL_PROVIDER_TYPE, 16, CConfigureProviderTypesDlg::SelChosenDlProviderType, VTS_DISPATCH)
//	ON_EVENT(CConfigureProviderTypesDlg, IDC_DL_PROVIDER_TYPE, 2, CConfigureProviderTypesDlg::SelChangedDlProviderType, VTS_DISPATCH VTS_DISPATCH)
ON_EVENT(CConfigureProviderTypesDlg, IDC_DL_UNSELECTED_PROVIDERS, 3, CConfigureProviderTypesDlg::DblClickCellDlUnselectedProviders, VTS_DISPATCH VTS_I2)
ON_EVENT(CConfigureProviderTypesDlg, IDC_DL_SELECTED_PROVIDERS, 3, CConfigureProviderTypesDlg::DblClickCellDlSelectedProviders, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

//(r.wilson 4/22/2014) PLID 61826 - When a new provider type is chosen
void CConfigureProviderTypesDlg::SelChosenDlProviderType(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		if(!pRow){
			return;
		}

		//Make sure that they actually want to change the selection
		long nselectedRowCount = m_DL_SelectedProviders->GetRowCount();

		if (nselectedRowCount > 0) // We only care if there are selected providers
		{
			CString message;
			message.Format("You have %s(s) in the selected list. Changing this option will clear this list. Are you sure you want to continue?", strCurSelContactType);
			//(r.wilson 4/22/2014) PLID 61826 - Verify that the user wants to chane the provider type (without saving first)
			if (MessageBox(message, "NexTech Practice", MB_YESNO) == IDNO)
			{				
				CString strSeachValue;
				strSeachValue.Format("%li", m_nProvTypeCurentId);
				m_DL_ProviderTypes->SearchByColumn(0, strSeachValue.AllocSysString(), m_DL_ProviderTypes->GetFirstRow(), TRUE);
				return;
			}
		}

		CWnd* pWndSel;
		CWnd* pWndUnsel;
		long nSelection = VarLong(pRow->GetValue(0));
		// We change the checkbox based on which provider type is selected
		//0 -> "Provider"   1 -> "Referring Physician"
		if(nSelection == 0)
		{
			m_cbAffPhy_Optician.SetWindowTextA("Optician");
			
			//Update the labels under the  selected/unselected datalists
			pWndSel = GetDlgItem(IDC_STATIC_CPT_UNSEL_PROVS);
			pWndSel->SetWindowTextA("Unselected Providers");
			pWndUnsel = GetDlgItem(IDC_STATIC_CPT_SEL_PROVS);
			pWndUnsel->SetWindowTextA("Selected Providers");
			strCurSelContactType = "Provider";
		}
		else if(nSelection == 1)
		{
			m_cbAffPhy_Optician.SetWindowTextA("Affiliate Physician");

			//Update the labels under the  selected/unselected datalists
			pWndSel = GetDlgItem(IDC_STATIC_CPT_UNSEL_PROVS);
			pWndSel->SetWindowTextA("Unselected Referring Physicians");
			pWndUnsel = GetDlgItem(IDC_STATIC_CPT_SEL_PROVS);
			pWndUnsel->SetWindowTextA("Selected Referring Physicians");
			strCurSelContactType = "Referring Physician";
		}
		else{

			ASSERT(FALSE);
		}

		m_nProvTypeCurentId = VarLong(pRow->GetValue(0));

		UncheckAllBoxes();
		ClearSelectedProvidersDatalist();
		PopulateUnselectedDatalist();
		HideOrShowOpticianCB();
		CreateAndSetToolTipText();
		
		
	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/22/2014) PLID 61826 -  Function to uncheck all checkbox on the screen
void CConfigureProviderTypesDlg::UncheckAllBoxes()
{
	m_cbAffPhy_Optician.SetCheck(FALSE);
	m_cbReferringProvider.SetCheck(FALSE);
	m_cbOrderingProvider.SetCheck(FALSE);
	m_cbSupervisingProvider.SetCheck(FALSE);
}

//(r.wilson 4/22/2014) PLID 61826 - This creates the sql to commit changes to the db. IT executes them as well
void CConfigureProviderTypesDlg::CreateUpdateSQL(BOOL bupdate)
{
	try{

		CString strSqlBatch = "";
		CNxParamSqlArray aryParams;
		
		
		//Get the values from all the dialog check boxes
		BOOL bAff_Opt = m_cbAffPhy_Optician.GetCheck();
		BOOL bRefPro = m_cbReferringProvider.GetCheck();
		BOOL bOrderingProv = m_cbOrderingProvider.GetCheck();
		BOOL bSupProv = m_cbSupervisingProvider.GetCheck();
		
		//Start at the top of the selected providers datalist
		IRowSettingsPtr pRow = m_DL_SelectedProviders->GetFirstRow();

		if(!pRow){
			// Need to show a message box telling the user that they have nothing selected..
			MessageBox("Please select one or more providers to apply these types to first.", "NexTech Practice");
			return;
		}

		//Detetmine which table we will update 
		CString strTable, strCol, strColValue;
		if(GetSelectedProviderType() == eptProvider){
			// If updating the provider table then the optician bit will be the column that gets updated
			strTable = "ProvidersT";
			strCol = "Optician";
			strColValue = "Optician";
		}
		else if(GetSelectedProviderType() == eptReferringPhysician){
			//// (s.tullis 2014-05-28 12:45) - PLID 61827 -  If updating the Referring Physicians table then the Affiliate Physician bit will be the column that gets updated
			strTable = "ReferringPhysT";
			strCol = "AffiliatePhysician";
			strColValue = "AffiliatePhysician";
		}
		else
		{
			//error 
			// Shouldn't be anything else but if so then we need to abort
			ASSERT(FALSE);
			return;
		}
		
		
		//Loop through all the rows in the Selected Providers Datalist
		while(pRow)
		{		
			long nPersonID = VarLong(pRow->GetValue(0));
		
			arrID.Add(nPersonID);
			pRow = pRow->GetNextRow();
		}
		
	
		
		if (bupdate){//if applying types
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE {CONST_STRING} SET {CONST_STRING} = CASE WHEN  {INT}=1 THEN 1 ELSE {CONST_STRING} END, ReferringProvider = CASE WHEN {INT}=1 THEN 1 ELSE ReferringProvider END , OrderingProvider = CASE WHEN {INT}=1 THEN 1 ELSE OrderingProvider END , SupervisingProvider  = CASE WHEN {INT}=1 THEN 1 ELSE SupervisingProvider END WHERE PersonID IN ({INTARRAY}); ", strTable, strCol, bAff_Opt, strColValue, bRefPro, bOrderingProv, bSupProv, arrID);
		}
		else if (!bupdate){//else if clearing types
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE {CONST_STRING} SET {CONST_STRING} =0 , ReferringProvider = 0 , OrderingProvider =  0, SupervisingProvider  =0  WHERE PersonID IN ({INTARRAY}); ", strTable, strCol,  arrID);
		}
		
			if (!strSqlBatch.IsEmpty()) {
				ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
				AuditChanges(bupdate);
				UpdateSelectedDatalist();
			}


			
		
	}NxCatchAll(__FUNCTION__)
	//Show Popup -Updated blah blah
}
void CConfigureProviderTypesDlg::OnBnClickedBtnCloseCptDlg()
{
	// TODO: Add your control notification handler code here
	try{
		long nProvidersSelected = m_DL_SelectedProviders->GetRowCount();		
		CNxDialog::OnOK();	
	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/22/2014) PLID 61826 - Clear Provider types from selected providers
void CConfigureProviderTypesDlg::OnBnClickedBtnClearTypes()
{
	try{
		CString message;
	

			message.Format("You have selected to clear all the Provider Type(s) for the selected %s(s). Are you sure you want to do this ?",  strCurSelContactType);
		//(r.wilson 4/22/2014) PLID 61826 - Verify that they want to clear the types first
		if (MessageBox(message, "NexTech Practice", MB_YESNO) == IDNO)
		{			
			return;
		}
		
		/// (s.tullis 2014-05-28 14:45) - PLID 61826 - Uncheck Boxes, Save, Audit Changes, Update Select List 
		UncheckAllBoxes();
		CreateUpdateSQL(FALSE);
		
		
		

		
	}NxCatchAll(__FUNCTION__);
}

void CConfigureProviderTypesDlg::UpdateSelectBox(){

	try{



	}NxCatchAll(__FUNCTION__);
}

//(r.wilson 4/22/2014) PLID 61826 - Apply Changes
void CConfigureProviderTypesDlg::OnBnClickedBtnApplyChanges()
{
	try{

		CString message="";
		message.Format("Are you sure you want to the checked Provider types to all selected %s(s)?",  strCurSelContactType);
		
		if(MessageBox(message, "NexTech Practice", MB_YESNO) == IDYES)
		{
			/// (s.tullis 2014-05-28 14:45) - PLID 61826 - Save, Audit , Update Select List
			CreateUpdateSQL(TRUE);
			
		}
	}NxCatchAll(__FUNCTION__);
}




//(r.wilson 4/22/2014) PLID 61826
//void CConfigureProviderTypesDlg::SelChangedDlProviderType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
//{	
//	try
//	{
//		/*long nselectedRowCount = m_DL_SelectedProviders->GetRowCount();
//		
//		if (nselectedRowCount > 0) // We only care if there are selected providers
//		{
//			//(r.wilson 4/22/2014) PLID 61826 - Verify that the user wants to chane the provider type (without saving first)
//			if (MessageBox("You have providers in the selected list. Changing this option will clear this list. Are you sure you want to continue?", "You sure you want to change?", MB_YESNO) == IDNO)
//			{
//				IRowSettingsPtr pRowOld(lpOldSel);
//				m_DL_ProviderTypes->CurSel = pRowOld;
//			}
//		}*/
//	}NxCatchAll(__FUNCTION__);
//}

//(r.wilson 4/22/2014) PLID 61826 -
void CConfigureProviderTypesDlg::HideOrShowOpticianCB()
{
	CWnd* pWnd = GetDlgItem(IDC_CB_AFF_PHYSICIAN_OPTICIAN);
	//IDC_CB_AFF_PHYSICIAN_OPTICIAN
	if (GetSelectedProviderType() == eptProvider)
	{
		
		if (g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
			pWnd->ShowWindow(SW_SHOW);
		}
		else{
			pWnd->ShowWindow(SW_HIDE);
		}
	}
	else if (GetSelectedProviderType() == eptReferringPhysician)
	{
		pWnd->ShowWindow(SW_SHOW);
	}
}

// (j.jones 2014-05-20 12:19) - PLID 61828 - dynamically create the ? tooltip text
// based on the provider/referring physician selection
void CConfigureProviderTypesDlg::CreateAndSetToolTipText()
{
	CString strToolTipTextTopHalf, strToolTipTextAll, strOpticianText, strAffiliatePhyText;
	strOpticianText = "- Optician - Provider responsible for placing glasses orders. \r\n" ;
	if (!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
		strOpticianText == "";
	}
	
	if (GetSelectedProviderType() == eptProvider) {
		strToolTipTextTopHalf = "Configure Provider Types\r\n"
		"1. Select a Provider Type, and use the arrows to move Providers \r\n"
		"    from the Unselected list to the Selected list. \r\n"
		"2. Check off Provider Types under the Apply As section. \r\n"
		"3. Press Apply to configure the Selected Providers with the checked \r\n"
		"    Provider Types. \r\n\r\n"
		"Provider Types: \r\n";
	}
	else if (GetSelectedProviderType() == eptReferringPhysician) {
		strToolTipTextTopHalf = "Configure Referring Physician Types\r\n"
		"1. Select a Provider Type, and use the arrows to move Referring Physicians \r\n"
		"    from the Unselected list to the Selected list. \r\n"
		"2. Check off Referring Physician Types under the Apply As section. \r\n"
		"3. Press Apply to configure the Selected Referring Physicians with the \r\n"
		"    checked Referring Physician Types. \r\n\r\n"
		"Referring Physician Types: \r\n";
	}
	else {
		ASSERT(FALSE);
	}

	strAffiliatePhyText = "- Affiliate Physician - Alternate Physician responsible for providing pre-op \r\n   and post-op services.\r\n";

	strToolTipTextAll.Format("%s"
		"%s"		
		"- Referring Provider - Referring Provider on a charge, not claim. Used in \r\n   HCFA box 17 with a DN qualifier, and ANSI Loop 2420F.\r\n"
		"- Ordering Provider - Ordering provider on a charge. Used in HCFA box 17  \r\n   with a DK qualifier, and ANSI Loop 2420E.\r\n"
		"- Supervising Provider - Supervising Provider on a charge. Used in HCFA \r\n   box 17 with a DQ qualifier, and ANSI Loop 2420D.\r\n",
		strToolTipTextTopHalf,
		GetSelectedProviderType() == eptProvider ? strOpticianText : strAffiliatePhyText);

	//(r.wilson 4/22/2014) PLID 61828 - Set the Question mark hover text
	m_icoConfigProvTypeInfo.LoadToolTipIcon(
		AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK),
		strToolTipTextAll,
		true, true, false);
}


void CConfigureProviderTypesDlg::AuditChanges(BOOL bupdate)
{
	
	long nAuditTransactionID = -1;

	BOOL bAff_Opt = m_cbAffPhy_Optician.GetCheck();
	BOOL bRefProv = m_cbReferringProvider.GetCheck();
	BOOL bOrderingProv = m_cbOrderingProvider.GetCheck();
	BOOL bSupProv = m_cbSupervisingProvider.GetCheck();
	CString strOldValue, strNewValue;
	//std::pair<long, ProvidersSetTypes>

	IRowSettingsPtr pRow = m_DL_SelectedProviders->GetFirstRow();
	while (pRow)
	{
		long nPersonID = VarLong(pRow->GetValue(0));
		CString ContactName = VarString(pRow->GetValue(1));
		std::map<long, ProvidersSetTypes>::iterator it;
		//BOOL bOrderingProvNew;
		CString strAff_Opt = "";
		AuditEventItems eAff_Opt;// (s.tullis 2014-05-28 14:35) - PLID 61826 - Need a mass configuration utility for configuring provider types. (Master Item)
		if (GetSelectedProviderType() == eptProvider)
		{
			 it = m_listOldProviderTypes.find(nPersonID);
			 strAff_Opt = "Optician";
			 eAff_Opt = aeiOpticianStatus;
		}// (s.tullis 2014-05-28 14:34) - PLID 61827 - The mass configuration utility for configuring provider types needs to also support referring physicians.
		else if (GetSelectedProviderType()== eptReferringPhysician)
		{
			it = m_listOldReferringPhysicianTypes.find(nPersonID);
			strAff_Opt = "Affiliate Physician";
			eAff_Opt = aeiAffiliatePhysician;
		}
				
				
		if (bOrderingProv || bupdate == FALSE){
			if (it->second.OrderingProvider == !bupdate ){
				if (nAuditTransactionID == -1){
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditField(nAuditTransactionID, ContactName, aeiProviderOrderingProvider, (it->second.OrderingProvider == TRUE) ? "Ordering Provider Checked" : "Ordering Provider Not Checked", ( bupdate== TRUE) ? "Ordering Provider Checked" : "Ordering Provider Not Checked");
			}
		}
		if (bRefProv ||bupdate == FALSE){
			if (it->second.ReferringProvider == !bupdate ){
				if (nAuditTransactionID == -1){
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditField(nAuditTransactionID, ContactName, aeiProviderReferringProvider, (it->second.ReferringProvider == TRUE) ? "Referring Provider Checked" : "Referring Provider Not Checked", (bupdate==TRUE) ? "Referring Provider Checked" : "Referring Provider Not Checked");
			}
		}
		if (bSupProv || bupdate == FALSE){
			if (it->second.SupervisingProvider == !bupdate){
				if (nAuditTransactionID == -1){
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditField(nAuditTransactionID, ContactName, aeiProviderSupervisingProvider, (it->second.SupervisingProvider == TRUE) ? "Supervising Provider Checked" : "Supervising Provider Not Checked", (bupdate==TRUE) ? "Supervising Provider Checked" : "Supervising Provider Not Checked");
		}
		}
		if (bAff_Opt || bupdate == FALSE){
			if ((GetSelectedProviderType() == eptProvider)){
				if ((it->second.Optician == !bupdate)){
					if (nAuditTransactionID == -1){
						nAuditTransactionID = BeginAuditTransaction();
					}

					AuditField(nAuditTransactionID, ContactName, eAff_Opt, (it->second.Optician == TRUE) ? strAff_Opt + " Checked" : strAff_Opt + " Not Checked", (bupdate == TRUE) ? strAff_Opt + " Checked" : strAff_Opt + " Not Checked");
				}
			}
			else
			{
				if ((it->second.AffiliateProvider == !bupdate)){
					if (nAuditTransactionID == -1){
						nAuditTransactionID = BeginAuditTransaction();
					}

					AuditField(nAuditTransactionID, ContactName, eAff_Opt, (it->second.AffiliateProvider == TRUE) ? strAff_Opt + " Checked" : strAff_Opt + " Not Checked", (bupdate == TRUE) ? strAff_Opt + " Checked" : strAff_Opt + " Not Checked");
				}
			}
		}
		

		pRow = pRow->GetNextRow();
	}
	if (nAuditTransactionID != -1){
		CommitAuditTransaction(nAuditTransactionID);
	}
}

void CConfigureProviderTypesDlg::AuditField( long nTransactionAuditID,CString ContactName,AuditEventItems aeiItem, CString strOld, CString strNew, AuditEventPriorities aepPriority /* = aepMedium */, AuditEventTypes aetType /* = aetChanged */) {

	
	AuditEvent(-1, ContactName, nTransactionAuditID, aeiItem, GetActiveContactID(), strOld, strNew, aepPriority, aetType);

	//SendContactsTablecheckerMsg();

}

void CConfigureProviderTypesDlg::DblClickCellDlUnselectedProviders(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		// moving from unselected to selected datalist

		IRowSettingsPtr pRow(lpRow);
		if (!pRow){
			return;
		}

		m_DL_UnselectedProviders->RemoveRow(pRow);
		m_DL_SelectedProviders->AddRowAtEnd(pRow, NULL);

		m_DL_SelectedProviders->Sort();
	}NxCatchAll(__FUNCTION__);
}




void CConfigureProviderTypesDlg::DblClickCellDlSelectedProviders(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		// Removing selected items and putting them back into the unselected list
		IRowSettingsPtr pRow(lpRow);

		if (!pRow){
			return;
		}

		m_DL_SelectedProviders->RemoveRow(pRow);
		m_DL_UnselectedProviders->AddRowSorted(pRow, NULL);
		m_DL_UnselectedProviders->Sort();

	}NxCatchAll(__FUNCTION__);
}
