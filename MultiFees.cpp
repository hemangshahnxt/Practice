// MultiFees.cpp : implementation file
//

#include "stdafx.h"
#include "MultiFees.h"
#include "PracProps.h"
#include "MsgBox.h"
#include "NxStandard.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "UpdateMultiFeeScheduleDlg.h"
#include "UpdateFeesByRVUDlg.h"
#include "MultiFeesImportDlg.h"
#include "DontShowDlg.h"
#include "InternationalUtils.h"
#include "NxAPIManager.h"
#include "NxAPIUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CMultiFees dialog

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_UPDATE_BY_PERCENT	42753
#define ID_UPDATE_BY_RVU		42754
#define ID_UPDATE_FROM_FILE		42755

// (j.jones 2013-04-11 15:15) - PLID 12136 - turned the CPT column defines into an enum
enum ServiceListColumns {
	slcID = 0,
	slcIsAnesth,
	slcIsFacFee,
	slcCode,
	slcSubCode,
	slcName,
	slcPrice,
	slcNewFee,
	slcAllowable,
};

// (j.jones 2013-04-11 15:15) - PLID 12136 - added products
enum ProductListColumns {
	plcID = 0,
	plcInsCode,
	plcName,
	plcCost,
	plcPrice,
	plcNewFee,
	plcAllowable,
};


CMultiFees::CMultiFees(CWnd* pParent)
	: CNxDialog(CMultiFees::IDD, pParent)
	,
	m_feegroupsChecker(NetUtils::MultiFeeGroupsT, false),
	m_companyChecker(NetUtils::InsuranceCoT), // All insurance companies
	m_providerChecker(NetUtils::Providers), // Use Gen1Providers because it has the same filter
	m_CPTChecker(NetUtils::CPTCodeT),
	m_feeitemsChecker(NetUtils::MultiFeeItemsT), // CH 3/5: not sure if this should be false
	m_locationChecker(NetUtils::LocationsT),
	m_ProductChecker(NetUtils::Products)	// (j.jones 2013-04-11 15:10) - PLID 12136 - added products
{
	//{{AFX_DATA_INIT(CMultiFees)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Billing_Setup/Insurance_Billing_Setup/Create_a_Multi_Fee_Group.htm";

	m_bHasInventory = true;
}


void CMultiFees::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiFees)
	// (j.jones 2007-05-03 11:19) - PLID 25840 - added option to include patient resp. in allowable
	DDX_Control(pDX, IDC_INCLUDE_PATRESP_ALLOWABLE, m_checkIncludePatRespAllowable);
	DDX_Control(pDX, IDC_INACTIVE_FEE_SCHEDULE, m_checkInactive);
	DDX_Control(pDX, IDC_INCLUDE_COPAY_ALLOWABLE, m_checkIncludeCoPayAllowable);
	DDX_Control(pDX, IDC_REMOVE_LOCATION, m_btnRemLoc);
	DDX_Control(pDX, IDC_ADD_LOCATION, m_btnAddLoc);
	DDX_Control(pDX, IDC_WARN_ALLOWABLE, m_btnWarnAllowable);
	DDX_Control(pDX, IDC_UPDATE_FEES, m_btnUpdateFees);
	DDX_Control(pDX, IDC_DELETE, m_deleteButton);
	DDX_Control(pDX, IDC_ADD, m_addButton);
	DDX_Control(pDX, IDC_REMOVE_PROVIDER, m_remProvBtn);
	DDX_Control(pDX, IDC_REMOVE_INS, m_remInsBtn);
	DDX_Control(pDX, IDC_ADD_PROVIDER, m_addProvBtn);
	DDX_Control(pDX, IDC_ADD_INS, m_addInsBtn);
	DDX_Control(pDX, IDC_WARN_MULTIFEE, m_btnWarnMultiFee);
	DDX_Control(pDX, IDC_BILLSTANDARD, m_btnStandard);
	DDX_Control(pDX, IDC_ACCEPT, m_btnAccept);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_ANESTH_FEE_LABEL, m_nxstaticAnesthFeeLabel);
	DDX_Control(pDX, IDC_LOCATION_UNSELECTED_LABEL, m_nxstaticLocationUnselectedLabel);
	DDX_Control(pDX, IDC_LOCATION_SELECTED_LABEL, m_nxstaticLocationSelectedLabel);
	DDX_Control(pDX, IDC_RADIO_FEE_SCHED_LOCATION, m_radioLocation);
	DDX_Control(pDX, IDC_RADIO_FEE_SCHED_POS, m_radioPOS);
	DDX_Control(pDX, IDC_RADIO_FEE_SCHED_CPT, m_radioServiceCodes);
	DDX_Control(pDX, IDC_RADIO_FEE_SCHED_INVENTORY, m_radioInventoryItems);
	// (d.lange 2015-09-28 10:37) - PLID 67118
	DDX_Control(pDX, IDC_LEFT_FEE_BTN, m_previousFeeSchedBtn);
	DDX_Control(pDX, IDC_RIGHT_FEE_BTN, m_nextFeeSchedBtn);
	// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
	DDX_Control(pDX, IDC_RENAME, m_renameButton);
	// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
	DDX_Control(pDX, IDC_MULTIFEE_EFFECTIVEDATE_FROM_DATE, m_dtEffectiveDateFromDate);
	DDX_Control(pDX, IDC_MULTIFEE_EFFECTIVEDATE_TO_DATE, m_dtEffectiveDateToDate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiFees, CNxDialog)
	//{{AFX_MSG_MAP(CMultiFees)
	ON_BN_CLICKED(IDC_ADD_INS, OnAddIns)
	ON_BN_CLICKED(IDC_REMOVE_INS, OnRemoveIns)
	ON_BN_CLICKED(IDC_ADD_PROVIDER, OnAddProvider)
	ON_BN_CLICKED(IDC_REMOVE_PROVIDER, OnRemoveProvider)
	ON_WM_PAINT()
	ON_EN_KILLFOCUS(IDC_NOTES, OnKillfocusNotes)
	ON_BN_CLICKED(IDC_WARN_MULTIFEE, OnWarnMultiFee)
	ON_BN_CLICKED(IDC_ACCEPT, OnAccept)
	ON_BN_CLICKED(IDC_BILLSTANDARD, OnBillstandard)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_UPDATE_FEES, OnUpdateFees)
	ON_BN_CLICKED(IDC_WARN_ALLOWABLE, OnWarnAllowable)
	ON_BN_CLICKED(IDC_REMOVE_LOCATION, OnRemoveLocation)
	ON_BN_CLICKED(IDC_ADD_LOCATION, OnAddLocation)
	ON_BN_CLICKED(IDC_INCLUDE_COPAY_ALLOWABLE, OnIncludeCopayAllowable)
	ON_BN_CLICKED(IDC_INACTIVE_FEE_SCHEDULE, OnInactiveFeeSchedule)
	ON_COMMAND(ID_UPDATE_BY_PERCENT, OnUpdatePricesByPercentage)
	ON_COMMAND(ID_UPDATE_BY_RVU, OnUpdatePricesByRVU)
	ON_COMMAND(ID_UPDATE_FROM_FILE, OnUpdatePricesFromFile)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	// (j.jones 2007-05-03 11:19) - PLID 25840 - added option to include patient resp. in allowable
	ON_BN_CLICKED(IDC_INCLUDE_PATRESP_ALLOWABLE, OnIncludePatrespAllowable)
	// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
	ON_BN_CLICKED(IDC_RADIO_FEE_SCHED_LOCATION, OnRadioFeeSchedLocation)
	ON_BN_CLICKED(IDC_RADIO_FEE_SCHED_POS, OnRadioFeeSchedPos)
	ON_BN_CLICKED(IDC_RADIO_FEE_SCHED_CPT, OnRadioFeeSchedCpt)
	ON_BN_CLICKED(IDC_RADIO_FEE_SCHED_INVENTORY, OnRadioFeeSchedInventory)
	// (d.lange 2015-09-28 10:38) - PLID 67118
	ON_BN_CLICKED(IDC_LEFT_FEE_BTN, OnPreviousFeeSchedClicked)
	ON_BN_DOUBLECLICKED(IDC_LEFT_FEE_BTN, OnPreviousFeeSchedDoubleClicked)
	ON_BN_CLICKED(IDC_RIGHT_FEE_BTN, OnNextFeeSchedClicked)
	ON_BN_DOUBLECLICKED(IDC_RIGHT_FEE_BTN, OnNextFeeSchedDoubleClicked)
	// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_MULTIFEE_EFFECTIVEDATE_FROM_DATE, OnChangeMultiFeeEffectiveDateFromDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_MULTIFEE_EFFECTIVEDATE_TO_DATE, OnChangeMultiFeeEffectiveDateToDate)
	ON_BN_CLICKED(IDC_MULTIFEE_EFFECTIVEDATES_CHECK, OnMultiFeeEffectiveDatesCheck)
	ON_BN_CLICKED(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK, OnMultiFeeEffectiveDateToCheck)
	// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
	ON_BN_CLICKED(IDC_HIDE_INACTIVE_CHECK, OnHideInactiveCheck)
	//}}AFX_MSG_MAP		
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiFees message handlers

// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
// (b.cardillo 2015-11-02 13:14) - PLID 67113 - Only hide inactive/expired; future ones are still to be shown
const LPCTSTR l_strHideInactiveWhereClause = R"(
 Inactive = 0 AND (EffectiveToDate IS NULL OR EffectiveToDate >= dbo.AsDateNoTime(GETDATE()))
)";

BOOL CMultiFees::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (j.jones 2013-04-11 15:10) - PLID 12136 - added products, but we can disable that
		// feature if they don't have the license
		if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) {
			m_bHasInventory = false;
			m_radioServiceCodes.ShowWindow(SW_HIDE);
			m_radioInventoryItems.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PRODUCTS)->ShowWindow(SW_HIDE);
		}
	
		// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date. (default to hide)
		CheckDlgButton(IDC_HIDE_INACTIVE_CHECK, BST_CHECKED);
		m_pFeeGroups = BindNxDataListCtrl(IDC_FEEGROUPS, false);
		m_pFeeGroups->PutWhereClause(AsBstr((IsDlgButtonChecked(IDC_HIDE_INACTIVE_CHECK) == BST_CHECKED) ? l_strHideInactiveWhereClause : ""));
		m_pFeeGroups->Requery();

		m_pInsYes = BindNxDataListCtrl(IDC_INSYES, false);
		
		m_pInsNo = BindNxDataListCtrl(IDC_INSNO, false);
		
		m_pProvYes = BindNxDataListCtrl(IDC_PROVYES, false);
		
		m_pProvNo = BindNxDataListCtrl(IDC_PROVNO, false);

		m_pLocYes = BindNxDataListCtrl(IDC_LOCYES, false);
		
		m_pLocNo = BindNxDataListCtrl(IDC_LOCNO, false);
		
		m_pCPTCodes = BindNxDataListCtrl(IDC_CPTCODES, false);
		// (j.jones 2013-04-11 15:10) - PLID 12136 - added products
		m_pProducts = BindNxDataList2Ctrl(IDC_PRODUCTS, false);
		
		m_pFeeGroups->CurSel = 0;

		m_addButton.AutoSet(NXB_NEW);
		// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
		m_renameButton.AutoSet(NXB_MODIFY);
		m_deleteButton.AutoSet(NXB_DELETE);
		// (z.manning, 04/25/2008) - PLID 29566 - Fixed the update fees' button's style
		m_btnUpdateFees.AutoSet(NXB_MODIFY);
		
		m_addInsBtn.AutoSet(NXB_RIGHT);
		m_remInsBtn.AutoSet(NXB_LEFT);
		m_addProvBtn.AutoSet(NXB_RIGHT);
		m_remProvBtn.AutoSet(NXB_LEFT);
		m_btnAddLoc.AutoSet(NXB_RIGHT);
		m_btnRemLoc.AutoSet(NXB_LEFT);

		// (j.jones 2013-04-11 15:31) - PLID 12136 - always default to showing service codes
		m_radioServiceCodes.SetCheck(TRUE);
		OnRadioFeeSchedCpt();

		// (d.lange 2015-09-28 11:09) - PLID 67118
		m_previousFeeSchedBtn.AutoSet(NXB_LEFT);
		m_nextFeeSchedBtn.AutoSet(NXB_RIGHT);

		if(m_pFeeGroups->GetCurSel()!=-1) {
			Load();
		}
		else {
			GetDlgItem(IDC_DELETE)->EnableWindow(false);
			// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
			GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_WARN_MULTIFEE)->EnableWindow(FALSE);
			GetDlgItem(IDC_WARN_ALLOWABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INCLUDE_COPAY_ALLOWABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INCLUDE_PATRESP_ALLOWABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_NOTES)->EnableWindow(FALSE);
			//(e.lally 2007-04-04) PLID 25519 - Disable the inactive box when there is no selection
			GetDlgItem(IDC_INACTIVE_FEE_SCHEDULE)->EnableWindow(FALSE);
			// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
			GetDlgItem(IDC_RADIO_FEE_SCHED_LOCATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_FEE_SCHED_POS)->EnableWindow(FALSE);
			// (j.jones 2013-04-11 14:40) - PLID 12136 - disable the list, and the service/product radios
			m_pCPTCodes->PutEnabled(VARIANT_FALSE);			
			m_pProducts->PutEnabled(VARIANT_FALSE);
			GetDlgItem(IDC_RADIO_FEE_SCHED_CPT)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_FEE_SCHED_INVENTORY)->EnableWindow(FALSE);
			// (d.lange 2015-09-28 10:57) - PLID 67118
			m_previousFeeSchedBtn.EnableWindow(FALSE);
			m_nextFeeSchedBtn.EnableWindow(FALSE);

			// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
			UpdateEffectiveDateControls();
		}

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;
}



