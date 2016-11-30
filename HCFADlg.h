#pragma once

// HCFADlg.h : header file
//
#include "HCFASetupInfo.h"

typedef CArray<COleVariant, COleVariant> VarAry;

#define SCROLL_TOP_POS				((long)0)
#define SCROLL_BOTTOM_POS			((long)1250)
#define SCROLL_POS_PER_PAGE			((long)200)

void UpdateCharges(int &firstcharge);//needed to page up/down

/////////////////////////////////////////////////////////////////////////////
// CHCFADlg dialog

class CHCFADlg : public CDialog
{
// Construction
public:

	CHCFASetupInfo m_HCFAInfo;

	CHCFADlg(CWnd* pParent);   // standard constructor
	~CHCFADlg();
	long m_ID;
	long m_PatientID;
	long m_InsuredPartyID;
	long m_OthrInsuredPartyID;
	BOOL m_ShowWindowOnInit;
	CString m_strBillName;

	BOOL m_bShowSecondaryInBox11;

	CPrintDialog *m_printDlg;
	BOOL m_bPrintWarnings;

	long ScrollBottomPos;

	void (*m_pOnCommand)(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlID, class CFormDisplayDlg* dlg);
	BOOL (*m_pPrePrintFunc)(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);	

	long DoScrollTo(long nNewTopPos);

	void ShowSignature();
	void ShowAddress();
	void LoadDefaultBox11Value(int form, long InsuredPartyID);

	void BuildFormsT_Form33();

	void BuildGRPNumber(CStringArray* pastrParams, VarAry* pavarParams);

	void BuildHCFAChargesT();

	void SetHCFADateFormats();

	// (j.jones 2013-08-07 10:23) - PLID 57299 - removed GetHCFAGroupID()

	void FindBatch();
	void UpdateBatch();
	
	////////////////////////////////////////
	// Saves the whole HCFA
	void Save(BOOL boSaveAll, CDialog* pdlgWait, int& iPage);

	////////////////////////////////////////
	// Saves one page of the HCFA
	void SaveHistoricalData(BOOL boSaveAll);

// Dialog Data
	//{{AFX_DATA(CHCFADlg)
	enum { IDD = IDD_HCFA };
	BOOL	m_CapOnPrint;
	// (j.jones 2007-06-25 10:14) - PLID 25663 - changed the buttons to NxIconButtons
	CNxIconButton m_btnRestoreDefaults;
	CNxIconButton m_btnAlignForm;
	CNxIconButton m_btnSave;
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHCFADlg)
	public:
	virtual int DoModal(int billID);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// (j.jones 2007-06-22 13:31) - PLID 25665 - required for info text labeling
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	void LoadBox25Default();	
	CButton			*m_pleftbutton,
					*m_prightbutton,
					*m_pupbutton,
					*m_pdownbutton;

	// (j.dinatale 2010-07-23) - PLID 39692 - New member variable to store a device context so we can pass it to the batchprintdlg
	CDC	*m_pPrintDC;

	// (j.dinatale 2010-07-28) - PLID 39803 - Flag to determine if the dialog should be handling ClaimsHistoryT, ClaimHistoryDetailsT, and HCFATrackT updates
	bool m_bUpdateClaimsTables;

	void SetControlPositions(void);
	// Generated message map functions
	//{{AFX_MSG(CHCFADlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickCapitalizeOnPrint();
	afx_msg void OnRestore();
	afx_msg void OnCancel();
	afx_msg void OnCheck();
	afx_msg void OnPrint();
	afx_msg void OnAlign();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnRadioNoBatch();
	afx_msg void OnRadioElectronic();
	afx_msg void OnRadioPaper();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};