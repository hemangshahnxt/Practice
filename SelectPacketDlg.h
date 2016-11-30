#if !defined(AFX_SELECTPACKETDLG_H__BB4D0615_9FB6_4172_85FA_EA3ED7403576__INCLUDED_)
#define AFX_SELECTPACKETDLG_H__BB4D0615_9FB6_4172_85FA_EA3ED7403576__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectPacketDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CSelectPacketDlg dialog

class CSelectPacketDlg : public CNxDialog
{
// Construction
public:
	CSelectPacketDlg(CWnd* pParent);   // standard constructor
	int m_nPacketID;
	BOOL m_bReverseMerge;
	BOOL m_bChooseOnly; // (c.haag 2004-03-23 15:04) - TRUE if we invoke this without
						// actually wanting to merge anything
	BOOL m_bSeparateDocuments;

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CSelectPacketDlg)
	enum { IDD = IDD_SELECT_PACKET };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnMerge;
	NxButton	m_btnReverseMerge;
	NxButton	m_btnSeparateDocuments;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectPacketDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pPacketList;

	// Generated message map functions
	//{{AFX_MSG(CSelectPacketDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnConfigPackets();
	afx_msg void OnSelChosenPacketList(long nRow);
	afx_msg void OnRequeryFinishedPacketList(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTPACKETDLG_H__BB4D0615_9FB6_4172_85FA_EA3ED7403576__INCLUDED_)
