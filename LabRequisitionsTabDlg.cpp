// LabRequisitionsTabDlg.cpp : implementation file
//TES 11/25/2009 - PLID 36193 - Created
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "LabRequisitionsTabDlg.h"
#include "LabRequisitionDlg.h"
#include "LabEntryDlg.h"

// CLabRequisitionsTabDlg dialog

IMPLEMENT_DYNAMIC(CLabRequisitionsTabDlg, CNxDialog)

CLabRequisitionsTabDlg::CLabRequisitionsTabDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabRequisitionsTabDlg::IDD, pParent)
{
	m_nInitialLabID = -1;
	m_nPatientID = -1;
	m_nDefaultLocationID = -1;
	m_nLabProcedureID = -1;
	m_ltType = ltInvalid;
	m_nInitialAnatomicLocationID = -1;
	m_nInitialAnatomicQualifierID = -1;
	m_asInitialAnatomicSide = asNone;
	//TES 11/25/2009 - PLID 36193 - Remember our parent (we should only ever be a child of a CLabEntryDlg)
	m_pLabEntryDlg = (CLabEntryDlg*)pParent;
}

CLabRequisitionsTabDlg::~CLabRequisitionsTabDlg()
{
	//TES 11/25/2009 - PLID 36193 - Clean up all our children.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->DestroyWindow();
		delete m_arRequisitions[i];
	}
	m_arRequisitions.RemoveAll();
}

void CLabRequisitionsTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLabRequisitionsTabDlg, CNxDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CLabRequisitionsTabDlg message handlers
void CLabRequisitionsTabDlg::PostLoad()
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->PostLoad();
	}
}

void CLabRequisitionsTabDlg::SetPatientID(long nPatientID)
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children.
	m_nPatientID = nPatientID;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetPatientID(m_nPatientID);
	}
}

// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
void CLabRequisitionsTabDlg::SetDefaultLocationID(long nLocationID)
{
	m_nDefaultLocationID = nLocationID;
	//since this is a default, this ought to have no benefit to
	//existing requisitions, but set the default value anyways
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetDefaultLocationID(m_nDefaultLocationID);
	}
}

// (r.gonet 07/22/2013) - PLID 57683 - Sets the providers from the source EMN.
void CLabRequisitionsTabDlg::SetEMNProviders(CDWordArray &dwProviderIDs)
{
	m_dwEMNProviderIDs.Copy(dwProviderIDs);
	//since this is a default, this ought to have no benefit to
	//existing requisitions, but set the default value anyways
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetEMNProviders(dwProviderIDs);
	}
}

void CLabRequisitionsTabDlg::SetLabProcedureID(long nLabProcedureID)
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children.
	m_nLabProcedureID = nLabProcedureID;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetLabProcedureID(m_nLabProcedureID);
	}
}

void CLabRequisitionsTabDlg::SetLabProcedureType(LabType ltType)
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children.
	m_ltType = ltType;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetLabProcedureType(m_ltType);
	}
}

//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
void CLabRequisitionsTabDlg::SetInitialAnatomicLocation(long nAnatomicLocationID, long nAnatomicQualifierID, AnatomySide asSide)
{
	m_nInitialAnatomicLocationID = nAnatomicLocationID;
	m_nInitialAnatomicQualifierID = nAnatomicQualifierID;
	m_asInitialAnatomicSide = asSide;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetInitialAnatomicLocation(nAnatomicLocationID, nAnatomicQualifierID, asSide);
	}
}

COleDateTime CLabRequisitionsTabDlg::GetFirstBiopsyDate()
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children.
	COleDateTime dtBiopsy = COleDateTime::GetCurrentTime();
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		COleDateTime dt = m_arRequisitions[i]->GetBiopsyDate();
		if(dt.GetStatus() == COleDateTime::valid && dt < dtBiopsy) {
			dtBiopsy = dt;
		}
	}
	return dtBiopsy;
}

