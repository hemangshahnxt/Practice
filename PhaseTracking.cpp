#include "stdafx.h"
#include "PhaseTracking.h"
#include "GlobalDataUtils.h"
#include "marketUtils.h"
#include "EnterActiveDateDlg.h"
#include "SelectTemplateDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "PatientView.h"
#include "PhaseRecordPickerDlg.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"
#include "SelectDlg.h"
#include "MsgBox.h"
#include "TodoUtils.h"
#include "AddProcedureDlg.h"
#include "DecisionRuleUtils.h"
#include "NxAPI.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define XOR(a,b) (a && !b || !a && b)

namespace PhaseTracking
{
	// (c.haag 2013-11-27) - PLID 59835 - Converts a Practice phase action enumeration to an API one
	bool ToAPIPhaseAction(PhaseTracking::PhaseAction action, OUT NexTech_Accessor::PhaseAction& result)
	{
		switch (action)
		{
		case PA_Manual:
			result = NexTech_Accessor::PhaseAction_Manual;
			break;
		case PA_QuoteSurgery:
			result = NexTech_Accessor::PhaseAction_QuoteSurgery;
			break;
		case PA_Quote:
			result = NexTech_Accessor::PhaseAction_Quote;
			break;
		case PA_QuoteCPT:
			result = NexTech_Accessor::PhaseAction_QuoteCPT;
			break;
		case PA_QuoteInventory:
			result = NexTech_Accessor::PhaseAction_QuoteInventory;
			break;
		case PA_Bill:
			result = NexTech_Accessor::PhaseAction_Bill;
			break;
		case PA_BillSurgery:
			result = NexTech_Accessor::PhaseAction_BillSurgery;
			break;
		case PA_BillCPT:
			result = NexTech_Accessor::PhaseAction_BillCPT;
			break;
		case PA_BillInventory:
			result = NexTech_Accessor::PhaseAction_BillInventory;
			break;
		case PA_BillQuote:
			result = NexTech_Accessor::PhaseAction_BillQuote;
			break;
		case PA_WriteTemplate:
			result = NexTech_Accessor::PhaseAction_WriteTemplate;
			break;
		case PA_WritePacket:
			result = NexTech_Accessor::PhaseAction_WritePacket;
			break;
		case PA_ScheduleAptCategory:
			result = NexTech_Accessor::PhaseAction_ScheduleAptCategory;
			break;
		case PA_ScheduleAptType:
			result = NexTech_Accessor::PhaseAction_ScheduleAptType;
			break;
		case PA_ActualAptCategory:
			result = NexTech_Accessor::PhaseAction_ActualAptCategory;
			break;
		case PA_ActualAptType:
			result = NexTech_Accessor::PhaseAction_ActualAptType;
			break;
		case PA_Ladder:
			result = NexTech_Accessor::PhaseAction_Ladder;
			break;
		case PA_Payment:
			result = NexTech_Accessor::PhaseAction_Payment;
			break;
		case PA_CreateEMRByCollection:
			result = NexTech_Accessor::PhaseAction_CreateEMRByCollection;
			break;
		case PA_CreateEMRByTemplate:
			result = NexTech_Accessor::PhaseAction_CreateEMRByTemplate;
			break;
		default:
			return false;
		}
		return true;
	}

	// (c.haag 2013-11-27) - PLID 59822 - Converts a Practice phase action enumeration to an API one
	bool ToPracticePhaseAction(NexTech_Accessor::PhaseAction action, OUT PhaseTracking::PhaseAction& result)
	{
		switch (action)
		{
		case NexTech_Accessor::PhaseAction_Manual:
			result = PA_Manual;
			break;
		case NexTech_Accessor::PhaseAction_QuoteSurgery:
			result = PA_QuoteSurgery;
			break;
		case NexTech_Accessor::PhaseAction_Quote:
			result = PA_Quote;
			break;
		case NexTech_Accessor::PhaseAction_QuoteCPT:
			result = PA_QuoteCPT;
			break;
		case NexTech_Accessor::PhaseAction_QuoteInventory:
			result = PA_QuoteInventory;
			break;
		case NexTech_Accessor::PhaseAction_Bill:
			result = PA_Bill;
			break;
		case NexTech_Accessor::PhaseAction_BillSurgery:
			result = PA_BillSurgery;
			break;
		case NexTech_Accessor::PhaseAction_BillCPT:
			result = PA_BillCPT;
			break;
		case NexTech_Accessor::PhaseAction_BillInventory:
			result = PA_BillInventory;
			break;
		case NexTech_Accessor::PhaseAction_BillQuote:
			result = PA_BillQuote;
			break;
		case NexTech_Accessor::PhaseAction_WriteTemplate:
			result = PA_WriteTemplate;
			break;
		case NexTech_Accessor::PhaseAction_WritePacket:
			result = PA_WritePacket;
			break;
		case NexTech_Accessor::PhaseAction_ScheduleAptCategory:
			result = PA_ScheduleAptCategory;
			break;
		case NexTech_Accessor::PhaseAction_ScheduleAptType:
			result = PA_ScheduleAptType;
			break;
		case NexTech_Accessor::PhaseAction_ActualAptCategory:
			result = PA_ActualAptCategory;
			break;
		case NexTech_Accessor::PhaseAction_ActualAptType:
			result = PA_ActualAptType;
			break;
		case NexTech_Accessor::PhaseAction_Ladder:
			result = PA_Ladder;
			break;
		case NexTech_Accessor::PhaseAction_Payment:
			result = PA_Payment;
			break;
		case NexTech_Accessor::PhaseAction_CreateEMRByCollection:
			result = PA_CreateEMRByCollection;
			break;
		case NexTech_Accessor::PhaseAction_CreateEMRByTemplate:
			result = PA_CreateEMRByTemplate;
			break;
		default:
			return false;
		}
		return true;
	}

	// (c.haag 2013-11-27) - PLID 59835 - Converts an API event type enumeration to a Practice one 
	bool ToPracticeEventType(NexTech_Accessor::EventType eventType, OUT PhaseTracking::EventType& result)
	{
		switch (eventType)
		{
		case NexTech_Accessor::EventType_Invalid:
			result = ET_Invalid;
			break;
		case NexTech_Accessor::EventType_BillCreated:
			result = ET_BillCreated;
			break;
		case NexTech_Accessor::EventType_MarkedDone:
			result = ET_MarkedDone;
			break;
		case NexTech_Accessor::EventType_QuoteCreated:
			result = ET_QuoteCreated;
			break;
		case NexTech_Accessor::EventType_AppointmentCreated:
			result = ET_AppointmentCreated;
			break;
		case NexTech_Accessor::EventType_TemplateSent:
			result = ET_TemplateSent;
			break;
		case NexTech_Accessor::EventType_ActualAppointment:
			result = ET_ActualAppointment;
			break;
		case NexTech_Accessor::EventType_PacketSent:
			result = ET_PacketSent;
			break;
		case NexTech_Accessor::EventType_MarkedDoneFromTodo:
			result = ET_MarkedDoneFromTodo;
			break;
		case NexTech_Accessor::EventType_Skipped:
			result = ET_Skipped;
			break;
		case NexTech_Accessor::EventType_LadderCreated:
			result = ET_LadderCreated;
			break;
		case NexTech_Accessor::EventType_PaymentApplied:
			result = ET_PaymentApplied;
			break;
		case NexTech_Accessor::EventType_EMRCreated:
			result = ET_EMRCreated;
			break;
		default:
			return false;
		}
		return true;
	}

	// (c.haag 2013-11-26) - PLID 59832 - Converts a Practice event type enumeration to an API one 
	bool ToAPIEventType(PhaseTracking::EventType eventType, OUT NexTech_Accessor::EventType& result)
	{
		switch (eventType)
		{
		case ET_Invalid:
			result = NexTech_Accessor::EventType_Invalid;
			break;
		case ET_BillCreated:
			result = NexTech_Accessor::EventType_BillCreated;
			break;
		case ET_MarkedDone:
			result = NexTech_Accessor::EventType_MarkedDone;
			break;
		case ET_QuoteCreated:
			result = NexTech_Accessor::EventType_QuoteCreated;
			break;
		case ET_AppointmentCreated:
			result = NexTech_Accessor::EventType_AppointmentCreated;
			break;
		case ET_TemplateSent:
			result = NexTech_Accessor::EventType_TemplateSent;
			break;
		case ET_ActualAppointment:
			result = NexTech_Accessor::EventType_ActualAppointment;
			break;
		case ET_PacketSent:
			result = NexTech_Accessor::EventType_PacketSent;
			break;
		case ET_MarkedDoneFromTodo:
			result = NexTech_Accessor::EventType_MarkedDoneFromTodo;
			break;
		case ET_Skipped:
			result = NexTech_Accessor::EventType_Skipped;
			break;
		case ET_LadderCreated:
			result = NexTech_Accessor::EventType_LadderCreated;
			break;
		case ET_PaymentApplied:
			result = NexTech_Accessor::EventType_PaymentApplied;
			break;
		case ET_EMRCreated:
			result = NexTech_Accessor::EventType_EMRCreated;
			break;
		default:
			return false;
		}
		return true;
	}

	// (c.haag 2013-11-25) - PLID 59507 - Converts a PersonID integer to a chart number string. Publicly-exposed API
	// functions always use chart numbers as inputs.
	_bstr_t PersonToPatient(int nPersonID)
	{
		return (LPCTSTR)FormatString("%d", GetExistingPatientUserDefinedID(nPersonID));
	}

	// (c.haag 2013-11-25) - PLID 59507 - Converts a numeric Practice ID to a string ID used in the API
	_bstr_t IDToString(int ID)
	{
		return (LPCTSTR)FormatString("%d", ID);
	}

	// (c.haag 2013-11-25) - PLID 59507 - Converts a numeric Practice ID to a string ID used in the API.
	// Values of -1 are treated as nulls; in C++ you can't make integers nullable but in C# you can. In many
	// places we use sentinel values of -1 to designate a value we would treat as null.
	_bstr_t NullableIDToString(int ID)
	{
		// This is important: Check -1, but not any other negative number.
		if (-1 == ID) {
			return _bstr_t();
		} else {
			return IDToString(ID);
		}
	}

	// (c.haag 2013-11-26) - PLID 59818 - Converts a string to a "nullable ID." In the API, nullable ID's
	// are nullable integers. In Practice, they are sentinel values of -1.
	long StringToNullableID(_bstr_t ID)
	{
		if (0 == ID.length())
		{
			return -1;
		}
		else
		{
			return atol(ID);
		}
	}

	TrackingEvent PickRecordToCompleteStep(const CArray<TrackingEvent,TrackingEvent&>& arEvents, const CString& strProcedureName, long nAction, const CString& strPersonName)
	{
		// First, make sure we support the action. If we don't, just
		// return the first ID for backwards compatibility.
		CPhaseRecordPickerDlg dlg(NULL);
		switch (nAction)
		{
			case PhaseTracking::PA_ScheduleAptCategory:
			case PhaseTracking::PA_ScheduleAptType:
			case PhaseTracking::PA_ActualAptCategory:
			case PhaseTracking::PA_ActualAptType:
				break;
			default:
				return arEvents[0];
		}

		
		// Now bring up the picker
		if (IDOK == dlg.Open(arEvents, strProcedureName, nAction, strPersonName))
		{
			// The user clicked OK - return the selected ID 
			return dlg.GetSelectedEvent();
		}
		// The user clicked Cancel or something went wrong
		TrackingEvent te;
		te.nID = -1;
		return te;
	}

	// (c.haag 2013-11-25) - PLID 59507 - Encapsulates AdvanceTrackingActions in the API
	NexTech_Accessor::_TrackingTransactionPtr AdvanceTrackingActions(NexTech_Accessor::_TrackingTransactionPtr pendingTransaction)
	{
		return GetAPI()->AdvanceTrackingActions(GetAPISubkey(), GetAPILoginToken(), pendingTransaction);
	}

	// (c.haag 2013-11-25) - PLID 59507 - Completes a SetActiveDate action
	void CompletePendingAction(NexTech_Accessor::_SetActiveDateActionPtr pSetActiveDateAction)
	{
		// We need to prompt them for a date to make the ladder step active.
		CEnterActiveDateDlg dlg(NULL);
		dlg.m_dtDate = COleDateTime::GetCurrentTime();
		dlg.m_strPrompt = (LPCTSTR)pSetActiveDateAction->QuestionPrompt;
		dlg.DoModal();
		pSetActiveDateAction->ActiveDate = dlg.m_dtDate;
	}

