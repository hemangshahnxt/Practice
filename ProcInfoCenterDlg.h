#if !defined(AFX_PROCINFOCENTERDLG_H__F578B8EB_2146_4CF7_9BB7_73AEE0EF17E4__INCLUDED_)
#define AFX_PROCINFOCENTERDLG_H__F578B8EB_2146_4CF7_9BB7_73AEE0EF17E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcInfoCenterDlg.h : header file
//
#include "PatientDialog.h"
#include "ApplyManagerDlg.h"// (a.vengrofski 2010-02-08 09:23) - PLID <34617> - Added a way to remove quotes from the PIC.

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;

class CPicContainerDlg;
class CBillingModuleDlg;

/////////////////////////////////////////////////////////////////////////////
// CProcInfoCenterDlg dialog

//"Area"s which can be independently loaded
#define PIC_AREA_ALL	0
#define PIC_AREA_QUOTE	1
#define PIC_AREA_BILL	2
#define PIC_AREA_PAY	3
#define PIC_AREA_APPT	4
#define PIC_AREA_MED	5
#define PIC_AREA_CASE	6	// (j.jones 2009-08-06 14:23) - PLID 7397

// (d.singleton 2012-04-05 12:36) - PLID 40019
struct AppointmentInfo
{
	CString strAptType;
	COleDateTime dtStartDate;
	CString strAptTime;
	CString strCategory;
	long nCategoryID;
	long nApptDay;
};

class CProcInfoCenterDlg : public CPatientDialog
{
// Construction
public:
	CProcInfoCenterDlg(CWnd* pParent);   // standard constructor

	long m_nProcInfoID; //The ProcInfoT record we're displaying
	OLE_COLOR m_nColor;
	
	// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
	//CPicContainerDlg *m_pContainerDlg;
	class CPicContainerDlg* GetPicContainer() const
	{
		return m_pPicContainer;
	}

	void SetPicContainer(class CPicContainerDlg* pPicContainer)
	{
		ASSERT(!m_pPicContainer || !pPicContainer);
		m_pPicContainer = pPicContainer;
	}

protected:
	class CPicContainerDlg* m_pPicContainer;

public:

	// (j.jones 2009-08-06 14:05) - PLID 7397 - added case history combo
	NXDATALISTLib::_DNxDataListPtr m_pProcNames, m_pSurgeon, m_pCoSurgeon, m_pNurse, m_pAnesthesiologist, m_pApptList, m_pQuoteList, m_pMedList,
		m_pAnesthesiaList, m_pProcDetailNames, m_pCaseHistoryCombo;

	// (j.dinatale 2012-07-09 13:10) - PLID 3073 - added bill combo
	NXDATALIST2Lib::_DNxDataListPtr m_pBillCombo;

	bool IsEmpty();

// Dialog Data
	//{{AFX_DATA(CProcInfoCenterDlg)
	enum { IDD = IDD_PROC_INFO_DLG };
	CNxColor	m_nxc1;
	CNxColor	m_nxc2;
	CNxColor	m_nxc3;
	CNxColor	m_nxc4;
	CNxEdit	m_nxeditAnesthesia;
	CNxEdit	m_nxeditPatCoord;
	// (j.dinatale 2012-07-10 16:07) - PLID 3073 - no longer need these
	/*CNxEdit	m_nxeditBillAmt;
	CNxEdit	m_nxeditBillDatePic;
	CNxEdit	m_nxeditBillBalance;*/
	CNxEdit	m_nxeditPrepaysEntered;
	CNxEdit	m_nxeditPrepaysApplied;	
	CNxStatic	m_nxstaticQuoteAmt;
	CNxStatic	m_nxstaticRemainingAmt;
	CNxStatic	m_nxstaticProcInfo;
	CNxStatic	m_nxstaticSurgInfo;
	CNxIconButton m_btnAddProc;
	CNxIconButton m_btnRemoveProc;
	CNxIconButton m_btnAddProcDetail;
	CNxIconButton m_btnRemoveProcDetail;
	CNxIconButton m_btnAddPrePay;
	CNxIconButton m_btnAddBill;
	CNxIconButton m_btnNewQuote;
	CNxIconButton m_btnViewQuote;
	CNxIconButton m_btnAnesthesiaConfig;
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDefaultQuote;
	NxButton m_nxbtnCoSurgeonProvider;
	NxButton m_nxbtnCoSurgeonRefPhys;
	// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
	CNxIconButton m_btnViewCase;
	CNxIconButton m_btnNewCase;
	CNxIconButton m_btnDefaultCase;
	CNxStatic	m_nxstaticCaseHistoryLabel;
	CNxIconButton m_btnUnapplyQuote;// (a.vengrofski 2010-03-16 13:45) - PLID <34617> - New Button
	CNxIconButton m_btnUnapplyBill;	// (j.dinatale 2011-10-03 16:06) - PLID 43528 - unapply bill button
	CNxIconButton m_btnPreOpCalendar;// (d.singleton 2012-04-30 14:26) - PLID 
	CNxIconButton m_btnMarkBillActive;	// (j.dinatale 2012-07-10 16:54) - PLID 3073
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcInfoCenterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	NXTIMELib::_DNxTimePtr m_nxtArrivalTime;

	bool m_bProcLoaded;

	// (a.walling 2008-05-05 11:11) - PLID 29894 - Surgery appointment ID; -2 = uninit, -1 = none
	long m_nSurgApptID;

	// (a.walling 2008-06-11 09:32) - PLID 30351 - Font for labels
	CFont* m_pFont;

public:
	void Load(int nArea);

public:
	// (c.haag 2007-03-07 15:58) - PLID 25110 - Patient ID and name functions
	long GetPatientID() const;
	CString GetPatientName() const;

protected:
	void LoadProcSpecific(); //Loads just the stuff that's dependent on selected procedure.

