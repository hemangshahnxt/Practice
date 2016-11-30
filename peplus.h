////////////////////////////////////////////////////////////////////////////////
// File:        peplus.h
//
// Authors:     Craig Chaplin
//
// Synopsis:    This file contains the class definitions for CCRPEngine and
//              CCRPEJob as well as any supporting classes and structures.
//
//              Note that the constants defined within this file correspond
//              directly with the #defines found in crpe.h.  Instead of the PE_
//              prefix, the class library uses the PEP_ prefix.
//
//              Note that the structures defined within this file correspond to
//              those present in crpe.h.  Instead of the PE prefix, the class
//              library uses the CRPE prefix.  The class library structures are
//              self initialising and also provide convenient constructors.
//              At this time, the export destination structures
//              (e.g. UXDDiskOptions) and the export format option structures
//              (e.g. UXFCharSepOptions) have NOT been included in the class
//              library.  You will need to include the appropriate header files
//              for these structures.  Search the online help under PEExportTo
//              for more information.
//
// History:     22/06/95 - CC  - Created.
//              27/07/95 - CC  - Added 4.0 Print Engine calls
//              26/10/95 - CC  - Internationalized (only MBCS so far)
//              09/05/96 - CC  - Added 5.0 Print Engine calls
//              10/01/97 - CC  - Corrected CRPESectionOptions structure to match
//                              definition in crpe.h
//              15/08/97 - CC  - Modified CRPEParameterFieldInfo to reflect new
//                               6.0 structure
//              03/09/98 - ABC - Added the 7.0 APIs and all previously missed 
////////////////////////////////////////////////////////////////////////////////


#ifndef PEPLUS_H
#define PEPLUS_H

#pragma once



// structures MUST be byte aligned to match crpe.dll byte alignment

#if _MSC_VER >= 900
    #pragma pack (push)
#endif

#pragma pack(1)

// error codes returned by CRPEngine::GetErrorCode() and CRPEJob::GetErrorCode()
const int PEP_ERR_NOERROR                     = 0;

const int PEP_ERR_NOTENOUGHMEMORY             = 500;
const int PEP_ERR_INVALIDJOBNO                = 501;
const int PEP_ERR_INVALIDHANDLE               = 502;
const int PEP_ERR_STRINGTOOLONG               = 503;
const int PEP_ERR_NOSUCHREPORT                = 504;
const int PEP_ERR_NODESTINATION               = 505;
const int PEP_ERR_BADFILENUMBER               = 506;
const int PEP_ERR_BADFILENAME                 = 507;
const int PEP_ERR_BADFIELDNUMBER              = 508;
const int PEP_ERR_BADFIELDNAME                = 509;
const int PEP_ERR_BADFORMULANAME              = 510;
const int PEP_ERR_BADSORTDIRECTION            = 511;
const int PEP_ERR_ENGINENOTOPEN               = 512;
const int PEP_ERR_INVALIDPRINTER              = 513;
const int PEP_ERR_PRINTFILEEXISTS             = 514;
const int PEP_ERR_BADFORMULATEXT              = 515;
const int PEP_ERR_BADGROUPSECTION             = 516;
const int PEP_ERR_ENGINEBUSY                  = 517;
const int PEP_ERR_BADSECTION                  = 518;
const int PEP_ERR_NOPRINTWINDOW               = 519;
const int PEP_ERR_JOBALREADYSTARTED           = 520;
const int PEP_ERR_BADSUMMARYFIELD             = 521;
const int PEP_ERR_NOTENOUGHSYSRES             = 522;
const int PEP_ERR_BADGROUPCONDITION           = 523;
const int PEP_ERR_JOBBUSY                     = 524;
const int PEP_ERR_BADREPORTFILE               = 525;
const int PEP_ERR_NODEFAULTPRINTER            = 526;
const int PEP_ERR_SQLSERVERERROR              = 527;
const int PEP_ERR_BADLINENUMBER               = 528;
const int PEP_ERR_DISKFULL                    = 529;
const int PEP_ERR_FILEERROR                   = 530;
const int PEP_ERR_INCORRECTPASSWORD           = 531;
const int PEP_ERR_BADDATABASEDLL              = 532;
const int PEP_ERR_BADDATABASEFILE             = 533;
const int PEP_ERR_ERRORINDATABASEDLL          = 534;
const int PEP_ERR_DATABASESESSION             = 535;
const int PEP_ERR_DATABASELOGON               = 536;
const int PEP_ERR_DATABASELOCATION            = 537;
const int PEP_ERR_BADSTRUCTSIZE               = 538;
const int PEP_ERR_BADDATE                     = 539;
const int PEP_ERR_BADEXPORTDLL                = 540;
const int PEP_ERR_ERRORINEXPORTDLL            = 541;
const int PEP_ERR_PREVATFIRSTPAGE             = 542;
const int PEP_ERR_NEXTATLASTPAGE              = 543;
const int PEP_ERR_CANNOTACCESSREPORT          = 544;
const int PEP_ERR_USERCANCELLED               = 545;
const int PEP_ERR_OLE2NOTLOADED               = 546;
const int PEP_ERR_BADCROSSTABGROUP            = 547;
const int PEP_ERR_NOCTSUMMARIZEDFIELD         = 548;
const int PEP_ERR_DESTINATIONNOTEXPORT        = 549;
const int PEP_ERR_INVALIDPAGENUMBER           = 550;
const int PEP_ERR_NOTSTOREDPROCEDURE          = 552;
const int PEP_ERR_INVALIDPARAMETER            = 553;
const int PEP_ERR_GRAPHNOTFOUND               = 554;
const int PEP_ERR_INVALIDGRAPHTYPE            = 555;
const int PEP_ERR_INVALIDGRAPHDATA            = 556;
const int PEP_ERR_CANNOTMOVEGRAPH             = 557;
const int PEP_ERR_INVALIDGRAPHTEXT            = 558;
const int PEP_ERR_INVALIDGRAPHOPT             = 559;
const int PEP_ERR_BADSECTIONHEIGHT            = 560;
const int PEP_ERR_BADVALUETYPE                = 561;
const int PEP_ERR_INVALIDSUBREPORTNAME        = 562;
const int PEP_ERR_NOPARENTWINDOW              = 564; // dialog parent window
const int PEP_ERR_INVALIDZOOMFACTOR           = 565; // zoom factor
const int PEP_ERR_PAGESIZEOVERFLOW            = 567;
const int PEP_ERR_LOWSYSTEMRESOURCES          = 568;
const int PEP_ERR_BADGROUPNUMBER              = 570;
const int PEP_ERR_INVALIDOBJECTFORMATNAME     = 571;
const int PEP_ERR_INVALIDNEGATIVEVALUE        = 572;
const int PEP_ERR_INVALIDMEMORYPOINTER        = 573;
const int PEP_ERR_INVALIDOBJECTTYPE           = 574;
const int PEP_ERR_INVALIDGRAPHDATATYPE        = 577;
const int PEP_ERR_INVALIDSUBREPORTLINKNUMBER  = 582;
const int PEP_ERR_SUBREPORTLINKEXIST          = 583;
const int PEP_ERR_BADROWCOLVALUE              = 584;
const int PEP_ERR_INVALIDSUMMARYNUMBER        = 585;
const int PEP_ERR_INVALIDGRAPHDATAFIELDNUMBER = 586;
const int PEP_ERR_INVALIDSUBREPORTNUMBER      = 587;
const int PEP_ERR_INVALIDFIELDSCOPE           = 588;
const int PEP_ERR_FIELDINUSE                  = 590;
const int PEP_ERR_INVALIDPARAMETERNUMBER      = 594;
const int PEP_ERR_INVALIDPAGEMARGINS          = 595;
const int PEP_ERR_REPORTONSECUREQUERY         = 596;
const int PEP_ERR_CANNOTOPENSECUREQUERY       = 597;
const int PEP_ERR_INVALIDSECTIONNUMBER        = 598;
const int PEP_ERR_SQLSERVERNOTOPENED          = 599;
const int PEP_ERR_TABLENAMEEXIST              = 606;
const int PEP_ERR_INVALIDCURSOR               = 607;
const int PEP_ERR_FIRSTPASSNOTFINISHED        = 608;
const int PEP_ERR_CREATEDATASOURCE            = 609;
const int PEP_ERR_CREATEDRILLDOWNPARAMETERS   = 610;
const int PEP_ERR_CHECKFORDATASOURCECHANGES   = 613;
const int PEP_ERR_STARTBACKGROUNDPROCESSING   = 614;
const int PEP_ERR_SQLSERVERINUSE              = 619;
const int PEP_ERR_GROUPSORTFIELDNOTSET        = 620;
const int PEP_ERR_CANNOTSETSAVESUMMARIES      = 621;
const int PEP_ERR_LOADOLAPDATABASEMANAGER     = 622;
const int PEP_ERR_OPENOLAPCUBE                = 623;
const int PEP_ERR_READOLAPCUBEDATA            = 624;
const int PEP_ERR_CANNOTSAVEQUERY             = 626;
const int PEP_ERR_CANNOTREADQUERYDATA         = 627;
const int PEP_ERR_MAINREPORTFIELDLINKED       = 629;
const int PEP_ERR_INVALIDMAPPINGTYPEVALUE     = 630;
const int PEP_ERR_HITTESTFAILED               = 636;
const int PEP_ERR_BADSQLEXPRESSIONNAME        = 637; // no SQL expression by the specified *name* exists in this report.
const int PEP_ERR_BADSQLEXPRESSIONNUMBER      = 638; // no SQL expression by the specified *number* exists in this report.
const int PEP_ERR_BADSQLEXPRESSIONTEXT        = 639; // not a valid SQL expression
const int PEP_ERR_INVALIDDEFAULTVALUEINDEX    = 641; // invalid index for default value of a parameter.
const int PEP_ERR_NOMINMAXVALUE               = 642; // the specified PE_PF_* type does not have min/max values.
const int PEP_ERR_INCONSISTANTTYPES           = 643; // if both min and max values are specified in PESetParameterMinMaxValue,
                                                    // the value types for the min and max must be the same.

const int PEP_ERR_CANNOTLINKTABLES            = 645;
const int PEP_ERR_CREATEROUTER                = 646;

const int PEP_ERR_INVALIDFIELDINDEX           = 647;
const int PEP_ERR_INVALIDGRAPHTITLETYPE       = 648;
const int PEP_ERR_INVALIDGRAPHTITLEFONTTYPE   = 649;

const int PEP_ERR_OTHERERROR                  = 997;
const int PEP_ERR_INTERNALERROR               = 998; // programming error
const int PEP_ERR_NOTIMPLEMENTED              = 999;


// values to be used for CRPEngine::GetVersion() versionRequested parameter

const int PEP_GV_DLL    = 100;
const int PEP_GV_ENGINE = 200;

// values returned by CRPEJob::GetJobStatus()

const int PEP_JOBNOTSTARTED = 1;
const int PEP_JOBINPROGRESS = 2;
const int PEP_JOBCOMPLETED  = 3;
const int PEP_JOBFAILED     = 4; // an error occurred
const int PEP_JOBCANCELLED  = 5; // cancelled by user
const int PEP_JOBHALTED     = 6; // too many records or too much time

