// InvFramesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InventoryRc.h"
#include "InvFramesDlg.h"
#include "FileUtils.h"
#include "InternationalUtils.h"
#include "FramesData.h"
#include "InvUtils.h"
#include "InvNew.h"
#include "FramesOPtionsdlg.h"
#include "AuditTrail.h"
#include "ShowProgressFeedbackDlg.h"
#include "ShowConnectingFeedbackDlg.h"
#include "MarkupFormulaEditDlg.h"
#include "SingleSelectDlg.h"
#include "NxExpression.h"
#include "InvEditDlg.h"

//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "MarkUpFormula.h" //r.wilson 3/9/2012 PLID 46664
#include "NxSystemUtilitiesLib\BulkImportUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

using namespace NXDATALIST2Lib;

// CInvFramesDlg dialog
// (z.manning 2010-06-17 15:46) - PLID 39222 - Created

#define LINKED_TO_PRODUCT_COLOR GetNxColor(GNC_INVENTORY, -1)

IMPLEMENT_DYNAMIC(CInvFramesDlg, CNxDialog)

CInvFramesDlg::CInvFramesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvFramesDlg::IDD, pParent)
	, m_strFilterAllText(" { All } ")
{
	m_bDatalistFiltered = FALSE;
}

CInvFramesDlg::~CInvFramesDlg()
{
}

void CInvFramesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FRAMES_IMPORT_BUTTON, m_btnImportFramesData);
	DDX_Control(pDX, IDC_FRAMES_CREATE_SELECTED_PRODUCTS, m_btnCreateProducts);
	DDX_Control(pDX, IDC_FRAMES_UPDATE_EXISTING_PRODUCTS, m_btnUpdateExistingProducts);
	DDX_Control(pDX, IDC_FRAMES_OPTIONS, m_btnOptions);
	DDX_Control(pDX, IDC_FRAMES_LIST_COUNT_LABEL, m_nxstaticFramesCount);
	DDX_Control(pDX, IDC_FRAMES_VERSION_DATE_LABEL, m_nxstaticFramesDate);
	DDX_Control(pDX, IDC_FRAMES_RELOAD, m_btnReload);
	DDX_Control(pDX, IDC_FRAMES_EDIT_MARKUPS, m_btnEditMarkups);
	DDX_Control(pDX, IDC_FRAMES_APPLY_MARKUP, m_btnApplyMarkups);
}


BEGIN_MESSAGE_MAP(CInvFramesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_FRAMES_IMPORT_BUTTON, &CInvFramesDlg::OnBnClickedFramesImportButton)
	ON_BN_CLICKED(IDC_FRAMES_SELECT_ALL, &CInvFramesDlg::OnBnClickedFramesSelectAll)
	ON_BN_CLICKED(IDC_FRAMES_SELECT_NONE, &CInvFramesDlg::OnBnClickedFramesSelectNone)
	ON_BN_CLICKED(IDC_FRAMES_CREATE_SELECTED_PRODUCTS, &CInvFramesDlg::OnBnClickedFramesCreateSelectedProducts)
	ON_BN_CLICKED(IDC_FRAMES_UPDATE_EXISTING_PRODUCTS, &CInvFramesDlg::OnBnClickedFramesUpdateExistingProducts)
	ON_BN_CLICKED(IDC_FRAMES_OPTIONS, &CInvFramesDlg::OnBnClickedFramesOptions)
	ON_BN_CLICKED(IDC_FRAMES_RELOAD, &CInvFramesDlg::OnBnClickedFramesReload)
	ON_BN_CLICKED(IDC_PRICE_CHANGE_REPORT, &CInvFramesDlg::OnBnClickedPriceChangeReport)
	ON_BN_CLICKED(IDC_FRAMES_EDIT_MARKUPS, &CInvFramesDlg::OnBnClickedFramesEditMarkups)
	ON_BN_CLICKED(IDC_FRAMES_APPLY_MARKUP, &CInvFramesDlg::OnBnClickedFramesApplyMarkup)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInvFramesDlg, CNxDialog)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_MANUFACTURER, 16, CInvFramesDlg::SelChosenFramesComboManufacturer, VTS_DISPATCH)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_COLLECTION, 16, CInvFramesDlg::SelChosenFramesComboCollection, VTS_DISPATCH)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_STYLE, 16, CInvFramesDlg::SelChosenFramesComboStyle, VTS_DISPATCH)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_MANUFACTURER, 18, CInvFramesDlg::RequeryFinishedFramesComboManufacturer, VTS_I2)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_BRAND, 18, CInvFramesDlg::RequeryFinishedFramesComboBrand, VTS_I2)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_COLLECTION, 18, CInvFramesDlg::RequeryFinishedFramesComboCollection, VTS_I2)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_GROUP, 18, CInvFramesDlg::RequeryFinishedFramesComboGroup, VTS_I2)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_STYLE, 18, CInvFramesDlg::RequeryFinishedFramesComboStyle, VTS_I2)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_GROUP, 16, CInvFramesDlg::SelChosenFramesComboGroup, VTS_DISPATCH)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_COMBO_BRAND, 16, CInvFramesDlg::SelChosenFramesComboBrand, VTS_DISPATCH)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_CATALOG_LIST, 18, CInvFramesDlg::RequeryFinishedFramesCatalogList, VTS_I2)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_CATALOG_LIST, 6, CInvFramesDlg::RButtonDownFramesCatalogList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_CATALOG_LIST, 9, CInvFramesDlg::EditingFinishingFramesCatalogList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
