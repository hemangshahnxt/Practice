// AttendanceUserSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ContactsRc.h"
#include "AttendanceUserSetupDlg.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

enum EUserComboColumns
{
	uccID = 0,
	uccUserName,
};

enum EUserTypeColumns
{
	utcID = 0,
	utcType,
};

enum EDepartmentColumns
{
	dcID = 0,
	dcName,
};

enum EYearComboColumns
{
	yccYear = 0,
};

/////////////////////////////////////////////////////////////////////////////
// CAttendanceUserSetupDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29147 - Created

CAttendanceUserSetupDlg::CAttendanceUserSetupDlg(AttendanceInfo *pAttendanceInfo, CWnd* pParent)
	: CNxDialog(CAttendanceUserSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttendanceUserSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pTempAttendanceInfo = pAttendanceInfo;
	m_pMainAttendanceInfo = pAttendanceInfo;
	m_nDefaultUserID = -1;
	m_nLastSelectedUserID = -1;
	m_bChanged = FALSE;
	m_bAtLeastOneSave = FALSE;
}

CAttendanceUserSetupDlg::~CAttendanceUserSetupDlg()
{
	// (z.manning, 12/19/2007) - Make sure we don't delete the main attendance info pointer as other
	// dialogs use this pointer as well.
	if(m_pTempAttendanceInfo != NULL && m_pTempAttendanceInfo != m_pMainAttendanceInfo) {
		delete m_pTempAttendanceInfo;
	}
}

void CAttendanceUserSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttendanceUserSetupDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ATTENDANCE_USER_SETUP_OK, m_btnOk);
	DDX_Control(pDX, IDC_DEPARTMENT_LABEL, m_nxlDepartments);
	DDX_Control(pDX, IDC_VACATION_ALLOWANCE, m_nxeditVacationAllowance);
	DDX_Control(pDX, IDC_VACATION_BONUS, m_nxeditVacationBonus);
	DDX_Control(pDX, IDC_SICK_ALLOWANCE, m_nxeditSickAllowance);
	DDX_Control(pDX, IDC_ATTENDANCE_USER_NOTES, m_nxeditNotes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAttendanceUserSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAttendanceUserSetupDlg)
	ON_EN_CHANGE(IDC_SICK_ALLOWANCE, OnChangeSickAllowance)
	ON_EN_CHANGE(IDC_VACATION_ALLOWANCE, OnChangeVacationAllowance)
	ON_EN_CHANGE(IDC_VACATION_BONUS, OnChangeVacationBonus)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_ATTENDANCE_USER_SETUP_OK, OnAttendanceUserSetupOk)
	//}}AFX_MSG_MAP
	ON_EN_CHANGE(IDC_ATTENDANCE_USER_NOTES, &CAttendanceUserSetupDlg::OnEnChangeAttendanceUserNotes)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CAttendanceUserSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAttendanceUserSetupDlg)
	ON_EVENT(CAttendanceUserSetupDlg, IDC_ATTENDANCE_USER_LIST, 16 /* SelChosen */, OnSelChosenAttendanceUserList, VTS_DISPATCH)
	ON_EVENT(CAttendanceUserSetupDlg, IDC_ATTENDANCE_USER_TYPE, 16 /* SelChosen */, OnSelChosenAttendanceUserType, VTS_DISPATCH)
	ON_EVENT(CAttendanceUserSetupDlg, IDC_DEPARTMENT_SELECT, 18 /* RequeryFinished */, OnRequeryFinishedDepartmentSelect, VTS_I2)
	ON_EVENT(CAttendanceUserSetupDlg, IDC_DEPARTMENT_SELECT, 16 /* SelChosen */, OnSelChosenDepartmentSelect, VTS_DISPATCH)
	ON_EVENT(CAttendanceUserSetupDlg, IDC_EFFECTIVE_YEAR, 16 /* SelChosen */, OnSelChosenEffectiveYear, VTS_DISPATCH)
	ON_EVENT(CAttendanceUserSetupDlg, IDC_ATTENDANCE_DATE_OF_HIRE, 2 /* Changed */, OnChangedAttendanceDateOfHire, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttendanceUserSetupDlg message handlers

BOOL CAttendanceUserSetupDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pdlUsers = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_USER_LIST, GetRemoteData(), false);
		m_pdlUserTypes = BindNxDataList2Ctrl(this, IDC_ATTENDANCE_USER_TYPE, GetRemoteData(), false);
		m_pdlDepartments = BindNxDataList2Ctrl(this, IDC_DEPARTMENT_SELECT, GetRemoteData(), true);
		m_pdlYear = BindNxDataList2Ctrl(this, IDC_EFFECTIVE_YEAR, GetRemoteData(), false);

		m_nxeditVacationAllowance.SetLimitText(6);
		m_nxeditVacationBonus.SetLimitText(6);
		m_nxeditSickAllowance.SetLimitText(6);
		m_nxeditNotes.SetLimitText(255);

		// (z.manning, 04/11/2008) - PLID 29628 - Added date of hire
		m_nxtDateOfHire = GetDlgItemUnknown(IDC_ATTENDANCE_DATE_OF_HIRE);

		m_nxlDepartments.SetType(dtsHyperlink);
		m_nxlDepartments.SetColor(GetNxColor(GNC_CONTACT, 0));

		int nYear = COleDateTime::GetCurrentTime().GetYear();
		for(int nYearIndex = 0; nYearIndex < 5; nYearIndex++) {
			IRowSettingsPtr pYearRow = m_pdlYear->GetNewRow();
			pYearRow->PutValue(yccYear, (long)nYear++);
			m_pdlYear->AddRowAtEnd(pYearRow, NULL);
			if(nYearIndex == 0) {
				m_pdlYear->PutCurSel(pYearRow);
			}
		}

		// (z.manning, 12/04/2007) - For now anyway, we just have 2 hardcoded types.
		IRowSettingsPtr pTypeRow = m_pdlUserTypes->GetNewRow();
		pTypeRow->PutValue(utcID, (short)autSalary);
		pTypeRow->PutValue(utcType, "Salary");
		m_pdlUserTypes->AddRowSorted(pTypeRow, NULL);
		pTypeRow = m_pdlUserTypes->GetNewRow();
		pTypeRow->PutValue(utcID, (short)autHourly);
		pTypeRow->PutValue(utcType, "Hourly");
		m_pdlUserTypes->AddRowSorted(pTypeRow, NULL);

		for(int nUserIndex = 0; nUserIndex < m_pTempAttendanceInfo->m_arypAttendanceUsers.GetSize(); nUserIndex++)
		{
			AttendanceUser *pUser = m_pTempAttendanceInfo->m_arypAttendanceUsers.GetAt(nUserIndex);
			IRowSettingsPtr pNewRow = m_pdlUsers->GetNewRow();
			pNewRow->PutValue(uccID, pUser->m_nUserID);
			pNewRow->PutValue(uccUserName, _bstr_t(pUser->GetFullName()));
			m_pdlUsers->AddRowAtEnd(pNewRow, NULL);
		}
		m_pdlUsers->Sort();

		if(m_nDefaultUserID > 0) {
			m_pdlUsers->SetSelByColumn(uccID, m_nDefaultUserID);
		}
		if(m_pdlUsers->GetCurSel() == NULL) {
			m_pdlUsers->PutCurSel(m_pdlUsers->GetFirstRow());
		}
		if(m_pdlUsers->GetCurSel() != NULL) {
			m_nLastSelectedUserID = VarLong(m_pdlUsers->GetCurSel()->GetValue(uccID));
		}

		Load();

	}NxCatchAll("CAttendanceUserSetupDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttendanceUserSetupDlg::Load()
{
	if(m_pdlUsers->GetCurSel() == NULL) {
		ASSERT(FALSE);
		return;
	}

	if(m_pdlYear->GetCurSel() == NULL) {
		ASSERT(FALSE);
		return;
	}

	int nYear = VarLong(m_pdlYear->GetCurSel()->GetValue(yccYear));

	if(m_pTempAttendanceInfo->m_nYear != nYear)
	{
		if(nYear == m_pMainAttendanceInfo->m_nYear) {
			if(m_pTempAttendanceInfo != NULL) {
				delete m_pTempAttendanceInfo;
				m_pTempAttendanceInfo = NULL;
			}
			m_pTempAttendanceInfo = m_pMainAttendanceInfo;
		}
		else {
			if(m_pTempAttendanceInfo == NULL) {
				m_pTempAttendanceInfo = new AttendanceInfo;
			}
			m_pTempAttendanceInfo->LoadAllByYear(nYear);
		}
	}

	AttendanceUser *pUser = m_pTempAttendanceInfo->GetAttendanceUserByID(VarLong(m_pdlUsers->GetCurSel()->GetValue(uccID)));
	SetDlgItemInt(IDC_VACATION_ALLOWANCE, (UINT)pUser->m_nVacationAllowance);
	SetDlgItemText(IDC_VACATION_BONUS, AsString((long)pUser->m_nVacationBonus));
	SetDlgItemInt(IDC_SICK_ALLOWANCE, (UINT)pUser->m_nSickAllowance);

	if(m_pdlUserTypes->SetSelByColumn(utcID, (short)pUser->m_eType) == NULL) {
		ThrowNxException("CAttendanceUserSetupDlg::Load - User type %i is not valid.", pUser->m_eType);
	}

	// (z.manning, 04/11/2008) - PLID 29628 - Added date of hire
	if(pUser->m_dtDateOfHire.GetStatus() == COleDateTime::valid) {
		m_nxtDateOfHire->SetDateTime(pUser->m_dtDateOfHire);
	}
	else {
		// (z.manning 2010-01-04 09:32) - PLID 36743 - Make sure we clear out this field if the
		// currently selected user has no date of hire.
		m_nxtDateOfHire->Clear();
	}

	m_arynDepartmentIDs.RemoveAll();
	m_arynDepartmentIDs.Append(pUser->m_arynDepartmentIDs);
	RefreshDepartmentDisplay();

	// (z.manning 2008-11-18 14:46) - PLID 32073 - Load the user's notes for the selected year
	ADODB::_RecordsetPtr prsNotes = CreateParamRecordset(
		"SELECT Notes \r\n"
		"FROM AttendanceAdjustmentsT \r\n"
		"WHERE Year = {INT} AND UserID = {INT} \r\n"
		, nYear, pUser->m_nUserID);
	if(!prsNotes->eof) {
		m_nxeditNotes.SetWindowText(AdoFldString(prsNotes->GetFields(), "Notes", ""));
	}
	else {
		m_nxeditNotes.SetWindowText("");
	}

	m_bChanged = FALSE;
}

void CAttendanceUserSetupDlg::SetDefaultUserID(long nUserID)
{
	m_nDefaultUserID = nUserID;
}

void CAttendanceUserSetupDlg::OnSelChosenAttendanceUserList(LPDISPATCH lpRow) 
{
	try
	{
		IRowSettingsPtr pOldSel = m_pdlUsers->FindByColumn(uccID, m_nLastSelectedUserID, NULL, VARIANT_FALSE);
		if(pOldSel == NULL) {
			ThrowNxException("Unable to find previously selected user row (ID = %li)", m_nLastSelectedUserID);
		}

		if(m_bChanged)
		{
			int nResult = MessageBox("Would you like to save your changes to user '" + VarString(pOldSel->GetValue(uccUserName)) + "?'", "Save?", MB_YESNO);
			if(nResult == IDYES) {
				if(!Validate()) {
					return;
				}

				Save(VarLong(pOldSel->GetValue(uccID)));
			}
		}

		IRowSettingsPtr pNewSel(lpRow);
		if(pNewSel != NULL) {
			m_nLastSelectedUserID = VarLong(pNewSel->GetValue(uccID));
		}

		Load();

	}NxCatchAll("CAttendanceUserSetupDlg::OnSelChosenAttendanceUserList");
}

void CAttendanceUserSetupDlg::OnChangeSickAllowance() 
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll("CAttendanceUserSetupDlg::OnChangeSickAllowance");
}

