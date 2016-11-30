// EmrActionFilterAnatomyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrActionFilterAnatomyDlg.h"
#include "afxdialogex.h"

#include "EmrActionFilterAnatomyDetailDlg.h"

#include <NxUILib/DatalistUtils.h>

// (a.walling 2014-08-06 15:13) - PLID 62686 - Laterality - Setup

// (a.walling 2014-08-06 16:39) - PLID 62688 - Laterality - CEmrActionAnatomyFilterDlg

// CEmrActionFilterAnatomyDlg dialog

namespace {
	enum ListColumns {
		lcID
		, lcChecked
		, lcDescription
		, lcDetails
	};
}

IMPLEMENT_DYNAMIC(CEmrActionFilterAnatomyDlg, CNxDialog)

CEmrActionFilterAnatomyDlg::CEmrActionFilterAnatomyDlg(CWnd* pParent, const Emr::ActionFilter& filter)
	: CNxDialog(CEmrActionFilterAnatomyDlg::IDD, pParent, "CEmrActionFilterAnatomyDlg")
	, m_filter(filter)
{

}

CEmrActionFilterAnatomyDlg::~CEmrActionFilterAnatomyDlg()
{
}

void CEmrActionFilterAnatomyDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEmrActionFilterAnatomyDlg, CNxDialog)
END_MESSAGE_MAP()


// CEmrActionFilterAnatomyDlg message handlers
BEGIN_EVENTSINK_MAP(CEmrActionFilterAnatomyDlg, CNxDialog)
	ON_EVENT(CEmrActionFilterAnatomyDlg, IDC_LIST, 10, CEmrActionFilterAnatomyDlg::EditingFinishedList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrActionFilterAnatomyDlg, IDC_LIST, 19, CEmrActionFilterAnatomyDlg::LeftClickList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CEmrActionFilterAnatomyDlg::UpdateDetailsColumn(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if (!pRow) {
		return;
	}

	long anatomicLocationID = VarLong(pRow->Value[lcID]);

	// (a.walling 2014-08-08 09:56) - PLID 62685 - Laterality - Side / Qualifier text description generation
	CString desc;

	// (a.walling 2014-09-16 08:36) - PLID 62689 - Blank if only any side / any qualifier
	if (m_filter.GetAnyQualifierSide(anatomicLocationID) == Emr::sideAny && m_filter.count(anatomicLocationID) == 1) {
		// keep desc blank
	}
	else {
		desc = DescribeFilter(m_filter, m_qualifierMap, anatomicLocationID);
	}

	// (a.walling 2014-08-13 09:46) - PLID 62689 - show on hover by setting normal (non-hover) text to transparent
	if (desc.IsEmpty()) {
		pRow->CellForeColor[lcDetails] = NXDATALIST2Lib::dlColorTransparent;
		pRow->CellForeColorSel[lcDetails] = NXDATALIST2Lib::dlColorTransparent;
		pRow->Value[lcDetails] = "<choose more details>";
	}
	else {
		if (VarBool(pRow->Value[lcChecked], FALSE)) {
			pRow->CellForeColor[lcDetails] = NXDATALIST2Lib::dlColorNotSet;
			pRow->CellForeColorSel[lcDetails] = NXDATALIST2Lib::dlColorNotSet;
		}
		else {
			// if unchecked, we keep the description around so they don't lose anything until they save. make it grey
			pRow->CellForeColor[lcDetails] = HEXRGB(0x808080);
			pRow->CellForeColorSel[lcDetails] = HEXRGB(0x808080);
		}
		pRow->Value[lcDetails] = (const char*)desc;
	}
}

void CEmrActionFilterAnatomyDlg::EditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if (!pRow) {
			return;
		}

		if (nCol != lcChecked) {
			return;
		}

		// (a.walling 2014-09-11 15:25) - PLID 62689 - If newly checked, we add the any qualifier / any side default detail.
		// if unchecked, we remove from the map entirely if it only
		// has the default (any) detail.

		long id = VarLong(pRow->Value[lcID]);

		if (VarBool(varNewValue, FALSE)) {
			auto range = m_filter.equal_range(id);
			if (range.first == range.second) {
				// empty, yet checked? insert the any qualifier / any side detail
				m_filter.anatomicLocationFilters.emplace(id, -1, Emr::sideAny);
			}
		}
		else {
			// not checked, and has any qualifier / any side
			if (m_filter.GetAnyQualifierSide(id) == Emr::sideAny && m_filter.count(id) == 1) {
				// this was the only detail, and its any qualifier / any side, so just get rid of it from the map entirely
				// since we don't need to 'save' any previous data in case they unchecked accidentally
				m_filter.anatomicLocationFilters.erase({ id, -1 });
			}
		}

		UpdateDetailsColumn(pRow);
	} NxCatchAll(__FUNCTION__);
}


