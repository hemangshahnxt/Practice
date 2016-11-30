// EmrHeaderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "EmrHeaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEmrHeaderDlg dialog


CEmrHeaderDlg::CEmrHeaderDlg(CWnd* pParent)
	: CNxDialog(CEmrHeaderDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrHeaderDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEmrHeaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrHeaderDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_HEADER_TITLE, m_nxeditHeaderTitle);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrHeaderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrHeaderDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrHeaderDlg message handlers

BOOL CEmrHeaderDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 17:09) - PLID 29837 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) {
			SetWindowText("EMR Header Configuration");
		}
		else {
			SetWindowText("Header Configuration");
		}
		//Bind my datalists.
		m_pField1 = BindNxDataListCtrl(IDC_EMR_HEADER_FIELD1, false);
		m_pField2 = BindNxDataListCtrl(IDC_EMR_HEADER_FIELD2, false);
		m_pField3 = BindNxDataListCtrl(IDC_EMR_HEADER_FIELD3, false);
		m_pField4 = BindNxDataListCtrl(IDC_EMR_HEADER_FIELD4, false);

		//Fill in my four datalists (they all have exactly the same options:
		IRowSettingsPtr pRow;
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("<None>"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, _bstr_t("Patient Name (First Middle Last)"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, _bstr_t("Patient Name (Last, First Middle)"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)3);
		pRow->PutValue(1, _bstr_t("Date of EMR"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)4);
		pRow->PutValue(1, _bstr_t("Patient Birth Date"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)5);
		pRow->PutValue(1, _bstr_t("Patient Age"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);
		pRow = m_pField1->GetRow(-1);
		pRow->PutValue(0, (long)6);
		pRow->PutValue(1, _bstr_t("Provider"));
		m_pField1->AddRow(pRow);
		m_pField2->AddRow(pRow);
		m_pField3->AddRow(pRow);
		m_pField4->AddRow(pRow);

		((CNxEdit*)GetDlgItem(IDC_HEADER_TITLE))->LimitText(255);

		//Now, load the actual settings.
		SetDlgItemText(IDC_HEADER_TITLE, GetRemotePropertyText("EmrHeaderTitle", "<Procedure> Report", 0, "<None>", true));
		long nField1 = (long)GetRemotePropertyInt("EmrHeaderField1", -1, 0, "<None>", true);
		if(nField1 != -1) {
			m_pField1->SetSelByColumn(0, nField1);
		}
		long nField2 = (long)GetRemotePropertyInt("EmrHeaderField2", -1, 0, "<None>", true);
		if(nField2 != -1) {
			m_pField2->SetSelByColumn(0, nField2);
		}
		long nField3 = (long)GetRemotePropertyInt("EmrHeaderField3", -1, 0, "<None>", true);
		if(nField3 != -1) {
			m_pField3->SetSelByColumn(0, nField3);
		}
		long nField4 = (long)GetRemotePropertyInt("EmrHeaderField4", -1, 0, "<None>", true);
		if(nField4 != -1) {
			m_pField4->SetSelByColumn(0, nField4);
		}
	} NxCatchAll("Error in CEmrHeaderDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrHeaderDlg::OnOK() 
{
	CString strTitle;
	GetDlgItemText(IDC_HEADER_TITLE, strTitle);
	SetRemotePropertyText("EmrHeaderTitle", strTitle, 0, "<None>");
	if(m_pField1->CurSel == -1) SetRemotePropertyInt("EmrHeaderField1", -1, 0, "<None>");
	else SetRemotePropertyInt("EmrHeaderField1", VarLong(m_pField1->GetValue(m_pField1->CurSel, 0)), 0, "<None>");
	if(m_pField2->CurSel == -1) SetRemotePropertyInt("EmrHeaderField2", -1, 0, "<None>");
	else SetRemotePropertyInt("EmrHeaderField2", VarLong(m_pField2->GetValue(m_pField2->CurSel, 0)), 0, "<None>");
	if(m_pField3->CurSel == -1) SetRemotePropertyInt("EmrHeaderField3", -1, 0, "<None>");
	else SetRemotePropertyInt("EmrHeaderField3", VarLong(m_pField3->GetValue(m_pField3->CurSel, 0)), 0, "<None>");
	if(m_pField4->CurSel == -1) SetRemotePropertyInt("EmrHeaderField4", -1, 0, "<None>");
	else SetRemotePropertyInt("EmrHeaderField4", VarLong(m_pField4->GetValue(m_pField4->CurSel, 0)), 0, "<None>");

	CDialog::OnOK();
}

void CEmrHeaderDlg::OnCancel() 
{	
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CEmrHeaderDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrHeaderDlg)
	ON_EVENT(CEmrHeaderDlg, IDC_EMR_HEADER_FIELD1, 16 /* SelChosen */, OnSelChosenEmrHeaderField1, VTS_I4)
	ON_EVENT(CEmrHeaderDlg, IDC_EMR_HEADER_FIELD2, 16 /* SelChosen */, OnSelChosenEmrHeaderField2, VTS_I4)
	ON_EVENT(CEmrHeaderDlg, IDC_EMR_HEADER_FIELD3, 16 /* SelChosen */, OnSelChosenEmrHeaderField3, VTS_I4)
	ON_EVENT(CEmrHeaderDlg, IDC_EMR_HEADER_FIELD4, 16 /* SelChosen */, OnSelChosenEmrHeaderField4, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrHeaderDlg::OnSelChosenEmrHeaderField1(long nRow) 
{
	if(nRow != -1) {
		if(VarLong(m_pField1->GetValue(nRow, 0)) == -1) {
			m_pField1->CurSel = -1;
		}
	}
}

void CEmrHeaderDlg::OnSelChosenEmrHeaderField2(long nRow) 
{
	if(nRow != -1) {
		if(VarLong(m_pField2->GetValue(nRow, 0)) == -1) {
			m_pField2->CurSel = -1;
		}
	}
}

void CEmrHeaderDlg::OnSelChosenEmrHeaderField3(long nRow) 
{
	if(nRow != -1) {
		if(VarLong(m_pField3->GetValue(nRow, 0)) == -1) {
			m_pField3->CurSel = -1;
		}
	}
}

void CEmrHeaderDlg::OnSelChosenEmrHeaderField4(long nRow) 
{
	if(nRow != -1) {
		if(VarLong(m_pField4->GetValue(nRow, 0)) == -1) {
			m_pField4->CurSel = -1;
		}
	}
}
