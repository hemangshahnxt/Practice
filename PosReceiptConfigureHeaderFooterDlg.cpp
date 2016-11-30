// PosReceiptConfigureHeaderFooterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "PosReceiptConfigureHeaderFooterDlg.h"
#include "GlobalReportUtils.h"

// (j.gruber 2008-01-21 15:41) - PLID 28308 - created for

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_ADD_LOCATION_NAME  50023
#define IDM_ADD_LOCATION_ADDRESS_1  50024
#define IDM_ADD_LOCATION_ADDRESS_2  50025
#define IDM_ADD_LOCATION_CITY  50026
#define IDM_ADD_LOCATION_STATE  50027
#define IDM_ADD_LOCATION_PHONE  50028
#define IDM_ADD_LOCATION_ZIP  50029

#define IDM_ADD_PATIENT_FIRST_NAME  50050
#define IDM_ADD_PATIENT_MIDDLE_NAME  50051
#define IDM_ADD_PATIENT_LAST_NAME  50052
#define IDM_ADD_PATIENT_ID  50053
// (j.gruber 2008-06-11 16:17) - PLID 28631 - added next appt date
#define IDM_ADD_PATIENT_NEXT_APPT_DATE  50054

#define IDM_ADD_LEFT_JUSTIFY 51020
#define IDM_ADD_CENTER_LINE 51021
#define IDM_ADD_RIGHT_JUSTIFY 51022
#define IDM_ADD_BOLD 51023
#define IDM_ADD_ITALIC 51024
#define IDM_ADD_UNDERLINE 51025
#define IDM_ADD_DOUBLE_WIDE 51026
#define IDM_ADD_DOUBLE_HIGH 51027
#define IDM_ADD_DOUBLE_WIDE_HIGH 51028

//this must be last because its added to (but if you had to do it, there probably isn't more then 10 fonts a printer supports, ours supports 3)
#define IDM_ADD_FONT 52023

/////////////////////////////////////////////////////////////////////////////
// CPosReceiptConfigureHeaderFooterDlg dialog


enum FontListColumns {
	flcNumber = 0,
	flcName,
	flcChars,
};


CPosReceiptConfigureHeaderFooterDlg::CPosReceiptConfigureHeaderFooterDlg(long nReceiptID, BOOL bIsHeader, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPosReceiptConfigureHeaderFooterDlg::IDD, pParent)
{

	m_bIsHeader = bIsHeader;
	m_nReceiptID = nReceiptID;
	//{{AFX_DATA_INIT(CPosReceiptConfigureHeaderFooterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPosReceiptConfigureHeaderFooterDlg::~CPosReceiptConfigureHeaderFooterDlg() {

	try {
		//delete the fonts
		long nSize = m_paryFonts.GetSize();
		if (nSize > 0) {
			for (int i = nSize - 1; i >= 0; i--) {
				FontType *pFont = (FontType*)m_paryFonts.GetAt(i);
				if (pFont) {
					delete pFont;
				}
			}
		}
	}catch (...) {
	}
	

}


void CPosReceiptConfigureHeaderFooterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPosReceiptConfigureHeaderFooterDlg)
	DDX_Control(pDX, IDC_TEXT, m_edtText);
	DDX_Control(pDX, IDC_DESC_LABEL, m_nxstaticDescLabel);
	DDX_Control(pDX, IDC_INSERT_FIELD, m_btnInsertField);
	DDX_Control(pDX, IDC_PRINT_TEST, m_btnPrintTest);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPosReceiptConfigureHeaderFooterDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPosReceiptConfigureHeaderFooterDlg)
	ON_BN_CLICKED(IDC_INSERT_FIELD, OnInsertField)
	ON_BN_CLICKED(IDC_INSERT_FORMAT, OnInsertFormat)
	ON_BN_CLICKED(IDC_PRINT_TEST, OnPrintTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPosReceiptConfigureHeaderFooterDlg message handlers

void CPosReceiptConfigureHeaderFooterDlg::OnInsertField() 
{
	try {

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_NAME, "<Location Name>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_ADDRESS_1, "<Location Address 1>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_ADDRESS_2, "<Location Address 2>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_CITY, "<Location City>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_STATE, "<Location State>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_ZIP, "<Location ZipCode>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LOCATION_PHONE, "<Location Main Phone>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_PATIENT_FIRST_NAME, "<Patient First Name>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_PATIENT_MIDDLE_NAME, "<Patient Middle Name>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_PATIENT_LAST_NAME, "<Patient Last Name>");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_PATIENT_ID, "<Patient ID>");
		// (j.gruber 2008-06-11 16:17) - PLID 28631 - added next appt date
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_PATIENT_NEXT_APPT_DATE, "<Patient Next Appt. Date>");


		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_INSERT_FIELD);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}	

	}NxCatchAll("Error in CPOSReceiptConfigureHeaderFooterDlg::OnInsertField() ");
	
}

