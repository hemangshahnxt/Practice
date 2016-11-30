// MarketRetentionPrintSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "marketingRc.h"
#include "MarketRetentionPrintSetupDlg.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketRetentionPrintSetupDlg dialog


CMarketRetentionPrintSetupDlg::CMarketRetentionPrintSetupDlg(CWnd* pParent /*=NULL*/, BOOL bPreview /*= TRUE*/)
	: CNxDialog(CMarketRetentionPrintSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMarketRetentionPrintSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bPreview = bPreview;
}


void CMarketRetentionPrintSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMarketRetentionPrintSetupDlg)
	DDX_Control(pDX, IDC_PREVIEW_BTN, m_btnPreview);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRINT_STYLE_GROUPBOX, m_btnPrintStyleGroupbox);
	DDX_Control(pDX, IDC_REPORT_ON, m_btnReportOn);
	DDX_Control(pDX, IDC_SUMMARY_RADIO, m_radioSummary);
	DDX_Control(pDX, IDC_DETAILED_RADIO, m_radioDetailed);
	DDX_Control(pDX, IDC_RETAINED_RADIO, m_radioRetained);
	DDX_Control(pDX, IDC_UNRETAINED_RADIO, m_radioUnretained);
	DDX_Control(pDX, IDC_EXCLUDE_APPTS, m_radioApptExclude);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMarketRetentionPrintSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMarketRetentionPrintSetupDlg)
	ON_BN_CLICKED(IDC_PREVIEW_BTN, OnPreviewBtn)
	ON_BN_CLICKED(IDC_DETAILED_RADIO, OnDetailedRadio)
	ON_BN_CLICKED(IDC_SUMMARY_RADIO, OnSummaryRadio)
	ON_BN_CLICKED(IDC_EXCLUDE_APPTS, OnExcludeAppts)
	ON_BN_CLICKED(IDC_UNRETAINED_RADIO, OnUnretainedRadio)
	ON_BN_CLICKED(IDC_RETAINED_RADIO, OnRetainedRadio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMarketRetentionPrintSetupDlg message handlers

BOOL CMarketRetentionPrintSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 12:58) - PLID 29824 - NxIconify more buttons
		m_btnPreview.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//bind the datalist
		m_pPurposeList = BindNxDataListCtrl(this, IDC_RETENTION_PURPOSE_LIST, GetRemoteData(), true);
		
		if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>")) {
			CheckDlgButton(IDC_DETAILED_RADIO, 1);
			GetDlgItem(IDC_RETAINED_RADIO)->EnableWindow(TRUE);
			GetDlgItem(IDC_UNRETAINED_RADIO)->EnableWindow(TRUE);
		}
		else {
			CheckDlgButton(IDC_SUMMARY_RADIO, 1);
			GetDlgItem(IDC_RETAINED_RADIO)->EnableWindow(FALSE);
			GetDlgItem(IDC_UNRETAINED_RADIO)->EnableWindow(FALSE);
		}

		if (GetRemotePropertyInt("MarketRetentionOption", 1, 0, "<None>")) {
			CheckDlgButton(IDC_RETAINED_RADIO, 1);
			GetDlgItem(IDC_EXCLUDE_APPTS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(FALSE);
		}
		else {
			CheckDlgButton(IDC_UNRETAINED_RADIO, 1);
			GetDlgItem(IDC_EXCLUDE_APPTS)->EnableWindow(TRUE);
			GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(FALSE);
		}

		CString strTemp;
		if (m_bPreview) {
			strTemp = "Preview";
		} else {
			strTemp = "Print";
		}
		SetDlgItemText(IDC_PREVIEW_BTN, "&" + strTemp);
	}
	NxCatchAll("Error in CMarketRetentionPrintSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMarketRetentionPrintSetupDlg::OnPreviewBtn() 
{
	if (IsDlgButtonChecked(IDC_DETAILED_RADIO)) {
		SetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>");
		
	}
	else {
		SetRemotePropertyInt("MarketPrintStyleOption", 0, 0, "<None>");
		
	}

	if (IsDlgButtonChecked(IDC_RETAINED_RADIO)) {
		SetRemotePropertyInt("MarketRetentionOption", 1, 0, "<None>");
	}
	else {
		SetRemotePropertyInt("MarketRetentionOption", 0, 0, "<None>");
	}

	if (IsDlgButtonChecked(IDC_EXCLUDE_APPTS)) {
		m_bExcludeAppts = TRUE;
	}
	else {
		m_bExcludeAppts = FALSE;
	}

	OnOK();
	
}

void CMarketRetentionPrintSetupDlg::OnDetailedRadio() 
{
	if (IsDlgButtonChecked(IDC_DETAILED_RADIO)) {
		GetDlgItem(IDC_RETAINED_RADIO)->EnableWindow(TRUE);
		GetDlgItem(IDC_UNRETAINED_RADIO)->EnableWindow(TRUE);
		GetDlgItem(IDC_EXCLUDE_APPTS)->EnableWindow(TRUE);
		GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(TRUE);
	}
	
}

void CMarketRetentionPrintSetupDlg::OnSummaryRadio() 
{
	if (IsDlgButtonChecked(IDC_SUMMARY_RADIO)) {
		GetDlgItem(IDC_RETAINED_RADIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_UNRETAINED_RADIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_EXCLUDE_APPTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(FALSE);
	}
	
}

BEGIN_EVENTSINK_MAP(CMarketRetentionPrintSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMarketRetentionPrintSetupDlg)
	ON_EVENT(CMarketRetentionPrintSetupDlg, IDC_RETENTION_PURPOSE_LIST, 16 /* SelChosen */, OnSelChosenRetentionPurposeList, VTS_I4)
	ON_EVENT(CMarketRetentionPrintSetupDlg, IDC_RETENTION_PURPOSE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRetentionPurposeList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMarketRetentionPrintSetupDlg::OnSelChosenRetentionPurposeList(long nRow) 
{
	if(m_pPurposeList->CurSel == -1) {
		m_strMultiPurposeIds = "-1";
		return;
	}
	
	//check to see if it is -2
	if (VarLong(m_pPurposeList->GetValue(nRow, 0)) == -2) {
	
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptPurposeT");

		//preselect existing stuff
		CString str = m_strMultiPurposeIds;
		long nComma = str.Find(",");
		while(nComma > 0) {
			dlg.PreSelect(atoi(str.Left(nComma)));
			str = str.Right(str.GetLength() - (nComma + 1));

			nComma = str.Find(",");
		}

		if(dlg.Open("AptPurposeT", "", "ID", "Name", "Select purposes") == IDCANCEL) {
			m_pPurposeList->SetSelByColumn(0, (long)0);
			return;
		}
		else {
			CString strOut = dlg.GetMultiSelectIDString();
			if(strOut.IsEmpty()) {
				MsgBox("You cannot filter on no purposes.  The filter will be reset to { All Purposes }");
				m_pPurposeList->SetSelByColumn(0, (long)-1);
				long nNewId = -1;	//just change it here, and let us continue on
			}

			m_strMultiPurposeIds = strOut;
			m_strPurposeNames = dlg.GetMultiSelectString();
			m_strMultiPurposeIds.Replace(" ", ",");
			
		}
	}
	else if (VarLong(m_pPurposeList->GetValue(nRow, 0)) == -1) {
		m_strMultiPurposeIds = "-1";
		m_strPurposeNames = "";
	}
	else {
		m_strMultiPurposeIds = AsString(m_pPurposeList->GetValue(nRow, 0));
		m_strPurposeNames = AsString(m_pPurposeList->GetValue(nRow, 1));
	}	

	
}

void CMarketRetentionPrintSetupDlg::OnExcludeAppts() 
{
	if (IsDlgButtonChecked(IDC_EXCLUDE_APPTS)) {
		GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(TRUE);
		OnSelChosenRetentionPurposeList(0);
		m_bExcludeAppts = TRUE;
	}
	else {
		//gray it out
		GetDlgItem(IDC_RETENTION_PURPOSE_LIST)->EnableWindow(FALSE);
		m_bExcludeAppts = FALSE;
	}

	
}

void CMarketRetentionPrintSetupDlg::OnRequeryFinishedRetentionPurposeList(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow;
	pRow = m_pPurposeList->GetRow(-1);
	pRow->PutValue(0, (long) -2);
	pRow->PutValue(1, _variant_t("{Multiple Purposes}"));
	m_pPurposeList->InsertRow(pRow, 0);
	
	pRow = m_pPurposeList->GetRow(-1);
	pRow->PutValue(0, (long) -1);
	pRow->PutValue(1, _variant_t("{All Purposes}"));
	m_pPurposeList->InsertRow(pRow, 0);
	
}

void CMarketRetentionPrintSetupDlg::OnUnretainedRadio() 
{
	GetDlgItem(IDC_EXCLUDE_APPTS)->EnableWindow(TRUE);
}

void CMarketRetentionPrintSetupDlg::OnRetainedRadio() 
{
	GetDlgItem(IDC_EXCLUDE_APPTS)->EnableWindow(TRUE);
	
}
