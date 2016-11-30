// EMRCopyTodosDlg.cpp : implementation file
//
// (c.haag 2008-07-16 11:54) - PLID 30752 - Initial implementation
//

#include "stdafx.h"
#include "EMRCopyTodosDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
	eclTaskID = 0L,
	eclCategory,
	eclMethod,
	eclPriority,
	eclNotes,
	eclAssignedTo,
	eclDeadline,
	eclDone,
	eclCopy
} ECopyTodosColumns;

/////////////////////////////////////////////////////////////////////////////
// CEMRCopyTodosDlg dialog


CEMRCopyTodosDlg::CEMRCopyTodosDlg(CArray<long,long>& anTodoIDs,
								   CStringArray& astrTodoDetailNames,
								   long nSourceEMNID,
								   CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRCopyTodosDlg::IDD, pParent),
	m_anTodoIDs(anTodoIDs),
	m_astrTodoDetailNames(astrTodoDetailNames),
	m_nSourceEMNID(nSourceEMNID)
{
	//{{AFX_DATA_INIT(CEMRCopyTodosDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEMRCopyTodosDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRCopyTodosDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRCopyTodosDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRCopyTodosDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRCopyTodosDlg message handlers

BOOL CEMRCopyTodosDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// Set the button icons
		m_btnOK.AutoSet(NXB_OK);

		// Bind the datalist
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_COPY_EMR_TODO_ALARMS, false);

		// Set up the Where Clause and requery
		CString strWhere = FormatString("TodoList.TaskID IN (%s)", ArrayAsString(m_anTodoIDs));
		m_dlList->WhereClause = _bstr_t(strWhere);
		m_dlList->Requery();

	}
	NxCatchAll("Error in CEMRCopyTodosDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRCopyTodosDlg::OnOK() 
{
	try {
		// (c.haag 2008-07-16 11:56) - Rebuild m_anTodoIDs
		m_anTodoIDs.RemoveAll();
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetFirstRow();
		while (NULL != pRow) {
			if (VarBool(pRow->Value[eclCopy])) {
				m_anTodoIDs.Add( VarLong(pRow->Value[eclTaskID]) );
			}
			pRow = pRow->GetNextRow();
		}
		CNxDialog::OnOK();
	}
	NxCatchAll("Error in CEMRCopyTodosDlg::OnOK");
}

void CEMRCopyTodosDlg::OnCancel()
{
	// We don't allow cancelling
}
