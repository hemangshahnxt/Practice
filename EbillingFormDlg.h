// EbillingFormDlg.h : header file
//
#include "progressbar.h"
#include "HCFAdlg.h"
#include "NxAPI.h"

#if !defined(AFX_EBILLINGFORMDLG_H__1E890A49_AB20_11D2_9890_00104B318376__INCLUDED_)
#define AFX_EBILLINGFORMDLG_H__1E890A49_AB20_11D2_9890_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum EEbillingValidationResults {
	eevrSuccess = 0,
	eevrFailedError,
	eevrFailedOther,
};

/////////////////////////////////////////////////////////////////////////////
// CEbillingFormDlg dialog

class CEbillingFormDlg : public CNxDialog
{
// Construction
public:

	HANDLE m_hIconCheck, m_hIconRedX, m_hIconGrayX, m_hIconQuestion;

	void ShowErrors();
	// (j.jones 2007-03-01 08:59) - PLID 25015 - changed Validate to return an enum
	EEbillingValidationResults Validate(long HCFAID);
	void ChangeFormats();
	// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program
	void PrepareImage();
	void PrepareANSI();
	void PrepareOHIP();
	// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
	void PrepareAlberta();
	void UpdateCurrentSelect(int iList);
	void RefreshTotals();
	int m_CurrList;  //0 - Unselected, 1 - Selected	
	CString PromptReturn;

	void UpdateLabels(CString strFormType);

	CEbillingFormDlg(CWnd* pParent);	// standard constructor
	~CEbillingFormDlg(); // desctructor
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	void CalcShowBatchedChargeCount();

	// (d.singleton 2011-07-05) - PLID 44422 - added functions to help format text from the alberta billing assessment file
	// (j.jones 2014-07-24 10:48) - PLID 62579 - moved the Alberta assessment parsing to its own class	

	// (j.jones 2009-08-14 12:29) - PLID 35235 - added provider tablechecker
	CTableChecker m_providerChecker;

	// (j.jones 2008-07-02 15:40) - PLID 30604 - added m_btnFormatOHIPBatchEdit
	// (j.jones 2008-07-02 15:41) - PLID 21968 - added m_btnFormatOHIPClaimsError
	// (j.jones 2008-12-17 09:41) - PLID 31900 - removed the OHIP report formatting buttons and added m_btnOHIPReportManager
	// (j.jones 2009-03-09 15:07) - PLID 32405 - added m_btnOHIPDialerSetup
	// (j.jones 2009-03-09 15:07) - PLID 33419 - added m_btnDownloadReports
// Dialog Data
	//{{AFX_DATA(CEbillingFormDlg)
	enum { IDD = IDD_EBILLINGFORM_DIALOG };
	CNxIconButton	m_btnOHIPReportManager;
	//CNxIconButton	m_btnFormatOHIPClaimsError;
	//CNxIconButton	m_btnFormatOHIPBatchEdit;
	CNxIconButton	m_btnConfigureClaimValidation;
	CNxIconButton	m_btnClearinghouseIntegration;
	CNxIconButton	m_btnFormat997;
	CNxIconButton	m_btnFormat277;
	CNxIconButton	m_btnRetrieveBatches;
	NxButton	m_production;
	NxButton	m_test;
	CNxIconButton	m_btnValidateUnselected;
	CNxIconButton	m_btnValidateSelected;
	CNxIconButton	m_btnValidateAll;
	CNxIconButton	m_btnExportBatch;
	CNxIconButton	m_btnPrintAll;
	CNxIconButton	m_btnResetAll;
	CNxIconButton	m_btnPrintUnselected;
	CNxIconButton	m_btnPrintSelected;
	CNxIconButton	m_btnResetSelected;
	CNxIconButton	m_btnResetUnselected;
	CNxIconButton	m_btnUnselectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_Config;
	CNxIconButton	m_OpenButton;
	CNxStatic			m_StyleText;
	int				m_LineCount;
	CProgressBar	m_ExportProg;
	int				m_Counter;
	CNxStatic	m_nxstaticEbillingUnselectedLabel;
	CNxStatic	m_nxstaticUnselectedTotal;
	CNxStatic	m_nxstaticFormTypeLabel;
	CNxStatic	m_nxstaticEbillingSelectedLabel;
	CNxStatic	m_nxstaticSelectedTotal;
	CNxIconButton m_btnOHIPDialerSetup;
	CNxIconButton m_btnDownloadReports;
	// (j.jones 2009-08-14 09:49) - PLID 35235 - added provider filter
	CNxStatic	m_nxstaticProviderLabel;
	// (j.jones 2010-07-23 16:15) - PLID 34105 - added warning label & button for assignment of benefits
	CNxStatic	m_nxstaticAssignmentOfBenefitsWarningLabel;
	CNxIconButton	m_btnAssignmentOfBenefitsWarning;
    // (d.singleton 2011-07-05) - PLID 44422 - button to open Alberta billing error reports parser dialog
	// (j.jones 2014-07-24 10:57) - PLID 62535 - renamed this button
	CNxIconButton m_btnAlbertaAssessmentFile; 
	CNxIconButton m_btnLaunchTrizettoWebsite;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEbillingFormDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	NXDATALISTLib::_DNxDataListPtr m_UnselectedBatchList;
	NXDATALISTLib::_DNxDataListPtr m_SelectedBatchList;
	NXDATALISTLib::_DNxDataListPtr m_ExportStyle;
	NXDATALISTLib::_DNxDataListPtr m_LocationCombo;
	NXDATALISTLib::_DNxDataListPtr m_FormTypeCombo;
	NXDATALISTLib::_DNxDataListPtr m_ExportFormat;
	// (j.jones 2009-08-14 09:49) - PLID 35235 - added provider filter
	NXDATALISTLib::_DNxDataListPtr m_ProviderCombo;

