// SelectProcedureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectProcedureDlg.h"
#include "PracticeRc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectProcedureDlg dialog


CSelectProcedureDlg::CSelectProcedureDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectProcedureDlg::IDD, pParent)
{
	m_nProcedureID = -1;
	m_bIncludeNoProcedure = TRUE;
	//{{AFX_DATA_INIT(CSelectProcedureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectProcedureDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectProcedureDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_SELECT_PROCEDURE_CAPTION, m_nxstaticSelectProcedureCaption);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectProcedureDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectProcedureDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectProcedureDlg message handlers

BEGIN_EVENTSINK_MAP(CSelectProcedureDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectProcedureDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CSelectProcedureDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-04-25 15:24) - PLID 29793 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pProcList = BindNxDataListCtrl(this, IDC_SELECT_PROCEDURE_LIST, GetRemoteData(), true);

		// (a.walling 2007-03-27 14:48) - PLID 25376 - Only include no procedure if desired.
		if (m_bIncludeNoProcedure) {
			IRowSettingsPtr pRow = m_pProcList->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _bstr_t("{No Procedure}"));
			m_pProcList->InsertRow(pRow, 0);
		}

		GetDlgItem(IDC_SELECT_PROCEDURE_CAPTION)->SetWindowText(m_strCaption);

		if (m_nProcedureID > 0) {
			if (sriNoRow == m_pProcList->SetSelByColumn(0, m_nProcedureID)) {
				// (c.haag 2009-01-08 12:45) - PLID 32539 - If we didn't find the procedure, it may be inactive. Add
				// the inactive row.
				ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM ProcedureT WHERE ID = {INT}",m_nProcedureID);
				if (!prs->eof) {
					NXDATALISTLib::IRowSettingsPtr pRow = m_pProcList->GetRow(-1);
					pRow->PutValue(0,m_nProcedureID);
					pRow->PutValue(1,prs->Fields->Item["Name"]->Value);
					m_pProcList->AddRow(pRow);
					m_pProcList->Sort();
					m_pProcList->SetSelByColumn(0, m_nProcedureID);
				} else {
					// It was deleted. Nothing we can do.
				}
			}
		}
	}
	NxCatchAll("Error in CSelectProcedureDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (a.walling 2007-03-27 14:46) - PLID 25376 - Modularize this dialog
HRESULT CSelectProcedureDlg::Open(CString strCaption, BOOL bIncludeNoProcedure /* = TRUE*/, CString strBadSelectionWarning /* = "Please choose an item from the list."*/)
{
	m_strCaption = strCaption;
	m_bIncludeNoProcedure = bIncludeNoProcedure;
	m_strBadSelectionWarning = strBadSelectionWarning;

	return DoModal();
}

void CSelectProcedureDlg::OnOK() 
{
	if(m_pProcList->CurSel == -1) {
		// (a.walling 2007-03-27 14:57) - PLID 25376
		MessageBox(m_strBadSelectionWarning);
		return;
	}

	m_nProcedureID = VarLong(m_pProcList->GetValue(m_pProcList->CurSel, 0));
	m_strProcedureName = VarString(m_pProcList->GetValue(m_pProcList->CurSel, 1));

	CDialog::OnOK();
}
