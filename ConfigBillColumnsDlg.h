#if !defined(AFX_CONFIGBILLCOLUMNSDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_)
#define AFX_CONFIGBILLCOLUMNSDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigBillColumnsDlg.h : header file
//

#include "AdministratorRc.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CConfigBillColumnsDlg dialog

class CConfigBillColumnsDlg : public CNxDialog
{
// Construction
public:
	CConfigBillColumnsDlg(CWnd* pParent);   // standard constructor

	void Load();
	void Save();

	long m_nLocationID;

	// (j.dinatale 2010-11-02) - PLID 41279 - dont need to worry about the billing tab anymore
	// (j.jones 2008-09-16 15:55) - PLID 31412 - split the bChanged variable into
	// three parts so we didn't reset stored bill/quote widths unnecessarily
	BOOL m_bBillColumnsChanged;
	BOOL m_bQuoteColumnsChanged;
	//BOOL m_bBillTabColumnsChanged;

	NXDATALISTLib::_DNxDataListPtr m_LocationCombo;

	// (j.dinatale 2010-11-02) - PLID 41279 - removed all the buttons associated with billing tab preferences
	// (z.manning, 04/30/2008) - PLID 29852 - Added NxIconButtons
	// (j.jones 2008-09-16 15:34) - PLID 31412 - added billing tab options
	// (j.gruber 2009-03-20 10:39) - PLID 33385 - combine percentoff and discount into total discount
// Dialog Data
	//{{AFX_DATA(CConfigBillColumnsDlg)
	enum { IDD = IDD_CONFIG_BILL_COLUMNS_DLG };	
	/*NxButton	m_checkBillTabProvider;
	NxButton	m_checkBillTabPOSName;
	NxButton	m_checkBillTabPOSCode;
	NxButton	m_checkBillTabLocation;
	NxButton	m_checkBillTabInputDate;
	NxButton	m_checkBillTabCreatedBy;
	NxButton	m_checkBillTabChargeDate;
	NxButton	m_checkBillTabBillID;
	NxButton	m_checkBillTabDiscountIcon;*/
	// (s.tullis 2015-03-27 14:57) - PLID 64977 - Added Charge Category
	NxButton    m_checkBillChargeCategory;
	NxButton    m_checkQuoteChargeCategory;
	NxButton	m_checkBillServiceDateTo;
	NxButton	m_checkQuoteMod4;
	NxButton	m_checkQuoteMod3;
	NxButton	m_checkQuoteMod2;
	NxButton	m_checkQuoteMod1;
	NxButton	m_checkBillMod4;
	NxButton	m_checkBillMod3;
	NxButton	m_checkBillTotalDiscount;	
	NxButton	m_checkBillPatientCoordinator;
	NxButton	m_checkBillInputDate;
	NxButton	m_checkQuoteTax2;
	NxButton	m_checkQuoteTax1;
	NxButton	m_checkQuoteProvider;
	NxButton	m_checkQuoteTotalDiscount;
	NxButton	m_checkQuoteOutsideCost;	
	NxButton	m_checkQuoteCPTSubCode;
	NxButton	m_checkQuoteCPTCode;
	NxButton	m_checkBillTOS;
	NxButton	m_checkBillTax2;
	NxButton	m_checkBillTax1;
	NxButton	m_checkBillMod2;
	NxButton	m_checkBillMod1;
	NxButton	m_checkBillDiagCs;
	NxButton	m_checkBillCPTSubCode;
	NxButton	m_checkBillCPTCode;
	// (j.jones 2010-08-31 17:48) - PLID 40330 - added bill allowable
	NxButton	m_checkBillAllowable;
	// (j.gruber 2009-10-19 13:38) - PLID 36000 - quote allowable
	NxButton	m_checkQuoteAllowable;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2010-05-26 14:17) - PLID 28184 - added billing tab diag code options
	//NxButton	m_checkBillTabDiagCode1;
	//NxButton	m_checkBillTabDiagCode1WithDesc;
	//NxButton	m_checkBillTabAllDiagCodes;
	//// (j.jones 2010-08-31 17:53) - PLID 40331 - added bill tab allowable
	//NxButton	m_checkBillTabAllowable;
	// (j.jones 2010-11-09 09:29) - PLID 41390 - added bill claim provider
	NxButton	m_checkBillClaimProvider;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigBillColumnsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfigBillColumnsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenCbqLocationCombo(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGBILLCOLUMNSDLG_H__703541C2_6AE7_4AA9_899C_99B4AE4972B4__INCLUDED_)