void CAttendanceUserSetupDlg::OnChangeVacationAllowance() 
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll("CAttendanceUserSetupDlg::OnChangeVacationAllowance");
}

void CAttendanceUserSetupDlg::Save(long nUserID)
{
	if(!m_bChanged) {
		// (z.manning, 11/19/2007) - Nothing changed so no need to save.
		return;
	}

	int nYear = COleDateTime::GetCurrentTime().GetYear();;
	if(m_pdlYear->GetCurSel() != NULL) {
		nYear = VarLong(m_pdlYear->GetCurSel()->GetValue(yccYear));
	}
	int nVacationAllowance = GetDlgItemInt(IDC_VACATION_ALLOWANCE);
	CString strVacationBonus = "0";
	GetDlgItemText(IDC_VACATION_BONUS, strVacationBonus);
	int nVacationBonus = atoi(strVacationBonus);
	int nSickAllowance = GetDlgItemInt(IDC_SICK_ALLOWANCE);
	EAttendanceUserTypes eType = (EAttendanceUserTypes)VarShort(m_pdlUserTypes->GetCurSel()->GetValue(utcID));

	_variant_t varDateOfHire;
	if(m_nxtDateOfHire->GetStatus() == 3) {
		varDateOfHire.vt = VT_NULL;
	}
	else {
		varDateOfHire = COleVariant(COleDateTime(m_nxtDateOfHire->GetDateTime()));
	}

	CString strNotes;
	m_nxeditNotes.GetWindowText(strNotes);

	// (z.manning 2008-11-18 14:57) - PLID 32073 - Need to also save the notes
	ExecuteParamSql(
		"IF EXISTS (SELECT UserID FROM AttendanceAllowanceHistoryT WHERE UserID = {INT} AND Year = {INT}) BEGIN \r\n"
		"	UPDATE AttendanceAllowanceHistoryT \r\n"
		"	SET VacationAllowance = {INT}, SickAllowance = {INT}, UserType = {INT} \r\n"
		"	WHERE UserID = {INT} AND Year = {INT} \r\n"
		"END \r\n"
		"ELSE BEGIN \r\n"
		"	INSERT INTO AttendanceAllowanceHistoryT (UserID, Year, VacationAllowance, SickAllowance, UserType) \r\n"
		"	VALUES ({INT}, {INT}, {INT}, {INT}, {INT}) \r\n"
		"END \r\n"
		"\r\n"
		"DECLARE @bAdjustmentExists bit \r\n"
		"SET @bAdjustmentExists = (CASE WHEN EXISTS (SELECT UserID FROM AttendanceAdjustmentsT WHERE UserID = {INT} AND Year = {INT}) THEN 1 ELSE 0 END) \r\n"
		"IF {INT} <> 0 OR {STRING} <> '' BEGIN \r\n"
		"	IF @bAdjustmentExists = 1 BEGIN \r\n"
		"		UPDATE AttendanceAdjustmentsT SET VacationAdjustment = {INT}, Notes = {STRING} \r\n"
		"		WHERE UserID = {INT} AND Year = {INT} \r\n"
		"	END \r\n"
		"	ELSE BEGIN \r\n"
		"		INSERT INTO AttendanceAdjustmentsT (UserID, Year, VacationAdjustment, Notes) \r\n"
		"		VALUES ({INT}, {INT}, {INT}, {STRING}) \r\n"
		"	END \r\n"
		"END \r\n"
		"ELSE BEGIN \r\n"
		"	IF @bAdjustmentExists = 1 BEGIN \r\n"
		"		DELETE FROM AttendanceAdjustmentsT WHERE UserID = {INT} AND Year = {INT} \r\n"
		"	END \r\n"
		"END \r\n"
		"\r\n"
		"DELETE FROM UserDepartmentLinkT WHERE UserID = {INT} \r\n"
		"\r\n"
		// (z.manning, 04/11/2008) - PLID 29628 - Update date of hire
		"UPDATE UsersT SET DateOfHire = {VT_DATE} WHERE PersonID = {INT} \r\n"
		, nUserID, nYear, nVacationAllowance, nSickAllowance, eType, nUserID, nYear
		, nUserID, nYear, nVacationAllowance, nSickAllowance, eType
		, nUserID, nYear, nVacationBonus, strNotes, nVacationBonus, strNotes, nUserID, nYear
		, nUserID, nYear, nVacationBonus, strNotes
		, nUserID, nYear
		, nUserID
		, varDateOfHire, nUserID);

	if(m_arynDepartmentIDs.GetSize() > 0)
	{
		// (z.manning, 12/17/2007) - We have at least one department for this user, so let's create a 2nd
		// batch of insert statements for each department.
		CString strDeptInserts;
		CArray<long,long> arynParameters;
		for(int nDeptIndex = 0; nDeptIndex < m_arynDepartmentIDs.GetSize(); nDeptIndex++)
		{
			strDeptInserts += "INSERT INTO UserDepartmentLinkT (UserID, DepartmentID) VALUES (?, ?); \r\n";
			arynParameters.Add(nUserID);
			arynParameters.Add(m_arynDepartmentIDs.GetAt(nDeptIndex));
		}
		ADODB::_CommandPtr pCmd = OpenParamQuery(strDeptInserts);
		for(int nParamIndex = 0; nParamIndex < arynParameters.GetSize(); nParamIndex++) {
			AddParameterLong(pCmd, "ID", arynParameters.GetAt(nParamIndex));
		}

		CreateRecordset(pCmd);
	}

	AttendanceUser *pUser = m_pTempAttendanceInfo->GetAttendanceUserByID(nUserID);
	if(nYear == m_pTempAttendanceInfo->m_nYear) {
		pUser->m_nVacationAllowance = nVacationAllowance;
		pUser->m_nVacationBonus = nVacationBonus;
		pUser->m_nSickAllowance = nSickAllowance;
		pUser->m_eType = eType;
		pUser->m_arynDepartmentIDs.RemoveAll();
		pUser->m_arynDepartmentIDs.Append(m_arynDepartmentIDs);
	}

	m_bAtLeastOneSave = TRUE;
}

