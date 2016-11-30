#if !defined(AFX_REPORTVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
#define AFX_REPORTVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdminView.h : header file
//

#include "NxTabView.h"
#include "Reports.h"
#include "ExternalForm.h"
//TES 1/2/2008 - PLID 28000 - VS2008 - Attempting to test 28000, I discovered that this is dead code.
//#include "EditReportBatch.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code


/////////////////////////////////////////////////////////////////////////////
// CReportView view

class CReportView : public CNxView
{
public:
	CReportView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CReportView)
	//virtual void OnSize(UINT nType, int cx, int cy);

// Attributes
public:


// Operations
public:
	BOOL CheckPermissions();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	
	// (a.walling 2011-08-04 14:36) - PLID 44788 - Get the active CNxDialog sheet - Also we don't necessarily want a const pointer
	virtual CNxDialog* GetActiveSheet()
	{
		return reinterpret_cast<CNxDialog*>(&m_frame);
	}

	virtual const CNxDialog* GetActiveSheet() const
	{
		return reinterpret_cast<const CNxDialog*>(&m_frame);
	}

	CReports m_frame;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CReportView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
		
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the frame to get the active tab
	virtual short GetActiveTab();
	// (c.haag 2009-01-12 16:45) - PLID 32683 - Sets the active tab in the reports sheet.
	// Returns TRUE on success, and FALSE on failure.
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Renamed
	virtual BOOL SetActiveTab(short tab);
	//BOOL SetActiveReportTab(ReportsModule::Tab tab);

	// (c.haag 2009-01-12 16:56) - PLID 32683 - Clears the current report batch
	void ClearReportBatch();

	// (c.haag 2009-01-12 16:56) - PLID 32683 - Adds a report to the current batch
	void AddReportToBatch(long nReportID);

	// Generated message map functions
protected:
	//{{AFX_MSG(CReportView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPrint();
	afx_msg void OnPrintPreview();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADMINVIEW_H__E6E05066_968F_11D2_80F4_00104B2FE914__INCLUDED_)