BEGIN_EVENTSINK_MAP(CMultiFees, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMultiFees)
	ON_EVENT(CMultiFees, IDC_FEEGROUPS, 2 /* SelectionChanged */, OnSelectionChangeFeeGroups, VTS_I4)
	ON_EVENT(CMultiFees, IDC_INSNO, 3 /* DblClickCell */, OnDblClickInsno, VTS_I4 VTS_I2)
	ON_EVENT(CMultiFees, IDC_INSYES, 3 /* DblClickCell */, OnDblClickInsyes, VTS_I4 VTS_I2)
	ON_EVENT(CMultiFees, IDC_PROVNO, 3 /* DblClickCell */, OnDblClickProvno, VTS_I4 VTS_I2)
	ON_EVENT(CMultiFees, IDC_PROVYES, 3 /* DblClickCell */, OnDblClickProvyes, VTS_I4 VTS_I2)
	ON_EVENT(CMultiFees, IDC_CPTCODES, 10 /* EditingFinished */, OnEditingFinishedCptcodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CMultiFees, IDC_CPTCODES, 18 /* RequeryFinished */, OnRequeryFinishedCptcodes, VTS_I2)
	ON_EVENT(CMultiFees, IDC_CPTCODES, 9 /* EditingFinishing */, OnEditingFinishingCptcodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMultiFees, IDC_LOCNO, 3 /* DblClickCell */, OnDblClickCellLocno, VTS_I4 VTS_I2)
	ON_EVENT(CMultiFees, IDC_LOCYES, 3 /* DblClickCell */, OnDblClickCellLocyes, VTS_I4 VTS_I2)
	ON_EVENT(CMultiFees, IDC_CPTCODES, 8 /* EditingStarting */, OnEditingStartingCptcodes, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CMultiFees, IDC_PRODUCTS, 8, OnEditingStartingProducts, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CMultiFees, IDC_PRODUCTS, 9, OnEditingFinishingProducts, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMultiFees, IDC_PRODUCTS, 10, OnEditingFinishedProducts, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMultiFees::OnSelectionChangeFeeGroups(long iNewRow) 
{
	// TODO: Add your control notification handler code here
	Load();
}

void CMultiFees::OnAddIns() 
{
	try {

		//DRT 3/2/2004 - PLID 11184 - We have to check for no selection first
		if(m_pInsNo->GetCurSel() == -1)
			return;

		if(m_pFeeGroups->GetCurSel() == -1)
			return;

		EnsureRemoteData();

		CWaitCursor pWait;

		// (j.jones 2007-02-20 14:29) - PLID 24801 - supported selecting multiple companies
		long p = m_pInsNo->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p) {

			m_pInsNo->GetNextSelEnum(&p, &pDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			long nInsuranceCoID = VarLong(pRow->GetValue(0),-1);
			long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->GetCurSel(),0),-1);

			CString strInsuranceCoName = VarString(pRow->GetValue(1),"");

			//check various permutations

			//no provider, no location
			if(m_pProvYes->GetRowCount()==0 && m_pLocYes->GetRowCount()==0) {
				// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
				if(!CheckValidFeeGroup(nInsuranceCoID,-1,-1, GetEffectiveFromDate(), GetEffectiveToDate())) {
					//do NOT auto-delete the group, because otherwise it will be impossible to add a company/provider combo
					// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
					CString strWarn;
					strWarn.Format("The insurance company '%s' is already in a fee schedule without a provider or location."
							"\nThe same insurance company / provider / location combination can not be in multiple groups."
							"\nIf you do not intend to add a provider or location, please remove this company.",strInsuranceCoName);
					AfxMessageBox(strWarn);
				}
			}
			//has provider, no location
			else if(m_pProvYes->GetRowCount()>0 && m_pLocYes->GetRowCount()==0) {
				BOOL bStopChecking = FALSE;
				for(int i=0; i<m_pProvYes->GetRowCount() && !bStopChecking; i++) {					
					// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
					if(!CheckValidFeeGroup(nInsuranceCoID,m_pProvYes->GetValue(i,0).lVal,-1, GetEffectiveFromDate(), GetEffectiveToDate())) {

						CString strWarn;
						// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
						strWarn.Format("The insurance company '%s' is already in a fee schedule with one of the selected providers and no location."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add a location, please remove this company.",strInsuranceCoName);
						AfxMessageBox(strWarn);
						
						bStopChecking = TRUE;
					}
				}
			}
			//no provider, has location
			else if(m_pProvYes->GetRowCount()==0 && m_pLocYes->GetRowCount()>0) {
				BOOL bStopChecking = FALSE;
				for(int i=0; i<m_pLocYes->GetRowCount() && !bStopChecking; i++) {					
					// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
					if(!CheckValidFeeGroup(nInsuranceCoID,-1,m_pLocYes->GetValue(i,0).lVal, GetEffectiveFromDate(), GetEffectiveToDate())) {

						CString strWarn;
						// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
						strWarn.Format("The insurance company '%s' is already in a fee schedule with one of the selected locations and no provider."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add a provider, please remove this company.",strInsuranceCoName);
						AfxMessageBox(strWarn);

						bStopChecking = TRUE;
					}
				}
			}
			//has provider, has location
			else {
				for(int i=0; i<m_pProvYes->GetRowCount(); i++) {
					for(int j=0; j<m_pLocYes->GetRowCount(); j++) {
						// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
						if(!CheckValidFeeGroup(nInsuranceCoID,m_pProvYes->GetValue(i,0).lVal,m_pLocYes->GetValue(j,0).lVal, GetEffectiveFromDate(), GetEffectiveToDate())) {

							CString strWarn;
							// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
							strWarn.Format("The insurance company '%s' is already in a fee schedule with one of the selected providers and one of the selected locations."
								"\nThe same insurance company / provider / location combination can not be in multiple fee schedules.",strInsuranceCoName);
							AfxMessageBox(strWarn);
							return;
						}
					}
				}
			}

			ExecuteSql("INSERT INTO MultiFeeInsuranceT (FeeGroupID, InsuranceCoID) VALUES (%li, %li);", nGroupID, nInsuranceCoID);
			m_pInsYes->TakeRow(pRow);

			pDisp->Release();
		}
	}NxCatchAll("Error in OnAddIns()");

}

void CMultiFees::OnRemoveIns() 
{
	try {
		//DRT 3/2/2004 - PLID 11184 - We have to check for no selection first
		if(m_pInsYes->GetCurSel() == -1)
			return;

		if(m_pFeeGroups->GetCurSel() == -1)
			return;

		EnsureRemoteData();

		CWaitCursor pWait;

		// (j.jones 2007-02-20 14:29) - PLID 24801 - supported selecting multiple companies
		long p = m_pInsYes->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p) {

			m_pInsYes->GetNextSelEnum(&p, &pDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			long nInsuranceCoID = VarLong(pRow->GetValue(0),-1);
			long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->GetCurSel(),0),-1);

			ExecuteSql("DELETE FROM MultiFeeInsuranceT WHERE FeeGroupID = %li AND InsuranceCoID = %li", nGroupID, nInsuranceCoID);
			m_pInsNo->TakeRow(pRow);

			pDisp->Release();
		}
	}NxCatchAll("Error in OnRemoveIns()");	
}

void CMultiFees::OnAddProvider() 
{
	try {
		//DRT 3/2/2004 - PLID 11184 - We have to check for no selection first
		if(m_pProvNo->GetCurSel() == -1)
			return;

		if(m_pFeeGroups->GetCurSel() == -1)
			return;

		EnsureRemoteData();
		
		CWaitCursor pWait;

		// (j.jones 2007-02-20 14:29) - PLID 24801 - supported selecting multiple providers
		long p = m_pProvNo->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p) {

			m_pProvNo->GetNextSelEnum(&p, &pDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			long nProviderID = VarLong(pRow->GetValue(0),-1);
			long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->GetCurSel(),0),-1);

			CString strProviderName = VarString(pRow->GetValue(1),"");

			//check various permutations

			//no company, no location
			if(m_pInsYes->GetRowCount()==0 && m_pLocYes->GetRowCount()==0) {
				// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
				if(!CheckValidFeeGroup(-1,nProviderID,-1, GetEffectiveFromDate(), GetEffectiveToDate())) {
					//do NOT auto-delete the group, because otherwise it will be impossible to add a company/location combo

					CString strWarn;
					// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
					strWarn.Format("The provider '%s' is already in a fee schedule without an insurance company or location."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add an insurance company or location, please remove this provider.", strProviderName);
					AfxMessageBox(strWarn);
				}
			}
			//has company, no location
			else if(m_pInsYes->GetRowCount()>0 && m_pLocYes->GetRowCount()==0) {
				BOOL bStopChecking = FALSE;
				for(int i=0; i<m_pInsYes->GetRowCount() && !bStopChecking; i++) {
					// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
					if(!CheckValidFeeGroup(m_pInsYes->GetValue(i,0).lVal,nProviderID,-1, GetEffectiveFromDate(), GetEffectiveToDate())) {

						CString strWarn;
						// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
						strWarn.Format("The provider '%s' is already in a fee schedule with one of the selected insurance companies and no location."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add a location, please remove this provider.", strProviderName);
						AfxMessageBox(strWarn);

						bStopChecking = TRUE;
					}
				}
			}
			//no company, has location
			else if(m_pInsYes->GetRowCount()==0 && m_pLocYes->GetRowCount()>0) {
				BOOL bStopChecking = FALSE;
				for(int i=0; i<m_pLocYes->GetRowCount() && !bStopChecking; i++) {
					// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
					if(!CheckValidFeeGroup(-1,nProviderID,m_pLocYes->GetValue(i,0).lVal, GetEffectiveFromDate(), GetEffectiveToDate())) {

						CString strWarn;
						// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
						strWarn.Format("The provider '%s' is already in a fee schedule with one of the selected locations and no insurance company."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add an insurance company, please remove this provider.", strProviderName);
						AfxMessageBox(strWarn);

						bStopChecking = TRUE;
					}
				}
			}
			//has company, has location
			else {
				for(int i=0; i<m_pInsYes->GetRowCount(); i++) {
					for(int j=0; j<m_pLocYes->GetRowCount(); j++) {
						// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
						if(!CheckValidFeeGroup(m_pInsYes->GetValue(i,0).lVal,nProviderID,m_pLocYes->GetValue(j,0).lVal, GetEffectiveFromDate(), GetEffectiveToDate())) {

							CString strWarn;
							// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
							strWarn.Format("The provider '%s' is already in a fee schedule with one of the selected insurance companies and one of the selected locations."
								"\nThe same insurance company / provider / location combination can not be in multiple fee schedules.", strProviderName);
							AfxMessageBox(strWarn);

							return;
						}
					}
				}
			}

			ExecuteSql("INSERT INTO MultiFeeProvidersT (FeeGroupID, ProviderID) VALUES (%li, %li);", nGroupID, nProviderID);
			m_pProvYes->TakeRow(pRow);

			pDisp->Release();
		}
	}NxCatchAll("Error in OnAddProvider()");

}

void CMultiFees::OnRemoveProvider() 
{
	try {
		//DRT 3/2/2004 - PLID 11184 - We have to check for no selection first
		if(m_pProvYes->GetCurSel() == -1)
			return;

		if(m_pFeeGroups->GetCurSel() == -1)
			return;

		EnsureRemoteData();

		CWaitCursor pWait;

		// (j.jones 2007-02-20 14:29) - PLID 24801 - supported selecting multiple providers
		long p = m_pProvYes->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p) {

			m_pProvYes->GetNextSelEnum(&p, &pDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			long nProviderID = VarLong(pRow->GetValue(0),-1);
			long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->GetCurSel(),0),-1);

			ExecuteSql("DELETE FROM MultiFeeProvidersT WHERE FeeGroupID = %li AND ProviderID = %li", nGroupID, nProviderID);
			m_pProvNo->TakeRow(pRow);

			pDisp->Release();
		}
	}NxCatchAll("Error in OnRemoveProvider()");
	
}