void CAttendanceUserSetupDlg::OnOK() 
{
	try
	{
		// (z.manning 2008-11-18 17:35) - PLID 32073 - Don't want to commit dialog on enter

	}NxCatchAll("CAttendanceUserSetupDlg::OnOK");
}

void CAttendanceUserSetupDlg::OnAttendanceUserSetupOk()
{
	try
	{
		if(!Validate()) {
			return;
		}

		if(m_pdlUsers->GetCurSel() != NULL) {
			Save(VarLong(m_pdlUsers->GetCurSel()->GetValue(uccID)));
		}

		CDialog::OnOK();

	}NxCatchAll("CAttendanceUserSetupDlg::OnOK");
}

void CAttendanceUserSetupDlg::OnSelChosenAttendanceUserType(LPDISPATCH lpRow) 
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll("CAttendanceUserSetupDlg::OnSelChosenAttendanceUserType");
}

void CAttendanceUserSetupDlg::OnChangeEffectiveYear() 
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll("CAttendanceUserSetupDlg::OnChangeEffectiveYear");
}

BOOL CAttendanceUserSetupDlg::Validate()
{
	int nYear = COleDateTime::GetCurrentTime().GetYear();
	if(m_pdlYear->GetCurSel() != NULL) {
		nYear = VarLong(m_pdlYear->GetCurSel()->GetValue(yccYear));
	}
	if(nYear < COleDateTime::GetCurrentTime().GetYear()) {
		MessageBox(FormatString("You must enter a year equal to at least %i.", COleDateTime::GetCurrentTime().GetYear()));
		return FALSE;
	}

	// (z.manning 2008-11-18 17:49) - PLID 32073 - The vacation bonus field is now a text edit control
	// to allow for negative numbers, so let's validate that value now.
	CString strVacationBonus = "0";
	GetDlgItemText(IDC_VACATION_BONUS, strVacationBonus);
	int nVacationBonus = atoi(strVacationBonus);
	strVacationBonus.TrimLeft("0");
	if(nVacationBonus == 0 && !strVacationBonus.IsEmpty()) {
		// (d.thompson 2014-02-27) - PLID 61016 - Rename vacation to PTO
		MessageBox("Invalid amount for PTO bonus", NULL, MB_ICONERROR);
		return FALSE;
	}

	switch(m_nxtDateOfHire->GetStatus())
	{
		case 1:
			// (z.manning, 04/11/2008) - Date format is valid, so make sure date is valid (1/1/1800 is copied from contacs => general)
			if(m_nxtDateOfHire->GetDateTime() < COleDateTime(1800,1,1,0,0,0)) {
				MessageBox("Please enter a date of hire later than Jan 1 1800.");
				return FALSE;
			}
			break;

		case 2:
			// (z.manning, 04/11/2008) - Invalid date of hire.
			MessageBox("Please enter a valid date of hire.");
			return FALSE;
			break;
	}

	// (z.manning 2008-11-18 17:59) - PLID 32073 - Warn them if they entered a vacation bonus but no notes
	// about it.
	CString strNotes;
	m_nxeditNotes.GetWindowText(strNotes);
	if(nVacationBonus != 0 && strNotes.IsEmpty()) {
		// (d.thompson 2014-02-27) - PLID 61016 - Rename vacation to PTO
		int nResult = MessageBox("You entered a PTO bonus but no notes as to why. Are you sure you want to continue?", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return FALSE;
		}
	}

	return TRUE;
}

