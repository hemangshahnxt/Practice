#pragma once

#include "NewCropSoapFunctions.h"
#include "GlobalParamUtils.h"
#include "NewCropUtils.h"

// CNewCropBrowserDlg dialog

// (j.jones 2009-02-16 12:24) - PLID 33053 - created

// (j.jones 2014-07-29 10:09) - PLID 63085 - forward declarations
// in the Accessor namespaces allow us to remove .h includes
namespace NexTech_Accessor
{
	enum NewCropRequestedPageType;
}

//the NewCropActionType enum represents a type of action to send to the browser
enum NewCropActionType {

	ncatInvalid = -1,
	ncatNewBrowserWindow = 1,
	ncatAccessPatientAccount,
	// (j.gruber 2009-05-15 14:17) - PLID 28541 - renewal requests
	ncatProcessRenewalRequest,
	//ncatSubmitPrescription, (temporarily removed)	
};

// (c.haag 2009-05-13 16:33) - PLID 34257 - Rather than passing an integer into the
// NXM_NEWCROP_BROWSER_DLG_CLOSED message, we now pass in this structure. The fields
// nOldAllergiesInfoID and nNewAllergiesInfoID are new as of this item.
struct NewCropBrowserResult
{
	long nEMNID;
	long nPatientID;
	long nOldAllergiesInfoID;
	long nNewAllergiesInfoID;
	// (c.haag 2009-05-15 09:53) - PLID 34271 - Added current medications ID's
	long nOldCurrentMedicationsInfoID;
	long nNewCurrentMedicationsInfoID;
	// (c.haag 2010-02-18 09:55) - PLID 37424 - The array of newly added patient prescriptions during the session
	CArray<NewCropPatientMedication,NewCropPatientMedication&> aNewlyAddedPatientPrescriptions;
};

// (j.armen 2012-06-06 12:39) - PLID 50830 - Removed GetMinMaxInfo
class CNewCropBrowserDlg : public CNxDialog
{

public:
	// (j.gruber 2009-03-30 17:39) - PLID 33736 - added nEMNID and nPatientID
	// (j.gruber 2009-03-30 17:25) - PLID 33728 - added window ptr for closing message to be sent to
	// (j.gruber 2009-05-15 17:13) - PLID 28541 - added strXMl for renewal responses
	// (j.gruber 2009-06-08 10:16) - PLID 34515 - added role
	// (j.jones 2013-01-04 08:52) - PLID 49624 - removed role, and added UserDefinedID
	CNewCropBrowserDlg(long nPatientPersonID, long nPatientUserDefinedID, long nEMNID, CWnd* pWndToSendClosingMsgTo, CString strXML, CWnd* pParent);
	virtual ~CNewCropBrowserDlg();

	//m_ncatDefaultAction tells us the default action to fire when
	//starting the browser, and defines what m_nActionID represents
	NewCropActionType m_ncatDefaultAction;
	long m_nActionID;
	long m_nPatientPersonID;	// (j.jones 2009-04-01 11:24) - PLID 33736
	long m_nPatientUserDefinedID;	// (j.jones 2013-01-04 08:52) - PLID 49624

	CString m_strDefaultWindowText;

	// (j.gruber 2009-05-15 17:20) - PLID 28541 - moved from PerformDefaultAction
	// (j.gruber 2009-05-15 17:39) - PLID 28541 - added ability to say whether to close nxscript and route page
	// (j.jones 2013-04-16 17:00) - PLID 56275 - the requested page enum is now in the API
	BOOL GeneratePatientLoginXML(CString &strXmlDocument, NexTech_Accessor::NewCropRequestedPageType ncrptRequestedPage, BOOL bAddNcScriptFooter = TRUE);

	// (j.gruber 2009-05-15 17:16) - PLID 28541 - pass in xml for renewal responses
	CString m_strPassedInXML;

	//these need to be public for use in NewWindow2
	IWebBrowser2Ptr m_pBrowser;
	BOOL m_bIsPopupWindow;

	// (c.haag 2009-05-13 16:43) - PLID 34257 - This was the active EMR
	// allergy info ID when the dialog was opened
	long m_nOriginalAllergiesInfoID;
	// (c.haag 2009-05-15 09:54) - PLID 34271 - This was the active EMR
	// current medications info ID when the dialog was opened
	long m_nOriginalCurrentMedicationsInfoID;

// Dialog Data
	enum { IDD = IDD_NEWCROP_BROWSER_DLG };
	CNxIconButton m_btnUpdateClose;
	CNxEdit m_editBrowserStatus;

protected:
	// (c.haag 2010-02-18 09:55) - PLID 37424 - The array of newly added patient prescriptions in this session.
	// It is later passed on to the NewCropBrowserResult object when the form is dismissed.
	CArray<NewCropPatientMedication,NewCropPatientMedication&> m_aNewlyAddedPatientPrescriptions;

protected:
	// (a.walling 2009-04-07 13:20) - PLID 33306 - const CString reference
	void SendXMLToBrowser(CString strURL, const CString &strXmlDocument);

