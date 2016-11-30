#ifndef Practice_Global_Report_Utilities_h
#define Practice_Global_Report_Utilities_h

#pragma once



#include "ReportInfo.h"

#define BYPATIENT 11232
#define BYLOCATION 11233
#define BYPROVIDER 11234
/*
DT: Created 3/10/2000

  Globally used Report Functions

  */



struct CRParameterInfo 
{
	CString m_Data;
	CString m_Name;
};

class CNxReportJob;

// Print the report being previewed (that is the active mdi window under MainFrame), pass the CRPEJob pointer of the corresponding job
void PrintActiveReportPreview(CNxReportJob *pMustBeJob);


void AddPartToClause(OUT CString &strClause, IN const CString &strPart, IN const CString &strAddWithOp = "AND");

// event handling for crystal reports
BOOL CALLBACK ReportEventHandle (short eventID, void *param, void *userData);
void CALLBACK EndReportPrinting(void *RepJob);

// crystal report viewing 
// - dialog based
bool ViewReport (CString strTitle, short nNumber, CString strFile, bool bPreview, CString strFilter);
// - MDI based (preferred)
bool ViewReport (CString strTitle, CString strFile, CString strFilter, CPtrArray *paramList, bool bPreview, CWnd *wndParent);
bool ViewReport(ADODB::_RecordsetPtr &prsSource, CString strTitle, CString strFile, CString strFilter, const CPtrArray *paramList, BOOL bPreview, CWnd *wndParent);

const CString &GetReportsPath();
const CString &GetCustomReportsPath();
const void EnsureCustomReportsPath();
// (a.walling 2013-08-30 09:01) - PLID 57998 - Removed GetParameter which should have been internal to CReportInfo anyway
// extra processing required to fire up the shoesaw
void RunStatement(short Style, bool bPreview, CWnd *parentWnd);

//Is the specified report one of the statements?
BOOL IsStatement(long nID);


//License checking stuff
bool IsReportRefractiveOnly(IN const CReportInfo *pReport);
bool IsReportNexTechOnly(IN const CReportInfo *pReport);
bool IsReportSpaOnly(IN const CReportInfo *pReport);
bool IsReportMarketingOnly(IN const CReportInfo *pReport);
bool IsReportNexWebOnly(IN const CReportInfo *pReport);
bool IsReportRetentionOnly(IN const CReportInfo *pReport);
// (j.gruber 2007-02-21 12:18) - PLID 24048 - added emr license check
bool IsReportNexEMROnly(IN const CReportInfo *pReport);
bool IsReportCustomRecordsOnly(IN const CReportInfo *pReport);
bool IsReportNexEMRORCustomRecords(IN const CReportInfo *pReport);
// (j.jones 2006-11-13 16:09) - PLID 23530 - added ebilling license check
bool IsReportEbillingOnly(IN const CReportInfo *pReport);
// (j.jones 2007-06-29 08:58) - PLID 23951 - added E-Remittance licensing
bool IsReportERemittanceOnly(IN const CReportInfo *pReport);
// (a.walling 2007-08-02 11:18) - PLID 26899 - check for CC Processing license
// (j.jones 2015-09-30 10:45) - PLID 67178 - renamed to clarify which reports are available
// under any CC processing license
bool IsReportAnyCCOnly(IN const CReportInfo *pReport);
// (j.jones 2015-09-30 10:45) - PLID 67178 - defines reports that should be hidden if
// ICCP is both licensed and enabled
bool IsReportNonICCPCCOnly(IN const CReportInfo *pReport);
// (j.jones 2015-09-30 10:58) - PLID 67179 - Defines reports that require the ICCP license.
// ICCP does not have to be enabled for these reports to be available.
bool IsReportICCPCCOnly(IN const CReportInfo *pReport);
// (a.walling 2008-02-14 13:08) - PLID 28388 - added licensing for Advanced Inventory
bool IsReportAdvInventoryOnly(IN const CReportInfo *pReport);
// (r.gonet 2015-11-12 02:46) - PLID 67466 - Some reports will only be available when the global capitation preference is on.
bool IsReportCapitationOnly(IN const CReportInfo *pReport);

