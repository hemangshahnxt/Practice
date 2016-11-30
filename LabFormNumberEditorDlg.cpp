// LabFormNumberEditorDlg.cpp: implementation of the CLabFormNumberEditorDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PatientsRc.h"
#include "LabFormNumberEditorDlg.h"

using namespace ADODB;

// (r.gonet 03/29/2012) - PLID 45856 - Added lab procedure group id
CString GetNewLabFormNumber(CString strFormat, long nLabProcedureGroupID)
{
	long nDummyFormNumber = -1;
	return GetNewLabFormNumber(strFormat, nDummyFormNumber, nLabProcedureGroupID);
}

// (r.gonet 03/29/2012) - PLID 45856 - Added lab procedure group id
CString GetNewLabFormNumber(CString strFormat, OUT long &nLabFormNumberCounter, long nLabProcedureGroupID)
{
	// (z.manning, 07/25/2006) - PLID 21576

	// Get the number of counter digits
	int nCounterStartPos = strFormat.ReverseFind('#');
	int nCounterDigits = 5;
	if(nCounterStartPos >= 0) { // Should be impossible for this to not be found, but just in case.
		int nLength = strFormat.GetLength();
		nCounterDigits = AsLong( _bstr_t(strFormat.Right(nLength - nCounterStartPos - 1)) );
		strFormat.Delete(nCounterStartPos, nLength - nCounterStartPos);
	}
	
	// Fill in year values.
	COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
	CString strYear = AsString((long)dtCurrent.GetYear());
	//TES 3/30/2012 - PLID 48205 - Check whether the year actually shows up in the form number
	bool bNumberIncludesYear = false;
	if(strFormat.Replace( "%Y", strYear ) > 0) {
		bNumberIncludesYear = true;
	}
	if( strFormat.Replace( "%y", strYear.Right(2) ) > 0) {
		bNumberIncludesYear = true;
	}

	// Fill in the increment value and pad it with leading zeros if necessary.
	//TES 3/30/2012 - PLID 48205 - Added bNumberIncludesYear
	nLabFormNumberCounter = GetNextLabFormNumber(nLabProcedureGroupID, bNumberIncludesYear);
	CString strID = AsString(nLabFormNumberCounter);
	while(strID.GetLength() < nCounterDigits) {
		strID.Insert(0, "0");
	}
	strFormat.Replace( "%n", strID );

	return strFormat;
}

// (r.gonet 03/29/2012) - PLID 45856 - Added lab procedure group id. This function retrieves the current format 
//  of the lab form numbers for a certain group and returns it. -1 specifies system default.
CString GetLabFormNumberFormat(long nLabProcedureGroupID)
{
	CParamSqlBatch sqlBatch;
	// (r.gonet 03/29/2012) - PLID 45856 - Get the system default format
	sqlBatch.Add(
		"SELECT LabFormNumberFormat, NextLabFormNumber "
		"FROM LabProcedureGroupsT "
		"WHERE ID = -1; ");
	// (r.gonet 03/29/2012) - PLID 45856 - Get the non-system default format if this is not the default group.
	if(nLabProcedureGroupID != -1) {
		sqlBatch.Add(
			"SELECT UseDefaultLabFormNumberFormat, LabFormNumberFormat "
			"FROM LabProcedureGroupsT WHERE ID = {INT}; ",
			nLabProcedureGroupID);
	}
	_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());
	// (r.gonet 03/29/2012) - PLID 45856 - Get the system default format
	CString strLabFormNumberFormat = VarString(prs->Fields->Item["LabFormNumberFormat"]->Value, LAB_FORM_NUMBER_FORMAT_DEFAULT);
	if(nLabProcedureGroupID != -1) {
		prs = prs->NextRecordset(NULL);
		if(prs->eof) {
			ThrowNxException(FormatString("%s : Lab Procedure Group with ID = %li does not exist!", __FUNCTION__, nLabProcedureGroupID));
		} else {
			// (r.gonet 03/29/2012) - PLID 45856 - User wants to override the default format?
			BOOL bUseDefaultLabFormNumberFormat = VarBool(prs->Fields->Item["UseDefaultLabFormNumberFormat"]->Value);
			
			if(!bUseDefaultLabFormNumberFormat) {
				// (r.gonet 03/29/2012) - PLID 45856 - Override the default format then.
				strLabFormNumberFormat = VarString(prs->Fields->Item["LabFormNumberFormat"]->Value, "%y-%n#4");
			}
		}
	}

	return strLabFormNumberFormat;
}

