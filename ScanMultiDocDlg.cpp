// ScanMultiDocDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practiceRc.h"
#include "ScanMultiDocDlg.h"
#include "mergeengine.h"
//#include "historydlg.h"
#include "GlobalUtils.h"
#include "ScannedImageViewerDlg.h"
#include "InternationalUtils.h"
#include "DontShowDlg.h"
#include "SelectDlg.h"
#include "FileUtils.h"
//TES 9/18/2008 - PLID 31413 - GetPatientDocumentPath() moved to HistoryUtils
#include "HistoryUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2008-07-25 10:53) - PLID 30836 - Massive overhaul of this dialog, too numerous to document every change,
// since entire functions were completely redone.
// (a.walling 2008-07-25 10:53) - PLID 30836 - Added support for detecting barcodes and processing them.

// (r.galicki 2008-09-09 14:22) - PLID 31280 - Convert DocList to DL2, use DL2 namespace
using namespace NXDATALIST2Lib;
/////////////////////////////////////////////////////////////////////////////
// CScanMultiDocDlg dialog

CBarcodeDetector CScanMultiDocDlg::m_BarcodeDetector;

CScanMultiDocDlg::CScanMultiDocDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CScanMultiDocDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScanMultiDocDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pScannedImageDlg = NULL;
	m_BarcodeDetector.SetScanRegion(CBarcodeDetector::eScanTop, 0.33f, 100);
	m_BarcodeDetector.SetLimit(1);
	m_BarcodeDetector.SetEnabled(TRUE);

	m_nCurrentIndex = 0;
	m_nLastUserDefinedID = LONG_MIN;
	m_nDefaultCategoryID = -1;

	m_nPatientID = -1;

	m_bFinishedScanning = FALSE;

	m_nDocNum = 0;
	m_nPIC = -1; // (r.galicki 2008-11-04 17:52) - PLID 31242 - Initialize PIC ID

	// (j.gruber 2012-08-13 17:43) - PLID 52094 - initialize
	m_dtService.SetDate(1800,12,31);
	m_dtService.SetStatus(COleDateTime::invalid);
	
}


void CScanMultiDocDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScanMultiDocDlg)
	DDX_Control(pDX, IDC_BTN_GROUP, m_nxibGroup);
	DDX_Control(pDX, IDC_LBL_SELECTED_DOCUMENTS, m_lblSelectedDocs);
	DDX_Control(pDX, IDC_BTN_ASSOCIATE_CATEGORY, m_nxibAssocCat);
	DDX_Control(pDX, IDC_LABEL_DEFAULT_CATEGORY, m_lblDefCat);
	DDX_Control(pDX, IDC_BTN_SHOW_PREVIEW, m_nxibShowPreview);
	DDX_Control(pDX, IDC_LABEL_DEFAULT_DOCUMENT_PREFIX, m_lblDocPrefix);
	DDX_Control(pDX, IDC_EDIT_DEFAULT_DOCUMENT_PREFIX, m_editDocPrefix);
	DDX_Control(pDX, IDC_CHECK_DETECT_BARCODES, m_nxbDetectBarcodes);
	DDX_Control(pDX, IDC_BEGIN_SCAN, m_nxibBeginScan);
	DDX_Control(pDX, IDC_ASSOCIATE, m_nxibAssociate);
	DDX_Control(pDX, IDC_CANCEL_CHANGES, m_btnClose);
	DDX_Control(pDX, IDC_COMMIT_SELECTION, m_btnCommit);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_nxcolor);
	DDX_Control(pDX, IDC_CHECK_MSCAN_ALWAYS_SAVE_PDF, m_nxbAlwaysSavePDF);
	DDX_Control(pDX, IDC_DEFAULT_PREFIX_CHECK, m_chkDefaultPrefix);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScanMultiDocDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ASSOCIATE, OnAssociate)
	ON_BN_CLICKED(IDC_BEGIN_SCAN, OnScanMultiple)
	ON_BN_CLICKED(IDC_COMMIT_SELECTION, OnCommit)
	ON_BN_CLICKED(IDC_CANCEL_CHANGES, OnCancelChanges)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_SHOW_PREVIEW, OnBtnShowPreview)
	ON_MESSAGE(NXM_TWAIN_IMAGE_SCANNED, OnTwainImageScanned)
	ON_MESSAGE(NXM_TWAIN_XFERDONE, OnTwainXferdone)
	ON_BN_CLICKED(IDC_CHECK_DETECT_BARCODES, OnCheckDetectBarcodes)
	ON_EN_CHANGE(IDC_EDIT_DEFAULT_DOCUMENT_PREFIX, OnChangeEditDefaultDocumentPrefix)
	ON_BN_CLICKED(IDC_BTN_ASSOCIATE_CATEGORY, OnBtnAssociateCategory)
	ON_BN_CLICKED(IDC_BTN_GROUP, OnBtnGroup)
	ON_BN_CLICKED(IDC_CHECK_MSCAN_ALWAYS_SAVE_PDF, OnBtnAlwaysSaveAsPDF)	
	ON_BN_CLICKED(IDC_DEFAULT_PREFIX_CHECK, &CScanMultiDocDlg::OnBnClickedDefaultPrefixCheck)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScanMultiDocDlg message handlers

