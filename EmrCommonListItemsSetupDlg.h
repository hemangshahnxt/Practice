// (c.haag 2011-03-17) - PLID 42890 - Initial implementation. Unless otherwise commented, everything
// in this class falls on this item.

#pragma once

#include "EmrInfoCommonListCollection.h"

// CEmrCommonListItemsSetupDlg dialog

class CEmrInfoDataElementArray;

class CEmrCommonListItemsSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrCommonListItemsSetupDlg)

private:
	// The common list we're editing
	CEmrInfoCommonList m_data;

private:
	// The list of EMR data elements we have to choose from
	CEmrInfoDataElementArray* m_pAvailableEmrDataElements;

private:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnSelectAll;
	CNxIconButton m_btnUnselectAll;
	NXDATALIST2Lib::_DNxDataListPtr m_ListItems;

public:
	CEmrCommonListItemsSetupDlg(
		const CEmrInfoCommonList& srcData, 
		CEmrInfoDataElementArray* pAvailableEmrDataElements,
		CWnd* pParent);   // standard constructor
	virtual ~CEmrCommonListItemsSetupDlg();

// Dialog Data
	enum { IDD = IDD_EMR_COMMON_LIST_ITEMS_SETUP_DLG };

public:
	// Returns the resultant changes (test for an IDOK return value before calling this)
	const CEmrInfoCommonList& GetResult() const;

private:
	// Repopulate the visible list
	void UpdateView();
	// Save any changes
	void Save();
	// Select/unselect all items
	void SelectAllItems(BOOL bSelect);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnBtnSelectAll();
	virtual void OnBtnUnselectAll();
};
