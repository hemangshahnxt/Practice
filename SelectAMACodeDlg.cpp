// SelectAMACodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectAMACodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectAMACodeDlg dialog

// (d.singleton 2011-10-04 16:41) - PLID 45904 add flag for ama vs alberta billing import
CSelectAMACodeDlg::CSelectAMACodeDlg(CWnd* pParent, BOOL bIsHLINK)
	: CNxDialog(CSelectAMACodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelectAMACodeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	//HLINK flag to see if this is an AMA import or an alberta billing import
	m_bIsHLINK = bIsHLINK;
	m_bAlwaysChoose = TRUE;
}

void CSelectAMACodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectAMACodeDlg)
	DDX_Control(pDX, IDC_ALWAYS_CHOOSE, m_btnAlwaysChoose);
	DDX_Control(pDX, IDC_CODE_DESCRIPTION, m_nxstaticCodeDescription);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSelectAMACodeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectAMACodeDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectAMACodeDlg message handlers

BOOL CSelectAMACodeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();


	m_pList = BindNxDataListCtrl(this, IDC_SERVICE_CODE_LIST, GetRemoteData(), false);
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	CString strWhere;
	if(!m_strCode.IsEmpty()) {
		strWhere.Format("CPTCodeT.Code = '%s'", _Q(m_strCode));
	}

	SetDlgItemText(IDC_CODE_DESCRIPTION, m_strDesc);
	CheckDlgButton(IDC_ALWAYS_CHOOSE, m_bAlwaysChoose);

	// (d.singleton 2011-10-04 16:39) - PLID 45904 if HLINK flag is true this dialog is during the alberta billing code import so take out all the ama specific stuff
	if(m_bIsHLINK)
	{
		//hide the IsAMA column
		IColumnSettingsPtr pCol = m_pList->GetColumn(4);
		pCol->PutStoredWidth(0);

		//take out "AMA" in text box
		SetDlgItemText(IDC_STATIC, "The code you are attempting to update exists multiple times in your data.  Please select which code should be updated.");	
	}

	m_pList->PutWhereClause(_bstr_t(strWhere));
	m_pList->Requery();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectAMACodeDlg::OnOK() 
{
	m_bAlwaysChoose = IsDlgButtonChecked(IDC_ALWAYS_CHOOSE);

	long nSel = m_pList->GetCurSel();
	if(nSel == sriNoRow) {

		// if "Always choose the first code" was checked, auto-select the first row
		if (m_pList->GetRowCount() > 0 && m_bAlwaysChoose) {
			m_pList->PutCurSel(0);
			nSel = 0;
		}
		else {
			MsgBox("You must choose a code to continue.  Please choose \"Skip This Code\" if you do not wish to overwrite.");
			return;
		}
	}

	m_nChosen = VarLong(m_pList->GetValue(nSel, 0));

	CDialog::OnOK();
}

void CSelectAMACodeDlg::OnCancel() 
{
	CDialog::OnCancel();
}
