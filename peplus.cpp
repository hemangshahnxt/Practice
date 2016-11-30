////////////////////////////////////////////////////////////////////////////////
// File:		peplus.cpp
//
// Authors:		Craig Chaplin
//
// Synopsis:	This file contains the class implementations for CRPEngine and
//				CRPEJob as well as any supporting classes and structures.
//
// History:     27/07/95 - CC - added GetNFormulas (4.0 only)
//                            - added GetNthFormula (4.0 only)  
//              26/10/95 - CC - Internationalized (only MBCS so far)
//              09/05/96 - CC - Added 5.0 Print Engine calls
//              03/02/97 - CC - Added assertion check to GetSectionOptions
//              15/08/97 - CC - Modified CRPEParameterFieldInfo to reflect new
//                              6.0 structure
////////////////////////////////////////////////////////////////////////////////

 
#include "stdafx.h"
#include "peplus.h"
#include "crpe.h"

#include "NxReportJob.h"
#include "NxRPEMDIChildWnd.h"


CRPEngine *CRPEngine::thePrintEngine = NULL;

//////////////////////////////////////////////////////////////
// Supporting structures used by CRPEngine and CRPEJob classes
//////////////////////////////////////////////////////////////
CRPEReportOptions::CRPEReportOptions (short saveDataWithReport,
                                      short saveSummariesWithReport,
                                      short useIndexForSpeed,
                                      short translateDOSStrings,
                                      short translateDOSMemos,
                                      short convertDateTimeType,
                                      short convertNullFieldToDefault,
                                      short morePrintEngineErrorMessages,
                                      short caseInsensitiveSQLData,
                                      short verifyOnEveryPrint,
                                      short zoomMode,
                                      short hasGroupTree,
                                      short dontGenerateDataForHiddenObjects,
                                      short performGroupingOnServer
                                     ) : 
                                      m_StructSize(sizeof(CRPEReportOptions)), 
                                      m_saveDataWithReport(saveDataWithReport),
                                      m_saveSummariesWithReport(saveSummariesWithReport),
                                      m_useIndexForSpeed(useIndexForSpeed),
                                      m_translateDOSStrings(translateDOSStrings),
                                      m_translateDOSMemos(translateDOSMemos),
                                      m_convertDateTimeType(convertDateTimeType),
                                      m_convertNullFieldToDefault(convertNullFieldToDefault),
                                      m_morePrintEngineErrorMessages(morePrintEngineErrorMessages),
                                      m_caseInsensitiveSQLData(caseInsensitiveSQLData),
                                      m_verifyOnEveryPrint(verifyOnEveryPrint),
                                      m_zoomMode(zoomMode),
                                      m_hasGroupTree(hasGroupTree),
                                      m_dontGenerateDataForHiddenObjects(dontGenerateDataForHiddenObjects),
                                      m_performGroupingOnServer(performGroupingOnServer)

{}


CRPELogOnInfo::CRPELogOnInfo (const _TCHAR *serverName,
                              const _TCHAR *databaseName,
                              const _TCHAR *userID,
                              const _TCHAR *password
                             ) :
                              m_StructSize(sizeof(CRPELogOnInfo))
{
   lstrcpyn(m_serverName,serverName,sizeof(m_serverName) - 1);
   lstrcpyn(m_databaseName,databaseName,sizeof(m_databaseName) - 1);
   lstrcpyn(m_userID,userID,sizeof(m_userID) - 1);
   lstrcpyn(m_password,password,sizeof(m_password) - 1);
}


CRPESessionInfo::CRPESessionInfo (const _TCHAR *userID,
                                  const _TCHAR *password,
                                  DWORD sessionHandle
                                 ) :
                                  m_StructSize(sizeof(CRPESessionInfo)),
                                  m_sessionHandle(sessionHandle)
{
   lstrcpyn(m_userID,userID,sizeof(m_userID) - 1);
   lstrcpyn(m_password,password,sizeof(m_password) - 1);
}


CRPETableLocation::CRPETableLocation (const _TCHAR *location
                                     ) :
                                      m_StructSize(sizeof(CRPETableLocation))
{
   lstrcpyn(m_location,location,sizeof(m_location) - 1);
}


CRPEExportOptions::CRPEExportOptions (const _TCHAR *formatDLLName,
                                      DWORD formatType,
                                      void *formatOptions,
                                      const _TCHAR *destinationDLLName,
                                      DWORD destinationType,
                                      void *destinationOptions
                                     ) :
                                      m_StructSize(sizeof(CRPEExportOptions)),
                                      m_formatType(formatType),
                                      m_formatOptions(formatOptions),
                                      m_destinationType(destinationType),
                                      m_destinationOptions(destinationOptions)
{
   lstrcpyn(m_formatDLLName,formatDLLName,sizeof(m_formatDLLName) - 1);
   lstrcpyn(m_destinationDLLName,destinationDLLName,sizeof(m_destinationDLLName) - 1);
}

                                      
CRPEGraphTextInfo::CRPEGraphTextInfo (const _TCHAR *graphTitle,
                                      const _TCHAR *graphSubTitle,
                                      const _TCHAR *graphFootNote,
                                      const _TCHAR *graphGroupsTitle,
                                      const _TCHAR *graphSeriesTitle,
                                      const _TCHAR *graphXAxisTitle,
                                      const _TCHAR *graphYAxisTitle,
                                      const _TCHAR *graphZAxisTitle
                                     ) :
                                      m_StructSize(sizeof(CRPEGraphTextInfo))
{
   lstrcpyn(m_graphTitle,graphTitle,sizeof(m_graphTitle) - 1);
   lstrcpyn(m_graphSubTitle,graphSubTitle,sizeof(m_graphSubTitle) - 1);
   lstrcpyn(m_graphFootNote,graphFootNote,sizeof(m_graphFootNote) - 1);
   lstrcpyn(m_graphGroupsTitle,graphGroupsTitle,sizeof(m_graphGroupsTitle) - 1);
   lstrcpyn(m_graphSeriesTitle,graphSeriesTitle,sizeof(m_graphSeriesTitle) - 1);
   lstrcpyn(m_graphXAxisTitle,graphXAxisTitle,sizeof(m_graphXAxisTitle) - 1);
   lstrcpyn(m_graphYAxisTitle,graphYAxisTitle,sizeof(m_graphYAxisTitle) - 1);
   lstrcpyn(m_graphZAxisTitle,graphZAxisTitle,sizeof(m_graphZAxisTitle) - 1);
}

                                      
CRPEGraphOptions::CRPEGraphOptions (double graphMaxValue,
                                    double graphMinValue,
                                    BOOL showDataValue,
                                    BOOL showGridLine,
                                    BOOL verticalBars,
                                    BOOL showLegend,
                                    const _TCHAR *fontFaceName
                                   ) :
                                    m_StructSize(sizeof(CRPEGraphOptions)),
                                    m_graphMaxValue(graphMaxValue),
                                    m_graphMinValue(graphMinValue),
                                    m_showDataValue(showDataValue),
                                    m_showGridLine(showGridLine),
                                    m_verticalBars(verticalBars),
                                    m_showLegend(showLegend)
{
   lstrcpyn(m_fontFaceName,fontFaceName,sizeof(m_fontFaceName) - 1);
}


CRPEParameterFieldInfo::CRPEParameterFieldInfo (WORD ValueType,
                                                WORD DefaultValueSet,
                                                WORD CurrentValueSet,
                                                const _TCHAR *name,
                                                const _TCHAR *prompt,
                                                const _TCHAR *DefaultValue,
                                                const _TCHAR *CurrentValue,
												WORD isLimited,
                                                double MinSize,
                                                double MaxSize,
                                                const _TCHAR *EditMask,
                                                WORD isHidden
                                               ) :
                                                m_StructSize(sizeof(CRPEParameterFieldInfo)),
                                                m_ValueType(ValueType),
                                                m_DefaultValueSet(DefaultValueSet),
                                                m_CurrentValueSet(CurrentValueSet),
												m_isLimited(isLimited),
                                                m_MinSize(MinSize),
                                                m_MaxSize(MaxSize),
                                                m_isHidden(isHidden)                                                
                                                