	// (j.jones 2009-08-14 09:59) - PLID 35235 - added a filter function
	void RefilterLists();
	
	// (s.tullis 2014-08-08 10:16) - PLID 62780
	void UpdateAlbertaBillstatus();

	//called when we would like to show the button to launch the Trizetto website,
	//will show/hide based on whether any web passwords have been entered
	void TryShowTrizettoWebButton();

	//Checks to see if the user is about to auto-upload ANSI claims to Trizetto,
	//and if so, do they have multiple SiteIDs configured. If they do, the claim
	//list is validated to confirm the export qualifies for only one Site ID.
	//If this returns true, the export can continue with the provided Site ID and password.
	//If this returns false, the export should be cancelled.
	//This function will have told the user why the claims could not be exported.
	bool GetAutoUploadCredentialsForClaims(NexTech_Accessor::_ClearinghouseLoginPtr& pClearinghouseLogin);

	// (j.jones 2008-07-02 15:40) - PLID 30604 - added OnFormatOhipBatchEditBtn
	// (j.jones 2008-07-02 15:41) - PLID 21968 - added OnFormatOhipClaimsErrorBtn
	// (j.jones 2008-12-17 09:44) - PLID 31900 - added the OHIP Report Manager, which
	// replaces the batch edit / claims error buttons
	// (j.jones 2009-03-09 15:07) - PLID 32405 - added OnBtnOhipDialerSetup
	// (j.jones 2009-03-09 15:07) - PLID 33419 - added OnBtnDownloadReports
	// Generated message map functions
	//{{AFX_MSG(CEbillingFormDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnExport();
	afx_msg void OnRemoveHCFA();
	afx_msg void OnReset();
	afx_msg void OnPrintList();
	afx_msg void OnOpenHCFA();
	afx_msg void OnValidateOne();
	afx_msg void OnGoToPatient();
	afx_msg void OnStyleChange(long iNewRow);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void EnsureClearinghouseIntegrationControls();
	afx_msg void SendToPaper();
	afx_msg void OnConfig();
	afx_msg void OnRButtonDownUnselectedBatchlist(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedUnselectedBatchlist(short nFlags);
	afx_msg void OnRButtonDownSelectedBatchlist(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedSelectedBatchlist(short nFlags);
	afx_msg void OnDblClickCellUnselectedBatchlist(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedBatchlist(long nRowIndex, short nColIndex);
	afx_msg void OnSelectOne();
	afx_msg void OnSelectAll();
	afx_msg void OnUnselectOne();
	afx_msg void OnUnselectAll();
	afx_msg void OnPrintSelectedList();
	afx_msg void OnResetSelected();
	afx_msg void OnResetUnselected();
	afx_msg void OnPrintUnselectedList();
	afx_msg void OnValidateAllClaims();
	afx_msg void OnValidateSelectedClaims();
	afx_msg void OnValidateUnselectedClaims();
	afx_msg void OnProductionBatch();
	afx_msg void OnTestBatch();
	afx_msg void OnBtnRetrieveBatches();
	afx_msg void OnFormat277Btn();
	afx_msg void OnFormat997Btn();
	afx_msg void OnSelectInsCo();
	afx_msg void OnSelectInsGroup();
	afx_msg void OnSelectProvider();
	// (j.jones 2012-03-26 10:43) - PLID 48854 - added ability to select by claim provider
	afx_msg void OnSelectClaimProvider();
	afx_msg void OnSelChosenExportstyle(long nRow);
	afx_msg void OnSelChosenLocationComboElectronic(long nRow);
	afx_msg void OnSelChosenEbillingFormTypeCombo(long nRow);
	afx_msg void OnBtnConfigureClaimValidation();
	afx_msg void OnSelChosenExportformat(long nRow);
	//afx_msg void OnFormatOhipBatchEditBtn();
	//afx_msg void OnFormatOhipClaimsErrorBtn();
	afx_msg void OnBtnOhipReportManager();
	afx_msg void OnBtnOhipDialerSetup();
	afx_msg void OnBtnDownloadReports();
	afx_msg void OnSendListToHcfa();
	// (j.jones 2014-07-24 10:56) - PLID 62535 - renamed this function
	afx_msg void OnBtnAlbertaAssessmentFile();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	//(r.wilson 10/8/2012) plid 40712 -
	void OnMarkClaimSent(NXDATALISTLib::_DNxDataListPtr);
	void OnMarkAllClaimsSent(NXDATALISTLib::_DNxDataListPtr);
	void OnMarkClaimSentSelected();
	void OnMarkAllClaimsSentSelected();
	void OnMarkClaimSentUnselected();
	void OnMarkAllClaimsSentUnselected();
	// (b.spivey, August 27th, 2014) - PLID 63492 - preview the claim file. 
	void OnPreviewClaimFile(long nBillID); 
	void OnPreviewClaimFileUnselected();
	void OnPreviewClaimFileSelected(); 


private:

	int	FormatID;
	int FormatStyle; //the style we are exporting (Image, ANSI, OHIP)
	int ClaimType; //HCFA or UB92
	CString	StrTemp;
	void OnSelChosenEbillingProviderCombo(long nRow);
	// (j.jones 2010-07-23 16:25) - PLID 34105 - only shown if assignment of benefits can be blank,
	// clicking this button should explain why the warning is displayed
	afx_msg void OnBtnWarnAssignmentOfBenefitsEbilling();
public:
	afx_msg void OnBnClickedClearinghouseIntegrationSetupBtn();
	afx_msg void OnBtnLaunchTrizettoWebsite();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBILLINGFORMDLG_H__1E890A49_AB20_11D2_9890_00104B318376__INCLUDED_)