BOOL CScanMultiDocDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//(e.lally 2010-01-13) PLID 32201 - Bulk cache preferences
		g_propManager.CachePropertiesInBulk("MultiDocScanDialog_Int", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'EnableBarcodeDetection' "
				" OR Name = 'ScanMultiDocCategory' "
				" OR Name = 'ScanMultiDocDefaultIsPhoto'"
				" OR Name = 'PDF_AutoLandscape' "
				" OR Name = 'PDF_UseThumbs' "
				" OR Name = 'PDF_PageSize' "
				" OR Name = 'ScanMultiDocAlwaysSavePDF' " //(e.lally 2010-01-13) PLID 32201
				")",
				_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("MultiDocScanDialog_Text", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'ScanMultiDocPrefix' "
				")",
				_Q(GetCurrentUserName()));

		// (c.haag 2010-02-01 11:41) - PLID 37086 - The default service date for newly scanned records
		// used to be the workstation date. It needs to instead be the server's date. So, figure out the
		// time differential here, and then we can apply it to every place we get COleDateTime::GetCurrentTime.
		try 
		{
			ADODB::_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS Now");
			if(!rs->eof) {
				COleDateTime dtServerNow = AdoFldDateTime(rs, "Now");
				COleDateTime dtLocalNow = COleDateTime::GetCurrentTime();

				m_dtOffset = dtServerNow - dtLocalNow;
				
				//now we can add m_dtOffset to COleDateTime::GetCurrentTime() to get a closer
				//approximation of what the server time actually is, without having to ask
				//the server
			}
			rs->Close();
		}
		catch (...) {
			m_dtOffset.SetDateTimeSpan(0,0,0,0);
			throw;
		}

		m_BarcodeDetector.SetHwnd(GetSafeHwnd());


		m_nxdService = GetDlgItemUnknown(IDC_EDIT_DEFAULT_SERVICE_DATE);

		// (a.walling 2008-08-01 11:03) - PLID 30916 - Check for barcode license
		BOOL bEnableBarcodeDetection = 
			GetRemotePropertyInt("EnableBarcodeDetection", 0, 0, "<None>", true)
				&&
			g_pLicense->CheckForLicense(CLicense::lcBarcodeRead, CLicense::cflrSilent);

		m_BarcodeDetector.SetEnabled(bEnableBarcodeDetection);
		m_nxbDetectBarcodes.SetCheck(bEnableBarcodeDetection ? BST_CHECKED : BST_UNCHECKED);

		//(e.lally 2010-01-13) PLID 32201 - Load option to always save scanned files as PDFs
		m_nxbAlwaysSavePDF.SetCheck(GetRemotePropertyInt("ScanMultiDocAlwaysSavePDF", 0, 0, GetCurrentUserName(), true)==0 ? BST_UNCHECKED : BST_CHECKED);
		
		m_strDefaultPrefix = GetRemotePropertyText("ScanMultiDocPrefix", "", 0, "<None>", true);
		m_editDocPrefix.SetWindowText(m_strDefaultPrefix);
		// (a.walling 2008-08-07 17:45) - PLID 30836 - Allow them to have a blank prefix
		/*
		if (m_strDocString.IsEmpty()) {
			m_strDocString = "Scanned Document";
		}
		*/
		m_nDefaultCategoryID = GetRemotePropertyInt("ScanMultiDocCategory", -1, 0, "<None>", true);
		//TES 8/3/2011 - PLID 44814 - Make sure the user has permissions for this category
		if(m_nDefaultCategoryID != -1 && !CheckCurrentUserPermissions(bioHistoryCategories, sptView, TRUE, m_nDefaultCategoryID, TRUE)) {
			m_nDefaultCategoryID = -1;
		}
		// (a.wilson 2012-5-8) PLID 48223 - Reusing this unused preference.
		// (r.galicki 2008-09-04 16:23) - PLID 31242 - Added default for IsPhoto field
		m_bDefaultIsPhoto = GetRemotePropertyInt("ScanMultiDocDefaultIsPhoto", FALSE, 0, "<None>", true);

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		
		// (a.walling 2008-08-07 16:40) - PLID 30836 - These should not be cancel/ok buttons, but close.
		// and associate should have modify-style text
		//m_btnClose.AutoSet(NXB_CANCEL);
		//m_btnCommit.AutoSet(NXB_OK);
		// (r.galicki 2008-09-11 12:40) - PLID 31242 - Close is now Cancel, Commit is now OK
		m_btnCommit.AutoSet(NXB_OK);
		m_btnClose.AutoSet(NXB_CANCEL);
		//m_btnClose.AutoSet(NXB_CLOSE);
		m_nxibAssociate.AutoSet(NXB_UP);
		m_nxibAssocCat.AutoSet(NXB_MODIFY);
		m_nxibAssociate.AutoSet(NXB_MODIFY);
		m_nxibShowPreview.AutoSet(NXB_INSPECT);

		// (r.galicki 2008-09-16 12:21) - PLID 31242 - Group NxIconButton
		m_nxibGroup.AutoSet(NXB_MODIFY);

		m_btnCommit.EnableWindow(FALSE);

		// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
		// (a.walling 2010-04-16 14:37) - PLID 38259
		m_pPatList = BindNxDataList2Ctrl(this, IDC_PATIENT_SCAN_LIST, GetRemoteData(), false);
		if (m_nPatientID != -1) {
			m_pPatList->WhereClause = (LPCTSTR)FormatString("PersonID >= 0 AND PersonID <> %li", m_nPatientID);
		}
		m_pPatList->Requery();
		
		//m_pPatientList = BindNxDataListCtrl(this, IDC_PATIENT_SCAN_LIST, GetRemoteData(), true);
		
		// (r.galicki 2008-09-08 17:34) - PLID 31242 - Add <No Patient> to patient list
		// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
		//NXDATALISTLib::IRowSettingsPtr pRow = m_pPatientList->GetRow(-1);
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatList->GetNewRow();
			pRow->PutValue(eplcPersonID, (long)-1);
			pRow->PutValue(eplcUserDefinedID, g_cvarNull);
			pRow->PutValue(eplcLast, " <No Patient>");
			pRow->PutValue(eplcFirst, g_cvarNull);
			pRow->PutValue(eplcMiddle, g_cvarNull);
			pRow->PutValue(eplcBirthDate, g_cvarNull);
			pRow->PutValue(eplcColor, g_cvarNull);
			m_pPatList->AddRowSorted(pRow, NULL);
		}

		// (a.walling 2010-04-16 14:43) - PLID 38259
		// Manually add the current patient row
		if (m_nPatientID != -1) {
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT PersonT.ID, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.BirthDate, CASE WHEN PersonT.Archived = 1 THEN 12632256 ELSE 0 END AS Color FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PersonT.ID = {INT}", m_nPatientID);
			if (!prs->eof) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatList->GetNewRow();
				pRow->PutValue(eplcPersonID, prs->Fields->Item["ID"]->Value);
				pRow->PutValue(eplcUserDefinedID, prs->Fields->Item["UserDefinedID"]->Value);
				pRow->PutValue(eplcLast, prs->Fields->Item["Last"]->Value);
				pRow->PutValue(eplcFirst, prs->Fields->Item["First"]->Value);
				pRow->PutValue(eplcMiddle, prs->Fields->Item["Middle"]->Value);
				pRow->PutValue(eplcBirthDate, prs->Fields->Item["BirthDate"]->Value);
				pRow->PutValue(eplcColor, prs->Fields->Item["Color"]->Value);
				m_pPatList->AddRowSorted(pRow, NULL);
			}
		}	

		// (r.galicki 2008-09-05 11:55) - PLID 31242 - Load current patient into patient list
		// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
		// (a.walling 2010-04-16 14:41) - PLID 38259
		if (m_nPatientID != -1) {
			m_pPatList->SetSelByColumn(eplcPersonID, _variant_t(m_nPatientID));
		}
		/*if(m_nPatientID != -1) {
			m_pPatientList->SetSelByColumn(eplcPersonID, _variant_t(m_nPatientID));
		}*/

		// (r.galicki 2008-09-09 10:44) - PLID 31280 - Converted Document List to a DataList 2
		m_pDocList = BindNxDataList2Ctrl(this, IDC_DOCUMENT_LIST, GetRemoteData(), false);
		//m_pDocList = BindNxDataListCtrl(this, IDC_DOCUMENT_LIST, GetRemoteData(), false);

		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		// (b.eyers 7/28/2014) - PLID 56981 - Got rid of the check on IsPatientTab
		m_pCatList = BindNxDataList2Ctrl(this, IDC_SCANMULTI_CATEGORY, GetRemoteData(), false);
		m_pCatList->FromClause = _bstr_t("(SELECT ID, Description FROM NoteCatsF WHERE " + GetAllowedCategoryClause("ID") + 
			" UNION SELECT -1 AS ID, ' {No Category}' AS Description) SubQ");
		m_pCatList->Requery();

		IColumnSettingsPtr pCol = m_pDocList->GetColumn(eCategory);
		if (pCol) {
			//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
			// (b.eyers 7/28/2014) - PLID 56981 - Got rid of the check on IsPatientTab
			pCol->PutComboSource(_bstr_t("SELECT ID, Description, 1 FROM NoteCatsF WHERE " + GetAllowedCategoryClause("ID") + " UNION SELECT -1, '{No Category}', 1 ORDER BY Description"));
		}

		m_pCatList->SetSelByColumn(0, _variant_t(m_nDefaultCategoryID));

		m_nxibBeginScan.SetFocus();
		CNxDialog::GetControlPositions(); 

	} NxCatchAll("Error in ScanMultiDocDlg::OnInitDialog");

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScanMultiDocDlg::OnAssociate()  {

	try {
		//make sure that at least one thing in each list is highlighted
		// (r.galicki 2008-09-09 14:20) - PLID 31280 - Converted DocList to DL2
		// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
		NXDATALIST2Lib::IRowSettingsPtr pCurPatSel = m_pPatList->GetCurSel();
		if (pCurPatSel == NULL || m_pDocList->GetCurSel() == NULL) { //m_pDocList->CurSel == -1) {
			MessageBox("Please select at least one item from each list.");
			return;
		}

		//get the patient that is highlighted first
		// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
		long nSelPatID = VarLong(pCurPatSel->GetValue(eplcPersonID), -1);
		
		if (nSelPatID < 0) {
			MessageBox("Invalid patient selected");
			return;
		}


		//now get the array IDs that are highlighted in the other list
		// (r.galicki 2008-09-09 10:22) - PLID 31280 - Converted DocList to DL2
		// (r.galicki 2008-09-11 12:26) - PLID 31242 - Added check to not associate any pages currently in a group.
		long nCount = 0;
		IRowSettingsPtr pRow = m_pDocList->GetFirstSelRow();
		while(pRow) {

			if(pRow->GetParentRow() == NULL) {
				// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
				pRow->PutValue(ePersonID, pCurPatSel->GetValue(eplcPersonID));
				pRow->PutValue(eUserDefinedID, pCurPatSel->GetValue(eplcUserDefinedID));
				pRow->PutValue(eLast, pCurPatSel->GetValue(eplcLast));
				pRow->PutValue(eFirst, pCurPatSel->GetValue(eplcFirst));
				pRow->PutValue(eMiddle, pCurPatSel->GetValue(eplcMiddle));
				//->PutForeColor(RGB(0,0,0));
			}
			else {
				nCount++;
			}
			EnsureRowColor(pRow);
			pRow = pRow->GetNextSelRow();
		}
		/*long p = m_pDocList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	
			m_pDocList->GetNextSelEnum(&p, &pDisp);
			IRowSettingsPtr pRow(pDisp);
			
			pRow->PutValue(ePersonID, m_pPatientList->GetValue(m_pPatientList->CurSel, eplcPersonID));
			pRow->PutValue(eUserDefinedID, m_pPatientList->GetValue(m_pPatientList->CurSel, eplcUserDefinedID));
			pRow->PutValue(eLast, m_pPatientList->GetValue(m_pPatientList->CurSel, eplcLast));
			pRow->PutValue(eFirst, m_pPatientList->GetValue(m_pPatientList->CurSel, eplcFirst));
			pRow->PutValue(eMiddle, m_pPatientList->GetValue(m_pPatientList->CurSel, eplcMiddle));

			pRow->PutForeColor(RGB(0,0,0));
			
			pDisp->Release();
		}*/
		
		// (r.galicki 2008-09-11 12:30) - PLID 31242 - Warn that certain documents are already attached to a group, cannot associate with patient.
		if(nCount > 0) {
			MessageBox(FormatString("%li documents could not be associated to the selected patient, since they are part of a group.", nCount), NULL, MB_OK | MB_ICONEXCLAMATION);
		}

	}NxCatchAll("Error Associating Documents");
}

// (a.walling 2008-09-03 14:16) - PLID 22821 - use a gdiplus bitmap
// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
void WINAPI CALLBACK CScanMultiDocDlg::OnTWAINCallback(NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
		const CString& strFileName, /* The full filename of the document that was scanned */
		BOOL& bAttach, void* pUserData, CxImage& cxImage)
{
	try {
		//we just need to add our info to the datalist
		if (pUserData) {
			CScannedDocument sd;

			CBarcodeDetector* pBarcodeDetector = (CBarcodeDetector*)pUserData;

			if (pBarcodeDetector->GetEnabled()) {
				CStringArray saResults;
				// (a.walling 2010-04-08 08:23) - PLID 38170 - Back to CxImage!
				pBarcodeDetector->FindBarcode(cxImage, saResults);
				if (saResults.GetSize() > 0) {
					sd.strBarcode = saResults[0];
				}
			}

			sd.strFilename = strFileName;

			if (::IsWindow(pBarcodeDetector->GetHwnd())) {
				::SendMessage(pBarcodeDetector->GetHwnd(), NXM_TWAIN_IMAGE_SCANNED, (WPARAM)&sd, (LPARAM)0);
			}

			bAttach = FALSE;
		}
	} NxCatchAll_NoParent("CScanMultiDocDlg::OnTWAINCallback"); // (a.walling 2014-05-05 13:32) - PLID 61945
}

// (a.walling 2010-01-28 14:31) - PLID 28806 - Now passes a connection pointer
void WINAPI CALLBACK CScanMultiDocDlg::OnNxTwainPreCompress(
		const LPBITMAPINFO pbmi, // 
		BOOL& bAttach, // bAttach
		BOOL& bAttachChecksum, // bAttachChecksum
		long& nChecksum, // Checksum
		ADODB::_Connection* lpCon) // Connection
{
	return;
}


//void CALLBACK OnTWAINCallback(NXTWAINlib::EScanType type, const CString& strFileName, BOOL& bAttach, void* pUserData );


void CScanMultiDocDlg::OnScanMultiple()
{
	try {
		m_bFinishedScanning = FALSE;
		m_pCatList->PutEnabled(VARIANT_FALSE);
		m_editDocPrefix.EnableWindow(FALSE);
		m_nxbDetectBarcodes.EnableWindow(FALSE);
		m_nxibBeginScan.EnableWindow(FALSE);
		m_btnCommit.EnableWindow(FALSE);
		m_btnClose.EnableWindow(FALSE);

		BOOL bFalse = FALSE;
		// (a.walling 2008-08-01 11:03) - PLID 30916 - Use the barcode license
		if (m_nxbDetectBarcodes.GetCheck() == BST_CHECKED && g_pLicense->CheckForLicense(CLicense::lcBarcodeRead, CLicense::cflrSilent)) {
			m_BarcodeDetector.SetEnabled(g_pLicense->CheckForLicense(CLicense::lcBarcodeRead, CLicense::cflrUse));
		} else {
			m_BarcodeDetector.SetEnabled(FALSE);
		}
		
		GetMainFrame()->RegisterTwainDlgWaiting(this);

		// (a.walling 2009-12-11 13:24) - PLID 36518 - Force scanning to native so we can scan the dib/Bitmap*
		NXTWAINlib::Acquire(-25, GetNxTempPath(), CScanMultiDocDlg::OnTWAINCallback, CScanMultiDocDlg::OnNxTwainPreCompress, (void*)&m_BarcodeDetector, "", -1, -1, -1, NXTWAINlib::eScanToNative);
	} NxCatchAll("Error in CScanMultiDocDlg::OnScanMultiple()");
}