{
   lstrcpyn(m_Name,name,sizeof(m_Name) - 1);
   lstrcpyn(m_Prompt,prompt,sizeof(m_Prompt) - 1);
   lstrcpyn(m_DefaultValue,DefaultValue,sizeof(m_DefaultValue) - 1);
   lstrcpyn(m_CurrentValue,CurrentValue,sizeof(m_CurrentValue) - 1);
   lstrcpyn(m_EditMask, EditMask, sizeof(m_EditMask) - 1);
}
												
											
//////////////////
// CRPEngine Class
//////////////////

CRPEngine *CRPEngine::GetEngine(void)
{
   return CRPEngine::thePrintEngine;
}


CRPEngine::Status CRPEngine::GetEngineStatus(void)
{
	return(thePrintEngine != 0) ? thePrintEngine->m_engineStatus : engineMissing;
}


// constructors and destructor


CRPEngine::CRPEngine (BOOL open) : m_engineStatus(engineClosed),
                                   m_engineError(0)
{
	ASSERT (thePrintEngine == NULL); // only one object of CRPEngine is allowed

	CRPEngine::thePrintEngine = this;

	if (open)
		Open();
}


CRPEngine::~CRPEngine()
{
	// (j.jones 2005-09-09 13:27) - this should presumably always be called from CMainFrame::ClearReportEngine()

	ASSERT (thePrintEngine == this); // only one object of CRPEngine allowed
	
	Close();
	CRPEngine::thePrintEngine = NULL;
}


// operations


BOOL CRPEngine::Open(void)
{
	m_engineError = 0;

	if (m_engineStatus == engineOpen)
		return TRUE;

	if (PEOpenEngine()) {
		m_engineStatus = engineOpen;
		return TRUE;
	}
	else {
		m_engineStatus = engineClosed;
		return FALSE;
	}
}


void CRPEngine::Close(void)
{
	m_engineError = 0;

	if (m_engineStatus == engineOpen) {

		while (m_printJobs.GetSize())
			((CRPEJob *)m_printJobs[0]) -> Close();

		PECloseEngine();
		m_engineStatus = engineClosed;
	}
}


CRPEJob *CRPEngine::OpenJob(const _TCHAR *reportFileName)
{
	ASSERT (m_engineStatus == engineOpen); // engine must be open

	short jobHandle = 0;
	CRPEJob *newJob = NULL;

 	m_engineError = 0;

	if ((jobHandle = PEOpenPrintJob((_TCHAR *)reportFileName)) == 0)
		return NULL;

	if ((newJob = new CNxReportJob(jobHandle)) == NULL) {
		PEClosePrintJob(jobHandle);
		m_engineError = PEP_ERR_NOTENOUGHMEMORY;
		return NULL;
	}

	return newJob;
}


BOOL CRPEngine::LogOnServer(const _TCHAR *dllName,const CRPELogOnInfo *logOnInfo)
{
	// ensure CRPELogOnInfo object is size CRPE expects
	ASSERT (sizeof(CRPELogOnInfo) == sizeof(PELogOnInfo));

	ASSERT (m_engineStatus == engineOpen); // engine must be open

 	m_engineError = 0;

	return (PELogOnServer((_TCHAR *)dllName,(struct PELogOnInfo *) logOnInfo));
}


BOOL CRPEngine::LogOffServer(const _TCHAR *dllName,
							 const CRPELogOnInfo *logOnInfo)
{										   
	// ensure CRPELogOnInfo object is size CRPE expects
	ASSERT (sizeof(CRPELogOnInfo) == sizeof(PELogOnInfo));

	ASSERT (m_engineStatus == engineOpen); // engine must be open

 	m_engineError = 0;

	return (PELogOffServer((_TCHAR *)dllName,(struct PELogOnInfo *) logOnInfo));
}


BOOL CRPEngine::LogOnSQLServerWithPrivateInfo(const _TCHAR *dllName,
                                              void *privateInfo)
{
    return PELogOnSQLServerWithPrivateInfo((_TCHAR *)dllName,privateInfo);
}


short CRPEngine::PrintReport(const _TCHAR *reportFilePath,
                             BOOL toPrinter,
                             BOOL toWindow,
                             const _TCHAR *title,
                             int left,
                             int top,
                             int width,
                             int height,
                             DWORD style,
                             CWnd *parentWindow)
{
	ASSERT (m_engineStatus == engineOpen); // engine must be open

	m_engineError = 0;

	return PEPrintReport((_TCHAR *)reportFilePath,
                        toPrinter,
                        toWindow,
                        (_TCHAR *)title,
                        left,
                        top,
                        width,
                        height,
                        style,
                        parentWindow != 0 ? parentWindow->m_hWnd : 0);
}


// attributes
	

BOOL CRPEngine::CanClose()
{
    return (m_engineStatus != engineOpen) ? TRUE : PECanCloseEngine();
}


short CRPEngine::GetVersion(short versionRequested)
{
	ASSERT (m_engineStatus == engineOpen); // engine must be open

 	m_engineError = 0;

	return PEGetVersion(versionRequested);
}


short CRPEngine::GetErrorCode(void)
{
	return (m_engineError) ? m_engineError : PEGetErrorCode(0);
}


CString CRPEngine::GetErrorText(void)
{
	HANDLE textHandle;
	short textLength;
	CString errorText = "";

	if (!m_engineError)
		if (PEGetErrorText(0,&textHandle,&textLength))
			GetHandleString(textHandle,textLength,errorText);
	
	return errorText;
}


int CRPEngine::GetNPrintJobs()
{
    return m_printJobs.GetSize();
}


// implementation


void CRPEngine::AddJob(CRPEJob *job)
{
	m_printJobs.Add(job);
}


void CRPEngine::RemoveJob(CRPEJob *job)
{
	// (j.jones 2005-09-08 17:08) - this is one function that will be hit
	// when a report is closed by the user
	//
	// When a report is closed, 
	//    CNxRPEMDIChildWnd::~CNxRPEMDIChildWnd() is fired,
	// which deletes the report job, causing both
	//    CNxReportJob::~CNxReportJob() and
	//    CRPEJob::~CRPEJob() to fire in succession
	// and ~CRPEJob will call CRPEngine::RemoveJob()
	//

	int numJobs = m_printJobs.GetSize();

	while (numJobs--) {
		if (m_printJobs[numJobs] == job) {
			m_printJobs.RemoveAt(numJobs);
			break;
		}
	}

	//if this is the last job, close the engine
	if(m_printJobs.GetSize() == 0) {
		
		// (j.jones 2005-09-09 13:28) - ClearReportEngine() will delete the mainframe pointer, thus
		// causing the engine to go out of scope and be closed properly in CRPEngine::~CRPEngine()
		CMainFrame *pFrame = GetMainFrame();
		if(pFrame)
			pFrame->ClearReportEngine();
	}
}


BOOL CRPEngine::GetHandleString(HANDLE textHandle,
                                short textLength,
                                CString &string)
{
	BOOL result = FALSE;
	_TCHAR *buffer = string.GetBufferSetLength(textLength);

	if (buffer) {
		result = PEGetHandleString(textHandle,buffer,textLength);
		string.ReleaseBuffer();
	}
	else
		m_engineError = PEP_ERR_NOTENOUGHMEMORY;

	return result;
}


// CRPEngine diagnostics


#ifdef _DEBUG
void CRPEngine::AssertValid() const
{
	CObject::AssertValid();
}


void CRPEngine::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}

#endif //_DEBUG


////////////////
// CRPEJob Class
////////////////

// constructors & destructor

