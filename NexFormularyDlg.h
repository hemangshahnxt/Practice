
#pragma once
#include "PatientsRc.h"
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include "NxAPI.h"

// (b.savon 2013-08-14 14:04) - PLID 57587 - Created 

// CNexFormularyDlg dialog
enum NexFormularySource{
	nfsPrescriptionEditDlg,
	nfsSearchList,
	nfsQueue,
};

struct NexFormularyDrug{
	CString strDrugName;
	long nFDBID;

	NexFormularyDrug()
	{
		strDrugName = "";
		nFDBID = -1;
	}
};

class CNexFormularyDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexFormularyDlg)
private:
	//Controls
	CNxIconButton m_btnClose;
	// (b.cardillo 2013-09-24 20:20) - PLID 58749 - Use multiple nxcolors instead of a groupbox inside nxcolor
	CNxColor m_nxcBackgroundEligibility;
	CNxColor m_nxcBackgroundDrug;
	CNxColor m_nxcBackgroundAlternates;
	CNxStatic m_nxsMedication;
	CNxStatic m_nxsInsurance;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlInsurance;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlFormulary;
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlDrug;
	HICON m_hIconWriteMedication;
	HICON m_hIconError;
	CNxIconButton m_btnWrit;// (s.dhole 2013-08-28 10:33) - PLID 58000
	CNxIconButton m_btnViewDerm;
	//Members Varz
	long m_nPatientID;
	long m_nFDBID;
	long m_nDrugListID; // (r.farnworth 2013-08-14 10:45) - PLID 58001
	long m_nProviderID; // (r.farnworth 2013-09-24 16:21) - PLID 58386
	NexFormularySource m_nfsSource;
	CString m_strDrugName;
	BOOL m_bInsuranceRequeryFinished;
	BOOL m_bMedicationRequeryFinished;
	NexTech_Accessor::_FormularyDataResponsePtr m_fdrFormularyResponse;
	long m_nFInsuranceID; // (s.dhole 2013-10-18 13:01) - PLID 59068
	//Functions
	void PrepareDialog();
	void PrepareControls();
	void LoadInsuranceDatalist();
	void CheckRequeryStatus();
	void GetFormularyInformation();
	BOOL IsValidRequestState();
	long GetMedicationFDBID();
	long GetInsuranceResponseDetailID();
	NexTech_Accessor::_FormularyDataRequestPtr GetFormularyDataRequestPtr();
	void DisplayFormularyResults(NexTech_Accessor::_FormularyInsurancePtr formularyInsurance);
	void AddUnableToMakeEligibilityRequestRow(const CString &strDisplayMessage);
	void AddUnableToMakeFormularyRequestRow(const CString &strDisplayMessage);
	// (s.dhole 2013-08-20 10:10) - PLID 58000 
	void AddFormularySuccessRow(const Nx::SafeArray<IUnknown *> &saryDruggList);
	void AddFormularyAlternative(const Nx::SafeArray<IUnknown *> &saryAlternatives);
	void AddUnknownFailureRow();
	void AddErrorRow(const CString& strErrorMessage);
	CString BuildDisplayMessage(const Nx::SafeArray<BSTR> &saryDisplayMessage);
	// (s.dhole 2013-08-20 10:10) - PLID 58000 
	//TES 8/28/2013 - PLID 57999 - Moved GetCopayInformation to PrescriptionUtilsAPI
	void LoadSearchMedication(const BOOL& bEnabale,const CString& strMedication,const CString& strCoverage,const CString& strCopay,const CString& strStatus, const long &nStatus );
	long m_nStatus;
	long m_nLastInsuranceID; // (r.farnworth 2013-09-24 10:04) - PLID 58358 - Want to track the last insurance selected.
	// (s.dhole 2013-09-12 17:26) - PLID 58556
	void CheckExistingFormularyData();
public:
	// (s.dhole 2013-10-28 10:08) - PLID 59084 added nFInsuranceID
	CNexFormularyDlg(NexFormularySource nfsSource, long nPatientID, NexFormularyDrug nfdDrug, long nProviderID,long nFInsuranceID= -1, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexFormularyDlg();
	virtual BOOL OnInitDialog();

	inline long GetDrugListID() { return m_nDrugListID; } // (r.farnworth 2013-08-14 10:45) - PLID 58001
	inline long GetFDBID() { return m_nFDBID; } //TES 9/24/2013 - PLID 58287
	inline CString GetDrugName() { return m_strDrugName; } //TES 9/24/2013 - PLID 58287
	inline long GetFInsuranceID() { return m_nFInsuranceID; } // (s.dhole 2013-10-18 13:01) - PLID 59068
// Dialog Data
	enum { IDD = IDD_NEXFORMULARY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void OnLeftClickFormularyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags); // (r.farnworth 2013-08-13 16:40) - PLID 58001

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedNxdlNexformularyInsurance(short nFlags);
	void RequeryFinishedNxdlNexformularyDrug(short nFlags);
	void SelChangingNxdlNexformularyDrug(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingNxdlNexformularyInsurance(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenNxdlNexformularyDrug(LPDISPATCH lpRow);
	void SelChosenNxdlNexformularyInsurance(LPDISPATCH lpRow);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnWriteERx();// (s.dhole 2013-08-28 10:33) - PLID 58000 Reset display
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);// (s.dhole 2013-08-28 10:33) - PLID 58000 Reset display
	afx_msg void OnBnClickedViewDems(); // (j.fouts 2013-09-12 12:11) - PLID 58701 - Added
};