	void Save(int nID);

	// (j.jones 2009-08-06 13:50) - PLID 7397 - tracked active quote amount
	COleCurrency m_cyActiveQuoteAmount;

	//Goes through each row in the appts datalist, and sets the fore and back colors as appropriate.
	//It is assumed that there is no dragging taking place.
	void SetApptColors();
	
	long CProcInfoCenterDlg::GetActiveQuoteID();	//DRT 7/5/2007 - PLID 23496
	
	long m_nLastSelQuote;

	// (j.jones 2009-08-06 15:01) - PLID 7397 - added m_nLastSelCase
	long m_nLastSelCase;

	CBillingModuleDlg* m_pBillingDlg;
	CWnd* m_pOldFinancialDlg;

	bool m_bLButtonDownHandled;

	void ApplyPrePaysToBill(long iBillID);

	BOOL CheckWarnPersonLicenses(long nPersonID, CString strPersonType);

	// (z.manning 2008-11-19 10:05) - PLID 31687 - Function to requey the co-surgeon combo now that
	// it can also include referrying physicians.
	void RequeryCoSurgeonCombo();

	void HandleCoSurgeonRequeryFinished();
	void LoadHiddenCoSurgeonRow();

	// (z.manning 2008-11-19 11:21) - PLID 31687 - Added this variable to keep track of the co-surgeon ID
	_variant_t m_varCoSurgeonID;

	// (d.singleton 2012-04-25 12:07) - PLID 
	CArray<long, long> m_arProcedureIDs;

	// (a.walling 2007-11-07 10:18) - PLID 27998 - VS2008 - OnPostEditBill should use WPARAM/LPARAM
	// Generated message map functions
	//{{AFX_MSG(CProcInfoCenterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedProcNameList(short nFlags);
	virtual void OnOK();
	afx_msg void OnDefaultQuote();
	afx_msg void OnRequeryFinishedQuotes(short nFlags);
	afx_msg void OnSelChosenQuotes(long nRow);
	afx_msg void OnViewQuote();
	afx_msg void OnSelChosenSurgeon(long nRow);
	afx_msg void OnSelChosenNurse(long nRow);
	afx_msg void OnSelChosenAnesthesiologistList(long nRow);
	afx_msg void OnAddPrepay();
	afx_msg void OnPrepayNew();
	afx_msg void OnBnClickedUnapplyQuote();// (a.vengrofski 2010-02-08 09:23) - PLID <34617> - Added a way to remove quotes from the PIC.
	afx_msg void OnPrepayExisting();
	afx_msg void OnAddBill();
	afx_msg void OnBillNew();
	afx_msg void OnBillExisting();
	afx_msg void OnBillActiveQuote();
	afx_msg LRESULT OnPostEditBill(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRequeryFinishedOtherAppts(short nFlags);
	afx_msg void OnAdd();
	afx_msg void OnApptNew();
	afx_msg void OnApptExisting();
	afx_msg void OnNewQuotePic();
	afx_msg void OnAddProc();
	afx_msg void OnRemoveProc();
	virtual void OnCancel();
	afx_msg void OnKillFocusArrivalHr();
	afx_msg void OnDragBeginOtherAppts(BOOL FAR* pbShowDrag, long nRow, short nCol, long nFlags);
	afx_msg void OnDragOverCellOtherAppts(BOOL FAR* pbShowDrop, long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
	afx_msg void OnDragEndOtherAppts(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
	afx_msg void OnAnesthesiaConfig();
	afx_msg void OnSelChosenAnesthesiaList(long nRow);
	afx_msg void OnEditingFinishedProcNameListDetail(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnSelChangedProcNameList(long nNewSel);
	afx_msg void OnRemoveProcDetail();
	afx_msg void OnAddProcDetail();
	afx_msg void OnPreviewPic();
	afx_msg void OnLButtonDownOtherAppts(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnLButtonUpOtherAppts(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTrySetSelFinishedNurse(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedAnesthesiologistList(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedSurgeon(long nRowEnum, long nFlags);
	afx_msg void OnSelChosenCoSurgeon(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (z.manning 2008-11-19 11:02) - PLID 31687 - Added functions to support referring physicians in the co-surgeon field
	afx_msg void OnBnClickedCosurgeonRefphys();
	afx_msg void OnBnClickedCosurgeonProvider();
	void RequeryFinishedCosurgeon(short nFlags);
	// (j.jones 2009-08-06 14:01) - PLID 7397 - added case history to the PIC
	afx_msg void OnViewCase();
	afx_msg void OnNewCase();
	afx_msg void OnSelChosenCaseHistoryPic(long nRow);
	afx_msg void OnRequeryFinishedCaseHistoryPic(short nFlags);	
	afx_msg void OnDefaultCaseHistory();
	// (j.dinatale 2011-10-03 17:07) - PLID 43528 - unapply a bill
	afx_msg void OnBnClickedUnapplyBill();
	// (d.singleton 2012-04-04 17:40) - PLID 
	afx_msg void OnBnClickedPreOpSched();
	afx_msg void OnAddNewCalendar();
	afx_msg void OnOpenExistingCalendar();

public:
	// (j.dinatale 2012-07-10 16:31) - PLID 3073 - be able to mark a bill active
	afx_msg void OnBnClickedPicMarkBillActive();
	// (j.dinatale 2012-07-10 16:40) - PLID 3073 - ensure active bill is selected
	afx_msg void OnRequeryFinishedBillComboPic(short nFlags);	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCINFOCENTERDLG_H__F578B8EB_2146_4CF7_9BB7_73AEE0EF17E4__INCLUDED_)