CString CScanMultiDocDlg::GetDocumentName() {

	CString strDoc, strDocTmp;
	// (a.wilson 2013-05-07 13:49) - PLID 52960 - pull whatever is currently contained in the prefix edit control.
	m_editDocPrefix.GetWindowText(strDocTmp);
	// (a.walling 2008-08-07 17:45) - PLID 30836 - Allow them to have a blank prefix
	/*
	else {
		strDocTmp = "Scanned Document";
	}
	*/

	//now figure out the number be are on
	DWORD nID = ++m_nDocNum;

	// (c.haag 2010-02-01 11:46) - PLID 37086 - Add m_dtOffset so that we store the time in the server's "time zone"
	strDoc.Format("%s %s %04lu", strDocTmp, FormatDateTimeForInterface(COleDateTime::GetCurrentTime() + m_dtOffset, NULL, dtoDate), nID);

	strDoc.TrimLeft();
	strDoc.TrimRight();

	return strDoc;
}


void CScanMultiDocDlg::OnCommit()  {
	IProgressDialog* pProgressDialog = NULL;
	HMODULE hMod = NULL;
	try {
		CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_ALL, IID_IProgressDialog, (void **)&pProgressDialog);

		if (pProgressDialog) {
			pProgressDialog->SetTitle(_bstr_t("Import into Practice History"));
			pProgressDialog->SetCancelMsg(_bstr_t("This operation cannot be cancelled."), NULL);   // Will only be displayed if Cancel button is pressed.
			pProgressDialog->SetLine(1, _bstr_t("Documents are being imported..."), FALSE, NULL);
			pProgressDialog->SetLine(2, _bstr_t(""), FALSE, NULL);
			
			DWORD dwFlags = PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE;
			if (IsVistaOrGreater()) {
#define PROGDLG_NOCANCEL        0x00000040      // No cancel button (operation cannot be canceled) (use sparingly)
				dwFlags |= PROGDLG_NOCANCEL;
			} else {
				hMod = LoadLibrary("Shell32.dll");
				pProgressDialog->SetAnimation(hMod, 165);               // Set the animation to play.
			}

			pProgressDialog->StartProgressDialog(NULL, NULL, dwFlags, NULL); // Display and enable automatic estimated time remaining.
		}

		//enter everything into the mail sent table
		long nFailed = 0, nSkipped = 0;
		DWORD nCurrent = 0;
		DWORD nCount = m_pDocList->GetRowCount();
		
		// (r.galicki 2008-09-09 12:10) - PLID 31280 - Converted DocList to DL2
		IRowSettingsPtr pRow = m_pDocList->GetFirstRow();
		//(e.lally 2010-01-13) PLID 32201 - Check option to always save scanned files as PDFs
		const BOOL bAlwaysCreatePDF = (m_nxbAlwaysSavePDF.GetCheck() == BST_CHECKED);
		while (pRow) {
			BOOL bRowSkipped = FALSE;
			if (pProgressDialog) {
				pProgressDialog->SetProgress(nCurrent, nCount);
			}
			//get the patient ID
			long nPatientID = VarLong(pRow->GetValue(ePersonID), LONG_MIN);
			CString strDocName = VarString(pRow->GetValue(eName), "");
			CString strDocFile = VarString(pRow->GetValue(ePath), "");
			_variant_t varDocCat = pRow->GetValue(eCategory);
			long nDocCat = -1;
			if (varDocCat.vt == VT_BSTR) {
				CString strDocCat = VarString(varDocCat, "");
				nDocCat = atoi(strDocCat);
				if (nDocCat == 0 && strDocCat != "0") {
					nDocCat = -1;
				}
			} else if (varDocCat.vt == VT_I4) {
				nDocCat = VarLong(varDocCat);
			}

			CString strFirst = VarString(pRow->GetValue(eFirst), "");
			CString strLast = VarString(pRow->GetValue(eLast), "");
			long nUserDefinedID = VarLong(pRow->GetValue(eUserDefinedID), -1);

			// (a.wilson 2012-5-8) PLID 48223 - m_bDefaultIsPhoto is not a reliable default.  Defaulting to FALSE.
			// (r.galicki 2008-09-04 10:42) - PLID 31242 - Added support for IsPhoto and ServiceDate
			BOOL bIsPhoto = VarBool(pRow->GetValue(eIsPhoto), FALSE);

			COleDateTime dtService = VarDateTime(pRow->GetValue(eServiceDate));

			CStringArray saTempFiles;
			if (VarLong(pRow->GetValue(eScanID), -2) == -1) { // Grouped document
				IRowSettingsPtr pChild = pRow->GetFirstChildRow();
				ASSERT(pChild != NULL);

				while (pChild) {
					saTempFiles.Add(VarString(pChild->GetValue(ePath), ""));

					pChild = pChild->GetNextRow();
				}

				bIsPhoto = FALSE;
			}

			if (nPatientID != LONG_MIN) {
				try {
					// (r.galicki 2008-09-04 10:42) - PLID 31242 - Added support for IsPhoto and ServiceDate
					//	Also changed the category ID parameter to be passed in as a long rather than as a string (for simplifying use with CreateNewMailSentEntry)
					//(e.lally 2010-01-13) PLID 32201 - Pass in option to always save scanned files as PDFs
					AddToDocuments(pProgressDialog, nPatientID, nDocCat, strDocName, strDocFile, strFirst, strLast, nUserDefinedID, bIsPhoto, bAlwaysCreatePDF, dtService, saTempFiles);
					
				} catch (CException *e) {
					char errorMessage[4096];
					// (b.cardillo 2006-12-13 12:09) - PLID 23852 - If the call to GetErrorMessage() fails, we need to fill 
					// the string with a generic message instead of leaving it as garbage.
					if (!e->GetErrorMessage(errorMessage, 4096)) {
						strcpy(errorMessage, "Unknown CException!");
					}
					LogDetail("Error importing scanned document into Practice: %s", errorMessage);
					nFailed++;
				} catch (_com_error e) {
					CString strErrorText;
					strErrorText.Format("Error %li: %s; Description '%s'", e.Error(), e.ErrorMessage(), _Q((LPCTSTR)e.Description()));
					LogDetail("Error importing scanned document into Practice: %s", strErrorText);
					nFailed++;
				} catch (...) {
					// well if we get this, who knows what happened, pass it to our standard handler and quit
					LogDetail("Unknown error importing scanned document into Practice");
					nFailed++;
					if (pProgressDialog) {
						pProgressDialog->StopProgressDialog();
						pProgressDialog->Release();
						pProgressDialog = NULL;
					}
					throw;
				}
			} else {
				nSkipped++;
				bRowSkipped = TRUE;
			}

			nCurrent++;
			
			IRowSettingsPtr pOldRow = pRow;
			pRow = pOldRow->GetNextRow();
			if(!bRowSkipped) {
				m_pDocList->RemoveRow(pOldRow);
			}
		}

		if (pProgressDialog) {
			pProgressDialog->StopProgressDialog();
			pProgressDialog->Release();
			pProgressDialog = NULL;
		}
		if (hMod) {
			FreeLibrary(hMod);
			hMod = NULL;
		}

		CString strMessage;
		
		if (nFailed > 0) {
			strMessage += FormatString("%li documents failed to be attached! Please see the log file for further information and contact NexTech Technical Support if necessary.\r\n", nFailed);
		}
		if (nSkipped > 0) {
			strMessage += FormatString("%li documents had no associated patient, and were skipped.\r\n", nSkipped);
		}
		if (!strMessage.IsEmpty()) {
			MessageBox(strMessage, NULL, MB_OK | MB_ICONEXCLAMATION);
		}

		// (j.jones 2014-08-04 13:31) - PLID 63141 - removed the tablechecker call here, because
		// CreateMailSentEntry already sent one for us

	}NxCatchAllCall("Could not commit documents to history", {
			if (pProgressDialog) {
				pProgressDialog->StopProgressDialog();
				pProgressDialog->Release();
			}
			if (hMod) {
				FreeLibrary(hMod);
				hMod = NULL;
			}
		});
}

