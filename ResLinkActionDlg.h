#if !defined(AFX_RESLINKACTIONDLG_H__04D77968_A0D8_4E8D_B93C_0BDA06D79445__INCLUDED_)
#define AFX_RESLINKACTIONDLG_H__04D77968_A0D8_4E8D_B93C_0BDA06D79445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResLinkActionDlg.h : header file
//

#include "schedulerrc.h"

/////////////////////////////////////////////////////////////////////////////
// CResLinkActionDlg dialog

class CResLinkActionDlg : public CNxDialog
{
// Construction
public:
	long m_nDurationChange;
	long m_nActiveLinkedID;
	COleDateTime m_dtPivot;

	int Open(long nResID);
	BOOL LinkedApptsViolateTemplates();
	BOOL MoveLinkedAppointments();
	void Load();
	void SetAppointmentColors();
	void SetDays();

	typedef enum {
		eMoveLinkedAppts = 0,
		eMakeTodo = 1,
		eDoNothing = 2,
	} ERecourse;

	CResLinkActionDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResLinkActionDlg)
	enum { IDD = IDD_RES_LINK_ACTION_DLG };
	int		m_iRecourse;
	CNxIconButton m_btnOK;
	NxButton	m_radioMoveAll;
	NxButton	m_radioCreateTodo;
	NxButton	m_radioDoNothing;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResLinkActionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr	m_dlAptList;
	long m_nResID;

	virtual void OnOK();
	virtual void OnCancel();

	// Generated message map functions
	//{{AFX_MSG(CResLinkActionDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedAppointments(short nFlags);
	afx_msg void OnSelChangedAppointments(long nNewSel);
	afx_msg void OnDblClickCellAppointments(long nRowIndex, short nColIndex);
	afx_msg void OnRButtonDownAppointments(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGoAppointment();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESLINKACTIONDLG_H__04D77968_A0D8_4E8D_B93C_0BDA06D79445__INCLUDED_)
