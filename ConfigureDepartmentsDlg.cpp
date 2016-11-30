// ConfigureDepartmentsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigureDepartmentsDlg.h"
#include "DepartmentsAssignUsersDlg.h"
#include "AttendanceDlg.h"
#include "MultiSelectDlg.h"
#include "GlobalStringUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

enum EDepartmentListColumns
{
	dlcID = 0,
	dlcName,
	dlcManagerIDList,
	dlcManagers,
	dlcResources,
};

/////////////////////////////////////////////////////////////////////////////
// CConfigureDepartmentsDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29152 - Created

CConfigureDepartmentsDlg::CConfigureDepartmentsDlg(AttendanceInfo *pInfo, CWnd* pParent)
	: CNxDialog(CConfigureDepartmentsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureDepartmentsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pAttendanceInfo = pInfo;
}


void CConfigureDepartmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureDepartmentsDlg)
	DDX_Control(pDX, IDC_ASSIGN_USERS_TO_DEPARMENTS, m_btnAssignUsers);
	DDX_Control(pDX, IDC_DELETE_DEPARTMENT, m_btnDelete);
	DDX_Control(pDX, IDC_ADD_DEPARTMENT, m_btnAdd);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureDepartmentsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureDepartmentsDlg)
	ON_BN_CLICKED(IDC_ADD_DEPARTMENT, OnAddDepartment)
	ON_BN_CLICKED(IDC_DELETE_DEPARTMENT, OnDeleteDepartment)
	ON_BN_CLICKED(IDC_ASSIGN_USERS_TO_DEPARMENTS, OnAssignUsersToDeparments)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CConfigureDepartmentsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureDepartmentsDlg)
	ON_EVENT(CConfigureDepartmentsDlg, IDC_DEPARTMENT_LIST, 9 /* EditingFinishing */, OnEditingFinishingDepartmentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CConfigureDepartmentsDlg, IDC_DEPARTMENT_LIST, 10 /* EditingFinished */, OnEditingFinishedDepartmentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConfigureDepartmentsDlg, IDC_DEPARTMENT_LIST, 19 /* LeftClick */, OnLeftClickDepartmentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CConfigureDepartmentsDlg, IDC_DEPARTMENT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedDepartmentList, VTS_I2)
	ON_EVENT(CConfigureDepartmentsDlg, IDC_DEPARTMENT_LIST, 7 /* RButtonUp */, OnRButtonUpDepartmentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureDepartmentsDlg message handlers

BOOL CConfigureDepartmentsDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnAssignUsers.AutoSet(NXB_MODIFY);

		m_pdlDepartments = BindNxDataList2Ctrl(this, IDC_DEPARTMENT_LIST, GetRemoteData(), true);
			
	}NxCatchAll("CConfigureDepartmentsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureDepartmentsDlg::OnAddDepartment() 
{
	try
	{
		AddDepartment();

	}NxCatchAll("CConfigureDepartmentsDlg::OnAddDepartment");
}

void CConfigureDepartmentsDlg::OnDeleteDepartment() 
{
	try
	{
		DeleteDepartment();

	}NxCatchAll("CConfigureDepartmentsDlg::OnDeleteDepartment");
}

void CConfigureDepartmentsDlg::AddDepartment()
{
	IRowSettingsPtr pNewRow = m_pdlDepartments->GetNewRow();
	pNewRow->PutValue(dlcID, (long)-1);
	pNewRow->PutValue(dlcName, "[Enter Department Name]");
	pNewRow->PutValue(dlcManagerIDList, "");
	pNewRow->PutValue(dlcManagers, "");
	m_pdlDepartments->AddRowAtEnd(pNewRow, NULL);
	m_pdlDepartments->StartEditing(pNewRow, dlcName);
}

void CConfigureDepartmentsDlg::DeleteDepartment()
{
	IRowSettingsPtr pRow = m_pdlDepartments->GetCurSel();
	if(pRow == NULL) {
		MessageBox("You must select a department first.");
		return;
	}

	if(IDYES != MessageBox("Are you sure you want to delete the '" + VarString(pRow->GetValue(dlcName),"") + "' department?", NULL, MB_YESNO)) {
		return;
	}

	long nDepartmentID = VarLong(pRow->GetValue(dlcID));
	ExecuteParamSql(
		"DELETE FROM DepartmentResourceLinkT WHERE DepartmentID = {INT}; \r\n"
		"DELETE FROM UserDepartmentLinkT WHERE DepartmentID = {INT}; \r\n"
		"DELETE FROM DepartmentManagersT WHERE DepartmentID = {INT}; \r\n"
		"DELETE FROM DepartmentsT WHERE ID = {INT}; \r\n"
		, nDepartmentID, nDepartmentID, nDepartmentID, nDepartmentID);

	m_pdlDepartments->RemoveRow(pRow);
}

void CConfigureDepartmentsDlg::OnEditingFinishingDepartmentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case dlcName:
			{
				if(*pbCommit)
				{
					CString strDepartment = VarString(*pvarNewValue);
					strDepartment.TrimLeft();
					strDepartment.TrimRight();

					// (z.manning, 11/16/2007) - No blank names allowed.
					if(strDepartment.IsEmpty()) {
						MessageBox("You may not have blank department names.");
						*pbContinue = FALSE;
						return;
					}

					// (z.manning, 11/16/2007) - No duplicate department names.
					for(IRowSettingsPtr pTemp = m_pdlDepartments->GetFirstRow(); pTemp != NULL; pTemp = pTemp->GetNextRow())
					{
						if(!pRow->IsSameRow(pTemp)) {
							CString strExistingDepartment = VarString(pTemp->GetValue(dlcName));
							if(strDepartment.CompareNoCase(strExistingDepartment) == 0) {
								MessageBox("There is already a department named '" + strDepartment + ".'");
								*pbContinue = FALSE;
								return;
							}
						}
					}
				}
			}
		}

	}NxCatchAll("CConfigureDepartmentsDlg::OnEditingFinishingDepartmentList");
}

