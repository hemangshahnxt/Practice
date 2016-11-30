#if !defined(AFX_SALESDLG_H__41CD77A6_9157_45C0_996B_0F624B70E988__INCLUDED_)
#define AFX_SALESDLG_H__41CD77A6_9157_45C0_996B_0F624B70E988__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SalesDlg.h : header file
//

#include "PatientDialog.h"

/////////////////////////////////////////////////////////////////////////////
// SalesDlg dialog

class CSalesDlg : public CPatientDialog
{
// Construction
public:
	CSalesDlg(CWnd* pParent);   // standard constructor
	virtual void SetColor(OLE_COLOR nNewColor);
	void Save(int nID);
	void Load();
	void StoreDetails();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	// (j.armen 2011-06-28 11:51) - PLID 44342 - Functions for handling the Society box/label
	void StoreSociety(NXDATALISTLib::_DNxDataListPtr customList, UINT customListIDC );
	void RefreshSociety();
	long m_id;
	NXDATALISTLib::_DNxDataListPtr m_altContact, m_society, m_status, m_pRefStatus, m_pRefRating, m_pDiscountType;
	//(a.wilson 2011-4-28) PLID 43355 added a pointer to handle the drop down for discount types.

// Dialog Data
	//{{AFX_DATA(SalesDlg)
	enum { IDD = IDD_SALESDLG };
	CNxIconButton	m_btnCreateTodo;
	CNxIconButton	m_btnSelReferral;
	CNxEdit	m_nxeditCompany;
	CNxEdit	m_nxeditTitleBox;
	CNxEdit	m_nxeditFirstNameBox;
	CNxEdit	m_nxeditMiddleNameBox;
	CNxEdit	m_nxeditLastNameBox;
	CNxEdit	m_nxeditAddress1Box;
	CNxEdit	m_nxeditAddress2Box;
	CNxEdit	m_nxeditZipBox;
	CNxEdit	m_nxeditCityBox;
	CNxEdit	m_nxeditStateBox;
	CNxEdit	m_nxeditCallerBox;
	CNxEdit	m_nxeditWorkPhoneBox;
	CNxEdit	m_nxeditExtPhoneBox;
	CNxEdit	m_nxeditFaxPhoneBox;
	CNxEdit	m_nxeditBacklineBox;
	CNxEdit	m_nxeditCellPhoneBox;
	CNxEdit	m_nxeditPagerPhoneBox;
	CNxEdit	m_nxeditWebsiteBox;
	CNxEdit	m_nxeditDocEmailBox;
	CNxEdit	m_nxeditPracEmailBox;
	CNxEdit	m_nxeditRefDate;
	CNxEdit	m_nxeditSourceBox;
	CNxEdit	m_nxeditReferral;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticCustom1Label;
	CNxStatic	m_nxstaticCustom3Label;
	CNxStatic	m_nxstaticCustom1Label3;
	//(a.wilson 2011-4-28) PLID 43355 - variables to control the percent discount controls
	CNxEdit m_neditDiscountPercent;
	CNxEdit m_neditAddOnDiscountPercent;
	// (j.armen 2011-06-28 11:53) - PLID 44342 - Added label for society
	CNxLabel m_nxlSocietyLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SalesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bFormatPhoneNums;
	CString m_strPhoneFormat;



	//DRT 5/24/2007 - PLID 25892 - Added OnViewOpportunities

	// Generated message map functions
	//{{AFX_MSG(SalesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedAltContactList(long nNewSel);
	afx_msg void OnSelChangedStatus(long nNewSel);
	afx_msg void OnSelChangedSocietyBox(long nNewSel);
	afx_msg void OnSelChosenRefRating(long nNewSel);
	afx_msg void OnSelChosenRefStatus(long nNewSel);
	afx_msg void OnEmail();
	afx_msg void OnEmail2();
	afx_msg void OnViewTodo();
	afx_msg void OnCreateTodo();
	afx_msg void OnViewOpportunities();
	//(a.wilson 2011-4-28) PLID 43355 - functions to control combo box behaviors for the discount portion
	afx_msg void OnSelChosenDiscountType(long nSelection);	
	afx_msg void OnSelChangingDiscountType(long FAR* nRow);
	afx_msg void OnBnClickedVisitWebsite(); //(a.wilson 2011-5-6) PLID 9702
	// (j.armen 2011-06-28 11:53) - PLID 44342 - Added event handling for labels
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SALESDLG_H__41CD77A6_9157_45C0_996B_0F624B70E988__INCLUDED_)
