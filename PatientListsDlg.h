// (r.gonet 12/02/2013) - PLID 59830 - Added.

#pragma once

#include "PatientsRc.h"
#include "LetterWritingRc.h"
#include "SelectionFiltering.h"
#include <list>

// CPatientListsDlg dialog

enum FilterBasedOnEnum;
class CFilterDlg;

// (r.gonet 12/02/2013) - PLID 59830 - Dialog that lets the user select patients by LW filters
// and view and sort the field values that he or she filtered on.
class CPatientListsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPatientListsDlg)
private:
// Controls
	// (r.gonet 12/02/2013) - PLID 59830 - Background for the dialog
	CNxColor m_nxcolorBackground;
	// (r.gonet 12/02/2013) - PLID 59830 - Label for the filter combo
	CNxStatic m_nxstaticFilterHeader;
	// (r.gonet 12/02/2013) - PLID 59830 - Combo of the LW filters to apply
	NXDATALIST2Lib::_DNxDataListPtr m_pFilterCombo;
	// (r.gonet 12/02/2013) - PLID 59830 - Button that adds a new LW filter
	CNxIconButton m_btnNew;
	// (r.gonet 12/02/2013) - PLID 59830 - Button that edits an existing LW filter
	CNxIconButton m_btnEdit;
	// (r.gonet 12/02/2013) - PLID 59830 - Button that deletes an existing LW filter 
	CNxIconButton m_btnDelete;
	// (r.gonet 12/02/2013) - PLID 59830 - Label for the matched records datalist
	CNxStatic m_nxstaticMatchesHeader;
	// (r.gonet 12/02/2013) - PLID 59830 - Datalist of the patient records that match the filter
	NXDATALIST2Lib::_DNxDataListPtr m_pPatientList;
	// (r.gonet 12/02/2013) - PLID 59830 - Label of the count of the patients and/or records that matched the filter
	CNxStatic m_nxstaticCountLabel;
	// (r.gonet 12/02/2013) - PLID 59830 - Button that closes the dialog
	CNxIconButton m_btnClose;
	// (j.jones 2014-05-20 16:40) - PLID 61814 - added clipboard functionality
	CNxIconButton m_btnCopyToClipboard;

	// (r.gonet 12/02/2013) - PLID 59830 - Filter string of a temporary filter that is not saved
	CString m_strOneTimeFilter;

public:
// Dialog Data
	enum { IDD = IDD_PATIENT_LISTS_DLG };
private:
// Dialog data
	// (r.gonet 12/02/2013) - PLID 59830 - Columns for the filter combo
	enum EPatientListsFilterColumns
	{
		eplfcID = 0,
		eplfcName,
		eplfcCreatedDate,
		eplfcModifiedDate,
	};

public:
// Methods
	// (r.gonet 12/02/2013) - PLID 59830 - Constructs a new CPatientListsDlg
	CPatientListsDlg(CWnd* pParent = NULL);   // standard constructor
	// (r.gonet 12/02/2013) - PLID 59830 - Destructor
	virtual ~CPatientListsDlg();

protected:
// Methods
	// (r.gonet 12/02/2013) - PLID 59830
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
// Methods
	// (r.gonet 12/02/2013) - PLID 58821 - Add the dynamic columns of the datalist given the recordset of the select statement and a map of the fields in the select statement.
	// Returns a map of the recordset column indexes to datalist column indexes.
	void LoadPatientListColumns(ADODB::_RecordsetPtr prs, CMapStringToField &mapSelectedFields, OUT CMap<long, long, short, short> &mapRsFieldIndexToDatalistColIndex);
	// (r.gonet 12/02/2013) - PLID 58821 - Gets the corresponding datalist field type given a selected field and a database type. 
	void AdoType2DatalistFieldType(CSelectedFieldPtr pSelectedField, ADODB::DataTypeEnum dteFieldType, OUT NXDATALIST2Lib::EColumnFieldType &cftColumnFieldType);
	// (r.gonet 12/02/2013) - PLID 58821 - Gets the corresponding variant type given a database type. Returns true if a variant type could be
	// determined and false if one could not.
	bool AdoType2DatalistDataType(ADODB::DataTypeEnum dteFieldType, OUT VARTYPE &vt);
	// (r.gonet 12/02/2013) - PLID 59830 - Opens the filter editor dialog. If bNewFilter is true, then the filter will be a new filter. If bNewFilter is false, then we are editing an existing filter.
	void OpenFilterEditor(bool bNewFilter);
	// (r.gonet 12/02/2013) - PLID 59830 - Populates the filter combo with the databases' letter writing filters.
	void FillFilterCombo(long nSelectId, LPCTSTR strFilterString = NULL);
	// (r.gonet 12/02/2013) - PLID 59830 - When the user chooses a filter from the combo, we load the results.
	void ChangeFilterSelection(long nFilterId);
	// (r.gonet 12/02/2013) - PLID 59830 - Clears the results list, removes all the dynamic columns, and resets the count label.
	void ClearResultsList();
	// (r.gonet 12/02/2013) - PLID 59830 - Ensures the buttons are enabled or disabled appropriately.
	void EnsureControls();

	// (j.jones 2014-05-20 16:40) - PLID 61814 - added clipboard functionality
	void CalcResultsetAsCSVAndHTML(OUT CString &strHTML, OUT CString &strCSV);
	
	// (r.gonet 12/02/2013) - PLID 59830 - Initializes the dialog
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user clicks the new filter button
	afx_msg void OnBnClickedPatientListsNewFilterBtn();
	// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user clicks the edit filter button
	afx_msg void OnBnClickedPatientListsEditFilterBtn();
	// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user clicks the delete filter button
	afx_msg void OnBnClickedPatientListsDeleteFilterBtn();
	// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the user has chosen a filter row from the filter combo
	void SelChosenPatientListsFilterCombo(LPDISPATCH lpRow);
	// (r.gonet 12/02/2013) - PLID 59830 - Handler for when the filter combo is changing. Don't let the user select nothing.
	void SelChangingPatientListsFilterCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (j.jones 2014-05-20 16:40) - PLID 61814 - added clipboard functionality
	afx_msg void OnBtnCopyToClipboard();
};