#pragma once

struct attachmentFile
{
	CString pathname;
	CString docname;
	Nx::SafeArray<BYTE> fileBytes; // (j.gruber 2013-11-11 11:50) - PLID 59403 - added filtbytes and bool field
	BOOL bHasFileBytes;
	long nMailID;	
};


// (j.camacho 2013-10-09 17:37) - PLID 58929 - CCDA message dialog 
// CDirectMessageSendDlg dialog


class CDirectMessageSendDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDirectMessageSendDlg)

public:
	CDirectMessageSendDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirectMessageSendDlg();

// member variable
	long m_lMailID; // (j.camacho 2013-11-15 17:09) - PLID 59514

// Interface improvements
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnQuestion;
	CNxIconButton m_btnAttachFiles; 
	CNxEdit m_nxeditCCDATo;
	CNxEdit m_nxeditCCDAMessage;
	CNxColor m_nxcolorBackgroundTop;
	CNxColor m_nxcolorBackgroundBottom;

	// (b.spivey, May 13th, 2014) - PLID 61804 - mutator for patient ID 
	void SetPatientID(long nPatientID); 
	
// inherited Fucntions
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// Dialog Data
	enum { IDD = IDD_DIRECTMESSAGE_SEND };

//
	//void AttachFilesToMessage();// (j.camacho 2013-11-05 11:23) - PLID 59303
	void AddToAttachments(CString attachmentPath,CString fileName);// (j.camacho 2013-11-05 09:14) - PLID 59303 
	void AddToAttachments(Nx::SafeArray<BYTE> fileBytes, CString fileName);// (j.gruber 2013-11-11 09:19) - PLID 59403 - overload 
	void AddToAttachments(CString attachmentPath,CString fileName, long nMailID); // (d.singleton 2014-04-23 17:30) - PLID 61806

protected:
	// (a.walling 2014-04-24 12:00) - VS2013 - no using std in global headers
	std::vector<attachmentFile> m_vAttachments;// (j.camacho 2013-11-05 09:14) - PLID 59303
	void CDirectMessageSendDlg::LoadAttachmentsList();// (j.camacho 2013-11-06 15:25) - PLID 59303
	void DisableControls(); // (b.spivey, February 5th, 2014) - PLID 60648 - Disable the controls except the cancel button.

	// (b.spivey, May 13th, 2014) - PLID 61804 - functions to attach files to a direct message. 
	void AttachExistingFile();
	void AttachNewFile(); 
	afx_msg void OnBnClickedAttachFiles();

	// (b.spivey, May 14th, 2014) PLID 62163 
	void RemoveFromAttachments(CString strFileName);

	long m_nPatientID; 

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr m_dlDirectmessageFrom;
	NXDATALIST2Lib::_DNxDataListPtr m_dlAttachments;
	NXDATALIST2Lib::_DNxDataListPtr m_dlRefAddress;

	// (b.spivey, May 14th, 2014) PLID 62163
	void RButtonDownDirectMessageAttachments(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void SelChangingDirectmessageSendFrom(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingDirectMessageSendRefaddress(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenDirectMessageSendRefaddress(LPDISPATCH lpRow); // (j.camacho 2013-11-13 15:51) - PLID 59444
	afx_msg void OnBnClickedDirectmessageToHelp();
};

