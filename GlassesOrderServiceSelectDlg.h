#pragma once
#include "InventoryRc.h"
#include "InvVisionWebUtils.h"
#include <set>

// (j.dinatale 2013-04-03 15:24) - PLID 56075 - Created

// CGlassesOrderServiceSelectDlg dialog
namespace GOServiceList {
	enum Columns {
		ID = 0,
		Selected = 1,
		Code = 2,
		Name = 3,
	};
};

typedef std::set<long> GlassesOrderServices;

class CGlassesOrderServiceSelectDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CGlassesOrderServiceSelectDlg)

public:
	CGlassesOrderServiceSelectDlg(OrderServiceListType::ServiceListType slt, long nID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CGlassesOrderServiceSelectDlg();
	virtual BOOL OnInitDialog();

	std::set<long> GetSelectedServices();

// Dialog Data
	enum { IDD = IDD_GLASSES_ORDER_SELECT_SERVICES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_pServiceList;
	CNxIconButton m_btnOK;

	DECLARE_MESSAGE_MAP()

private:
	OrderServiceListType::ServiceListType m_sltListType;	// the list type (i.e. designs/materials/treatments)
	long m_nID;	// the ID of the design/material/treatment we wish to get services for
	std::set<long> m_setSelectedServiceIDs;

	CString GetTableName(OrderServiceListType::ServiceListType slt);
	CString GetColumnName(OrderServiceListType::ServiceListType slt);
	CString GenerateListSql();

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
