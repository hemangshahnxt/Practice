#pragma once

// (j.dinatale 2010-12-13) - PLID 41777
#include <SentinelValues.h>
#include "LabResultsAttachmentView.h"

// CLabResultsTabDlg dialog
//TES 11/20/2009 - PLID 36191 - Created.  The vast majority of code was moved here from LabEntryDlg.h

#define IDT_DISABLE_BROWSER	1001
//TES 2/24/2012 - PLID 44841 - Moved the LabResultField enum to GlobalLabUtils
enum LabResultField;

struct stResults {
	stResults() {
		// (a.walling 2010-02-25 16:17) - PLID 37546 - I don't like uninitialized variables.
		nResultID = -1;
		dtReceivedDate.SetStatus(COleDateTime::null);
		dtPerformedDate.SetStatus(COleDateTime::null);
		nFlagID = -1;
		nMailID = -1;
		nStatusID = -1;
		nAcknowledgedUserID = -1;
		dtAcknowledgedDate.SetStatus(COleDateTime::null);
		pRow = NULL;
		dtReceivedByLab.SetStatus(COleDateTime::null);
		dtSpecimenStartTime.SetStatus(COleDateTime::null);
		dtSpecimenEndTime.SetStatus(COleDateTime::null);
		dtObservationDate.SetStatus(COleDateTime::null);
	};

	long nResultID;
	CString strResultName;
	//TES 12/1/2008 - PLID 32281 - It now stores the date received as a COleDateTime, not a CString.
	COleDateTime dtReceivedDate;
	COleDateTime dtPerformedDate; // (a.walling 2010-02-25 15:48) - PLID 37546
	CString strLOINC; // (a.walling 2010-01-18 10:18) - PLID 36936
	CString strSlideNum;
	CString strDiagnosis;
	CString strMicroDesc;
	long nFlagID;
	CString strFlag;
	CString strValue;
	CString strUnits; // (c.haag 2009-05-06 15:02) - PLID 33789
	CString strReference;
	long nMailID;
	CString strDocPath;
	long nStatusID;		//TES 12/1/2008 - PLID 32191
	CString strStatus;	//TES 12/1/2008 - PLID 32191
	CString strComments; // (z.manning 2009-04-30 16:53) - PLID 28560
	// (c.haag 2009-05-07 14:34) - PLID 28561 - Acknowledged fields
	long nAcknowledgedUserID;
	COleDateTime dtAcknowledgedDate;
	COleDateTime dtReceivedByLab;	//TES 4/28/2011 - PLID 43426
	//TES 12/7/2009 - PLID 36191 - Need to track which row (parent row for the result) this is associated with.
	LPDISPATCH pRow;
	// (d.singleton 2013-06-20 15:14) - PLID 57937 - Update HL7 lab messages to support latest MU requirements.
	CString strSpecimenID;
	CString strSpecimenIdText;
	COleDateTime dtSpecimenStartTime;
	COleDateTime dtSpecimenEndTime;
	CString strRejectReason;
	CString strCondition;
	// (d.singleton 2013-07-16 17:09) - PLID 57600 - show CollectionStartTime and CollectionEndTime in the html view of lab results
	COleDateTime dtServiceStartTime;
	COleDateTime dtServiceEndTime;
	// (d.singleton 2013-08-07 16:01) - PLID 57912 - need to show the Performing Provider on report view of labresult tab dlg
	CString strPerformingProvider;
	// (d.singleton 2013-10-25 12:22) - PLID 59181 - need new option to pull performing lab from obx23 and show on the html view
	CString strPerformingLab;
	CString strPerfLabAddress, strPerfLabCity, strPerfLabState, strPerfLabZip, strPerfLabCountry, strPerfLabParish;	
	// (d.singleton 2013-11-04 12:14) - PLID 59294 - add observation date to the html view for lab results. 
	COleDateTime dtObservationDate;	
};


// (b.spivey, July 11, 2013) - PLID 45194 - Struct to pass the dialog and the row we selected off to the TWAIN Callback function. 
struct DialogRowStruct {
	CLabResultsTabDlg* pDialog;
	NXDATALIST2Lib::IRowSettingsPtr pRow;
};

// (k.messina 2011-11-11) - PLID 41438 - use this to set the results view
enum ResultsView 
{
	rvNotSet = -1, // (j.dinatale 2010-12-14) - PLID 41438 - indicates the view has not been set yet
	rvPDF = 0, 
	rvNexTechReport, 
	rvDiscreteValues,
};

