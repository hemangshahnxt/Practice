//(c.copits 2011-06-16) PLID 28219 - Use guided dialog to edit licensing options

// SupportChangeLicenseWizardMasterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"

#include "SupportChangeLicenseWizardMasterDlg.h"
#include "SupportChangeLicenseWizardSheet.h"

#include "SupportChangeLicenseTasksSheet.h"
#include "SupportChangeLicenseAddSheet.h"
#include "SupportChangeLicenseConfirmSheet.h"
#include "SupportChangeLicenseReturnSheet.h"
#include "SupportChangeLicenseDeclineSupportSheet.h"
#include "SupportChangeLicenseSwapPDANexSync.h"
#include "SupportChangeLicenseSwapPalmNexSync.h"

// CSupportChangeLicenseWizardMasterDlg dialog

IMPLEMENT_DYNAMIC(CSupportChangeLicenseWizardMasterDlg, CNxDialog)

CSupportChangeLicenseWizardMasterDlg::CSupportChangeLicenseWizardMasterDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSupportChangeLicenseWizardMasterDlg::IDD, pParent)
{
		m_nActiveSheetIndex = 0;

}

CSupportChangeLicenseWizardMasterDlg::~CSupportChangeLicenseWizardMasterDlg()
{
	try {
			for(int i = 0; i < m_arypWizardSheets.GetSize(); i++)
			{
				if(m_arypWizardSheets.GetAt(i) != NULL)
				{
					m_arypWizardSheets.GetAt(i)->DestroyWindow();
					delete m_arypWizardSheets.GetAt(i);
				}
			}
			m_arypWizardSheets.RemoveAll();

	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseWizardMasterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSupportChangeLicenseWizardMasterDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NEXT, &CSupportChangeLicenseWizardMasterDlg::OnNext)
	ON_BN_CLICKED(IDC_BACK, &CSupportChangeLicenseWizardMasterDlg::OnBack)
	ON_BN_CLICKED(IDC_FINISH, &CSupportChangeLicenseWizardMasterDlg::OnFinish)
END_MESSAGE_MAP()


// CSupportChangeLicenseWizardMasterDlg message handlers

BOOL CSupportChangeLicenseWizardMasterDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_FINISH))->AutoSet(NXB_OK);

		m_nLicensingChangeType = loNone;
		m_nOldLicensingChangeType = loNone;

		ResetLicenseValues();

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Begin adding sheets to the NexForms import wizard

		CSupportChangeLicenseTasksSheet *pdlgTasksSheet = new CSupportChangeLicenseTasksSheet(this);
		m_arypWizardSheets.Add(pdlgTasksSheet);

		CSupportChangeLicenseAddSheet *pdlgAddSheet = new CSupportChangeLicenseAddSheet(this);
		m_arypWizardSheets.Add(pdlgAddSheet);

		CSupportChangeLicenseDeclineSupportSheet *pdlgDeclineSheet = new CSupportChangeLicenseDeclineSupportSheet(this);
		m_arypWizardSheets.Add(pdlgDeclineSheet);

		CSupportChangeLicenseReturnSheet *pdlgReturnSheet = new CSupportChangeLicenseReturnSheet(this);
		m_arypWizardSheets.Add(pdlgReturnSheet);

		CSupportChangeLicenseSwapPDANexSync *pdlgSwapPDANexSync = new CSupportChangeLicenseSwapPDANexSync(this);
		m_arypWizardSheets.Add(pdlgSwapPDANexSync);

		CSupportChangeLicenseSwapPalmNexSync *pdlgSwapPalmNexSync = new CSupportChangeLicenseSwapPalmNexSync(this);
		m_arypWizardSheets.Add(pdlgSwapPalmNexSync);
	
		CSupportChangeLicenseConfirmSheet *pdlgConfirmSheet = new CSupportChangeLicenseConfirmSheet(this);
		m_arypWizardSheets.Add(pdlgConfirmSheet);

		// End adding sheets to the NexForms import wizard
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		for(int i = 0; i < m_arypWizardSheets.GetSize(); i++)
		{
			CSupportChangeLicenseWizardSheet *pdlgSheet = m_arypWizardSheets.GetAt(i);
			pdlgSheet->Create(pdlgSheet->GetDialogID(), this);
		}

		SetActiveSheet(0);

	} NxCatchAll(__FUNCTION__);

	return TRUE;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}