CRPEJob::CRPEJob(short jobHandle) : m_jobHandle(jobHandle),
                                    m_parentJob(NULL),
                                    m_mdiOutput(FALSE)
{
 	// engine object must exist
	ASSERT (CRPEngine::GetEngine() != NULL);
	// engine must be open
	ASSERT (CRPEngine::GetEngineStatus() == CRPEngine::engineOpen);
	// job handle can't be 0
	ASSERT (jobHandle != 0);

    CRPEngine::GetEngine()->AddJob(this);
}


CRPEJob::CRPEJob(short jobHandle,
                 CRPEJob *parentJob
                ) :
                  m_jobHandle(jobHandle)
{
 	// engine object must exist
	ASSERT (CRPEngine::GetEngine() != NULL);
	// engine must be open
	ASSERT (CRPEngine::GetEngineStatus() == CRPEngine::engineOpen);
	// job handle can't be 0
	ASSERT (jobHandle != 0);
    // parent job can't be NULL
    ASSERT (parentJob != NULL);

    m_parentJob = parentJob;
    m_parentJob->AddSubReportJob(this);
}


CRPEJob::~CRPEJob()
{
	// (j.jones 2005-09-08 17:08) - this is one function that will be hit
	// when a report is closed by the user
	//
	// When a report is closed, 
	//    CNxRPEMDIChildWnd::~CNxRPEMDIChildWnd() is fired,
	// which deletes the report job, causing both
	//    CNxReportJob::~CNxReportJob() and
	//    CRPEJob::~CRPEJob() to fire in succession
	// and ~CRPEJob will call CRPEngine::RemoveJob()
	//

	ASSERT (CRPEngine::GetEngine() != NULL); // engine object must exist

    while (m_subReportJobs.GetSize())
        ((CRPEJob *)m_subReportJobs[0]) -> Close();

    if (m_parentJob)
        m_parentJob->RemoveSubReportJob(this);
    else
        CRPEngine::GetEngine()->RemoveJob(this);

    PEClosePrintJob(m_jobHandle);
}


// operations

BOOL CRPEJob::GetReportOptions (CRPEReportOptions *reportOptions)
{
	return PEGetReportOptions(m_jobHandle,
                              (struct PEReportOptions *) reportOptions);
}


BOOL CRPEJob::SetReportOptions (CRPEReportOptions *reportOptions)
{
	return PESetReportOptions(m_jobHandle,
                              (struct PEReportOptions *) reportOptions);
}

BOOL CRPEJob::GetReportSummaryInfo(CRPEReportSummaryInfo *summaryInfo)
{
    ASSERT (sizeof(CRPEReportSummaryInfo) == sizeof(PEReportSummaryInfo));

    return PEGetReportSummaryInfo(m_jobHandle, 
                                  (struct PEReportSummaryInfo *) summaryInfo);
}

BOOL CRPEJob::SetReportSummaryInfo(CRPEReportSummaryInfo *summaryInfo)
{
    ASSERT (sizeof(CRPEReportSummaryInfo) == sizeof(PEReportSummaryInfo));

    return PESetReportSummaryInfo(m_jobHandle, 
                                  (struct PEReportSummaryInfo *) summaryInfo);
}

BOOL CRPEJob::Start()
{
    if (m_mdiOutput)
    {
        CNxRPEMDIChildWnd *child = new CNxRPEMDIChildWnd;

		// TODO: This is pretty unconventional: calling a derived-class function; it's only ok in this case because CNxReportJob doesn't have any member variables of its own
		(reinterpret_cast<CNxReportJob *>(this))->GatherPrintInfo(&(child->m_objPrintInfo));

        CRect rect(m_mdiLeft,m_mdiTop,m_mdiLeft + m_mdiWidth,m_mdiTop + m_mdiHeight);
        
        child->Create(NULL,
                      m_mdiTitle,
                      WS_CHILD | WS_VISIBLE | WS_OVERLAPPEDWINDOW | m_mdiStyle,
                      rect);


        child->GetClientRect(rect);

		// TODO: Set the Window options (CRPEWindowOptions) to not show the print button (right 
		// now we leave the print button, but handle the PEP_PRINT_BUTTON_CLICKED_EVENT event)

		//////////

        // Send the report to the window
		if (!PEOutputToWindow(m_jobHandle,
                              0,
                              (int)rect.left,
                              (int)rect.top,
                              (int)rect.Width (),
                              (int)rect.Height (),
                              WS_CHILD | WS_VISIBLE,
                              child->m_hWnd)) {
			// Failure
            return FALSE;
		}
    	
        // Run the report
		if (!PEStartPrintJob(m_jobHandle,TRUE)) {
			// Failure
            return FALSE;
		}

        if ((child->m_printWnd = GetWindowHandle()) != NULL) {
			// Success
			child->m_pJob = this;

			return 2;
		} else {
			// Failure
			return FALSE;
		}
    }

	return PEStartPrintJob(m_jobHandle,TRUE);
}

void CRPEJob::Close()
{
	delete this;
}

void CRPEJob::Cancel()
{
	PECancelPrintJob(m_jobHandle);
}


BOOL CRPEJob::ShowNextPage(void)
{
	return PEShowNextPage(m_jobHandle);
}

		
BOOL CRPEJob::ShowFirstPage(void)
{
	return PEShowFirstPage(m_jobHandle);
}


BOOL CRPEJob::ShowPreviousPage(void)
{
	return PEShowPreviousPage(m_jobHandle);
}


BOOL CRPEJob::ShowLastPage(void)
{
	return PEShowLastPage(m_jobHandle);
}


BOOL CRPEJob::ShowNthPage(short pageN)
{
    return PEShowNthPage(m_jobHandle,pageN);
}


BOOL CRPEJob::ShowPrintControls(BOOL showControls)
{
	/*
	Run like this makes it so that the whole crystal toolbar disappears
	return PEShowPrintControls(m_jobHandle,FALSE);*/
	
	return PEShowPrintControls(m_jobHandle,showControls);
}


BOOL CRPEJob::ZoomPreviewWindow(short level)
{
	return PEZoomPreviewWindow(m_jobHandle,level);
}


BOOL CRPEJob::NextWindowMagnification(void)
{
	return PENextPrintWindowMagnification(m_jobHandle);
}


BOOL CRPEJob::PrintWindow(void)
{
	return PEPrintWindow(m_jobHandle,TRUE);
}


BOOL CRPEJob::ExportPrintWindow(BOOL toMail)
{
	return PEExportPrintWindow(m_jobHandle,toMail,TRUE);
}

	
void CRPEJob::CloseWindow(void)
{
	PECloseWindow(m_jobHandle);
}


BOOL CRPEJob::TestNthTableConnectivity(short tableN)
{
	return PETestNthTableConnectivity(m_jobHandle,tableN);
}


BOOL CRPEJob::DiscardSavedData (void)
{
    return PEDiscardSavedData(m_jobHandle);
}


// report fomula text attributes


short CRPEJob::GetNFormulas()
{
	return PEGetNFormulas(m_jobHandle);
}


BOOL CRPEJob::GetNthFormula(short formulaN,
                            CString &formulaName,
                            CString &formulaText)
{
	HANDLE nameHandle;
	short nameLength;
	HANDLE textHandle;
	short textLength;

	BOOL result = FALSE;

	formulaName = "";
	formulaText = "";

	if (PEGetNthFormula(m_jobHandle,
	                    formulaN,
                       &nameHandle,
                       &nameLength,
                       &textHandle,
                       &textLength))
		if ((result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                            textLength,
                                                            formulaText)))
            result = CRPEngine::GetEngine()->GetHandleString(nameHandle,
                                                             nameLength,
                                                             formulaName);


	return result;
}


BOOL CRPEJob::GetFormula(const _TCHAR *formulaName,
                         CString &formulaText)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	formulaText = "";

	if (PEGetFormula(m_jobHandle,
                    (_TCHAR *)formulaName,
                    &textHandle,
                    &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       formulaText);

	return result;
}


