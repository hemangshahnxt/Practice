#include "PatientDialog.h"
#include "MirrorImageButton.h"
#include "MirrorPatientImageMgr.h"

#if !defined(AFX_GENERAL1DLG_H__18527913_CE66_11D1_804C_00104B2FE914__INCLUDED_)
#define AFX_GENERAL1DLG_H__18527913_CE66_11D1_804C_00104B2FE914__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Client.h"	// Network optimization code

// General1Dlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CGeneral1Dlg dialog

// Used for asynchronous image loads
class CGeneral1ImageLoad
{
public:
	class CGeneral1Dlg* m_pMsgWnd;
	CString m_strRemoteID;
	long m_nImageIndex;
	long m_nImageCount;
	long m_nPatientID;
	CMirrorImageButton* m_pButton;
	// (c.haag 2010-02-23 15:00) - PLID 37364 - This object is used for accessing Mirror images from the thread.
	CMirrorPatientImageMgr* m_pMirrorImageMgr;

	// (a.walling 2010-03-09 14:17) - PLID 37640 - Moved to cpp
	CGeneral1ImageLoad(CGeneral1Dlg* pMsgWnd, CMirrorPatientImageMgr* pMirrorImageMgr, CString strRemoteID, long nImageIndex, long nImageCount, long nPatientID, CMirrorImageButton *pButton);
	~CGeneral1ImageLoad();
};

class CGeneral1Dlg : public CPatientDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_DoctorCombo;
	NXDATALISTLib::_DNxDataListPtr m_CoordCombo;
	NXDATALISTLib::_DNxDataListPtr m_PrefixCombo;
	NXDATALISTLib::_DNxDataListPtr m_PartnerCombo;
	NXDATALISTLib::_DNxDataListPtr m_GenderCombo;
	NXDATALISTLib::_DNxDataListPtr m_PreferredContactCombo;
	CString m_InformPath;
	void SaveCustomInfo(int id, CString value);
	void UpdatePalm();
	CGeneral1Dlg(CWnd* pParent);   // standard constructor

	// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
	CTableChecker m_doctorChecker, m_coordChecker, m_partnerChecker;

	void			Save	(int nID);
	void			Load	(bool overwrite = true);	
	// (a.walling 2010-10-12 15:00) - PLID 40908 - Tablechecker-based refreshes moved to their own function
	void			HandleTableCheckers();
	virtual void	SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	virtual int	Hotkey(int key);
	virtual void	SecureControls();
	virtual BOOL	IsEditBox(CWnd* pWnd);
	long			m_id;
	CString			m_strPatientName;