void CConfigureDepartmentsDlg::OnEditingFinishedDepartmentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case dlcName:
			{
				if(bCommit)
				{
					CString strDepartment = VarString(varNewValue);
					Trim(strDepartment);

					long nID = VarLong(pRow->GetValue(dlcID));
					if(nID == -1) {
						// (z.manning, 11/16/2007) - It's a new department, so insert it.
						ADODB::_RecordsetPtr prsInsert = CreateParamRecordset(
							"SET NOCOUNT ON \r\n"
							"INSERT INTO DepartmentsT (Name) VALUES ({STRING}) \r\n"
							"SET NOCOUNT OFF \r\n"
							"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID "
							, strDepartment);

						pRow->PutValue(dlcID, AdoFldLong(prsInsert, "ID"));
					}
					else {
						// (z.manning, 11/16/2007) - It's an existing department, so update it if it changed.
						if(VarString(varOldValue) != strDepartment) {
							ExecuteParamSql("UPDATE DepartmentsT SET Name = {STRING} WHERE ID = {INT}"
								, strDepartment, VarLong(pRow->GetValue(dlcID)));
						}
					}
				}
				else
				{
					// (z.manning, 11/16/2007) - If it's a new row and they did not committ it then remove the row.
					if(VarLong(pRow->GetValue(dlcID)) == -1) {
						m_pdlDepartments->RemoveRow(pRow);
					}
				}
			}
		}

	}NxCatchAll("CConfigureDepartmentsDlg::OnEditingFinishedDepartmentList");	
}

void CConfigureDepartmentsDlg::OnAssignUsersToDeparments() 
{
	try
	{
		if(m_pdlDepartments->GetRowCount() <= 0) {
			// (z.manning, 11/30/2007) - We can't do this if there aren't any departments.
			MessageBox("You must have at least 1 department before you can assign users.");
			return;
		}

		CArray<AttendanceDepartment,AttendanceDepartment&> aryDepartments;
		for(IRowSettingsPtr pRow = m_pdlDepartments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			AttendanceDepartment dept;
			dept.nID = VarLong(pRow->GetValue(dlcID));
			dept.strName = VarString(pRow->GetValue(dlcName), "");
			// (z.manning, 11/16/2007) - Don't add the "{ All }" row.
			if(dept.nID  > 0) {
				aryDepartments.Add(dept);
			}
		}

		CDepartmentsAssignUsersDlg dlg(aryDepartments, m_pAttendanceInfo, this);
		if(m_pdlDepartments->GetCurSel() != NULL) {
			dlg.SetDefaultDepartmentID(VarLong(m_pdlDepartments->GetCurSel()->GetValue(dlcID)));
		}
		dlg.DoModal();

	}NxCatchAll("CConfigureDepartmentsDlg::OnAssignUsersToDeparments");
}