BOOL CRPEJob::SetFormula(const _TCHAR *formulaName,
                         const _TCHAR *formulaText)
{
	return PESetFormula(m_jobHandle,(_TCHAR *)formulaName,(_TCHAR *)formulaText);
}


BOOL CRPEJob::CheckFormula(const _TCHAR *formulaName)
{
    return PECheckFormula(m_jobHandle,(_TCHAR *)formulaName);
}


// record selection formula attributes


BOOL CRPEJob::GetSelectionFormula(CString &formulaText)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	formulaText = "";

	if (PEGetSelectionFormula(m_jobHandle,
                             &textHandle,
                             &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       formulaText);

	return result;
}


BOOL CRPEJob::SetSelectionFormula(const _TCHAR *formulaText)
{
	return PESetSelectionFormula(m_jobHandle,(_TCHAR *)formulaText);
}


BOOL CRPEJob::CheckSelectionFormula ()
{
    return PECheckSelectionFormula(m_jobHandle);
}


// group selection formula attributes


BOOL CRPEJob::GetGroupSelectionFormula(CString &formulaText)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	formulaText = "";

	if (PEGetGroupSelectionFormula(m_jobHandle,
                                  &textHandle,
                                  &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       formulaText);

	return result;
}


BOOL CRPEJob::SetGroupSelectionFormula(const _TCHAR *formulaText)
{
	return PESetGroupSelectionFormula(m_jobHandle,(_TCHAR *)formulaText);
}


BOOL CRPEJob::CheckGroupSelectionFormula()
{
    return PECheckGroupSelectionFormula(m_jobHandle);
}

//SQL Expressions
short CRPEJob::GetNSQLExpressions()
{
	return PEGetNSQLExpressions(m_jobHandle);
}


BOOL CRPEJob::GetNthSQLExpression(short expressionN,
                                  CString &expressionName,
                                  CString &expressionText)
{
	HANDLE nameHandle;
	short nameLength;
	HANDLE textHandle;
	short textLength;

	BOOL result = FALSE;

	expressionName = "";
	expressionText = "";

	if (PEGetNthSQLExpression(m_jobHandle,
	                          expressionN,
                              &nameHandle,
                              &nameLength,
                              &textHandle,
                              &textLength))

		if ((result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                              textLength,
                                                              expressionText)))

            result = CRPEngine::GetEngine()->GetHandleString(nameHandle,
                                                             nameLength,
                                                             expressionName);

	return result;
}


BOOL CRPEJob::GetSQLExpression(const _TCHAR *expressionName,
                               CString &expressionText)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	expressionText = "";

	if (PEGetSQLExpression(m_jobHandle,
                           (_TCHAR *)expressionName,
                           &textHandle,
                           &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                         textLength,
                                                         expressionText);

	return result;
}


BOOL CRPEJob::SetSQLExpression(const _TCHAR *expressionName,
                               const _TCHAR *expressionText)
{
	return PESetSQLExpression(m_jobHandle,(_TCHAR *)expressionName,(_TCHAR *)expressionText);
}


BOOL CRPEJob::CheckSQLExpression(const _TCHAR *expressionName)
{
    return PECheckSQLExpression(m_jobHandle,(_TCHAR *)expressionName);
}

// group condition attributes


short CRPEJob::GetNGroups ()
{
    return PEGetNGroups(m_jobHandle);
}


BOOL CRPEJob::GetGroupCondition (short sectionCode,
                                 CString &conditionField,
                                 short *condition,
                                 short *sortDirection)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	conditionField = "";

	if (PEGetGroupCondition(m_jobHandle,
                           sectionCode,
                           &textHandle,
                           &textLength,
                           condition,
                           sortDirection))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       conditionField);

	return result;
}


BOOL CRPEJob::SetGroupCondition(short sectionCode,
                                const _TCHAR *conditionField,
                                short condition,
                                short sortDirection)
{
	return PESetGroupCondition(m_jobHandle,
                              sectionCode,
                              (_TCHAR *)conditionField,
                              condition,
                              sortDirection);
}


// record sort order field attributes


short CRPEJob::GetNSortFields(void)
{
	return PEGetNSortFields(m_jobHandle);
}


BOOL CRPEJob::GetNthSortField(short sortFieldN,
                              CString &field,
                              short *direction)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	field = "";

	if (PEGetNthSortField(m_jobHandle,
                         sortFieldN,
                         &textHandle,
                         &textLength,
                         direction))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       field);

	return result;
}


BOOL CRPEJob::SetNthSortField(short sortFieldN,
                              const _TCHAR *field,
                              short direction)
{
	return PESetNthSortField(m_jobHandle,sortFieldN,(_TCHAR *)field,direction);
}


BOOL CRPEJob::DeleteNthSortField(short sortFieldN)
{
	return PEDeleteNthSortField(m_jobHandle,sortFieldN);
}


// group sort order field attributes


short CRPEJob::GetNGroupSortFields(void)
{
	return PEGetNGroupSortFields(m_jobHandle);
}


BOOL CRPEJob::GetNthGroupSortField(short sortFieldN,
                                   CString &field,
                                   short *direction)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	field = "";

	if (PEGetNthGroupSortField(m_jobHandle,
                              sortFieldN,
                              &textHandle,
                              &textLength,
                              direction))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       field);

	return result;
}

								   	
BOOL CRPEJob::SetNthGroupSortField(short sortFieldN,
                                   const _TCHAR *field,
                                   short direction)
{
	return PESetNthGroupSortField(m_jobHandle,
                                 sortFieldN,
                                 (_TCHAR *)field,
                                 direction);
}


BOOL CRPEJob::DeleteNthGroupSortField(short sortFieldN)
{
	return PEDeleteNthGroupSortField(m_jobHandle,sortFieldN);
}


// database table attributes


short CRPEJob::GetNTables(void)
{
	return PEGetNTables(m_jobHandle);
}


BOOL CRPEJob::GetNthTableType(short tableN,
                              CRPETableType *tableType)
{
	return PEGetNthTableType(m_jobHandle,
                            tableN,
                            (struct PETableType *) tableType);
}


BOOL CRPEJob::GetNthTableSessionInfo(short tableN,
                                     CRPESessionInfo *sessionInfo)
{
	return PEGetNthTableSessionInfo(m_jobHandle,
                                   tableN,
                                   (struct PESessionInfo *) sessionInfo);
}


BOOL CRPEJob::SetNthTableSessionInfo(short tableN,
                                     const CRPESessionInfo *sessionInfo,
                                     BOOL propagate)
{
	// ensure CRPESessionInfo object is size CRPE expects
	ASSERT (sizeof(CRPESessionInfo) == sizeof(PESessionInfo));

	return PESetNthTableSessionInfo(m_jobHandle,
                                   tableN,
                                   (struct PESessionInfo *) sessionInfo,
                                   propagate);
}


BOOL CRPEJob::GetNthTableLocation(short tableN,
                                  CRPETableLocation *tableLocation)
{
	return PEGetNthTableLocation(m_jobHandle,
                                tableN,
                                (struct PETableLocation *) tableLocation);
}


BOOL CRPEJob::SetNthTableLocation(short tableN,
                                  const CRPETableLocation *tableLocation)
{
	// ensure CRPETableLocation object is size CRPE expects
	ASSERT (sizeof(CRPETableLocation) == sizeof(PETableLocation));

	return PESetNthTableLocation(m_jobHandle,
                                tableN,
                                (struct PETableLocation *) tableLocation);
}


BOOL CRPEJob::GetNthTableLogonInfo(short tableN,
                                   CRPELogOnInfo *logonInfo)
{
	return PEGetNthTableLogOnInfo(m_jobHandle,
                                 tableN,
                                 (struct PELogOnInfo *) logonInfo);
}