bool CheckLicenseForReport(IN const CReportInfo *pReport, bool bSilent);
BOOL GetReportFileName(long nReportID, CString &strFileNameToRun, long &nDefaultCustomReport);
void ClearRPIParameterList(CPtrArray *pParamList);
// (j.camacho 2014-10-21 12:34) - PLID 62716 - added new parameter to specify if you are setting dates, defaults to FALSE
BOOL RunReport(CReportInfo *pReport, CPtrArray *paramList, BOOL bPreview, CWnd *wndParent, CString strTitle = "", CPrintInfo* pInfo = 0, BOOL bUseDateFilter = FALSE);
// (j.camacho 2014-10-21 12:34) - PLID 62716 - added new parameter to specify if you are setting dates, defaults to FALSE
BOOL RunReport(CReportInfo *pReport, BOOL bPreview, CWnd *pParentWnd, CString strTitle = "", CPrintInfo* pInfo = 0, BOOL bUseDateFilter = FALSE);
// (j.jones 2015-04-27 09:13) - PLID 65388 - added shared query for calculating a gift certificates total value and balance
CString GetGiftCertificateValueQuery();

BOOL RunEStatements(CReportInfo *pReport, BOOL bSummary, BOOL bIndiv);

// (j.dinatale 2011-03-28 15:13) - PLID 43023 - Utility functions to clean up RunEStatements
// (j.dinatale 2011-03-30 11:53) - PLID 42982 - ShowEStatementPatSel needs the Report Info object and needs to know if the report is a summary
bool DetermineSavePath(CString &strExportPath);
bool AppendFilters(CReportInfo *pReport);
// (c.haag 2016-03-21) - PLID 68251 - Since patient statement queries can have more than just SQL now, use a CComplexReportQuery object
ADODB::_RecordsetPtr GetIndividualEStatement(CReportInfo *pReport, IN const CComplexReportQuery& patientStatementQuery);
ADODB::_RecordsetPtr GetAllEStatements(IN const CComplexReportQuery& patientStatementQuery);
// (c.haag 2016-05-19 14:15) - PLID-68687 - Deprecated in a larger effort to simply e-statement patient selection
//bool ShowEStatementPatSel(IN const CComplexReportQuery& patientStatementQuery, OUT ADODB::_RecordsetPtr rs, CReportInfo *pReport, BOOL bSummary);
bool ProcessEStatements(ADODB::_RecordsetPtr rs, CFile &OutFile, CReportInfo *pReport, BOOL bSummary, BOOL bIndiv);

BOOL GenerateFilter(IN CReportInfo *pReport, OUT CString *pstrOutFilter = NULL, OUT long *pnItemCount = NULL);
void AddToReportList(CString strReportName, long nReportID, NXDATALISTLib::_DNxDataListPtr pDataList, CString strSelectedFileName);
// (j.gruber 2010-03-11 12:22) - PLID 29120 - added variable to reset selection
void SetReportList(NXDATALISTLib::_DNxDataListPtr pDataList, long nReportID, CReportInfo * pReport, BOOL bResetSelection);
long GetStatementType(long nReportID);
CString StmtConvertTo70Name(CString str60Name);
CString GetStatementFileName(long nReportID);
void SetCommonFileArray(CDWordArray *dwAry, long nReportID);

bool CheckCurrentUserReportAccess(long nReportID, BOOL bSilent, BOOL bForcePermissionCheck = FALSE);
// (j.jones 2008-07-01 12:35) - PLID 30501 - this function will check
// non-report-specific permissions needed for the given report ID, and
// it should NOT be called outside of CheckCurrentUserReportAccess without
// also calling CheckCurrentUserReportAccess
BOOL CheckIndivReportAccess(long nReportID);

void LoadAvailableReportsMap ();
void EnsureReportGroups();
void SetReportGroup(long nNewReportID, long nRepIDforGroup);

CString GetPaymentsExtendedSqlString(BOOL bIncludeAdjustments, BOOL bIncludeRefunds, BOOL bIncludeGCPayments);

// (s.dhole 2012-06-21 12:35) - PLID 49341 - this function will check Optical Shop reports
bool IsReportOpticalShopOnly(IN const CReportInfo *pReport);
    // (s.tullis 2014-07-10 13:47) - PLID 62560- Batch Vision Payments
	// (s.tullis 2014-07-14 13:34) - PLID 62559 - Batch Medical Payments
