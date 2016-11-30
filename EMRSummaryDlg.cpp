// EMRSummaryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRSummaryDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "UnitedLink.h"
#include "MirrorLink.h"
#include "Mirror.h"
#include "FileUtils.h"
#include "ImageDlg.h"
#include "Graphics.h"
#include "EMRSummaryConfigDlg.h"
#include "EMNDetail.h"
#include "LabsSetupDlg.h"
#include "DontShowDlg.h"
#include "EMN.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

/////////////////////////////////////////////////////////////////////////////
// CEMRSummaryDlg dialog

#define IDM_COPY 40000
#define IDM_VIEW 40001

#define TIMER_LOAD_EMN_LIST 1

enum PatientListColumns {

	plcLeft = 0,
	plcRight,
};

enum EMNListColumns {

	elcLeft = 0,
	elcRight,
};


// (a.walling 2010-03-09 14:16) - PLID 37640 - Moved to cpp
CEMRSummaryImageLoad::CEMRSummaryImageLoad(CEMRSummaryDlg* pMsgWnd, CMirrorPatientImageMgr* pMirrorImageMgr, CString strRemoteID, long nImageIndex, long nImageCount, long nPatientID, CMirrorImageButton *pButton)
{
	m_pMsgWnd = pMsgWnd;
	m_pMirrorImageMgr = new CMirrorPatientImageMgr(pMirrorImageMgr);
	m_strRemoteID = strRemoteID;
	m_nImageIndex = nImageIndex;
	m_nImageCount = nImageCount;
	m_nPatientID = nPatientID;
	m_pButton = pButton;
}

CEMRSummaryImageLoad::~CEMRSummaryImageLoad()
{
	if (NULL != m_pMirrorImageMgr) {
		delete m_pMirrorImageMgr;
	}
}

CEMRSummaryDlg::CEMRSummaryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRSummaryDlg::IDD, pParent)
{
	// (c.haag 2008-12-16 15:32) - PLID 32467 - Added m_bShowPtEmrProblems, m_bShowPtNonEmrProblems
	//{{AFX_DATA_INIT(CEMRSummaryDlg)
		m_nPatientID = -1;
		m_nEMRID = -1;
		m_nEMNID = -1;
		m_nCurEMNIndex = -1;
		m_bDisplayArrows = TRUE;
		m_nUnitedID = -1;
		m_strMirrorID = "";
		m_nImageIndex = -1;
		m_nUserDefinedID = -1;
		m_pLoadImageThread = NULL;
		m_bShowAllergies = TRUE;
		m_bShowCurrentMeds = TRUE;
		m_bShowPrescriptions = TRUE;
		m_bShowLabs = TRUE;
		m_bShowProblemList = TRUE;
		m_bShowEMRDocuments = TRUE;
		m_bShowPtEmrProblems = TRUE;
		m_bShowPtNonEmrProblems = TRUE;
	//}}AFX_DATA_INIT
	m_pMirrorImageMgr = NULL;
}


void CEMRSummaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRSummaryDlg)
	DDX_Control(pDX, IDC_BTN_CONFIGURE_SUMMARY, m_btnConfigure);
	DDX_Control(pDX, IDC_PT_IMAGE, m_imageButton);
	DDX_Control(pDX, IDC_PT_IMAGE_LEFT, m_imageButtonLeft);
	DDX_Control(pDX, IDC_PT_IMAGE_RIGHT, m_imageButtonRight);
	DDX_Control(pDX, IDC_PT_IMAGE_UPPER_LEFT, m_imageButtonUpperLeft);
	DDX_Control(pDX, IDC_PT_IMAGE_UPPER_RIGHT, m_imageButtonUpperRight);
	DDX_Control(pDX, IDC_PT_IMAGE_LOWER_LEFT, m_imageButtonLowerLeft);
	DDX_Control(pDX, IDC_PT_IMAGE_LOWER_RIGHT, m_imageButtonLowerRight);
	DDX_Control(pDX, IDC_PT_IMAGE_NEXT, m_btnPtImageNext);
	DDX_Control(pDX, IDC_PT_IMAGE_LAST, m_btnPtImageLast);
	DDX_Control(pDX, IDC_BTN_NEXT_EMN, m_btnNextEMN);
	DDX_Control(pDX, IDC_BTN_PREV_EMN, m_btnPrevEMN);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_EMR_SUMMARY_PREVIEW, m_btnEMRSummaryPreview);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRSummaryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRSummaryDlg)
	ON_BN_CLICKED(IDC_BTN_PREV_EMN, OnBtnPrevEmn)
	ON_BN_CLICKED(IDC_BTN_NEXT_EMN, OnBtnNextEmn)
	ON_BN_CLICKED(IDC_BTN_TOGGLE_VIEW_TYPE, OnToggleViewType)
	ON_BN_CLICKED(IDC_EMR_SUMMARY_PREVIEW, OnEmrSummaryPreview)
	ON_BN_CLICKED(IDC_PT_IMAGE_LAST, OnPtImageLast)
	ON_BN_CLICKED(IDC_PT_IMAGE_NEXT, OnPtImageNext)
	ON_MESSAGE(NXM_G1THUMB_IMAGELOADED, OnImageLoaded)
	ON_BN_CLICKED(IDC_PT_IMAGE, OnPtImage)
	ON_BN_CLICKED(IDC_PT_IMAGE_LEFT, OnPtImageLeft)
	ON_BN_CLICKED(IDC_PT_IMAGE_RIGHT, OnPtImageRight)
	ON_BN_CLICKED(IDC_PT_IMAGE_UPPER_LEFT, OnPtImageUpperLeft)
	ON_BN_CLICKED(IDC_PT_IMAGE_UPPER_RIGHT, OnPtImageUpperRight)
	ON_BN_CLICKED(IDC_PT_IMAGE_LOWER_LEFT, OnPtImageLowerLeft)
	ON_BN_CLICKED(IDC_PT_IMAGE_LOWER_RIGHT, OnPtImageLowerRight)
	ON_COMMAND(IDM_COPY, OnCopyImage)
	ON_COMMAND(IDM_VIEW, OnViewImage)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_SUMMARY, OnBtnConfigureSummary)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRSummaryDlg message handlers

BOOL CEMRSummaryDlg::OnInitDialog() 
{
	try {

		CWaitCursor pWait;

		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-19 16:58) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnEMRSummaryPreview.AutoSet(NXB_PRINT_PREV);
		
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connections
		m_PatientList = BindNxDataList2Ctrl(this, IDC_EMR_PATIENT_SUMMARY_LIST, GetRemoteDataSnapshot(), false);
		m_EMNList = BindNxDataList2Ctrl(this, IDC_EMR_SUMMARY_LIST, GetRemoteDataSnapshot(), false);

		m_btnPrevEMN.AutoSet(NXB_LEFT);
		m_btnNextEMN.AutoSet(NXB_RIGHT);

		// (j.jones 2008-06-13 09:12) - PLID 30385 - added patient image controls
		m_btnPtImageLast.AutoSet(NXB_LEFT);
		m_btnPtImageNext.AutoSet(NXB_RIGHT);

		// (j.jones 2008-06-19 09:24) - PLID 30436 - added m_btnConfigure
		m_btnConfigure.AutoSet(NXB_MODIFY);
		
		OLE_COLOR nColor = 0x00F8CCC7;

		// (j.jones 2008-06-25 17:22) - PLID 21168 - bulk load all our preferences
		// (c.haag 2008-12-16 15:28) - PLID 32467 - Include support for showing problems in
		// the general box at the top of the window
		g_propManager.CachePropertiesInBulk("CEMRSummaryDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMRSummaryShowAllergies' OR "
			"Name = 'EMRSummaryShowCurrentMeds' OR "
			"Name = 'EMRSummaryShowPrescriptions' OR "
			"Name = 'EMRSummaryShowLabs' OR "
			"Name = 'EMRSummaryShowProblemList' OR "
			"Name = 'EMRSummaryShowPtEmrProblems' OR "
			"Name = 'EMRSummaryShowPtNonEmrProblems' OR "
			"Name = 'EMRSummaryShowEMRDocuments' OR "
			"Name = 'EMRSummaryShowArrows' OR "
			"Name = 'General1ImageCount' OR "
			"Name = 'MirrorAllowAsyncOperations' OR "
			// (c.haag 2011-01-20) - PLID 42166 - MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp
			"Name = 'MirrorLinkHideSDKImagesImportedFromMirrorImageImportApp' "
			")",
			_Q(GetCurrentUserName()));

		m_bDisplayArrows = GetRemotePropertyInt("EMRSummaryShowArrows", 1, 0, GetCurrentUserName(), true);

		ToggleNavigationType();

		Load();		
	}
	NxCatchAll("Error in CEMRSummaryDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.jones 2008-06-27 10:20) - PLID 21168 - added a proper Load function
void CEMRSummaryDlg::Load()
{
	try {

		CWaitCursor pWait;

		// (c.haag 2010-02-23 09:55) - PLID 37364 - Delete the Mirror image manager if it exists
		EnsureNotMirrorImageMgr();

		// (j.jones 2008-07-02 10:55) - PLID 30580 - disable the preview button while loading
		m_btnEMRSummaryPreview.EnableWindow(FALSE);

		// (j.jones 2008-06-25 17:49) - PLID 21168 - load EMR Summary configuration settings
		LoadConfigSettings();

		// (c.haag 2008-12-16 16:19) - PLID 32467 - Load all patient problems
		LoadPatientProblems();

		PopulatePatientList();

		// (j.jones 2008-06-13 09:55) - PLID 30385 - added patient images,
		// this must be called AFTER PopulatePatientList() which loads some
		// member variables we need
		LoadPatientImagingInfo();

		// (j.jones 2008-07-02 10:55) - PLID 30580 - enable the preview button after loading finishes
		// (though the EMN load will soon disable it once again)
		m_btnEMRSummaryPreview.EnableWindow(TRUE);

		//now load the EMN list, but do it with a timer, because if they
		//used scrollbar navigation it could potentially take a while,
		//and ideally we want to display the patient information now while
		//we are waiting
		SetTimer(TIMER_LOAD_EMN_LIST, 100, NULL);

	}NxCatchAll("Error in CEMRSummaryDlg::Load");
}

// (j.jones 2008-06-25 17:49) - PLID 21168 - load EMR Summary configuration settings
void CEMRSummaryDlg::LoadConfigSettings()
{
	try {
		
		// (j.jones 2008-06-25 17:49) - PLID 21168 - load EMR Summary configuration settings
		m_bShowAllergies = GetRemotePropertyInt("EMRSummaryShowAllergies", 1, 0, GetCurrentUserName(), true) == 1;
		m_bShowCurrentMeds = GetRemotePropertyInt("EMRSummaryShowCurrentMeds", 1, 0, GetCurrentUserName(), true) == 1;
		m_bShowPrescriptions = GetRemotePropertyInt("EMRSummaryShowPrescriptions", 1, 0, GetCurrentUserName(), true) == 1;
		m_bShowLabs = GetRemotePropertyInt("EMRSummaryShowLabs", 1, 0, GetCurrentUserName(), true) == 1;
		m_bShowProblemList = GetRemotePropertyInt("EMRSummaryShowProblemList", 1, 0, GetCurrentUserName(), true) == 1;
		m_bShowEMRDocuments = GetRemotePropertyInt("EMRSummaryShowEMRDocuments", 1, 0, GetCurrentUserName(), true) == 1;
		// (c.haag 2008-12-16 15:28) - PLID 32467
		m_bShowPtEmrProblems = GetRemotePropertyInt("EMRSummaryShowPtEmrProblems", 1, 0, GetCurrentUserName(), true) == 1;
		m_bShowPtNonEmrProblems = GetRemotePropertyInt("EMRSummaryShowPtNonEmrProblems", 1, 0, GetCurrentUserName(), true) == 1;

		m_aryCategoryIDs.RemoveAll();
		m_arystrCategoryNames.RemoveAll();

		//load EMR categories, in order
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsCategories = CreateRecordset(GetRemoteDataSnapshot(), "SELECT ID, Name "
			"FROM EMRCategoriesT "
			"WHERE SummarySortOrder Is Not Null "
			"ORDER BY SummarySortOrder");

		while(!rsCategories->eof) {

			long nID = AdoFldLong(rsCategories, "ID", -1);
			CString strName = AdoFldString(rsCategories, "Name", "");

			m_aryCategoryIDs.Add(nID);
			m_arystrCategoryNames.Add(strName);

			rsCategories->MoveNext();
		}
		rsCategories->Close();

		ASSERT(m_aryCategoryIDs.GetSize() == m_arystrCategoryNames.GetSize());

	}NxCatchAll("Error in CEMRSummaryDlg::LoadConfigSettings");
}

void CEMRSummaryDlg::GenerateEMNIDList()
{
	try {

		CWaitCursor pWait;

		if(m_nPatientID == -1) {
			m_nPatientID = GetActivePatientID();
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//TODO: should these group by EMR? Odds are they naturally will, but
		//we aren't specifically doing that right now.

		//now loop through each EMN, in date descending order

		CSqlFragment sqlWhere;

		if(m_nEMRID != -1) {
			sqlWhere += CSqlFragment(" AND EMRMasterT.EMRGroupID = {INT} ", m_nEMRID);
		}

		if(m_nEMNID != -1) {
			sqlWhere += CSqlFragment(" AND EMRMasterT.ID = {INT} ", m_nEMNID);
		}

		// (c.haag 2010-08-04 12:20) - PLID 39980 - We now do LEFT joins on PicT so that EMR's without PIC's
		// (questionable data) are still displayed and not hidden.
		// (z.manning 2011-05-20 12:03) - PLID 33114 - Filter out EMNs the current user doesn't have access to because of chart permissions
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRMasterT.ID FROM EMRMasterT "
			"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"LEFT JOIN PicT ON EMRGroupsT.ID = PicT.EMRGroupID "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"WHERE EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) "
			"AND EMRMasterT.PatientID = {INT} {SQL} {SQL} "
			"ORDER BY EMRMasterT.Date DESC, EMRMasterT.ID DESC"
			, m_nPatientID, sqlWhere, GetEmrChartPermissionFilter());

		while(!rs->eof) {

			// (r.gonet 2010-03-11 10:02) - PLID 37683 - Deleted default value of nEMNID, which was uninitialized
			long nEMNID = AdoFldLong(rs, "ID");

			m_aryEMNIDs.Add(nEMNID);

			rs->MoveNext();
		}
		rs->Close();

	}NxCatchAll("Error in GenerateEMNIDList");
}