// (r.gonet 03/29/2012) - PLID 45856 - Gets the next form number number in the sequence for a given group. -1 specifies the
//  system default group.
//TES 3/30/2012 - PLID 48205 - Added bNumberIncludesYear; if it is false, then the counter will not be reset even if it's a new year
long GetNextLabFormNumber(long nLabProcedureGroupID, bool bNumberIncludesYear)
{
	CParamSqlBatch sqlBatch;
	// We have to roll over numbers each year.
	//TES 3/30/2012 - PLID 48205 - Only if they actually include the year in the form number
	if(bNumberIncludesYear) {
		sqlBatch.Add(
			"SET NOCOUNT ON; "
			"IF EXISTS (SELECT ID FROM LabProcedureGroupsT WHERE COALESCE(LabFormNumberYear, 0) < DATEPART(yyyy, GETDATE())) "
			"	UPDATE LabProcedureGroupsT "
			"	SET LabFormNumberYear = DATEPART(yyyy, GETDATE()), " 
			"		NextLabFormNumber = 1 "
			"	WHERE COALESCE(LabFormNumberYear, 0) < DATEPART(yyyy, GETDATE()); "
			"SET NOCOUNT OFF; ");
	}
	// Get the system default count
	sqlBatch.Add(
		"SELECT LabFormNumberFormat, NextLabFormNumber, LabFormNumberYear "
		"FROM LabProcedureGroupsT "
		"WHERE ID = -1; ");
	// Get the lab procedure group specific count if our group isn't the default
	if(nLabProcedureGroupID != -1) {
		sqlBatch.Add(
			"SELECT UseDefaultLabFormNumberFormat, LabFormNumberFormat, UseDefaultLabFormNumberSequence, NextLabFormNumber, LabFormNumberYear "
			"FROM LabProcedureGroupsT WHERE ID = {INT}; ",
			nLabProcedureGroupID);
	}
	_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());

	// Get the system default count
	long nNextLabFormNumber = VarLong(prs->Fields->Item["NextLabFormNumber"]->Value, 1);
	if(nLabProcedureGroupID != -1) {
		prs = prs->NextRecordset(NULL);
		if(prs->eof) {
			ThrowNxException(FormatString("%s : Lab Procedure Group with ID = %li does not exist!", __FUNCTION__, nLabProcedureGroupID));
		} else {
			// We are not the default group.
			BOOL bUseDefaultLabFormNumberFormat = VarBool(prs->Fields->Item["UseDefaultLabFormNumberFormat"]->Value);
			BOOL bUseDefaultLabFormNumberSequence = VarBool(prs->Fields->Item["UseDefaultLabFormNumberSequence"]->Value);

			// to not use the default sequence, we must override the default format, which is why this is nested.
			if(!bUseDefaultLabFormNumberFormat && !bUseDefaultLabFormNumberSequence) {
				// Now it is safe to get the non-default's group number since we are overring the format AND using a separate sequence.
				nNextLabFormNumber = VarLong(prs->Fields->Item["NextLabFormNumber"]->Value, 1);
			}
		}
	}

	return nNextLabFormNumber;
}

// (r.gonet 03/29/2012) - PLID 45856 - Sets the next form number of a lab procedure group if nCount is less than the group's next number.
void SetLabFormNumberCounterIfLess(long nCount, long nLabProcedureGroupID)
{
	ASSERT(nCount > 0);
	ExecuteParamSql(GetRemoteData(),
		"IF 1 = ( "
		"	SELECT CONVERT(BIT, "
		"		CASE " 
		"			WHEN (UseDefaultLabFormNumberFormat = 0 AND UseDefaultLabFormNumberSequence = 0) "
		"			THEN 1 " 
		"			ELSE 0 "
		"		END) "
		"	FROM LabProcedureGroupsT "
		"	WHERE LabProcedureGroupsT.ID = {INT}) "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET NextLabFormNumber = {INT} "
		"	WHERE LabProcedureGroupsT.ID = {INT} AND {INT} < COALESCE(LabProcedureGroupsT.NextLabFormNumber, 0); "
		"END "
		"ELSE "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET NextLabFormNumber = {INT} "
		"	WHERE LabProcedureGroupsT.ID = -1 AND {INT} < COALESCE(LabProcedureGroupsT.NextLabFormNumber, 0); "
		"END ",
		nLabProcedureGroupID, 
		nCount, nLabProcedureGroupID, nCount,
		nCount, nCount);
}