	// (c.haag 2013-11-25) - PLID 59832 - Completes an action that involves selecting a template
	SAFEARRAY* ChooseTemplates(SAFEARRAY* possibleSelections, bool allowMultiple)
	{
		Nx::SafeArray<IUnknown *> saPossibleSelections(possibleSelections);
		CArray<long,long> anProcedureIDs;
		CArray<long,long> anTemplateIDs;
		foreach (NexTech_Accessor::_LadderTemplateSelectionPtr pSel, saPossibleSelections)
		{
			anProcedureIDs.Add( atol((LPCTSTR)pSel->ProcedureID) );
			anTemplateIDs.Add( atol((LPCTSTR)pSel->LadderTemplateID) );
		}

		_RecordsetPtr rsLadderTemplates = CreateParamRecordset(
				"SELECT ProcedureT.ID AS ProcedureID, "
					"ProcedureT.Name AS ProcedureName, "
					"LadderTemplatesT.ID AS LadderTemplateID, "
					"LadderTemplatesT.Name AS LadderTemplateName "
				"FROM ProcedureT "
				"LEFT JOIN ProcedureT MasterProcedure "
				"ON ProcedureT.MasterProcedureID = MasterProcedure.ID "
				"INNER JOIN ProcedureLadderTemplateT ON "
					"CASE WHEN MasterProcedure.ID Is Null "
					"THEN ProcedureT.ID "
					"ELSE MasterProcedure.ID "
					"END = ProcedureLadderTemplateT.ProcedureID "
				"INNER JOIN LadderTemplatesT ON "
				"ProcedureLadderTemplateT.LadderTemplateID = LadderTemplatesT.ID "
				"WHERE ProcedureT.ID IN ({INTARRAY}) AND LadderTemplatesT.ID IN ({INTARRAY}) "
				"ORDER BY ProcedureT.Name, LadderTemplatesT.Name "
				, anProcedureIDs, anTemplateIDs);

		// There is more than one possible ladder to add procedures for.
		// Select one or more ladders from a dialog.
		CSelectTemplateDlg dlg(NULL);
		dlg.m_rsTemplates = rsLadderTemplates;
		dlg.m_bAllowMultiple = allowMultiple;
		dlg.DoModal();

		// (c.haag 2013-11-25) - PLID 59507 - Save the selection(s) to pAddPatientProceduresAction->SelectionResponses
		Nx::SafeArray<IUnknown *> saSelections;
		// (d.moore 2007-07-09 14:17) - PLID 14670 - Get the list of template Id values
		//  that were selected. May have only one selection.
		long nLadSelCount = dlg.GetSelectionCount();
		if (nLadSelCount > 1) 
		{
			CMap<long, long, long, long> mLadderTemplateIdMap; // Key = ProcedureID, Value = LadderTemplateID
			dlg.GetSelectedLadderIds(mLadderTemplateIdMap);
			POSITION pos = mLadderTemplateIdMap.GetStartPosition();
			while (pos != NULL) 
			{
				long nProcedureID;
				long nTemplateID;
				mLadderTemplateIdMap.GetNextAssoc( pos, nProcedureID, nTemplateID );
				NexTech_Accessor::_LadderTemplateSelectionPtr pSel(__uuidof(NexTech_Accessor::LadderTemplateSelection));
				pSel->ProcedureID = (LPCTSTR)AsString(nProcedureID);
				pSel->LadderTemplateID = (LPCTSTR)AsString(nTemplateID);
				saSelections.Add(pSel);
			}
		} 
		else 
		{
			// There should only be one possible selection.
			long nTemplateID = dlg.GetSelectedLadderID();
			NexTech_Accessor::_LadderTemplateSelectionPtr pSel(__uuidof(NexTech_Accessor::LadderTemplateSelection));
			for (int i=0; i < anTemplateIDs.GetCount(); i++)
			{
				if (anTemplateIDs[i] == nTemplateID)
				{
					pSel->ProcedureID = (LPCTSTR)AsString(anProcedureIDs[i]);
					pSel->LadderTemplateID = (LPCTSTR)AsString(anTemplateIDs[i]);
					saSelections.Add(pSel);
					break;
				}
			}
		}
		return saSelections.Detach();
	}

	// (c.haag 2013-11-25) - PLID 59507 - Completes an AddPatientProcedures action.
	void CompletePendingAction(NexTech_Accessor::_AddPatientProceduresActionPtr pAddPatientProceduresAction)
	{
		pAddPatientProceduresAction->SelectionResponses = ChooseTemplates(pAddPatientProceduresAction->PossibleSelections, true);
	}

	// (c.haag 2013-11-25) - PLID 59832 - Completes a CreateAndApplyEvent action.
	void CompletePendingAction(NexTech_Accessor::_CreateAndApplyEventActionPtr pCreateAndApplyEventAction)
	{
		pCreateAndApplyEventAction->SelectionResponses = ChooseTemplates(pCreateAndApplyEventAction->PossibleSelections, true);
	}

	// (c.haag 2013-11-25) - PLID 59817 - Completes a FillProcInfoAction action.
	void CompletePendingAction(NexTech_Accessor::_FillProcInfoActionPtr pFillProcInfoAction)
	{
		if (VARIANT_FALSE != pFillProcInfoAction->WarnNurse)
		{
			if (IDNO == MsgBox(MB_YESNO, (LPCTSTR)pFillProcInfoAction->WarnNurseQuestion)) 
			{
				MsgBox("You can choose a different Nurse in the Procedure Information Center for this ladder.");
				pFillProcInfoAction->WarnNurseResponse = VARIANT_FALSE;
			}
			else
			{
				pFillProcInfoAction->WarnNurseResponse = VARIANT_TRUE;
			}
		}

		if (VARIANT_FALSE != pFillProcInfoAction->WarnAnesthesiologist)
		{
			if (IDNO == MsgBox(MB_YESNO, (LPCTSTR)pFillProcInfoAction->WarnAnesthesiologistQuestion)) 
			{
				MsgBox("You can choose a different Anesthesiologist in the Procedure Information Center for this ladder.");
				pFillProcInfoAction->WarnAnesthesiologistResponse = VARIANT_FALSE;
			}
			else
			{
				pFillProcInfoAction->WarnAnesthesiologistResponse = VARIANT_TRUE;
			}
		}

		if (VARIANT_FALSE != pFillProcInfoAction->AskAnesthesia)
		{
			//Ask the user to select an anesthesia
			CSelectDlg dlg(NULL);
			CString strAnesthesia;
			dlg.m_strTitle = "Select an Anesthesia";
			dlg.m_strCaption = "The anesthesia to be used for the combination of these procedures could not be determined.  Please select an anesthesia from the list below.";
			Nx::SafeArray<BSTR> saAnesthesiaTypes(pFillProcInfoAction->AnesthesiaTypes);
			CString strSql;
			foreach (_bstr_t anesthesiaType, saAnesthesiaTypes)
			{
				if (strSql.IsEmpty())
					strSql = FormatString("(SELECT '%s' AS Anesthesia", _Q((LPCTSTR)anesthesiaType));
				else
					strSql += FormatString(" UNION SELECT '%s' AS Anesthesia", _Q((LPCTSTR)anesthesiaType));
			}
			strSql += ") SubQ";

			dlg.m_strFromClause = strSql;
			dlg.AddColumn("Anesthesia", "Anesthesia", TRUE, FALSE);
			if(dlg.DoModal() == IDOK) {

				//Set our selected anesthesia
				strAnesthesia = VarString(dlg.m_arSelectedValues[0], "");

				//Determine if <No Anesthesia Selected> has been chosen
				if(strAnesthesia == "<No Anesthesia Selected>")
					strAnesthesia.Empty();				
			}
			else
				// We pressed cancel, so make sure the anesthesia is empty.  This shouldn't need to be done, but is a safety measure.
				strAnesthesia.Empty();

			pFillProcInfoAction->SelectedAnesthesiaType = (LPCTSTR)strAnesthesia;
		}
	}

	// (c.haag 2013-12-04) - PLID 59818 - Completes an AddLadderToProcInfo action.
	void CompletePendingAction(NexTech_Accessor::_AddLadderToPatientProcedureInfoActionPtr pAddLadderToPatientProcedureInfoAction)
	{
		pAddLadderToPatientProcedureInfoAction->SelectionResponses = ChooseTemplates(pAddLadderToPatientProcedureInfoAction->PossibleSelections, false);
	}
	
	// (c.haag 2013-12-04) - PLID 59822 - Completes an PickRecordToCompleteStepAction action.
	void CompletePendingAction(NexTech_Accessor::_PickRecordToCompleteStepActionPtr pPickRecordToCompleteStep)
	{
		// Get the native values of each object
		CArray<TrackingEvent,TrackingEvent&> arEvents;
		Nx::SafeArray<IUnknown *> saPossibleEvents(pPickRecordToCompleteStep->PossibleEvents);
		foreach (NexTech_Accessor::_TrackingEventPtr pTrackingEvent, saPossibleEvents)
		{
			TrackingEvent te;
			te.dtDate = pTrackingEvent->EventDate;
			te.nID = (VARIANT_FALSE == pTrackingEvent->ID->IsNull()) ? VarLong(pTrackingEvent->ID->GetValue()) : -1;
			te.strDescription = (LPCTSTR)pTrackingEvent->EventDescription;
			arEvents.Add(te);
		}
		CString strProcedureName = (LPCTSTR)pPickRecordToCompleteStep->ProcedureName;
		PhaseAction action;
		if (!ToPracticePhaseAction(pPickRecordToCompleteStep->StepAction, action))
		{
			ThrowNxException("CompletePendingAction was called with an invalid action %d!", (int)pPickRecordToCompleteStep->StepAction);
		}
		CString strPersonName = (LPCTSTR)pPickRecordToCompleteStep->PersonName;

		// Now call the native version of PickRecordToCompleteStep which does nothing but open up a dialog box if warranted
		// and return a result
		TrackingEvent teResult = PickRecordToCompleteStep(arEvents, strProcedureName, (int)action, strPersonName);

		// Store the result in the action object
		NexTech_Accessor::_TrackingEventPtr pTrackingEvent(__uuidof(NexTech_Accessor::TrackingEvent));
		pTrackingEvent->EventDate = teResult.dtDate;
		pTrackingEvent->EventDescription = (LPCTSTR)teResult.strDescription;
		if (teResult.nID != -1)
		{
			NexTech_Accessor::_NullableIntPtr pID(__uuidof(NexTech_Accessor::NullableInt));
			pID->SetInt(teResult.nID);
			pTrackingEvent->ID = pID;
			pPickRecordToCompleteStep->ChosenRecord = pTrackingEvent;
		}
	}

	// (c.haag 2014-10-20) - PLID 63862 - Completes an UnapplyEvent action
	void CompletePendingAction(NexTech_Accessor::_UnapplyEventActionPtr pUnapplyEventAction)
	{
		CString strMsg((LPCTSTR)pUnapplyEventAction->PromptMessage);
		pUnapplyEventAction->PromptResponse = (IDYES == MsgBox(MB_YESNO, "%s", strMsg)) ? VARIANT_TRUE : VARIANT_FALSE;
	}

