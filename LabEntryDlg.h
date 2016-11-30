#if !defined(AFX_LABENTRYDLG_H__07B00003_F817_4B70_A3B1_F707610CC09B__INCLUDED_)
#define AFX_LABENTRYDLG_H__07B00003_F817_4B70_A3B1_F707610CC09B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LabEntryDlg.h : header file
//
// (c.haag 2007-03-16 10:02) - PLID 21622 - Added an "Add" button for diagnoses and removed
// the old dropdown

/////////////////////////////////////////////////////////////////////////////
// CLabEntryDlg dialog
#include "LabRequisitionsTabDlg.h"
#include "LabResultsTabDlg.h"
#include "EMNLab.h"

//TES 11/17/2009 - PLID 36190 - Many controls are now split onto a separate Requisition tab.  Therefore, a whole lot of member variables and
// functions have been moved from this file to LabRequisitionDlg.h

//TES 11/20/2009 - PLID 36191 - Likewise, many member variables and functions have now been moved to LabResultsTabDlg.h

enum LabType; // (r.galicki 2008-10-17 11:55) - PLID 31552

// (a.vengrofski 2010-05-12 16:42) - PLID <38547> - added enum for clarity.
enum LabLinkListColumns {
	lllID = 0,
	lllName = 1,
	//TES 6/24/2011 - PLID 44261 - Took out columns for HL7 Settings
};

//TES 11/25/2009 - PLID 36193 - Used to pass shared information to our sub dialogs.
struct SharedLabInformation {
	CString strFormTextID;
	CString strPatientFirst;
	CString strPatientMiddle;
	CString strPatientLast;
	CString strSourceActionID;
	CString strSourceDetailID;
	CString strPicID;
	CString strSourceDataGroupID;
	CString strOrderSetID;
	CString strSourceDetailImageStampID; // (z.manning 2010-02-26 16:42) - PLID 37540
};

class CLabEntryDlg : public CNxDialog
{

// Construction
public:	
	// (z.manning 2008-10-06 10:31) - PLID 21094 - Changed the constructor to take a CWnd
	CLabEntryDlg(CWnd* pParent);   // standard constructor
	~CLabEntryDlg();  
	void SetPatientID(long nPatientID);
	//TES 11/25/2009 - PLID 36193 - Instead of setting a lab ID, we now set an initial lab ID, because this dialog may have multiple labs on it.
	void SetInitialLabID(long nLabID);
	long GetInitialLabID() const; // (c.haag 2010-07-15 17:23) - PLID 34338 - Returns the initial lab ID
	void SetLabProcedureID(long nLabProcedureID);
	void SetLabProcedureType(LabType ltType);	// (r.galicki 2008-10-17 11:46) - PLID 31552

	// (z.manning 2013-10-30 16:34) - PLID 59240 - Removed this function and moved its logic to SetLabProcedureID
	//void SetLabProcedureGroup(long nLabProcedureID); // (r.gonet 03/29/2012) - PLID 45856 - Inits the lab procedure group associated with a lab procedure id

	// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
	void SetDefaultLocationID(long nLocationID);
	// (r.gonet 07/22/2013) - PLID 57683 - Sets the providers from the source EMN.
	void SetEMNProviders(CDWordArray &dwProviderIDs);

	//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
	void SetInitialAnatomicLocation(long nAnatomicLocationID, long nAnatomicQualifierID, AnatomySide asSide);

	// (z.manning 2008-10-06 14:43) - PLID 21094
	void SetSourceActionID(const long nSourceActionID);
	void SetSourceDetailID(const long nSourceDetailID);
	// (z.manning 2009-02-27 10:08) - PLID 33141 - SourceDataGroupID
	void SetSourceDataGroupID(const long nSourceDataGroupID);
	void SetSourceDetailImageStampID(const long nSourceDetailImageStampID);

	// (z.manning 2009-05-08 09:39) - PLID 28554 - Sets the order set ID if this lab is part of an order set.
	void SetOrderSetID(const long nOrderSetID);

	// (b.spivey, August 28, 2013) - PLID 46295 - Allows the Results dialog to get stuff from the req dialog. 
	CString GetAnatomicLocationByLabID(long nLabID); 

	// (z.manning 2009-05-19 09:17) - PLID 28554 - Set the default to be ordered text
	void SetDefaultToBeOrdered(const CString &strToBeOrdered);

	// (j.jones 2010-05-06 11:19) - PLID 38524 - moved age calculation into its own function
	void CalculateAndDisplayPatientAge();

	bool HasDataChanged(); // (a.walling 2006-07-14 10:25) - PLID 21073 True if any data has been saved
	bool HasOpenedReport(); // (a.walling 2006-07-14 10:25) - PLID 21073 True if any data has been saved

