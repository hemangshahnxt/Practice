//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_UB92SETUP_H__7B4C52C3_4423_11D3_AD42_00104B318376__INCLUDED_)
#define AFX_UB92SETUP_H__7B4C52C3_4423_11D3_AD42_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UB92Setup.h : header file
//

#include "Client.h"
#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CUB92Setup dialog

class CUB92Setup : public CNxDialog
{
// Construction
public:
	void UpdateTable(CString BoxName, long data);
	void SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue);
	void Load();
	void Save (int nID);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	CUB92Setup(CWnd* pParent);   // standard constructor

	// (a.walling 2007-06-21 09:36) - PLID 26414 - Added m_checkUseICD9 button
	// (j.jones 2008-05-21 10:25) - PLID 30129 - added Box 66 controls
// Dialog Data
	//{{AFX_DATA(CUB92Setup)
	enum { IDD = IDD_UB92_SETUP };
	CNxIconButton	m_nxibCodesToExpand;
	NxButton	m_checkUseICD9;
	NxButton	m_checkUseThree;
	NxButton	m_checkBox42Line23;
	NxButton	m_checkBox57;
	NxButton	m_checkBox51;
	NxButton	m_checkBox64;
	NxButton	m_radioBox5LocEIN;
	NxButton	m_radioBox5ProvTaxID;
	CNxIconButton	m_btnAdvEbillingSetup;
	NxButton	m_checkShowEmployerCompany;
	NxButton	m_radioBox80Diag;
	NxButton	m_radioBox80CPT;
	NxButton	m_radioBox80None;
	CNxIconButton	m_btnEditUB92Dates;
	NxButton	m_checkShowApplies;
	NxButton	m_checkFill42With44;
	NxButton	m_checkBox76;
	NxButton	m_checkPunctuateDiagCodes;
	NxButton	m_checkPunctuateChargeLines;
	NxButton	m_radioBox1POS;
	NxButton	m_radioBox1Location;
	NxButton	m_checkShowInsAddr50;
	NxButton	m_checkShowInscoAdd;
	NxButton	m_checkShowTotals;
	NxButton	m_checkShowPhoneNum;
	NxButton	m_checkGroupCharges;
	NxButton	m_checkBox85;
	NxButton	m_radioDateYearFirst;
	NxButton	m_radioDateMonthFirst;
	CNxIconButton	m_btnEditCategories;
	NxButton	m_radioBox38InsuredParty;
	NxButton	m_radioBox38InsuranceCo;
	CNxIconButton	m_btnNewGroup;
	CNxIconButton	m_btnEditInsInfo;
	CNxIconButton	m_btnDeleteGroup;
	CNxIconButton	m_btnAlignForm;
	CNxIconButton	m_btnAddOne;
	CNxIconButton	m_btnAddAll;
	CNxIconButton	m_btnRemOne;
	CNxIconButton	m_btnRemAll;
	CNxEdit	m_nxeditEditBox42line23;
	CNxEdit	m_nxeditEditBox51Default;
	CNxEdit	m_nxeditEditBox4;
	CNxEdit	m_nxeditEditBox8;
	CNxEdit	m_nxeditEditBox10;
	CNxEdit	m_nxeditEditBox18;
	CNxEdit	m_nxeditEditBox19;
	CNxEdit	m_nxeditEditBox20;
	CNxEdit	m_nxeditEditBox22;
	CNxEdit	m_nxeditEditBox32;
	CNxEdit	m_nxeditEditBox79;
	CNxEdit	m_nxeditEditBox66;
	CNxEdit	m_nxeditBox81aQual;
	CNxEdit	m_nxeditBox81bQual;
	CNxEdit	m_nxeditBox81cQual;
	CNxEdit	m_nxeditBox76Qual;
	CNxEdit	m_nxeditBox78Qual;
	CNxEdit	m_nxeditBox77Qual;
	CNxStatic	m_nxstaticBox51Label;
	CNxStatic	m_nxstaticBox56Label;
	CNxStatic	m_nxstaticDefBatchLabel;
	CNxStatic	m_nxstaticDefUb92BatchNotPrimaryLabel;
	CNxStatic	m_nxstaticBox2Label;
	CNxStatic	m_nxstaticBox80Label;
	CNxStatic	m_nxstaticBox8Label;
	CNxStatic	m_nxstaticBox10Label;
	CNxStatic	m_nxstaticBox18Label;
	CNxStatic	m_nxstaticBox19Label;
	CNxStatic	m_nxstaticBox20Label;
	CNxStatic	m_nxstaticBox22Label;
	CNxStatic	m_nxstaticBox32Label;
	CNxStatic	m_nxstaticBox79Label;
	CNxStatic	m_nxstaticBox66Label;
	CNxStatic	m_nxstaticBox81Label;
	CNxStatic	m_nxstaticLabelBox81A;
	CNxStatic	m_nxstaticLabelBox81B;
	CNxStatic	m_nxstaticLabelBox81C;
	CNxStatic	m_nxstaticBox82Label;
	CNxStatic	m_nxstaticBox77Label;
	CNxStatic	m_nxstaticBox83Label;
	CNxStatic	m_nxstatic8283FormatLabel;
	CNxStatic	m_nxstaticBox8283Label;
	// (j.jones 2012-05-24 10:57) - PLID 50597 - added InsRelANSI
	NxButton	m_checkUseAnsiInsRelCodesBox59;
	// (j.jones 2012-09-05 14:37) - PLID 52191 - added Box74Qual
	CNxStatic	m_nxstaticBox74QualLabel;
	// (j.jones 2013-07-18 09:20) - PLID 41823 - added DontBatchSecondary
	NxButton	m_checkDontBatchSecondary;
	CNxIconButton	m_btnSecondaryExclusions;
	// (b.spivey July 8, 2015) - PLID 66515 -added FillBox12 and 16 checks. 
	NxButton	m_checkBox16;
	NxButton	m_checkBox12_13;
	// (j.jones 2016-05-06 8:53) - NX-100514 - added Box 39
	CNxStatic	m_nxstaticBox39Label;
	CNxEdit		m_nxeditBox39Code;
	CNxEdit		m_nxeditBox39Value;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_pGroups;
	NXDATALISTLib::_DNxDataListPtr	m_pUnselected;
	NXDATALISTLib::_DNxDataListPtr	m_pSelected;
	NXDATALISTLib::_DNxDataListPtr m_BatchCombo;
	NXDATALISTLib::_DNxDataListPtr m_SecondaryBatchCombo;
	NXDATALISTLib::_DNxDataListPtr m_Box82Combo;
	NXDATALISTLib::_DNxDataListPtr m_Box83Combo;
	NXDATALISTLib::_DNxDataListPtr m_Box82Number;
	NXDATALISTLib::_DNxDataListPtr m_Box83Number;
	NXDATALISTLib::_DNxDataListPtr m_8283Format;
	NXDATALISTLib::_DNxDataListPtr m_Box53Accepted;
	// (j.jones 2007-03-20 12:46) - PLID 25278 - supported UB04 Box 56 NPI
	NXDATALIST2Lib::_DNxDataListPtr m_Box56NPICombo;
	// (j.jones 2007-03-21 09:41) - PLID 25279 - supported UB04 Box 81 Taxonomy Codes
	NXDATALIST2Lib::_DNxDataListPtr m_Box81aCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box81bCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box81cCombo;

	//TES 3/19/2007 - PLID 25235 - Support UB04 Box 77
	NXDATALIST2Lib::_DNxDataListPtr m_Box77Combo;
	NXDATALIST2Lib::_DNxDataListPtr m_Box77Number;

	// (a.walling 2007-06-05 12:15) - PLID 26228 - Revenue code to expand
	// (a.walling 2007-08-17 14:16) - PLID 27092 - Now we use a button to open a multiselect dialog
	// NXDATALIST2Lib::_DNxDataListPtr m_RevCodeExpand;

	// (j.jones 2007-07-11 15:46) - PLID 26621 - added Box 2 setup
	NXDATALIST2Lib::_DNxDataListPtr m_Box2Combo;	
	// (j.jones 2007-07-12 10:22) - PLID 26625 - added new Box 1 setup (UB04 only)
	NXDATALIST2Lib::_DNxDataListPtr m_Box1Combo;

	// (j.jones 2012-09-05 14:37) - PLID 52191 - added Box74Qual
	NXDATALIST2Lib::_DNxDataListPtr m_Box74QualCombo;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUB92Setup)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_loading;
	CTableChecker m_companyChecker, m_groupsChecker, m_labelChecker;

	CBrush m_brush;

	//TES 3/12/2007 - PLID 24993 - Lets them select UB92 vs. UB04
	NXDATALIST2Lib::_DNxDataListPtr m_pFormTypes;
	//TES 3/12/2007 - PLID 24993 - Updates the screen to reflect what form type they've selected (mainly changing box numbers).
	void ReflectFormType();

	// (j.jones 2010-04-16 09:11) - PLID 38149 - track the Box76 qualifier
	CString m_strOldBox76Qualifier;

	// (a.walling 2007-06-21 09:36) - PLID 26414 - Added OnCheckUseIcd9ProcedureCodes handler
	// Generated message map functions
	//{{AFX_MSG(CUB92Setup)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenUb92Groups(long nRow);
	afx_msg void OnDblClickCellUb92Unselected(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUb92Selected(long nRowIndex, short nColIndex);
	afx_msg void OnAddCompany();
	afx_msg void OnAddAllCompanies();
	afx_msg void OnRemoveCompany();
	afx_msg void OnRemoveAllCompanies();
	afx_msg void OnNewUb92Group();
	afx_msg void OnDeleteUb92Group();
	afx_msg void OnEditInsInfo();
	afx_msg void OnAlignForm();
	afx_msg void OnEditCategories();
	afx_msg void OnSelChosenBatchCombo(long nRow);
	afx_msg void OnSelChosenSecondaryBatchCombo(long nRow);
	afx_msg void OnSelChosenBox82Combo(long nRow);
	afx_msg void OnSelChosenBox83Combo(long nRow);
	afx_msg void OnSelChosen8283Format(long nRow);
	afx_msg void OnSelChosenBox82Number(long nRow);
	afx_msg void OnSelChosenBox83Number(long nRow);
	afx_msg void On85Check();
	afx_msg void OnGroupChargesCheck();
	afx_msg void OnCheckShowPhoneNumber();
	afx_msg void OnCheckShowTotalAmount();
	afx_msg void OnCheckShowInscoAddress();
	afx_msg void OnCheckShowInsAddrBox50();
	afx_msg void OnCheckPunctuateChargeLines();
	afx_msg void OnCheckPunctuateDiags();
	afx_msg void On76Check();
	afx_msg void OnCheckFill42With44();
	afx_msg void OnCheckShowApplies();
	afx_msg void OnBtnUb92Dates();
	afx_msg void OnCheckShowEmployerCompany();
	afx_msg void OnSelChosenBox53Accepted(long nRow);
	afx_msg void OnAdvEbillingSetup();
	afx_msg void On64Check();
	afx_msg void OnSelChangingUbFormType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenUbFormType(LPDISPATCH lpRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChosenBox77Number(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox77Combo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox56Npi(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox81aTaxonomyList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox81bTaxonomyList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox81cTaxonomyList(LPDISPATCH lpRow);
	afx_msg void OnBox51Check();
	afx_msg void OnBox57Check();
	afx_msg void OnCheckBox42Line23();
	afx_msg void OnCheckUseThree();
	afx_msg void OnCheckUseIcd9ProcedureCodes();
	afx_msg void OnSelChosenBox2Options(LPDISPATCH lpRow);
	afx_msg void OnSelChosenBox1Options(LPDISPATCH lpRow);
	afx_msg void OnBtnExpandCodes();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2012-05-24 10:57) - PLID 50597 - added InsRelANSI
	afx_msg void OnBnClickedCheckUseAnsiInsRelCodesBox59();
	// (j.jones 2012-09-05 15:04) - PLID 52191 - added Box74Qual
	afx_msg void OnSelChangingBox74QualCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnSelChosenBox74QualCombo(LPDISPATCH lpRow);
	// (j.jones 2013-07-18 09:15) - PLID 41823 - added DontBatchSecondary
	afx_msg void OnCheckDontBatchSecondary();
	// (j.jones 2013-07-18 09:19) - PLID 41823 - added 'DontBatchSecondary' exclusions
	afx_msg void OnBtnEditSecondaryExclusions();
	// (b.spivey July 8, 2015) - PLID 66515 - handlers. 
	afx_msg void OnCheckFill12_13();
	afx_msg void OnCheckFill16();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UB92SETUP_H__7B4C52C3_4423_11D3_AD42_00104B318376__INCLUDED_)