// (r.gonet 03/29/2012) - PLID 45856 - Increments a group's next number by a certain amount.
void IncrementLabFormNumberCounter(int nIncrementBy, long nLabProcedureGroupID)
{
	ExecuteParamSql(GetRemoteData(),
		"IF 1 = ( "
		"	SELECT CONVERT(BIT, "
		"		CASE " 
		"			WHEN (UseDefaultLabFormNumberFormat = 0 AND UseDefaultLabFormNumberSequence = 0) "
		"			THEN 1 " 
		"			ELSE 0 "
		"		END) "
		"	FROM LabProcedureGroupsT "
		"	WHERE LabProcedureGroupsT.ID = {INT}) "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET NextLabFormNumber = NextLabFormNumber + {INT} "
		"	WHERE LabProcedureGroupsT.ID = {INT}; "
		"END "
		"ELSE "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET NextLabFormNumber = NextLabFormNumber + {INT} "
		"	WHERE LabProcedureGroupsT.ID = -1; "
		"END ",
		nLabProcedureGroupID, 
		nIncrementBy, nLabProcedureGroupID,
		nIncrementBy);
}

// (r.gonet 03/29/2012) - PLID 45856 - Sets the lab form number format for a specific Lab Procedure Group.
void SetLabFormNumberFormat(CString strFormat, long nLabProcedureGroupID)
{
	ExecuteParamSql(GetRemoteData(),
		"IF 1 = ( "
		"	SELECT CASE WHEN UseDefaultLabFormNumberFormat = 0 THEN 1 ELSE 0 END "
		"	FROM LabProcedureGroupsT "
		"	WHERE LabProcedureGroupsT.ID = {INT}) "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET LabFormNumberFormat = {STRING} "
		"	WHERE LabProcedureGroupsT.ID = {INT}; "
		"END "
		"ELSE "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET LabFormNumberFormat = {STRING} "
		"	WHERE LabProcedureGroupsT.ID = -1; "
		"END ",
		nLabProcedureGroupID, 
		strFormat, nLabProcedureGroupID,
		strFormat);
}

// (r.gonet 03/29/2012) - PLID 45856 - Sets the next lab form number for a specific lab procedure group.
void SetLabFormNumberCounter(long nCount, long nLabProcedureGroupID)
{
	ExecuteParamSql(GetRemoteData(),
		"IF 1 = ( "
		"	SELECT CONVERT(BIT, "
		"		CASE " 
		"			WHEN (UseDefaultLabFormNumberFormat = 0 AND UseDefaultLabFormNumberSequence = 0) "
		"			THEN 1 " 
		"			ELSE 0 "
		"		END) "
		"	FROM LabProcedureGroupsT "
		"	WHERE LabProcedureGroupsT.ID = {INT}) "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET NextLabFormNumber = {INT} "
		"	WHERE LabProcedureGroupsT.ID = {INT}; "
		"END "
		"ELSE "
		"BEGIN "
		"	UPDATE LabProcedureGroupsT "
		"	SET NextLabFormNumber = {INT} "
		"	WHERE LabProcedureGroupsT.ID = -1; "
		"END ",
		nLabProcedureGroupID, 
		nCount, nLabProcedureGroupID,
		nCount);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLabFormNumberEditorDlg::CLabFormNumberEditorDlg(CDialog* pParent /* = NULL */)
	: CNxDialog(CLabFormNumberEditorDlg::IDD, pParent)
{
	m_nOriginalIncrementValue = 0;
	m_bIncrementValueChanged = FALSE;
	m_nLabProcedureGroupID = -1;
	m_bOldOverrideDefaultFormat = FALSE;
	m_bOldUseSeparateSequence = FALSE;
}

CLabFormNumberEditorDlg::CLabFormNumberEditorDlg(long nLabProcedureGroupID, CDialog* pParent/* = NULL*/)
	: CNxDialog(CLabFormNumberEditorDlg::IDD, pParent)
{
	m_nOriginalIncrementValue = 0;
	m_bIncrementValueChanged = FALSE;
	m_nLabProcedureGroupID = nLabProcedureGroupID;
	// (r.gonet 03/29/2012) - PLID 45856 - To track modifications
	m_bOldOverrideDefaultFormat = FALSE;
	m_bOldUseSeparateSequence = FALSE;
}

void CLabFormNumberEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabsSetupDlg)
	DDX_Control(pDX, IDC_LFNE_OVERRIDE_DEFAULT_FORMAT, m_checkOverrideDefaultFormat);
	DDX_Control(pDX, IDC_LAB_NUMBER_FORMAT, m_nxeditLabNumberFormat);
	DDX_Control(pDX, IDC_INCREMENTAL_PORTION_DIGITS, m_nxeditIncrementalPortionDigits);
	DDX_Control(pDX, IDC_INCREMENTAL_PORTION_VALUE, m_nxeditIncrementalPortionValue);
	DDX_Control(pDX, IDC_PREVIEW_FORM_NUMBER_LABEL, m_nxstaticPreviewFormNumberLabel);
	DDX_Control(pDX, IDC_LFNE_USE_SEPARATE_COUNT, m_checkUseSeparateCount);
	DDX_Control(pDX, IDC_PREVIEW_FORM_NUMBER, m_nxstaticPreviewFormNumber);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLabFormNumberEditorDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLabsSetupDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LFNE_OVERRIDE_DEFAULT_FORMAT, &CLabFormNumberEditorDlg::OnBnClickedLfneOverrideDefaultFormat)
