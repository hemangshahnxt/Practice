#if !defined(AFX_ELIGIBILITYREQUESTDETAILDLG_H__36207008_AD03_4B8E_A61C_8A56094D5B62__INCLUDED_)
#define AFX_ELIGIBILITYREQUESTDETAILDLG_H__36207008_AD03_4B8E_A61C_8A56094D5B62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EligibilityRequestDetailDlg.h : header file
//

// (j.jones 2007-06-19 17:37) - PLID 26387 - created the EligibilityRequestDetailDlg

#include "FinancialRc.h"

/////////////////////////////////////////////////////////////////////////////
// CEligibilityRequestDetailDlg dialog

class CEligibilityRequestDetailDlg : public CNxDialog
{
// Construction
public:
	CEligibilityRequestDetailDlg(CWnd* pParent);   // standard constructor

	//this dialog is called with one or more request IDs,
	//this list will not change during the lifespan of this dialog
	std::vector<long> m_aryAllRequestIDs;

	//this dialog can occasionally be called to only show certain
	//responses, mainly when a large batch of responses have
	//just been imported
	//this list will not change during the lifespan of this dialog
	std::vector<long> m_aryAllResponseIDs;

	// (r.goldschmidt 2014-10-13 15:33) - PLID 62644 - reorganize some controls because dialog is now modeless
	void EnsureControls();

	// (j.jones 2008-05-07 15:34) - PLID 29854 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CEligibilityRequestDetailDlg)
	enum { IDD = IDD_ELIGIBILITY_REQUEST_DETAIL_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRebatch;
	CNxIconButton	m_btnEdit;
	// (j.jones 2010-03-25 10:19) - PLID 37832 - added arrow buttons and response labels
	CNxIconButton	m_btnLeftResponse;
	CNxIconButton	m_btnRightResponse;
	CNxStatic		m_nxstaticResponseLabel;
	CNxStatic		m_nxstaticResponseCountLabel;
	// (j.jones 2010-03-26 15:50) - PLID 37619 - added ability to filter the details
	NxButton		m_checkFilterDetails;
	CNxIconButton	m_btnConfigureFiltering;
	CNxStatic		m_nxstaticServiceTypeFilterLabel;
	CNxStatic		m_nxstaticCoverageLevelFilterLabel;
	CNxStatic		m_nxstaticBenefitTypeFilterLabel;
	// (j.jones 2010-07-07 09:39) - PLID 39534 - supported showing multiple requests on this dialog
	CNxIconButton	m_btnLeftRequest;
	CNxIconButton	m_btnRightRequest;
	CNxIconButton	m_btnMultiRequestInfo;
	// (b.spivey, June 04, 2012) - PLID 48696 - Added this for the print preview icon. 
	CNxIconButton	m_btnPrintPreview; 
	// (b.eyers 2015-04-17) - PLID 44309
	CNxIconButton	m_btnGoToPatient;
	// (j.jones 2016-05-12 11:18) - NX-100625 - added labels for patient name & payers
	CNxStatic		m_nxstaticPatientName;
	CNxStatic		m_nxstaticInsuredParties;
	// (j.jones 2016-05-19 16:39) - NX-100357 - added other payors label
	CNxStatic		m_nxstaticOtherPayorsLabel;
	//controls for filtering by response type
	CNxStatic		m_nxstaticFilterResponseTypeLabel;
	NxButton		m_radioResponseTypeAll;
	NxButton		m_radioResponseTypeActive;
	NxButton		m_radioResponseTypeInactive;
	NxButton		m_radioResponseTypeFailed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEligibilityRequestDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_RequestInfoList;
	// (j.jones 2010-03-25 09:11) - PLID 37832 - split out a response info list, and added a list of all details
	NXDATALIST2Lib::_DNxDataListPtr m_ResponseInfoList;
	NXDATALIST2Lib::_DNxDataListPtr m_DetailList;
	// (j.jones 2010-03-26 17:32) - PLID 37619 - added ability to filter the details
	NXDATALIST2Lib::_DNxDataListPtr m_ServiceTypeFilterCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_CoverageLevelFilterCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_BenefitTypeFilterCombo;
	// (j.jones 2016-05-16 10:10) - NX-100357 - added Other Payors list
	NXDATALIST2Lib::_DNxDataListPtr m_OtherPayorsList;
	//the list of pay groups, copays, coinsurance
	NXDATALIST2Lib::_DNxDataListPtr m_PayGroupsList;
	//the list of the deductible total/remaining and out of pocket total/remaining
	NXDATALIST2Lib::_DNxDataListPtr m_DedOOPList;

	// (j.jones 2010-03-25 12:20) - PLID 37832 - cache the locations of the initial
	// placements of both response lists, as we dynamically re-size them later
	long m_nDetailListInitialTop;
	long m_nDetailListInitialBottom;
	long m_nResponseListInitialTop;
	long m_nResponseListInitialBottom;

	// (j.jones 2010-03-25 10:17) - PLID 37832 - changed the Load() function
	// into FullReload() which loads the request & count of responses,
	// and LoadResponse() which loads a specific response
	void FullReload(long nInitialResponseIndex = 0);
	// (j.jones 2010-03-29 09:37) - PLID 37619 - added bFiltersChanged which will
	// determine whether the load is due to manually changing the filters
	void LoadResponse(long nResponseID, BOOL bManualFiltersChanged = FALSE);

