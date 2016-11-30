// InactiveICD9sDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InactiveICD9sDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CInactiveICD9sDlg dialog


CInactiveICD9sDlg::CInactiveICD9sDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInactiveICD9sDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInactiveICD9sDlg)
		m_Changed = FALSE;
	//}}AFX_DATA_INIT
}


void CInactiveICD9sDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInactiveICD9sDlg)
	DDX_Control(pDX, IDC_INACTIVE_TITLE_LABEL, m_nxstaticInactiveTitleLabel);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInactiveICD9sDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInactiveICD9sDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInactiveICD9sDlg message handlers

BOOL CInactiveICD9sDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_Changed = FALSE;

	// (z.manning, 05/01/2008) - PLID 29860 - Set button styles
	m_btnClose.AutoSet(NXB_CLOSE);
	
	m_List = BindNxDataListCtrl(this,IDC_INACTIVE_ICD9_LIST,GetRemoteData(),true);

	m_List->Requery();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CInactiveICD9sDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInactiveICD9sDlg)
	ON_EVENT(CInactiveICD9sDlg, IDC_INACTIVE_ICD9_LIST, 19 /* LeftClick */, OnLeftClickInactiveIcd9List, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInactiveICD9sDlg, IDC_INACTIVE_ICD9_LIST, 3 /* DblClickCell */, OnDblClickCellInactiveIcd9List, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CInactiveICD9sDlg, IDC_INACTIVE_ICD9_LIST, 18, CInactiveICD9sDlg::RequeryFinishedInactiveIcd9List, VTS_I2)
END_EVENTSINK_MAP()

void CInactiveICD9sDlg::OnLeftClickInactiveIcd9List(long nRow, short nCol, long x, long y, long nFlags) 
{
	//DRT 7/25/03 - Moved to double click!
}

void CInactiveICD9sDlg::RestoreItem(long ID)
{
	try {

		CString str, strDesc;

		strDesc = VarString(m_List->GetValue(m_List->CurSel, eidccCodeNumber),"") + " - " + VarString(m_List->GetValue(m_List->CurSel, eidccCodeDesc),"");		

		// (j.kuziel 2014-03-10) - PLID 61213 - We support ICD-10 now so this needs to read diagnosis code.
		str.Format("You are about to restore the diagnosis code '%s'.\n"
			"This code will become available for use and editing in the Administrator module, in billing, and in General 2.\n\n"
			"Are you sure you wish to restore this code to active use?",strDesc);

		if(IDYES==MessageBox(str,"Practice",MB_YESNO|MB_ICONQUESTION)) {

			ExecuteSql("UPDATE DiagCodes SET Active = 1 WHERE ID = %li",ID);

			m_List->RemoveRow(m_List->CurSel);

			m_Changed = TRUE;

			long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();

			// (a.walling 2007-08-06 12:29) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			CClient::RefreshTable(NetUtils::DiagCodes, ID);				
			if(nAuditID != -1)
				AuditEvent(-1, strDesc, nAuditID, aeiICD9Active, ID, "", "<Marked Active>",aepMedium, aetCreated);
		}
		else
			return;		

	}NxCatchAll("Error restoring item.");
}

void CInactiveICD9sDlg::OnDblClickCellInactiveIcd9List(long nRowIndex, short nColIndex) 
{
	//DRT 7/25/03 - Moved here from OnLeftClick
	if(nRowIndex == -1)
		return;

	try {

		long DiagID = m_List->GetValue(nRowIndex, 0).lVal;

		RestoreItem(DiagID);

	}NxCatchAll("Error selecting item.");
}

void CInactiveICD9sDlg::RequeryFinishedInactiveIcd9List(short nFlags)
{
	try {
		// (j.kuziel 2014-03-10) - PLID 61213 - Set the ICD-9 codes to light grey to distinguish them.
		long nRowCount = m_List->GetRowCount();
		for(int i = 0; i < nRowCount; i++) {
			NXDATALISTLib::IRowSettingsPtr pRow = m_List->GetRow(i);
			BOOL bICD10 = VarBool(m_List->GetValue(i, eidccICD10));
			if( ! bICD10) {
				pRow->BackColor = RGB( 239, 239, 239 );
			}
		}
	}NxCatchAll(__FUNCTION__);
}
