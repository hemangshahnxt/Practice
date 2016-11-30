// GenProductBarcodesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "GenProductBarcodesDlg.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenProductBarcodesDlg dialog


CGenProductBarcodesDlg::CGenProductBarcodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGenProductBarcodesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenProductBarcodesDlg)
	m_nStart = 1;
	//}}AFX_DATA_INIT
}


void CGenProductBarcodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenProductBarcodesDlg)
	DDX_Text(pDX, IDC_EDIT_BARGEN_START, m_nStart);
	DDX_Control(pDX, IDC_EDIT_BARGEN_START, m_nxeditEditBargenStart);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenProductBarcodesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CGenProductBarcodesDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenProductBarcodesDlg message handlers

void CGenProductBarcodesDlg::OnOK() 
{
	CString str;
	GetDlgItemText(IDC_EDIT_BARGEN_START, str);
	if (!str.GetLength())
	{
		MsgBox("Please enter a number in the starting field.");
		return;
	}

	UpdateData(TRUE);
	if (m_nStart < 1 || m_nStart > 100000000L)
	{
		MsgBox("Please enter a starting auto-generation number between or including 1 and 100,000,000");
		GetDlgItem(IDC_EDIT_BARGEN_START)->SetFocus();
		((CNxEdit*)GetDlgItem(IDC_EDIT_BARGEN_START))->SetSel(0,-1);
		return;
	}

	if (IDNO == MsgBox(MB_YESNO, "This will generate barcodes for all products that do not have any. All generated numbers will be higher than, or including the number %d. Do you wish to continue?",
		m_nStart))
		return;

	try {
		GenerateBarcodes();
		TryPrintBarcodes();
		CNxDialog::OnOK();
	}
	NxCatchAll("Error Generating Barcodes");
}

BOOL CGenProductBarcodesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_dlProducts = BindNxDataListCtrl(this,IDC_LIST_ACTIVEPRODUCTS,GetRemoteData(),true);

	try {
		_RecordsetPtr prs = CreateRecordset("SELECT SERVICET.ID FROM SERVICET INNER JOIN PRODUCTT ON PRODUCTT.ID = SERVICET.ID WHERE BARCODE IS NULL OR LEN(BARCODE) = 0");
		if (prs->eof)
		{
			MsgBox("All of the products in your system have barcodes assigned to them. This operation will have no effect on your data until you manually discard any individual item barcodes.");
			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}
	}
	NxCatchAll("Error initializing product barcode generation dialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CGenProductBarcodesDlg::GenerateBarcodes()
{
	CWaitCursor wc;
	CStringArray astrUsedBarcodes;
	long nOrdinal = m_nStart;

	// (c.haag 2004-02-10 10:18) - Populate the list of barcodes already used
	// in the system
	_RecordsetPtr rs = CreateRecordset("SELECT BARCODE FROM SERVICET WHERE Barcode IS NOT NULL AND LEN(Barcode) > 0");
	while (!rs->eof)
	{
		astrUsedBarcodes.Add(AdoFldString(rs, "Barcode"));
		rs->MoveNext();
	}
	rs->Close();
	rs.Release();

	// (c.haag 2004-02-10 10:35) - Now run a query of all PRODUCTS that need barcodes
	// and fill each one in with a unique barcode
	rs = CreateRecordset("SELECT SERVICET.ID AS ID FROM SERVICET INNER JOIN PRODUCTT ON PRODUCTT.ID = SERVICET.ID WHERE Barcode IS NULL OR LEN(Barcode) = 0");
	while (!rs->eof)
	{
		CString strOrdinal;
		long nServiceID = AdoFldLong(rs, "ID");
		BOOL bFoundOrdinal = FALSE;

		while (!bFoundOrdinal)
		{
			// Find the next available number to use
			strOrdinal.Format("%d", nOrdinal);
			for (long i=0; i < astrUsedBarcodes.GetSize(); i++)
			{
				if (astrUsedBarcodes[i] == strOrdinal)
					break;
			}
			if (i == astrUsedBarcodes.GetSize())
			{
				bFoundOrdinal = TRUE;				
			}
			else
			{
				nOrdinal++;
			}
		}
		ExecuteSql("UPDATE SERVICET SET BARCODE = '%s' WHERE ID = %d",
			strOrdinal, nServiceID);

		// (c.haag 2004-02-10 10:40) - Now update the products list
		int nRow = m_dlProducts->FindByColumn(0, nServiceID, 0, FALSE);
		if (nRow > -1)
			m_dlProducts->PutValue(nRow, 2, (LPCTSTR)strOrdinal);

		nOrdinal++;
		rs->MoveNext();
	}
}

void CGenProductBarcodesDlg::TryPrintBarcodes()
{
	//(e.lally 2010-10-20) PLID 26807 - Changed spelling of Bar Code to Barcode
	if (IDNO == MsgBox(MB_YESNO, "The barcodes have successfully been generated. Would you like to run the Inventory Barcode report now?"))
		return;

	CWaitCursor wc;
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(331)]);
	CPtrArray	paParam;

	// DON'T FORGET TO HANDLE CUSTOM REPORTS!!!
	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, &paParam, true, (CWnd *)this, "Inventory BarCodes");
	
}
