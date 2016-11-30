#pragma once

// (a.walling 2010-05-19 08:26) - PLID 38558 - Include NxInkPictureImport.h rather than #import "NxInkPicture.tlb" so the proper tlb is chosen based on current SDK path (patch or main)
#include "NxInkPictureImport.h"

// CSignatureDlg dialog
// (z.manning 2008-10-15 14:55) - PLID 21082 - Created

class CSignatureDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSignatureDlg)

	// (a.walling 2009-12-24 14:29) - PLID 36377 - Helper functions for caching a signature now that it lives in UsersT rather than preference
	// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
	static CString GetDefaultSignatureFileName(long nSignatureUserID);
	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
	static void GetDefaultSignatureData(long nSignatureUserID, OUT _variant_t &varData, OUT _variant_t &varText, OUT BOOL &bSignatureDateOnly);
	
	// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
	static void EnsureDefaultSignatureCache(long nSignatureUserID);
	static void ClearSignatureCache();

	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
	static void SetDefaultSignatureData(long nSignatureUserID, _variant_t& varNewData, _variant_t &varText, BOOL &bSignatureDateOnly);
	// (j.jones 2013-08-08 14:04) - PLID 42958 - now takes in a user ID
	static void SetDefaultSignatureFileName(long nSignatureUserID, const CString& strNewFileName);

	// (j.jones 2013-08-08 16:07) - PLID 42958 - added a user ID for who is cached
	static long g_nSignatureUserID;
	static bool g_bSignatureCached;
	static _variant_t g_varSignatureData;
	static CString g_strSignatureFileName;
	// (j.jones 2010-04-12 14:08) - PLID 16594 - handle the signature date stamp
	static _variant_t g_varSignatureText;
	static BOOL g_bSignatureDateOnly;
	static short g_nSignatureEmrScaleFactor;

	// (j.jones 2013-08-08 13:35) - PLID 42958 - added optional user ID and username, for cases when
	// the signature is being created by a user who is not the logged-in user
	int DoModal(OPTIONAL long nSignatureUserID = GetCurrentUserID(),
		OPTIONAL CString strSignatureUserName = GetCurrentUserName());

public:
	CSignatureDlg(CWnd* pParent);   // standard constructor
	virtual ~CSignatureDlg();

	_variant_t GetSignatureInkData();
	// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
	_variant_t GetSignatureTextData();
	BOOL GetSignatureDateOnly();
	CString GetSignatureFileName();
	short GetSignatureEmrScaleFactor(); // (z.manning 2011-09-23 12:39) - PLID 42648

	// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
	void SetSignature(const CString strFile, const _variant_t varInkData, const _variant_t varTextData);

	void SetText(const CString strText);

	BOOL m_bReadOnly;

	// (z.manning 2008-10-22 10:40) - PLID 23110 - If this is true then this dialog will auto-commit
	// itself if the current user has a signature and it's set to auto load on this dialog.
	BOOL m_bAutoCommitIfSignature;

	// (z.manning 2008-10-17 10:35) - PLID 23110 - Determines whether or not to require a
	// signature before committing this dialog.
	BOOL m_bRequireSignature;

	// (z.manning 2008-10-22 15:47) - Set this to true before opening the dialog to have this dialog
	// be in "setup" mode where you can edit the signature but there's no need to commit or cancel
	// the dialog for use elsewhere.
	BOOL m_bSetupOnly;

	// (z.manning 2008-12-09 08:58) - PLID 32260 - We now have a lab preference and an EMR preference
	// for whether or not we should check for the password on load as requested and approved by c.majeed.
	BOOL m_bCheckPasswordOnLoad;

// Dialog Data
	enum { IDD = IDD_SIGNATURE };

protected:

	// (j.jones 2013-08-08 13:35) - PLID 42958 - we now track the user ID and username for who is signing,
	// which supports cases when the signature is being created by a user who is not the logged-in user
	long m_nSignatureUserID;
	CString m_strSignatureUserName;

	CWnd m_wndInkPic;
	NXINKPICTURELib::_DNxInkPicturePtr m_pSignatureImageCtrl;
	// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - More versatile replacement for g_EmrImageCache

	// (z.manning 2008-10-17 09:35) - These variables are mainly intended for use for setting intial values
	// for the signature and for other dialogs to get the values after this dialog has been closed. If you
	// want the real-time values, get them from the ink picture control.
	_variant_t m_varSignatureInkData;
	CString m_strSignatureFileName;
	_variant_t m_varOriginalSignatureInkData;
	// (j.jones 2010-04-12 12:30) - PLID 16594 - signatures now have a stamp
	_variant_t m_varSignatureTextData;
	_variant_t m_varOriginalSignatureTextData;

	CString m_strText;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	void BrowseForImageFile();

	BOOL HasSignatureChanged();

	// (j.jones 2013-08-08 16:53) - PLID 42958 - added 'skip password'
	void LoadDefaultSignature(BOOL bSilent, BOOL bSkipPassword);
	void SaveDefaultSignature(BOOL bCheckPassword);

	BOOL SaveScalingFactor();  // (z.manning 2011-09-23 16:36) - PLID 42648

	DECLARE_MESSAGE_MAP()
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSelectImage;
	CNxIconButton m_btnLoadDefaultSignature;
	CNxIconButton m_btnSaveDefaultSignature;
	CNxIconButton m_btnEraseInk;
	NxButton m_nxbtnAutoPromptDefaultSignature;
	CNxIconButton m_btnClose;

	CNxIconButton m_btnTopaz;
	// (j.jones 2010-04-12 16:15) - PLID 16594 - added option for the date stamp to not include time
	NxButton m_checkSignatureDateNoTime;
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnBnClickedSelectSignatureImage();
	afx_msg void OnBrowseInkPicture(BOOL FAR* pbProcessed);
	afx_msg void OnBnClickedUseDefaultSignature();
	afx_msg void OnBnClickedSaveDefaultSignature();
	afx_msg void OnBnClickedEraseSignatureInk();
	afx_msg void OnBnClickedAutoPromptForSignature();
	afx_msg void OnClose();
	CNxStatic m_nxstaticText;
	afx_msg void OnCheckSignatureDateNoTime();
	afx_msg void OnEnKillfocusSignaturePercentSize();
	afx_msg void OnBtnClickedTopaz();	// (d.singleton 2013-05-03 16:59) - PLID 56421 - add button and functionality to the NxInkControl to use the topaz signature pad
};
