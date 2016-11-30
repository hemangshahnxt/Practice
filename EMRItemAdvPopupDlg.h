#if !defined(AFX_EMRITEMADVPOPUPDLG_H__6FAE1044_2960_4046_A01F_172F64497FFA__INCLUDED_)
#define AFX_EMRITEMADVPOPUPDLG_H__6FAE1044_2960_4046_A01F_172F64497FFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRItemAdvPopupDlg.h : header file
//


// (a.walling 2009-04-03 09:29) - PLID 33831 - Let's just define these in one place, emrutils.h
/*
#define LIST_TYPE_TEXT		3
#define LIST_TYPE_DROPDOWN	4
#define LIST_TYPE_CHECKBOX	5
#define LIST_TYPE_LINKED	6
*/
#include "emrutils.h"
#include "EmrItemAdvImageDlg.h"
#include "EMRItemAdvPopupWnd.h"

// (a.walling 2010-05-19 08:26) - PLID 38558 - Include NxInkPictureImport.h rather than #import "NxInkPicture.tlb" so the proper tlb is chosen based on current SDK path (patch or main)
#include "NxInkPictureImport.h"
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXINKPICTURELib;

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvPopupDlg dialog

class CEMRItemAdvPopupDlg : public CNxDialog
{
// Construction
public:
	CEMRItemAdvPopupDlg(CWnd* pParent);   // standard constructor
	~CEMRItemAdvPopupDlg();

protected:
	CEMNDetail *m_pDetail; //This dialog assumes that m_pDetail is an independently loaded copy, so, for example, it will lock spawning and never unlock it.
	//TES 4/6/2007 - PLID 25456 - The above comment is no longer quite true; in the case of narratives, m_pDetail is NOT
	// an independently loaded copy, so this variable must be set to FALSE so that the dialog knows not to, for example,
	// lock spawning and never unlock it.  Also, if the detail is not independent, then changes can't be cancelled, so the
	// interface will be updated appropriately.
	CEMNDetail *m_pRealDetail; // (a.walling 2008-01-18 12:15) - PLID 14982 - If the detail is independent, then this will be the real detail. Otherwise, it
	// will just be == to m_pDetail.

	BOOL m_bIsDetailIndependent;

public:
	//TES 4/6/2007 - PLID 25456 - Because I added the variable for whether the detail was independent, I didn't want people
	// setting the detail without specifying whether it was independent, so I protected that variable, it can now only
	// be set through this function.
	// (a.walling 2008-01-18 12:16) - PLID 14982 - Added parameter for the real detail
	void SetDetail(CEMNDetail *pDetail, BOOL bIsDetailIndependent, CEMNDetail* pRealDetail = NULL);
	//TES 4/6/2007 - PLID 25456 - I added this accessor for the same reason.
	CEMNDetail* GetDetail();

	long m_clrHilightColor;

	//for image
	long m_nCurPenColor;
	float m_fltCurPenSize;
	BOOL m_bIsEraserState;
	// (a.wetta 2007-04-10 10:19) - PLID 25532 - Added variable for custom stamps
	CString m_strCustomImageStamps;

	//Because this is a copy of a detail, we can't calculate the linked items, so we need this.
	CString m_strLinkedItemList;

	void ResizeWindowToControls();

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	// (a.walling 2007-08-22 17:00) - PLID 27160 - Call to reflect current state (only needed for narratives currently)
	void ReflectCurrentState();

	//TES 6/3/2008 - PLID 29098 - Like ReflectCurrentState(), currently only needed for narratives.
	void ReflectCurrentContent();

// Dialog Data
	//{{AFX_DATA(CEMRItemAdvPopupDlg)
	enum { IDD = IDD_EMR_ITEM_ADV_POPUP_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticPlaceholder;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRItemAdvPopupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// (a.walling 2012-06-07 11:59) - PLID 46649 - Use some aero glass
	virtual void UpdateCompositionFrame();
	//}}AFX_VIRTUAL

// Implementation
protected:

	CEMRItemAdvPopupWnd *m_EMRItemAdvPopupWnd;

public:
	inline CEMRItemAdvPopupWnd* GetEMRItemAdvPopupWnd() { return m_EMRItemAdvPopupWnd; }

protected:
	void CreateControlsWindow();

	//DRT 4/30/2008 - PLID 29771
	CBrush m_brBackground;
	
	// (z.manning 2010-05-05 16:45) - PLID 38503 - Moved some repeated code to its own function
	void UpdateRememberedPenState();
	
	// (a.walling 2009-01-12 12:35) - PLID 29800 - Get the image properties regardless of whether the popup was cancelled or not
	// Generated message map functions
	//{{AFX_MSG(CEMRItemAdvPopupDlg)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg LRESULT OnPopupResized(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostStateChanged(WPARAM wParam, LPARAM lParam);
	// (a.walling 2012-06-07 11:59) - PLID 46649 - Use some aero glass
	afx_msg LRESULT OnPrintClientInRect(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMADVPOPUPDLG_H__6FAE1044_2960_4046_A01F_172F64497FFA__INCLUDED_)
