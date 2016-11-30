#pragma once


// CDirectMessageReceivedDlg dialog
// (j.camacho 2013-10-22 16:04) - PLID 59064

class CDirectMessageReceivedDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDirectMessageReceivedDlg)

	// (b.spivey, October 21, 2013) - PLID 59115 - datalist enum
	enum DirectMessageReceivedList {
		dmrlReadStateIcon = 0,
		dmrlID,
		dmrlMessageSize,
		dmrlDateReceived, 
		dmrlSender,
		dmrlSubject,
		dmrlMessageState, // (b.spivey - May 2nd, 2014) - PLID 61799 - new column for state. 
	};

	// (b.spivey - May 5th, 2014) PLID 61799 - supported stats for direct messages. 
	enum DirectMessageState {
		dmsOther = 0, //failsafe state
		dmsNew = 5, 
		dmsRead, 
	};

	// (b.spivey - May 5th, 2014) PLID 61797 - enum for address list. 
	enum DirectMessageAddressList {
		dmalLastName = 0, 
		dmalFirstName, 
		dmalEmail, 
	};

public:
	CDirectMessageReceivedDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectMessageReceivedDlg();
	CString m_strActiveEmail;

	//interface
	CNxIconButton m_btnSend;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnSetupDirectCertificate; // b.spivey, January 22nd, 2014 - PLID 59596	

	// (b.spivey - June 2, 2014) - PLID 62289 - Variables for controls. 
	CNxIconButton m_btnNextPage;
	CNxIconButton m_btnPrevPage;

	// (b.spivey - June 2, 2014) - PLID 62291 - Label to show which bracket of messages are on screen. 
	CNxLabel m_lblMessageBracket; 
	

// Dialog Data
	enum { IDD = IDD_DIRECTMESSAGE_RECEIVED };

protected:
	//datalist
	NXDATALIST2Lib::_DNxDataListPtr m_dlReceivedList;
	NXDATALIST2Lib::_DNxDataListPtr m_dlEmailList;
	CNxColor m_background;

	// (b.spivey - June 2, 2014) - PLID 62289 - member to track the current page. 
	long m_nCurrentPage; 

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	// (b.spivey, October 21, 2013) - PLID 59115 - Load messages from list. 
	// (b.spivey - June 2, 2014) - PLID 62289 - Changed this around to take a safe array and load that into the datalist. 
	void LoadMessagesIntoList(Nx::SafeArray<IUnknown *> aryHeaders);
	// (d.singleton 2014-05-02 11:17) - PLID 61803 - Add a filter to the direct message lists to show messages within a date range filter.
	// (d.singleton 2014-05-02 11:17) - PLID 61802 - Add an option to hide read messages in the direct message list.
	NxButton m_btnFilterRead;
	NxButton m_btnFilterDate;
	CDateTimePicker m_dtFrom;
	CDateTimePicker m_dtTo;
	afx_msg void OnBnClickedFilterRead();
	afx_msg void OnBnClickedFilterDate();
	void MarkMessageAsRead();
	void MarkMessageAsUnread();
	void RefreshFiltersOnList();
	void RButtonDownDirectmessageReceived(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	// (b.spivey - June 2, 2014) - PLID 62289 - Function to update the controls based on remaining rows. 
	void UpdateControls(long nRowCount);

	//Message handlers for changing pages. 
	void OnBnClickedNextPage();
	void OnBnClickedPrevPage();

	//Function that builds the filter and passes it to the API for us, then gives us the resulting array. 
	Nx::SafeArray<IUnknown *> GetMessageHeaderListFromAPI(CString strEmailID);

	// (b.spivey - June 2, 2014) - PLID 62291 - Update the label. 
	void UpdateMessageCountLabel();

	DECLARE_MESSAGE_MAP()

	
	afx_msg void OnBnClickedDirectmessageSend();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellDirectmessageReceived(LPDISPATCH lpRow, short nColIndex);
	
	CString ViewerEmail;// (j.camacho 2013-10-21 18:07) - PLID 59148

	// b.spivey, January 22nd, 2014 - PLID 59596
	void OnBnClickedDirectMessageCertificate();

	// (b.spivey - May 5th, 2014) PLID 61799 - Icon. 
	HANDLE m_hIconDirectMessageState, m_hIconPlaceholder;

public:
	afx_msg void OnBnClickedDirectmessageDelete();
	void RemoveMessageFromList(__int64 messageID );
	void SelChangedDirectmessageEmail(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangingDirectmessageEmail(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	virtual void UpdateView(bool bForceRefresh = true);		
	afx_msg void OnDtnDatetimechangeFilterFromDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeFilterToDate(NMHDR *pNMHDR, LRESULT *pResult);	
};
