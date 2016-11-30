#if !defined(AFX_EDITREPORTPICKERDLG_H__3307918B_2174_4079_A656_B41952233C44__INCLUDED_)
#define AFX_EDITREPORTPICKERDLG_H__3307918B_2174_4079_A656_B41952233C44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditReportPickerDlg.h : header file
//
#include "ReportInfo.h"
/////////////////////////////////////////////////////////////////////////////
// CEditReportPickerDlg dialog

class CEditReportPickerDlg : public CNxDialog
{
private:
	// (r.gonet 11/23/2011) - PLID 46437 - Added in an enumeration for the datalist columns since it didn't exist before.
	enum EReportPickerListColumns
	{
		erpclNumber = 0,
		erpclDetailOption,
		erpclDateOption,
		erpclAveryStyle,
		erpclName,
		erpclGenerateBarcode,
	};
// Construction
public:
	CEditReportPickerDlg(CWnd* pParent);   // standard constructor
	CEditReportPickerDlg(CWnd* pParent, CReportInfo *CurrReport);   // takes a pointer to a report info object
	NXDATALISTLib::_DNxDataListPtr  m_ReportPicker;
	bool GetReportFileName(long nRow, CString &strFileName);
	CReportInfo *m_CurrReport;
	void SetDefaultColor();
	long m_nDefault;
	
	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CEditReportPickerDlg)
	enum { IDD = IDD_EDITREPORTPICKER };
	CNxIconButton	m_btnNewReport;
	CNxIconButton	m_btnEditReport;
	CNxIconButton	m_btnDeleteReport;
	CNxIconButton	m_btnMakeDefault;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRevert;
	CNxIconButton	m_btnExtra;
	// (r.gonet 10/11/2011) - PLID 46437 - Let the report have a custom checkbox that can do different things depending on the report type.
	NxButton		m_checkExtra;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditReportPickerDlg)
	public:
	virtual int DoModal(long nID);
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Goes through and adds each possible option as a record in the datalist, for the given date option.
	void AddStandardRecords(long nDateOption, CString strDateName);
	//Returns success or failure.
	bool OpenReportEditor(OUT bool &bIsSaved, OUT bool &bIsCustom, OUT CString &strSaveFileName);

	void Refresh();

	//TES 2/1/2010 - PLID 37143 - Does this report use the "extra" button (for additional report-specific functionality)?
	bool UseExtraButton();
	// (r.gonet 10/11/2011) - PLID 46437 - Does this report use the extra checkbox (for additional report-specific functionality)?
	bool UseExtraCheckbox();

	// Generated message map functions
	//{{AFX_MSG(CEditReportPickerDlg)
	afx_msg void OnSelChangedEditreportlist(long nNewSel);
	afx_msg void OnEditreport();
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnMakedefault();
	afx_msg void OnNewreport();
	afx_msg void OnRevert();
	afx_msg void OnDelete();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnExtraBtn();
	// (r.gonet 10/11/2011) - PLID 46437
	afx_msg void OnExtraCheckbox();
	// (r.gonet 10/16/2011) - PLID 45968
	void RButtonUpEditreportlist(long nRow, short nCol, long x, long y, long nFlags);
	// (r.gonet 10/16/2011) - PLID 45968
	afx_msg void OnEditCustomReportFields();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITREPORTPICKERDLG_H__3307918B_2174_4079_A656_B41952233C44__INCLUDED_)
