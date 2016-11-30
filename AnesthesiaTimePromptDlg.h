#if !defined(AFX_ANESTHESIATIMEPROMPTDLG_H__227599A9_619D_46E9_9D08_8CB9BE7E5C00__INCLUDED_)
#define AFX_ANESTHESIATIMEPROMPTDLG_H__227599A9_619D_46E9_9D08_8CB9BE7E5C00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnesthesiaTimePromptDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaTimePromptDlg dialog

// (j.jones 2010-11-22 17:19) - PLID 39602 - added enum for which time we are prompting for
enum TimePromptType {

	tptAnesthesia = 0,
	tptFacilityFee,
	tptAssistingCode,
};

class CAnesthesiaTimePromptDlg : public CNxDialog
{
// Construction
public:
	CAnesthesiaTimePromptDlg(CWnd* pParent);   // standard constructor

	NXTIMELib::_DNxTimePtr	m_nxtStart, m_nxtEnd;

	long m_nServiceID;

	long m_nMinutes;

	CString m_strStartTime, m_strEndTime;

	// (j.jones 2010-11-22 17:19) - PLID 39602 - added enum for which time we are prompting for,
	// can be anesthesia, facility, or assisting code
	TimePromptType m_eTimePromptType;

// Dialog Data
	//{{AFX_DATA(CAnesthesiaTimePromptDlg)
	enum { IDD = IDD_ANESTHESIA_TIME_PROMPT_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticStartLabel;
	CNxStatic	m_nxstaticEndLabel;
	CNxStatic	m_nxstaticCodeLabel;
	CNxStatic	m_nxstaticTopLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnesthesiaTimePromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnesthesiaTimePromptDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANESTHESIATIMEPROMPTDLG_H__227599A9_619D_46E9_9D08_8CB9BE7E5C00__INCLUDED_)
