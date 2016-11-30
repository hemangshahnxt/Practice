// ProcedureGroupsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "ProcedureGroupsDlg.h"
#include "GlobalDataUtils.h"
#include "EditComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CProcedureGroupsDlg dialog


using namespace ADODB;

CProcedureGroupsDlg::CProcedureGroupsDlg(CWnd* pParent)
	: CNxDialog(IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcedureGroupsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProcedureGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureGroupsDlg)
	DDX_Control(pDX, IDC_DESEL_PROC, m_btDesel);
	DDX_Control(pDX, IDC_SEL_PROC, m_btSel);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcedureGroupsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SEL_PROC, OnSelProc)
	ON_BN_CLICKED(IDC_DESEL_PROC, OnDeselProc)
	ON_BN_CLICKED(IDC_EDIT_PROC_GROUPS, OnEditProcGroups)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcedureGroupsDlg message handlers

BOOL CProcedureGroupsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_pProcGroups = BindNxDataListCtrl(IDC_PROC_GROUP_COMBO);
	m_pAvailProcs = BindNxDataListCtrl(IDC_AVAIL_PROCS);
	m_pSelectProcs = BindNxDataListCtrl(IDC_SELECT_PROCS, false);
	m_pLadderTemplates = BindNxDataListCtrl(IDC_LADDER);

	m_pProcGroups->CurSel = 0;

	m_pProcGroups->TrySetSelByColumn(0, (long)m_nProcGroupID);

	m_btDesel.AutoSet(NXB_LEFT);
	m_btSel.AutoSet(NXB_RIGHT);
	m_btnClose.AutoSet(NXB_CLOSE);

	Refresh();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcedureGroupsDlg::Refresh() 
{
	
	long nCurGroupID = -1;
	if(m_pProcGroups->CurSel != -1)
		nCurGroupID = VarLong(m_pProcGroups->GetValue(m_pProcGroups->CurSel, 0));

	if(nCurGroupID == -1) {
		m_pSelectProcs->Clear();
		m_pLadderTemplates->CurSel = -1;
	}
	else {
		CString strWhere;
		strWhere.Format("ProcedureGroupID = %li AND MasterProcedureID Is Null", nCurGroupID);
		m_pSelectProcs->WhereClause = _bstr_t(strWhere);
		m_pSelectProcs->Requery();

		_RecordsetPtr rsLadderTemplateID = CreateRecordset("SELECT LadderTemplateID FROM ProcedureGroupsT WHERE ID = %li", nCurGroupID);
		m_pLadderTemplates->SetSelByColumn(0, AdoFldLong(rsLadderTemplateID, "LadderTemplateID", -1));

	}

	m_pAvailProcs->Requery();

	//OK, if there are no Groups in the data, we need to disable everything.
	EnableAppropriateWindows();

}



BEGIN_EVENTSINK_MAP(CProcedureGroupsDlg, CNxDialog)
	ON_EVENT(CProcedureGroupsDlg, IDC_PROC_GROUP_COMBO, 2 /* SelChanged */, OnSelChangedProcGroupCombo, VTS_I4)
	ON_EVENT(CProcedureGroupsDlg, IDC_AVAIL_PROCS, 3 /* DblClickCell */, OnDblClickCellAvailProcs, VTS_I4 VTS_I2)
	ON_EVENT(CProcedureGroupsDlg, IDC_SELECT_PROCS, 3 /* DblClickCell */, OnDblClickCellSelectProcs, VTS_I4 VTS_I2)
	ON_EVENT(CProcedureGroupsDlg, IDC_LADDER, 16 /* SelChosen */, OnSelChosenLadder, VTS_I4)
END_EVENTSINK_MAP()

void CProcedureGroupsDlg::OnSelChangedProcGroupCombo(long nNewSel) 
{
	try {
		Refresh();
	}NxCatchAll(__FUNCTION__);
}

