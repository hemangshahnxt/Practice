#pragma once
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes

#include "AdministratorRc.h"

// CApptPrototypesDlg dialog

class CApptPrototypesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptPrototypesDlg)

public:
	CApptPrototypesDlg(CWnd* pParent);   // standard constructor
	virtual ~CApptPrototypesDlg();

// Dialog Data
	enum { IDD = IDD_APPTPROTOTYPES_DLG };
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnEdit;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnClose;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void ReflectEnabledControls(BOOL bIsRowSelected);
	void ReflectEnabledControls();
	void EditApptPrototypeRow(NXDATALIST2Lib::IRowSettings *lpRow);
	void GetOtherPrototypeNames(OUT CStringArray &arystrOtherNames, NXDATALIST2Lib::IRowSettings *lpExceptRow);

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void DblClickCellAppointmentPrototypeList(LPDISPATCH lpRow, short nColIndex);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedEditBtn();
	afx_msg void OnBnClickedDeleteBtn();
	void SelSetAppointmentPrototypeList(LPDISPATCH lpSel);
};
