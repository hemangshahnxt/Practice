// ViewSupportHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ViewSupportHistoryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CViewSupportHistoryDlg dialog


CViewSupportHistoryDlg::CViewSupportHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CViewSupportHistoryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewSupportHistoryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CViewSupportHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewSupportHistoryDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewSupportHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CViewSupportHistoryDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_HIDEAUTOUPDATE, &CViewSupportHistoryDlg::OnBnClickedHideautoupdate)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSupportHistoryDlg message handlers

BOOL CViewSupportHistoryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_bHideAutoUpdate = false;

	try {
		m_btnClose.AutoSet(NXB_CLOSE);
		m_pList = BindNxDataListCtrl(this, IDC_SUPPORT_HISTORY_LIST, GetRemoteData(), false);
		LoadHistory();
	} NxCatchAll("Error loading support history dialog.");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CViewSupportHistoryDlg::OnOK() 
{
	CDialog::OnOK();
}

void CViewSupportHistoryDlg::OnCancel() 
{
	CDialog::OnCancel();
}

enum SupportHistoryList {
	shlID = 0,
	shlDate,
	// (j.camacho 2013-03-21 10:31) - PLID 53900 add day of week column
	shlDayofweek,
	shlUser,
	shlField,
	shlOld,
	shlNew, 
	shlNotes,
};

//loads history for GetActivePatientID()
void CViewSupportHistoryDlg::LoadHistory()
{
	//select all changes from the support audit table
	// (d.thompson 2009-08-11) - PLID 35166 - Hack!  We can't really change the DB fields, but we've renamed
	//	unicorn to ChaseHealthAdvance.  So I'm just going to catch it here and fix it for display purposes.
	_RecordsetPtr prs = CreateRecordset("SELECT ID, Date, Username, Replace(Fieldchanged, 'Unicorn', 'ChaseHealthAdvance') AS Fieldchanged, OldValue, NewValue, AuditNotes FROM NxSupportAuditT WHERE PersonID = %li", GetActivePatientID());
	while(!prs->eof) {
		FieldsPtr flds = prs->Fields;
		long nID = VarLong(flds->Item["ID"]->Value);
		COleDateTime dt = VarDateTime(flds->Item["Date"]->Value);
		CString strUser = VarString(flds->Item["Username"]->Value);
		CString strField = VarString(flds->Item["Fieldchanged"]->Value);
		CString strOld = VarString(flds->Item["OldValue"]->Value);
		CString strNew = VarString(flds->Item["NewValue"]->Value);
		CString strNotes = VarString(flds->Item["AuditNotes"]->Value);

		// (j.shoben 2013-06-24 11:05) - PLID 56163 - Checkbox to hide automatic update client download for auditing purposes
		if (m_bHideAutoUpdate && (strNotes.Find("Automatic Update") == 0)) {
			prs->MoveNext();
			continue;
		}

		//now load all these in the datalist
		IRowSettingsPtr pRow = m_pList->GetRow(sriNoRow);
		pRow->PutValue(shlID, (long)nID);
		_variant_t var;
		var.vt = VT_DATE;
		var.date = dt;
		pRow->PutValue(shlDate, var);
		// (j.camacho 2013-03-21 10:28) - PLID 53900 converting to day of week for history
		// Added additional column to the dialog IDD_VIEW_SUPPORT_HISTORY named "Day of the week"
		CString day;
		day = FormatDateTimeForInterface(dt,"%a");
		pRow->PutValue(shlDayofweek,(_bstr_t)day);
		pRow->PutValue(shlUser, _bstr_t(strUser));
		pRow->PutValue(shlField, _bstr_t(strField));
		pRow->PutValue(shlOld, _bstr_t(strOld));
		pRow->PutValue(shlNew, _bstr_t(strNew));
		pRow->PutValue(shlNotes, _bstr_t(strNotes));
		m_pList->AddRow(pRow);

		prs->MoveNext();
	}
}

// (j.shoben 2013-06-24 11:05) - PLID 56163 - Checkbox to hide automatic update client download for auditing purposes
void CViewSupportHistoryDlg::OnBnClickedHideautoupdate()
{
	try {
		if (m_bHideAutoUpdate)
			m_bHideAutoUpdate = false;
		else
			m_bHideAutoUpdate = true;

		m_pList->Clear();
		LoadHistory();
	} NxCatchAll(__FUNCTION__);
}