// (a.walling 2008-09-12 14:06) - PLID 31364 - Added saTempFiles for creating a grouped PDF document
//(e.lally 2010-01-13) PLID 32201 - Added bAlwaysCreatePDF
void CScanMultiDocDlg::AddToDocuments(IProgressDialog* pProgressDialog, long nPatientID, long nDocCat, CString strDocName, CString strDocFile, CString strFirst, CString strLast, long nUserDefinedID, BOOL bIsPhoto, const BOOL bCreateAsPDF, const COleDateTime &dtService, CStringArray& saTempFiles)
{
	CString strFileName, strPatientFolder;
	//we aren't going to let them actually choose the filename, we are going to make it for them and they get to choose the note

	// (a.walling 2008-09-12 14:06) - PLID 31364 - At some point we'll probably make a preference to import all of the documents
	// as PDF by default, ie the single-page ones that are not grouped. It is currently valid to have a group with only one page
	//(e.lally 2010-01-13) PLID 32201 - Option to always save scanned files as PDFs is now passed in
	//const BOOL bAlwaysCreatePDF = FALSE;

	// (a.walling 2008-09-12 14:06) - PLID 31364 - Use a PDF extension if necessary
	if (bCreateAsPDF || saTempFiles.GetSize() > 0) {
		strFileName = GetPatientDocumentName(nPatientID, "pdf");
	} else {
		strFileName = GetPatientDocumentName(nPatientID, FileUtils::GetFileExtension(strDocFile));
	}
	// (a.walling 2010-04-28 17:38) - PLID 38410 - Pass in a connection to GetPatientDocumentPath
	strPatientFolder = GetPatientDocumentPath(GetRemoteData(), nPatientID, strFirst, strLast, nUserDefinedID);

	// (a.walling 2010-05-04 14:33) - PLID 38493 - Don't trim the output file name!)(@*)(!! Otherwise it does not
	// save in the correct format, causing GetPatientDocumentName to ignore this entry once it is saved to MailSent.
	//strFileName.TrimLeft();
	//strFileName.TrimRight();
	strPatientFolder.TrimRight();
	strPatientFolder.TrimLeft();
	strDocFile.TrimLeft();
	strDocFile.TrimRight();
	// (r.galicki 2008-09-05 11:38) - PLID 31242 - DocCat has been changed to a long parameter rather than a string
	//strDocCat.TrimLeft();
	//strDocCat.TrimRight();

	if (DoesExist(strPatientFolder)) {

		if (pProgressDialog) {
			pProgressDialog->SetLine(2, _bstr_t(strPatientFolder^strFileName), TRUE, NULL);
		}

		BOOL bSuccess = FALSE;

		// (a.walling 2008-09-12 14:07) - PLID 31364 - Create a PDF if we have a listing of page files
		if (bCreateAsPDF || saTempFiles.GetSize() > 0) {
			NxPdf::IPdfDocumentPtr pPdfDocument;
			HRESULT hr = pPdfDocument.CreateInstance("NxPdf.PdfDocument");
			if (pPdfDocument) {
				// (a.walling 2008-09-08 12:58) - PLID 31293 - Apply PDF preferences
				pPdfDocument->AutoLandscape = GetRemotePropertyInt("PDF_AutoLandscape", TRUE, 0, "<None>") ? VARIANT_TRUE : VARIANT_FALSE;
				pPdfDocument->UseThumbs = GetRemotePropertyInt("PDF_UseThumbs", TRUE, 0, "<None>") ? VARIANT_TRUE : VARIANT_FALSE;
				pPdfDocument->PageSize = GetRemotePropertyInt("PDF_PageSize", NxPdf::psLetter, 0, "<None>", true);

				if (saTempFiles.GetSize() > 0) {
					// create a PDF
					ASSERT(strDocFile.IsEmpty());

					for (int i = 0; i < saTempFiles.GetSize(); i++) {
						// (a.walling 2010-01-06 14:15) - PLID 36771 - Add from PNG
						CString strExtension = FileUtils::GetFileExtension(saTempFiles[i]);
						strExtension.MakeLower();
						if (strExtension == "png") {
							pPdfDocument->AddNewPageFromPNG(_bstr_t(saTempFiles[i]));
						} else {						
							pPdfDocument->AddNewPageFromJPEG(_bstr_t(saTempFiles[i]));
						}
					}
				} else if (bCreateAsPDF) {
					// create a PDF
					ASSERT(!strDocFile.IsEmpty());

					// (a.walling 2010-01-06 14:15) - PLID 36771 - Add from PNG
					CString strExtension = FileUtils::GetFileExtension(strDocFile);
					strExtension.MakeLower();
					if (strExtension == "png") {
						pPdfDocument->AddNewPageFromPNG(_bstr_t(strDocFile));
					} else {						
						pPdfDocument->AddNewPageFromJPEG(_bstr_t(strDocFile));
					}
				} 

				HR(pPdfDocument->SaveToFile(_bstr_t(strPatientFolder^strFileName)));

				bSuccess = TRUE;
			} else {
				_com_issue_error(hr);
			}
		} else {
			// just copy the image
			bSuccess = CopyFile(strDocFile, strPatientFolder^strFileName, TRUE);
		}
		//copy the file from the temp folder to the patient's document path
		if (bSuccess) {
			
			// (a.wetta 2007-07-09 16:49) - PLID 17467 - If this file is an image, determine if it should be added as a photo
			// (a.walling 2007-07-25 15:55) - PLID 17467 - For safety sake, don't bother prompting if we are not a single person			
			// (r.galicki 2008-09-04 09:34) - PLID 31242 - IsPhoto is now passed in as a parameter
			// (r.galicki 2008-09-05 11:19) - PLID 31242 - Using CreateMailSentEntry function
			// (a.walling 2009-12-28 13:34) - PLID 36713 - Do not _Q these; that is the responsibility of CreateNewMailSentEntry
			// (d.singleton 2013-11-15 11:23) - PLID 59513 - need to insert the CCDAType when generating a CCDA
			CreateNewMailSentEntry(nPatientID,  strDocName, "BITMAP:FILE", strFileName, GetCurrentUserName(),
				"", GetCurrentLocationID(), dtService, 
				-1, nDocCat, m_nPIC, -1, bIsPhoto, -1, "", ctNone);

			// now delete the temp copy
			if (!strDocFile.IsEmpty()) {
				GetMainFrame()->DeleteFileWhenPossible(strDocFile);
			}

			for (int i = 0; i < saTempFiles.GetSize(); i++) {
				GetMainFrame()->DeleteFileWhenPossible(saTempFiles[i]);
			}
		}
		else {
			DWORD dw = GetLastError();

			CString strErr;
			AfxThrowNxException("The document %s for patient %li was not able to be correctly attached to their history file. (Error %li)", strDocName, nPatientID, dw);
		}
	}
	else {
		ASSERT(FALSE);
		AfxThrowNxException("Person document folder '%s' could not be found", strPatientFolder);
	}
}

void CScanMultiDocDlg::OnCancelChanges()  {
	OnCancel();
}

BEGIN_EVENTSINK_MAP(CScanMultiDocDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CScanMultiDocDlg)
	ON_EVENT(CScanMultiDocDlg, IDC_SCANMULTI_CATEGORY, 29 /* SelSet */, OnSelSetCategory, VTS_DISPATCH)
	ON_EVENT(CScanMultiDocDlg, IDC_DOCUMENT_LIST, 29 /* SelSet */, OnSelSetDocumentList, VTS_DISPATCH)
	ON_EVENT(CScanMultiDocDlg, IDC_DOCUMENT_LIST, 6 /* RButtonDown */, OnRButtonDownDocumentList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CScanMultiDocDlg, IDC_DOCUMENT_LIST, 8 /* EditingStarting */, OnEditingStartingDocumentList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CScanMultiDocDlg, IDC_DOCUMENT_LIST, 14 /* DragEnd */, OnDragEndDocumentList, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CScanMultiDocDlg, IDC_DOCUMENT_LIST, 12 /* DragBegin */, OnDragBeginDocumentList, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CScanMultiDocDlg, IDC_DOCUMENT_LIST, 9, CScanMultiDocDlg::OnEditingFinishingDocumentList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CScanMultiDocDlg, IDC_PATIENT_SCAN_LIST, 16, CScanMultiDocDlg::SelChosenPatientScanList, VTS_DISPATCH)
	ON_EVENT(CScanMultiDocDlg, IDC_EDIT_DEFAULT_SERVICE_DATE, 1 /* KillFocus */, CScanMultiDocDlg::OnEnKillfocusEditDefaultServiceDate, VTS_NONE)
END_EVENTSINK_MAP()

// (r.galicki 2008-09-09 12:48) - PLID 31280 - Coverted DocList to DL2 - These functions are obsolete (were not even used anyway)
/*
void CScanMultiDocDlg::OnColumnClickingDocumentList(short nCol, BOOL FAR* bAllowSort) 
{
	
}

void CScanMultiDocDlg::OnLeftClickDocumentList(long nRow, short nCol, long x, long y, long nFlags) 
{

}

void CScanMultiDocDlg::OnEditingFinishedDocumentList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{

}
void CScanMultiDocDlg::OnEditingFinishingDocumentList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	
}
*/

void CScanMultiDocDlg::PreviewImage(const CString& strPath)
{	
	if (m_pScannedImageDlg == NULL)
		return;

	if (m_pScannedImageDlg->IsWindowVisible()) {
		m_pScannedImageDlg->SetImage(strPath);
	}
}

void CScanMultiDocDlg::OnDestroy() 
{
	CNxDialog::OnDestroy();
	
	try {
		if (m_pScannedImageDlg) {
			::DestroyWindow(m_pScannedImageDlg->GetSafeHwnd());
			delete m_pScannedImageDlg;
			m_pScannedImageDlg = NULL;
		}
	} NxCatchAll("Error in CScanMultiDocDlg::OnDestroy");
}

void CScanMultiDocDlg::OnCancel()
{
	if (!m_btnClose.IsWindowEnabled())
		return;

	long nCount = 0;

	IRowSettingsPtr pRow = m_pDocList->GetFirstRow();
	while (pRow) {
		nCount++;
		pRow = pRow->GetNextRow();
	}

	if (nCount > 0) {
		if (IDNO == MessageBox(FormatString("There are %li documents that have not been imported into Practice. By closing, these files will be deleted and must be scanned again. Are you sure you want to close?", nCount), NULL, MB_YESNO | MB_ICONEXCLAMATION)) {
			return;
		} else {
			CleanupFiles();
		}
	}
	CNxDialog::OnCancel();
}

void CScanMultiDocDlg::OnOK()
{
	OnCommit();
}

void CScanMultiDocDlg::OnBtnShowPreview() 
{	
	try {
		CString strPath;
		// (r.galicki 2008-09-09 11:13) - PLID 31280 - Converted DocList to DL2
		IRowSettingsPtr pRow = m_pDocList->GetCurSel();
		if(pRow) {//if (m_pDocList->CurSel != -1) {
			//strPath = VarString(m_pDocList->GetValue(m_pDocList->CurSel, ePath), "");
			strPath = VarString(pRow->GetValue(ePath), "");
		}

		if (m_pScannedImageDlg == NULL) {
			m_pScannedImageDlg = new CScannedImageViewerDlg(strPath, this);
			m_pScannedImageDlg->Create(IDD_SCANNED_IMAGE_VIEWER_DLG, GetMainFrame());
		}

		m_pScannedImageDlg->ShowWindow(SW_SHOWNA);
		PreviewImage(strPath);
	} NxCatchAll("CScanMultiDocDlg::OnBtnShowPreview");
}

