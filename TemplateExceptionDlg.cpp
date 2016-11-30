// TemplateExceptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TemplateExceptionDlg.h"
#include "TemplateExceptionInfo.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTemplateExceptionDlg dialog
//
// (c.haag 2006-11-13 10:02) - PLID 5993 - Initial implementation
//

CTemplateExceptionDlg::CTemplateExceptionDlg(CWnd* pParent /*= NULL*/)
	: CNxDialog(CTemplateExceptionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTemplateExceptionDlg)
	m_strDescription = _T("");
	//}}AFX_DATA_INIT
	m_pException = NULL;
}


void CTemplateExceptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateExceptionDlg)
	DDX_Text(pDX, IDC_EDIT_EXCEPTION_DESCRIPTION, m_strDescription);
	DDV_MaxChars(pDX, m_strDescription, 255);
	DDX_Control(pDX, IDC_EXCEPTION_DATE_END, m_dtpEndDate);
	DDX_Control(pDX, IDC_EXCEPTION_DATE_START, m_dtpStartDate);
	DDX_Control(pDX, IDC_EDIT_EXCEPTION_DESCRIPTION, m_nxeditEditExceptionDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DATES_GROUPBOX, m_btnDatesGroupbox);
	DDX_Control(pDX, IDC_EXCEPTION_TYPE_GROUPBOX, m_btnExceptionTypeGroupbox);
	DDX_Control(pDX, IDC_DESC_GROUPBOX, m_btnDescGroupbox);
	DDX_Control(pDX, IDC_CHECK_TOP_PRIORITY, m_checkTopPriority);
	DDX_Control(pDX, IDC_CHECK_IGNORE_TEMPLATE, m_checkIgnoreTemplate);
	//}}AFX_DATA_MAP
}

//TES 6/19/2010 - PLID 39262 - Added a parameter for whether we're using the Resource Availability templates
int CTemplateExceptionDlg::ZoomException(bool bUseResourceAvailTemplates, CTemplateExceptionInfo *pException, CArray<CTemplateExceptionInfo*, CTemplateExceptionInfo*>& apTemplateExceptions)
{
	ASSERT(pException);
	// (c.haag 2006-11-14 09:49) - m_nTemplateID can be -1 if we're dealing with a new template
	//ASSERT(pException->m_nTemplateID > -1);

	// Initialize the dialog
	m_pException = pException;
	m_apTemplateExceptions.RemoveAll();
	for (int i=0; i < apTemplateExceptions.GetSize(); i++) {
		m_apTemplateExceptions.Add( apTemplateExceptions[i] );
	}

	//TES 6/19/2010 - PLID 39262 - Remember wehther we're using Resource Availability templates
	m_bUseResourceAvailTemplates = bUseResourceAvailTemplates;

	// Run the dialog
	return DoModal();
}

void CTemplateExceptionDlg::SetFlags(long nFlags)
{
	CheckDlgButton(IDC_CHECK_TOP_PRIORITY, (nFlags & SCHED_TEMPLATE_FLAG_TOP_PRIORITY) ? 1 : 0);
	CheckDlgButton(IDC_CHECK_IGNORE_TEMPLATE, (nFlags & SCHED_TEMPLATE_FLAG_IGNORE) ? 1 : 0);
}

long CTemplateExceptionDlg::GetFlags()
{
	return (IsDlgButtonChecked(IDC_CHECK_TOP_PRIORITY) ? SCHED_TEMPLATE_FLAG_TOP_PRIORITY : 0) |
		(IsDlgButtonChecked(IDC_CHECK_IGNORE_TEMPLATE) ? SCHED_TEMPLATE_FLAG_IGNORE : 0);
}

BOOL CTemplateExceptionDlg::Load()
{
	// (c.haag 2006-11-13 10:10) - Load the description
	m_strDescription = m_pException->m_strDescription;

	// (c.haag 2006-11-13 10:11) - Load the dates
	// (a.walling 2008-05-13 10:36) - PLID 27591 - COleVariant conversion no longer necessary
	m_dtpStartDate.SetValue(m_pException->m_dtStartDate);
	m_dtpEndDate.SetValue(m_pException->m_dtEndDate);

	// (c.haag 2006-11-13 10:11) - Load the flags
	SetFlags(m_pException->m_nFlags);

	UpdateData(FALSE);
	return TRUE;
}

