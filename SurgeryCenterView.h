#if !defined(AFX_SURGERYCENTERVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_SURGERYCENTERVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SurgeryCenterView.h : header file
//

// (j.jones 2013-05-08 08:57) - PLID 56591 - removed the .h files for the child tabs
class CPreferenceCardsDlg;
class CCaseHistoriesDlg;
class CCredentialsDlg;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

/////////////////////////////////////////////////////////////////////////////
// CSurgeryCenterView view

class CSurgeryCenterView : public CNxTabView
{
public:
	CSurgeryCenterView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSurgeryCenterView)
// Attributes
public:
// Operations
public:
	BOOL CheckPermissions();
	virtual int Hotkey(int tab);

	int ShowPrefsDlg();

	LRESULT WarnIfStillOpen();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CContactView)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnSelectTab(short newTab, short oldTab);//used for the new NxTab
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2013-05-08 08:59) - PLID 56591 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	CPreferenceCardsDlg &m_sheetPreferenceCardSetup;	// (j.jones 2009-08-25 17:40) - PLID 35338 - this is now the preference cards dialog
	CCaseHistoriesDlg &m_sheetCaseHistories;
	CCredentialsDlg &m_sheetCredentials;

	virtual ~CSurgeryCenterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	// Generated message map functions
protected:
	void PreClose();
	//{{AFX_MSG(CContactView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_SURGERYCENTERVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