END_MESSAGE_MAP()

BOOL CLabFormNumberEditorDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-04-25 14:05) - PLID 29790 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (r.gonet 03/29/2012) - PLID 45856 - Refactored to encapsulate in a new function.
		InitializeFormNumberEditor();

	} NxCatchAll("CLabFormNumberEditorDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 03/29/2012) - PLID 45856 - Initializes the form fields based on data values.
void CLabFormNumberEditorDlg::InitializeFormNumberEditor()
{
	// (r.gonet 03/29/2012) - PLID 45856 - Get the initial values for the Lab Procedure Group flags to see if they were modified later.
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteData(),
		"SELECT UseDefaultLabFormNumberFormat, UseDefaultLabFormNumberSequence "
		"FROM LabProcedureGroupsT "
		"WHERE LabProcedureGroupsT.ID = {INT}; ",
		m_nLabProcedureGroupID);
	if(!prs->eof) {
		m_bOldOverrideDefaultFormat = VarBool(prs->Fields->Item["UseDefaultLabFormNumberFormat"]->Value, TRUE) ? FALSE : TRUE;
		m_bOldUseSeparateSequence = VarBool(prs->Fields->Item["UseDefaultLabFormNumberSequence"]->Value, TRUE) ? FALSE : TRUE;
	}

	// (r.gonet 03/29/2012) - PLID 45856 - Set the override checkboxes
	m_checkOverrideDefaultFormat.SetCheck(m_bOldOverrideDefaultFormat);
	m_checkUseSeparateCount.SetCheck(m_bOldUseSeparateSequence);

	m_nxeditIncrementalPortionValue.SetLimitText(10);

	// The length of the lab form number counter is stored at the end of the format string.
	CString strFormat = GetLabFormNumberFormat(m_nLabProcedureGroupID);
	int nCounterStartPos = strFormat.ReverseFind('#');
	int nCounterDigits = 5;
	if(nCounterStartPos >= 0) {
		int nLength = strFormat.GetLength();
		nCounterDigits = AsLong( _bstr_t(strFormat.Right(nLength - nCounterStartPos - 1)) );
		strFormat.Delete(nCounterStartPos, nLength - nCounterStartPos);
	}
	SetDlgItemInt(IDC_INCREMENTAL_PORTION_DIGITS, nCounterDigits);
	SetDlgItemText(IDC_LAB_NUMBER_FORMAT, strFormat);

	// (z.manning 2010-02-01 14:46) - PLID 34808
	//TES 3/30/2012 - PLID 48205 - We need to let GetNextLabFormNumber know whether the form number includes the year
	bool bNumberIncludesYear = false;
	if(strFormat.Find("%Y") != -1 || strFormat.Find("%y") != -1) {
		bNumberIncludesYear = true;
	}
	int nNextIncrementValue = GetNextLabFormNumber(m_nLabProcedureGroupID, bNumberIncludesYear);
	SetDlgItemInt(IDC_INCREMENTAL_PORTION_VALUE, nNextIncrementValue);
	m_nOriginalIncrementValue = nNextIncrementValue;
	
	UpdatePreview();

	// (r.gonet 03/29/2012) - PLID 45856 - Enable and disable controls according to the state of the dialog.
	EnsureControls();
}

