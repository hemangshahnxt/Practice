// (c.haag 2011-06-17) - PLID 36477 - This class encapsulates the population of a pop-up menu pertaining
// to an appointment in the scheduler, room manager and appointments tab of the patients module. In a perfect
// world this would be inherited from a CMenu and invoked from all three places; but due to their diverse needs
// and the time that would be necessary to refactor all the code, this class will only have part of the functionality
// needed between the three.
//
// The commands added to the menu are actually ID's from AppointmentsDlg.h, SchedulerRc.h, and special
// sentinel values (like MIN_ID_SUPERBILL_TEMPLATES) that have large number ranges, and miMarkAsShowStateStatus.
// The numeric menu values are like:
//
// Less than negative 1000: miMarkAsShowStateStatus
// 10000 - 19999: Superbill Templates
// 20000 - 29999: Co-pay resps
// > 30000: wParams used in WM_COMMAND messages

#include <stdafx.h>
#include "ApptPopupMenuManager.h"
#include "AppointmentsDlg.h"
#include "GlobalSchedUtils.h"
#include "Superbill.h"
#include "InvUtils.h"
#include "SharedInsuranceUtils.h"
#include "GlobalFinancialUtils.h"
#include <NxPracticeSharedLib\SharedScheduleUtils.h>

const int MIN_ID_SUPERBILL_TEMPLATES = 10000;
const int MIN_ID_COPAYMENT_RESPS = 20000;

using namespace ADODB;

CApptPopupMenuManager::CApptPopupMenuManager(CMenu& mnu, long& nPrimaryInsuredPartyID, CArray<long, long>& aryCopayInsuredPartyIDs, 
											 CWnd* pParent, long nApptID, long nPatientID) :
m_mnu(mnu),
m_nPrimaryInsuredPartyID(nPrimaryInsuredPartyID),
m_aryCopayInsuredPartyIDs(aryCopayInsuredPartyIDs),
m_pParent(pParent),
m_nApptID(nApptID),
m_nPatientID(nPatientID),
m_bPromptIfSuperbillEmpty(false)
{	
}

// This function adds confirmation-related options to an appointment pop-up menu. This is utilized in the scheduler,
// room manager, and appointments tab.
void CApptPopupMenuManager::FillConfirmedOptions(long nConfirmed)
{	
	if(nConfirmed == acsUnconfirmed){
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_CONFIRMED, "Mark as Con&firmed");
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_LEFTMESSAGE, "Mark as &Left Message");
	}
	else if(nConfirmed == acsConfirmed){
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_UNCONFIRMED, "Mark as &Unconfirmed");
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_LEFTMESSAGE, "Mark as &Left Message");
	}
	else{
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_CONFIRMED, "Mark as Con&firmed");
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_UNCONFIRMED, "Mark as &Unconfirmed");
	}
}

// (c.haag 2011-06-23) - PLID 44286 - This function adds confirmation-related options to an appointment pop-up menu.
// This is utilized in the scheduler and room manager.
void CApptPopupMenuManager::FillSuperbillOptions()
{
	// The scheduler will not show these options for patientless appointments; so neither should we
	if (-25 == m_nPatientID) {
		return;
	}

	m_mnu.AppendMenu(MF_SEPARATOR);
	m_mnu.AppendMenu(MF_ENABLED, ID_PRINTSUPERBILL, "Print &Superbill");

	GetDefaultSuperbillTemplates(m_nApptID, &m_arySuperbillTemplates, m_bPromptIfSuperbillEmpty, eFmtSuperbillPath);

	long nTemplateCount = m_arySuperbillTemplates.GetSize();
	if(nTemplateCount > 0) {
		//There are some templates to be used.
		if(nTemplateCount == 1) {
			// If there is just 1, leave the menu item as-is, "Print Superbill".
			//	The user will click that and it can use the right superbill.
			//Nothing further to do here.
		}
		else {
			//There are many.  Give them a list in a pop-out menu so they can pick.

			//Remove the old Print Superbills menu

			//Create a submenu for our options			
			m_mnuSuperbills.CreateMenu();

			//Append the templates.  We'll give them IDs of 10000 onward
			for(int i = 0; i < m_arySuperbillTemplates.GetSize(); i++) {
				m_mnuSuperbills.AppendMenu(MF_ENABLED|MF_BYCOMMAND, (MIN_ID_SUPERBILL_TEMPLATES + i) , m_arySuperbillTemplates.GetAt(i));
			}

			//Add a new menu item to access the submenu.  
			m_mnu.InsertMenu(ID_PRINTSUPERBILL, MF_ENABLED|MF_POPUP, (UINT)m_mnuSuperbills.m_hMenu, "Print Superbill");
			//Remove the other one we'd added earlier as a placeholder.
			m_mnu.RemoveMenu(ID_PRINTSUPERBILL, MF_BYCOMMAND);
		}
	}
	else if(m_bPromptIfSuperbillEmpty) {
		//There are no templates specified, the user has been asked to be prompted to pick one manually.
		//	Again, this will use the default menu option of "print superbill", no further work to do here.
	}
	else {
		//If this happens, there are no superbills possible to use, and there is no global default.  I'm
		//	leaving the option enabled, so the user will get warned about it when they try to print.  
		//	It's too hard to figure out why if we just disable.
	}
}

