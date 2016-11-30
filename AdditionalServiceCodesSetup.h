#pragma once
#include "AdministratorRc.h"

//AdditionServiceCodesSetup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Admin Billing AdditionServiceCodesSetup dialog


// (s.tullis 2014-05-02 10:17) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
class CAdditionServiceCodesSetup: public CNxDialog
{
	// Construction
public:
	
	
	CAdditionServiceCodesSetup(CWnd* pParent);   // standard constructor
	long m_nServiceID;
	CNxIconButton	m_btnOk;
	NxButton	m_checkUBSetup;
	NxButton	m_checkAnesthesiaSetup;
	




	// Dialog Data
	enum { IDD = IDD_BILLTAB_SERVICE_SETUPDLG };
	





protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	afx_msg void OnBtnUBSetup();
	afx_msg void OnBtnAnesethiaSetup();
	BOOL OnInitDialog();
	void SetChecks(int nRet);


	
	DECLARE_MESSAGE_MAP()
	


private:
	
};