	// (c.haag 2013-11-25) - PLID 59507 - Completes any pending actions in a transaction and returns the final action.
	NexTech_Accessor::_TrackingActionPtr CompletePendingActions(NexTech_Accessor::_TrackingTransactionPtr transaction)
	{
		if (NULL != transaction)
		{
			BOOL bRetry = TRUE;
			while (bRetry)
			{
				if (NULL != transaction->PendingAction)
				{
					// We have a pending action! Figure out the type of the most immediate one and process it.
					IDispatchPtr pDisp = (IDispatchPtr)transaction->PendingAction;

					// Go through all known actions to find the one that we need to properly complete
					BOOL bActionFound = FALSE;
					NexTech_Accessor::_SetActiveDateActionPtr pSetActiveDateAction(pDisp);
					if (NULL != pSetActiveDateAction)
					{
						CompletePendingAction(pSetActiveDateAction);
						bActionFound = TRUE;
					}
					if (!bActionFound)
					{
						NexTech_Accessor::_AddPatientProceduresActionPtr pAddPatientProceduresAction(pDisp);
						if (NULL != pAddPatientProceduresAction)
						{
							CompletePendingAction(pAddPatientProceduresAction);
							bActionFound = TRUE;
						}
					}
					if (!bActionFound)
					{
						NexTech_Accessor::_CreateAndApplyEventActionPtr pCreateAndApplyEventAction(pDisp);
						if (NULL != pCreateAndApplyEventAction)
						{
							CompletePendingAction(pCreateAndApplyEventAction);
							bActionFound = TRUE;
						}
					}
					if (!bActionFound)
					{
						NexTech_Accessor::_FillProcInfoActionPtr pFillProcInfoAction(pDisp);
						if (NULL != pFillProcInfoAction)
						{
							CompletePendingAction(pFillProcInfoAction);
							bActionFound = TRUE;
						}
					}
					if (!bActionFound)
					{
						NexTech_Accessor::_AddLadderToPatientProcedureInfoActionPtr pAddLadderToPatientProcedureInfoAction(pDisp);
						if (NULL != pAddLadderToPatientProcedureInfoAction)
						{
							CompletePendingAction(pAddLadderToPatientProcedureInfoAction);
							bActionFound = TRUE;
						}
					}
					if (!bActionFound)
					{
						NexTech_Accessor::_PickRecordToCompleteStepActionPtr pPickRecordToCompleteStepAction(pDisp);
						if (NULL != pPickRecordToCompleteStepAction)
						{
							CompletePendingAction(pPickRecordToCompleteStepAction);
							bActionFound = TRUE;
						}
					}
					if (!bActionFound)
					{
						NexTech_Accessor::_UnapplyEventActionPtr pUnapplyEventAction(pDisp);
						if (NULL != pUnapplyEventAction)
						{
							CompletePendingAction(pUnapplyEventAction);
							bActionFound = TRUE;
						}
					}

					if (!bActionFound)
					{
						// If we get here, we don't have a handler for this pending action
						ThrowNxException("An unsupported pending tracking action was encountered!");
					}

					// Now advance the actions
					transaction = AdvanceTrackingActions(transaction);

				} // if (NULL != transaction->PendingAction)
				else
				{
					// PendingAction is null; nothing to do
					bRetry = FALSE;
				}
			} // while (bRetry)

			// Return the last completed action so the caller can cast it and get any desired return value
			return transaction->LastCompletedAction;

		} // if (NULL != transaction)
		else
		{
			// No transaction, so no object to return
			return NULL;
		}		
	}

// (j.jones 2009-11-02 10:06) - PLID 36082 - if an event auto-launched a ladder, it will pass in the event type and ID that did so
void AddPtntProcedures(CArray<int, int> &ary, long personID, bool bIsActive /* = true */, long nOverrideLadderTemplateID /*= -1*/,
					   PhaseTracking::EventType nLaunchedByType  /*= ET_Invalid*/, long nLaunchedByItemID  /*= -1*/)
{
	try
	{
		// (c.haag 2013-11-25) - PLID 59507 - Call from the tracking API. We also explicitly check for non-patients now.
		if (personID <= 0)
			return;

		CArray<_bstr_t> bstrAry;
		foreach (int n, ary)
		{
			bstrAry.Add(IDToString(n));
		}
		_bstr_t patientID = PersonToPatient(personID);
		VARIANT_BOOL isActive = (bIsActive)? VARIANT_TRUE : VARIANT_FALSE;
		_bstr_t overrideLadderTemplateID = NullableIDToString(nOverrideLadderTemplateID);
		NexTech_Accessor::EventType launchedByType;
		if (!ToAPIEventType(nLaunchedByType, launchedByType)) {
			ThrowNxException("Unexpected launchedByType: %d", (int)nLaunchedByType);
		}
		_bstr_t launchedByItemID = NullableIDToString(nOverrideLadderTemplateID);

		// Call the function
		NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->AddPatientProcedures(
			GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<BSTR>::From(bstrAry), patientID, isActive, overrideLadderTemplateID, launchedByType, launchedByItemID);

		// Complete any pending actions. We don't care about the result.
		CompletePendingActions(pTrackingTransaction);
	}
	NxCatchAll("Error in CPhaseTracking::AddPtntProcedures()");
}

// (z.manning 2008-10-28 09:39) - PLID 31371 - Returns true if we added a tracking ladder and false if not
//   Copied from CPatientProcedureDlg
BOOL PromptToAddPtnProcedures(const long nPatientID)
{
	if(!g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
		return FALSE;
	}
	//(e.lally 2008-09-19) PLID 31386 - Check for permission to create a new procedure ladder
	if(!CheckCurrentUserPermissions(bioPatientTracking, sptWrite)){
		return FALSE;
	}
	CAddProcedureDlg dlg(NULL);
	if(IDOK == dlg.DoModal()) {
		CWaitCursor cuWait;

		if( dlg.m_bProcedures)
		{
			if (dlg.m_arProcIDs.GetSize())
			{	
				PhaseTracking::AddPtntProcedures(dlg.m_arProcIDs, nPatientID);
				//(e.lally 2009-03-11) PLID 33470 - Moved auditing into the function that actually changes the data.
				
				return TRUE;
			}
		}
		else
		{
			if (dlg.m_arProcGroupIDs.GetSize())
			{
				PhaseTracking::AddProcedureGroups(dlg.m_arProcGroupIDs, nPatientID);
				//(e.lally 2009-03-11) PLID 33470 - Moved auditing into the function that actually changes the data.
				
				return TRUE;
			}
		}
	}

	return FALSE;
}

long CreateProcInfo(long nPatientID, CArray<int, int> &arProcIDs, bool bCreatePic /*= true*/)
{
	// (c.haag 2013-11-25) - PLID 59817 - Call from the tracking API. We also explicitly check for non-patients now.
	if (nPatientID <= 0)
		return -1; // Callers treat this as an invalid procInfoID

	_bstr_t userDefinedID = PersonToPatient(nPatientID);
	CArray<_bstr_t> procedureIDAry;
	foreach (int n, arProcIDs)
	{
		procedureIDAry.Add(IDToString(n));
	}
	VARIANT_BOOL createProcedureInformationCenter = (bCreatePic) ? VARIANT_TRUE : VARIANT_FALSE;

	// Call the function
	NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->CreatePatientProcedureInformation(
		GetAPISubkey(), GetAPILoginToken(), userDefinedID, Nx::SafeArray<BSTR>::From(procedureIDAry), createProcedureInformationCenter);

	// Complete any pending actions
	NexTech_Accessor::_CreateProcInfoActionPtr action = (NexTech_Accessor::_CreateProcInfoActionPtr)CompletePendingActions(pTrackingTransaction);

	// Return the result. We never expect an empty string, but we'll convert it to -1 if it is so.
	return StringToNullableID(action->ProcedureInfoID);
}

//Used in this function to store the results of a recordset.
struct Step {
	long nStepID;
	BOOL bSkippable;
	long nAction;
	long nStepTemplateID;
};

// (c.haag 2009-02-11 12:33) - PLID 33008 - Auto-complete as many ladder steps as possible
// (c.haag 2013-11-26) - PLID 59822 - We no longer pass in the patient ID
void TryToCompleteAllLadderSteps(long nLadderID)
{
	// (c.haag 2013-11-26) - PLID 59822 - Call from the tracking API. We don't need the patient ID
	// because we can calculate it from the ladder ID. Having it declared separately just creates the
	// potential for bad data.
	_bstr_t ladderID = IDToString(nLadderID);

	// Call the function
	NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->TryToCompleteAllLadderSteps(GetAPISubkey(), GetAPILoginToken(), ladderID);

	// Complete any pending actions. We don't care about the result.
	CompletePendingActions(pTrackingTransaction);
}

// (j.jones 2009-11-02 10:50) - PLID 36082 - if given an event that launched the ladder,
// we will need to set it as the first step's completed event
long AddLadderToProcInfo(long nProcInfoID, long nPatientID, long nLadderTemplateID /*= -1*/,
						 PhaseTracking::EventType nLaunchedByType /*= ET_Invalid*/, long nLaunchedByItemID /*= -1*/)
{
	// (c.haag 2013-11-25) - PLID 59818 - Call from the tracking API We also explicitly check for non-patients now.
	if (nPatientID <= 0)
		return -1; // Callers treat this as an invalid ladder ID

	_bstr_t userDefinedID = PersonToPatient(nPatientID);
	_bstr_t procedureInfoID = IDToString(nProcInfoID);
	_bstr_t ladderTemplateID = NullableIDToString(nLadderTemplateID);
	NexTech_Accessor::EventType launchedByType;
	if (!ToAPIEventType(nLaunchedByType, launchedByType)) {
		ThrowNxException("Unexpected launchedByType: %d", (int)nLaunchedByType);
	}
	_bstr_t launchedByItemID = NullableIDToString(nLaunchedByItemID);

	// Call the function
	NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->AddLadderToPatientProcedureInfo(
		GetAPISubkey(), GetAPILoginToken(), procedureInfoID, userDefinedID, ladderTemplateID, launchedByType, launchedByItemID);

	// Complete any pending actions
	NexTech_Accessor::_AddLadderToPatientProcedureInfoActionPtr action = (NexTech_Accessor::_AddLadderToPatientProcedureInfoActionPtr)CompletePendingActions(pTrackingTransaction);

	// Return the result. We never expect an empty string, but we'll convert it to -1 if it is so.
	return StringToNullableID(action->LadderID);
}

void MoveStepUp(long ladder, long newStep, long oldStep, bool bIsTracked)
{
	try	{
		//(e.lally 2007-05-10) PLID 25112 - Move these executes into a batch transaction
		// (d.thompson 2009-12-29) - PLID 36728 - Rewrote again to use proper parameterized batches.
		CString strBatch = BeginSqlBatch();
		CNxParamSqlArray args;

		if(bIsTracked) {
			//Well, crap.  We're going to have to deactivate oldStep and newStep.
			// (d.thompson 2009-12-23) - PLID 36706 - Parameterized
			_RecordsetPtr rsDoomedTemplates = CreateParamRecordset("SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = {INT} AND (StepOrder = {INT} OR StepOrder = {INT}) AND Inactive = 0", ladder, newStep, oldStep);
			FieldsPtr fDoomedTemplates = rsDoomedTemplates->Fields;
			AddDeclarationToSqlBatch(strBatch, "DECLARE @nNewStepTemplateID INT \r\n");
			while(!rsDoomedTemplates->eof) {
				AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET Inactive = 1 WHERE ID = {INT};", AdoFldLong(fDoomedTemplates, "ID"));
				//long nNewStepTemplateID = NewNumber("StepTemplatesT", "ID");
				//Update our new stepTemplateID
				AddDeclarationToSqlBatch(strBatch, "SET @nNewStepTemplateID = (SELECT COALESCE(Max(ID), 0)+1 FROM StepTemplatesT) \r\n");

				// (j.jones 2008-11-17 17:27) - PLID 30926 - added OpenPIC
				//TES 8/6/2010 - PLID 39705 - Added DefaultScope
				AddParamStatementToSqlBatch(strBatch, args, "INSERT INTO StepTemplatesT (ID, LadderTemplateID, StepOrder, StepName, Action, Note, ActivateType, ActivateInterval, ActivateStringData, Inactive, Skippable, Todo, ToDoPriority, TodoCategory, OpenPIC, DefaultScope) "
					"SELECT @nNewStepTemplateID, LadderTemplateID, StepOrder, StepName, Action, Note, ActivateType, ActivateInterval, ActivateStringData, 0, Skippable, Todo, ToDoPriority, TodoCategory, OpenPIC, DefaultScope "
					"FROM StepTemplatesT WHERE ID = {INT};", AdoFldLong(fDoomedTemplates, "ID"));
				AddParamStatementToSqlBatch(strBatch, args, "INSERT INTO StepCriteriaT (StepTemplateID, ActionID) "
					"SELECT @nNewStepTemplateID, ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT};", 
					AdoFldLong(fDoomedTemplates, "ID"));
				// (j.jones 2008-11-26 13:35) - PLID 30830 - we now support multiple users
				AddParamStatementToSqlBatch(strBatch, args, "INSERT INTO StepTemplatesAssignToT (StepTemplateID, UserID) "
					"SELECT @nNewStepTemplateID, UserID FROM StepTemplatesAssignToT WHERE StepTemplateID = {INT};",
					AdoFldLong(fDoomedTemplates, "ID"));
				rsDoomedTemplates->MoveNext();
			}
			//OK, now we can go ahead with the other stuff as normal.
		}
		//create a new slot
		AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET StepOrder = StepOrder + 1 WHERE LadderTemplateID = {INT} AND StepOrder >= {INT} AND Inactive = 0",
			ladder, newStep);

		oldStep++;	//we incremented our oldstep position

		//move from the old to the new slot
		AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET StepOrder = {INT} WHERE LadderTemplateID = {INT} AND StepOrder = {INT} AND Inactive = 0",
			newStep, ladder, oldStep);

		//fill in the old slot
		AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET StepOrder = StepOrder - 1 WHERE LadderTemplateID = {INT} AND StepOrder > {INT} AND Inactive = 0",
			ladder, oldStep);

		//Commit our execute statements
		ExecuteParamSqlBatch(GetRemoteData(), strBatch, args);
		return;
	}
	NxCatchAll("Could not change step");
}