void CEmrActionFilterAnatomyDlg::LeftClickList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if (!pRow) {
			return;
		}

		if (nCol != lcDetails) {
			return;
		}

		// (a.walling 2014-09-11 08:35) - PLID 62688 - Set this row as the cursel
		m_pList->CurSel = pRow;

		// (a.walling 2014-08-06 16:39) - PLID 62690 - Laterality - CEmrActionAnatomyFilterDetailDlg - Side / Qualifier Selection
		long anatomicLocationID = VarLong(pRow->Value[lcID]);

		auto newFilter = m_filter;

		if (newFilter.GetAnyQualifierSide(anatomicLocationID) == Emr::sideAny && newFilter.count(anatomicLocationID) == 1) {
			// (a.walling 2014-09-16 08:36) - PLID 62689 - only has any side / any qualifier, so delete it so the dialog is blank
			newFilter.anatomicLocationFilters.erase({ anatomicLocationID, -1 });
		}

		CEmrActionFilterAnatomyDetailDlg dlg(this, newFilter, anatomicLocationID, VarString(pRow->Value[lcDescription]));

		if (IDOK == dlg.DoModal()) {
			m_filter = dlg.m_filter;

			// (a.walling 2014-09-16 08:36) - PLID 62689 - not having anything checked == having any qualifier / any side checked
			if (!m_filter.count(anatomicLocationID)) {
				m_filter.anatomicLocationFilters.emplace(anatomicLocationID, -1, Emr::sideAny);
			}

			pRow->Value[lcChecked] = true; // always check

			UpdateDetailsColumn(pRow);
		}
	} NxCatchAll(__FUNCTION__);
}


BOOL CEmrActionFilterAnatomyDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {

		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDOK))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);

		using namespace NXDATALIST2Lib;

		m_pList = BindNxDataList2Ctrl(IDC_LIST, false);

		m_pList->AllowMultiSelect = VARIANT_TRUE;

		AppendColumns(m_pList, {
				{ "", "ID",					0,	csFixedWidth }, 
				{ "", "Checked",			60,	csFixedWidth | csEditable,	cftBoolCheckbox },
				{ "", "Description",		0,	csWidthAuto,				cftTextSingleLine,	0},
				{ "", "Side / Qualifier",	30,	csWidthPercent,				cftTextSingleLineLink }
		});

		m_qualifierMap = Emr::MakeAnatomyQualifiersMap();

		for (ADODB::_RecordsetPtr prs = CreateRecordsetStd("SELECT ID, Description, Inactive FROM LabAnatomyT ORDER BY Description"); !prs->eof; prs->MoveNext()) {
			long id = AdoFldLong(prs, "ID");
			CString desc = AdoFldString(prs, "Description");
			bool inactive = !!AdoFldBool(prs, "Inactive");

			size_t count = m_filter.count(id);

			if (inactive && !count) {
				// inactive and does not exist in the filter, so ignore
				continue;
			}		

			auto row = MakeNewRow(m_pList, {id, count ? true : false, (const char*)desc});

			if (inactive) {
				row->ForeColor = HEXRGB(0x808080);
			}

			UpdateDetailsColumn(row);

			m_pList->AddRowAtEnd(row, nullptr);
		}
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrActionFilterAnatomyDlg::OnOK()
{
	try {
		for (auto row = m_pList->GetFirstRow(); row; row = row->GetNextRow()) {
			long id = VarLong(row->Value[lcID]);
			auto range = m_filter.equal_range(id);
			if (!VarBool(row->Value[lcChecked])) {
				// erase unchecked ones
				m_filter.anatomicLocationFilters.erase(range.first, range.second);
			}
			else if (range.first == range.second) {
				// empty, yet checked? insert the any qualifier / any side detail
				m_filter.anatomicLocationFilters.emplace(id, -1, Emr::sideAny);
			}

			// (a.walling 2014-09-11 15:25) - PLID 62689 - There may be redundant details if any qualifier / any side is selected, but we keep them anyway.

		}
	} NxCatchAll(__FUNCTION__);
	__super::OnOK();
}
