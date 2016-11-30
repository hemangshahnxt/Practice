#include "PatientDialog.h"
#if !defined(AFX_CUSTOMRECORDSTABDLG_H__942E3668_0182_4BBC_B945_02ADB9DDBE29__INCLUDED_)
#define AFX_CUSTOMRECORDSTABDLG_H__942E3668_0182_4BBC_B945_02ADB9DDBE29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomRecordsTabDlg.h : header file
//

class CBillingModuleDlg;

/////////////////////////////////////////////////////////////////////////////
// CustomRecordsTabDlg dialog

class CCustomRecordsTabDlg : public CPatientDialog
{
// Construction
public:
	long m_ID; //member patient ID

	NXDATALISTLib::_DNxDataListPtr m_EMRList;

	CCustomRecordsTabDlg(CWnd* pParent);   // standard constructor

	virtual void SetColor(OLE_COLOR nNewColor);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	CPatientView *m_pParent;

	CBillingModuleDlg * GetBillingDlg();
	CBillingModuleDlg *m_BillingDlg;

	//TES 12/29/2006 - PLID 23400 - This function appears to never be called.
	//void OpenEMR(int nEmrID);

	enum EmrListColumns {
		elcID = 0,
		elcProcedureID = 1,
		elcDate = 2,
		elcProcedureName = 3,
		elcProviderName = 4,
		elcPatientAge = 5,
		elcPatientGender = 6,
		elcEmrGroupID = 7,
	};

// Dialog Data
	//{{AFX_DATA(CustomRecordsTabDlg)
	enum { IDD = IDD_CUSTOM_RECORDS_TAB_DLG };
	CNxIconButton	m_btnNewEMR;
	CNxColor	m_bkg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CustomRecordsTabDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SecureControls();
	CDialog* m_pEMRDlg;		//used to keep a pointer to the emr popup dialog in case we get a table changed message

	// Generated message map functions
	//{{AFX_MSG(CustomRecordsTabDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewEmr();
	afx_msg void OnEditEmrTemplate();
	afx_msg void OnRButtonDownEmrList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellEmrList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMRECORDSTABDLG_H__942E3668_0182_4BBC_B945_02ADB9DDBE29__INCLUDED_)