// (j.jones 2008-06-26 09:15) - PLID 21168 - takes in an array of CEMRSummaryListInfoObjects,
// and creates a new CEMRSummaryListInfoObject at the end of the array with the given data
void CEMRSummaryDlg::AddListDataToArray(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects, 
										CString strLabel /*= ""*/, COLORREF cColor /*= RGB(0,0,0)*/)
{
	//let exceptions be thrown to the caller

	if(aryObjects == NULL) {
		return;
	}

	CEMRSummaryListInfoObject *pNew = new CEMRSummaryListInfoObject(strLabel, cColor);
	aryObjects->Add(pNew);
}

// (j.jones 2008-06-26 09:15) - PLID 21168 - safely cleans up an array of CEMRSummaryListInfoObjects
void CEMRSummaryDlg::ClearListData(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> &aryObjects)
{
	//let exceptions be thrown to the caller

	for(int i=aryObjects.GetSize()-1; i>=0; i--) {
		CEMRSummaryListInfoObject *pInfo = (CEMRSummaryListInfoObject*)(aryObjects.GetAt(i));
		delete pInfo;
	}
	aryObjects.RemoveAll();
}

void CEMRSummaryDlg::PopulatePatientList()
{
	try {

		m_PatientList->Clear();

		// (j.jones 2008-06-26 09:15) - PLID 21168 - since the lists are now configurable,
		// we will build up separate lists per column with text and potentially colors

		CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> arypListInfoLeft;
		CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> arypListInfoRight;

		//////////////////////////////////////////////////////////////////////////////////////

		//first show the patient's name, DOB, current age, Gender, and referring physician

		if(m_nPatientID == -1) {
			m_nPatientID = GetActivePatientID();
		}

		// (j.jones 2008-06-13 09:40) - PLID 30385 - added MirrorID, UnitedID, ImageIndex, and more patient fields
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PersonT.BirthDate, CASE WHEN PersonT.Gender = 2 THEN 'Female' WHEN PersonT.Gender = 1 THEN 'Male' ELSE '' END AS Gender, "
			"RefPhyT.Last + ', ' + RefPhyT.First + ' ' + RefPhyT.Middle + ' ' + RefPhyT.Title AS RefPhyName, "
			"MirrorID, UnitedID, PatientsT.ImageIndex "
			"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN PersonT RefPhyT ON PatientsT.DefaultReferringPhyID = RefPhyT.ID "
			"WHERE PatientsT.PersonID = {INT}", m_nPatientID);

		if(!rs->eof) {

			CString strPatientName = AdoFldString(rs, "PatName", "");
			CString strBirthDate = "";
			_variant_t varBD = rs->Fields->Item["BirthDate"]->Value;
			if(varBD.vt == VT_DATE) {
				COleDateTime dtBDate = VarDateTime(varBD);
				strBirthDate = FormatDateTimeForInterface(dtBDate, dtoDate);
			}
			CString strGender = AdoFldString(rs, "Gender", "");
			CString strRefPhy = AdoFldString(rs, "RefPhyName", "");

			CString strPatientInfo1, strPatientInfo2;
			strPatientInfo1.Format("%s     DOB: %s     Gender: %s",
				strPatientName, strBirthDate, strGender);
			strPatientInfo2.Format("Referring Physician: %s", strRefPhy);

			// (j.jones 2008-06-13 09:40) - PLID 30385 - added imaging values
			m_strMirrorID = AdoFldString(rs, "MirrorID", "");
			m_nUnitedID = AdoFldLong(rs, "UnitedID", -1);
			m_nImageIndex = AdoFldLong(rs, "ImageIndex", 0);

			m_nUserDefinedID = AdoFldLong(rs, "UserDefinedID", -1);
			m_strLast = AdoFldString(rs, "Last", "");
			m_strFirst = AdoFldString(rs, "First", "");
			m_strMiddle = AdoFldString(rs, "Middle", "");

			// (j.jones 2008-06-26 09:23) - PLID 21168 - use the arypListInfoLeft/Right
			AddListDataToArray(&arypListInfoLeft, strPatientInfo1);
			AddListDataToArray(&arypListInfoRight, strPatientInfo2);
		}
		rs->Close();

		//insert a blank row
		AddListDataToArray(&arypListInfoLeft);
		AddListDataToArray(&arypListInfoRight);

		//////////////////////////////////////////////////////////////////////////////////////

		//Current Allergies

		// (j.jones 2008-06-26 09:36) - PLID 21168 - displaying allergies is now configurable
		if(m_bShowAllergies) {			
			LoadAllergies(FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//Current Medications

		// (j.jones 2008-06-26 09:36) - PLID 21168 - displaying current medications is now configurable
		if(m_bShowCurrentMeds) {
			LoadCurrentMeds(FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//Medications Prescribed

		// (j.jones 2008-06-26 09:36) - PLID 21168 - displaying prescriptions is now configurable
		if(m_bShowPrescriptions) {
			LoadPrescriptions(FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//Labs

		// (j.jones 2008-06-26 09:36) - PLID 21168 - added labs information, which is configurable
		if(m_bShowLabs) {
			LoadLabs(FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		// (c.haag 2008-12-16 17:00) - PLID 32467 - EMR-independent problems
		if (m_bShowPtNonEmrProblems) {
			LoadPtNonEmrProblems(FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		// (c.haag 2008-12-16 17:00) - PLID 32467 - EMR-dependent problems
		if (m_bShowPtEmrProblems) {
			LoadPtEmrProblems(FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////


		//now build the list from the arrays

		//we may not have an equal number of values for the left and right columns,
		//so we need to add enough rows to match the largest list, and fill the
		//left and right columns appropriately
		long nCountRows = max(arypListInfoLeft.GetSize(), arypListInfoRight.GetSize());

		for(int i=0; i<nCountRows; i++) {

			CEMRSummaryListInfoObject *pLeft = NULL;
			CEMRSummaryListInfoObject *pRight = NULL;

			if(i < arypListInfoLeft.GetSize()) {
				pLeft = (CEMRSummaryListInfoObject*)(arypListInfoLeft.GetAt(i));
			}

			if(i < arypListInfoRight.GetSize()) {
				pRight = (CEMRSummaryListInfoObject*)(arypListInfoRight.GetAt(i));
			}

			COLORREF cLeft = RGB(0,0,0);
			COLORREF cRight = RGB(0,0,0);
			CString strLeftText = "";
			CString strRightText = "";

			if(pLeft != NULL) {
				strLeftText = pLeft->m_strLabel;
				cLeft = pLeft->m_cColor;
			}
			if(pRight != NULL) {
				strRightText = pRight->m_strLabel;
				cRight = pRight->m_cColor;
			}

			//now add the row
			IRowSettingsPtr pRow = m_PatientList->GetNewRow();
			pRow->PutValue(plcLeft, _bstr_t(strLeftText));
			pRow->PutValue(plcRight, _bstr_t(strRightText));
			pRow->PutCellForeColor(plcLeft, cLeft);
			pRow->PutCellForeColor(plcRight, cRight);
			m_PatientList->AddRowAtEnd(pRow, NULL);
		}

		//now clear our arrays
		ClearListData(arypListInfoLeft);
		ClearListData(arypListInfoRight);

	}NxCatchAll("Error in PopulatePatientList");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadAllergies(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL) {
			return;
		}

		if(!m_bShowAllergies) {
			return;
		}

		// (c.haag 2007-04-03 14:40) - PLID 25482 - We now get the allergy name from EmrDataT
		// since AllergyT.Name has been depreciated
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsAllergies = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT [Data] AS Name, Description FROM PatientAllergyT "
			"INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
			"INNER JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID "
			"WHERE PersonID = {INT}", m_nPatientID);

		AddListDataToArray(aryObjects, "Allergies:", RGB(0,0,192));

		if(rsAllergies->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		long nCount = 0;

		while(!rsAllergies->eof) {

			nCount++;

			CString strName = AdoFldString(rsAllergies, "Name", "");				
			CString strDescription = AdoFldString(rsAllergies, "Description", "");

			strDescription.TrimRight();

			if(!strDescription.IsEmpty())
				strDescription = " - " + strDescription;

			CString strAllergyInfo;
			strAllergyInfo.Format("%li. %s%s", nCount, strName, strDescription);

			AddListDataToArray(aryObjects, strAllergyInfo);
		
			rsAllergies->MoveNext();
		}
		rsAllergies->Close();

		//insert a blank row
		AddListDataToArray(aryObjects);

	}NxCatchAll("Error in CEMRSummaryDlg::LoadAllergies");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadCurrentMeds(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL) {
			return;
		}

		if(!m_bShowCurrentMeds)  {
			return;
		}

		// (c.haag 2007-02-02 18:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		// (j.jones 2012-01-24 10:48) - PLID 47736 - hide discontinued meds
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsCurMeds = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRDataT.Data AS Name FROM CurrentPatientMedsT "
			"INNER JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE PatientID = {INT} "
			"AND Discontinued = 0", m_nPatientID);

		AddListDataToArray(aryObjects, "Current Medications:", RGB(0,0,192));

		if(rsCurMeds->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		long nCount = 0;

		while(!rsCurMeds->eof) {

			nCount++;

			CString strName = AdoFldString(rsCurMeds, "Name", "");

			CString strCurMedInfo;
			strCurMedInfo.Format("%li. %s", nCount, strName);
		
			//store in our array
			AddListDataToArray(aryObjects, strCurMedInfo);
		
			rsCurMeds->MoveNext();
		}
		rsCurMeds->Close();

		//insert a blank row
		AddListDataToArray(aryObjects);

	}NxCatchAll("Error in CEMRSummaryDlg::LoadCurrentMeds");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadPrescriptions(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL) {
			return;
		}

		if(!m_bShowPrescriptions) {
			return;
		}

		// (c.haag 2007-02-02 18:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation, PillsPerBottle to Quantity
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsMeds = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRDataT.Data AS Name, PatientMedications.PrescriptionDate, "
			"PatientMedications.PatientExplanation, PatientMedications.RefillsAllowed, PatientMedications.Quantity "
			"FROM PatientMedications "
			"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE PatientMedications.PatientID = {INT} AND PatientMedications.Deleted = 0 "
			"ORDER BY PatientMedications.PrescriptionDate DESC ", m_nPatientID);

		AddListDataToArray(aryObjects, "Medications Prescribed:", RGB(0,0,192));

		if(rsMeds->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		long nCount = 0;

		while(!rsMeds->eof) {

			nCount++;

			CString strName = AdoFldString(rsMeds, "Name", "");				
			CString strPatientExplanation = AdoFldString(rsMeds, "PatientExplanation", "");

			strPatientExplanation.TrimRight();

			if(!strPatientExplanation.IsEmpty()) {
				strPatientExplanation = " - " + strPatientExplanation;
			}

			strPatientExplanation.Replace("\r", " ");
			strPatientExplanation.Replace("\n", " ");
			strPatientExplanation.Replace("\r\n", " ");
			
			CString strDate = "";
			_variant_t varDate = rsMeds->Fields->Item["PrescriptionDate"]->Value;
			if(varDate.vt == VT_DATE) {
				COleDateTime dt = VarDateTime(varDate);
				strDate = FormatDateTimeForInterface(varDate, dtoDate);
			}

			CString strMedInfo;
			strMedInfo.Format("%li. %s: %s%s", nCount, strDate, strName, strPatientExplanation);

			//store in our array
			AddListDataToArray(aryObjects, strMedInfo);
		
			rsMeds->MoveNext();
		}
		rsMeds->Close();

		//insert a blank row
		AddListDataToArray(aryObjects);

	}NxCatchAll("Error in CEMRSummaryDlg::LoadPrescriptions");
}

// (j.jones 2008-06-26 10:30) - PLID 21168 - copied from PatientLabsDlg, used in LoadLabs()
//TES 11/10/2009 - PLID 36260 - This is no longer an enum, but a configurable table, so the description should be loaded along with the ID.
//TES 12/8/2009 - PLID 36512 - AnatomySide is back!
CString CEMRSummaryDlg::GetAnatomySideString(long nSide)
{
	CString strSideName = "";
	try {
		switch(nSide) {
		case asLeft:
			strSideName = "Left"; //Left
			break;
		case asRight:
			strSideName = "Right"; //Right
			break;
		case asNone:
		default:
			strSideName = ""; //(No selection)
			break;
		}
	} NxCatchAll("Error in CEMRSummaryDlg::GetAnatomySideString");
	return strSideName;
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadLabs(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL) {
			return;
		}

		if(!m_bShowLabs) {
			return;
		}

		//if they don't have the license for labs, don't bother trying to load them
		if(!g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
			return;
		}

		// (j.gruber 2008-10-10 09:44) - PLID 31432 - changed to reflect new lab structure
		// (z.manning 2008-10-30 10:44) - PLID 31864 - Added to be ordered
		//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
		//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
		// (j.jones 2010-04-06 17:15) - PLID 37411 - added InitialDiagnosis & FinalDiagnosis
		// (j.jones 2010-06-29 11:22) - PLID 38944 - we now show lab requisitions only once, and
		// results are indented below
		// (c.haag 2010-12-13 16:37) - PLID 41806 - Use stored functions to get lab completed info
		//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
		CSqlFragment sqlFrag = GetAllowedLocationClause_Param("LabsT.LocationID");
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsLabs = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT LabsT.*, LabAnatomyT.Description AS AnatomicLocation, "
			"AnatomyQualifiersT.Name AS LocationQualifier, "
			"UsersT.UserName "
			"FROM ( "
			"SELECT LabsT.ID, LabsT.FormNumberTextID, LabsT.AnatomyID, LabsT.AnatomyQualifierID, "
			"LabsT.InitialDiagnosis, dbo.GetLabFinalDiagnosisList(LabsT.ID) AS FinalDiagnosis, "
			"LabsT.Specimen, LabsT.InputDate, LabsT.AnatomySide, "			
			"dbo.GetLabCompletedDate(LabsT.ID) AS LabCompletedDate, "
			"dbo.GetLabCompletedBy(LabsT.ID) AS LabCompletedBy, "
			"LabsT.ToBeOrdered, LabsT.Type "
			"FROM LabsT "
			"WHERE LabsT.PatientID = {INT} AND LabsT.Deleted = 0 AND {SQLFRAGMENT} "
			") LabsT "
			"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
			"LEFT JOIN AnatomyQualifiersT ON AnatomyQualifiersT.ID = LabsT.AnatomyQualifierID "			
			"LEFT JOIN UsersT ON LabsT.LabCompletedBy = UsersT.PersonID "
			"ORDER BY LabsT.InputDate DESC "
			" "
			"SELECT LabsT.ID AS LabID, LabResultFlagsT.Name AS ResultFlagName "
			"FROM LabResultsT "
			"INNER JOIN LabsT ON LabResultsT.LabID = LabsT.ID "
			"LEFT JOIN LabResultFlagsT ON LabResultsT.FlagID = LabResultFlagsT.ID "
			"WHERE LabsT.PatientID = {INT} AND LabsT.Deleted = 0 AND LabResultsT.Deleted = 0 "
			"AND {SQLFRAGMENT} "
			"ORDER BY LabsT.InputDate ASC, LabResultsT.DateReceived DESC, LabResultsT.ResultID DESC ",
			m_nPatientID, sqlFrag, m_nPatientID, sqlFrag);

		AddListDataToArray(aryObjects, "Labs:", RGB(0,0,192));

		if(rsLabs->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		_RecordsetPtr rsLabResults = rsLabs->NextRecordset(NULL);

		long nCount = 0;

		// (j.jones 2010-06-29 11:32) - PLID 38944 - now just show the lab info first, then
		// each of its results below

		while(!rsLabs->eof) {

			nCount++;
			long nLabID = AdoFldLong(rsLabs, "ID");
			CString strFormNumberTextID = AdoFldString(rsLabs, "FormNumberTextID", "");
			CString strSpecimen = AdoFldString(rsLabs, "Specimen", "");
			COleDateTime dtInput = AdoFldDateTime(rsLabs, "InputDate");

			_variant_t vLabCompletedDate = rsLabs->Fields->Item["LabCompletedDate"]->Value;
			BOOL bLabComplete = vLabCompletedDate.vt == VT_DATE;				
			CString strUserName = AdoFldString(rsLabs, "UserName", "");
			
			// (z.manning 2008-10-30 10:45) - PLID 31864 - Only use anatomic location on biopsy labs
			LabType eType = (LabType)AdoFldByte(rsLabs->GetFields(), "Type", ltBiopsy);
			CString strTypeDesc;
			if(eType == ltBiopsy) {
				//Assemble the anatomic location(s)
				CString strAnatomicLocation = AdoFldString(rsLabs, "AnatomicLocation", "");
				//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
				//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
				CString strLocationQualifier = AdoFldString(rsLabs, "LocationQualifier", "");
				// (z.manning 2010-04-30 17:34) - PLID 37553 - We now have a function to format the anatomic location
				strTypeDesc = ::FormatAnatomicLocation(strAnatomicLocation, strLocationQualifier, (AnatomySide)AdoFldLong(rsLabs, "AnatomySide", 0));
			}
			else {
				strTypeDesc = AdoFldString(rsLabs->GetFields(), "ToBeOrdered", "");
			}
		
			//Assemble the description for the lab
			CString strDescription;
			strDescription.Format("%s-%s - %s", strFormNumberTextID, strSpecimen, strTypeDesc);

			// (c.haag 2010-09-13 11:26) - PLID 38989 - We now show the input time as well
			CString strInputDate = FormatDateTimeForInterface(dtInput, NULL, dtoDateTime);
		
			CString strLabInfo;
			strLabInfo.Format("%li. %s (%s)", nCount, strDescription, strInputDate);

			//store in our array
			AddListDataToArray(aryObjects, strLabInfo);

			// (j.jones 2010-04-05 17:21) - PLID 37411 - added initial diagnosis & final diagnosis
			CString strInitialDiagnosis = AdoFldString(rsLabs, "InitialDiagnosis", "");
			if(!strInitialDiagnosis.IsEmpty()) {
				CString strInitialDiagnosisLine;
				strInitialDiagnosisLine.Format(" - Initial Diagnosis: %s", strInitialDiagnosis);
				//store in our array
				AddListDataToArray(aryObjects, strInitialDiagnosisLine);
			}

			CString strFinalDiagnosis = AdoFldString(rsLabs, "FinalDiagnosis", "");
			if(!strInitialDiagnosis.IsEmpty()) {
				CString strFinalDiagnosisLine;
				strFinalDiagnosisLine.Format(" - Final Diagnosis: %s", strFinalDiagnosis);
				//store in our array
				AddListDataToArray(aryObjects, strFinalDiagnosisLine);
			}			

			if(bLabComplete) {
				CString strCompleted;
				strCompleted.Format(" - Completed on %s by %s", FormatDateTimeForInterface(VarDateTime(vLabCompletedDate), NULL, dtoDate), strUserName);
				AddListDataToArray(aryObjects, strCompleted);
			}

			//add all results
			if(rsLabResults->GetRecordCount() > 0) {
				rsLabResults->MoveFirst();

				long nResultCount = 0;

				while(!rsLabResults->eof) {

					long nResultLabID = AdoFldLong(rsLabResults, "LabID");

					if(nLabID == nResultLabID) {
						nResultCount++;
						CString strResultFlag = AdoFldString(rsLabResults, "ResultFlagName", "Pending");
						CString strResultLine;
						strResultLine.Format(" - Result Flag %li: %s", nResultCount, strResultFlag);

						//store in our array
						AddListDataToArray(aryObjects, strResultLine);
					}

					rsLabResults->MoveNext();
				}
			}
			
			rsLabs->MoveNext();
		}

		rsLabs->Close();
		rsLabResults->Close();

		//insert a blank row
		AddListDataToArray(aryObjects);

	}NxCatchAll("Error in CEMRSummaryDlg::LoadLabs");
}

void CEMRSummaryDlg::LoadPtNonEmrProblems(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {
		if(aryObjects == NULL) {
			return;
		}

		AddListDataToArray(aryObjects, "EMR-Independent Problems:", RGB(0,0,192));

		// (c.haag 2008-12-16 16:52) - PLID 32467 - We now load the problems from memory
		long nCount = 0;
		for (int i=0; i < m_aAllProblems.GetSize(); i++) {
			const CEMRSummaryProblem p = m_aAllProblems.GetAt(i);
			if (0 == p.m_anEMNIDs.GetSize()) {
				CString strProblemInfo = FormatString("%li. %s", ++nCount, p.m_strDisplayText);
				AddListDataToArray(aryObjects, strProblemInfo);
			}
		}
		if (0 == nCount) {
			AddListDataToArray(aryObjects, "<None>");
		}

		//insert a blank row
		AddListDataToArray(aryObjects);
	}
	NxCatchAll("Error in CEMRSummaryDlg::LoadPtNonEmrProblems");
}

void CEMRSummaryDlg::LoadPtEmrProblems(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {
		if(aryObjects == NULL) {
			return;
		}

		AddListDataToArray(aryObjects, "EMR-Dependent Problems:", RGB(0,0,192));

		// (c.haag 2008-12-16 16:52) - PLID 32467 - We now load the problems from memory
		long nCount = 0;
		for (int i=0; i < m_aAllProblems.GetSize(); i++) {
			const CEMRSummaryProblem p = m_aAllProblems.GetAt(i);
			if (p.m_anEMNIDs.GetSize() > 0) {
				CString strProblemInfo = FormatString("%li. %s", ++nCount, p.m_strDisplayText);
				AddListDataToArray(aryObjects, strProblemInfo);
			}
		}
		if (0 == nCount) {
			AddListDataToArray(aryObjects, "<None>");
		}

		//insert a blank row
		AddListDataToArray(aryObjects);
	}
	NxCatchAll("Error in CEMRSummaryDlg::LoadPtEmrProblems");
}


// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadEMRProblemList(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL || nEMNID == -1) {
			return;
		}

		if(!m_bShowProblemList) {
			return;
		}


		AddListDataToArray(aryObjects, "Problem List:", RGB(0,0,192));

		// (c.haag 2008-12-16 16:52) - PLID 32467 - We now load the problems from memory
		long nCount = 0;
		for (int i=0; i < m_aAllProblems.GetSize(); i++) {
			const CEMRSummaryProblem p = m_aAllProblems.GetAt(i);
			if (p.IsForEMN(nEMNID)) {
				CString strProblemInfo = FormatString("%li. %s", ++nCount, p.m_strDisplayText);
				AddListDataToArray(aryObjects, strProblemInfo);
			}
		}
		if (0 == nCount) {
			AddListDataToArray(aryObjects, "<None>");
		}

		//insert a blank row
		AddListDataToArray(aryObjects, "");

	}NxCatchAll("Error in CEMRSummaryDlg::LoadEMRProblemList");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadEMRDocuments(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL || nEMNID == -1) {
			return;
		}

		if(!m_bShowEMRDocuments) {
			return;
		}
		
		// (j.jones 2008-09-05 12:38) - PLID 30288 - supported MailSentNotesT
		// (c.haag 2010-08-04 12:20) - PLID 39980 - We now do LEFT joins on PicT so that EMR's without PIC's
		// (questionable data) are still displayed and not hidden.
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsDocs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT MailSent.ServiceDate, MailSentNotesT.Note, MailSent.PathName "
			"FROM EMRMasterT "
			"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"LEFT JOIN PicT ON EMRGroupsT.ID = PicT.EMRGroupID "
			"LEFT JOIN MailSent ON PicT.ID = MailSent.PicID "
			"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
			"WHERE EMRMasterT.ID = {INT} "
			"ORDER BY ServiceDate", nEMNID);

		AddListDataToArray(aryObjects, "Attached Documents:", RGB(0,0,192));

		if(rsDocs->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		long nCount = 0;			

		while(!rsDocs->eof) {

			nCount++;

			//for some inexplicable reason, this can be null
			_variant_t varDate = rsDocs->Fields->Item["ServiceDate"]->Value;
			CString strNote = AdoFldString(rsDocs, "Note", "");
			CString strPathName = AdoFldString(rsDocs, "PathName", "");

			CString strDescription = strNote;
			if(strDescription.IsEmpty()) {
				strDescription = strPathName;			
			}

			CString strDiagInfo;
			if(varDate.vt == VT_DATE) {
				strDiagInfo.Format("%li. %s (%s)", nCount, strDescription, FormatDateTimeForInterface(VarDateTime(varDate), NULL, dtoDate));
			}
			else {
				strDiagInfo.Format("%li. %s", nCount, strDescription);
			}

			AddListDataToArray(aryObjects, strDiagInfo);
		
			rsDocs->MoveNext();
		}
		rsDocs->Close();

		//insert a blank row
		AddListDataToArray(aryObjects, "");

	}NxCatchAll("Error in CEMRSummaryDlg::LoadEMRDocuments");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
// (j.jones 2008-10-30 12:02) - PLID 31857 - added an EMN pointer, NULL by default, only loaded if any Narrative or Table details are found
void CEMRSummaryDlg::LoadEMRCategory(long nEMNID, CEMN *&pEMN, long nCategoryID, CString strCategoryName, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL || nCategoryID == -1 || nEMNID == -1) {
			return;
		}

		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsDetails = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRDetailsT.ID, EMRInfoT.DataType "
			"FROM EMRDetailsT "
			"INNER JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			"INNER JOIN EMRInfoCategoryT ON EMRInfoT.ID = EMRInfoCategoryT.EMRInfoID "
			"WHERE EMRDetailsT.Deleted = 0 "
			"AND EMRDetailsT.EMRID = {INT} "
			"AND EMRInfoCategoryT.EMRCategoryID = {INT}", nEMNID, nCategoryID);

		AddListDataToArray(aryObjects, strCategoryName + ":", RGB(0,0,192));

		if(rsDetails->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		CArray<CEMNDetail*, CEMNDetail*> aryDetails;
		CArray<CEMNDetail*, CEMNDetail*> aryLocalDetails;

		while(!rsDetails->eof) {

			long nDetailID = AdoFldLong(rsDetails, "ID");
			EmrInfoType eDataType = (EmrInfoType)AdoFldByte(rsDetails, "DataType");

			// (j.jones 2008-10-30 11:40) - PLID 31857 - If we have a Narrative or a Table,
			// we need to load the entire EMN, otherwise our results won't be reliable
			// (narratives won't show their linked items, tables won't show linked details).
			// pEMN is passed to us such that we only have to load it once.
			if(pEMN == NULL && (eDataType == eitNarrative || eDataType == eitTable)) {
				pEMN = new CEMN(NULL);
				pEMN->LoadFromEmnID(nEMNID);
			}

			CEMNDetail *pDetail = NULL;
			// (j.jones 2008-10-30 11:43) - PLID 31857 - if we have an EMN, just find the already-loaded detail in it
			if(pEMN) {
				pDetail = pEMN->GetDetailByID(nDetailID);
			}

			//if we have no detail, load it now, which is way faster
			//than always loading the EMN
			BOOL bIsLocal = FALSE;
			if(pDetail == NULL) {
				// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
				pDetail = CEMNDetail::CreateDetail(NULL, "EMR summary local detail");
				// Load the detail
				pDetail->LoadFromDetailID(nDetailID, NULL);
				bIsLocal = TRUE;
			}
				
			//add to our array
			aryDetails.Add(pDetail);

			//if we created it locally, we need to delete it later,
			//so track it in a separate array
			if(bIsLocal) {
				aryLocalDetails.Add(pDetail);
			}

			rsDetails->MoveNext();
		}
		rsDetails->Close();

		if(aryDetails.GetSize() > 0) {

			if(aryDetails.GetSize() > 1) {
				//this will sort the details in order of topic, and then placement inside a topic
				SortDetailArray(aryDetails);
			}

			int i=0;
			for(i=0; i<aryDetails.GetSize(); i++) {

				CEMNDetail *pDetail = (CEMNDetail*)(aryDetails.GetAt(i));

				CString strName = pDetail->GetMergeFieldName(FALSE);
				CString strSentence;
				//copied roughly from the problem list dlg
				if(pDetail->GetStateVarType() == VT_NULL || (pDetail->GetStateVarType() == VT_BSTR && VarString(pDetail->GetState()).IsEmpty())) {
					//TES 2/26/2010 - PLID 37463 - The m_strLongForm variable has never been set at this point, and having the sentence
					// be blank for empty details seems like a better idea anyway (certainly nobody has complained).  So let's just
					// explicitly make it blank.
					strSentence = "";//pDetail->m_strLongForm;
				} else {
					CStringArray saDummy;
					// (c.haag 2006-11-14 10:49) - PLID 23543 - If the info type is an image, we may get a debug assertion
					// failure when calling GetSentence. Rather than try to get a sentence, just return a sentinel value.
					if (eitImage == pDetail->m_EMRInfoType) {
						strSentence = "<image>";
					} else {
						// (c.haag 2008-02-22 13:53) - PLID 29064 - GetSentence may access the database when doing calculations on
						// dropdown table columns. Make sure we pass in our connection object so it won't try to use the global one
						// which belongs to the main thread.
						strSentence = ::GetSentence(pDetail, NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL);
					}
				}
		
				CString strCategoryData;
				strCategoryData.Format("%li. %s: %s", i+1, strName, strSentence);

				AddListDataToArray(aryObjects, strCategoryData);
			}
		}

		// (j.jones 2008-10-30 11:41) - PLID 31857 - We don't need to clear the entire detail list
		// because it would be handled by deleting the EMN, and deleting the EMN is handled by the
		// calling function. But we may have created details on our own, and if so, they would be
		// in the aryLocalDetails array. Delete those details now.
		int i=0;
		for(i=aryLocalDetails.GetSize()-1; i>=0; i--) {
			CEMNDetail *pDetail = (CEMNDetail*)(aryLocalDetails.GetAt(i));
			// (a.walling 2009-10-12 17:20) - PLID 36024 - Properly release the detail
			pDetail->__QuietRelease();
			//delete pDetail;
		}
		aryLocalDetails.RemoveAll();
		aryDetails.RemoveAll();

		//insert a blank row
		AddListDataToArray(aryObjects, "");

	}NxCatchAll("Error in CEMRSummaryDlg::LoadEMRCategory");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadEMRDiagList(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL || nEMNID == -1) {
			return;
		}

		// (j.jones 2007-01-05 10:02) - PLID 24070 - supported OrderIndex
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		// (r.gonet 03/01/2014) - PLID 60760 - Added support for ICD10 codes.
		_RecordsetPtr rsDiags = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT ICD9DiagCodes.ID  AS ICD9ID,  ICD9DiagCodes.CodeNumber  AS ICD9CodeNumber,  ICD9DiagCodes.CodeDesc  AS ICD9CodeDesc, "
			"       ICD10DiagCodes.ID AS ICD10ID, ICD10DiagCodes.CodeNumber AS ICD10CodeNumber, ICD10DiagCodes.CodeDesc AS ICD10CodeDesc "
			"FROM EMRDiagCodesT "
			"LEFT JOIN DiagCodes ICD9DiagCodes ON EMRDiagCodesT.DiagCodeID = ICD9DiagCodes.ID "
			"LEFT JOIN DiagCodes ICD10DiagCodes ON EMRDiagCodesT.DiagCodeID_ICD10 = ICD10DiagCodes.ID "
			"WHERE EMRDiagCodesT.EMRID = {INT} AND EMRDiagCodesT.Deleted = 0 AND (ICD9DiagCodes.ID IS NOT NULL OR ICD10DiagCodes.ID IS NOT NULL) "
			"ORDER BY OrderIndex", nEMNID);


		AddListDataToArray(aryObjects, "Diagnosis Codes:", RGB(0,0,192));

		if(rsDiags->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		long nCount = 0;			

		while(!rsDiags->eof) {

			nCount++;

			// (r.gonet 03/01/2014) - PLID 60760 - Added support for ICD-10 codes.
			long nICD9DiagCodeID = AdoFldLong(rsDiags, "ICD9ID", -1);
			CString strICD9DiagCode = AdoFldString(rsDiags, "ICD9CodeNumber", "");
			CString strICD9DiagDesc = AdoFldString(rsDiags, "ICD9CodeDesc", "");
			long nICD10DiagCodeID = AdoFldLong(rsDiags, "ICD10ID", -1);
			CString strICD10DiagCode = AdoFldString(rsDiags, "ICD10CodeNumber", "");
			CString strICD10DiagDesc = AdoFldString(rsDiags, "ICD10CodeDesc", "");
			
			if(nICD9DiagCodeID == -1 && nICD10DiagCodeID == -1) {
				// (r.gonet 03/01/2014) - PLID 60760 - We have a condition in the diagnosis code SQL to prevent this but without that,
				// there is nothing stopping both DiagCodeID fields from being null. That would be bad data though.
				ThrowNxException("%s : Both the ICD-9 and ICD-10 codes are null!", __FUNCTION__);
			}
			// (r.gonet 03/01/2014) - PLID 60760 - According to the requirements, we need to show the ICD-9 code first and then the ICD-10 code on the next line.
			//  If one is missing, then just show the remaining one on one line. Note that we do not respect the global diagnosis search preference here.
			CString strDiagInfo = FormatString("%li. ", nCount);
			if(nICD9DiagCodeID != -1) {
				strDiagInfo += FormatString("%s - %s", strICD9DiagCode, strICD9DiagDesc);
			}
			if(nICD10DiagCodeID != -1) {
				if(nICD9DiagCodeID != -1) {
					// (r.gonet 03/01/2014) - PLID 60760 - Without a fixed width font, this will never be perfect. However it comes pretty close in the default font when there are less than
					// 10 diagnosis codes, which is probably the common case.
					strDiagInfo += FormatString("\r\n     ");
				}
				strDiagInfo += FormatString("%s - %s", strICD10DiagCode, strICD10DiagDesc);
			}

			AddListDataToArray(aryObjects, strDiagInfo);
		
			rsDiags->MoveNext();
		}
		rsDiags->Close();

		//insert a blank row
		AddListDataToArray(aryObjects, "");

	}NxCatchAll("Error in CEMRSummaryDlg::LoadEMRDiagList");
}

// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
void CEMRSummaryDlg::LoadEMRPrescriptionList(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects)
{
	try {

		if(aryObjects == NULL || nEMNID == -1) {
			return;
		}

		// (c.haag 2007-02-02 18:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		//TES 2/10/2009 - PLID 33034 - Renamed Description to PatientExplanation, PillsPerBottle to Quantity
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rsMeds = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRDataT.Data AS Name, PatientMedications.PrescriptionDate, "
			"PatientMedications.PatientExplanation, PatientMedications.RefillsAllowed, PatientMedications.Quantity "
			"FROM EMRMedicationsT "
			"INNER JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
			"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE EMRMedicationsT.EMRID = {INT} AND EMRMedicationsT.Deleted = 0 AND PatientMedications.Deleted = 0", nEMNID);

		AddListDataToArray(aryObjects, "Medications Prescribed:", RGB(0,0,192));

		if(rsMeds->eof) {
			AddListDataToArray(aryObjects, "<None>");
		}

		long nCount = 0;	

		while(!rsMeds->eof) {

			nCount++;

			CString strName = AdoFldString(rsMeds, "Name", "");				
			CString strPatientExplanation = AdoFldString(rsMeds, "PatientExplanation", "");

			strPatientExplanation.TrimRight();

			if(!strPatientExplanation.IsEmpty()) {
				strPatientExplanation = " - " + strPatientExplanation;
			}

			strPatientExplanation.Replace("\r", " ");
			strPatientExplanation.Replace("\n", " ");
			strPatientExplanation.Replace("\r\n", " ");

			CString strMedInfo;
			strMedInfo.Format("%li. %s%s", nCount, strName, strPatientExplanation);

			AddListDataToArray(aryObjects, strMedInfo);
		
			rsMeds->MoveNext();
		}
		rsMeds->Close();

		//insert a blank row
		AddListDataToArray(aryObjects, "");

	}NxCatchAll("Error in CEMRSummaryDlg::LoadEMRPrescriptionList");
}