void CMultiFees::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

void CMultiFees::Load()
{
	// (c.haag 2007-03-13 11:26) - PLID 25185 - The entire function is now protected
	// in a try/catch
	try{
		CString strFrom;
		CMsgBox dlg(this);
		CString strTmp;

		if(m_pFeeGroups->GetCurSel() == NXDATALISTLib::sriNoRow){
			m_pInsYes->Clear();
			m_pInsNo->Clear();
			m_pProvYes->Clear();
			m_pProvNo->Clear();
			m_pLocYes->Clear();
			m_pLocNo->Clear();
			m_pCPTCodes->Clear();
			m_pProducts->Clear();
			GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
			// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
			GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_WARN_MULTIFEE)->EnableWindow(FALSE);
			GetDlgItem(IDC_WARN_ALLOWABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INCLUDE_COPAY_ALLOWABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INCLUDE_PATRESP_ALLOWABLE)->EnableWindow(FALSE);			
			GetDlgItem(IDC_NOTES)->EnableWindow(FALSE);
			//(e.lally 2007-04-04) PLID 25519 - Disable the inactive box when there is no selection
			GetDlgItem(IDC_INACTIVE_FEE_SCHEDULE)->EnableWindow(FALSE);
			// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
			GetDlgItem(IDC_RADIO_FEE_SCHED_LOCATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_FEE_SCHED_POS)->EnableWindow(FALSE);
			// (j.jones 2013-04-11 14:40) - PLID 12136 - disable the list, and the service/product radios
			m_pCPTCodes->PutEnabled(VARIANT_FALSE);
			m_pProducts->PutEnabled(VARIANT_FALSE);
			GetDlgItem(IDC_RADIO_FEE_SCHED_CPT)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_FEE_SCHED_INVENTORY)->EnableWindow(FALSE);

			// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
			UpdateEffectiveDateControls();
			return;
		}
		
		long nCurSel = m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0);
		BOOL bUsePOS = FALSE;
		{
			_RecordsetPtr rs = CreateParamRecordset(R"(
SELECT WarnMultiFee, WarnAllowable, IncludeCoPayAllowable, IncludePatRespAllowable, AcceptAssignment
 , BillStandardFee, Inactive, Note, UsePOS
 , EffectiveFromDate, EffectiveToDate 
FROM MultiFeeGroupsT 
WHERE ID = {INT}
)"
				, nCurSel);
			if (!rs->eof) {
				// (c.haag 2007-03-13 11:25) - PLID 25185 - We now pull the fields in cleanly
				FieldsPtr f = rs->Fields;
				m_btnWarnMultiFee.SetCheck(AdoFldBool(f, "WarnMultiFee") ? 1 : 0);
				m_btnWarnAllowable.SetCheck(AdoFldBool(f, "WarnAllowable") ? 1 : 0);
				m_checkIncludeCoPayAllowable.SetCheck(AdoFldBool(f, "IncludeCoPayAllowable") ? 1 : 0);
				// (j.jones 2007-05-03 11:19) - PLID 25840 - added option to include patient resp. in allowable
				m_checkIncludePatRespAllowable.SetCheck(AdoFldBool(f, "IncludePatRespAllowable") ? 1 : 0);
				m_btnAccept.SetCheck(AdoFldBool(f, "AcceptAssignment") ? 1 : 0);
				m_btnStandard.SetCheck(AdoFldBool(f, "BillStandardFee") ? 1 : 0);
				m_checkInactive.SetCheck(AdoFldBool(f, "Inactive") ? 1 : 0);
				SetDlgItemText(IDC_NOTES, AdoFldString(f, "Note", ""));
				// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
				bUsePOS = AdoFldBool(f, "UsePOS");
				m_radioLocation.SetCheck(!bUsePOS);
				m_radioPOS.SetCheck(bUsePOS);
				// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
				COleDateTime dtEffectiveFrom = AdoFldDateTime(rs, "EffectiveFromDate", g_cdtNull);
				COleDateTime dtEffectiveTo = AdoFldDateTime(rs, "EffectiveToDate", g_cdtNull);
				m_dtEffectiveDateFromDate.SetValue(dtEffectiveFrom);
				m_dtEffectiveDateToDate.SetValue(dtEffectiveTo);
				CheckDlgButton(IDC_MULTIFEE_EFFECTIVEDATES_CHECK, dtEffectiveFrom != g_cdtNull ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK, (dtEffectiveFrom != g_cdtNull && dtEffectiveTo != g_cdtNull) ? BST_CHECKED : BST_UNCHECKED);
			}
			rs->Close();
		}

		strFrom.Format("(SELECT MultiFeeInsuranceT.* FROM MultiFeeInsuranceT WHERE FeeGroupID = %li) AS MultiFeeInsuranceT RIGHT JOIN InsuranceCoT ON MultiFeeInsuranceT.InsuranceCoID = InsuranceCoT.PersonID LEFT JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID", nCurSel);
		m_pInsYes->FromClause = _bstr_t(strFrom);
		m_pInsNo->FromClause = _bstr_t(strFrom);
		
		strFrom.Format("(SELECT MultiFeeProvidersT.* FROM MultiFeeProvidersT WHERE FeeGroupID = %li) AS MultiFeeProvidersT RIGHT JOIN ProvidersT ON MultiFeeProvidersT.ProviderID = ProvidersT.PersonID INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID", nCurSel);
		m_pProvYes->FromClause = _bstr_t(strFrom);
		m_pProvNo->FromClause = _bstr_t(strFrom);

		// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
		// (d.singleton 2013-05-08 11:28) - PLID 53852 - needs to show inactive locations and providers if they are in the selected list.
		// (d.lange 2015-09-30 11:12) - PLID 67117 - Replaced "Group" with "Fee Schedule"
		if(!bUsePOS) {
			m_nxstaticLocationUnselectedLabel.SetWindowText("Locations Not In Fee Schedule");
			m_nxstaticLocationSelectedLabel.SetWindowText("Locations In Fee Schedule");
			m_pLocYes->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Not Null AND LocationsT.Managed = 1 AND LocationsT.TypeID = 1");
			m_pLocNo->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Null AND LocationsT.Managed = 1 AND LocationsT.Active = 1 AND LocationsT.TypeID = 1");
		}
		else {
			// (d.singleton 2013-05-08 11:28) - PLID 53852 - needs to show inactive locations and providers if they are in the selected list.
			m_nxstaticLocationUnselectedLabel.SetWindowText("Places Of Service Not In Fee Schedule");
			m_nxstaticLocationSelectedLabel.SetWindowText("Places Of Service In Fee Schedule");
			m_pLocYes->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Not Null AND LocationsT.TypeID = 1");
			m_pLocNo->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Null AND LocationsT.Active = 1 AND LocationsT.TypeID = 1");
		}

		strFrom.Format("(SELECT MultiFeeLocationsT.* FROM MultiFeeLocationsT WHERE FeeGroupID = %li) AS MultiFeeLocationsT RIGHT JOIN LocationsT ON MultiFeeLocationsT.LocationID = LocationsT.ID", nCurSel);
		m_pLocYes->FromClause = _bstr_t(strFrom);
		m_pLocNo->FromClause = _bstr_t(strFrom);

		// (j.jones 2007-02-20 09:03) - PLID 24115 - ensured that the "new fee" field is blank unless
		// there really is a new multifee
		strFrom.Format("(SELECT CPTCodeT.ID, CPTCodeT.Code, CPTCodeT.SubCode, CASE WHEN Anesthesia = 1 AND UseAnesthesiaBilling = 1 THEN 1 ELSE 0 END AS IsAnesthesia, CASE WHEN FacilityFee = 1 AND UseFacilityBilling = 1 THEN 1 ELSE 0 END AS IsFacilityFee, ServiceT.Name, ServiceT.Price, MultiFeeItemsT.Price AS NewFee, ServiceT.Active, MultiFeeItemsT.Allowable "
			"FROM CPTCodeT INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID LEFT OUTER JOIN (SELECT MultiFeeItemsT.* FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li) AS MultiFeeItemsT ON CPTCodeT.ID = MultiFeeItemsT.ServiceID) AS Query", nCurSel);
		m_pCPTCodes->FromClause = _bstr_t(strFrom);
		// (j.jones 2013-04-11 15:38) - PLID 12136 - supported products
		if(m_bHasInventory) {
			strFrom.Format("(SELECT ProductT.ID, ProductT.InsCode, ServiceT.Name, ProductT.LastCost, ServiceT.Price, MultiFeeItemsT.Price AS NewFee, ServiceT.Active, MultiFeeItemsT.Allowable "
				"FROM ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"LEFT OUTER JOIN (SELECT MultiFeeItemsT.* FROM MultiFeeItemsT WHERE MultiFeeItemsT.FeeGroupID = %li) AS MultiFeeItemsT ON ProductT.ID = MultiFeeItemsT.ServiceID) AS Query", nCurSel);
			m_pProducts->FromClause = _bstr_t(strFrom);
		}

		m_pInsYes->Requery();
		m_pInsNo->Requery();
		m_pProvYes->Requery();
		m_pProvNo->Requery();
		m_pLocYes->Requery();
		m_pLocNo->Requery();
		GetDlgItem(IDC_DELETE)->EnableWindow(FALSE);
		// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
		GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_WARN_MULTIFEE)->EnableWindow(FALSE);
		GetDlgItem(IDC_WARN_ALLOWABLE)->EnableWindow(FALSE);
		GetDlgItem(IDC_INCLUDE_COPAY_ALLOWABLE)->EnableWindow(FALSE);
		GetDlgItem(IDC_INCLUDE_PATRESP_ALLOWABLE)->EnableWindow(FALSE);		
		GetDlgItem(IDC_NOTES)->EnableWindow(FALSE);
		//(e.lally 2007-04-04) PLID 25519 - Disable the inactive box while we requery
		GetDlgItem(IDC_INACTIVE_FEE_SCHEDULE)->EnableWindow(FALSE);
		// (j.jones 2013-04-11 15:40) - PLID 12136 - now this just clears the text
		GetDlgItem(IDC_ANESTH_FEE_LABEL)->SetWindowText("");
		// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
		GetDlgItem(IDC_RADIO_FEE_SCHED_LOCATION)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_FEE_SCHED_POS)->EnableWindow(FALSE);

		// (j.jones 2013-04-11 14:40) - PLID 12136 - enable the lists now, and the service/product radios
		m_pCPTCodes->PutEnabled(VARIANT_TRUE);
		m_pProducts->PutEnabled(VARIANT_TRUE);
		GetDlgItem(IDC_RADIO_FEE_SCHED_CPT)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_FEE_SCHED_INVENTORY)->EnableWindow(TRUE);

		// Enable the related controls as appropriate
		UpdateEffectiveDateControls();

		// Update the fee schedule display name
		UpdateFeeSchedDisplayName();

		m_pCPTCodes->Requery();
		// (j.jones 2013-04-11 15:38) - PLID 12136 - supported products
		if(m_bHasInventory) {
			m_pProducts->Requery();
		}
	}NxCatchAll("Error in Load");
}



void CMultiFees::OnKillfocusNotes() 
{
	CString strNotes, strNotesSql;
	GetDlgItemText(IDC_NOTES, strNotes);
	try{
		// (m.hancock 2006-10-18 12:51) - PLID 17453 - Removed 50 character limitation for the note field and ensured
		// a blank string is always inserted if the note is empty.
		CString strNotesSql = "";
		if(strNotes.GetLength() > 0)
			strNotesSql = strNotes;
		ExecuteSql("UPDATE MultiFeeGroupsT SET Note = '%s' WHERE ID = %li", _Q(strNotesSql), m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);
	}NxCatchAll("Error in OnKillfocusNotes()");
}

void CMultiFees::OnWarnMultiFee() 
{
	int nChecked;
	if(m_btnWarnMultiFee.GetCheck()) nChecked = -1;
	else nChecked = 0;
	try{
		ExecuteSql("UPDATE MultiFeeGroupsT SET WarnMultiFee = %li WHERE ID = %li", nChecked, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);
	}NxCatchAll("Error in OnWarnMultiFee()");
}

