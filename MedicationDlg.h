#include "PatientDialog.h"
#if !defined(AFX_MEDICATIONDLG_H__6E980C76_2BCB_11D2_B1FE_0000C0832801__INCLUDED_)
#define AFX_MEDICATIONDLG_H__6E980C76_2BCB_11D2_B1FE_0000C0832801__INCLUDED_

#include "stdafx.h"
#include <NxUILib/NxStaticIcon.h>
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MedicationDlg.h : header file
//

// (j.fouts 2012-11-14 11:42) - PLID 53439 - Moved writing, printing, interactions, and printer templates out of this dlg and into the queue so it can be reused in 
//	EMR and anywhere else we are going to integrate this in the future
// (j.fouts 2012-12-26 12:42) - PLID 54340 - Removed the NewCrop button and functionality it is now handled in PrescriptionQueueDlg

// (j.jones 2012-11-30 15:12) - PLID 53968 - removed .h reference
class CPrescriptionQueueDlg;

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CMedicationDlg dialog

class CReconcileMedicationsDlg;
class CMedicationDlg : public CPatientDialog
{
	// Construction
public:

	long m_tempMedSchedID;
	CStringArray m_astrMedSchedEntries[367];
	CDWordArray m_adwMedSchedColors[367];
	long m_nMedSchedDays;
	void LoadMedSchedPrintInfo();
	BOOL bMedSchedPrintInfoLoaded;
	void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	void Print(CDC *pDC, CPrintInfo *pInfo);
	long DrawPageHeader(CDC *pDC, LPRECT rcPage, long nProcId);
	long DrawDayNumber(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey);
	long DrawDayHeader(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey);
	BOOL DoesDayOutputFit(CDC *pDC, CStringArray *pastrNames, long nBoxWidth, long nBoxHeight);

	BOOL m_bIsMedSchedPrinting;
	BOOL m_bToggleEndMedSchedPrinting;

	void OnEditMedSchedule(long MedSchedID);
	virtual void SetColor(OLE_COLOR nNewColor);
	CMedicationDlg(CWnd* pParent);   // standard constructor
	// (j.jones 2012-11-30 15:16) - PLID 53968 - added destructor
	~CMedicationDlg();

	// (j.fouts 2013-01-03 14:46) - PLID 54429 - Updates the Interaction count for its embeded queue
	void SetInteractionCount(long nInteractionCount);

	// (j.jones 2008-06-06 11:40) - PLID 29154 - added m_btnConfigureTemplates and removed m_btnMakeDefault
	// (j.jones 2008-10-07 08:35) - PLID 31596 - added m_btnEditFavoritePharmacies
	// (j.jones 2008-11-25 08:50) - PLID 28508 - added m_checkReviewedAllergies
	// (j.gruber 2009-06-02 17:26) - PLID 34449 - added inactive checkbox for medications
	// Dialog Data
	//{{AFX_DATA(CMedicationDlg)
	enum { IDD = IDD_MEDICATION_DLG };
	NxButton	m_checkReviewedAllergies;
	CNxIconButton	m_btnPreview;
	NxButton	m_btnShowAllergyWarning;
	CNxIconButton	m_btnDeleteMedSched;
	CNxIconButton	m_btnAddMedSched;
	CNxIconButton	m_btnEditAllergy;
	CNxIconButton	m_btnWrite;
	CNxColor	m_bkg;
	CNxColor	m_bg3;
	CNxColor	m_bkg4;
	CNxIconButton	m_btnViewSureScriptsMessages;
	NxButton	m_chkHideInactive;
	NxButton	m_chkMergeToPrinter;
	// (j.jones 2012-10-16 14:35) - PLID 53179 - added "no known allergies" checkbox
	NxButton		m_checkHasNoAllergies;
	// (j.jones 2012-10-17 11:06) - PLID 51713 - added "no current meds." checkbox
	NxButton		m_checkHasNoMeds;
	// (j.fouts 2013-01-07 10:43) - PLID 54468 - Readded the interactions button to the medications tab
	CNxIconButton	m_btnInteractions;
	// (r.gonet 09/20/2013) - PLID 58416 - Button to open the medication history dialog.
	CNxIconButton	m_btnMedicationHistory;
	// (b.savon 2013-01-11 12:54) - PLID 54592 - Embed the Med Pick List and Google search
	CNxIconButton	m_btnEditMeds;
	// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
	NxButton		m_checkHideDiscontinuedAllergies;
	// (j.jones 2016-01-21 09:40) - PLID 67994 - added checkbox to include free text meds in current meds
	NxButton		m_checkIncludeFreeTextCurrentMeds;
	// (j.jones 2016-01-21 14:52) - PLID 67995 - added checkbox to include free text allergies
	NxButton		m_checkIncludeFreeTextAllergies;
	// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
	CNxStaticIcon m_icoAboutCurrentMedicationColors;
	CNxStaticIcon m_icoAboutCurrentAllergiesColors;

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlMedSearchResults;
	void AddMedToCurrentMedicationsList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	BOOL AreMedicationConrolsLoaded();
	// (b.savon 2013-01-14 16:04) - PLID 54613 - Embed the Allergy Pick List and Google search
	CNxIconButton	m_btnEditAllergies;
	
