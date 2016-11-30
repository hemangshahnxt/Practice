// (r.gonet 09/21/2011) - PLID 45555 - Added

#pragma once

#include "PatientsRc.h"
#include "LabCustomField.h"
#include "LabCustomFieldsView.h"

// CLabCustomFieldsDlg dialog

// (r.gonet 09/21/2011) - PLID 45555 - An easy one. Just a place to hold the custom fields view and a close button.
class CLabCustomFieldsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabCustomFieldsDlg)

private:
	CNxColor m_nxcolor;
	// Placeholder for the custom fields view
	CNxStatic m_nxsPlaceholder;
	CLabCustomFieldsView m_dlgFieldsView;
	CNxIconButton m_nxbClose;

public:
	CLabCustomFieldsDlg(CCFTemplateInstance *pTemplateInstance, CWnd* pParent);   // standard constructor
	virtual ~CLabCustomFieldsDlg();

// Dialog Data
	enum { IDD = IDD_LAB_CUSTOM_FIELDS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	// Generated message map functions
	virtual BOOL OnInitDialog();

public:
	afx_msg void OnBnClickedOk();
	bool SaveAndClose();
};