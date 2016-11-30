// DeactivateWorkstationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practicerc.h"
#include "DeactivateWorkstationsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CDeactivateWorkstationsDlg dialog


CDeactivateWorkstationsDlg::CDeactivateWorkstationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDeactivateWorkstationsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeactivateWorkstationsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDeactivateWorkstationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeactivateWorkstationsDlg)
	DDX_Control(pDX, IDC_DEACTIVATE_WORKSTATION, m_nxbDeactivateWorkstation);
	DDX_Control(pDX, IDC_DEACTIVATE_WORKSTATIONS_LABEL, m_nxstaticDeactivateWorkstationsLabel);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeactivateWorkstationsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CDeactivateWorkstationsDlg)
	ON_BN_CLICKED(IDC_DEACTIVATE_WORKSTATION, OnDeactivateWorkstation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeactivateWorkstationsDlg message handlers

BOOL CDeactivateWorkstationsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_pUsedList = BindNxDataListCtrl(this, IDC_USED_WORKSTATIONS, NULL, false);
	m_pInactiveList = BindNxDataListCtrl(this, IDC_INACTIVE_WORKSTATIONS, NULL, false);
	
	m_btnClose.AutoSet(NXB_CLOSE);

	CStringArray saUsedWorkstations, saUsedIpads;
	g_pLicense->GetUsedWorkstations(saUsedWorkstations);
	//TES 11/5/2007 - PLID 27978 - VS2008 - for() loops
	int i = 0;
	for(i = 0; i < saUsedWorkstations.GetSize(); i++) {
		IRowSettingsPtr pRow = m_pUsedList->GetRow(-1);
		CString strWs = saUsedWorkstations.GetAt(i);
		CString strFullWs = strWs;

		strWs.Replace("Practice.mde", "");
		pRow->PutValue(ulcText, _bstr_t(strWs));
		pRow->PutValue(ulcIsIpad, g_cvarFalse);
		pRow->PutValue(ulcFullText, _bstr_t(strFullWs));
		
		m_pUsedList->AddRow(pRow);
	}

	// (z.manning 2012-06-14 11:12) - PLID 50975 - Also show iPads
	g_pLicense->GetUsedIpads(saUsedIpads);
	for(i = 0; i < saUsedIpads.GetSize(); i++)
	{
		CString strIpad = saUsedIpads.GetAt(i);
		IRowSettingsPtr pRow = m_pUsedList->GetRow(-1);
		pRow->PutValue(ulcText, _bstr_t(strIpad));
		pRow->PutValue(ulcIsIpad, g_cvarTrue);
		pRow->PutValue(ulcFullText, _bstr_t(strIpad));
		m_pUsedList->AddRow(pRow);
	}

	CStringArray saInactiveWorkstations;
	g_pLicense->GetInactiveWorkstations(saInactiveWorkstations);
	for(i = 0; i < saInactiveWorkstations.GetSize(); i++) {
		IRowSettingsPtr pRow = m_pInactiveList->GetRow(-1);
		CString strWs = saInactiveWorkstations.GetAt(i);
		strWs.Replace("Practice.mde", "");
		pRow->PutValue(ilcText, _bstr_t(strWs));
		m_pInactiveList->AddRow(pRow);
	}

	m_nxbDeactivateWorkstation.AutoSet(NXB_RIGHT);
	
	SetDlgItemText(IDC_DEACTIVATE_WORKSTATIONS_LABEL, "Deactivating a workstation will free up a license for use by another computer.  However, once a workstation has been deactivated, it can NEVER be reactivated, and Practice can NEVER be run from that workstation again.  ONLY deactivate workstations which will never use Practice again!");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDeactivateWorkstationsDlg::OnDeactivateWorkstation() 
{
	if(m_pUsedList->CurSel == -1) return;

	// (j.kuziel 2012-05-02 10:29) - PLID 50139 - Retrieve the full workstation name rather than just appending 'Practice.mde'.
	CString strWorkstationFull = VarString(m_pUsedList->GetValue(m_pUsedList->CurSel, ulcFullText));
	CString strWorkstationPath = VarString(m_pUsedList->GetValue(m_pUsedList->CurSel, ulcText));
	BOOL bIsIpad = VarBool(m_pUsedList->GetValue(m_pUsedList->CurSel, ulcIsIpad));
	if(IDYES == MsgBox(MB_YESNO, "Are you SURE you wish to deactivate the workstation '%s'?  You will NEVER be able to log in from '%s' again!", strWorkstationPath, strWorkstationPath)) {
		if(g_pLicense->DeactivateWorkstation(strWorkstationFull, bIsIpad)) {
			m_pInactiveList->TakeCurrentRow(m_pUsedList);
			OnSelChangedUsedWorkstations(m_pUsedList->CurSel);
		}
	}
}

BEGIN_EVENTSINK_MAP(CDeactivateWorkstationsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDeactivateWorkstationsDlg)
	ON_EVENT(CDeactivateWorkstationsDlg, IDC_USED_WORKSTATIONS, 2 /* SelChanged */, OnSelChangedUsedWorkstations, VTS_I4)
	ON_EVENT(CDeactivateWorkstationsDlg, IDC_USED_WORKSTATIONS, 3 /* DblClickCell */, OnDblClickCellUsedWorkstations, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CDeactivateWorkstationsDlg::OnSelChangedUsedWorkstations(long nNewSel) 
{
	GetDlgItem(IDC_DEACTIVATE_WORKSTATION)->EnableWindow(nNewSel != -1);
}

void CDeactivateWorkstationsDlg::OnDblClickCellUsedWorkstations(long nRowIndex, short nColIndex) 
{
	OnDeactivateWorkstation();
}