BOOL CTemplateExceptionDlg::Validate()
{
	int i;

	UpdateData(TRUE);

	// (c.haag 2006-11-13 10:04) - Make sure the description is valid
	CString strDesc = m_strDescription;
	strDesc.TrimLeft(" \r\n\t");
	strDesc.TrimRight(" \r\n\t");
	if (!strDesc.GetLength()) {
		MsgBox(MB_OK | MB_ICONWARNING, "You must enter a description for this template exception before saving your changes.");
		GetDlgItem(IDC_EDIT_EXCEPTION_DESCRIPTION)->SetFocus();
		return FALSE;
	} else if (strDesc.GetLength() > 255) {
		MsgBox(MB_OK | MB_ICONWARNING, "The description you have entered is too long. Please shorten your description.");
		GetDlgItem(IDC_EDIT_EXCEPTION_DESCRIPTION)->SetFocus();
		return FALSE;
	}

	// (c.haag 2006-11-13 11:44) - Make sure the dates are in order
	COleDateTime dtStartDate = m_dtpStartDate.GetValue();
	COleDateTime dtEndDate = m_dtpEndDate.GetValue();
	dtStartDate.SetDate( dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay() );
	dtEndDate.SetDate( dtEndDate.GetYear(), dtEndDate.GetMonth(), dtEndDate.GetDay() );
	if (dtStartDate > dtEndDate) {
		MsgBox(MB_OK | MB_ICONWARNING, "Please set the start date on before the end date before saving your changes.");
		return FALSE;
	}

	// (c.haag 2006-11-13 10:05) - Make sure the checkbox combination is valid
	long nFlags = GetFlags();
	if (!nFlags) {
		MsgBox(MB_OK | MB_ICONWARNING, "You must choose one exception rule before saving your changes.");
		return FALSE;
	} else if (nFlags == (SCHED_TEMPLATE_FLAG_TOP_PRIORITY | SCHED_TEMPLATE_FLAG_IGNORE)) {
		MsgBox(MB_OK | MB_ICONWARNING, "You cannot have both exception rules selected at the same time.");
		return FALSE;	
	}

	// (c.haag 2006-11-13 12:42) - Make sure exceptions have unique description for a template
	for (i=0; i < m_apTemplateExceptions.GetSize(); i++) {
		CTemplateExceptionInfo* pException = m_apTemplateExceptions[i];
		if (pException != m_pException) {
			if (!pException->m_strDescription.CompareNoCase( strDesc )) {
				MsgBox(MB_OK | MB_ICONWARNING, "There is another exception for this template with the same description. Please choose another description.");
				return FALSE;
			}
		}
	}

	// (c.haag 2006-11-13 12:27) - Go over the exception rules in this template to look for conflicting date ranges
	// (c.haag 2006-11-28 10:02) - A "conflicting date range" for a single template is one where two exceptions with
	// differing flags overlap
	CStringArray astrPriorityExceptions;
	for (i=0; i < m_apTemplateExceptions.GetSize(); i++) {
		CTemplateExceptionInfo* pException = m_apTemplateExceptions[i];
		if (pException != m_pException) {
			if ((pException->m_nFlags != nFlags) &&
				!((pException->m_dtStartDate > dtEndDate && pException->m_dtEndDate > dtEndDate) || (pException->m_dtStartDate < dtStartDate && pException->m_dtEndDate < dtStartDate))) {
				astrPriorityExceptions.Add(pException->FormatDescriptionText());
			}
		}
	}

	// (c.haag 2006-11-13 12:33) - Now warn the user if there are any conflicts
	if (astrPriorityExceptions.GetSize() > 0) {
		CString strMsg = "The following scheduler template exceptions conflict with this exception:\n\n";
		for (int i=0; i < astrPriorityExceptions.GetSize(); i++) {
			strMsg += astrPriorityExceptions[i] + "\n";
		}
		// (z.manning 2011-12-09 09:15) - PLID 46906 - Fixed a string formatting issue here
		MsgBox(MB_OK | MB_ICONWARNING, "%s\nPlease choose a different date range before saving this exception.", strMsg);
		return FALSE;	
	}

	// (c.haag 2006-11-13 12:25) - Now for the hard part: If this is a "top priority" exception,
	// meaning that the parent template has priority above all other templates in the given date
	// range, then we need to make sure no other template in the entire data has this rule.
	if (nFlags & SCHED_TEMPLATE_FLAG_TOP_PRIORITY) {
		try {
			//TES 6/19/2010 - PLID 39262 - Pull from the correct table
			_RecordsetPtr prsExceptions = CreateRecordset("SELECT * FROM %sTemplateExceptionT WHERE TemplateID <> %d", 
				m_bUseResourceAvailTemplates?"ResourceAvail":"",m_pException->m_nTemplateID);
			while (!prsExceptions->eof) {
				CTemplateExceptionInfo tei(prsExceptions->Fields);

				// Check for overlapping dates with top priority exceptions
				if ((tei.m_nFlags & SCHED_TEMPLATE_FLAG_TOP_PRIORITY) &&
					!((tei.m_dtStartDate > dtEndDate && tei.m_dtEndDate > dtEndDate) || (tei.m_dtStartDate < dtStartDate && tei.m_dtEndDate < dtStartDate))) {
					astrPriorityExceptions.Add(tei.FormatDescriptionText());
				}

				prsExceptions->MoveNext();
			}

			// (c.haag 2006-11-13 12:33) - Now warn the user if there are any conflicts
			if (astrPriorityExceptions.GetSize() > 0) {
				CString strMsg = "The following scheduler template exceptions flag other templates as top priority in your specified time range:\n\n";
				for (int i=0; i < astrPriorityExceptions.GetSize(); i++) {
					strMsg += astrPriorityExceptions[i] + "\n";
				}
				// (z.manning 2011-12-09 09:15) - PLID 46906 - Fixed a string formatting issue here
				MsgBox(MB_OK | MB_ICONWARNING, "%s\nPlease choose a different date range before saving this exception.", strMsg);
				return FALSE;	
			}

		} NxCatchAllCall("Error validating scheduler template exception", return FALSE;);
	}

	// Everything looks good
	return TRUE;
}

