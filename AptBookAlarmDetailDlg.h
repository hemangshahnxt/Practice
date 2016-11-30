#if !defined(AFX_APTBOOKALARMDETAILDLG_H__8A6F8592_D328_4D18_B492_93222AB13344__INCLUDED_)
#define AFX_APTBOOKALARMDETAILDLG_H__8A6F8592_D328_4D18_B492_93222AB13344__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AptBookAlarmDetailDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmDetailDlg dialog

class CAptBookAlarmDetailDlg : public CNxDialog
{
// Construction
public:
	CAptBookAlarmDetailDlg(CWnd* pParent);   // standard constructor

	long m_AptTypeID, m_AptPurposeID;

	NXDATALISTLib::_DNxDataListPtr m_AptTypeList, m_AptPurposeList;

	// (z.manning, 04/30/2008) - PLID 29845 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CAptBookAlarmDetailDlg)
	enum { IDD = IDD_APT_BOOK_ALARM_DETAIL_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAptBookAlarmDetailDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAptBookAlarmDetailDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenChooseType(long nRow);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APTBOOKALARMDETAILDLG_H__8A6F8592_D328_4D18_B492_93222AB13344__INCLUDED_)