void CLabFormNumberEditorDlg::OnOK()
{
	// (z.manning, 07/24/2006) - PLID 21576 - Make sure we have a valid format, and then save it.
	try {
		BOOL bOverrideDefaultFormat = m_checkOverrideDefaultFormat.GetCheck() > 0 ? TRUE : FALSE;
		BOOL bUseSeparateSequence = m_checkUseSeparateCount.GetCheck() > 0 ? TRUE : FALSE;

		CString strNewFormat;
		UINT nNewIncrementValue;
		if(bOverrideDefaultFormat || m_nLabProcedureGroupID == -1) {
			CString strFormat;
			GetDlgItemText(IDC_LAB_NUMBER_FORMAT, strFormat);

			int nNewCounterDigits = GetDlgItemInt(IDC_INCREMENTAL_PORTION_DIGITS);

			nNewIncrementValue = GetDlgItemInt(IDC_INCREMENTAL_PORTION_VALUE);

			// If we don't have the incremental value in this part, then we're not going to be able
			// to automatically generate form numbers, which is ok, but we should warn them.
			if(strFormat.Find("%n") == -1) 
			{
				if( IDYES != MessageBox("You do not have %n anywhere in the format. Practice will not be able to automatically generate form numbers without it.  Are you sure you want to save?", NULL, MB_YESNO) )
				{
					return;
				}
			}

			// Let's also warn if it looks like we're going to be generating form numbers longer than 25 chars.
			if(CalculateProjectedFormNumberLength() >= 25)
			{
				if( IDYES != MessageBox("The format you have entered may result in form numbers that are too long. Are you sure you want to save?", NULL, MB_YESNO) )
				{
					return;
				}
			}

			if(nNewCounterDigits < 0) 
			{
				MessageBox("Please enter a non-negative value for the number of digits in the incremental portion.");
				return;
			}
			
			strNewFormat = FormatString("%s#%i", strFormat, nNewCounterDigits);

			// (r.gonet 03/29/2012) - PLID 45856 - We must check if the form number format is a duplicate
			//  and prevent saving if it is.
			if((bOverrideDefaultFormat || m_nLabProcedureGroupID == -1) &&
				ReturnsRecordsParam(GetRemoteData(), 
					"SELECT ID "
					"FROM LabProcedureGroupsT "
					"WHERE ID <> {INT} AND LabFormNumberFormat = {STRING} ",
					m_nLabProcedureGroupID, strNewFormat))
			{
				MsgBox("All form number format's must be unique. "
					"Another Lab Procedure Group is using this format currently. "
					"Please enter a different format.");
				return;
			}
		}

		// (r.gonet 03/29/2012) - PLID 45856 - Save the override flags for the Lab Procedure Group.
		if(bOverrideDefaultFormat != m_bOldOverrideDefaultFormat ||
			bUseSeparateSequence != m_bOldUseSeparateSequence) {
				
				CSqlFragment sqlFragment = CSqlFragment(
					"UPDATE LabProcedureGroupsT "
					"SET UseDefaultLabFormNumberFormat = {BOOL} "
					"	 , UseDefaultLabFormNumberSequence = {BOOL} ",
					!bOverrideDefaultFormat,
					(bOverrideDefaultFormat && !bUseSeparateSequence)
					);
				if(!bOverrideDefaultFormat) {
					// (r.gonet 03/29/2012) - PLID 45856 - User does not want to override. Unset in data.
					sqlFragment += CSqlFragment(
						", LabFormNumberFormat = NULL "
						);
					if(!bUseSeparateSequence) {
						// (r.gonet 03/29/2012) - PLID 45856 - user does not want to use separate sequence. Unset in data.
						sqlFragment += CSqlFragment(
							", NextLabFormNumber = NULL "
							);
					}
				}
				sqlFragment += CSqlFragment("WHERE LabProcedureGroupsT.ID = {INT}; ", m_nLabProcedureGroupID);
				ExecuteParamSql(GetRemoteData(), sqlFragment);
		}

		// (r.gonet 03/29/2012) - PLID 45856 - The following will be disabled unless the user has overrided the default format
		//  or this is the default group.
		if(bOverrideDefaultFormat || m_nLabProcedureGroupID == -1) {
			SetLabFormNumberFormat(strNewFormat, m_nLabProcedureGroupID);
			// (z.manning 2010-02-01 15:19) - PLID 34808
			SetLabFormNumberCounter(nNewIncrementValue, m_nLabProcedureGroupID);
			if(m_nOriginalIncrementValue != nNewIncrementValue) {
				m_bIncrementValueChanged = TRUE;
			}
		}

		CDialog::OnOK();

	} NxCatchAll("CLabFormNumberEditorDlg::OnOK");
}

