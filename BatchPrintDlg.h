// BatchPrintDlg.h : header file
//
#include "progressbar.h"
#include "HCFAdlg.h"

#if !defined(AFX_BATCHPRINTDLG_H__1E890A49_AB20_11D2_9890_00104B318376__INCLUDED_)
#define AFX_BATCHPRINTDLG_H__1E890A49_AB20_11D2_9890_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

enum EPaperValidationResults {
	epvrSuccess = 0,
	epvrFailedError,
	epvrFailedOther,
};

/////////////////////////////////////////////////////////////////////////////
// CBatchPrintDlg dialog

class CBatchPrintDlg : public CNxDialog
{
// Construction
public:

	HANDLE m_hIconCheck, m_hIconRedX, m_hIconGrayX, m_hIconQuestion;

	void ShowErrors();
	// (j.jones 2007-03-01 08:59) - PLID 25015 - changed Validate to return an enum
	EPaperValidationResults Validate(long HCFAID);	

	CPrintDialog *m_printDlg;

	long m_CurrList; //0 = Unselected, 1 = Selected
	NXDATALISTLib::_DNxDataListPtr m_pList;
	NXDATALISTLib::_DNxDataListPtr m_FormTypeCombo;
	NXDATALISTLib::_DNxDataListPtr m_LocationCombo;

	NXDATALISTLib::_DNxDataListPtr m_UnselectedList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedList;
	CBatchPrintDlg(CWnd* pParent);	// standard constructor
	~CBatchPrintDlg(); // desctructor
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	void RefreshTotals();
	void UpdateCurrentSelect(int iList);

	void CalcShowBatchedChargeCount();

// Dialog Data
	//{{AFX_DATA(CBatchPrintDlg)
	enum { IDD = IDD_BATCH_PRINT_DLG };
	CNxIconButton	m_btnConfigureClaimValidation;
	CNxIconButton	m_btnValidateSelected;
	CNxIconButton	m_btnValidateAll;
	CNxIconButton	m_btnValidateUnselected;
	CNxIconButton	m_btnPrintUnselected;
	CNxIconButton	m_btnPrintSelected;
	CNxIconButton	m_btnPrintBatch;
	CNxIconButton	m_btnPrintAll;
	CNxIconButton	m_btnResetUnselected;
	CNxIconButton	m_btnResetSelected;
	CNxIconButton	m_btnResetAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectOne;
	int		m_LineCount;
	CNxStatic	m_nxstaticUnselectedLabel;
	CNxStatic	m_nxstaticPaperUnselectedTotal;
	CNxStatic	m_nxstaticSelectedLabel;
	CNxStatic	m_nxstaticPaperSelectedTotal;
	// (j.jones 2010-07-23 16:58) - PLID 34105 - added warning label & button for assignment of benefits
	CNxStatic	m_nxstaticAssignmentOfBenefitsWarningLabel;
	CNxIconButton	m_btnAssignmentOfBenefitsWarning;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBatchPrintDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CBatchPrintDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRemoveHCFA();
	afx_msg void OnPrintList();
	afx_msg void OnGoToPatient();
	afx_msg void OnOpenHCFA();
	afx_msg void OnValidateOne();
	afx_msg void SendToEBill();
	afx_msg void OnPrintBatch();
	afx_msg void OnReset();
	afx_msg void OnRButtonDownPaperSelectedBatchlist(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellPaperSelectedBatchlist(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellPaperUnselectedBatchlist(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonDownPaperUnselectedBatchlist(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedPaperSelectedBatchlist(short nFlags);
	afx_msg void OnRequeryFinishedPaperUnselectedBatchlist(short nFlags);
	afx_msg void OnSelectOneHcfa();
	afx_msg void OnSelectAllHcfas();
	afx_msg void OnUnselectOneHcfa();
	afx_msg void OnUnselectAllHcfas();
	afx_msg void OnPrintSelectedList();
	afx_msg void OnResetSelectedPaper();
	afx_msg void OnResetUnselectedPaper();
	afx_msg void OnPrintUnselectedPaper();
	afx_msg void OnSelChosenFormTypeCombo(long nRow);
	afx_msg void OnSelectInsCo();
	afx_msg void OnSelectInsGroup();
	afx_msg void OnSelChosenLocationComboPaper(long nRow);
	afx_msg void OnValidateUnselectedPaperClaims();
	afx_msg void OnValidateAllPaperClaims();
	afx_msg void OnValidateSelectedPaperClaims();
	afx_msg void OnBtnConfigureClaimValidation();
	afx_msg void OnSendListToEBill();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2010-07-23 16:58) - PLID 34105 - only shown if assignment of benefits can be blank,
	// clicking this button should explain why the warning is displayed
	afx_msg void OnBtnWarnAssignmentOfBenefitsPaper();
	//(r.wilson 10/8/2012) plid 40712 -
	void OnMarkClaimSent(NXDATALISTLib::_DNxDataListPtr);
	void OnMarkAllClaimsSent(NXDATALISTLib::_DNxDataListPtr);
	void OnMarkClaimSentSelected();
	void OnMarkAllClaimsSentSelected();
	void OnMarkClaimSentUnselected();
	void OnMarkAllClaimsSentUnselected();

private:

	long	m_Counter;
	CString StrTemp;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BATCHPRINTDLG_H__1E890A49_AB20_11D2_9890_00104B318376__INCLUDED_)
