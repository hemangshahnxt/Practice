#if !defined(AFX_MEDICATIONHISTORYDLG_H__812F7C1B_EC43_4ac6_B6D7_70941EF0F8E4__INCLUDED_)
#define AFX_MEDICATIONHISTORYDLG_H__812F7C1B_EC43_4ac6_B6D7_70941EF0F8E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MedicicationHistoryDlg.h : header file
//

#include "PatientsRc.h"
#include "NxAPI.h"

/////////////////////////////////////////////////////////////////////////////
// CMedicationHistoryDlg dialog


// (r.gonet 08/05/2013) - PLID 58200 - Created to allow the user to request the medication history
// of a patient, view the resulting medications, and import them.
class CMedicationHistoryDlg : public CNxDialog
{
// Construction
public:
	// (r.gonet 08/09/2013) - PLID 58200 - Constructs the medication history dialog.
	// - pParent: Parent window
	// - nPatientID: The current patient. We make requests for this patient.
	CMedicationHistoryDlg(CWnd* pParent, long nPatientID);   // standard constructor
	// (r.gonet 08/09/2013) - PLID 58200 - Destructor.
	virtual ~CMedicationHistoryDlg();
	
// Dialog Data
protected:
	//{{AFX_DATA(CMedicationHistoryDlg)
	enum { IDD = IDD_MEDICATION_HISTORY_DLG };
	// (a.wilson 2013-10-02 14:52) - PLID 57844
	CNxStatic m_nxMedImportStatus;
	// (r.gonet 08/09/2013) - PLID 58200 - Background for the request creation controls.
	CNxColor m_nxcolorRequest;
	// (r.gonet 08/09/2013) - PLID 58200 - Label for the request creation group of controls.
	CNxStatic m_nxsRequestLabel;
	// (r.gonet 08/09/2013) - PLID 58200 - Button to commit the request and start the request session.
	CNxIconButton m_nxbRequestMedicationHistory;
	// (r.gonet 09/21/2013) - PLID 58200
	CNxStatic m_nxsLastRequestedLabel;
	// (r.gonet 09/21/2013) - PLID 57978
	NxButton m_checkSeeingPatientWithinThreeDays;
	// (r.gonet 09/21/2013) - PLID 57978
	CNxIconButton m_nxbViewEligibilityDetails;
	// (r.gonet 08/09/2013) - PLID 58200 - Label for the location combo.
	CNxStatic m_nxsLocation;
	// (r.gonet 08/09/2013) - PLID 58200 - Combo to select the location the request is being made from. Affects which Providers are available.
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationCombo;
	// (r.gonet 08/09/2013) - PLID 58200 - Label for the requesting provider combo.
	CNxStatic m_nxsRequestingProvider;
	// (r.gonet 08/09/2013) - PLID 58200 - Combo to select the provider who is requesting the medication history.
	NXDATALIST2Lib::_DNxDataListPtr m_pRequestingProviderCombo;
	// (r.gonet 08/09/2013) - PLID 58200 - Enables date filters on the  Last Fill Date. Request will only retrieve medications with Last fill date on or between these dates.
	NxButton m_checkRangeFrom;
	// (r.gonet 08/09/2013) - PLID 58200 - The lower bound date filter for the request. Filters on Last Fill Date.
	CDateTimePicker m_dtpFromDate;
	// (r.gonet 08/09/2013) - PLID 58200 - Label for the upper bound date filter for the request.
	CNxStatic m_nxsTo;
	// (r.gonet 08/09/2013) - PLID 58200 - The upper bound date filter for the request. Filters on Last Fill Date.
	CDateTimePicker m_dtpToDate;
	// (r.gonet 08/09/2013) - PLID 57979 - Checkbox to indicate that the patient has consented to medication history retrieval.
	// Requests cannot be made without consent. We default consent to be given to reduce clicks.
	NxButton m_checkPatientConsentObtained;
	// (r.gonet 08/09/2013) - PLID 57981 - The background for the request status list.
	CNxColor m_nxcolorRequestsStatuses;
	// (r.gonet 08/09/2013) - PLID 57981 - A label for the request status list
	CNxStatic m_nxsRequestsStatusesLabel;
	// (r.gonet 08/09/2013) - PLID 57981 - A datalist containing the current medication history requests and their
	// statuses. Although the user clicks the Request button once, many child requests can be made. The act
	// of clicking the Request button creates a Master Request, which contains all of the parameters that are
	// common to all of the child requests. Each child request is for an individual active coverage returned
	// by the eligibility request PLUS one for fill data that SureScripts has already. Each of these child requests
	// can result in a partial response of medications returned since they can have up to 50 meds per response.
	// This means that a sequence of requests must automatically be made to ensure we have all the meds for each
	// coverage plus the fill data. We hide the fact that we do these sequences from the user. They only see one request
	// per sequence in the request status list. If one request in the sequence fails, then the whole sequence fails and is reported
	// in this datalist.
	NXDATALIST2Lib::_DNxDataListPtr m_pMedicationHistoryRequestList;
	// (r.gonet 08/19/2013) - PLID 58200 - A edit box that contains the SureScripts mandated disclaimer in fine print.
	CNxEdit m_nxeditDisclaimer;
	// (r.gonet 08/09/2013) - PLID 58200 - The background for the response list.
	CNxColor m_nxcolorResponses;
	// (r.gonet 08/09/2013) - PLID 58200 - A label for the response list.
	CNxStatic m_nxsResponsesLabel;
	// (r.gonet 08/09/2013) - PLID 58200 - The medication history response list. This contains all historical medications
	// returned from a request, with duplicates removed.
	NXDATALIST2Lib::_DNxDataListPtr m_pMedicationHistoryList;
	// (r.gonet 08/09/2013) - PLID 58200 - Button to close the request dialog. Simply closes it.
	CNxIconButton m_nxbClose;
	// (a.wilson 2013-08-19 10:12) - PLID 58744
	CNxIconButton m_nxbImport;
	CNxIconButton m_nxbCheckAllImport;

