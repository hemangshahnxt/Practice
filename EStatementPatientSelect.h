#pragma once
// (j.dinatale 2011-03-21 14:52) - PLID 41444 - Created

// CEStatementPatientSelectDlg dialog
#include "ReportInfo.h"
#include "ComplexReportQuery.h"
#include "NxAdoLib\AdoEvents.h"

class CEStatementPatientSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEStatementPatientSelectDlg)

public:
	CEStatementPatientSelectDlg(CWnd* pParent);
	virtual ~CEStatementPatientSelectDlg();

	virtual BOOL OnInitDialog();
	// (j.dinatale 2011-03-30 11:53) - PLID 42982 - Dialog cannot be modal
	//virtual int DoModal();

	// (j.dinatale 2011-03-30 12:30) - PLID 42982 - need to be able to set the report to run, check if saved work exists
	//	in the dialog and also need to control when the patient list is reloaded
	// (c.haag 2016-05-19 14:18) - PLID-68687 - We now require the report SQL to generate the qualifying patient list
	void SetReport(CReportInfo *pReport, BOOL bSummary, const CComplexReportQuery& reportQuery);
	bool HasSavedWork();
	void ClearDialog(bool bClearCache = false); // (j.dinatale 2011-04-05 11:49) - PLID 42982 - renamed appropriately
	void ReloadPatientList();

	CString GenerateSelectedPersonIDList();
	CString GenerateSelectedPersonIDLocationIDList(); // (r.goldschmidt 2016-01-15 12:28) - PLID 67899

// Dialog Data
	enum { IDD = IDD_ESTATEMENT_PATIENTSELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_pUnselectedList;

	// (c.haag 2016-05-19 14:18) - PLID-68687 - The asynchronously populated recordset that is used to populate
	// the selected datalist with all qualifying patients
	ADODB::_ConnectionPtr m_pconasyncSelectedList;
	ADODB::_RecordsetPtr m_prsasyncSelectedList;
	bool m_bTempTableExists;
	// (c.haag 2016-05-19 14:18) - PLID-68687 - The event sink that notifies us of the status of m_prsasyncSelectedList
	class CSelectedListConnectionSink : public NxAdo::Events::Connection::Sink
	{
		STDMETHOD(raw_ExecuteComplete)(
			long RecordsAffected,
			struct ADODB::Error *pError,
			enum ADODB::EventStatusEnum *adStatus,
			struct ADODB::_Command *pCommand,
			struct ADODB::_Recordset *pRecordset,
			struct ADODB::_Connection *pConnection);

	public:
		CEStatementPatientSelectDlg* m_pdlg;
	};
	CSelectedListConnectionSink m_asyncSelectedListListener;
	// (c.haag 2016-05-19 14:18) - PLID-68687 - The temporary SQL table that stores the results of the report query.
	// This is used to populate the datalist and also the result set for passing into ProcessEStatements
	CString m_strTempReportResultsTable;

	// (j.dinatale 2011-03-30 14:29) - PLID 42982 - cache the unselected patients
	CArray<long, long> m_aryCachedUnselPatIDs;
	
	// (r.goldschmidt 2016-01-15 12:28) - PLID 67899 - cache unselected patient/location pairs for by location reports
	std::unordered_multimap<long, long> m_mapCachedUnselPatIDLocIDs;

	// (r.goldschmidt 2016-01-15 12:28) - PLID 67899 - boolean for if dialog is in estatement by patient or by patient by location
	bool m_bByLocation;

	// (j.dinatale 2011-03-30 12:31) - PLID 42982 - need to store info about the report the user desires to run
	// (a.walling 2013-08-13 10:43) - PLID 57998 - EStatementPatientSelectDlg needs to make a copy of the CReportInfo, otherwise the underlying CReportInfo and parameters can change while the dialog is still active.
	CReportInfo m_report;
	BOOL m_bSummary;
	// (c.haag 2016-05-19 14:18) - PLID-68687 - The report SQL from the call to SetReport which we use to build m_strTempReportResultsTable
	// and in turn the patient selection datalist and the result set for ProcessEStatements
	CComplexReportQuery m_reportQuery;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnUp;
	CNxIconButton m_btnAllUp;
	CNxIconButton m_btnDown;
	NxButton m_chkExportGroup;	// (j.dinatale 2011-04-04 15:25) - PLID 42983
	CNxStatic m_lblUnselCount;
	CNxStatic m_lblSelCount;

	// (j.dinatale 2011-04-04 15:25) - PLID 42983 - necessary to cause the letter writing module group list to refresh
	CTableChecker m_groupChecker;

	// (j.dinatale 2011-04-05 11:22) - PLID 41444 - used to determine if the information currently in the dialog is "saved" work
	bool m_bHasSavedWork;

	// (r.goldschmidt 2016-01-14 16:45) - PLID 67899 - e-statements may now be run by location
	CString GenerateLocationIDList(); 
	void EnsureControls();
	// (c.haag 2016-05-19 14:18) - PLID-68687 - Enables or disables all the controls
	void EnableControls(BOOL bEnable);

	void UpdateListCounts();

	// (j.dinatale 2011-04-04 10:50) - PLID 42983
	bool CreateGroup();

	// (j.dinatale 2011-03-30 12:30) - PLID 42982 - need to be able to rebuild our unselected ID catch
	void BuildUnselectedListCache();

	// (c.haag 2016-05-19 14:18) - PLID-68687 - Called when the asynchronous query finished running and the datalist is populated
	void OnDatalistPopulated();

	// (c.haag 2016-05-19 14:18) - PLID-68687 - Must be called to dismiss the dialog
	void CleanUpAsyncQueryObjectsAndHideWindow();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMovepatup();
	afx_msg void OnBnClickedMovepatdown();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellSelectedpatients(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellUnselectedpatients(LPDISPATCH lpRow, short nColIndex);
	void LeftClickPatient(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedMoveallpatup();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCreateunselgroup();
	afx_msg void OnBnClickedCancel();
};