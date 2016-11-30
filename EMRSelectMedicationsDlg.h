#pragma once

// (j.jones 2013-03-04 17:32) - PLID 55376 - created

#include <NxUILib/NxStaticIcon.h>
#include "AdministratorRc.h"
#include "GlobalUtils.h"

// CEMRSelectMedicationsDlg dialog

class CEMRSelectMedicationsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMRSelectMedicationsDlg)

public:
	CEMRSelectMedicationsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEMRSelectMedicationsDlg();
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CArray<IdName, IdName> m_arySelectedMeds;
	// (j.jones 2016-01-25 15:44) - PLID 67999 - added checkbox to include free text meds in the medication search
	NxButton	m_checkIncludeFreeTextMeds;
	// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med search color
	CNxStaticIcon m_icoAboutEMRSelectMedsColors;

// Dialog Data
	enum { IDD = IDD_EMR_SELECT_MEDICATIONS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_nxdlMedicationSearch; // (b.savon 2015-12-30 13:10) - PLID 67759
	NXDATALIST2Lib::_DNxDataListPtr m_List;

	// (b.savon 2015-12-30 13:10) - PLID 67759
	void AddMedicationFromSearch(LPDISPATCH lpRow);
	void AddMedicationToList(const long &nDrugListID, const CString& strMedName, const long &nFDBID, const BOOL &bFDBOutOfDate);

	// (j.jones 2016-01-25 15:44) - PLID 67999 - resets the medication search provider based
	// on the value of the 'include free text' checkbox
	void ResetMedicationSearchProvider();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnOk();
	afx_msg void OnRemoveMedication();
	DECLARE_EVENTSINK_MAP()
	void SelChosenNxdlMedSearchAdd(LPDISPATCH lpRow);
	void RButtonDownSelectMedicationList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2016-01-25 15:42) - PLID 67999 - added checkbox to include free text meds in the medication search
	afx_msg void OnCheckIncludeFreeTextMedsItems();
};