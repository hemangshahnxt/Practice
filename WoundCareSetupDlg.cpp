// WoundCareSetupDlg.cpp : implementation file
//
// (r.gonet 08/03/2012) - PLID 51947 - Added

#include "stdafx.h"
#include "Practice.h"
#include "EmrActionDlg.h"
#include "WoundCareSetupDlg.h"
#include "WoundCareCalculator.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CWoundCareSetupDlg dialog

IMPLEMENT_DYNAMIC(CWoundCareSetupDlg, CNxDialog)

CWoundCareSetupDlg::CWoundCareSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CWoundCareSetupDlg::IDD, pParent)
{
}

CWoundCareSetupDlg::~CWoundCareSetupDlg()
{
}

void CWoundCareSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWoundCareSetupDlg)
	DDX_Control(pDX, IDC_WCSETUP_HEADER, m_nxstaticHeaderText);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_WC_DEFAULTS_BTN, m_btnResetToDefaults);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWoundCareSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_WC_DEFAULTS_BTN, &CWoundCareSetupDlg::OnBnClickedWcDefaultsBtn)
END_MESSAGE_MAP()


// CWoundCareSetupDlg message handlers

BOOL CWoundCareSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnResetToDefaults.AutoSet(NXB_MODIFY);
		m_pConditionalActionList = BindNxDataList2Ctrl(IDC_WOUND_CARE_CODING_ACTION_LIST, false);
		// (r.gonet 08/03/2012) - PLID 51947 - Fill the datalist with the conditions and actions
		ReloadConditionalActionsDataList();

		// (r.gonet 08/03/2012) - PLID 51947 - If we detect that Wound Care Coding is not setup, offer
		//  to setup the default values.
		if(!IsConfigured()) {
			// Set up the defaults
			if(IDYES == MsgBox(MB_YESNO|MB_ICONQUESTION, "Practice has detected that Wound Care Coding is not setup yet. " 
				"Would you like Practice to attempt to auto-configure the correct service codes per action at this time?")) 
			{
				if(!AutoConfigure()) {
					MsgBox(MB_OK|MB_ICONERROR, "Auto-configuration has been aborted.");
				} else {
					// (r.gonet 08/03/2012) - PLID 51947 - Action count has probably changed.
					ReloadConditionalActionsDataList();
				}
			}
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// (r.gonet 08/03/2012) - PLID 51947 - Fills the conditional actions datalist
void CWoundCareSetupDlg::ReloadConditionalActionsDataList()
{
	m_pConditionalActionList->Requery();
	m_pConditionalActionList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	// (r.gonet 08/03/2012) - PLID 51947 - Create an array that will be indexed by a WoundCareCondition ID
	//  and contains the counts of how many actions are associated with each condition.
	int aryActionCount[wcccEndPlaceholder] = {0};

	// (r.gonet 08/03/2012) - PLID 51947 - Just get an int array of the condition ids
	CArray<long, long> aryConditionIDs;
	for(int i = 1; i < wcccEndPlaceholder; i++) {
		aryConditionIDs.Add((EWoundCareCodingCondition)i);
	}

	// (r.gonet 08/03/2012) - PLID 51947 - Load up all action counts associated with the conditions.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT SourceID, COUNT(*) AS ActionCount "
		"FROM EMRActionsT "
		"WHERE SourceType = {INT} AND SourceID IN ({INTARRAY}) AND Deleted = 0"
		"GROUP BY SourceID "
		"ORDER BY SourceID ASC ",
		eaoWoundCareCodingCondition, aryConditionIDs);
	while(!prs->eof) {
		EWoundCareCodingCondition wccc = (EWoundCareCodingCondition)VarLong(prs->Fields->Item["SourceID"]->Value);
		long nActionCount = VarLong(prs->Fields->Item["ActionCount"]->Value);
		aryActionCount[wccc] = nActionCount;
		prs->MoveNext();
	}
	prs->Close();

	// (r.gonet 08/03/2012) - PLID 51947 - Go through each condition and write its action count
	//  to its corresponding datalist row.
	for(int i = 1; i < wcccEndPlaceholder; i++) {
		EWoundCareCodingCondition wccc = (EWoundCareCodingCondition)i;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConditionalActionList->FindByColumn(wccacID, _variant_t((long)wccc, VT_I4), m_pConditionalActionList->GetFirstRow(), VARIANT_FALSE);
		if(!pRow) {
			ThrowNxException(FormatString("%s : The datalist row for EWoundCareCodingCondition = %li doesn't exist.", __FUNCTION__, (long)wccc));
		}
		pRow->PutValue(wccacActions, _bstr_t(FormatString("<%li Action(s)>", aryActionCount[wccc])));
	}
}