// (j.jones 2010-05-06 09:37) - PLID 38520 - get the earliest input date of any requisition
COleDateTime CLabRequisitionsTabDlg::GetFirstInputDate()
{
	COleDateTime dtInput = COleDateTime::GetCurrentTime();
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		COleDateTime dt = m_arRequisitions[i]->GetInputDate();
		if(dt.GetStatus() == COleDateTime::valid && dt < dtInput) {
			dtInput = dt;
		}
	}
	return dtInput;
}

// (j.jones 2016-02-22 09:57) - PLID 68348 - get the first lab's provider
// "first" is an arbritrary choice here
long CLabRequisitionsTabDlg::GetFirstLabProviderID()
{
	for (int i = 0; i < m_arRequisitions.GetSize(); i++) {
		CDWordArray dwProviderIDs;
		m_arRequisitions[i]->GetCurrentProviderIDs(dwProviderIDs);
		for (int j = 0; j < dwProviderIDs.GetSize(); j++) {
			long nProviderID = (long)dwProviderIDs.GetAt(j);
			if (nProviderID != -1) {
				//return the first valid provider we find
				return nProviderID;
			}
		}
	}

	return -1;
}

// (j.jones 2016-02-22 09:57) - PLID 68348 - get the first lab's location
// "first" is an arbritrary choice here
long CLabRequisitionsTabDlg::GetFirstLocationID()
{
	for (int i = 0; i < m_arRequisitions.GetSize(); i++) {
		long nLocationID = m_arRequisitions[i]->GetCurrentLocationID();
		if (nLocationID != -1) {
			//return the first valid location we find
			return nLocationID;
		}
	}

	return -1;
}

void CLabRequisitionsTabDlg::LoadNew()
{
	//TES 11/25/2009 - PLID 36193 - We shouldn't have any children yet.
	ASSERT(m_arRequisitions.GetSize() == 0);

	//TES 11/25/2009 - PLID 36193 - Create a new dialog.
	CLabRequisitionDlg *pDlg = new CLabRequisitionDlg(this);
	m_arRequisitions.Add(pDlg);
	pDlg->Create(IDD_LAB_REQUISITION_DLG, this);
	//TES 11/25/2009 - PLID 36193 - Position it.
	CRect rcSubDlg;
	GetDlgItem(IDC_REQUISITION_PLACEHOLDER)->GetWindowRect(&rcSubDlg);
	ScreenToClient(&rcSubDlg);
	pDlg->MoveWindow(&rcSubDlg);
	//TES 11/25/2009 - PLID 36193 - Set all stored variables.
	PreloadRequisition(pDlg);
	//TES 11/25/2009 - PLID 36193 - Load.
	pDlg->LoadNew();
	
	//TES 11/25/2009 - PLID 36193 - Now set up our tabs.
	m_tab->Size = 1;
	m_tab->TabWidth = 5;
	// (z.manning 2010-06-30 17:57) - PLID 38976 - Handled in CLabRequisitionDlg::LoadNew
	//m_tab->PutLabel(0, _bstr_t(m_pLabEntryDlg->GetFormNumber()));
	m_tab->CurSel = 0;
	//TES 11/25/2009 - PLID 36193 - Show the new dialog.
	ReflectCurrentTab();
	
}