BOOL CTemplateExceptionDlg::Save()
{
	UpdateData(TRUE);

	// (c.haag 2006-11-13 10:09) - Save the description
	m_pException->m_strDescription = m_strDescription;
	m_pException->m_strDescription.TrimLeft();
	m_pException->m_strDescription.TrimRight();

	// (c.haag 2006-11-13 10:09) - Save the dates
	m_pException->m_dtStartDate = m_dtpStartDate.GetValue();
	m_pException->m_dtStartDate.SetDate(
		m_pException->m_dtStartDate.GetYear(),
		m_pException->m_dtStartDate.GetMonth(),
		m_pException->m_dtStartDate.GetDay()
		);

	m_pException->m_dtEndDate = m_dtpEndDate.GetValue();
	m_pException->m_dtEndDate.SetDate(
		m_pException->m_dtEndDate.GetYear(),
		m_pException->m_dtEndDate.GetMonth(),
		m_pException->m_dtEndDate.GetDay()
		);

	// (c.haag 2006-11-13 10:09) - Save the flags
	m_pException->m_nFlags = GetFlags();

	return TRUE;
}

BEGIN_MESSAGE_MAP(CTemplateExceptionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTemplateExceptionDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateExceptionDlg message handlers

BOOL CTemplateExceptionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/29/2008) - PLID 29814 - Set Button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	// (c.haag 2006-11-13 10:15) - We should not have to set any defaults here
	// because ZoomException should always be called first, and the exception
	// object itself has defaults

	Load();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTemplateExceptionDlg::OnOK() 
{
	if (!Validate()) {
		return;
	}
	if (!Save()) {
		return;
	}
	CNxDialog::OnOK();
}