void CEMRSummaryDlg::PopulateEMNList()
{
	try {

		CWaitCursor pWait;

		// (j.jones 2008-07-02 10:55) - PLID 30580 - disable the preview button while loading
		m_btnEMRSummaryPreview.EnableWindow(FALSE);
	
		m_EMNList->Clear();

		 //only process if we actually have any EMNs
		if(m_aryEMNIDs.GetSize() > 0) {

			//if showing the arrows, then only load one EMN
			if(m_bDisplayArrows) {
				m_nCurEMNIndex = 0;
				AddToEMNList(m_aryEMNIDs.GetAt(m_nCurEMNIndex));
			}
			else {
				//otherwise, add all EMNs, with dividers in between

				m_nCurEMNIndex = -1;

				for(int i=0; i<m_aryEMNIDs.GetSize(); i++) {				
					AddToEMNList(m_aryEMNIDs.GetAt(i));
				}
			}
		}

		UpdateArrows();

		// (j.jones 2008-07-02 10:55) - PLID 30580 - enable the preview button after loading finishes
		m_btnEMRSummaryPreview.EnableWindow(TRUE);
	
	}NxCatchAll("Error in PopulateEMNList");
}

void CEMRSummaryDlg::AddToEMNList(long nEMNID)
{
	try {

		if(m_nPatientID == -1) {
			m_nPatientID = GetActivePatientID();
		}

		if(m_EMNList->GetRowCount() > 0) {		

			//insert a gray row
			IRowSettingsPtr pRow = m_EMNList->GetNewRow();
			pRow->PutValue(elcLeft, _bstr_t(""));
			pRow->PutValue(elcRight, _bstr_t(""));
			pRow->PutBackColor(GetSysColor(COLOR_3DFACE));
			m_EMNList->AddRowAtEnd(pRow, NULL);
		}

		// (j.jones 2008-06-26 15:47) - PLID 21168 - we now build up separate lists per column
		// with text and potentially colors

		CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> arypListInfoLeft;
		CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> arypListInfoRight;

		// (c.haag 2010-08-04 12:20) - PLID 39980 - We now do LEFT joins on PicT so that EMR's without PIC's
		// (questionable data) are still displayed and not hidden.
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EMRMasterT.Date, EMRMasterT.Description AS EMNDescription, "
			"EMRGroupsT.Description AS EMRDescription "
			"FROM EMRMasterT "
			"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"LEFT JOIN PicT ON EMRGroupsT.ID = PicT.EMRGroupID "
			"WHERE EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) "
			"AND EMRMasterT.PatientID = {INT} AND EMRMasterT.ID = {INT}", m_nPatientID, nEMNID);

		if(!rs->eof) {

			//for each EMN:

			//////////////////////////////////////////////////////////////////////////////////////

			//show the EMN date, description, EMR description
			CString strDate = "";
			_variant_t varDate = rs->Fields->Item["Date"]->Value;
			if(varDate.vt == VT_DATE) {
				COleDateTime dt = VarDateTime(varDate);
				strDate = FormatDateTimeForInterface(varDate, dtoDate);
			}
			CString strEMNDescription = AdoFldString(rs, "EMNDescription", "");
			CString strEMRDescription = AdoFldString(rs, "EMRDescription", "");

			CString strEMNInfo;
			strEMNInfo.Format("EMN Date: %s     Description: %s", strDate, strEMNDescription);

			// (j.jones 2008-06-26 15:50) - PLID 21168 - use the arypListInfoLeft/Right
			AddListDataToArray(&arypListInfoLeft, strEMNInfo);
			AddListDataToArray(&arypListInfoRight, "");

			CString strEMRInfo;
			strEMRInfo.Format("EMR Description: %s", strEMRDescription);

			// (j.jones 2008-06-26 15:50) - PLID 21168 - use the arypListInfoLeft/Right
			AddListDataToArray(&arypListInfoLeft, strEMRInfo);
			AddListDataToArray(&arypListInfoRight, "");

			//insert a blank row
			AddListDataToArray(&arypListInfoLeft);
			AddListDataToArray(&arypListInfoRight);
		}
		rs->Close();

		//////////////////////////////////////////////////////////////////////////////////////

		//Categories

		// (j.jones 2008-06-27 09:12) - PLID 21168 - added support for EMR categories

		CEMN *pEMN = NULL;

		int i=0;
		for(i=0; i<m_aryCategoryIDs.GetSize(); i++) {

			long nCategoryID = m_aryCategoryIDs.GetAt(i);
			CString strName = m_arystrCategoryNames.GetAt(i);

			//always load into the left list, as we will load these first every time,
			//and we want all categories to show in the same column

			// (j.jones 2008-10-30 12:03) - PLID 31857 - we now have an EMN pointer, which
			// will be loaded if any detail is loaded, and passed back to us for re-use
			LoadEMRCategory(nEMNID, pEMN, nCategoryID, strName, &arypListInfoLeft);
		}

		// (j.jones 2008-10-30 12:04) - PLID 31857 - if we have an EMN pointer,
		// delete it now
		if(pEMN) {
			delete pEMN;
			pEMN = NULL;
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//Problem List

		// (j.jones 2008-06-26 11:28) - PLID 21168 - load the problems, if the configuration tells us to
		if(m_bShowProblemList) {
			LoadEMRProblemList(nEMNID, FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//Diagnosis Codes and their descriptions

		// (j.jones 2008-06-26 15:53) - PLID 21168 - moved to its own function
		LoadEMRDiagList(nEMNID, FindShortestList(arypListInfoLeft, arypListInfoRight));

		//////////////////////////////////////////////////////////////////////////////////////

		//Medications Prescribed

		LoadEMRPrescriptionList(nEMNID, FindShortestList(arypListInfoLeft, arypListInfoRight));

		//////////////////////////////////////////////////////////////////////////////////////

		//Document List

		// (j.jones 2008-06-26 11:28) - PLID 21168 - load the documents, if the configuration tells us to
		if(m_bShowEMRDocuments) {
			LoadEMRDocuments(nEMNID, FindShortestList(arypListInfoLeft, arypListInfoRight));
		}

		//////////////////////////////////////////////////////////////////////////////////////

		//now build the list from the arrays

		//we may not have an equal number of values for the left and right columns,
		//so we need to add enough rows to match the largest list, and fill the
		//left and right columns appropriately
		long nCountRows = max(arypListInfoLeft.GetSize(), arypListInfoRight.GetSize());

		for(i=0; i<nCountRows; i++) {

			CEMRSummaryListInfoObject *pLeft = NULL;
			CEMRSummaryListInfoObject *pRight = NULL;

			if(i < arypListInfoLeft.GetSize()) {
				pLeft = (CEMRSummaryListInfoObject*)(arypListInfoLeft.GetAt(i));
			}

			if(i < arypListInfoRight.GetSize()) {
				pRight = (CEMRSummaryListInfoObject*)(arypListInfoRight.GetAt(i));
			}

			COLORREF cLeft = RGB(0,0,0);
			COLORREF cRight = RGB(0,0,0);
			CString strLeftText = "";
			CString strRightText = "";

			if(pLeft != NULL) {
				strLeftText = pLeft->m_strLabel;
				cLeft = pLeft->m_cColor;
			}
			if(pRight != NULL) {
				strRightText = pRight->m_strLabel;
				cRight = pRight->m_cColor;
			}

			//now add the row
			IRowSettingsPtr pRow = m_EMNList->GetNewRow();
			pRow->PutValue(elcLeft, _bstr_t(strLeftText));
			pRow->PutValue(elcRight, _bstr_t(strRightText));
			pRow->PutCellForeColor(elcLeft, cLeft);
			pRow->PutCellForeColor(elcRight, cRight);
			m_EMNList->AddRowAtEnd(pRow, NULL);
		}

		//now clear our arrays
		ClearListData(arypListInfoLeft);
		ClearListData(arypListInfoRight);

	}NxCatchAll("Error in PopulateEMNList");
}

void CEMRSummaryDlg::OnBtnPrevEmn() 
{
	try {

		if(m_nCurEMNIndex <= 0) {
			UpdateArrows();
			return;
		}

		CWaitCursor pWait;

		// (j.jones 2008-07-02 10:55) - PLID 30580 - disable the preview button while loading
		m_btnEMRSummaryPreview.EnableWindow(FALSE);

		m_EMNList->Clear();

		m_nCurEMNIndex--;
		AddToEMNList(m_aryEMNIDs.GetAt(m_nCurEMNIndex));

		UpdateArrows();

		// (j.jones 2008-07-02 10:55) - PLID 30580 - enable the preview button after loading finishes
		m_btnEMRSummaryPreview.EnableWindow(TRUE);

	}NxCatchAll("Error in CEMRSummaryDlg::OnBtnPrevEmn");
}

void CEMRSummaryDlg::OnBtnNextEmn() 
{
	try {

		if(m_nCurEMNIndex >= m_aryEMNIDs.GetSize()) {
			UpdateArrows();
			return;
		}

		CWaitCursor pWait;

		// (j.jones 2008-07-02 10:55) - PLID 30580 - disable the preview button while loading
		m_btnEMRSummaryPreview.EnableWindow(FALSE);

		m_EMNList->Clear();

		m_nCurEMNIndex++;
		AddToEMNList(m_aryEMNIDs.GetAt(m_nCurEMNIndex));

		UpdateArrows();

		// (j.jones 2008-07-02 10:55) - PLID 30580 - enable the preview button after loading finishes
		m_btnEMRSummaryPreview.EnableWindow(TRUE);

	}NxCatchAll("Error in CEMRSummaryDlg::OnBtnNextEmn");
}

void CEMRSummaryDlg::UpdateArrows()
{
	if(m_aryEMNIDs.GetSize() <= 1) {
		GetDlgItem(IDC_BTN_PREV_EMN)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_NEXT_EMN)->EnableWindow(FALSE);
	}
	else {
		if(m_nCurEMNIndex <= 0) {
			GetDlgItem(IDC_BTN_PREV_EMN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_NEXT_EMN)->EnableWindow(TRUE);
		}
		else if(m_nCurEMNIndex >= m_aryEMNIDs.GetSize()-1) {
			GetDlgItem(IDC_BTN_NEXT_EMN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_PREV_EMN)->EnableWindow(TRUE);
		}
		else if(m_nCurEMNIndex > 0 && m_nCurEMNIndex < m_aryEMNIDs.GetSize()-1) {
			GetDlgItem(IDC_BTN_PREV_EMN)->EnableWindow(TRUE);
			GetDlgItem(IDC_BTN_NEXT_EMN)->EnableWindow(TRUE);
		}
	}

}

void CEMRSummaryDlg::OnToggleViewType() 
{
	try {
		//change the view and save the setting

		// (z.manning 2010-01-12 14:38) - PLID 31945 - Since scrollbar navigation is slow much slower
		// let's give a warning when they try to use it.
		if(m_bDisplayArrows) {
			int nResult = DontShowMeAgain(this, "Warning: Scrollbar navigation mode loads all EMR data for the current patient so loading may take a while on large accounts.\r\n\r\n"
				"Are you sure you want to continue?"
				, "EmrSummaryScrollbarNavigationWarning", "", FALSE, TRUE);

			// (j.dinatale 2013-05-13 10:14) - PLID 53433 - we need to be looking for IDOK here, because if the user set do not remind me in the past,
			//		the dialog returns IDOK to indicate as such
			if(nResult != IDYES && nResult != IDOK) {
				return;
			}
		}

		m_bDisplayArrows = !m_bDisplayArrows;

		ToggleNavigationType();

		SetRemotePropertyInt("EMRSummaryShowArrows", m_bDisplayArrows ? 1 : 0, 0, GetCurrentUserName());

		//now redraw appropriately

		PopulateEMNList();
	
	}NxCatchAll("Error in CEMRSummaryDlg::OnToggleViewType");
}

void CEMRSummaryDlg::OnEmrSummaryPreview() 
{
	try {

		CWaitCursor pWait;

		// (j.gruber 2007-02-26 12:10) - PLID 24609 run the report
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(584)]);

		// (j.jones 2008-07-02 09:26) - PLID 30580 - the summary is now a raw dump of the datalists,
		// such that it matches the current view exactly

		long nCount = 0;

		//we have to create a temp table to run the report off of
		CString strTempTableName;
		strTempTableName.Format("#TempEMRSummaryT_%lu", GetTickCount());

		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "CREATE TABLE %s ("
			"ID INT NOT NULL PRIMARY KEY, "
			"LeftText ntext, "
			"RightText ntext, "
			"IsLeftHeader bit NOT NULL DEFAULT 0, "
			"IsRightHeader bit NOT NULL DEFAULT 0, "
			"IsDivider bit NOT NULL DEFAULT 0 "
			") ", strTempTableName);

		//first get the patient information
		{
			IRowSettingsPtr	pRow = m_PatientList->GetFirstRow();
			while(pRow) {

				nCount++;

				CString strLeft = VarString(pRow->GetValue(plcLeft), "");
				CString strRight = VarString(pRow->GetValue(plcRight), "");

				OLE_COLOR cLeft = pRow->GetCellForeColor(plcLeft);
				OLE_COLOR cRight = pRow->GetCellForeColor(plcRight);

				//insert into our temp table
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO %s "
					"(ID, LeftText, RightText, IsLeftHeader, IsRightHeader, IsDivider) "
					"VALUES (%li, '%s', '%s', %li, %li, 0)",
					strTempTableName, nCount, _Q(strLeft), _Q(strRight), cLeft == 0 ? 0 : 1, cRight == 0 ? 0 : 1);

				pRow = pRow->GetNextRow();
			}
		}

		//add a divider
		if(nCount > 0) {
			
			nCount++;

			//insert into our temp table
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO %s "
				"(ID, LeftText, RightText, IsLeftHeader, IsRightHeader, IsDivider) "
				"VALUES (%li, '', '', 0, 0, 1)",
				strTempTableName, nCount);
		}

		//now get the EMN information, which may be one EMN or all EMNs,
		//depending on the current view
		{
			IRowSettingsPtr	pRow = m_EMNList->GetFirstRow();
			while(pRow) {

				nCount++;

				CString strLeft = VarString(pRow->GetValue(elcLeft), "");
				CString strRight = VarString(pRow->GetValue(elcRight), "");

				OLE_COLOR cLeft = pRow->GetCellForeColor(plcLeft);
				OLE_COLOR cRight = pRow->GetCellForeColor(plcRight);

				OLE_COLOR cBack = pRow->GetBackColor();
				BOOL bIsDivider = cBack != 1;

				//insert into our temp table
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO %s "
					"(ID, LeftText, RightText, IsLeftHeader, IsRightHeader, IsDivider) "
					"VALUES (%li, '%s', '%s', %li, %li, %li)",
					strTempTableName, nCount, _Q(strLeft), _Q(strRight), cLeft == 0 ? 0 : 1, cRight == 0 ? 0 : 1, bIsDivider ? 1 : 0);

				pRow = pRow->GetNextRow();
			}
		}

		if(nCount == 0) {
			AfxMessageBox("There is no information to preview.");
			return;
		}

		// (a.walling 2009-09-08 13:32) - PLID 35178 - Run in snapshot connection
		ExecuteSqlBatch(GetRemoteDataSnapshot(), strSqlBatch);

		infReport.strExtendedSql = strTempTableName;

		RunReport(&infReport, TRUE, (CWnd *)this, infReport.strReportName);

		//now drop the table - were an exception to be thrown previously, the table would auto-drop
		//when practice closed, but since this code is self-contained, we can drop it ourselves
		ExecuteSql(GetRemoteDataSnapshot(), "DROP TABLE %s", strTempTableName);

		//close the window
		OnOK();

	}NxCatchAll("Error in CEMRSummaryDlg::OnEmrSummaryPreview");	
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - added support for images
void CEMRSummaryDlg::OnPtImageLast() 
{
	try {

		CWaitCursor pWait;

		m_nImageIndex--;
		LoadPatientImage(eImageBack);
		ExecuteParamSql("UPDATE PatientsT SET ImageIndex = {INT} WHERE PersonID = {INT}", 
			m_nImageIndex, m_nPatientID);

	}NxCatchAll("Error in CEMRSummaryDlg::OnPtImageLast");	
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - added support for images
void CEMRSummaryDlg::OnPtImageNext() 
{
	try {

		CWaitCursor pWait;

		m_nImageIndex++;
		LoadPatientImage();
		ExecuteParamSql("UPDATE PatientsT SET ImageIndex = {INT} WHERE PersonID = {INT}", 
			m_nImageIndex, m_nPatientID);

	}NxCatchAll("Error in CEMRSummaryDlg::OnPtImageNext");	
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::LoadPatientImagingInfo()
{
	try {

		// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
		// same-named function in CGeneral1Dlg also needs changed

		// This will be used later to change the interface depending on if this patient in attached to united or mirror
		bool bHasImaging = false;
		bool bShowImage = true;

		// Reset the error status of the image button
		m_imageButton.m_nError = eNoError;

		// Do they have United?
		m_nUnitedID = -1;
		if (g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrUse) && IsUnitedEnabled()) {
			
			// Yes, they have United

			// Figure out if this patient has a United record
			CMainFrame *pMainFrame = GetMainFrame();
			if (pMainFrame) {
				CUnitedLink *pUnitedLink = pMainFrame->GetUnitedLink();
				if (pUnitedLink && pUnitedLink->GetRemotePath() != "") {
					// Is this patient attached to United?
					if (m_nUnitedID != -1) {
						bHasImaging = true;
					}
				}
			}
		}

		// Do they have Mirror?
		if (Mirror::HasMirrorLinkLicense() && Mirror::IsMirrorEnabled())
		{
			// Yes, they have Mirror
			// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
			const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
			if (!bUsingSDKFunctionality) {
				// Does the mirror database even exist?
				if (GetFileAttributes(Mirror::GetMirrorDataPath()) == -1) {
					// Uh-oh. Lets tell the user in the form of a
					// thumbnail of text.
					m_imageButton.m_nError = eErrorUnspecified;
					GetDlgItem(IDC_PT_IMAGE_LAST)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_PT_IMAGE_NEXT)->ShowWindow(SW_HIDE);
					ShowImagingButtons(1);
				}
			}

			// Is this patient attached to Mirror?
			if (m_strMirrorID != "") {
				// Yes, this patient is already attached to Mirror
				bHasImaging = true;
			}
		}

		// Do they have pictures in the history tab?
		if (GetPatientAttachedImageCount(m_nPatientID) > 0) {
			bHasImaging = true;
		}
		
		if (bHasImaging) {
			// Show the default image (LoadImage() also sets the enabled states of the prev/next buttons)
			LoadPatientImage();
		} else {
			// Change interface depending on if the patient is attached to imaging software
			GetDlgItem(IDC_PT_IMAGE_LAST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PT_IMAGE_NEXT)->ShowWindow(SW_HIDE);
			ShowImagingButtons(0);
		}

	}NxCatchAll("Error in CEMRSummaryDlg::LoadPatientImagingInfo");
}

