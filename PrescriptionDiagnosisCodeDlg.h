#pragma once

#include "PatientsRc.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "DiagSearchConfig.h"

// (b.savon 2013-09-24 08:05) - PLID 58747 - Move the Diagnosis Codes from the Prescription Edit screen to a separate dialog - Created

// CPrescriptionDiagnosisCodeDlg dialog

class CPrescriptionDiagnosisCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPrescriptionDiagnosisCodeDlg)
private:
	CNxColor m_nxcBackground;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnMoveDiagUp;
	CNxIconButton m_btnMoveDiagDown;
	// (r.gonet 03/21/2014) - PLID 61187 - Diagnosis Dual Search control
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDiagnosisSearch;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDiagnosisCodes;

	void EnableDiagButtons();
	void CommitDiagnosisCodeList();

	long m_nPatientMedicationID;
	CString m_strDiagnosisCodeDisplay;
	Nx::SafeArray<int> m_saryDiagnosisCodes;
	BOOL m_bCommitChanges;

public:
	CPrescriptionDiagnosisCodeDlg(long nPatientMedicationID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CPrescriptionDiagnosisCodeDlg();
	virtual BOOL OnInitDialog();
	
	inline Nx::SafeArray<int> GetDiagnosisCodes(){ return m_saryDiagnosisCodes; }
	inline BOOL CommitChanges(){ return m_bCommitChanges; }
	inline CString GetDiagnosisCodeDisplay(){ return m_strDiagnosisCodeDisplay; }

// Dialog Data
	enum { IDD = IDD_PRESCRIPTION_DIAGNOSIS_CODES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	// (r.gonet 03/21/2014) - PLID 61187 - Add a diagnosis code selected from the dual search to the list and save it.
	void AddDiagCode(const CDiagSearchResults::CDiagCode& diagCode);
	void OnSelChangedDiagnosisCodes(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnMoveDiagUp();
	afx_msg void OnMoveDiagDown();
	void OnRButtonUpDiagnosisCodes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (r.gonet 03/21/2014) - PLID 61187 - Handle the case where the user selects a search result from the diagnosis dual search.
	void SelChosenRxDiagCodeSearch(LPDISPATCH lpRow);
};