//TES 11/22/2009 - PLID 36191 - Columns in the tree of lab results on the lft.
enum LabResultTreeColumns{
	lrtcResultID = 0, //TES 11/22/2009 - PLID 36191 - The ID of the result (-1 for new results)
	lrtcFieldID = 1, //TES 11/22/2009 - PLID 36191 - A LabResultField enum identifying the field represented by this row.
	lrtcFieldName = 2, //TES 11/22/2009 - PLID 36191 - The user-friendly name corresponding to the FieldID
	lrtcValue = 3, //TES 11/22/2009 - PLID 36191 - The actual (display) value of this field.
	lrtcForeignKeyID = 4, //TES 11/22/2009 - PLID 36191 - For items such as Flag, stores the ID of the record (i.e., FlagID).
	lrtcSpecimen = 5, //TES 11/30/2009 - PLID 36452 - Valid only for Lab-type rows, stores the "Specimen" field for this requisition (VT_EMPTY on all other rows)
	lrtcHL7MessageID = 6, // (z.manning 2010-05-12 10:12) - PLID 37400 - The HL7 message ID of the result if applicable	
	// (c.haag 2010-12-02 09:53) - PLID 38633 - Enumerations for completed columns
	lrtcCompletedBy = 7,
	lrtcCompletedDate = 8,
	lrtcCompletedUsername = 9,
	// (c.haag 2010-11-18 17:27) - PLID 37372 - Enumerations for signature columns
	lrtcSignatureImageFile = 10,
	lrtcSignatureInkData = 11,
	lrtcSignatureTextData = 12,
	// (c.haag 2010-12-02 09:53) - PLID 38633 - Enumerations for completed columns
	lrtcSavedCompletedBy = 13,
	lrtcSavedCompletedDate = 14,
	// (c.haag 2010-11-23 10:46) - PLID 37372 - Enumerations for new signature columns
	lrtcSignedBy = 15,
	lrtcSignedDate = 16,
	lrtcSignedUsername = 17,
	lrtcSavedSignedBy = 18,
	lrtcSavedSignedDate = 19,
	// (c.haag 2011-01-27) - PLID 41618 - Generic date (used in attachment rows as attach date)
	lrtcDate = 20,
	// (b.spivey, April 09, 2013) - PLID 44387 - Column that tells us if a lab started as incomplete.
	lrtcLoadedAsIncomplete = 21, 
	lrtcExtraValue = 22, //TES 8/6/2013 - PLID 51147 - Used to store LabResultFlagsT.Priority, feel free to overload for other fields
							//TES 9/10/2013 - PLID 58511 - I overloaded it for top-level lab and result rows, to indicate whether the record was replaced
	lrtcSpecimenIDText = 23,
	lrtcSpecimenCollectionStartTime = 24,
	lrtcSpecimenCollectionEndTime = 25,
	lrtcSpecimenRejectReason = 26,
	lrtcSpecimenCondition = 27,
};


enum LabType;
class CLabEntryDlg;

class CLabResultsTabDlg : public CNxDialog
{
	friend CLabResultsAttachmentView;
	DECLARE_DYNAMIC(CLabResultsTabDlg)

public:
	CLabResultsTabDlg(CLabEntryDlg *pParentDlg);   // standard constructor
	virtual ~CLabResultsTabDlg();

	// (z.manning 2011-06-21 17:04) - PLID 44154 - Simple struct for tracking lab signature, completion, or acknowledge data.
	struct LabCompletionInfo
	{
		long nUserID;
		COleDateTime dtDate;

		bool operator==(LabCompletionInfo& source)
		{
			if(this->nUserID == source.nUserID && this->dtDate == source.dtDate) {
				return true;
			}
			return false;
		}
	};

	class CLabCompletionInfoArray : public CArray<LabCompletionInfo,LabCompletionInfo&>
	{
	public:
		BOOL AlreadyExists(LabCompletionInfo info)
		{
			for(int nIndex = 0; nIndex < GetCount(); nIndex++)
			{
				if(info == GetAt(nIndex)) {
					return TRUE;
				}
			}
			return FALSE;
		}
	};

private:
	CLabResultsAttachmentView* m_pLabResultsAttachmentView;

private:
	// (c.haag 2010-11-18 16:08) - PLID 37372 - Returns the lab row given a lab ID
	NXDATALIST2Lib::IRowSettingsPtr GetLabRowByID(const long nLabID) const;
	// (c.haag 2010-12-10 13:53) - PLID 37372 - Given an arbitrary row, this function will try to get the parent lab row, and
	// then the result row out of it if one and only one exists.
	NXDATALIST2Lib::IRowSettingsPtr GetFirstAndOnlyLabResultRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

private:
	bool m_bControlsHidden; // (c.haag 2011-12-28) - PLID 41618 - Set to true if the controls in the
							// discrete values view are hidden

public:
	// (j.gruber 2010-11-30 09:14) - PLID 41606
	void LoadHTMLReport(NXDATALIST2Lib::IRowSettingsPtr pSpecRow);
	
	// (j.gruber 2010-11-30 09:14) - PLID 41653
	CString GetResultHTML(NXDATALIST2Lib::IRowSettingsPtr pResultRow);
	CString GetStyle();
	CString GetFooterHTML();
	// (j.gruber 2010-12-08 11:50) - PLID 41759
	CString GetHeaderHTML(NXDATALIST2Lib::IRowSettingsPtr pSpecRow);
	
	// (j.gruber 2010-11-30 09:14) - PLID 41606
	CString GetHTMLFileName(long nLabID);
	NXDATALIST2Lib::IRowSettingsPtr GetSpecFromCurrentRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	BOOL IsVerticalScrollBarVisible();
	CStringArray m_straryLabReports;	

	// (j.gruber 2010-11-24 11:27) - PLID 41607 - scroll buttons
	void UpdateScrollButtons();
	void RefreshHTMLReportOnlyControls(BOOL bOnReportView);

