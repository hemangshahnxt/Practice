// DiagAddNew.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "DiagAddNew.h"
#include "Client.h"
#include "GlobalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CDiagAddNew dialog


CDiagAddNew::CDiagAddNew(CWnd* pParent)
	: CNxDialog(CDiagAddNew::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiagAddNew)
		m_ID = -1;
	//}}AFX_DATA_INIT
}


void CDiagAddNew::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiagAddNew)
	DDX_Control(pDX, IDC_CODE, m_nxeditCode);
	DDX_Control(pDX, IDC_DESC, m_nxeditDesc);
	DDX_Control(pDX, ID_OK_BTN, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiagAddNew, CNxDialog)
	//{{AFX_MSG_MAP(CDiagAddNew)
	ON_BN_CLICKED(ID_OK_BTN, OnOkBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiagAddNew message handlers

void CDiagAddNew::OnOK() 
{

}

void CDiagAddNew::OnOkBtn() 
{
	CString code,
			desc;

	GetDlgItemText (IDC_CODE, code);
	GetDlgItemText (IDC_DESC, desc);

	if (code.IsEmpty()) {
		MessageBox ("Please Enter a Code or Cancel");
		return;
	}

	//DRT 1/16/2007 - PLID 24177 - Do not allow | to exist.
	if(code.Find("|") > -1) {
		AfxMessageBox("The character '|' is not a valid character.  Please remove this before saving.");
		return;
	}

	if (desc.IsEmpty()) desc = " ";

	CString sql;
	_RecordsetPtr rs, rsCount;

	try {

		rs = CreateRecordset("SELECT * FROM DiagCodes");
		if (!rs->eof) {
			rs->Close();
			rsCount = CreateRecordset("SELECT Count(CodeNumber) AS DiagCount FROM DiagCodes WHERE CodeNumber = '%s'", _Q(code));

			if (rsCount->Fields->GetItem("DiagCount")->Value.lVal != 0) {
				AfxMessageBox(IDS_DIAG_DUPLICATE);
				return;
			}
			rsCount->Close();
		}
		else rs->Close();

		// (d.thompson 2014-02-14) - PLID 60716 - I moved creating new diag codes to the API for ICD-10.  Let's use it here.
		m_ID = CreateNewAdminDiagnosisCode(code, desc);

		//save the values so they can be entered by the calling function
		strCode = code;
		strDesc = desc;

		CDialog::OnOK();

	}NxCatchAll("Error in OnOkBtn()");
}

void CDiagAddNew::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CDiagAddNew::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29852 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	((CNxEdit*)GetDlgItem(IDC_CODE))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_DESC))->SetLimitText(255);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
