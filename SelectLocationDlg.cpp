// SelectLocationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorrc.h"
#include "SelectLocationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelectLocationDlg dialog


CSelectLocationDlg::CSelectLocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectLocationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectLocationDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectLocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectLocationDlg)
	DDX_Control(pDX, IDC_SELECT_LOCATION_CAPTION, m_nxstaticSelectLocationCaption);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectLocationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectLocationDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectLocationDlg message handlers

BOOL CSelectLocationDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	SetDlgItemText(IDC_SELECT_LOCATION_CAPTION, m_strCaption);

	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_pNewLocation = BindNxDataListCtrl(this, IDC_NEW_LOCATION,GetRemoteData(),false);
	CString strWhere;
	// (z.manning, 11/13/2006) - PLID 23440 - Only include general locations (not labs & pharmacies).
	strWhere.Format("Active = 1 AND TypeID = 1 AND ID <> %li", m_nCurrentLocationID);
	m_pNewLocation->WhereClause = _bstr_t(strWhere);
	m_pNewLocation->Requery();
	m_pNewLocation->CurSel = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectLocationDlg::OnOK() 
{
	if(m_pNewLocation->CurSel == -1) {
		MessageBox("You must select a new location.");
		return;
	}
	else {
		m_nNewLocationID = VarLong(m_pNewLocation->GetValue(m_pNewLocation->CurSel,0));
		CDialog::OnOK();
	}
	
}
