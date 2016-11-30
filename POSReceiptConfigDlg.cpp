// POSReceiptConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "POSReceiptConfigDlg.h"
#include "EditComboBox.h"
#include "FileUtils.h"
#include "PosReceiptConfigureHeaderFooterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.gruber 2007-05-08 12:32) - PLID 25931 - Class Created For


/////////////////////////////////////////////////////////////////////////////
// CPOSReceiptConfigDlg dialog


CPOSReceiptConfigDlg::CPOSReceiptConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPOSReceiptConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPOSReceiptConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPOSReceiptConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPOSReceiptConfigDlg)
	DDX_Control(pDX, IDC_POS_PRINTER_SHOW_LOGO, m_btnShowLogo);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_POS_PRINTER_LOGO_PATH, m_nxeditPosPrinterLogoPath);
	DDX_Control(pDX, IDC_POS_PRINTER_LINE_AFTER_HEADER, m_nxeditPosPrinterLineAfterHeader);
	DDX_Control(pDX, IDC_POS_PRINTER_LINE_AFTER_DETAILS, m_nxeditPosPrinterLineAfterDetails);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPOSReceiptConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPOSReceiptConfigDlg)
	ON_BN_CLICKED(IDC_POS_PRINTER_BROWSE, OnPosPrinterBrowse)
	ON_EN_KILLFOCUS(IDC_POS_PRINTER_LINE_AFTER_DETAILS, OnKillfocusPosPrinterLineAfterDetails)
	ON_EN_KILLFOCUS(IDC_POS_PRINTER_LINE_AFTER_HEADER, OnKillfocusPosPrinterLineAfterHeader)
	ON_EN_KILLFOCUS(IDC_POS_PRINTER_LOGO_PATH, OnKillfocusPosPrinterLogoPath)
	ON_BN_CLICKED(IDC_POS_PRINTER_RECEIPT_NAME_CONFIG, OnPosPrinterReceiptNameConfig)
	ON_BN_CLICKED(IDC_POS_PRINTER_SHOW_LOGO, OnPosPrinterShowLogo)
	ON_BN_CLICKED(IDC_POS_PRINTER_MAKE_DEFAULT, OnPosPrinterMakeDefault)
	ON_BN_CLICKED(IDC_FORMAT_LOCATION_HEADER, OnFormatLocationHeader)
	ON_BN_CLICKED(IDC_FORMAT_FOOTER, OnFormatFooter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPOSReceiptConfigDlg message handlers

BEGIN_EVENTSINK_MAP(CPOSReceiptConfigDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPOSReceiptConfigDlg)
	ON_EVENT(CPOSReceiptConfigDlg, IDC_POS_PRINTER_RECEIPTS_LIST, 1 /* SelChanging */, OnSelChangingPosPrinterReceiptsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPOSReceiptConfigDlg, IDC_POS_PRINTER_RECEIPTS_LIST, 16 /* SelChosen */, OnSelChosenPosPrinterReceiptsList, VTS_DISPATCH)
	ON_EVENT(CPOSReceiptConfigDlg, IDC_POS_PRINTER_RECEIPTS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPosPrinterReceiptsList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPOSReceiptConfigDlg::OnOK() 
{	
	CDialog::OnOK();
}


void CPOSReceiptConfigDlg::OnPosPrinterBrowse() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;

		if (pRow) {

			//for now, we are only supporting bmp files
			CString strSupportedTypes = "*.bmp";

			CFileDialog OpenLogo(TRUE, "", NULL, OFN_FILEMUSTEXIST, 
				"Supported Files (" + strSupportedTypes + ")|" + strSupportedTypes + "|All Files (*.*)|*.*||",
				this);
			// (j.armen 2011-10-25 15:56) - PLID 46139 - we are prompting the user, so use the shared path
			// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
			CString dir = GetEnvironmentDirectory();
			OpenLogo.m_ofn.lpstrInitialDir = (LPCSTR)dir;
			if (OpenLogo.DoModal() != IDOK) {
				//EndDialog(IDCANCEL);
				return;
			}

			m_strLogoPath = OpenLogo.GetPathName();
			
			if (!FileUtils::DoesFileOrDirExist(m_strLogoPath)) {
				m_strLogoPath = "";
			} else {
				GetDlgItem(IDC_POS_PRINTER_LOGO_PATH)->SetWindowText(m_strLogoPath);
			}
			long nReceiptID = VarLong(pRow->GetValue(0));
			ExecuteSql("UPDATE POSReceiptsT SET LogoPath = '%s' WHERE ID = %li", _Q(m_strLogoPath), nReceiptID); 

		}

	}NxCatchAll("Error in OnPosPrinterBrowse");
	
	
}



