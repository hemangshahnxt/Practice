#pragma once
#include "ReportsRc.h"
// (r.gonet 06/12/2013) - PLID 55151 - Removed a header and added a forward declaration.
enum CCHITReportConfigType;

// CCCHITReportsConfigOptionsDlg dialog
// (d.thompson 2010-01-20) - PLID 36927

class CCCHITReportsConfigOptionsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCCHITReportsConfigOptionsDlg)

public:
	// (j.gruber 2010-09-10 14:36) - PLID 40487 - added type
	CCCHITReportsConfigOptionsDlg(CWnd* pParent, CString strReportName, CCHITReportConfigType crctType);   // standard constructor
	virtual ~CCCHITReportsConfigOptionsDlg();

// Dialog Data
	enum { IDD = IDD_CCCHIT_REPORTS_CONFIG_OPTIONS_DLG };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//Data members
	CString m_strReportName;		//Name of the report we're configuring, used to save data.
	CCHITReportConfigType m_crctType; // (j.gruber 2010-09-10 14:36) - PLID 40487

	//Controls
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxStatic m_stBottomLabel;
	NXDATALIST2Lib::_DNxDataListPtr m_pItemList;
	NXDATALIST2Lib::_DNxDataListPtr m_pItemList2; // (j.gruber 2010-09-14 09:34) - PLID 40514 - 2 Item Lists
	NXDATALIST2Lib::_DNxDataListPtr m_pDataList;
	NXDATALIST2Lib::_DNxDataListPtr m_pCatList; // (j.gruber 2010-09-21 13:55) - PLID 40617

	//Functionality
	void LoadDataElements();

	// (j.gruber 2010-09-10 14:43) - PLID 40487 - hide the bottom box in certain circumstances
	void HideBottomBox();

	// (j.gruber 2010-09-21 13:56) - PLID 40617 - show the category list
	void ShowCatList();

	// (j.gruber 2010-09-14 09:30) - PLID 40514 - created for
	void ShowSecondItemList();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void SelChangedCchitEmrItemList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