using namespace ADODB;
void CLabRequisitionsTabDlg::LoadExisting()
{
	//TES 11/25/2009 - PLID 36193 - We shouldn't have any chidren yet.
	ASSERT(m_arRequisitions.GetSize() == 0);
	//TES 11/25/2009 - PLID 36193 - Figure out where we're going to put our children.
	CRect rcSubDlg;
	GetDlgItem(IDC_REQUISITION_PLACEHOLDER)->GetWindowRect(&rcSubDlg);
	ScreenToClient(&rcSubDlg);
	//TES 11/25/2009 - PLID 36193 - Load all labs that have the same form number (and patient ID, just in case) as 
	// our initial lab.
	//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
	_RecordsetPtr rsReqs = CreateParamRecordset("SELECT LabsT.ID, LabsT.FormNumberTextID, LabsT.Specimen "
		"FROM LabsT INNER JOIN (SELECT * FROM LabsT WHERE ID = {INT}) AS RequestedLab "
		"ON LabsT.FormNumberTextID = RequestedLab.FormNumberTextID AND LabsT.PatientID = RequestedLab.PatientID "
		"WHERE LabsT.Deleted = 0 AND {SQLFRAGMENT}",
		m_nInitialLabID, GetAllowedLocationClause_Param("LabsT.LocationID"));
	int nInitialIndex = -1;
	while(!rsReqs->eof) {
		//TES 11/25/2009 - PLID 36193 - Create the dialog.
		CLabRequisitionDlg *pDlg = new CLabRequisitionDlg(this);
		m_arRequisitions.Add(pDlg);
		pDlg->Create(IDD_LAB_REQUISITION_DLG, this);
		//TES 11/25/2009 - PLID 36193 - Position it.
		pDlg->MoveWindow(&rcSubDlg);
		long nID = AdoFldLong(rsReqs, "ID");
		pDlg->SetLabID(nID);
		if(nID == m_nInitialLabID) {
			//TES 11/25/2009 - PLID 36193 - This is the one we want to start with, and therefore is the only one that
			// can potentially have been launched from EMR.
			nInitialIndex = m_arRequisitions.GetSize()-1;
		}
		//TES 11/25/2009 - PLID 36193 - Set all stored variables.
		PreloadRequisition(pDlg);
		//TES 11/25/2009 - PLID 36193 - Load
		pDlg->LoadExisting();

		//TES 11/25/2009 - PLID 36193 - Set up our tabs, we set the width to 5, or one more than our count, whichever is lower.
		int nTabCount = m_arRequisitions.GetSize();
		m_tab->PutSize(nTabCount);
		if(nTabCount > 5) {
			m_tab->PutTabWidth(nTabCount+1);
		}
		CString strLabel = AdoFldString(rsReqs, "FormNumberTextID", "");
		CString strSpecimen = AdoFldString(rsReqs, "Specimen", "");
		if(!strSpecimen.IsEmpty()) {
			strLabel += " - " + strSpecimen;
		}
		m_tab->PutLabel(m_arRequisitions.GetSize()-1, _bstr_t(strLabel));

		rsReqs->MoveNext();
	}

	if(nInitialIndex == -1) {
		//TES 11/25/2009 - PLID 36193 - Our initial lab wasn't loaded!
		//TES 12/7/2009 - PLID 36510 - The initial lab must have been deleted, so abort.  This is easily doable by deleting a lab that
		// was spawned through EMR.
		MsgBox("This lab has been deleted (possibly by another user).  The lab cannot be opened.");
		m_pLabEntryDlg->EndDialog(IDOK);
		nInitialIndex = 0;
	}
	m_tab->CurSel = nInitialIndex;
	//TES 11/25/2009 - PLID 36193 - Show the initial dialog.
	ReflectCurrentTab();
}

