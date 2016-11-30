#pragma once
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes

#include "AdministratorRc.h"

// CApptPrototypePropertySetAvailabilityDlg dialog

class CApptPrototypePropertySetAvailabilityDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptPrototypePropertySetAvailabilityDlg)

public:
	CApptPrototypePropertySetAvailabilityDlg(CWnd* pParent);   // standard constructor
	virtual ~CApptPrototypePropertySetAvailabilityDlg();

// Dialog Data
	enum { IDD = IDD_APPTPROTOTYPE_PROPERTYSET_AVAILABILITY_DLG };
	// (b.cardillo 2011-02-28 16:51) - Oops, forgot the button styles on this dialog for PLID 40419
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

public:
	LONG m_nAvailDays;
	LONG m_nAvailTimes;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
};
