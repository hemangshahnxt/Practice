#pragma once
#include "PracticeRc.h"
// (s.dhole 2013-06-18 14:54) - PLID 57219 new Dialog

// CReconcileMergeMedicationsDlg dialog

class CReconcileMergeMedicationsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CReconcileMergeMedicationsDlg)

public:
	CReconcileMergeMedicationsDlg(CWnd* pParent = NULL);   // standard constructor
	
	virtual ~CReconcileMergeMedicationsDlg();

// Dialog Data
	enum { IDD = IDD_RECONCILE_MEDICATIONS_MERGE_DLG };

public:
	virtual BOOL OnInitDialog();

	enum ReconciliationType
	{
		erMergNone = 0L,
		erMergMedication,// (s.dhole 2013-10-30 15:12) - PLID 59235
		erMergAllergy,
		erMergProblem,// (s.dhole 2013-10-30 11:58) - PLID 56933
	};
	ReconciliationType m_ReconciliationType;
	struct ReconcileValidationItem
	{
		enum { eKeepCurItem  , eAddItem, eDeleteItem, eMergeCurItem , eExcludeCurItem } Action; // Indicates what we're doing to the record
		CString strName; // The name of the prescription
		CString strDescription;
		COleDateTime dtLastDate;
		BOOL bIsActive;
		CString strCurrentName; // The name of the medication
		CString strCurrentDescription;
		BOOL bCurrentIsActive;
		COleDateTime dtCurrentLastDate;
	};
	typedef CArray<ReconcileValidationItem,ReconcileValidationItem&> CReconcileValidationItemArray;
	CReconcileValidationItemArray m_aReconcileValidationItem;
	
	// (b.spivey, October 30, 2015) PLID 67514
	CString m_strOkButtonOverrideText;
	
	
protected:
	void LoadData();
	CNxColor m_nxcReconcilationBack;
	CNxIconButton m_btnReconcilationOk;
	CNxIconButton m_btnReconcilationCancel;
	OLE_COLOR m_clrReconcilationBack;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	NXDATALIST2Lib::_DNxDataListPtr	m_dlMergeMedRx;
	DECLARE_MESSAGE_MAP()
	void SetColumnHeaders();
	void UpdateColumnCaption(NXDATALIST2Lib::_DNxDataListPtr ListPtr, int  nColumn, CString strCaption );
public: 
	inline void SetBackColor(OLE_COLOR clr) { m_clrReconcilationBack = clr; }
	afx_msg void OnBnClickedReconciliationOk();
	afx_msg void OnBnClickedReconciliationCancel();
};
