#pragma once

// (j.jones 2013-01-09 13:25) - PLID 54530 - created, and moved functions here from MainFrame

// (c.haag 2010-02-18 17:23) - PLID 37384 - Lets the user apply a new prescription to the current medications list.
// Returns TRUE if any data changed.
// (j.jones 2010-08-23 09:17) - PLID 40178 - takes in a NewCropGUID
// (j.jones 2011-05-02 15:24) - PLID 43350 - this now takes in a Sig for the current med
// (j.jones 2013-01-08 12:00) - PLID 47303 - added parent wnd
// (j.jones 2013-01-09 11:55) - PLID 54530 - no longer need the Sig nor strNewCropGUID
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL ReconcileCurrentMedicationsWithOneNewPrescription(long nPatientID, long nNewPrescriptionID, OLE_COLOR clrBack, CWnd *pParentWnd, IN OUT CDWordArray &arNewCDSInterventions);
// (c.haag 2010-02-18 10:03) - PLID 37424 - Lets the user apply new prescriptions imported from our NewCrop
// integration to the current medications list. Returns TRUE if any data changed.
// (j.jones 2013-01-08 10:19) - PLID 47302 - added parent wnd
// (j.jones 2013-01-09 11:55) - PLID 54530 - added ability to add multiple prescriptions by prescription ID, not necessarily NewCrop IDs
//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
BOOL ReconcileCurrentMedicationsWithMultipleNewPrescriptions(long nPatientID, CArray<long, long> &aryNewPrescriptionIDs, const CStringArray& astrNewCropRxGUIDs, OLE_COLOR clrBack, CWnd *pParentWnd, IN OUT CDWordArray &arNewCDSInterventions);