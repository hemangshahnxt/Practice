#if !defined(AFX_CONFIGPACKETSDLG_H__42B53E6E_58C8_4D39_BB78_A548D9FB56C0__INCLUDED_)
#define AFX_CONFIGPACKETSDLG_H__42B53E6E_58C8_4D39_BB78_A548D9FB56C0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigPacketsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConfigPacketsDlg dialog

class CConfigPacketsDlg : public CNxDialog
{
// Construction
public:
	CConfigPacketsDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pExistingPackets;
	bool m_bShowProcSpecific;

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CConfigPacketsDlg)
	enum { IDD = IDD_CONFIG_PACKETS_DLG };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnAddCopy;
	CNxIconButton	m_btnEditPacket;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnDone;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigPacketsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigPacketsDlg)
	afx_msg void OnAdd();
	virtual BOOL OnInitDialog();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	virtual void OnOK();
	afx_msg void OnAddCopy();
	virtual void OnCancel();
	afx_msg void OnEditingFinishingExistingPackets(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedExistingPackets(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGPACKETSDLG_H__42B53E6E_58C8_4D39_BB78_A548D9FB56C0__INCLUDED_)