// (c.haag 2011-06-23) - PLID 44287 - This function adds inventory-related options to an appointment pop-up menu. This is utilized in the scheduler,
// room manager, and appointments tab. Returns TRUE if items were added to the menu.
BOOL CApptPopupMenuManager::FillInventoryOptions()
{
	// The scheduler will not show these options for patientless appointments; so neither should we
	if (-25 == m_nPatientID) {
		return FALSE;
	}

	BOOL bAddedSeparator = FALSE;
	// (a.walling 2008-03-21 12:43) - PLID 28946 - Check for adv inventory licensing
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(g_pLicense->HasCandAModule(CLicense::cflrSilent) && g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) 
	{
		// (j.gruber 2008-09-10 15:23) - PLID 30282 add an edit if there is already an allocation attached
		// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
		if (ReturnsRecordsParam(FormatString("SELECT ID FROM PatientInvAllocationsT WHERE AppointmentID = {INT} AND Status != %li ", InvUtils::iasDeleted), m_nApptID))  {

			if (GetCurrentUserPermissions(bioInventoryAllocation) & SPT___W________ANDPASS) {	
				
				m_mnu.InsertMenu(-1, MF_BYPOSITION);
				bAddedSeparator = TRUE;
				m_mnu.InsertMenu(-1, MF_BYPOSITION, ID_APPT_EDIT_INV_ALLOCATION, "Edit &Inventory Allocation");
			}
		}
		else if ((GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS)) {																			  
			m_mnu.InsertMenu(-1, MF_BYPOSITION);
			bAddedSeparator = TRUE;
			m_mnu.InsertMenu(-1, MF_BYPOSITION, ID_APPT_NEW_INV_ALLOCATION, "Create &Inventory Allocation");
		}
	}

	// (j.jones 2008-03-18 14:23) - PLID 29309 - add the ability to create an order,
	// provided the have the license and potential permission to do so
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(g_pLicense->HasCandAModule(CLicense::cflrSilent)
		&& (GetCurrentUserPermissions(bioInvOrder) & SPT____C_______ANDPASS)) {
	
		if(!bAddedSeparator) {
			m_mnu.AppendMenu(MF_SEPARATOR);
			bAddedSeparator = TRUE;
		}
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_NEW_INV_ORDER, "Create Inventory &Order");
	}

	return bAddedSeparator;
}