// (c.haag 2008-12-16 16:20) - PLID 32467 - Load all problems for this patient
void CEMRSummaryDlg::LoadPatientProblems()
{
	try {
		// Load all the problems for this patient. This is actually a bit tricky because we also need
		// to pull EMN ID's for each problem. Problems not associated with an EMR will have none. EMR's
		// will have multiple, and everything else has only one. To get the EMN ID's of the EMR, we have
		// to run a second query.

		// Optimization: Don't bother if the user doesn't want to see problems
		if (!m_bShowProblemList && !m_bShowPtEmrProblems && !m_bShowPtNonEmrProblems) {
			m_aAllProblems.RemoveAll();
			return;
		}

		// (c.haag 2009-05-13 11:17) - PLID 34234 - Use the new EMR problem structure
		// (j.jones 2009-05-27 09:52) - PLID 34352 - supported lab problems
		//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
		// (j.jones 2014-03-24 10:06) - PLID 61507 - if a problem is linked to a diagnosis, show the 9 and 10 code if both exist
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT EMRProblemsT.Description, "
			"EMRRegardingType, "
			"EMRProblemStatusT.Name AS Status, "
			"CASE WHEN EMRRegardingType = {CONST_INT} THEN 'EMR Item' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'List Item' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'Topic' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'EMN' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'EMR' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'Diag Code' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'Charge' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'Medication' "
			"WHEN EMRRegardingType = {CONST_INT} THEN 'Lab' "
			"ELSE 'Item' END "
			"AS ProblemType, "
			""
			"CASE WHEN (EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT}) THEN "
			"	CASE WHEN MergeOverride IS NULL THEN (CASE WHEN EMRInfoT.ID = -27 THEN MacroName ELSE EMRInfoT.Name END) ELSE MergeOverride END "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemTopic.Name "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemMaster.Description "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemGroup.Description "
			"WHEN EMRRegardingType = {CONST_INT} THEN "
			"	CASE WHEN EMRDiagCodes9.ID Is Not Null AND EMRDiagCodes10.ID Is Not Null THEN EMRDiagCodes10.CodeNumber + ' (' + EMRDiagCodes9.CodeNumber + ')' "
			"	ELSE Coalesce(EMRDiagCodes10.CodeNumber, EMRDiagCodes9.CodeNumber) END "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRChargesT.Description "
			"WHEN EMRRegardingType = {CONST_INT} THEN DrugDataT.Data "
			"WHEN EMRRegardingType = {CONST_INT} THEN '<None>' "
			"WHEN EMRRegardingType = {CONST_INT} THEN COALESCE(LabsT.FormNumberTextID, '') + '-' + COALESCE(LabsT.Specimen, '') + ' - ' + CASE WHEN LabsT.Type = {INT} THEN COALESCE(LabAnatomyT.Description, '') ELSE LabsT.ToBeOrdered END "
			"ELSE '<Unknown>' END AS ProblemItemName, "
			""
			"CASE WHEN (EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT}) THEN EMRDetailsT.EMRID "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemTopic.EMRID "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemMaster.ID "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRProblemLinkT.EMRRegardingID "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRDiagCodesT.EMRID "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRChargesT.EMRID "
			"WHEN EMRRegardingType = {CONST_INT} THEN EMRMedicationsT.EMRID "
			"WHEN EMRRegardingType = {CONST_INT} OR EMRRegardingType = {CONST_INT} THEN NULL "
			"ELSE '<Unknown>' END AS EMNID "
			""
			"FROM EMRProblemsT "
			"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "

			"LEFT JOIN EMRProblemStatusT ON EMRProblemStatusT.ID = EMRProblemsT.StatusID "
			""
			"LEFT JOIN EMRDetailsT ON EMRDetailsT.ID = EMRProblemLinkT.EMRRegardingID "
			"LEFT JOIN EMRInfoT ON EMRDetailsT.EMRInfoID = EMRInfoT.ID "
			""
			"LEFT JOIN EMRTopicsT AS EMRProblemTopic ON EMRProblemLinkT.EMRRegardingID = EMRProblemTopic.ID "
			""
			"LEFT JOIN EMRMasterT AS EMRProblemMaster ON EMRProblemMaster.ID = EMRProblemLinkT.EMRRegardingID "
			""
			"LEFT JOIN EMRGroupsT AS EMRProblemGroup ON EMRProblemGroup.ID = EMRProblemLinkT.EMRRegardingID "
			""			
			"LEFT JOIN EMRDiagCodesT ON EMRProblemLinkT.EMRRegardingID = EMRDiagCodesT.ID "
			"LEFT JOIN DiagCodes EMRDiagCodes9 ON EMRDiagCodes9.ID = EMRDiagCodesT.DiagCodeID "
			"LEFT JOIN DiagCodes EMRDiagCodes10 ON EMRDiagCodes10.ID = EMRDiagCodesT.DiagCodeID_ICD10 "
			""			
			"LEFT JOIN EMRChargesT ON EMRProblemLinkT.EMRRegardingID = EMRChargesT.ID "
			""
			"LEFT JOIN EMRMedicationsT ON EMRProblemLinkT.EMRRegardingID = EMRMedicationsT.MedicationID "
			"LEFT JOIN PatientMedications ON EMRMedicationsT.MedicationID = PatientMedications.ID "
			"LEFT JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"LEFT JOIN EMRDataT AS DrugDataT ON DrugList.EMRDataID = DrugDataT.ID "
			""
			"LEFT JOIN LabsT ON EMRProblemLinkT.EMRRegardingID = LabsT.ID "
			"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
			""
			"WHERE EMRProblemsT.PatientID = {INT} AND EMRProblemsT.Deleted = 0 "
			";\r\n"
			""
			"SELECT EMRGroupID, ID FROM EMRMasterT WHERE EMRGroupID IN ("
			"	SELECT EMRRegardingID FROM EMRProblemsT "
			"	INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
			"	WHERE EMRRegardingType = {CONST_INT} AND EMRProblemsT.PatientID = {INT} AND EMRProblemsT.Deleted = 0 "
			")\r\n"
			,
			eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtLab,
			eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtUnassigned, eprtLab, ltBiopsy,
			eprtEmrItem, eprtEmrDataItem, eprtEmrTopic, eprtEmrEMN, eprtEmrEMR, eprtEmrDiag, eprtEmrCharge, eprtEmrMedication, eprtUnassigned, eprtLab,
			m_nPatientID,
			eprtEmrEMR, m_nPatientID
			);
		FieldsPtr f = prs->Fields;

		// Read in all the problems
		m_aAllProblems.RemoveAll();
		while (!prs->eof) {
			CEMRSummaryProblem problem;
			CString strProblemType = AdoFldString(f, "ProblemType", "");
			CString strProblemItemName = AdoFldString(f, "ProblemItemName", "");
			CString strStatus = AdoFldString(f, "Status", "");
			CString strDescription = AdoFldString(f, "Description", "");
			long nEMNID = AdoFldLong(f, "EMNID", -1);
			
			problem.m_Type = (EMRProblemRegardingTypes)AdoFldLong(f, "EMRRegardingType");
			if (eprtEmrEMR == problem.m_Type) {
				// We'll get the EMN ID later. For now, store the EMR ID (which is actually in the EMNID field for simplicity).
				problem.m_nEMRID = nEMNID;
			} else if (nEMNID > -1) {
				problem.m_anEMNIDs.Add(nEMNID);
			}

			if (eprtUnassigned == problem.m_Type) {
				problem.m_strDisplayText = FormatString("%s, Status: %s", strDescription, strStatus);
			} else {
				problem.m_strDisplayText = FormatString("%s: %s, Problem: %s, Status: %s", strProblemType, strProblemItemName, strDescription, strStatus);
			}

			m_aAllProblems.Add(problem);
			prs->MoveNext();
		}

		// Now for all EMR-level problems, read in the EMN ID's
		prs = prs->NextRecordset(NULL);
		f = prs->Fields;
		while (!prs->eof) {
			long nEMRID = AdoFldLong(f, "EMRGroupID");
			long nEMNID = AdoFldLong(f, "ID");
			for (int i=0; i < m_aAllProblems.GetSize(); i++) {
				CEMRSummaryProblem p = m_aAllProblems.GetAt(i);
				if (p.m_nEMRID == nEMRID) {
					p.m_anEMNIDs.Add(nEMNID);
					m_aAllProblems.SetAt(i, p);
				}
			}
			prs->MoveNext();
		}
	}
	NxCatchAll("Error in CEMRSummaryDlg::LoadPatientProblems");
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
UINT EMRSummaryLoadImageAsyncThread(LPVOID p)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// LoadImageAsyncThread in CGeneral1Dlg also needs changed

	try {
		// (a.walling 2007-07-20 10:55) - PLID 26762 - We were throwing an exception within this thread, which had no
		// exception handling!
		CEMRSummaryImageLoad* pLoadInfo = (CEMRSummaryImageLoad*)p;
		HBITMAP hBitmap = NULL;

		if (!p) {
			ThrowNxException("Error in EMRSummaryLoadImageAsyncThread - The image information is empty");
		}

		// Only load the image if we actually have information to work with
		// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
		// (c.haag 2010-02-22 17:49) - PLID 37364 - We now do this from the Mirror Image Manager object.
		if (NULL != pLoadInfo && NULL != pLoadInfo->m_pMirrorImageMgr) {
			hBitmap = pLoadInfo->m_pMirrorImageMgr->LoadMirrorImage(pLoadInfo->m_nImageIndex, pLoadInfo->m_nImageCount, -1);
			delete pLoadInfo->m_pMirrorImageMgr;
			pLoadInfo->m_pMirrorImageMgr = NULL;
		} else {
			ThrowNxException("Attempted to load a Mirror thumbnail from a null Mirror Image Manager object!");
		}

		// Now tell the window that we are done loading the image
		if (pLoadInfo->m_pMsgWnd->GetSafeHwnd()) {
			//might as well use the same message as G1, since it is going directly to the calling window
			pLoadInfo->m_pMsgWnd->PostMessage(NXM_G1THUMB_IMAGELOADED, (WPARAM)pLoadInfo, (LPARAM)hBitmap);
		}
	} NxCatchAllThread("Error in EMRSummaryLoadImageAsyncThread");
	return 0;
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::LoadImageAsync(CEMRSummaryImageLoad* pLoadInfo)
{
	//will throw exceptions to the caller

	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	//If we're already loading, add to our list.
	if (m_pLoadImageThread) {
		m_WaitingImages.AddTail(pLoadInfo);
		return;
	}
	
	// Create the thread
	m_pLoadImageThread = AfxBeginThread(EMRSummaryLoadImageAsyncThread, pLoadInfo, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!m_pLoadImageThread) {
		return;
	}

	// Disable the image navigation windows
	GetDlgItem(IDC_PT_IMAGE_LAST)->EnableWindow(FALSE);
	GetDlgItem(IDC_PT_IMAGE_NEXT)->EnableWindow(FALSE);

	// Execute the thread
	m_pLoadImageThread->m_bAutoDelete = FALSE;
	m_pLoadImageThread->ResumeThread();
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::LoadPatientImage(EImageTraversal dir /*= eImageFwd*/)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	try
	{
		//First of all:
		ShowImagingButtons(0);

		//Here's the plan: there's a bunch of different reasons we might want to hide the arrows.
		//If any of them happen, we'll hide them.  If none of them happen, we'll show them.
		BOOL bShowPrimaryThumbnailOnly = GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0) ? TRUE : FALSE;
		bool bShowArrows = true;
		long nImageCountTotal = 0;
	
		// CAH 6/25/03 - We need to wait for Mirror to open.
		// (c.haag 2009-03-31 14:48) - PLID 33630 - We now use InitCanfieldSDK
		Mirror::InitCanfieldSDK(TRUE);//Mirror::IsMirror61(TRUE);
		
		// Get the counts of images in our two imaging providers		
		// (c.haag 2006-10-23 12:06) - PLID 23181 - If we are only showing the primary thumbnail, don't bother to run
		// unnecessary Mirror calculations
		// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter from GetImageCount
		long nImageCountMirror = (GetPropertyInt("MirrorImageDisplay", 1, 0) && !m_strMirrorID.IsEmpty()) ? Mirror::GetImageCount(m_strMirrorID) : 0;
		//TES 2/25/2004: GetImageCount() may return -1 if the patient isn't linked.
		if(nImageCountMirror == -1) nImageCountMirror = 0;
		long nImageCountUnited = (GetPropertyInt("UnitedShowImages", 1, 0) && m_nUnitedID >= 0) ? GetMainFrame()->GetUnitedLink()->GetImageCount(m_nUnitedID) : 0;

		// (j.jones 2008-06-13 10:25) - PLID 30385 - we decided to always show images here and not follow the G1 preference to hide
		//long nImageCountPractice = (GetPropertyInt("PracticeShowImages", 1, 0) ? GetPatientAttachedImageCount(m_nPatientID) : 0);
		long nImageCountPractice = GetPatientAttachedImageCount(m_nPatientID);

		nImageCountTotal = nImageCountMirror + nImageCountUnited + nImageCountPractice;
		// (c.haag 2010-02-24 10:38) - PLID 37364 - Get the image index limits from the Mirror image manager object
		long nMirrorFirst = (nImageCountMirror > 0 ? GetFirstValidMirrorImageIndex(m_strMirrorID) : 0);
		long nMirrorLast = (nImageCountMirror > 0 ? GetLastValidMirrorImageIndex(m_strMirrorID) : -1);
		long nPracticeFirst = 0;
		long nPracticeLast = nImageCountPractice - 1;

		//////////////////////////////////////////////////////////////
		// If we're just using the primary thumbnail, load it
		if (bShowPrimaryThumbnailOnly)
		{
			//
			// (c.haag 2006-11-07 13:18) - PLID 23181 - Use this flag to determine whether to show a Practice image
			// for the primary thumb. The legacy behavior was to show Mirror images if PracticeShowImages was zero,
			// so we will have to carry on with it.
			//
			// The general logic is:
			//
			// 1. Try to load the Practice primary thumbnail, if it exists
			// 2. Try to load the Mirror primary thumbnail, even if PracticeShowImages is false, if it exists
			//
			
			// (j.jones 2008-06-13 10:25) - PLID 30385 - we decided to always show images here and not follow the G1 preference to hide
			BOOL bUsePracticeImages = TRUE; //GetPropertyInt("PracticeShowImages", 1, 0);

			//TES 2/25/2004: Just show the big button.
			ShowImagingButtons(1);

			// (c.haag 2006-11-07 13:20) - PLID 23181 - Try to load the Practice image now
			if (bUsePracticeImages) {
				//JMM 11/04/04 - need to set the source to be Practice 
				m_imageButton.m_source = eImageSrcPractice;  				
				m_imageButton.m_image = LoadPatientAttachedImage(m_nPatientID, 0, &m_imageButton.m_progress, m_imageButton.m_strPracticeFileName);
				if (!m_imageButton.m_image)
				{
					// (c.haag 2003-10-01 11:24) - Check to see if we even have a primary image.
					// (j.jones 2008-06-13 11:00) - PLID 30388 - filtered only on photos (or unknown), and parameterized
					//TES 8/3/2011 - PLID 44814 - Make sure we filter only on categories this user has permission to view
					//(e.lally 2012-02-06) PLID 35377 - Use Snapshot connection
					_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PathName "
						"FROM MailSent "
						"INNER JOIN PatientsT ON PatientsT.PatPrimaryHistImage = MailSent.MailID "
						"WHERE PatientsT.PersonID = {INT} "
						"AND (MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1) "
						"AND {SQLFRAGMENT}", m_nPatientID, GetAllowedCategoryClause_Param("CategoryID"));
					if (!prs->eof)
					{
						// Don't try to load
						// (c.haag 2006-11-07 13:21) - PLID 23181 - Keep bUsePracticeImages TRUE so that
						// the user can see the error. There should have been a Practice image!
						//GetDlgItem(IDC_PT_IMAGE)->ShowWindow(SW_HIDE);
						m_imageButton.m_nError = eErrorUnspecified;
					}
					else
					{
						bUsePracticeImages = FALSE;
					}
				} else {
					// (c.haag 2006-11-07 13:21) - PLID 23181 - We successfully loaded the Practice image.
					// No need to load the Mirror image.
					m_imageButton.m_nError = eNoError;
				}
			}

			if (!bUsePracticeImages) {
				//
				// (c.haag 2006-10-23 11:29) - PLID 23181 - If we get here, try to load the primary Mirror thumbnail
				//
				if (nImageCountMirror > 0 && (GetPropertyInt("MirrorImageDisplay", 1, 0) && !m_strMirrorID.IsEmpty())) {
					GetDlgItem(IDC_PT_IMAGE)->ShowWindow(SW_SHOW);
					if(m_imageButton.m_image) {
						DeleteObject(m_imageButton.m_image);
						m_imageButton.m_image = NULL;
					}
					m_imageButton.m_nError = eNoError;
					LoadSingleImage(MIRROR_INDEX_PRIMARY_THUMBNAIL, nImageCountMirror, &m_imageButton, eImageSrcMirror);
					m_imageButton.Invalidate();
				} else {
					// (c.haag 2006-11-07 13:24) - PLID 23181 - This wasn't here before, but we really should hide
					// the imaging button if there is nothing to show.
					ShowImagingButtons(0);
					m_imageButton.m_nError = eNoError;
				}
			}

			GetDlgItem(IDC_PT_IMAGE_LAST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PT_IMAGE_NEXT)->ShowWindow(SW_HIDE);
			
			// Display whatever new image we have, if any
			bShowArrows = false;
			m_imageButton.Invalidate();
			return;
		} // if (bShowPrimaryThumbnailOnly)

		//////////////////////////////////////////////////////////////
		// Make sure the index is in range
		if (m_nImageIndex < 0) {
			m_nImageIndex = nImageCountTotal-1;
		}
		else if (m_nImageIndex > nImageCountTotal-1) {
			m_nImageIndex = 0;
		}

		if (!nImageCountTotal)
		{
			// This means no pictures were found
			bShowArrows = false;
			ShowImagingButtons(0);
			m_nImageIndex = -1;
		}
		else
		{
		
			long nLastValidIndex = m_nImageIndex;

			////////////////////////////////////////////////////////
			// Load the image
			GetDlgItem(IDC_PT_IMAGE)->ShowWindow(SW_SHOW);

			//TES 2/25/2004: We may be loading multiple images at a time.
			int nImagesToShow = GetRemotePropertyInt("General1ImageCount", 1, 0, GetCurrentUserName(), true);
			if(nImageCountTotal <= nImagesToShow) {
				bShowArrows = false;
				nImagesToShow = nImageCountTotal;
			}
			
			CMirrorImageButton * pFirstImage = NULL;
			CMirrorImageButton * pSecondImage = NULL;
			if(nImagesToShow == 1) {
				pFirstImage = &m_imageButton;
			}
			else if(nImagesToShow ==2) {
				pFirstImage = &m_imageButtonLeft;
				pSecondImage = &m_imageButtonRight;
			}
			else {
				pFirstImage = &m_imageButtonUpperLeft;
				pSecondImage = &m_imageButtonUpperRight;
			}
			ShowImagingButtons(nImagesToShow);


			if (m_nImageIndex >= 0 && m_nImageIndex < nImageCountTotal) {
				for(int i = 0; i < nImagesToShow; i++) {
					int nIndex = m_nImageIndex+i >= nImageCountTotal ? m_nImageIndex+i-nImageCountTotal : m_nImageIndex+i;
					CMirrorImageButton *pButton;
					if(i == 0) {
						pButton = pFirstImage;
					}
					else if(i == 1) {
						pButton = pSecondImage;
					}
					else if(i == 2) {
						pButton = &m_imageButtonLowerLeft;
					}
					else if(i == 3) {
						pButton = &m_imageButtonLowerRight;
					}

					//Initialize this button.
					if(pButton->m_image) {
						DeleteObject(pButton->m_image);
						pButton->m_image = NULL;
					}
					pButton->m_nError = eNoError;

					//Now, load this sucker.
					if (nIndex >= 0 && nIndex < nImageCountMirror) {
						// Load from Mirror
						LoadSingleImage(nIndex, nImageCountMirror, pButton, eImageSrcMirror);
					} else if (nIndex >= nImageCountMirror && nIndex < nImageCountMirror + nImageCountUnited) {
						// Load from United
						LoadSingleImage(nIndex - nImageCountMirror, nImageCountUnited, pButton, eImageSrcUnited);
					} else if (nIndex >= nImageCountMirror + nImageCountUnited && nIndex < nImageCountTotal) {
						// Load from NexTech
						LoadSingleImage(nIndex - nImageCountMirror - nImageCountUnited, nImageCountPractice, pButton, eImageSrcPractice);
					}

					// Display whatever new image we have, if any
					pButton->Invalidate();
				}			
			}
		}
			

		// Decide what the prev/next buttons should look like
		if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0)) {
			bShowArrows = false;
		}
		
		//OK, let's show the arrows as appropriate
		if(bShowArrows) {
			GetDlgItem(IDC_PT_IMAGE_LAST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_PT_IMAGE_NEXT)->ShowWindow(SW_SHOW);
		}
		else {
			GetDlgItem(IDC_PT_IMAGE_LAST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PT_IMAGE_NEXT)->ShowWindow(SW_HIDE);
		}
	}
	NxCatchAll("Could not load patient image");
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::ShowImagingButtons(int nCountToShow)
{
	//will throw exceptions to the caller

	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	//TES 2/25/2004: I have this theory that calling ShowWindow(SW_HIDE) on an already hidden window causes 
	//some minor drawing issues.
	if(nCountToShow == 0) { //Hide everything.

		if(m_imageButton.GetStyle() & WS_VISIBLE) {
			m_imageButton.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonRight.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonUpperLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonUpperRight.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerRight.ShowWindow(SW_HIDE);
		}
	}
	else if(nCountToShow == 1) { //Just show the big one.

		if(!(m_imageButton.GetStyle() & WS_VISIBLE)) {
			m_imageButton.ShowWindow(SW_SHOW);
		}

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonRight.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonUpperLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonUpperRight.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerRight.ShowWindow(SW_HIDE);
		}
	}
	else if(nCountToShow == 2) { //Just show the left and right.

		if(m_imageButton.GetStyle() & WS_VISIBLE) {
			m_imageButton.ShowWindow(SW_HIDE);
		}

		if(!(m_imageButtonLeft.GetStyle() & WS_VISIBLE)) {
			m_imageButtonLeft.ShowWindow(SW_SHOW);
		}

		if(!(m_imageButtonRight.GetStyle() & WS_VISIBLE)) {
			m_imageButtonRight.ShowWindow(SW_SHOW);
		}

		if(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonUpperLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonUpperRight.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerRight.ShowWindow(SW_HIDE);
		}
	}
	else if(nCountToShow == 3) {//Show three of the four small ones.

		if(m_imageButton.GetStyle() & WS_VISIBLE) {
			m_imageButton.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonRight.ShowWindow(SW_HIDE);
		}

		if(!(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE)) {
			m_imageButtonUpperLeft.ShowWindow(SW_SHOW);
		}

		if(!(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE)) {
			m_imageButtonUpperRight.ShowWindow(SW_SHOW);
		}

		if(!(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE)) {
			m_imageButtonLowerLeft.ShowWindow(SW_SHOW);
		}

		if(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonLowerRight.ShowWindow(SW_HIDE);
		}
	}
	else {//Show the four small ones.

		if(m_imageButton.GetStyle() & WS_VISIBLE) {
			m_imageButton.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonLeft.GetStyle() & WS_VISIBLE) {
			m_imageButtonLeft.ShowWindow(SW_HIDE);
		}

		if(m_imageButtonRight.GetStyle() & WS_VISIBLE) {
			m_imageButtonRight.ShowWindow(SW_HIDE);
		}

		if(!(m_imageButtonUpperLeft.GetStyle() & WS_VISIBLE)) {
			m_imageButtonUpperLeft.ShowWindow(SW_SHOW);
		}

		if(!(m_imageButtonUpperRight.GetStyle() & WS_VISIBLE)) {
			m_imageButtonUpperRight.ShowWindow(SW_SHOW);
		}

		if(!(m_imageButtonLowerLeft.GetStyle() & WS_VISIBLE)) {
			m_imageButtonLowerLeft.ShowWindow(SW_SHOW);
		}

		if(!(m_imageButtonLowerRight.GetStyle() & WS_VISIBLE)) {
			m_imageButtonLowerRight.ShowWindow(SW_SHOW);
		}
	}
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::LoadSingleImage(long nImageIndexAgainstSource, long nImageCountSource, CMirrorImageButton *pButton, EImageSource Source)
{
	//will throw exceptions to the caller

	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	//Regardless...
	pButton->m_source = Source;
	switch(Source) {
	case eImageSrcMirror:
	{
		// Load from Mirror
		//Now that we know there's something to view, check whether they have permission to view it.
		if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
			//bShowArrows = false;
			// Clear whatever image is there
			if (pButton->m_image) {
				DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_nError = eErrorNoPermission;		
		}
		else {
			// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
			const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
			if (bUsingSDKFunctionality && GetRemotePropertyInt("MirrorAllowAsyncOperations", 1)) {
				pButton->m_image = (HBITMAP)MIRRORIMAGEBUTTON_PENDING;
				// (c.haag 2010-02-22 17:08) - PLID 37364 - We must ensure that the Mirror Image manager object exists before this thread begins.
				// We don't want it to initialize in the thread because the constructor accesses the global connection pointer.
				EnsureMirrorImageMgr();
				LoadImageAsync(new CEMRSummaryImageLoad(this, m_pMirrorImageMgr, m_strMirrorID, nImageIndexAgainstSource, nImageCountSource, m_nPatientID, pButton));
			}
			else {
				pButton->m_source = eImageSrcMirror;
				// (c.haag 2009-04-02 12:49) - PLID 33630 - Took out synchronous parameter
				if (bUsingSDKFunctionality) {
					// (c.haag 2010-02-22 17:08) - PLID 37364 - We must ensure that the Mirror Image manager object exists before
					// trying to load images.
					EnsureMirrorImageMgr();
					if (m_pMirrorImageMgr) {
						pButton->m_image = m_pMirrorImageMgr->LoadMirrorImage(nImageIndexAgainstSource, nImageCountSource, -1);
					} else {
						pButton->m_image = NULL; // This should never happen
					}
				} else {
					// If we're not using the SDK, defer to the old style image loading
					pButton->m_image = Mirror::LoadMirrorImage(m_strMirrorID, nImageIndexAgainstSource, nImageCountSource, -1);
				}
				
				if (pButton->m_image == NULL) {
					pButton->m_nError = eErrorUnspecified;
				}
				else {
					pButton->m_nError = eNoError;
				}
			}
		}
	} 
	break;
	case eImageSrcUnited:
	{
		// Load from United
		//Now that we know there's something to view, check whether they have permission to view it.
		if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
			//bShowArrows = false;
			// Clear whatever image is there
			if (pButton->m_image)
			{	DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_nError = eErrorNoPermission;		
		}
		else {
			pButton->m_source = eImageSrcUnited;
			pButton->m_image = GetMainFrame()->GetUnitedLink()->LoadImage(m_nUnitedID, nImageIndexAgainstSource);
			if (pButton->m_image == NULL)
				pButton->m_nError = eErrorUnspecified;
			else
				pButton->m_nError = eNoError;
		}

	} 
	break;
	case eImageSrcPractice:
	{
		// Load from NexTech
		//Now that we know there's something to view, check whether they have permission to view it.
		if (!g_userPermission[ViewImage] || !UserPermission(ViewImage)) {
			//bShowArrows = false;
			// Clear whatever image is there
			if (pButton->m_image) {
				DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_nError = eErrorNoPermission;		
		}
		else {
			pButton->m_source = eImageSrcPractice;
			if(pButton->m_image) {
				DeleteObject(pButton->m_image);
				pButton->m_image = NULL;
			}
			pButton->m_image = LoadPatientAttachedImage(m_nPatientID, nImageIndexAgainstSource, &pButton->m_progress, pButton->m_strPracticeFileName);
			if (pButton->m_image == NULL) {
				pButton->m_nError = eErrorUnspecified;
			}
			else {
				pButton->m_nError = eNoError;
			}
		}
	}
	break;
	}
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
LRESULT CEMRSummaryDlg::OnImageLoaded(WPARAM wParam, LPARAM lParam)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	CEMRSummaryImageLoad* pLoadInfo = NULL;

	try {

		pLoadInfo = (CEMRSummaryImageLoad*)wParam;
		HBITMAP hBitmap = (HBITMAP)lParam;
		BOOL bReload = FALSE;

		// Quit right away if we are being destroyed
		if (!GetSafeHwnd()) {
			delete pLoadInfo;
			return 0;
		}

		if (pLoadInfo->m_nPatientID != m_nPatientID) {
			bReload = TRUE;
		}

		// If we loaded the image and did not change the patient since starting
		// the load, then draw the image.
		if (!bReload) {
			pLoadInfo->m_pButton->m_image = hBitmap;
			if (hBitmap == NULL) {
				pLoadInfo->m_pButton->m_nError = eErrorUnspecified;
			}
			pLoadInfo->m_pButton->Invalidate();
			GetDlgItem(IDC_PT_IMAGE_LAST)->EnableWindow(TRUE);
			GetDlgItem(IDC_PT_IMAGE_NEXT)->EnableWindow(TRUE);
		}

		// Destroy our thread object
		if (m_pLoadImageThread) {
			// Wait for the thread to terminate
			WaitForSingleObject(m_pLoadImageThread->m_hThread, 10000);

			// Get the exit code
			DWORD dwExitCode = 0;
			::GetExitCodeThread(m_pLoadImageThread->m_hThread, &dwExitCode);
			// See if the thread is still active
			if (dwExitCode == STILL_ACTIVE) {
				// The thread is still going so post a quit message to it and let it delete itself
				// (a.walling 2006-09-26 12:46) - PLID 22713 - Fix memory leak by telling thread object to deallocate itself.
				m_pLoadImageThread->m_bAutoDelete = TRUE;
				PostThreadMessage(m_pLoadImageThread->m_nThreadID, WM_QUIT, 0, 0);
			} else {
				// The thread is finished, so just delete it
				delete m_pLoadImageThread;
			}
			m_pLoadImageThread = NULL;

			//Do we have another one waiting?
			CEMRSummaryImageLoad *pNextOne = m_WaitingImages.GetCount() == 0 ? NULL : (CEMRSummaryImageLoad*)m_WaitingImages.RemoveHead();
			if(pNextOne) {
				LoadImageAsync(pNextOne);
			}
		}

		// If the user went to another patient, load the thumbnail again
		if (bReload) {
			LoadPatientImage();
		}
	
	}NxCatchAll("Error in CEMRSummaryDlg::OnImageLoaded (1)");

	try {

		if(pLoadInfo) {
			delete pLoadInfo;
		}

	}NxCatchAll("Error in CEMRSummaryDlg::OnImageLoaded (2)");

	return 0;
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::HandleImageClick(CMirrorImageButton *pClicked)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	try {
		if (pClicked->m_nError != eNoError)
		{
			CWnd* pWnd = GetFocus();
			switch(pClicked->m_nError) {
			case eErrorNoPermission:
				MsgBox("You do not have permission to view patient images.  Contact your office manager for assistance.");
				break;
			case eErrorUnspecified:
				switch (pClicked->m_source)
				{
				case eImageSrcMirror:
					Mirror::Troubleshoot();
					break;
				case eImageSrcUnited:
					MsgBox("Practice could not load the patient's United Imaging thumbnail. Please ensure you are properly connected to your United server and that the pictures are accessible over your network.");
					break;
				case eImageSrcPractice:
					MsgBox("Practice could not load the image at %s. Please ensure that this patient's pictures are accessible over your network. You may also go to the History tab and click on 'Open Default Folder' to browse for the picture.",
						pClicked->m_strPracticeFileName);					
					break;				
				default:
					MsgBox("Practice could not load the patient's picture. Please ensure that this patient's pictures are accessible over your network.");
					break;
				}
				break;
			// (c.haag 2009-04-01 17:24) - PLID 33630 - Nothing to do. This is not an interactive mode.
			case eImageBtnInitializingMirror:
				break;
			}
			if (pWnd) {
				pWnd->SetFocus();
			}
			return;
		}
		else if (pClicked->m_image == NULL) {
			return;
		}
		
		if (GetPropertyInt("PracticeShowPrimaryImageOnly", 0, 0) && pClicked->m_source == eImageSrcPractice) {
			CFile file;
			CString strFilename = pClicked->m_strPracticeFileName;
			
			// Open the image with explorer
			if (!file.Open(strFilename, CFile::modeRead | CFile::shareCompat)) {
				MsgBox("Could not open image");
				return;
			}
			file.Close();
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32) {
				MsgBox("Could not open image");
				return;
			}
			
		}
		else if (pClicked->m_source == eImageSrcMirror) {
			Mirror::Run();
		}
		else if (pClicked->m_source == eImageSrcUnited) {

			// (j.armen 2011-10-25 08:56) - PLID 46136 - Added more exception handling to ensure that the working directory is set correctly
			try
			{
				///////////////////////////////////////////////////////
				// Open united (TODO: Put this in the generic link)
				_ConnectionPtr pConRemote(__uuidof(Connection));
				CUnitedLink *pUnitedLink = GetMainFrame()->GetUnitedLink();
				const CString strExecutePath = GetUnitedExecutePath();
				CString strParams, strID;
				CString strCon, strSQL;

				// Get the correct path
				// (j.armen 2011-10-25 08:50) - PLID 46136 - We already know what the current directory is supposed to be by using GetPracPath(PracPath:SessionPath), so no need to get it
				//GetCurrentDirectory(512, szCurrentDirectory);

				// Set the path because United needs it that way
				// (c.haag 2006-06-30 12:22) - PLID 21263 - We now use the path
				// to the local United install. I'm leaving the current directory
				// logic as is.
				SetCurrentDirectory(FileUtils::GetFilePath(strExecutePath));
				
				// Get the active patient ID
				strID.Format("%li", m_nUserDefinedID);

				// Update the ID in United
				strCon = "Provider=Microsoft.Jet.OLEDB.4.0;" +
					(pUnitedLink->GetRemotePassword().GetLength() ? ("Jet OLEDB:Database Password=" + pUnitedLink->GetRemotePassword() + ";") : "") +
					"Data Source=" + pUnitedLink->GetRemotePath() + ";";
				strSQL.Format("UPDATE tblPatient SET uExternalID = '%s' WHERE ID = %d",
					strID, m_nUnitedID);
				pConRemote->Open(_bstr_t((LPCTSTR)strCon), "","",NULL);
				pConRemote->Execute(_bstr_t(strSQL), NULL, adCmdText);
				pConRemote->Close();
				pConRemote.Detach();

				// Build the actual parameters
				strParams.Format("%s %s", strExecutePath, strID);
				int nReturnCode = WinExec (strParams, SW_SHOW);
				if(nReturnCode != 0 && nReturnCode != 33) {//TES 2/25/2004: It consistently returns 33 for me even after successfully
															//opening.  It's a mystery to me.
					if(nReturnCode == ERROR_FILE_NOT_FOUND || nReturnCode == ERROR_PATH_NOT_FOUND) {
						MsgBox("Failed to open United.  The specified file '%s' could not be found.", strExecutePath);
					}
					else {
						MsgBox("Failed to open United.  Unspecified error.");
					}
				}

			}NxCatchAll("EMRSummaryDlg::HandleImageClick()::United");

			// Set our current directory back to what it was
			// (j.armen 2011-10-25 08:52) - PLID 46136 - Force the path back to the session path
			// moved it outside of exception handling to ensure that if we changed the path and United fails, 
			// we still set the current directory back
			SetCurrentDirectory(GetPracPath(PracPath::SessionPath));
		}
		else if (pClicked->m_source == eImageSrcPractice) {
			CFile file;
			CString strFilename = pClicked->m_strPracticeFileName;
			
			// Open the image with explorer
			if (!file.Open(strFilename, CFile::modeRead | CFile::shareCompat))
			{
				MsgBox("Could not open image");
				return;
			}
			file.Close();

			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, GetFilePath(strFilename), SW_MAXIMIZE) < 32)
			{
				MsgBox("Could not open image");
				return;
			}
		}

	}NxCatchAll("CEMRSummaryDlg::HandleImageClick()");
}

