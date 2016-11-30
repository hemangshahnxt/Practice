#pragma once

// (b.spivey, May 08, 2012) - PLID 50224 - Created. 

// CEditPatientTypeDlg dialog

class CEditPatientTypeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditPatientTypeDlg)

public:
	CEditPatientTypeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditPatientTypeDlg();

// Dialog Data
	enum { IDD = IDD_EDIT_PATIENT_TYPE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	enum PatientTypeList {
		ptlID = 0, 
		ptlName = 1, 
		ptlColorValue = 2, 
		ptlColor = 3, 
	};

	DECLARE_MESSAGE_MAP()

	DECLARE_EVENTSINK_MAP()

	NXDATALIST2Lib::_DNxDataListPtr m_dlPatientType; 
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnEdit;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnClose;

	bool IsDefaultSelected();
	void EnableAppropriateButtons(); 
	void RemoveDefaultPatientType(); 
	// (j.fouts 2012-06-13 16:31) - PLID 50863 - Use the table checker when this changes
	void RefreshTable();

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnAdd();
	afx_msg void OnEdit();
	afx_msg void OnDelete();
	afx_msg void OnEditingFinishedPatientTypeList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownPatientTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChangedPatientTypeList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	// (b.spivey, May 15, 2012) - PLID 20752 - Need to watch for a click on the color column. 
	void LeftClickPatientTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