	// (j.gruber 2010-11-30 09:14) - PLID 41606
	void ReloadCurrentPDF();
	void UnloadHTMLReports();

	// (c.haag 2011-02-22) - PLID 41618 - This function should be called when changing to
	// either the report or discrete values view. The attachments view has its own internal
	// way of handling these, and this function should never be called when the attachments
	// view is visible.
	void RefreshSharedControls(NXDATALIST2Lib::IRowSettingsPtr pActiveRow);
	// (c.haag 2011-02-22) - PLID 41618 - All detach/filename button toggles are now centralized here
	void UpdateDetachButton();
	// (c.haag 2011-02-22) - PLID 41618 - All zoom button toggles are now centralized here
	void UpdateZoomButton();

	// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if there is at least one result and all the results are completed
	// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
	// because it may come in handy someday and may even help developers better understand the lab code.
	//BOOL IsLabCompleted(long nLabID, OUT long& nCompletedBy, OUT COleDateTime& dtCompletedDate);
	// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if this lab has any unsigned results
	// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
	// because it may come in handy someday and may even help developers better understand the lab code.
	//BOOL LabHasUnsignedResults(long nLabID);

private:
	// (c.haag 2010-12-10 10:19) - PLID 40556 - Returns TRUE if all the results for a specific lab are acknowledged
	BOOL AllResultsAreAcknowledged(long nLabID);
	// (c.haag 2010-11-23 16:17) - PLID 37372 - Returns TRUE if all the results for a specific lab are signed
	BOOL AllResultsAreSigned(long nLabID);
	// (c.haag 2010-12-02 09:59) - PLID 38633 - Returns TRUE if all the results for a specific lab are marked completed
	BOOL AllResultsAreCompleted(long nLabID);
	// (c.haag 2010-11-22 15:39) - PLID 40556 - Formats the Acknowledged button text based on the number
	// of results and the state of the results.
	// (c.haag 2011-02-21) - PLID 41618 - Now takes in a row. pActiveRow can be a result or a specimen or null.
	void FormatAcknowledgedButtonText(NXDATALIST2Lib::IRowSettingsPtr pActiveRow);
	// (c.haag 2010-11-23 16:17) - PLID 37372 - Formats the Signature button based on the current requisition and
	// whether it has been signed
	// (c.haag 2011-02-21) - PLID 41618 - We now pass in the active row. It can be a result, or a specimen, or null.
	void FormatSignButtonText(NXDATALIST2Lib::IRowSettingsPtr pActiveRow);
	// (c.haag 2010-11-23 16:17) - PLID 38633 - Formats the Completed button based on the current requisition and
	// whether it has been completed
	// (c.haag 2011-02-21) - PLID 41618 - Now takes in a row. pActiveRow can be a result or a specimen or null.
	void FormatMarkCompletedButtonText(NXDATALIST2Lib::IRowSettingsPtr pActiveRow);
	// (c.haag 2010-11-22 13:49) - PLID 37372 - Views a signature given a result row
	// (j.luckoski 2013-03-21 10:39) - PLID 55424 - Format button to enable or not.
	void FormatMarkAllCompleteButtonText(NXDATALIST2Lib::IRowSettingsPtr pActiveRow);
	void ViewSignature(NXDATALIST2Lib::IRowSettingsPtr& pResultRow);
	// (c.haag 2010-12-01 11:03) - PLID 37372 - Populates apResults with all the result rows for a specified lthat have no signatures
	void GetUnsignedResults(long nLabID, CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr>& apResults);
	// (c.haag 2010-12-02 09:59) - PLID 38633 - Populates apResults with all the result rows that are not marked completed
	// but have been signed
	void GetIncompleteSignedResults(long nLabID, CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr>& apResults);

	// (z.manning 2011-06-22 09:43) - PLID 44154
	void GetUnacknowledgedResults(long nLabID, CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResults);

	// (z.manning 2011-06-21 17:09) - PLID 44154
	void GetUniqueSignatures(IN const CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResultRows, OUT CLabCompletionInfoArray &aryLabSignatures);
	void GetUniqueCompletionInfo(IN const CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResultRows, OUT CLabCompletionInfoArray &aryCompletionInfo);
	void GetUniqueAcknowledgers(IN const CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResultRows, OUT CLabCompletionInfoArray &arystrAcknowledgers);

	// (z.manning 2011-06-22 10:39) - PLID 44154
	CString GetResultRowText(NXDATALIST2Lib::IRowSettingsPtr pResultRow);
	// (f.gelderloos 2013-08-28 16:23) - PLID 57826
	void FormatAcknowledgeAndSignButtonText(NXDATALIST2Lib::IRowSettingsPtr pActiveRow);

	// (d.singleton 2013-10-29 12:08) - PLID 59197 - need to move the specimen segment values to its own header in the html view for labs,  so it will show regardless of any results. 
	CString GetSpecimenFieldsHTML(NXDATALIST2Lib::IRowSettingsPtr pSpecRow);

private:
	// (c.haag 2011-01-27) - PLID 41618 - Returns the time on the server
	COleDateTime GetServerTime();

