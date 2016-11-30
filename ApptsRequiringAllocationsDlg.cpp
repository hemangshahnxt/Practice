// ApptsRequiringAllocationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "ApptsRequiringAllocationsDlg.h"
#include "RequiredAllocationsDetailDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 6/12/2008 - PLID 28078 - Created
/////////////////////////////////////////////////////////////////////////////
// CApptsRequiringAllocationsDlg dialog


CApptsRequiringAllocationsDlg::CApptsRequiringAllocationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptsRequiringAllocationsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CApptsRequiringAllocationsDlg)
	//}}AFX_DATA_INIT
}


void CApptsRequiringAllocationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CApptsRequiringAllocationsDlg)
	DDX_Control(pDX, IDC_REQUIREMENT_DAYS, m_nxeRequiredDays);
	DDX_Control(pDX, IDC_REMIND_ME, m_nxbRemindMe);
	DDX_Control(pDX, IDC_EDIT_REQUIREMENT, m_nxbEdit);
	DDX_Control(pDX, IDC_DELETE_REQUIREMENT, m_nxbDelete);
	DDX_Control(pDX, IDC_CLOSE_APPTS_REQUIRING_ALLOCATIONS, m_nxbClose);
	DDX_Control(pDX, IDC_ADD_REQUIREMENT, m_nxbAdd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CApptsRequiringAllocationsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CApptsRequiringAllocationsDlg)
	ON_BN_CLICKED(IDC_ADD_REQUIREMENT, OnAddRequirement)
	ON_BN_CLICKED(IDC_CLOSE_APPTS_REQUIRING_ALLOCATIONS, OnCloseApptsRequiringAllocations)
	ON_BN_CLICKED(IDC_DELETE_REQUIREMENT, OnDeleteRequirement)
	ON_BN_CLICKED(IDC_EDIT_REQUIREMENT, OnEditRequirement)
	ON_BN_CLICKED(IDC_REMIND_ME, OnRemindMe)
	ON_EN_KILLFOCUS(IDC_REQUIREMENT_DAYS, OnKillfocusRequirementDays)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CApptsRequiringAllocationsDlg message handlers

BOOL CApptsRequiringAllocationsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		g_propManager.CachePropertiesInBulk("ApptsRequiringAllocations", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'RemindForApptsWithoutAllocations' OR "
			"Name = 'ApptsRequiringAllocationsDays' "
			")",
			_Q(GetCurrentUserName()));

		m_nxbAdd.AutoSet(NXB_NEW);
		m_nxbEdit.AutoSet(NXB_MODIFY);
		m_nxbDelete.AutoSet(NXB_DELETE);
		m_nxbClose.AutoSet(NXB_CLOSE);

		m_pList = BindNxDataList2Ctrl(IDC_REQUIREMENTS_LIST);

		//TES 6/16/2008 - PLID 30394 - Load their preference about being reminded.
		CheckDlgButton(IDC_REMIND_ME, GetRemotePropertyInt("RemindForApptsWithoutAllocations", 0, 0, GetCurrentUserName(), true));

		//TES 6/16/2008 - PLID 30394 - Load the setting for how far in the future to check.
		SetDlgItemInt(IDC_REQUIREMENT_DAYS, GetRemotePropertyInt("ApptsRequiringAllocationsDays", 14, 0, "<None>", true));
	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CApptsRequiringAllocationsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CApptsRequiringAllocationsDlg)
	ON_EVENT(CApptsRequiringAllocationsDlg, IDC_REQUIREMENTS_LIST, 2 /* SelChanged */, OnSelChangedRequirementsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CApptsRequiringAllocationsDlg, IDC_REQUIREMENTS_LIST, 3 /* DblClickCell */, OnDblClickCellRequirementsList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

using namespace NXDATALIST2Lib;

enum RequirementListColumns {
	rlcID = 0,
	rlcDescription = 1,
};

void CApptsRequiringAllocationsDlg::OnSelChangedRequirementsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		//TES 6/12/2008 - PLID 28078 - Update the Edit/Delete button states.
		IRowSettingsPtr pNewSel(lpNewSel);
		if(pNewSel == NULL) {
			m_nxbEdit.EnableWindow(FALSE);
			m_nxbDelete.EnableWindow(FALSE);
		}
		else {
			m_nxbEdit.EnableWindow(TRUE);
			m_nxbDelete.EnableWindow(TRUE);
		}
	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnSelChangedRequirementsList()");
}

