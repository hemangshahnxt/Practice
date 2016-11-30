#pragma once

#include "PrescriptionUtilsAPI.h"

// (j.fouts 2013-04-12 10:24) - PLID 56155 - Created this to generate a sig
// (b.savon 2016-01-19 15:29) - PLID 59879 - Added AutoCalculate

class AutoPopulatingSig
{
public:
	AutoPopulatingSig();

	bool GetSig(CString &strTextOut);
	bool GetQuantity(double *pValueOut);
	bool GetDaysSupply(long *pValueOut);
	bool GetAutoCalculate();

	void SetRoute(CString &strText);
	void SetDosageQuantity(CString &strText, double dValue);
	void SetUnit(CString &strText, CString &strTextPlural);
	void SetFrequency(CString &strText, double dValue);
	void SetQuantity(CString &strText);
	void SetDaysSupply(CString &strText);

	void SetSig(CString &strText);

	void SetAutoCalculate(const NexTech_Accessor::_NexERxMedicationPackageInformationPtr &pPackage);

private:
	void RefreshSigStatus(bool bCanDisable);

	//Dosage Text values used for populating sig
	CString m_strRoute;
	CString m_strDosageQuantity;
	CString m_strUnit;
	CString m_strUnitPl;
	CString m_strFrequency;

	//Used to calculate values
	double m_dFrequency;
	double m_dDosageQuantity;

	//Used to store the calculated values
	double m_dQuantity;
	long m_nDaysSupply;
	CString m_strSig;

	//Used to track when we should calculate values
	bool m_bShouldAutoPopulate;
	bool m_bNeedToUpdate;

	//Flag to autocalculate quantity and days supply
	bool m_bAutocalculate;
};
