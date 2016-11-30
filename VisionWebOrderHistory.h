#pragma once
#include "InventoryRc.h"
// CVisionWebOrderHistory dialog

// (s.dhole 2010-11-08 17:30) - PLID 41384 new DLG
class CVisionWebOrderHistory : public CNxDialog
{
	DECLARE_DYNAMIC(CVisionWebOrderHistory)

public:
	CVisionWebOrderHistory(CWnd* pParent);   // standard constructor
	virtual ~CVisionWebOrderHistory();
	CNxIconButton	m_BtnVisionWebOrderHistoryClose;
// Dialog Data
	enum { IDD = IDD_VISIONWEB_ORDER_HISTORY };
	long m_nVisionWebOrderID;
	CString  m_strVisionWebOrderID;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	NXDATALIST2Lib::_DNxDataListPtr m_VisionWebOrderHistoryList;
	DECLARE_MESSAGE_MAP()
};