	NXDATALIST2Lib::_DNxDataListPtr m_nxdlAllergySearchResults;
	
	void AddAllergyToCurrentAllergyList(NXDATALIST2Lib::IRowSettingsPtr pRow);
	BOOL AreAllergyControlsLoaded();
	// (b.savon 2013-01-30 10:03) - PLID 54922
	VARIANT_BOOL ImportMedication(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewDrugListID);
	// (b.savon 2013-01-30 10:56) - PLID 54927
	VARIANT_BOOL ImportAllergy(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewAllergyID);
	
	//}}AFX_DATA

public:
	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMedicationDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

	// Implementation
protected:

	// (j.jones 2008-05-16 09:22) - PLID 29732 - removed m_arydwPresIDs and PrintPrescription()

	long m_nDefTemplateRowIndex;
	NXDATALISTLib::_DNxDataListPtr   m_template;
	NXDATALISTLib::_DNxDataListPtr  m_pPatAllergies;
	NXDATALISTLib::_DNxDataListPtr  m_NxDlMedSchedules;
	NXDATALISTLib::_DNxDataListPtr  m_CurMedList;
	//NXDATALISTLib::_DNxDataListPtr  m_pAllergyList;

	// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
	HICON m_hInfoButtonIcon;

	bool m_bHistoryUse;

	// (j.fouts 2012-11-14 11:42) - PLID 53439 - The embeded queue dlg
	// (j.jones 2012-11-30 15:13) - PLID 53968 - changed to a pointer
	CPrescriptionQueueDlg *m_pPrescriptionQueueDlg;

protected:
	long m_nSelItem, m_nMedSchedSelItem;
	long m_id;

	void SecureControls();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	// (a.walling 2010-10-14 15:59) - PLID 40978 - Refresh
	virtual void Refresh();
	void EnableAppropriateButtons();
	//void UpdateFirstDate();

protected:
	// (j.jones 2008-11-25 09:50) - PLID 28508 - UpdateAllergyReviewStatus can be called
	// by a permissioned user when checking/unchecking the allergy review checkbox, or
	// can be called by any user if they change allergy information
	void UpdateAllergyReviewStatus(BOOL bReviewedAllergies);

	// (j.jones 2008-11-25 09:22) - PLID 28508 - CalcAllergyReviewLabel is a modular function that
	// will calculate the label for the allergy review checkbox, and apply it
	void UpdateAllergyReviewLabel(BOOL bReviewedAllergies, COleDateTime dtReviewedOn, CString strUserName);

	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Added flags for the call to SetWindowPosition
	void RepositionQueue(int nSetWindowPosFlags = 0);

protected:
	// (j.jones 2008-11-25 12:18) - PLID 32183 - added member tablechecker values
	// (r.gonet 09/20/2013) - PLID 58396 - Added a medication history response table checker.
	CTableChecker m_CurrentMedsChecker, m_PatientAllergiesChecker, m_MedicationHistoryResponseChecker;

	// (j.jones 2016-01-21 09:40) - PLID 67994 - resets the current med search provider based
	// on the value of the current meds' 'include free text' checkbox
	void ResetCurrentMedsSearchProvider();

	// (j.jones 2016-01-21 14:55) - PLID 67995 - resets the allergy search provider based
	// on the value of the allergy 'include free text' checkbox
	void ResetAllergySearchProvider();

