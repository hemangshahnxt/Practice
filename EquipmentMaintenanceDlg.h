#if !defined(AFX_EQUIPMENTMAINTENANCEDLG_H__F6941EC6_1730_4290_8C19_388048B09467__INCLUDED_)
#define AFX_EQUIPMENTMAINTENANCEDLG_H__F6941EC6_1730_4290_8C19_388048B09467__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EquipmentMaintenanceDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEquipmentMaintenanceDlg dialog

class CEquipmentMaintenanceDlg : public CNxDialog
{
// Construction
public:
	CEquipmentMaintenanceDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_MaintList,
					 m_IntervalCombo;

	long m_ProductID;

// Dialog Data
	//{{AFX_DATA(CEquipmentMaintenanceDlg)
	enum { IDD = IDD_EQUIPMENT_MAINTENANCE_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditCost;
	CNxEdit	m_nxeditMaintInterval;
	CNxStatic	m_nxstaticEquipmentName;
	CNxIconButton m_btnAddNewMaintEntry;
	CNxIconButton m_btnOK;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEquipmentMaintenanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEquipmentMaintenanceDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnAddNewMaintEntry();
	afx_msg void OnEditingFinishedMaintenanceList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownMaintenanceList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingMaintenanceList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EQUIPMENTMAINTENANCEDLG_H__F6941EC6_1730_4290_8C19_388048B09467__INCLUDED_)