LRESULT CScanMultiDocDlg::OnTwainImageScanned(WPARAM wParam, LPARAM lParam)
{
	try {
		CScannedDocument* pSD = (CScannedDocument*)wParam;

		if (pSD) {
			// (r.galicki 2008-09-09 10:53) - PLID 31280 - Converted DocList to DL2
			IRowSettingsPtr pRow = m_pDocList->GetNewRow();
			//IRowSettingsPtr pRow = m_pDocList->GetRow(sriGetNewRow);
			pRow->PutValue(eScanID, m_nCurrentIndex++);
			pRow->PutValue(ePath, (LPCTSTR)pSD->strFilename);
			pRow->PutValue(eName, (LPCTSTR)GetDocumentName());
			if (m_nDefaultCategoryID != -1) {
				pRow->PutValue(eCategory, _variant_t((LPCTSTR)AsString(m_nDefaultCategoryID)));
			} else {
				pRow->PutValue(eCategory, g_cvarNull);
			}

			long nUserDefinedID = m_nLastUserDefinedID;

			if (!pSD->strBarcode.IsEmpty()) {
				CString strParsedBarcode = pSD->strBarcode;
				LogDetail("Barcode detected: '%s'", strParsedBarcode);
				if (strParsedBarcode.Left(4) == "NXID") {
					strParsedBarcode = strParsedBarcode.Mid(4);
					long nParsedUserDefinedID = atoi(strParsedBarcode);

					if (nParsedUserDefinedID == 0 && strParsedBarcode != "0") {
						// bad code!
						nUserDefinedID = LONG_MIN;
					} else {
						nUserDefinedID = nParsedUserDefinedID;
					}
					
					m_nLastUserDefinedID = nUserDefinedID;
				}
			}

			if (nUserDefinedID != LONG_MIN) {
				// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
				if (m_pPatList->IsRequerying() == VARIANT_TRUE) {
					// (r.galicki 2008-09-09 14:21) - PLID 31280 - Converted DocList to DL2 - Must specify use of DL1 namespace
					// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
					m_pPatList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
				}
				// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
				NXDATALIST2Lib::IRowSettingsPtr pPatRow = m_pPatList->FindByColumn(eplcUserDefinedID, _variant_t(nUserDefinedID), NULL, VARIANT_FALSE);
				if (pPatRow != NULL) {
					pRow->PutValue(ePersonID, pPatRow->GetValue(eplcPersonID));
					pRow->PutValue(eLast, pPatRow->GetValue(eplcLast));
					pRow->PutValue(eFirst, pPatRow->GetValue(eplcFirst));
					pRow->PutValue(eMiddle, pPatRow->GetValue(eplcMiddle));
					pRow->PutValue(eUserDefinedID, pPatRow->GetValue(eplcUserDefinedID));
				} else {
					// cannot find the ID! color it red
					//pRow->PutForeColor(RGB(128, 0, 0));

					pRow->PutValue(ePersonID, g_cvarNull);
					pRow->PutValue(eLast, g_cvarNull);
					pRow->PutValue(eFirst, g_cvarNull);
					pRow->PutValue(eMiddle, g_cvarNull);
					pRow->PutValue(eUserDefinedID, nUserDefinedID);			
				}
			} else {
				//pRow->PutForeColor(RGB(128, 32, 64));

				// (r.galicki 2008-09-08 09:53) - PLID 31242 - If patient ID has been defined, load that patient's info for new scanned image
				if(m_nPatientID != -1) {
					// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
					NXDATALIST2Lib::IRowSettingsPtr pPatRow = m_pPatList->FindByColumn(eplcPersonID, _variant_t(m_nPatientID), NULL, VARIANT_FALSE);
					if(pPatRow != NULL) { //valid row
						pRow->PutValue(ePersonID, pPatRow->GetValue(eplcPersonID));
						pRow->PutValue(eUserDefinedID, pPatRow->GetValue(eplcUserDefinedID));
						pRow->PutValue(eLast, pPatRow->GetValue(eplcLast));
						pRow->PutValue(eFirst, pPatRow->GetValue(eplcFirst));
						pRow->PutValue(eMiddle, pPatRow->GetValue(eplcMiddle));
					}
					else {
						pRow->PutValue(ePersonID, g_cvarNull);
						pRow->PutValue(eLast, g_cvarNull);
						pRow->PutValue(eFirst, g_cvarNull);
						pRow->PutValue(eMiddle, g_cvarNull);
						pRow->PutValue(eUserDefinedID, g_cvarNull);
					}
				}
				else {
					pRow->PutValue(ePersonID, g_cvarNull);
					pRow->PutValue(eLast, g_cvarNull);
					pRow->PutValue(eFirst, g_cvarNull);
					pRow->PutValue(eMiddle, g_cvarNull);
					pRow->PutValue(eUserDefinedID, g_cvarNull);
				}	
			}
			// (a.wilson 2012-5-8) PLID 48223 - if they have the preference ScanMultiDocDefaultIsPhoto set to true
			//	then we need to check if the file is actually a photo and mark as such otherwise mark unchecked.
			// (r.galicki 2008-09-03 17:27) - PLID 31242 - set IsPhoto and Service Date defaults (for all scanned documents)
			if (m_bDefaultIsPhoto) {
				pRow->PutValue(eIsPhoto, (IsImageFile(pSD->strFilename) ? g_cvarTrue : g_cvarFalse)); 
			} else {
				pRow->PutValue(eIsPhoto, g_cvarFalse);
			}
			// (c.haag 2010-02-01 11:46) - PLID 37086 - Add m_dtOffset so that we store the time in the server's "time zone"
			// (j.gruber 2012-08-13 17:02) - PLID 52094 - unless we have a default service date
			if (m_dtService.GetStatus() == COleDateTime::valid) {
				pRow->PutValue(eServiceDate, _variant_t(m_dtService, VT_DATE));
			}
			else {
				pRow->PutValue(eServiceDate, _variant_t(AsDateNoTime(COleDateTime::GetCurrentTime() + m_dtOffset), VT_DATE));
			}

			// (r.galicki 2008-09-09 10:54) - PLID 31280 - Converted DocList to DL2
			m_pDocList->AddRowAtEnd(pRow, NULL);
			//m_pDocList->AddRow(pRow);

			EnsureRowColor(pRow);
		}
	} NxCatchAll("CScanMultiDocDlg::OnTwainImageScanned");

	return 0;
}

void CScanMultiDocDlg::OnCheckDetectBarcodes() 
{
	try {
		BOOL bEnabled = m_nxbDetectBarcodes.GetCheck() == BST_CHECKED;
		SetRemotePropertyInt("EnableBarcodeDetection", bEnabled, 0, "<None>");

		if (bEnabled) {
			// (a.walling 2008-08-01 11:07) - PLID 30916 - Prompt if unlicensed
			if (g_pLicense->CheckForLicense(CLicense::lcBarcodeRead, CLicense::cflrSilent)) {
				DontShowMeAgain(this, "Images will be scanned for Patient ID barcodes and all following documents will be associated with that patient. The top third of the page will be scanned for the barcode. You may generate these by running the 'Patient Barcode Sheet' report. For the most accurate detection of scanned barcodes, the image should be scanned in at 300dpi or greater, or an absolute minimum resolution of 150dpi.",
					"ScanMultiDocPatientBarcodes", "Practice", FALSE, FALSE);
			} else {
				MessageBox("You are not currently licensed to use barcode detection. This feature allows patients to be automatically assigned to scanned documents.\r\n\r\n"
							"If you would like more information on this product, please contact your sales representative at "
							"(800) 490-0821.");

				m_nxbDetectBarcodes.SetCheck(BST_UNCHECKED);
			}
		}	
	} NxCatchAll("CScanMultiDocDlg::OnCheckDetectBarcodes");
}
// (a.wilson 2013-05-07 13:52) - PLID 52960 - check if the current edit control text is the same as the default and set the check.
void CScanMultiDocDlg::OnChangeEditDefaultDocumentPrefix() 
{
	try {
		CString strCurrent;
		m_editDocPrefix.GetWindowText(strCurrent);
		if (m_strDefaultPrefix == strCurrent) {	//if the edit text and the default are the same then set checked.
			m_chkDefaultPrefix.SetCheck(BST_CHECKED);
		} else {	//if they are at all different then set unchecked.
			m_chkDefaultPrefix.SetCheck(BST_UNCHECKED);
		}
	} NxCatchAll(__FUNCTION__);
}

void CScanMultiDocDlg::OnSelSetCategory(LPDISPATCH lpSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpSel);

		if (pRow) {
			m_nDefaultCategoryID = VarLong(pRow->GetValue(0));
			SetRemotePropertyInt("ScanMultiDocCategory", m_nDefaultCategoryID, 0, "<None>");
		} else {
			// (a.walling 2008-08-07 17:48) - PLID 30836 - If they choose no row, just assume it is the {No Category} row.
			m_nDefaultCategoryID = -1;
		}
	} NxCatchAll("CScanMultiDocDlg::OnSelSetCategory");
}

LRESULT CScanMultiDocDlg::OnTwainXferdone(WPARAM wParam, LPARAM lParam)
{
	try {
		m_bFinishedScanning = TRUE;

		m_nLastUserDefinedID = LONG_MIN;

		m_pCatList->PutEnabled(VARIANT_TRUE);
		m_editDocPrefix.EnableWindow(TRUE);
		m_nxbDetectBarcodes.EnableWindow(TRUE);
		m_nxibBeginScan.EnableWindow(TRUE);
		m_btnCommit.EnableWindow(m_pDocList->GetRowCount() > 0 ? TRUE : FALSE);
		m_btnClose.EnableWindow(TRUE);
		
		GetMainFrame()->UnregisterTwainDlgWaiting(this);	
	} NxCatchAll("CScanMultiDocDlg::OnTwainXferdone");
	return 0;
}

void CScanMultiDocDlg::OnBtnAssociateCategory() 
{
	try {
		// (r.galicki 2008-09-09 11:37) - PLID 31280 - Converted DocList to DL2
		if (m_pDocList->GetCurSel() == NULL) {
			return;
		}
		
		/*if (m_pDocList->GetCurSel() == sriNoRow)
			return;*/
		
		long nSelectedCatID = -2;

		CSelectDlg dlgCat(this);
		dlgCat.m_strTitle = "Select a category";
		dlgCat.m_strCaption = "Choose a category to apply to all selected documents";
		dlgCat.m_strFromClause = "NoteCatsF";
		//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
		// (b.eyers 7/28/2014) - PLID 56981 - Got rid of the check on IsPatientTab
		dlgCat.m_strWhereClause = GetAllowedCategoryClause("ID");
		dlgCat.AddColumn("ID", "ID", FALSE, FALSE);
		dlgCat.AddColumn("Description", "Name", TRUE, FALSE);
		if (IDOK == dlgCat.DoModal()) {
			ASSERT(dlgCat.m_arSelectedValues.GetSize() == 2);
			nSelectedCatID = dlgCat.m_arSelectedValues[0];
		} else {
			return;
		}

		if (nSelectedCatID >= 0) {
			//now get the array IDs that are highlighted in the other list
			// (r.galicki 2008-09-09 11:38) - PLID 31280 - Converted DocList to DL2
			IRowSettingsPtr pRow = m_pDocList->GetFirstSelRow();

			CArray<LPDISPATCH, LPDISPATCH> arrGroupRows; //PLID 31280 - Keep track of parent rows
			while (pRow) {
				// (r.galicki 2008-09-18 15:39) - PLID 31242 - Only associate category if it is unassociated or the "root" of a group
				if(pRow->GetParentRow() == NULL) {
					pRow->PutValue(eCategory, _variant_t((LPCTSTR)AsString(nSelectedCatID)));
				}
				else { // (r.galicki 2008-09-23 10:19) - PLID 31280 - Alert user when selection is part of a group
					IRowSettingsPtr pParent = pRow->GetParentRow();
					if(pParent) {
						if(!pParent->GetSelected()) { //No need to prompt if the parent is selected
							BOOL bParentExists = FALSE;

							//if this parent row has been prompted before, just skip
							for(int i = 0; !bParentExists && i < arrGroupRows.GetSize(); i++) {
								if (pParent == arrGroupRows[i]) {
									bParentExists = TRUE;
								}
							}
							if(!bParentExists) {
								arrGroupRows.Add(pParent);
								BOOL bSameCat = pParent->GetValue(eCategory) == _variant_t((LPCTSTR)AsString(nSelectedCatID));

								if(!bSameCat) { //No need to prompt if the category matches
									CString strMessage = FormatString("The document '%s' is already in the group '%s'.\r\n\r\n"
											"Do you want to associate the entire group with the category '%s'?",
									VarString(pRow->GetValue(eName), ""), //"page" 
									VarString(pParent->GetValue(eName), ""), //"group"
									AsString(dlgCat.m_arSelectedValues[1])); //new category
								
									//prompt to associate category with documents
									if(IDYES == MessageBox(strMessage,NULL, MB_YESNO|MB_ICONQUESTION)) {
										pParent->PutValue(eCategory, _variant_t((LPCTSTR)AsString(nSelectedCatID)));
									}
								}
							}
						}
					}
				}
				pRow = pRow->GetNextSelRow();
			}
			/*long p = m_pDocList->GetFirstSelEnum();

			LPDISPATCH pDisp = NULL;

			while (p)
			{	
				m_pDocList->GetNextSelEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);

				pRow->PutValue(eCategory, _variant_t((LPCTSTR)AsString(nSelectedCatID)));
				
				pDisp->Release();
			}*/
		}
	} NxCatchAll("Error in CScanMultiDocDlg::OnBtnAssociateCategory");
}