// Dialog Data
	//TES 4/4/2008 - PLID 29550 - Converted to CNxEdits.
	// (j.jones 2008-07-09 10:30) - PLID 24624 - added m_btnPatientSummary
	//{{AFX_DATA(CGeneral1Dlg)
	enum { IDD = IDD_GENERAL_1_DLG_32 };
	CNxIconButton	m_btnPatientSummary;
	CNxEdit	m_nxeTitle;
	CNxEdit	m_nxeWorkPhone;
	CNxEdit	m_nxeZip;
	CNxEdit	m_nxeState;
	CNxEdit	m_nxeSSN;
	CNxEdit	m_nxeReferral;
	CNxEdit	m_nxePager;
	CNxEdit	m_nxeOtherPhone;
	CNxEdit	m_nxeNotes;
	CNxEdit	m_nxeLastName;
	CNxEdit	m_nxeID;
	CNxEdit	m_nxeHomePhone;
	CNxEdit	m_nxeFax;
	CNxEdit	m_nxeExtension;
	CNxEdit	m_nxeEnteredBy;
	CNxEdit	m_nxeEmergWorkPhone;
	CNxEdit	m_nxeEmergRelation;
	CNxEdit	m_nxeEmergLast;
	CNxEdit	m_nxeEmergHomePhone;
	CNxEdit	m_nxeEmergFirst;
	CNxEdit	m_nxeEmail;
	CNxEdit	m_nxeDear;
	CNxEdit	m_nxeCustom4;
	CNxEdit	m_nxeCustom3;
	CNxEdit	m_nxeCustom2;
	CNxEdit	m_nxeCustom1;
	CNxEdit	m_nxeCity;
	CNxEdit	m_nxeCellPhone;
	CNxEdit	m_nxeAge;
	CNxEdit	m_nxeAddress2;
	CNxEdit	m_nxeAddress1;
	CNxEdit	m_nxeMiddleName;
	CNxEdit	m_nxeFirstName;
	NxButton	m_btnEmailPriv;
	NxButton	m_btnOtherPriv;
	NxButton	m_btnFaxPriv;
	NxButton	m_btnPagerPriv;
	NxButton	m_CellPrivCheck;
	NxButton	m_ExcludeMailingsCheck;
	CNxIconButton	m_btnSendToThirdParty;
	CNxIconButton	m_imageNext;
	CNxIconButton	m_imageLast;
	CNxIconButton	m_nxbtnVisonPriscription;// (s.dhole 2012-02-27 16:55) - PLID  48354 
	CMirrorImageButton	m_imageButton;
	CMirrorImageButton	m_imageButtonLeft;
	CMirrorImageButton	m_imageButtonRight;
	CMirrorImageButton	m_imageButtonUpperLeft;
	CMirrorImageButton	m_imageButtonUpperRight;
	CMirrorImageButton	m_imageButtonLowerLeft;
	CMirrorImageButton	m_imageButtonLowerRight;
	CNxEdit	m_MarriageOther;
	NxButton			m_HomePrivCheck;
	NxButton			m_InactiveCheck;
	NxButton			m_ForeignCheck;
	NxButton			m_WorkPrivCheck;
	NxButton			m_singleRad;
	NxButton			m_marriedRad;
	NxButton			m_otherRad;
	NxButton			m_isPatient;
	NxButton			m_isPatientProspect;
	NxButton			m_isProspect;
	CNxColor	m_customBkg;
	CNxColor	m_emergBkg;
	CNxColor	m_phoneBkg;
	CNxColor	m_referralBkg;
	CNxColor	m_demBkg;
	CNxStatic	m_nxstaticPartnerLabel;
	CNxStatic	m_nxstaticCustom1Label;
	CNxStatic	m_nxstaticCustom4Label;
	CNxStatic	m_nxstaticCustom3Label;
	CNxStatic	m_nxstaticCustom2Label;
	NxButton	m_btnTextMessage;
	CNxLabel	m_nxlabelDeclinedEmail; // (z.manning 2009-07-09 10:33) - PLID 27251
	CNxLabel	m_nxlabelSetEmailDeclined; // (z.manning 2009-07-09 10:33) - PLID 27251
	CNxLabel	m_nxlabelSetCompanyLink; // (s.dhole 2010-03-26 16:50) - PLID 37796
	CNxIconButton m_btnEditSecurityGroups;	//TES 1/5/2010 - PLID 35774
	CNxIconButton m_btnRecall;	// (j.armen 2012-02-24 16:18) - PLID 48303
	CNxIconButton m_btnReminder;	//(s.dhole 8/28/2014 1:36 PM ) - PLID 62747 
	CNxStatic	m_nxstaticReminder;//(s.dhole 8/28/2014 1:36 PM ) - PLID 62747 
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGeneral1Dlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnMSRDataEvent(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnCanfieldSDKInitComplete(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pCountryList;			// v.arth 2009-05-29 PLID 34386
	typedef enum { eImageFwd, eImageBack } EImageTraversal;
	void OnRightClickImage(HWND hwndClicked, UINT nFlags, CPoint pt);
	void SetInactiveCheck();
	
	/////////////////
	// For imaging
	/////////////////
	// Variables
	CString m_strMirrorID;
	long m_nUnitedID;
	long m_nImageIndex;
	// Functions
	void LoadPatientImagingInfo(ADODB::_RecordsetPtr prsCurPatient);
	void LoadPatientImage(EImageTraversal dir = eImageFwd);
	void LoadImageAsync(CGeneral1ImageLoad* pLoadInfo);
	void LoadSingleImage(long nImageIndexAgainstSource, long nImageSourceCount, CMirrorImageButton *pButton, EImageSource Source);
	long GetLinks();
	void UpdateSendToThirdPartyButton();
	void GenerateLinkMenu();
	void FillAreaCode(long nID);
	bool SaveAreaCode(long nID);
	CString m_strFirstName;
	CString m_strMiddleName;
	CString m_strLastName;
	CString m_strSocialSecurity;
	CWinThread *m_pLoadImageThread;
	CPtrList m_WaitingImages;
	void ShowImagingButtons(int nCountToShow);
	void HandleImageClick(CMirrorImageButton *pClicked);
	CMirrorImageButton *m_RightClicked;
	CString RetrievePhoneNumber();
	//////////////////////////////////

	DWORD m_dwSendToStatus;

	void OnLinkToInform();
	void OnLinkToMirror();
	void OnLinkToUnited();
	void OnLinkToQuickbooks();
	void OnLinkToCareCredit();

	int GetLabelFieldID(int nID);
	BOOL EnsureCorrectAge();
	virtual void StoreDetails();
	void ShowMirrorButtons();
	//(a.wilson 2012-1-11) PLID 47485 - to handle barcode scanner data events.
	virtual LRESULT OnBarcodeScannerDataEvent(WPARAM wParam, LPARAM lParam);	

	// (r.gonet 2010-08-30 16:36) - PLID 39939 - Set the color of the Groups button based on patient's group membership
	void UpdateGroupsButton();

	// (j.gruber 2007-08-27 09:39) - PLID 24628 - used for updating HL7
	void UpdateHL7Data();

	// (c.haag 2009-04-01 17:06) - PLID 33630 - This function will determine whether the Canfield SDK
	// needs to initialize. It will do so by trying to asynchronously initialize the link. If the result
	// is that the initialization is in progress, the image thumbnail is changed to the "Initializing Mirror"
	// sentinel value, and this returns TRUE. In all other circumstances, will return FALSE.
	BOOL DoesCanfieldSDKNeedToInitialize();

	CString			m_sql;
	bool			m_loading;
	CString         m_strAreaCode;
	BOOL			ChangeCustomLabel (const int nID);
	void			UpdateEmailButton();
	BOOL			UserCanChangeName();

	BOOL			m_bFormatPhoneNums;
	CString			m_strPhoneFormat;

	NXTIMELib::_DNxTimePtr		m_nxtBirthDate, m_nxtFirstContact;
	COleDateTime	m_dtBirthDate, m_dtFirstContact;
	bool			m_bBirthDateSet;		//DRT 9/29/03 - If we just use the above for comparisons, we have problems w/ a real date of 12/30/1899.
	bool			m_bSavingContactDate;

	bool			m_bFormattingField;
	bool			m_bFormattingAreaCode;

	// (a.wetta 2007-03-20 10:17) - PLID 24983 - Keep track of if a driver's license swipe is being processed
	//(a.wilson 2012-1-19) PLID 47485 - changed name to better suit its additional purpose
	bool			m_bProcessingCard;

	// (j.gruber 2007-08-08 12:05) - PLID 25045 - need to save old values to avoid checking database
	CString m_strEmail;
	CString m_strHomePhone;
	CString m_strWorkPhone;
	CString m_strOtherPhone;
	CString m_strCellPhone;
	CString m_strFax;
	CString m_strPager;
	CString m_strAddress1;
	CString m_strAddress2;
	CString m_strCity;
	CString m_strState;
	CString m_strZip;

	// (j.gruber 2009-10-08 09:56) - PLID 35825 - Override the tab order if they have city lookup
	BOOL m_bLookupByCity;

	// (c.haag 2009-07-07 13:12) - PLID 34379 - Internal ID in the RSI MMS system
	long	m_nMMSPatientID;

	// (c.haag 2010-02-23 09:51) - PLID 37364 - This object is used for loading patient Mirror images
	CMirrorPatientImageMgr* m_pMirrorImageMgr;

	// (z.manning 2009-07-09 11:38) - PLID 27251
	void UpdateEmail(const BOOL bDeclinedEmail, const CString &strEmail);

	// (c.haag 2010-02-23 10:21) - PLID 37364 - This function ensures that the Mirror image manager exists
	// and is ready for getting image counts and loading images
	void EnsureMirrorImageMgr();
	// (c.haag 2010-02-23 15:08) - PLID 37364 - This function ensures that the Mirror image manager does
	// not exist. If it did, it is deleted.
	void EnsureNotMirrorImageMgr();
	// (c.haag 2010-02-23 10:22) - PLID 37364 - Returns the patient's Mirror image count. This only includes
	// images that were not imported by the Mirror image import app
	long GetMirrorImageCount();
	// (c.haag 2010-02-23 10:22) - PLID 37364 - Loads a Mirror image. The input index must be between 0 and
	// the number of patient images that were not imported by the Mirror image import app
	HBITMAP LoadMirrorImage(long &nIndex, long &nCount, long nQualityOverride);
	// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the first valid Mirror image index for this patient
	long GetFirstValidMirrorImageIndex(const CString& strMirrorID);
	// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the last valid Mirror image index for this patient
	long GetLastValidMirrorImageIndex(const CString& strMirrorID);

	//(c.copits 2011-09-22) PLID 45632 - General 1 email validation lets through invalid addresses.
	CString m_strOldEmail;
	bool m_bDeclinedEmail;
	bool m_bDeclinedEmailWasBlank;


	//(s.dhole 8/28/2014 1:43 PM ) - PLID 62747
	void SetReminderLabel();
	//HBITMAP			m_image;

	// (a.walling 2008-04-10 12:52) - PuntoEXE image processing no longer used
	// (j.jones 2008-07-09 10:30) - PLID 24624 - added OnBtnPatientSummary
	// (z.manning 2008-07-11 09:20) - PLID 30678 - Added OnCellTextMessage
	// Generated message map functions
	//{{AFX_MSG(CGeneral1Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnForeignCheck();
	afx_msg void OnInactiveCheck();
	afx_msg void OnHomePrivCheck();
	afx_msg void OnWorkPrivCheck();
	afx_msg void OnPullUpPrefixCombo(long iNewRow);
	afx_msg void OnPullUpDoctorCombo(long nRowIndex);
	afx_msg void OnPullUpCoordCombo(long nRowIndex);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnStatusChanged();
	afx_msg void OnImageLast();
	afx_msg void OnImageNext();
	afx_msg void OnImage();
	afx_msg void OnImageLeft();
	afx_msg void OnImageRight();
	afx_msg void OnImageUpperLeft();
	afx_msg void OnImageUpperRight();
	afx_msg void OnImageLowerLeft();
	afx_msg void OnImageLowerRight();
	afx_msg void OnEmail();
	afx_msg void OnUpdateInform();
	afx_msg void OnSingleRad();
	afx_msg void OnMarriedRad();
	afx_msg void OnOtherRad();
	afx_msg void OnUpdateMirror();
	afx_msg void OnUpdateUnited();
	afx_msg void OnUpdateQBooks();
	afx_msg void OnKillfocusNotes();
	afx_msg void OnCopyImage();
	afx_msg void OnViewImage();
	afx_msg void OnSelChosenPartnerList(long nRow);
	afx_msg void OnClosedUpGenderList(long nSelRow);
	afx_msg void OnPartnerBtn();
	afx_msg void OnGroups();
	afx_msg void OnSelChangeFaxChoice();
	afx_msg LRESULT OnImageLoaded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSendToThirdParty();
	afx_msg void OnSelChosenPreferredContactList(long nRow);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnLostFocus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnExcludeMailings();
	afx_msg void OnRequeryFinishedPrefixCombo(short nFlags);
	afx_msg void OnSelChosenPrefixCombo(long nRow);
	afx_msg void OnSelChosenGenderList(long nRow);
	afx_msg void OnEditPrefixes();
	afx_msg void OnKillFocusBirthDateBox();
	afx_msg void OnKillFocusContactDate();
	afx_msg void OnCellPrivCheck();
	afx_msg void OnKillfocusEmailBox();
	afx_msg void OnTrySetSelFinishedDoctorCombo(long nRowEnum, long nFlags);
	afx_msg void OnPagerPrivCheck();
	afx_msg void OnFaxPrivCheck();
	afx_msg void OnOtherPrivCheck();
	afx_msg void OnEmailPrivCheck();
	afx_msg void OnTrySetSelFinishedCoordCombo(long nRowEnum, long nFlags);
	afx_msg void OnRequeryFinishedDoctorCombo(short nFlags);
	afx_msg void OnRequeryFinishedCoordCombo(short nFlags);
	afx_msg void OnBtnPatientSummary();
	afx_msg void OnCellTextMessage();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);	
	afx_msg void OnEditSecurityGroups();
	afx_msg void OnBtnRecallClicked();	// (j.armen 2012-02-24 16:19) - PLID 48303
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// v.arth 2009-05-26 PLID 20913 - Outlook-like phone number entry
	void SelChosenCountryList(LPDISPATCH lpRow);

	// (d.lange 2010-06-30 15:45) - PLID 38687 - This function determines whether there is a device plugin enabled that supports 
	// the 'LaunchDevice' functionality
	// (j.gruber 2013-04-02 16:28) - PLID 56012 - renamed and moved to launch utils
	void GenerateDeviceMenu();
	afx_msg void OnBnClickedShowVisionPrescription();
	afx_msg void OnBnClickedBtnSentReminder();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERAL1DLG_H__18527913_CE66_11D1_804C_00104B2FE914__INCLUDED_)