// (c.haag 2011-06-24) - PLID 44317 - This function adds billing-related items to an appointment pop-up menu. This is utilized
// in the scheduler, room manager, and appointments tab in the patients module. Returns TRUE if items were added to the menu.
BOOL CApptPopupMenuManager::FillBillingOptions(BOOL bAddedSeparator)
{
	if (-25 == m_nPatientID) {
		return bAddedSeparator;
	}

	// (j.jones 2008-06-23 16:20) - PLID 30455 - added the ability to create a bill
	if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)
		&& (GetCurrentUserPermissions(bioBill) & SPT____C_______ANDPASS)) {
	
		if(!bAddedSeparator) {
			m_mnu.AppendMenu(MF_SEPARATOR);
			bAddedSeparator = TRUE;
		}
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_NEW_BILL, "Create &Bill");
	}

	// (z.manning 2015-07-22 14:01) - PLID 67241 - Add an option to open the payment profile dialog
	if (IsICCPEnabled())
	{
		if(!bAddedSeparator) {
			m_mnu.AppendMenu(MF_SEPARATOR);
			bAddedSeparator = TRUE;
		}
		m_mnu.AppendMenu(MF_ENABLED, ID_APPT_MANAGE_PAYMENT_PROFILES, "Manage Pa&yment Profiles");
	}

	// (j.jones 2010-09-24 11:45) - PLID 34518 - added ability to create a copayment
	if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)
		&& (GetCurrentUserPermissions(bioPayment) & SPT____C_______ANDPASS)) {

		//load up all resps. they have
		_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID AS InsuredPartyID, "
			"InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName, "
			"RespTypeT.CategoryType, RespTypeT.Priority "
			"FROM InsuredPartyT "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE RespTypeT.Priority <> -1 AND InsuredPartyT.PatientID = {INT} "
			"ORDER BY RespTypeT.Priority", m_nPatientID);

		BOOL bNeedsSubMenu = FALSE;
		BOOL bHasCreatedSubMenu = FALSE;
		CMenu mnuCopays;

		while(!rs->eof) {

			long nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID");
			CString strInsCoName = AdoFldString(rs, "InsCoName", "");
			CString strRespTypeName = AdoFldString(rs, "TypeName", "");
			RespCategoryType rctCategory = (RespCategoryType)AdoFldLong(rs, "CategoryType", (long)rctMedical);
			long nPriority = AdoFldLong(rs, "Priority");

			//if they have a primary medical company, add a dedicated option to create a copayment for it
			if(nPriority == 1 && rctCategory == rctMedical) {

				//track this ID for later
				m_nPrimaryInsuredPartyID = nInsuredPartyID;

				if(!bAddedSeparator) {
					m_mnu.AppendMenu(MF_SEPARATOR);
					bAddedSeparator = TRUE;
				}

				m_mnu.AppendMenu(MF_ENABLED, ID_APPT_NEW_PRIMARY_COPAY, "Collect &Primary Co-Payment");
			}
			else {
				//we don't need to display the sub menu unless a non-primary company exists
				bNeedsSubMenu = TRUE;
			}
			
			//in all cases, append to our submenu, even if primary
			if(!bHasCreatedSubMenu) {
				mnuCopays.CreateMenu();
				bHasCreatedSubMenu = TRUE;
			}

			CString strLabel;
			strLabel.Format("%s (%s)", strInsCoName, strRespTypeName);

			//track the insured party ID, and add its array index to MIN_ID_COPAYMENT_RESPS, that's the menu ID
			m_aryCopayInsuredPartyIDs.Add(nInsuredPartyID);
			mnuCopays.AppendMenu(MF_ENABLED|MF_BYCOMMAND, MIN_ID_COPAYMENT_RESPS + (m_aryCopayInsuredPartyIDs.GetSize() - 1), strLabel);

			rs->MoveNext();
		}
		rs->Close();

		if(bNeedsSubMenu && bHasCreatedSubMenu) {
			//we created a sub menu, it has more than primary in it,
			//so we need to actually display it
			
			if(!bAddedSeparator) {
				m_mnu.AppendMenu(MF_SEPARATOR);
				bAddedSeparator = TRUE;
			}

			m_mnu.AppendMenu(MF_ENABLED|MF_POPUP, (UINT)mnuCopays.m_hMenu, "Collect &Co-Payment For...");
		}
	}

	return bAddedSeparator;
}

// (c.haag 2011-06-24) - PLID 44319 - This function adds e-eligibility-related items to an appointment pop-up menu. This is utilized
// in the scheduler, room manager, and appointments tab in the patients module. Returns TRUE if items were added to the menu.
BOOL CApptPopupMenuManager::FillEEligibilityOptions(BOOL bAddedSeparator)
{
	if (-25 == m_nPatientID) {
		return bAddedSeparator;
	}

	// (j.jones 2010-09-27 11:25) - PLID 38447 - added ability to view e-eligibility responses,
	// we have an eligibility license but the permissions are e-billing permissions
	// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
	if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)
		&& (GetCurrentUserPermissions(bioEEligibility) & (sptRead|sptReadWithPass))) {
	
		if(!bAddedSeparator) {
			m_mnu.InsertMenu(-1, MF_BYPOSITION);
			bAddedSeparator = TRUE;
		}
		m_mnu.InsertMenu(-1, MF_BYPOSITION, ID_APPT_VIEW_ELIGIBILITY_RESPONSES, "&View E-Eligibility Responses");
	}
	return bAddedSeparator;
}

