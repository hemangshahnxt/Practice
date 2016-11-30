//{{AFX_INCLUDES()
#include "MirrorImageButton.h"
//}}AFX_INCLUDES
#include "MirrorPatientImageMgr.h"
#if !defined(AFX_EMRSUMMARYDLG_H__A991219B_DA13_47BE_AD59_9E5CF3E8B588__INCLUDED_)
#define AFX_EMRSUMMARYDLG_H__A991219B_DA13_47BE_AD59_9E5CF3E8B588__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRSummaryDlg.h : header file
//

// (j.jones 2013-05-16 15:18) - PLID 56596 - replaced EMN.h with a forward declare
class CEMN;

/////////////////////////////////////////////////////////////////////////////
// CEMRSummaryDlg dialog

class CEMRSummaryImageLoad
{
public:
	class CEMRSummaryDlg* m_pMsgWnd;
	CString m_strRemoteID;
	long m_nImageIndex;
	long m_nImageCount;
	long m_nPatientID;
	CMirrorImageButton* m_pButton;
	// (c.haag 2010-02-23 15:00) - PLID 37364 - This object is used for accessing Mirror images from the thread.
	CMirrorPatientImageMgr* m_pMirrorImageMgr;

	// (a.walling 2010-03-09 14:16) - PLID 37640 - Moved to cpp
	CEMRSummaryImageLoad(CEMRSummaryDlg* pMsgWnd, CMirrorPatientImageMgr* pMirrorImageMgr, CString strRemoteID, long nImageIndex, long nImageCount, long nPatientID, CMirrorImageButton *pButton);
	~CEMRSummaryImageLoad();
};

// (j.jones 2008-06-26 09:15) - PLID 21168 - since the lists are now configurable,
// we will build up separate lists per column with text and potentially colors
class CEMRSummaryListInfoObject
{
public:
	CString m_strLabel;
	COLORREF m_cColor;

	CEMRSummaryListInfoObject(CString strLabel = "", COLORREF cColor = RGB(0,0,0))
	{
		m_strLabel = strLabel;
		m_cColor = cColor;
	}
};

// (c.haag 2008-12-16 16:03) - PLID 32467 - This represents a problem in the EMR
// summary list. We used to load them on demand, but because the top section can
// persistently show all problems, we now load all of them for the patient in one
// query.
class CEMRSummaryProblem
{
public:
	// Regarding type
	EMRProblemRegardingTypes m_Type;
	// If this is an EMR-level problem, this is the EMR ID. Otherwise, -1.
	long m_nEMRID;
	// EMN ID's (When showing EMN summary info, we must include EMR problems). An
	// EMR can have multiple EMN's.
	CArray<long,long> m_anEMNIDs;
	// Display text
	CString m_strDisplayText;

public:
	void operator =(const CEMRSummaryProblem& p)
	{
		m_nEMRID = p.m_nEMRID;
		m_anEMNIDs.Copy(p.m_anEMNIDs);
		m_strDisplayText = p.m_strDisplayText;
	}
	CEMRSummaryProblem() {
		m_nEMRID = -1;
	}
	CEMRSummaryProblem(const CEMRSummaryProblem& p)
	{
		*this = p;
	}

public:
	BOOL IsForEMN(long nEMNID) const {
		for (int i=0; i < m_anEMNIDs.GetSize(); i++) {
			if (m_anEMNIDs[i] == nEMNID) {
				return TRUE;
			}
		}
		return FALSE;
	}
};

class CEMRSummaryDlg : public CNxDialog
{
// Construction
public:
	CEMRSummaryDlg(CWnd* pParent);   // standard constructor

	long m_nPatientID;
	long m_nEMRID;
	long m_nEMNID;

	// (j.jones 2008-06-13 09:05) - PLID 30385 - added patient image controls
	// (j.jones 2008-06-19 09:24) - PLID 30436 - added m_btnConfigure
// Dialog Data
	//{{AFX_DATA(CEMRSummaryDlg)
	enum { IDD = IDD_EMR_SUMMARY_DLG };
	CNxIconButton	m_btnConfigure;
	CMirrorImageButton	m_imageButton;
	CMirrorImageButton	m_imageButtonLeft;
	CMirrorImageButton	m_imageButtonRight;
	CMirrorImageButton	m_imageButtonUpperLeft;
	CMirrorImageButton	m_imageButtonUpperRight;
	CMirrorImageButton	m_imageButtonLowerLeft;
	CMirrorImageButton	m_imageButtonLowerRight;
	CNxIconButton	m_btnPtImageNext;
	CNxIconButton	m_btnPtImageLast;
	CNxIconButton	m_btnNextEMN;
	CNxIconButton	m_btnPrevEMN;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnEMRSummaryPreview;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRSummaryDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_PatientList;
	NXDATALIST2Lib::_DNxDataListPtr m_EMNList;

	void PopulatePatientList();
	void PopulateEMNList();
	void AddToEMNList(long nEMNID);

	void GenerateEMNIDList();

	CArray<long,long> m_aryEMNIDs;

	long m_nCurEMNIndex;

	// (c.haag 2010-02-23 09:51) - PLID 37364 - This object is used for loading patient Mirror images
	CMirrorPatientImageMgr* m_pMirrorImageMgr;

	void UpdateArrows();

	BOOL m_bDisplayArrows;

	// (c.haag 2008-12-16 16:13) - PLID 32467 - List of all problems for the patient
	CArray<CEMRSummaryProblem,CEMRSummaryProblem&> m_aAllProblems;

	// (j.jones 2008-06-27 10:20) - PLID 21168 - added a proper Load function
	void Load();