// (z.manning 2010-03-22 11:52) - PLID 37439 - Added bUseDefaultAnatomicLocation
void CLabRequisitionsTabDlg::AddNew(BOOL bUseDefaultAnatomicLocation)
{
	CLabRequisitionDlg *pdlgCurrentLabReq = GetActiveLabRequisitionDlg();

	//TES 11/25/2009 - PLID 36193 - Create a new dialog.
	CLabRequisitionDlg *pDlg = new CLabRequisitionDlg(this);
	m_arRequisitions.Add(pDlg);

	// (z.manning 2010-03-22 12:12) - PLID 37439 - Do we need to set the default anatomic location?
	if(bUseDefaultAnatomicLocation) {
		SetInitialAnatomicLocation(m_nInitialAnatomicLocationID, m_nInitialAnatomicQualifierID, m_asInitialAnatomicSide);
	}

	pDlg->Create(IDD_LAB_REQUISITION_DLG, this);
	//TES 11/25/2009 - PLID 36193 - Position it.
	CRect rcSubDlg;
	GetDlgItem(IDC_REQUISITION_PLACEHOLDER)->GetWindowRect(&rcSubDlg);
	ScreenToClient(&rcSubDlg);
	pDlg->MoveWindow(&rcSubDlg);

	// (z.manning 2010-05-05 10:17) - PLID 37190 - There is now a preference to pull the initial diagnosis 
	// when adding a new requisition to the same lab.
	if(GetRemotePropertyInt("LabsRetainInitialDiagnosis", 1, 0, "<None>") == 1)
	{
		if(pdlgCurrentLabReq != NULL) {
			pDlg->SetDefaultInitialDiagnosis(pdlgCurrentLabReq->GetLastSavedInitialDiagnosis());
		}
	}

	//TES 9/29/2010 - PLID 40644 - There is now an equivalent preference for the Comments field
	if(GetRemotePropertyInt("LabsRetainComments", 1, 0, "<None>") == 1)
	{
		if(pdlgCurrentLabReq != NULL) {
			pDlg->SetDefaultComments(pdlgCurrentLabReq->GetLastSavedComments());
		}
	}

	//TES 9/29/2010 - PLID 40644 - There is now an equivalent preference for the Provider field
	if(GetRemotePropertyInt("LabsRetainProviders", 1, 0, "<None>") == 1)
	{
		if(pdlgCurrentLabReq != NULL) {
			CDWordArray dwDefaultProviderIDs;
			pdlgCurrentLabReq->GetLastSavedProviderIDs(dwDefaultProviderIDs);
			pDlg->SetDefaultProviders(dwDefaultProviderIDs, pdlgCurrentLabReq->GetLastSavedProviderNames());
		}
	}

	if(pdlgCurrentLabReq != NULL) {
		//TES 9/29/2010 - PLID 40644 - We also copy the Lab Location (no preference, just do it).
		pDlg->SetDefaultLabLocationID(pdlgCurrentLabReq->GetLastSavedLabLocationID());
		// (r.gonet 07/22/2013) - PLID 57683 - Also pass the EMN Providers since that may be used to initialize the lab provider.
		pDlg->SetEMNProviders(m_dwEMNProviderIDs);
	}

	pDlg->SetLabEntryDlg(m_pLabEntryDlg);
	//TES 11/25/2009 - PLID 36193 - Set stored variables.
	PreloadRequisition(pDlg);
	//TES 9/29/2010 - PLID 40644 - We copy the Location, need to do it after Preload as that might overwrite our default.
	if(pdlgCurrentLabReq != NULL) {
		pDlg->SetDefaultLocationID(pdlgCurrentLabReq->GetLastSavedLocationID());
	}
	//TES 11/25/2009 - PLID 36193 - Load
	pDlg->LoadNew();
	pDlg->PostLoad();

	//TES 11/25/2009 - PLID 36193 - Update our tabs.
	int nTabCount = m_arRequisitions.GetSize();
	m_tab->PutSize(nTabCount);
	if(nTabCount > 5) {
		m_tab->PutTabWidth(nTabCount+1);
	}
	HandleSpecimenChange(pDlg, pDlg->GetSpecimen());
	m_tab->CurSel = nTabCount-1;
	//TES 11/25/2009 - PLID 36193 - Show the new dialog.
	ReflectCurrentTab();
}

BOOL CLabRequisitionsTabDlg::AreAllLabsValid()
{
	//TES 11/25/2009 - PLID 36193 - Check each of our children.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		if(!m_arRequisitions[i]->IsLabValid()) {
			return FALSE;
		}
	}
	return TRUE;
}

// (r.gonet 06/11/2013) - PLID 56389 - Returns the number of new, unsaved labs
long CLabRequisitionsTabDlg::GetNewLabsCount()
{
	long nCount = 0;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		if(m_arRequisitions[i]->IsNew()) {
			nCount++;
		}
	}
	return nCount;
}

void CLabRequisitionsTabDlg::SetDefaultToBeOrdered(const CString &strToBeOrdered)
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children
	m_strToBeOrdered = strToBeOrdered;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetDefaultToBeOrdered(strToBeOrdered);
	}
}

void CLabRequisitionsTabDlg::SetCurrentDate(const _variant_t &varCurDate)
{
	//TES 11/25/2009 - PLID 36193 - Pass on to our children
	m_varCurDate = varCurDate;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		m_arRequisitions[i]->SetCurrentDate(varCurDate);
	}
}

