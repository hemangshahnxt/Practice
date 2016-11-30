#pragma once


// GlassesContactLenseRxPrintSelectDlg dialog
//(r.wilson 6/4/2012) PLID 48952

class GlassesContactLenseRxPrintSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(GlassesContactLenseRxPrintSelectDlg)

public:
	GlassesContactLenseRxPrintSelectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~GlassesContactLenseRxPrintSelectDlg();

	//(r.wilson 3/19/2012) PLID 48952
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_BtnSelect;
	CNxIconButton m_BtnPrintPreview;
	BOOL m_bPrintGlassesRx;
	BOOL m_bPrintContactLensRx;
	BOOL m_bPrintPreview;
	NxButton m_BtnPrintGlassesRx;
	NxButton m_BtnPrintConLensRx;
	CNxIconButton m_BtnOk;
	CNxIconButton m_BtnCancel;
	CString m_strProviderIDs;
	CString m_strCLProviderIDs;
	CString m_strProviders;
	CString m_strCLProviders;
	CString m_strPatientName;

	//(r.wilson 7/10/2012) PLID 48952
	CString m_strGlassesExamDate;
	CString m_strConLensExamDate;

	long m_nProviderID;
	long m_nCLProviderID;
	long m_nCommonProviderID;
	long m_nSelectedProviderID;

	NXDATALIST2Lib::_DNxDataListPtr m_pProviderList;
	CNxLabel m_pGlassesProviderLabel;
	CNxLabel m_pContactLensProviderLabel;


// Dialog Data
	enum { IDD = IDD_SELECT_GLASSES_CONTACT_RX_PRINT_DIALOG };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void ParseProviderIDs(); //r.wilson

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedPrint();	
	DECLARE_EVENTSINK_MAP()
	void SelChangingNxdatalistProviders(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBnClickedBtnRxPrintPreview();
	void SelChosenNxdatalistProviders(LPDISPATCH lpRow);
	afx_msg void OnBnClickedCheckGlassesRx();
	afx_msg void OnBnClickedCheckContactLenseRx();
};