void MoveStepDown(long ladder, long newStep, long oldStep, bool bIsTracked)
{
	try	{
		//(e.lally 2007-05-10) PLID 25112 - Move these executes into a batch transaction
		// (d.thompson 2009-12-29) - PLID 36728 - Rewrote again to be in a proper parameterized batch
		CString strBatch = BeginSqlBatch();
		CNxParamSqlArray args;

		if(bIsTracked) {
			//Well, crap.  We're going to have to deactivate oldStep and newStep.
			// (d.thompson 2009-12-23) - PLID 36706 - Parameterized
			_RecordsetPtr rsDoomedTemplates = CreateParamRecordset("SELECT ID FROM StepTemplatesT WHERE LadderTemplateID = {INT} AND (StepOrder = {INT} OR StepOrder = {INT}) AND Inactive = 0", ladder, newStep, oldStep);
			FieldsPtr fDoomedTemplates = rsDoomedTemplates->Fields;
			AddDeclarationToSqlBatch(strBatch, "DECLARE @nNewStepTemplateID INT \r\n"
			"SET @nNewStepTemplateID = (SELECT COALESCE(Max(ID), 0)+1 FROM StepTemplatesT) \r\n");
			while(!rsDoomedTemplates->eof) {
				AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET Inactive = 1 WHERE ID = {INT}", AdoFldLong(fDoomedTemplates, "ID"));
				//long nNewStepTemplateID = NewNumber("StepTemplatesT", "ID");

				// (j.jones 2008-11-17 17:27) - PLID 30926 - added OpenPIC
				//TES 8/6/2010 - PLID 39705 - Added DefaultScope
				AddParamStatementToSqlBatch(strBatch, args, "INSERT INTO StepTemplatesT (ID, LadderTemplateID, StepOrder, StepName, Action, Note, ActivateType, ActivateInterval, ActivateStringData, Inactive, Skippable, Todo, ToDoPriority, TodoCategory, OpenPIC, DefaultScope) "
					"SELECT @nNewStepTemplateID, LadderTemplateID, StepOrder, StepName, Action, Note, ActivateType, ActivateInterval, ActivateStringData, 0, Skippable, Todo, ToDoPriority, TodoCategory, OpenPIC, DefaultScope "
					"FROM StepTemplatesT WHERE ID = {INT}", AdoFldLong(fDoomedTemplates, "ID"));
				AddParamStatementToSqlBatch(strBatch, args, "INSERT INTO StepCriteriaT (StepTemplateID, ActionID) "
					"SELECT @nNewStepTemplateID, ActionID FROM StepCriteriaT WHERE StepTemplateID = {INT}", 
					AdoFldLong(fDoomedTemplates, "ID"));
				// (j.jones 2008-11-26 13:35) - PLID 30830 - we now support multiple users
				AddParamStatementToSqlBatch(strBatch, args, "INSERT INTO StepTemplatesAssignToT (StepTemplateID, UserID) "
					"SELECT @nNewStepTemplateID, UserID FROM StepTemplatesAssignToT WHERE StepTemplateID = {INT}",
					AdoFldLong(fDoomedTemplates, "ID"));
				rsDoomedTemplates->MoveNext();
				//Increment our new stepTemplateID since we know the table is locked.
				AddDeclarationToSqlBatch(strBatch, "SET @nNewStepTemplateID = @nNewStepTemplateID +1 \r\n");
			}
			//OK, now we can go ahead with the other stuff as normal.
		}

		//create a new slot - one slot ahead
		AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET StepOrder = StepOrder + 1 WHERE LadderTemplateID = {INT} AND StepOrder > {INT} AND Inactive = 0",
			ladder, newStep);

		newStep++;	//we incremented our newstep position

		//move from the old to the new slot
		AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET StepOrder = {INT} WHERE LadderTemplateID = {INT} AND StepOrder = {INT} AND Inactive = 0",
			newStep, ladder, oldStep);

		//fill in the old slot
		AddParamStatementToSqlBatch(strBatch, args, "UPDATE StepTemplatesT SET StepOrder = StepOrder - 1 WHERE LadderTemplateID = {INT} AND StepOrder > {INT} AND Inactive = 0",
			ladder, oldStep);

		//Commit our execute statements
		ExecuteParamSqlBatch(GetRemoteData(), strBatch, args);
		return;
	}
	NxCatchAll("Could not change step");
}

void MoveStep (long ladder, long newStep, long oldStep, bool bIsTracked)
{
	if (newStep > oldStep)
		MoveStepDown(ladder, newStep, oldStep, bIsTracked);
	else if (oldStep > newStep)
		MoveStepUp(ladder, newStep, oldStep, bIsTracked);
	else
	{	ASSERT(FALSE);
		return;
	}
}

bool CreateAndApplyEvent(PhaseTracking::EventType nType, int nPatientID, COleDateTime &dt, int nItemID, bool bFancyDateStuff /*= true*/, long nStepID/*= -1*/, bool bAllowCreateLadder /*= true*/)
{
	try {
		// (c.haag 2013-11-26) - PLID 59832 - Call from the tracking API. We also explicitly check for non-patients now.
		if (nPatientID <= 0) {
			return false;
		}
		// (d.singleton 2014-02-14 14:21) - PLID 60790 - ref phys, suppliers etc. still have "patientID" but when you try to convert to userdefinedID
		//	it will be -1. this will then cause an exception in the api code.  so try to convert patientID to userdefinedID.  if its -1 then we have
		//	another case of a non-patient,  return false.
		_bstr_t userDefinedID = PersonToPatient(nPatientID);
		if(userDefinedID == _bstr_t("-1")) {
			return false;
		}

		NexTech_Accessor::EventType eventType;
		if (!ToAPIEventType(nType, eventType)) {
			ThrowNxException("Unexpected event type: %d", (int)eventType);
		}		
		_bstr_t itemID = NullableIDToString(nItemID);
		VARIANT_BOOL fancyDateStuff = (bFancyDateStuff) ? VARIANT_TRUE : VARIANT_FALSE;
		_bstr_t stepID = NullableIDToString(nStepID);
		VARIANT_BOOL allowCreateLadder = (bAllowCreateLadder) ? VARIANT_TRUE : VARIANT_FALSE;

		// Call the function
		NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->CreateAndApplyTrackingEvent(
			GetAPISubkey(), GetAPILoginToken(), eventType, userDefinedID, dt, itemID, fancyDateStuff, stepID, allowCreateLadder);

		// Complete any pending actions
		NexTech_Accessor::_CreateAndApplyEventActionPtr action = (NexTech_Accessor::_CreateAndApplyEventActionPtr)CompletePendingActions(pTrackingTransaction);

		// Return the result. We never expect an empty string, but we'll convert it to -1 if it is so.
		return (VARIANT_FALSE == action->EventApplied) ? false : true;

	}NxCatchAll("Error in PhaseTracking::CreateAndApplyEvent()");
	return false;
}

CString GetEventDescription(PhaseTracking::EventType nType)
{
	// (c.haag 2013-11-26) - PLID 59835 - Call from the tracking API
	NexTech_Accessor::EventType eventType;
	if (!ToAPIEventType(nType, eventType)) {
		return "";
	}

	// Call the function
	return (LPCTSTR)GetAPI()->GetTrackingEventDescription(eventType);
}

void UnapplyEvent(int nPatientID, PhaseTracking::EventType nType, int nItemID, int nStepID)
{
	try {
		// (c.haag 2014-10-13) - PLID 63862 - Call from the tracking API. We also explicitly check for non-patients now.
		if (nPatientID <= 0) {
			return;
		}
		// (d.singleton 2014-02-14 14:21) - PLID 60790 - ref phys, suppliers etc. still have "patientID" but when you try to convert to userdefinedID
		//	it will be -1. this will then cause an exception in the api code.  so try to convert patientID to userdefinedID.  if its -1 then we have
		//	another case of a non-patient,  return false.
		_bstr_t userDefinedID = PersonToPatient(nPatientID);
		if (userDefinedID == _bstr_t("-1")) {
			return;
		}

		NexTech_Accessor::EventType eventType;
		if (!ToAPIEventType(nType, eventType)) {
			ThrowNxException("Unexpected event type: %d", (int)eventType);
		}
		_bstr_t itemID = NullableIDToString(nItemID);
		_bstr_t stepID = NullableIDToString(nStepID);

		// Call the function
		NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->UnapplyEvent(
			GetAPISubkey(), GetAPILoginToken(), userDefinedID, eventType, itemID, stepID);

		// Complete any pending actions
		CompletePendingActions(pTrackingTransaction);
	}
	NxCatchAll("Error in PhaseTracking::UnapplyEvent()");
}

long GetNextStepID(int nLadderID)
{
	//Find the PhaseDetail record which has not been applied to and has the lowest Step.
	// (c.haag 2013-12-02) - PLID 59835 - We use the API now
	_bstr_t ladderID = IDToString(nLadderID);
	_bstr_t result = GetAPI()->GetNextStepID(GetAPISubkey(), GetAPILoginToken(), ladderID);
	return StringToNullableID(result);
}

CString GetItemDescription(PhaseTracking::EventType nType, int nItemID)
{
	// (j.jones 2008-09-03 11:53) - PLID 10417 - parameterized all these queries
	// (j.jones 2008-09-16 11:17) - PLID 31365 - made all the recordsets check for eof,
	// and assert if it is eof (because it would mean bad data)
	_RecordsetPtr rsTemp;
	switch(nType) {
	case ET_QuoteCreated:
	case ET_BillCreated:
		rsTemp = CreateParamRecordset("SELECT Description FROM BillsT WHERE ID = {INT}", nItemID);
		if(rsTemp->eof) {
			ASSERT(FALSE);
			return "";
		}
		else  {
			return AdoFldString(rsTemp, "Description", "");
		}
		break;
	case ET_MarkedDone:
		return "Manually marked done";
		break;
	case ET_MarkedDoneFromTodo:
		return "Marked done from ToDo List";
		break;
	case ET_AppointmentCreated:
	case ET_ActualAppointment:
		rsTemp = CreateParamRecordset("SELECT CASE WHEN AptTypeT.Name Is Null THEN '' ELSE AptTypeT.Name END + ' ' + CASE WHEN AptPurposeT.Name Is Null THEN '' ELSE AptPurposeT.Name END + ' ' + Notes AS Description FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID LEFT JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID LEFT JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID WHERE AppointmentsT.ID = {INT}", nItemID);
		if(rsTemp->eof) {
			ASSERT(FALSE);
			return "";
		}
		else  {
			return AdoFldString(rsTemp, "Description", "");
		}
		break;
	case ET_TemplateSent:
		// (j.jones 2008-09-05 12:54) - PLID 30288 - supported MailSentNotesT
		rsTemp = CreateParamRecordset("SELECT Note FROM MailSentNotesT WHERE MailID = {INT}", nItemID);
		if(!rsTemp->eof) {
			return AdoFldString(rsTemp, "Note", "");
		}
		else {
			ASSERT(FALSE);
			return "";
		}
		break;
	case ET_PacketSent:
		rsTemp = CreateParamRecordset("SELECT Name FROM PacketsT WHERE ID = (SELECT PacketID FROM MergedPacketsT WHERE ID = {INT})", nItemID);
		if(rsTemp->eof) {
			ASSERT(FALSE);
			return "";
		}
		else {
			return AdoFldString(rsTemp, "Name", "");
		}
		break;
	case ET_Skipped:
		return "Skipped";
		break;
	case ET_LadderCreated:
		rsTemp = CreateParamRecordset("SELECT dbo.CalcProcInfoName(ProcInfoID) AS Name FROM LaddersT WHERE ID = {INT}", nItemID);
		if(rsTemp->eof) {
			ASSERT(FALSE);
			return "";
		}
		else {
			return AdoFldString(rsTemp, "Name","");
		}
		break;
	case ET_PaymentApplied:
		rsTemp = CreateParamRecordset("SELECT Description FROM LineItemT WHERE ID = {INT}", nItemID);
		if(rsTemp->eof) {
			ASSERT(FALSE);
			return "";
		}
		else {
			return AdoFldString(rsTemp, "Description", "");
		}
		break;
	case ET_EMRCreated:
		{
			rsTemp = CreateParamRecordset("SELECT Name FROM ProcedureT INNER JOIN EmrProcedureT ON ProcedureT.ID = EmrProcedureT.ProcedureID INNER JOIN EMRMasterT ON EmrProcedureT.EMRID = EMRMasterT.ID WHERE EMRProcedureT.Deleted = 0 AND EMRMasterT.ID = {INT}", nItemID);
			if(rsTemp->eof) {
				//TES 7/22/2010 - PLID 39785 - There's no reason to assert here, all this means is that the EMR doesn't have any procedures.
				// What's wrong with that?
				//ASSERT(FALSE);
				return "<No Procedure>";
			}
			else {
				CString strProcNames;
				while(!rsTemp->eof) {
					strProcNames += AdoFldString(rsTemp, "Name") + ", ";
					rsTemp->MoveNext();
				}
				strProcNames = strProcNames.Left(strProcNames.GetLength()-2);
				if(strProcNames.IsEmpty()) {
					strProcNames = "<No Procedure>";
				}
				return strProcNames;
			}
		}
		break;
	default:
		return "";
		break;
	}
}

CString GetPhaseActionDescription(PhaseTracking::PhaseAction nAction)
{
	// (c.haag 2013-11-27) - PLID 59835 - Call from the tracking API
	NexTech_Accessor::PhaseAction accessorAction;
	bool bSuccess = ToAPIPhaseAction(nAction, accessorAction);
	if (bSuccess) {
		return (LPCTSTR)GetAPI()->GetPhaseActionDescription( accessorAction );
	} else {
		return "";
	}
}