void CEMRSummaryDlg::OnPtImage() 
{
	HandleImageClick(&m_imageButton);
}

void CEMRSummaryDlg::OnPtImageLeft()
{
	HandleImageClick(&m_imageButtonLeft);
}

void CEMRSummaryDlg::OnPtImageRight()
{
	HandleImageClick(&m_imageButtonRight);
}

void CEMRSummaryDlg::OnPtImageUpperLeft()
{
	HandleImageClick(&m_imageButtonUpperLeft);
}

void CEMRSummaryDlg::OnPtImageUpperRight()
{
	HandleImageClick(&m_imageButtonUpperRight);
}

void CEMRSummaryDlg::OnPtImageLowerLeft()
{
	HandleImageClick(&m_imageButtonLowerLeft);
}

void CEMRSummaryDlg::OnPtImageLowerRight()
{
	HandleImageClick(&m_imageButtonLowerRight);
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::OnRightClickImage(HWND hWndClicked, UINT nFlags, CPoint pt)
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	try {

		CMenu popup;

		//Figure out which button they right-clicked on.
		if(m_imageButton.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButton;
		}
		else if(m_imageButtonLeft.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButtonLeft;
		}
		else if(m_imageButtonRight.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButtonRight;
		}
		else if(m_imageButtonUpperLeft.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButtonUpperLeft;
		}
		else if(m_imageButtonUpperRight.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButtonUpperRight;
		}
		else if(m_imageButtonLowerLeft.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButtonLowerLeft;
		}
		else if(m_imageButtonLowerRight.GetSafeHwnd() == hWndClicked) {
			m_RightClicked = &m_imageButtonLowerRight;
		}
		else {
			return;
		}

		if (!m_RightClicked->m_image || m_RightClicked->m_nError != eNoError) {
			return;
		}

		//CPoint point(pts.x, pts.y);

		m_RightClicked->ClientToScreen(&pt);

		popup.CreatePopupMenu();
		popup.AppendMenu (MF_ENABLED, IDM_COPY, "Copy");
		popup.AppendMenu (MF_ENABLED, IDM_VIEW, "View");
		popup.TrackPopupMenu (TPM_LEFTALIGN|TPM_RIGHTBUTTON|TPM_LEFTBUTTON, 
			pt.x, pt.y, this);

	}NxCatchAll("Error in CEMRSummaryDlg::OnRightClickImage");
}