//TES 12/1/2009 - PLID 36452 - Added an output number for the id of the new lab (if any) that gets created.
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
void CLabRequisitionsTabDlg::Save(long &nAuditTransID, IN const SharedLabInformation &sli, OUT long  &nNewLabID, IN OUT CDWordArray &arNewCDSInterventions)
{
	//TES 11/25/2009 - PLID 36193 - Just tell each of our children to save.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		if(m_arRequisitions[i]->IsNew()) {
			//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
			m_arRequisitions[i]->SaveNew(nAuditTransID, sli, nNewLabID, arNewCDSInterventions);
		}
		else {
			//TES 10/31/2013 - PLID 59251 - Pass in arNewCDSInterventions
			m_arRequisitions[i]->SaveExisting(nAuditTransID, arNewCDSInterventions);
		}
	}
}

BOOL CLabRequisitionsTabDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//TES 11/25/2009 - PLID 36193 - Initialize the tab (our only control)
		m_tab = GetDlgItemUnknown(IDC_REQUISITION_TABS);

	}NxCatchAll("Error in CLabRequisitionsTabDlg::OnInitDialog()");
	return TRUE;
}

void CLabRequisitionsTabDlg::SetInitialLabID(long nInitialLabID)
{
	m_nInitialLabID = nInitialLabID;
}

BEGIN_EVENTSINK_MAP(CLabRequisitionsTabDlg, CNxDialog)
ON_EVENT(CLabRequisitionsTabDlg, IDC_REQUISITION_TABS, 1, CLabRequisitionsTabDlg::OnSelectTabRequisitionTabs, VTS_I2 VTS_I2)
END_EVENTSINK_MAP()

void CLabRequisitionsTabDlg::OnSelectTabRequisitionTabs(short newTab, short oldTab)
{
	try {
		if(newTab != oldTab) {
			m_pLabEntryDlg->SetCurrentLab(m_arRequisitions[newTab]->GetLabID());
		}
		ReflectCurrentTab();
	}NxCatchAll("Error in CLabRequisitionsTabDlg::OnSelectTabRequisitionTabs()");
}

void CLabRequisitionsTabDlg::ReflectCurrentTab()
{
	//TES 11/25/2009 - PLID 36193 - Show the corresponding CLabRequisitionDlg, hide all the others.
	long nCurrentTab = m_tab->CurSel;
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		if(i == nCurrentTab) {
			m_arRequisitions[i]->ShowWindow(SW_SHOW);
			//TES 2/1/2010 - PLID 37143 - Our parent needs to know which custom request form to use, based on this lab's Receiving Lab.
			m_pLabEntryDlg->SetRequestForm(m_arRequisitions[i]->GetRequestForm());
			//TES 7/27/2012 - PLID 51849 - Also set the Results Form
			m_pLabEntryDlg->SetResultsForm(m_arRequisitions[i]->GetResultsForm());
		}
		else {
			m_arRequisitions[i]->ShowWindow(SW_HIDE);
		}
	}
}

void CLabRequisitionsTabDlg::HandleSpecimenChange(CLabRequisitionDlg *pChangedDlg, const CString &strNewSpecimen)
{
	//TES 11/25/2009 - PLID 36193 - Find the corresponding tab, and refresh its label.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		if(m_arRequisitions[i] == pChangedDlg) {
			CString strLabel = m_pLabEntryDlg->GetFormNumber();
			if(!strNewSpecimen.IsEmpty()) {
				strLabel += " - " + strNewSpecimen;
			}
			m_tab->PutLabel(i, _bstr_t(strLabel));
			m_pLabEntryDlg->HandleSpecimenChange(m_arRequisitions[i]->GetLabID(), strNewSpecimen);
			return;
		}
	}
}