	// (r.gonet 09/20/2013) - PLID 58200 - A small font for the disclaimer
	CFont *m_pFinePrint;
	// (r.gonet 08/09/2013) - PLID 57981 - Table checker for a concept that is not a table. Is raised when a request sequence 
	// has completed. We update the request status list when we get this.
	CTableChecker m_checkMedHistory_SequenceCompleted;
	// (r.gonet 09/20/2013) - PLID 57981 - Table checker for when the MedicationHistoryResponseT table is updated so we
	// can reload the response list.
	CTableChecker m_checkMedicationHistoryResponses;	
	// (r.gonet 08/08/2013) - PLID 58200 - The person ID of the patient we are working with at present.
	long m_nPatientID;
	// (r.gonet 09/20/2013) - PLID 57978 - The medication history master request we are working with currently.
	// The requests contained in the master list are displayed in the request status list. Will only be null
	// if we have made no previous requests for this patient.
	NexTech_Accessor::_RxHistoryMasterRequestPtr m_pCurrentMasterRequest;
	// (r.gonet 09/20/2013) - PLID 58200 - The color for the backgrounds. Should reflect the patient's status.
	OLE_COLOR m_nColor;
	
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMedicationHistoryDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (r.gonet 08/09/2013) - PLID 58200 - Column enum for the location combo.
	enum EMedicationHistoryLocationColumn
	{
		emhlcID = 0,
		emhlcName,
	};

	// (r.gonet 08/09/2013) - PLID 58200 - Column enum for the provider combo.
	enum EMedicationHistoryProviderColumn
	{
		emhpcID = 0,
		emhpcLast,
		emhpcFirst,
		emhpcMiddle,
		emhpcFullName,
	};

	// (r.gonet 08/09/2013) - PLID 57981 - Column enum for the request list.
	enum ERequestsListColumns
	{
		erlcID = 0,
		erlcSource,
		erlcFromDate,
		erlcToDate,
		erlcStatusID,
		erlcStatusText,
		erlcFullErrorMessage,
	};

	enum EERxMedicationInfoType
	{
		emitPrescribed = 0,
		emitDespensed,
	};

	// Generated message map functions
	//{{AFX_MSG(CMedicationHistoryDlg)
	// (r.gonet 08/05/2013) - PLID 58200 - Initializes the dialog control values.
	virtual BOOL OnInitDialog();
	// (r.gonet 08/05/2013) - PLID 57978 - Handler for the Request Med History button. Validates control
	// values and starts a request from SureScripts.
	afx_msg void OnBnClickedMedHistoryRequestBtn();
	// (r.gonet 08/05/2013) - PLID 58200 - Handler for clicking the last fill date range checkbox.
	afx_msg void OnBnClickedMedHistoryFromDateCheck();
	// (r.gonet 10/03/2013) - PLID 57978 - Handler for viewing eligibility
	afx_msg void OnBnClickedMedHistoryViewEligibilityDetailsBtn();
	// (r.gonet 08/05/2013) - PLID 57979 - Handler for clicking the consent checkbox. Saves the consent value to the database.
	afx_msg void OnBnClickedMedHistoryConsentCheck();
	// (r.gonet 08/09/2013) - PLID 57981 - Handler for when the user clicks on a cell in the request list. 
	// Display any error message when the user clicks on the status text column when it is an error.
	void LeftClickMedHistoryRequestsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (r.gonet 08/05/2013) - PLID 58200 - Handler for when the location combo box finishes querying. Starts the load
	// of the provider combo box.
	afx_msg void OnRequeryFinishedLocationCombo(short nFlags);
	// (r.gonet 08/05/2013) - PLID 57981 - Clean up any resources upon destruction of the window.	
	afx_msg void OnDestroy();
	// (r.gonet 08/05/2013) - PLID 58200 - Handler for when the user selects a row in the location combo but before the selection is committed.
	void SelChangingMedHistoryLocationCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (r.gonet 08/09/2013) - PLID 58200 - Handler for when the user selects a row from the location combo. 
	// Load the provider combo when we make a selection in the location combo.
	void SelChosenLocationCombo(LPDISPATCH lpRow);
	// (r.gonet 08/05/2013) - PLID 58200 - Handler for when the user selects a row in the provider combo but before the selection is committed.
	void SelChangingMedHistoryReqProviderCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (r.gonet 08/05/2013) - PLID 57981 - Handler for table checker messages.
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void PopulateMedicationHistoryList(NexTech_Accessor::_PatientRxHistoryArrayPtr pRxHistoryResults);  //r.wilson 
	// (r.gonet 08/09/2013) - PLID 57981 - Handle timeouts.
	afx_msg void OnTimer(UINT nIDEvent);

