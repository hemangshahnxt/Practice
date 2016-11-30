#pragma once

namespace RecallUtils
{
	const static long INTELLISENSE_RECALL_HEADING_ID = -1;
	const static CString INTELLISENSE_RECALL_HEADING_NAME = "***  Recall Templates Linked to Diagnosis: ";

	const static long BLANK_HEADING_ID = -2;
	const static CString BLANK_HEADING_CODE = "";
	const static CString BLANK_HEADING_DESC = "";

	enum eRecallStatusID {
		eNone = 0,
		eDiscontinued = 1,
		eComplete = 2,
		eScheduled = 3,
		eNeedToSchedule = 4,
		ePastDue = 5,
	};

	enum eRecallStatusColors {
		eNoneColor = RGB(0, 0, 0),
		eDiscontinuedColor = RGB(192,192,192),
		eCompleteColor = RGB(204, 255, 204),
		eScheduledColor = RGB(128, 255, 128),
		eScheduledTextColor = RGB(0, 120, 0),
		eNeedToScheduleColor = RGB(204, 204, 255),
		ePastDueColor = RGB(255, 102, 102),
		eNeedToScheduleTextColor = RGB(255, 0, 0),
	};

	struct RecallListMap{
		RecallListMap()
		{
			nDiagCodeID = -1;
			nRecallTemplateID = -1;
		}

		RecallListMap(long DiagCodeID, long RecallTemplateID)
		{
			nDiagCodeID = DiagCodeID;
			nRecallTemplateID = RecallTemplateID;
		}

		long nDiagCodeID, nRecallTemplateID;
	};

	// (j.armen 2012-03-13 10:17) - PLID 48634 - Passing a pat id only updates records for the current pat.
	// You will still need to append a where clause to filter the select.
	CSqlFragment SelectRecalls();
	// (a.walling 2013-11-25 11:44) - PLID 60007 - We don't need to update recalls every time we call SelectRecalls -- split into UpdateRecalls
	void UpdateRecalls(long nUpdatePatID = -1);

	// (j.jones 2016-02-22 09:03) - PLID 68376 - removed obsolete functions that are now in the API

	// (a.walling 2013-12-13 10:20) - PLID 60010 - Creating recalls now in the shared RecallUtils namespace
	// (j.jones 2016-02-17 16:59) - PLID 68348 - recalls now have an optional provider ID and location ID, either can be -1
	void CreateRecalls(long nPatientID, long nEMRID, long nApptID, long nLabID, long nProviderID, long nLocationID,
		CArray<RecallListMap, RecallListMap>& aryRecallListMap);

	// (a.walling 2013-12-13 10:20) - PLID 60010 - Gets any templates/diagcode pairs that can be automatically created for an EMR
	// (r.gonet 04/13/2014) - PLID 60870 - Added bIsAutomatic so that this function could be shared between the EMR saving code
	// and the EMN recall manual creation dialog.
	// (j.jones 2016-02-18 08:41) - PLID 68390 - this now also loads the providerID and locationID
	// (j.jones 2016-02-19 11:03) - PLID 68378 - renamed and added a bool to control auto-creating recalls
	void CalculateRecallsForEMR(bool bAutoCreate, long nEMRGroupID, OUT long &nProviderID, OUT long &nLocationID, CArray<RecallListMap, RecallListMap>& aryRecallListMap);
	
	eRecallStatusColors GeneratePatientRecallStatusTextColor(const long& nPatientID); //(a.wilson 2012-3-5) PLID 48485

	// (j.jones 2016-02-18 10:53) - PLID 68350 - checks to see if a recall and appt. share the same provider/location,
	// warns if they do not, asks the user to continue.
	// Return value is true if the link is ok to create.
	bool CanLinkRecallAndApptByProviderLocation(CWnd *pParent, long nRecallID, long nAppointmentID, OUT bool &bWasWarned);
};


