#if !defined(AFX_EMRMERGEPRECEDENCEDLG_H__2D079503_15B4_4F67_B778_D0F99D0BB58C__INCLUDED_)
#define AFX_EMRMERGEPRECEDENCEDLG_H__2D079503_15B4_4F67_B778_D0F99D0BB58C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRMergePrecedenceDlg.h : header file
//
#include "patientsrc.h"

/////////////////////////////////////////////////////////////////////////////
// CEMRMergePrecedenceDlg dialog

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_MERGEPREC_SELECTTEMPLATE		33767
#define ID_MERGEPREC_UNSELECTTEMPLATE	33768

class CEMRMergePrecedenceDlg : public CNxDialog
{
// Construction
public:
	CEMRMergePrecedenceDlg(CWnd* pParent);   // standard constructor

	bool b_OutOfEMR;

// Dialog Data
	//{{AFX_DATA(CEMRMergePrecedenceDlg)
	enum { IDD = IDD_EMR_MERGE_PRECEDENCE_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticWordtemp;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnOK2Merge;
	CNxIconButton m_btnCancel2Merge;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRMergePrecedenceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlTemplates;
	NXDATALISTLib::_DNxDataListPtr m_dlCollectionTemplates;
	void Load();
	void Save();

	// (m.hancock 2006-05-22 11:06) - PLID 20747 - Keep track of which rows in the datalists have changed.  This speeds up saving.
	CDWordArray m_adwModTemplates;
	CDWordArray m_adwModCollectionTemplates;
	bool AddToArray(CDWordArray &adwArray, long nRow);

	// Generated message map functions
	//{{AFX_MSG(CEMRMergePrecedenceDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnMenuSelectTemplate();
	afx_msg void OnMenuUnselectTemplate();
	afx_msg void OnEditingFinishedListTemplates(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnLButtonDownListCollectiontemplates(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownListCollectiontemplates(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnOk2();
	afx_msg void OnCancel2merge();
	afx_msg void OnOk2merge();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRMERGEPRECEDENCEDLG_H__2D079503_15B4_4F67_B778_D0F99D0BB58C__INCLUDED_)