void CScanMultiDocDlg::CleanupFiles()
{
	// (r.galicki 2008-09-09 14:16) - PLID 31280 - Converted DocList to DL2
	try {
		IRowSettingsPtr pRow = m_pDocList->FindAbsoluteFirstRow(FALSE);
		while(pRow) {
			if(pRow->GetValue(ePath).vt == VT_BSTR) {
				CString strPath = VarString(pRow->GetValue(ePath), "");

				if (!strPath.IsEmpty()) {
					GetMainFrame()->DeleteFileWhenPossible(strPath);
				}
			}

			IRowSettingsPtr pOldRow = pRow;
			pRow = pOldRow->GetFirstChildRow();
			if (pRow == NULL) {
				pRow = pOldRow->GetNextRow();

				if (pRow == NULL) {
					pRow = pOldRow->GetParentRow();
					if (pRow) {
						pRow = pRow->GetNextRow();
					}
				}
			}
		}
		/*long p = m_pDocList->GetFirstRowEnum();
		while (p) {			
			LPDISPATCH lpDisp;
			m_pDocList->GetNextRowEnum(&p, &lpDisp);
			if (lpDisp) {
				IRowSettingsPtr pRow(lpDisp);
				lpDisp->Release();

				CString strPath = VarString(pRow->GetValue(ePath), "");

				if (!strPath.IsEmpty()) {
					GetMainFrame()->DeleteFileWhenPossible(strPath);
				}
			}
		}*/
	} NxCatchAll("Error cleaning up temp files");
}

void CScanMultiDocDlg::OnSelSetDocumentList(LPDISPATCH lpSel) 
{
	try {
		IRowSettingsPtr pRow(lpSel);

		if (pRow) {
			if(pRow->GetValue(ePath).vt == VT_BSTR) {
				CString strPath = VarString(pRow->GetValue(ePath), "");
				PreviewImage(strPath);
			}
		}
	} NxCatchAll("CScanMultiDocDlg::OnSelSetDocumentList");
}