	int m_nPIC; // (r.galicki 2008-10-08 12:02) - PLID 31555 - Added PIC ID to labs dialog.  Needs member variable when saving a new lab.
	LabType m_ltType; // (r.galicki 2008-10-20 09:55) - PLID 31552

	BOOL m_bHideResults; // (z.manning 2008-10-28 12:36) - OKUD 31838
	
	// (z.manning 2008-10-06 12:05) - PLID 21094 - Keep track of newly created labs
	// (z.manning 2008-10-07 11:24) - PLID 31561 - Changed this to an array of EMNLab objects
	CArray<EMNLab,EMNLab> m_aryNewLabs;

	// (z.manning 2008-10-08 11:39) - PLID 31613 - Remember the last saved existing lab
	//TES 12/17/2009 - PLID 36622 - The lab dialog can have multiple labs on it now, so this needs to be an array.
	CArray<EMNLab,EMNLab> m_arSavedExistingLabs;
//TES 11/25/2009 - PLID 36191 - All EMR Problem code moved to CLabRequisitionDlg

	//TES 11/20/2009 - PLID 36191 - Called by CLabResultsDlg when a result has changed, requiring a ToDo Alarm to be created
	//TES 8/6/2013 - PLID 51147 - Pass in a TodoPriority as well, and whether that priority was loaded from an abnormal flag
	void AddToDoDescription(const CString &strToDoDesc, TodoPriority priority, bool bIsFlag);

	//TES 11/25/2009 - PLID 36193 - Access the current form number
	CString GetFormNumber();
	//TES 11/25/2009 - PLID 36193 - Access the saved form number (used for auditing)
	CString GetSavedFormNumber();
	// (j.jones 2010-10-07 13:37) - PLID 40743 - allowed updating the saved form number
	void SetSavedFormNumber(CString strFormNumberTextID);

	//TES 11/25/2009 - PLID 36193 - Called by the requisition dialog when it saves a new EMN-originated lab
	void AddNewEmnLab(EMNLab lab);

	//TES 11/25/2009 - PLID 36193 - We no longer differentiate between new and existing, we leave that up to our sub dialogs.
	BOOL Save();

	//TES 11/30/2009 - PLID 36452 - Called by the requisitions tab when the specimen changes, meaning that we need to update the results tab.
	void HandleSpecimenChange(long nLabID, const CString &strNewSpecimen);

	//TES 11/30/2009 - PLID 36193, 36452 - Keeps both tabs showing the same Lab.
	void SetCurrentLab(long nLabID);
	
	//TES 2/1/2010 - PLID 37143 - Called by the Requisition when the Receiving Lab changes, selects the given report in the dropdown.  
	// -1 = System report, -2 = overall default (i.e., what they set as Default in the EditReportPickerDlg).
	void SetRequestForm(long nCustomReportNumber);

	//TES 7/27/2012 - PLID 51849 - Same function for the Results Form
	void SetResultsForm(long nCustomReportNumber);

	// (z.manning 2010-03-16 15:50) - PLID 37439 - Function to add a new lab specimen to the current lab group
	void AddNew();
	BOOL m_bAddNewLabImmediatelyOnLoad;

	// (j.jones 2010-04-19 12:45) - PLID 37875 - added variable that the Labs Needing Attention dialog
	// will use to let this dialog know that another lab will open upon closing
	BOOL m_bNextLabWillOpenOnClose;

	// (a.vengrofski 2010-05-12 16:55) - PLID <38547> - Function that will chanage the HL7 Link when the recving lab is changed.
	void SetHL7Link(long nLabID);

	// (z.manning 2010-05-13 11:35) - PLID 37405 - Returns the currently active lab req dialog
	CLabRequisitionDlg* GetActiveLabRequisitionDlg();
	CLabRequisitionDlg* GetLabRequisitionDlgByID(const long nLabID);

	// (c.haag 2010-07-15 17:23) - PLID 34338 - This function will open this lab on the screen. You should use this instead of DoModal().
	// (j.jones 2010-09-01 09:40) - PLID 40094 - added a location ID to be used on new labs that are created
	int OpenLab(long nPatientID, long nProcedureID, LabType ltType, long nInitialLabID, long nOrderSetID, const CString& strToBeOrdered, long nPicID, BOOL bNextLabWillOpenOnClose, BOOL bModal, HWND hwndNotify, long nNewLabLocationID = GetCurrentLocationID(), long nResultID = -1);
	// (c.haag 2010-07-15 17:23) - PLID 34338 - This must be called every time a lab window is closed
	void CloseCleanup(int result);

	// (b.spivey - January 20, 2014) - PLID 46370 - Added InitialResultID
	void SetInitialResultID(long nResultID);
	long GetInitialResultID();

	// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if there is at least one result and all the results are completed
	// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
	// because it may come in handy someday and may even help developers better understand the lab code.
	//BOOL IsLabCompleted(long nLabID, OUT long& nCompletedBy, OUT COleDateTime& dtCompletedDate);
	// (c.haag 2010-12-15 16:34) - PLID 41825 - Returns TRUE if this lab has any unsigned results
	// (c.haag 2011-01-27) - PLID 41825 - I deprecated the need for this code; but I'm keeping it commented out
	// because it may come in handy someday and may even help developers better understand the lab code.
	//BOOL LabHasUnsignedResults(long nLabID);

protected:
// Dialog Data
	//{{AFX_DATA(CLabEntryDlg)
	enum { IDD = IDD_LAB_ENTRY_DLG };
	CNxIconButton	m_cancelBtn;
	CNxIconButton	m_saveResumePrevBtn;
	CNxIconButton	m_saveAddNewBtn;
	CNxEdit	m_nxeditLabPatient;
	CNxEdit	m_nxeditLabGender;
	CNxEdit	m_nxeditLabAge;
	CNxStatic	m_nxstaticPatientLabel;
	CNxStatic	m_nxstaticGenderLabel;
	CNxStatic	m_nxstaticAgeLabel;
	CNxIconButton m_btnPrint;
	CNxIconButton m_btnPreview;
	NxButton m_nxbtnLabel;
	NxButton m_nxbtnRequestForm;
	NxButton m_nxbtnResultsForm;
	CNxEdit m_nxeditFormNumber;
	CNxIconButton m_EditFormNumberFormatBtn;
	// (j.jones 2010-05-06 09:22) - PLID 38520 - supported input date
	CNxEdit m_nxeditInputDate;
	CNxIconButton m_btnCreateTodo; // (z.manning 2010-05-13 09:18) - PLID 37405
	CNxIconButton m_btnCreateRecall; // (b.savon 2012-02-28 12:24) - PLID 48443
	// (a.vengrofski 2010-05-13 11:25) - PLID <38547>
	NxButton m_nxbtnSendHL7;
	CNxIconButton m_btnEditLabelPrinterBtn;
	//}}AFX_DATA

	//TES 11/20/2009 - PLID 36190
	NxTab::_DNxTabPtr m_tab;

	//TES 1/29/2010 - PLID 34439 - A dropdown list of the available Lab Request Forms.
	NXDATALIST2Lib::_DNxDataListPtr m_pRequestFormsList;

	//TES 7/27/2012 - PLID 51071 - A dropdown list of the available Lab Results Forms.
	NXDATALIST2Lib::_DNxDataListPtr m_pResultFormsList;

	// (a.vengrofski 2010-05-11 15:00) - PLID <38547> - A dropdown list of the available Lab Links
	NXDATALIST2Lib::_DNxDataListPtr m_pLabLinkList;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	DWORD m_color;
	CBrush m_brush;

	HWND m_hwndNotify; // (c.haag 2010-07-15 17:23) - PLID 34338 - The window to notify when the lab is closing
	long m_nPatientID;
	//(e.lally 2007-08-08) PLID 26978 - Cache the patient's name to save in the record
	// (d.singleton 2013-10-25 14:36) - PLID 59195 - need patient id in the patient name field of the lab entry dialog
	CString m_strPatientFirst, m_strPatientMiddle, m_strPatientLast, m_strUserDefinedID;
	// (j.jones 2010-05-06 11:17) - PLID 38524 - cache the birthdate
	_variant_t m_varBirthDate;
	long m_nLabProcedureID;
	long m_nLabProcedureGroupID; // (r.gonet 03/29/2012) - PLID 45856 - Stores the lab procedure group id assoc with this lab's lab procedure.
	long m_nInitialLabID;
	BOOL m_bIsNewLab;
	long m_nGender;
	// (z.manning 2010-01-13 11:23) - PLID 22672 - Age is now a string
	CString m_strAge;

	// (j.gruber 2008-10-02 10:56) - PLID 31332 - take out any variables not used anymore
	CString m_strSavedFormNumberTextID;
	_variant_t m_varCurDate;
	//TES 11/20/2009 - PLID 36190 - Sets the current date here and on any subdialogs
	void SetCurrentDate(_variant_t varCurDate);

	// (z.manning, 07/28/2006) - PLID 21576 - We save the last form number counter value in case
	// the user cancels and we need to revert back.
	long m_nLastFormNumberCount;

	// (z.manning 2009-05-08 09:39) - PLID 28554 - Used if this lab is part of an order set.
	_variant_t m_varOrderSetID;

	// (b.spivey - January 20, 2014) - PLID 46370 - Added InitialResultID
	long m_nInitialResultID; //46370