void CAttendanceUserSetupDlg::OnChangeVacationBonus() 
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll("CAttendanceUserSetupDlg::OnChangeVacationBonus");
}

void CAttendanceUserSetupDlg::OnRequeryFinishedDepartmentSelect(short nFlags) 
{
	try
	{
		// (z.manning, 12/12/2007) - Add rows for none and multiple
		IRowSettingsPtr pRow;
		pRow = m_pdlDepartments->GetNewRow();
		pRow->PutValue(dcID, (long)-2);
		pRow->PutValue(dcName, "{ Multiple }");
		m_pdlDepartments->AddRowBefore(pRow, m_pdlDepartments->GetFirstRow());

		pRow = m_pdlDepartments->GetNewRow();
		pRow->PutValue(dcID, (long)-1);
		pRow->PutValue(dcName, "{ None }");
		m_pdlDepartments->AddRowBefore(pRow, m_pdlDepartments->GetFirstRow());

		RefreshDepartmentDisplay();

	}NxCatchAll("CAttendanceUserSetupDlg::OnRequeryFinishedDepartmentSelect");
}

void CAttendanceUserSetupDlg::OnSelChosenDepartmentSelect(LPDISPATCH lpRow) 
{
	try
	{
		m_bChanged = TRUE;

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nDeptID = VarLong(pRow->GetValue(dcID));
		if(nDeptID == -2) {
			// (z.manning, 12/12/2007) - They want to select multiple departments.
			HandleDepartmentMultipleSelection();
		}
		else if(nDeptID > 0) {
			m_arynDepartmentIDs.RemoveAll();
			m_arynDepartmentIDs.Add(nDeptID);
		}
		else {
			ASSERT(nDeptID == -1);
			m_arynDepartmentIDs.RemoveAll();
		}

	}NxCatchAll("CAttendanceUserSetupDlg::OnSelChosenDepartmentSelect");
}

