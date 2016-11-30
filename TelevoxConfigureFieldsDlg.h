#if !defined(AFX_TELEVOXCONFIGUREFIELDSDLG_H__7FADD2B4_C40A_4662_8276_CFAC1F21CAB7__INCLUDED_)
#define AFX_TELEVOXCONFIGUREFIELDSDLG_H__7FADD2B4_C40A_4662_8276_CFAC1F21CAB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TelevoxConfigureFieldsDlg.h : header file
//

// (z.manning 2008-07-10 12:37) - PLID 20543 - Created
/////////////////////////////////////////////////////////////////////////////
// CTelevoxConfigureFieldsDlg dialog

class CTelevoxConfigureFieldsDlg : public CNxDialog
{
// Construction
public:
	CTelevoxConfigureFieldsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTelevoxConfigureFieldsDlg)
	enum { IDD = IDD_TELEVOX_CONFIGURE_FIELDS };
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTelevoxConfigureFieldsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pdlFields;
	enum FieldListColumns
	{
		flcCheck = 0,
		flcID,
		flcDisplayName,
		flcSortOrder,
	};

	void UpdateButtons();
	void SwapRows(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);

	// Generated message map functions
	//{{AFX_MSG(CTelevoxConfigureFieldsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTelevoxFieldUp();
	afx_msg void OnTelevoxFieldDown();
	afx_msg void OnEditingFinishedTelevoxFieldList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangedTelevoxFieldList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TELEVOXCONFIGUREFIELDSDLG_H__7FADD2B4_C40A_4662_8276_CFAC1F21CAB7__INCLUDED_)