void PhaseTracking::SetActiveDate(long nLadderID, long nPreviousActiveStepID, long nPreviousActiveStepOrder, bool bFancyDateStuff /*= true*/, bool bAllowLadderCreation /*= true*/)
{
	// (c.haag 2013-11-27) - PLID 59835 - Call from the tracking API
	_bstr_t ladderID = IDToString(nLadderID);
	_bstr_t previousActiveStepID = NullableIDToString(nPreviousActiveStepID);
	_bstr_t previousActiveStepOrder = NullableIDToString(nPreviousActiveStepOrder);
	VARIANT_BOOL fancyDateStuff = (bFancyDateStuff) ? VARIANT_TRUE : VARIANT_FALSE;
	VARIANT_BOOL allowLadderCreation = (bAllowLadderCreation) ? VARIANT_TRUE : VARIANT_FALSE;

	// Call the function
	NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->SetLadderActiveDate(
		GetAPISubkey(), GetAPILoginToken(), ladderID, previousActiveStepID, previousActiveStepOrder, 
			fancyDateStuff, allowLadderCreation);

	// Complete any pending actions. There is no return value for this function so don't retain it from this call.
	CompletePendingActions(pTrackingTransaction);
}

void AddProcedureGroups(CArray<int, int> &arProcGroupIDs, long personID, bool bIsActive /* = true */)
{
	if (!arProcGroupIDs.GetSize())
		return;

	//We want to start a transaction here so that if any of our ladders fail to be created, nothing it committed.
	try
	{
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("Adding Patient Procedure Groups.");
		trans.Begin();

		// (d.thompson 2009-12-31) - PLID 36740 - This hasn't been used in a long time!
		//long	id = NewNumber("LaddersT", "ID");

		for (int i = 0; i < arProcGroupIDs.GetSize(); i++)
		{
			// (c.haag 2009-01-07 09:50) - PLID 32571 - Don't include inactive procedures
			// (d.thompson 2009-12-31) - PLID 36740 - Combined 2 trips into a single trip.
			_RecordsetPtr rsPhaseID = CreateParamRecordset("SELECT ID FROM ProcedureT WHERE Inactive = 0 AND ProcedureGroupID = {INT};\r\n"
				"SELECT LadderTemplateID FROM ProcedureGroupsT WHERE ID = {INT};\r\n", arProcGroupIDs[i], arProcGroupIDs.GetAt(i));

			CArray<int,int> arProcIDs;
			rsPhaseID->MoveFirst();
			while(!rsPhaseID->eof) {
				arProcIDs.Add(AdoFldLong(rsPhaseID, "ID"));
				rsPhaseID->MoveNext();
			}
			// (d.thompson 2009-12-31) - PLID 36740 - Moved the query to a single trip above
			//_RecordsetPtr rsLadderTemplate = CreateRecordset("SELECT LadderTemplateID FROM ProcedureGroupsT WHERE ID = %li", arProcGroupIDs.GetAt(i));
			_RecordsetPtr rsLadderTemplate = rsPhaseID->NextRecordset(NULL);
			long nLadderTemplateID = AdoFldLong(rsLadderTemplate, "LadderTemplateID");
			rsLadderTemplate->Close();

			long nLadderID = AddLadderToProcInfo(CreateProcInfo(personID, arProcIDs), personID, nLadderTemplateID);

			if(nLadderID != -1 && !bIsActive) {
				// (d.thompson 2009-12-31) - PLID 36740 - Parameterized.  I decided to leave this in the loop parameterized
				//	because it's an infrequently used query, and this function is run most of the time with just a single
				//	loop iteration.
				ExecuteParamSql("UPDATE LaddersT SET Status = 0 WHERE ID = {INT}", nLadderID);
			}
		}
		trans.Commit();
		return;
	}
	NxCatchAll("Could not add patient procedure groups");
}

void GetMatchingEvents(long nStepTemplateID, long nLadderID, long nPatientID, PhaseAction nAction, OUT CArray<TrackingEvent,TrackingEvent&> &arEvents, BOOL bIncludeDescription /*= FALSE*/, BOOL bManuallyApplied /*= FALSE*/)
{
	// (c.haag 2013-11-27) - PLID 59835 - Call from the tracking API. We also explicitly check for non-patients now.
	if (nPatientID <= 0)
		return;

	// GetMatchingEvents(string databaseSubkey, string loginTokenUID, string stepTemplateID, string ladderID, string patientID, PhaseAction action, bool includeDescription, bool manuallyApplied)
	_bstr_t stepTemplateID = IDToString(nStepTemplateID);
	_bstr_t ladderID = NullableIDToString(nLadderID);
	_bstr_t userDefinedID = PersonToPatient(nPatientID);
	NexTech_Accessor::PhaseAction accessorAction;
	if (!ToAPIPhaseAction(nAction, accessorAction)) ThrowNxException("Unsupported action for GetMatchingEvents: %d", (int)nAction);
	VARIANT_BOOL includeDescription = (bIncludeDescription) ? VARIANT_TRUE : VARIANT_FALSE;
	VARIANT_BOOL manuallyApplied = (bManuallyApplied) ? VARIANT_TRUE : VARIANT_FALSE;

	// Call the function
	NexTech_Accessor::_TrackingEventsPtr pTrackingEvents = GetAPI()->GetMatchingTrackingEvents(
		GetAPISubkey(), GetAPILoginToken(), stepTemplateID, ladderID, userDefinedID, accessorAction, includeDescription, manuallyApplied);

	// Return the events
	Nx::SafeArray<IUnknown *> saTrackingEvents(pTrackingEvents->Values);
	foreach (NexTech_Accessor::_TrackingEventPtr tep, saTrackingEvents)
	{
		TrackingEvent te;
		te.dtDate = tep->EventDate;
		if (VARIANT_FALSE == tep->ID->IsNull())
		{
			te.nID = VarLong(tep->ID->GetValue());			
		}
		else
		{
			te.nID = -1; // Should never happen
		}
		te.strDescription = (LPCTSTR)tep->EventDescription;
		arEvents.Add(te);
	}
}

// (c.haag 2013-11-26) - PLID 59822 - We no longer need the ladder, patient, step template or actions. Those are redundant
// fields that can be (and have been) calculated from the step.
bool TryToCompleteStep(long nStepID, bool bApplyIfMultiple)
{
	// (c.haag 2013-11-26) - PLID 59822 - Call from the tracking API.
	_bstr_t stepID = IDToString(nStepID);
	VARIANT_BOOL applyIfMultiple = (bApplyIfMultiple) ? VARIANT_TRUE : VARIANT_FALSE;

	// Call the function
	NexTech_Accessor::_TrackingTransactionPtr pTrackingTransaction = GetAPI()->TryToCompleteStep(GetAPISubkey(), GetAPILoginToken(), 
		stepID, applyIfMultiple);

	// Complete any pending actions
	NexTech_Accessor::_TryToCompleteStepActionPtr action = (NexTech_Accessor::_TryToCompleteStepActionPtr)CompletePendingActions(pTrackingTransaction);

	// Return the result.
	return (VARIANT_FALSE == action->EventApplied) ? FALSE : TRUE;
}

// (c.haag 2013-11-26) - PLID 59822 - No function ever checked the return value, so I'm making this void.
void CompleteStep(PhaseTracking::EventType nType, int nPatientID, COleDateTime &dt, int nItemID, long nStep, COleDateTime dtEventDate, long nLadderID, long nEventID /*=-1*/) 
{
	// (c.haag 2013-11-26) - PLID 59822 - Call from the tracking API. We also explicitly check for non-patients now.
	if (nPatientID <= 0)
		return;

	NexTech_Accessor::EventType eventType;
	if (!ToAPIEventType(nType, eventType)) {
		ThrowNxException("Unexpected event type: %d", (int)nType);
	}
	_bstr_t userDefinedID = PersonToPatient(nPatientID);
	_bstr_t itemID = NullableIDToString(nItemID);
	_bstr_t stepID = IDToString(nStep);
	_bstr_t ladderID = IDToString(nLadderID);
	_bstr_t eventID = NullableIDToString(nEventID);

	// Call the function
	GetAPI()->CompleteStep(GetAPISubkey(), GetAPILoginToken(), eventType, userDefinedID, dt, itemID, stepID, 
		dtEventDate, ladderID, eventID);
}

//TES 11/28/2007 - PLID 28210 - Need to pass strSql by reference!
// (d.thompson 2010-01-13) - PLID 36865 - This now returns a parameterized query batch
void PhaseTracking::GenerateDetachFromProcInfoStatement(PhaseTracking::EventType nType, int nItemID, 
														IN OUT CString &strSqlBatch, IN OUT CNxParamSqlArray &args)
{
	//(e.lally 2007-05-22) PLID 25112 - Add ability for the detach from proc info function to add to our input sql string
		//instead of executing right away
	CString strTemp;
	switch (nType) {
	case ET_BillCreated:
		{
			AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE ProcInfoT SET BillID = NULL WHERE BillID = {INT};\r\n", nItemID);
		}
		break;
	case ET_AppointmentCreated:
		{
			//(e.lally 2007-05-10) PLID 25112 - combine into one execute statement
			AddParamStatementToSqlBatch(strSqlBatch, args, 
				//First, clear out any ProcInfoAppointments.			
				"DELETE FROM ProcInfoAppointmentsT WHERE AppointmentID = {INT}; \r\n"
				//Next, loop through any PICs that have this as their Surgery appointment.
				"DECLARE @ProcInfoID int; \r\n"
				"DECLARE cur CURSOR FOR \r\n"
				//Using ProcInfoT.ID here as a standin for InputDate.
				"SELECT ID FROM ProcInfoT WHERE SurgeryApptID = {INT} ORDER BY ProcInfoT.ID DESC; \r\n"
				"OPEN cur \r\n"
				"FETCH NEXT FROM cur INTO @ProcInfoID \r\n"
				"WHILE @@FETCH_STATUS = 0 "
				"BEGIN "
					//Clear out the SurgeryApptID
				"	UPDATE ProcInfoT SET SurgeryApptID = NULL WHERE ID = @ProcInfoID; \r\n"
					//TES 11/28/2007 - PLID 27953 - Now, search for an appointment that can be the new SurgeryApptID
					// for this ProcInfo.
					//TES 11/25/2008 - PLID 32066 - Also, attempt to update the Surgeon field based on the new
					// SurgeryApptID.  NOTE: This query must be kept in sync with the query in UpdateProcInfoSurgeon()
				"	DECLARE @nSurgeryApptID INT \r\n"
				"	SELECT @nSurgeryApptID = (SELECT TOP 1 AppointmentsT.ID "
				"	FROM AppointmentsT INNER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"	INNER JOIN AppointmentPurposeT ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
				//TES 8/24/2009 - PLID 35322 - Include "Minor Procedure" appointments (Category 3).
				"	WHERE AppointmentsT.Status <> 4 AND AptTypeT.Category IN (3,4) AND AppointmentPurposeT.PurposeID IN "
				"	(SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID = @ProcInfoID) "
				"	AND AppointmentsT.ID <> {INT} AND AppointmentsT.PatientID = (SELECT PatientID FROM ProcInfoT WHERE ID = @ProcInfoID) "
				"	AND AppointmentsT.ID NOT IN (SELECT SurgeryApptID FROM ProcInfoT WHERE SurgeryApptID Is Not Null) "
				"	ORDER BY AppointmentsT.Date DESC); \r\n"
				"	UPDATE ProcInfoT SET SurgeryApptID = @nSurgeryApptID WHERE ProcInfoT.ID = @ProcInfoID "
				"	DECLARE @nLinkedProviderID INT \r\n"
				"	SELECT @nLinkedProviderID = ProviderID "
				"		FROM ResourceProviderLinkT INNER JOIN AppointmentResourceT ON ResourceProviderLinkT.ResourceID = "
				"		AppointmentResourceT.ResourceID "
				"		WHERE ProviderID Is Not Null AND AppointmentResourceT.AppointmentID = @nSurgeryApptID \r\n"
				"	UPDATE ProcInfoT SET SurgeonID = @nLinkedProviderID "
				"		WHERE ProcInfoT.ID = @ProcInfoID AND COALESCE(@nLinkedProviderID,-1) <> -1 \r\n"
				"	FETCH NEXT FROM cur INTO @ProcInfoID \r\n"
				//Cleanup
				"END \r\n"
				"CLOSE cur\r\n"
				"DEALLOCATE cur\r\n", nItemID, nItemID, nItemID);
		}
		break;
	default:
		break;
	}

}

