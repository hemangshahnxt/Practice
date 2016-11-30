// (r.gonet 08/03/2012) - PLID 51027 - Added

#include "stdafx.h"
#include "WoundCareCalculator.h"
#include "EMNDetail.h"
#include "EMR.h"
#include "EMNUnspawner.h"
#include "WoundCareSetupDlg.h"

using namespace ADODB;

// (r.gonet 08/03/2012) - PLID 51027 - Creates a new condition that has been met 0 times and is invalid....
CWoundCareMetCondition::CWoundCareMetCondition()
{
	m_wcccCondition = wcccInvalid;
	m_nTimesMet = 0;
}

// (r.gonet 08/03/2012) - PLID 51027 - Creates a new condition that has been met a certain number of times.
CWoundCareMetCondition::CWoundCareMetCondition(EWoundCareCodingCondition wcccCondition, long nTimesMet)
{
	m_wcccCondition = wcccCondition;
	m_nTimesMet = nTimesMet;
}

// (r.gonet 08/03/2012) - PLID 51027 - Creates a new calculator on a table detail. pDetail must be a table
CWoundCareCalculator::CWoundCareCalculator(CEMNDetail *pDetail)
{
	if(!pDetail) {
		ThrowNxException("%s : Invalid argument NULL passed in pDetail.", __FUNCTION__);
	}
	if(!pDetail->GetParentEMR()) {
		ThrowNxException("%s : pDetail doesn't belong to an EMR.", __FUNCTION__);
	}
	if(pDetail->m_EMRInfoType != eitTable) {
		ThrowNxException("%s : pDetail must be a table.", __FUNCTION__);
	}

	m_pDetail = pDetail;
}

CWoundCareCalculator::~CWoundCareCalculator()
{
}

// (r.gonet 08/03/2012) - PLID 51027 - Staying with the calculator theme, Calculate analyzes 
//  m_pDetail's data and determines what conditions are met, then performs the conditional actions.
void CWoundCareCalculator::Calculate()
{
	// (r.gonet 08/03/2012) - PLID 51027 - Offer to configure Wound Care Coding if it isn't already setup.
	if(!CWoundCareSetupDlg::IsConfigured()) {
		if(IDYES == MsgBox(MB_YESNO|MB_ICONQUESTION, "Practice has detected that Wound Care Coding is not setup yet. " 
				"Would you like Practice to attempt to auto-configure the correct CPT codes per action at this time?")) 
		{
			if(!CWoundCareSetupDlg::AutoConfigure()) {
				MsgBox(MB_OK|MB_ICONERROR, "Auto-configuration has been aborted. Please configure Wound Care Coding before running it again. "
					"You can configure it by going to Administrator => Activities Menu => EMR => Configure Wound Care Coding.");
				return;
			}
		} else {
			MsgBox(MB_OK|MB_ICONERROR, "Please configure Wound Care Coding before running it again. "
					"You can configure it by going to Administrator => Activities Menu => EMR => Configure Wound Care Coding.");
			return;
		}
	}

	// (r.gonet 08/03/2012) - PLID 51027 - Get a map of wound care data types to the columns they are associated with.
	CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> mapWoundCareDataTypeToColumn;
	FillColumnMap(mapWoundCareDataTypeToColumn);
	if(!ValidateColumnMap(mapWoundCareDataTypeToColumn)) {
		MsgBox(MB_OK|MB_ICONERROR, "Wound Care Coding was unsuccessful.");
		return;
	}

	// (r.gonet 08/03/2012) - PLID 51027 - We have the columns, so do the calculations
	
	// (r.gonet 08/03/2012) - PLID 51027 - Total the sum of the debridement surface areas
	CArray<double, double> arydDebridementTotals;
	if(!CalculateDebridementSums(mapWoundCareDataTypeToColumn, arydDebridementTotals)) {
		MsgBox(MB_OK|MB_ICONERROR, "Wound Care Coding was unsuccessful.");
		return;
	}	

	// (r.gonet 08/03/2012) - PLID 51027 - Check which, if any, of the conditions for coding have been met.
	CArray<CWoundCareMetCondition,CWoundCareMetCondition> aryMetConditions;
	CheckConditions(aryMetConditions, arydDebridementTotals);

	// (r.gonet 08/03/2012) - PLID 51027 - Now, they may have done the auto coding before, so make sure to unspawn the actions we have spawned previously
	Reset();
	if(aryMetConditions.GetSize() > 0) {
		m_pDetail->GetParentEMR()->ProcessEmrWoundCalculatorActions(aryMetConditions, m_pDetail->GetParentEMN(), m_pDetail, false);
	}

	MsgBox(MB_OK|MB_ICONINFORMATION, "Wound Care Automatic Coding finished successfully.");
}

