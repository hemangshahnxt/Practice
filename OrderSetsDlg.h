#pragma once


// COrderSetsDlg dialog
// (z.manning 2009-05-07 16:16) - PLID 28554 - Created

class COrderSetsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COrderSetsDlg)

public:
	COrderSetsDlg(const long nPatientID, CWnd* pParent);   // standard constructor
	virtual ~COrderSetsDlg();

// Dialog Data
	enum { IDD = IDD_ORDER_SETS };

protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlOrderSets;
	enum EOrderSetColumns {
		oscOrderSetID = 0,
		oscParentID,
		oscObjectID,
		oscType,
		oscDescription,
		oscInstructions,
		oscInputDate,
	};

	long m_nPatientID;

	BOOL m_bNeedToCloseDlg; // (c.haag 2010-07-16 10:55) - PLID 34338 - Because the main frame
	BOOL m_bHasLabDataChanged;	// now manages lab entry dialogs, we need to track whether we need to
												// close or not with a member variable, and track whether data changed
	

	BOOL IsOrderSetRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (z.manning 2009-05-14 15:56) - PLID 34269 - Utility functions to handle this. They return
	// true if the object is successfully added to the order set
	BOOL AddMedicationToOrderSet(const long nOrderSetID, const long nMedicationID);
	BOOL AddLabToOrderSet(const long nOrderSetID, const long nLabProcedureID, const CString &strToBeOrdered, OUT BOOL &bNeedToCloseDialog);
	BOOL AddReferralOrderToOrderSet(const long nOrderSetID, const long nRefPhysID, OUT BOOL &bNeedToCloseDialog);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CNxIconButton m_btnClose;
	CNxIconButton m_btnNewOrderSet;
	CNxColor m_nxcolor;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedNewOrderSet();
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedOrderSetList(short nFlags);
	void LeftClickOrderSetList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (z.manning 2009-05-14 15:38) - PLID 34269
	void RButtonDownOrderSetList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	LRESULT OnLabEntryDlgClosed(WPARAM wParam, LPARAM lParam);
};
