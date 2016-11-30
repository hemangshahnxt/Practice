#if !defined(AFX_REFRACTIVEDLG_H__4D75E73C_7A41_4E5F_BC11_8A62D4ABD2D0__INCLUDED_)
#define AFX_REFRACTIVEDLG_H__4D75E73C_7A41_4E5F_BC11_8A62D4ABD2D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RefractiveDlg.h : header file
//

#include "patientdialog.h"

/////////////////////////////////////////////////////////////////////////////
// CRefractiveDlg dialog

class CRefractiveDlg : public CPatientDialog
{
// Construction
public:
	CRefractiveDlg(CWnd* pParent);   // standard constructor
	void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	//{{AFX_DATA(CRefractiveDlg)
	enum { IDD = IDD_REFRACTIVE };
	CNxColor	m_EyeBG;
	CNxIconButton	m_btnNewRecord;
	CNxIconButton	m_btnDeleteRecord;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRefractiveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_SurgeryCombo;

	void EnableAppropriateButtons();

	// Generated message map functions
	//{{AFX_MSG(CRefractiveDlg)
	afx_msg void OnNewRecord();
	virtual BOOL OnInitDialog();
	afx_msg void OnEyeGraph();
	afx_msg void OnDeleteRecord();
	afx_msg void OnDblClickCellSurgeries(long nRowIndex, short nColIndex);
	afx_msg void OnSelChangedSurgeries(long nNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFRACTIVEDLG_H__4D75E73C_7A41_4E5F_BC11_8A62D4ABD2D0__INCLUDED_)