bool IsReportVisionPostingOnly(IN const CReportInfo *pReport);

// (a.walling 2011-08-05 12:04) - PLID 44902 - Added constructor to initialize dblMaxTax1 and dblMaxTax2
struct TotalsStruct
{
	TotalsStruct()
		: dblMaxTax1(0.0)
		, dblMaxTax2(0.0)
	{
	}

	COleCurrency cyChargeTotal;
	COleCurrency cyDiscountTotal;
	double dblMaxTax1;
	double dblMaxTax2;
	COleCurrency cyTax1Total;
	COleCurrency cyTax2Total;
	COleCurrency cyCashTotal;
	COleCurrency cyCheckTotal;
	COleCurrency cyCreditTotal;
	COleCurrency cyGCTotal;
	COleCurrency cyAdjTotal;
	COleCurrency cyRefundTotal;
	COleCurrency cyTipTotal;
	COleCurrency cyChangeGivenTotal;
	COleCurrency cyAmountReceivedTotal;
	// (j.gruber 2008-02-19 12:42) - PLID 28896 - added total to fix rounding issue
	COleCurrency cyChargeWithDiscountsTotal;
	// (j.jones 2009-11-11 12:42) - PLID 34165 - supported showing other applies
	COleCurrency cyOtherChargeApplies;
	COleCurrency cyOtherPaymentApplies;
};

// (j.gruber 2007-07-16 10:34) - PLID 26686 - Change printing to POS to be here
void PrintSalesReceiptToReceiptPrinter(CString strLineItemFilter, CString strPaymentFilter, CString strIDs, long nReportNumber );
//TES 12/6/2007 - PLID 28192 - Added the POSPrinter as a parameter to these functions as necessary, that way it can be 
// claimed once, then passed through to the various functions that need it.
BOOL PrintReceiptPaymentSummary(COPOSPrinterDevice* pPOSPrinter, TotalsStruct *pTotals, CString strCardHoldersName,long nCharCount);
BOOL GetSummaryDescription(COPOSPrinterDevice* pPOSPrinter, CString strWhite, CString &strDesc, CString strAmount, long nCharCount);
CString CalcLargestInfo(TotalsStruct *pTotals);
CString GetLargestString(CString str1, CString str2);
BOOL PrintReceiptDetail(COPOSPrinterDevice* pPOSPrinter, CString strLineItemFilter, CString strPaymentFilter, TotalsStruct *pTotals, CString &strCardHolderName, long nCharCount);
BOOL GetReceiptDescription(COPOSPrinterDevice* pPOSPrinter, CString strDate, CString &strDescription, CString strQuantity, CString strAmount, BOOL bShowParenthesis, long nCharCount);
BOOL PrintReceiptHeader(COPOSPrinterDevice* pPOSPrinter, CString strIDs, long nShowLogo, CString strLogoPath, CString strLocationFormat, long nFontNumber, long nCharCount);

// (j.gruber 2008-01-16 17:37) - PLID 28310 - functions/modified added for format printing
void ReplaceFormattedText(COPOSPrinterDevice* pPOSPrinter, CString strIDs, CString IN strText, CStringArray OUT *aryStr, long nFontNumber, long nCharCount);
void WordWrap(CStringArray *strText, long nLineWidth);
long GetTextLengthIgnoringFormatsWithDoubleFormatting(CString strText, BOOL &bLastDoubleWide);
CString RemoveAllFormats(CString strText);
CString FormatText(COPOSPrinterDevice* pPOSPrinter, CString strTextToFormat, BOOL bIsDoubleWide, long nLineChars);
long GetTextLengthIgnoringFormats(CString strText);
BOOL PrintReceiptFooter(COPOSPrinterDevice* pPOSPrinter, CString strIDs, CString strFooterMessage, long nFontNumber, long nCharCount);
BOOL IsFormat(CString strText, CString &strFormat, long &nFormatLength);


void RunSalesReceipt(COleDateTime dtPayDate, long nReportNum, long nPaymentID, BOOL bPreview, long nReportID, long nPatientID, CWnd *pWnd);
void RunSalesReceipt(COleDateTime dtPayDate, long nPaymentID, long nReportID, long nPatientID, CWnd *pWnd);

#endif