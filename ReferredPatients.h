#if !defined(AFX_REFERREDPATIENTS_H__3EC33D11_606C_4CCD_88D0_76D57DFD8456__INCLUDED_)
#define AFX_REFERREDPATIENTS_H__3EC33D11_606C_4CCD_88D0_76D57DFD8456__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReferredPatients.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CReferredPatients dialog

class CReferredPatients : public CNxDialog
{
// Construction
public:
	CReferredPatients(CWnd* pParent);   // standard constructor
	long m_nPersonID;		// (m.hancock 2006-08-02 15:42) - PLID 21752 - Person to display referrals from
	bool m_bRefPhys;		// (m.hancock 2006-08-02 16:02) - PLID 21752 - Set true to query based on referrals from a referring physician
	NXDATALISTLib::_DNxDataListPtr	m_pPatients;
	long m_nStatus;			//1 = Patient, 2 = Prospect

// Dialog Data
	//{{AFX_DATA(CReferredPatients)
	enum { IDD = IDD_REFERRED_PATIENTS };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticTypeText;
	CNxStatic	m_nxstaticNameLabel;
	CNxIconButton m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReferredPatients)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReferredPatients)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void GoToPatient();
	afx_msg void OnRButtonDownPatients(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REFERREDPATIENTS_H__3EC33D11_606C_4CCD_88D0_76D57DFD8456__INCLUDED_)