BOOL CRPEJob::SetNthTableLogonInfo(short tableN,
                                   const CRPELogOnInfo *logonInfo,
                                   BOOL propagate)
{
	// ensure CRPELogOnInfo object is size CRPE expects
	ASSERT (sizeof(CRPELogOnInfo) == sizeof(PELogOnInfo));

	return PESetNthTableLogOnInfo(m_jobHandle,
                                 tableN,
                                 (struct PELogOnInfo *) logonInfo,
                                 propagate);
}


// SQL query attributes


BOOL CRPEJob::GetSQLQuery(CString &query)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	query = "";

	if (PEGetSQLQuery(m_jobHandle,
                     &textHandle,
                     &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       query);

	return result;
}


BOOL CRPEJob::SetSQLQuery(const _TCHAR *query)
{
	return PESetSQLQuery(m_jobHandle,(_TCHAR *)query);
}

BOOL CRPEJob::VerifyDatabase()
{
	return PEVerifyDatabase(m_jobHandle);
}

// report section attributes


BOOL CRPEJob::GetNDetailCopies(short *nCopies)
{
    return PEGetNDetailCopies(m_jobHandle,nCopies);
}


BOOL CRPEJob::SetNDetailCopies(short nCopies)
{
	return PESetNDetailCopies(m_jobHandle, nCopies);
}

BOOL CRPEJob::GetGroupOptions(short groupN,
                              CRPEGroupOptions *groupOptions)
{
    ASSERT (sizeof(CRPEGroupOptions) == sizeof(PEGroupOptions));

    return PEGetGroupOptions(m_jobHandle,
                             groupN,
                             (struct PEGroupOptions *) groupOptions);
}

BOOL CRPEJob::SetGroupOptions(short groupN,
                              CRPEGroupOptions *groupOptions)
{
    ASSERT (sizeof(CRPEGroupOptions) == sizeof(PEGroupOptions));

    return PESetGroupOptions(m_jobHandle,
                             groupN,
                             (struct PEGroupOptions *) groupOptions);
}

short CRPEJob::GetNSections ()
{
    return PEGetNSections(m_jobHandle);
}


short CRPEJob::GetSectionCode (short sectionN)
{
    return PEGetSectionCode(m_jobHandle,sectionN);
}

BOOL CRPEJob::GetSectionHeight(short sectionCode,
                               short *height)
{
	return PEGetSectionHeight(m_jobHandle,
                              sectionCode,
                              height);
}


BOOL CRPEJob::SetSectionHeight(short sectionCode,
                               short height)
{
	return PESetSectionHeight(m_jobHandle,
                              sectionCode,
                              height);
}

BOOL CRPEJob::SetFont(short sectionCode,
                      short scopeCode,
                      const _TCHAR *faceName,
                      short fontFamily,
                      short fontPitch,
                      short charSet,
                      short pointSize,
                      short isItalic,
                      short isUnderlined,
                      short isStruckOut,
                      short weight)
{
	return PESetFont(m_jobHandle,
                    sectionCode,
                    scopeCode,
                    (_TCHAR *)faceName,
                    fontFamily,
                    fontPitch,
                    charSet,
                    pointSize,
                    isItalic,
                    isUnderlined,
                    isStruckOut,
                    weight);
}


BOOL CRPEJob::GetSectionFormat (short sectionCode,
                                CRPESectionOptions *options)
{
    // ensure CRPESectionOptions is size CRPE expects
    ASSERT (sizeof(CRPESectionOptions) == sizeof(PESectionOptions));

    return PEGetSectionFormat(m_jobHandle,
                              sectionCode,
                              (struct PESectionOptions *)options);
}


BOOL CRPEJob::SetSectionFormat (short sectionCode,
                                const CRPESectionOptions *options)
{
    // ensure CRPESectionOptions is size CRPE expects
    ASSERT (sizeof(CRPESectionOptions) == sizeof(PESectionOptions));

    return PESetSectionFormat (m_jobHandle,
                               sectionCode,
                               (struct PESectionOptions *)options);
}

BOOL CRPEJob::GetAreaFormatFormula(short areaCode,
                                   short formulaName, // an area PEP_FFN_ constant
                                   CString &formulaText)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	formulaText = "";

	if (PEGetAreaFormatFormula(m_jobHandle,
                               areaCode,
                               formulaName,
                               &textHandle,
                               &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       formulaText);

	return result;
}

BOOL CRPEJob::SetAreaFormatFormula(short areaCode,
                                   short formulaName, // an area PEP_FFN_ constant
                                   CString formulaText)
{
	return PESetAreaFormatFormula(m_jobHandle, 
                                  areaCode, 
                                  formulaName,
                                  formulaText);
}


BOOL CRPEJob::GetSectionFormatFormula(short areaCode,
                                      short sectionCode,
                                      short formulaName, // an area PEP_FFN_ constant
                                      CString &formulaText)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	formulaText = "";

	if (PEGetSectionFormatFormula(m_jobHandle,
                                  sectionCode,
                                  formulaName,
                                  &textHandle,
                                  &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       formulaText);

	return result;
}

BOOL CRPEJob::SetSectionFormatFormula(short sectionCode,
                                      short formulaName, // an area PEP_FFN_ constant
                                      CString formulaText)
{
	return PESetSectionFormatFormula(m_jobHandle, 
                                     sectionCode, 
                                     formulaName,
                                     formulaText);
}

// graphing attributes


BOOL CRPEJob::GetGraphType (short sectionCode,
                            short graphN,
                            short *graphType)
{
    return PEGetGraphType(m_jobHandle,
                          sectionCode,
                          graphN,
                          graphType);
}


BOOL CRPEJob::SetGraphType (short sectionCode,
                            short graphN,
                            short graphType)
{
    return PESetGraphType (m_jobHandle,
                           sectionCode,
                           graphN,
                           &graphType);
}


BOOL CRPEJob::GetGraphData (short sectionCode,
                            short graphN,
                            CRPEGraphDataInfo *graphDataInfo)
{
	// ensure CRPEGraphDataInfo is size CRPE expects
	ASSERT (sizeof(CRPEGraphDataInfo) == sizeof(PEGraphDataInfo));

    return PEGetGraphData (m_jobHandle,
                           sectionCode,
                           graphN,
                           (struct PEGraphDataInfo *) graphDataInfo);
}


BOOL CRPEJob::SetGraphData (short sectionCode,
                            short graphN,
                            CRPEGraphDataInfo *graphDataInfo)
{
	// ensure CRPEGraphDataInfo is size CRPE expects
	ASSERT (sizeof(CRPEGraphDataInfo) == sizeof(PEGraphDataInfo));
    
    return PESetGraphData (m_jobHandle,
                           sectionCode,
                           graphN,
                           (struct PEGraphDataInfo *) graphDataInfo);
}


BOOL CRPEJob::GetGraphText (short sectionCode,
                            short graphN,
                            CRPEGraphTextInfo *graphTextInfo)
{
	// ensure CRPEGraphTextInfo is size CRPE expects
	ASSERT (sizeof(CRPEGraphTextInfo) == sizeof(PEGraphTextInfo));

    return PEGetGraphText (m_jobHandle,
                           sectionCode,
                           graphN,
                           (struct PEGraphTextInfo *) graphTextInfo);
}


BOOL CRPEJob::SetGraphText (short sectionCode,
                            short graphN,
                            CRPEGraphTextInfo *graphTextInfo)
{
	// ensure CRPEGraphTextInfo is size CRPE expects
	ASSERT (sizeof(CRPEGraphTextInfo) == sizeof(PEGraphTextInfo));

    return PESetGraphText (m_jobHandle,
                           sectionCode,
                           graphN,
                           (struct PEGraphTextInfo *) graphTextInfo);
}


BOOL CRPEJob::GetGraphOptions (short sectionCode,
                               short graphN,
                               CRPEGraphOptions *graphOptions)
{
	// ensure CRPEGraphOptions is size CRPE expects
	ASSERT (sizeof(CRPEGraphOptions) == sizeof(PEGraphOptions));

    return PEGetGraphOptions (m_jobHandle,
                              sectionCode,
                              graphN,
                              (struct PEGraphOptions *) graphOptions);
}