BOOL CEMRSummaryDlg::PreTranslateMessage(MSG* pMsg) 
{
	try {
	
		switch(pMsg->message)
		{
		case WM_RBUTTONDOWN:
			{
				OnRightClickImage(pMsg->hwnd, pMsg->wParam, pMsg->lParam);
			}
			break;

		default:
			break;
		}

	}NxCatchAll("Error in CEMRSummaryDlg::PreTranslateMessage");

	return CNxDialog::PreTranslateMessage(pMsg);
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::OnViewImage()
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	try {

		if(!m_RightClicked) {
			return;
		}

		CImageDlg dlg(this);
		dlg.DoModal(m_RightClicked->m_image, m_strLast + ", " + m_strFirst +  " " + m_strMiddle);

	}NxCatchAll("Error in CEMRSummaryDlg::OnViewImage");
}

// (j.jones 2008-06-13 09:05) - PLID 30385 - this was modified from the similar function in G1
void CEMRSummaryDlg::OnCopyImage()
{
	// (j.jones 2008-06-13 11:12) - if you change this function, check and see if the
	// same-named function in CGeneral1Dlg also needs changed

	HBITMAP hBmp;
	try
	{
		if (!m_RightClicked->m_image || m_RightClicked->m_nError != eNoError) {
			MsgBox("Invalid image handle");
			return;
		}
		
		hBmp = CopyBitmap(m_RightClicked->m_image);
		
		if (!hBmp) {
			AfxThrowNxException("Could not duplicate image");
		}

		if (!OpenClipboard()) {
			AfxThrowNxException("Could not open clipboard");
		}
	
		if (!EmptyClipboard()) {
			AfxThrowNxException("Could not clear clipboard");
		}

		if (!SetClipboardData(CF_BITMAP, hBmp)) {
			AfxThrowNxException("Could not set bitmap to clipboard");
		}

		if (!CloseClipboard()) {
			AfxThrowNxException("Could not close clipboard");
		}
	
	}NxCatchAll("Error in CEMRSummaryDlg::OnCopyImage");
}