// values used to specify the sort direction for CRPEJob group condition
// attributes, record sort order field attributes, and group sort order
// field attributes

const int PEP_SF_DESCENDING = 0;
const int PEP_SF_ASCENDING  = 1;

// group section codes used by CRPEJob group condition attributes, report
// section attributes, and graphing attributes

// macro to create section codes
// - allows up to 25 groups and 40 sections of a given type, although
//   Crystal Reports itself has no such limitations
#define PEP_SECTION_CODE(sectionType,groupN,sectionN) \
    (((sectionType) * 1000) + ((groupN) % 25) + (((sectionN) % 40) * 25))

const int PEP_ALLSECTIONS        = 0;
const int PEP_SECT_REPORT_HEADER = 1;
const int PEP_SECT_PAGE_HEADER   = 2;
const int PEP_SECT_GROUP_HEADER  = 3;
const int PEP_SECT_DETAIL        = 4;
const int PEP_SECT_GROUP_FOOTER  = 5;
const int PEP_SECT_PAGE_FOOTER   = 7;
const int PEP_SECT_REPORT_FOOTER = 8;

// macros to decode section codes
#define PEP_SECTION_TYPE(sectionCode) ((sectionCode) / 1000)
#define PEP_GROUP_N(sectionCode)      ((sectionCode) % 25)
#define PEP_SECTION_N(sectionCode)    (((sectionCode) / 25) % 40)

// old section constants redefined in terms of the new 5.x codes above
const int PEP_TITLESECTION      = PEP_SECTION_CODE (PEP_SECT_REPORT_HEADER, 0, 0);
const int PEP_HEADERSECTION     = PEP_SECTION_CODE (PEP_SECT_PAGE_HEADER,   0, 0);
const int PEP_GROUPHEADER       = PEP_SECTION_CODE (PEP_SECT_GROUP_HEADER,  0, 0);
const int PEP_DETAILSECTION     = PEP_SECTION_CODE (PEP_SECT_DETAIL,        0, 0);
const int PEP_GROUPFOOTER       = PEP_SECTION_CODE (PEP_SECT_GROUP_FOOTER,  0, 0);
const int PEP_SUMMARYSECTION    = PEP_SECTION_CODE (PEP_SECT_REPORT_FOOTER, 0, 0);
const int PEP_GRANDTOTALSECTION = PEP_SUMMARYSECTION;
const int PEP_FOOTERSECTION     = PEP_SECTION_CODE (PEP_SECT_PAGE_FOOTER,   0, 0);

// group condition codes used by CRPEJob group condition attributes

const int PEP_GC_ANYCHANGE     = 0; // use for all field types except Date and Boolean

const int PEP_GC_DAILY         = 0; // use these constants for Date fields
const int PEP_GC_WEEKLY        = 1;
const int PEP_GC_BIWEEKLY      = 2;
const int PEP_GC_SEMIMONTHLY   = 3;
const int PEP_GC_MONTHLY       = 4;
const int PEP_GC_QUARTERLY     = 5;
const int PEP_GC_SEMIANNUALLY  = 6;
const int PEP_GC_ANNUALLY      = 7;

const int PEP_GC_TOYES         = 1; // use these constants for Boolean fields
const int PEP_GC_TONO          = 2;
const int PEP_GC_EVERYYES      = 3;
const int PEP_GC_EVERYNO       = 4;
const int PEP_GC_NEXTISYES     = 5;
const int PEP_GC_NEXTISNO      = 6;

const int PEP_GC_CONDITIONMASK = 0x00ff; // masks to get the condition and type
const int PEP_GC_TYPEMASK      = 0x0f00; // after calling CRPEJob::GetGroupCondition()
                                         
const int PEP_GC_TYPEOTHER     = 0x0000; // condition parameter
const int PEP_GC_TYPEDATE      = 0x0200;
const int PEP_GC_TYPEBOOLEAN   = 0x0400;

// values used by CRPEJob::ZoomPreviewWindow()
const int PEP_ZOOM_FULL_SIZE           = 0;
const int PEP_ZOOM_SIZE_FIT_ONE_SIDE   = 1;
const int PEP_ZOOM_SIZE_FIT_BOTH_SIDES = 2;

// values used in CRPEJob::SetFont()
const int PEP_FIELDS    = 0x0001;
const int PEP_TEXT      = 0x0002;
const int PEP_UNCHANGED = -1;
#define   PEP_UNCHANGED_COLOR (COLORREF) -2

// values used by CRPELogOnInfo structure
const int PEP_SERVERNAME_LEN   = 128;
const int PEP_DATABASENAME_LEN = 128;
const int PEP_USERID_LEN       = 128;
const int PEP_PASSWORD_LEN     = 128;

// value used in CRPEJob::SetMargins and CRPEJob::GetMargins
const int PEP_SM_DEFAULT = 0x800;

struct CRPEReportOptions 
{
    CRPEReportOptions () : m_StructSize(sizeof(CRPEReportOptions))
    {
        short m_saveDataWithReport = PEP_UNCHANGED;
        short m_saveSummariesWithReport = PEP_UNCHANGED;
        short m_useIndexForSpeed = PEP_UNCHANGED;
        short m_translateDOSStrings = PEP_UNCHANGED;
        short m_translateDOSMemos = PEP_UNCHANGED;
        short m_convertDateTimeType = PEP_UNCHANGED;
        short m_convertNullFieldToDefault = PEP_UNCHANGED;
        short m_morePrintEngineErrorMessages = PEP_UNCHANGED;
        short m_caseInsensitiveSQLData = PEP_UNCHANGED;
        short m_verifyOnEveryPrint = PEP_UNCHANGED;
        short m_zoomMode = PEP_UNCHANGED;
        short m_hasGroupTree = PEP_UNCHANGED;
        short m_dontGenerateDataForHiddenObjects = PEP_UNCHANGED;
        short m_performGroupingOnServer = PEP_UNCHANGED;
    }

    CRPEReportOptions (short saveDataWithReport,
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
                       short performGroupingOnServer);

    WORD  m_StructSize;
    short m_saveDataWithReport;
    short m_saveSummariesWithReport;
    short m_useIndexForSpeed;
    short m_translateDOSStrings;
    short m_translateDOSMemos;
    short m_convertDateTimeType;
    short m_convertNullFieldToDefault;
    short m_morePrintEngineErrorMessages;
    short m_caseInsensitiveSQLData;
    short m_verifyOnEveryPrint;
    short m_zoomMode;
    short m_hasGroupTree;
    short m_dontGenerateDataForHiddenObjects;
    short m_performGroupingOnServer;
};

const int PEP_SI_APPLICATION_NAME_LEN = 128;
const int PEP_SI_TITLE_LEN            = 128;
const int PEP_SI_SUBJECT_LEN          = 128;
const int PEP_SI_AUTHOR_LEN           = 128;
const int PEP_SI_KEYWORDS_LEN         = 128;
const int PEP_SI_COMMENTS_LEN         = 512;
const int PEP_SI_REPORT_TEMPLATE_LEN  = 128;

struct CRPEReportSummaryInfo
{
    CRPEReportSummaryInfo () : m_StructSize(sizeof(CRPEReportSummaryInfo))
    {
        m_applicationName[0] = '\0';
        m_title[0] = '\0';
        m_subject[0] = '\0';
        m_author[0] = '\0';
        m_keywords[0] = '\0';
        m_comments[0] = '\0';
        m_reportTemplate[0] = '\0';
    }

    CRPEReportSummaryInfo (_TCHAR *applicationName,
                           _TCHAR *title,
                           _TCHAR *subject,
                           _TCHAR *author,
                           _TCHAR *keywords,
                           _TCHAR *comments,
                           _TCHAR *reportTemplate
                          ) : m_StructSize(sizeof(CRPEReportSummaryInfo))
    {
        lstrcpyn(m_applicationName, applicationName, sizeof(m_applicationName) - 1);
        lstrcpyn(m_title, title, sizeof(m_title) - 1);
        lstrcpyn(m_subject, subject, sizeof(m_subject) - 1);
        lstrcpyn(m_author, author,sizeof(m_author) - 1);
        lstrcpyn(m_keywords, keywords, sizeof(m_keywords) - 1);
        lstrcpyn(m_comments, comments, sizeof(m_comments) - 1);
        lstrcpyn(m_reportTemplate, reportTemplate,sizeof(m_reportTemplate) - 1);
    }

    WORD   m_StructSize;
    _TCHAR m_applicationName[PEP_SI_APPLICATION_NAME_LEN]; // read only.
    _TCHAR m_title[PEP_SI_TITLE_LEN];
    _TCHAR m_subject[PEP_SI_SUBJECT_LEN];
    _TCHAR m_author[PEP_SI_AUTHOR_LEN];
    _TCHAR m_keywords[PEP_SI_KEYWORDS_LEN];
    _TCHAR m_comments[PEP_SI_COMMENTS_LEN];
    _TCHAR m_reportTemplate[PEP_SI_REPORT_TEMPLATE_LEN];
};

struct CRPELogOnInfo
{
    CRPELogOnInfo () : m_StructSize(sizeof(CRPELogOnInfo))
    {
        m_serverName[0] = m_databaseName[0] = m_userID[0] = m_password[0] = '\0';
    }

    CRPELogOnInfo (const _TCHAR *serverName,
                   const _TCHAR *databaseName,
                   const _TCHAR *userID,
                   const _TCHAR *password);

    // Initialize to sizeof(CRPELogOnInfo).
    WORD m_StructSize;

    // For any of the following values an empty string ("") means to use
    // the value already set in the report.  To override a value in the
    // report use a non-empty string (e.g. "Server A").  All strings are
    // null-terminated.
    //
    // For Netware SQL, pass the dictionary path name in ServerName and
    // data path name in DatabaseName.
    _TCHAR m_serverName [PEP_SERVERNAME_LEN];
    _TCHAR m_databaseName [PEP_DATABASENAME_LEN];
    _TCHAR m_userID [PEP_USERID_LEN];

    // Password is undefined when getting information from report.
    _TCHAR m_password [PEP_PASSWORD_LEN];
};


struct CRPEJobInfo
{
    CRPEJobInfo () : m_StructSize(sizeof(CRPEJobInfo)),
                     m_numRecordsRead(0),
                     m_numRecordsSelected(0),
                     m_numRecordsPrinted(0),
                     m_displayPageN(0),
                     m_latestPageN(0),
                     m_startPageN(0),
                     m_printEnded(FALSE)   // full report print completed?
    {}
    
    WORD m_StructSize;    // initialize to sizeof (CRPEJobInfo)

    DWORD m_numRecordsRead;
    DWORD m_numRecordsSelected;
    DWORD m_numRecordsPrinted;
    WORD  m_displayPageN;       // the page being displayed in window
    WORD  m_latestPageN;        // the page being generated
    WORD  m_startPageN;         // user opted, default to 1
    BOOL  m_printEnded;         // full report print completed?
};

const int PEP_FIELD_NAME_LEN = 512;

const int PEP_GO_TBN_ALL_GROUPS_UNSORTED = 0;
const int PEP_GO_TBN_ALL_GROUPS_SORTED   = 1;
const int PEP_GO_TBN_TOP_N_GROUPS        = 2;
const int PEP_GO_TBN_BOTTOM_N_GROUPS     = 3;