BOOL PhaseTracking::CheckWarnPersonLicenses(long nPersonID, CString strPersonType)
{
	try {

		ECredentialWarning eCredWarning = CheckPersonCertifications(nPersonID);

		if(eCredWarning != ePassedAll) {

			CString str;

			CString strName = "";
			// (d.thompson 2009-12-23) - PLID 36706 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}",nPersonID);
			if(!rs->eof) {
				strName = AdoFldString(rs, "Name","");
				strName.TrimRight();
			}
			rs->Close();

			if(eCredWarning == eFailedLicenseExpired) {

				CString strLicenses;
				// (d.thompson 2009-12-23) - PLID 36706 - Parameterized
				_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expired: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
					"WHERE PersonID = {INT} AND ExpDate < Convert(datetime,(Convert(nvarchar,GetDate(),1)))",nPersonID);
				while(!rs2->eof) {
					strLicenses += AdoFldString(rs2, "ExpiredLicense","");
					strLicenses += "\n";
					rs2->MoveNext();
				}
				rs2->Close();

				str.Format("The default %s (%s) for this ladder has the following expired licenses:\n\n%s\n"
					"Do you still wish to use this %s for this ladder?",strPersonType,strName,strLicenses,strPersonType);
			}
			else if(eCredWarning == eFailedLicenseExpiringSoon) {

				//check if a license will expire within the given day range
				long nLicenseWarnDayRange = GetRemotePropertyInt("DefaultASCLicenseWarnDayRange",30,0,"<None>",true);

				CString strLicenses;
				// (d.thompson 2009-12-23) - PLID 36706 - Parameterized
				_RecordsetPtr rs2 = CreateParamRecordset("SELECT '''' + Name + ''' - Expires on: ' + Convert(nvarchar,ExpDate,1) AS ExpiredLicense FROM PersonCertificationsT "
					"WHERE PersonID = {INT} AND ExpDate < DateAdd(day, {INT}, Convert(datetime,(Convert(nvarchar,GetDate(),1))))",nPersonID,nLicenseWarnDayRange);
				while(!rs2->eof) {
					strLicenses += AdoFldString(rs2, "ExpiredLicense","");
					strLicenses += "\n";
					rs2->MoveNext();
				}
				rs2->Close();

				str.Format("The following licenses are about to expire for the default %s (%s) for this ladder:\n\n%s\n"
					"Do you still wish to use this %s for this ladder?",strPersonType,strName,strLicenses,strPersonType);
			}

			if(IDNO == MsgBox(MB_YESNO,str)) {
				CString strInfo;
				strInfo.Format("You can choose a different %s in the Procedure Information Center for this ladder.",strPersonType);
				MsgBox(strInfo);
				return FALSE;
			}
		}

	}NxCatchAll("Error validating licensing.");

	return TRUE;
}

// (a.walling 2006-10-23 17:50) - PLID 20421 - Pass in a relevant AppointmentID to include cancellation info in the task.
void SyncTodoWithLadder(IN long nLadderID, OPTIONAL IN long nAppointmentIDForReason /* = -1 */)
{
	// (c.haag 2013-12-02) - PLID 59821 - Call the API version, which also does license checking
	_bstr_t ladderID = IDToString(nLadderID);
	_bstr_t appointmentIDForReason = NullableIDToString(nAppointmentIDForReason);

	// Call the function
	GetAPI()->SyncTodoWithLadder(GetAPISubkey(), GetAPILoginToken(), ladderID, appointmentIDForReason);
}

void SyncLadderWithTodo(long nTodoID)
{
	//What step was this?
	// (c.haag 2008-06-10 17:14) - PLID 11599 - Converting to use new todo data structure
	// (j.jones 2008-09-03 12:40) - PLID 31236 - parameterized this query
	_RecordsetPtr rsStepID = CreateParamRecordset("SELECT RegardingID, Done, dbo.GetTodoAssignToIDString(ToDoList.TaskID) AS AssignedToIDs, PersonID, Remind FROM ToDoList WHERE TaskID = {INT} AND RegardingType = {INT}", nTodoID, ttTrackingStep);
	if(rsStepID->eof) return;
	long nStepID = AdoFldLong(rsStepID, "RegardingID");
	CArray<long,long> anAssignTo;
	ParseDelimitedStringToLongArray(AdoFldString(rsStepID, "AssignedToIDs"), " ", anAssignTo);
	long nPatientID = AdoFldLong(rsStepID, "PersonID");
	CString strDateTime = FormatDateTimeForSql(AdoFldDateTime(rsStepID, "Remind"), dtoDate);
	_variant_t varDone = rsStepID->Fields->GetItem("Done")->Value;
	rsStepID->Close();
	if(varDone.vt == VT_NULL) {
		//Make sure the step isn't done, and the user and deadline are the same.
		//(e.lally 2007-05-10) PLID 25112 - combine into one execute statement		
		// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step

		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM StepsAssignToT WHERE StepID = {INT}", nStepID);
		int i=0;
		for(i=0;i<anAssignTo.GetSize();i++) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO StepsAssignToT (UserID, StepID) "
				"VALUES ({INT}, {INT})", (long)anAssignTo.GetAt(i), nStepID);
		}
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE StepsT SET ActiveDate = {STRING} "
			"WHERE ID = {INT}", strDateTime, nStepID);
		
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM EventAppliesT WHERE StepID = {INT}", nStepID);
		// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
	}
	else {
		//Make sure the user is correct.
		// (j.jones 2008-12-01 14:05) - PLID 30830 - we now support multiple users per step
		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM StepsAssignToT WHERE StepID = {INT}", nStepID);
		int i=0;
		for(i=0;i<anAssignTo.GetSize();i++) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO StepsAssignToT (UserID, StepID) "
				"VALUES ({INT}, {INT})", (long)anAssignTo.GetAt(i), nStepID);
		}
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE StepsT SET ActiveDate = {STRING} "
			"WHERE ID = {INT}", strDateTime, nStepID);
		// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

		//Now, if the step isn't already done, mark it done.
		// (j.jones 2008-09-03 12:40) - PLID 31236 - parameterized this query		
		_RecordsetPtr rsHasRecords = CreateParamRecordset("SELECT TOP 1 StepID FROM EventAppliesT WHERE StepID = {INT}", nStepID);
		// (z.manning 2008-12-30 17:33) - PLID 32557 - We should be marking the step done if there 
		// are not any records in EventAppliesT. 31236 messed this up.
		if(rsHasRecords->eof) {
			CreateAndApplyEvent(ET_MarkedDoneFromTodo, nPatientID, COleDateTime::GetCurrentTime(), -1, true, nStepID);
		}
		rsHasRecords->Close();
	}

	// (j.jones 2006-04-13 11:12) - per t.schneider, this tablechecker is unnecessary
	/*
	CTableChecker tblToDo(NetUtils::TodoList);
	tblToDo.Refresh();
	*/

	if(GetActivePatientID() == nPatientID) {
		CMainFrame *pMain = GetMainFrame();
		if(pMain) {
			CNxTabView *pView = pMain->GetActiveView();
			if(pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) {
				if( ((CPatientView*)pView)->GetActiveTab() == PatientsModule::ProcedureTab) {
					((CPatientView*)pView)->GetActiveSheet()->Refresh();
				}
			}
		}
	}
				
}

bool IsActionLinkable(PhaseAction nAction) 
{
	// (c.haag 2013-11-26) - PLID 59832 - Call from the tracking API
	NexTech_Accessor::PhaseAction phaseAction;
	if (!ToAPIPhaseAction(nAction, phaseAction)) {
		return false;
	}
	// Call the function
	return (VARIANT_FALSE == GetAPI()->IsTrackingActionLinkable(phaseAction)) ? false : true;
}

bool IsEventLinkable(EventType nType)
{
	// (c.haag 2013-11-26) - PLID 59832 - Call from the tracking API
	NexTech_Accessor::EventType eventType;
	if (!ToAPIEventType(nType, eventType)) {
		return false;
	}
	// Call the function
	return (VARIANT_FALSE == GetAPI()->IsTrackingEventLinkable(eventType)) ? false : true;
}