void CConfigureDepartmentsDlg::OnLeftClickDepartmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CWaitCursor wc;

		long nDepartmentID = VarLong(pRow->GetValue(dlcID));
		CString strDepartment = VarString(pRow->GetValue(dlcName), "");

		switch(nCol)
		{
			case dlcManagers:
			{
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "UsersT");
				CString strDelimitedIDs = VarString(pRow->GetValue(dlcManagerIDList), "");
				strDelimitedIDs.Replace(';', ' ');
				dlg.PreSelect(strDelimitedIDs);
				CVariantArray aryvarIDsToSkip;
				aryvarIDsToSkip.Add(_variant_t((long)-1, VT_I4));
				CString strSourceData;
				for(int nUserIndex = 0; nUserIndex < m_pAttendanceInfo->m_arypAttendanceUsers.GetSize(); nUserIndex++)
				{
					AttendanceUser *pUser = m_pAttendanceInfo->m_arypAttendanceUsers.GetAt(nUserIndex);
					if(pUser->m_nUserID > 0) {
						strSourceData += AsString(pUser->m_nUserID) + ";";
						CString strUser = pUser->GetFullName();
						strUser.Replace(';', '_');
						strSourceData += strUser + ";";
					}
				}

				if(IDOK == dlg.OpenWithDelimitedComboSource(_bstr_t(strSourceData), aryvarIDsToSkip, FormatString("Select Managers for %s", VarString(pRow->GetValue(dlcName),""))))
				{
					CArray<long,long> arynManagerIDs, arynParams;
					dlg.FillArrayWithIDs(arynManagerIDs);
					CString strSql = "DELETE FROM DepartmentManagersT WHERE DepartmentID = ?;\r\n";
					arynParams.Add(nDepartmentID);
					CString strManagerIDs;
					for(int nIndex = 0; nIndex < arynManagerIDs.GetSize(); nIndex++)
					{
						strSql += "INSERT INTO DepartmentManagersT (UserID, DepartmentID) VALUES (?, ?);\r\n";
						arynParams.Add(arynManagerIDs.GetAt(nIndex));
						arynParams.Add(nDepartmentID);
						strManagerIDs += AsString(arynManagerIDs.GetAt(nIndex)) + ";";
					}
					strManagerIDs.TrimRight(';');

					ADODB::_CommandPtr pcmd = OpenParamQuery(strSql);
					for(int nParamIndex = 0; nParamIndex < arynParams.GetSize(); nParamIndex++) {
						AddParameterLong(pcmd, "ID", arynParams.GetAt(nParamIndex));
					}

					CreateRecordset(pcmd);

					pRow->PutValue(dlcManagerIDList, _bstr_t(strManagerIDs));
					UpdateManagerDisplay();
				}
			}
			break;

			// (z.manning, 02/15/2008) - PLID 28909 - We can now associate departments with resources.
			case dlcResources:
			{
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "ResourceT");
				// (z.manning, 02/15/2008) - Load the IDs we need to pre-select.
				ADODB::_RecordsetPtr prsPreSelect = CreateParamRecordset(
					"SELECT ResourceT.ID "
					"FROM ResourceT "
					"INNER JOIN DepartmentResourceLinkT ON ResourceT.ID = DepartmentResourceLinkT.ResourceID "
					"WHERE DepartmentID = {INT} "
					, nDepartmentID);
				CArray<long,long> arynPreSelect;
				for(; !prsPreSelect->eof; prsPreSelect->MoveNext()) {
					arynPreSelect.Add(AdoFldLong(prsPreSelect, "ID"));
				}
				dlg.PreSelect(arynPreSelect);

				CString strWindowText = FormatString("Select associated schedule resources.");
				if(IDOK == dlg.Open("ResourceT", "Inactive = 0", "ID", "Item", strWindowText))
				{
					CArray<long,long> arynResourceIDs;
					dlg.FillArrayWithIDs(arynResourceIDs);
					CString strSql = 
						"SET NOCOUNT ON \r\n"
						"DELETE FROM DepartmentResourceLinkT WHERE DepartmentID = ?;\r\n";
					CArray<long,long> arynParams;
					arynParams.Add(nDepartmentID);
					for(int nResourceIndex = 0; nResourceIndex < arynResourceIDs.GetSize(); nResourceIndex++) {
						strSql += "INSERT INTO DepartmentResourceLinkT (DepartmentID, ResourceID) VALUES (?, ?);\r\n";
						arynParams.Add(nDepartmentID);
						arynParams.Add(arynResourceIDs.GetAt(nResourceIndex));
					}
					strSql += 
						"SET NOCOUNT OFF\r\n"
						"SELECT dbo.GetDepartmentResourceList(?) AS ResourceList ";
					arynParams.Add(nDepartmentID);

					ADODB::_CommandPtr pCmd = OpenParamQuery(strSql);
					for(int nParamIndex = 0; nParamIndex < arynParams.GetSize(); nParamIndex++) {
						AddParameterLong(pCmd, "ID", arynParams.GetAt(nParamIndex));
					}
					ADODB::_RecordsetPtr prs = CreateRecordset(pCmd);

					pRow->PutValue(dlcResources, _bstr_t(AdoFldString(prs, "ResourceList", "")));
				}
			}
			break;
		}

	}NxCatchAll("CConfigureDepartmentsDlg::OnLeftClickDepartmentList");
}

