#pragma once

#include "GlobalInsuredPartyUtils.h"

// (j.gruber 2009-10-13 09:34) - PLID 10723 - created for
// CNewPatientAddInsuredDlg dialog

// (c.haag 2010-10-04 10:21) - PLID 39447 - This structure holds information for
// a single insured party
class CNewPatientInsuredParty
{
public:
	// (j.gruber 2009-10-09 10:53) - PLID 10723 - add insurance fields
	// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
	CString m_strInsFirst, m_strInsMiddle, m_strInsLast, m_strInsTitle, m_strInsAddress1, m_strInsAddress2;
	CString m_strInsCity, m_strInsState, m_strInsZip, m_strInsPhone;
	CString m_strInsSSN, m_strInsEmployer;
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added m_strInsCountry, it's a string because that's what we
	// somewhat foolishly store in data
	CString m_strInsCountry;
	long m_nInsGender;
	COleDateTime m_dtInsBirthDate;

	// (j.gruber 2012-08-01 11:05) - PLID 51908 - moved to public
	CString m_strInsCompanyName; // Cached value
	CString m_strRespType; // Cached value

	// (r.goldschmidt 2014-07-24 11:28) - PLID 62775 - add deductible, pay group list
	bool m_bPerPayGroup;
	COleCurrency m_cyTotalDeductible, m_cyTotalOOP;
	CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues> m_mapPayGroupVals;

	// (c.haag 2010-10-04 10:21) - PLID 39447 - More fields
private:
	long m_nInsCoID;
	
public:
	CString m_strRelationToPt;
	CString m_strPatientIDNumber;
	CString m_strGroupNumber;
private:
	long m_nRespTypeID;
	

public:
	void operator =(const CNewPatientInsuredParty &s)
	{
		m_strInsFirst = s.m_strInsFirst;
		m_strInsMiddle = s.m_strInsMiddle;
		m_strInsLast = s.m_strInsLast;
		// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
		m_strInsTitle = s.m_strInsTitle;
		m_strInsAddress1 = s.m_strInsAddress1;
		m_strInsAddress2 = s.m_strInsAddress2;
		m_strInsCity = s.m_strInsCity;
		m_strInsState = s.m_strInsState;
		m_strInsZip = s.m_strInsZip;
		// (j.jones 2012-11-12 13:32) - PLID 53622 - added m_strInsCountry
		m_strInsCountry = s.m_strInsCountry;
		m_strInsPhone = s.m_strInsPhone;
		m_strInsSSN = s.m_strInsSSN;
		m_strInsEmployer = s.m_strInsEmployer;
		m_nInsGender = s.m_nInsGender;
		m_dtInsBirthDate = s.m_dtInsBirthDate;
		m_nInsCoID = s.m_nInsCoID;
		m_strRelationToPt = s.m_strRelationToPt;
		m_strInsCompanyName = s.m_strInsCompanyName;
		m_strPatientIDNumber = s.m_strPatientIDNumber;
		m_strGroupNumber = s.m_strGroupNumber;
		m_nRespTypeID = s.m_nRespTypeID;
		m_strRespType = s.m_strRespType;

		// (r.goldschmidt 2014-07-24 11:28) - PLID 62775 - add deductible, pay group list
		m_bPerPayGroup = s.m_bPerPayGroup;
		m_cyTotalDeductible = s.m_cyTotalDeductible;
		m_cyTotalOOP = s.m_cyTotalOOP;

		// (r.goldschmidt 2014-07-24 11:28) - PLID 62775 - copy of the CMap. yuck.
		m_mapPayGroupVals.RemoveAll();
		POSITION pos = s.m_mapPayGroupVals.GetStartPosition();
		long nKey;
		InsuredPartyPayGroupValues sPayGroupVals;
		while (pos != NULL){
			s.m_mapPayGroupVals.GetNextAssoc(pos, nKey, sPayGroupVals);
			m_mapPayGroupVals.SetAt(nKey, sPayGroupVals);
		}

	}

	CNewPatientInsuredParty()
	{
		m_nInsGender = 0;
		m_dtInsBirthDate = COleDateTime(0,0,0,0,0,0);
		m_nInsCoID = 0;
		m_nRespTypeID = 0;
		m_bPerPayGroup = false;
		m_cyTotalDeductible.SetStatus(COleCurrency::invalid);
		m_cyTotalOOP.SetStatus(COleCurrency::invalid);
	}