BOOL CRPEJob::SetGraphOptions (short sectionCode,
                               short graphN,
                               CRPEGraphOptions *graphOptions)
{
	// ensure CRPEGraphOptions is size CRPE expects
	ASSERT (sizeof(CRPEGraphOptions) == sizeof(PEGraphOptions));

    return PESetGraphOptions (m_jobHandle,
                              sectionCode,
                              graphN,
                              (struct PEGraphOptions *) graphOptions);
}


// report attributes


BOOL CRPEJob::GetReportTitle (CString &title)
{
	HANDLE textHandle;
	short textLength;
	BOOL result = FALSE;

	title = "";

	if (PEGetReportTitle(m_jobHandle,
                        &textHandle,
                        &textLength))
		result = CRPEngine::GetEngine()->GetHandleString(textHandle,
                                                       textLength,
                                                       title);

	return result;
}


BOOL CRPEJob::SetReportTitle(const _TCHAR *title)
{
	return PESetReportTitle(m_jobHandle,(_TCHAR *)title);
}


BOOL CRPEJob::GetPrintDate (short *year,
                            short *month,
                            short *day)
{
    return PEGetPrintDate(m_jobHandle,year,month,day);
}


BOOL CRPEJob::SetPrintDate(short year,
                           short month,
                           short day)
{
	return PESetPrintDate(m_jobHandle,year,month,day);
}


BOOL CRPEJob::HasSavedData (BOOL *hasSavedData)
{
    return PEHasSavedData(m_jobHandle,hasSavedData);
}


BOOL CRPEJob::GetWindowOptions (CRPEWindowOptions *options)
{
    return PEGetWindowOptions (m_jobHandle,(struct PEWindowOptions *)options);
}


BOOL CRPEJob::SetWindowOptions(const CRPEWindowOptions *options)
{
	// ensure CRPEWindowOptions is size CRPE expects
	ASSERT (sizeof(CRPEWindowOptions) == sizeof(PEWindowOptions));

	return PESetWindowOptions(m_jobHandle,
                            (struct PEWindowOptions *) options);
}

BOOL CRPEJob::GetFieldMappingType (WORD mappingType) //use PE_FM_ constant
{
    return PEGetFieldMappingType(m_jobHandle, &mappingType);    
}

BOOL CRPEJob::SetFieldMappingType (WORD mappingType) //use PE_FM_ constant
{
    return PESetFieldMappingType(m_jobHandle, mappingType);    
}

BOOL CRPEJob::SetTrackCursorInfo(CRPETrackCursorInfo *cursorInfo)
{
    return PESetTrackCursorInfo(m_jobHandle,
							    (struct PETrackCursorInfo *) cursorInfo);
}

BOOL CRPEJob::GetTrackCursorInfo(CRPETrackCursorInfo *cursorInfo)
{
    return PEGetTrackCursorInfo(m_jobHandle,
							    (struct PETrackCursorInfo *) cursorInfo);
}


BOOL CRPEJob::EnableEvent (const CRPEEnableEventInfo *enableEventInfo)
{
    return PEEnableEvent (m_jobHandle,(struct PEEnableEventInfo *)enableEventInfo);
}

BOOL CRPEJob::GetEnableEventInfo (CRPEEnableEventInfo *enableEventInfo)
{
    return PEGetEnableEventInfo (m_jobHandle,(struct PEEnableEventInfo *)enableEventInfo);
}

BOOL CRPEJob::SetEventCallback ( BOOL ( CALLBACK * callbackProc )(short eventID, void *param, void *userData), void *userData)
{

	return PESetEventCallback ( m_jobHandle, callbackProc, (void *) userData);
}

short CRPEJob::GetNPages ()
{
    return PEGetNPages(m_jobHandle);
}


// report printer attributes


BOOL CRPEJob::GetSelectedPrinter (CString &driverName,
                                  CString &printerName,
                                  CString &portName,
                                  DEVMODE **mode)
{
   HANDLE driverHandle, printerHandle, portHandle;
   short driverLength, printerLength, portLength;
   BOOL result = FALSE;

   driverName = "";
   printerName = "";
   portName = "";

	if (PEGetSelectedPrinter(m_jobHandle,
                            &driverHandle,
                            &driverLength,
                            &printerHandle,
                            &printerLength,
                            &portHandle,
                            &portLength,
                            mode))
		if (CRPEngine::GetEngine()->GetHandleString(driverHandle,
                                                  driverLength,
                                                  driverName))
            if (CRPEngine::GetEngine()->GetHandleString(printerHandle,
                                                        printerLength,
                                                        printerName))
                result = CRPEngine::GetEngine()->GetHandleString(portHandle,
                                                                 portLength,
                                                                 portName);

	return result;
}


BOOL CRPEJob::SelectPrinter(const _TCHAR *driverName,
                            const _TCHAR *printerName,
                            const _TCHAR *portName,
                            const DEVMODE *mode)
{
	return PESelectPrinter(m_jobHandle,
                          (_TCHAR *)driverName,
                          (_TCHAR *)printerName,
                          (_TCHAR *)portName,
                          (DEVMODE *)mode);
}


BOOL CRPEJob::GetPrintOptions (CRPEPrintOptions *options)
{
    return PEGetPrintOptions (m_jobHandle,(struct PEPrintOptions *)options);
}


BOOL CRPEJob::SetPrintOptions(const CRPEPrintOptions *options)
{
	// ensure CRPEPrintOptions is size CRPE expects
	ASSERT (sizeof(CRPEPrintOptions) == sizeof(PEPrintOptions));

	return PESetPrintOptions(m_jobHandle,
                            (struct PEPrintOptions *) options);
}


// output destination options


BOOL CRPEJob::OutputToPrinter(short nCopies)
{
    m_mdiOutput = FALSE;

    return PEOutputToPrinter(m_jobHandle,nCopies);
}


BOOL CRPEJob::OutputToWindow(const _TCHAR *title,
                             int left,
                             int top,
                             int width,
                             int height,
                             int style,
                             CWnd *parentWindow)
{
    m_mdiOutput = FALSE;

    return PEOutputToWindow(m_jobHandle,
                           (_TCHAR *)title,
                           left,
                           top,
                           width,
                           height,
                           style,
                           parentWindow != 0 ? parentWindow->m_hWnd : 0);
}


BOOL CRPEJob::OutputToWindow(const char *title,
                             int left,
                             int top,
                             int width,
                             int height,
                             int style,
                             CMDIFrameWnd *parentWindow)
{
    ASSERT (parentWindow != 0);
    
    m_mdiOutput = TRUE;
    m_mdiTitle = title;
    m_mdiLeft = left;
    m_mdiTop = top;
    m_mdiWidth = width;
    m_mdiHeight = height;
    m_mdiStyle = style;
    m_mdiFrameWnd = parentWindow;

    return TRUE;
}


HWND CRPEJob::GetWindowHandle(void)
{
	return PEGetWindowHandle(m_jobHandle);
}


BOOL CRPEJob::PrintControlsShowing(BOOL *controlsShowing)
{
    return PEPrintControlsShowing(m_jobHandle,controlsShowing);
}


BOOL CRPEJob::GetExportOptions(CRPEExportOptions *options)
{
    return PEGetExportOptions(m_jobHandle,
                              (struct PEExportOptions *) options);
}


BOOL CRPEJob::ExportTo(const CRPEExportOptions *options)
{
	// ensure CRPEExportOptions is size CRPE expects
	ASSERT (sizeof(CRPEExportOptions) == sizeof(PEExportOptions));

    m_mdiOutput = FALSE;

    return PEExportTo(m_jobHandle,
                      (struct PEExportOptions *) options);
}


BOOL CRPEJob::GetMargins(short *left,
                         short *right,
                         short *top,
                         short *bottom)
{
	return PEGetMargins(m_jobHandle,
                       left,
                       right,
                       top,
                       bottom);
}