// (r.gonet 08/03/2012) - PLID 51027 - Reset unspawns all things spawned from the wound care
//  conditions.
void CWoundCareCalculator::Reset()
{
	CEMNUnspawner eu(m_pDetail->GetParentEMN());
	eu.RemoveActionsByWoundCareCalculator(m_pDetail);
}

// (r.gonet 08/03/2012) - PLID 51027 - Returns a description of a wound care data type
CString GetWoundCareDataTypeDescription(EWoundCareDataType ewcc)
{
	switch(ewcc) {
	case wcdtNone:
		return "<None>";
	case wcdtWoundSurfaceArea:
		return "Wound Surface Area";
	case wcdtSkinDebridement:
		return "Skin Debridement";
	case wcdtSubQDebridement:
		return "Subcutaneous Debridement";
	case wcdtMuscleDebridement:
		return "Muscle Debridement";
	case wcdtBoneDebridement:
		return "Bone Debridement";
	case wcdtNoDebridement:
		return "No Debridement";
	default:
		ASSERT(FALSE);
		return "<None>";
	}
}

// (r.gonet 08/03/2012) - PLID 51027 - Create a map of the calculation types to the actual table columns
void CWoundCareCalculator::FillColumnMap(CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> &mapWoundCareDataTypeToColumn)
{
	// (r.gonet 08/03/2012) - PLID 51027 - Fill it with the calculation related columns
	for(int i = 1; i < (int)wcdtEndPlaceholder; i++)
	{
		EWoundCareDataType ewcc = (EWoundCareDataType)i;
		mapWoundCareDataTypeToColumn.SetAt(ewcc, NULL);
	}
	// (r.gonet 08/03/2012) - PLID 51027 - Get the columns associated with the calculator types
	for(int c = 0; c < m_pDetail->GetColumnCount(); c++) {
		TableColumn *pCol = m_pDetail->GetColumnPtr(c);
		if(pCol != NULL) {
			if(pCol->m_ewccWoundCareDataType != wcdtNone) {
				mapWoundCareDataTypeToColumn.SetAt(pCol->m_ewccWoundCareDataType, pCol);
			} else {
				// (r.gonet 08/03/2012) - PLID 51027 - Columns that are marked wcdtNone are simply regular columns that we don't use in our calculation
			}
		} else {
			// (r.gonet 08/03/2012) - PLID 51027 - Unknown if this can happen, ignore if it can
		}
	}
}

