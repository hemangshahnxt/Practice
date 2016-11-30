// AddProcedureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AddProcedureDlg.h"
#include "PhaseTracking.h"
#include "GlobalDataUtils.h"
#include "ProcedureInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CAddProcedureDlg dialog


CAddProcedureDlg::CAddProcedureDlg(CWnd* pParent)
	: CNxDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddProcedureDlg)
		m_bProcedures = true;
		m_bAllowProcGroups = true;
	//}}AFX_DATA_INIT
}


void CAddProcedureDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddProcedureDlg)
	DDX_Control(pDX, IDC_PROCEDURES, m_btnProcedures);
	DDX_Control(pDX, IDC_PROC_GROUPS, m_btnProcedureGroups);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddProcedureDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAddProcedureDlg)
	ON_BN_CLICKED(IDC_PROC_GROUPS, OnProcGroups)
	ON_BN_CLICKED(IDC_PROCEDURES, OnProcedures)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CAddProcedureDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAddProcedureDlg)
	ON_EVENT(CAddProcedureDlg, IDC_PROCEDURE_LIST, 19 /* LeftClick */, OnLeftClickProcedureList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CAddProcedureDlg message handlers

BOOL CAddProcedureDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	if(m_strProcWhereClause == "") {
		m_procedureList = BindNxDataListCtrl(IDC_PROCEDURE_LIST);
	}
	else {
		m_procedureList = BindNxDataListCtrl(IDC_PROCEDURE_LIST, false);
		// (c.haag 2008-12-18 10:34) - PLID 10776 - Filter out inactive procedures (in case the caller did not)
		m_procedureList->WhereClause = _bstr_t(m_strProcWhereClause + " AND ProcedureT.Inactive = 0");
		m_procedureList->Requery();
	}
	if(m_strGroupWhereClause == "") {
		m_pProcGroupList = BindNxDataListCtrl(IDC_PROC_GROUP_LIST);
	}
	else {
		m_pProcGroupList = BindNxDataListCtrl(IDC_PROC_GROUP_LIST, false);
		m_pProcGroupList->WhereClause = _bstr_t(m_strGroupWhereClause);
		m_pProcGroupList->Requery();
	}

	if(!m_bAllowProcGroups) {
		//The radio button will be checked, but invisible.
		CheckRadioButton(IDC_PROCEDURES, IDC_PROC_GROUPS, IDC_PROCEDURES);
		GetDlgItem(IDC_PROCEDURES)->EnableWindow(FALSE);
		GetDlgItem(IDC_PROC_GROUPS)->EnableWindow(FALSE);
		m_bProcedures = true;
		OnProcedures();
	}
	else if(!GetRemotePropertyInt("AddProcedureGroups", 0, 0, GetCurrentUserName(), true)) {
		m_bProcedures = true;
		CheckRadioButton(IDC_PROCEDURES, IDC_PROC_GROUPS, IDC_PROCEDURES);
		OnProcedures();
	}
	else {
		m_bProcedures = false;
		CheckRadioButton(IDC_PROCEDURES, IDC_PROC_GROUPS, IDC_PROC_GROUPS);
		OnProcGroups(); 
	}

	// (c.haag 2008-04-22 16:49) - PLID 29751 - NxIconify
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	LoadFromArray();
	
	return TRUE;
}

void CAddProcedureDlg::LoadFromArray()
{
	try {
		variant_t varTrue;
		varTrue.vt = VT_BOOL;
		varTrue.boolVal = TRUE;
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		int i = 0;

		for (i = 0; i < m_arProcIDs.GetSize(); i++)
		{	
			long id = m_procedureList->FindByColumn(0, (long)m_arProcIDs[i], 0, FALSE);
			if(id >= 0)
				m_procedureList->Value[id][1] = varTrue;
		}

		
		 for (i = 0; i < m_arProcGroupIDs.GetSize(); i++)
		{
			long id = m_pProcGroupList->FindByColumn(0, (long)m_arProcGroupIDs[i], 0, FALSE);
			if(id >= 0) 
				m_pProcGroupList->Value[id][1] = varTrue;
		}
	}NxCatchAll("Error in CAddProcedureDlg::LoadFromArray()");
}

