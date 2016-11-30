#include "commondialog.h"
#if !defined(AFX_MEDSCHEDDETAIL_H__96D996F6_CA88_43D8_9EAA_D58A0C7823DB__INCLUDED_)
#define AFX_MEDSCHEDDETAIL_H__96D996F6_CA88_43D8_9EAA_D58A0C7823DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MedSchedDetail.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMedSchedDetail dialog

class CMedSchedDetail : public CNxDialog
{
// Construction
public:
	void UpdateNotes(CString strName, long Type);
	void ChangeMethodLabels();
	void ChangeDurationLabels();
	void DeleteDetail();

	CRect m_rcStopDayLabel1, m_rcStopDayLabel2, m_rcStopMethod;
	CRect m_rcStdStopDayLabel1, m_rcStdStopDayLabel2, m_rcStdStopMethod;
	void DrawStopDayLabel(CDC *pdc);
	void DrawStopMethodLabel(CDC *pdc);

	BOOL Save();
	void Load();
	void DoClickHyperlink(UINT nFlags, CPoint point);
	CMedSchedDetail(CWnd* pParent);   // standard constructor

public:
	long m_DetailID,
		 m_MedSchedID;
	// (d.singleton 2012-06-08 11:35) - PLID 50442
	BOOL m_bIsPreOpSchedule;

	NXDATALISTLib::_DNxDataListPtr m_DetailType,
					m_Med_Combo,
					m_Event_Combo;

	COleDateTime m_dtStartDate;
	BOOL m_bIsBeforeAppt;

	int iStopMethod,	//1 - Last note on Stop Day, 2 - Last note on day after Stop Day
		iDurationType,	//1 - Run until Stop Day, 2 - Run for [Stop Day] days
		iApptDurationType; //1 - days before appt, 2 - days after appt

// Dialog Data
	//{{AFX_DATA(CMedSchedDetail)
	enum { IDD = IDD_MEDSCHEDDETAIL_DLG };
	CCommonDialog60	m_ctrlColorPicker;
	CNxColor	m_color;
	CString	m_strStopDayLabel1;
	CString	m_strStopDayLabel2;
	CString	m_strStopDayMethod;
	CNxEdit	m_nxeditMedscheddetailName;
	CNxEdit	m_nxeditMeddetailStartDay;
	CNxEdit	m_nxeditMeddetailStopDay;
	CNxEdit	m_nxeditMeddetailStartNote;
	CNxEdit	m_nxeditMeddetailMiddleNote;
	CNxEdit	m_nxeditMeddetailStopNote;
	CNxStatic	m_nxstaticStopDayLabel;
	CNxStatic	m_nxstaticStopDayLabel2;
	CNxStatic	m_nxstaticStopDayMethod;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnDeleteDetail;
	CNxIconButton	m_btnWritePrescription;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMedSchedDetail)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMedSchedDetail)
	afx_msg void OnClickTypeColor();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenMedscheddetailTypeCombo(long nRow);
	afx_msg void OnBtnEditDetailList();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnDeleteDetail();
	afx_msg void OnSelChosenMedscheddetailMedCombo(long nRow);
	afx_msg void OnSelChosenMedscheddetailEventCombo(long nRow);
	afx_msg void OnBtnWritePrescription();
	afx_msg void OnTrySetSelFinishedMedscheddetailMedCombo(long nRowEnum, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void RequeryFinishedMedscheddetailMedCombo(short nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDSCHEDDETAIL_H__96D996F6_CA88_43D8_9EAA_D58A0C7823DB__INCLUDED_)
