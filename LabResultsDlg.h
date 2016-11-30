#pragma once

//TES 5/14/2009 - PLID 28559 - Created
// CLabResultsDlg dialog

class CLabResultsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabResultsDlg)

public:
	CLabResultsDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabResultsDlg();

	//TES 5/14/2009 - PLID 28559 - We need a patient ID and name.
	long m_nPatientID;
	CString m_strPatientName;

	CNxIconButton m_nxbClose;

	NXDATALIST2Lib::_DNxDataListPtr m_pResults;

// Dialog Data
	enum { IDD = IDD_LAB_RESULTS_DLG };

protected:
	//TES 5/15/2009 - PLID 28559 - Variables for filtering.
	short m_iFilterColumnID;
	_variant_t m_varFilterColumnData;
	BOOL m_bFiltered;

	//TES 11/11/2009 - PLID 36277 - We want to be able to save column widths.
	BOOL m_bRememberColumnWidths;
	void RestoreColumnWidths();
	void SaveColumnWidths();

	//TES 11/11/2009 - PLID 36194 - Fill the controls at the bottom of the dialog based on the currently selected result.
	void FillControls();

	//TES 11/11/2009 - PLID 36194
	CNxEdit m_nxeProcedure, m_nxeFormNumber, m_nxeUnits, m_nxeReference, m_nxeStatus, m_nxeFlag, 
		m_nxeDiagnosis, m_nxeMicroscopic;
	NxButton m_nxbRememberColumnWidths;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void OnRButtonDownLabResults(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnSelChangedLabResults(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnRequeryFinishedLabResults(short nFlags);
	afx_msg void OnRememberColumnWidths();
};
