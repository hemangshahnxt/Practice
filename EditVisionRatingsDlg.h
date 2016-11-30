#if !defined(AFX_EDITVISIONRATINGSDLG_H__4D1C7E06_2E85_40E8_86CC_7592B9731BA0__INCLUDED_)
#define AFX_EDITVISIONRATINGSDLG_H__4D1C7E06_2E85_40E8_86CC_7592B9731BA0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditVisionRatingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditVisionRatingsDlg dialog

class CEditVisionRatingsDlg : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_pRatingList;

	CEditVisionRatingsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditVisionRatingsDlg)
	enum { IDD = IDD_VISION_RATING_DLG };
	CNxIconButton	m_btnDown;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditVisionRatingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nSelectedID;

	// Generated message map functions
	//{{AFX_MSG(CEditVisionRatingsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEyeRatingDown();
	afx_msg void OnEyeRatingUp();
	afx_msg void OnEditingFinishedVisionRatings(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedVisionRatings(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITVISIONRATINGSDLG_H__4D1C7E06_2E85_40E8_86CC_7592B9731BA0__INCLUDED_)