void CConfigureDepartmentsDlg::UpdateManagerDisplay()
{
	for(IRowSettingsPtr pRow = m_pdlDepartments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		CString strManagerIDs = VarString(pRow->GetValue(dlcManagerIDList), "");
		CString strManagerUserNames;
		while(!strManagerIDs.IsEmpty())
		{
			int nSemicolon = strManagerIDs.Find(';');
			if(nSemicolon == -1) {
				nSemicolon = strManagerIDs.GetLength();
			}
			long nManagerID = AsLong(_bstr_t(strManagerIDs.Left(nSemicolon)));
			strManagerIDs.Delete(0, nSemicolon);
			strManagerIDs.TrimLeft(';');

			AttendanceUser *pUser = m_pAttendanceInfo->GetAttendanceUserByID(nManagerID);
			CString strUser = pUser->m_strFirstName.Left(1) + "." + pUser->m_strLastName;
			strManagerUserNames += strUser + ", ";
		}
		strManagerUserNames.TrimRight(", ");
		pRow->PutValue(dlcManagers, _bstr_t(strManagerUserNames));
	}
}

void CConfigureDepartmentsDlg::OnRequeryFinishedDepartmentList(short nFlags) 
{
	try
	{
		UpdateManagerDisplay();

	}NxCatchAll("CConfigureDepartmentsDlg::OnRequeryFinishedDepartmentList");
}

void CConfigureDepartmentsDlg::OnRButtonUpDepartmentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// (z.manning, 12/03/2007) - If the row isn't already, ensure it's highlighted.
		if(pRow != NULL) {
			if(!pRow->IsHighlighted()) {
				m_pdlDepartments->PutCurSel(pRow);
			}
		}

		enum EMenuOptions
		{
			moAdd = 1,
			moRename,
			moDelete,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();

		UINT nRowSelectedFlag = pRow == NULL ? MF_GRAYED : MF_ENABLED;
		mnu.AppendMenu(MF_ENABLED, moAdd, "Add");
		mnu.AppendMenu(nRowSelectedFlag, moRename, "Rename");
		mnu.AppendMenu(nRowSelectedFlag, moDelete, "Delete");

		CPoint ptClicked(x, y);
		GetDlgItem(IDC_DEPARTMENT_LIST)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);

		switch(nResult)
		{
			case moAdd:
				AddDepartment();
				break;

			case moRename:
				m_pdlDepartments->StartEditing(pRow, dlcName);
				break;

			case moDelete:
				DeleteDepartment();
				break;

			default:
				ASSERT(nResult == 0);
				break;
		}

	}NxCatchAll("CConfigureDepartmentsDlg::OnRButtonUpDepartmentList");
}
