#if !defined(AFX_HL7EXPORTDLG_H__E02C75A8_063D_40BD_82BD_BB3DF59AA580__INCLUDED_)
#define AFX_HL7EXPORTDLG_H__E02C75A8_063D_40BD_82BD_BB3DF59AA580__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HL7ExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHL7ExportDlg dialog
#include <afxmt.h>

// (r.gonet 12/11/2012) - PLID 54115 - Forward declaration
// (z.manning 2013-05-20 11:20) - PLID 56777 - Renamed
class CHL7Client_Practice;
enum HL7PracticeRecordType;

class CHL7ExportDlg : public CNxDialog
{
// Construction
public:
	// (z.manning 2008-07-18 12:27) - PLID 30782 - Must now specify export type
	CHL7ExportDlg(CWnd* pParent = NULL);   // standard constructor
	// (r.gonet 12/11/2012) - PLID 54115 - Added destructor
	~CHL7ExportDlg();

	// (j.jones 2008-05-08 09:46) - PLID 29953 - added nxiconbuttons for modernization
// Dialog Data
	//{{AFX_DATA(CHL7ExportDlg)
	enum { IDD = IDD_HL7_EXPORT_DLG };
	CNxIconButton	m_btnSend;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRemAll;
	CNxIconButton	m_btnRem;
	CNxIconButton	m_btnAdd;
	CNxStatic	m_nxstaticApptLabel;
	CNxStatic	m_nxstaticApptExportLabel;
	CNxStatic	m_nxstaticPatientLabel;
	CNxStatic	m_nxstaticPatientExportLabel;
	CNxStatic	m_nxstaticEmnBillLabel;
	CNxStatic	m_nxstaticEmnBillExportLabel;
	CNxStatic	m_nxstaticLockedEmnLabel;
	CNxStatic	m_nxstaticLockedEmnExportLabel;
	CNxStatic	m_nxstaticLabResultLabel;
	CNxStatic	m_nxstaticLabResultExportLabel;
	CNxStatic	m_nxstaticRefPhysicianLabel;
	CNxStatic	m_nxstaticRefPhysicianExportLabel;
	NxButton	m_nxbSendPrimaryImage; //TES 9/29/2015 - PLID 66193
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHL7ExportDlg)
	virtual int DoModal(HL7PracticeRecordType eExportType);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pSettings;
	NXDATALISTLib::_DNxDataListPtr m_pPatients;
	NXDATALISTLib::_DNxDataListPtr m_pExport;
	NXDATALISTLib::_DNxDataListPtr m_pAppts; // (z.manning 2008-07-18 12:17) - PLID 30782
	NXDATALISTLib::_DNxDataListPtr m_pEmnBills; //TES 7/10/2009 - PLID 34845
	NXDATALISTLib::_DNxDataListPtr m_pLockedEmns; // (d.singleton 2012-10-04 16:39) - PLID 53041
	NXDATALISTLib::_DNxDataListPtr m_pLabResults; // (d.singleton 2012-10-04 16:39) - PLID 53282
	NXDATALISTLib::_DNxDataListPtr m_pSyndromicList; // (b.spivey, November 1, 2013) - PLID 59267
	NXDATALISTLib::_DNxDataListPtr m_pReferringPhyList; // (r.farnworth 2014-12-22 14:46) - PLID 64473

	CMutex m_mtxProcessMessage;

	HL7PracticeRecordType m_eExportType;

	// (r.gonet 12/11/2012) - PLID 54115 - Manages connection between NxServer and us for HL7.
	// (z.manning 2013-05-20 11:21) - PLID 56777 - Renamed
	CHL7Client_Practice *m_pHL7Client;

	// (z.manning 2008-07-18 12:26) - PLID 30782 - Functions to show/hide patient or appointment controls
	void ShowApptControls(UINT nShow);
	void ShowPatientControls(UINT nShow);
	//TES 7/10/2009 - PLID 34845
	void ShowEmnBillControls(UINT nShow);
	// (d.singleton 2012-10-04 16:56) - PLID 53041
	void ShowLockedEmnControls(UINT nShow);
	// (d.singleton 2012-10-19 16:43) - PLID 43282
	void ShowLabResultControls(UINT nShow);
	// (r.farnworth 2014-12-22 14:28) - PLID 64473
	void ShowReferringPhyControls(UINT nShow);

	// (b.spivey, November 1, 2013) - PLID 59267
	void ShowSyndromicControls(UINT nShow);

	// (b.spivey, November 1, 2013) - PLID 59267 - for returning syndromic data. 
	// (r.gonet 03/18/2014) - PLID 60782 - We now store the diagnosis code IDs rather than the code numbers due to ICD-9, ICD-10 overlap
	CArray<long, long> m_aryDiagCodeIDs;
	CArray<long, long> m_arySyndromicPersonIDs;

	CString GetExportDescription();

	//TES 11/10/2015 - PLID 67500 - Separate export functionality, for message types that use the mass export
	void ExportUsingMultiple();

	//TES 9/21/2010 - PLID 40595 - Cache whether we're currently excluding prospects, and whether we've requeried at least once.
	BOOL m_bExcludeProspects;
	BOOL m_bRequeriedOnce;

	// Generated message map functions
	//{{AFX_MSG(CHL7ExportDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnHl7Send();
	afx_msg void OnHl7Add();
	afx_msg void OnHl7Remove();
	afx_msg void OnHl7RemoveAll();
	afx_msg void OnHl7Close();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellPracList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellExportList(long nRowIndex, short nColIndex);
	afx_msg void OnEditSettings();
	afx_msg void OnDblClickCellApptList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellApptExportList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLockedEmnList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLockedEmnExportList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLabResultList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLabResultExportList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSyndromicSurveillanceExportList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSyndromicSurveillanceList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellRefPhysList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellRefPhysExport(long nRowIndex, short nColIndex);

	virtual int DoModal();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void OnDblClickCellHl7exportEmnBillList(long nRowIndex, short nColIndex);
	void OnDblClickCellHl7exportEmnBillExportList(long nRowIndex, short nColIndex);
	void OnSelChosenSettingsList(long nRow);

	// (b.spivey, November 1, 2013) - PLID 59267 - accessor and mutator for diagnosis codes and personIDs respectively. 
	// (r.gonet 03/18/2014) - PLID 60782 - Changed the array to diagnosis code IDs rather than the code numbers due to ICD-9, ICD-10 overlap
	void SetDiagnosisCodeArray(const CArray<long, long>& aryDiagCodeIDs); 
	void GetSyndromicPersonIDs(CArray<long, long>& aryPersonIDs);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HL7EXPORTDLG_H__E02C75A8_063D_40BD_82BD_BB3DF59AA580__INCLUDED_)