void CLabRequisitionsTabDlg::PreloadRequisition(CLabRequisitionDlg *pDlg)
{
	//TES 11/25/2009 - PLID 36193 - Pass in all our variables.
	pDlg->SetLabEntryDlg(m_pLabEntryDlg);
	pDlg->SetPatientID(m_nPatientID);
	// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
	pDlg->SetDefaultLocationID(m_nDefaultLocationID);
	pDlg->SetLabProcedureID(m_nLabProcedureID);
	pDlg->SetLabProcedureType(m_ltType);
	pDlg->SetDefaultToBeOrdered(m_strToBeOrdered);
	pDlg->SetCurrentDate(m_varCurDate);
	//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information, only do this on the first lab.
	if(m_arRequisitions.GetSize() == 1) {
		pDlg->SetInitialAnatomicLocation(m_nInitialAnatomicLocationID, m_nInitialAnatomicQualifierID, m_asInitialAnatomicSide);
	}
	// (r.gonet 07/22/2013) - PLID 57683 - Carry over the EMN Providers.
	pDlg->SetEMNProviders(m_dwEMNProviderIDs);
}

void CLabRequisitionsTabDlg::SetCurrentLab(long nLabID)
{
	//TES 11/30/2009 - PLID 36193 - Make sure that our current lab is the same one we've been asked to show.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		if(m_arRequisitions[i]->GetLabID() == nLabID) {
			if(m_tab->CurSel != i) {
				m_tab->CurSel = i;
				ReflectCurrentTab();
			}
			return;
		}
	}
}

void CLabRequisitionsTabDlg::HandleFormNumberChange(const CString &strNewFormNumber)
{
	//TES 11/30/2009 - PLID 36193 - Refresh all our tab labels with the new form number.
	for(int i = 0; i < m_arRequisitions.GetSize(); i++) {
		CString strLabel = strNewFormNumber;
		CString strSpecimen = m_arRequisitions[i]->GetSpecimen();
		if(!strSpecimen.IsEmpty()) {
			strLabel += " - " + strSpecimen;
		}
		m_tab->PutLabel(i, _bstr_t(strLabel));
	}
}