void CAddProcedureDlg::UpdateArray()
{
	try
	{
		//Update the procedures list.
		long	i = 0,
				p = m_procedureList->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		m_arProcIDs.RemoveAll();
		//(e.lally 2009-03-11) PLID 33453 - Clear out the procedure string name array too.
		m_arProcString.RemoveAll();

		while (p)
		{
			m_procedureList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if (VarBool(pRow->Value[1]) != FALSE) {
				m_arProcIDs.Add(VarLong(pRow->Value[0]));
				m_arProcString.Add(VarString(pRow->Value[2]));
			}
			pDisp->Release();
		}

		//Now, update the procedure groups list.
		i = 0;
		p = m_pProcGroupList->GetFirstRowEnum();

		pDisp = NULL;

		m_arProcGroupIDs.RemoveAll();
		//(e.lally 2009-03-11) PLID 33453 - Clear out the procedure string group array too.
		m_arProcGroupString.RemoveAll();

		while(p) {
			m_pProcGroupList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			if(VarBool(pRow->Value[1]) != FALSE) {
				m_arProcGroupIDs.Add(VarLong(pRow->Value[0]));
				m_arProcGroupString.Add(VarString(pRow->Value[2]));
			}
			pDisp->Release();
		}
	}
	NxCatchAll("Could not remove all rows from export list");
}

void CAddProcedureDlg::OnOK() 
{
	if(m_bAllowProcGroups) { //If they weren't even allowed to choose, don't save their choice.
		if(m_bProcedures) {
			SetRemotePropertyInt("AddProcedureGroups", 0, 0, GetCurrentUserName());
		}
		else {
			SetRemotePropertyInt("AddProcedureGroups", 1, 0, GetCurrentUserName());
		}
	}
	UpdateArray();
	EndDialog(IDOK);
}

void CAddProcedureDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CAddProcedureDlg::OnLeftClickProcedureList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow == -1)
		return;

	switch(nCol)
	{
		case 2:
		{	
			CProcedureInfo dlg(this);
			dlg.m_arProcIDs.Add(VarLong(m_procedureList->Value[nRow][0]));
			dlg.DoModal();
			break;
		}
	}
	
}

void CAddProcedureDlg::OnProcGroups() 
{
	GetDlgItem(IDC_PROCEDURE_LIST)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_PROC_GROUP_LIST)->ShowWindow(SW_SHOW);
	m_bProcedures = false;		
}

void CAddProcedureDlg::OnProcedures() 
{
	GetDlgItem(IDC_PROCEDURE_LIST)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_PROC_GROUP_LIST)->ShowWindow(SW_HIDE);
	m_bProcedures = true;
}


void CAddProcedureDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	// (a.walling 2012-07-16 12:27) - PLID 46648 - Dialogs must set a parent
	//CMainFrame* pMain = GetMainFrame();
	//if(pMain) {
	//	pMain->ActivateFrame();
	//	pMain->BringWindowToTop();
	//}

	//SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);	
}

void CAddProcedureDlg::Refresh()
{
	if(m_procedureList != NULL && m_pProcGroupList != NULL) {
		// (c.haag 2008-12-18 10:34) - PLID 10776 - Filter out inactive procedures (in case the caller did not)
		CString str = m_strProcWhereClause;
		if (!str.IsEmpty()) { str += " AND ProcedureT.Inactive = 0"; }
		m_procedureList->WhereClause = _bstr_t(str);
		m_procedureList->Requery();

		m_pProcGroupList->WhereClause = _bstr_t(m_strGroupWhereClause);
		m_pProcGroupList->Requery();

		LoadFromArray();
		UpdateArray();
	}
}
