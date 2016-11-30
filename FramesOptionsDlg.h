#pragma once
#include "InventoryRc.h"

// CFramesOptionsDlg dialog

// (j.gruber 2010-06-24 09:56) - PLID 39314 - created
class CFramesOptionsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CFramesOptionsDlg)

public:
	CFramesOptionsDlg(CWnd* pParent);   // standard constructor
	virtual ~CFramesOptionsDlg();

protected:
	NxButton	m_radioNotTrackable;
	NxButton	m_radioTrackOrders;
	NxButton	m_radioTrackQuantity;
	NxButton	m_taxable2;	
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	NxButton	m_taxable;
	NxButton	m_billable;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pCatList;


// Dialog Data
	enum { IDD = IDD_FRAMES_OPTIONS_DLG };

protected:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnChangeBillable();
	afx_msg void OnRadioNotTrackableItem();
	afx_msg void OnRadioTrackOrdersItem();
	afx_msg void OnRadioTrackQuantityItem();	

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedFrOpCatList(short nFlags);
};


