#if !defined(AFX_CONFIGUREPATDASHDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_)
#define AFX_CONFIGUREPATDASHDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigurePatDashDlg.h : header file
//

// (j.gruber 2012-04-13 15:46) - PLID 49700 - created for

#include "PatientsRc.h"
#include "PatientDashboardDlg.h"


/////////////////////////////////////////////////////////////////////////////
// CConfigurePatDashboardDlg dialog

//SAVED IN DATA, DO NOT CHANGE!!!!
enum FilterType
{
    LastTypeFormat = 0,
    timeIncrement,    
    ApptType,        
    includeDiscontinued,
    includeResolved,
	DoNotShowOnCCDA,// (s.tullis 2015-02-25 09:53) - PLID 64740
	// (r.gonet 2015-03-17 09:43) - PLID 65020 - Added excludeDoNotShowOnProblemPrompt as a filter type
	// which applies to EMR Problem controls and causes EMR Problems that are flagged to 'Do not show on
	// problem prompt' to be excluded from the control's data.
	excludeDoNotShowOnProblemPrompt,
	HistoryCategory, // (c.haag 2015-04-29) - NX-100441
};
 
struct PDFilter
{
    FilterType filterType;
    CString strValue;
	long nValue;
	CString strDisplay;
};
//STORED IN DATA DO NOT CHANGE!!
enum PDRegion
{
    pdrLeft = 0,
    pdrRight,
    pdrCentered,
    pdrFarRight,
	pdrNone, //these are pop up divs which are hardcoded atm, so practice will never use it, but just for reference, I added it
};

struct PDControl
{
    long nID;
    long nBlock;
	long nEMRInfoMasterID;
    PatientDashboardType pdtType;
    CString strTitle;
    long nMoreIncrement;
    long nOrder;
    PDRegion pdrRegion;
    CArray<PDFilter*, PDFilter*> aryFilters;
};

//THESE EXIST IN DATA
enum LastTimeFormat
{
	ltfAll = -1,
	ltfDay = 0,
	ltfMonth,
	ltfYear,
	lftRecord, // (z.manning 2015-04-15 10:11) - NX-100388
};

class CConfigurePatDashboardDlg : public CNxDialog
{
public:
	// (c.haag 2016-05-05 15:24) - NX-100441 -  Removes missing values from dashboard category filters
	static void RemoveMissingFilterValues(PatientDashboardType dashboardType,
		FilterType filterType, const CString& strCatTable, const CString& strCatColumn);

// Construction
public:
	CConfigurePatDashboardDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigurePatDashboardDlg();

	void Load();
	void Save();

	CMap<long, long, PDControl*, PDControl*> m_controls;
	CMap<long, long, PDControl*, PDControl*> m_deletedControls;

	NXDATALIST2Lib::_DNxDataListPtr m_pAdminList;	
	NXDATALIST2Lib::_DNxDataListPtr m_pUserList;	

	void AddUserRow(PDControl *pControl, BOOL bStartEditing = FALSE, BOOL bAddRowSorted = FALSE);
	void UpdateUserRowWithControl(NXDATALIST2Lib::IRowSettingsPtr pRow, PDControl *pControl);
	void AddAdminRow(PDControl *pControl, BOOL bSetCurSel = FALSE);
	void UpdateAdminRowWithControl(NXDATALIST2Lib::IRowSettingsPtr pRow, PDControl *pControl);

	BOOL CanMoveRight(PDControl *pControl);
	BOOL MoveLeftAfterWarning(PDControl *pControl);

	void InitializeNewControl(PDControl *pControl);
	CString GetTypeNameFromID(PDControl *pControl);
	CString FormatFilters(PDControl *pControl);
	BOOL Validate();

	CNxColor m_bkg;

	// (j.gruber 2012-07-01 16:28) - PLID 51210 - diagram link
	CNxLabel m_diagramlink;

	void AddRowSorted(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void GetRowPosition(NXDATALIST2Lib::IRowSettingsPtr &pCheckRow, NXDATALIST2Lib::IRowSettingsPtr pRowToAdd, short nColumnToCheck);
	void GetRowPosition(NXDATALIST2Lib::IRowSettingsPtr &pCheckRow, NXDATALIST2Lib::IRowSettingsPtr pRowToAdd, short nColumnToCheck, short nColumnToEqual, long nValToEqual);
	void GetRowPosition(NXDATALIST2Lib::IRowSettingsPtr &pCheckRow, NXDATALIST2Lib::IRowSettingsPtr pRowToAdd, short nColumnToCheck, short nColumnToEqual, long nValToEqual, short nColumnToEqual2, long nValToEqual2);

	void ConvertStringToIntArray(CString str, CArray<int, int> &values);
	

	long m_nNextControlID;
	
// Dialog Data
	//{{AFX_DATA(CConfigurePatDashboardDlg)
	enum { IDD = IDD_CONFIGURE_PAT_DASHBOARD_DLG };		
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigurePatDashboardDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support	
	//}}AFX_VIRTUAL

// Implementation
protected:

	CNxIconButton m_btnMoveLeft;
	CNxIconButton m_btnMoveRight;
	CNxIconButton m_btnMoveUp;
	CNxIconButton m_btnMoveDown;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnEdit;
	CNxIconButton m_btnEditBottom; // (z.manning 2015-04-28 13:22) - NX-100396
	CNxIconButton m_btnRemove;

	// (z.manning 2015-04-28 13:46) - NX-100396
	void HandleSelChanged();
	void EditControl(NXDATALIST2Lib::IRowSettingsPtr pRow, const BOOL bAdminRow);
	// (z.manning 2015-05-04 15:24) - NX-100401
	void DoContextMenu(NXDATALIST2Lib::IRowSettingsPtr pRow, const BOOL bAdminRow);

	// Generated message map functions
	//{{AFX_MSG(CConfigurePatDashboardDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();	
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCpdAdd();
	afx_msg void OnBnClickedCpdMoveRight();
	afx_msg void OnBnClickedCpdMoveLeft();
	afx_msg void OnBnClickedCpdMoveUp();
	afx_msg void OnBnClickedCpdMoveDown();
	afx_msg void OnBnClickedCpdRemove();
	afx_msg void OnBnClickedCpdEdit();
	afx_msg void OnBnClickedCpdEditBottom();
	void DblClickCellCpdAdminList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellCpdUserList(LPDISPATCH lpRow, short nColIndex);
	// (j.gruber 2012-07-01 16:35) - PLID 51210
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void OnSelChangedAdminList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnSelChangedUserList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void ShowContextMenuCpdAdminList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue);
	void ShowContextMenuCpdUserList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

// (z.manning 2015-05-07 11:21) - NX-100439 - Made a global function for this
CString GetDashboardTypeNameFromID(const PatientDashboardType type);

#endif // !defined(AFX_CONFIGUREPATDASHDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_)