	// (r.gonet 08/05/2013) - PLID 58200 - Ensure every dialog control is enabled/disabled that should 
	// be enabled/disabled.
	void EnsureControls();
	// (r.gonet 08/09/2013) - PLID 57981 - Start waiting for request status updates.
	void StartWaitingForRequestStatusUpdates();
	// (r.gonet 08/09/2013) - PLID 57981 - Stop waiting for request status updates
	void StopWaitingForRequestStatusUpdates();
	// (r.gonet 08/09/2013) - PLID 57981 - Returns true if all of the requests in the current request session have either succeeded or failed.
	bool AreAllRequestsComplete();
	// (r.gonet 09/21/2013) - PLID 57981 - Loads the request list with the medication history requests from
	// the most recent master request for the patient.
	void LoadLastMasterRequest();
	// (r.gonet 08/09/2013) - PLID 57981 - Syncs the request list with the request statuses in the database.
	void ReloadRequestList(bool bRequery = true);
	// (r.gonet 10/24/2013) - PLID 59104 - Checks for failures in the eligibility response. Sets the appropriate controls.
	void CheckEligibilityResponse();
	void CheckForRxHistoryTruncation(long nPatientID, long nProviderID, long nLocationID);
	void CheckForTruncation(ADODB::FieldsPtr pFields, CString strFieldName, int nMaxLength, CString strFriendlyName, IN OUT CString &strDisplayMessage);
	// (r.gonet 08/09/2013) - PLID 58200 - Load the provider combo. Once something has been chosen in 
	// the location combo, we can pull the registered prescribers for that location and put them into 
	// the provider combo.
	void LoadProviderCombo();
	void UpdateDisplayedMedicationCount();

public:
	// (r.gonet 08/05/2013) - PLID 58200 - Set the NxColor background color.
	void SetColor(OLE_COLOR nColor);
	//r.wilson 
	void LoadResponseList();
	void ShowPopupForClickedRow(NXDATALIST2Lib::IRowSettingsPtr pRow); //(r.wilson 8/6/2013) -
	void AddRowToMedHistoryDatalist(NexTech_Accessor::_PatientRxHistoryPtr pRxHistory, NexTech_Accessor::_ERxMedicationInfoPtr pMedInfo, EERxMedicationInfoType eMedInfoType, long &nGeneratedKey, long nMasterRequestID ,BOOL bCheckIfPrevIsDuplicate);	
	CString FormatName(const _variant_t& varValue);

	class MedHistoryRowInfo
	{
	public:
		long m_nMasterRequestID;
		long m_nEligibilityResponseDetailID;
		NexTech_Accessor::_PatientRxHistoryPtr pPatientRxHistory;
		NexTech_Accessor::_ERxMedicationInfoPtr pMedicationInfo;

		EERxMedicationInfoType eMedicationInfoType;

		MedHistoryRowInfo(NexTech_Accessor::_PatientRxHistoryPtr _pPatientRxHistory, NexTech_Accessor::_ERxMedicationInfoPtr _pMedicationInfo, long nEligibilityResponseDetailID, EERxMedicationInfoType _eMedicationInfoType)
		{
			pPatientRxHistory = _pPatientRxHistory;
			pMedicationInfo = _pMedicationInfo;
			m_nEligibilityResponseDetailID = nEligibilityResponseDetailID;
			eMedicationInfoType = _eMedicationInfoType;
		}

	};

	std::map<long, shared_ptr<MedHistoryRowInfo>> m_mapMedHistoryInfoRows;
	void DblClickCellMedHistoryList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedImportMedicationBtn();
	afx_msg void OnBnClickedCheckAllMedicationsBtn();
	BOOL MedsEqual(shared_ptr<MedHistoryRowInfo> pMedInfo1, shared_ptr<MedHistoryRowInfo> pMedInfo2);	
	void RemoveDuplicateMedications();
	afx_msg void OnNMClickHistoryMedicationImportResults(NMHDR *pNMHDR, LRESULT *pResult);
	// (a.wilson 2013-08-30 14:31) - PLID 57844
	CString m_strImportFailedPopupText;
	bool m_bImportedHistoryMeds;
//	void ColumnClickingMedHistoryList(short nCol, BOOL* bAllowSort);
	void LeftClickMedHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (r.gonet 12/20/2013) - PLID 57844
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDICATIONHISTORYDLG_H__812F7C1B_EC43_4ac6_B6D7_70941EF0F8E4__INCLUDED_)