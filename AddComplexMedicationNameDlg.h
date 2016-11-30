#pragma once

// CAddComplexMedicationNameDlg dialog
// (d.thompson 2008-12-01) - PLID 32175 - Created

// (j.jones 2014-07-29 10:09) - PLID 63085 - forward declarations
// in the Accessor namespaces allow us to remove .h includes
namespace NexTech_Accessor
{
	enum DrugType;
}

class CAddComplexMedicationNameDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAddComplexMedicationNameDlg)

public:
	CAddComplexMedicationNameDlg(CWnd* pParent);   // standard constructor
	virtual ~CAddComplexMedicationNameDlg();

	// (j.jones 2009-06-12 15:19) - PLID 34613 - required when editing an existing description
	long m_nMedicationID;

	// (d.thompson 2008-12-10) - PLID 32175 - DoModal, with a flag to auto calculate the description.
	UINT BeginAddNew();

	// (d.thompson 2008-11-25) - PLID 32175 - This dialog is designed to support letting
	//	the user enter drug name information, and it will calculate the name properly
	//	for the user based on the possible fields.  No data access, just give and take.
	IN OUT CString m_strDrugName;
	IN OUT CString m_strStrength;
	IN OUT long m_nStrengthUnitID;
	IN OUT long m_nDosageFormID;
	IN OUT CString m_strFullDrugDescription;
	// (s.dhole 2012-11-16 10:22) - PLID 53697 
	IN OUT long m_nQuntityUnitID;
	// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
	IN OUT long m_nRouteID;
	OUT CString m_strDosageRoute;

	// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added Dosage Unit
	IN OUT long m_nDosageUnitID;
	OUT CString m_strDosageUnit;

	// (d.thompson 2008-12-10) - PLID 32175 - Disables the ability to change the drug description field.
	IN bool m_bAllowDrugDescriptionToChange;
	// (j.gruber 2010-11-02 12:25) - PLID 39048 - ID for FDB if connected, -1 otherwise
	IN long m_nFDBID;
	// (j.fouts 2012-11-27 16:46) - PLID 51889 - Added drug type and notes
	IN OUT NexTech_Accessor::DrugType m_drugType;
	IN OUT CString m_strNotes;

// Dialog Data
	enum { IDD = IDD_ADD_COMPLEX_MEDICATION_NAME_DLG };

protected:
	//Member controls
	NXDATALIST2Lib::_DNxDataListPtr m_pStrengthList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDosageList;
	NXDATALIST2Lib::_DNxDataListPtr m_pQuntityList;
	// (j.fouts 2013-02-01 15:08) - PLID 54528 - Added Dosage Route
	NXDATALIST2Lib::_DNxDataListPtr m_pRouteList;
	// (j.fouts 2013-02-01 15:05) - PLID 54985 - Added Dosage Unit and an NxColor
	NXDATALIST2Lib::_DNxDataListPtr m_pDosageUnitsList;
	CNxColor m_bkg;
	CNxEdit m_editDrugName;
	CNxEdit m_editStrength;
	CNxEdit m_editFullDesc;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	// (j.fouts 2012-11-27 16:46) - PLID 51889 - Optional Text controls set based on selected drug type
	CNxStatic m_nxstaticOptionalText1;
	CNxStatic m_nxstaticOptionalText2;
	// (j.fouts 2012-11-27 16:46) - PLID 51889 - Added radio controls for drug type
	NxButton m_radioSingleMed;
	NxButton m_radioSupply;
	NxButton m_radioCompound;
	// (d.thompson 2008-12-10) - PLID 32175 - Determines if the "Full Description" field should auto calculate
	//	as changes are made to the 4 sub-data fields.
	bool m_bAutoCalcName;

	//virtuals
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	//Functionality
	void UpdateDrugDescription();

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenUnitsList(LPDISPATCH lpRow);
	void OnSelChosenDosageList(LPDISPATCH lpRow);
	afx_msg void OnChangeDrugName();
	afx_msg void OnChangeDrugStrength();
	afx_msg void OnChangeDrugDescription();
	void OnSelChangingUnitsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingDosageList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingQuntityUnitsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);

public:
	afx_msg void OnBnClickedDrugPrescriptionBtn();
	afx_msg void OnBnClickedSupplyPrescriptionButton();
	afx_msg void OnBnClickedCompoundPrescriptionButton();
};