// (r.gonet 08/03/2012) - PLID 51027 - Ensures that the associated columns are as we need them.
// Returns true if they are and false otherwise
bool CWoundCareCalculator::ValidateColumnMap(CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> &mapWoundCareDataTypeToColumn)
{
	// (r.gonet 08/03/2012) - PLID 51027 - Ensure that we have all the columns we need and they are the correct types
	CString strMissingColumns = "", strTypeErrors = "";
	POSITION pos = mapWoundCareDataTypeToColumn.GetStartPosition();
	while(pos) {
		EWoundCareDataType ewcc;
		TableColumn *pCol;
		mapWoundCareDataTypeToColumn.GetNextAssoc(pos, ewcc, pCol);
	
		if(pCol == NULL) {
			// (r.gonet 08/03/2012) - PLID 51027 - This code doesn't exist in the database
			if(!strMissingColumns.IsEmpty()) {
				strMissingColumns += "\r\n";
			}
			strMissingColumns += GetWoundCareDataTypeDescription(ewcc);
		} else {
			// (r.gonet 08/03/2012) - PLID 51027 - There is a column associated with this data type, check the type
			CString strTypeError = "";
			switch(ewcc) {
				case wcdtWoundSurfaceArea:
					if(pCol->nType != LIST_TYPE_TEXT) {
						strTypeError = FormatString("The '%s' %s must be a Text %s", GetWoundCareDataTypeDescription(ewcc), !m_pDetail->m_bTableRowsAsFields ? "column" : "row", !m_pDetail->m_bTableRowsAsFields ? "column" : "row");
					}
					break;
				case wcdtSkinDebridement:
				case wcdtSubQDebridement:
				case wcdtMuscleDebridement:
				case wcdtBoneDebridement:
				case wcdtNoDebridement:
					if(pCol->nType != LIST_TYPE_CHECKBOX) {
						strTypeError = FormatString("The '%s' %s must be a Checkbox %s", GetWoundCareDataTypeDescription(ewcc), !m_pDetail->m_bTableRowsAsFields ? "column" : "row", !m_pDetail->m_bTableRowsAsFields ? "column" : "row");
					}
					break;
				default:
					// Unhandled wound care data type
					ASSERT(FALSE);
					break;
			}
			if(!strTypeErrors.IsEmpty()) {
				strTypeErrors += "\r\n";
			}
			strTypeErrors += strTypeError;
		}
	}
	if(!strMissingColumns.IsEmpty()) {
		// (r.gonet 08/03/2012) - PLID 51027 - There were errors encountered. Alert the user and fail.
		MsgBox(MB_OK|MB_ICONERROR,
			"Although the table '%s' is marked as being used in Wound Care Coding Calculation, it is missing "
			"the required %s(s) listed below. Please edit the table and assign these Wound Care Data Types:\r\n"
			"\r\n"
			"%s",
			m_pDetail->GetMergeFieldName(FALSE),
			!m_pDetail->m_bTableRowsAsFields ? "column" : "row",
			strMissingColumns);
		return false;
	}

	if(!strTypeErrors.IsEmpty()) {
		// (r.gonet 08/03/2012) - PLID 51027 - This code should only be executed if the table column type of at least one Wound Care Coding associated column
		//  is not what is needed. But we do validate that in the EMRItemEntry, so I'm not sure how this happened.
		MsgBox(MB_OK|MB_ICONERROR,
			"Although the table '%s' is marked as being used in Wound Care Coding Calculation, the %s types "
			"are not what are required. Please edit the table and correct the following:\r\n"
			"\r\n"
			"%s",
			m_pDetail->GetMergeFieldName(FALSE),
			!m_pDetail->m_bTableRowsAsFields ? "column" : "row",
			strTypeErrors);
		return false;
	}

	return true;
}