void CEMRSummaryDlg::OnDestroy() 
{
	try {

		// Destroy our thread object
		if (m_pLoadImageThread)
		{
			// Wait for the thread to terminate
			WaitForSingleObject(m_pLoadImageThread->m_hThread, 2000);

			// Get the exit code
			DWORD dwExitCode = 0;
			::GetExitCodeThread(m_pLoadImageThread->m_hThread, &dwExitCode);
			// See if the thread is still active
			if (dwExitCode == STILL_ACTIVE) {
				// The thread is still going so post a quit message to it and let it delete itself
				m_pLoadImageThread->m_bAutoDelete = TRUE;
				PostThreadMessage(m_pLoadImageThread->m_nThreadID, WM_QUIT, 0, 0);
			} else {
				// The thread is finished, so just delete it
				delete m_pLoadImageThread;
			}
			m_pLoadImageThread = NULL;
				
		}

		// (c.haag 2010-02-23 09:55) - PLID 37364 - Delete the Mirror image manager if it exists
		EnsureNotMirrorImageMgr();

	}NxCatchAll("Error in CEMRSummaryDlg::OnDestroy");

	CNxDialog::OnDestroy();	
}

// (j.jones 2008-06-19 09:24) - PLID 30436 - added ability to configure the EMR Summary
void CEMRSummaryDlg::OnBtnConfigureSummary() 
{
	try {

		CEMRSummaryConfigDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			Load();
		}

	}NxCatchAll("Error in CEMRSummaryDlg::OnBtnConfigureSummary");
}