// (r.galicki 2008-09-10 09:06) - PLID 31242 - Group set of documents into a single document
void CScanMultiDocDlg::OnBtnGroup() 
{
	try {
		//make sure that at least two documents are highlighted
		IRowSettingsPtr pRow = m_pDocList->GetFirstSelRow();

		CMap<long, long, long, long> mIDCounts;
		CMap<long, long, long, long> mCatCounts;
		CMap<COleDateTime, COleDateTime, long, long> mDateCounts;

		//establish valid grouping, error report
		CString strErrors;
		BOOL bFailed = FALSE;
		long nCountOfValidRows = 0;
		while(pRow) {
			if (pRow->GetFirstChildRow() != NULL) { //invalid if a top-level document
				// we can safely ignore this
				strErrors += FormatString("The document %s is already a group!\r\n", VarString(pRow->GetValue(eName), ""));
			}
			else {
				nCountOfValidRows++;
				// need to inherit patient IDs and etc from parent row
				IRowSettingsPtr pInfoRow;
				if (pRow->GetParentRow()) {
					pInfoRow = pRow->GetParentRow();
				} else {
					pInfoRow = pRow;
				}

				_variant_t varPersonID = pInfoRow->GetValue(ePersonID);
				long nPersonID = -1;
				if(varPersonID.vt != VT_NULL) {
					nPersonID = VarLong(varPersonID);
				}
				if (nPersonID != -1) {
					long nCount;
					if (mIDCounts.Lookup(nPersonID, nCount)) { // already exists in map
						mIDCounts[nPersonID] = nCount + 1;
					} else {	// new item
						mIDCounts[nPersonID] = 1;
					}
				}

				_variant_t varSelCategoryID = pInfoRow->GetValue(eCategory);
				long nSelCategoryID = LONG_MIN;
				if (varSelCategoryID.vt == VT_I4) {
					nSelCategoryID = VarLong(varSelCategoryID);
				} else if (varSelCategoryID.vt == VT_BSTR) {
					nSelCategoryID = atoi(VarString(varSelCategoryID));
				}

				if (nSelCategoryID != LONG_MIN) {
					long nCount;
					if (mCatCounts.Lookup(nSelCategoryID, nCount)) { // already exists in map
						mCatCounts[nSelCategoryID] = nCount + 1;
					} else {	// new item
						mCatCounts[nSelCategoryID] = 1;
					}					
				}

				_variant_t varServiceDate = pInfoRow->GetValue(eServiceDate);
				COleDateTime dtServiceDate;
				
				if(varServiceDate.vt == VT_DATE) {
					dtServiceDate = VarDateTime(varServiceDate);
				}

				if(0 != dtServiceDate) {
					long nCount;
					if (mDateCounts.Lookup(dtServiceDate, nCount)) { // already exists in map
						mDateCounts[dtServiceDate] = nCount + 1;
					} else {	// new item
						mDateCounts[dtServiceDate] = 1;
					}
				}
			}
			pRow = pRow->GetNextSelRow();
		}

		if (nCountOfValidRows < 1) {
			strErrors += "\r\nPlease select at least one document to group.";
			bFailed = TRUE;
		}

		if (!strErrors.IsEmpty()) {
			MessageBox(FormatString("A new group could not be made:\r\n\r\n%s", strErrors), NULL, MB_OK|MB_ICONEXCLAMATION);
			return;
		}

		//determine if map is homogenous (all selected documents attached to the same patient)
		//if not, obtain the ID for the most frequent patient selected
		long nDistinctPatientCount = 0;
		POSITION idx = mIDCounts.GetStartPosition();
		long nTempID, nTempValue;
		long nSelectedPersonID = -1, nMaxValue = -1;
		while(idx) {
			mIDCounts.GetNextAssoc(idx, nTempID, nTempValue);
			nDistinctPatientCount++;

			if(nTempValue >= nMaxValue) {
				nSelectedPersonID = nTempID;
				nMaxValue = nTempValue;
			}
		}
		
		// also get the most common category
		long nDistinctCatCount = 0;
		idx = mCatCounts.GetStartPosition();
		long nSelCategoryID = -1;
		CArray<long, long> arrMostCommonIDs;
		nMaxValue = -1;
		while (idx) {
			mCatCounts.GetNextAssoc(idx, nTempID, nTempValue);
			nDistinctCatCount++;

			if(nTempValue > nMaxValue) {
				arrMostCommonIDs.RemoveAll();
				arrMostCommonIDs.Add(nTempID);
				nSelCategoryID = nTempID;
				nMaxValue = nTempValue;
			}
			else if(nTempValue == nMaxValue) {
				arrMostCommonIDs.Add(nTempID);
			}
		}
		_variant_t varCategoryID;
		// (r.galicki 2008-10-09 09:24) - PLID 31280 - In the event of mismatched categories of equal frequencies, prompt for a new category.
		//											 - If only one most common exists, prompt to associate with that category...allow cancel
		if (nDistinctCatCount > 1) {	
			if(arrMostCommonIDs.GetSize() > 1) {
				CSelectDlg dlgCat(this);
				dlgCat.m_strTitle = "Select a category";
				dlgCat.m_strCaption = "The selected documents do not have matching categories.  Select a category to associate with the selected documents.";
				dlgCat.m_strFromClause = "NoteCatsF";
				//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
				// (b.eyers 7/28/2014) - PLID 56981 - Got rid of the check on IsPatientTab
				dlgCat.m_strWhereClause = GetAllowedCategoryClause("ID");
				dlgCat.AddColumn("ID", "ID", FALSE, FALSE);
				dlgCat.AddColumn("Description", "Name", TRUE, FALSE);
				if (IDOK == dlgCat.DoModal()) {
					ASSERT(dlgCat.m_arSelectedValues.GetSize() == 2);
					nSelCategoryID = dlgCat.m_arSelectedValues[0];
				} else {
					return;
				}
			}
			else {
				if(IDCANCEL == MessageBox("The selected documents do not have matching categories. The most common category will be used instead.", NULL, MB_OKCANCEL|MB_ICONINFORMATION)) {
					return;
				}
			}
		}
		varCategoryID = _variant_t(nSelCategoryID);

		

		// also get the most common service date
		long nDistinctDateCount = 0;
		idx = mDateCounts.GetStartPosition();
		COleDateTime dtTempDate, dtSelDate;
		nMaxValue = -1;
		while (idx) {
			mDateCounts.GetNextAssoc(idx, dtTempDate, nTempValue);
			nDistinctDateCount++;

			if(nTempValue > nMaxValue) {
				dtSelDate = dtTempDate;
				nMaxValue = nTempValue;
			}
		}
		_variant_t varServiceDate = _variant_t(AsDateNoTime(dtSelDate), VT_DATE);
		
		if (nDistinctDateCount > 1) {
			CString strMessage;
			strMessage.Format("The selected documents do not have matching service dates. The most common date, %s, will be used will be used instead.", AsString(varServiceDate));
			// (r.galicki 2008-10-09 10:21) - PLID 31280 - Show user which date is being set.  Allow canceling of grouping.
			if(IDCANCEL == MessageBox(strMessage, NULL, MB_OKCANCEL|MB_ICONINFORMATION)) {
				return;
			}
		}
		


		if(nDistinctPatientCount > 1) { //non-homogenous grouping - prompt to attach group to a single patient
			CSelectDlg dlg(this);
			dlg.m_strTitle = "Select a Patient";
			dlg.m_strCaption = "These documents are associated with different patients. To create a new group, they must all be associated with the same patient.";
			dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
			dlg.m_strWhereClause = "PersonID >= 0 AND Archived = 0";
			
			dlg.AddColumn("Last","Last", TRUE, FALSE);
			dlg.AddColumn("First","First", TRUE, FALSE);
			dlg.AddColumn("Middle","Middle", TRUE, FALSE);
			dlg.AddColumn("PatientsT.UserDefinedID", "ID", TRUE, FALSE);
			dlg.AddColumn("PersonID", "PersonID", FALSE, FALSE);

			//pre select ID based on the 
			dlg.SetPreSelectedID(4, nSelectedPersonID);

			if (IDOK == dlg.DoModal()) {
				nSelectedPersonID = dlg.m_arSelectedValues[4];
			} else {
				return;
			}
		}/* else if(-1 == nSelectedPersonID) { //attempting to make a group of <No Patient>s, prompt to attach group to a single patient
			if(IDYES == MessageBox("Selected documents must be associated with a single patient before grouping.\r\n"
				"Do you wish to associate selected documents with a specific patient now?\r\n", NULL, MB_YESNO|MB_ICONEXCLAMATION)) {
			
				CSelectDlg dlg(this);
				dlg.m_strTitle = "Select a Patient";
				dlg.m_strCaption = "Choose a patient to group all selected documents by";
				dlg.m_strFromClause = "PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID";
				dlg.m_strWhereClause = "PersonID >= 0 AND Archived = 0";
				
				dlg.AddColumn("Last","Last", TRUE, FALSE);
				dlg.AddColumn("First","First", TRUE, FALSE);
				dlg.AddColumn("Middle","Middle", TRUE, FALSE);
				dlg.AddColumn("PatientsT.UserDefinedID", "ID", TRUE, FALSE);
				dlg.AddColumn("PersonID", "PersonID", FALSE, FALSE);

				if (IDOK == dlg.DoModal()) {
					nSelectedPersonID = dlg.m_arSelectedValues[4];
				} 
				else {
					strErrors += FormatString("Documents not associated with a patient.");
				}
			}
			else {
				strErrors += FormatString("Documents not associated with a patient.");
			}
		}*/

		// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2
		NXDATALIST2Lib::IRowSettingsPtr pSelPatientRow = m_pPatList->FindByColumn(eplcPersonID, (long)nSelectedPersonID, NULL, VARIANT_TRUE);
		
		_variant_t varPersonID, varLast, varFirst, varMiddle, varUserDefinedID;
		if (pSelPatientRow) {
			varPersonID = pSelPatientRow->GetValue(eplcPersonID);
			varLast = pSelPatientRow->GetValue(eplcLast);
			varFirst = pSelPatientRow->GetValue(eplcFirst);
			varMiddle = pSelPatientRow->GetValue(eplcMiddle);
			varUserDefinedID = pSelPatientRow->GetValue(eplcUserDefinedID);
		} else {
			varPersonID = g_cvarNull;
			varLast = g_cvarNull;
			varFirst = g_cvarNull;
			varMiddle = g_cvarNull;
			varUserDefinedID = g_cvarNull;
		}
		//if still valid, make new group
		IRowSettingsPtr pParentRow = m_pDocList->GetNewRow();

		pParentRow->PutValue(eScanID, (long)-1);
		pParentRow->PutValue(ePath, g_cvarNull);
		pParentRow->PutValue(eName, (LPCTSTR)GetDocumentName());
		pParentRow->PutValue(ePersonID, varPersonID);
		pParentRow->PutValue(eCategory, varCategoryID);
		pParentRow->PutValue(eLast, varLast);
		pParentRow->PutValue(eFirst, varFirst);
		pParentRow->PutValue(eMiddle, varMiddle);
		pParentRow->PutValue(eUserDefinedID, varUserDefinedID);
		pParentRow->PutValue(eIsPhoto, g_cvarNull);
		pParentRow->PutValue(eServiceDate, varServiceDate);
		
		// Add after the last group, or at the top if we are the first.
		{
			IRowSettingsPtr pInsertRow = m_pDocList->GetFirstRow();
			while (pInsertRow) {
				if (VarLong(pInsertRow->GetValue(eScanID), -2) != -1) {
					break;
				}
				pInsertRow = pInsertRow->GetNextRow();
			}

			if (pInsertRow) {
				m_pDocList->AddRowBefore(pParentRow, pInsertRow);
			} else { // we reached the end, all are groups, so just put at the end
				m_pDocList->AddRowAtEnd(pParentRow, NULL);
			}
		}

		//add selected to group
		//pRow = m_pDocList->GetFirstSelRow();
		// this the preferred way to handle selections, but we want to ensure it is in the order
		// of the datalist, not the order it was selected in, so we'll scan through ourselves.
		pRow = m_pDocList->GetFirstRow();
		while(pRow) {
			BOOL bSelected = FALSE;
			if (pRow->GetSelected() == VARIANT_TRUE) {				
				bSelected = TRUE;
				// and these should be blank now, probably others as well
				pRow->PutValue(ePersonID, g_cvarNull);
				pRow->PutValue(eCategory, g_cvarNull);
				pRow->PutValue(eLast, g_cvarNull);
				pRow->PutValue(eFirst, g_cvarNull);
				pRow->PutValue(eMiddle, g_cvarNull);
				pRow->PutValue(eUserDefinedID, g_cvarNull);
				pRow->PutValue(eIsPhoto, g_cvarNull);
				pRow->PutValue(eServiceDate, g_cvarNull);

				m_pDocList->AddRowAtEnd(pRow, pParentRow);
				EnsureRowColor(pRow);
			}
			
			IRowSettingsPtr pOldRow = pRow;
			pRow = pOldRow->GetFirstChildRow();
			if (pRow == NULL) {
				pRow = pOldRow->GetNextRow();

				if (pRow == NULL) {
					pRow = pOldRow->GetParentRow();
					if (pRow) {
						pRow = pRow->GetNextRow();
					}
				}
			}
			
			/*if (bSelected) {
				m_pDocList->RemoveRow(pOldRow);
			}*/
		}

		pRow = m_pDocList->GetFirstSelRow();
		while(pRow) {
			IRowSettingsPtr pOldRow = pRow;
			pRow = pOldRow->GetNextSelRow();

			m_pDocList->RemoveRow(pOldRow);
		}

		pParentRow->PutExpanded(VARIANT_TRUE);
		m_pDocList->PutCurSel(pParentRow);
		m_pDocList->EnsureRowInView(pParentRow);
		EnsureRowColor(pParentRow);

		RemoveEmptyDocumentRows();

	}NxCatchAll("Error Grouping Documents");
}

void CScanMultiDocDlg::RemoveEmptyDocumentRows()
{
	IRowSettingsPtr p = m_pDocList->GetFirstRow();

	while (p) {
		IRowSettingsPtr pEmptyRow;
		long nScanID = VarLong(p->GetValue(eScanID), -2);
		if (nScanID == -1) {
			if (p->GetFirstChildRow() == NULL) {
				pEmptyRow = p;
			}
		}

		p = p->GetNextRow();

		if (pEmptyRow) {
			m_pDocList->RemoveRow(pEmptyRow);
		}
	}
}

// (r.galicki 2008-09-17 14:27) - PLID 31242 - Added Right click functionality
void CScanMultiDocDlg::OnRButtonDownDocumentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	IRowSettingsPtr pSelRow(lpRow), pRow, pOldRow;

	if(pSelRow) {
		enum EMenuOptions {
			eAddToGroup = 1,
			eRemoveAll,
			eRemove,
		};

		pRow = m_pDocList->GetFirstSelRow();
		if(pRow) {
			CMenu mPopup;
			mPopup.CreatePopupMenu();

			mPopup.InsertMenu(1, MF_BYPOSITION, eAddToGroup, "Group Selected As Single Document");
			mPopup.InsertMenu(2, MF_BYPOSITION|MF_SEPARATOR);
			if ((pRow->GetNextSelRow() != NULL)) {
				mPopup.InsertMenu(3, MF_BYPOSITION, eRemoveAll, "Delete All Selected");
			} else {
				CString strDel = FormatString("Delete %s", VarString(pSelRow->GetValue(eName), ""));
				mPopup.InsertMenu(4, MF_BYPOSITION, eRemove, strDel);
			}
			
			CPoint pt;
			GetMessagePos(&pt);
			long nResult = mPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

			switch(nResult) {
				case eAddToGroup:
					OnBtnGroup();
					break;
				case eRemove:
				case eRemoveAll:
					{
						while(pRow) {
							pOldRow = pRow;
							if(pRow->GetValue(ePath).vt == VT_BSTR) {
								CString strPath = VarString(pRow->GetValue(ePath), "");
								if (!strPath.IsEmpty()) {
									DeleteFileWhenPossible(strPath);
								}
							}

							IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
							while (pChildRow) {		
								IRowSettingsPtr pOldChildRow = pChildRow;
								if(pChildRow->GetValue(ePath).vt == VT_BSTR) {
									CString strPath = VarString(pChildRow->GetValue(ePath), "");
									if (!strPath.IsEmpty()) {
										DeleteFileWhenPossible(strPath);
									}
								}
								pChildRow = pChildRow->GetNextSelRow();
								m_pDocList->RemoveRow(pOldChildRow);
							}

							pRow = pRow->GetNextSelRow();
							if(pOldRow) {
								m_pDocList->RemoveRow(pOldRow);
							}
						}

						RemoveEmptyDocumentRows();
					}
					break;
			}
		}
	}
}

// (r.galicki 2008-09-11 11:17) - PLID 31242 - Prevent editing of "pages"
void CScanMultiDocDlg::OnEditingStartingDocumentList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	IRowSettingsPtr pRow(lpRow);

	if(pRow->GetParentRow() != NULL) {
		*pbContinue = FALSE;
	}
}