	// (j.jones 2008-05-14 11:08) - PLID 29732 - added OnLeftClickNxdlhistory and removed
	// OnEditingFinishingNxdlhistory and OnEditingFinishedNxdlhistory
	// (j.jones 2008-06-06 11:40) - PLID 29154 - added OnBtnConfigureTemplates and removed
	// OnDeftemplateBtn and OnSelChosenTemplate
	// (j.jones 2008-10-07 08:35) - PLID 31596 - added OnBtnEditFavoritePharmacies
	// (j.jones 2008-11-25 08:50) - PLID 28508 - added OnCheckReviewedAllergies
	// (j.gruber 2009-03-30 17:21) - PLID 33728 - added post newcropbrowserdlg function
	// Generated message map functions
	//{{AFX_MSG(CMedicationDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	afx_msg void OnAddAllergy();
	afx_msg void OnClickDeleteAllergy();
	afx_msg void OnEditingFinishedNxdlpatallergies(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, long bCommit);
	afx_msg void OnPaint();
	afx_msg void OnAddMedsched();
	afx_msg void OnDeleteMedsched();
	afx_msg void OnRButtonDownNxdlmedschedules(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDblClickCellNxdlmedschedules(long nRowIndex, short nColIndex);
	afx_msg void OnEditingFinishingNxdlpatallergies(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnWarn();
	afx_msg void OnSelChangedCurrentMedList(long nNewSel);
	afx_msg void OnSelChangedNxdlpatallergies(long nNewSel);
	afx_msg void OnSelChangedNxdlmedschedules(long nNewSel);
	afx_msg void OnCheckReviewedAllergies();
	afx_msg LRESULT OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual int SetControlPositions();
	afx_msg void OnBnClickedInteractions();
	afx_msg void OnBnClickedBtnEditMeds();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	afx_msg void OnBnClickedBtnEditAllergies();
	// (r.gonet 09/20/2013) - PLID 58416 - Handler for when the user clicks on the med history button.
	afx_msg void OnBnClickedMedicationHistoryBtn();
	
	void RButtonUpNxdlpatallergies(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// (j.gruber 2009-05-13 15:14) - PLID 34251 - color allergy list for newcrop
	void RequeryFinishedNxdlpatallergies(short nFlags);
	// (j.gruber 2009-05-14 13:30) - PLID 34259 - color current med list
	void RequeryFinishedCurrentMedList(short nFlags);
	afx_msg void OnBnClickedMedHideInactive();
	// (j.jones 2011-05-02 16:49) - PLID 43450 - supported editing the Sig
	void OnEditingStartingCurrentMedList(long nRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	// (j.jones 2011-05-02 16:49) - PLID 43450 - supported editing the Sig
	void OnEditingFinishedCurrentMedList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (j.fouts 2012-08-10 09:40) - PLID 52090 - Added a popup list to show monograph for a drug
	void RButtonUpCurrentMedList(long nRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2012-10-16 14:35) - PLID 53179 - added "no known allergies" checkbox
	afx_msg void OnCheckHasNoAllergies();
	// (j.jones 2012-10-17 11:06) - PLID 51713 - added "no current meds." checkbox
	afx_msg void OnCheckHasNoMeds();
	// (j.fouts 2013-01-07 10:43) - PLID 54468 - Open the interactions dlg

	// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
	afx_msg void OnCheckHideDiscontinuedAllergies();
	// (r.gonet 09/20/2013) - PLID 58396 - Handler for the refresh medication history button message.
	LRESULT CMedicationDlg::OnBackgroundMedHistoryRequestComplete(WPARAM wParam, LPARAM lParam);
	// (j.jones 2013-10-17 15:55) - PLID 58983 - added left click handler
	void OnLeftClickCurrentMedList(long nRow, short nCol, long x, long y, long nFlags);
	// (s.dhole 2013-06-18 11:43) - PLID 56926
	void  UpdateCurrentMedRecord(long nRow, short nColumn, const VARIANT&  varValue);
	void EditingFinishingCurrentMedList(long nRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void ColumnSizingFinishedCurrentMedList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	// (s.dhole 2013-07-08 14:17) - PLID 56931
	void ColumnSizingFinishedpatallergies(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);

	long GetColumnWidth(const CString& strColWidths, short nColumn);
	// (s.dhole 2013-07-08 14:17) - PLID 56931
	CString  ReadColumnWidths(NXDATALISTLib::_DNxDataListPtr pList);
	void ResizeCurrentMedicationColumns(const CString strColWidths);
	void ResizeAllergyColumns(const CString strColWidths);
	//(s.dhole 3/10/2015 11:26 AM ) - PLID 64561
	void SelChosenNxdlMedSearchResults(LPDISPATCH lpRow);
	//(s.dhole 3/10/2015 5:25 PM ) - PLID 64564
	void SelChosenNxdlAllergySearchResults(LPDISPATCH lpRow);
	// (j.jones 2016-01-21 09:40) - PLID 67994 - added checkbox to include free text meds in current meds
	afx_msg void OnMedIncludeFreeTextMeds();
	// (j.jones 2016-01-21 14:52) - PLID 67995 - added checkbox to include free text allergies
	afx_msg void OnMedIncludeFreeTextAllergies();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEDICATIONDLG_H__6E980C76_2BCB_11D2_B1FE_0000C0832801__INCLUDED_)