void CPosReceiptConfigureHeaderFooterDlg::OnInsertFormat()
{
	try {

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		
		//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();
		POSPrinterAccess pPOSPrinter;

		if (pPOSPrinter) {

			int nFlags = MF_BYPOSITION|MF_DISABLED|MF_GRAYED;

			//all printers should support these:
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_LEFT_JUSTIFY, "<LEFT JUSTIFY>");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_CENTER_LINE, "<CENTER>");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_RIGHT_JUSTIFY, "<RIGHT JUSTIFY>");

			if (pPOSPrinter->IsBoldSupported()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_BOLD, "<BOLD>");
			}
			else {
				mnu.InsertMenu(nIndex++, nFlags, IDM_ADD_BOLD, "<BOLD>");
			}

			if (pPOSPrinter->IsItalicSupported()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_ITALIC, "<ITALIC>");
			}
			else {
				mnu.InsertMenu(nIndex++, nFlags, IDM_ADD_ITALIC, "<ITALIC>");
			}

			if (pPOSPrinter->IsUnderlineSupported()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_UNDERLINE, "<UNDL>");
			}
			else {

				mnu.InsertMenu(nIndex++, nFlags, IDM_ADD_UNDERLINE, "<UNDL>");
			}

			//double wide
			if (pPOSPrinter->IsDoubleWideSupported()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_DOUBLE_WIDE, "<Double Wide>");
			}
			else {
				mnu.InsertMenu(nIndex++, nFlags, IDM_ADD_DOUBLE_WIDE, "<Double Wide>");
			}

			if (pPOSPrinter->IsDoubleHighSupported()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_DOUBLE_HIGH, "<Double High>");
			}
			else {
				mnu.InsertMenu(nIndex++, nFlags, IDM_ADD_DOUBLE_HIGH, "<Double High>");
			}

			if (pPOSPrinter->IsDoubleWideAndHighSupported()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDM_ADD_DOUBLE_WIDE_HIGH, "<Double Wide and High>");
			}
			else {
				mnu.InsertMenu(nIndex++, nFlags, IDM_ADD_DOUBLE_WIDE_HIGH, "<Double Wide and High>");
			}


			CRect rc;
			CWnd *pWnd = GetDlgItem(IDC_INSERT_FORMAT);
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
			} else {
				CPoint pt;
				GetCursorPos(&pt);
				mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			}	

			//GetMainFrame()->ReleasePOSPrinter();
		}

		

	}NxCatchAll("Error in CPOSReceiptConfigureHeaderFooterDlg::OnInsertFormat() ");
	
	
}

void CPosReceiptConfigureHeaderFooterDlg::OnPrintTest() 
{
	try {

		//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();

		POSPrinterAccess pPOSPrinter;

		if (pPOSPrinter) {

			ADODB::_RecordsetPtr rs = CreateRecordset("SELECT * FROM POSReceiptsT WHERE ID = %li", m_nReceiptID);
			if (! rs->eof) {

				long nShowLogo;
				CString strLogoPath, strFormat;
				nShowLogo = AdoFldBool(rs, "ShowLogo", FALSE) ? 1 :  0;
				strLogoPath = AdoFldString(rs, "LogoPath", "");
				GetDlgItemText(IDC_TEXT, strFormat);

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFontList->CurSel;

				if (pRow) {

					long nFontNumber = VarLong(pRow->GetValue(flcNumber));
					long nCharCount = VarLong(pRow->GetValue(flcChars));

					if (m_bIsHeader) {

						PrintReceiptHeader(pPOSPrinter, "-1", nShowLogo, strLogoPath, strFormat, nFontNumber, nCharCount);
					}
					else {
						PrintReceiptFooter(pPOSPrinter, "-1", strFormat, nFontNumber, nCharCount);
					}

					//since the normal header/footer prints things after this, we need to feed it
					for (int i = 0; i < 4; i++) {
						pPOSPrinter->PrintText("\r\n");
					}

					//now cut
					// (a.walling 2011-04-28 10:02) - PLID 43492
					pPOSPrinter->FlushAndTryCut();
				}				
			}

			//GetMainFrame()->ReleasePOSPrinter();
			
		}
	}NxCatchAll("Error in CPOSReceiptConfigureHeaderFooterDlg::OnPrintTest() ");
	
}