	//TES 2/24/2012 - PLID 44841 - Variables to store the configuration for the Report View
	bool m_bLoadedResultFieldPositions;
	struct ResultFieldPosition {
		LabResultField lrf;
		long nColumn;
		long nRow;
		long nActualColumn;
		//TES 3/27/2012 - PLID 49209 - Added bShowField
		BOOL bShowField;
		ResultFieldPosition() {
			lrf = (LabResultField)-1;
			nColumn = nRow = nActualColumn = -1;
			bShowField = TRUE;
		}
	};
	CArray<ResultFieldPosition,ResultFieldPosition&> m_arResultFieldPositions;
	//TES 2/24/2012 - PLID 44841 - Determines the actual number of columns that will be displayed in the report view, based on the configuration
	// as well as the field values for the current result.
	long GetColumnCount();
	//TES 2/24/2012 - PLID 44841 - Split out the code to calculate the width of the browser window into its own function.
	long GetBrowserWidth();
	//TES 2/24/2012 - PLID 44841 - Fills m_arResultFieldPositions from ConfigRT, if necessary.
	void EnsureResultFieldPositions();

	//TES 2/24/2012 - PLID 44841 - New function to get the HTML for a single result field.  Takes in the row, a ResultFieldPosition that specifies
	// both the field and its position in the report, and the column width, and outputs whether or not it actually filled any information.
	CString GetResultFieldHTML(NXDATALIST2Lib::IRowSettingsPtr pResultRow, const ResultFieldPosition &rfp, bool &bFilled, long nColumnWidth);

public:
	//TES 11/20/2009 - PLID 36191 - Data passed in from CLabEntryDlg.
	void SetPatientID(long nPatientID);
	//TES 11/30/2009 - PLID 36452 - We now set just the initial lab ID, as this tab may have multiple labs on it.
	void SetInitialLabID(long nLabID);
	void SetLabProcedureID(long nLabProcedureID);
	void SetLabProcedureType(LabType ltType);

	// (b.savon 2012-02-28 17:06) - PLID 48443 - Addded some Accessors for Recall Creation
	long GetPatientID(){ return m_nPatientID; }
	long GetInitialLabID(){ return m_nInitialLabID; }

	//TES 11/20/2009 - PLID 36191 - Load a new lab
	void LoadNew();
	//TES 11/20/2009 - PLID 36191 - Load an existing lab (SetLabID() needs to have been called).
	void LoadExisting();

	//TES 11/20/2009 - PLID 36191 - Call to do any processing that needs to be done after data has been loaded.
	void PostLoad();

	//TES 11/20/2009 - PLID 36191 - Call to reset to a "New Lab" state.
	//void SetNew();
	//TES 11/30/2009 - PLID 36452 - Instead of SetNew(), we now call AddNew(), which keeps all our existing data and just adds a new requisition.
	void AddNew();

	BOOL CheckResultSave(BOOL bSilent);
	
	//TES 11/20/2009 - PLID 36191 - Save all current results.
	//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
	void Save(long nNewLabID, long &nAuditTransactionID, BOOL &bSpawnToDo, IN OUT CDWordArray &arNewCDSInterventions);

	void SetCurrentDate(_variant_t varCurDate);

	//TES 11/22/2009 - PLID 36191 - Adds a new result to the tree.
	void AddResult();

	//TES 11/22/2009 - PLID 36191 - Pulls the value from the tree that corresponds to the result represented by pRow, the field represented by lrf,
	// and the column represented by lrtc
	_variant_t GetTreeValue(NXDATALIST2Lib::IRowSettingsPtr pRow, LabResultField lrf, LabResultTreeColumns lrtc);
	//TES 11/22/2009 - PLID 36191 - Puts the given value into the tree, using the given column, and the row given by lrf that is for the same result
	// as pRow is for.
	void SetTreeValue(NXDATALIST2Lib::IRowSettingsPtr pRow, LabResultField lrf, LabResultTreeColumns lrtc, _variant_t varValue);

	//TES 11/30/2009 - PLID 36452 - We'll need to update our lab descriptors whenever a form number or specimen changes.
	void HandleSpecimenChange(long nLabID, const CString &strSpecimen);
	void HandleFormNumberChange(const CString &strFormNumber);

	// (c.haag 2010-11-18 17:35) - PLID 37372 - Returns the current lab ID
	long GetCurrentLab() const;
	//TES 11/30/2009 - PLID 36452 - Tells us to reflect the lab identified by nLabID (may be -1, which is fine, as there can only be one
	// new lab at a time).
	void SetCurrentLab(long nLabID);

	// (z.manning 2010-05-13 11:35) - PLID 37405 - Returns the currently active lab req dialog
	CLabRequisitionDlg* GetActiveLabRequisitionDlg();

	// (k.messina 2011-11-11) - PLID 41438
	void SetCurrentResultsView(int nResultsView);

	// (k.messina 2011-11-11) - PLID 41438
	// (c.haag 2011-02-22) - PLID 42589 - Renamed to UpdateControlStates and we now take in the current selection
	void UpdateControlStates(bool bHideControls, NXDATALIST2Lib::IRowSettingsPtr pCurTreeSel);