// (d.thompson 2010-01-15) - PLID 36702 - Parameterized and improved the speed of this function.
BOOL MergeLadders(long nLadderIDToBeMerged, long nLadderIDToMergeInto, long nPatientID) {

	//start a transaction
	// (d.thompson 2010-01-15) - PLID 36702 - This batch is now parameterized
	CString strMergeBatch;
	CNxParamSqlArray args;
	long nProcInfoIDToBeMerged, nProcInfoIDToMergeInto;
	long nPicIDToBeMerged = -1, nPicIDToMergeInto = -1;
	try {
		//if we got here, then they've already been warned, so proceed on
		//get the corresponding procIds for each ladder because we are going to need them
		// (d.thompson 2010-01-15) - PLID 36702 - Merged 3 pre-loop queries into 1 trip.
		_RecordsetPtr rsProcInfo = CreateParamRecordset(
			"SET NOCOUNT ON;\r\n"
			"DECLARE @ProcInfoToMergeInto INT;\r\n"
			"DECLARE @ProcInfoToBeMerged INT;\r\n"
			"SET @ProcInfoToMergeInto = (SELECT ProcInfoID FROM LaddersT WHERE ID = {INT});\r\n"
			"SET @ProcInfoToBeMerged = (SELECT ProcInfoID FROM LaddersT WHERE ID = {INT});\r\n"
			"SET NOCOUNT OFF;\r\n"
			"SELECT @ProcInfoToMergeInto AS ProcInfoToMergeInto, @ProcInfoToBeMerged AS ProcInfoToBeMerged;\r\n"
			"SELECT ProcedureID "
			" FROM ProcInfoT INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
			" INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			" WHERE ProcInfoT.ID = @ProcInfoToBeMerged AND ProcedureT.MasterProcedureID IS NULL;\r\n ", 
			nLadderIDToMergeInto, nLadderIDToBeMerged);

		if (! rsProcInfo->eof) {
			nProcInfoIDToMergeInto = AdoFldLong(rsProcInfo, "ProcInfoToMergeInto");
			nProcInfoIDToBeMerged = AdoFldLong(rsProcInfo, "ProcInfoToBeMerged");
		}
		else {
			MsgBox("Could not find the correct ProcInfoID to merge into, cannot continue");
			return FALSE;
		}

		//first get all the procedures from the ladder to be merged and attach them to the other ladder
		
		//select all the master ProcedureID's from the existing ladder
		// (d.thompson 2010-01-15) - PLID 36702 - Merged into above trip
		//_RecordsetPtr rsProcs = CreateRecordset("SELECT ProcedureID "
		//	" FROM ProcInfoT INNER JOIN ProcInfoDetailsT ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
		//	" INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
		//	" WHERE ProcInfoT.ID = %li AND ProcedureT.MasterProcedureID IS NULL ", nProcInfoIDToBeMerged);
		_RecordsetPtr rsProcs = rsProcInfo->NextRecordset(NULL);

		strMergeBatch = BeginSqlBatch();
		while (!rsProcs->eof) {

			long nProcID = AdoFldLong(rsProcs, "ProcedureID");

			//see if this procedure already exists for the new ladder
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr prsTest = CreateParamRecordset("SELECT ID FROM ProcInfoDetailsT WHERE ProcedureID = {INT} AND ProcInfoID = {INT};", nProcID, nProcInfoIDToMergeInto);
			//if (! ReturnsRecords(...
			if(prsTest->eof) {

				//this procedure doesn't exist in the new ladder, so add it and all of its details
				// (j.armen 2013-07-01 12:20) - PLID 57362 - Idenitate ProcInfoDetailsT
				AddParamStatementToSqlBatch(strMergeBatch, args, "INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID) VALUES ({INT}, {INT})", 
					nProcInfoIDToMergeInto, nProcID);

				//now for the details for just this procedure
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				_RecordsetPtr rsDetails = CreateParamRecordset("SELECT ProcedureT.ID, ProcInfoDetailsT.Chosen "
					" FROM ProcInfoDetailsT  "
					" LEFT JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
					" WHERE ProcedureT.MasterProcedureID = {INT} "
					" AND ProcInfoDetailsT.ProcInfoID = {INT} ", nProcID, nProcInfoIDToBeMerged);
				while(!rsDetails->eof) {
					BOOL bChosen = AdoFldBool(rsDetails, "Chosen");
					// (j.armen 2013-07-01 12:20) - PLID 57362 - Idenitate ProcInfoDetailsT
					AddParamStatementToSqlBatch(strMergeBatch, args, "INSERT INTO ProcInfoDetailsT (ProcInfoID, ProcedureID, Chosen) VALUES ({INT}, {INT}, {INT})", 
						nProcInfoIDToMergeInto, AdoFldLong(rsDetails, "ID"), bChosen == FALSE ? 0 : 1);
					rsDetails->MoveNext();
				}
							
			}
			
			rsProcs->MoveNext();
		}
		rsProcs->Close();


		//now that we have the procedures done, lets work on the appointments
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		_RecordsetPtr rsProcAppts = CreateParamRecordset("SELECT AppointmentID FROM ProcInfoAppointmentsT WHERE ProcInfoID = {INT};", nProcInfoIDToBeMerged);
		while (! rsProcAppts->eof) {
			
			long nApptID = AdoFldLong(rsProcAppts, "AppointmentID");

			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr prsTest = CreateParamRecordset("SELECT ProcInfoID FROM ProcInfoAppointmentsT WHERE ProcInfoID = {INT} AND AppointmentID = {INT}", nProcInfoIDToMergeInto, nApptID);
			//if (! ReturnsRecords(
			if(prsTest->eof) {

				//its not already there, so we are good to go
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				AddParamStatementToSqlBatch(strMergeBatch, args, "INSERT INTO ProcInfoAppointmentsT (ProcInfoID, AppointmentID) VALUES "
					" ({INT}, {INT}) ", nProcInfoIDToMergeInto, nApptID);
			}
			//else we don't need to merge it

			rsProcAppts->MoveNext();
		}
		rsProcAppts->Close();

		// (j.dinatale 2012-07-11 14:12) - PLID 51474 - handle the bills associated with the PIC
		_RecordsetPtr rsBills = CreateParamRecordset("SELECT BillID FROM ProcInfoBillsT WHERE ProcInfoID = {INT};", nProcInfoIDToBeMerged);
		while (!rsBills->eof) {
			long nBillID = AdoFldLong(rsBills, "BillID");

			_RecordsetPtr prsTest = CreateParamRecordset("SELECT ProcInfoID FROM ProcInfoBillsT WHERE ProcInfoID = {INT} and BillID = {INT}", nProcInfoIDToMergeInto, nBillID);
			if(prsTest->eof) {
				AddParamStatementToSqlBatch(strMergeBatch, args, "INSERT INTO ProcInfoBillsT (ProcInfoID, BillID) "
					" VALUES ({INT}, {INT})", nProcInfoIDToMergeInto, nBillID);
			}

			rsBills->MoveNext();
		}
		rsBills->Close();

		//Appointments Done, move onto Payments
		//first check if this procedure already has a payment, because if so, we aren't going to do anything
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		_RecordsetPtr rsPays = CreateParamRecordset("SELECT PayID FROM ProcInfoPaymentsT WHERE ProcInfoID = {INT}", nProcInfoIDToBeMerged);
		while (! rsPays->eof) {

			long nPayID = AdoFldLong(rsPays, "PayID");

			//make sure it doesn't already exist for the procinfo we are merging into
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr prsTest = CreateParamRecordset("SELECT ProcInfoID FROM ProcInfoPaymentsT WHERE ProcInfoID = {INT} AND PayID = {INT}", nProcInfoIDToMergeInto, nPayID);
			//if (! ReturnsRecords(
			if(prsTest->eof) {
				//add it
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				AddParamStatementToSqlBatch(strMergeBatch, args, "INSERT INTO ProcInfoPaymentsT (ProcInfoID, PayID) "
					" VALUES ({INT}, {INT}) ", nProcInfoIDToMergeInto, nPayID);
			}
		
			rsPays->MoveNext();
		}
		rsPays->Close();


		//Payments are done, Quotes are next!
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		_RecordsetPtr rsQuotes = CreateParamRecordset("SELECT QuoteID FROM ProcInfoQuotesT WHERE ProcInfoID = {INT};", nProcInfoIDToBeMerged);
		while (! rsQuotes->eof) {

			long nQuoteID = AdoFldLong(rsQuotes, "QuoteID");

			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr prsTest = CreateParamRecordset("SELECT ProcInfoID FROM ProcInfoQuotesT WHERE ProcInfoId = {INT} and QuoteID = {INT}", nProcInfoIDToMergeInto, nQuoteID);
			//if (! ReturnsRecords(
			if(prsTest->eof) {
				//add it because it doesn't exist already
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				AddParamStatementToSqlBatch(strMergeBatch, args, "INSERT INTO ProcInfoQuotesT (ProcInfoID, QuoteID) "
					" VALUES ({INT}, {INT})", nProcInfoIDToMergeInto, nQuoteID);
			}

			rsQuotes->MoveNext();
		}
		rsQuotes->Close();

		//Quotes are done PicT is next!
		//check to see if there is already a record for this Pic
		//time for EMR
		//first see if this Ladder has an associated EMR

		// (z.manning, 10/23/2006) - PLID 22514 - We used to check for an EMR and custom records license here,
		// but we need to handle this data whether they have a license for it or not.
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		_RecordsetPtr rsGroupID = CreateParamRecordset("SELECT EMRGroupID FROM PicT WHERE ProcInfoID = {INT} AND EMRGroupID IS NOT NULL", nProcInfoIDToBeMerged);
		if (! rsGroupID->eof) {
			long nGroupIDToBeMerged = AdoFldLong(rsGroupID, "EMRGroupID");

			// (j.jones 2006-09-18 11:43) - PLID 22532 - you are NOT allowed to merge FROM a ladder that has a locked EMN

			//check that this isn't a locked EMR
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr rsLocked = CreateParamRecordset("SELECT ID FROM EmrMasterT WHERE Deleted = 0 AND Status = 2 AND EmrGroupID = {INT}", nGroupIDToBeMerged);
			if (! rsLocked->eof) {
				MsgBox("The ladder you are merging from is connected to an EMR with a locked EMN, the ladders will not be merged.\n"
					"You may, however, merge another ladder without any locked EMNs into this ladder.");
				return FALSE;
			}
			rsLocked->Close();

			// (a.walling 2008-06-26 09:05) - PLID 30515 - Ensure no one is modifying these EMNs
			// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			// (j.armen 2013-05-14 12:37) - PLID 56680 - EMN Access Refactoring
			_RecordsetPtr rsInUse = CreateParamRecordset(
				"SELECT ID FROM EmrMasterT\r\n"
				"INNER JOIN EMNAccessT WITH(UPDLOCK, HOLDLOCK) ON EmrMasterT.ID = EMNAccessT.EmnID\r\n"
				"WHERE Deleted = 0 AND EmrGroupID = {INT}", nGroupIDToBeMerged);
			if (!rsInUse->eof) {
				MsgBox("The ladder you are merging from is connected to an EMR with %li EMNs that are currently being modified. The ladders will not be merged.\n"
					"You may, however, merge another ladder without any in-use EMNs into this ladder, or try again later.", 
					long(rsInUse->RecordCount));
				return FALSE;
			}
			rsInUse->Close();
			
			//the Ladder we are merging does have an EMR associated with it, so add the associated EMNs to the other ladder
			//find the GroupID of the EMR associated with the other ladder
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			rsGroupID = CreateParamRecordset("SELECT EMRGroupID FROM PicT WHERE ProcInfoID = {INT} AND EMRGroupID IS NOT NULL" , nProcInfoIDToMergeInto);
			if (rsGroupID->eof) {

				//this means that the other ladder doesn't have an EMR associated with it, so we need to associate this EMR with it
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE PicT SET EmrGroupID = {INT} WHERE ProcInfoID = {INT}", nGroupIDToBeMerged, nProcInfoIDToMergeInto);
				AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE PicT SET EmrGroupID = NULL WHERE ProcInfoID = {INT}", nProcInfoIDToBeMerged);
				
			}
			else {
				
				long nGroupIDToMergeInto = AdoFldLong(rsGroupID, "EMRGroupID");

				// (a.walling 2008-06-26 09:08) - PLID 30515 - Failsafe
				// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				// (j.armen 2013-05-14 12:38) - PLID 56680 - EMN Access Refactoring
				AddParamStatementToSqlBatch(strMergeBatch, args, 
					"IF EXISTS(\r\n"
					"	SELECT 1 FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
					"	WHERE EmnID IN (SELECT ID FROM EMRMasterT WHERE EmrGroupID = {INT}))\r\n"
					"BEGIN\r\n"
					"	RAISERROR('EMN cannot be merged; it is being modified by another user.', 16, 43)\r\n"
					"	ROLLBACK TRAN\r\n"
					"	RETURN\r\n"
					"END", nGroupIDToBeMerged);

				AddParamStatementToSqlBatch(strMergeBatch, args, "Update EMRMasterT SET EMRGroupID = {INT} WHERE EMRGroupID = {INT}", nGroupIDToMergeInto, nGroupIDToBeMerged);
				AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE PicT SET EmrGroupID = NULL WHERE ProcInfoID = {INT}", nProcInfoIDToBeMerged);
				
				// (j.jones 2006-04-26 09:37) - PLID 20064 - we now simply mark these records as being deleted
				//AddStatementToSqlBatch(strMerge, "DELETE FROM EMRGroupsT WHERE ID = %li ", nGroupIDToBeMerged);
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE EMRGroupsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = {STRING} WHERE ID = {INT} ", GetCurrentUserName(), nGroupIDToBeMerged);
				// (c.haag 2008-08-05 11:09) - PLID 30853 - Migrate problems to the target EMN
				// (c.haag 2009-05-11 17:32) - PLID 28494 - Update the new EMR problem linking table instead
				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
				AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE EMRProblemLinkT SET EMRRegardingID = {INT} WHERE EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT} ", nGroupIDToMergeInto, eprtEmrEMR, nGroupIDToBeMerged);
			}
		}
		rsGroupID->Close();


		//If medications ever get implemented, here is the code that "should" work for it, notice I say "should because since its not implemented, 
		// I can't test it and maybe the way I think it would be implemented is wrong
		/*_RecordsetPtr rsMeds = CreateRecordset("SELECT MedicationsID FROM ProcInfoMedicationsT WHERE ProcInfoID = %li", nProcInfoIDToBeMerged);
		while (! rsMeds->eof) {

			long nMedID = AdoFldLong(rsMeds, "MedicationID");

			if (! ReturnsRecords("SELECT ProcInfoID FROM ProcInfoMedicationsT WHERE ProcInfoId = %li and MedicationID = %li", nProcInfoIDToMergeInto, nMedID)) {

				//add it because it doesn't exist already
				ExecuteSql("INSERT INTO ProcInfoMedicationT (ProcInfoID, MedicationID) "
					" VALUES (%li, %li)", nProcInfoIDToMergeInto, nMedID);
			}

			rsMeds->MoveNext();
		}
		rsMeds->Close();*/

		//documents
		//get the PicIDs
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		_RecordsetPtr rsPic = CreateParamRecordset("SELECT ID FROM PicT WHERE ProcInfoID = {INT}", nProcInfoIDToBeMerged);
		if (! rsPic->eof) {
			nPicIDToBeMerged = AdoFldLong(rsPic, "ID", -1);
		}
		rsPic->Close();

		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		rsPic = CreateParamRecordset("SELECT ID FROM PicT WHERE ProcInfoID = {INT}", nProcInfoIDToMergeInto);
		if (! rsPic->eof) {
			nPicIDToMergeInto = AdoFldLong(rsPic, "ID", -1);
		}
		rsPic->Close();

		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE MailSent SET PicID = {INT} WHERE PicID = {INT}", nPicIDToMergeInto, nPicIDToBeMerged);
		// (r.galicki 2008-10-07 14:55) - PLID 31555 - Adding Labs tab to PIC, need to handle merge
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE LabsT SET PicID = {INT} WHERE PicID = {INT}", nPicIDToMergeInto, nPicIDToBeMerged);

		//ok now that we are done with all the separate tables, its time for the updates!!
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized these all into the same method as above.
		//BillID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET BillID = (SELECT BillID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND ProcInfoT.BillID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		// (j.jones 2009-08-07 09:01) - PLID 7397 - support case history ID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET CaseHistoryID = (SELECT CaseHistoryID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND CaseHistoryID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);
		
		//SurgeonID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET SurgeonID = (SELECT SurgeonID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND SurgeonID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		//CoSurgeonID // (a.walling 2006-11-28 12:59) - PLID 21003 - Merge the CoSurgeon info as well
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET CoSurgeonID = (SELECT CoSurgeonID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND CoSurgeonID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		//NurseID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET NurseID = (SELECT NurseID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND ProcInfoT.NurseID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);
		
		//AnesthesiologistID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET AnesthesiologistID = (SELECT AnesthesiologistID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND ProcInfoT.AnesthesiologistID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		//Anesthesia
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET Anesthesia = (SELECT Anesthesia FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND (ProcInfoT.Anesthesia IS NULL OR ProcInfoT.Anesthesia = ''); \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		// (z.manning, 4/25/2006, PLID 19093) - This pulls from the appt now.
		//Arrival Time
		//strTemp = strUpdate;
		//strUpdate.Format(" %s UPDATE ProcInfoT SET ArrivalTime = (SELECT ArrivalTime FROM ProcInfoT INNERQ WHERE INNERQ.ID = %li) "
		//	" WHERE ProcInfoT.ID = %li AND ProcInfoT.ArrivalTime IS NULL; \r\n", strTemp, nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		//SurgeryApptID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET SurgeryApptID = (SELECT SurgeryApptID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND ProcInfoT.SurgeryApptID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		//ActiveQuoteID
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE ProcInfoT SET ActiveQuoteID = (SELECT ActiveQuoteID FROM ProcInfoT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE ProcInfoT.ID = {INT} AND ProcInfoT.ActiveQuoteID IS NULL; \r\n", nProcInfoIDToBeMerged, nProcInfoIDToMergeInto);

		//Notes
		AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE LaddersT SET Notes = (SELECT Notes FROM LaddersT INNERQ WHERE INNERQ.ID = {INT}) "
			" WHERE LaddersT.ID = {INT} AND (LaddersT.Notes IS NULL OR LaddersT.Notes = ''); \r\n", nLadderIDToBeMerged, nLadderIDToMergeInto);

		//finally, we have to update any events for this step that are already marked done
		// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
		_RecordsetPtr rsSteps = CreateParamRecordset("SELECT StepsT.ID, StepTemplateID FROM StepsT INNER JOIN EventAppliesT ON "
			" StepsT.ID = EventAppliesT.StepID "
			" WHERE LadderID = {INT}", nLadderIDToBeMerged);
		while (! rsSteps->eof) {
			long nStepIDToBeMerged = AdoFldLong(rsSteps, "ID");
			long nStepTemplateID = AdoFldLong(rsSteps, "StepTemplateID");

			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr rsNewStep = CreateParamRecordset("SELECT ID FROM StepsT WHERE LadderID = {INT} AND StepTemplateID = {INT}",
				nLadderIDToMergeInto, nStepTemplateID);
			if (!rsNewStep->eof) {
				long nStepIDToMergeInto = AdoFldLong(rsNewStep, "ID");

				// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
				_RecordsetPtr prsTest = CreateParamRecordset("SELECT ID FROM EventAppliesT WHERE StepID = {INT}", nStepIDToMergeInto);
				//if (! ReturnsRecords(
				if(prsTest->eof) {
					// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
					AddParamStatementToSqlBatch(strMergeBatch, args, "UPDATE EventAppliesT SET StepID = {INT} WHERE StepID = {INT}", nStepIDToMergeInto, nStepIDToBeMerged);
				}
			}
			rsNewStep->Close();

			rsSteps->MoveNext();
		}
		rsSteps->Close();

	}NxCatchAllCall("Error Generating Merge String", return FALSE;);

		//now that we are done with all of our updates, the only thing left to do is check which steps can be marked done and then delete the current Ladder
		//we are going to do this inside its own transaction and then leave the deleting of the PIC up to itself because it uses its own transactions
		try{ 
#ifdef _DEBUG 
			//(e.lally 2008-04-10)- Switched to our CMsgBox dialog
			CMsgBox dlg(NULL);
			dlg.msg = strMergeBatch;
			dlg.DoModal();
#endif
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Removed some commented out code
			// (a.walling 2008-06-26 09:11) - PLID 30515 - Use transaction control directly within SQL statement
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			ExecuteParamSqlBatch(GetRemoteData(), strMergeBatch, args);
			//ExecuteSqlStd(strMerge);
		}NxCatchAllCall("Error Merging Ladders", /*RollbackTrans("MergeLadders");*/ return FALSE;);

		//we'll put a try catch block around it, just in case
		try {
			//delete the first ladder first
			//get the PicID
			if (nPicIDToBeMerged != -1) {
				long nPicID = nPicIDToBeMerged;
				CDWordArray arNewCDSInterventions;
				// (c.haag 2010-07-20 17:34) - PLID 30894 - This function will fire a labs table checker
				//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
				AttemptRemovePICRecord(nPicID, arNewCDSInterventions, TRUE);
				GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
			}
			else {
				//ummmm...this shouldn't happen
				DeleteProcInfo(nProcInfoIDToBeMerged);
				// (c.haag 2010-07-20 17:34) - PLID 30894 - We need to send a labs table refresh
				// in case any labs changed due to the preceeding code.
				// (r.gonet 09/02/2014) - PLID 63221 - Send the patient ID too
				CClient::RefreshLabsTable(nPatientID, -1);
			}
		}NxCatchAllCall("Error Deleting PIC", return FALSE);

		try {
			//now apply the any new things we have for our ladder we are keeping
			//taken directly from AddPtntProcedure
			// (d.thompson 2010-01-15) - PLID 36702 - Parameterized
			_RecordsetPtr rsSteps = CreateParamRecordset("SELECT StepsT.ID, Skippable, StepTemplatesT.ID AS StepTemplateID, StepTemplatesT.Action FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID WHERE LadderID = {INT} ORDER BY StepsT.StepOrder ASC", nLadderIDToMergeInto);
			CArray<Step,Step> arSteps;
			while(!rsSteps->eof) {
				Step step;
				step.nStepID = AdoFldLong(rsSteps, "ID", -1);
				step.bSkippable = AdoFldBool(rsSteps, "Skippable", FALSE);
				step.nStepTemplateID = AdoFldLong(rsSteps, "StepTemplateID", -1);
				step.nAction = AdoFldLong(rsSteps, "Action", -1);
				arSteps.Add(step);
				rsSteps->MoveNext();
			}
			rsSteps->Close();

			for(int i = 0; i < arSteps.GetSize(); i++) {
				Step step = arSteps[i];
				// (c.haag 2013-11-26) - PLID 59822 - We no longer need the ladder, patient, step template or actions. Those are
				// calculated within TryToCompleteStep.
				if(TryToCompleteStep(step.nStepID, true)) {
					//We completed one, and it handled all skipping and whatever else, 
					//so skip the rest of the ladder.
					i = arSteps.GetSize();
				}
				else if(!step.bSkippable) {
					//Well, there's nothing else to be done on this ladder.
					i = arSteps.GetSize();
				}
			}
		}NxCatchAllCall("Error autoapplying Steps", return FALSE);
	
		//we succeded if we are here
			
		
	return TRUE;	

}

EventType GetEventType(PhaseAction Action)
{
	// (c.haag 2013-11-27) - PLID 59835 - Call from the tracking API
	NexTech_Accessor::PhaseAction accessorAction;
	bool bSuccess = ToAPIPhaseAction(Action, accessorAction);
	if (!bSuccess) {
		//We shouldn't be able to get here!
		AfxThrowNxException("Invalid action %i passed to GetEventType()!", (int)Action);
	}

	NexTech_Accessor::EventType eventType = GetAPI()->GetTrackingEventType(accessorAction);
	EventType pracEventType;
	bSuccess = ToPracticeEventType(eventType, pracEventType);
	if (!bSuccess) {
		//We shouldn't be able to get here!
		AfxThrowNxException("Invalid event type %i returned from GetEventType()!", (int)eventType);
	}
	return pracEventType;
}

CString GetIDFieldNameFromAction(PhaseAction eAction)
{
	switch(eAction) 
	{
		case PA_Bill:
		case PA_Quote:
		case PA_QuoteCPT:
		case PA_QuoteInventory:
		case PA_BillCPT:
		case PA_BillInventory:
			return "BillsT.ID";
			break;

		case PA_WriteTemplate:
			return "MailSent.MailID";
			break;
			
		case PA_WritePacket:
			return "MergedPacketsT.ID";
			break;
		
		case PA_ScheduleAptCategory:
		case PA_ActualAptCategory:
		case PA_ScheduleAptType:
		case PA_ActualAptType:
			return "AppointmentsT.ID";
			break;

		case PA_Ladder:
			return "LaddersT.ID";
			break;

		// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
		// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
		case PA_CreateEMRByCollection:
		case PA_CreateEMRByTemplate:
			return "EmrMasterT.ID";
			break;

		case PA_Payment:
			return "LineItemT.ID";
			break;

		case PA_Manual:
		case PA_QuoteSurgery:
		case PA_BillSurgery:
			return "";
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	return "";
}

// (z.manning, 07/27/2007) - PLID 26846 - Determines if a given action is licensed or not.
BOOL IsTrackingActionLicensed(long nActionID)
{
	switch(nActionID)
	{
		// (z.manning, 07/27/2007) - PLID 26846 - It's not necessary to have a letter writing license
		// to merge from the PIC, so there's no need to restrict the merge template & packet steps
		// based on any licensing.
		case PA_WriteTemplate:
		case PA_WritePacket:
		case PA_Manual:
			return TRUE;
			break;

		case PA_Quote:
		case PA_QuoteSurgery:
		case PA_QuoteCPT:
		case PA_QuoteInventory:
			return g_pLicense->CheckForLicense(CLicense::lcQuotes, CLicense::cflrSilent);
			break;

		case PA_Bill:
		case PA_BillSurgery:
		case PA_BillCPT:
		case PA_BillInventory:
		case PA_BillQuote:
		case PA_Payment:
			return g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent);
			break;

		case PA_ScheduleAptCategory:
		case PA_ScheduleAptType:
		case PA_ActualAptCategory:
		case PA_ActualAptType:
			//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
			return g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent);
			break;

		case PA_Ladder:
			// (z.manning, 07/27/2007) - PLID 26846 - It's silly that we'd be doing something tracking
			// related without a tracking license, but let's check anyway.
			return g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent);
			break;

		// (j.jones 2010-04-30 10:01) - PLID 38353 - renamed PA_CreateEMR to PA_CreateEMRByCollection (existing behavior),
		// and added PA_CreateEMRByTemplate, which lets you match by template instead of collection
		case PA_CreateEMRByCollection:
		case PA_CreateEMRByTemplate:
			return (g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
			break;
	}

	// (z.manning, 07/27/2007) - PLID 26846 - Looks like someone added a new tracking action
	// but didn't handle it here. Even if the new action isn't a licensed feature, it should be
	// handled here for completeness. And if it is licensed at all, then I smell a return.
	ASSERT(FALSE);
	return TRUE;
}