void CPOSReceiptConfigDlg::OnKillfocusPosPrinterLineAfterDetails() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;
		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));

			long nLinesAfterDetails;
			nLinesAfterDetails = GetDlgItemInt(IDC_POS_PRINTER_LINE_AFTER_DETAILS);
			if (nLinesAfterDetails != m_nLinesAfterDetails) {
				m_nLinesAfterDetails = nLinesAfterDetails;
				ExecuteSql("UPDATE POSReceiptsT SET LinesAfterDetails = %li WHERE ID = %li", nLinesAfterDetails, nReceiptID);
			}
		}
	}NxCatchAll("Error in OnKillfocusPosPrinterLineAfterDetails");
	
	
}

void CPOSReceiptConfigDlg::OnKillfocusPosPrinterLineAfterHeader() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;
		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));

			long nLinesAfterHeader;
			nLinesAfterHeader = GetDlgItemInt(IDC_POS_PRINTER_LINE_AFTER_HEADER);
			if (nLinesAfterHeader != m_nLinesAfterHeader) {
				m_nLinesAfterHeader = nLinesAfterHeader;
				ExecuteSql("UPDATE POSReceiptsT SET LinesAfterHeader = %li WHERE ID = %li", nLinesAfterHeader, nReceiptID);
			}
		}
	}NxCatchAll("Error in OnKillfocusPosPrinterLineAfterHeader");
	
}



void CPOSReceiptConfigDlg::OnKillfocusPosPrinterLogoPath() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;
		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));

			CString strLogoPath;
			GetDlgItemText(IDC_POS_PRINTER_LOGO_PATH, strLogoPath);
			if (strLogoPath.Compare(m_strLogoPath) != 0) {

				m_strLogoPath = strLogoPath;
				ExecuteSql("UPDATE POSReceiptsT SET LogoPath = '%s' WHERE ID = %li", _Q(strLogoPath), nReceiptID);
			}
		}
	}NxCatchAll("Error in OnKillfocusPosPrinterLogoPath");
	
}

void CPOSReceiptConfigDlg::OnPosPrinterReceiptNameConfig() 
{
	try {
		_variant_t varValue;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pReceiptList->CurSel;
		if (pRow) {
			varValue = pRow->GetValue(0);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 17, m_pReceiptList, "Edit Receipt Name").DoModal();

		if (pRow) {
			m_pReceiptList->SetSelByColumn(0, varValue);
		}

		GetDlgItem(IDC_POS_PRINTER_RECEIPTS_LIST)->SetFocus();



	}NxCatchAll("Error in OnPosPrinterReceiptNameConfig");
	
}

void CPOSReceiptConfigDlg::OnSelChangingPosPrinterReceiptsList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in OnSelChangingPosPrinterReceiptsList");
	
}

void CPOSReceiptConfigDlg::OnSelChosenPosPrinterReceiptsList(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			//load the values
			long nReceiptID = VarLong(pRow->GetValue(0));

			ADODB::_RecordsetPtr rs = CreateRecordset("SELECT ShowLogo, LogoPath, LocationFormat, LinesAfterHeader, LinesAfterDetails, FooterMessage "
				" FROM POSReceiptsT WHERE ID = %li", nReceiptID);
			ADODB::FieldsPtr flds = rs->Fields;

			if (! rs->eof) {

				m_nShowLogo = AdoFldBool(flds, "ShowLogo", FALSE) ? 1 :  0;
				m_strLogoPath = AdoFldString(flds, "LogoPath", "");
				m_strLocationFormat = AdoFldString(flds, "LocationFormat", "");
				m_nLinesAfterHeader = AdoFldLong(flds, "LinesAfterHeader",2);
				m_nLinesAfterDetails = AdoFldLong(flds, "LinesAfterDetails", 2);
				m_strFooterMessage = AdoFldString(flds, "FooterMessage", "");
			


				CheckDlgButton(IDC_POS_PRINTER_SHOW_LOGO, m_nShowLogo);
				SetDlgItemText(IDC_POS_PRINTER_LOGO_PATH, m_strLogoPath);
				SetDlgItemInt(IDC_POS_PRINTER_LINE_AFTER_HEADER, m_nLinesAfterHeader);
				SetDlgItemInt(IDC_POS_PRINTER_LINE_AFTER_DETAILS, m_nLinesAfterDetails);
				
				if (m_nShowLogo == 1) {
					GetDlgItem(IDC_POS_PRINTER_LOGO_PATH)->EnableWindow(TRUE);
					GetDlgItem(IDC_POS_PRINTER_BROWSE)->EnableWindow(TRUE);
				}
				else {
					GetDlgItem(IDC_POS_PRINTER_LOGO_PATH)->EnableWindow(FALSE);
					GetDlgItem(IDC_POS_PRINTER_BROWSE)->EnableWindow(FALSE);
				}

			

			}
		}
	}NxCatchAll("Error in OnSelChosenPosPrinterReceiptsList");
		
}