struct CRPEGroupOptions
{ 
    CRPEGroupOptions () : m_StructSize(sizeof(CRPEGroupOptions)),
                          m_condition(0),
                          m_sortDirection(0),
                          m_repeatGroupHeader(0),
                          m_keepGroupTogether(0),
                          m_topOrBottomNGroups(0),
                          m_nTopOrBottomGroups(0),
                          m_discardOtherGroups(0)
    {
        m_fieldName[0] = '\0';
        m_topOrBottomNSortFieldName[0] = '\0';
    }    

    CRPEGroupOptions (short condition,
                      _TCHAR *fieldName,
                      short  sortDirection,
                      short  repeatGroupHeader,
                      short  keepGroupTogether,
                      short  topOrBottomNGroups,
                      _TCHAR *topOrBottomNSortFieldName,
                      short  nTopOrBottomGroups,
                      short  discardOtherGroups
                     ) : m_StructSize(sizeof(CRPEGroupOptions)),
                         m_condition(condition),
                         m_sortDirection(sortDirection),
                         m_repeatGroupHeader(repeatGroupHeader),
                         m_keepGroupTogether(keepGroupTogether),
                         m_topOrBottomNGroups(topOrBottomNGroups),
                         m_nTopOrBottomGroups(nTopOrBottomGroups),
                         m_discardOtherGroups(discardOtherGroups)
    {
        lstrcpyn(m_fieldName, fieldName, sizeof(m_fieldName) - 1);
        lstrcpyn(m_topOrBottomNSortFieldName, topOrBottomNSortFieldName, sizeof(m_topOrBottomNSortFieldName) - 1);
    }    

    WORD m_StructSize;
    // when setting, pass a PEP_GC_ constant, or PEP_UNCHANGED for no change.
    // when getting, use PEP_GC_TYPEMASK and PEP_GC_CONDITIONMASK to
    // decode the condition.
    short  m_condition;
    _TCHAR m_fieldName [PEP_FIELD_NAME_LEN]; // formula form, or empty for no change.
    short  m_sortDirection;                // a PE_SF_ const, or PEP_UNCHANGED for no change.
    short  m_repeatGroupHeader;            // BOOL value, or PEP_UNCHANGED for no change.
    short  m_keepGroupTogether;            // BOOL value, or PEP_UNCHANGED for no change.
    short  m_topOrBottomNGroups;           // a PEP_GO_TBN_ constant, or PE_UNCHANGED for no change.
    _TCHAR m_topOrBottomNSortFieldName [PEP_FIELD_NAME_LEN]; // formula form, or empty for no change.
    short  m_nTopOrBottomGroups;           // the number of groups to keep, 0 for all, or PEP_UNCHANGED for no change.
    short  m_discardOtherGroups;           // BOOL value, or PEP_UNCHANGED for no change.
};



// values used in CRPETableType structure
const int  PEP_DLL_NAME_LEN     = 64;
const int  PEP_FULL_NAME_LEN    = 256;
const WORD PEP_DT_STANDARD      = 1;
const WORD PEP_DT_SQL           = 2;

struct CRPETableType
{
    CRPETableType () : m_StructSize(sizeof(CRPETableType)),
                       m_dbType(PEP_DT_SQL)
    {
        m_dllName[0] = m_descriptiveName[0] = '\0';
    }
    
    // Initialize to sizeof(CRPETableType)
    WORD m_StructSize;

    // All strings are null-terminated.
    _TCHAR m_dllName [PEP_DLL_NAME_LEN];
    _TCHAR m_descriptiveName [PEP_FULL_NAME_LEN];

    WORD m_dbType;
};


// values used in CRPESessionInfo structure
const int PEP_SESS_USERID_LEN   = 128;
const int PEP_SESS_PASSWORD_LEN = 128;

struct CRPESessionInfo
{
    CRPESessionInfo () : m_StructSize(sizeof(CRPESessionInfo)),
                         m_sessionHandle(0)
    {
        m_userID[0] = m_password[0] = '\0';
    }

    CRPESessionInfo (const _TCHAR *userID,
                     const _TCHAR *password,
                     DWORD sessionHandle);

    // Initialize to sizeof(CRPESessionInfo)
    WORD m_StructSize;

    // All strings are null-terminated.
    _TCHAR m_userID [PEP_SESS_USERID_LEN];

    // Password is undefined when getting information from report.
    _TCHAR m_password [PEP_SESS_PASSWORD_LEN];

    // SessionHandle is undefined when getting information from report.
    // When setting information, if it is = 0 the UserID and Password
    // settings are used, otherwise the SessionHandle is used.
    DWORD m_sessionHandle;
};


// values used in CRPETableLocation structure
const int PEP_TABLE_LOCATION_LEN      = 256;

struct CRPETableLocation
{
    CRPETableLocation () : m_StructSize(sizeof(CRPETableLocation))
    {
        m_location[0] = '\0';
    }

    CRPETableLocation (const _TCHAR *location);

    // Initialize to sizeof(CRPETableLocation)
    WORD m_StructSize;

    // String is null-terminated.
    _TCHAR m_location [PEP_TABLE_LOCATION_LEN];
};


// values used in CRPEPrintOptions structures
const int PEP_MAXPAGEN            = 65535;

const short PEP_FILE_PATH_LEN     = 512;

const short PEP_UNCOLLATED       = 0;
const short PEP_COLLATED         = 1;
const short PEP_DEFAULTCOLLATION = 2;

struct CRPEPrintOptions
{
    CRPEPrintOptions () : m_StructSize(sizeof(CRPEPrintOptions)),
                          m_startPageN(0),
                          m_stopPageN(0),
                          m_nReportCopies(0),
                          m_collation(PEP_DEFAULTCOLLATION)                          

    {
        m_outputFileName[0] = '\0';
    }

    CRPEPrintOptions (unsigned short startPageN,
                      unsigned short stopPageN,
                      unsigned short nReportCopies,
                      unsigned short collation,
                      _TCHAR *outputFileName
                     ) :
                      m_StructSize(sizeof(CRPEPrintOptions)),
                      m_startPageN(startPageN),
                      m_stopPageN(stopPageN),
                      m_nReportCopies(nReportCopies),
                      m_collation(collation)                 
    {
        lstrcpyn(m_outputFileName, outputFileName, sizeof(m_outputFileName) - 1);                  
    }

    WORD m_StructSize;            // initialize to sizeof (PEPrintOptions)

    // page and copy numbers are 1-origin
    // use 0 to preserve the existing settings
    unsigned short m_startPageN,
                   m_stopPageN;

    unsigned short m_nReportCopies;
    unsigned short m_collation;

    _TCHAR m_outputFileName[PEP_FILE_PATH_LEN];     
};

    
struct CRPEExportOptions
{
    CRPEExportOptions () : m_StructSize(sizeof(CRPEExportOptions)),
                           m_formatType(0),
                           m_formatOptions(NULL),
                           m_destinationType(0),
                           m_destinationOptions(NULL),
                           m_nFormatOptionsBytes(0),
                           m_nDestinationOptionsBytes(0)
    {
        m_formatDLLName[0] = m_destinationDLLName[0] = '\0';
    }

    CRPEExportOptions (const _TCHAR *formatDLLName,
                       DWORD formatType,
                       void *formatOptions,
                       const _TCHAR *destinationDLLName,
                       DWORD destinationType,
                       void *destinationOptions);

    WORD m_StructSize;               // initialize to sizeof(CRPEExportOptions)

    _TCHAR m_formatDLLName [PEP_DLL_NAME_LEN];
    DWORD m_formatType;
    void *m_formatOptions;
    _TCHAR m_destinationDLLName [PEP_DLL_NAME_LEN];
    DWORD m_destinationType;
    void *m_destinationOptions;
    WORD m_nFormatOptionsBytes;      // Set by 'GetExportOptions',
                                     // ignored by 'ExportTo'.
    WORD m_nDestinationOptionsBytes; // Set by 'GetExportOptions',
                                     // ignored by 'ExportTo'.
};

// New naming convention for Area/Section format formulas
const int PEP_FFN_AREASECTION_VISIBILITY  = 58;  // area & section format
const int PEP_FFN_SECTION_VISIBILITY      = 58;  // section format
const int PEP_FFN_SHOW_AREA               = 59;  // area format
const int PEP_FFN_NEW_PAGE_BEFORE         = 60;  // area & section format
const int PEP_FFN_NEW_PAGE_AFTER          = 61;  // area & section format
const int PEP_FFN_KEEP_TOGETHER           = 62;  // area & section format
const int PEP_FFN_SUPPRESS_BLANK_SECTION  = 63;  // section format
const int PEP_FFN_RESET_PAGE_N_AFTER      = 64;  // area & section format
const int PEP_FFN_PRINT_AT_BOTTOM_OF_PAGE = 65;  // area & section format
const int PEP_FFN_UNDERLAY_SECTION        = 66;  // section format
const int PEP_FFN_SECTION_BACK_COLOUR     = 67;  // section format
const int PEP_FFN_SECTION_BACK_COLOR      = 67;  // section format


struct CRPESectionOptions
{
    CRPESectionOptions () : m_StructSize(sizeof(CRPESectionOptions)),
                            m_visible(0),
                            m_newPageBefore(0),
                            m_newPageAfter(0),
                            m_keepTogether(0),
                            m_suppressBlankLines(0),
                            m_resetPageNAfter(0),
                            m_printAtBottomOfPage(0),
                            m_backgroundColour(PEP_UNCHANGED_COLOR),
                            m_underlaySection(0),
                            m_showArea(0),
                            m_freeFormPlacement(0)
    {}

    CRPESectionOptions (short visible,
                        short newPageBefore,
                        short newPageAfter,
                        short keepTogether,
                        short suppressBlankLines,
                        short resetPageNAfter,
                        short printAtBottomOfPage,
                        COLORREF backgroundColour,
                        short underlaySection,
                        short showArea,
                        short freeFormPlacement
                       ) :
                        m_StructSize(sizeof(CRPESectionOptions)),
                        m_visible(visible),
                        m_newPageBefore(newPageBefore),
                        m_newPageAfter(newPageAfter),
                        m_keepTogether(keepTogether),
                        m_suppressBlankLines(suppressBlankLines),
                        m_resetPageNAfter(resetPageNAfter),
                        m_printAtBottomOfPage(printAtBottomOfPage),
                        m_backgroundColour(backgroundColour),
                        m_underlaySection(underlaySection),
                        m_showArea(showArea),
                        m_freeFormPlacement(freeFormPlacement)
    {}

    WORD m_StructSize; // initialize to sizeof (CRPESectionOptions)

    short m_visible;              // BOOLEAN values, except use PEP_UNCHANGED
    short m_newPageBefore;        // to preserve the existing settings
    short m_newPageAfter;
    short m_keepTogether;
    short m_suppressBlankLines;
    short m_resetPageNAfter;
    short m_printAtBottomOfPage;
    COLORREF m_backgroundColour;
    short m_underlaySection;
    short m_showArea;
    short m_freeFormPlacement;
};