// (r.gonet 08/03/2012) - PLID 51027 - Sums up the debridement amounts and then places the amounts in aryDebridementTotals,
//  which is indexed by EWoundCareDebridementLevels
// Returns true if the summing was successful and false if it was not. 
bool CWoundCareCalculator::CalculateDebridementSums(CMap<EWoundCareDataType, EWoundCareDataType, TableColumn*, TableColumn*> &mapWoundCareDataTypeToColumn,
													CArray<double, double> &arydDebridementTotals)
{
	arydDebridementTotals.RemoveAll();
	arydDebridementTotals.SetSize(wcdlEndPlaceholder);
	// (r.gonet 08/03/2012) - PLID 51027 - For each row, add its area to the corresponding debridement sum.
	for(int r = 0; r < m_pDetail->GetRowCount(); r++) {
		TableRow *pRow = m_pDetail->GetRowPtr(r);
		if(pRow) {
			// (r.gonet 08/03/2012) - PLID 51027 - It is possible for the following values to be null if they have never been filled with any value.
			TableElement *pWoundSurfaceAreaElement = m_pDetail->GetTableElementByRowColPtr(pRow, mapWoundCareDataTypeToColumn.PLookup(wcdtWoundSurfaceArea)->value);			
			TableElement *pSkinDebridementElement = m_pDetail->GetTableElementByRowColPtr(pRow, mapWoundCareDataTypeToColumn.PLookup(wcdtSkinDebridement)->value);
			TableElement *pSubQDebridementElement = m_pDetail->GetTableElementByRowColPtr(pRow, mapWoundCareDataTypeToColumn.PLookup(wcdtSubQDebridement)->value);
			TableElement *pMuscleDebridementElement = m_pDetail->GetTableElementByRowColPtr(pRow, mapWoundCareDataTypeToColumn.PLookup(wcdtMuscleDebridement)->value);
			TableElement *pBoneDebridementElement = m_pDetail->GetTableElementByRowColPtr(pRow, mapWoundCareDataTypeToColumn.PLookup(wcdtBoneDebridement)->value);
			TableElement *pNoDebridementElement = m_pDetail->GetTableElementByRowColPtr(pRow, mapWoundCareDataTypeToColumn.PLookup(wcdtNoDebridement)->value);

			// (r.gonet 08/03/2012) - PLID 51027 - Wound Surface Area must be blank or a double. If its blank, skip this row. It wouldn't have any effect anyway.
			double dWoundCareSurfaceArea = 0.0;
			CString strWoundCareSurfaceAreaValue = pWoundSurfaceAreaElement ? pWoundSurfaceAreaElement->GetValueAsString() : "";
			if(strWoundCareSurfaceAreaValue.IsEmpty()) {
				continue;
			} else if(!IsValidDouble(strWoundCareSurfaceAreaValue)){
				MsgBox(MB_OK|MB_ICONERROR, "The '%s' %s contains a non-number. Auto-coding has been aborted. Please correct the value to be a number.", 
					mapWoundCareDataTypeToColumn.PLookup(wcdtWoundSurfaceArea)->value->strName, !m_pDetail->m_bTableRowsAsFields ? "column" : "row");
				return false;
			} else {
				dWoundCareSurfaceArea = strtod(strWoundCareSurfaceAreaValue, NULL);
				if(dWoundCareSurfaceArea < 0) {
					MsgBox(MB_OK|MB_ICONERROR, "The '%s' %s contains a negative number. Auto-coding has been aborted. Please correct the value to be non-negative.", 
						mapWoundCareDataTypeToColumn.PLookup(wcdtWoundSurfaceArea)->value->strName, !m_pDetail->m_bTableRowsAsFields ? "column" : "row");
					return false;
				} else {
					// (r.gonet 08/03/2012) - PLID 51027 - Number is positive or zero and thus we can use it.
				}
			}

			// (r.gonet 08/03/2012) - PLID 51027 - We use AsBool rather than VarBool because the variant returned is of type VT_I2.
			BOOL bSkinDebridementPerformed = pSkinDebridementElement ? AsBool(pSkinDebridementElement->GetValueAsVariant()) : FALSE;
			BOOL bSubQDebridementPerformed = pSubQDebridementElement ? AsBool(pSubQDebridementElement->GetValueAsVariant()) : FALSE;
			BOOL bMuscleDebridementPerformed = pMuscleDebridementElement ? AsBool(pMuscleDebridementElement->GetValueAsVariant()) : FALSE;
			BOOL bBoneDebridementPerformed = pBoneDebridementElement ? AsBool(pBoneDebridementElement->GetValueAsVariant()) : FALSE;
			BOOL bNoDebridementPerformed = pNoDebridementElement ? AsBool(pNoDebridementElement->GetValueAsVariant()) : FALSE;

			// (r.gonet 08/03/2012) - PLID 51027 - We can only have one of these checked per row since it is the maximum level of debridement
			int nNumChecked = 
				(bSkinDebridementPerformed ? 1 : 0) + 
				(bSubQDebridementPerformed ? 1 : 0) +
				(bMuscleDebridementPerformed ? 1 : 0) + 
				(bBoneDebridementPerformed ? 1 : 0) +
				(bNoDebridementPerformed ? 1 : 0);
			if(nNumChecked != 1) {
				MsgBox(MB_OK|MB_ICONERROR,
					"Please check one, and only one, debridement level per %s. This should be the deepest level of debridement of wound.", !m_pDetail->m_bTableRowsAsFields ? "row" : "column");
				return false;
			} else {
				// (r.gonet 08/03/2012) - PLID 51027 - We have now validated everything. Hurray.
			}

			arydDebridementTotals[(int)wcdlSkin] += bSkinDebridementPerformed ? dWoundCareSurfaceArea : 0.0;
			arydDebridementTotals[(int)wcdlSubQ] += bSubQDebridementPerformed ? dWoundCareSurfaceArea : 0.0;
			arydDebridementTotals[(int)wcdlMuscle] += bMuscleDebridementPerformed ? dWoundCareSurfaceArea : 0.0;
			arydDebridementTotals[(int)wcdlBone] += bBoneDebridementPerformed ? dWoundCareSurfaceArea : 0.0;
		} else {
			// (r.gonet 08/03/2012) - PLID 51027 - Huh? The table row is null.... assert and skip it.
			ASSERT(FALSE);
			continue;
		}
	}

	return true;
}