// (r.gonet 08/03/2012) - PLID 51947 - Guesses if the wound care coding is setup or not.
//  Pretty basic, just see if there 
bool CWoundCareSetupDlg::IsConfigured()
{
	// (r.gonet 08/03/2012) - PLID 51947 - Get an int array of all condition ids
	CArray<long, long> aryConditionIDs;
	for(int i = 1; i < wcccEndPlaceholder; i++) {
		aryConditionIDs.Add((EWoundCareCodingCondition)i);
	}

	// (r.gonet 08/03/2012) - PLID 51947 - Now see if any conditions are setup with actions.
	if(ReturnsRecordsParam(
		"SELECT SourceID "
		"FROM EMRActionsT "
		"WHERE SourceType = {INT} AND SourceID IN ({INTARRAY}) AND Deleted = 0",
		eaoWoundCareCodingCondition, aryConditionIDs))
	{
		// (r.gonet 08/03/2012) - PLID 51947 - We regard partial configuration as configuration
		return true;
	}

	// (r.gonet 08/03/2012) - PLID 51947 - No configuration has been done
	return false;
}

// (r.gonet 08/03/2012) - PLID 51947 - Attempts to setup each wound care coding condition
//  with the CPT code spawning actions appropriate for it.
bool CWoundCareSetupDlg::AutoConfigure()
{
	// (r.gonet 08/03/2012) - PLID 51947 - Create a map from the met condition to the default CPT code we need to spawn.
	//  We don't know if these codes even exist in the Practice database though.
	CMap<EWoundCareCodingCondition, EWoundCareCodingCondition, CString, LPCTSTR> mapConditionToDefaultCPTCode;
	mapConditionToDefaultCPTCode.SetAt(wcccAnySkinDebridement, "97597");
	mapConditionToDefaultCPTCode.SetAt(wcccAnySubQDebridement, "11042");
	mapConditionToDefaultCPTCode.SetAt(wcccAnyMuscleDebridement, "11043");
	mapConditionToDefaultCPTCode.SetAt(wcccAnyBoneDebridement, "11044");
	mapConditionToDefaultCPTCode.SetAt(wccc20CMSkinDebridement, "97598");
	mapConditionToDefaultCPTCode.SetAt(wccc20CMSubQDebridement, "11045");
	mapConditionToDefaultCPTCode.SetAt(wccc20CMMuscleDebridement, "11046");
	mapConditionToDefaultCPTCode.SetAt(wccc20CMBoneDebridement, "11047");

	// (r.gonet 08/03/2012) - PLID 51947 - Get a CString array of those codes we listed above
	CArray<CString, CString> aryCPTCodes;
	// (r.gonet 08/03/2012) - PLID 51947 - Declare a map that will hold the CPT code to its internal id.
	CMap<CString, LPCTSTR, long, long> mapCPTCodes;
	POSITION pos = mapConditionToDefaultCPTCode.GetStartPosition();
	int nIndex = 0;
	while(pos) {
		EWoundCareCodingCondition wcccKey; CString strCPTCode;
		mapConditionToDefaultCPTCode.GetNextAssoc(pos, wcccKey, strCPTCode);
		aryCPTCodes.Add(strCPTCode);
		// (r.gonet 08/03/2012) - PLID 51947 - Initialize the map value to -1 which will mean,
		//  that there is no code in the database associated with that code CString.
		mapCPTCodes.SetAt(aryCPTCodes[nIndex++], -1);
	}

	CMap<CString, LPCTSTR, bool, bool> mapDuplicateCodesFound;
	CArray<long, long> aryDuplicateCodeConditions;
	CString strMultipleCodesWarning;
	// (r.gonet 08/03/2012) - PLID 51947 - Verify that the CPT codes we need to spawn exist in the data,
	// if not, fail.
	// The pair Code,SubCode is unique in the CPTCodeT table. We must choose a subcode, so choose 0.
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT ID, Code "
		"FROM CPTCodeT "
		"WHERE Code IN ({STRINGARRAY}) "
		"ORDER BY SubCode ASC; ",
		aryCPTCodes);
	while(!prs->eof) {
		long nCPTCodeID = VarLong(prs->Fields->Item["ID"]->Value);
		CString strCPTCode = VarString(prs->Fields->Item["Code"]->Value);
		long nExistingCPTCodeID;
		if(mapCPTCodes.Lookup(strCPTCode, nExistingCPTCodeID) && nExistingCPTCodeID != -1) {
			// This code already exists meaning that the system has multiple CPTCodeT records with the same code. We choose the first one and warn the user.
			mapDuplicateCodesFound.SetAt(strCPTCode, true);
		} else {
			mapCPTCodes.SetAt(strCPTCode, nCPTCodeID);
		}
		prs->MoveNext();
	}
	prs->Close();

	CString strMissingCPTCodes = "";
	
	pos = mapCPTCodes.GetStartPosition();
	while(pos) {
		long nCPTCodeID = -1;
		CString strCPTCode;
		mapCPTCodes.GetNextAssoc(pos, strCPTCode, nCPTCodeID);
		if(nCPTCodeID == -1) {
			// This code doesn't exist in the database
			strMissingCPTCodes += strCPTCode + ", ";
		}
	}
	if(!strMissingCPTCodes.IsEmpty()) {
		// Chop off the last comma and space
		strMissingCPTCodes = strMissingCPTCodes.Left(strMissingCPTCodes.GetLength() - 2);
		MsgBox(MB_OK|MB_ICONERROR, "Auto-configure could not find the following service codes in Practice. "
			"Please make sure the below codes exist and then try auto-configuring again: \r\n"
			"\r\n"
			"%s",
			strMissingCPTCodes);
		return false;
	}

	// (r.gonet 08/03/2012) - PLID 51947 - Now create actions for these
	CParamSqlBatch sqlBatch;
	sqlBatch.Add("SET NOCOUNT ON; ");
	// (r.gonet 08/03/2012) - PLID 51947 - Remove all present actions for Wound Care Calculator
	sqlBatch.Add(
		"UPDATE EMRActionsT SET Deleted = 1 \r\n"
		"WHERE SourceType = {INT}; \r\n",
		(long)eaoWoundCareCodingCondition);

	sqlBatch.Add("DECLARE @NewActionID INT; \r\n");
	pos = mapConditionToDefaultCPTCode.GetStartPosition();
	while(pos) {
		EWoundCareCodingCondition wcccKey;
		CString strCPTCode;
		mapConditionToDefaultCPTCode.GetNextAssoc(pos, wcccKey, strCPTCode);
		// (r.gonet 08/03/2012) - PLID 51947 - Now get the CPT code ID that is associated
		long nMappedCPTCodeID;
		if(mapCPTCodes.Lookup(strCPTCode, nMappedCPTCodeID)) {
			// (r.gonet 08/03/2012) - PLID 51947 - Add the aciton.
			sqlBatch.Add(
				"INSERT INTO EMRActionsT (SourceType, SourceID, DestType, DestID, SortOrder, Popup, SpawnAsChild) \r\n"
				"VALUES \r\n"
				"({INT}, {INT}, {INT}, {INT}, {INT}, {BIT}, {BIT}); \r\n",
				(long)eaoWoundCareCodingCondition, (long)wcccKey, (long)eaoCpt, nMappedCPTCodeID, 1, FALSE, FALSE);
			sqlBatch.Add("SET @NewActionID = (SELECT CONVERT(INT, SCOPE_IDENTITY())); \r\n");
			sqlBatch.Add("INSERT INTO EmrActionChargeDataT (ActionID, Prompt, DefaultQuantity, Modifier1Number, "
				"Modifier2Number, Modifier3Number, Modifier4Number) values (@NewActionID, 0, 1.0, NULL, NULL, NULL, NULL);");

			// Also add this to our list of duplicated codes so we can report on the condition that has a code that is duplicated in the system
			if(mapDuplicateCodesFound.PLookup(strCPTCode)) {
				aryDuplicateCodeConditions.Add((long)wcccKey);
			}
		} else {
			// We couldn't find a code to associate by default, skip it and have the user configure it on their own.
			//  We shouldn't even be executing this code since we have found all default codes in the data.
			ASSERT(FALSE);
		}
	}
	sqlBatch.Add("SET NOCOUNT OFF; ");

	if(aryDuplicateCodeConditions.GetSize() > 0) {
		sqlBatch.Add(
			"SELECT Name "
			"FROM WoundCareConditionT "
			"WHERE ConditionID IN ({INTARRAY}); ",
			aryDuplicateCodeConditions);
	}

	_RecordsetPtr prsDuplicates = sqlBatch.CreateRecordset(GetRemoteConnection());
	if(aryDuplicateCodeConditions.GetSize() > 0) {
		CString strDuplicateWarning;
		while(!prsDuplicates->eof) {
			CString strConditionName = AdoFldString(prsDuplicates->Fields, "Name");
			if(!strDuplicateWarning.IsEmpty()) {
				strDuplicateWarning += "\r\n";
			}
			strDuplicateWarning += strConditionName;
			prsDuplicates->MoveNext();
		}
		prsDuplicates->Close();

		if(!strDuplicateWarning.IsEmpty()) {
			MsgBox(MB_OK|MB_ICONWARNING, "Practice completed auto-configuration. However, you should ensure the correctness "
				"of the codes spawned by the following conditions, as they were configured to spawn codes that are in "
				"Practice more than once: \r\n"
				"\r\n"
				+ strDuplicateWarning);
		}
	}

	return true;
}