// (c.haag 2011-06-24) - PLID 44319 - This function adds case history-related items to an appointent pop-up menu. This is utilized
// in the scheduler, room manager, and appointments tab in the patients module. Returns TRUE if items were added to the menu.
BOOL CApptPopupMenuManager::FillCaseHistoryOptions(BOOL bAddedSeparator, long nAptTypeCategory)
{
	if (-25 == m_nPatientID) {
		return bAddedSeparator;
	}

	// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options, for surgeries only
	if(IsSurgeryCenter(false) && nAptTypeCategory == PhaseTracking::AC_SURGERY) {
		if(!bAddedSeparator) {
			m_mnu.AppendMenu(MF_SEPARATOR);
			bAddedSeparator = TRUE;
		}
		_RecordsetPtr rsCase = CreateParamRecordset("SELECT TOP 1 ID FROM CaseHistoryT WHERE AppointmentID = {INT}", m_nApptID);
		if(rsCase->eof) {
			m_mnu.AppendMenu(MF_ENABLED, ID_APPT_CREATE_CASE_HISTORY, "Create Case &History");
		}
		else {
			m_mnu.AppendMenu(MF_ENABLED, ID_APPT_EDIT_CASE_HISTORY, "Edit Case &History");
		}
		rsCase->Close();
	}

	return bAddedSeparator;
}

