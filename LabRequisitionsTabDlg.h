#pragma once


// CLabRequisitionsTabDlg dialog
//TES 11/25/2009 - PLID 36193 - Created
class CLabRequisitionDlg;
class CLabEntryDlg;
struct SharedLabInformation;
enum LabType;

class CLabRequisitionsTabDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabRequisitionsTabDlg)

public:
	CLabRequisitionsTabDlg(CWnd* pParent);   // standard constructor
	virtual ~CLabRequisitionsTabDlg();

public:
	void PostLoad();
	//TES 11/25/2009 - PLID 36193 - Functions that are passed through to the CLabRequisitionDlgs.
	void SetPatientID(long nPatientID);
	void SetInitialLabID(long nInitialLabID);
	void SetLabProcedureID(long nLabProcedureID);
	void SetLabProcedureType(LabType ltType);
	void SetDefaultToBeOrdered(const CString &strToBeOrdered);
	void SetCurrentDate(const _variant_t &varCurDate);
	// (j.jones 2010-01-28 10:36) - PLID 37059 - added ability to default the location ID
	void SetDefaultLocationID(long nLocationID);
	// (r.gonet 07/22/2013) - PLID 57683 - Sets the providers from the source EMN.
	void SetEMNProviders(CDWordArray &dwProviderIDs);

	//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
	void SetInitialAnatomicLocation(long nAnatomicLocationID, long nAnatomicQualifierID, AnatomySide asSide);

	//TES 11/25/2009 - PLID 36193 - Get the earliest Biopsy date of any requisition (used for calculating patient age)
	COleDateTime GetFirstBiopsyDate();

	// (j.jones 2010-05-06 09:37) - PLID 38520 - get the earliest input date of any requisition
	COleDateTime GetFirstInputDate();

	// (j.jones 2016-02-22 09:57) - PLID 68348 - get the first lab's provider
	// "first" is an arbritrary choice here
	long GetFirstLabProviderID();

	// (j.jones 2016-02-22 09:57) - PLID 68348 - get the first lab's location
	// "first" is an arbritrary choice here
	long GetFirstLocationID();

	//TES 11/25/2009 - PLID 36193 - Loads a single new requisition
	void LoadNew();

	//TES 11/25/2009 - PLID 36193 - Loads all existing requisitions with the same form number as m_nInitialLabID
	void LoadExisting();

	//TES 11/25/2009 - PLID 36193 - Add a new requisition.
	// (z.manning 2010-03-22 11:52) - PLID 37439 - Added bUseDefaultAnatomicLocation
	void AddNew(BOOL bUseDefaultAnatomicLocation);

	//TES 11/25/2009 - PLID 36193 - Calls IsLabValid() for all requisitions.
	BOOL AreAllLabsValid();
	// (r.gonet 06/11/2013) - PLID 56389 - Returns the number of new, unsaved labs
	long GetNewLabsCount();

	//TES 11/25/2009 - PLID 36193 - Save all our labs, takes in shared information like form number and patient name.
	//TES 12/1/2009 - PLID 36452 - Added an output number for the id of the new lab (if any) that gets created.
	//TES 10/31/2013 - PLID 59251 - Added arNewCDSInterventions, any interventions triggered in this function will be added to it
	void Save(long &nAuditTransID, IN const SharedLabInformation &sli, OUT long &nNewLabID, IN OUT CDWordArray &arNewCDSInterventions);

	//TES 11/25/2009 - PLID 36193 - Update the requisition's tab label with the new specimen identifier
	void HandleSpecimenChange(CLabRequisitionDlg *pChangedDlg, const CString &strNewSpecimen);

	//TES 11/30/2009 - PLID 36193 - Reflect the form number on our tab labels.
	void HandleFormNumberChange(const CString &strNewFormNumber);

	//TES 11/30/2009 - PLID 36193 - Make sure we're reflecting the same current lab as the Results tab.
	void SetCurrentLab(long nLabID);

	// (z.manning 2010-05-05 10:31) - PLID 37190 - Returns the currently selected lab req dialog
	CLabRequisitionDlg* GetActiveLabRequisitionDlg();
	CLabRequisitionDlg* GetLabRequisitionDlgByID(const long nLabID);

	// (z.manning 2010-05-13 14:02) - PLID 37405
	void ClearAllPendingTodos();
	void DeleteAllPendingTodos();
	long GetTotalPendingTodoCount();

	// (z.manning 2010-06-02 13:08) - PLID 38976
	char GetNextSpecimenCode();

// Dialog Data
	enum { IDD = IDD_LAB_REQUISITIONS_TAB_DLG };

protected:
	//TES 11/25/2009 - PLID 36193 - Our tab control.
	NxTab::_DNxTabPtr m_tab;
	//TES 11/25/2009 - PLID 36193 - All of our child dialogs.
	CArray<CLabRequisitionDlg*,CLabRequisitionDlg*> m_arRequisitions;

	//TES 11/25/2009 - PLID 36193 - Our parent
	CLabEntryDlg* m_pLabEntryDlg;

	//TES 11/25/2009 - PLID 36193 - Variables for passing to our children.
	long m_nInitialLabID;
	long m_nPatientID;
	long m_nLabProcedureID;
	// (j.jones 2010-01-28 10:55) - PLID 37059 - added location ID
	long m_nDefaultLocationID;
	// (r.gonet 07/22/2013) - PLID 45187 - The primary providers from the source EMN. 
	CDWordArray m_dwEMNProviderIDs;
	LabType m_ltType;
	CString m_strToBeOrdered;
	_variant_t m_varCurDate;

	//TES 2/15/2010 - PLID 37375 - Added the ability to pre-load Anatomic Location information
	long m_nInitialAnatomicLocationID, m_nInitialAnatomicQualifierID;
	AnatomySide m_asInitialAnatomicSide;

	//TES 11/25/2009 - PLID 36193 - Show the dialog corresponding to the currently loaded tab.
	void ReflectCurrentTab();

	//TES 11/25/2009 - PLID 36193 - Set all of our stored variables (like m_nPatientID) on the given dialog.
	void PreloadRequisition(CLabRequisitionDlg* pDlg);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	void OnSelectTabRequisitionTabs(short newTab, short oldTab);
	afx_msg void OnSize(UINT nType, int cx, int cy); // (z.manning 2010-04-29 12:39) - PLID 38420
};
