// NexSyncEditSubjectLineDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "NexSyncEditSubjectLineDlg.h"
#include "NxOutlookUtils.h"


// CNexSyncEditSubjectLineDlg dialog
// (z.manning 2009-10-19 16:44) - PLID 35997 - Created

IMPLEMENT_DYNAMIC(CNexSyncEditSubjectLineDlg, CNxDialog)

CNexSyncEditSubjectLineDlg::CNexSyncEditSubjectLineDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexSyncEditSubjectLineDlg::IDD, pParent)
{
	SetDefaultWindowText("Edit NexSync Subject Line");
	SetDefaultDescription("Please define the layout of the appointment subject line. This will be the text that appears in the subject line of the appointment on your phone.");
	SetDefaultSampleText("Sample Subject Line");
	SetDefaultSubjectText("Subject");
	// (c.haag 2010-03-31 11:35) - PLID 37891 - Set the default callback for populating fields
	m_cbFillSubjectLineList = NxOutlookUtils::FillSubjectLineFieldList;
	// (c.haag 2010-04-01 17:13) - PLID 37891 - Set the default callback for populating the sample box
	m_cbHandleSubjectLineEditChange = NxOutlookUtils::HandleSubjectLineEditChange;
	m_cbHandleSubjectLineFieldsListEditingFinished = NxOutlookUtils::HandleSubjectLineFieldsListEditingFinished;
}

CNexSyncEditSubjectLineDlg::~CNexSyncEditSubjectLineDlg()
{
}

// (c.haag 2010-03-29 13:15) - PLID 37891 - We can now configure the default window text
void CNexSyncEditSubjectLineDlg::SetDefaultWindowText(const CString& str)
{
	m_strDefaultWindowText = str;
}

// (c.haag 2010-03-29 13:15) - PLID 37891 - We can now configure the default description
void CNexSyncEditSubjectLineDlg::SetDefaultDescription(const CString& str)
{
	m_strDefaultDescription = str;
}

// (c.haag 2010-04-01 12:13) - PLID 37891 - Assigns a callback for populating the text above the preview
void CNexSyncEditSubjectLineDlg::SetDefaultSampleText(const CString& str)
{
	m_strDefaultSampleText = str;
}

// (c.haag 2010-04-01 12:13) - PLID 37891 - Assigns a callback for populating the text above the subject format
void CNexSyncEditSubjectLineDlg::SetDefaultSubjectText(const CString& str)
{
	m_strDefaultSubjectText = str;
}

// (c.haag 2010-03-31 11:36) - PLID 37891 - Assigns a callback for populating the subject field list
void CNexSyncEditSubjectLineDlg::SetSubjectLineListCallbackFunc(FillNexSyncEditSubjectLineListCallbackFunc func)
{
	m_cbFillSubjectLineList = func;
}

// (c.haag 2010-04-01 17:14) - PLID 37891 - Assigns a callback for populating the sample box
void CNexSyncEditSubjectLineDlg::SetHandleSubjectLineEditChangeCallbackFunc(HandleSubjectLineEditChangeCallbackFunc func)
{
	m_cbHandleSubjectLineEditChange = func;
}

// (c.haag 2010-04-01 17:37) - PLID 37891 - Assigns a callback for populating the sample box
void CNexSyncEditSubjectLineDlg::SetHandleSubjectLineFieldsListEditingFinishedCallbackFunc(HandleSubjectLineFieldsListEditingFinishedCallbackFunc func)
{
	m_cbHandleSubjectLineFieldsListEditingFinished = func;
}

void CNexSyncEditSubjectLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEXSYNC_SUBJECT_BACKGROUND, m_nxcolorBackground);
	DDX_Control(pDX, IDC_NEXSYNC_SUBJECT_EDIT, m_nxeditSubject);
	DDX_Control(pDX, IDC_SUBJECT_PREVIEW, m_nxeditSubjectPreview);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Text(pDX, IDC_NEXSYNC_SUBJECT_EDIT, m_strSubject);
	DDX_Control(pDX, IDC_STATIC_DESCRIPTION, m_nxsDescription);
	DDX_Control(pDX, IDC_STATIC_SAMPLE, m_nxsSample);
	DDX_Control(pDX, IDC_STATIC_SUBJECT, m_nxsSubject);
}


BEGIN_MESSAGE_MAP(CNexSyncEditSubjectLineDlg, CNxDialog)
	ON_EN_CHANGE(IDC_NEXSYNC_SUBJECT_EDIT, &CNexSyncEditSubjectLineDlg::OnEnChangeNexsyncSubjectEdit)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNexSyncEditSubjectLineDlg, CNxDialog)
	ON_EVENT(CNexSyncEditSubjectLineDlg, IDC_SUBJECT_LINE_FIELD_LIST, 10, CNexSyncEditSubjectLineDlg::EditingFinishedSubjectLineFieldList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

// CNexSyncEditSubjectLineDlg message handlers

BOOL CNexSyncEditSubjectLineDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		// (c.haag 2010-03-29 13:15) - PLID 37891 - Set the window texts
		SetWindowText(m_strDefaultWindowText);
		SetDlgItemText(IDC_STATIC_DESCRIPTION, m_strDefaultDescription);
		SetDlgItemText(IDC_STATIC_SAMPLE, m_strDefaultSampleText);
		SetDlgItemText(IDC_STATIC_SUBJECT, m_strDefaultSubjectText);

		m_nxcolorBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_nxeditSubject.SetLimitText(255);

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pdlFields = BindNxDataListCtrl(IDC_SUBJECT_LINE_FIELD_LIST, false);

		// (c.haag 2010-03-31 11:34) - PLID 37891 - We can now override how the field list is populate
		if (NULL != m_cbFillSubjectLineList) {
			m_cbFillSubjectLineList(m_pdlFields);
		} 
		else {
			ThrowNxException("Failed to populate the subject line list");
		}
		OnEnChangeNexsyncSubjectEdit();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CNexSyncEditSubjectLineDlg::OnEnChangeNexsyncSubjectEdit()
{
	try
	{
		// (c.haag 2010-04-01 17:13) - PLID 37891 - We now go through a callback
		m_cbHandleSubjectLineEditChange(&m_nxeditSubject, &m_nxeditSubjectPreview, m_pdlFields);

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncEditSubjectLineDlg::EditingFinishedSubjectLineFieldList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		// (c.haag 2010-04-01 17:34) - PLID 37891 - We now go through a callback
		m_cbHandleSubjectLineFieldsListEditingFinished(this, &m_nxeditSubject, &m_nxeditSubjectPreview, m_pdlFields, nRow, varNewValue, m_strSubject);

	}NxCatchAll(__FUNCTION__);
}

CString CNexSyncEditSubjectLineDlg::GetSubject()
{
	return m_strSubject;
}

void CNexSyncEditSubjectLineDlg::SetSubject(const CString &strSubject)
{
	m_strSubject = strSubject;
}