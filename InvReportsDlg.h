#pragma once

// CInvReportsDlg dialog
// (c.haag 2009-01-12 15:21) - PLID 32683 - Initial implementation
#include "InventoryRc.h"

class CInvReportsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvReportsDlg)

public:
	CInvReportsDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvReportsDlg();

// Dialog Data
	enum { IDD = IDD_INV_REPORTS };

protected:
	// (c.haag 2009-01-12 16:32) - Opens the reports module, selects the inventory tab,
	// and selects the report defined by nReportID. If nReportID is not -1, the batch
	// is cleared and the report is added to it. If it is -1, the batch is unchanged.
	void JumpToReportsModule(long nReportID);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnConsignmentHistoryByDate();
	afx_msg void OnBnConsignmentList();
	afx_msg void OnBnConsignmentTurnRateByMonth();
	afx_msg void OnBnSerialByPt();
	afx_msg void OnBnSerialInStock();
	afx_msg void OnBnPhysicalInventory();
	afx_msg void OnBnAllocationList();
	afx_msg void OnBnGoReports();
};