void CAttendanceUserSetupDlg::RefreshDepartmentDisplay()
{
	CWnd *pwndDeptSelect = GetDlgItem(IDC_DEPARTMENT_SELECT);
	if(m_arynDepartmentIDs.GetSize() == 0) {
		m_pdlDepartments->SetSelByColumn(dcID, (long)-1);
		m_nxlDepartments.ShowWindow(SW_HIDE);
		if(!pwndDeptSelect->IsWindowVisible()) {
			pwndDeptSelect->ShowWindow(SW_SHOW);
		}
	}
	else if(m_arynDepartmentIDs.GetSize() == 1) {
		m_pdlDepartments->SetSelByColumn(dcID, m_arynDepartmentIDs.GetAt(0));
		m_nxlDepartments.ShowWindow(SW_HIDE);
		if(!pwndDeptSelect->IsWindowVisible()) {
			pwndDeptSelect->ShowWindow(SW_SHOW);
		}
	}
	else {
		CString strDepts;
		for(int nDeptIndex = 0; nDeptIndex < m_arynDepartmentIDs.GetSize(); nDeptIndex++) {
			IRowSettingsPtr pRow = m_pdlDepartments->FindByColumn(dcID, m_arynDepartmentIDs.GetAt(nDeptIndex), NULL, VARIANT_FALSE);
			if(pRow != NULL) {
				strDepts += VarString(pRow->GetValue(dcName), "") + ", ";
			}
			else {
				// (z.manning, 12/12/2007) - We should have found the department.
				ASSERT(FALSE);
			}
		}
		strDepts.TrimRight(", ");
		m_nxlDepartments.SetText(strDepts);

		GetDlgItem(IDC_DEPARTMENT_SELECT)->ShowWindow(SW_HIDE);
		m_nxlDepartments.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_DEPARTMENT_LABEL)->Invalidate();
	}
}

