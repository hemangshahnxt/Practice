// EmrCommonListItemsSetupDlg.cpp : implementation file
// (c.haag 2011-03-17) - PLID 42890 - Initial implementation. Unless otherwise commented, everything
// in this class falls on this item.

#include "stdafx.h"
#include "Practice.h"
#include "EmrCommonListItemsSetupDlg.h"
#include "EmrItemEntryDlg.h"

using namespace NXDATALIST2Lib;

enum ListColumns {
	lcEmrDataElementPtr = 0,
	lcChecked,
	lcName,
};

// CEmrCommonListItemsSetupDlg dialog

IMPLEMENT_DYNAMIC(CEmrCommonListItemsSetupDlg, CNxDialog)

CEmrCommonListItemsSetupDlg::CEmrCommonListItemsSetupDlg(
	const CEmrInfoCommonList& srcData, 
	CEmrInfoDataElementArray* pAvailableEmrDataElements,
	CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrCommonListItemsSetupDlg::IDD, pParent)
{
	// Load the initial data
	m_data = srcData;
	m_pAvailableEmrDataElements = pAvailableEmrDataElements;
}

CEmrCommonListItemsSetupDlg::~CEmrCommonListItemsSetupDlg()
{
}

// Returns the resultant changes (test for an IDOK return value before calling this)
const CEmrInfoCommonList& CEmrCommonListItemsSetupDlg::GetResult() const
{
	return m_data;
}

// Repopulate the visible list (only called on startup)
void CEmrCommonListItemsSetupDlg::UpdateView()
{
	m_ListItems->Clear();

	int nAvailableItems = m_pAvailableEmrDataElements->GetSize();
	for (int i=0; i < nAvailableItems; i++)
	{
		CEmrInfoDataElement* pElement = m_pAvailableEmrDataElements->GetAt(i);
		// Only display active items
		if (!pElement->m_bInactive)
		{
			IRowSettingsPtr pRow = m_ListItems->GetNewRow();
			pRow->Value[lcEmrDataElementPtr] = (long)pElement;
			pRow->Value[lcChecked] = m_data.DoesExist(pElement) ? g_cvarTrue : g_cvarFalse;
			pRow->Value[lcName] = _bstr_t(pElement->m_strData);
			m_ListItems->AddRowSorted(pRow, NULL);
		}
	}
}

// Save any changes
void CEmrCommonListItemsSetupDlg::Save()
{
	// Clear all the items out of this list and repopulate them based on selected rows
	m_data.Clear();
	IRowSettingsPtr pPrevRow = NULL;
	// Get the first selected row
	IRowSettingsPtr pFirstRow = m_ListItems->FindByColumn(lcChecked, g_cvarTrue, pPrevRow, VARIANT_FALSE);
	IRowSettingsPtr pRow = NULL;

	// Special loop that can tolerate circular searches
	if (NULL != pFirstRow)
	{
		// Go through all selected rows and add them to the common list m_data. The internal ID's of each entry
		// in EmrInfoCommonListItemsT is not relevant because EMR Data ID's are either in or out of the list; there's
		// nothing to modify.
		pRow = pFirstRow;
		do
		{
			CEmrInfoDataElement* pElement = (CEmrInfoDataElement*)VarLong(pRow->Value[lcEmrDataElementPtr]);
			m_data.AddItem(pElement);
			pPrevRow = pRow->GetNextRow();
			pRow = m_ListItems->FindByColumn(lcChecked, g_cvarTrue, pPrevRow, VARIANT_FALSE);
		}
		while (pRow != pFirstRow);
	}
}

// Select/unselect all items
void CEmrCommonListItemsSetupDlg::SelectAllItems(BOOL bSelect)
{
	IRowSettingsPtr pRow = m_ListItems->GetFirstRow();
	while (NULL != pRow)
	{
		pRow->Value[lcChecked] = bSelect ? g_cvarTrue : g_cvarFalse;			
		pRow = pRow->GetNextRow();
	}
}

void CEmrCommonListItemsSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SELECT_ALL_COMMON_LIST_ITEMS, m_btnSelectAll);
	DDX_Control(pDX, IDC_UNSELECT_ALL_COMMON_LIST_ITEMS, m_btnUnselectAll);
}


BEGIN_MESSAGE_MAP(CEmrCommonListItemsSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SELECT_ALL_COMMON_LIST_ITEMS, OnBtnSelectAll)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_COMMON_LIST_ITEMS, OnBtnUnselectAll)
END_MESSAGE_MAP()


// CEmrCommonListItemsSetupDlg message handlers

BOOL CEmrCommonListItemsSetupDlg::OnInitDialog() 
{		
	try 
	{
		CNxDialog::OnInitDialog();
		CString strCaption;
		strCaption.Format("Available Items for '%s'", m_data.GetName());
		SetWindowText(strCaption);

		// Initialize controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSelectAll.AutoSet(NXB_MODIFY);
		m_btnUnselectAll.AutoSet(NXB_MODIFY);
		m_ListItems = BindNxDataList2Ctrl(IDC_LIST_COMMON_ITEMS, false);

		// Populate the datalist with alll the rows and select the ones that the
		// common list already has selected
		UpdateView();
	}
	NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrCommonListItemsSetupDlg::OnOK()
{
	try {
		Save();
		CDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCommonListItemsSetupDlg::OnBtnSelectAll()
{
	try {
		SelectAllItems(TRUE);
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrCommonListItemsSetupDlg::OnBtnUnselectAll()
{
	try {
		SelectAllItems(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}
