// SchedCountSetupDlg.cpp : implementation file
//
// (c.haag 2009-12-22 16:00) - PLID 28977 - Initial implementation
//

#include "stdafx.h"
#include "Practice.h"
#include "SchedCountSetupDlg.h"
#include "SchedulerRc.h"

enum {
	ecChecked,
	ecID,
	ecName
} EColumns;

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (c.haag 2009-12-22 16:02) - PLID 28977 - Load settings
CSchedulerCountSettings::CSchedulerCountSettings()
{
	// Get the global property for the current count settings. We want this to be global to avoid
	// problems where individuals decide on their own counting methods, and cause confusion as they
	// can be different from screen to screen.
	//
	// This property is a string in the form "0,1,2,3,4,5" where 0 is the flag telling us whether 
	// to include non-patient appointments, and 1,2,3,4,5...are the appointment types we are HIDING
	// from the count.
	//
	// The reason why the string contains ID's of appt types to HIDE is so that no matter how we add a 
	// new appointment type to Practice, it is guaranteed to be included in the counts at the bottom of
	// the scheduler by default.
	//
	CString strProp = GetRemotePropertyText("SchedulerCountSettings", "", 0, "<None>");
	if (strProp.IsEmpty()) {
		// If we get here, this has never been set up before...probably a client going to the scheduler
		// for the first time. Make sure the map is clear and we include non-patient appts.
		m_mapAptTypesToHide.RemoveAll();
		m_bIncludeNonPtAppts = TRUE;
	}
	else {
		// If we get here, we have an existing value. Parse everything out.
		// String is in the form "0,1,2,3,4,5" where 0 is the flag telling us whether 
		// to include non-patient appointments, and 1,2,3,4,5...are the appointment types
		// to HIDE from the scheduler counts.
		m_bIncludeNonPtAppts = (atol(strProp.Left(1)) != 0) ? TRUE : FALSE;
		CArray<long,long> anIDsOfHiddenTypes;
		if (strProp.GetLength() > 2) {
			ParseDelimitedStringToLongArray(strProp.Right( strProp.GetLength()-2 ), ",", anIDsOfHiddenTypes);
			for (int i=0; i < anIDsOfHiddenTypes.GetSize(); i++) {
				m_mapAptTypesToHide.SetAt(anIDsOfHiddenTypes[i], TRUE);
			}
		} else {
			// No appt types selected
		}
	}
}

CSchedulerCountSettings::CSchedulerCountSettings(BOOL bIncludeNonPtAppts, const CArray<long,long>& anIDsOfHiddenTypes)
{
	m_bIncludeNonPtAppts = bIncludeNonPtAppts;
	for (int i=0; i < anIDsOfHiddenTypes.GetSize(); i++) {
		m_mapAptTypesToHide.SetAt(anIDsOfHiddenTypes[i], TRUE);
	}
}

// (c.haag 2009-12-22 16:02) - PLID 28977 - Save settings in the form of a comma-delimited string of numbers.
// The first number is the "include non-patient appts" flag, the rest are types to exclude from counting.
void CSchedulerCountSettings::Save()
{
	// First, get a map of all appointment type ID's. If we have an ID in m_mapAptTypesToHide, but it doesn't
	// exist in AptTypeT, then we need to get rid of it.
	CMap<long,long,BOOL,BOOL> mapAptTypeIDs;
	_RecordsetPtr prs = CreateRecordset("SELECT ID FROM AptTypeT");
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		mapAptTypeIDs.SetAt( AdoFldLong(f, "ID"), TRUE );
		prs->MoveNext();
	}
	prs->Close();
	mapAptTypeIDs.SetAt(-1, TRUE); // "< No Type >" is also valid in this map.

	// Now begin building our property string
	CString strProp;
	// Start with including whether to include non-patient appointments
	strProp.Format("%d,", (m_bIncludeNonPtAppts) ? 1 : 0);
	// Now go through our map of appointment types that we want to hide, and 
	// add those ID's to the string if they exist in data
	POSITION pos = m_mapAptTypesToHide.GetStartPosition();
	while (pos) {
		long nTypeID;
		BOOL bSelected;
		m_mapAptTypesToHide.GetNextAssoc(pos, nTypeID, bSelected);
		if (bSelected) {
			// The user intends to hide this ID from the count...but we must be sure it's in data first.
			BOOL bExistsInData = FALSE;
			if (mapAptTypeIDs.Lookup(nTypeID, bExistsInData)) {
				if (bExistsInData) {
					strProp += FormatString("%d,", nTypeID);
				}
			}
		}
	}
	strProp.TrimRight(",");
	SetRemotePropertyText("SchedulerCountSettings", strProp, 0, "<None>");
}