void CPosReceiptConfigureHeaderFooterDlg::InsertField(CString strFieldToInsert)  {

	try {

		int nStartSel, nEndSel;
		m_edtText.GetSel(nStartSel, nEndSel);

		if (nStartSel != -1 && (nStartSel == nEndSel)) {

			//we just need to insert the field at the position
			CString strText;
			m_edtText.GetWindowText(strText);

			CString strNewText;
			strNewText = strText.Left(nStartSel);
			strNewText += strFieldToInsert;
			strNewText += strText.Right(strText.GetLength() - nStartSel);

			m_edtText.SetWindowText(strNewText);
		}


	}NxCatchAll("Error in CPOSReceiptConfigureHeaderFooterDlg::InsertField");


}


void CPosReceiptConfigureHeaderFooterDlg::InsertFormat(CString strFormatToInsert)  {

	try {

		int nStartSel, nEndSel;
		m_edtText.GetSel(nStartSel, nEndSel);

		if (nStartSel != -1 && (nStartSel != nEndSel)) {

			//put the start delimitor at the beginning and the end at the end
			CString strText;
			m_edtText.GetWindowText(strText);

			CString strNewText;
			strNewText = strText.Left(nStartSel);
			strNewText += "<START_" + strFormatToInsert + ">";
			CString strMiddle;
			strMiddle = strText.Mid(nStartSel, (nEndSel - nStartSel));
			strNewText += strMiddle;

			//now put the end delimintator to get it back to regular text
			strNewText += "<END_" + strFormatToInsert + ">";

			//now the end
			strNewText += strText.Right(strText.GetLength() - nEndSel);

			m_edtText.SetWindowText(strNewText);
		}


	}NxCatchAll("Error in CPOSReceiptConfigureHeaderFooterDlg::InsertField");


}

BOOL CPosReceiptConfigureHeaderFooterDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try  {

		switch (wParam) {

			case IDM_ADD_LOCATION_NAME:
				InsertField("<Name>");
			break;

			case IDM_ADD_LOCATION_ADDRESS_1:
				InsertField("<Address1>");
			break;
			
			case IDM_ADD_LOCATION_ADDRESS_2:
				InsertField("<Address2>");
			break;

			case IDM_ADD_LOCATION_CITY:
				InsertField("<City>");
			break;
			case IDM_ADD_LOCATION_STATE:
				InsertField("<State>");
			break;
			
			case IDM_ADD_LOCATION_PHONE:
				InsertField("<MainPhone>");
			break;
			case IDM_ADD_LOCATION_ZIP :
				InsertField("<Zip>");
			break;
			case IDM_ADD_PATIENT_FIRST_NAME:
				InsertField("<PatFirst>");
			break;
			case IDM_ADD_PATIENT_MIDDLE_NAME:
				InsertField("<PatMiddle>");
			break;
			case IDM_ADD_PATIENT_LAST_NAME:
				InsertField("<PatLast>");
			break;
			case IDM_ADD_PATIENT_ID:
				InsertField("<PatID>");
			break;
			// (j.gruber 2008-06-11 16:17) - PLID 28631 - added next appt date
			case IDM_ADD_PATIENT_NEXT_APPT_DATE:
				InsertField("<PatNextApptDate>");
			break;
			case IDM_ADD_BOLD:
				InsertFormat("BOLD");
			break;
			case IDM_ADD_ITALIC:
				InsertFormat("ITL");
			break;
			case IDM_ADD_UNDERLINE:
				InsertFormat("UNDL");
			break;
			case IDM_ADD_DOUBLE_WIDE:
				InsertFormat("DBLWD");
			break;
			case IDM_ADD_DOUBLE_HIGH:
				InsertFormat("DBLHGH");
			break;
			case IDM_ADD_DOUBLE_WIDE_HIGH:
				InsertFormat("DBLWDHGH");
			break;

			case IDM_ADD_LEFT_JUSTIFY:
				InsertFormat("LFTJST");
			break;

			case IDM_ADD_CENTER_LINE:
				InsertFormat("CNTRJST");
			break;

			case IDM_ADD_RIGHT_JUSTIFY:
				InsertFormat("RHTJST");
			break;
			
			default:
				
			break;
		}



	}NxCatchAll("Error in CPOSReceiptConfigureHeaderFooterDlg::OnCommand");
	
	return CDialog::OnCommand(wParam, lParam);
}

