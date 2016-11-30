#pragma once
#include "AdministratorRc.h"


//AnethesiaFacilitySetupDLG.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Admin Billing CAnesthesiaFacilitySetupDlg dialog


// (s.tullis 2014-05-21 17:36) - PLID 62023 - Remove the Anesthesia/Facility Setup fuctionality from the Admin Billing dialog to a new dialog that launchs from the Additional Service Code Setup Menu
class CAnesthesiaFacilitySetupDlg : public CNxDialog
{
	// Construction
public:


	CAnesthesiaFacilitySetupDlg(CWnd* pParent);   // standard constructor
	long m_nServiceID;
	CString m_strServCode;
	
	BOOL bCanWrite;





	// Dialog Data
	enum { IDD = IDD_BILLTAB_FACILITY_SETUP };
	CNxIconButton	m_btnFacilityFeeSetup;//s.tullis
	CNxIconButton	m_btnAnesthesiaSetup;
	NxButton	m_checkUseFacilityBilling;//s.tullis
	NxButton	m_checkUseAnesthesiaBilling;//s.tullis
	NxButton	m_checkFacility;
	NxButton	m_checkAnesthesia;
	CNxEdit	m_editAnesthBaseUnits;
	CNxIconButton	m_btnOk;






protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	

	//afx_msg void OnBtnUBSetup();
	BOOL OnInitDialog();
	
	void SecureControls();
	afx_msg void OnCloseClick();
	afx_msg void OnCheckAnesthesia();
	afx_msg void OnCheckUseAnesthesiaBilling();
	afx_msg void  OnBtnFacilityFeeSetup();
	afx_msg void OnCheckUseFacilityBilling();
	afx_msg void OnBtnAnesthesiaSetup();
	afx_msg void OnCheckFacility();
	void CheckEnableAnesthesiaFacilityAssistingControls();
	
	


	DECLARE_MESSAGE_MAP()
};