void CMultiFees::OnWarnAllowable() 
{
	int nChecked;
	if(m_btnWarnAllowable.GetCheck()) nChecked = -1;
	else nChecked = 0;
	try{
		ExecuteSql("UPDATE MultiFeeGroupsT SET WarnAllowable = %li WHERE ID = %li", nChecked, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);
	}NxCatchAll("Error in OnWarnAllowable()");
}


void CMultiFees::OnAccept() 
{
	int nChecked;
	if(m_btnAccept.GetCheck()) nChecked = -1;
	else nChecked = 0;
	try{
		ExecuteSql("UPDATE MultiFeeGroupsT SET AcceptAssignment = %li WHERE ID = %li", nChecked, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);
	}NxCatchAll("Error in OnAccept()");	
}

void CMultiFees::OnBillstandard() 
{
	int nChecked;
	if(m_btnStandard.GetCheck()) nChecked = -1;
	else nChecked = 0;
	try{
		ExecuteSql("UPDATE MultiFeeGroupsT SET BillStandardFee = %li WHERE ID = %li", nChecked, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);
	}NxCatchAll("Error in OnBillstandard()");
}

void CMultiFees::OnDblClickInsno(long nRowIndex, short nColIndex) 
{
	OnAddIns();
	
}

void CMultiFees::OnDblClickInsyes(long nRowIndex, short nColIndex) 
{
	OnRemoveIns();
	
}

void CMultiFees::OnDblClickProvno(long nRowIndex, short nColIndex) 
{
	OnAddProvider();
	
}

void CMultiFees::OnDblClickProvyes(long nRowIndex, short nColIndex) 
{
	OnRemoveProvider();
	
}

void CMultiFees::OnEditingFinishingCptcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(nCol == slcNewFee || nCol == slcAllowable) {
		CString str = strUserEntered;
		str.TrimRight();
		if(str == "") {
			pvarNewValue->vt = VT_NULL;
			return;
		}
		COleCurrency cy = COleCurrency(pvarNewValue->cyVal);
		RoundCurrency(cy);
		pvarNewValue->cyVal = cy;
	}	
}

void CMultiFees::OnEditingFinishedCptcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(m_pFeeGroups->CurSel==-1)
		return;

	try {

		// (j.jones 2008-05-30 17:47) - PLID 30233 - check the bCommit flag
		if(!bCommit) {
			return;
		}

		if(nCol != slcNewFee && nCol != slcAllowable) {
			return;
		}

		long GroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 0), -1);
		CString strGroupName = VarString(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 1), "");
		long ServiceID = VarLong(m_pCPTCodes->GetValue(nRow, slcID), -1);
		CString strServiceCode = VarString(m_pCPTCodes->GetValue(nRow, slcCode), "");

		CString strNewValue = "NULL";

		//for auditing		
		CString strOldAmount = "", strNewAmount = "";
		if(varOldValue.vt == VT_CY) {
			strOldAmount = FormatCurrencyForInterface(VarCurrency(varOldValue), TRUE, TRUE);
		}
		else {
		}

		if(varNewValue.vt == VT_CY) {
			COleCurrency cyNewValue = VarCurrency(varNewValue,COleCurrency(0,0));
			strNewValue.Format("Convert(money,'%s')",_Q(FormatCurrencyForSql(cyNewValue)));
			strNewAmount = FormatCurrencyForInterface(cyNewValue, TRUE, TRUE);
		}
		else {
			strNewValue = "NULL";
		}

		_RecordsetPtr rs;
		switch(nCol) {
		case slcNewFee: {	
			rs = CreateRecordset("SELECT Count(ServiceID) AS ItemCount FROM MultiFeeItemsT WHERE ServiceID = %li AND FeeGroupID = %li", ServiceID, GroupID);
			if(VarLong(rs->Fields->GetItem("ItemCount")->Value, 0) != 0)
				ExecuteSql("UPDATE MultiFeeItemsT SET Price = %s WHERE ServiceID = %li AND FeeGroupID = %li", strNewValue, ServiceID, GroupID);
			else
				ExecuteSql("INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Price) VALUES (%li, %li, %s)", GroupID, ServiceID, strNewValue);
			rs->Close();

			// (j.jones 2008-05-27 12:32) - PLID 26652 - added auditing
			// (d.lange 2015-11-18 14:45) - PLID 67117 - Renamed to use "Fee Schedule" instead of "Group"
			if(strOldAmount != strNewAmount) {
				long nAuditID = BeginNewAuditEvent();			
				CString strOld;
				strOld.Format("%s (Code: %s, Fee Schedule: %s)", strOldAmount.IsEmpty() ? "<None>" : strOldAmount, strServiceCode, strGroupName);
				CString strNew;
				strNew = strNewAmount.IsEmpty() ? "<None>" : strNewAmount;
				AuditEvent(-1, "", nAuditID, aeiIndivMultiFee, GroupID, strOld, strNew, aepMedium, aetChanged);
			}

			break;
		}
		case slcAllowable: {
			rs = CreateRecordset("SELECT Count(ServiceID) AS ItemCount FROM MultiFeeItemsT WHERE ServiceID = %li AND FeeGroupID = %li", ServiceID, GroupID);
			if(VarLong(rs->Fields->GetItem("ItemCount")->Value, 0) != 0)
				ExecuteSql("UPDATE MultiFeeItemsT SET Allowable = %s WHERE ServiceID = %li AND FeeGroupID = %li", strNewValue, ServiceID, GroupID);
			else
				ExecuteSql("INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Allowable) VALUES (%li, %li, %s)", GroupID, ServiceID, strNewValue);
			rs->Close();

			// (j.jones 2008-05-27 12:32) - PLID 26652 - added auditing
			// (d.lange 2015-11-18 14:45) - PLID 67117 - Renamed to use "Fee Schedule" instead of "Group"
			if(strOldAmount != strNewAmount) {
				long nAuditID = BeginNewAuditEvent();
				CString strOld;
				strOld.Format("%s (Code: %s, Fee Schedule: %s)", strOldAmount.IsEmpty() ? "<None>" : strOldAmount, strServiceCode, strGroupName);
				CString strNew;
				strNew = strNewAmount.IsEmpty() ? "<None>" : strNewAmount;
				AuditEvent(-1, "", nAuditID, aeiIndivMultiFeeAllowable, GroupID, strOld, strNew, aepMedium, aetChanged);
			}

			break;
		}
		}

	} NxCatchAll("Error editing fee schedule.");
}

void CMultiFees::OnRequeryFinishedCptcodes(short nFlags) 
{
	try {

		m_deleteButton.EnableWindow(TRUE);
		// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
		GetDlgItem(IDC_RENAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_WARN_MULTIFEE)->EnableWindow(TRUE);
		GetDlgItem(IDC_WARN_ALLOWABLE)->EnableWindow(TRUE);
		GetDlgItem(IDC_INCLUDE_COPAY_ALLOWABLE)->EnableWindow(TRUE);
		GetDlgItem(IDC_INCLUDE_PATRESP_ALLOWABLE)->EnableWindow(TRUE);
		GetDlgItem(IDC_NOTES)->EnableWindow(TRUE);
		//(e.lally 2007-04-04) PLID 25519 - Re-enable the inactive box
		GetDlgItem(IDC_INACTIVE_FEE_SCHEDULE)->EnableWindow(TRUE);
		// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
		GetDlgItem(IDC_RADIO_FEE_SCHED_LOCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_FEE_SCHED_POS)->EnableWindow(TRUE);

		//if we are using the advanced anesthesia/facility billing, gray out any anesthesia and/or facility codes

		BOOL bItemsGrayed = FALSE;

		//now loop through the codes and gray out ones accordingly
		for(int i=0;i<m_pCPTCodes->GetRowCount();i++) {

			if(VarLong(m_pCPTCodes->GetValue(i,slcIsAnesth),0) == 1
				|| VarLong(m_pCPTCodes->GetValue(i,slcIsFacFee),0) == 1) {

				((NXDATALISTLib::IRowSettingsPtr)m_pCPTCodes->GetRow(i))->PutForeColor(RGB(127,127,127));

				bItemsGrayed = TRUE;
			}
		}

		//enable the warning to tell the user what grayed out codes mean
		if(bItemsGrayed) {
			// (j.jones 2013-04-11 15:40) - PLID 12136 - now this just sets the text, it might not
			// be shown if the user is viewing products
			GetDlgItem(IDC_ANESTH_FEE_LABEL)->SetWindowText("Anesthesia / Facility Fees are grayed out because their fees are calculated and will not be pulled from this table.");
			if(m_radioServiceCodes.GetCheck()) {
				GetDlgItem(IDC_ANESTH_FEE_LABEL)->ShowWindow(SW_SHOW);
			}
		}
		else {
			GetDlgItem(IDC_ANESTH_FEE_LABEL)->SetWindowText("");
		}

		// (d.lange 2015-09-28 11:41) - PLID 67118 - Based on the current selection, enable/disable the navigation buttons
		UpdateFeeSchedNavigationButtons();
	}NxCatchAll("Error in OnRequeryFinishedCptcodes");
}

// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
void CMultiFees::RequeryFeeGroups()
{
	// We're about to requery, so forget the fact that we're out of date
	m_feegroupsChecker.Changed();
			// (b.cardillo 2015-09-30 14:00) - PLID 67154 - Don't abandon our current selection!
			// We're going to requery the dropdown and reload the screen; try to keep the current selection 
			long nCurSelMultiFeeGroupID;
			{
				long nCurSel = m_pFeeGroups->GetCurSel();
				if (nCurSel != NXDATALISTLib::sriNoRow) {
					nCurSelMultiFeeGroupID = VarLong(m_pFeeGroups->GetValue(nCurSel, 0));
				} else {
					nCurSelMultiFeeGroupID = -1;
				}
			}
			// Requery
	m_pFeeGroups->PutWhereClause(AsBstr((IsDlgButtonChecked(IDC_HIDE_INACTIVE_CHECK) == BST_CHECKED) ? l_strHideInactiveWhereClause : ""));
	m_pFeeGroups->Requery();
	// Set back to the current selection, or the first row if there wans't one before
	if (nCurSelMultiFeeGroupID != -1) {
		// (b.cardillo 2015-10-06 09:52) - PLID 67113 (supplemental)
		// Try to set to whatever we had selected last, and failing that, set to the first entry.
		if (m_pFeeGroups->SetSelByColumn(0, nCurSelMultiFeeGroupID) == NXDATALISTLib::sriNoRow) {
			m_pFeeGroups->PutCurSel(0);
		}
	} else {
		// We had nothing selected before, initialize to the first entry.
		m_pFeeGroups->PutCurSel(0);
	}
	// Reload the screen
	Load();
}

