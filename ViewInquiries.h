#if !defined(AFX_VIEWINQUIRIES_H__111285E5_2F6C_43CB_86FA_864C4BF297D5__INCLUDED_)
#define AFX_VIEWINQUIRIES_H__111285E5_2F6C_43CB_86FA_864C4BF297D5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewInquiries.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewInquiries dialog

class CViewInquiries : public CNxDialog
{
// Construction
public:
	CViewInquiries(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CViewInquiries)
	enum { IDD = IDD_VIEW_INQUIRIES };
	CNxIconButton	m_btnPreviewReport;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxIconButton	m_btnConvertToPatient;
	CNxIconButton	m_btnConvertToProspect;
	CNxIconButton	m_btnNewInquiry;
	CNxIconButton	m_btnDeleteInquiry;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewInquiries)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL ConvertToDifferentStatus(long nPersonID, long nNewStatus);

	NXDATALISTLib::_DNxDataListPtr m_dlInquiryList;
	NXDATALISTLib::_DNxDataListPtr m_dlReferralFilter;
	NXDATALISTLib::_DNxDataListPtr m_dlProcedureFilter;
	NXDATALISTLib::_DNxDataListPtr m_dlLocationFilter;
	

	void FillProcedureColumn();
	void SetFilter();

	// (j.gruber 2008-06-03 13:01) - PLID 25447 - Added the getfilter function
	CString GetFilter();
	
	// Generated message map functions
	//{{AFX_MSG(CViewInquiries)
	virtual BOOL OnInitDialog();
	afx_msg void OnConvertToPatient();
	afx_msg void OnConvertToProspect();
	afx_msg void OnRButtonDownInquiryList(long nRow, short nCol, long x, long y, long nFlags);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnNewInquiry();
	afx_msg void OnChangeFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteInquiry();
	afx_msg void OnSelChosenProcedureFilter(long nRow);
	afx_msg void OnSelChosenReferralFilter(long nRow);
	afx_msg void OnSelChosenInquiryLocationFilter(long nRow);
	afx_msg void OnPreviewInquiry();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWINQUIRIES_H__111285E5_2F6C_43CB_86FA_864C4BF297D5__INCLUDED_)
