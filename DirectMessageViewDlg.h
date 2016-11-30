#pragma once


// CDirectMessageViewDlg dialog
// (j.camacho 2013-10-22 16:04) - PLID 59112

class CDirectMessageViewDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDirectMessageViewDlg)

public:
	CDirectMessageViewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectMessageViewDlg();

	void SetMessageID(__int64 id,CString email);
	bool CheckDelete();
	// (d.singleton 2014-05-16 08:35) - PLID 62173 - add option to close message and mark as Unread.  in case they open the wrong message. 
	bool CheckUnread();

	void SetMessageSize(__int64 nMessageSize); 

// Dialog Data
	enum { IDD = IDD_DIRECTMESSAGE_VIEW };

	bool m_bErrorOnLoading; // (j.camacho 2013-12-23 12:08) - PLID 59112 - Flag for if there was an error on loading viewer dialog 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	CNxEdit m_nxeditTo;
	CNxEdit m_nxeditFrom;
	CNxEdit m_nxeditSubject;
	IWebBrowser2Ptr m_pBrowser;	
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	// (d.singleton 2014-05-16 07:59) - PLID 62173 - add option to close message and mark as Unread.  in case they open the wrong message. 
	CNxIconButton	m_btnCloseUnread;
	

	__int64  m_lDirectMessageID;
	CString m_strEmailID; // (j.camacho 2013-10-21 18:07) - PLID 59112
	__int64 m_nDirectMessageSize; 
	bool m_bDeleted;//(j.camacho 2013-12-23 12:08)- PLID 59112 - Checked to see if the message was deleted with this bool flag	
	bool m_bCloseUnread;// (d.singleton 2014-05-16 07:59) - PLID 62173 - add option to close message and mark as Unread.  in case they open the wrong message. 
	
	//b.spivey (November 19th, 2013) - PLID 59336 - Hold the directory as a scoped ptr. When we're done it'll fly out of scope and destroy itself. 
	CString m_strTempDirectory; 
	scoped_ptr<FileUtils::CAutoRemoveTempDirectory> m_tempDirectory; 

	// (b.spivey - November 5, 2013) - PLID 59336 - member datalist
	NXDATALIST2Lib::_DNxDataListPtr m_dlAttachments;
	//function to load into the attachments datalist
	void LoadAttachmentsIntoList(const CArray<CString, CString>& aryAttachments); 
	//function to view the file
	afx_msg void LeftClickDirectMessageAttachments(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	// (b.spivey - December 6, 2013) - PLID 59336 - Clean up files. 
	void CleanUpTempFiles();

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedDirectmessageDelete();
	afx_msg void OnBnClickedDirectmessageUnread();
};