void CMultiFees::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		if (m_feegroupsChecker.Changed())
		{
			// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Moved to a function to share code with 
			// the ability to hide inactive / out of date.
			RequeryFeeGroups();
		}
		if (m_companyChecker.Changed())
		{
			m_pInsYes->Requery();
			m_pInsNo->Requery();
		}
		if (m_providerChecker.Changed())
		{
			m_pProvYes->Requery();
			m_pProvNo->Requery();
		}
		if (m_locationChecker.Changed())
		{
			m_pLocYes->Requery();
			m_pLocNo->Requery();
		}
		if (m_CPTChecker.Changed() || m_feeitemsChecker.Changed())
		{
			if(m_pFeeGroups->GetCurSel()!=-1) {
				// (j.jones 2013-04-11 15:40) - PLID 12136 - now this just sets the text
				GetDlgItem(IDC_ANESTH_FEE_LABEL)->SetWindowText("");
				m_pCPTCodes->Requery();
				// (j.jones 2013-04-11 15:38) - PLID 12136 - supported products
				if(m_bHasInventory) {
					m_pProducts->Requery();
				}
			}
		}
		// (j.jones 2013-04-11 15:10) - PLID 12136 - added products
		if (m_ProductChecker.Changed())
		{
			if(m_pFeeGroups->GetCurSel()!=-1) {
				m_pProducts->Requery();
			}
		}
		if (m_feeitemsChecker.Changed())
		{
			if(m_pFeeGroups->GetCurSel()!=-1) {
				// (j.jones 2013-04-11 15:40) - PLID 12136 - now this just sets the text
				GetDlgItem(IDC_ANESTH_FEE_LABEL)->SetWindowText("");
				m_pCPTCodes->Requery();
				// (j.jones 2013-04-11 15:10) - PLID 12136 - added products,
				// fees could have possibly changed for both services & products
				m_pProducts->Requery();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Made this static since it has nothing to do with our current fee schedule
// Returns false if the given combination of insco, provider, and location already 
// exist on any multifee in data whose effective dates overlap with the given 
// effective dates.
// If no such combination exists, returns true.
// Parameter values must be either real data IDs or -1 to indicate null. For dates, 
// use COleDateTime::DateTimeStatus::null values to indicate null.
bool CMultiFees::CheckValidFeeGroup(long insuranceCoID, long providerID, long locationID, const COleDateTime &effectiveFromDate, const COleDateTime &effectiveToDate)
{
	//JMJ 8/7/2003 - this query now translates the absence of data to be data,
	//i.e. if an insurance/provider/location is not present, the value is -1
	
	// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range; parameterize
	return !ReturnsRecordsParam(R"(
SELECT * FROM (
 SELECT MultiFeeGroupsT.Inactive
  , ISNULL(MultiFeeGroupsT.EffectiveFromDate, '1753-01-01') AS EffectiveFromDate
  , ISNULL(MultiFeeGroupsT.EffectiveToDate, '9999-12-31') AS EffectiveToDate
  , ISNULL(MultiFeeInsuranceT.InsuranceCoID, -1) AS InsuranceCoID
  , ISNULL(MultiFeeProvidersT.ProviderID, -1) AS ProviderID
  , ISNULL(MultiFeeLocationsT.LocationID, -1) AS LocationID 
 FROM MultiFeeGroupsT 
 LEFT JOIN MultiFeeInsuranceT ON MultiFeeGroupsT.ID = MultiFeeInsuranceT.FeeGroupID 
 LEFT JOIN MultiFeeProvidersT ON MultiFeeGroupsT.ID = MultiFeeProvidersT.FeeGroupID 
 LEFT JOIN MultiFeeLocationsT ON MultiFeeGroupsT.ID = MultiFeeLocationsT.FeeGroupID
) AS MultiFeeCombinationsQ
CROSS JOIN (
 SELECT 
    {INT} AS InsuranceCoID
  , {INT} AS ProviderID
  , {INT} AS LocationID
  , ISNULL({VT_DATE}, '1753-01-01') AS EffectiveFromDate
  , ISNULL({VT_DATE}, '9999-12-31') AS EffectiveToDate
) AS ParametersQ
WHERE MultiFeeCombinationsQ.Inactive = 0
 AND MultiFeeCombinationsQ.InsuranceCoID = ParametersQ.InsuranceCoID
 AND MultiFeeCombinationsQ.ProviderID = ParametersQ.ProviderID
 AND MultiFeeCombinationsQ.LocationID = ParametersQ.LocationID
 AND MultiFeeCombinationsQ.EffectiveFromDate BETWEEN '1753-01-01' AND ParametersQ.EffectiveToDate
 AND MultiFeeCombinationsQ.EffectiveToDate BETWEEN ParametersQ.EffectiveFromDate AND '9999-12-31'
)"
		, insuranceCoID
		, providerID
		, locationID
		, AsVariant(effectiveFromDate)
		, AsVariant(effectiveToDate)
		);
}

void CMultiFees::OnAdd() 
{
	CString strResult;
	long	nID;
	_variant_t tmpVar;
	// (d.lange 2015-10-29 12:22) - PLID 67117 - Renamed to refer to fee schedule
	int nResult = InputBoxLimited(this, "Enter a name for this Fee Schedule.", strResult, "",50,false,false,NULL);

	if (nResult == IDOK && strResult != "")
	try
	{
		tmpVar.SetString(strResult);
		if(m_pFeeGroups->FindByColumn(1,tmpVar,0,FALSE)!=-1) {
			AfxMessageBox("This name already exists in the list, please enter a new name.");
			return;
		}

		nID = NewNumber("MultiFeeGroupsT","ID");

		ExecuteSql("INSERT INTO MultiFeeGroupsT (ID, Name) VALUES (%li, '%s')", nID, _Q(strResult));

		m_deleteButton.EnableWindow(FALSE);
		// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
		GetDlgItem(IDC_RENAME)->EnableWindow(FALSE);
		// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
		m_pFeeGroups->PutWhereClause(AsBstr((IsDlgButtonChecked(IDC_HIDE_INACTIVE_CHECK) == BST_CHECKED) ? l_strHideInactiveWhereClause : ""));
		m_pFeeGroups->Requery();
		m_pFeeGroups->SetSelByColumn(1, &tmpVar);

		// Network stuff
		m_feegroupsChecker.Refresh();

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiMultifeeCreate, nID, "", strResult, aepMedium, aetCreated);

		Load();
	}NxCatchAll("Error in OnClickNew()");	
}

// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Added rename button to change the name of an existing fee schedule
void CMultiFees::OnRename()
{
	try {
		// Get current selected row
		long nCurSel = m_pFeeGroups->CurSel;
		if (nCurSel == -1) {
			return;
		}

		// Get the data ID and the name from the datalist
		long nID = VarLong(m_pFeeGroups->GetValue(nCurSel, 0));
		CString strOldName = VarString(m_pFeeGroups->GetValue(nCurSel, 1)).Trim();
		CString strNewName = strOldName;

		// Prompt the user for a new name. This is in a loop so the user can try again if something 
		// is invalid. Success breaks out of the loop, exceptions throw out.
		while (InputBoxLimited(this, "Enter a new name for this Fee Schedule.", strNewName, "", 50, false, false, NULL) == IDOK) {
			strNewName.Trim();
			if (!strNewName.IsEmpty() && strNewName != strOldName) {
				// Ask the API to change the name
				if (VARIANT_FALSE != GetAPI()->RenameMultiFeeGroup(GetAPISubkey(), GetAPILoginToken(), AsBstr(FormatString("%li", nID)), AsBstr(strNewName))->Result) {
					// Success. Reflect the new name in the datalist.
					m_pFeeGroups->PutValue(nCurSel, 1, AsVariant(strNewName));

					// (d.lange 2015-10-06 09:51) - PLID 67122 - Update the Fee Schedule display name with the new name
					UpdateFeeSchedDisplayName();

					// Success, break out of the loop
					break;
				} else {
					// This name is already in use by another fee schedule
					MessageBox("The specified name is already used by another Fee Schedule. Please enter a unique name.", NULL, MB_OK | MB_ICONEXCLAMATION);
					// Loop back up and prompt the user again
				}
			} else {
				// User hit ok but didn't change the name or left it empty
				MessageBox("Please enter a new name for this Fee Schedule.", NULL, MB_OK | MB_ICONEXCLAMATION);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnDelete() 
{
	CString		sql;
	long		nGroupID;
	_variant_t	tmpVar;

	try {
		if(m_pFeeGroups->CurSel==-1)
			return;

		// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
		if(MessageBox("Are you sure you wish to delete this Fee Schedule?","NexTech",MB_YESNO)==IDNO)
			return;

		CWaitCursor pWait;

		tmpVar = m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 0);

		if(tmpVar.vt != VT_EMPTY)
		{
			nGroupID = tmpVar.lVal;

			CString strOld = CString(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 1).bstrVal);

			BEGIN_TRANS("DeleteMultiFeeGroup") {
				ExecuteSql("DELETE FROM MultiFeeLocationsT WHERE FeeGroupID = %li", nGroupID);
				ExecuteSql("DELETE FROM MultiFeeInsuranceT WHERE FeeGroupID = %li", nGroupID);
				ExecuteSql("DELETE FROM MultiFeeItemsT WHERE FeeGroupID = %li", nGroupID);
				ExecuteSql("DELETE FROM MultiFeeProvidersT WHERE FeeGroupID = %li", nGroupID);
				ExecuteSql("DELETE FROM MultiFeeGroupsT WHERE ID = %li", nGroupID);
			} END_TRANS_CATCH_ALL("DeleteMultiFeeGroup");

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiMultifeeDelete, nGroupID, strOld, "<Deleted>", aepMedium, aetDeleted);

			m_pFeeGroups->RemoveRow(m_pFeeGroups->CurSel);
			m_pFeeGroups->CurSel = 0;

			// Network stuff
			CClient::RefreshTable(NetUtils::MultiFeeGroupsT);

			// (b.cardillo 2015-10-02 22:54) - PLID 67120 - This item would have added once again 
			// to this code like everyone else, but since this entire clearing logic is duplicated 
			// in the Load() function, just call it and be done.
			Load();
		}
	} NxCatchAll("Error deleting fee schedule.");	
}

void CMultiFees::OnUpdateFees() 
{
	try {

		// (j.jones 2013-04-11 16:01) - PLID 12136 - Update by RVU and Update From File
		// are only supported on service codes. So if they are viewing products, we
		// can skip the menu and go directly to the update by percentage screen.
		if(m_bHasInventory && m_radioInventoryItems.GetCheck()) {
			OnUpdatePricesByPercentage();
			return;
		}

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UPDATE_BY_PERCENT, "Update By &Percentage");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UPDATE_BY_RVU, "Update By &RVU");
		// (a.walling 2007-02-22 13:09) - PLID 2470 - Update multifees from a CSV file
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UPDATE_FROM_FILE, "Update From &File");
		
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_UPDATE_PRICES);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}	

	}NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnUpdatePricesByPercentage()
{
	try {

		long nCurSel = m_pFeeGroups->CurSel;
		if(nCurSel == -1) {
			MsgBox("Please select a fee schedule.");
			return;
		}

		//Invoke the dialog that allows them to update all fees.
		CUpdateMultiFeeScheduleDlg dlg(this);

		// (j.jones 2013-04-11 16:01) - PLID 12136 - tell the dialog whether to update products or services,
		// we will never update both at the same time
		if(m_bHasInventory && m_radioInventoryItems.GetCheck()) {
			dlg.m_bUpdatingProducts = true;
		}
		else {
			dlg.m_bUpdatingProducts = false;
		}

		dlg.m_nCurrentID = VarLong(m_pFeeGroups->GetValue(nCurSel, 0), -1);
		dlg.m_strCurrentSchedule = VarString(m_pFeeGroups->GetValue(nCurSel, 1), "");
		int res = dlg.DoModal();
		if(res == IDOK) {
			//refresh the list
			Load();
		}
	}NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnUpdatePricesByRVU()
{
	try {

		// (j.jones 2013-04-11 16:01) - PLID 12136 - Update by RVU is only supported on service codes.
		if(m_radioInventoryItems.GetCheck()) {
			//this should have never been permitted on products
			ASSERT(FALSE);
			MessageBox("Updating prices by RVU is only allowed on Service Codes.");
			return;
		}

		long nCurSel = m_pFeeGroups->CurSel;
		if(nCurSel == -1) {
			MsgBox("Please select a fee schedule.");
			return;
		}

		CUpdateFeesByRVUDlg dlg(this);
		dlg.m_FeeScheduleID = VarLong(m_pFeeGroups->GetValue(nCurSel, 0), -1);
		dlg.DoModal();
		Load();

	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2007-02-22 13:09) - PLID 2470 - Update multifees from a CSV file
void CMultiFees::OnUpdatePricesFromFile()
{
	try {

		// (j.jones 2013-04-11 16:01) - PLID 12136 - Update by File is only supported on service codes.
		if(m_radioInventoryItems.GetCheck()) {
			//this should have never been permitted on products
			ASSERT(FALSE);
			MessageBox("Updating prices from a file is only allowed on Service Codes.");
			return;
		}

		long nCurSel = m_pFeeGroups->CurSel;
		if(nCurSel == -1) {
			MsgBox("Please select a fee schedule.");
			return;
		}

		CMultiFeesImportDlg dlg(this);
		dlg.m_nFeeGroup = VarLong(m_pFeeGroups->GetValue(nCurSel, 0), -2);
		dlg.m_strFeeGroup = VarString(m_pFeeGroups->GetValue(nCurSel, 1), "");

		dlg.DoModal();

		Load(); // refresh this dialog

	}NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnRemoveLocation() 
{
	try {
		//DRT 3/2/2004 - PLID 11184 - We have to check for no selection first
		if(m_pLocYes->GetCurSel() == -1)
			return;

		if(m_pFeeGroups->GetCurSel() == -1)
			return;

		EnsureRemoteData();

		CWaitCursor pWait;

		// (j.jones 2007-02-20 14:29) - PLID 24801 - supported selecting multiple locations
		long p = m_pLocYes->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p) {

			m_pLocYes->GetNextSelEnum(&p, &pDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			long nLocationID = VarLong(pRow->GetValue(0),-1);
			long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->GetCurSel(),0),-1);

			ExecuteSql("DELETE FROM MultiFeeLocationsT WHERE FeeGroupID = %li AND LocationID = %li", nGroupID, nLocationID);
			m_pLocNo->TakeRow(pRow);

			pDisp->Release();
		}
		// (d.singleton 2013-05-08 11:49) - PLID 53852 - if we remove an inactive location we need to requery to remove from our unselected list
		m_pLocNo->Requery();
	}NxCatchAll("Error in OnRemoveLocation()");
}

void CMultiFees::OnAddLocation() 
{
	try {
		//DRT 3/2/2004 - PLID 11184 - We have to check for no selection first
		if(m_pLocNo->GetCurSel() == -1)
			return;

		if(m_pFeeGroups->GetCurSel() == -1)
			return;

		EnsureRemoteData();

		CWaitCursor pWait;

		// (j.jones 2007-02-20 14:29) - PLID 24801 - supported selecting multiple locations
		long p = m_pLocNo->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p) {

			m_pLocNo->GetNextSelEnum(&p, &pDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(pDisp);

			long nLocationID = VarLong(pRow->GetValue(0),-1);
			long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->GetCurSel(),0),-1);

			CString strLocationName = VarString(pRow->GetValue(1),"");

			//check various permutations

			//no company, no provider
			if(m_pInsYes->GetRowCount()==0 && m_pProvYes->GetRowCount()==0) {
				// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
				if(!CheckValidFeeGroup(-1,-1,nLocationID, GetEffectiveFromDate(), GetEffectiveToDate())) {
					//do NOT auto-delete the group, because otherwise it will be impossible to add a company/location combo

					CString strWarn;
					// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
					strWarn.Format("The location '%s' is already in a fee schedule without an insurance company or provider."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add an insurance company or provider, please remove this location.", strLocationName);

					AfxMessageBox(strWarn);
				}
			}
			//has company, no provider
			else if(m_pInsYes->GetRowCount()>0 && m_pProvYes->GetRowCount()==0) {
				BOOL bStopChecking = FALSE;
				for(int i=0; i<m_pInsYes->GetRowCount() && !bStopChecking; i++) {					
					// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
					if(!CheckValidFeeGroup(m_pInsYes->GetValue(i,0).lVal,nLocationID,-1, GetEffectiveFromDate(), GetEffectiveToDate())) {

						CString strWarn;
						// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
						strWarn.Format("The location '%s' is already in a fee schedule with one of the selected insurance companies and no provider."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add a provider, please remove this location.", strLocationName);
						AfxMessageBox(strWarn);

						bStopChecking = TRUE;
					}
				}
			}
			//no company, has provider
			else if(m_pInsYes->GetRowCount()==0 && m_pProvYes->GetRowCount()>0) {
				BOOL bStopChecking = FALSE;
				for(int i=0; i<m_pProvYes->GetRowCount() && !bStopChecking; i++) {					
					// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
					if(!CheckValidFeeGroup(-1,m_pProvYes->GetValue(i,0).lVal,nLocationID, GetEffectiveFromDate(), GetEffectiveToDate())) {

						CString strWarn;
						// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
						strWarn.Format("The location '%s' is already in a fee schedule with one of the selected providers and no insurance company."
							"\nThe same insurance company / provider / location combination can not be in multiple fee schedules."
							"\nIf you do not intend to add an insurance company, please remove this location.", strLocationName);
						AfxMessageBox(strWarn);

						bStopChecking = TRUE;
					}
				}
			}
			//has company, has provider
			else {
				for(int i=0; i<m_pInsYes->GetRowCount(); i++) {
					for(int j=0; j<m_pProvYes->GetRowCount(); j++) {
						// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range
						if(!CheckValidFeeGroup(m_pInsYes->GetValue(i,0).lVal,m_pProvYes->GetValue(j,0).lVal,nLocationID, GetEffectiveFromDate(), GetEffectiveToDate())) {

							CString strWarn;
							// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
							strWarn.Format("The location '%s' is already in a fee schedule with one of the selected insurance companies and one of the selected providers."
								"\nThe same insurance company / provider / location combination can not be in multiple fee schedules.", strLocationName);
							AfxMessageBox(strWarn);
							return;
						}
					}
				}
			}

			ExecuteSql("INSERT INTO MultiFeeLocationsT (FeeGroupID, LocationID) VALUES (%li, %li);", nGroupID, nLocationID);
			m_pLocYes->TakeRow(pRow);

			pDisp->Release();
		}
	}NxCatchAll("Error in OnAddLocation()");
}

void CMultiFees::OnDblClickCellLocno(long nRowIndex, short nColIndex) 
{
	OnAddLocation();
}

void CMultiFees::OnDblClickCellLocyes(long nRowIndex, short nColIndex) 
{
	OnRemoveLocation();	
}


void CMultiFees::OnIncludeCopayAllowable() 
{
	int nChecked;
	if(m_checkIncludeCoPayAllowable.GetCheck()) nChecked = -1;
	else nChecked = 0;
	try{
		ExecuteSql("UPDATE MultiFeeGroupsT SET IncludeCoPayAllowable = %li WHERE ID = %li", nChecked, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);

		// (j.jones 2007-05-03 11:41) - PLID 25840 - if both pat. resp. and copay allowable options
		// are checked, give a don't show warning
		TryWarnAllowableBehavior();

	}NxCatchAll("Error in OnIncludeCopayAllowable()");	
}

LRESULT CMultiFees::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			// (b.cardillo 2015-09-30 14:00) - PLID 67154 - We should probably not respond to any 
			// of these, but for this pl item just stopped responding to NetUtils::MultiFeeGroupsT.
			case NetUtils::InsuranceCoT:
			case NetUtils::Providers:
			case NetUtils::CPTCodeT:
			case NetUtils::MultiFeeItemsT:
			case NetUtils::LocationsT:{
				try {
					UpdateView();
				} NxCatchAll("Error in CMultiFees::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CMultiFees::OnTableChanged");

	return 0;
}

void CMultiFees::OnEditingStartingCptcodes(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	if(nRow == -1) {
		*pbContinue = FALSE;
		return;
	}

	if(VarLong(m_pCPTCodes->GetValue(nRow,slcIsAnesth),0) == 1
		|| VarLong(m_pCPTCodes->GetValue(nRow,slcIsFacFee),0) == 1) {

		*pbContinue = FALSE;
		return;
	}	
}

// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Static utility to warn about potential changes to the current fee schedule
// Pass an empty CSqlFragment for anything you want to pull from the existing data state, or a 
// populated CSqlFragment for anything you want to simulate changing to warn the user of a conflict.
bool CMultiFees::CheckFeeGroupChange(long feeGroupID, const CSqlFragment &sqlChangeInactive, const CSqlFragment &sqlChangeEffectiveFromDate, const CSqlFragment &sqlChangeEffectiveToDate)
{
	_RecordsetPtr rs = CreateParamRecordset(R"(
; WITH MultiFeeCombinationsQ AS (
 SELECT MultiFeeGroupsT.ID, MultiFeeGroupsT.Name, MultiFeeGroupsT.Inactive
  , ISNULL(MultiFeeGroupsT.EffectiveFromDate, '1753-01-01') AS EffectiveFromDate
  , ISNULL(MultiFeeGroupsT.EffectiveToDate, '9999-12-31') AS EffectiveToDate
  , ISNULL(MultiFeeInsuranceT.InsuranceCoID, -1) AS InsuranceCoID
  , ISNULL(MultiFeeProvidersT.ProviderID, -1) AS ProviderID
  , ISNULL(MultiFeeLocationsT.LocationID, -1) AS LocationID 
 FROM MultiFeeGroupsT 
 LEFT JOIN MultiFeeInsuranceT ON MultiFeeGroupsT.ID = MultiFeeInsuranceT.FeeGroupID 
 LEFT JOIN MultiFeeProvidersT ON MultiFeeGroupsT.ID = MultiFeeProvidersT.FeeGroupID 
 LEFT JOIN MultiFeeLocationsT ON MultiFeeGroupsT.ID = MultiFeeLocationsT.FeeGroupID
)
, MineQ AS (
 SELECT Q.ID, Q.Name, Q.InsuranceCoID, Q.ProviderID, Q.LocationID
  , {SQL} AS Inactive
  , ISNULL({SQL}, '1753-01-01') AS EffectiveFromDate
  , ISNULL({SQL}, '9999-12-31') AS EffectiveToDate
 FROM MultiFeeCombinationsQ Q
 WHERE Q.ID = {INT}
)
, TheirsQ AS (
 SELECT Q.*
 FROM MultiFeeCombinationsQ Q
 INNER JOIN MineQ M ON Q.ID <> M.ID
  AND Q.InsuranceCoID = M.InsuranceCoID 
  AND Q.ProviderID = M.ProviderID 
  AND Q.LocationID = M.LocationID 
  AND Q.EffectiveFromDate BETWEEN '1753-01-01' AND M.EffectiveToDate
  AND Q.EffectiveToDate BETWEEN M.EffectiveFromDate AND '9999-12-31'
  AND Q.Inactive = 0 
  AND M.Inactive = 0
)
SELECT TOP 1 T.Name
 , (SELECT A.Name FROM InsuranceCoT A WHERE A.PersonID = NULLIF(T.InsuranceCoID, -1)) AS InsuranceCoName
 , (SELECT A.Last + ', ' + A.First + ' ' + A.Middle AS Name FROM PersonT A WHERE A.ID = NULLIF(T.ProviderID, -1)) AS ProviderName
 , (SELECT A.Name FROM LocationsT A WHERE A.ID = NULLIF(T.LocationID, -1)) AS LocationName
 , NULLIF(T.EffectiveFromDate, '1753-01-01') AS EffectiveFromDate
 , NULLIF(T.EffectiveToDate, '9999-12-31') AS EffectiveToDate
FROM TheirsQ T
ORDER BY T.Name, T.InsuranceCoID, T.ProviderID, T.LocationID, T.ID
)"
		, sqlChangeInactive.IsEmpty() ? CSqlFragment("Q.Inactive") : sqlChangeInactive
		, sqlChangeEffectiveFromDate.IsEmpty() ? CSqlFragment("Q.EffectiveFromDate") : sqlChangeEffectiveFromDate
		, sqlChangeEffectiveToDate.IsEmpty() ? CSqlFragment("Q.EffectiveToDate") : sqlChangeEffectiveToDate
		, feeGroupID
		);

	if (!rs->eof) {
		// There is a duplicate, so return after giving a friendly message

		COleDateTime dtFrom = AdoFldDateTime(rs, "EffectiveFromDate", g_cdtNull);
		COleDateTime dtTo = AdoFldDateTime(rs, "EffectiveToDate", g_cdtNull);

		CString str;
		// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
		str.Format("There is another active overlapping fee schedule, named '%s', that uses the combination of:\n"
			"    Insurance Company: %s\n"
			"    Provider: %s\n"
			"    Location: %s\n"
			"    Effective: %s%s\n"
			"\n"
			"The same insurance company / provider / location combination should not be in multiple active overlapping fee schedules. Please choose a date range that does not conflict or change the fee schedule members to be unique."
			, AdoFldString(rs, "Name")
			, AdoFldString(rs, "InsuranceCoName", "<Any Insurance Company>")
			, AdoFldString(rs, "ProviderName", "<Any Provider>")
			, AdoFldString(rs, "LocationName", "<Any Location>")
			, (dtFrom.GetStatus() == COleDateTime::DateTimeStatus::null) ? "<All Dates>" : FormatDateTimeForInterface(dtFrom)
			, (dtFrom.GetStatus() == COleDateTime::DateTimeStatus::null) ? "" : (
				(dtTo.GetStatus() == COleDateTime::DateTimeStatus::null) ? " and later" : (" to " + FormatDateTimeForInterface(dtTo))
				)
			);

		AfxMessageBox(str);

		// We gave the warning, let the caller know the check failed
		return false;
	} else {
		// All good, no conflict, return success
		return true;
	}
}
void CMultiFees::OnInactiveFeeSchedule() 
{
	int nChecked;
	if(m_checkInactive.GetCheck())
		nChecked = -1;
	else
		nChecked = 0;

	try{
		//(e.lally 2007-04-04) PLID 25519 - On the off chance the inactive box was enabled when nothing
			//is selected, just return.
		if(m_pFeeGroups->GetCurSel() == NXDATALISTLib::sriNoRow)
			return;

		// (j.jones 2006-04-10 09:53) - warn if the group has similar combinations as other groups,
		// if we are re-activating
		if(nChecked == 0) {
			// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Include date range; parameterize; single sql round trip
			bool bAllowed = CheckFeeGroupChange(VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 0))
				, CSqlFragment("CONVERT(BIT, 0)") // simulate our becoming non-inactive
				, CSqlFragment() // use the real data value for FROM date
				, CSqlFragment() // use the real data value for TO date
				);

			if (!bAllowed) {
				// There is a duplicate, and the message has already been popped up, so short circuit
				m_checkInactive.SetCheck(!m_checkInactive.GetCheck());
				return;
			}
		}

		//if we made it here, we're okay to change the status
		ExecuteSql("UPDATE MultiFeeGroupsT SET Inactive = %li WHERE ID = %li", nChecked, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal);

		m_pFeeGroups->PutValue(m_pFeeGroups->CurSel, 5, (long)nChecked);

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiMultiFeeInactive, m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0).intVal, CString(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 1).bstrVal), nChecked == 0 ? "Active" : "Inactive", aepMedium, aetCreated);	

		// Network stuff
		m_feegroupsChecker.Refresh();

	}NxCatchAll("Error in OnInactiveFeeSchedule()");	
}