void CProcedureGroupsDlg::OnSelProc() 
{
	try {
		if(m_pAvailProcs->GetCurSel() == -1 || m_pProcGroups->GetCurSel() == sriNoRow) {
			return;
		}

		ExecuteSql("UPDATE ProcedureT SET ProcedureGroupID = %li WHERE ID = %li", VarLong(m_pProcGroups->GetValue(m_pProcGroups->CurSel, 0), -1), VarLong(m_pAvailProcs->GetValue(m_pAvailProcs->CurSel, 0)));

		m_pSelectProcs->TakeRow(m_pAvailProcs->GetRow(m_pAvailProcs->CurSel));
	}NxCatchAll("Error in CProcedureGroupsDlg::OnSelProc");
}

void CProcedureGroupsDlg::OnDeselProc() 
{
	try {
		if(m_pSelectProcs->CurSel == -1 || m_pProcGroups->GetCurSel() == sriNoRow) {
			return;
		}

		ExecuteSql("UPDATE ProcedureT SET ProcedureGroupID = -1 WHERE ID = %li", VarLong(m_pSelectProcs->GetValue(m_pSelectProcs->CurSel, 0)));

		m_pAvailProcs->TakeRow(m_pSelectProcs->GetRow(m_pSelectProcs->CurSel));
	}NxCatchAll("Error in CProcedureGroupsDlg::OnDeselProc");
}

void CProcedureGroupsDlg::OnDblClickCellAvailProcs(long nRowIndex, short nColIndex) 
{
	OnSelProc();
}

void CProcedureGroupsDlg::OnDblClickCellSelectProcs(long nRowIndex, short nColIndex) 
{
	OnDeselProc();
}

void CProcedureGroupsDlg::OnSelChosenLadder(long nRow) 
{
	try {
		if(m_pProcGroups->GetCurSel() == sriNoRow) {
			return;
		}
		
		if(nRow != sriNoRow){
			ExecuteSql("UPDATE ProcedureGroupsT SET LadderTemplateID = %li WHERE ID = %li", VarLong(m_pLadderTemplates->GetValue(nRow, 0)), VarLong(m_pProcGroups->GetValue(m_pProcGroups->CurSel, 0)));
		}
		else{
			ExecuteSql("UPDATE ProcedureGroupsT SET LadderTemplateID = NULL WHERE ID = %li", VarLong(m_pProcGroups->GetValue(m_pProcGroups->CurSel, 0)));
		}

	}NxCatchAll("Error in CProcedureGroupsDlg::OnSelChoseLadder");
}

void CProcedureGroupsDlg::OnEditProcGroups() 
{
	try {
		_variant_t value;
		long curSel = m_pProcGroups->CurSel;
		if (curSel != -1)
			value = m_pProcGroups->Value[curSel][1];

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 7, m_pProcGroups, "Edit Procedure Groups").DoModal();

		if (curSel != -1)
			m_pProcGroups->SetSelByColumn(1, value);
		else
			m_pProcGroups->CurSel = 0;

		Refresh();
	}NxCatchAll(__FUNCTION__);
}

void CProcedureGroupsDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}

void CProcedureGroupsDlg::EnableAppropriateWindows()
{
	// if there is no procedure selected, the user should not be able to change anything
	if(m_pProcGroups->GetCurSel() == sriNoRow) {
		GetDlgItem(IDC_AVAIL_PROCS)->EnableWindow(FALSE);
		GetDlgItem(IDC_SELECT_PROCS)->EnableWindow(FALSE);
		GetDlgItem(IDC_SEL_PROC)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESEL_PROC)->EnableWindow(FALSE);
		GetDlgItem(IDC_LADDER)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_AVAIL_PROCS)->EnableWindow(TRUE);
		GetDlgItem(IDC_SELECT_PROCS)->EnableWindow(TRUE);
		GetDlgItem(IDC_SEL_PROC)->EnableWindow(TRUE);
		GetDlgItem(IDC_DESEL_PROC)->EnableWindow(TRUE);
		GetDlgItem(IDC_LADDER)->EnableWindow(TRUE);
	}
}