	// (j.jones 2008-06-25 17:49) - PLID 21168 - added EMR Summary configuration settings
	BOOL m_bShowAllergies;
	BOOL m_bShowCurrentMeds;
	BOOL m_bShowPrescriptions;
	BOOL m_bShowLabs;
	BOOL m_bShowProblemList;
	BOOL m_bShowEMRDocuments;
	// (c.haag 2008-12-16 15:34) - PLID 32467
	BOOL m_bShowPtEmrProblems;
	BOOL m_bShowPtNonEmrProblems;
	CArray<long, long> m_aryCategoryIDs;
	CStringArray m_arystrCategoryNames;

	void LoadConfigSettings();

	// (j.jones 2008-06-26 09:40) - PLID 21168 - split out each section of data into its own function
	void LoadAllergies(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	void LoadCurrentMeds(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	void LoadPrescriptions(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	void LoadLabs(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);	
	// (c.haag 2008-12-16 17:01) - PLID 32467 - We now load problems in the first section
	void LoadPtNonEmrProblems(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);	
	void LoadPtEmrProblems(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);	
	
	void LoadEMRDiagList(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	void LoadEMRPrescriptionList(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	void LoadEMRProblemList(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	void LoadEMRDocuments(long nEMNID, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);
	// (j.jones 2008-10-30 12:02) - PLID 31857 - added an EMN pointer, NULL by default, only loaded if any Narrative or Table details are found
	void LoadEMRCategory(long nEMNID, CEMN *&pEMN, long nCategoryID, CString strCategoryName, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects);

	// (j.jones 2008-06-26 17:48) - PLID 21168 - given two lists, select the list with the least amount of objects
	CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*>* FindShortestList(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> &aryObjects1, CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> &aryObjects2);

	// (j.jones 2008-06-26 10:30) - PLID 21168 - copied from PatientLabsDlg, used in LoadLabs()
	//TES 11/10/2009 - PLID 36260 - This is no longer an enum, but a configurable table, so the description should be loaded along with the ID.
	//TES 12/8/2009 - PLID 36512 - AnatomySide is back!
	CString GetAnatomySideString(long nSide);

	// (j.jones 2008-06-13 09:05) - PLID 30385 - added patient image functionality
	void LoadPatientImagingInfo();

	// (c.haag 2008-12-16 16:20) - PLID 32467 - Load all problems for this patient
	void LoadPatientProblems();

	// (z.manning 2010-01-12 14:36) - PLID 31945 - Switch between scrollbar and arrow navigation
	void ToggleNavigationType();

	long m_nUnitedID;
	CString m_strMirrorID;
	long m_nImageIndex;
	void ShowImagingButtons(int nCountToShow);
	void HandleImageClick(CMirrorImageButton *pClicked);
	CMirrorImageButton *m_RightClicked;
	CWinThread *m_pLoadImageThread;
	CPtrList m_WaitingImages;
	typedef enum { eImageFwd, eImageBack } EImageTraversal;
	void OnRightClickImage(HWND hwndClicked, UINT nFlags, CPoint pt);
	void LoadPatientImage(EImageTraversal dir = eImageFwd);
	void LoadImageAsync(CEMRSummaryImageLoad* pLoadInfo);
	void LoadSingleImage(long nImageIndexAgainstSource, long nImageSourceCount, CMirrorImageButton *pButton, EImageSource Source);
	
	long m_nUserDefinedID;
	CString m_strLast, m_strFirst, m_strMiddle;

	// (j.jones 2008-06-26 09:15) - PLID 21168 - takes in an array of CEMRSummaryListInfoObjects,
	// and creates a new CEMRSummaryListInfoObject at the end of the array with the given data
	void AddListDataToArray(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> *aryObjects, CString strLabel = "", COLORREF cColor = RGB(0,0,0));
	// (j.jones 2008-06-26 09:15) - PLID 21168 - safely cleans up an array of CEMRSummaryListInfoObjects
	void ClearListData(CArray<CEMRSummaryListInfoObject*, CEMRSummaryListInfoObject*> &aryObjects);

	// (c.haag 2010-02-23 10:21) - PLID 37364 - This function ensures that the Mirror image manager exists
	// and is ready for getting image counts and loading images
	void EnsureMirrorImageMgr();
	// (c.haag 2010-02-23 15:08) - PLID 37364 - This function ensures that the Mirror image manager does
	// not exist. If it did, it is deleted.
	void EnsureNotMirrorImageMgr();
	// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the first valid Mirror image index for this patient
	long GetFirstValidMirrorImageIndex(const CString& strMirrorID);
	// (c.haag 2010-02-24 10:24) - PLID 37364 - Returns the last valid Mirror image index for this patient
	long GetLastValidMirrorImageIndex(const CString& strMirrorID);

	// (j.jones 2008-06-13 09:05) - PLID 30385 - added patient image functions
	// (j.jones 2008-06-19 09:24) - PLID 30436 - added OnBtnConfigureSummary
	// Generated message map functions
	//{{AFX_MSG(CEMRSummaryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnPrevEmn();
	afx_msg void OnBtnNextEmn();
	afx_msg void OnToggleViewType();
	afx_msg void OnEmrSummaryPreview();
	afx_msg void OnPtImageLast();
	afx_msg void OnPtImageNext();
	afx_msg LRESULT OnImageLoaded(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPtImage();
	afx_msg void OnPtImageLeft();
	afx_msg void OnPtImageRight();
	afx_msg void OnPtImageUpperLeft();
	afx_msg void OnPtImageUpperRight();
	afx_msg void OnPtImageLowerLeft();
	afx_msg void OnPtImageLowerRight();
	afx_msg void OnCopyImage();
	afx_msg void OnViewImage();
	afx_msg void OnDestroy();
	afx_msg void OnBtnConfigureSummary();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRSUMMARYDLG_H__A991219B_DA13_47BE_AD59_9E5CF3E8B588__INCLUDED_)