	// (c.haag 2010-11-22 15:39) - PLID 40556 - Returns the number of results for the current lab
	// (z.manning 2011-06-17 10:36) - PLID 44154 - This now returns all the result rows
	//TES 11/6/2012 - PLID 53591 - Provide options to return rows for all specimens, only the current specimen, or check the preference
	enum GetResultOptions {
		groAllSpecimens,
		groCurrentSpecimen,
		groCheckPreference,
	};
	void GetResults(long nLabID, OUT CArray<NXDATALIST2Lib::IRowSettingsPtr,NXDATALIST2Lib::IRowSettingsPtr> &arypResultRows, GetResultOptions gro);

	// (c.haag 2011-02-23) - PLID 41618 - Deprecated these functions. We should not be updating
	// static labels by themselves at random parts in code; they should be updated together with
	// the buttons in our member Format... functions.
	/*
	// (j.dinatale 2010-12-10) - PLID 41777 - functions to set the labels accordingly
	void SetCurrentResultSigned(long nUserID = -1, const COleDateTime& dtDate = g_cdtInvalid);
	void SetCurrentResultCompleted(long nUserID = -1, const COleDateTime& dtDate = g_cdtInvalid);
	// (c.haag 2010-01-06) - PLID 40556 - This function now only services to update the label
	void SetCurrentResultAcknowledged(long nUserID, const COleDateTime& dtDate);
	*/

	// (j.dinatale 2010-12-13) - PLID 41438 - Utility functions to configure the view
	void SetUpDiscreteView();
	void SetUpReportView();
	void SetUpPDFView();

	// (j.dinatale 2010-12-14) - PLID 41438 - need to maintain the current view to work around the z-order issue with the web browser activex control
	long m_nCurrentView;

	// (c.haag 2010-11-22 13:49) - PLID 38633 - Added m_signatureBtn
	// (c.haag 2010-11-22 15:39) - PLID 40556 - Added m_nxstaticAcknowledgedBy
// Dialog Data
	enum { IDD = IDD_LAB_RESULTS_TAB_DLG };
	CNxStatic	m_nxstaticDateReceived;
	CNxLabel	m_nxlDocPath;
	CNxStatic	m_nxstaticValueLabel;
	CNxStatic	m_nxstaticUnitsLabel;
	CNxStatic	m_nxstaticSlideLabel;
	CNxStatic	m_nxstaticNameLabel;
	CNxLabel	m_nxlabelLOINC; // (a.walling 2010-01-18 10:38) - PLID 36936
	CNxStatic	m_nxstaticRefLabel;
	CNxStatic	m_nxstaticResultCommentsLabel;
	CNxStatic	m_nxstaticMicroDesLabel;
	CNxStatic	m_nxstaticFlagLabel;
	CNxStatic	m_nxstaticDiagnosisLabel;
	CNxIconButton   m_btnAttachDoc;
	CNxIconButton	m_btnDetachDoc;
	CNxIconButton	m_btnDeleteResult;
	CNxIconButton	m_btnAddResult;
	CNxIconButton   m_btnAddDiagnosis;
	CNxIconButton   m_markCompletedBtn;
	CNxIconButton	m_markAllCompleteBtn; // (j.luckoski 2013-03-21 10:39) - PLID 55424
	CNxIconButton	m_AcknowledgeAndSignBtn; // (f.gelderloos 2013-08-26 10:57) - PLID 57826
	CNxIconButton   m_signatureBtn;
	CNxStatic	m_nxstaticCompletedByLabel;
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the completed date heading label
	//CNxStatic	m_nxstaticCompletedDateLabel; // (c.haag 2010-12-02 16:23) - PLID 38633
	CNxStatic	m_nxstaticLabCompletedBy; // (c.haag 2010-11-22 14:26) - PLID 40556 - Now a label
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the completed date label
	//CNxStatic	m_nxstaticLabCompletedDate; // (c.haag 2010-12-02 16:23) - PLID 38633
	CNxStatic	m_nxstaticSignedByLabel; // (c.haag 2010-11-23 11:19) - PLID 37372
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the signed date heading label
	//CNxStatic	m_nxstaticSignedDateLabel; // (c.haag 2010-12-01 9:17) - PLID 37372
	CNxStatic	m_nxstaticLabSignedBy; // (c.haag 2010-11-23 11:19) - PLID 37372
	// (j.dinatale 2010-12-13) - PLID 41777 - No longer need the signed date label
	//CNxStatic	m_nxstaticLabSignedDate; // (c.haag 2010-12-01 9:17) - PLID 37372
	CDateTimePicker	m_dtpDateReceived;
	CNxEdit	m_nxeditSlideNumber;
	CNxEdit	m_nxeditLabDiagDescription;
	CNxEdit	m_nxeditClinicalDiagnosisDesc;
	CNxEdit m_nxeditResultValue;
	CNxEdit	m_nxeditResultUnits;
	CNxEdit m_nxeditResultComments;
	CNxEdit m_nxeditResultReference;
	CNxEdit m_nxeditResultName;
	CNxEdit m_nxeditResultLOINC; // (a.walling 2010-01-18 10:38) - PLID 36936
	CNxStatic	m_nxstaticLabResultLabel;
	CNxIconButton m_EditFlagBtn;
	CNxIconButton m_EditLabDiagnosesBtn;
	CNxIconButton m_EditClinicalDiagnosisList;	
	CNxIconButton m_nxbEditStatus;
	CNxIconButton m_nxbtnAcknowledgeResult;
	CNxStatic m_nxstaticAttachedFileLabel;
	CNxIconButton m_btnZoom;
	CNxStatic m_nxstaticAcknowledgedBy;
	CNxStatic m_nxstaticAcknowledgedByLabel;
	CNxIconButton m_btnScrollLeft; // (j.gruber 2010-11-24 11:29) - PLID 41607
	CNxIconButton m_btnScrollRight; // (j.gruber 2010-11-24 11:29) - PLID 41607
	CNxIconButton m_btnResultScrollLeft; // (c.haag 2010-12-06 15:55) - PLID 41618
	CNxIconButton m_btnResultScrollRight; // (c.haag 2010-12-06 15:55) - PLID 41618
	CNxIconButton m_btnNotes;	// (j.dinatale 2010-12-22) - PLID 41591
	CNxIconButton m_btnAddNote;	// (j.dinatale 2010-12-22) - PLID 41591
	CNxStatic m_nxsOrderStatusLabel;
	CNxIconButton m_btnEditOrderStatus;
	