BOOL CRPEJob::SetMargins(short left,
                         short right,
                         short top,
                         short bottom)
{
	return PESetMargins(m_jobHandle,
                       left,
                       right,
                       top,
                       bottom);
}


// job status attributes
short CRPEJob::GetJobStatus(CRPEJobInfo *jobStatus)
{
	return PEGetJobStatus(m_jobHandle,(struct PEJobInfo *)jobStatus);
}


BOOL CRPEJob::IsJobFinished(void)
{
	return PEIsPrintJobFinished(m_jobHandle);
}


short CRPEJob::GetErrorCode(void)
{
	return PEGetErrorCode(m_jobHandle);
}


CString CRPEJob::GetErrorText(void)
{
	HANDLE textHandle;
	short textLength;
	CString errorText = "";

	if (PEGetErrorText(m_jobHandle,&textHandle,&textLength))
		CRPEngine::GetEngine()->GetHandleString(textHandle,
                                              textLength,
                                              errorText);
	
	return errorText;
}


// dialog control attributes
BOOL CRPEJob::SetDialogParentWindow(CWnd *parentWindow)
{
    return PESetDialogParentWindow(m_jobHandle,
                                   parentWindow != 0 ? parentWindow->m_hWnd : 0);
}


BOOL CRPEJob::EnableProgressDialog(BOOL enable)
{
    return PEEnableProgressDialog(m_jobHandle, enable);
}


// Controlling Paremeter Field Prompting Dialog
// Set boolean to indicate whether CRPE is allowed to prompt for parameter values
// during printing.

BOOL CRPEJob::GetAllowPromptDialog(void)
{
	return PEGetAllowPromptDialog(m_jobHandle);
}

BOOL CRPEJob::SetAllowPromptDialog(BOOL showPromptDialog)
{
	return PESetAllowPromptDialog(m_jobHandle, showPromptDialog);
}

/********************************************************************************/
// NOTE : Stored Procedures
//
// The previous Stored Procedure Functions GetNParams, GetNthParam,
// GetNthParamInfo and SetNthParam have been made obsolete.  Older 
// applications that used these Functions will still work as before, but for new 
// development please use the new Parameter Functions below, 
//
// The Stored Procedure Parameters have now been unified with the Parameter 
// Fields.
//
// The replacements for these calls are as follows : 
//		GetNParams		= GetNParameterFields
//		GetNthParam		= GetNthParameterField
//		GetNthParamInfo	= GetNthParameterValueInfo
//		SetNthParam		= SetNthParameterField
//
// NOTE : To tell if a Parameter Field is a Stored Procedure, use the 
//		  GetNthParameterType or GetNthParameterField Functions
//
// If you wish to SET a parameter to NULL then set the CurrentValue to CRWNULL.
// The CRWNULL is of Type String and is independant of the datatype of the 
// parameter.
//
/********************************************************************************/

// parameter field attributes
short CRPEJob::GetNParameterFields(void)
{
    return PEGetNParameterFields(m_jobHandle);
}


BOOL CRPEJob::GetNthParameterField(short parameterN,
                                   CRPEParameterFieldInfo *parameterInfo)
{
    // ensure CRPEParameterFieldInfo object is size CRPE expects
    ASSERT (sizeof(CRPEParameterFieldInfo) == sizeof(PEParameterFieldInfo));

    return PEGetNthParameterField(m_jobHandle,
                                  parameterN,
                                  (struct PEParameterFieldInfo *) parameterInfo);
}


BOOL CRPEJob::SetNthParameterField(short parameterN,
                                   const CRPEParameterFieldInfo *parameterInfo)
{
    // ensure CRPEParameterFieldInfo object is size CRPE expects
    ASSERT (sizeof(CRPEParameterFieldInfo) == sizeof(PEParameterFieldInfo));

    return PESetNthParameterField(m_jobHandle,
                                  parameterN,
                                  (struct PEParameterFieldInfo *) parameterInfo);
}

// Default values for Parameter fields.
// ////////////////////////////////////
short CRPEJob::GetNParameterDefaultValues (const _TCHAR *parameterFieldName, 
                                           const _TCHAR *reportName)
{
    return PEGetNParameterDefaultValues (m_jobHandle,
                                         (_TCHAR *)parameterFieldName, 
                                         (_TCHAR *)reportName);
}

BOOL CRPEJob::GetNthParameterDefaultValue (const _TCHAR *parameterFieldName, 
                                           const _TCHAR *reportName, 
                                           short index, 
                                           CRPEValueInfo *valueInfo)
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEGetNthParameterDefaultValue(m_jobHandle,
                                         (_TCHAR *)parameterFieldName, 
                                         (_TCHAR *)reportName,
                                         index,
                                         (struct PEValueInfo *) valueInfo);
}

BOOL  CRPEJob::SetNthParameterDefaultValue (const _TCHAR *parameterFieldName, 
                                            const _TCHAR *reportName, 
                                            short index, 
                                            CRPEValueInfo *valueInfo)
{    
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PESetNthParameterDefaultValue(m_jobHandle,
                                         (_TCHAR *)parameterFieldName, 
                                         (_TCHAR *)reportName,
                                         index,
                                         (struct PEValueInfo *) valueInfo);
}


BOOL CRPEJob::AddParameterDefaultValue(const _TCHAR *parameterFieldName, 
                                       const _TCHAR *reportName, 
                                       CRPEValueInfo *valueInfo)
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEAddParameterDefaultValue (m_jobHandle,
                                       (_TCHAR *)parameterFieldName, 
                                       (_TCHAR *)reportName,
                                       (struct PEValueInfo *) valueInfo);
}

BOOL CRPEJob::DeleteNthParameterDefaultValue(const _TCHAR *parameterFieldName, 
                                             const _TCHAR *reportName, 
                                             short index)
{
    return PEDeleteNthParameterDefaultValue(m_jobHandle,
                                            (_TCHAR *)parameterFieldName, 
                                            (_TCHAR *)reportName,
                                            index);
}

// Min/Max values for Parameter fields.
// ////////////////////////////////////
BOOL CRPEJob::GetParameterMinMaxValue(const _TCHAR *parameterFieldName,
                                      const _TCHAR *reportName,
                                      CRPEValueInfo *valueMin, // Set to NULL to retrieve MAX only; must be non-NULL if valueMax is NULL.
                                      CRPEValueInfo *valueMax  // Set to NULL to retrieve MIN only; must be non-NULL if valueMin is NULL.
                                     )
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEGetParameterMinMaxValue (m_jobHandle,
                                      (_TCHAR *)parameterFieldName, 
                                      (_TCHAR *)reportName,
                                      (struct PEValueInfo *) valueMin,
                                      (struct PEValueInfo *) valueMax);
}

BOOL CRPEJob::SetParameterMinMaxValue(const _TCHAR *parameterFieldName,
                                      const _TCHAR *reportName,
                                      CRPEValueInfo *valueMin, // Set to NULL to set MAX only; must be non-NULL if valueMax is NULL.
                                      CRPEValueInfo *valueMax  // Set to NULL to set MIN only; must be non-NULL if valueMin is NULL.
                                                                    // If both valueInfo and valueMax are non-NULL then
                                                                    // valueMin->valueType MUST BE THE SAME AS valueMax->valueType.
                                                                    // If different, PE_ERR_INCONSISTANTTYPES is returned.
                                     )
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PESetParameterMinMaxValue (m_jobHandle,
                                      (_TCHAR *)parameterFieldName, 
                                      (_TCHAR *)reportName,
                                      (struct PEValueInfo *) valueMin,
                                      (struct PEValueInfo *) valueMax);
}

BOOL CRPEJob::GetParameterValueInfo(const _TCHAR *parameterFieldName, 
                                    const _TCHAR *reportName, 
                                    CRPEParameterValueInfo *valueInfo)
{
    // ensure CRPEParameterValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEParameterValueInfo) == sizeof(PEParameterValueInfo));

    return PEGetParameterValueInfo(m_jobHandle,
                                   (_TCHAR *)parameterFieldName, 
                                   (_TCHAR *)reportName, 
                                   (struct PEParameterValueInfo *) valueInfo);
}

