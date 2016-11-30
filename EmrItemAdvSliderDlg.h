#if !defined(AFX_EMRITEMADVSLIDERDLG_H__260C069D_60EB_462F_B894_3E854B3AC22A__INCLUDED_)
#define AFX_EMRITEMADVSLIDERDLG_H__260C069D_60EB_462F_B894_3E854B3AC22A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrItemAdvSliderDlg.h : header file
//

#include "EmrItemAdvDlg.h"

//TES 1/29/2008 - PLID 28673 - Define IDCs for any controls we create.
#define SLIDER_IDC	1000
/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvSliderDlg dialog

class CEmrItemAdvSliderDlg : public CEmrItemAdvDlg
{
// Construction
public:
	CEmrItemAdvSliderDlg(class CEMNDetail *pDetail);


	CNxSliderCtrl m_Slider;
	CNxStatic	m_Caption;
	CNxStatic m_MinCaption;
	CNxStatic m_MaxCaption;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrItemAdvSliderDlg)
	//}}AFX_VIRTUAL

public:
	virtual void ReflectCurrentState();
	//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
	// SetReadOnly() to ReflectReadOnlyStatus()
	virtual void ReflectReadOnlyStatus(BOOL bReadOnly);
	virtual void ReflectCurrentContent();
	virtual void DestroyContent();

public:
	virtual BOOL RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly);

// Implementation
protected:	

	int ValueToSliderPos(double dValue);
	double SliderPosToValue(int nSliderPos);

	// Generated message map functions
	//{{AFX_MSG(CEmrItemAdvSliderDlg)
	afx_msg void OnChangeEdit();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRITEMADVSLIDERDLG_H__260C069D_60EB_462F_B894_3E854B3AC22A__INCLUDED_)