	//TES 11/20/2009 - PLID 36190
	//TES 11/25/2009 - PLID 36193 - Changed from CLabRequisitionDlg to ClabRequisitionsTabDlg
	CLabRequisitionsTabDlg &m_dlgRequisitions;
	//TES 11/20/2009 - PLID 36191
	CLabResultsTabDlg &m_dlgResults;

	void LoadNew();
	void LoadExisting();
	BOOL IsLabValid();
	// (j.gruber 2008-10-01 10:54) - PLID 31332 - changed to not take a description
	// (z.manning 2010-05-12 15:48) - PLID 37405 - Added lab ID parameter
	void SpawnToDo(const long nLabID);
	
	// (z.manning 2008-11-04 09:01) - PLID 31904 - Changed this function to be able to print multiple forms at once
	void PrintLabForms(BOOL bPreview);
	
	bool m_bDataChanged; // returned by HasDataChanged() a.walling PLID 21073
	bool m_bOpenedReport; // returned by HasOpenedReport()

	CStringArray m_arystrToDoDesc;
	//TES 8/6/2013 - PLID 51147 - Track the priority for any todos we might spawn, as well as whether that priority came from an abnormal flag
	TodoPriority m_TodoPriority;
	bool m_bTodoHasFlag;

	//TES 11/20/2009 - PLID 36190 - Show the appropriate subdialog based on the tab control's state.
	void ReflectCurrentTab();
	
	CString GetNewFormNumber();

	//TES 11/30/2009 - PLID 36193, 36452 - Tell our subtabs to reflect the new form number, when it changes.
	void HandleNewFormNumber();

	// (r.gonet 10/11/2011) - PLID 46437 - Generate the barcodes for this lab
	void GenerateBarcodes(OUT CString &strBarcode, OUT CString &strOverflowBarcode);

	// (z.manning 2010-11-16 15:36) - PLID 41499 - Will enable or disable all the buttons that can close this dialog.
	void EnableClosingButtons(BOOL bEnable);
	class CLabEntryButtonDisabler
	{
	public:
		CLabEntryDlg *m_pdlgLabEntry;
		CLabEntryButtonDisabler(CLabEntryDlg *pdlg) {
			m_pdlgLabEntry = pdlg;
			m_pdlgLabEntry->EnableClosingButtons(FALSE);
		}

		~CLabEntryButtonDisabler() {
			m_pdlgLabEntry->EnableClosingButtons(TRUE);
		}
	};

	// (z.manning 2008-10-06 14:41) - PLID 21094
	_variant_t m_varSourceActionID;
	_variant_t m_varSourceDetailID;
	// (z.manning 2009-02-27 10:09) - PLID 33141
	_variant_t m_varSourceDataGroupID;
	_variant_t m_varSourceDetailImageStampID; // (z.manning 2010-02-26 16:43) - PLID 37540

	//TES 2/1/2010 - PLID 37143 - The overall default Request Form (-2 if we need to look it up).
	long m_nDefaultRequestForm;
	//TES 2/1/2010 - PLID 37143 - An accessor that returns the overall default Request Form, without needless data access.
	long GetDefaultRequestForm();

	//TES 7/27/2012 - PLID 51849 - The overall default Results Form (-2 if we need to look it up).
	long m_nDefaultResultForm;
	//TES 7/27/2012 - PLID 51849 - An accessor that returns the overall default Results Form, without needless data access.
	long GetDefaultResultForm();

	// Generated message map functions
	//{{AFX_MSG(CLabEntryDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSaveAndAddNew();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSaveAndResumePrevious();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPrint();
	afx_msg void OnPreview();
	BOOL AutoExportTheLab();// (a.vengrofski 2010-06-10 16:28) - PLID <38544> - function to export and/or batch a newly minted lab.
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnSelectTabLabEntryTabs(short newTab, short oldTab);
	afx_msg void OnEditFormNumberFormat();
	afx_msg void OnKillfocusFormNumber();
	afx_msg void OnBnClickedCancelLab();
	afx_msg void OnRequestFormCheck();
	afx_msg void OnSize(UINT nType, int cx, int cy); // (z.manning 2010-04-29 12:39) - PLID 38420
	afx_msg void OnDestroy();
	afx_msg void OnSendHL7Check();// (a.vengrofski 2010-05-11 15:41) - PLID <38547>
	afx_msg void OnSelChosenLabLinkList(LPDISPATCH lpRow);// (a.vengrofski 2010-05-11 15:41) - PLID <38547> 
	afx_msg void OnBnClickedLabEntryCreateTodo();
	void SelChangingHl7List(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);// (a.vengrofski 2010-05-11 15:41) - PLID <38547>
	afx_msg void OnBnClickedEditLabelPrintSettings();
public:
	afx_msg void OnBnClickedNxbtnCreateRecall();
	afx_msg void OnResultFormCheck();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABENTRYDLG_H__07B00003_F817_4B70_A3B1_F707610CC09B__INCLUDED_)