	void PerformDefaultAction();

	// (j.jones 2009-08-13 14:31) - PLID 35213 - added a boolean to make sure the
	// default action function absolutely cannot be called twice in one browser window
	BOOL m_bHasPerformedDefaultAction;

	//OnUpdatePatientAccount should be called before closing this dialog,
	//and will return TRUE upon success, FALSE if a problem occurred
	void OnUpdatePatientAccount();

	//used to track new browser windows
	CArray<CNewCropBrowserDlg*, CNewCropBrowserDlg*> m_paryPopupWindows;

	BOOL GetNewCropAllergyHistory(CPtrArray *paryAllergies);

	// (j.gruber 2009-03-30 17:39) - PLID 33051 - sync back from newcrop function
	// (j.gruber 2009-05-13 17:33) - PLID 34259 - split prescriptions from medications
	BOOL GetNewCropPrescriptionHistory(CPtrArray *paryMedications, CPtrArray *paryPrescriptions);

	// (j.jones 2009-09-10 17:16) - PLID 35508 - I renamed CreateNewDrug to EnsureDrugExists, which will create the drug
	// if it doesn't exist, update the drug if the name changed, or simply return the existing drug.
	// Returns the DrugID, or -1 if an error occurred.
	long EnsureDrugExists(NewCropPatientMedication *pMed, CString &strFullDrugDescription);

	// (j.gruber 2009-05-13 08:40) - PLID 34251 - sync back allergies
	//added a discontinued flag
	// (a.walling 2010-01-14 13:41) - PLID 36888 - Pass in the old RxNorm concept unique identifier
	// (j.jones 2012-10-17 10:32) - PLID 53179 - Added bCurHasNoAllergiesStatus, the current value of PatientsT.HasNoAllergies.
	// This function will clear that status if it adds an allergy.
	BOOL UpdatePatientAllergy(BOOL bIsNew, long nAllergyID, NewCropPatientAllergy *pAllergy, CString strAllergyNotes,
		CString &strSqlBatch, long &nAuditTransactionID, CNxParamSqlArray &args,
		BOOL &bCurHasNoAllergiesStatus, BOOL &bNeedsReviewed, BOOL bDiscontinued,
		CString strOldNotes = "", CString strOldRXCUI = "");

	long CreateNewAllergy(NewCropPatientAllergy *pAllergy, CString strAllergyDescription);

	// (j.jones 2010-01-21 17:03) - PLID 37004 - added ability to calculate the NDC number,
	// using the NewCropID as the FirstDataBank ID - now FDBID
	// (j.jones 2010-01-26 17:40) - PLID 37078 - moved to FirstDataBankUtils
	//CString ChooseNDCNumberFromFirstDataBank(NewCropPatientMedication *pMed, CString strFullDescription);

	// (j.jones 2010-01-21 17:46) - PLID 37004 - we now require FirstDataBank here
	BOOL m_bHasEnsuredFirstDataBankOnce;
	
	// (j.gruber 2009-03-30 11:36) - PLID 33736 - added for EMN linking
	long m_nEMNID;

	// (j.gruber 2009-03-30 12:21) - PLID 33728 - need window to send message back to that we closed
	CWnd* m_pWndToSendClosingMsgTo;

	// (j.jones 2013-01-04 13:46) - PLID 49624 - added special handling for when we access the API
	// to handle the NewCrop login, this function will turn expected soap exceptions into messageboxes,
	// return TRUE if it did, FALSE if the error still needs handled
	BOOL HandleAPIComError(_com_error &e);
	
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnBtnUpdateAndClose();
	afx_msg void OnCancel();
	void OnBeforeNavigate2NewcropBrowser(LPDISPATCH pDisp, VARIANT* URL, VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData, VARIANT* Headers, BOOL* Cancel);
	void OnNavigateComplete2NewcropBrowser(LPDISPATCH pDisp, VARIANT* URL);
	void OnDocumentCompleteNewcropBrowser(LPDISPATCH pDisp, VARIANT* URL);
	void OnNewWindow2(LPDISPATCH* ppDisp, BOOL* Cancel);
	virtual void PostNcDestroy();
	// (j.jones 2009-03-04 12:48) - PLID 33332 - added OnPostShowWindow to be called
	// after the dialog has loaded
	LRESULT OnPostShowWindow(WPARAM wParam, LPARAM lParam);
};