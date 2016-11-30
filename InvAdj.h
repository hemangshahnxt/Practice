#if !defined(AFX_INVADJ_H__A835A1E3_7E31_43DE_B89B_B294F704B533__INCLUDED_)
#define AFX_INVADJ_H__A835A1E3_7E31_43DE_B89B_B294F704B533__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvAdj.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInvAdj dialog

class CInvAdj : public CNxDialog
{
// Construction
public:
	CInvAdj(CWnd* pParent);   // standard constructor

	BOOL m_bUseUU;

	long m_UUConversion;

	// (j.jones 2008-06-02 15:46) - PLID 28076 - AdjustProductItems will now fill an array with IDs
	// of products that need adjusted, and actually adjust them off later
	//(e.lally 2008-07-01) PLID 24534 - Added support for entering a serial/exp date per Unit of Order on positive product adjustments
	// (j.jones 2009-01-15 15:40) - PLID 32749 - moved to InvUtils
	//BOOL AdjustProductItems(long ProductID, double dblQuantity, CArray<long, long> &aryProductItemIDsToRemove, BOOL bCanUseUOAdjustment);

// Dialog Data
	//{{AFX_DATA(CInvAdj)
	enum { IDD = IDD_INVADJUST };
	CNxIconButton	m_nxbEditAdjCategories;
	CNxIconButton	m_btnAutoCalc;
	CNxIconButton	m_okBtn;
	CNxIconButton	m_cancelBtn;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditStock;
	CNxEdit	m_nxeditStockUo;
	CNxEdit	m_nxeditQuantity;
	CNxEdit	m_nxeditQuantityUo;
	CNxEdit	m_nxeditCost;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticInvadjQuantity;
	CNxStatic	m_nxstaticUuAdjText;
	CNxStatic	m_nxstaticUoAdjText;
	//}}AFX_DATA
	int DoModal(long product, long nLocationID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvAdj)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//Sets the "On Hand" amount to the correct value for the selected location.
	void RefreshOnHand();

	NXDATALISTLib::_DNxDataListPtr   m_location;

	//TES 6/24/2008 - PLID 26142 - A dropdown for ProductAdjustmentCategoriesT
	NXDATALIST2Lib::_DNxDataListPtr m_pAdjCategories;

	//(e.lally 2008-07-01) PLID 24534 - Use the SerialPerUO field option for entering positive product adjustments
	BOOL m_bEnterSingleSerialPerUO;

	//TES 6/27/2008 - PLID 30523 - Load the category list, filtering out categories they don't have permission for.
	void LoadCategories();

	long m_product, m_nLocationID;
	// Generated message map functions
	//{{AFX_MSG(CInvAdj)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnKillfocusCost();
	afx_msg void OnKillfocusQuantity();
	virtual void OnCancel();
	afx_msg void OnSelChosenLocation(long nRow);
	afx_msg void OnKillfocusQuantityUo();
	afx_msg void OnBtnAutoCalc();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditAdjCategories();
	afx_msg void OnSelChosenAdjCategories(LPDISPATCH lpRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVADJ_H__A835A1E3_7E31_43DE_B89B_B294F704B533__INCLUDED_)
