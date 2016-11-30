
#include "stdafx.h"
#include "AutoPopulatingSig.h"
#include "PrescriptionUtilsNonAPI.h"

// (b.savon 2016-01-19 15:29) - PLID 59879 - Added AutoCalculate throughout

AutoPopulatingSig::AutoPopulatingSig()
{
	m_strRoute = "";
	m_strDosageQuantity = "";
	m_strUnit = "";
	m_strUnitPl = "";
	m_strFrequency = "";
	m_dFrequency = -1.0;
	m_dDosageQuantity = -1.0;
	m_nDaysSupply = -1;
	m_dQuantity = -1.0;
	m_strSig = "";
	m_bShouldAutoPopulate = true;
	m_bNeedToUpdate = true;
	m_bAutocalculate = true;
}

bool AutoPopulatingSig::GetSig(CString &strTextOut)
{
	if(m_bShouldAutoPopulate && m_bNeedToUpdate)
	{
		m_strSig = GenerateSig(m_strDosageQuantity, m_strUnit, m_strUnitPl, m_strRoute, m_strFrequency);
		m_bNeedToUpdate = false;
	}
	strTextOut = m_strSig;
	return m_bShouldAutoPopulate;
}

bool AutoPopulatingSig::GetQuantity(double *pValueOut)
{
	*pValueOut = m_dQuantity;

	return m_dFrequency >= 0.0 &&
		m_dDosageQuantity >= 0.0 &&
		m_nDaysSupply >= 0 &&
		m_dQuantity >= 0.0;
}

bool AutoPopulatingSig::GetDaysSupply(long *pValueOut)
{
	*pValueOut = m_nDaysSupply;

	return m_dFrequency >= 0.0 &&
		m_dDosageQuantity >= 0.0 &&
		m_nDaysSupply >= 0 &&
		m_dQuantity >= 0.0;
}

void AutoPopulatingSig::SetRoute(CString &strText)
{
	m_strRoute = strText;
	m_bNeedToUpdate = true;

	RefreshSigStatus(false);
}

void AutoPopulatingSig::SetDosageQuantity(CString &strText, double dValue)
{
	m_strDosageQuantity = strText;
	m_dDosageQuantity = dValue;
	m_bNeedToUpdate = true;

	if(m_bAutocalculate && 
		m_dFrequency >= 0.0 &&
		m_dDosageQuantity >= 0.0 &&
		m_nDaysSupply >= 0)
	{
		m_dQuantity = ceil(m_dDosageQuantity * m_dFrequency * (double)m_nDaysSupply);
	}

	RefreshSigStatus(false);
}

void AutoPopulatingSig::SetUnit(CString &strText, CString &strTextPlural)
{
	m_strUnit = strText;
	m_strUnitPl = strTextPlural;
	m_bNeedToUpdate = true;

	RefreshSigStatus(false);
}

void AutoPopulatingSig::SetFrequency(CString &strText, double dValue)
{
	m_strFrequency = strText;
	m_dFrequency = dValue;
	m_bNeedToUpdate = true;

	if(m_bAutocalculate && 
		m_dFrequency >= 0.0 &&
		m_dDosageQuantity >= 0.0 &&
		m_nDaysSupply >= 0)
	{
		m_dQuantity = ceil(m_dDosageQuantity * m_dFrequency * (double)m_nDaysSupply);
	}

	RefreshSigStatus(false);
}

void AutoPopulatingSig::SetQuantity(CString &strText)
{
	char *pEnd;
	m_dQuantity = strtod(strText.GetBuffer(strText.GetLength()), &pEnd);

	if(*pEnd != '\0' || strText.IsEmpty())
	{
		m_dQuantity = -1.0;
	}

	strText.ReleaseBuffer();

	if(m_bAutocalculate && 
		m_dFrequency >= 0.0 &&
		m_dDosageQuantity >= 0.0 &&
		m_dQuantity >= 0.0)
	{
		m_nDaysSupply = (long)floor(m_dQuantity / (m_dDosageQuantity * m_dFrequency));
	}
}

void AutoPopulatingSig::SetDaysSupply(CString &strText)
{
	char *pEnd;
	m_nDaysSupply = strtol(strText.GetBuffer(strText.GetLength()), &pEnd, 10/*Base 10*/);

	if(*pEnd != '\0' || strText.IsEmpty())
	{
		m_nDaysSupply = -1;
	}

	strText.ReleaseBuffer();
	
	if(m_bAutocalculate && 
		m_dFrequency >= 0.0 &&
		m_dDosageQuantity >= 0.0 &&
		m_nDaysSupply >= 0)
	{
		m_dQuantity = ceil(m_dDosageQuantity * m_dFrequency * (double)m_nDaysSupply);
	}
}

void AutoPopulatingSig::SetSig(CString &strText)
{
	m_bNeedToUpdate = false;
	m_strSig = strText;
	RefreshSigStatus(true);
}

void AutoPopulatingSig::RefreshSigStatus(bool bCanDisable)
{
	if(m_strSig.IsEmpty()) {
		//They cleared out the sig
		//We will (re)start autopopulating
		m_bShouldAutoPopulate = true;
	} else if(m_strSig.Compare(GenerateSig(m_strDosageQuantity, m_strUnit, m_strUnitPl, m_strRoute, m_strFrequency)) != 0) {
		if(bCanDisable)
		{
			//This new sig does not equal our calculated sig.
			//We should stop generating the sig
			m_bShouldAutoPopulate = false;
		}
	} else {
		//The sig and calculated sig match
		m_bShouldAutoPopulate = true;
	}
}

// (b.savon 2016-01-19 15:29) - PLID 59879 - Added
void AutoPopulatingSig::SetAutoCalculate(const NexTech_Accessor::_NexERxMedicationPackageInformationPtr &pPackage)
{
	CString strDrugForm = AsString(pPackage->GetDrugForm());
	if (strDrugForm.CompareNoCase("each") == 0) {
		m_bAutocalculate = true;
	}
	else {
		m_bAutocalculate = false;
	}
}

// (b.savon 2016-01-19 15:29) - PLID 59879 - Added
bool AutoPopulatingSig::GetAutoCalculate()
{
	return m_bAutocalculate;
}