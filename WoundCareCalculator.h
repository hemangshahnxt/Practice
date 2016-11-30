//WoundCareCalculator.h

#ifndef WOUND_CARE_CALCULATOR_H
#define WOUND_CARE_CALCULATOR_H

#pragma once

#include "EMNDetailStructures.h"

// (r.gonet 08/03/2012) - PLID 51027 - The list of debridement (injured tissue removal) levels
enum EWoundCareDebridementLevels
{
	wcdlSkin = 0,
	wcdlSubQ,
	wcdlMuscle,
	wcdlBone,
	wcdlEndPlaceholder,
};

// (r.gonet 08/03/2012) - PLID 51947 - The list of available conditions to meet.
enum EWoundCareCodingCondition
{
	wcccInvalid = -1,
	wcccAnySkinDebridement = 1,
	wcccAnySubQDebridement = 2,
	wcccAnyMuscleDebridement = 3,
	wcccAnyBoneDebridement = 4,
	wccc20CMSkinDebridement = 5,
	wccc20CMSubQDebridement = 6,
	wccc20CMMuscleDebridement = 7,
	wccc20CMBoneDebridement = 8,
	wcccEndPlaceholder, // (r.gonet 08/03/2012) - PLID 51947 - So we can iterate this enum.
};

// (r.gonet 08/03/2012) - PLID 51027 - This enumeration defines the columns that the Wound Care CPT Coding Calculator will need
//  to reference in order to determine the correct CPT codes to add or remove from the EMN.
// Do not alter, these are saved to the database.
enum EWoundCareDataType
{
	wcdtNone = 0,
	wcdtWoundSurfaceArea = 1,
	wcdtSkinDebridement = 2,
	wcdtSubQDebridement = 3,
	wcdtMuscleDebridement = 4,
	wcdtBoneDebridement = 5,
	wcdtNoDebridement = 6,
	// Insert new data types here
	wcdtEndPlaceholder, // Don't touch this one
};

// (r.gonet 08/03/2012) - PLID 51027 - A tiny class containing what condition was met
//  and how many times.
class CWoundCareMetCondition
{
public:
	EWoundCareCodingCondition m_wcccCondition;
	long m_nTimesMet;
public:
	CWoundCareMetCondition();
	CWoundCareMetCondition(EWoundCareCodingCondition wcccCondition, long nTimesMet);
};

class CWoundCareCalculator
{
private:
	// (r.gonet 08/03/2012) - PLID 51027 - The table detail being calculated with.
	CEMNDetail *m_pDetail;
public:
	CWoundCareCalculator(CEMNDetail *pDetail);
	~CWoundCareCalculator();

	// (r.gonet 08/03/2012) - PLID 51027 - Staying with the calculator theme, Calculate analyzes 
	//  m_pDetail's data and determines what conditions are met, then performs the conditional actions.
	void Calculate();
	// (r.gonet 08/03/2012) - PLID 51027 - Reset unspawns all things spawned from the wound care
	//  conditions.
	void Reset();

	// (r.gonet 08/03/2012) - PLID 51027 - Gets a map of wound care data types to the associated table columns.
	void FillColumnMap(CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> &mapWoundCareDataTypeToColumn);
	// (r.gonet 08/03/2012) - PLID 51027 - Ensures that the associated columns are as we need them.
	// Returns true if they are and false otherwise
	bool ValidateColumnMap(CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> &mapWoundCareDataTypeToColumn);
	// (r.gonet 08/03/2012) - PLID 51027 - Sums up the debridement amounts and then places the amounts in aryDebridementTotals,
	//  which is indexed by EWoundCareDebridementLevels
	// Returns true if the summing was successful and false if it was not. 
	bool CalculateDebridementSums(CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> &mapWoundCareDataTypeToColumn,
									CArray<double, double> &arydDebridementTotals);
	// (r.gonet 08/03/2012) - PLID 51027 - Analyzes the sums generated in previous steps and determines which conditions are met, then performs the actions.
	void CheckConditions(CArray<CWoundCareMetCondition, CWoundCareMetCondition> &aryMetConditions, CArray<double, double> &arydDebridementTotals);
};

// (r.gonet 08/03/2012) - PLID 51027 - Returns a description of the wound care data type.
CString GetWoundCareDataTypeDescription(EWoundCareDataType ewcc);

#endif