#if !defined(AFX_OPPORTUNITYEDITDLG_H__9EC9EED7_AC5A_4568_A58E_491517361466__INCLUDED_)
#define AFX_OPPORTUNITYEDITDLG_H__9EC9EED7_AC5A_4568_A58E_491517361466__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpportunityEditDlg.h : header file
//
/*
//For saved arrays
struct CAssociate {
	long nID;
	CString strName;
};

struct CPartner {
	long nID;
	CString strName;
};

struct CCompetitor {
	long nID;
	CString strName;
};
*/

// (z.manning, 12/11/2007) - PLID 27972 - Global function for determing if given type ID is an add-on.
BOOL IsAddOn(long nTypeID);

/////////////////////////////////////////////////////////////////////////////
// COpportunityEditDlg dialog

class COpportunityEditDlg : public CNxDialog
{
// Construction
public:
	COpportunityEditDlg(CWnd* pParent);   // standard constructor

	void SetID(long nIDToLoad);

// Dialog Data
	//{{AFX_DATA(COpportunityEditDlg)
	enum { IDD = IDD_OPPORTUNITY_EDIT_DLG };
	CNxIconButton	m_btnPrevious;
	CNxIconButton	m_btnNext;
	CNxIconButton	m_btnEmail;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnAddProposal;
	CNxLabel	m_nxlCompetitionLabel;
	CNxLabel	m_nxlAssociateLabel;
	CNxLabel	m_nxlAllianceLabel;
	NxButton	m_btnDiscussedHardware;
	NxButton	m_btnITPersonKnown;
	CNxEdit	m_nxeditOppName;
	CNxEdit	m_nxeditPriceEstimate;
	CNxEdit	m_nxeditQbEstimate;
	CNxEdit	m_nxeditOppNotes;
	CNxEdit	m_nxeditLostReason;
	CNxEdit	m_nxeditItPersonName;
	CNxEdit	m_nxeditItNotes;
	CNxStatic	m_nxstaticLostReasonLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpportunityEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//Functionality
	bool ApplyChanges();
	void EnsureAssociates();
	void EnsurePartners();
	void EnsureCompetitors();
	void OnAssociateLabelClick();
	void OnAllianceLabelClick();
	void OnCompetitionLabelClick();
	void EnsureLostReason();
	long GetActiveProposalID();
	void EnsureActiveProposal();
	void EnsureType();				//DRT 4/3/2008 - PLID 29493
	bool IsFinancing();				// (d.thompson 2009-08-27) - PLID 35365


	//Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pClient;
	NXDATALIST2Lib::_DNxDataListPtr m_pType;
	NXDATALIST2Lib::_DNxDataListPtr m_pPriCoord;
	NXDATALIST2Lib::_DNxDataListPtr m_pSecCoord;
	NXDATALIST2Lib::_DNxDataListPtr m_pAssociate;
	//NXDATALIST2Lib::_DNxDataListPtr m_pCategory;	// (d.lange 2010-11-11 10:07) - PLID 41442 - Removed Category dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_pStage;
	NXDATALIST2Lib::_DNxDataListPtr m_pCompetition;
	NXDATALIST2Lib::_DNxDataListPtr m_pAlliance;
	NXDATALIST2Lib::_DNxDataListPtr m_pProposalList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPayMethod; // (z.manning, 11/05/2007) - PLID 27972
	NXDATALIST2Lib::_DNxDataListPtr m_pITContact;	// (d.lange 2010-11-09 09:52) - PLID 41335 - Added IT Contact list
	CDateTimePicker m_pickerCloseDate;

	//Member data
	long m_nID;				//ID of the current opportunity, -1 if unsaved
	bool m_bChanged;		//Set to true when the dialog has some data changed and needs saved

	// (z.manning, 11/06/2007) - PLID 27972 - This variable tells us if the user has modified the
	// opportunity name field at all.
	BOOL m_bUserModifiedOppName;

	//These variables correspond to our last-saved state.  Typically this is what is loaded 
	//	from data, but if the user does a save mid-dialog, then these will be updated.
	long m_nSavedClientID;
	long m_nSavedTypeID;
	CString m_strSavedName;
	long m_nSavedPriCoordID;
	long m_nSavedSecCoordID;
	CDWordArray m_arySavedAssociates;
	COleDateTime m_dtSavedCloseDate;
	long m_nSavedCategoryID;
	long m_nSavedStageID;
	CDWordArray m_arySavedCompetition;
	CDWordArray m_arySavedAlliance;
	CString m_strSavedNotes;
	CString m_strSavedLostReason;
	COleCurrency m_cySavedEstPrice;
	CString m_strSavedQBEstimate;
	long m_nSavedPayMethodID;
	BOOL m_bSavedDiscussedHardware;
	//BOOL m_bSavedITPersonKnown;			// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
	//CString m_strSavedITPersonName;		// (d.lange 2010-11-09 12:25) - PLID 41335 - Removed this control
	CString m_strSavedITNotes;

	//These variables correspond to current on-screen data.  These should be always in sync
	//	with what the interface displays.  Not every field needs a variable, if the interface
	//	is "simple" enough to keep it (single selects, text boxes, etc).
	CDWordArray m_aryCurrentAssociates;
	CDWordArray m_aryCurrentCompetition;
	CDWordArray m_aryCurrentAlliance;



	// Generated message map functions
	//{{AFX_MSG(COpportunityEditDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddProposal();
	afx_msg void OnOppEmailSummary();
	afx_msg void OnOppPrevious();
	afx_msg void OnOppNext();
	afx_msg LRESULT OnLoadOpportunity(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblClickCellOppProposalList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRequeryFinishedOppProposalList(short nFlags);
	afx_msg void OnSelChosenOppAssociate(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOppCompetition(LPDISPATCH lpRow);
	afx_msg void OnSelChosenOppAlliance(LPDISPATCH lpRow);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelChosenOppStage(LPDISPATCH lpRow);
	afx_msg void OnKillfocusPriceEstimate();
	afx_msg void OnRButtonDownOppProposalList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedOppClient(short nFlags);
	afx_msg void OnChangeOppName();
	afx_msg void OnSelChosenOppClient(LPDISPATCH lpRow);
	//afx_msg void OnITPersonKnown();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnGotocontact();
	void OnRequeryFinishedITContactList(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPPORTUNITYEDITDLG_H__9EC9EED7_AC5A_4568_A58E_491517361466__INCLUDED_)