// (z.manning, 08/30/2007) - PLID 18359 - Returns true if the give ladder template ID can be safely
// deleted, false otherwise.
BOOL CanDeleteLadderTemplate(long nLadderTemplateID, BOOL bSilent, CWnd *pwndParent)
{
	// (d.moore 2007-06-08 11:08) - PLID 14670 - Modified query to use the ProcedureLadderTemplateT
	//  this allows a procedure to be associated with multiple ladders.
	// (c.haag 2009-01-09 10:58) - PLID 32571 - List the procedures in use. Show inactive procedures
	// with a special note.
	// (d.thompson 2009-12-23) - PLID 36706 - Joined both queries in this function into one trip.
	_RecordsetPtr prs = CreateParamRecordset("SELECT ProcedureT.Name, ProcedureT.Inactive FROM ProcedureLadderTemplateT INNER JOIN "
		"ProcedureT ON ProcedureLadderTemplateT.ProcedureID = ProcedureT.ID INNER JOIN "
		"LadderTemplatesT ON ProcedureLadderTemplateT.LadderTemplateID = LadderTemplatesT.ID "
		"WHERE LadderTemplatesT.ID = {INT} ORDER BY ProcedureT.Name;\r\n"
		"SELECT TOP 1 StepsT.ID "
		"FROM StepsT INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
		"WHERE LadderTemplateID = {INT};\r\n", nLadderTemplateID, nLadderTemplateID);
	if (!prs->eof) {
		if(!bSilent) {
			CString strMsg = "This ladder cannot be deleted because it in use by the following procedures:\r\n\r\n";
			FieldsPtr f = prs->Fields;
			int n = 0;
			while (!prs->eof) {
				if (++n > 20) {
					strMsg += "<more omitted>\r\n";
					break;
				}
				CString strProc = AdoFldString(f, "Name");
				BOOL bInactive = AdoFldBool(f, "Inactive");
				strMsg += strProc;
				if (bInactive) {
					strMsg += " (Inactive)";
				}
				strMsg += "\r\n";
				prs->MoveNext();
			}
			pwndParent->MessageBox(strMsg);
		}
		return FALSE;
	}

	// (d.thompson 2009-12-23) - PLID 36706 - Combined this query into a single trip above, just check for empty.
	prs = prs->NextRecordset(NULL);
	if(!prs->eof) {
		if(!bSilent) {
			pwndParent->MessageBox("You may not delete a ladder that is being tracked by a patient.");
		}
		return FALSE;
	}

	return TRUE;
}

//TES 11/25/2008 - PLID 32066 - Call this function to update a ProcInfo record's Surgeon to match the ProcInfo's 
// Surgery appointment (tied through the resource).  You may pass in -2 to for EITHER nProcInfoID OR nSurgeryApptID;
// if you do, you must pass in a valid ID for the other one, and the code will find all associated records and update
// them appropriately.  This code also checks whether the preference for this is even on, and does nothing if it isn't.
// You may pass in a pointer to a SQL batch, if you do then this function will add to the batch, but will not actually
// change any data.
// (d.thompson 2009-12-31) - PLID 36740 - Parameterized the query batch. If pstrBatch is non-null, pargs must be non-null
// (c.haag 2013-11-27) - PLID 59831 - We no longer allow a batch mode that returns SQL. API functions must not reveal
// database structure information.
void UpdateProcInfoSurgeon(long nProcInfoID, long nSurgeryApptID)
{
	// (c.haag 2013-11-27) - PLID 59831 - Call from the tracking API.
	_bstr_t procInfoID = IDToString(nProcInfoID);
	_bstr_t surgeryApptID = IDToString(nSurgeryApptID);

	// Call the function
	GetAPI()->UpdateProcInfoSurgeon(GetAPISubkey(), GetAPILoginToken(), procInfoID, surgeryApptID);
}

// (c.haag 2013-12-17) - PLID 60018 - This should be called when procedures are added to an EMR outside 
// of built-in EMR functionality. This function will update related tracking data
void HandleNewEmrProcedures(int nProcInfoID, CArray<long, long> &arProcIDs)
{
	_bstr_t procInfoID = IDToString(nProcInfoID);
	CArray<_bstr_t> bstrAry;
	foreach (long n, arProcIDs)
	{
		bstrAry.Add(IDToString(n));
	}

	NexTech_Accessor::_TrackingTransactionPtr pTransaction = GetAPI()->HandleNewEmrProcedures(
		GetAPISubkey(), GetAPILoginToken(), procInfoID, Nx::SafeArray<BSTR>::From(bstrAry));
	// Complete any incomplete actions
	PhaseTracking::CompletePendingActions(pTransaction);
}

} //end namespace