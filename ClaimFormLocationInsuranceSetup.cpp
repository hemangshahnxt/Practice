// ClaimFormLocationInsuranceSetup.cpp : implementation file

//
// (s.tullis 2016-02-29 21:21) - PLID 68262 - Created
#include "stdafx.h"
#include "Practice.h"
#include "ClaimFormLocationInsuranceSetup.h"
#include "afxdialogex.h"

enum ClaimFormList {
	cflID = 0,
	cflName,
} ;

enum InsuranceClaimList{
	iclIsSetupRecord,
	iclID,
	iclName,
	iclAddress,
	iclFormType,
};

enum LocationList {
	llID = 0,
	llName,
};

// CClaimFormLocationInsuranceSetup dialog

IMPLEMENT_DYNAMIC(CClaimFormLocationInsuranceSetup, CNxDialog)

CClaimFormLocationInsuranceSetup::CClaimFormLocationInsuranceSetup(CWnd* pParent /*=NULL*/)
	: CNxDialog(CClaimFormLocationInsuranceSetup::IDD, pParent)
{
	m_nCurLocationID = -1;
}

CClaimFormLocationInsuranceSetup::~CClaimFormLocationInsuranceSetup()
{
}

void CClaimFormLocationInsuranceSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLAIM_APPLY_ALL, m_buttonApplyAll);
	DDX_Control(pDX, IDOK, m_buttonClose);
	DDX_Control(pDX, IDC_LOCATION_CLAIM_COLOR, m_nxc);
}

BOOL CClaimFormLocationInsuranceSetup::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		m_buttonClose.AutoSet(NXB_CLOSE);
		m_buttonApplyAll.AutoSet(NXB_OK);
		m_nxc.SetColor(m_nColor);
		m_pLocationList = BindNxDataList2Ctrl(IDC_CLAIM_LOCATION, true);
		m_pInsuranceList = BindNxDataList2Ctrl(IDC_CLAIM_INSURANCE_COMPANY, GetRemoteData(),false);
		m_pClaimFormList = BindNxDataList2Ctrl(IDC_CLAIM_FORM_LIST, GetRemoteData(), false);

		InitializeClaimFormList();
		m_pInsuranceList->FromClause = _bstr_t(GetInsuranceClaimListFROMSQL());
		m_pLocationList->SetSelByColumn(llID, GetCurrentLocationID());
		SelChosenClaimLocation(m_pLocationList->GetCurSel());
	
	}NxCatchAll(__FUNCTION__)
		
	return TRUE;
}

BEGIN_MESSAGE_MAP(CClaimFormLocationInsuranceSetup, CNxDialog)
	ON_BN_CLICKED(IDC_CLAIM_APPLY_ALL, &CClaimFormLocationInsuranceSetup::OnBnClickedClaimApplyAll)
	ON_BN_CLICKED(IDOK, &CClaimFormLocationInsuranceSetup::OnBnClickedOk)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CClaimFormLocationInsuranceSetup message handlers