// (r.galicki 2008-09-17 14:36) - PLID 31280 - Drag and drop funcitonality
void CScanMultiDocDlg::OnDragBeginDocumentList(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags) 
{
	IRowSettingsPtr pRow(lpRow);

	if (pRow) {
		if (pRow->GetParentRow() == NULL) {
			*pbShowDrag = FALSE;
		} else {
			*pbShowDrag = TRUE;
		}
	}	
}

void CScanMultiDocDlg::OnDragEndDocumentList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags) 
{
	if (nFlags & defCanceled)
		return;

	IRowSettingsPtr pFromRow(lpFromRow), pToRow(lpRow);

	IRowSettingsPtr pNewRow;

	if (pFromRow == NULL) return;

	BOOL bFromGroup = pFromRow->GetParentRow() != NULL;
	BOOL bFromGroupRow = VarLong(pFromRow->GetValue(eScanID), -2) == -1;

	if (bFromGroupRow) {
		return;
	}

	BOOL bToGroup = FALSE;
	BOOL bToGroupRow = FALSE;

	if (pToRow != NULL) {
		bToGroup = (pToRow->GetFirstChildRow() != NULL) || (pToRow->GetParentRow() != NULL);
		bToGroupRow = VarLong(pToRow->GetValue(eScanID), -2) == -1;
	}

	if (bToGroup) {
		// just move the row
		pFromRow->PutValue(ePersonID, g_cvarNull);
		pFromRow->PutValue(eCategory, g_cvarNull);
		pFromRow->PutValue(eLast, g_cvarNull);
		pFromRow->PutValue(eFirst, g_cvarNull);
		pFromRow->PutValue(eMiddle, g_cvarNull);
		pFromRow->PutValue(eUserDefinedID, g_cvarNull);
		pFromRow->PutValue(eIsPhoto, g_cvarNull);
		pFromRow->PutValue(eServiceDate, g_cvarNull);

		if (bToGroupRow) {
			pNewRow = m_pDocList->AddRowAtEnd(pFromRow, pToRow);
		} else {
			if (m_pDocList->IsRowEarlierInList(pFromRow, pToRow) == VARIANT_TRUE) {
				if (pToRow->GetNextRow()) {
					pNewRow = m_pDocList->AddRowBefore(pFromRow, pToRow->GetNextRow());
				} else {
					pNewRow = m_pDocList->AddRowAtEnd(pFromRow, pToRow->GetParentRow());
				}
			} else {
				pNewRow = m_pDocList->AddRowBefore(pFromRow, pToRow);
			}
		}

		m_pDocList->RemoveRow(pFromRow);		
	} else if (bFromGroup) {
		// unassociate this row from the group
		pFromRow->PutValue(ePersonID, pFromRow->GetParentRow()->GetValue(ePersonID));
		pFromRow->PutValue(eCategory, pFromRow->GetParentRow()->GetValue(eCategory));
		pFromRow->PutValue(eLast, pFromRow->GetParentRow()->GetValue(eLast));
		pFromRow->PutValue(eFirst, pFromRow->GetParentRow()->GetValue(eFirst));
		pFromRow->PutValue(eMiddle, pFromRow->GetParentRow()->GetValue(eMiddle));
		pFromRow->PutValue(eUserDefinedID, pFromRow->GetParentRow()->GetValue(eUserDefinedID));

		// (a.wilson 2012-5-8) PLID 48223 - check whether or not the default is on and check for image format.
		// (r.galicki 2008-09-03 17:27) - PLID 31242 - set IsPhoto default and service date (for all scanned documents)
		if (m_bDefaultIsPhoto) {
			pFromRow->PutValue(eIsPhoto, (IsImageFile(VarString(pFromRow->GetValue(ePath), "")) ? g_cvarTrue : g_cvarFalse)); 
		} else {
			pFromRow->PutValue(eIsPhoto, g_cvarFalse);
		}
		pFromRow->PutValue(eServiceDate, pFromRow->GetParentRow()->GetValue(eServiceDate));

		if(pToRow != NULL) {
			if (m_pDocList->IsRowEarlierInList(pFromRow, pToRow) == VARIANT_TRUE) {
				if (pToRow->GetNextRow()) {
					pNewRow = m_pDocList->AddRowBefore(pFromRow, pToRow->GetNextRow());
				} else {
					pNewRow = m_pDocList->AddRowAtEnd(pFromRow, NULL);
				}
			} else {
				pNewRow = m_pDocList->AddRowBefore(pFromRow, pToRow);
			}
		}
		else {
			pNewRow = m_pDocList->AddRowAtEnd(pFromRow, NULL);
		}
		m_pDocList->RemoveRow(pFromRow);
	} else {
		// just moving around rows
		if(pToRow != NULL) {
			if (m_pDocList->IsRowEarlierInList(pFromRow, pToRow) == VARIANT_TRUE) {
				if (pToRow->GetNextRow()) {
					pNewRow = m_pDocList->AddRowBefore(pFromRow, pToRow->GetNextRow());
				} else {
					pNewRow = m_pDocList->AddRowAtEnd(pFromRow, NULL);
				}
			} else {
				pNewRow = m_pDocList->AddRowBefore(pFromRow, pToRow);
			}
			m_pDocList->RemoveRow(pFromRow);
		}
	}

	RemoveEmptyDocumentRows();

	if (pNewRow) {
		m_pDocList->PutCurSel(pNewRow);
		m_pDocList->EnsureRowInView(pNewRow);
		EnsureRowColor(pNewRow);
	}
}

// (r.galicki 2008-09-17 14:29) - PLID 31242 - Row color helper function
void CScanMultiDocDlg::EnsureRowColor(IRowSettingsPtr pRow) {

	if(pRow == NULL) {
		return;
	}

	_variant_t varPatient;

	if(pRow->GetFirstChildRow() != NULL) { //group
		IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
		while((pChildRow != NULL) && (pChildRow->GetParentRow() == pRow)) {
			EnsureRowColor(pChildRow);
			pChildRow = pChildRow->GetNextRow();
		}
		varPatient = pRow->GetValue(ePersonID);
	}
	else if(pRow->GetParentRow() != NULL) { //child
		varPatient = pRow->GetParentRow()->GetValue(ePersonID);
	}
	else { //unassociated
		varPatient = pRow->GetValue(ePersonID);
	}

	if(varPatient.vt != VT_I4) {
		pRow->PutForeColor(RGB(220, 0, 0));
		return;
	}

	long nPersonID = VarLong(varPatient, -1);
	if(-1 != nPersonID)  {
		pRow->PutForeColor(RGB(0, 0, 0));
	}
	else {
		pRow->PutForeColor(RGB(220, 0, 0));
	}
}

// (r.galicki 2008-10-08 17:52) - PLID 31242 - Check for valid edit
void CScanMultiDocDlg::OnEditingFinishingDocumentList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	switch(nCol) {
		case eServiceDate:
			COleDateTime dtNewDate(*pvarNewValue);
			if(COleDateTime::invalid == dtNewDate.GetStatus() || dtNewDate <= COleDateTime(1899,12,31,0,0,0)) {
				*pbCommit = false;
			}
			break;
	}
}

//(e.lally 2010-01-13) PLID 32201 - Save option to always save scanned files as PDFs
void CScanMultiDocDlg::OnBtnAlwaysSaveAsPDF()
{
	try{
		if(m_nxbAlwaysSavePDF.GetCheck() == BST_CHECKED) {
			SetRemotePropertyInt("ScanMultiDocAlwaysSavePDF", 1, 0, GetCurrentUserName());
		}
		else {
			SetRemotePropertyInt("ScanMultiDocAlwaysSavePDF", 0, 0, GetCurrentUserName());
		}
	}NxCatchAll(__FUNCTION__)
}

// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2, this is the dl1 function
/*
void CScanMultiDocDlg::OnSelSetPatientScanList(long nRow) 
{
	// (r.galicki 2008-09-08 17:52) - PLID 31242 - Update patient ID member with new selection.
	NXDATALISTLib::IRowSettingsPtr pPatRow = m_pPatientList->GetRow(nRow);
	if(pPatRow) {
		m_nPatientID = pPatRow->GetValue(eplcPersonID);
	}
}
*/

// (d.thompson 2010-03-23) - PLID 37839 - Convert patient list to dl2, this is the dl2 version.  I also converted to OnSelChosen
//	instead of SelSet.
void CScanMultiDocDlg::SelChosenPatientScanList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_nPatientID = pRow->GetValue(eplcPersonID);
		}
		//d.thompson 2010-03-23 NOTE:  The existing behavior was to NOT change the patient if no row
		//	was selected.  I am leaving that behavior in place, as my item is just to convert
		//	the type of list, not how it behaves.
	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-08-23 13:41) - PLID 52094
void CScanMultiDocDlg::OnEnKillfocusEditDefaultServiceDate()
{
	try {
		COleDateTime dtMin;
		dtMin.SetDate(1899,12,31);
		if (m_nxdService->GetStatus() == 1) {
			COleDateTime dtService = m_nxdService->GetDateTime();
			if (dtService.GetStatus() == COleDateTime::valid && dtService > dtMin) {
				m_dtService = dtService;
			}	
			else {
				MsgBox("The Service Date you entered is invalid and will be reset");
				if (m_dtService.GetStatus() == COleDateTime::valid && m_dtService > dtMin) {
					m_nxdService->SetDateTime(m_dtService);
				}
				else {
					m_nxdService->Clear();
				}
			}

		}
		else if (m_nxdService->GetStatus() == 3) {
			//they cleared it out
			m_dtService.SetDate(1800,12,31);
			m_dtService.SetStatus(COleDateTime::invalid);
		}
		else {
			//its invalid
			MsgBox("The Service Date you entered is invalid and will be reset");
			if (m_dtService.GetStatus() == COleDateTime::valid && m_dtService > dtMin) {
				m_nxdService->SetDateTime(m_dtService);
			}
			else {
				m_nxdService->Clear();
			}
		}
	}NxCatchAll(__FUNCTION__);
}
// (a.wilson 2013-05-07 13:40) - PLID 52960 - save the current prefix as the default.
void CScanMultiDocDlg::OnBnClickedDefaultPrefixCheck()
{
	try {
		if (m_chkDefaultPrefix.GetCheck() == BST_CHECKED) {	//save the new default and assign to the default member variable.
			CString strCurrent;
			m_editDocPrefix.GetWindowText(strCurrent);
			SetRemotePropertyText("ScanMultiDocPrefix", strCurrent, 0, "<None>");
			m_strDefaultPrefix = strCurrent;
		} else {	//set the default as blank if they unchecked the current default setting.
			SetRemotePropertyText("ScanMultiDocPrefix", "", 0, "<None>");
			m_strDefaultPrefix = "";
		}
	} NxCatchAll(__FUNCTION__);
}