LRESULT CAttendanceUserSetupDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch(nIdc)
		{
			case IDC_DEPARTMENT_LABEL:
				HandleDepartmentMultipleSelection();
				break;

			default:
				ASSERT(FALSE);
				break;
		}

	}NxCatchAll("CAttendanceUserSetupDlg::OnLabelClick");

	return S_OK;
}

void CAttendanceUserSetupDlg::HandleDepartmentMultipleSelection()
{
	AttendanceUser *pUser = m_pTempAttendanceInfo->GetAttendanceUserByID(VarLong(m_pdlUsers->GetCurSel()->GetValue(uccID)));
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "DepartmentsT");
	dlg.PreSelect(m_arynDepartmentIDs);
	CString strSource;
	for(IRowSettingsPtr pRow = m_pdlDepartments->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		long nDeptID = VarLong(pRow->GetValue(dcID));
		if(nDeptID > 0) {
			CString strDept = VarString(pRow->GetValue(dcName), "");
			strDept.Replace(';', '_');
			strSource += AsString(nDeptID) + ';' + strDept + ';';
		}
	}

	CVariantArray aryvarIDsToSkip;
	if(IDOK == dlg.OpenWithDelimitedComboSource(_bstr_t(strSource), aryvarIDsToSkip, FormatString("Select departments for %s", pUser->GetFullName())))
	{
		m_bChanged = TRUE;
		m_arynDepartmentIDs.RemoveAll();
		dlg.FillArrayWithIDs(m_arynDepartmentIDs);
	}
	RefreshDepartmentDisplay();
}

BOOL CAttendanceUserSetupDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		AttendanceUser *pUser = m_pTempAttendanceInfo->GetAttendanceUserByID(VarLong(m_pdlUsers->GetCurSel()->GetValue(uccID)));
		if(m_arynDepartmentIDs.GetSize() > 1)
		{
			CRect rc;
			GetDlgItem(IDC_DEPARTMENT_LABEL)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

	}NxCatchAll("CAttendanceUserSetupDlg::OnSetCursor");
	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CAttendanceUserSetupDlg::OnSelChosenEffectiveYear(LPDISPATCH lpRow) 
{
	try
	{
		Load();

	}NxCatchAll("CAttendanceUserSetupDlg::OnSelChosenEffectiveYear");
}

void CAttendanceUserSetupDlg::OnChangedAttendanceDateOfHire() 
{
	try
	{
		m_bChanged = TRUE;

	}NxCatchAll("CAttendanceUserSetupDlg::OnChangedAttendanceDateOfHire");
}

void CAttendanceUserSetupDlg::OnEnChangeAttendanceUserNotes()
{
	try
	{
		// (z.manning 2008-11-18 15:01) - PLID 32073 - Mark the dialog as changed when the notes change
		m_bChanged = TRUE;
	
	}NxCatchAll("CAttendanceUserSetupDlg::OnEnChangeAttendanceUserNotes");
}