// (s.tullis 2016-03-01 11:47) - PLID 68262 - Initialize claim form list with our hardcoded values
void CClaimFormLocationInsuranceSetup::InitializeClaimFormList()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)1);
		pRow->PutValue(cflName, "HCFA");
		m_pClaimFormList->AddRowSorted(pRow,NULL);

		pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)2);
		pRow->PutValue(cflName, "UB");
		m_pClaimFormList->AddRowSorted(pRow,NULL);

		pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)3);
		pRow->PutValue(cflName, "ADA");
		m_pClaimFormList->AddRowSorted(pRow,NULL);

		pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)4);
		pRow->PutValue(cflName, "IDPA");
		m_pClaimFormList->AddRowSorted(pRow,NULL);

		pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)5);
		pRow->PutValue(cflName, "NYWC");
		m_pClaimFormList->AddRowSorted(pRow,NULL);

		pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)6);
		pRow->PutValue(cflName, "MICR");
		m_pClaimFormList->AddRowSorted(pRow,NULL);

		pRow = m_pClaimFormList->GetNewRow();
		pRow->PutValue(cflID, (long)7);
		pRow->PutValue(cflName, "NY Medicaid");
		m_pClaimFormList->AddRowSorted(pRow,NULL);
		//Default to HCFA
		m_pClaimFormList->SetSelByColumn(cflID, 1);
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-03-01 11:47) - PLID 68262 - Set all our insurance companies formtype to the formtype dropdown
void CClaimFormLocationInsuranceSetup::OnBnClickedClaimApplyAll()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pClaimFormList->GetCurSel();
		if (pRow)
		{
			_variant_t ClaimFormID = pRow->GetValue(cflID);
			pRow = m_pInsuranceList->GetFirstRow();
			while (pRow)
			{
				pRow->PutValue(InsuranceClaimList::iclFormType, ClaimFormID);
				AddUpdateEditedItems(pRow);
				pRow = pRow->GetNextRow();
			}

			SaveEdited();
		}
	}NxCatchAll(__FUNCTION__)
}
BEGIN_EVENTSINK_MAP(CClaimFormLocationInsuranceSetup, CNxDialog)
	ON_EVENT(CClaimFormLocationInsuranceSetup, IDC_CLAIM_LOCATION, 16, CClaimFormLocationInsuranceSetup::SelChosenClaimLocation, VTS_DISPATCH)
	ON_EVENT(CClaimFormLocationInsuranceSetup, IDC_CLAIM_INSURANCE_COMPANY, 10, CClaimFormLocationInsuranceSetup::EditingFinishedClaimInsuranceCompany, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

// (s.tullis 2016-03-01 11:47) - PLID 68262 - Requery our insurance List according to our Location dropDown
void CClaimFormLocationInsuranceSetup::SelChosenClaimLocation(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {
				long nLocationID = VarLong(pRow->GetValue(LocationList::llID));
				SaveEdited();
				m_nCurLocationID = nLocationID;
				m_pInsuranceList->WhereClause = _bstr_t(FormatString(" ClaimLocationInsuranceQ.LocationID = %li ", nLocationID ));
				m_pInsuranceList->Requery();
			}
	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2016-03-01 11:47) - PLID 68262 
// Get Actually records and populate the rest with of the insurance companies with the locations Default Claim Form
CString CClaimFormLocationInsuranceSetup::GetInsuranceClaimListFROMSQL()
{
	
return R"(
(
    -- Select our Setup Records
	Select 1 as IsSetupRecord,LocationID, InsuranceID, FormType 
	FROM ClaimFormLocationInsuranceSetupT
	UNION ALL -- Also select locations / insurance combinations that are not in the setup tables with the preference
	Select 0 as IsSetupRecord, Locations.LocationID as LocationID , PersonID as InsuranceID, Locations.FormType as FormType FROM InsuranceCoT
	CROSS JOIN ( SELECT ID as LocationID , DefaultClaimForm as FormType FROM LocationsT WHERE Managed = 1 ) Locations
	LEFT JOIN ClaimFormLocationInsuranceSetupT
	ON InsuranceCoT.PersonID = ClaimFormLocationInsuranceSetupT.InsuranceID AND Locations.LocationID = ClaimFormLocationInsuranceSetupT.LocationID
	WHERE ClaimFormLocationInsuranceSetupT.InsuranceID IS NULL AND ClaimFormLocationInsuranceSetupT.LocationID IS NULL  

) ClaimLocationInsuranceQ
Inner Join PersonT
ON InsuranceID = PersonT.ID AND PersonT.Archived = 0
Inner Join InsuranceCoT
ON InsuranceID = InsuranceCoT.PersonID			

		)";
}
// (s.tullis 2016-03-01 11:47) - PLID 68262 - Update Edited Items if not Add it to the edited list
void CClaimFormLocationInsuranceSetup::AddUpdateEditedItems(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		if (pRow) {
			long nInsuranceID = VarLong(pRow->GetValue(InsuranceClaimList::iclID));
			long nClaimType = VarLong(pRow->GetValue(InsuranceClaimList::iclFormType));
			bool bIsSetupRecord = VarLong(pRow->GetValue(InsuranceClaimList::iclIsSetupRecord)) == TRUE ? true : false;
			std::map<InsuranceID, ClaimTypeConfig>::iterator it = m_mInsuranceClaimFormMap.find(nInsuranceID);

			if (it != m_mInsuranceClaimFormMap.end())
			{
				it->second.bIsSetupRecord= bIsSetupRecord;
				it->second.nClaimFormType = nClaimType;
			}
			else {
				m_mInsuranceClaimFormMap.insert(std::pair<InsuranceID, ClaimTypeConfig>(nInsuranceID,  ClaimTypeConfig(bIsSetupRecord,nClaimType) ));
			}

		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-03-01 11:47) - PLID 68262 - Saved Edited 
void CClaimFormLocationInsuranceSetup::SaveEdited()
{
	CParamSqlBatch Sql;
	try {
		
		if (m_mInsuranceClaimFormMap.empty())
		{
			return;
		}

		std::map<InsuranceID, ClaimTypeConfig>::iterator it = m_mInsuranceClaimFormMap.begin();
		while (it != m_mInsuranceClaimFormMap.end())
		{
			if (it->second.bIsSetupRecord == true)
			{
				Sql.Add(R"(
						Update ClaimFormLocationInsuranceSetupT 
						SET FormType = {INT}
						WHERE LocationID = {INT} AND InsuranceID = {INT}
						)",it->second.nClaimFormType , m_nCurLocationID , it->first);
			}
			else {
				Sql.Add(R"(
						Insert into ClaimFormLocationInsuranceSetupT (InsuranceID, LocationID, FormType)
						VALUES ({INT}, {INT} , {INT} )
						)", it->first, m_nCurLocationID, it->second.nClaimFormType);
			}

			++it;
		}


		CString strSql = Sql.Flatten();
		NxAdo::PushMaxRecordsWarningLimit pmr(-1);
		ExecuteSql(strSql);
	m_mInsuranceClaimFormMap.clear();
	}NxCatchAll(__FUNCTION__)
}

void CClaimFormLocationInsuranceSetup::OnBnClickedOk()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__)
}


void CClaimFormLocationInsuranceSetup::EditingFinishedClaimInsuranceCompany(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		InsuranceClaimList icl = (InsuranceClaimList)nCol;
		if (icl == InsuranceClaimList::iclFormType)
		{
			AddUpdateEditedItems(pRow);
			SaveEdited();
		}
		
	}NxCatchAll(__FUNCTION__)
}


void CClaimFormLocationInsuranceSetup::OnClose()
{
	try {
	
		CNxDialog::OnClose();

	}NxCatchAll(__FUNCTION__)
}
