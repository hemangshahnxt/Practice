#if !defined(AFX_ACTIVEEMRDLG_H__D885F96C_824A_4139_A23F_C2108C0DA18D__INCLUDED_)
#define AFX_ACTIVEEMRDLG_H__D885F96C_824A_4139_A23F_C2108C0DA18D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ActiveEMRDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CActiveEMRDlg dialog

class CActiveEMRDlg : public CNxDialog
{
// Construction
public:
	CActiveEMRDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_ProviderCombo, m_List;

	void Refresh();

// Dialog Data
	//{{AFX_DATA(CActiveEMRDlg)
	enum { IDD = IDD_ACTIVE_EMR_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticActiveRecordCount;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CActiveEMRDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	HICON m_hIconPreview; // (a.walling 2010-01-11 12:11) - PLID 31482
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;

	// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
	// (z.manning 2012-09-10 12:31) - PLID 52543 - Added modified date
	void ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate);

	// Generated message map functions
	//{{AFX_MSG(CActiveEMRDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenEmrProviderCombo(long nRow);
	afx_msg void OnRequeryFinishedActiveEmrList(short nFlags);
	afx_msg void OnDblClickCellActiveEmrList(long nRowIndex, short nColIndex);
	virtual void OnOK();
	virtual void OnCancel();
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	afx_msg void OnDestroy();
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	void OnLeftClickActiveEmrList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACTIVEEMRDLG_H__D885F96C_824A_4139_A23F_C2108C0DA18D__INCLUDED_)
