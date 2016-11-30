// ConfigurePrimaryEMNProviderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ConfigurePrimaryEMNProviderDlg.h"


namespace SelectedEMRProviderList
{
	enum ProvListCol
	{
		PersonID = 0,
		ProviderFullName,
		PrimaryEMRProviderID,
	};
}

namespace LinkedEMRProviderList
{
	enum ProvListCol
	{
		PersonID = 0,
		ProviderFullName,
		
	};
}

// CConfigurePrimaryEMNProviderDlg dialog
//(r.wilson 7/29/2013) PLID 48684 - Created Dialog

IMPLEMENT_DYNAMIC(CConfigurePrimaryEMNProviderDlg, CNxDialog)

CConfigurePrimaryEMNProviderDlg::CConfigurePrimaryEMNProviderDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigurePrimaryEMNProviderDlg::IDD, pParent)
{	
}

CConfigurePrimaryEMNProviderDlg::~CConfigurePrimaryEMNProviderDlg()
{
}

void CConfigurePrimaryEMNProviderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK , m_btnClose);
}


BEGIN_MESSAGE_MAP(CConfigurePrimaryEMNProviderDlg, CNxDialog)
END_MESSAGE_MAP()


// CConfigurePrimaryEMNProviderDlg message handlers


BOOL CConfigurePrimaryEMNProviderDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try
	{		
		m_btnClose.AutoSet(NXB_CLOSE);
		CStatic *staticTextDialogDesc = (CStatic *)GetDlgItem( IDC_STATIC_EMR_PRIM_PROV_DESC );		
		staticTextDialogDesc->SetWindowTextA("In this window you can set a primary provider for each secondary provider. When creating a new EMN with a secondary provider the default primary provider set here will be assigned to the EMN.");
					
		g_pLicense->GetUsedEMRProviders(m_dwaLicensedProvIDs);
		
		m_SelectedProviderList = BindNxDataList2Ctrl(IDC_SELECTED_PROVIDER_DATALIST, false);
		m_LinkedToProviderList = BindNxDataList2Ctrl(IDC_LINKED_TO_PROVIDER_DATALIST, false);

		m_SelectedProviderList->FromClause = "ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";
		m_SelectedProviderList->WhereClause = "PersonT.Archived = 0";

		m_SelectedProviderList->Requery();
		m_SelectedProviderList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_SelectedProviderList->CurSel = m_SelectedProviderList->GetFirstRow();

		m_LinkedToProviderList->FromClause = "ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID";		

		// Load Providers into the bottom datalist (All Licensed EMR Providers)
		PopulateLinkedProviderList();

		RefreshLinkedToProviderList();

	}NxCatchAll(__FUNCTION__)

	return TRUE;
}BEGIN_EVENTSINK_MAP(CConfigurePrimaryEMNProviderDlg, CNxDialog)
ON_EVENT(CConfigurePrimaryEMNProviderDlg, IDC_LINKED_TO_PROVIDER_DATALIST, 16, CConfigurePrimaryEMNProviderDlg::SelChosenLinkedToProviderDatalist, VTS_DISPATCH)
ON_EVENT(CConfigurePrimaryEMNProviderDlg, IDC_SELECTED_PROVIDER_DATALIST, 16, CConfigurePrimaryEMNProviderDlg::SelChosenSelectedProviderDatalist, VTS_DISPATCH)
ON_EVENT(CConfigurePrimaryEMNProviderDlg, IDC_SELECTED_PROVIDER_DATALIST, 1, CConfigurePrimaryEMNProviderDlg::SelChangingSelectedProviderDatalist, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CConfigurePrimaryEMNProviderDlg, IDC_LINKED_TO_PROVIDER_DATALIST, 1, CConfigurePrimaryEMNProviderDlg::SelChangingLinkedToProviderDatalist, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

//(r.wilson 7/29/2013) PLID 48684
void CConfigurePrimaryEMNProviderDlg::RefreshLinkedToProviderList()
{
	long nSelectedProvID = VarLong(m_SelectedProviderList->GetCurSel()->GetValue(SelectedEMRProviderList::PrimaryEMRProviderID), -1);
	CString strSelectedProvID;
	strSelectedProvID.Format("%li",nSelectedProvID);
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_LinkedToProviderList->SearchByColumn(LinkedEMRProviderList::PersonID, bstr_t(strSelectedProvID), m_LinkedToProviderList->GetFirstRow(), TRUE);	
}

//(r.wilson 7/29/2013) PLID 48684 - 
CString CConfigurePrimaryEMNProviderDlg::CreateLicensedProviderInClauseSqlSnippet(BOOL bIN)
{	
	CString strInValues = "";
	CString strAndIN;
	if(bIN){
		strAndIN = "AND PersonT.ID IN (";
	}
	else{
		strAndIN = "AND PersonT.ID NOT IN (";
	}
	for(int i = 0 ; i < m_dwaLicensedProvIDs.GetSize(); i++)
	{			
		long nProvID = m_dwaLicensedProvIDs[i];		
		if(i == 0){
			strInValues += strAndIN;
		}
		else{
			strInValues += ", ";
		}
		CString strNumVal = "";
		strNumVal.Format("%li", nProvID);
		strInValues += strNumVal;
	}
	if(m_dwaLicensedProvIDs.GetSize() > 0){
		strInValues += ")";
		
		
	}

	return strInValues;
}

//(r.wilson 7/29/2013) PLID 48684
void CConfigurePrimaryEMNProviderDlg::PopulateLinkedProviderList()
{	
	CString strInValues = CreateLicensedProviderInClauseSqlSnippet(TRUE);

	long nSelectedProvID = VarLong(m_SelectedProviderList->GetCurSel()->GetValue(SelectedEMRProviderList::PersonID), -1);

	CString strSelectedProvID;
	strSelectedProvID.Format("%li", nSelectedProvID);
	strInValues += " AND PersonT.ID <> " + strSelectedProvID;

	CString strWhereClause = "Archived <> 1 " + strInValues;
	
	//CString strQuery = "Select PersonT.ID, PersonT.Last, PersonT.First FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID WHERE Archived <> 1 AND PersonT.ID IN ({ARRAY})";		
	m_LinkedToProviderList->WhereClause = bstr_t(strWhereClause);	

	//_RecordsetPtr rs = CreateParamRecordset(GetConnection(),strQuery, m_dwaLicensedProvIDs);

	m_LinkedToProviderList->Requery();
	m_LinkedToProviderList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	//Add the <No Provider Selected> row to the top
	NXDATALIST2Lib::IRowSettingsPtr pNoProviderRow = m_LinkedToProviderList->GetNewRow();
	if(pNoProviderRow)
	{
		pNoProviderRow->PutValue(SelectedEMRProviderList::PersonID, -1);
		pNoProviderRow->PutValue(SelectedEMRProviderList::ProviderFullName, "<No Provider Selected>");
		m_LinkedToProviderList->AddRowBefore(pNoProviderRow, m_LinkedToProviderList->GetFirstRow());
	}	
}

//(r.wilson 7/29/2013) PLID 48684 
void CConfigurePrimaryEMNProviderDlg::SelChosenLinkedToProviderDatalist(LPDISPATCH lpRow)
{	
	try
	{		
		//Get the selected provider id so we can exclude it from the datalist below
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if(!pRow){
			return;
		}

		//Get the id of the Selected Provider from top datalist		
		_variant_t vtSelectedProvID =  m_SelectedProviderList->GetCurSel()->GetValue(SelectedEMRProviderList::PersonID);

		//Get the id of the provider from the newly selected row which came from the bottom datalist		
		_variant_t vtLinkedProvID = pRow->GetValue(LinkedEMRProviderList::PersonID);

		//Create Query to update providers defualt emr linked provider id
		CSqlFragment sqlFragQuery("Update ProvidersT SET EMRDefaultProviderID = {VT_I4} WHERE PersonID = {VT_I4}",VarLong(vtLinkedProvID) == -1 ? g_cvarNull:vtLinkedProvID,vtSelectedProvID);
	
		//Execute query AKA Save()
		ExecuteParamSql(sqlFragQuery);
				
		m_SelectedProviderList->CurSel->PutValue(SelectedEMRProviderList::PrimaryEMRProviderID, vtLinkedProvID);		

	}NxCatchAll(__FUNCTION__);
}

void CConfigurePrimaryEMNProviderDlg::SelChosenSelectedProviderDatalist(LPDISPATCH lpRow)
{
	try
	{		
		PopulateLinkedProviderList();
		RefreshLinkedToProviderList();
	}NxCatchAll(__FUNCTION__);
}

void CConfigurePrimaryEMNProviderDlg::SelChangingSelectedProviderDatalist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if (*lppNewSel == NULL) {		
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}


void CConfigurePrimaryEMNProviderDlg::SelChangingLinkedToProviderDatalist(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try
	{
		if (*lppNewSel == NULL) {		
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}