void CSupportChangeLicenseWizardMasterDlg::SetActiveSheet(int nSheetIndex)
{
	try {
		if(nSheetIndex < 0 || nSheetIndex >= m_arypWizardSheets.GetSize()) {
			AfxThrowNxException("Sheet index out of range: %li", nSheetIndex);
		}

		m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->ShowWindow(SW_HIDE);

		CRect rcSheet;
		GetDlgItem(IDC_SHEET_PLACEHOLDER)->GetClientRect(rcSheet);
		m_arypWizardSheets.GetAt(nSheetIndex)->SetWindowPos(NULL, rcSheet.left, rcSheet.top, rcSheet.Width(), rcSheet.Height(), SWP_SHOWWINDOW | SWP_NOZORDER);
		m_nActiveSheetIndex = nSheetIndex;
		
		// Check whether we need to reset values if user selects a fundamentally different activity
		if (m_nLicensingChangeType == loNone) {
			m_nOldLicensingChangeType = loNone;
		}
		else {

			if (m_nOldLicensingChangeType != m_nLicensingChangeType) {

				// "Activity" sheets only
				if (m_arypWizardSheets.GetAt(nSheetIndex)->IsActivity()) { // Sheet that can actually change values (as opposed to confirm sheet)
					m_arypWizardSheets.GetAt(nSheetIndex)->SetChangeFromAnotherActivity(true);

				}

			}
			else {
				m_arypWizardSheets.GetAt(nSheetIndex)->SetChangeFromAnotherActivity(false);
			}

		}
		
		m_arypWizardSheets.GetAt(nSheetIndex)->Load();

		RefreshButtons();

	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseWizardMasterDlg::OnBack() 
{
	//CWaitCursor wc;
	try
	{
		if(m_nActiveSheetIndex >= 1)
		{
			SetActiveSheet(m_nActiveSheetIndex - 1);

			while(m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->m_bSkipSheet && m_nActiveSheetIndex >= 1) {
				SetActiveSheet(m_nActiveSheetIndex - 1);
			}
		}
		else
		{
			ASSERT(FALSE);
		}

	} NxCatchAll(__FUNCTION__);
	
}

void CSupportChangeLicenseWizardMasterDlg::OnNext()
{
	//CWaitCursor wc;
	try
	{
		if(!m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->Validate()) {
			return;
		}

		if(m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1)
		{
			SetActiveSheet(m_nActiveSheetIndex + 1);
			while(m_arypWizardSheets.GetAt(m_nActiveSheetIndex)->m_bSkipSheet && m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1) {
				SetActiveSheet(m_nActiveSheetIndex + 1);
			}
		}
		else
		{
			ASSERT(FALSE);
		}

	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseWizardMasterDlg::OnCancel()
{
	try
	{
		//SetEvent(m_ImportInfo.m_hevDestroying);
		CDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseWizardMasterDlg::RefreshButtons()
{
	try {
		if(m_nActiveSheetIndex < m_arypWizardSheets.GetSize() - 1)
		{
			GetDlgItem(IDC_NEXT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_FINISH)->ShowWindow(SW_HIDE);
		}
		else
		{
			GetDlgItem(IDC_NEXT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FINISH)->ShowWindow(SW_SHOW);
		}

		if(m_nActiveSheetIndex <= 0)
		{
			GetDlgItem(IDC_BACK)->EnableWindow(FALSE);
		}
		else
		{
			if(m_arypWizardSheets.GetAt(m_nActiveSheetIndex - 1)->m_bNoReturn) {
				GetDlgItem(IDC_BACK)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_BACK)->EnableWindow(TRUE);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CSupportChangeLicenseWizardMasterDlg::OnFinish()
{
	try {

		if (AnyChanges()) {
			//MessageBox("Changes Successful!", "Done");
		}
		else {
			MessageBox("No changes to make!", "Done");
		}
		EndDialog(IDOK);

	} NxCatchAll(__FUNCTION__);
}


BOOL CSupportChangeLicenseWizardMasterDlg::AnyChanges()
{

	if (
			GetBLicenses() == GetBNewLicenses() &&
			GetBConc() == GetBNewConc() &&
			GetBDoctors() == GetBNewDoctors() &&
			GetBNexSync() == GetBNewNexSync() &&
			GetBPalm() == GetBNewPalm() &&
			GetBNexPDA() == GetBNewNexPDA() &&
			GetBDatabases() == GetBNewDatabases() &&
			GetBEMRProv() == GetBNewEMRProv() &&
			GetPLicenses() == GetPNewLicenses() &&
			GetPDoctors() == GetPNewDoctors() &&
			GetPNexSync() == GetPNewNexSync() &&
			GetPPalm() == GetPNewPalm() &&
			GetPNexPDA() == GetPNewNexPDA() &&
			GetPDatabases() == GetPNewDatabases() &&
			GetPEMRProv() == GetPNewEMRProv()
		) {
		return FALSE;
	}
	else {
		return TRUE;
	}

}

void CSupportChangeLicenseWizardMasterDlg::SetLicenseChangeType(int nType)
{
	m_nLicensingChangeType = nType;
}

int CSupportChangeLicenseWizardMasterDlg::GetLicenseChangeType()
{
	return m_nLicensingChangeType;
}
// Set initial bought
void CSupportChangeLicenseWizardMasterDlg::SetBLicenses(long nLicenses)
{
	m_nBLicenses = nLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetBConc(long nConc)
{
	m_nBConc = nConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetBDoctors(long nDoctors)
{
	m_nBDoctors = nDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNexSync(long nNexSync)
{
	m_nBNexSync = nNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetBPalm(long nPalm)
{
	m_nBPalm = nPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNexPDA(long nNexPDA)
{
	m_nBNexPDA = nNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetBDatabases(long nDatabases)
{
	m_nBDatabases = nDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetBEMRProv(long nEMRProv)
{
	m_nBEMRProv = nEMRProv;
}
// Get new bought
long CSupportChangeLicenseWizardMasterDlg::GetBNewLicenses()
{
	return m_nBNewLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewConc()
{
	return m_nBNewConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewDoctors()
{
	return m_nBNewDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewNexSync()
{
	return m_nBNewNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewPalm()
{
	return m_nBNewPalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewNexPDA()
{
	return m_nBNewNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewDatabases()
{
	return m_nBNewDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNewEMRProv()
{
	return m_nBNewEMRProv;
}
// Set new bought
void CSupportChangeLicenseWizardMasterDlg::SetBNewLicenses(long nNewLicenses)
{
	m_nBNewLicenses = nNewLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNewConc(long nNewConc)
{
	m_nBNewConc = nNewConc;
}


void CSupportChangeLicenseWizardMasterDlg::SetBNewDoctors(long nNewDoctors)
{
	m_nBNewDoctors = nNewDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNewNexSync(long nNewNexSync)
{
	m_nBNewNexSync = nNewNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNewPalm(long nNewPalm)
{
	m_nBNewPalm = nNewPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNewNexPDA(long nNewNexPDA)
{
	m_nBNewNexPDA = nNewNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNewDatabases(long nNewDatabases)
{
	m_nBNewDatabases = nNewDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetBNewEMRProv(long nNewEMRProv)
{
	m_nBNewEMRProv = nNewEMRProv;
}
// Get initial bought
long CSupportChangeLicenseWizardMasterDlg::GetBLicenses()
{
	return m_nBLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetBConc()
{
	return m_nBConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetBDoctors()
{
	return m_nBDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNexSync()
{
	return m_nBNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetBPalm()
{
	return m_nBPalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetBNexPDA()
{
	return m_nBNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetBDatabases()
{
	return m_nBDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetBEMRProv()
{
	return m_nBEMRProv;
}
// Get change in bought
long CSupportChangeLicenseWizardMasterDlg::GetBChangeLicenses()
{
	return m_nBChangeLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangeConc()
{
	return m_nBChangeConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangeDoctors()
{
	return m_nBChangeDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangeNexSync()
{
	return m_nBChangeNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangePalm()
{
	return m_nBChangePalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangeNexPDA()
{
	return m_nBChangeNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangeDatabases()
{
	return m_nBChangeDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetBChangeEMRProv()
{
	return m_nBChangeEMRProv;
}
// Set change in bought
void CSupportChangeLicenseWizardMasterDlg::SetBChangeLicenses(long nLicenses)
{
	m_nBChangeLicenses = nLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetBChangeConc(long nConc)
{
	m_nBChangeConc = nConc;
}


void CSupportChangeLicenseWizardMasterDlg::SetBChangeDoctors(long nDoctors)
{
	m_nBChangeDoctors = nDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetBChangeNexSync(long nNexSync)
{
	m_nBChangeNexSync = nNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetBChangePalm(long nPalm)
{
	m_nBChangePalm = nPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetBChangeNexPDA(long nNexPDA)
{
	m_nBChangeNexPDA = nNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetBChangeDatabases(long nDatabases)
{
	m_nBChangeDatabases = nDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetBChangeEMRProv(long nEMRProv)
{
	m_nBChangeEMRProv = nEMRProv;
}
// Pending
void CSupportChangeLicenseWizardMasterDlg::SetPLicenses(long nLicenses)
{
	m_nPLicenses = nLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetPConc(long nConc)
{
	m_nPConc = nConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetPDoctors(long nDoctors)
{
	m_nPDoctors = nDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNexSync(long nNexSync)
{
	m_nPNexSync = nNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetPPalm(long nPalm)
{
	m_nPPalm = nPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNexPDA(long nNexPDA)
{
	m_nPNexPDA = nNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetPDatabases(long nDatabases)
{
	m_nPDatabases = nDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetPEMRProv(long nEMRProv)
{
	m_nPEMRProv = nEMRProv;
}
// Pending
long CSupportChangeLicenseWizardMasterDlg::GetPLicenses()
{
	return m_nPLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetPConc()
{
	return m_nPConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetPDoctors()
{
	return m_nPDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNexSync()
{
	return m_nPNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetPPalm()
{
	return m_nPPalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNexPDA()
{
	return m_nPNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetPDatabases()
{
	return m_nPDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetPEMRProv()
{
	return m_nPEMRProv;
}
// Set change in pending
void CSupportChangeLicenseWizardMasterDlg::SetPChangeLicenses(long nLicenses)
{
	m_nPChangeLicenses = nLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangeConc(long nConc)
{
	m_nPChangeConc = nConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangeDoctors(long nDoctors)
{
	m_nPChangeDoctors = nDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangeNexSync(long nNexSync)
{
	m_nPChangeNexSync = nNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangePalm(long nPalm)
{
	m_nPChangePalm = nPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangeNexPDA(long nNexPDA)
{
	m_nPChangeNexPDA = nNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangeDatabases(long nDatabases)
{
	m_nPChangeDatabases = nDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetPChangeEMRProv(long nEMRProv)
{
	m_nPChangeEMRProv = nEMRProv;
}
// Get change in pending
long CSupportChangeLicenseWizardMasterDlg::GetPChangeLicenses()
{
	return m_nPChangeLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangeConc()
{
	return m_nPChangeConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangeDoctors()
{
	return m_nPChangeDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangeNexSync()
{
	return m_nPChangeNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangePalm()
{
	return m_nPChangePalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangeNexPDA()
{
	return m_nPChangeNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangeDatabases()
{
	return m_nPChangeDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetPChangeEMRProv()
{
	return m_nPChangeEMRProv;
}
// Set new pending
void CSupportChangeLicenseWizardMasterDlg::SetPNewLicenses(long nNewLicenses)
{
	m_nPNewLicenses = nNewLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewConc(long nNewConc)
{
	m_nPNewConc = nNewConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewDoctors(long nNewDoctors)
{
	m_nPNewDoctors = nNewDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewNexSync(long nNewNexSync)
{
	m_nPNewNexSync = nNewNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewPalm(long nNewPalm)
{
	m_nPNewPalm = nNewPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewNexPDA(long nNewNexPDA)
{
	m_nPNewNexPDA = nNewNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewDatabases(long nNewDatabases)
{
	m_nPNewDatabases = nNewDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetPNewEMRProv(long nNewEMRProv)
{
	m_nPNewEMRProv = nNewEMRProv;
}
// Get new pending
long CSupportChangeLicenseWizardMasterDlg::GetPNewLicenses()
{
	return m_nPNewLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewConc()
{
	return m_nPNewConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewDoctors()
{
	return m_nPNewDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewNexSync()
{
	return m_nPNewNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewPalm()
{
	return m_nPNewPalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewNexPDA()
{
	return m_nPNewNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewDatabases()
{
	return m_nPNewDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetPNewEMRProv()
{
	return m_nPNewEMRProv;
}

// Set initial in-use
void CSupportChangeLicenseWizardMasterDlg::SetIULicenses(long nLicenses)
{
	m_nIULicenses = nLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUConc(long nConc)
{
	m_nIUConc = nConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUDoctors(long nDoctors)
{
	m_nIUDoctors = nDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNexSync(long nNexSync)
{
	m_nIUNexSync = nNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUPalm(long nPalm)
{
	m_nIUPalm = nPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNexPDA(long nNexPDA)
{
	m_nIUNexPDA = nNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUDatabases(long nDatabases)
{
	m_nIUDatabases = nDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUEMRProv(long nEMRProv)
{
	m_nIUEMRProv = nEMRProv;
}

// Get new in-use
long CSupportChangeLicenseWizardMasterDlg::GetIUNewLicenses()
{
	return m_nIUNewLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewConc()
{
	return m_nIUNewConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewDoctors()
{
	return m_nIUNewDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewNexSync()
{
	return m_nIUNewNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewPalm()
{
	return m_nIUNewPalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewNexPDA()
{
	return m_nIUNewNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewDatabases()
{
	return m_nIUNewDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNewEMRProv()
{
	return m_nIUNewEMRProv;
}

// Set new in-use
void CSupportChangeLicenseWizardMasterDlg::SetIUNewLicenses(long nNewLicenses)
{
	m_nIUNewLicenses = nNewLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewConc(long nNewConc)
{
	m_nIUNewConc = nNewConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewDoctors(long nNewDoctors)
{
	m_nIUNewDoctors = nNewDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewNexSync(long nNewNexSync)
{
	m_nIUNewNexSync = nNewNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewPalm(long nNewPalm)
{
	m_nIUNewPalm = nNewPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewNexPDA(long nNewNexPDA)
{
	m_nIUNewNexPDA = nNewNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewDatabases(long nNewDatabases)
{
	m_nIUNewDatabases = nNewDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUNewEMRProv(long nNewEMRProv)
{
	m_nIUNewEMRProv = nNewEMRProv;
}

// Get initial in-use
long CSupportChangeLicenseWizardMasterDlg::GetIULicenses()
{
	return m_nIULicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUConc()
{
	return m_nIUConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUDoctors()
{
	return m_nIUDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNexSync()
{
	return m_nIUNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUPalm()
{
	return m_nIUPalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUNexPDA()
{
	return m_nIUNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUDatabases()
{
	return m_nIUDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUEMRProv()
{
	return m_nIUEMRProv;
}

// Get change in in-use
long CSupportChangeLicenseWizardMasterDlg::GetIUChangeLicenses()
{
	return m_nIUChangeLicenses;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangeConc()
{
	return m_nIUChangeConc;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangeDoctors()
{
	return m_nIUChangeDoctors;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangeNexSync()
{
	return m_nIUChangeNexSync;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangePalm()
{
	return m_nIUChangePalm;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangeNexPDA()
{
	return m_nIUChangeNexPDA;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangeDatabases()
{
	return m_nIUChangeDatabases;
}

long CSupportChangeLicenseWizardMasterDlg::GetIUChangeEMRProv()
{
	return m_nIUChangeEMRProv;
}

// Set change in in-use
void CSupportChangeLicenseWizardMasterDlg::SetIUChangeLicenses(long nLicenses)
{
	m_nIUChangeLicenses = nLicenses;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangeConc(long nConc)
{
	m_nIUChangeConc = nConc;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangeDoctors(long nDoctors)
{
	m_nIUChangeDoctors = nDoctors;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangeNexSync(long nNexSync)
{
	m_nIUChangeNexSync = nNexSync;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangePalm(long nPalm)
{
	m_nIUChangePalm = nPalm;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangeNexPDA(long nNexPDA)
{
	m_nIUChangeNexPDA = nNexPDA;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangeDatabases(long nDatabases)
{
	m_nIUChangeDatabases = nDatabases;
}

void CSupportChangeLicenseWizardMasterDlg::SetIUChangeEMRProv(long nEMRProv)
{
	m_nIUChangeEMRProv = nEMRProv;
}

void CSupportChangeLicenseWizardMasterDlg::ResetLicenseValues()
{
		m_nBNewLicenses = m_nBLicenses;
		m_nBNewConc = m_nBConc;
		m_nBNewDoctors = m_nBDoctors;
		m_nBNewNexSync = m_nBNexSync;
		m_nBNewPalm = m_nBPalm;
		m_nBNewNexPDA = m_nBNexPDA;
		m_nBNewDatabases = m_nBDatabases;
		m_nBNewEMRProv = m_nBEMRProv;

		m_nPNewLicenses = m_nPLicenses;
		m_nPNewConc = m_nPConc;
		m_nPNewDoctors = m_nPDoctors;
		m_nPNewNexSync = m_nPNexSync;
		m_nPNewPalm = m_nPPalm;
		m_nPNewNexPDA = m_nPNexPDA;
		m_nPNewDatabases = m_nPDatabases;
		m_nPNewEMRProv = m_nPEMRProv;

		m_nIUNewLicenses = m_nIULicenses;
		m_nIUNewConc = m_nIUConc;
		m_nIUNewDoctors = m_nIUDoctors;
		m_nIUNewNexSync = m_nIUNexSync;
		m_nIUNewPalm = m_nIUPalm;
		m_nIUNewNexPDA = m_nIUNexPDA;
		m_nIUNewDatabases = m_nIUDatabases;
		m_nIUNewEMRProv = m_nIUEMRProv;

		m_nBChangeLicenses = 0;
		m_nBChangeConc = 0;
		m_nBChangeDoctors = 0;
		m_nBChangeNexSync = 0;
		m_nBChangePalm = 0;
		m_nBChangeNexPDA = 0;
		m_nBChangeDatabases = 0;
		m_nBChangeEMRProv = 0;

		m_nPChangeLicenses = 0;
		m_nPChangeConc = 0;
		m_nPChangeDoctors = 0;
		m_nPChangeNexSync = 0;
		m_nPChangePalm = 0;
		m_nPChangeNexPDA = 0;
		m_nPChangeDatabases = 0;
		m_nPChangeEMRProv = 0;

		m_nIUChangeLicenses = 0;
		m_nIUChangeConc = 0;
		m_nIUChangeDoctors = 0;
		m_nIUChangeNexSync = 0;
		m_nIUChangePalm = 0;
		m_nIUChangeNexPDA = 0;
		m_nIUChangeDatabases = 0;
		m_nIUChangeEMRProv = 0;
}