// (j.jones 2007-05-03 11:21) - PLID 25840 - added option to include patient resp. in allowable
void CMultiFees::OnIncludePatrespAllowable() 
{
	if(m_pFeeGroups->CurSel == -1)
		return;

	int nChecked;
	if(m_checkIncludePatRespAllowable.GetCheck())
		nChecked = -1;
	else
		nChecked = 0;

	try{

		ExecuteSql("UPDATE MultiFeeGroupsT SET IncludePatRespAllowable = %li WHERE ID = %li", nChecked, VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0)));

		// (j.jones 2007-05-03 11:37) - PLID 25840 - if both pat. resp. and copay allowable options
		// are checked, give a don't show warning
		TryWarnAllowableBehavior();

	}NxCatchAll("Error in OnIncludePatrespAllowable()");
}

// (j.jones 2007-05-03 11:37) - PLID 25840 - if both pat. resp. and copay allowable options
// are checked, give a don't show warning
void CMultiFees::TryWarnAllowableBehavior()
{
	try{
		
		//warn if both options are checked
		if(m_checkIncludePatRespAllowable.GetCheck() && m_checkIncludeCoPayAllowable.GetCheck()) {
			
			// (j.jones 2010-12-16 15:22) - PLID 41869 - renamed the pat. resp. option
			DontShowMeAgain(this, "If both 'Include CoPay Amount In Allowable' and 'Include Patient / Other Ins. Resps In Allowable' are checked,\n"
							"the larger of the two values will be used in the allowable calculation.\n\n"
							"For example, if the patient responsibility is zero, the copay amount will be used if one exists,\n"
							"but they will not be added together if the copay has been paid.", "WarnMultiFeeAllowableBehavior", "Practice");
		}

	}NxCatchAll("Error in TryWarnAllowableBehavior()");
}

// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
void CMultiFees::OnRadioFeeSchedLocation()
{
	try {

		OnRadioFeeSchedPos();

	}NxCatchAll("Error in CMultiFees::TryWarnAllowableBehavior()");
}

void CMultiFees::OnRadioFeeSchedPos()
{
	try {

		if(m_pFeeGroups->CurSel == -1) {
			return;
		}

		long nID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel,0));

		BOOL bUsePOS = m_radioPOS.GetCheck();
		ExecuteParamSql("UPDATE MultiFeeGroupsT SET UsePOS = {INT} WHERE ID = {INT}", bUsePOS ? 1 : 0, nID);

		//update location labels
		// (d.singleton 2013-05-08 11:28) - PLID 53852 - needs to show inactive locations and providers if they are in the selected list.
		// (d.lange 2015-09-30 11:12) - PLID 67117 - Replaced "Group" with "Fee Schedule"
		if(!bUsePOS) {
			m_nxstaticLocationUnselectedLabel.SetWindowText("Locations Not In Fee Schedule");
			m_nxstaticLocationSelectedLabel.SetWindowText("Locations In Fee Schedule");
			m_pLocYes->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Not Null AND LocationsT.Managed = 1 AND LocationsT.TypeID = 1");
			m_pLocNo->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Null AND LocationsT.Managed = 1 AND LocationsT.Active = 1 AND LocationsT.TypeID = 1");
		}
		else {
			// (d.singleton 2013-05-08 11:28) - PLID 53852 - needs to show inactive locations and providers if they are in the selected list.
			m_nxstaticLocationUnselectedLabel.SetWindowText("Places Of Service Not In Fee Schedule");
			m_nxstaticLocationSelectedLabel.SetWindowText("Places Of Service In Fee Schedule");
			m_pLocYes->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Not Null AND LocationsT.TypeID = 1");
			m_pLocNo->WhereClause = _bstr_t("MultiFeeLocationsT.FeeGroupID Is Null AND LocationsT.Active = 1 AND LocationsT.TypeID = 1");
		}

		//requery the location lists
		m_pLocYes->Requery();
		m_pLocNo->Requery();

	}NxCatchAll("Error in CMultiFees::OnRadioFeeSchedPos()");
}

// (j.jones 2013-04-11 14:40) - PLID 12136 - supported toggling between services & products
void CMultiFees::OnRadioFeeSchedCpt()
{
	try {

		if(m_radioServiceCodes.GetCheck() || !m_bHasInventory) {
			GetDlgItem(IDC_CPTCODES)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_PRODUCTS)->ShowWindow(SW_HIDE);		
			GetDlgItem(IDC_ANESTH_FEE_LABEL)->ShowWindow(SW_SHOW);
		}
		else if(m_radioInventoryItems.GetCheck()) {
			GetDlgItem(IDC_PRODUCTS)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_CPTCODES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ANESTH_FEE_LABEL)->ShowWindow(SW_HIDE);
		}

	}NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnRadioFeeSchedInventory()
{
	try {

		if(!m_bHasInventory) {
			//how was this an option, the controls should have been hidden!
			ASSERT(FALSE);
			m_radioInventoryItems.SetCheck(FALSE);
			m_radioServiceCodes.SetCheck(TRUE);
			OnRadioFeeSchedCpt();
			return;
		}

		if(m_radioInventoryItems.GetCheck()) {
			GetDlgItem(IDC_PRODUCTS)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_CPTCODES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ANESTH_FEE_LABEL)->ShowWindow(SW_HIDE);
		}
		else if(m_radioServiceCodes.GetCheck()) {
			GetDlgItem(IDC_CPTCODES)->ShowWindow(SW_SHOWNOACTIVATE);
			GetDlgItem(IDC_PRODUCTS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ANESTH_FEE_LABEL)->ShowWindow(SW_SHOW);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-11 16:55) - PLID 12136 - supported products
void CMultiFees::OnEditingStartingProducts(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			*pbContinue = FALSE;
			return;
		}

		if(m_pFeeGroups->CurSel == -1) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-11 16:55) - PLID 12136 - supported products
void CMultiFees::OnEditingFinishingProducts(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			*pbContinue = FALSE;
			return;
		}

		if(m_pFeeGroups->CurSel == -1) {
			*pbContinue = FALSE;
			return;
		}

		if(nCol == plcNewFee || nCol == plcAllowable) {
			CString str = strUserEntered;
			str.TrimRight();
			if(str == "") {
				pvarNewValue->vt = VT_NULL;
				return;
			}
			COleCurrency cy = COleCurrency(pvarNewValue->cyVal);
			RoundCurrency(cy);
			pvarNewValue->cyVal = cy;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-04-11 16:55) - PLID 12136 - supported products
void CMultiFees::OnEditingFinishedProducts(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(m_pFeeGroups->CurSel == -1) {
			return;
		}

		if(!bCommit) {
			return;
		}

		if(nCol != plcNewFee && nCol != plcAllowable) {
			return;
		}

		long nGroupID = VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 0), -1);
		CString strGroupName = VarString(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 1), "");
		long nProductID = VarLong(pRow->GetValue(plcID), -1);
		CString strProductName = VarString(pRow->GetValue(plcName), "");

		//for auditing		
		CString strOldAmount = "", strNewAmount = "";
		if(varOldValue.vt == VT_CY) {
			strOldAmount = FormatCurrencyForInterface(VarCurrency(varOldValue), TRUE, TRUE);
		}

		_variant_t varNewPrice = g_cvarNull;
		if(varNewValue.vt == VT_CY) {
			varNewPrice = varNewValue;
			COleCurrency cyNewValue = VarCurrency(varNewPrice,COleCurrency(0,0));
			strNewAmount = FormatCurrencyForInterface(cyNewValue, TRUE, TRUE);
		}

		switch(nCol) {
			case plcNewFee: {
				ExecuteParamSql("IF EXISTS (SELECT ServiceID FROM MultiFeeItemsT WHERE ServiceID = {INT} AND FeeGroupID = {INT})"
					"	UPDATE MultiFeeItemsT SET Price = {VT_CY} WHERE ServiceID = {INT} AND FeeGroupID = {INT} "
					"ELSE "
					"	INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Price) VALUES ({INT}, {INT}, {VT_CY})",
					nProductID, nGroupID,
					varNewPrice, nProductID, nGroupID,
					nGroupID, nProductID, varNewPrice);

				// (d.lange 2015-11-18 14:45) - PLID 67117 - Renamed to use "Fee Schedule" instead of "Group"
				if(strOldAmount != strNewAmount) {
					long nAuditID = BeginNewAuditEvent();			
					CString strOld;
					strOld.Format("%s (Product: %s, Fee Schedule: %s)", strOldAmount.IsEmpty() ? "<None>" : strOldAmount, strProductName, strGroupName);
					CString strNew;
					strNew = strNewAmount.IsEmpty() ? "<None>" : strNewAmount;
					AuditEvent(-1, "", nAuditID, aeiIndivMultiFee, nGroupID, strOld, strNew, aepMedium, aetChanged);
				}

				break;
			}
			case plcAllowable: {
				ExecuteParamSql("IF EXISTS (SELECT ServiceID FROM MultiFeeItemsT WHERE ServiceID = {INT} AND FeeGroupID = {INT})"
					"	UPDATE MultiFeeItemsT SET Allowable = {VT_CY} WHERE ServiceID = {INT} AND FeeGroupID = {INT} "
					"ELSE "
					"	INSERT INTO MultiFeeItemsT (FeeGroupID, ServiceID, Allowable) VALUES ({INT}, {INT}, {VT_CY})",
					nProductID, nGroupID,
					varNewPrice, nProductID, nGroupID,
					nGroupID, nProductID, varNewPrice);

				// (d.lange 2015-11-18 14:45) - PLID 67117 - Renamed to use "Fee Schedule" instead of "Group"
				if(strOldAmount != strNewAmount) {
					long nAuditID = BeginNewAuditEvent();
					CString strOld;
					strOld.Format("%s (Product: %s, Fee Schedule: %s)", strOldAmount.IsEmpty() ? "<None>" : strOldAmount, strProductName, strGroupName);
					CString strNew;
					strNew = strNewAmount.IsEmpty() ? "<None>" : strNewAmount;
					AuditEvent(-1, "", nAuditID, aeiIndivMultiFeeAllowable, nGroupID, strOld, strNew, aepMedium, aetChanged);
				}

				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2015-09-28 11:18) - PLID 67118
void CMultiFees::OnPreviousFeeSchedClicked()
{
	try {
		if (m_pFeeGroups->CurSel > 0) {
			m_pFeeGroups->CurSel = m_pFeeGroups->CurSel - 1;
			OnSelectionChangeFeeGroups(m_pFeeGroups->CurSel);
		}
	} NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnPreviousFeeSchedDoubleClicked()
{
	try {
		OnPreviousFeeSchedClicked();
	} NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnNextFeeSchedClicked()
{
	try {
		if (m_pFeeGroups->CurSel < m_pFeeGroups->GetRowCount() - 1 && m_pFeeGroups->CurSel != -1) {
			m_pFeeGroups->CurSel = m_pFeeGroups->CurSel + 1;
			OnSelectionChangeFeeGroups(m_pFeeGroups->CurSel);
		}
	} NxCatchAll(__FUNCTION__);
}

void CMultiFees::OnNextFeeSchedDoubleClicked()
{
	try {
		OnNextFeeSchedClicked();
	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2015-09-28 11:40) - PLID 67118 - Enable/disable the fee schedule navigation buttons based on the current selection
void CMultiFees::UpdateFeeSchedNavigationButtons()
{
	try{
		if (m_pFeeGroups->CurSel < 1) {
			m_previousFeeSchedBtn.EnableWindow(FALSE);
		}
		else {
			m_previousFeeSchedBtn.EnableWindow(TRUE);
		}

		if (m_pFeeGroups->CurSel == m_pFeeGroups->GetRowCount() - 1) {
			m_nextFeeSchedBtn.EnableWindow(FALSE);
		}
		else {
			m_nextFeeSchedBtn.EnableWindow(TRUE);
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Utility function just to warn if the new 
// date range overlaps with another fee schedule with the same properties. This function 
// was originally meant to be modular and called in advance of allowing the date to 
// change, but that proved inconvenient for the user to deal with so it's now only called 
// from one place: AFTER you have changed the date, so it's just used as a warning.
bool CMultiFees::CheckEffectiveDateChange(const COleDateTime &dtNewEffectiveFromDate, const COleDateTime &dtNewEffectiveToDate)
{
	// Assume everything in data is good EXCEPT the dates. Using the given dates instead, 
	// see if there are any OTHER matching active fee schedules that overlap these dates.
	return CheckFeeGroupChange(VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 0))
		, CSqlFragment() // use the real data value for Inactive
		, CSqlFragment("{VT_DATE}", AsVariant(dtNewEffectiveFromDate)) // simulate new FROM date
		, CSqlFragment("{VT_DATE}", AsVariant(dtNewEffectiveToDate)) // simulate new TO date
		);
}

// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
void CMultiFees::StoreEffectiveDates()
{
	// Make sure the from and to dates are valid; change them on screen if needed. Get the final 
	// date values (or null) depending on the checkboxes and the date controls.
	COleDateTime dtFrom, dtTo;
	if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATES_CHECK) == BST_CHECKED) {
		dtFrom = m_dtEffectiveDateFromDate.GetDateTime();
		if (dtFrom.m_status != COleDateTime::valid || dtFrom < g_cdtSqlMin || dtFrom > g_cdtSqlMax) {
			// dtFrom is bad so change to today
			dtFrom = COleDateTime::GetCurrentTime();
			dtFrom.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
			m_dtEffectiveDateFromDate.SetValue(dtFrom);
			MessageBox("The 'from' date you entered is invalid. It will be set to today's date.", NULL, MB_OK | MB_ICONINFORMATION);
		} else {
			// dtFrom is good
		}

		// The from date can't be after the to date, so set it to the from date if needed
		if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK) == BST_CHECKED) {
			dtTo = m_dtEffectiveDateToDate.GetDateTime();
			if (dtTo.m_status != COleDateTime::valid || dtTo < dtFrom || dtTo > g_cdtSqlMax) {
				// dtTo is bad so change to the same as dtFrom
				dtTo = dtFrom;
				m_dtEffectiveDateToDate.SetValue(dtTo);
				MessageBox("The 'to' date must be the same as or later than the 'from' date. It will be set to the 'from' date.", NULL, MB_OK | MB_ICONINFORMATION);
			} else {
				// dtTo is good
			}
		} else {
			// There is no to date, so set to null so we save that way below
			dtTo = g_cdtNull;
			m_dtEffectiveDateToDate.SetValue(dtTo);
		}
	} else {
		// None are selected, set both to null so we save that way below
		dtFrom = g_cdtNull;
		dtTo = g_cdtNull;
		m_dtEffectiveDateFromDate.SetValue(dtFrom);
		m_dtEffectiveDateToDate.SetValue(dtTo);
	}

	// Update data to reflect what we just got from the controls on screen
	GetAPI()->SetMultiFeeGroupEffectiveDates(GetAPISubkey(), GetAPILoginToken()
		, AsBstr(FormatString("%li", VarLong(m_pFeeGroups->GetValue(m_pFeeGroups->CurSel, 0)))) // Let the exception fly if there's no current selection!
		, NexTech_Accessor::GetNullableDateTime(dtFrom)
		, NexTech_Accessor::GetNullableDateTime(dtTo)
		);

	// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Warn if it now overlaps with another fee schedule
	// All we can do is warn at this point, but at least do that if this change resulted in a duplicate
	CheckEffectiveDateChange(dtFrom, dtTo);
}

// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
void CMultiFees::OnChangeMultiFeeEffectiveDateFromDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// Something changed, so validate and store it
		StoreEffectiveDates();

		// (d.lange 2015-10-05 15:27) - PLID 67122 - Refresh the fee schedule list since the effective from date changed
		UpdateFeeSchedDisplayName();
	} NxCatchAll(__FUNCTION__);

	try {
		*pResult = 0;
	} NxCatchAllIgnore();
}

// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
void CMultiFees::OnChangeMultiFeeEffectiveDateToDate(NMHDR* pNMHDR, LRESULT* pResult)
{
	try {
		// Something changed, so validate and store it
		StoreEffectiveDates();

		// (d.lange 2015-10-05 15:27) - PLID 67122 - Update the Fee Schedule display name to include the effective date or date range
		UpdateFeeSchedDisplayName();
	} NxCatchAll(__FUNCTION__);

	try {
		*pResult = 0;
	} NxCatchAllIgnore();
}

// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
void CMultiFees::UpdateEffectiveDateControls()
{
	// Detect if no multi-fee is selected, and force things to their "unselectable" state
	bool bIsEffectiveDateSelectable;
	if (m_pFeeGroups->GetCurSel() == NXDATALISTLib::sriNoRow) {
		// Nope, not selectable! Update the screen to be unselectable
		CheckDlgButton(IDC_MULTIFEE_EFFECTIVEDATES_CHECK, BST_UNCHECKED);
		CheckDlgButton(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK, BST_UNCHECKED);
		bIsEffectiveDateSelectable = false;
	} else {
		// Yep, trust what's on screen
		bIsEffectiveDateSelectable = true;
	}

	// Figure out the state of things on screen
	bool bHasEffectiveFromDate = (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATES_CHECK) == BST_CHECKED);
	bool bHasEffectiveFromDateAndToDate = (bHasEffectiveFromDate && (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK) == BST_CHECKED));

	// Enable/disable the controls as appropiate
	GetDlgItem(IDC_MULTIFEE_EFFECTIVEDATES_CHECK)->EnableWindow(bIsEffectiveDateSelectable ? TRUE : FALSE);
	GetDlgItem(IDC_MULTIFEE_EFFECTIVEDATE_FROM_LABEL)->EnableWindow(bHasEffectiveFromDate ? TRUE : FALSE);
	GetDlgItem(IDC_MULTIFEE_EFFECTIVEDATE_FROM_DATE)->EnableWindow(bHasEffectiveFromDate ? TRUE : FALSE);
	GetDlgItem(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK)->EnableWindow(bHasEffectiveFromDate ? TRUE : FALSE);
	GetDlgItem(IDC_MULTIFEE_EFFECTIVEDATE_TO_DATE)->EnableWindow(bHasEffectiveFromDateAndToDate ? TRUE : FALSE);

	// Set the date controls' formats to either DEFAULT (null) or BLANK (''), the latter being if 
	// the control is meant to have no value at all so as not to confuse the user with random dates.
	m_dtEffectiveDateFromDate.SetFormat(bHasEffectiveFromDate ? NULL : "''");
	m_dtEffectiveDateToDate.SetFormat(bHasEffectiveFromDateAndToDate ? NULL : "''");
}

// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Quick way to get the current FROM date from screen
COleDateTime CMultiFees::GetEffectiveFromDate()
{
	if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATES_CHECK) == BST_CHECKED) {
		COleDateTime dtFrom = m_dtEffectiveDateFromDate.GetDateTime();
		if (dtFrom.m_status != COleDateTime::valid || dtFrom < g_cdtSqlMin || dtFrom > g_cdtSqlMax) {
			// dtFrom is bad so change to today
			dtFrom = COleDateTime::GetCurrentTime();
			dtFrom.SetDate(dtFrom.GetYear(), dtFrom.GetMonth(), dtFrom.GetDay());
		} else {
			// dtFrom is good
		}
		return dtFrom;
	} else {
		// Not checked, so null
		return g_cdtNull;
	}
}

// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Quick way to get the current TO date from screen
COleDateTime CMultiFees::GetEffectiveToDate()
{
	COleDateTime dtFrom = GetEffectiveFromDate();
	if (dtFrom.m_status == COleDateTime::valid) {
		// We have a from date, so there might be a to date
		if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK) == BST_CHECKED) {
			// Yep, there's a to date, get it
			COleDateTime dtTo = m_dtEffectiveDateToDate.GetDateTime();
			if (dtTo.m_status != COleDateTime::valid || dtTo < dtFrom || dtTo > g_cdtSqlMax) {
				// dtTo is bad so by convention we use the same as dtFrom
				return dtFrom;
			} else {
				// dtTo is good
				return dtTo;
			}
		} else {
			// There is no to date, so null
			return g_cdtNull;
		}
	} else {
		// None are selected, so null
		return g_cdtNull;
	}
}

// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
void CMultiFees::OnMultiFeeEffectiveDatesCheck()
{
	try {
		if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATES_CHECK) == BST_CHECKED) {
			// The user has just checked effective dates, initialize to 'today'.
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			m_dtEffectiveDateFromDate.SetValue(dtToday);

		} else {
			// The user just unchecked the effective dates, so clear the FROM date
			m_dtEffectiveDateFromDate.SetValue(g_cdtNull);
		}

		// Either way, the TO checkbox should be unchecked if it isn't already, and ensure the TO 
		// date is cleared out.
		CheckDlgButton(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK, BST_UNCHECKED);
		m_dtEffectiveDateToDate.SetValue(g_cdtNull);

		// Enable the related controls as appropriate
		UpdateEffectiveDateControls();

		// Something changed, so validate and store it
		StoreEffectiveDates();

		// (d.lange 2015-10-05 15:27) - PLID 67122 - Update the Fee Schedule display name to include the effective date or date range
		UpdateFeeSchedDisplayName();
	} NxCatchAll(__FUNCTION__);
}

// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
void CMultiFees::OnMultiFeeEffectiveDateToCheck()
{
	try {
		if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK) == BST_CHECKED) {
			// It only makes sense (and should only be possible) to check the TO box if the overall 
			// checkbox is already checked.
			ASSERT(IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATES_CHECK) == BST_CHECKED);

			// The user just checked the TO checbox, so initialize the date to match the FROM date.
			m_dtEffectiveDateToDate.SetValue(m_dtEffectiveDateFromDate.GetDateTime());
		} else {
			// The user just unschecked the TO checkbox, so clear the TO date.
			m_dtEffectiveDateToDate.SetValue(g_cdtNull);
		}

		// Enable the related controls as appropriate
		UpdateEffectiveDateControls();

		// Something changed, so validate and store it
		StoreEffectiveDates();

		// (d.lange 2015-10-05 15:27) - PLID 67122 - Update the Fee Schedule display name to include the effective date or date range
		UpdateFeeSchedDisplayName();
	} NxCatchAll(__FUNCTION__);
}

// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
void CMultiFees::OnHideInactiveCheck()
{
	try {
		RequeryFeeGroups();
	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2015-10-06 09:09) - PLID 67122 - Updates the Fee Schedule display name to include the effective date or date range
void CMultiFees::UpdateFeeSchedDisplayName()
{
	long nCurSel = m_pFeeGroups->GetCurSel();
	CString strFeeSchedName = m_pFeeGroups->GetValue(nCurSel, 1);
	CString strFeeSchedDisplayName = strFeeSchedName;
	CString strEffectiveFromDate, strEffectiveToDate;
	_variant_t varEffectiveFromDate, varEffectiveToDate;

	if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATES_CHECK) == BST_CHECKED) {
		COleDateTime dtEffectiveFromDate = m_dtEffectiveDateFromDate.GetValue();
		if (dtEffectiveFromDate.GetStatus() != COleDateTime::invalid) {
			// Format the display name to include the effective from date
			strEffectiveFromDate = FormatDateTimeForInterface(dtEffectiveFromDate, NULL, dtoDate);
			strFeeSchedDisplayName.Format("%s (%s)", strFeeSchedName, strEffectiveFromDate);

			varEffectiveFromDate = _variant_t(dtEffectiveFromDate, VT_DATE);

			if (IsDlgButtonChecked(IDC_MULTIFEE_EFFECTIVEDATE_TO_CHECK) == BST_CHECKED) {
				COleDateTime dtEffectiveToDate = m_dtEffectiveDateToDate.GetValue();
				if (dtEffectiveToDate != COleDateTime::invalid) {
					// Format the display name to include the effective date range
					strEffectiveToDate = FormatDateTimeForInterface(dtEffectiveToDate, NULL, dtoDate);
					strFeeSchedDisplayName.Format("%s (%s - %s)", strFeeSchedName, strEffectiveFromDate, strEffectiveToDate);

					varEffectiveToDate = _variant_t(dtEffectiveToDate, VT_DATE);
				}
				else {
					// At this point, if the effective to date is invalid for any reason only display the effective from date
				}
			}
		}
	}

	// Manually update the fee schedule display name
	m_pFeeGroups->PutValue(nCurSel, 2, AsVariant(strFeeSchedDisplayName));

	// Manually update the effective from date column
	m_pFeeGroups->PutValue(nCurSel, 3, varEffectiveFromDate);

	// Manually update the effective to date column
	m_pFeeGroups->PutValue(nCurSel, 4, varEffectiveToDate);
}
