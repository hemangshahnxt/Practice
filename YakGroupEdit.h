#if !defined(AFX_YAKGROUPEDIT_H__40C9C903_DEEE_4E64_9C67_400679A36CD3__INCLUDED_)
#define AFX_YAKGROUPEDIT_H__40C9C903_DEEE_4E64_9C67_400679A36CD3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// YakGroupEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CYakGroupEdit dialog


enum Mode
	{
		mSelect = 0,
		mEdit = 1,
	};

class CYakGroupEdit : public CNxDialog
{
// Construction
public:
	CYakGroupEdit(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CYakGroupEdit)
	enum { IDD = IDD_YAK_GROUP_EDIT };
	CNxIconButton	m_buRRight;
	CNxIconButton	m_buRight;
	CNxIconButton	m_buLLeft;
	CNxIconButton	m_buLeft;
	CNxStatic	m_nxstaticAvailableUsers;
	CNxStatic	m_nxstaticSelectedUsers;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CYakGroupEdit)
	public:
	virtual int DoModal(long nGroupID, CString strGroupName = "");
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	long m_nGroupID;
	CString m_strGroupName;
	NXDATALISTLib::_DNxDataListPtr m_dlSelected;
	NXDATALISTLib::_DNxDataListPtr m_dlAvail;
	

	// Generated message map functions
	//{{AFX_MSG(CYakGroupEdit)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRight();
	afx_msg void OnRright();
	afx_msg void OnLeft();
	afx_msg void OnLleft();
	afx_msg void OnDblClickCellAvailable(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelected(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_YAKGROUPEDIT_H__40C9C903_DEEE_4E64_9C67_400679A36CD3__INCLUDED_)
