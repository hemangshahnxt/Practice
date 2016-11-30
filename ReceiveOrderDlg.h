#if !defined(AFX_RECEIVEORDERDLG_H__F791D762_3C7D_40F5_8C07_18BEAB5DE354__INCLUDED_)
#define AFX_RECEIVEORDERDLG_H__F791D762_3C7D_40F5_8C07_18BEAB5DE354__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReceiveOrderDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReceiveOrderDlg dialog

//DRT 11/6/2007 - PLID 19682 - Created

//(e.lally 2008-03-25) PLID 29335 - we need to keep a record of the productID, serial number combinations
	//for reporting duplicate instances of a serial number for a given product.
struct ItemSerialList{
	long nProductID;
	CString strProductName;
	CString strSerialNum;
};

class CReceiveOrderDlg : public CNxDialog
{
// Construction
public:
	CReceiveOrderDlg(CWnd* pParent);   // standard constructor
	~CReceiveOrderDlg();	// (j.jones 2008-12-02 16:44) - PLID 31526 - added destructor

	void SetOrderID(long nOrderID)	{	m_nOrderID = nOrderID;	}
	//DRT 12/4/2007 - PLID 28235 - If this is set, after the dialog loads it will 'fake' a barcode
	//	scan message with this code.
	BSTR m_bstrFromBarcode;

// Dialog Data
	//{{AFX_DATA(CReceiveOrderDlg)
	enum { IDD = IDD_RECEIVE_ORDER_DLG };
	NxButton	m_btnPromptInc;
	NxButton	m_btnScanOnlyMode;
	NxButton	m_btnAutoInc;
	CNxIconButton	m_btnMarkAllReceived;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxStatic	m_nxstaticOrderPlacedDate;
	CNxStatic	m_nxstaticOrderPlacedFrom;
	CNxStatic	m_nxstaticOrderTrackingNum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReceiveOrderDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	LRESULT OnPostEditProductItemsDlg(WPARAM wParam, LPARAM lParam);

	//The ID of the OrderT record that we are marking received
	long m_nOrderID;

	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;	//Location datalist
	NXDATALIST2Lib::_DNxDataListPtr m_pList;			//Main datalist

	//Called during dialog initialization, loads the current order
	void LoadCurrentOrder();

	//Called when the user either barcodes or clicks the "qty received" column, pops up a prompt for number of items received.
	void HandleReceivedClickForCurrentRow(bool bIsBarcode);

	//Given a row in the datalist, updates the colors appropriately, depending on the qty ordered vs qty received relationship
	void UpdateColorsForRow(NXDATALIST2Lib::IRowSettingsPtr pRow);

	// (j.jones 2008-03-20 09:09) - PLID 29311 - added AppointmentID
	long m_nLinkedApptID;

	// (j.jones 2008-07-03 16:28) - PLID 30609 - store the order's location ID
	long m_nLocationID;

	//(c.copits 2010-09-14) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	NXDATALIST2Lib::IRowSettingsPtr GetBestUPCProduct(CString strCode);

	// Generated message map functions
	//{{AFX_MSG(CReceiveOrderDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnLeftClickReceiveOrderList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangingReceiveOrderLocation(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnRadReceiveOrderBarcodeInc();
	afx_msg void OnRadReceiveOrderBarcodePrompt();
	afx_msg void OnScanOnlyMode();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSelChosenReceiveOrderLocation(LPDISPATCH lpRow);
	afx_msg void OnMarkAllReceived();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RECEIVEORDERDLG_H__F791D762_3C7D_40F5_8C07_18BEAB5DE354__INCLUDED_)