	CString m_strResultUnits;	// (c.haag 2009-05-06 15:18) - PLID 33789 - For Units text limit

	// (b.spivey, August 23, 2013) - PLID 46295 - Show anatomical location on the discrete/report views.
	CNxStatic m_nxstaticAnatomicalLocationTextDiscrete;
	CNxStatic m_nxstaticAnatomicalLocationTextReport;

	// (j.jones 2013-10-18 11:32) - PLID 58979 - added infobutton features
	CNxIconButton m_btnPatientEducation;
	CNxLabel m_nxlabelPatientEducation;

protected:

	//TES 11/20/2009 - PLID 36191 - We need to have access to our parent.
	CLabEntryDlg *m_pLabEntryDlg;

	long m_nPatientID;
	//TES 11/30/2009 - PLID 36452 - We now set just the initial lab ID, as this tab may have multiple labs on it.
	long m_nInitialLabID;
	long m_nLabProcedureID;
	LabType m_ltType;

	// (j.jones 2007-07-19 14:13) - PLID 26751 - removed datalists for diagnosis description and microscopic description
	NXDATALIST2Lib::_DNxDataListPtr m_pFlagCombo,
					m_pStatusCombo, //TES 11/28/2008 - PLID 32191
					m_pClinicalDiagOptionsList, //TES 11/10/2009 - PLID 36128
					m_pOrderStatusCombo; //TES 5/2/2011 - PLID 43428

	//TES 11/23/2009 - PLID 36192 - A web browser to display attached files (.pdf, particularly).
	IWebBrowser2Ptr m_pBrowser;	
	
	//TES 2/3/2010 - PLID 37191 - Track which file we're currently previewing, so we can open it if requested.
	CString m_strCurrentFileName;

	//TES 11/23/2009 - PLID 36192 - A map of files attached to given parent rows.
	//TES 1/27/2010 - PLID 36862 - Changed to a map of maps; mapping requisition-level rows to maps of result-level rows.
	CMap<LPDISPATCH,LPDISPATCH, CMap<LPDISPATCH,LPDISPATCH,CString,CString&>*, CMap<LPDISPATCH,LPDISPATCH,CString,CString&>*> m_mapAttachedFiles;

	//TES 1/27/2010 - PLID 36862 - Remember that the given file is attached to the given result.
	void SetAttachedFile(NXDATALIST2Lib::IRowSettingsPtr pResultRow, CString strFile);
	//TES 1/27/2010 - PLID 36862 - Get all result/file pairs for the requisition corresponding to the given result.
	BOOL GetAttachedFiles(NXDATALIST2Lib::IRowSettingsPtr pResultRow, OUT CMap<LPDISPATCH,LPDISPATCH,CString,CString&>* &pMap);

	// (z.manning 2010-11-09 15:05) - PLID 41395 - This function will display the only attached file in the
	// entire lab (regardless of specimen) if there's exactly one file attached.
	// (c.haag 2011-01-24) - PLID 41618 - Rather than trying to display the only attached file, we now try
	// to display the oldest attached file.
	void TryDisplayOldestAttachedFile();

	// (z.manning 2010-11-09 17:17) - PLID 41395
	// (c.haag 2010-12-28 13:41) - PLID 41618 - Also get the MailID and result row
	void GetAllAttachedFiles(OUT CArray<CAttachedLabFile,CAttachedLabFile&> &aAttachedFiles);
	// (c.haag 2010-12-28 13:41) - PLID 41618 - Returns all attached files sorted by attachment date ascending
	// The first entry in this array is the oldest attachment.
	void GetAllAttachedFilesSorted(OUT CArray<CAttachedLabFile,CAttachedLabFile&> &aAttachedFiles);

