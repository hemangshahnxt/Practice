#if !defined(AFX_NEXWEBNOTECHANGEDLG_H__9D725011_9089_4A76_8DCA_54A2EA2BC12C__INCLUDED_)
#define AFX_NEXWEBNOTECHANGEDLG_H__9D725011_9089_4A76_8DCA_54A2EA2BC12C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NexWebNoteChangeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNexWebNoteChangeDlg dialog

class CNexWebNoteChangeDlg : public CNxDialog
{
// Construction
public:
	CNexWebNoteChangeDlg(long nNoteID, CWnd* pParent);   // standard constructor
	NXTIMELib::_DNxTimePtr m_DataDate;
	NXTIMELib::_DNxTimePtr m_NexwebDate;
	long m_nNoteID;
	CString m_strDataNote;
	COleDateTime m_dtDataDate;
	BOOL CompareDates(COleDateTime dt1, COleDateTime dt2);

// Dialog Data
	//{{AFX_DATA(CNexWebNoteChangeDlg)
	enum { IDD = IDD_NEXWEB_NOTE_CHANGE_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditDataNote;
	CNxEdit	m_nxeditNexwebNote;
	NxButton	m_btnNoteGroupbox;
	NxButton	m_btnChangeGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNexWebNoteChangeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNexWebNoteChangeDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEXWEBNOTECHANGEDLG_H__9D725011_9089_4A76_8DCA_54A2EA2BC12C__INCLUDED_)