// CSchedCountSetupDlg dialog

IMPLEMENT_DYNAMIC(CSchedCountSetupDlg, CNxDialog)

CSchedCountSetupDlg::CSchedCountSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedCountSetupDlg::IDD, pParent)
{

}

CSchedCountSetupDlg::~CSchedCountSetupDlg()
{
}

void CSchedCountSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_SCHEDCOUNTSETUP, m_nxStatic);
	DDX_Control(pDX, IDC_CHECK_INCLUDE_NONPT_APPTS, m_checkIncludeNonPtAppts);
	DDX_Control(pDX, IDC_BTN_APPTCT_SELECTALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_APPTCT_UNSELECTALL, m_btnUnselectAll);
}


BEGIN_MESSAGE_MAP(CSchedCountSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_APPTCT_SELECTALL, &CSchedCountSetupDlg::OnBnClickedBtnApptctSelectall)
	ON_BN_CLICKED(IDC_BTN_APPTCT_UNSELECTALL, &CSchedCountSetupDlg::OnBnClickedBtnApptctUnselectall)
	ON_BN_CLICKED(IDOK, &CSchedCountSetupDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSchedCountSetupDlg message handlers

BOOL CSchedCountSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// Initialize controls
		m_dlAptType = BindNxDataList2Ctrl(IDC_COUNT_TYPE_LIST);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Disable the OK button and list until everything is loaded
		m_btnOK.EnableWindow(FALSE);
		GetDlgItem(IDC_COUNT_TYPE_LIST)->EnableWindow(FALSE);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSchedCountSetupDlg::OnBnClickedBtnApptctSelectall()
{
	try {
		IRowSettingsPtr pRow = m_dlAptType->GetFirstRow();
		while (pRow) {
			pRow->Value[ecChecked] = _variant_t(VARIANT_TRUE, VT_BOOL);
			pRow = pRow->GetNextRow();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CSchedCountSetupDlg::OnBnClickedBtnApptctUnselectall()
{
	try {
		IRowSettingsPtr pRow = m_dlAptType->GetFirstRow();
		while (pRow) {
			pRow->Value[ecChecked] = _variant_t(VARIANT_FALSE, VT_BOOL);
			pRow = pRow->GetNextRow();
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CSchedCountSetupDlg::OnBnClickedOk()
{
	try {
		// Get the settings from the form
		const BOOL bIncludeNonPtAppts = m_checkIncludeNonPtAppts.GetCheck();
		CArray<long,long> anTypeIDsToHide;
		IRowSettingsPtr pRow = m_dlAptType->GetFirstRow();
		while (pRow) {
			if (FALSE == VarBool(pRow->Value[ecChecked])) {
				anTypeIDsToHide.Add( VarLong(pRow->Value[ecID]) );
			}
			pRow = pRow->GetNextRow();
		}

		// Now save them
		CSchedulerCountSettings s(bIncludeNonPtAppts, anTypeIDsToHide);
		s.Save();

		CNxDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CSchedCountSetupDlg, CNxDialog)
	ON_EVENT(CSchedCountSetupDlg, IDC_COUNT_TYPE_LIST, 18, CSchedCountSetupDlg::RequeryFinishedCountTypeList, VTS_I2)
END_EVENTSINK_MAP()

void CSchedCountSetupDlg::RequeryFinishedCountTypeList(short nFlags)
{
	try {
		// Load the current settings
		CSchedulerCountSettings s;
		if (s.GetIncludeNonPtAppts()) 
		{
			m_checkIncludeNonPtAppts.SetCheck(1);
		}
		else {
			m_checkIncludeNonPtAppts.SetCheck(0);
		}
		IRowSettingsPtr pRow = m_dlAptType->GetFirstRow();
		while (pRow) {
			if (s.IsAptTypeAllowed(VarLong(pRow->Value[ecID]))) {
				pRow->Value[ecChecked] = _variant_t(VARIANT_TRUE, VT_BOOL);
			} else {
				pRow->Value[ecChecked] = _variant_t(VARIANT_FALSE, VT_BOOL);
			}
			pRow = pRow->GetNextRow();
		}

		// Enable the OK button and list now that everything is loaded
		m_btnOK.EnableWindow(TRUE);
		GetDlgItem(IDC_COUNT_TYPE_LIST)->EnableWindow(TRUE);
	}
	NxCatchAll(__FUNCTION__);
}
