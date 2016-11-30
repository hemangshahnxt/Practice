#pragma once
#include "AdministratorRc.h"

// CLabEditDiagnosisLinkDlg dialog
// (j.gruber 2016-02-10 12:25) - PLID 68155 - Create a setup dialog to link Diagnosis Codes with Microscopic descriptions

typedef std::map<long, NXDATALIST2Lib::IRowSettingsPtr> MicroscopicDescriptionMap;
typedef std::map<long, NXDATALIST2Lib::IRowSettingsPtr>::iterator MicroscopicDescriptionMapIterator;
typedef std::map<long, NXDATALIST2Lib::IRowSettingsPtr>::value_type MicroscopicDescription;

class CLabEditDiagnosisLinkDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CLabEditDiagnosisLinkDlg)

public:
	CLabEditDiagnosisLinkDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLabEditDiagnosisLinkDlg();
	virtual BOOL OnInitDialog();
	
	// Dialog Data
	#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_LABS_EDIT_DIAGNOSIS_LINK_DLG };
	#endif

private:
	CNxIconButton m_btnClose;
	CNxIconButton m_btnApply;

	CNxIconButton m_btnSelectOneDiagnosis;
	CNxIconButton m_btnSelectAllDiagnosis;
	CNxIconButton m_btnUnSelectOneDiagnosis;
	CNxIconButton m_btnUnSelectAllDiagnosis;

	CNxIconButton m_btnSelectOneDescription;
	CNxIconButton m_btnSelectAllDescription;
	CNxIconButton m_btnUnSelectOneDescription;
	CNxIconButton m_btnUnSelectAllDescription;
	CNxIconButton m_btnInsert;

	NXDATALIST2Lib::_DNxDataListPtr m_pDiagnosisFilterList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiagnosisAvailableList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiagnosisSelectedList;

	NXDATALIST2Lib::_DNxDataListPtr m_pDescriptionAvailableList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDescriptionSelectedList;

	void SelectOneDiagnosis(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void SelectAllDiagnosis();
	void UnSelectOneDiagnosis(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void UnSelectAllDiagnosis();
	void SelectOneDescription(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void SelectAllDescription();
	void UnSelectOneDescription(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void UnSelectAllDescription();
	void LinkSelectedRecords(BOOL bDeleteAll); // (j.gruber 2016-02-12 11:48) - PLID 68248 - added option to delete all or not
	BOOL WarnOnApply();
	void SetSelLabDiagnosisCombo(long nID);
	void SetSelLabDiagnosisCombo(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void EnableWindows(BOOL bEnable);
	void EnableSaveButtons();

		
	// (j.gruber 2016-02-10 12:25) - PLID 68225
	BOOL m_bAdvancedSetup;
	CRect m_rectDescriptionAvail, m_rectDescriptionSelected, m_rectSelectOneDescription, m_rectSelectAllDescription,
		m_rectUnSelectOneDescription, m_rectUnSelectAllDescription, m_rectDescriptionSearch, m_rectClose, m_rectApply, m_rectToggle, m_rectDlg,
		m_rectDiagAvail, m_rectDiagSelected, m_rectDiagSelectOne, m_rectDiagSelectAll, m_rectDiagUnSelectOne, m_rectDiagUnSelectAll,
		m_rectAvailableLabel, m_rectSelectedLabel, m_rectAppend;
	void PositionBoxes();
	void DefaultMovingWindows();

	// (j.gruber 2016-02-10 16:25) - PLID 68226 - Color the rows in the diagnosis list if they already have a microscopic description (or more) associated
	void SetDiagnosisHasDescription(BOOL bSet);
	void ColorRows(NXDATALIST2Lib::_DNxDataListPtr pDataList);

	
	// (j.gruber 2016-02-10 12:40) - PLID 68157 
	MicroscopicDescriptionMap m_mapAvailList;
	BOOL m_bIsFiltered;
	void FilterAvailableDescriptionList(CiString strFilter);
	void ResetAvailableDescriptionList();	
	void AddRowFromMap(NXDATALIST2Lib::IRowSettingsPtr pRowFromMap);
	void RemoveRowFromMap(long nID);
	void LoadAvailableDescriptionList();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SelChosenLabdiagnosisCombo(LPDISPATCH lpRow);
	void RequeryFinishedLabdiagnosisCombo(short nFlags);
	afx_msg void OnBnClickedSelectOneDiagnosis();
	afx_msg void OnBnClickedSelectAllDiagnosis();
	afx_msg void OnBnClickedUnselectOneDiagnosis();
	afx_msg void OnBnClickedUnselectAllDiagnosis();
	afx_msg void OnBnClickedSelectOneDescription();
	afx_msg void OnBnClickedSelectAllDescription();
	afx_msg void OnBnClickedUnselectOneDescription();
	afx_msg void OnBnClickedUnselectAllDescription();
	afx_msg void OnBnClickedApplyLink();
	afx_msg void SelChangingLabdiagnosisCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	void DblClickCellLabdiagnosisAvailable(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellLabdiagnosisSelected(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellLabclinicaldiagnosisAvailable(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellLabclinicaldiagnosisSelected(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBnClickedToggleSetup();
	afx_msg void OnEnChangeDescriptionSearch();
	void RequeryFinishedLabdiagnosisAvailable(short nFlags);
	afx_msg void OnBnClickedAppendLink();
};
