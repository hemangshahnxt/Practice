#pragma once

// COMRScanDlg dialog
// (j.dinatale 2012-08-02 16:11) - PLID 51941 - created

#include "nxtwain.h"
#import "NexTech.COM.tlb"

class COMRScanDlg : public CNxDialog
{
	DECLARE_DYNAMIC(COMRScanDlg)

public:
	COMRScanDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COMRScanDlg();

// Dialog Data
	enum { IDD = IDD_OMR_SCAN_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	// (j.dinatale 2012-08-22 15:55) - PLID 52265
	void LoadPendingNxXML();

	CNxColor m_bkgrnd;
	CNxIconButton m_btnScan, m_btnClose;
	NXDATALIST2Lib::_DNxDataListPtr m_pFormList, m_pPendingOMRs;
	CString m_strCurrentScanPath;

	// (j.dinatale 2012-08-22 11:30) - PLID 52256 - need to pass along the file path to the NxXML
	void AddOMRRow(const CString &strNxXMLFilePath);

	afx_msg void OnLeftClickPendingOMRList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
public:
	afx_msg LRESULT OnTwainXferdone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOmrScanBtn();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	void AddPendingFile(CString strFilePath);

	static void WINAPI CALLBACK OnNxTwainPreCompress(
		const LPBITMAPINFO pbmi, // 
		BOOL& bAttach, // bAttach
		BOOL& bAttachChecksum, // bAttachChecksum
		long& nChecksum, // Checksum
		ADODB::_Connection* lpCon); // Connection

	static void WINAPI CALLBACK OnTWAINCallback(NXTWAINlib::EScanType type, /* Are we scanning to the patient folder, or to another location? */
		const CString& strFileName, /* The full filename of the document that was scanned */
		BOOL& bAttach, void* pUserData, CxImage& cxImage);
	
	void SelChangingOmrScanFormList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
};