BOOL CRPEJob::SetParameterValueInfo(const _TCHAR *parameterFieldName, 
                                    const _TCHAR *reportName, 
                                    CRPEParameterValueInfo *valueInfo)
{
    // ensure CRPEParameterValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEParameterValueInfo) == sizeof(PEParameterValueInfo));

    return PESetParameterValueInfo(m_jobHandle,
                                   (_TCHAR *)parameterFieldName, 
                                   (_TCHAR *)reportName, 
                                   (struct PEParameterValueInfo *) valueInfo);
}

unsigned short CRPEJob::GetNParameterCurrentValues(const _TCHAR *parameterFieldName, 
                                                   const _TCHAR *reportName)
{

    return PEGetNParameterCurrentValues(m_jobHandle,
                                        (_TCHAR *)parameterFieldName, 
                                        (_TCHAR *)reportName);
}

BOOL CRPEJob::GetNthParameterCurrentValue(const _TCHAR *parameterFieldName, 
                                          const _TCHAR *reportName, 
                                          short index, 
                                          CRPEValueInfo *currentValue)
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEGetNthParameterCurrentValue(m_jobHandle,
                                         (_TCHAR *)parameterFieldName, 
                                         (_TCHAR *)reportName,
                                         index,
                                         (struct PEValueInfo *) currentValue);
}

BOOL CRPEJob::AddParameterCurrentValue(const _TCHAR *parameterFieldName, 
                                       const _TCHAR *reportName, 
                                       CRPEValueInfo *currentValue)
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEAddParameterCurrentValue (m_jobHandle,
                                       (_TCHAR *)parameterFieldName, 
                                       (_TCHAR *)reportName,
                                       (struct PEValueInfo *) currentValue);
}

unsigned short CRPEJob::GetNParameterCurrentRanges(const _TCHAR *parameterFieldName, 
                                            const _TCHAR *reportName)
{

    return PEGetNParameterCurrentRanges(m_jobHandle,
                                       (_TCHAR *)parameterFieldName, 
                                       (_TCHAR *)reportName);
}


BOOL CRPEJob::GetNthParameterCurrentRange (const _TCHAR *parameterFieldName,
                                           const _TCHAR *reportName,
                                           short index,
                                           CRPEValueInfo *rangeStart,    // Set to NULL to set MAX only; must be non-NULL if valueMax is NULL.
                                           CRPEValueInfo *rangeEnd,      // Set to NULL to set MIN only; must be non-NULL if valueMin is NULL.
                                                                         // If both valueInfo and valueMax are non-NULL then
                                                                         // valueMin->valueType MUST BE THE SAME AS valueMax->valueType.
                                                                         // If different, PE_ERR_INCONSISTANTTYPES is returned.
                                           short *rangeInfo)
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEGetNthParameterCurrentRange (m_jobHandle,
                                          (_TCHAR *)parameterFieldName, 
                                          (_TCHAR *)reportName,
                                          index,
                                          (struct PEValueInfo *) rangeStart,
                                          (struct PEValueInfo *) rangeEnd,
                                          (short *) rangeInfo);
}

BOOL CRPEJob::AddParameterCurrentRange (const _TCHAR *parameterFieldName,
                                        const _TCHAR *reportName,
                                        CRPEValueInfo *rangeStart,    // Set to NULL to set MAX only; must be non-NULL if valueMax is NULL.
                                        CRPEValueInfo *rangeEnd,      // Set to NULL to set MIN only; must be non-NULL if valueMin is NULL.
                                                                      // If both valueInfo and valueMax are non-NULL then
                                                                      // valueMin->valueType MUST BE THE SAME AS valueMax->valueType.
                                                                      // If different, PE_ERR_INCONSISTANTTYPES is returned.
                                        short rangeInfo)
{
    // ensure CRPEValueInfo object is size CRPE expects
    ASSERT (sizeof(CRPEValueInfo) == sizeof(PEValueInfo));

    return PEAddParameterCurrentRange (m_jobHandle,
                                       (_TCHAR *)parameterFieldName, 
                                       (_TCHAR *)reportName,
                                       (struct PEValueInfo *) rangeStart,
                                       (struct PEValueInfo *) rangeEnd,
                                       rangeInfo);
}


short CRPEJob::GetNthParameterType (short index) // returns PEP_PO_* or -1 if index is invalid.
{
    return PEGetNthParameterType (m_jobHandle, index);
}

BOOL CRPEJob::ClearParameterCurrentValuesAndRanges (const _TCHAR *parameterFieldName,
                                                    const _TCHAR *reportName)
{
    return PEClearParameterCurrentValuesAndRanges (m_jobHandle,
                                                   (_TCHAR *)parameterFieldName, 
                                                   (_TCHAR *)reportName);
}

// subreport attributes
short CRPEJob::GetNSubreportsInSection(short sectionCode)
{
    return PEGetNSubreportsInSection(m_jobHandle,
                                     sectionCode);
}


DWORD CRPEJob::GetNthSubreportInSection(short sectionCode,
                                        short subreportN)
{
    return PEGetNthSubreportInSection(m_jobHandle,
                                      sectionCode,
                                      subreportN);
}


BOOL CRPEJob::GetSubreportInfo(DWORD subreportHandle,
                               CRPESubreportInfo *subreportInfo)
{
    // ensure CRPESubreportInfo object is size CRPE expects
    ASSERT (sizeof(CRPESubreportInfo) == sizeof(PESubreportInfo));

    return PEGetSubreportInfo(m_jobHandle,
                              subreportHandle,
                              (struct PESubreportInfo *) subreportInfo);
}


CRPEJob *CRPEJob::OpenSubreportJob (const _TCHAR *subreportName)
{
	// engine must be open
    ASSERT (CRPEngine::GetEngineStatus() == CRPEngine::engineOpen);

    short jobHandle = 0;
    CRPEJob *newJob = NULL;

    if ((jobHandle = PEOpenSubreport(m_jobHandle,(_TCHAR *)subreportName)) == 0)
        return NULL;

	if ((newJob = new CNxReportJob(jobHandle,this)) == NULL) {
		PECloseSubreport(jobHandle);
		return NULL;
	}

	return newJob;
}


// misc. attributes
short CRPEJob::GetJobHandle(void)
{
	return m_jobHandle;
}


// implementation


void CRPEJob::AddSubReportJob(CRPEJob *job)
{
	m_subReportJobs.Add(job);
}


void CRPEJob::RemoveSubReportJob(CRPEJob *job)
{
	int numJobs = m_subReportJobs.GetSize();

	while (numJobs--)
		if (m_subReportJobs[numJobs] == job) {
			m_subReportJobs.RemoveAt(numJobs);
			break;
		}
}


#ifdef _DEBUG
void CRPEJob::AssertValid() const
{
	CObject::AssertValid();
}


void CRPEJob::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}

#endif //_DEBUG


////////////////////////
// CRPEMDIChildWnd Class
////////////////////////

IMPLEMENT_DYNCREATE(CRPEMDIChildWnd, CMDIChildWnd)

CRPEMDIChildWnd::CRPEMDIChildWnd() : m_printWnd(0)
{
}

CRPEMDIChildWnd::~CRPEMDIChildWnd()
{
}


BEGIN_MESSAGE_MAP(CRPEMDIChildWnd, CMDIChildWnd)
	//{{AFX_MSG_MAP(CRPEMDIChildWnd)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRPEMDIChildWnd message handlers

void CRPEMDIChildWnd::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
	
    CRect r;
        
    GetClientRect (r);
        
    ::MoveWindow (m_printWnd, 
                  r.left, r.top, 
                  r.Width (), r.Height (),
                  TRUE);
}