	CNewPatientInsuredParty(const CNewPatientInsuredParty &s)
	: m_strInsFirst(s.m_strInsFirst)
	, m_strInsMiddle(s.m_strInsMiddle)
	, m_strInsLast(s.m_strInsLast)
	, m_strInsTitle(s.m_strInsTitle)
	, m_strInsAddress1(s.m_strInsAddress1)
	, m_strInsAddress2(s.m_strInsAddress2)
	, m_strInsCity(s.m_strInsCity)
	, m_strInsState(s.m_strInsState)
	, m_strInsZip(s.m_strInsZip)
	, m_strInsCountry(s.m_strInsCountry)
	, m_strInsPhone(s.m_strInsPhone)
	, m_strInsSSN(s.m_strInsSSN)
	, m_strInsEmployer(s.m_strInsEmployer)
	, m_nInsGender(s.m_nInsGender)
	, m_dtInsBirthDate(s.m_dtInsBirthDate)
	, m_nInsCoID(s.m_nInsCoID)
	, m_strRelationToPt(s.m_strRelationToPt)
	, m_strInsCompanyName(s.m_strInsCompanyName)
	, m_strPatientIDNumber(s.m_strPatientIDNumber)
	, m_strGroupNumber(s.m_strGroupNumber)
	, m_nRespTypeID(s.m_nRespTypeID)
	, m_strRespType(s.m_strRespType)
	, m_bPerPayGroup(s.m_bPerPayGroup)
	, m_cyTotalDeductible(s.m_cyTotalDeductible)
	, m_cyTotalOOP(s.m_cyTotalOOP)
	{
		// (r.goldschmidt 2014-07-24 11:28) - PLID 62775 - Copy constructor needs to be able to copy the CMap properly. Yuck.
		m_mapPayGroupVals.RemoveAll();
		POSITION pos = s.m_mapPayGroupVals.GetStartPosition();
		long nKey;
		InsuredPartyPayGroupValues sPayGroupVals;
		while (pos != NULL){
			s.m_mapPayGroupVals.GetNextAssoc(pos, nKey, sPayGroupVals);
			m_mapPayGroupVals.SetAt(nKey, sPayGroupVals);
		}
	}

	BOOL CompareDemographics(const CNewPatientInsuredParty &s)
	{
		if (m_strInsFirst.CompareNoCase(s.m_strInsFirst)) return TRUE;
		if (m_strInsMiddle.CompareNoCase(s.m_strInsMiddle)) return TRUE;
		if (m_strInsLast.CompareNoCase(s.m_strInsLast)) return TRUE;
		// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
		if (m_strInsTitle.CompareNoCase(s.m_strInsTitle)) return TRUE;
		if (m_strInsAddress1.CompareNoCase(s.m_strInsAddress1)) return TRUE;
		if (m_strInsAddress2.CompareNoCase(s.m_strInsAddress2)) return TRUE;
		if (m_strInsCity.CompareNoCase(s.m_strInsCity)) return TRUE;
		if (m_strInsState.CompareNoCase(s.m_strInsState)) return TRUE;
		if (m_strInsZip.CompareNoCase(s.m_strInsZip)) return TRUE;
		// (j.jones 2012-11-12 13:32) - PLID 53622 - added m_strInsCountry
		if (m_strInsCountry.CompareNoCase(s.m_strInsCountry)) return TRUE;
		if (m_strInsPhone.CompareNoCase(s.m_strInsPhone)) return TRUE;
		if (m_strInsSSN.CompareNoCase(s.m_strInsSSN)) return TRUE;
		if (m_nInsGender != s.m_nInsGender) return TRUE;
		if (m_dtInsBirthDate != s.m_dtInsBirthDate) return TRUE;
		return FALSE;
	}

	void SetInsuranceCompany(long nID, const CString& strName)
	{
		m_nInsCoID = nID;
		m_strInsCompanyName = strName;
	}

	CString GetInsuranceCompanyName() const
	{
		return m_strInsCompanyName;
	}

	long GetInsuranceCompanyID() const
	{
		return m_nInsCoID;
	}

	void SetRespType(long nID, const CString& strName)
	{
		m_nRespTypeID = nID;
		m_strRespType = strName;
	}

	CString GetRespTypeName() const
	{
		return m_strRespType;
	}

	long GetRespTypeID() const
	{
		return m_nRespTypeID;
	}
};

class CNewPatientAddInsuredDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNewPatientAddInsuredDlg)

public:
	// (j.gruber 2012-08-01 15:20) - PLID 51908 - added fromnewPatientDlg and existingpatientID
	CNewPatientAddInsuredDlg(CNewPatientInsuredParty& party, 
		CNewPatientInsuredParty& patient,
		CWnd* pParent, BOOL bFromNewPatientDlg = TRUE, long nExistingPatientID = -1);   // standard constructor
	virtual ~CNewPatientAddInsuredDlg();

// Dialog Data
	enum { IDD = IDD_NEW_PATIENT_ADD_INSURED_INFO };