void CApptsRequiringAllocationsDlg::OnAddRequirement() 
{
	try {
		//TES 6/12/2008 - PLID 28078 - Open up the configuration dialog, with an ID of -1.
		CRequiredAllocationsDetailDlg dlg(this);
		dlg.m_nID = -1;
		dlg.m_strDescription = "";
		if(dlg.DoModal() == IDOK) {
			//TES 6/12/2008 - PLID 28078 - They added something, add it to our list, pulling the values from the dialog.
			IRowSettingsPtr pRow = m_pList->GetNewRow();
			pRow->PutValue(rlcID, dlg.m_nID);
			pRow->PutValue(rlcDescription, _bstr_t(dlg.m_strDescription));
			m_pList->AddRowAtEnd(pRow, NULL);
		}
		
	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnAddRequirement()");

}

void CApptsRequiringAllocationsDlg::OnCloseApptsRequiringAllocations() 
{
	CNxDialog::OnOK();	
}

void CApptsRequiringAllocationsDlg::OnDeleteRequirement() 
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow == NULL) {
			MsgBox("Please select a requirement to delete");
			return;
		}

		long nID = VarLong(pRow->GetValue(rlcID));
		CString strDescription = VarString(pRow->GetValue(rlcDescription));

		if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to delete the '%s' requirement?  Any future appointments which "
			"match this criteria will no longer prompt the user to enter an order/allocation.", strDescription)) {
			//TES 8/4/2008 - PLID 28078 - Combined and parameterized.
			ExecuteParamSql("DELETE FROM ApptsRequiringAllocationsDetailT WHERE ParentID = {INT}; "
				"DELETE FROM ApptsRequiringAllocationsT WHERE ID = {INT}", nID, nID);
			m_pList->RemoveRow(pRow);

			OnSelChangedRequirementsList(m_pList->CurSel, m_pList->CurSel);
		}

	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnDeleteRequirement()");
}

void CApptsRequiringAllocationsDlg::OnEditRequirement() 
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow == NULL) {
			MsgBox("Please select a requirement to edit");
			return;
		}

		EditRequirement(pRow);

	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnEditRequirement()");
}

void CApptsRequiringAllocationsDlg::OnRemindMe() 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Set the preference.
		SetRemotePropertyInt("RemindForApptsWithoutAllocations", IsDlgButtonChecked(IDC_REMIND_ME), 0, GetCurrentUserName());
	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnRemindMe()");
}

void CApptsRequiringAllocationsDlg::OnKillfocusRequirementDays() 
{
	try {
		//TES 6/16/2008 - PLID 30394 - Validate.
		int nRequiredDays = GetDlgItemInt(IDC_REQUIREMENT_DAYS);
		if(nRequiredDays <= 0) {
			MsgBox("Please enter a positive number of days for which to require allocations.");
			m_nxeRequiredDays.SetFocus();
		}

		SetRemotePropertyInt("ApptsRequiringAllocationsDays", nRequiredDays, 0, "<None>");

	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnKillfocusRequirementDays()");
}

void CApptsRequiringAllocationsDlg::OnDblClickCellRequirementsList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		//TES 8/4/2008 - PLID 28078 - If they double-clicked on a valid row, edit that requirement.
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			EditRequirement(pRow);
		}
	}NxCatchAll("Error in CApptsRequiringAllocationsDlg::OnDblClickCellRequirementsList()");
}

//TES 8/4/2008 - PLID 28078 - Split out to be called from the DblClick handler as well as the button handler.
void CApptsRequiringAllocationsDlg::EditRequirement(IRowSettingsPtr pRow)
{
	//TES 6/12/2008 - PLID 28078 - Open the configuration dialog.
	CRequiredAllocationsDetailDlg dlg(this);
	dlg.m_nID = VarLong(pRow->GetValue(rlcID));
	dlg.m_strDescription = VarString(pRow->GetValue(rlcDescription));
	if(dlg.DoModal() == IDOK) {
		//TES 6/12/2008 - PLID 28078 - They may have changed the description.
		pRow->PutValue(rlcDescription, _bstr_t(dlg.m_strDescription));
	}
}