#if !defined(AFX_CUSTOMRECORDITEMDLG_H__0CEAFF60_EF01_4DE4_BBF5_83CB2E8C2E24__INCLUDED_)
#define AFX_CUSTOMRECORDITEMDLG_H__0CEAFF60_EF01_4DE4_BBF5_83CB2E8C2E24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomRecordItemDlg.h : header file
//

#define ITEM_OK		1
#define ITEM_NULL	2
#define	ITEM_EMPTY	3
#define ITEM_ERROR	4

/////////////////////////////////////////////////////////////////////////////
// CCustomRecordItemDlg dialog

class CCustomRecordItemDlg : public CNxDialog
{
// Construction
public:
	int GetItemInfo(long &InfoID, int &datatype, long &DataID, CString &text);
	void RefreshList(CString strNewInfoWhereClause);
	void LoadInfoID(long InfoID, BOOL bNewItem);
	void LoadDetailID(long DetailID, BOOL bNewItem);
	void SetDefaultData(long DataID);
	void ChangeInfoItem(long InfoID);

	CString m_strInfoWhereClause;

	//this item's ID in EMRDetailsT (-1 if new)
	long m_ID;

	//this item's index in the array
	long m_index;

	NXDATALISTLib::_DNxDataListPtr m_InfoCombo;
	NXDATALISTLib::_DNxDataListPtr m_DataCombo;

	CCustomRecordItemDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCustomRecordItemDlg)
	enum { IDD = IDD_CUSTOM_RECORD_ITEM_DLG };
	CNxEdit	m_Text;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomRecordItemDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nEmrInfoID;

	// Generated message map functions
	//{{AFX_MSG(CCustomRecordItemDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenInfoList(long nRow);
	afx_msg void OnBtnZoomEmrText();
	afx_msg void OnSelChosenDataList(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMRECORDITEMDLG_H__0CEAFF60_EF01_4DE4_BBF5_83CB2E8C2E24__INCLUDED_)