BEGIN_EVENTSINK_MAP(CWoundCareSetupDlg, CNxDialog)
	ON_EVENT(CWoundCareSetupDlg, IDC_WOUND_CARE_CODING_ACTION_LIST, 19, CWoundCareSetupDlg::LeftClickWoundCareCodingActionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (r.gonet 08/03/2012) - PLID 51947 - When the user clicks on the Actions column, take them to
//  the actions editor for this condition.
void CWoundCareSetupDlg::LeftClickWoundCareCodingActionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			return;
		}

		switch(nCol) {
			case wccacActions:
				{
					// (r.gonet 08/03/2012) - PLID 51947 - Open up the actions editor on this condition.
					CEmrActionDlg dlg(this);
					dlg.m_SourceType = eaoWoundCareCodingCondition;
					dlg.m_nSourceID = VarLong(pRow->GetValue(wccacID));
					dlg.m_strSourceObjectName = "";
					dlg.m_nOriginatingID = -1;
					if(dlg.DoModal() == IDOK) {
						// (r.gonet 08/03/2012) - PLID 51947 - The user might have changed up the actions,
						//  recalculate the count.
						long nActionCount = 0;
						for(int nActionIndex = 0; nActionIndex < dlg.m_arActions.GetSize(); nActionIndex++) {
							EmrAction ea = dlg.m_arActions.GetAt(nActionIndex);
							if(!ea.bDeleted) {
								nActionCount++;
							}
						}
						pRow->PutValue(wccacActions, _bstr_t("<" + AsString(nActionCount) + " Action(s)>"));
					}
				}
				break;
			default:
				break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/03/2012) - PLID 51947 - The user wants to reset the condition actions to
//  what Practice thinks is best. 
void CWoundCareSetupDlg::OnBnClickedWcDefaultsBtn()
{
	try {
		if(IDYES == MsgBox(MB_YESNO|MB_ICONQUESTION, "Resetting to defaults will attempt to reset your configuration to the default service code action per condition. "
			"This will not affect any patient data but it will clear all actions you have assigned to the Wound Care Coding Setup. \r\n\r\n"
			"Do you want to proceed?"))
		{
			if(!AutoConfigure()) {
				MsgBox(MB_OK|MB_ICONERROR, "Auto-configuration has been aborted.");
			} else {
				ReloadConditionalActionsDataList();
			}
		}
	} NxCatchAll(__FUNCTION__);
}
