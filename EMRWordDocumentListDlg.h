#if !defined(AFX_EMRWORDDOCUMENTLISTDLG_H__B0408EC6_23A8_4BEF_BB68_4C8CD68E61ED__INCLUDED_)
#define AFX_EMRWORDDOCUMENTLISTDLG_H__B0408EC6_23A8_4BEF_BB68_4C8CD68E61ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRWordDocumentListDlg.h : header file
//
//(a.wilson 2014-5-15) PLID 61808 - enum to determine what the list will contain.
enum EMRDocumentType {
	edtWord = 0,
	edtClinicalSummary = 1,
};
/////////////////////////////////////////////////////////////////////////////
// CEMRWordDocumentListDlg dialog

class CEMRWordDocumentListDlg : public CNxDialog
{
// Construction
public:
	CEMRWordDocumentListDlg(CWnd* pParent, EMRDocumentType edtType = edtWord);   

	NXDATALIST2Lib::_DNxDataListPtr m_List;

	long m_nPatientID;
	long m_nPICID;
	long m_nEMNID;
	EMRDocumentType m_edtType;

	BOOL OpenWordDocument(CString strFileName);

// Dialog Data
	//{{AFX_DATA(CEMRWordDocumentListDlg)
	enum { IDD = IDD_EMR_WORD_DOCUMENT_LIST_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticEmrDocumentLabel;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRWordDocumentListDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//(a.wilson 2014-5-15) PLID 61809 - icons for icon column.
	HICON m_hIconClinicalSummaryMerged; 
	HICON m_hIconClinicalSummaryNexWeb; 
	CBrush m_brush;

	// Generated message map functions
	//{{AFX_MSG(CEMRWordDocumentListDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblClickCellEmrWordDocumentList(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OpenSelectedDocument();
	afx_msg void OnDestroy();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRWORDDOCUMENTLISTDLG_H__B0408EC6_23A8_4BEF_BB68_4C8CD68E61ED__INCLUDED_)