void CPOSReceiptConfigDlg::OnPosPrinterShowLogo() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;

		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));
		
			if (IsDlgButtonChecked(IDC_POS_PRINTER_SHOW_LOGO)) {

				CheckDlgButton(IDC_POS_PRINTER_SHOW_LOGO, TRUE);
				GetDlgItem(IDC_POS_PRINTER_LOGO_PATH)->EnableWindow(TRUE);
				GetDlgItem(IDC_POS_PRINTER_BROWSE)->EnableWindow(TRUE);

				ExecuteSql("UPDATE POSReceiptsT SET ShowLogo = 1 WHERE ID = %li", nReceiptID);
				m_nShowLogo = 1;
			}
			else {

				CheckDlgButton(IDC_POS_PRINTER_SHOW_LOGO, FALSE);
				GetDlgItem(IDC_POS_PRINTER_LOGO_PATH)->EnableWindow(FALSE);
				GetDlgItem(IDC_POS_PRINTER_BROWSE)->EnableWindow(FALSE);

				ExecuteSql("UPDATE POSReceiptsT SET ShowLogo = 0 WHERE ID = %li", nReceiptID);
				m_nShowLogo = 0;
			}

		}
	}NxCatchAll("Error in OnPosPrinterShowLogo");
	
}

BOOL CPOSReceiptConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_pReceiptList = BindNxDataList2Ctrl(this, IDC_POS_PRINTER_RECEIPTS_LIST, GetRemoteData(), TRUE);

		m_btnClose.AutoSet(NXB_CLOSE);

		((CNxEdit*)GetDlgItem(IDC_POS_PRINTER_LOGO_PATH))->LimitText(200);
				
	}NxCatchAll("Error in OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPOSReceiptConfigDlg::OnRequeryFinishedPosPrinterReceiptsList(short nFlags) 
{
	try {
		//see if there are any items
		long nCount = m_pReceiptList->GetRowCount();

		if (nCount > 0) {

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pReceiptList->GetFirstRow();
			if (pRow) {
				m_pReceiptList->CurSel = pRow;
				OnSelChosenPosPrinterReceiptsList(pRow);
			}

			//see if we have a default
			//per location
			long nDefaultID = GetRemotePropertyInt("POSReceiptDefaultSettings", -1, 0, GetCurrentLocationName(), FALSE);

			pRow = m_pReceiptList->FindByColumn(0, nDefaultID, NULL, FALSE);
			if (pRow) {
				pRow->PutForeColor(RGB(255,0,0));
			}
		}
	}NxCatchAll("Error in OnRequeryFinishedPosPrinterReceiptsList");
}

void CPOSReceiptConfigDlg::OnPosPrinterMakeDefault() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;

		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));

			long nFormerDefault = GetRemotePropertyInt("POSReceiptDefaultSettings", -1, 0, GetCurrentLocationName(), FALSE);

			SetRemotePropertyInt("POSReceiptDefaultSettings", nReceiptID, 0, GetCurrentLocationName());

			pRow->PutForeColor(RGB(255,0,0));

			//now set the previous default to not be red
			pRow = m_pReceiptList->FindByColumn(0, nFormerDefault, NULL, FALSE);
			if (pRow) {
				pRow->PutForeColor(NXDATALIST2Lib::dlColorNotSet);
			}
		}
	}NxCatchAll("Error in OnPOSPrinterMakeDefault");

	
}

// (j.gruber 2008-01-21 15:55) - PLID 28308 - added functionality for formatting footer
void CPOSReceiptConfigDlg::OnFormatLocationHeader() 
{	
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;

		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));

			CPosReceiptConfigureHeaderFooterDlg dlg(nReceiptID, true, this);

			dlg.DoModal();

		}

	}NxCatchAll("Error in CPOSReceiptConfigDlg::OnFormatLocationHeader()");
	
}

// (j.gruber 2008-01-21 15:56) - PLID 28308 - added functionality for formatting footer
void CPOSReceiptConfigDlg::OnFormatFooter() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pReceiptList->CurSel;

		if (pRow) {

			long nReceiptID = VarLong(pRow->GetValue(0));

			CPosReceiptConfigureHeaderFooterDlg dlg(nReceiptID, false, this);

			dlg.DoModal();

		}

	}NxCatchAll("Error in CPOSReceiptConfigDlg::OnFormatLocationFooter()");
	
}