void CLabFormNumberEditorDlg::UpdatePreview()
{
	try {
		// (z.manning, 07/25/2006) - PLID 21576 - Update the static text on the dlg that shows an example form number.
		CString strFormat;
		GetDlgItemText(IDC_LAB_NUMBER_FORMAT, strFormat);
		strFormat += "#" + AsString((long)GetDlgItemInt(IDC_INCREMENTAL_PORTION_DIGITS));
		// (r.gonet 03/29/2012) - PLID 45856 - Added procedure group id
		SetDlgItemText(IDC_PREVIEW_FORM_NUMBER, GetNewLabFormNumber(strFormat, m_nLabProcedureGroupID));
		Invalidate();
	
	} NxCatchAll("CLabFormNumberEditorDlg::UpdatePreview");
}

BOOL CLabFormNumberEditorDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch(HIWORD(wParam))
	{
	case EN_CHANGE:
		UpdatePreview();
		break;
	}

	return CDialog::OnCommand(wParam, lParam);
}

int CLabFormNumberEditorDlg::CalculateProjectedFormNumberLength()
{
	CString strFormat;
	GetDlgItemText(IDC_LAB_NUMBER_FORMAT, strFormat);

	int nLength = strFormat.GetLength();

	for(int i = 0; i < strFormat.GetLength(); i++) {
		if(strFormat.GetAt(i) == '%') {
			char ch = strFormat.GetAt(i+1);
			switch(ch)
			{
				case 'Y':
					// Add 2 because 4 digit year (4) - %Y (2) = 2
					nLength += 2;
				break;

				case 'n':
					// We need to add the incremental length and subtract 2 for the '%n'
					nLength = nLength + (int)GetDlgItemInt(IDC_INCREMENTAL_PORTION_DIGITS) - 2;
				break;
			}
		}
	}

	return nLength;
}

// (r.gonet 03/29/2012) - PLID 45856 - Enables and disables controls appropriately according to whether or not the group overrides the default format.
void CLabFormNumberEditorDlg::EnsureControls()
{
	BOOL bOverrideDefaultFormat = m_checkOverrideDefaultFormat.GetCheck() > 0 || m_nLabProcedureGroupID == -1 ? TRUE : FALSE;
	GetDlgItem(IDC_LFNE_FORMAT_LABEL)->EnableWindow(bOverrideDefaultFormat);
	m_nxeditLabNumberFormat.EnableWindow(bOverrideDefaultFormat);
	GetDlgItem(IDC_LFNE_FORMAT_SPECIFIER_LABEL_1)->EnableWindow(bOverrideDefaultFormat);
	GetDlgItem(IDC_LFNE_FORMAT_SPECIFIER_LABEL_2)->EnableWindow(bOverrideDefaultFormat);
	GetDlgItem(IDC_LFNE_NUM_DIGITS_LABEL)->EnableWindow(bOverrideDefaultFormat);
	m_nxeditIncrementalPortionDigits.EnableWindow(bOverrideDefaultFormat);
	GetDlgItem(IDC_LFNE_NEXT_VALUE_LABEL)->EnableWindow(bOverrideDefaultFormat);
	m_nxeditIncrementalPortionValue.EnableWindow(bOverrideDefaultFormat);
	m_nxstaticPreviewFormNumberLabel.EnableWindow(bOverrideDefaultFormat);
	m_checkUseSeparateCount.EnableWindow(bOverrideDefaultFormat);
	m_nxstaticPreviewFormNumber.EnableWindow(bOverrideDefaultFormat);

	m_checkOverrideDefaultFormat.ShowWindow(m_nLabProcedureGroupID == -1 ? SW_HIDE : SW_SHOW);
	m_checkUseSeparateCount.ShowWindow(m_nLabProcedureGroupID == -1 ? SW_HIDE : SW_SHOW);
}

// (r.gonet 03/29/2012) - PLID 45856 - Toggle the formatting fields
void CLabFormNumberEditorDlg::OnBnClickedLfneOverrideDefaultFormat()
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}