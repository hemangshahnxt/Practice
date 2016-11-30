#pragma once

#include "PatientsRc.h"
// CEditImmunizationTypesDlg dialog
// (d.thompson 2009-05-18) - PLID 34232 - Created

class CEditImmunizationTypesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditImmunizationTypesDlg)

public:
	CEditImmunizationTypesDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditImmunizationTypesDlg();

	
	// (a.walling 2010-09-14 10:12) - PLID 40505 - Popup some examples of ANSI+ units
	static void PopupANSIPlusUnitsDescription(CWnd* pParent);

// Dialog Data
	enum { IDD = IDD_EDIT_IMMUNIZATION_TYPES_DLG };

protected:
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnClose;
	CNxLabel m_nxlabelUnits; // (a.walling 2010-09-13 14:44) - PLID 40505 - Support dosage units for vaccinations
	NXDATALIST2Lib::_DNxDataListPtr m_pList;


	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnDeleteType();
	afx_msg void OnAddType();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (a.walling 2010-09-13 14:35) - PLID 40505 - Support dosage units for vaccinations
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (a.walling 2010-09-13 14:35) - PLID 40505 - Support dosage units for vaccinations

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
