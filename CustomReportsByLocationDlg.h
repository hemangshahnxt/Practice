#pragma once

//TES 2/1/2010 - PLID 37143 - Created.  At the moment this is only used for the Lab Request Form, but it was designed to be extendable.
// CCustomReportsByLocationDlg dialog

class CCustomReportsByLocationDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CCustomReportsByLocationDlg)

public:
	CCustomReportsByLocationDlg(CWnd* pParent);   // standard constructor
	virtual ~CCustomReportsByLocationDlg();

	//TES 2/1/2010 - PLID 37143 - Set this before opening (at the moment it must be 658, the Lab Request Form).
	//TES 7/27/2012 - PLID 51849 - Added support for 567, Lab Results Form
	long m_nReportID;
// Dialog Data
	enum { IDD = IDD_CUSTOM_REPORTS_BY_LOCATION_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	CNxStatic m_nxsCaption;

	CNxIconButton m_nxbOK, m_nxbCancel;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOK();
};
