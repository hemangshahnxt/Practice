#pragma once

// (j.jones 2010-11-01 09:40) - PLID 40919 - created

// CAdv2010ABPayToOverrideDlg dialog

#include "AdministratorRc.h"

class CAdv2010ABPayToOverrideDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAdv2010ABPayToOverrideDlg)

public:
	CAdv2010ABPayToOverrideDlg(CWnd* pParent);   // standard constructor
	virtual ~CAdv2010ABPayToOverrideDlg();

	// (j.jones 2011-11-16 14:53) - PLID 46489 - added parameter for when this is overriding 2010AA and not 2010AB
	int DoModal(long nProviderID, long nLocationID, BOOL bIsUB04, long nSetupGroupID, BOOL b2010AA);

// Dialog Data
	enum { IDD = IDD_ADV_2010AB_PAYTO_OVERRIDE_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticProviderLabel;
	CNxStatic	m_nxstaticLocationLabel;
	CNxEdit		m_nxeditAddress1;
	CNxEdit		m_nxeditAddress2;
	CNxEdit		m_nxeditCity;
	CNxEdit		m_nxeditState;
	CNxEdit		m_nxeditZip;
	// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
	CNxStatic	m_nxstaticInfoLabel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	long m_nProviderID;
	long m_nLocationID;
	long m_bIsUB04;
	long m_nSetupGroupID;
	// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
	BOOL m_b2010AA;

	// (b.eyers 2015-04-09) - PLID 59169
	BOOL m_bLookupByCity;

	afx_msg void OnKillfocusZipBox(); // (b.eyers 2015-04-09) - PLID 59169
	afx_msg void OnKillfocusCityBox(); // (b.eyers 2015-04-09) - PLID 59169
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
};