ON_EVENT(CInvFramesDlg, IDC_FRAMES_CATALOG_LIST, 10, CInvFramesDlg::EditingFinishedFramesCatalogList,  VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()


// CInvFramesDlg message handlers

BOOL CInvFramesDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CInvFramesDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ( \r\n"
			"	Name = 'DefaultProductOrderingUser' \r\n" // (z.manning 2010-06-23 12:38) - PLID 39311
			"	OR Name = 'InvFrameOpBillable' "
			"	OR Name = 'InvFrameOpTaxable1' "
			"	OR Name = 'InvFrameOpTaxable2' "
			"	OR Name = 'InvFrameOpTracking' "
			"	OR Name = 'InvFrameOpCategory' "	
			"	OR Name = 'UpdateSurgeryPrices' "
			"	OR Name = 'UpdateSurgeryPriceWhen' "
			"	OR Name = 'UpdatePreferenceCardPrices' "
			"	OR Name = 'UpdatePreferenceCardPriceWhen' "
			") \r\n"
			, _Q(GetCurrentUserName()));
		g_propManager.CachePropertiesInBulk("CInvFramesDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND ( \r\n"
			"	Name = 'LastFramesDataImportPath' \r\n"
			") \r\n"
			, _Q(GetCurrentUserName()));
		g_propManager.CachePropertiesInBulk("CInvFramesDlg", propDateTime,
			"(Username = '<None>' OR Username = '%s') AND ( \r\n"
			"	Name = 'FramesImportVersionDate' \r\n"
			") \r\n"
			, _Q(GetCurrentUserName()));

		m_btnImportFramesData.AutoSet(NXB_MODIFY);
		m_btnUpdateExistingProducts.AutoSet(NXB_MODIFY);
		m_btnCreateProducts.AutoSet(NXB_NEW);
		// (j.gruber 2010-06-23 14:41) - PLID 39314
		m_btnOptions.AutoSet(NXB_MODIFY);
		m_btnReload.AutoSet(NXB_REFRESH);
		m_btnEditMarkups.AutoSet(NXB_MODIFY); // (z.manning 2010-09-15 11:23) - PLID 40319
		m_btnApplyMarkups.AutoSet(NXB_MODIFY); // (z.manning 2010-10-27 14:01) - PLID 40619

		m_pdlFramesList = BindNxDataList2Ctrl(IDC_FRAMES_CATALOG_LIST, false);
		m_pdlManufacturerFilter = BindNxDataList2Ctrl(IDC_FRAMES_COMBO_MANUFACTURER, false);
		m_pdlCollectionFilter = BindNxDataList2Ctrl(IDC_FRAMES_COMBO_COLLECTION, false);
		m_pdlStyleFilter = BindNxDataList2Ctrl(IDC_FRAMES_COMBO_STYLE, false);
		m_pdlBrandFilter = BindNxDataList2Ctrl(IDC_FRAMES_COMBO_BRAND, false);
		m_pdlGroupFilter = BindNxDataList2Ctrl(IDC_FRAMES_COMBO_GROUP, false);
		m_pdlManufacturerFilter->PutFromClause(_bstr_t(GetFilterFromClause("ManufacturerName")));
		m_pdlCollectionFilter->PutFromClause(_bstr_t(GetFilterFromClause("CollectionName")));
		m_pdlStyleFilter->PutFromClause(_bstr_t(GetFilterFromClause("StyleName")));
		m_pdlBrandFilter->PutFromClause(_bstr_t(GetFilterFromClause("BrandName")));
		m_pdlGroupFilter->PutFromClause(_bstr_t(GetFilterFromClause("ProductGroupName")));
		RequeryFilters();

		IColumnSettingsPtr pcolBackColor = m_pdlFramesList->GetColumn(flcRowBackColor);
		pcolBackColor->PutFieldName(_bstr_t(FormatString(
			"CASE WHEN ProductT.ID IS NULL THEN %li ELSE %li END"
			, RGB(255,255,255), LINKED_TO_PRODUCT_COLOR)));

		CFramesData framesData;
		for(int nFieldIndex = 0; nFieldIndex < framesData.m_aryFields.GetSize(); nFieldIndex++)
		{
			FramesDataField *pFramesField = framesData.m_aryFields.GetAt(nFieldIndex);
			short nCol = m_pdlFramesList->InsertColumn(m_pdlFramesList->GetColumnCount(), _bstr_t("FramesDataT." + pFramesField->strDbField)
				, _bstr_t(pFramesField->strDisplayName), pFramesField->nColumnWidth, csVisible);
			IColumnSettingsPtr pCol = m_pdlFramesList->GetColumn(nCol);
			pCol->PutFieldType(cftTextSingleLine);
		}

		UpdateImportVersionLabel();
		UpdateView();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CInvFramesDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try
	{
		// (z.manning 2010-07-12 10:27) - Don't do anything here because we don't want this list to reload every
		// time they select the tab. (This also means the refresh which is why we disable it when on this tab.)
		//RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFramesList()
{
	CWaitCursor wc;
	m_pdlFramesList->Clear();
	m_pdlFramesList->PutWhereClause(_bstr_t(GetWhereClause()));
	m_nxstaticFramesCount.SetWindowText("Loading...");
	m_pdlFramesList->SetRedraw(VARIANT_FALSE);
	m_pdlFramesList->Requery();
}

void CInvFramesDlg::RequeryFilters()
{
	m_pdlManufacturerFilter->Requery();
	m_pdlStyleFilter->Requery();
	m_pdlCollectionFilter->Requery();
	m_pdlBrandFilter->Requery();
	m_pdlGroupFilter->Requery();
}

#define APPEND_FILTER_WHERE(pdl, nCol, strFieldName) \
	if(!pdl->IsRequerying() && pdl->GetCurSel() != NULL) { \
		strFilterValue = VarString(pdl->GetCurSel()->GetValue(mccName), ""); \
		if(!strFilterValue.IsEmpty() && strFilterValue != m_strFilterAllText) { \
			strWhere += FormatString(" AND (FramesDataT.%s = '%s')", strFieldName, _Q(strFilterValue)); \
		}\
	}

CString CInvFramesDlg::GetWhereClause()
{
	CString strWhere = "(FramesDataT.IsCatalog = 1)";
	CString strFilterValue;

	APPEND_FILTER_WHERE(m_pdlManufacturerFilter, mccName, "ManufacturerName");
	APPEND_FILTER_WHERE(m_pdlBrandFilter, bccName, "BrandName");
	APPEND_FILTER_WHERE(m_pdlCollectionFilter, cccName, "CollectionName");
	APPEND_FILTER_WHERE(m_pdlStyleFilter, sccName, "StyleName");
	APPEND_FILTER_WHERE(m_pdlGroupFilter, gccName, "ProductGroupName");

	return strWhere;
}

CString CInvFramesDlg::GetFilterFromClause(const CString strFieldName)
{
	CString strFrom = FormatString(
		"( \r\n"
		"	SELECT DISTINCT %s AS FilterField FROM FramesDataT WHERE IsCatalog = 1 \r\n"
		"	UNION \r\n"
		"	SELECT '%s' AS FilterField \r\n"
		") FilterQ \r\n"
		, strFieldName, _Q(m_strFilterAllText));
	return strFrom;
}

void CInvFramesDlg::SelChosenFramesComboManufacturer(LPDISPATCH lpRow)
{
	try
	{
		//RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::SelChosenFramesComboCollection(LPDISPATCH lpRow)
{
	try
	{
		//RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::SelChosenFramesComboStyle(LPDISPATCH lpRow)
{
	try
	{
		//RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::SelChosenFramesComboGroup(LPDISPATCH lpRow)
{
	try
	{
		//RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::SelChosenFramesComboBrand(LPDISPATCH lpRow)
{
	try
	{
		//RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-06-22 17:01) - PLID 39306
void CInvFramesDlg::OnBnClickedFramesImportButton()
{
	try
	{
		// (j.jones 2015-12-14 10:51) - PLID 67713 - before we waste the user's time and clear out existing data,
		// make sure that we can actually bulk import into SQL by validating that SQL can read from our file import
		// path (or shared path) from NxDock
		CString strBulkImportFailureWarning = "";
		if (!BulkImportUtils::CanSQLReadFromBulkImportPath_NotForPHI(GetRemoteData(), GetSubRegistryKey(), strBulkImportFailureWarning)) {
			CString strMessage;
			strMessage.Format("Frames data can not be imported due to a problem accessing the file import path:\n\n%s", strBulkImportFailureWarning);
			MessageBox(strMessage, "Practice", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// (j.jones 2015-12-14 10:51) - PLID 67713 - create a unique, temporary bulk import file path
		FileUtils::CAutoRemoveTempDirectory bulkFilePath = BulkImportUtils::CreateBulkImportFolder_NotForPHI(GetSubRegistryKey());

		// (z.manning 2010-06-22 17:01) - PLID 39306 - First prompt for an import path
		CString strImportPath;
		CString strDefaultImportPath = GetPropertyText("LastFramesDataImportPath", "", 0);
		if(!BrowseToFolder(&strImportPath, "Select the folder where your FramesData files are located", GetSafeHwnd(), NULL, strDefaultImportPath)) {
			return;
		}

		// (z.manning 2010-06-22 17:01) - PLID 39306 - The SpexUPC path must have the config.upc file so check for it
		// to ensure we have a valid directory.
		const CString strConfigFileName = "CONFIG.upc";
		if(FileUtils::DoesFileOrDirExist(strImportPath ^ "SpexUPC")) {
			// (z.manning 2010-06-21 17:53) - PLID 39306 - They chose the parent directory so make sure we're in the SpexUPC folder
			strImportPath = strImportPath ^ "SpexUPC";
		}
		if(!FileUtils::DoesFileOrDirExist(strImportPath ^ strConfigFileName)) {
			MessageBox("The selected folder is not a valid FramesData folder.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		// (z.manning 2010-06-22 17:04) - PLID 39306 - Open the config file to get the date of these files
		CStdioFile fileConfig;
		if(!fileConfig.Open(strImportPath ^ strConfigFileName, CFile::modeRead|CFile::shareExclusive)) {
			MessageBox("Failed to open file " + strImportPath ^ strConfigFileName, NULL, MB_OK|MB_ICONERROR);
			return;
		}
		CString strLine;
		if(!fileConfig.ReadString(strLine)) {
			MessageBox("Failed to read from config file", NULL, MB_OK|MB_ICONERROR);
			return;
		}
		fileConfig.Close();
		COleDateTime dtImportVersion(1900, 1, 1, 0, 0, 0);
		if(strLine.GetLength() >= 6) {
			int nMonth = atoi(strLine.Left(2));
			int nYear = atoi(strLine.Mid(2, 4));
			if(nMonth != 0 && nYear != 0) {
				dtImportVersion.SetDate(nYear, nMonth, 1);
			}
		}

		int nResult = MessageBox(FormatString("Are you sure you want to import the FramesData catalog from %s?  (If you had a previous FramesData catalog it will be overwritten.)"
			, FormatDateTimeForInterface(dtImportVersion,0,dtoDate)), NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}

		// (z.manning 2010-06-21 12:58) - Remember the path
		SetPropertyText("LastFramesDataImportPath", strImportPath, 0);
		CWaitCursor wc;

		// (z.manning 2010-07-14 10:28) - PLID 39306 - First get the file count so we can have an accurate progress bar
		CFileFind ff;
		BOOL bContinue = ff.FindFile(strImportPath ^ "b*.upc");
		int nFileCount = 0;
		while(bContinue)
		{
			bContinue = ff.FindNextFile();
			if(ff.IsDots() || ff.IsDirectory()) {
				continue;
			}
			nFileCount++;
		}

		// (z.manning 2010-07-14 10:29) - PLID 39306 - Now create the progress bar and add another 10% for deleting the old catalog
		DWORD dwDeleteProgress = nFileCount / 10;
		DWORD dwProgressMax = nFileCount + dwDeleteProgress;
		DWORD dwProgressPos = 0;
		CShowProgressFeedbackDlg dlgProgress(0, FALSE);
		dlgProgress.SetCaption("Frames Import");
		dlgProgress.ShowProgress(SW_SHOW);
		dlgProgress.SetProgress(0, dwProgressMax, dwProgressPos);

		// (z.manning 2010-06-22 17:05) - PLID 39306 - Before we import the new catalog data, clear out the old catalog
		long nRecordsAffected = 0;
		int nDeleteIteration = 1;
		do {
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(1000000);
			ExecuteSql(&nRecordsAffected, ADODB::adCmdText, 
				"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
				"DELETE FROM FramesDataT WHERE ID IN (SELECT TOP 1000 FramesInner.ID FROM FramesDataT FramesInner WHERE FramesInner.IsCatalog = 1)");
			if(dwProgressPos < dwDeleteProgress && nDeleteIteration % 100 == 0) {
				dwProgressPos++;
			}
			dlgProgress.SetProgress(0, dwProgressMax, dwProgressPos);
			nDeleteIteration++;
		}while(nRecordsAffected != 0);

		dwProgressPos = dwDeleteProgress;
		dlgProgress.SetProgress(0, dwProgressMax, dwProgressPos);

		// (z.manning 2010-06-22 17:06) - PLID 39306 - Files with frames data are of the format bXXXX.upc where XXXX is a number.
		// Let's search through all those files to load the data.
		bContinue = ff.FindFile(strImportPath ^ "b*.upc");
		DWORD dwStart = GetTickCount();
		while(bContinue)
		{
			bContinue = ff.FindNextFile();

			if(ff.IsDots() || ff.IsDirectory()) {
				continue;
			}

			CString strFile = ff.GetFilePath();

			CStdioFile fileFrames;
			if(!fileFrames.Open(strFile, CFile::modeRead|CFile::shareExclusive)) {
				dlgProgress.ShowProgress(SW_HIDE);
				ThrowNxException("Failed to open file: " + strFile);
			}

			// (z.manning 2010-06-22 17:07) - PLID 39306 - For each frames file, read through all the data and store the
			// FramesData objects in an array.
			CFramesDataArray arypFramesToImport;
			while(fileFrames.ReadString(strLine))
			{
				CFramesData *pFramesData = new CFramesData();
				arypFramesToImport.Add(pFramesData);
				pFramesData->LoadFromFramesFileLine(strLine);
			}
			fileFrames.Close();

			// (z.manning 2010-06-22 17:07) - PLID 39306 - We bulk import after each file instead of one giant bulk import
			// at the end because otherwise we are likely to have memory issues when loading this much data.
			// (j.jones 2015-12-14 10:56) - PLID 67713 - now this takes in a file import folder to bulk import from
			arypFramesToImport.BulkImportCatalog(bulkFilePath.GetPath());
			
			dlgProgress.SetProgress(0, dwProgressMax, ++dwProgressPos);
		}

		TRACE("\nFramesData import time: %f seconds\n\n", Round((GetTickCount() - dwStart) / 1000., 1));

		dlgProgress.ShowProgress(SW_HIDE);
		RequeryFilters();
		//RequeryFramesList();
		m_pdlFramesList->Clear();
		SetRemotePropertyDateTime("FramesImportVersionDate", dtImportVersion, 0, "<None>");
		UpdateImportVersionLabel();

		MessageBox("Import Done!", NULL, MB_OK|MB_ICONEXCLAMATION);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-06-23 10:14) - PLID 39311
void CInvFramesDlg::SelectAllFramesRows(BOOL bSelect)
{
	m_pdlFramesList->SetRedraw(VARIANT_FALSE);
	for(IRowSettingsPtr pRow = m_pdlFramesList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
		// (z.manning 2010-06-23 18:23) - PLID 39311 - No selecting product rows
		if(IsRowProduct(pRow)) {
			pRow->PutValue(flcCheck, g_cvarNull);
		}
		else {
			pRow->PutValue(flcCheck, bSelect ? g_cvarTrue : g_cvarFalse);
		}
	}
	m_pdlFramesList->SetRedraw(VARIANT_TRUE);
}

// (z.manning 2010-06-23 10:14) - PLID 39311
void CInvFramesDlg::OnBnClickedFramesSelectAll()
{
	try
	{
		SelectAllFramesRows(TRUE);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-06-23 10:14) - PLID 39311
void CInvFramesDlg::OnBnClickedFramesSelectNone()
{
	try
	{
		SelectAllFramesRows(FALSE);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-06-23 17:35) - PLID 39311
void CInvFramesDlg::CreateProductsFromFramesDataRows(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	CArray<IRowSettingsPtr,IRowSettingsPtr> arypFramesRows;
	arypFramesRows.Add(pRow);
	CreateProductsFromFramesDataRows(arypFramesRows);
}

// (z.manning 2010-06-23 10:14) - PLID 39311
void CInvFramesDlg::CreateProductsFromFramesDataRows(CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypFramesDataRows)
{
	if(arypFramesDataRows.GetSize() == 0) {
		return;
	}

	COleCurrency cyZero(0, 0);
	BOOL bBillable = (GetRemotePropertyInt("InvFrameOpBillable", 1, 0, "<None>") == 1);
	BOOL bTaxable1 = (GetRemotePropertyInt("InvFrameOpTaxable1", 0, 0, "<None>") == 1);
	BOOL bTaxable2 = (GetRemotePropertyInt("InvFrameOpTaxable2", 0, 0, "<None>") == 1);
	TrackStatus eTrackStatus = (TrackStatus)GetRemotePropertyInt("InvFrameOpTracking", tsTrackQuantity, 0, "<None>");
	long nCategoryID = GetRemotePropertyInt("InvFrameOpCategory", fpcvNone, 0, "<None>");

	CString strCategory;
	BOOL bCategoryFromFramesData = FALSE;

	BOOL bProductAlreadyExists = FALSE; 
	long nAlreadyExistsCount = 0; 

	switch(nCategoryID)
	{
		case fpcvNone:
			strCategory = "NULL";
			break;

		case fpcvCollection:
		case fpcvManufacturer:
		case fpcvBrand:
		case fpcvGroup:
			bCategoryFromFramesData = TRUE;
			break;

		default:
			strCategory = AsString(nCategoryID);
			break;
	}

	long nUserID = GetRemotePropertyInt("DefaultProductOrderingUser", -1, 0, "<None>");
	// (j.jones 2005-01-31 17:22) - the user could have been deleted or inactivated
	if(nUserID == -1 || IsRecordsetEmpty("SELECT ID FROM PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID WHERE Archived = 0 AND ID = %li",nUserID)) {
		nUserID = GetCurrentUserID();
	}

	// (z.manning 2010-09-22 09:11) - PLID 40619 - Store all the markups in a map as we may need them when creating products
	//CMap<long,long,CString,LPCTSTR> mapMarkupIDToFormula;
	CMap<long,long, CMarkUpFormula , CMarkUpFormula> mapMarkupIDToFormula;
	PopulateFormulaMap(mapMarkupIDToFormula);

	CString strSqlBatch = BeginSqlBatch();
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductID int");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewFramesDataID int");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nCategoryID int");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @CreatedProductsT TABLE (ID int, RowPointer int)");
	AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @bImportedAtLeastOneCategory bit");
	AddStatementToSqlBatch(strSqlBatch, "SET @bImportedAtLeastOneCategory = 0");
	CString strBaseSql = strSqlBatch;
	BOOL bImportedAtLeastOneProduct = FALSE, bImportedAtLeastOneCategory = FALSE;
	long nNewProductID = -1;
	for(long nRowIndex = 0; nRowIndex < arypFramesDataRows.GetSize(); nRowIndex++)
	{
		IRowSettingsPtr pRow = arypFramesDataRows.GetAt(nRowIndex);
		if(IsRowProduct(pRow)) {
			continue;
		}

		bImportedAtLeastOneProduct = TRUE;

		CFramesData framesData;
		framesData.LoadFromDatalistRow(m_pdlFramesList, pRow);

		// (b.spivey, September 28, 2011) - PLID 45265 - FramesDataT has a constraint for IsCatalog and FPC UNIQUE. 
		//	 This means that there can never be more than 2 FPC entries since IsCatalog is a bit (0 means it's a product). 
		//	 So if there is already a product associated with an FPC, then we'll update the datalist and remove it from 
		//	 the list of selected frames. 
		if(ReturnsRecordsParam("SELECT ID FROM FramesDataT WHERE IsCatalog = 0 AND FPC = {STRING}", 
			framesData.GetOutputByDataField("FPC"))){

				pRow->PutValue(flcCheck, g_cvarNull);
				pRow->PutValue(flcProductID, nNewProductID);
				pRow->PutBackColor(LINKED_TO_PRODUCT_COLOR);

				//This makes sure that the end prompt is accurate, and that we tell the client what happened. 
				arypFramesDataRows.RemoveAt(nRowIndex); 
				nRowIndex--; 
				bProductAlreadyExists = TRUE; 
				nAlreadyExistsCount++; 
				continue; 
		}

		if(bCategoryFromFramesData) {
			CString strCategoryText = framesData.GetCategoryText((EFramesProductCategoryOption)nCategoryID);
			// (z.manning 2011-01-24 15:33) - PLID 42193 - As a failsafe, make sure the category name will
			// fit in the database field.
			if(strCategoryText.GetLength() > 100) {
				strCategoryText = strCategoryText.Left(100);
			}
			AddStatementToSqlBatch(strSqlBatch,
				"SET @nCategoryID = (SELECT ID FROM CategoriesT WHERE Name = '%s') \r\n"
				"IF @nCategoryID IS NULL BEGIN \r\n"
				"	SET @nCategoryID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM CategoriesT) \r\n"
				"	INSERT INTO CategoriesT (ID, Name, Parent) VALUES (@nCategoryID, '%s', 0) \r\n"
				"	SET @bImportedAtLeastOneCategory = 1 \r\n"
				"END \r\n"
				, _Q(strCategoryText), _Q(strCategoryText));
			strCategory = "@nCategoryID";
		}

		CString strProductName = framesData.GetProductName();
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewProductID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ServiceT)");
		InvUtils::AddCreateProductSqlToBatch(strSqlBatch, "@nNewProductID", strProductName, bBillable, bTaxable1, bTaxable2, eTrackStatus, nUserID);
		AddStatementToSqlBatch(strSqlBatch, framesData.GetInsertSql(FALSE));
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewFramesDataID = (SELECT CONVERT(int, SCOPE_IDENTITY()))");
		AddStatementToSqlBatch(strSqlBatch, 
			"UPDATE ProductT \r\n"
			"SET FramesDataID = @nNewFramesDataID \r\n"
			"	, LastCost = COALESCE(FramesDataT.CompletePrice, 0) \r\n"
			"FROM ProductT \r\n"
			"INNER JOIN FramesDataT ON FramesDataT.ID = @nNewFramesDataID \r\n"
			"WHERE ProductT.ID = @nNewProductID "
			);

		// (j.jones 2015-03-03 17:11) - PLID 65113 - update ServiceT.Category and ServiceMultiCategoriesT
		if (strCategory.GetLength() > 0 && strCategory != "NULL") {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET Category = %s WHERE ID = @nNewProductID", strCategory);
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceMultiCategoryT (ServiceID, CategoryID) VALUES (@nNewProductID, %s)", strCategory);
		}

		// (z.manning 2010-09-22 13:13) - PLID 40619 - Check for a markup for this frame and if we have one let's
		// calculate the price based on the frame's wholesale cost.
		CString strPriceUpdateSql;
		long nMarkupID = VarLong(pRow->GetValue(flcMarkup), -1);
		if(nMarkupID != -1) {
			// (z.manning 2010-09-22 13:14) - PLID 40619 - This frame is associated with a markup
			//CString strFormula;
			CMarkUpFormula cFormula;
			//if(mapMarkupIDToFormula.Lookup(nMarkupID, strFormula)) {
			if(mapMarkupIDToFormula.Lookup(nMarkupID, cFormula)) {
				_variant_t varCost = framesData.GetValueByDataField("CompletePrice");
				if(varCost.vt == VT_CY) {
					COleCurrency cyCost = VarCurrency(varCost);
					if(cyCost > cyZero) {
						// (z.manning 2010-09-22 13:14) - PLID 40619 - We have a valid, non-zero cost so let's calculate the price
						// (b.eyers 2016-03-14) - PLID 68590 - change roundup bool to long
						// (j.jones 2016-05-17 11:10) - PLID-68615 - markups now return a currency
						COleCurrency cyNewPrice = COleCurrency(0, 0);
						if(InvUtils::CalculateMarkupPrice(cyCost, cFormula.m_strFormula, cyNewPrice, cFormula.m_nRoundUpRule)) {
							// (j.jones 2016-05-17 11:10) - PLID-68615 - make absolutely sure the price is rounded
							strPriceUpdateSql = FormatString(", Price = Round(Convert(money, '%s'), 2)", FormatCurrencyForSql(cyNewPrice));
						}
						else {
							// (z.manning 2010-09-22 13:15) - PLID 40619 - We validate markup formulas before saving them so this
							// should have calculated properly, so if we get here it's probably something silly like overflow.
							// We'll simply ignore this and move on.
						}
					}
				}
			}
		}

		AddStatementToSqlBatch(strSqlBatch, 
			"UPDATE ServiceT \r\n"
			"SET Barcode = (SELECT FramesDataT.UPC FROM FramesDataT WHERE FramesDataT.ID = @nNewFramesDataID) \r\n"
			"	%s \r\n"
			"WHERE ID = @nNewProductID "
			, strPriceUpdateSql);

		// (z.manning 2010-06-24 13:52) - For auditing
		DWORD dwRowPointer = (DWORD)((LPDISPATCH)pRow);
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @CreatedProductsT (ID, RowPointer) VALUES (@nNewProductID, %u)", dwRowPointer);

		// (z.manning 2010-06-23 17:45) - Execute this every so often so SQL server doesn't error out
		if(nRowIndex > 0 && nRowIndex % 200 == 0 && !strSqlBatch.IsEmpty()) {
			CommitNewProducts(strSqlBatch, nNewProductID, bImportedAtLeastOneCategory);
			strSqlBatch.Empty();
			strSqlBatch = strBaseSql;
		}
	}

	if(!bImportedAtLeastOneProduct) {
		return;
	}

	if(!strSqlBatch.IsEmpty() && strSqlBatch != strBaseSql) {
		CommitNewProducts(strSqlBatch, nNewProductID, bImportedAtLeastOneCategory);
	}

	if(bImportedAtLeastOneCategory) {
		CClient::RefreshTable(NetUtils::CPTCategories);
	}

	if(arypFramesDataRows.GetSize() == 1) {
		CClient::RefreshTable(NetUtils::Products, nNewProductID);
	}
	else {
		CClient::RefreshTable(NetUtils::Products);
	}

	// (b.spivey, September 28, 2011) - PLID 45265 - If a product already exists, we'll tell them how many. 
	if(bProductAlreadyExists){
		MessageBox(FormatString("Found %li existing product(s).", nAlreadyExistsCount), NULL, MB_OK|MB_ICONEXCLAMATION); 
	}
}

// (z.manning 2010-09-22 13:04) - PLID 40619 - This code was run twice so I moved it to its own function
void CInvFramesDlg::CommitNewProducts(const CString &strSqlBatch, OUT long &nNewProductID, OUT BOOL &bImportedAtLeastOneCategory)
{
	COleCurrency cyZero(0, 0);
	ADODB::_RecordsetPtr prs = CreateRecordsetStd(
		"SET NOCOUNT ON \r\n"
		"BEGIN TRAN \r\n" + strSqlBatch + "\r\nCOMMIT TRAN \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT CreatedProductsT.ID, Name, RowPointer, Price, @bImportedAtLeastOneCategory AS ImportedAtLeastOneCategory \r\n"
		"FROM @CreatedProductsT CreatedProductsT \r\n"
		"LEFT JOIN ServiceT ON CreatedProductsT.ID = ServiceT.ID \r\n"
		);
	CAuditTransaction auditTran;
	for(; !prs->eof; prs->MoveNext()) {
		if(!bImportedAtLeastOneCategory) {
			bImportedAtLeastOneCategory = AdoFldBool(prs->GetFields(), "ImportedAtLeastOneCategory");
		}
		nNewProductID = AdoFldLong(prs->GetFields(), "ID");
		CString strProdName = AdoFldString(prs->GetFields(), "Name", "");
		AuditEvent(-1, strProdName, auditTran, aeiProductCreate, nNewProductID, "", strProdName, aepMedium, aetCreated);
		// (z.manning 2010-09-22 13:08) - PLID 40619 - Also audit the price if we calculated one
		COleCurrency cyPrice = AdoFldCurrency(prs->GetFields(), "Price", cyZero);
		if(cyPrice != cyZero) {
			AuditEvent(-1, strProdName, auditTran, aeiProductServicePrice, nNewProductID, "", FormatCurrencyForInterface(cyPrice), aepMedium, aetChanged);
		}

		LPDISPATCH lpRow = (LPDISPATCH)AdoFldLong(prs->GetFields(), "RowPointer");
		IRowSettingsPtr prowTemp(lpRow);
		prowTemp->PutValue(flcCheck, g_cvarNull);
		prowTemp->PutValue(flcProductID, nNewProductID);
		prowTemp->PutBackColor(LINKED_TO_PRODUCT_COLOR);
	}
	auditTran.Commit();
}

// (z.manning 2010-06-23 10:14) - PLID 39311
void CInvFramesDlg::OnBnClickedFramesCreateSelectedProducts()
{
	try
	{
		CArray<IRowSettingsPtr,IRowSettingsPtr> arynSelectedFramesRows;
		for(IRowSettingsPtr pRow = m_pdlFramesList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			if(VarBool(pRow->GetValue(flcCheck), FALSE)) {
				arynSelectedFramesRows.Add(pRow);
			}
		}

		if(arynSelectedFramesRows.GetSize() == 0) {
			MessageBox("There are not any frames selected.", NULL, MB_OK|MB_ICONERROR);
			return;
		}

		if(!CheckCurrentUserPermissions(bioInvItem, sptCreate)) {
			return;
		}

		int nResult = MessageBox("Are you sure you want to create product(s) for the selected frame(s)?", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}

		CreateProductsFromFramesDataRows(arynSelectedFramesRows);

		MessageBox(FormatString("Created %li product(s).", arynSelectedFramesRows.GetSize()), NULL, MB_OK|MB_ICONEXCLAMATION);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-06-23 10:14) - PLID 39311
void CInvFramesDlg::OnBnClickedFramesUpdateExistingProducts()
{

	try
	{
		if(!CheckCurrentUserPermissions(bioInvItem, sptWrite)) {
			return;
		}

		ADODB::_RecordsetPtr prsCount = CreateRecordset("SELECT COUNT(*) AS Count FROM FramesDataT WHERE IsCatalog = 0");
		long nCount = 0;
		if(!prsCount->eof) {
			nCount = AdoFldLong(prsCount->GetFields(), "Count", 0);
		}

		if(nCount == 0) {
			MessageBox("You do not have any frames inventory products to update.");
			return;
		}

		int nResult = MessageBox(FormatString("Are you sure you want to update the frames data for all %li existing frame(s) in your inventory?\r\n\r\nAny changes you made to that data will be lost.",nCount), NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}

		CWaitCursor wc;
		CFramesData framesData;
		CString strUpdateList = framesData.GetFramesDateSameTableUpdateFieldsList("FramesProduct", "FramesCatalog");

		CParamSqlBatch sqlBatch;

		sqlBatch.AddFormat(
			"UPDATE FramesProduct \r\n"
			"SET %s \r\n"
			"FROM FramesDataT FramesProduct \r\n"
			"INNER JOIN FramesDataT FramesCatalog ON FramesProduct.FPC = FramesCatalog.FPC AND FramesCatalog.IsCatalog = 1 \r\n"
			"WHERE FramesProduct.IsCatalog = 0 \r\n"
			, strUpdateList);

		// (z.manning 2010-07-16 10:47) - PLID 39458 - Also update the product name and barcode
		// (a.wilson 2014-07-11 08:28) - PLID 61222 - only update if there is actual data to update from.
		sqlBatch.AddFormat(R"(
			UPDATE ServiceT 
			SET Barcode = CASE WHEN FramesDataT.UPC IS NOT NULL AND FramesDataT.UPC <> '' THEN FramesDataT.UPC ELSE ServiceT.Barcode END, 
			Name = CASE WHEN %s <> '' THEN %s ELSE ServiceT.Name END 
			FROM ServiceT 
			INNER JOIN ProductT ON ServiceT.ID = ProductT.ID 
			INNER JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID 
			WHERE IsCatalog = 0 
			)", framesData.GetProductNameSql(), framesData.GetProductNameSql());

		//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
		// Save old costs of frames to compare against new costs
		// (ProductT.PreviousCost is what ProductT.LastCost was before the frames update.)
		// (z.manning 2010-07-30 17:06) - PLID 39311 - Also update the last cost
		//(c.copits 2010-09-23) PLID 40316 - Use PreviousPrice instead of PreviousCost
		// (a.wilson 2014-07-11 08:28) - PLID 61222 - only update if there is actual data to update from.
		// (j.jones 2016-05-17 11:10) - PLID-68615 - make absolutely sure the price is rounded
		sqlBatch.AddFormat(R"(
			UPDATE ProductT 
			SET PreviousPrice = Round(Convert(money, CASE WHEN ServiceT.Price <> '0.00' THEN ServiceT.Price ELSE ProductT.PreviousPrice END), 2), 
			LastCost = Round(Convert(money, CASE WHEN FramesDataT.CompletePrice <> '0.00' THEN FramesDataT.CompletePrice ELSE ProductT.LastCost END), 2)
			FROM ProductT 
			INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID 
			INNER JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID 
			)");

		{
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(1000000);
			sqlBatch.Execute(GetRemoteData());
		}

		// (z.manning 2010-09-22 16:10) - PLID 40619 - Now, in a separate batch, let's check and see
		// if we have to update the price of any products.
		//r.wilson 3/9/2012 PLID 46664 - UPDATED -> added RoundUp Column
		// (b.eyers 2016-03-14) - PLID 68590 - roundup is now rounduprule which is a long
		ADODB::_RecordsetPtr prsFramesProducts = CreateRecordset(
			"SELECT ProductT.ID, ServiceT.Name, ProductT.LastCost, ServiceT.Price, MarkupFormulasT.Formula, MarkupFormulasT.RoundUpRule \r\n"
			"	, CASE WHEN MIN(SurgProduct.ID) IS NULL THEN convert(bit, 0) ELSE convert(bit, 1) END AS InSurg \r\n"
			"	, CASE WHEN MIN(PrefCardProduct.ID) IS NULL THEN convert(bit, 0) ELSE convert(bit, 1) END AS InPrefCard \r\n"
			"FROM ProductT \r\n"
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID \r\n"
			"INNER JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID \r\n"
			"INNER JOIN FramesMarkupLinkT ON FramesDataT.FPC = FramesMarkupLinkT.FrameFPC \r\n"
			"INNER JOIN MarkupFormulasT ON FramesMarkupLinkT.MarkupFormulaID = MarkupFormulasT.ID \r\n"
			"LEFT JOIN SurgeryDetailsT SurgProduct ON ServiceT.ID = SurgProduct.ServiceID \r\n"
			"LEFT JOIN PreferenceCardDetailsT PrefCardProduct ON ServiceT.ID = PrefCardProduct.ServiceID \r\n"
			"GROUP BY ProductT.ID, ServiceT.Name, ProductT.LastCost, ServiceT.Price, MarkupFormulasT.Formula, MarkupFormulasT.RoundUpRule \r\n"
			);
		COleCurrency cyZero(0, 0);
		struct ProductMarkupInfo {
			long nID;
			CString strName;
			COleCurrency cyOldPrice;
			COleCurrency cyNewPrice;
			BOOL bInSurg;
			BOOL bInPrefCard;

			ProductMarkupInfo() {
				nID = -1;
				cyNewPrice = cyOldPrice = COleCurrency(0, 0);
				bInSurg = bInPrefCard = FALSE;
			}
			ProductMarkupInfo(long _nID, CString _strName, COleCurrency _cyOldPrice, COleCurrency _cyNewPrice, BOOL _bInSurg, BOOL _bInPrefCard) {
				nID = _nID;
				strName = _strName;
				cyOldPrice = _cyOldPrice;
				cyNewPrice = _cyNewPrice;
				bInSurg = _bInSurg;
				bInPrefCard = _bInPrefCard;
			}
		};
		// (z.manning 2010-09-23 10:24) - PLID 40619 - Go through every product with a markup assigned to it
		// and see if the price changed.
		CArray<ProductMarkupInfo,ProductMarkupInfo&> aryMarkupInfo;
		BOOL bAtLeastOneSurg = FALSE, bAtLeastOnePrefCard = FALSE;
		for(; !prsFramesProducts->eof; prsFramesProducts->MoveNext())
		{
			const long nProductID = AdoFldLong(prsFramesProducts->GetFields(), "ID");
			CString strName = AdoFldString(prsFramesProducts->GetFields(), "Name");
			const COleCurrency cyCost = AdoFldCurrency(prsFramesProducts->GetFields(), "LastCost");
			const COleCurrency cyCurrentPrice = AdoFldCurrency(prsFramesProducts->GetFields(), "Price");
			CString strMarkupFormula = AdoFldString(prsFramesProducts->GetFields(), "Formula");
			BOOL bInSurg = AdoFldBool(prsFramesProducts->GetFields(), "InSurg");
			BOOL bInPrefCard = AdoFldBool(prsFramesProducts->GetFields(), "InPrefCard");
			//r.wilson 3/9/2012 PLID 46664 
			// (b.eyers 2016-03-14) - PLID 68590 - changed roundup(bit) to rounduprule(long)
			long nRoundUpRule = AdoFldLong(prsFramesProducts->GetFields(), "RoundUpRule");
			// (j.jones 2016-05-17 11:10) - PLID-68615 - markups now return a currency
			COleCurrency cyNewPrice = COleCurrency(0, 0);
			if(cyCost > cyZero && InvUtils::CalculateMarkupPrice(cyCost, strMarkupFormula, cyNewPrice, nRoundUpRule)) {				
				if(cyCurrentPrice != cyNewPrice) {
					// (z.manning 2010-09-23 10:26) - PLID 40619 - The price changed. Let's store the relevant info
					// in an array so that we know exactly how much changed before we commit anything.
					aryMarkupInfo.Add(ProductMarkupInfo(nProductID, strName, cyCurrentPrice, cyNewPrice, bInSurg, bInPrefCard));
					if(bInSurg) {
						bAtLeastOneSurg = TRUE;
					}
					if(bInPrefCard) {
						bAtLeastOnePrefCard = TRUE;
					}
				}
			}
		}

		long nChangedPrices = aryMarkupInfo.GetSize();
		if(nChangedPrices > 0)
		{
			// (z.manning 2010-09-23 10:27) - PLID 40619 - Prompt the user to update all the prices
			int nResult = MessageBox(FormatString("%i product(s) have a new price based on their markup and updated cost.\r\n\r\n"
				"Would you like to update the prices of all of these product(s) in inventory?", nChangedPrices), NULL, MB_YESNO|MB_ICONQUESTION);
			if (nResult == IDYES)
			{
				// (z.manning 2010-11-02 17:34) - PLID 40619 - Also see if we should be updating surgery prices
				BOOL bUpdateSurgPrices = FALSE;
				if (bAtLeastOneSurg) {
					long nUpdateSurgPricesOption = GetRemotePropertyInt("UpdateSurgeryPrices", 1, 0, "<None>", true);
					if (nUpdateSurgPricesOption == 2) {
						bUpdateSurgPrices = TRUE;
					}
					else if (nUpdateSurgPricesOption == 1) {
						int nResult = MessageBox("Do you want to update the surgeries that use any of the frames having their price updated?"
							, NULL, MB_ICONQUESTION | MB_YESNO);
						if (nResult == IDYES) {
							bUpdateSurgPrices = TRUE;
						}
					}
				}

				// (z.manning 2010-11-02 17:34) - PLID 40619 - Also see if we should be updating preference card prices
				BOOL bUpdatePrefCardPrices = FALSE;
				if (bAtLeastOnePrefCard) {
					long nUpdatePrefCardPricesOption = GetRemotePropertyInt("UpdatePreferenceCardPrices", 1, 0, "<None>", true);
					if (nUpdatePrefCardPricesOption == 2) {
						bUpdatePrefCardPrices = TRUE;
					}
					else if (nUpdatePrefCardPricesOption == 1) {
						int nResult = MessageBox("Do you want to update the preference cards that use any of the frames having their price updated?"
							, NULL, MB_ICONQUESTION | MB_YESNO);
						if (nResult == IDYES) {
							bUpdatePrefCardPrices = TRUE;
						}
					}
				}

				// (z.manning 2010-09-23 10:32) - PLID 40619 - We could potentially have hundreds of thousands of price
				// updates so let's execute them in batches.
				const int nMaxProcess = 100;
				while (aryMarkupInfo.GetSize() > 0)
				{
					int nCountProcessed = 0;
					CParamSqlBatch batchMarkupPrice;
					CAuditTransaction auditTran;
					for (int nMarkupIndex = aryMarkupInfo.GetSize() - 1; nMarkupIndex >= 0; nMarkupIndex--) {
						ProductMarkupInfo markup = aryMarkupInfo.GetAt(nMarkupIndex);
						if (bUpdateSurgPrices && markup.bInSurg) {
							InvUtils::UpdateSurgeriesForPriceChange(this, batchMarkupPrice, markup.nID, markup.cyNewPrice, TRUE);
						}
						if (bUpdatePrefCardPrices && markup.bInPrefCard) {
							InvUtils::UpdatePrefCardsForPriceChange(this, batchMarkupPrice, markup.nID, markup.cyNewPrice, TRUE);
						}
						// (j.jones 2016-05-17 11:10) - PLID-68615 - make absolutely sure the price is rounded
						batchMarkupPrice.Add("UPDATE ServiceT SET Price = Round(Convert(money, {VT_CY}), 2) WHERE ID = {INT};", _variant_t(markup.cyNewPrice), markup.nID);
						AuditEvent(-1, markup.strName, auditTran, aeiProductServicePrice, markup.nID, FormatCurrencyForInterface(markup.cyOldPrice), FormatCurrencyForInterface(markup.cyNewPrice), aepMedium, aetChanged);
						aryMarkupInfo.RemoveAt(nMarkupIndex);
						nCountProcessed++;
						if (nCountProcessed >= nMaxProcess) {
							// (z.manning 2010-09-23 10:32) - PLID 40619 - We've reached the max amount that we want to process
							// in one batch so let's get out of here and commit everything so far.
							break;
						}
					}
					if (!batchMarkupPrice.IsEmpty()) {
						// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
						NxAdo::PushMaxRecordsWarningLimit pmr(nMaxProcess * 2);
						batchMarkupPrice.Execute(GetRemoteData());
						auditTran.Commit();
					}
				}

				CString strChanged;
				if (nChangedPrices > 1) {
					strChanged.Format(
						"Frames data has been updated for all frames inventory products.\n\n"
						"%ld frames have changed price.\n\n"
						"Would you like to run the report for new price labels?",
						nChangedPrices);
				}
				else {
					strChanged.Format(
						"Frames data has been updated for all frames inventory products.\n\n"
						"1 frame has changed price.\n\n"
						"Would you like to run the report for new price labels?"
					);
				}
				if (MessageBox(strChanged, "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
					ShowPriceChangeReport();
				}
			}
			else
			{
				MessageBox("All frame products are up to date. No price changes have been made.", NULL, MB_OK | MB_ICONEXCLAMATION);
			}
		}
		else {
			MessageBox("There were no price changes. All frame products are up to date.", NULL, MB_OK|MB_ICONEXCLAMATION);
		}

		CClient::RefreshTable(NetUtils::Products);

	}NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
// Returns the number of frames that have changed price since last update.
long CInvFramesDlg::CheckChangedPrices()
{
	long nChangedPriceCount = 0;	// Number of changed prices

	try {
		// Check new prices to see if they have changed since last time.
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT COUNT(ProductT.ID) AS ChangedPriceCount \r\n"
			"FROM ProductT \r\n"
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID \r\n"
			"INNER JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID \r\n"
			"WHERE COALESCE(ProductT.PreviousPrice, 0) <> ServiceT.Price \r\n"
			);
		if(!prs->eof) {
			nChangedPriceCount = AdoFldLong(prs, "ChangedPriceCount");
		}
	} NxCatchAll(__FUNCTION__);

	return nChangedPriceCount;
}

//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
// Show the frames price change report
void CInvFramesDlg::ShowPriceChangeReport()
{
	try
	{
		// Create the preview window
		CPrintInfo prInfo;
		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		prInfo.m_bPreview = false;
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if(prInfo.m_pPD != NULL) {
			delete prInfo.m_pPD;
		}
		prInfo.m_pPD = dlg;

		// Preview Avery 8167 Labels Report
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(368)]);
		CPtrArray aryParams;
		// Pass it the version with a special WHERE clause
		infReport.strListBoxFormat = "FramesPriceChanges";
		RunReport(&infReport, &aryParams, true, (CWnd*)this, "Frames Price Changes", &prInfo);
		ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-23 14:42) - PLID 39314
void CInvFramesDlg::OnBnClickedFramesOptions()
{
	try {
		CFramesOptionsDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFinishedFramesComboManufacturer(short nFlags)
{
	try
	{
		m_pdlManufacturerFilter->SetSelByColumn(mccName, _bstr_t(m_strFilterAllText));

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFinishedFramesComboBrand(short nFlags)
{
	try
	{
		m_pdlBrandFilter->SetSelByColumn(bccName, _bstr_t(m_strFilterAllText));

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFinishedFramesComboCollection(short nFlags)
{
	try
	{
		m_pdlCollectionFilter->SetSelByColumn(cccName, _bstr_t(m_strFilterAllText));

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFinishedFramesComboGroup(short nFlags)
{
	try
	{
		m_pdlGroupFilter->SetSelByColumn(gccName, _bstr_t(m_strFilterAllText));

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFinishedFramesComboStyle(short nFlags)
{
	try
	{
		m_pdlStyleFilter->SetSelByColumn(sccName, _bstr_t(m_strFilterAllText));

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::RequeryFinishedFramesCatalogList(short nFlags)
{
	CWaitCursor wc;
	try
	{
		// (z.manning 2010-07-22 16:40) - Show a please wait dialog since the synchronous drawing
		// of all those rows may take a while.
		CShowConnectingFeedbackDlg dlgWait;
		dlgWait.SetWindowTitle("Loading Frames Data");
		dlgWait.SetWaitMessage("Please wait while frames data is loaded...");
		COleCurrency cyRowCount(m_pdlFramesList->GetRowCount(), 0);
		m_nxstaticFramesCount.SetWindowText(FormatCurrencyForInterface(cyRowCount, FALSE, TRUE, FALSE));
		m_pdlFramesList->SetRedraw(VARIANT_TRUE);

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::UpdateImportVersionLabel()
{
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dtImportVersion = GetRemotePropertyDateTime("FramesImportVersionDate", NULL, 0, "<None>", false);
	if(dtImportVersion.GetStatus() == COleDateTime::valid) {
		m_nxstaticFramesDate.SetWindowText("Import Version Date: " + FormatDateTimeForInterface(dtImportVersion, 0, dtoDate));
	}
}

// (z.manning 2010-06-23 17:35) - PLID 39311
void CInvFramesDlg::RButtonDownFramesCatalogList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		m_pdlFramesList->PutCurSel(pRow);

		enum MenuItems {
			miCreateProduct = 1,
			miGoToProduct,
			miFilterIncluding,
			miFilterExcluding,
			miRemoveFilter,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();

		if(pRow != NULL)
		{
			if(IsRowProduct(pRow)) {
				// (z.manning 2010-09-30 17:29) - PLID 40754 - Ability to go to the inventory product for this frame
				mnu.AppendMenu(MF_ENABLED, miGoToProduct, "&Go to Product tab for this frame");
			}
			else {
				mnu.AppendMenu(MF_ENABLED, miCreateProduct, "&Create product for this frame");
			}
			// (z.manning 2010-10-01 10:52) - PLID 40761 - Added filter options
			mnu.AppendMenu(MF_SEPARATOR);
			IColumnSettingsPtr pCol = m_pdlFramesList->GetColumn(nCol);
			CString strClause = AsClauseString(pRow->GetOutputValue(nCol));
			CString strCellString;
			strCellString.Format("%s %s", (LPCTSTR)pCol->ColumnTitle, strClause);
			mnu.AppendMenu(MF_ENABLED, miFilterIncluding, "&Filter selection " + strCellString);
			mnu.AppendMenu(MF_ENABLED, miFilterExcluding, "Filter &excluding " + strCellString);
		}
		if(m_bDatalistFiltered) {
			mnu.AppendMenu(MF_ENABLED, miRemoveFilter, "&Remove Filter");
		}

		CPoint pt(x, y);
		GetDlgItem(IDC_FRAMES_CATALOG_LIST)->ClientToScreen(&pt);

		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		CWaitCursor wc;
		switch(nResult)
		{
			case miCreateProduct:
				if(!CheckCurrentUserPermissions(bioInvItem, sptCreate)) {
					return;
				}
				if(IDYES != MessageBox("Are you sure you want to create this frame as an inventory product?", NULL, MB_YESNO|MB_ICONQUESTION)) {
					return;
				}
				CreateProductsFromFramesDataRows(pRow);
				break;

			case miGoToProduct:
				// (z.manning 2010-09-30 18:05) - PLID 40754 - Switch to the product tab and load the product for this frame.
				if(GetMainFrame()->FlipToModule(INVENTORY_MODULE_NAME))
				{
					CNxTabView *pInvView = (CNxTabView*)GetMainFrame()->GetOpenView(INVENTORY_MODULE_NAME);
					if(pInvView != NULL) {
						pInvView->SetActiveTab(InventoryModule::ItemTab);
						long nProductID = VarLong(pRow->GetValue(flcProductID), -1);
						if(nProductID != -1) {
							BOOL bSet = ((CInvEditDlg*)pInvView->GetActiveSheet())->LoadSingleItem(nProductID);
							if(!bSet) {
								AfxMessageBox("The product could not be loaded, it may be inactive.");
							}
						}
					}
				}
				break;

			case miFilterIncluding:
			case miFilterExcluding:
				{
					// (z.manning 2010-10-01 11:50) - PLID 40761 - Handle filtering the datalist based on any column
					IRowSettingsPtr pTempRow = m_pdlFramesList->GetFirstRow();
					while(pTempRow != NULL)
					{
						IRowSettingsPtr pNextRow = pTempRow->GetNextRow();
						if((nResult == miFilterIncluding && pTempRow->GetValue(nCol) != pRow->GetValue(nCol)) ||
							(nResult == miFilterExcluding && pTempRow->GetValue(nCol) == pRow->GetValue(nCol))
							)
						{
							m_bDatalistFiltered = TRUE;
							m_pdlFramesList->RemoveRow(pTempRow);
						}
						pTempRow = pNextRow;
					}
					COleCurrency cyRowCount(m_pdlFramesList->GetRowCount(), 0);
					m_nxstaticFramesCount.SetWindowText(FormatCurrencyForInterface(cyRowCount, FALSE, TRUE, FALSE));
				}
				break;

			case miRemoveFilter:
				// (z.manning 2010-10-01 11:52) - PLID 40761 - Reload the list without any datalist filtering.
				m_bDatalistFiltered = FALSE;
				RequeryFramesList();
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CInvFramesDlg::IsRowProduct(IRowSettingsPtr pRow)
{
	return (pRow->GetValue(flcProductID).vt != VT_NULL);
}

void CInvFramesDlg::EditingFinishingFramesCatalogList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case flcCheck:
				if(VarBool(*pvarNewValue, FALSE)) {
					// (z.manning 2010-06-23 18:26) - PLID 39311 - No checking rows if they are already a product
					if(IsRowProduct(pRow)) {
						MessageBox("This frame already has a product in inventory.", NULL, MB_OK|MB_ICONWARNING);
						*pbCommit = FALSE;
					}
				}
				break;
		}

	}NxCatchAll(__FUNCTION__);
}

void CInvFramesDlg::OnBnClickedFramesReload()
{
	try
	{
		// (z.manning 2010-07-22 15:39) - There is too much frames data to do any automatic refreshing so the only
		// way to reload the list is manually.
		RequeryFramesList();

	}NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-08-31) PLID 40316 - Alert user of new prices for frames in inventory
//Show price change report to user
void CInvFramesDlg::OnBnClickedPriceChangeReport()
{

	try
	{
		// Want to prevent a blank page from being displayed

		long nChangedPriceCount = CheckChangedPrices();
		if (nChangedPriceCount < 1) {
			MessageBox("No price changes detected.", NULL, MB_OK);
		}
		else {
			ShowPriceChangeReport();
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-09-15 10:23) - PLID 40319
void CInvFramesDlg::OnBnClickedFramesEditMarkups()
{
	try
	{
		CMarkupFormulaEditDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			// (z.manning 2010-09-22 09:54) - PLID 40619 - Referesh the embedded combo
			IColumnSettingsPtr pMarkupCol = m_pdlFramesList->GetColumn(flcMarkup);
			pMarkupCol->PutComboSource(pMarkupCol->GetComboSource());
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-09-21 14:53) - PLID 40619
void CInvFramesDlg::OnBnClickedFramesApplyMarkup()
{
	try
	{
		if(m_pdlFramesList->GetRowCount() == 0) {
			MessageBox("There are currently no frames in the list.", NULL, MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (z.manning 2010-09-21 14:53) - PLID 40619 - Don't bother if they haven't created any markups
		if(!ReturnsRecords("SELECT TOP 1 ID FROM MarkupFormulasT")) {
			MessageBox("You do not have any markups to apply. You can create them by clicking the Edit Markups button.", NULL, MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (z.manning 2010-09-21 14:53) - PLID 40619 - Prompt for a markup
		CSingleSelectDlg dlg(this);
		int nSelectResult = dlg.Open("(SELECT ID, Name, Formula FROM MarkupFormulasT UNION SELECT -1, ' <No Markup>', '') MarkupQ"
			, "", "ID", "Name + CASE WHEN Formula = '' THEN '' ELSE ' (' + Formula + ')' END", "Select a markup to apply", true);
		if(nSelectResult == IDOK)
		{
			long nMarkupID = dlg.GetSelectedID();
			CString strMarkup = dlg.GetSelectedDisplayValue();
			int nMsgBoxResult = MessageBox("Are you sure you want to apply the '" + strMarkup + "' markup to all visible frames?", NULL, MB_YESNO|MB_ICONQUESTION);
			if(nMsgBoxResult != IDYES) {
				return;
			}

			// (z.manning 2010-09-21 14:53) - PLID 40619 - Apply the markup to all catalog frames in the current filter
			// Note: Do not call CInvFramesDlg::GetWhereClause here because that will take the current state of the filters,
			// however, that may not reflect what's actually visible since we don't update the list every time a filter
			// is changed.
			CWaitCursor wc;
			CParamSqlBatch sqlBatch;
			CString strWhere = VarString(m_pdlFramesList->GetWhereClause());
			sqlBatch.AddFormat(
				"DELETE FramesMarkupLinkT \r\n"
				"FROM FramesMarkupLinkT \r\n"
				"INNER JOIN FramesDataT ON FramesMarkupLinkT.FrameFPC = FramesDataT.FPC \r\n"
				"WHERE %s "
				, strWhere);
			if(nMarkupID != -1) {
				sqlBatch.Add(FormatString(
					"INSERT INTO FramesMarkupLinkT (FrameFPC, MarkupFormulaID) \r\n"
					"SELECT FramesDataT.FPC, {INT} FROM FramesDataT WHERE %s "
					, strWhere)
					, nMarkupID);
			}
			
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(500000);
			sqlBatch.Execute(GetRemoteData());

			for(IRowSettingsPtr pRow = m_pdlFramesList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
				pRow->PutValue(flcMarkup, nMarkupID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-09-22 09:11) - PLID 40619
void CInvFramesDlg::PopulateFormulaMap(OUT CMap<long,long,CMarkUpFormula,CMarkUpFormula> &mapMarkupIDToFormula)
{
	mapMarkupIDToFormula.RemoveAll();
	//r.wilson 3/9/2012 PLID 46664 -> added RoundUp column
	// (b.eyers 2016-03-14) - PLID 68590 - changed roundup(bit) to rounduprule(long)
	ADODB::_RecordsetPtr prs = CreateRecordset("SELECT ID, Formula, RoundUpRule FROM MarkupFormulasT");
	for(; !prs->eof; prs->MoveNext())
	{
		long nMarkupID = AdoFldLong(prs->GetFields(), "ID");
		CString strFormula = AdoFldString(prs->GetFields(), "Formula");
		long nRoundUpRule = AdoFldLong(prs->GetFields(), "RoundUpRule");
		
		CMarkUpFormula cFormula(nMarkupID,strFormula,nRoundUpRule);
		mapMarkupIDToFormula.SetAt(nMarkupID,cFormula);
		
		//mapMarkupIDToFormula.SetAt(nMarkupID, strFormula);
	}
}

// (z.manning 2010-09-22 09:36) - PLID 40619
void CInvFramesDlg::EditingFinishedFramesCatalogList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CFramesData frame;
		frame.LoadFromDatalistRow(m_pdlFramesList, pRow);

		switch(nCol)
		{
			case flcMarkup:
				// (z.manning 2010-09-22 10:33) - PLID 40619 - Apply the selected markup to the FPC corresponding to this frame row
				long nNewMarkupID = VarLong(varNewValue, -1);
				CString strFpc = frame.GetOutputByDataField("FPC");
				CParamSqlBatch sqlBatch;
				sqlBatch.Add("DELETE FROM FramesMarkupLinkT WHERE FrameFPC = {STRING}", strFpc);
				if(nNewMarkupID != -1) {
					sqlBatch.Add("INSERT INTO FramesMarkupLinkT (FrameFPC, MarkupFormulaID) VALUES ({STRING}, {INT})", strFpc, nNewMarkupID);
				}
				sqlBatch.Execute(GetRemoteData());
				break;
		}

	}NxCatchAll(__FUNCTION__);
}