// Setting Graph/Chart info
// ------------------------
//
// Two parameters are passed to uniquely identify the graph:
//      - section code
//      - graph number in that section
//
// The section code includes whether it is a header or footer, and the
// graph number starts at 0, 1...  The graph number identifies the graph
// by its position in the section
//      - looking top down first,
//      - then left to right if they have the same top.

// Graph Types

const int PEP_SIDE_BY_SIDE_BAR_GRAPH          = 0;
const int PEP_STACKED_BAR_GRAPH               = 2;
const int PEP_PERCENT_BAR_GRAPH               = 3;
const int PEP_FAKED_3D_SIDE_BY_SIDE_BAR_GRAPH = 4;
const int PEP_FAKED_3D_STACKED_BAR_GRAPH      = 5;
const int PEP_FAKED_3D_PERCENT_BAR_GRAPH      = 6;
const int PEP_PIE_GRAPH                       = 40;
const int PEP_MULTIPLE_PIE_GRAPH              = 42;
const int PEP_PROPORTIONAL_MULTI_PIE_GRAPH    = 43;
const int PEP_LINE_GRAPH                      = 80;
const int PEP_AREA_GRAPH                      = 120;
const int PEP_THREED_BAR_GRAPH                = 160;
const int PEP_USER_DEFINED_GRAPH              = 500;
const int PEP_UNKNOWN_TYPE_GRAPH              = 1000;

// Graph Directions.
const int PEP_GRAPH_ROWS_ONLY         = 0;
const int PEP_GRAPH_COLS_ONLY         = 1;
const int PEP_GRAPH_MIXED_ROW_COL     = 2;
const int PEP_GRAPH_MIXED_COL_ROW     = 3;
const int PEP_GRAPH_UNKNOWN_DIRECTION = 20;

// Graph constant for rowGroupN, colGroupN, summarizedFieldN in PEGraphDataInfo
const int PEP_GRAPH_DATA_NULL_SELECTION = -1;

// Graph text max length
const int PEP_GRAPH_TEXT_LEN = 128;


struct CRPEGraphDataInfo
{
    CRPEGraphDataInfo () : m_StructSize(sizeof(CRPEGraphDataInfo)),
                           m_rowGroupN(0),
                           m_colGroupN(0),
                           m_summarizedFieldN(0),
                           m_graphDirection(PEP_GRAPH_COLS_ONLY)
    {}
    
    CRPEGraphDataInfo (short rowGroupN,
                       short colGroupN,
                       short summarizedFieldN,
                       short graphDirection
                      ) :
                       m_StructSize(sizeof(CRPEGraphDataInfo)),
                       m_rowGroupN(rowGroupN),
                       m_colGroupN(colGroupN),
                       m_summarizedFieldN(summarizedFieldN),
                       m_graphDirection(graphDirection)
    {}
                           
    WORD m_StructSize;        // initialize to sizeof (CRPEGraphDataInfo)

    short m_rowGroupN;        // group number in report
    short m_colGroupN;        // group number in report
    short m_summarizedFieldN; // summarized field number for the group
                              // where the graph stays.
    short m_graphDirection;   // For normal group/total report, the direction,
                              // is always GRAPH_COLS_ONLY.  For CrossTab
                              // report all four options will change the
                              // graph data.
};


struct CRPEGraphTextInfo
{
    CRPEGraphTextInfo () : m_StructSize(sizeof(CRPEGraphTextInfo))
    {
        m_graphTitle[0] = m_graphSubTitle[0]
                        = m_graphFootNote[0]
                        = m_graphGroupsTitle[0]
                        = m_graphSeriesTitle[0]
                        = m_graphXAxisTitle[0]
                        = m_graphYAxisTitle[0]
                        = m_graphZAxisTitle[0]
                        = '\0';
    }
        
    CRPEGraphTextInfo (const _TCHAR *graphTitle,
                       const _TCHAR *graphSubTitle,
                       const _TCHAR *graphFootNote,
                       const _TCHAR *graphGroupsTitle,
                       const _TCHAR *graphSeriesTitle,
                       const _TCHAR *graphXAxisTitle,
                       const _TCHAR *graphYAxisTitle,
                       const _TCHAR *graphZAxisTitle);
        
    WORD m_StructSize; // initialize to sizeof (CRPEGraphTextInfo)

