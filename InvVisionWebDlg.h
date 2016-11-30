#pragma once
#include "InventoryRc.h"
// CInvVisionWebDlg dialog
// (s.dhole 2010-09-24 12:48) - PLID 40538 Create a VisionWeb tab in Inventory to track orders
class CInvVisionWebDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvVisionWebDlg)

public:
	CInvVisionWebDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvVisionWebDlg();
	virtual void Refresh();
	
	NxButton	m_radioAllOrderDates;
	NxButton	m_radioOrderDateRange;
	CNxIconButton	m_NewVisionWebOrderBtn;
	CNxIconButton	m_CheckOrderStatusBtn;
	CNxIconButton	m_ChckBatchOrderStatusBtn;
	// (s.dhole 2012-05-01 08:58) - PLID 50088 remove  InvEMNToGlassesOrderDlg Code
	
	CDateTimePicker	m_OrderDateFrom;
	CDateTimePicker	m_OrderDateTo;

	// (j.dinatale 2012-02-08 14:56) - PLID 47402 - filter buttons
	NxButton m_radioFilterAll;
	NxButton m_radioFilterBilled;
	NxButton m_radioFilterNotBilled;

	// (s.dhole 2011-03-14 18:01) - PLID 42835  Dialog to Maintain Glasses Catalog custom items
	CNxIconButton	m_CatalogSetupBtn;
	void SetVisionWebDefaultValue(); // (s.dhole 2010-12-07 12:44) - PLID 41281
	void SetVisionWebProperty(LPCTSTR strPropertyName,CString  strPropertyValue);

	void SelChosenVisionwebList(LPDISPATCH lpRow);
// Dialog Data
	enum { IDD = IDD_INV_VISIONWEB_DLG };
	//  (s.dhole 2010-11-09 15:18) - PLID 40538  order status type enum
	// (s.dhole 2011-04-05 16:31) - PLID 43077 used in CInvGlassesOrderStatusDlg
	// (r.wilson 4/23/2012) PLID 43741 - Added new status 'vwOSDispensed'
	// (r.wilson 4/30/2012) PLID 43741 - Removed All of these statuses because now we get them from the database
	/*
	enum VisionWebOrderStatus{
		vwOSPending = 1,
		vwOSSubmitted,
		vwOSRejected,
		vwOSReorder,
		vwOSReceived  ,
		vwOSDeleted ,
		vwOSDispensed,
		
	};
	*/



protected:
	CWinThread* m_pThread;
	BOOL m_bBachOrderStatusChecking;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CProgressCtrl m_ctrlOrderStatusProgressBar;
	CNxStatic m_nxstaticOrderStatusProgressText;

	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebPatientCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebSupplierCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebOpticianCombo; // (j.dinatale 2012-05-02 12:27) - PLID 49758 - optician combo
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebOrderStatusCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebOrderDateFilterCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebOrderList;
	NXDATALIST2Lib::_DNxDataListPtr m_VisonWebLocationList;	// (j.dinatale 2013-04-12 14:47) - PLID 56214

	// (j.dinatale 2012-05-02 15:51) - PLID 49758
	long m_nOpticianID;

	// (s.dhole 2011-05-16 12:12) - PLID 41986 
	HBITMAP m_hBitmap;

	void CheckPendingOrderList();// (s.dhole 2011-02-01 15:10) - PLID 41397 
	void StopCheckOrderStatusThread();// (s.dhole 2011-02-01 15:10) - PLID  41397 
	
	virtual void ReFilterVisionWebOrderList();
	virtual BOOL OnInitDialog();
	//list right click menu
	// (s.dhole 2011-02-18 16:08) - PLID 40538 Addedd OnSelChangingOrderPatientCombo,OnSelChangingOrderSupplierCombo,OnSelChangingOrderStatusCombo,OnSelChangingOrderTypeCombo
	afx_msg void OnRButtonDownVisionwebOrderList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	
	//afx_msg void OnRecivedOrder();
	afx_msg void OnChangeOrderStatus1();
	afx_msg void OnCheckOrderStatus();
	afx_msg void OnCheckBatchOrderStatus();
	afx_msg void OnShowOrderStatusHistory();
	afx_msg void OnEditOrder();
	afx_msg void OnDeleteOrder(); 
	afx_msg void OnGlassesReport(); // (s.dhole 2011-05-04 17:15) - PLID  42898 Create Show Glasses report 
	afx_msg void OnGlassesReportRx(); // (s.dhole 2011-05-05 09:54) - PLID 42953 - Add Glasses Rx Report
	afx_msg void OnGoToPatient();	// (j.dinatale 2012-02-22 17:24) - PLID 48326 - go to patient plox!
	afx_msg void OnBillOrder();		// (j.dinatale 2012-03-15 09:47) - PLID 47413 - lets bill it!	

	afx_msg void OnCreateVisionWebOrder();
	afx_msg void OnClickVisionWebAccountSetup();
	afx_msg void OnRequeryFinishedPatientList(short nFlags);
	afx_msg void OnRequeryFinishedSupplierList(short nFlags);
	afx_msg void OnRequeryFinishedOrderList(short nFlags);
	afx_msg void OnRequeryFinishedOptician(short nFlags);
	afx_msg void OnSelChosenOrderPatientCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChangingOrderPatientCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel); //
	afx_msg void OnSelChosenOrderSupplierCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChangingOrderSupplierCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel); //
	afx_msg void OnSelChosenOrderStatusCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChangingOrderStatusCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel); //
	afx_msg void OnSelChosenOpticianCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChangingOpticianCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel); //
	afx_msg void OnRadioOrderAllOverviewDates();
	afx_msg void OnRadioOrderOverviewDateRange();
	afx_msg void OnChangeOrderDtOverviewFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeOrderDtOverviewTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChosenOrderDateFilterTypes(LPDISPATCH lpRow);
	afx_msg void OnSelChangingOrderDateFilterTypes(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel); //
	afx_msg LRESULT OnOrderStatusProcessData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetOrderStatusProgressMinMax(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetOrderStatusProgressPosition(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetOrderStatusProgressText(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetOrderStatusListRequeryFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnVisionWebOrderDlgClosed(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()

public:
	void DblClickCellVisionwebOrderList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedBtnGlassesCatalogSetup();  
	afx_msg void OnVisionwebOrderpopupChangeOrderStatus();
 
	afx_msg void OnBnClickedAllOrders();
	afx_msg void OnBnClickedNotBilledOrders();
	afx_msg void OnBnClickedBilledOrders();
	void SelChangingVisionWebLocationFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenVisionWebLocationFilter(LPDISPATCH lpRow);
	void RequeryFinishedVisionWebLocationFilter(short nFlags);
};
struct VisionWebOrderstruct
{
	int  nOrderId;
	CString strVisionWebOrderId;
	int nVisionWebOrderType ,nVisionWebOrderStatus;
	VisionWebOrderstruct(const int  nId,const CString &strVisionWebId,const int nOrderType ,const int nOrderStatus)
	{
	nOrderId=nId;
	strVisionWebOrderId=strVisionWebId;
	nVisionWebOrderType =nOrderType ;
	nVisionWebOrderStatus=nOrderStatus;
	};


};