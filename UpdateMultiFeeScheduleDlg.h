#if !defined(AFX_UPDATEMULTIFEESCHEDULEDLG_H__4075B160_CD7E_45D5_A299_75B6F60BE766__INCLUDED_)
#define AFX_UPDATEMULTIFEESCHEDULEDLG_H__4075B160_CD7E_45D5_A299_75B6F60BE766__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdateMultiFeeScheduleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdateMultiFeeScheduleDlg dialog

class CUpdateMultiFeeScheduleDlg : public CNxDialog
{
// Construction
public:
	CUpdateMultiFeeScheduleDlg(CWnd* pParent);   // standard constructor

	long m_nCurrentID;
	long m_nBasedOnID;
	CString m_strCurrentSchedule;
	long m_nPercent;
	// (j.jones 2013-04-11 16:09) - PLID 56221 - supported products
	bool m_bUpdatingProducts;

	// (j.jones 2008-09-02 17:15) - PLID 24064 - added radio buttons to update fees, allowables, or both
// Dialog Data
	//{{AFX_DATA(CUpdateMultiFeeScheduleDlg)
	enum { IDD = IDD_UPDATE_MULTIFEE_SCHEDULE_DLG };
	NxButton	m_radioFeesOnly;
	NxButton	m_radioAllowablesOnly;
	NxButton	m_radioUpdateBoth;
	CNxEdit	m_nxeditPercentage;
	CNxStatic	m_nxstaticFeeName;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdateMultiFeeScheduleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_listGroups;

	// Generated message map functions
	//{{AFX_MSG(CUpdateMultiFeeScheduleDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEMULTIFEESCHEDULEDLG_H__4075B160_CD7E_45D5_A299_75B6F60BE766__INCLUDED_)
