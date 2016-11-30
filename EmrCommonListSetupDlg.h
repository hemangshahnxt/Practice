// (c.haag 2011-03-17) - PLID 42889 - Initial implementation. Unless otherwise commented, everything
// in this class falls on this item.
#pragma once

#include "EmrInfoCommonListCollection.h"

// CEmrCommonListSetupDlg dialog

class CEmrInfoDataElementArray;

class CEmrCommonListSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrCommonListSetupDlg)

private:
	// The current collection we're editing
	CEmrInfoCommonListCollection m_data;

private:
	// The list of EMR data elements we have to choose from
	CEmrInfoDataElementArray* m_pAvailableEmrDataElements;

	// The map of inactive EMR Data items by ID
	CMap<long,long,CEmrInfoDataElement*,CEmrInfoDataElement*> m_mapInactiveEmrDataIDs;

private:
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnUp;
	CNxIconButton	m_btnDown;
	NXDATALIST2Lib::_DNxDataListPtr m_CollectionList;

public:
	CEmrCommonListSetupDlg(const CEmrInfoCommonListCollection& srcData,
		CEmrInfoDataElementArray* pAvailableEmrDataElements,
		CWnd* pParent);   // standard constructor
	virtual ~CEmrCommonListSetupDlg();

// Dialog Data
	enum { IDD = IDD_EMR_COMMON_LIST_SETUP_DLG };

public:
	// Returns the resultant changes (test for an IDOK return value before calling this)
	const CEmrInfoCommonListCollection& GetResult() const;

private:
	// Returns the formatted text for the items column of a given row
	_bstr_t GetItemsColumnText(const CEmrInfoCommonList& list);
	// Ensure that it's safe to save the data and close the window
	BOOL Validate();
	// Take the datalist content and assign it to the common lists
	void ApplyFormDataToList();
	// Repopulate the visible list
	void UpdateView();
	// Update the colors of a row given its data content
	void UpdateRowColors(NXDATALIST2Lib::IRowSettingsPtr pRow);
	// Update the button enabled/disabled states
	void UpdateButtonStates();
	// Change list button text color
	void PromptForButtonTextColor(LPDISPATCH lpRow);
	// Swaps two datalist rows
	void SwapRows(NXDATALIST2Lib::IRowSettingsPtr p1, NXDATALIST2Lib::IRowSettingsPtr p2);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual void OnBtnAddCommonList();
	virtual void OnBtnDeleteCommonList();
	virtual void OnBtnUp();
	virtual void OnBtnDown();
	virtual void OnOK();

	DECLARE_EVENTSINK_MAP()
	void OnRButtonDownCollectionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnEditingFinishingCollectionList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void LeftClickCollectionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnSelChangedCollectionList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnEditingFinishedCollectionList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
};