	// (j.jones 2010-03-25 09:11) - PLID 37832 - split into request & response lists
	void AddNewRequestRow(CString strText, COLORREF cTextColor = RGB(0,0,0));
	void AddNewResponseRow(CString strText, COLORREF cTextColor = RGB(0,0,0));

	// (j.jones 2010-03-25 10:19) - PLID 37832 - added UpdateResponseControls,
	// which will hide/move/size/update the response controls based on the current
	// response information
	void UpdateResponseControls();

	// (j.jones 2010-07-07 09:49) - PLID 39534 - now we accept an array of request IDs,
	// and thus need to track the index we are on
	long m_nCurRequestIndex;
	// (j.jones 2010-07-07 09:55) - PLID 39534 - gets the request ID at m_nCurRequestIndex
	long GetRequestID();

	// (j.jones 2010-07-07 10:03) - PLID 39534 - updates the request buttons with proper text, enabling/disabling
	void UpdateRequestButtons();

	//this is the list of requests currently filtered,
	//such as 'active coverage', 'failed', etc.
	std::vector<long> m_aryCurFilteredRequestIDs;

	// (j.jones 2010-03-25 10:19) - PLID 37832 - added an array of response IDs,
	// and the current reponse ID
	std::vector<long> m_aryCurResponseIDs;

	long m_nCurResponseIndex;
	// (b.eyers 2015-04-17) - PLID 44309
	long m_nCurrentRequestPatientID;
	CString m_strCurrentRequestPatientName;
	long m_nCurrentRequestInsuredPartyID;
	// (b.spivey, March 30, 2012) - PLID 48696 - We need this to run the report. 
	long GetResponseID(); 
	// (b.spivey, June 05, 2012) - PLID 48696 - Needed to make this a member variable so that when I run the report 
	//		I don't get a prompt for this parameter field. 
	CString m_strProvName;

	// (j.jones 2010-03-26 16:00) - PLID 37619 - with filtering in place,
	// we can't assume an empty list means no details exist, so track that info.
	BOOL m_bCurResponseHasDetails;

	// (j.jones 2010-03-26 16:14) - PLID 37619 - cache whether there are default filters
	BOOL m_bHasDefaultFilters;

	// (r.goldschmidt 2014-10-10 14:34) - PLID 62644 - Refresh EEligibility Tab if it is the active sheet; refresh eligibility review dlg if it is present
	void RefreshParent();

	//if m_aryAllResponseIDs is filled, this will return an AND clause filtering
	//on EligibilityResponsesT to filter the results only by the desired reponse IDs
	CSqlFragment GetResponseFilterWhere();

	//called after filtering on Active, Inactive, or Failed responses,	
	//takes in a SQL fragment to filter (via joins) on the proper responses,
	//and a string of the reponse type for use in messaging
	void ApplyRadioButtonResponseTypeFilter(CSqlFragment sqlCoverageTypeJoin, CString strCoverageTypeName);

	//used to reset the response filter to "all" after a current
	//radio button check is complete
	afx_msg LRESULT OnResetResponseTypeFilterToAll(WPARAM wParam, LPARAM lParam);

	// Generated message map functions
	//{{AFX_MSG(CEligibilityRequestDetailDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnEditEligRequest();
	afx_msg void OnBtnRebatchEligRequest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	// (j.jones 2010-03-25 10:17) - PLID 37832 - added ability to traverse responses when multiple exist
	afx_msg void OnEligibilityPrevResponse();
	afx_msg void OnEligibilityNextResponse();
	// (j.jones 2010-03-26 15:50) - PLID 37619 - added ability to filter the details
	afx_msg void OnCheckFilterDetails();
	afx_msg void OnBtnConfigResponseFiltering();	
	void OnSelChosenEligServicetypeCombo(LPDISPATCH lpRow);
	void OnSelChosenEligCoveragelevelCombo(LPDISPATCH lpRow);
	void OnSelChosenEligBenefittypeCombo(LPDISPATCH lpRow);
	// (j.jones 2010-07-07 09:39) - PLID 39534 - supported showing multiple requests on this dialog
	afx_msg void OnEligibilityPrevReq();
	afx_msg void OnEligibilityNextReq();
	// (b.spivey, March 30, 2012) - PLID 48696 - Print preview
	afx_msg void OnBnClickedBtnPrintPreview();
	// (b.eyers 2015-04-17) - PLID 44309
	afx_msg void OnBtnGoToPatient();
	//handlers for response type radio button filters
	afx_msg void OnRadioResponseTypeAll();
	afx_msg void OnRadioResponseTypeActive();
	afx_msg void OnRadioResponseTypeInactive();
	afx_msg void OnRadioResponseTypeFailed();
	void OnEditingStartingEligPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingEligPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedEligPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void OnEditingFinishingEligDedoopList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedEligDedoopList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ELIGIBILITYREQUESTDETAILDLG_H__36207008_AD03_4B8E_A61C_8A56094D5B62__INCLUDED_)