    _TCHAR m_graphTitle       [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphSubTitle    [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphFootNote    [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphGroupsTitle [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphSeriesTitle [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphXAxisTitle  [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphYAxisTitle  [PEP_GRAPH_TEXT_LEN];
    _TCHAR m_graphZAxisTitle  [PEP_GRAPH_TEXT_LEN];
};


struct CRPEGraphOptions
{
    CRPEGraphOptions () : m_StructSize(sizeof(CRPEGraphOptions)),
                          m_graphMaxValue(0),
                          m_graphMinValue(0),
                          m_showDataValue(FALSE),
                          m_showGridLine(FALSE),
                          m_verticalBars(FALSE),
                          m_showLegend(FALSE)
    {
        m_fontFaceName[0] = '\0';
    }
    
    CRPEGraphOptions (double graphMaxValue,
                      double graphMinValue,
                      BOOL showDataValue,
                      BOOL showGridLine,
                      BOOL verticalBars,
                      BOOL showLegend,
                      const _TCHAR *fontFaceName);
                      
    WORD m_StructSize;      // initialize to sizeof (PEGraphOptions)

    double m_graphMaxValue;
    double m_graphMinValue;

    BOOL m_showDataValue;   // Show data values on risers.
    BOOL m_showGridLine;
    BOOL m_verticalBars;
    BOOL m_showLegend;

    _TCHAR m_fontFaceName[PEP_GRAPH_TEXT_LEN];
};


// controlling parameter fields
const int PEP_WORD_LEN           = 2;
const int PEP_PF_NAME_LEN        = 256;
const int PEP_PF_PROMPT_LEN      = 256;
const int PEP_PF_VALUE_LEN       = 256;
const int PEP_PF_EDITMASK_LEN = 256;
const int PEP_PF_NUMBER          = 0;
const int PEP_PF_CURRENCY        = 1;
const int PEP_PF_BOOLEAN         = 2;
const int PEP_PF_DATE            = 3;
const int PEP_PF_STRING          = 4;
const int PEP_PF_DATETIME        = 5;
const int PEP_PF_TIME            = 6;
const int PEP_PF_REPORT_NAME_LEN = 128;
 
struct CRPEParameterFieldInfo
{
    CRPEParameterFieldInfo () : m_StructSize(sizeof(CRPEParameterFieldInfo)),
                                m_ValueType(0),
                                m_DefaultValueSet(0),
                                m_CurrentValueSet(0),
                                m_needsCurrentValue(0),
                                m_isLimited(0),
                                m_MinSize(0),
                                m_MaxSize(0),
                                m_isHidden(0)
    {
        m_Name[0] = m_Prompt[0]
                  = m_DefaultValue[0]
                  = m_CurrentValue[0]
                  = m_ReportName[0]
                  = m_EditMask[0]
                  = '\0';
    }

    CRPEParameterFieldInfo (WORD ValueType,
                            WORD DefaultValueSet,
                            WORD CurrentValueSet,
                            const _TCHAR *Name,
                            const _TCHAR *Prompt,
                            const _TCHAR *DefaultValue,
                            const _TCHAR *CurrentValue,
                            WORD isLimited,
                            double MinSize,
                            double MaxSize,
                            const _TCHAR *EditMask,
                            WORD isHidden );

    // Initialize to size of CRPEParameterFieldInfo structure
    WORD m_StructSize;

    // PEP_PF_ constant
    WORD m_ValueType;

    // default value for variable
    WORD m_DefaultValueSet;

    // current value for variable
    WORD m_CurrentValueSet;

    // All strings are null-terminated.
    _TCHAR m_Name[PEP_PF_NAME_LEN];
    _TCHAR m_Prompt[PEP_PF_PROMPT_LEN];

    // Could be Number, Date, DateTime, Time, Boolean, or String
    _TCHAR m_DefaultValue[PEP_PF_VALUE_LEN];
    _TCHAR m_CurrentValue[PEP_PF_VALUE_LEN];

    // name of report where the field belongs, only used in
    // GetNthParameterField and NewParameterField
    _TCHAR m_ReportName[PEP_PF_REPORT_NAME_LEN];

    // returns false if parameter is linked, not in use, or has current value set
    WORD m_needsCurrentValue;

    //for String values this will be TRUE if the string is limited on length, for 
	//other types it will be TRUE if the parameter is limited by a range
    WORD m_isLimited;

    //For string fields, these are the minimum/maximum length of the string.
    //For numeric fields, they are the minimum/maximum numeric value.
    double m_MinSize;
    double m_MaxSize;

    //An edit mask that restricts what may be entered for string parameters.
    _TCHAR m_EditMask [PEP_PF_EDITMASK_LEN];

    //  return true if it is essbase sub var
    WORD m_isHidden;
};

// Parameter current values
///////////////////////////
// parameter field origin
const int PEP_PO_REPORT     = 0;
const int PEP_PO_STOREDPROC = 1;
const int PEP_PO_QUERY      = 2;

// range info
const int PEP_RI_INCLUDEUPPERBOUND = 1;
const int PEP_RI_INCLUDELOWERBOUND = 2;
const int PEP_RI_NOUPPERBOUND      = 4;
const int PEP_RI_NOLOWERBOUND      = 8;

struct CRPEParameterValueInfo 
{
    CRPEParameterValueInfo () : m_StructSize(sizeof(CRPEParameterValueInfo)),
                                m_isNullable(PEP_UNCHANGED),
                                m_disallowEditing(PEP_UNCHANGED),
                                m_allowMultipleValues(PEP_UNCHANGED),
                                m_hasDiscreteValues(PEP_UNCHANGED),
                                m_partOfGroup(PEP_UNCHANGED),
                                m_groupNum(PEP_UNCHANGED),
                                m_mutuallyExclusiveGroup(PEP_UNCHANGED)
    {}

    CRPEParameterValueInfo (short isNullable,
                            short disallowEditing,
                            short allowMultipleValues,
                            short hasDiscreteValues,
                            short partOfGroup,
                            short groupNum,
                            short mutuallyExclusiveGroup
                           ) : m_StructSize(sizeof(CRPEParameterValueInfo)),
                               m_isNullable(isNullable),
                               m_disallowEditing(disallowEditing),
                               m_allowMultipleValues(allowMultipleValues),
                               m_hasDiscreteValues(hasDiscreteValues),
                               m_partOfGroup(partOfGroup),
                               m_groupNum(groupNum),
                               m_mutuallyExclusiveGroup(mutuallyExclusiveGroup)
    {}

    WORD  m_StructSize;
    short m_isNullable;               // Boolean value or PEP_UNCHANGED for no change.
    short m_disallowEditing;          // Boolean value or PEP_UNCHANGED for no change.
    short m_allowMultipleValues;      // Boolean value or PEP_UNCHANGED for no change.
    short m_hasDiscreteValues;        // Boolean value or PEP_UNCHANGED for no change.
                                      // True: has discrete values
                                      // False: has ranges
    short m_partOfGroup;              // Boolean value or PEP_UNCHANGED for no change.
    short m_groupNum;                 // a group number or PEP_UNCHANGED for no change.
    short m_mutuallyExclusiveGroup;   // Boolean value or PEP_UNCHANGED for no change.
};



// subreport info
const int PEP_SUBREPORT_NAME_LEN = 128;

struct CRPESubreportInfo
{
    CRPESubreportInfo () : m_StructSize(sizeof(CRPESubreportInfo))
    {
        m_name[0] = '\0';
        m_NLinks = 0;        
        m_IsOnDemand = FALSE;
    }

    WORD m_StructSize; // Initialize to size of CRPESubreportInfo
    
    _TCHAR m_name[PEP_SUBREPORT_NAME_LEN];

        // number of links
    short m_NLinks;

    short m_IsOnDemand;    // TRUE if is subreport is On Demand
};

//new in 6.0 Window Options
struct CRPEWindowOptions
{
    CRPEWindowOptions () :  m_StructSize(sizeof(CRPEWindowOptions)),
							m_hasGroupTree(0),
							m_canDrillDown(0),
							m_hasNavigationControls(0),
							m_hasCancelButton(0),
							m_hasPrintButton(0),
							m_hasExportButton(0),
							m_hasZoomControl(0),
							m_hasCloseButton(0),
							m_hasProgressControls(0),
							m_hasSearchButton(0),
							m_hasPrintSetupButton(0),
							m_hasRefreshButton(0)
    {}

    CRPEWindowOptions (short hasGroupTree, 
					short canDrillDown, 
					short hasNavigationControls, 
					short hasCancelButton,
					short hasPrintButton,
					short hasExportButton, 
					short hasZoomControl, 
					short hasCloseButton, 
					short hasProgressControls,
					short hasSearchButton, 
					short hasPrintSetupButton, 
					short hasRefreshButton
					) :
					m_StructSize(sizeof(CRPEWindowOptions)),
					m_hasGroupTree(hasGroupTree),
					m_canDrillDown(canDrillDown),
					m_hasNavigationControls(hasNavigationControls),
					m_hasCancelButton(hasCancelButton),
					m_hasPrintButton(hasPrintButton),
					m_hasExportButton(hasExportButton),
					m_hasZoomControl(hasZoomControl),
					m_hasCloseButton(hasCloseButton),
					m_hasProgressControls(hasProgressControls),
					m_hasSearchButton(hasSearchButton),
					m_hasPrintSetupButton(hasPrintSetupButton),
					m_hasRefreshButton(hasRefreshButton)
    {}

    // Initialize to sizeof(CRPESessionInfo)
    WORD m_StructSize;

    short m_hasGroupTree, m_canDrillDown, m_hasNavigationControls, 
		m_hasCancelButton, m_hasPrintButton, m_hasExportButton, 
		m_hasZoomControl, m_hasCloseButton, m_hasProgressControls,
		m_hasSearchButton, m_hasPrintSetupButton, m_hasRefreshButton;
};

// define value type
const int PEP_VI_NUMBER     = 0;
const int PEP_VI_CURRENCY   = 1;
const int PEP_VI_BOOLEAN    = 2;
const int PEP_VI_DATE       = 3;
const int PEP_VI_STRING     = 4;
const int PEP_VI_DATETIME   = 5;
const int PEP_VI_TIME       = 6;
const int PEP_VI_INTEGER    = 7;
const int PEP_VI_COLOR      = 8;
const int PEP_VI_CHAR       = 9;
const int PEP_VI_LONG       = 10;
const int PEP_VI_NOVALUE    = 100;

const int PEP_VI_STRING_LEN = 256;

struct CRPEValueInfo
{

    CRPEValueInfo () : m_StructSize(sizeof(CRPEValueInfo)),
                       m_valueType(0),
                       m_viNumber(0),
                       m_viCurrency(0),
                       m_viBoolean(FALSE),
                       m_viColor(0x00000000),
                       m_viInteger(0),
                       m_viC('\0'),
                       m_ignored('\0'),
                       m_viLong(0)
    {
        m_viString[0] = ('\0');

        short i;
        for (i = 0; i < 3; i++)
        {
            m_viDate[i] = 0;
            m_viTime[i] = 0;
        }

        for (i = 0; i < 6; i++)
        {
            m_viDateTime[i] = 0;
        }    
    }

    CRPEValueInfo (WORD valueType,
                   double viNumber,
                   double viCurrency,
                   BOOL viBoolean,
                   const _TCHAR *viString,
                   short viDate[3],
                   short viDateTime[6],
                   short viTime[3],
                   COLORREF viColor,
                   short viInteger,
                   char viC,
	               char ignored,
	               long viLong
                  ) : m_StructSize(sizeof(CRPEValueInfo)),
                      m_valueType(valueType),
                      m_viNumber(viNumber),
                      m_viCurrency(viBoolean),
                      m_viBoolean(FALSE),
                      m_viColor(viColor),
                      m_viInteger(viInteger),
                      m_viC(viC),
                      m_ignored('\0'),
                      m_viLong(viLong)
    {
        lstrcpyn(m_viString, viString, sizeof(m_viString) - 1);

        short i;
        for (i = 0; i < 3; i++)
        {
            m_viDate[i] = viDate[i];
            m_viTime[i] = viTime[i];
        }

        for (i = 0; i < 6; i++)
        {
            m_viDateTime[i] = viDateTime[i];
        }
    }

    WORD     m_StructSize;
    WORD     m_valueType; // a PE_VI_ constant
    double   m_viNumber;
    double   m_viCurrency;
    BOOL     m_viBoolean;
    _TCHAR   m_viString[PEP_VI_STRING_LEN];
    short    m_viDate[3]; // year, month, day
    short    m_viDateTime[6]; // year, month, day, hour, minute, second
    short    m_viTime[3];  // hour, minute, second
    COLORREF m_viColor;
    short    m_viInteger;
    char     m_viC; //BYTE
	char     m_ignored; // for 4 byte alignment. ignored.
	long     m_viLong;
}; 

// event ID
const int PEP_CLOSE_PRINT_WINDOW_EVENT           = 1;
const int PEP_ACTIVATE_PRINT_WINDOW_EVENT        = 2;
const int PEP_DEACTIVATE_PRINT_WINDOW_EVENT      = 3;
const int PEP_PRINT_BUTTON_CLICKED_EVENT         = 4;
const int PEP_EXPORT_BUTTON_CLICKED_EVENT        = 5;
const int PEP_ZOOM_LEVEL_CHANGING_EVENT          = 6;
const int PEP_FIRST_PAGE_BUTTON_CLICKED_EVENT    = 7;
const int PEP_PREVIOUS_PAGE_BUTTON_CLICKED_EVENT = 8;
const int PEP_NEXT_PAGE_BUTTON_CLICKED_EVENT     = 9;
const int PEP_LAST_PAGE_BUTTON_CLICKED_EVENT     = 10;
const int PEP_CANCEL_BUTTON_CLICKED_EVENT        = 11;
const int PEP_CLOSE_BUTTON_CLICKED_EVENT         = 12;
const int PEP_SEARCH_BUTTON_CLICKED_EVENT        = 13;
const int PEP_GROUP_TREE_BUTTON_CLICKED_EVENT    = 14;
const int PEP_PRINT_SETUP_BUTTON_CLICKED_EVENT   = 15;
const int PEP_REFRESH_BUTTON_CLICKED_EVENT       = 16;
const int PEP_SHOW_GROUP_EVENT                   = 17;
const int PEP_DRILL_ON_GROUP_EVENT               = 18; // include drill on graph
const int PEP_DRILL_ON_DETAIL_EVENT              = 19;
const int PEP_READING_RECORDS_EVENT              = 20;
const int PEP_START_EVENT                        = 21;
const int PEP_STOP_EVENT                         = 22;
const int PEP_MAPPING_FIELD_EVENT                = 23;
const int PEP_RIGHT_CLICK_EVENT                  = 24; // right mouse click

// job destination
const int PEP_TO_NOWHERE = 0;
const int PEP_TO_WINDOW  = 1;
const int PEP_TO_PRINTER = 2;
const int PEP_TO_EXPORT  = 3;
const int PEP_FROM_QUERY = 4;

// mouse click action
const int PEP_MOUSE_NOTSUPPORTED  = 0;
const int PEP_MOUSE_DOWN          = 1;
const int PEP_MOUSE_UP            = 2;

// mouse click flags (virtual key state-masks)
const int PEP_CF_NONE       = 0x0000;
const int PEP_CF_LBUTTON    = 0x0001;
const int PEP_CF_RBUTTON    = 0x0002;
const int PEP_CF_SHIFTKEY   = 0x0004;
const int PEP_CF_CONTROLKEY = 0x0008;
const int PEP_CF_MBUTTON    = 0x0010;

// for PE_RIGHT_CLICK_EVENT
struct CRPEMouseClickEventInfo
{
    CRPEMouseClickEventInfo () : m_StructSize(sizeof(CRPEMouseClickEventInfo)),
                                 m_windowHandle(0),
                                 m_clickAction(0),
                                 m_clickFlags(0),
                                 m_xOffset(0),
                                 m_yOffset(0),
                                 m_fieldValue(NULL),
                                 m_objectHandle(0),
                                 m_sectionCode(0)
    {}

    CRPEMouseClickEventInfo (long windowHandle,
                             UINT clickAction,
                             UINT clickFlags,
                             int xOffset,
                             int yOffset,
                             CRPEValueInfo *fieldValue,
                             DWORD objectHandle,
                             short sectionCode
                            ) : m_StructSize(sizeof(CRPEMouseClickEventInfo)),
                                m_windowHandle(windowHandle),
                                m_clickAction(clickAction),
                                m_clickFlags(clickFlags),
                                m_xOffset(xOffset),
                                m_yOffset(yOffset),     
                                m_fieldValue(fieldValue),
                                m_objectHandle(objectHandle),
                                m_sectionCode(sectionCode)
    {
        
    }

        

    WORD          m_StructSize;
    long          m_windowHandle;
    UINT          m_clickAction;   // mouse button down or up
    UINT          m_clickFlags;    // any combination of PE_CF_*
    int           m_xOffset;       // x-coordinate of mouse click in pixels
    int           m_yOffset;       // y-coordinate of mouse click in pixels
    CRPEValueInfo *m_fieldValue;    // value of object at click point if it is a field
                                 // object, excluding MEMO and BLOB fields,
                                 // else valueType element = PE_VI_NOVALUE.
    DWORD         m_objectHandle;  // the design view object
    short         m_sectionCode;   // section in which click occurred.
}; 

const int PEP_TABLE_NAME_LEN          = 128;
const int PEP_DATABASE_FIELD_NAME_LEN = 128;

// Field value type
const int PEP_FVT_INT8SFIELD          = 1;
const int PEP_FVT_INT8UFIELD          = 2;
const int PEP_FVT_INT16SFIELD         = 3;
const int PEP_FVT_INT16UFIELD         = 4;
const int PEP_FVT_INT32SFIELD         = 5;
const int PEP_FVT_INT32UFIELD         = 6;
const int PEP_FVT_NUMBERFIELD         = 7;
const int PEP_FVT_CURRENCYFIELD       = 8;
const int PEP_FVT_BOOLEANFIELD        = 9;
const int PEP_FVT_DATEFIELD           = 10;
const int PEP_FVT_TIMEFIELD           = 11;
const int PEP_FVT_STRINGFIELD         = 12;
const int PEP_FVT_TRANSIENTMEMOFIELD  = 13;
const int PEP_FVT_PERSISTENTMEMOFIELD = 14;
const int PEP_FVT_BLOBFIELD           = 15;
const int PEP_FVT_DATETIMEFIELD       = 16;
const int PEP_FVT_BITMAPFIELD         = 17;
const int PEP_FVT_ICONFIELD           = 18;
const int PEP_FVT_PICTUREFIELD        = 19;
const int PEP_FVT_OLEFIELD            = 20;
const int PEP_FVT_GRAPHFIELD          = 21;
const int PEP_FVT_UNKNOWNFIELD        = 22;

// Field mapping types
const int PEP_FM_AUTO_FLD_MAP          = 0;     //Automatic field name mapping
                                                //NOTE : unmapped report fields will be removed
const int PEP_FM_CRPE_PROMPT_FLD_MAP   = 1;     //CRPE provides dialog box to map field manually
const int PEP_FM_EVENT_DEFINED_FLD_MAP = 2;     //CRPE provides list of field in report and new database
                                                //User needs to activate the PE_MAPPING_FIELD_EVENT
                                                //and define a callback function

struct CRPEReportFieldMappingInfo
{
    CRPEReportFieldMappingInfo () : m_StructSize(sizeof(CRPEReportFieldMappingInfo)), 
                                    m_valueType(0),
                                    m_mappingTo(0)
    {
        m_tableAliasName[0] = '\0';
        m_databaseFieldName[0] = '\0';
    }

    CRPEReportFieldMappingInfo (WORD         valueType,
                                const _TCHAR *tableAliasName,
                                const _TCHAR *databaseFieldName,
                                int          mappingTo
                               ) : m_StructSize(sizeof(CRPEReportFieldMappingInfo)), 
                                   m_valueType(valueType),
                                   m_mappingTo(mappingTo)
    {
        lstrcpyn(m_tableAliasName, tableAliasName, sizeof(m_tableAliasName) - 1);        
        lstrcpyn(m_databaseFieldName, databaseFieldName, sizeof(m_databaseFieldName) - 1);
    }

    WORD   m_StructSize;
    WORD   m_valueType;     // a PE_FVT_constant
    _TCHAR m_tableAliasName[PEP_TABLE_NAME_LEN];
    _TCHAR m_databaseFieldName[PEP_DATABASE_FIELD_NAME_LEN];
    int    m_mappingTo;     //mapped fields are assigned to the index of a field
                            //in array PEFieldMappingEventInfo->databaseFields
                            //unmapped fields are assigned to -1
};

struct CRPEFieldMappingEventInfo
{
    CRPEFieldMappingEventInfo () : m_StructSize(sizeof(CRPEFieldMappingEventInfo)), 
                                   m_reportFields(NULL),
                                   m_nReportFields(0),
                                   m_databaseFields(NULL),
                                   m_nDatabaseFields(0)

    {}

                                   
    CRPEFieldMappingEventInfo (CRPEReportFieldMappingInfo *reportFields,
                               WORD nReportFields,
                               CRPEReportFieldMappingInfo *databaseFields,
                               WORD nDatabaseFields
                              ) : m_StructSize(sizeof(CRPEFieldMappingEventInfo)), 
                                  m_reportFields(reportFields),
                                  m_nReportFields(nReportFields),
                                  m_databaseFields(databaseFields),
                                  m_nDatabaseFields(nDatabaseFields)

    {}

   WORD m_StructSize;

   CRPEReportFieldMappingInfo *m_reportFields;  //An array of fields in the report.
                                                //User need to modify the 'mappingTo' of
                                                //each new mapped field by assigning the value
                                                //of the index of a field in the array
                                                //databaseFields.
   WORD m_nReportFields;                        //Size of array reportFields

   CRPEReportFieldMappingInfo *m_databaseFields;    //An array of fields in the new database file
   WORD m_nDatabaseFields;                          //Size of array databaseField
};

struct CRPEEnableEventInfo
{
    CRPEEnableEventInfo (void) : m_StructSize(sizeof(CRPEWindowOptions)),
							 m_startStopEvent(0),
							 m_readingRecordEvent(0),
							 m_printWindowButtonEvent(0),
							 m_drillEvent(0),
							 m_closePrintWindowEvent(0),
							 m_activatePrintWindowEvent(0),
                             m_fieldMappingEvent(0),
	                         m_mouseClickEvent(0)
    {}

    CRPEEnableEventInfo (short startStopEvent, 
					     short readingRecordEvent, 
					     short printWindowButtonEvent,
					     short drillEvent,
					     short closePrintWindowEvent, 
					     short activatePrintWindowEvent,
                         short fieldMappingEvent,
	                     short mouseClickEvent
					    ) :
					     m_StructSize(sizeof(CRPEEnableEventInfo)),
					     m_startStopEvent(startStopEvent),
					     m_readingRecordEvent(readingRecordEvent),
					     m_printWindowButtonEvent(printWindowButtonEvent),
					     m_drillEvent(drillEvent),
					     m_closePrintWindowEvent(closePrintWindowEvent),
					     m_activatePrintWindowEvent(activatePrintWindowEvent),
                         m_fieldMappingEvent(fieldMappingEvent),
	                     m_mouseClickEvent(mouseClickEvent)
    {}

    // Initialize to sizeof(CRPESessionInfo)
    WORD m_StructSize;

    short m_startStopEvent, m_readingRecordEvent, m_printWindowButtonEvent, 
		m_drillEvent, m_closePrintWindowEvent, m_activatePrintWindowEvent,
        m_fieldMappingEvent, m_mouseClickEvent;
};

//For PE_DRILL_ON_GROUP_EVENT
const int PEP_DE_ON_GROUP     = 0;
const int PEP_DE_ON_GROUPTREE = 1;
const int PEP_DE_ON_GRAPH     = 2;
const int PEP_DE_ON_MAP       = 3;
const int PEP_DE_ON_SUBREPORT = 4;

struct CRPEDrillOnGroupEventInfo
{
    CRPEDrillOnGroupEventInfo () : m_StructSize(sizeof(CRPEDrillOnGroupEventInfo)),
                                   m_drillType(0),
                                   m_windowHandle(0),
                                   m_groupList(NULL),
                                   m_groupLevel(0)
    {}

    WORD     m_StructSize;
	WORD     m_drillType;   // a PE_DE_ constant
	long     m_windowHandle;
	_TCHAR **m_groupList;	// points to an array of group names for drillOnGroup, drillOnGroupTree, drillOnGraph, drillOnMap
						    // points to an array with one element, the subreport name, for drillOnSubreport.
						    // memory pointed by group list is freed after calling the call back function.
	WORD     m_groupLevel;
};

const int PEP_TC_DEFAULT_CURSOR     = 0; // CRPE set default cursor to be PE_TC_ARRAOW_CURSOR
const int PEP_TC_ARROW_CURSOR       = 1;
const int PEP_TC_CROSS_CURSOR       = 2;
const int PEP_TC_IBEAM_CURSOR       = 3;
const int PEP_TC_UPARROW_CURSOR     = 4;
const int PEP_TC_SIZEALL_CURSOR     = 5;
const int PEP_TC_SIZENWSE_CURSOR    = 6;
const int PEP_TC_SIZENESW_CURSOR    = 7;
const int PEP_TC_SIZEWE_CURSOR      = 8;
const int PEP_TC_SIZENS_CURSOR      = 9;
const int PEP_TC_NO_CURSOR          = 10;
const int PEP_TC_WAIT_CURSOR        = 11;
const int PEP_TC_APPSTARTING_CURSOR = 12;
const int PEP_TC_HELP_CURSOR        = 13;
const int PEP_TC_SIZE_CURSOR        = 14; // for 16bit
const int PEP_C_ICON_CURSOR         = 15; // for 16bit

const int PEP_TC_MAGNIFY_CURSOR     = 99; // CRPE specific cusorstruct CRPETrackCursorInfo

struct CRPETrackCursorInfo 
{
    CRPETrackCursorInfo () : m_StructSize(sizeof(CRPETrackCursorInfo)),
                             m_groupAreaCursor(0),
                             m_groupAreaFieldCursor(0),
                             m_detailAreaCursor(0),
                             m_detailAreaFieldCursor(0),
                             m_graphCursor(0),
                             m_groupAreaCursorHandle(0),
                             m_groupAreaFieldCursorHandle(0),
                             m_detailAreaCursorHandle(0),
                             m_detailAreaFieldCursorHandle(0),
                             m_graphCursorHandle(0)
    {}

	WORD  m_StructSize;
	short m_groupAreaCursor;             // a PEP_TC constant. PE_UNCHANGED for no change.
	short m_groupAreaFieldCursor;        // a PEP_TC constant. PE_UNCHAGNED for no change.
	short m_detailAreaCursor;            // a PEP_TC constant. PE_UNCHANGED for no change
	short m_detailAreaFieldCursor;       // a PEP_TC constant. PE_UNCHANGED for no change
	short m_graphCursor;			     // a PEP_TC constant. PE_UNCHANGED for no change.
	long  m_groupAreaCursorHandle;       // reserved
	long  m_groupAreaFieldCursorHandle;  // reserved
	long  m_detailAreaCursorHandle;      // reserved
	long  m_detailAreaFieldCursorHandle; // reserved
	long  m_graphCursorHandle;           // reserved
};

// for PE_DRILL_ON_DETAIL_EVENT
struct CRPEFieldValueInfo
{
    CRPEFieldValueInfo () : m_StructSize(sizeof(CRPEFieldValueInfo)),
	                        m_ignored(0),
	                        m_fieldValue(CRPEValueInfo())
    {
        m_fieldName[0] = '\0';
    }

	WORD          m_StructSize;
	WORD          m_ignored; // for 4 byte alignment. ignore.
	_TCHAR        m_fieldName[PEP_FIELD_NAME_LEN];
	CRPEValueInfo m_fieldValue;
};


struct CRPEDrillOnDetailEventInfo
{
    CRPEDrillOnDetailEventInfo () : m_StructSize(sizeof(CRPEDrillOnDetailEventInfo)),
                                    m_selectedFieldIndex(0),
                                    m_windowHandle(0),
                                    m_fieldValueList(NULL),
                                    m_nFieldValue(0)
    {}

	WORD                  m_StructSize;
	short                 m_selectedFieldIndex; // -1 if no field selected
	long                  m_windowHandle;
	CRPEFieldValueInfo ** m_fieldValueList;
	short                 m_nFieldValue; // number of field value in fieldValueList
};

struct CRPEStartEventInfo
{
    CRPEStartEventInfo () : m_StructSize(sizeof(CRPEStartEventInfo)),
                            m_destination(0)
    {}

	WORD m_StructSize;
	WORD m_destination; 	// a job destination constant.
};

struct CRPEStopEventInfo
{
    CRPEStopEventInfo () : m_StructSize(sizeof(CRPEStartEventInfo)),
                           m_destination(0),
                           m_jobStatus(0)
    {}

	WORD m_StructSize;
	WORD m_destination; 	// a job destination constant.
	WORD m_jobStatus; 	// a PE_JOB constant 
};

struct CRPEReadingRecordsEventInfo
{
    CRPEReadingRecordsEventInfo () : m_StructSize(sizeof(CRPEReadingRecordsEventInfo)),
                                     m_cancelled(FALSE),
                                     m_recordsRead(0),
                                     m_recordsSelected(0),
                                     m_done(FALSE)
    {}

	WORD  m_StructSize;
	short m_cancelled;        // BOOL value. 
	long  m_recordsRead;
	long  m_recordsSelected;
	short m_done;		// BOOL value.
};

// use this structure for 
// PEP_CLOSE_PRINT_WINDOW_EVENT
// PEP_PRINT_BUTTON_CLICKED_EVENT
// PEP_EXPORT_BUTTON_CLICKED_EVENT
// PEP_FIRST_PAGE_BUTTON_CLICKED_EVENT
// PEP_PREVIOUS_PAGE_BUTTON_CLICKED_EVENT
// PEP_NEXT_PAGE_BUTTON_CLICKED_EVENT
// PEP_LAST_PAGE_BUTTON_CLICKED_EVENT
// PEP_CANCEL_BUTTON_CLICKED_EVENT
// PEP_PRINT_SETUP_BUTTON_CLICKED_EVENT
// PEP_REFRESH_BUTTON_CLICKED_EVENT
// PEP_ACTIVATE_PRINT_WINDOW_EVENT
// PEP_DEACTIVATE_PRINT_WINDOW_EVENT
struct CRPEGeneralPrintWindowEventInfo
{
    CRPEGeneralPrintWindowEventInfo () : m_StructSize(sizeof(CRPEGeneralPrintWindowEventInfo)),
                                         m_ignored(0),
                                         m_windowHandle(0)
    {}

	WORD m_StructSize;
	WORD m_ignored; // for 4 byte alignment. ignore.
	long m_windowHandle; // HWND
};

struct CRPEZoomLevelChangingEventInfo
{
    CRPEZoomLevelChangingEventInfo () : m_StructSize(sizeof(CRPEZoomLevelChangingEventInfo)),
                                        m_zoomLevel(0),
                                        m_windowHandle(0)
    {}
    WORD m_StructSize;
	WORD m_zoomLevel;
	long m_windowHandle;
};

struct CRPECloseButtonClickedEventInfo
{
    CRPECloseButtonClickedEventInfo () : m_StructSize(sizeof(CRPECloseButtonClickedEventInfo)),
                                         m_viewIndex(0),
                                         m_windowHandle(0)
    {}
    WORD m_StructSize;
	WORD m_viewIndex;
	long m_windowHandle;
};

const int PEP_SEARCH_STRING_LEN = 128;

struct CRPESearchButtonClickedEventInfo
{
    CRPESearchButtonClickedEventInfo () : m_StructSize(sizeof(CRPECloseButtonClickedEventInfo)),
                                          m_windowHandle(0)
    {
        m_searchString[0] = '\0';
    }

	long   m_windowHandle;
	_TCHAR m_searchString[PEP_SEARCH_STRING_LEN];
	WORD   m_StructSize;
};

struct CRPEGroupTreeButtonClickedEventInfo
{
    CRPEGroupTreeButtonClickedEventInfo () : m_StructSize(sizeof(CRPEGroupTreeButtonClickedEventInfo)),
                                             m_visible(FALSE),
                                             m_windowHandle(0)
    {}

    WORD  m_StructSize;
	short m_visible;
	long  m_windowHandle;
};

struct CRPEShowGroupEventInfo
{
    CRPEShowGroupEventInfo () : m_StructSize(sizeof(CRPEShowGroupEventInfo)),
                                m_groupLevel(0),
                                m_windowHandle(0),
                                m_groupList(NULL)
    {}

	WORD      m_StructSize;
	WORD      m_groupLevel;
	long      m_windowHandle;
	_TCHAR ** m_groupList;	// points to an array of group names.
						    // memory pointed by group list is freed after calling the call back function.
};

class CRPEJob : public CObject
{
    public:
        
    // constructors

        CRPEJob (short jobHandle);

        CRPEJob (short jobHandle, CRPEJob *parentJob);

    // destructor

        ~CRPEJob ();

    // operations

        BOOL Start ();  // start generating print job output

        //Retrieve the information set in the File | Report Options menu in Designer
        BOOL GetReportOptions (CRPEReportOptions *reportOptions); 

        //Set the information set in the File | Report Options menu in Designer
        BOOL SetReportOptions (CRPEReportOptions *reportOptions);

        BOOL GetReportSummaryInfo(CRPEReportSummaryInfo *summaryInfo);

        BOOL SetReportSummaryInfo(CRPEReportSummaryInfo *summaryInfo);

        void Close ();  // closes print job AND deletes CRPEJob object
        
        void Cancel (); // cancel print job output generation

        BOOL ShowNextPage ();           // show next page in preview window
        
        BOOL ShowFirstPage ();          // show first page in preview window

        BOOL ShowPreviousPage ();       // show previous page in preview window

        BOOL ShowLastPage ();           // show last page of preview window

        BOOL ShowNthPage (short pageN); // show page N

        BOOL ShowPrintControls (BOOL showControls); // show controls in
                                                    // preview window

        BOOL ZoomPreviewWindow (short level); // set preview magnification

        BOOL NextWindowMagnification (); // step to the next preview window
                                         // magnification

        BOOL PrintWindow (); // print the preview window to printer

        BOOL ExportPrintWindow (BOOL toMail); // export the preview window

        void CloseWindow (); // close the preview window

        BOOL TestNthTableConnectivity (short tableN); // test to see if a valid
                                                      // connection exists to
                                                      // the specified table

        BOOL DiscardSavedData (); // discards any saved data in the report

    // attributes

        // report formula text attributes
        short GetNFormulas();

        BOOL GetNthFormula(short formulaN,
                           CString &formulaName,
                           CString &formulaText);

        BOOL GetFormula (const _TCHAR *formulaName,
                         CString &formulaText);

        BOOL SetFormula (const _TCHAR *formulaName,
                         const _TCHAR *formulaText);

        BOOL CheckFormula(const _TCHAR *formulaName);

        // record selection formula attributes
        BOOL GetSelectionFormula (CString &formulaText);

        BOOL SetSelectionFormula (const _TCHAR *formulaText);

        BOOL CheckSelectionFormula ();

        // group selection formula attributes
        BOOL GetGroupSelectionFormula (CString &formulaText);

        BOOL SetGroupSelectionFormula (const _TCHAR *formulaText);

        BOOL CheckGroupSelectionFormula();

        //SQL Expressions
        short GetNSQLExpressions();

        BOOL GetNthSQLExpression(short expressionN,
                                 CString &expressionName,
                                 CString &expressionText);

        BOOL GetSQLExpression(const _TCHAR *expressionName,
                              CString &expressionText);

        BOOL SetSQLExpression(const _TCHAR *expressionName,
                              const _TCHAR *expressionText);

        BOOL CheckSQLExpression(const _TCHAR *expressionName);

        // group condition attributes
        short GetNGroups ();

        BOOL GetGroupCondition (short sectionCode,
                                CString &conditionField,
                                short *condition,
                                short *sortDirection);

        BOOL SetGroupCondition (short sectionCode,
                                const _TCHAR *conditionField,
                                short condition,
                                short sortDirection);

        BOOL GetGroupOptions(short groupN,
                             CRPEGroupOptions *groupOptions);

        BOOL SetGroupOptions(short groupN,
                             CRPEGroupOptions *groupOptions);

        // record sort order field attributes
        short GetNSortFields ();

        BOOL GetNthSortField (short sortFieldN,
                              CString &field,
                              short *direction);

        BOOL SetNthSortField (short sortFieldN,
                              const _TCHAR *field,
                              short direction);

        BOOL DeleteNthSortField (short sortFieldN);

        // group sort order field attributes
        short GetNGroupSortFields ();

        BOOL GetNthGroupSortField (short sortFieldN,
                                   CString &field,
                                   short *direction);
                                    
        BOOL SetNthGroupSortField (short sortFieldN,
                                   const _TCHAR *field,
                                   short direction);

        BOOL DeleteNthGroupSortField (short sortFieldN);

        // database table attributes
        short GetNTables ();

        BOOL GetNthTableType (short tableN,
                              CRPETableType *tableType);

        BOOL GetNthTableSessionInfo (short tableN,
                                     CRPESessionInfo *sessionInfo);

        BOOL SetNthTableSessionInfo (short tableN,
                                     const CRPESessionInfo *sessionInfo,
                                     BOOL propagate);

        BOOL GetNthTableLocation (short tableN,
                                  CRPETableLocation *tableLocation);

        BOOL SetNthTableLocation (short tableN,
                                  const CRPETableLocation *tableLocation);

        BOOL GetNthTableLogonInfo (short tableN,
                                   CRPELogOnInfo *logonInfo);

        BOOL SetNthTableLogonInfo (short tableN,
                                   const CRPELogOnInfo *logonInfo,
                                   BOOL propagate);

        // SQL query attributes
        BOOL GetSQLQuery (CString &query);

        BOOL SetSQLQuery (const _TCHAR *query);

        BOOL VerifyDatabase();

        // report section attributes
        BOOL GetNDetailCopies(short *nCopies);
    
        BOOL SetNDetailCopies (short nCopies);

        short GetNSections ();

        short GetSectionCode (short sectionN);

        BOOL GetSectionHeight (short sectionCode,
                                      short *height);
        
        BOOL SetSectionHeight (short sectionCode,
                                      short height);

        BOOL SetFont (short sectionCode,
                      short scopeCode,
                      const _TCHAR *faceName,
                      short fontFamily,
                      short fontPitch,
                      short charSet,
                      short pointSize,
                      short isItalic,
                      short isUnderlined,
                      short isStruckOut,
                      short weight);

        BOOL GetSectionFormat (short sectionCode,
                               CRPESectionOptions *options);

        BOOL SetSectionFormat (short sectionCode,
                               const CRPESectionOptions *options);

        BOOL GetAreaFormatFormula(short areaCode,
                                   short formulaName, // an area PEP_FFN_ constant
                                   CString &formulaText);


        BOOL SetAreaFormatFormula(short areaCode,
                                  short formulaName, // an area PEP_FFN_ constant
                                  CString formulaText);

        BOOL GetSectionFormatFormula(short areaCode,
                                     short sectionCode,
                                     short formulaName, // an area PEP_FFN_ constant
                                     CString &formulaText);

        BOOL SetSectionFormatFormula(short sectionCode,
                                     short formulaName, // an area PEP_FFN_ constant
                                     CString formulaText);

        // graphing attributes
        BOOL GetGraphType (short sectionCode,
                           short graphN,
                           short *graphType);

        BOOL SetGraphType (short sectionCode,
                           short graphN,
                           short graphType);

        BOOL GetGraphData (short sectionCode,
                           short graphN,
                           CRPEGraphDataInfo *graphDataInfo);

        BOOL SetGraphData (short sectionCode,
                           short graphN,
                           CRPEGraphDataInfo *graphDataInfo);

        BOOL GetGraphText (short sectionCode,
                           short graphN,
                           CRPEGraphTextInfo *graphTextInfo);

        BOOL SetGraphText (short sectionCode,
                           short graphN,
                           CRPEGraphTextInfo *graphTextInfo);

        BOOL GetGraphOptions (short sectionCode,
                              short graphN,
                              CRPEGraphOptions *graphOptions);

        BOOL SetGraphOptions (short sectionCode,
                              short graphN,
                              CRPEGraphOptions *graphOptions);

        // report attributes
        BOOL GetReportTitle (CString &title);

        BOOL SetReportTitle (const _TCHAR *title);

        BOOL GetPrintDate (short *year,
                           short *month,
                           short *day);
        
        BOOL SetPrintDate (short year,
                           short month,
                           short day);

        BOOL HasSavedData (BOOL *hasSavedData);
		
		BOOL GetWindowOptions (CRPEWindowOptions *windowOptions);

		BOOL SetWindowOptions (const CRPEWindowOptions *windowOptions);

        BOOL GetFieldMappingType (WORD mappingType); //use PE_FM_ constant

        BOOL SetFieldMappingType (WORD mappingType); //use PE_FM_ constant
        
        BOOL SetTrackCursorInfo(CRPETrackCursorInfo *cursorInfo);
        
        BOOL GetTrackCursorInfo(CRPETrackCursorInfo *cursorInfo);

        BOOL EnableEvent (const CRPEEnableEventInfo *enableEventInfo);

		BOOL GetEnableEventInfo (CRPEEnableEventInfo *enableEventInfo);

		BOOL SetEventCallback ( BOOL ( CALLBACK * callbackProc )(short eventID, void *param, void *userData), void *userData);

        short GetNPages ();

        // report printer attributes
        BOOL GetSelectedPrinter (CString &driverName,
                                 CString &printerName,
                                 CString &portName,
                                 DEVMODE **mode);

        BOOL SelectPrinter (const _TCHAR *driverName,
                            const _TCHAR *printerName,
                            const _TCHAR *portName,
                            const DEVMODE *mode = 0);

        BOOL GetPrintOptions (CRPEPrintOptions *options);

        BOOL SetPrintOptions (const CRPEPrintOptions *options);

        // output destination options
        BOOL OutputToPrinter (short nCopies = 1);

        BOOL OutputToWindow (const _TCHAR *title,
                             int left,
                             int top,
                             int width,
                             int height,
                             int style,
                             CWnd *parentWindow);

        BOOL OutputToWindow (const char *title,
                             int left,
                             int top,
                             int width,
                             int height,
                             int style,
                             CMDIFrameWnd *parentWindow);

        HWND GetWindowHandle ();

        BOOL PrintControlsShowing (BOOL *controlsShowing);

        BOOL GetExportOptions (CRPEExportOptions *options);

        BOOL ExportTo (const CRPEExportOptions *options);

        BOOL GetMargins (short *left,
                         short *right,
                         short *top,
                         short *bottom);

        BOOL SetMargins (short left,
                         short right,
                         short top,
                         short bottom);

        // job status attributes
        short GetJobStatus (CRPEJobInfo *jobStatus);

        BOOL IsJobFinished ();

        short GetErrorCode ();
        
        CString GetErrorText ();

        // dialog control attributes
        BOOL SetDialogParentWindow (CWnd *parentWindow);

        BOOL EnableProgressDialog (BOOL enable);

		BOOL CRPEJob::GetAllowPromptDialog ();

		BOOL CRPEJob::SetAllowPromptDialog(BOOL showPromptDialog);

        // parameter field attributes
        short GetNParameterFields ();

        BOOL GetNthParameterField (short parameterN,
                                   CRPEParameterFieldInfo *parameterInfo);

        BOOL SetNthParameterField (short parameterN,
                                   const CRPEParameterFieldInfo *parameterInfo);

        short GetNParameterDefaultValues (const _TCHAR *parameterFieldName, 
                                          const _TCHAR *reportName);

        BOOL GetNthParameterDefaultValue (const _TCHAR *parameterFieldName, 
                                          const _TCHAR *reportName, 
                                          short index, 
                                          CRPEValueInfo *valueInfo);

        BOOL SetNthParameterDefaultValue (const _TCHAR *parameterFieldName, 
                                          const _TCHAR *reportName, 
                                          short index, 
                                          CRPEValueInfo *valueInfo);

        BOOL AddParameterDefaultValue(const _TCHAR *parameterFieldName, 
                                      const _TCHAR *reportName, 
                                      CRPEValueInfo *valueInfo);

        BOOL DeleteNthParameterDefaultValue(const _TCHAR *parameterFieldName, 
                                            const _TCHAR *reportName, 
                                            short index);

        BOOL GetParameterMinMaxValue(const _TCHAR *parameterFieldName,
                                     const _TCHAR *reportName,
                                     CRPEValueInfo *valueMin, // Set to NULL to retrieve MAX only; must be non-NULL if valueMax is NULL.
                                     CRPEValueInfo *valueMax  // Set to NULL to retrieve MIN only; must be non-NULL if valueMin is NULL.
                                     );

        BOOL SetParameterMinMaxValue(const _TCHAR *parameterFieldName,
                                     const _TCHAR *reportName,
                                     CRPEValueInfo *valueMin, // Set to NULL to set MAX only; must be non-NULL if valueMax is NULL.
                                     CRPEValueInfo *valueMax  // Set to NULL to set MIN only; must be non-NULL if valueMin is NULL.
                                                                    // If both valueInfo and valueMax are non-NULL then
                                                                    // valueMin->valueType MUST BE THE SAME AS valueMax->valueType.
                                                                    // If different, PE_ERR_INCONSISTANTTYPES is returned.
                                     );
        BOOL GetParameterValueInfo(const _TCHAR *parameterFieldName, 
                                   const _TCHAR *reportName, 
                                   CRPEParameterValueInfo *valueInfo);

        BOOL SetParameterValueInfo(const _TCHAR *parameterFieldName, 
                                   const _TCHAR *reportName, 
                                   CRPEParameterValueInfo *valueInfo);

        unsigned short GetNParameterCurrentValues(const _TCHAR *parameterFieldName, 
                                                  const _TCHAR *reportName);

        BOOL GetNthParameterCurrentValue(const _TCHAR *parameterFieldName, 
                                         const _TCHAR *reportName, 
                                         short index, 
                                         CRPEValueInfo *currentValue);

        BOOL AddParameterCurrentValue(const _TCHAR *parameterFieldName, 
                                      const _TCHAR *reportName, 
                                      CRPEValueInfo *currentValue);

        unsigned short GetNParameterCurrentRanges(const _TCHAR *parameterFieldName, 
                                                  const _TCHAR *reportName);

        BOOL GetNthParameterCurrentRange (const _TCHAR *parameterFieldName,
                                          const _TCHAR *reportName,
                                          short index,
                                          CRPEValueInfo *rangeStart,    // Set to NULL to set MAX only; must be non-NULL if valueMax is NULL.
                                          CRPEValueInfo *rangeEnd,      // Set to NULL to set MIN only; must be non-NULL if valueMin is NULL.
                                                                        // If both valueInfo and valueMax are non-NULL then
                                                                        // valueMin->valueType MUST BE THE SAME AS valueMax->valueType.
                                                                        // If different, PE_ERR_INCONSISTANTTYPES is returned.
                                          short *rangeInfo
                                         );

        BOOL AddParameterCurrentRange (const _TCHAR *parameterFieldName,
                                       const _TCHAR *reportName,
                                       CRPEValueInfo *rangeStart,    // Set to NULL to set MAX only; must be non-NULL if valueMax is NULL.
                                       CRPEValueInfo *rangeEnd,      // Set to NULL to set MIN only; must be non-NULL if valueMin is NULL.
                                                                     // If both valueInfo and valueMax are non-NULL then
                                                                     // valueMin->valueType MUST BE THE SAME AS valueMax->valueType.
                                                                     // If different, PE_ERR_INCONSISTANTTYPES is returned.
                                       short rangeInfo  
                                      );

        short GetNthParameterType (short index); // returns PEP_PO_* or -1 if index is invalid.

        BOOL ClearParameterCurrentValuesAndRanges (const _TCHAR *parameterFieldName,
                                                   const _TCHAR *reportName);

        // subreport attributes
        short GetNSubreportsInSection (short sectionCode);

        DWORD GetNthSubreportInSection (short sectionCode,
                                        short subreportN);

        BOOL GetSubreportInfo (DWORD subreportHandle,
                               CRPESubreportInfo *subreportInfo);

        CRPEJob *OpenSubreportJob (const _TCHAR *subreportName);

        // misc. attributes
        short GetJobHandle ();

        // implementation
        #ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
        #endif

        void AddSubReportJob (CRPEJob *job);

        void RemoveSubReportJob (CRPEJob *job);

    protected:
        short m_jobHandle;

        // MDI cached values
        BOOL m_mdiOutput;

        CString m_mdiTitle;
        
        int m_mdiLeft;
        
        int m_mdiTop;
        
        int m_mdiWidth;
        
        int m_mdiHeight;
        
        int m_mdiStyle;
        
        CMDIFrameWnd *m_mdiFrameWnd;
        
        CObArray m_subReportJobs;

        CRPEJob *m_parentJob;
};


class CRPEngine : public CObject
{
    public:
        enum Status {engineOpen, engineClosed, engineMissing};

        static CRPEngine *GetEngine ();

        static Status GetEngineStatus ();

    // constructors

        CRPEngine (BOOL open = FALSE);

    // destructor

        ~CRPEngine ();

    // operations

        BOOL Open ();

        void Close ();

        CRPEJob *OpenJob (const _TCHAR *reportFileName);

        BOOL LogOnServer (const _TCHAR *dllName,
                          const CRPELogOnInfo *logOnInfo);

        BOOL LogOffServer (const _TCHAR *dllName,
                           const CRPELogOnInfo *logOnInfo);

        BOOL LogOnSQLServerWithPrivateInfo(const _TCHAR *dllName,
                                           void *privateInfo);

        short PrintReport (const _TCHAR *reportFilePath,
                           BOOL toPrinter,
                           BOOL toWindow,
                           const _TCHAR *title,
                           int left,
                           int top,
                           int width,
                           int height,
                           DWORD style,
                           CWnd *parentWindow);

    // attributes

        BOOL CanClose ();
        
        short GetVersion (short versionRequested);

        short GetErrorCode ();

        CString GetErrorText ();

        int GetNPrintJobs ();

    // implementation

        void AddJob (CRPEJob *job);

        void RemoveJob (CRPEJob *job);

        BOOL GetHandleString (HANDLE textHandle,
                              short textLength,
                              CString &string);

        #ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
        #endif

    protected:
                                    
        static CRPEngine *thePrintEngine;
        Status m_engineStatus;
        CObArray m_printJobs;
        int m_engineError;
};


class CRPEMDIChildWnd : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CRPEMDIChildWnd)

    public:
	    CRPEMDIChildWnd();

        HWND m_printWnd;
        
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRPEMDIChildWnd)
	//}}AFX_VIRTUAL

// Implementation
    protected:
	    virtual ~CRPEMDIChildWnd();

	// Generated message map functions
	//{{AFX_MSG(CRPEMDIChildWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#if _MSC_VER >= 900
    #pragma pack (pop)
#else
    #pragma pack ()
#endif

#endif // PEPLUS_H