// (j.jones 2008-06-26 17:48) - PLID 21168 - given two lists, select the list with the least amount of objects
CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*>* CEMRSummaryDlg::FindShortestList(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> &aryObjects1, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> &aryObjects2)
{
	try {

		if(aryObjects1.GetSize() <= aryObjects2.GetSize()) {
			return &aryObjects1;
		}
		else {
			return &aryObjects2;
		}

	}NxCatchAll("Error in CEMRSummaryDlg::FindShortestList");

	return NULL;
}

void CEMRSummaryDlg::OnTimer(UINT nIDEvent) 
{
	try {

		switch(nIDEvent) {
			// (j.jones 2008-06-27 10:19) - PLID 21168 - load the EMN list when receiving this timer
		case TIMER_LOAD_EMN_LIST:
			{
				KillTimer(TIMER_LOAD_EMN_LIST);

				GenerateEMNIDList();

				PopulateEMNList();
			}
			break;
		}

	}NxCatchAll("Error in CEMRSummaryDlg::OnTimer");

	CNxDialog::OnTimer(nIDEvent);
}

// (z.manning 2010-01-12 14:36) - PLID 31945 - Switch between scrollbar and arrow navigation
void CEMRSummaryDlg::ToggleNavigationType()
{
	if(m_bDisplayArrows) {
		GetDlgItem(IDC_BTN_PREV_EMN)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_NEXT_EMN)->ShowWindow(SW_SHOW);
		SetDlgItemText(IDC_BTN_TOGGLE_VIEW_TYPE, "Use Scrollbar Navigation");
	}
	else {
		GetDlgItem(IDC_BTN_PREV_EMN)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTN_NEXT_EMN)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_BTN_TOGGLE_VIEW_TYPE, "Use Arrow Navigation");
	}
}

// (c.haag 2010-02-23 10:21) - PLID 37364 - This function ensures that the Mirror image manager exists
// and is ready for getting image counts and loading images
void CEMRSummaryDlg::EnsureMirrorImageMgr()
{
	if (NULL == m_pMirrorImageMgr) {
		m_pMirrorImageMgr = new CMirrorPatientImageMgr(m_nPatientID);
	}
}

// (c.haag 2010-02-23 15:08) - PLID 37364 - This function ensures that the Mirror image manager does
// not exist. If it did, it is deleted.
void CEMRSummaryDlg::EnsureNotMirrorImageMgr()
{
	if (NULL != m_pMirrorImageMgr) {
		delete m_pMirrorImageMgr;
		m_pMirrorImageMgr = NULL;
	}
}

// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the first valid Mirror image index for this patient
long CEMRSummaryDlg::GetFirstValidMirrorImageIndex(const CString& strMirrorID)
{
	EnsureMirrorImageMgr();
	return m_pMirrorImageMgr->GetFirstValidImageIndex();
}

// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the last valid Mirror image index for this patient
long CEMRSummaryDlg::GetLastValidMirrorImageIndex(const CString& strMirrorID)
{
	EnsureMirrorImageMgr();
	return m_pMirrorImageMgr->GetLastValidImageIndex();
}