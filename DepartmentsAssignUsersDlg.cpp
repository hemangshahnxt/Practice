// DepartmentsAssignUsersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DepartmentsAssignUsersDlg.h"
#include "AttendanceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


using namespace NXDATALIST2Lib;

// (z.manning, 11/16/2007) - This is shared between both the selected and unselected user lists.
enum UserSelectionColumns
{
	uscID = 0,
	uscUserName,
};

enum EDepartmentFilterColumns
{
	dfcID = 0,
	dfcName,
};

/////////////////////////////////////////////////////////////////////////////
// CDepartmentsAssignUsersDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29152 - Created

CDepartmentsAssignUsersDlg::CDepartmentsAssignUsersDlg(CArray<AttendanceDepartment,AttendanceDepartment&> &aryDepartments, AttendanceInfo *pInfo, CConfigureDepartmentsDlg* pdlgDepartment)
	: CNxDialog(CDepartmentsAssignUsersDlg::IDD, pdlgDepartment)
{
	//{{AFX_DATA_INIT(CDepartmentsAssignUsersDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nDefaultDepartmentID = -1;
	m_bChanged = FALSE;
	m_nLastSelectedDepartmentID = -1;
	m_pAttendanceInfo = pInfo;
	m_aryDepartments.Append(aryDepartments);
}


void CDepartmentsAssignUsersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDepartmentsAssignUsersDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SELECT_ONE_USER, m_btnSelectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_USERS, m_btnSelectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_USER, m_btnUnselectOne);
	DDX_Control(pDX, IDC_UNSELECT_ALL_USERS, m_btnUnselectAll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDepartmentsAssignUsersDlg, CNxDialog)
	//{{AFX_MSG_MAP(CDepartmentsAssignUsersDlg)
	ON_BN_CLICKED(IDC_SELECT_ALL_USERS, OnSelectAllUsers)
	ON_BN_CLICKED(IDC_SELECT_ONE_USER, OnSelectOneUser)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_USERS, OnUnselectAllUsers)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_USER, OnUnselectOneUser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CDepartmentsAssignUsersDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CDepartmentsAssignUsersDlg)
	ON_EVENT(CDepartmentsAssignUsersDlg, IDC_USERS_IN_DEPARTMENT, 3 /* DblClickCell */, OnDblClickCellUsersInDepartment, VTS_DISPATCH VTS_I2)
	ON_EVENT(CDepartmentsAssignUsersDlg, IDC_USERS_NOT_IN_DEPARTMENT, 3 /* DblClickCell */, OnDblClickCellUsersNotInDepartment, VTS_DISPATCH VTS_I2)
	ON_EVENT(CDepartmentsAssignUsersDlg, IDC_DEPARTMENT_FILTER, 16 /* SelChosen */, OnSelChosenDepartmentFilter, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDepartmentsAssignUsersDlg message handlers

BOOL CDepartmentsAssignUsersDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnUnselectOne.AutoSet(NXB_LEFT);
		m_btnUnselectAll.AutoSet(NXB_LLEFT);
		m_btnSelectOne.AutoSet(NXB_RIGHT);
		m_btnSelectAll.AutoSet(NXB_RRIGHT);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pdlDepartments = BindNxDataList2Ctrl(this, IDC_DEPARTMENT_FILTER, GetRemoteData(), false);
		m_pdlSelectedUsers = BindNxDataList2Ctrl(this, IDC_USERS_IN_DEPARTMENT, GetRemoteData(), false);
		m_pdlUnselectedUsers = BindNxDataList2Ctrl(this, IDC_USERS_NOT_IN_DEPARTMENT, GetRemoteData(), false);
		
		// (z.manning, 11/16/2007) - Load departments from the array of departments that was passed in.
		for(int nDeptIndex = 0; nDeptIndex < m_aryDepartments.GetSize(); nDeptIndex++)
		{
			IRowSettingsPtr pNewRow = m_pdlDepartments->GetNewRow();
			pNewRow->PutValue(dfcID, m_aryDepartments.GetAt(nDeptIndex).nID);
			pNewRow->PutValue(dfcName, _bstr_t(m_aryDepartments.GetAt(nDeptIndex).strName));
			m_pdlDepartments->AddRowSorted(pNewRow, NULL);
		}
		// (z.manning, 11/16/2007) - If we don't have any departments, then this whole dialog is pretty pointless.
		if(m_pdlDepartments->GetRowCount() == 0) {
			MessageBox("You must add at least one department before assigning users.");
			EndDialog(IDCANCEL);
			return TRUE;
		}
		if(m_nDefaultDepartmentID > 0) {
			m_pdlDepartments->SetSelByColumn(dfcID, m_nDefaultDepartmentID);
		}
		if(m_pdlDepartments->GetCurSel() == NULL) {
			m_pdlDepartments->PutCurSel(m_pdlDepartments->GetFirstRow());
		}
		if(m_pdlDepartments->GetCurSel() != NULL) {
			m_nLastSelectedDepartmentID = VarLong(m_pdlDepartments->GetCurSel()->GetValue(dfcID));
		}
		else {
			ASSERT(FALSE);
		}

		Load();
	
	}NxCatchAll("CDepartmentsAssignUsersDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDepartmentsAssignUsersDlg::Load()
{
	m_pdlSelectedUsers->Clear();
	m_pdlUnselectedUsers->Clear();
	long nDepartmentID = VarLong(m_pdlDepartments->GetCurSel()->GetValue(dfcID));
	// (z.manning, 11/16/2007) - Load the users.
	for(int nUserIndex = 0; nUserIndex < m_pAttendanceInfo->m_arypAttendanceUsers.GetSize(); nUserIndex++)
	{
		AttendanceUser *pUser = m_pAttendanceInfo->m_arypAttendanceUsers.GetAt(nUserIndex);
		if(pUser->m_nUserID > 0)
		{
			IRowSettingsPtr pNewRow;
			if(pUser->IsInDepartment(nDepartmentID)) {
				pNewRow = m_pdlSelectedUsers->GetNewRow();
				pNewRow->PutValue(uscID, pUser->m_nUserID);
				pNewRow->PutValue(uscUserName, _bstr_t(pUser->GetFullName()));
				m_pdlSelectedUsers->AddRowSorted(pNewRow, NULL);
			}
			else {
				pNewRow = m_pdlUnselectedUsers->GetNewRow();
				pNewRow->PutValue(uscID, pUser->m_nUserID);
				pNewRow->PutValue(uscUserName, _bstr_t(pUser->GetFullName()));
				m_pdlUnselectedUsers->AddRowSorted(pNewRow, NULL);
			}
		}
	}

	m_bChanged = FALSE;
}

void CDepartmentsAssignUsersDlg::OnSelectAllUsers() 
{
	try
	{
		while(m_pdlUnselectedUsers->GetRowCount() > 0)
		{
			MoveRowToSelectedList(m_pdlUnselectedUsers->GetFirstRow());
		}

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnSelectAllUsers");
}

void CDepartmentsAssignUsersDlg::OnSelectOneUser() 
{
	try
	{
		MoveRowToSelectedList(m_pdlUnselectedUsers->GetCurSel());

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnSelectOneUser");
}

void CDepartmentsAssignUsersDlg::OnUnselectAllUsers() 
{
	try
	{		
		while(m_pdlSelectedUsers->GetRowCount() > 0)
		{
			MoveRowToUnselectedList(m_pdlSelectedUsers->GetTopRow());
		}

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnUnselectAllUsers");
}

void CDepartmentsAssignUsersDlg::OnUnselectOneUser() 
{
	try
	{
		MoveRowToUnselectedList(m_pdlSelectedUsers->GetCurSel());

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnUnselectOneUser");
}

void CDepartmentsAssignUsersDlg::MoveRowToSelectedList(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	m_bChanged = TRUE;
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
	m_pdlSelectedUsers->TakeRowAddSorted(pRow);
}

void CDepartmentsAssignUsersDlg::MoveRowToUnselectedList(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	m_bChanged = TRUE;
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
	m_pdlUnselectedUsers->TakeRowAddSorted(pRow);
}

void CDepartmentsAssignUsersDlg::OnDblClickCellUsersInDepartment(LPDISPATCH lpRow, short nColIndex) 
{
	try
	{
		MoveRowToUnselectedList(lpRow);

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnDblClickCellUsersInDepartment");
}

void CDepartmentsAssignUsersDlg::OnDblClickCellUsersNotInDepartment(LPDISPATCH lpRow, short nColIndex) 
{
	try
	{
		MoveRowToSelectedList(lpRow);

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnDblClickCellUsersNotInDepartment");
}

void CDepartmentsAssignUsersDlg::SetDefaultDepartmentID(long nDepartmentID)
{
	m_nDefaultDepartmentID = nDepartmentID;
}

void CDepartmentsAssignUsersDlg::OnSelChosenDepartmentFilter(LPDISPATCH lpRow) 
{
	try
	{
		// (z.manning, 11/16/2007) - Take focus away from the dropdown to avoid annoying mouse wheel scrolling issues.
		GetDlgItem(IDC_USERS_NOT_IN_DEPARTMENT)->SetFocus();

		IRowSettingsPtr pOldSel = m_pdlDepartments->FindByColumn(dfcID, m_nLastSelectedDepartmentID, NULL, VARIANT_FALSE);
		if(pOldSel == NULL) {
			ThrowNxException("Unable to find previously selected department row (ID = %li)", m_nLastSelectedDepartmentID);
		}

		// (z.manning, 11/16/2007) - If the previous department changed, then prompt to save it.
		if(m_bChanged && pOldSel != NULL)
		{
			int nResult = MessageBox("Would you like to save your changes to the '" + VarString(pOldSel->GetValue(dfcName)) + "' department?", "Save?", MB_YESNO);
			if(nResult == IDYES) {
				Save(VarLong(pOldSel->GetValue(dfcID)));
			}
		}

		IRowSettingsPtr pNewSel(lpRow);
		m_nLastSelectedDepartmentID = VarLong(pNewSel->GetValue(dfcID));
		Load();

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnSelChosenDepartmentFilter");
}

void CDepartmentsAssignUsersDlg::OnOK() 
{
	CWaitCursor wc;
	try
	{
		Save(VarLong(m_pdlDepartments->GetCurSel()->GetValue(dfcID)));

		CDialog::OnOK();

	}NxCatchAll("CDepartmentsAssignUsersDlg::OnOK");
}

void CDepartmentsAssignUsersDlg::Save(long nDepartmentID)
{
	// (z.manning, 11/16/2007) - If nothing changed then there's no need to save anything.
	if(!m_bChanged) {
		return;
	}

	for(int nUserIndex = 0; nUserIndex < m_pAttendanceInfo->m_arypAttendanceUsers.GetSize(); nUserIndex++)
	{
		AttendanceUser *pUser = m_pAttendanceInfo->m_arypAttendanceUsers.GetAt(nUserIndex);
		for(int nDeptIndex = 0; nDeptIndex < pUser->m_arynDepartmentIDs.GetSize(); nDeptIndex++) {
			if(pUser->m_arynDepartmentIDs.GetAt(nDeptIndex) == nDepartmentID) {
				pUser->m_arynDepartmentIDs.RemoveAt(nDeptIndex);
				break;
			}
		}
	}

	CArray<long,long> arynParams;
	CString strSaveSql = "DELETE FROM UserDepartmentLinkT WHERE DepartmentID = ?;\r\n";
	arynParams.Add(nDepartmentID);
	for(IRowSettingsPtr pRow = m_pdlSelectedUsers->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		strSaveSql += "INSERT INTO UserDepartmentLinkT (UserID, DepartmentID) VALUES (?, ?);\r\n";
		long nUserID = VarLong(pRow->GetValue(uscID));
		arynParams.Add(nUserID);
		arynParams.Add(nDepartmentID);
		m_pAttendanceInfo->GetAttendanceUserByID(nUserID)->m_arynDepartmentIDs.Add(nDepartmentID);
	}

	ADODB::_CommandPtr pCmd = OpenParamQuery(strSaveSql);
	for(int nParamIndex = 0; nParamIndex < arynParams.GetSize(); nParamIndex++) {
		AddParameterLong(pCmd, "ID", arynParams.GetAt(nParamIndex));
	}

	CreateRecordset(pCmd);
}