protected:
	// (c.haag 2010-10-04 12:59) - PLID 39447 - Loads the demographic values from the party object
	void LoadDemographics(CNewPatientInsuredParty& party);
	// (c.haag 2010-10-04 12:59) - PLID 39447 - Loads the insurance values from the party object
	void LoadInsurance(CNewPatientInsuredParty& party);
	// (c.haag 2010-10-04 15:38) - PLID 39447 - Enables the patient demographic fields
	void EnableControls(BOOL bEnable);
	// (c.haag 2010-10-04 15:59) - PLID 39447 - Copys patient information into the
	// demographic fields
	void CopyPatientInfo();	

	// (r.goldschmidt 2014-07-28 13:26) - PLID 62775 - Loads the deductible/pay group info from the party object
	void LoadPayGroupInfo(CNewPatientInsuredParty& party);
	void DisablePayGroupControls();
	void ClearAndDisablePayGroupControls();
	void TogglePayGroupControls();

protected:
	// (c.haag 2010-10-04 12:59) - PLID 39447 - Insured party information. This is read from
	// when the dialog is initialized, and written to when the user clicks OK
	CNewPatientInsuredParty& m_Party;

	// (c.haag 2010-10-04 12:59) - PLID 39447 - Patient information. This is used for the
	// Copy button.
	CNewPatientInsuredParty& m_Patient;

	// (j.gruber 2012-08-01 11:53) - PLID 51908 - variable to say where its coming from
	BOOL m_bFromNewPatientDlg;
	long m_nExistingPatientID;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnCopyPatientInfo; // (c.haag 2010-10-04 12:59) - PLID 39447
	NxButton m_radioAllPaygroups; // (r.goldschmidt 2014-07-28 10:43) - PLID 62775 - added pay group
	NxButton m_radioIndividualPaygroups;// (r.goldschmidt 2014-07-28 10:43) - PLID 62775 - added pay group

	CNxEdit m_edtFirst;
	CNxEdit m_edtMiddle;
	CNxEdit m_edtLast;
	// (j.jones 2012-10-25 10:50) - PLID 36305 - added Title
	CNxEdit m_edtTitle;
	CNxEdit m_edtAddress1;
	CNxEdit m_edtAddress2;
	CNxEdit m_edtCity;
	CNxEdit m_edtState;
	CNxEdit m_edtZip;
	CNxEdit m_edtEmployer;
	CNxEdit m_edtPhone;
	CNxEdit m_edtSSN;
	// (r.goldschmidt 2014-07-28 10:43) - PLID 62775 - added deductible/oop
	CNxEdit m_edtTotalDeductible;
	CNxEdit m_edtTotalOOP;

	// (c.haag 2010-10-04 10:47) - PLID 39447 - Added insurance controls
	NXDATALIST2Lib::_DNxDataListPtr	m_pInsCoList;
	NXDATALIST2Lib::_DNxDataListPtr	m_pRelationToPatientList;
	NXDATALIST2Lib::_DNxDataListPtr	m_pRespList;
	// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_pCountryList;
	void LoadRelToPatList();

	NXDATALIST2Lib::_DNxDataListPtr m_pGenderList;
	NXTIMELib::_DNxTimePtr m_nxtBirthDate;
	BOOL m_bLookupByCity;
	BOOL m_bFormatPhoneNums;
	BOOL m_bAutoCapMiddle;

	// (r.goldschmidt 2014-07-23 15:58) - PLID 62775 - Add Pay Group List to New Patient Dialog
	NXDATALIST2Lib::_DNxDataListPtr m_pPayGroupsList;
	CString m_strTotalDeductible, m_strTotalOOP;

public: 
	CString m_strPhoneFormat;

protected:	
	BOOL OnInitDialog();
	void SelChangingGenderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
protected:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSetFocusPhoneNumber();
	afx_msg void OnChangeSSN();
	afx_msg void OnKillFocusCityBox();
	afx_msg void OnKillFocusZipCode();
	afx_msg void OnCopyPatientInfo();
	void SelChangingNewpatInsCoList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenNewpatInsCoList(LPDISPATCH lpRow);
	void RequeryFinishedNewpatInsCoList(short nFlags);
	void SelChangingNewpatInsRelation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenNewpatInsRelation(LPDISPATCH lpRow);
	// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
	void OnSelChangingCountryList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
public:
	// (r.goldschmidt 2014-07-23 17:34) - PLID 62775 - Add Deductible, Pay Group Info
	void EditingStartingNewpatPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishingNewpatPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnBnClickedAllPayGroupsRadio();
	afx_msg void OnBnClickedIndividualPayGroupsRadio();
	afx_msg void OnEnKillfocusTotalDeductibleEdit();
	afx_msg void OnEnKillfocusTotalOopEdit();
	void RequeryFinishedNewpatPayGroupList(short nFlags);
	// (s.tullis 2016-02-11 11:43) - 68212
	afx_msg void OnEnKillfocusSsnBox();
};