void CPosReceiptConfigureHeaderFooterDlg::OnOK() 
{
	try {

		CString strFormat;
		GetDlgItemText(IDC_TEXT, strFormat);

		//get the font and character count
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFontList->CurSel;
		if (pRow) {

			long nFontNumber = VarLong(pRow->GetValue(flcNumber));
			long nCharCount = VarLong(pRow->GetValue(flcChars));

			if (m_bIsHeader) {
				ExecuteParamSql("UPDATE POSReceiptsT SET LocationFormat = {STRING}, FontNumber = {INT}, CharacterCount = {INT} WHERE ID = {INT}", strFormat, nFontNumber, nCharCount, m_nReceiptID);
			}
			else {
				ExecuteParamSql("UPDATE POSReceiptsT SET FooterMessage = {STRING}, FontNumber = {INT}, CharacterCount = {INT} WHERE ID = {INT}", strFormat, nFontNumber, nCharCount, m_nReceiptID);
			}
		}

	
		CDialog::OnOK();
	
	}NxCatchAll("Error in OnOK");
	
	
}


BOOL CPosReceiptConfigureHeaderFooterDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try { 
		
		m_btnInsertField.AutoSet(NXB_NEW);
		m_btnPrintTest.AutoSet(NXB_PRINT);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		ADODB::_RecordsetPtr rs;
		if (m_bIsHeader) {
			rs = CreateParamRecordset("SELECT FontNumber, LocationFormat as Format FROM POSReceiptsT WHERE ID = {INT}", m_nReceiptID);
			SetDlgItemText(IDC_DESC_LABEL, "Header Text: ");
			SetWindowText("Configure Header");
			
		}
		else {
			rs = CreateParamRecordset("SELECT FontNumber, FooterMessage as Format FROM POSReceiptsT WHERE ID = {INT}", m_nReceiptID);
			SetDlgItemText(IDC_DESC_LABEL, "Footer Text: ");
			SetWindowText("Configure Footer");
		}

		long nFontNumber = -1;
	
		if (! rs->eof) {
			m_edtText.SetWindowText(AdoFldString(rs, "Format", ""));
			nFontNumber = AdoFldLong(rs, "FontNumber", -1);
		}

		m_pFontList = BindNxDataList2Ctrl(this, IDC_FONT_LIST, NULL, false);

		//COPOSPrinterDevice *pPOSPrinter = GetMainFrame()->ClaimPOSPrinter();
		
		POSPrinterAccess pPOSPrinter;
		
		if (pPOSPrinter) {
			pPOSPrinter->GetSupportedFonts(&m_paryFonts);

			for (int i = 0; i < m_paryFonts.GetSize(); i++) {
				FontType* pFont = (FontType*)m_paryFonts.GetAt(i);
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pFontList->GetNewRow();
				if (pRow) {
					pRow->PutValue(flcNumber, pFont->nFontNumber);
					pRow->PutValue(flcName, _variant_t(pFont->strFontName));
					pRow->PutValue(flcChars, pFont->nFontChars);
					m_pFontList->AddRowAtEnd(pRow, NULL);
				}
			}

			
			if (nFontNumber == -1) {
				m_pFontList->CurSel = m_pFontList->GetFirstRow();
			}
			else {
				m_pFontList->SetSelByColumn(flcNumber, nFontNumber);
			}

			//GetMainFrame()->ReleasePOSPrinter();

		}

		

	}NxCatchAll("Error in OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
