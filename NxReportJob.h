#if !defined(AFX_NXREPORTJOB_H__F579B6F3_F60C_11D3_ADCC_00104B318376__INCLUDED_)
#define AFX_NXREPORTJOB_H__F579B6F3_F60C_11D3_ADCC_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxReportJob.h : header file
//
#include "peplus.h"

/////////////////////////////////////////////////////////////////////////////
// CNxReportJob window
class CReportInfo;

class CNxReportJob : public CRPEJob
{
// Construction
public:
	
	CNxReportJob (short jobHandle);
	CNxReportJob (short jobHandle, CRPEJob *parentJob);
	~CNxReportJob();

// Attributes
public:

// Operations
public:

// Overrides
	BOOL OutputToPrinter(short nCopies = 1, CPrintInfo* pInfo = 0);
//	void Close();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxReportJob)
	//}}AFX_VIRTUAL

// Implementation
public:
	// Sets all the logon infor for the report (recurses to this report's subreports)
	BOOL SetAllLogonInfo(CRPELogOnInfo *pLogonInfo);
	// Sets the data source of the report based on the given reportinfo object (recurses to this report's subreports)
	BOOL SetDataSource(const CReportInfo *pReport, long nSubLevel, long nSubRepNum, OPTIONAL OUT ADODB::_Recordset **ppRecordsetOut = NULL);
	BOOL SetDataSource(const CReportInfo *pReport, OPTIONAL OUT ADODB::_Recordset **ppRecordsetOut = NULL);
	
	// This function sets the report to use the correct print settings (printer, page orientation, etc.) according to the following rules (3 is an exception)
	//   1. If the pInfo DOES NOT have device settings already, fill the pInfo with a copy of the app's device settings
	//   2. Now that pInfo is guaranteed to have device settings, use a copy of pInfo's device settings for the rpt file (3 is an exception)
	//   3. If the RPT file was designed with the "Use Default" option UNCHECKED, use the following fiedls from rpt file's device settings:
	//      - page orientation
	//
	void GatherPrintInfo(IN OUT CPrintInfo *pInfo);
	
	// Sets the job's printoptions based on the given pInfo object
	// Returns the number of copies to be printed (based on the pInfo's devmode object)
	short ApplyPrintInfo(IN OUT CPrintInfo *pInfo);
	ADODB::_RecordsetPtr m_rsReportInfo;
	long m_nReportInfoID;

	// (j.gruber 2011-10-11 15:43) - PLID 45937 - bool to write to history
	BOOL m_bWriteToHistoryStatus;
	

//	virtual ~CNxReportJob();

	// Generated message map functions
protected:
	void AuditPrinting(); // (a.walling 2009-06-01 17:02) - PLID 34240

	//{{AFX_MSG(CNxReportJob)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	//DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXREPORTJOB_H__F579B6F3_F60C_11D3_ADCC_00104B318376__INCLUDED_)
