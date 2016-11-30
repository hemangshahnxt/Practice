#pragma once

#include "BOLDSoapUtils.h"
// CBoldLinkDlg dialog
// (j.gruber 2010-06-01 10:58) - PLID 38337 - created for

/*THESE exist in data and cannot change!!*/
// (j.gruber 2010-06-07 12:09) - PLID 38211
enum BoldVisitTypes {
	bvtPatient = 1,	
	bvtPrevBar,
	bvtInsurance,
	bvtPreOp,
	bvtHospital,
	bvtAEBeforeDischarge,
	bvtPostOp,
	bvtPostOpAE,
	bvtSelfReported,
	bvtWeightCheck,
	bvtOther,
};


// (j.gruber 2010-06-07 12:10) - PLID 38211
struct BOLDDetail {
	long nSelectID;
	CString strSelection;
	CString strSelectCode;
};

// (j.gruber 2010-06-07 12:10) - PLID 38211
struct BOLDItem {
	long nItemID;
	CString strRow;		
	CArray<BOLDDetail*, BOLDDetail*> *paryDetails;
};

// (j.gruber 2010-06-07 12:10) - PLID 38211
struct BOLDCodeInfo {
	CString strCode;
	CString strCodeDesc;
	BOOL bAllowMultiples;	
	CArray<BOLDItem*, BOLDItem*> *aryItems;
	BOOL bContainsEMNID;
};

class CBoldLinkDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CBoldLinkDlg)

public:
	CBoldLinkDlg(CWnd* pParent);   // standard constructor
	virtual ~CBoldLinkDlg();

// Dialog Data
	enum { IDD = IDD_BOLD_LINK_DLG };