// This should be called to handle pop-up menu results that aren't handled from outside this object. Returns TRUE
// if we did anything, FALSE if not.
BOOL CApptPopupMenuManager::HandlePopupMenuResult(int nCmdID)
{
	if (ID_APPT_CONFIRMED == nCmdID)
	{
		if (HasPermissionForResource(m_nApptID,sptWrite,sptWriteWithPass)) {
			AppointmentMarkConfirmed(m_nApptID, acsConfirmed);
		}
	}
	else if (ID_APPT_UNCONFIRMED == nCmdID)
	{
		if (HasPermissionForResource(m_nApptID,sptWrite,sptWriteWithPass)) {
			AppointmentMarkConfirmed(m_nApptID, acsUnconfirmed);
		}
	}
	else if (ID_APPT_LEFTMESSAGE == nCmdID)
	{
		if(HasPermissionForResource(m_nApptID,sptWrite,sptWriteWithPass)) {
			AppointmentMarkConfirmed(m_nApptID, acsLeftMessage);
		}
	}
	// (c.haag 2011-06-23) - PLID 44286 - Handle Superbill selections
	else if ((nCmdID >= MIN_ID_SUPERBILL_TEMPLATES && nCmdID < MIN_ID_SUPERBILL_TEMPLATES + m_arySuperbillTemplates.GetSize())
		|| nCmdID == ID_PRINTSUPERBILL) 
	{
		//
		// This code is copied from NxSchedulerDlg.
		//
		BOOL bForcePrompt = 0;

		//Get the path.  If our selection is "print superbill", then there is only 1 option in the list, grab the first.
		CString strPath;
		if(nCmdID == ID_PRINTSUPERBILL) {
			//First, we have to check for validity of the array.  It is possible that nothing is in the array and we want to 
			//	prompt the user.
			if(m_arySuperbillTemplates.GetSize() >= 1) {
				strPath = m_arySuperbillTemplates.GetAt(0);
			}
			else if(m_bPromptIfSuperbillEmpty) {
				bForcePrompt = 1;
				strPath = "";
			}
			else {
				//We have no templates but have not been asked to prompt.  This probably means they're on "default", but don't 
				//	actually have a default set.  Nothing we can do, leave it empty.
				strPath = "";
			}
		}
		else {
			//We have many, grab the right one by subtracting the selected ID minus the base ID.  That will
			//	give us the array element.
			strPath = m_arySuperbillTemplates.GetAt(nCmdID - MIN_ID_SUPERBILL_TEMPLATES);
		}
		//Remember that we decided for display purposes to show only the "superbill path", so prepend the shared path and templates\forms 
		//	so we get all the way there.
		if(!strPath.IsEmpty()) {
			strPath = GetSharedPath() ^ "Templates\\Forms" ^ strPath;
		}

		//
		// ...and this code comes from CSchedulerView::OnPrintSuperbill
		//
		if(strPath.IsEmpty() && bForcePrompt) 
		{
			//Yep, we need to prompt the user
			CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Microsoft Word Templates|*.dot;*.dotx;*.dotm||", m_pParent);
			CString strInitialDir = GetSharedPath() ^ "Templates\\Forms";
			dlg.m_ofn.lpstrInitialDir = strInitialDir;
			int nResult = dlg.DoModal();
			if(nResult == IDOK) {
				strPath = dlg.GetPathName();
			}
			else {
				//cancelled, just abandon
				return TRUE;
			}
		}
		SuperbillByAppt(m_nApptID, strPath);
	}
	// (c.haag 2011-06-23) - PLID 44287
	else if (ID_APPT_EDIT_INV_ALLOCATION == nCmdID)
	{
		AppointmentEditInvAllocation(m_pParent, m_nApptID, FALSE);
	}
	else if (ID_APPT_NEW_INV_ALLOCATION == nCmdID)
	{
		AppointmentCreateInvAllocation(m_pParent, m_nApptID, FALSE);
	}
	else if (ID_APPT_NEW_INV_ORDER == nCmdID)
	{
		AppointmentCreateInvOrder(m_pParent, m_nApptID, FALSE);
	}
	// (c.haag 2011-06-24) - PLID 44317
	else if (ID_APPT_NEW_BILL == nCmdID)
	{
		AppointmentCreateNewBill(m_pParent, m_nApptID, FALSE);
	}
	// (z.manning 2015-07-22 15:10) - PLID 67241 - Open the payment profile dialog
	else if (nCmdID == ID_APPT_MANAGE_PAYMENT_PROFILES)
	{
		OpenPaymentProfileDlg(m_nPatientID, m_pParent);
	}
	// (c.haag 2011-06-24) - PLID 44317
	else if((nCmdID >= MIN_ID_COPAYMENT_RESPS && nCmdID < MIN_ID_COPAYMENT_RESPS + m_aryCopayInsuredPartyIDs.GetSize())
		|| (nCmdID == ID_APPT_NEW_PRIMARY_COPAY && m_nPrimaryInsuredPartyID != -1)) 
	{
		long nInsuredPartyID = -1;
		if(nCmdID == ID_APPT_NEW_PRIMARY_COPAY && m_nPrimaryInsuredPartyID != -1) {
			nInsuredPartyID = m_nPrimaryInsuredPartyID;
		}
		else if(nCmdID >= MIN_ID_COPAYMENT_RESPS && nCmdID < MIN_ID_COPAYMENT_RESPS + m_aryCopayInsuredPartyIDs.GetSize()) {
			nInsuredPartyID = m_aryCopayInsuredPartyIDs.GetAt(nCmdID - MIN_ID_COPAYMENT_RESPS);
		}

		if(nInsuredPartyID == -1) {
			//should be impossible
			ThrowNxException("Copay creation failed - no insured party selected!");
		}

		PromptForCopay(nInsuredPartyID);
	}
	// (c.haag 2011-06-24) - PLID 44319
	else if (ID_APPT_VIEW_ELIGIBILITY_RESPONSES == nCmdID)
	{
		ShowAllEligibilityRequestsForInsuredParty_ByPatientOrAppt(NULL, m_nPatientID, m_nApptID);
	}
	// (c.haag 2011-06-23) - PLID 44319
	else if (ID_APPT_CREATE_CASE_HISTORY == nCmdID)
	{
		AppointmentCreateCaseHistory(m_nApptID, m_nPatientID);
	}
	// (c.haag 2011-06-23) - PLID 44319
	else if (ID_APPT_EDIT_CASE_HISTORY == nCmdID)
	{
		AppointmentEditCaseHistory(m_nApptID, m_nPatientID);
	}
	else {
		// Didn't handle anything; return FALSE
		return FALSE;
	}
	return TRUE;
}
