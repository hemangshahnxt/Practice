#pragma once
#include "AdministratorRc.h"

// CUBSetupDLG.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Admin Billing UBSETUP dialog
// (s.tullis 2014-05-02 09:45) - PLID 61893 - Remove the UB fields from Admin Billing and place them in a new UB dialog
class CUBSetupDLG : public CNxDialog
{
// Construction
public:
	CUBSetupDLG(CWnd* pParent);   // standard constructor
	long m_nServiceID;
	BOOL bCanWrite;

	
// Dialog Data
	enum { IDD = IDD_BILLTAB_UBSETUPDLG };
	NXDATALIST2Lib::_DNxDataListPtr m_UB92_Category;
	NXDATALIST2Lib::_DNxDataListPtr m_ICD9V3List; 
	CNxEdit		m_editUBBox4; 
	CNxIconButton	m_btnSetupICD9v3;
	CNxIconButton	m_btnEditMultipleRevCodes;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnAdvRevCodeSetup;
	NxButton	m_checkMultipleRevCodes;
	NxButton	m_checkSingleRevCode;

	



	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	// Generated message map functions
	
	
	virtual BOOL OnInitDialog();

	void SecureControls();

	BOOL PreTranslateMessage(MSG* pMsg);
	
	afx_msg void OnOKClick();
	
	afx_msg void OnBtnEditMultipleRevCodes();
	afx_msg void OnBtnSetupIcd9v3();
	afx_msg void OnCheckSingleRevCode();
	afx_msg void OnCheckMultipleRevCodes();
	afx_msg void OnAdvRevcodeSetup();
	
	
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	

private:
	CWnd* m_pParent;
	CString strUB4;
	long nICD9ProcedureID;
	long nUB92CatID;
	long nRevCode;
	
	
};