protected:
	// (j.gruber 2010-06-07 12:10) - PLID 38337
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxIconButton m_btnSearch;
	CNxIconButton m_btnSend;
	CNxIconButton m_btnValidate;
	CNxLabel	m_nxlMultiProviders;
	CNxLabel	m_nxlMultiProcedures;
	CNxEdit		m_nxeditUsername;
	CNxEdit		m_nxeditPassword;
	NxButton  m_rdBoldSent;
	NxButton  m_rdBoldNotSent;
	NxButton  m_rdBoldAll;
	NxButton  m_chkFilterDates;
	NXDATALIST2Lib::_DNxDataListPtr m_pSearchList, m_pProviderFilter;

	// (j.gruber 2010-06-07 12:10) - PLID 38337
	BOOL EnsureValidSearchDateRange();
	//void RequeryProviderList();
	// (j.gruber 2010-06-07 12:10) - PLID 38337
	void LoadList();
	// (j.gruber 2010-06-07 12:10) - PLID 38337
	NXDATALIST2Lib::IRowSettingsPtr AddPatientRow(long nPatientID, long nEMNID, long nUserDefinedID, CString strPatientName, COleDateTime dtSendDate, long nSortOrder, COleDateTime dtEMN);
	// (j.gruber 2010-06-07 12:10) - PLID 38337
	void AddStepRow(NXDATALIST2Lib::IRowSettingsPtr pParentRow, long nEMNID, COleDateTime dtEMN, CString strEMNDescription, long nBoldTypeID, CString strBoldTypeName, COleDateTime dtLastSent, long nSortOrder);
	
	// (j.gruber 2010-06-07 12:10) - PLID 38211
	CString GetBOLDField(CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);	
	void BuildStringArray(CStringArray *strary, CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void GetValuesFromArray(CArray<BOLDItem*, BOLDItem*> *aryItems, CStringArray *strAry, long nItemID);
	void BuildSelectIDArray(CStringArray *strary, CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void GetSelectIDValuesFromArray(CArray<BOLDItem*, BOLDItem*> *aryItems, CStringArray *strAry, long nItemID);
	BOLDCodeInfo * GetBOLDInfo(CString strCode, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo);

	// (j.gruber 2010-06-07 12:10) - PLID 38211
	CString GetBOLDMedCode(CString strNDC, long nFormatIndicator);
	void ConvertToMedicationCodeArray(CStringArray *strarySelectIDs);
	
	// (j.gruber 2010-06-07 12:10) - PLID 38949
	BOOL DoesPrevSurgExist(CPtrArray *prevSurgeries, long nEMNID);
	CString GetNameFromCode(CString strCode);

	// (j.gruber 2010-06-07 12:11) - PLID 38949
	void FillPatientVisitInfo(long nPatientID, long nEMNID, BOLDPatientVisitInfo *pPatVisit, CString strPatientName);
	void FillPatientVisit(BOLDPatientVisitInfo *pPatVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void FillPrevBariatricSurgeries(CPtrArray *pPrevSurgs, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	BOOL GetPreviousBariatricProcedures(ADODB::_RecordsetPtr rsPat, CPtrArray *prevSurgeries, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo);
	
	// (j.gruber 2010-06-07 12:11) - PLID 38951
	void FillPreOpVisitInfo(long nPatientID, long nEMNID, BOLDPreOpVisitInfo *pPreOpVisit, CString strPatientName);
	void FillPreOpVisit(long nEMNID, BOLDPreOpVisitInfo *pPreOpVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);

	// (j.gruber 2010-06-07 12:11) - PLID 38951
	void FillHospVisitInfo(long nPatientID, long nEMNID, BOLDHospitalVisit *pHospVisit, CString strPatientName);
	void FillHospVisit(long nEMNID, BOLDHospitalVisit *pHospVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void FillAdverseEvents(CPtrArray *paryAdverseEvents, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	BOOL GetAdverseEvents(ADODB::_RecordsetPtr rsAE, CPtrArray *paryAdverseEvents, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo);
	BOOL DoesAdverseEventExist(CPtrArray *paryAE, long nEMNID);

	// (j.gruber 2010-06-07 12:11) - PLID 38953
	void FillPostOpVisit(long nEMNID, BOLDPostOpVisitInfo *pPostOpVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void FillPostOpVisitInfo(long nPatientID, long nEMNID, BOLDPostOpVisitInfo *pPostOpVisit, CString strPatientName);

	// (j.gruber 2010-06-07 12:11) - PLID 38955
	void FillGeneralVisit(long nEMNID, BOLDGeneralVisitInfo *pGenVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void FillGeneralVisitInfo(long nPatientID, long nEMNID, BOLDGeneralVisitInfo *pGenVisit, long nVisitType, CString strPatientName);

	// (j.gruber 2010-06-07 12:11) - PLID 38955
	void FillOtherVisit(long nEMNID, BOLDOtherVisitInfo *pOthVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void FillOtherVisitInfo(long nPatientID, long nEMNID, BOLDOtherVisitInfo *pOtherVisit, CString strPatientName);

	// (j.gruber 2010-06-07 12:11) - PLID 38954
	void FillPostOpAEVisit(long nEMNID, BOLDPostOpAEVisitInfo *pPOAEVisit, CArray<BOLDCodeInfo*, BOLDCodeInfo*> *aryInfo, CMap<CString, LPCTSTR, CString, LPCTSTR> *mapValuesFromDialog);
	void FillPostOpAEVisitInfo(long nPatientID, long nEMNID, BOLDPostOpAEVisitInfo *pPOAEVisit, CString strPatientName);

	// (j.gruber 2010-06-07 12:11) - PLID 38337
	void SetErrorMessages(NXDATALIST2Lib::IRowSettingsPtr pRow, CStringArray *staryMessages);
	void SaveVisit(NXDATALIST2Lib::IRowSettingsPtr pRow, long nEMNID, long nPatientID, long nBoldTypeID, CString strVisitID);

	// (j.gruber 2010-06-07 12:12) - PLID 38211
	BOOL CheckDate(COleDateTime dt, CStringArray *aryMessages, CString strField);
	BOOL CheckBool(BOLDBOOL bBool, CStringArray *aryMessages, CString strField);
	BOOL CheckString(CString str, CStringArray *aryMessages, CString strField);
	BOOL CheckBMU(BOLDMetricUnitType bmut, CStringArray *aryMessages, CString strField);
	BOOL CheckSurgeon(CString str, CStringArray *aryMessages, BoldVisitTypes bvType, CString strField);
	BOOL CheckFacility(CString str, CStringArray *aryMessages, BoldVisitTypes bvType, CString strField);
	BOOL CheckDouble(double dbl, CStringArray *aryMessages, CString strField);

	// (j.gruber 2010-06-07 12:12) - PLID 38949
	BOOL ValidatePatientVisit(BOLDPatientVisitInfo *pPatVisit, CStringArray *paryMessages);
	// (j.gruber 2010-06-07 12:12) - PLID 38950
	BOOL ValidatePreOpVisit(BOLDPreOpVisitInfo *pPreOpVisit, CStringArray *paryMessages);
	// (j.gruber 2010-06-07 12:12) - PLID 38951
	BOOL ValidateHospitalVisit(BOLDHospitalVisit *pHospVisit, CStringArray *paryMessages);
	// (j.gruber 2010-06-07 12:12) - PLID 38953
	BOOL ValidatePostOpVisit(BOLDPostOpVisitInfo *pPostOpVisit, CStringArray *paryMessages);
	// (j.gruber 2010-06-07 12:12) - PLID 38954
	BOOL ValidatePostOpAEVisit(BOLDPostOpAEVisitInfo *pPOAEVisit, CStringArray *paryMessages);
	// (j.gruber 2010-06-07 12:12) - PLID 38955
	BOOL ValidateGeneralVisit(BOLDGeneralVisitInfo *pGenVisit, CStringArray *paryMessages);
	BOOL ValidateOtherVisit(BOLDOtherVisitInfo *pOthVisit, CStringArray *paryMessages);

	// (j.gruber 2010-06-07 12:13) - PLID 38337
	BOOL CheckPriorVisitTypes(NXDATALIST2Lib::IRowSettingsPtr pRow);
	
	// (j.gruber 2010-06-07 12:13) - PLID 38211
	double _DBL(CString str);
	BOLDMetricUnitType _BMU(CString str);
	BOLDBOOL _BOOL(CString str);
	long _LONG(CString str);
	COleDateTime _DATE(CString str);
	BOLDTimeUnit _BTU(CString str);

	CString GetBOLDGenderCode(long nGender);

	void RefillMedicationCodes(CStringArray *arystrMedicationCodes, CStringArray *arystrMessages);
	BOOL FailedMedicationCodesOnly(CStringArray *arystr);
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	// (j.gruber 2010-06-07 12:13) - PLID 38337
public:	
	afx_msg void OnBnClickedBoldSearch();
	afx_msg void OnBnClickedBoldSend();
	DECLARE_EVENTSINK_MAP()
	void LeftClickBoldSendList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);	
	afx_msg void OnBnClickedBoldFilterDates();
	afx_msg void OnBnClickedBoldRdSent();
	afx_msg void OnBnClickedBoldRdNotSent();
	afx_msg void OnBnClickedBoldRdAll();
};