	//TES 11/22/2009 - PLID 36191 - Our new, tree-style list of results
	NXDATALIST2Lib::_DNxDataListPtr m_pResultsTree;
	void LoadResult(NXDATALIST2Lib::IRowSettingsPtr pRow);
	BOOL SaveResult(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void HandleDateReceivedChange();
	CMap<long, long, stResults*, stResults*> m_mapResults;
	// (c.haag 2010-01-27) - PLID 41618 - We now return a CAttachedLabFile object that has values for
	// the mail ID and attachment, and take in an attachment date
	CAttachedLabFile AttachFileToLab(CString strSourcePath, const COleDateTime& dtAttached);

	//TES 11/30/2009 - PLID 36452 - Utilities to get a particular type of row that matches an arbitary passed-in row.
	NXDATALIST2Lib::IRowSettingsPtr GetResultRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	NXDATALIST2Lib::IRowSettingsPtr GetLabRow(NXDATALIST2Lib::IRowSettingsPtr pRow) const;

	// (z.manning 2010-05-12 10:42) - PLID 37400 - Returns a new row for the result tree with all values initialized to null
	NXDATALIST2Lib::IRowSettingsPtr GetNewResultsTreeRow();

	// (z.manning 2010-05-12 11:08) - PLID 37400
	void EnableResultWnd(BOOL bEnable, CWnd *pwnd);

	BOOL m_bResultIsSaved;

	CDWordArray m_aryDeleteResults;
	CDWordArray m_aryDetachedDocs;

	//TES 5/2/2011 - PLID 43428 - The original Order Status, for auditing
	CString m_strSavedOrderStatus;

	// (c.haag 2009-05-07 16:29) - PLID 28561 - Acknowledged values for the current lab result displayed
	// on the screen
	// (c.haag 2011-01-06) - PLID 40556 - These are now stored in the tree
	//_variant_t m_vCurrentResultAcknowledgedUserID;
	//_variant_t m_vCurrentResultAcknowledgedDate;	
	
	_variant_t m_varCurDate;

	// (j.dinatale 2010-12-22) - PLID 41591 - functions to show the notes dialog
	void ShowNotesDlgForResult(NXDATALIST2Lib::IRowSettingsPtr pResultRow, bool bAutoAddNewNote);
	void ShowNotesDlgForLab(NXDATALIST2Lib::IRowSettingsPtr pLabRow, bool bAutoAddNewNote);

	// (j.dinatale 2010-12-22) - PLID 41591 - formats the notes button properly
	void FormatNotesButtonsForResult(NXDATALIST2Lib::IRowSettingsPtr pResultRow);
	void FormatNotesButtonsForLab(NXDATALIST2Lib::IRowSettingsPtr pLabRow);
	void FormatNotesButtons(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void DisableNotesButtons();

	// (j.dinatale 2010-12-23) - PLID 41591 - utility function to set up the notes dialog accordingly
	void SetUpNotesDlg(bool bAutoAddNewNote = false);

	// (j.dinatale 2010-12-22) - PLID 41591 - function to determine the result row an attachment is a part of
	NXDATALIST2Lib::IRowSettingsPtr GetResultRowFromAttachment(CString strFileName);

protected:
	// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for detaching files.
	void DetachFile(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for zooming
	void ZoomAttachment(const CString& strFilename);
	// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for acknowleding results. pActiveRow can be a result or a specimen or null
	// (j.luckoski 2013-03-20 11:40) - PLID 55424 - Added bool to determine if we need to pick from dialog or just do all
	void AcknowledgeResults(NXDATALIST2Lib::IRowSettingsPtr pResultRow,bool bIsMarkAll = false );
	// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for signing results. pActiveRow can be a result
	// or a specimen or null.
	// (j.luckoski 2013-03-20 11:40) - PLID 55424 - Added bool to determine if we need to pick from dialog or just do all
	void SignResults(NXDATALIST2Lib::IRowSettingsPtr pActiveRowbool, bool bIsMarkAll = false );
	// (c.haag 2011-02-22) - PLID 41618 - Self-contained function for completing results
	// (j.luckoski 2013-03-20 11:40) - PLID 55424 - Added bool to determine if we need to pick from dialog or just do all
	void MarkLabCompleted(NXDATALIST2Lib::IRowSettingsPtr pResultRow,bool bIsMarkAll = false  );

protected:
	// (c.haag 2011-02-22) - PLID 41618 - Returns attachment information for a given row
	CAttachedLabFile GetAttachedLabFile(LPDISPATCH lpRow);

	// (j.dinatale 2013-03-04 12:37) - PLID 34339 - functions to attach a file
	void AttachNewFile(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IRowSettingsPtr pParent);
	void AttachExistingFile(NXDATALIST2Lib::IRowSettingsPtr pRow, NXDATALIST2Lib::IRowSettingsPtr pParent);

	// (b.spivey, April 05, 2013) - PLID 44387 - Function to get all lab ids in an array, and check if a lab has results.
	void GetAllLabIDsAry(OUT CArray<long, long>& aryLabIDs);
	bool DoesLabHaveResults(long nLabID); 
	// (b.spivey, July 17, 2013) - PLID 45194 - Hold a member variable to this so we don't have a memory leak. 
	DialogRowStruct* m_drsThis;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void OnRequeryFinishedLabResults(short nFlags);
	afx_msg void OnAddDiagnosis();
	void OnSelChosenClinicalDiagnosisOptionsList(LPDISPATCH lpRow);
	CString  CLabResultsTabDlg::GetDocumentName(long  nMailID); // (s.dhole 2010-10-21 15:57) - PLID 36938 get new file name from database
	afx_msg void OnResult();
	afx_msg void OnEditFlag();
	afx_msg void OnEditLabDiagnoses();
	afx_msg void OnEditClinicalDiagnosisList();
	void OnTrySetSelFinishedLabFlag(long nRowEnum, long nFlags);
	afx_msg void OnAddResult();
	afx_msg void OnDeleteResult();
	void OnSelChangedLabResults(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDateTimeChangedDateReceived(NMHDR *pNMHDR, LRESULT *pResult);
	void OnSelChosenLabFlag(LPDISPATCH lpRow);
	void OnRequeryFinishedLabFlag(short nFlags);
	afx_msg void OnAttachFile();
	afx_msg void OnDetachFile();
	void OnSelChangingLabResults(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnRButtonDownLabResults(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnRequeryFinishedLabStatus(short nFlags);
	afx_msg void OnEditResultStatus();
	void OnSelChosenLabStatus(LPDISPATCH lpRow);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	void OnSelChangedResultsTree(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnSelChangingResultsTree(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnRButtonDownResultsTree(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnFileDownloadPdfPreview(BOOL ActiveDocument, BOOL* Cancel);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnZoom();
	afx_msg void OnSize(UINT nType, int cx, int cy); // (z.manning 2010-04-29 12:39) - PLID 38420
	// (b.spivey, August 23, 2013) - PLID 46295 - Get the anatomical location as shown from the labs tab. 
	CString GetAnatomicalLocationAsString(long nLabID); 
	void EnsureData();	// (b.spivey, August 23, 2013) - PLID 46295 - ensure the text is correct. 
	afx_msg void OnBnClickedPdfviewRadio(); // (k.messina 2011-11-11) - PLID 41438
	afx_msg void OnBnClickedReportviewRadio(); // (k.messina 2011-11-11) - PLID 41438
	afx_msg void OnBnClickedDiscretevaluesRadio(); // (k.messina 2011-11-11) - PLID 41438
	afx_msg void OnLabMarkCompleted();
	afx_msg void OnSignature();
	afx_msg void OnAcknowledgeResult();
	// (j.gruber 2010-11-24 11:27) - PLID 41607 - scroll buttons
	afx_msg void OnBnClickedScrollLeft();
	afx_msg void OnBnClickedScrollRight();
	afx_msg void OnBnClickedResultScrollLeft();
	afx_msg void OnBnClickedResultScrollRight();
	afx_msg void OnViewNotes(); // (j.dinatale 2010-12-22) - PLID 41591
	afx_msg void OnAddNote();	// (j.dinatale 2010-12-23) - PLID 41591
	afx_msg void OnEditOrderStatus();
	afx_msg void OnDestroy(); // (a.walling 2011-06-22 11:59) - PLID 44260
	afx_msg void OnConfigureReportView();
	afx_msg void OnBnClickedLabMarkAllComplete(); // (j.luckoski 2013-03-21 10:40) - PLID 55424
	// (b.spivey, July 10, 2013) - PLID 45194
	void DoTWAINAcquisition(NXTWAINlib::EScanTargetFormat eScanTargetFormat);
	void CLabResultsTabDlg::AttachFileToResult(CString strFilePath, NXDATALIST2Lib::IRowSettingsPtr pRow);
	static void WINAPI CALLBACK OnTWAINCallback(
		NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
		const CString& strFileName, /* The full filename of the document that was scanned */
		BOOL& bAttach,
		void* pUserData,
		CxImage& cxImage);
	afx_msg void OnBnClickedLabAcknowledgeandsign();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus); // (b.spivey, September 05, 2013) - PLID 46295
	// (j.jones 2013-10-18 11:32) - PLID 58979 - added infobutton
	afx_msg void OnBtnPtEducation();
	// (r.gonet 06/12/2014) - PLID 40426 - Clears the m_mapResults map.
	void ClearResultsMap();
	// (r.gonet 06/12/2014) - PLID 40426 - Clears the m_mapAttachedFiles map.
	void ClearAttachedFilesMap();
	// (r.goldschmidt 2016-02-18 18:03) - PLID 68267 - Filter the microscopic description list for any linked descriptions that are linked with the final lab diagnoses that the user chose
	void FilterClinicalDiagnosisOptionsList(const std::set<long>& arIDs);
	// (r.goldschmidt 2016-02-18 18:03) - PLID 68268 - Unfilter the list
	void RemoveFilterClinicalDiagnosisOptionsList();
	// (r.goldschmidt 2016-02-19 10:37) - PLID 68266 - add diagnosis to clinical diagnosis linking 
	void AddLabClinicalDiagnosisLink(const std::set<long>& arDiagnosisIDs, long nClinicalDiagnosisID);
public:
	void RequeryFinishedClinicalDiagnosisOptionsList(short nFlags);
};