// (z.manning 2010-04-29 12:41) - PLID 38420
void CLabRequisitionsTabDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{
		CNxDialog::OnSize(nType, cx, cy);

		SetControlPositions();

		CRect rcSubDlg;
		if(IsWindow(GetDlgItem(IDC_REQUISITION_PLACEHOLDER)->GetSafeHwnd())) {
			GetDlgItem(IDC_REQUISITION_PLACEHOLDER)->GetWindowRect(&rcSubDlg);
			ScreenToClient(&rcSubDlg);
			for(int nReqIndex = 0; nReqIndex < m_arRequisitions.GetSize(); nReqIndex++) {
				m_arRequisitions.GetAt(nReqIndex)->MoveWindow(rcSubDlg);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-05-05 10:31) - PLID 37190 - Returns the currently selected lab req dialog
CLabRequisitionDlg* CLabRequisitionsTabDlg::GetActiveLabRequisitionDlg()
{
	int nIndex = m_tab->GetCurSel();
	if(nIndex >= m_arRequisitions.GetSize()) {
		ThrowNxException("CLabRequisitionsTabDlg::GetActiveLabRequisitionDlg - More requisition tabs than dialogs");
	}
	return m_arRequisitions.GetAt(nIndex);
}

// (z.manning 2010-05-13 12:03) - PLID 37405
CLabRequisitionDlg* CLabRequisitionsTabDlg::GetLabRequisitionDlgByID(const long nLabID)
{
	for(int nReqIndex = 0; nReqIndex < m_arRequisitions.GetSize(); nReqIndex++) {
		CLabRequisitionDlg *pdlgReq = m_arRequisitions.GetAt(nReqIndex);
		if(pdlgReq->GetLabID() == nLabID) {
			return pdlgReq;
		}
	}
	
	return NULL;
}

// (z.manning 2010-05-13 14:02) - PLID 37405
void CLabRequisitionsTabDlg::ClearAllPendingTodos()
{
	for(int nReqIndex = 0; nReqIndex < m_arRequisitions.GetSize(); nReqIndex++) {
		CLabRequisitionDlg *pdlgReq = m_arRequisitions.GetAt(nReqIndex);
		pdlgReq->ClearPendingTodos();
	}
}

// (z.manning 2010-05-13 14:03) - PLID 37405
void CLabRequisitionsTabDlg::DeleteAllPendingTodos()
{
	for(int nReqIndex = 0; nReqIndex < m_arRequisitions.GetSize(); nReqIndex++) {
		CLabRequisitionDlg *pdlgReq = m_arRequisitions.GetAt(nReqIndex);
		pdlgReq->DeletePendingTodos();
	}
}

// (z.manning 2010-05-13 14:03) - PLID 37405
long CLabRequisitionsTabDlg::GetTotalPendingTodoCount()
{
	long nCount = 0;
	for(int nReqIndex = 0; nReqIndex < m_arRequisitions.GetSize(); nReqIndex++) {
		CLabRequisitionDlg *pdlgReq = m_arRequisitions.GetAt(nReqIndex);
		nCount += pdlgReq->GetPendingTodoCount();
	}
	return nCount;
}

// (z.manning 2010-06-02 13:08) - PLID 38976
char CLabRequisitionsTabDlg::GetNextSpecimenCode()
{
	char chSpecimen = 'A';
		
	//TES 8/10/2011 - PLID 44901 - Due to location permissions, not all specimens may be on our dialog.  So, we'll have to access the database,
	// but first let's check if this user has permission to all locations, since that's the most common scenario, and if it's true, then we
	// don't need to go to data.
	CArray<long,long> arLocations;
	if(PollLabLocationPermissions(arLocations)) {
		//TES 8/10/2011 - PLID 44901 - They can view all locations, so we know all specimens are on the dialog already, so just look there.
		// (z.manning 2010-06-02 13:36) - PLID 38976 - Start with A
		for(int nReqIndex = 0; nReqIndex < m_arRequisitions.GetSize(); nReqIndex++)
		{
			CLabRequisitionDlg *pdlgReq = m_arRequisitions.GetAt(nReqIndex);
			// (z.manning 2010-06-02 13:36) - PLID 38976 - Only cycle through saved requisitions
			if(!pdlgReq->IsNew()) {
				CString strSpecimen = pdlgReq->GetSpecimen();
				if(strSpecimen.GetLength() != 1) {
					// (z.manning 2010-06-02 13:37) - PLID 38976 - If we have a specimen value other than a single
					// character then we can't increment it and they must have modified something so don't try to
					// generate a new one.
					return (char)0;
				}
				else {
					if(strSpecimen.GetAt(0) >= chSpecimen) {
						chSpecimen = strSpecimen.GetAt(0) + 1;
					}
				}
			}
		}
	}
	else {
		//TES 8/10/2011 - PLID 44901 - They can't view all locations, so some specimens may be hidden.  Check the data to calculate the
		// next specimen.
		_RecordsetPtr rsMaxSpecimen = CreateParamRecordset("SELECT Max(Specimen) AS Specimen FROM LabsT WHERE FormNumberTextID = {STRING} "
			"AND Specimen LIKE '[ABCDEFGHIJKLMNOPQRSTUVWXYZ]' AND Deleted = 0", m_pLabEntryDlg->GetFormNumber());
		CString strMaxSpecimen;
		if(!rsMaxSpecimen->eof) {
			strMaxSpecimen = AdoFldString(rsMaxSpecimen, "Specimen", "");
		}
		if(strMaxSpecimen.IsEmpty()) {
			if(m_arRequisitions.GetSize() == 1) {
				//There aren't any unsaved specimens, so use 'A' for this specimen
				return 'A';
			}
			else {
				//The only saved specimens are not single capital letters, so we can't generate a new specimen
				return (char)0;
			}
		}
		else {
			chSpecimen = strMaxSpecimen.GetAt(0) + 1;
		}
	}
		


	// (z.manning 2010-06-02 13:58) - PLID 38976 - Only return single letter codes
	if(chSpecimen >= 'A' && chSpecimen <= 'Z') {
		return chSpecimen;
	}
	else {
		return (char)0;
	}
}
