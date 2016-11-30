// NexWebNoteChangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexWebNoteChangeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
/////////////////////////////////////////////////////////////////////////////
// CNexWebNoteChangeDlg dialog


CNexWebNoteChangeDlg::CNexWebNoteChangeDlg(long nNoteID,  CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebNoteChangeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexWebNoteChangeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nNoteID = nNoteID;
	m_strDataNote = "";
	m_dtDataDate = COleDateTime::GetCurrentTime();
}


void CNexWebNoteChangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebNoteChangeDlg)
	DDX_Control(pDX, IDC_DATA_NOTE, m_nxeditDataNote);
	DDX_Control(pDX, IDC_NEXWEB_NOTE, m_nxeditNexwebNote);
	DDX_Control(pDX, IDC_NOTE_GROUPBOX, m_btnNoteGroupbox);
	DDX_Control(pDX, IDC_CHANGE_GROUPBOX, m_btnChangeGroupbox);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebNoteChangeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebNoteChangeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebNoteChangeDlg message handlers

BOOL CNexWebNoteChangeDlg::CompareDates(COleDateTime dt1, COleDateTime dt2)  {

	if (dt1.GetYear() == dt2.GetYear()) {
		if (dt1.GetMonth() == dt2.GetMonth()) {
			if (dt1.GetDay() == dt2.GetDay()) {
				if (dt1.GetHour() == dt2.GetHour()) {
					if (dt1.GetMinute() == dt2.GetMinute()) {
						if (dt1.GetSecond() == dt2.GetSecond()) {
							return FALSE;
						}
						else {
							return TRUE;
						}
					}
					else {
						return TRUE;
					}
				}
				else {
					return TRUE;
				}
			}
			else {
				return TRUE;
			}
		}
		else {
			return TRUE;
		}
	}
	else {
		return TRUE;
	}
}


void CNexWebNoteChangeDlg::OnOK() 
{
	try {
		//check to see if anything changed
		COleDateTime dtDate = m_NexwebDate->GetDateTime();
		COleDateTime dtMin;
		dtMin.SetDate(1800,1,1);
		if (dtDate < dtMin || dtDate.GetStatus() != 0) {
			MessageBox("Please enter a valid date");
			return;
		}

		CString strNote;
		GetDlgItemText(IDC_NEXWEB_NOTE, strNote);
		if (strNote.GetLength() > 2000) {
			MessageBox("Note length cannot exceed 2000 characters.");
			return;
		}

		BOOL bDateChanged = CompareDates(m_dtDataDate, dtDate);

		if (bDateChanged || (m_strDataNote.CompareNoCase(strNote) != 0)) {
			if (IDYES == MsgBox(MB_YESNO, "You've made changes to the data from Nexweb, are you sure you want to overwrite the original NexWeb data? (Note: you can still choose whether or not to import this note)")) {
				//save the changes
				ExecuteSql("UPDATE NexwebTransactionsT SET Value = '%s' WHERE ID = (SELECT top 1 ID FROM NexwebTransactionsT WHERE ObjectID = %li AND Field = 2043 ORDER BY ID DESC)", FormatDateTimeForSql(dtDate), m_nNoteID);
				ExecuteSql("UPDATE NexwebTransactionsT SET Value = '%s' WHERE ID = (SELECT top 1 ID FROM NexwebTransactionsT WHERE ObjectID = %li AND Field = 2044 ORDER BY ID DESC)", strNote, m_nNoteID);
				m_dtDataDate = dtDate;
				m_strDataNote = strNote;
			}
		}	
		
		
		CDialog::OnOK();
	}NxCatchAll("Error Saving Changes");
}

BOOL CNexWebNoteChangeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//bind the date time controls
	m_DataDate  = GetDlgItemUnknown(IDC_DATA_DATE);
	m_NexwebDate  = GetDlgItemUnknown(IDC_NEXWEB_DATE);

	GetDlgItem(IDC_DATA_DATE)->EnableWindow(FALSE);

	
	//set the boxes
	_RecordsetPtr rs = CreateRecordset("SELECT Date, Note from Notes WHERE ID = %li", m_nNoteID);
	if (!rs->eof) {
		m_DataDate->SetDateTime(AdoFldDateTime(rs, "Date"));
		SetDlgItemText(IDC_DATA_NOTE, AdoFldString(rs, "Note"));
	}
	rs->Close();
	
	
	rs = CreateRecordset("SELECT "
		" CASE WHEN (SELECT top 1 Convert(datetime, value) FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2043) IS NULL then Notes.Date "
		" ELSE (SELECT top 1 Convert(datetime, value) FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2043 ORDER BY ID DESC) END as Date, "
		" CASE WHEN (SELECT top 1  value FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2044) IS NULL then Notes.Note"
		" ELSE (SELECT top 1 value FROM NexwebTransactionsT nTrans2 WHERE nTrans2.ObjectID = Notes.ID AND nTrans2.Field = 2044 ORDER BY ID DESC) END as Note "
		" FROM Notes WHERE ID = %li", m_nNoteID);
	if (!rs->eof) {
		m_dtDataDate = AdoFldDateTime(rs, "Date");
		m_strDataNote = AdoFldString(rs, "Note");
		m_NexwebDate->SetDateTime(m_dtDataDate);
		SetDlgItemText(IDC_NEXWEB_NOTE, m_strDataNote);
	}
	rs->Close();
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
