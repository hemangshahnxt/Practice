// SelectPrepayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SelectPrepayDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectPrepayDlg dialog


CSelectPrepayDlg::CSelectPrepayDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectPrepayDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectPrepayDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectPrepayDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectPrepayDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectPrepayDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectPrepayDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectPrepayDlg message handlers

BOOL CSelectPrepayDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pPrepayList = BindNxDataListCtrl(this, IDC_PREPAY_LIST, NULL, false);

	m_rsPrepays->MoveFirst();
	FieldsPtr fPrepays = m_rsPrepays->Fields;
	while(!m_rsPrepays->eof) {
		IRowSettingsPtr pRow = m_pPrepayList->GetRow(-1);
		pRow->PutValue(0, fPrepays->GetItem("ID")->Value);
		pRow->PutValue(1, fPrepays->GetItem("Date")->Value);
		pRow->PutValue(2, fPrepays->GetItem("Description")->Value);
		pRow->PutValue(3, fPrepays->GetItem("Amount")->Value);
		m_pPrepayList->AddRow(pRow);
		m_rsPrepays->MoveNext();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectPrepayDlg::OnOK() 
{
	LPDISPATCH lpRow = NULL;
	long p = m_pPrepayList->GetFirstSelEnum();
	while (p) {
		//this will output the next row's p, and the current row's lpRow.
		m_pPrepayList->GetNextSelEnum(&p, &lpRow);
		
		
		IRowSettingsPtr pRow(lpRow);
		//Add pay id to array
		long nPayID = VarLong(pRow->Value[0]);
		m_arSelectedIds.Add(nPayID);

		lpRow->Release();
	} 


	CDialog::OnOK();
}