// (r.gonet 08/03/2012) - PLID 51027 - Analyzes the sums generated in previous steps and determines which conditions are met, then performs the actions.
void CWoundCareCalculator::CheckConditions(CArray<CWoundCareMetCondition, CWoundCareMetCondition> &aryMetConditions, CArray<double, double> &arydDebridementTotals)
{
	if(arydDebridementTotals[wcdlSkin] > 0) {
		// Condition Met: Any skin debridement performed
		aryMetConditions.Add(CWoundCareMetCondition(wcccAnySkinDebridement, 1));
		
		if(arydDebridementTotals[wcdlSkin] > 20) {
			// For every 20 beyond the first 20, meet this condition. If there are any fractional parts after deviding by 20, meet the condition once more.
			long nQuantity = (long)floor(arydDebridementTotals[wcdlSkin] / 20) - 1;
			nQuantity += fmod(arydDebridementTotals[wcdlSkin], 20.0) > 0 ? 1 : 0;
			if(nQuantity > 0) {
				// Condition Met: 20 sq cm of skin debridement performed, nQuantity times.
				aryMetConditions.Add(CWoundCareMetCondition(wccc20CMSkinDebridement, nQuantity));
			}
		}
	}
	if(arydDebridementTotals[wcdlSubQ] > 0) {
		// Condition Met: Any subcutaneous tissue debridement performed
		aryMetConditions.Add(CWoundCareMetCondition(wcccAnySubQDebridement, 1));

		if(arydDebridementTotals[wcdlSubQ] > 20) {
			long nQuantity = (long)floor(arydDebridementTotals[wcdlSubQ] / 20) - 1;
			nQuantity += fmod(arydDebridementTotals[wcdlSubQ], 20.0) > 0 ? 1 : 0;
			if(nQuantity > 0) {
				// Condition Met: 20 sq cm of skin debridement performed, nQuantity times
				aryMetConditions.Add(CWoundCareMetCondition(wccc20CMSubQDebridement, nQuantity));
			}
		}
	}
	if(arydDebridementTotals[wcdlMuscle] > 0) {
		// Condition Met: Any muscle debridement performed
		aryMetConditions.Add(CWoundCareMetCondition(wcccAnyMuscleDebridement, 1));

		if(arydDebridementTotals[wcdlMuscle] > 20) {
			long nQuantity = (long)floor(arydDebridementTotals[wcdlMuscle] / 20) - 1;
			nQuantity += fmod(arydDebridementTotals[wcdlMuscle], 20.0) > 0 ? 1 : 0;
			if(nQuantity > 0) {
				// Condition Met: 20 sq cm of muscle debridement performed, nQuantity times
				aryMetConditions.Add(CWoundCareMetCondition(wccc20CMMuscleDebridement, nQuantity));
			}
		}
	}
	if(arydDebridementTotals[wcdlBone] > 0) {
		// Condition Met: Any bone debridement performed
		aryMetConditions.Add(CWoundCareMetCondition(wcccAnyBoneDebridement, 1));

		if(arydDebridementTotals[wcdlBone] > 20) {
			long nQuantity = (long)floor(arydDebridementTotals[wcdlBone] / 20) - 1;
			nQuantity += fmod(arydDebridementTotals[wcdlBone], 20.0) > 0 ? 1 : 0;
			if(nQuantity > 0) {
				// Condition Met: 20 sq cm of bone debridement performed, nQuantity times
				aryMetConditions.Add(CWoundCareMetCondition(wccc20CMBoneDebridement, nQuantity));
			}
		}
	}
}