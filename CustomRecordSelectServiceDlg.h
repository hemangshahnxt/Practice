#if !defined(AFX_CUSTOMRECORDSELECTSERVICEDLG_H__5512814A_E871_4D3F_B156_9B9F2A8761F6__INCLUDED_)
#define AFX_CUSTOMRECORDSELECTSERVICEDLG_H__5512814A_E871_4D3F_B156_9B9F2A8761F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomRecordSelectServiceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordSelectServiceDlg dialog

class CCustomRecordSelectServiceDlg : public CNxDialog
{
// Construction
public:
	CCustomRecordSelectServiceDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_CPTCombo;

	long m_ServiceID;

// Dialog Data
	//{{AFX_DATA(CCustomRecordSelectServiceDlg)
	enum { IDD = IDD_CUSTOM_RECORD_SELECT_SERVICE_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomRecordSelectServiceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCustomRecordSelectServiceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMRECORDSELECTSERVICEDLG_H__5512814A_E871_4D3F_B156_9B9F2A8761F6__INCLUDED_)
