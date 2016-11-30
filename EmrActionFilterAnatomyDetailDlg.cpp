// EmrActionFilterAnatomyDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "Practice.h"
#include "EmrActionFilterAnatomyDetailDlg.h"
#include "afxdialogex.h"


#include <NxUILib/DatalistUtils.h>

// (a.walling 2014-08-06 15:13) - PLID 62686 - Laterality - Setup

// (a.walling 2014-08-06 16:39) - PLID 62690 - Laterality - CEmrActionAnatomyFilterDetailDlg - Side / Qualifier Selection

// CEmrActionFilterAnatomyDetailDlg dialog

namespace {
	enum ListColumns {
		lcID
		, lcBilateral
		, lcLeft
		, lcRight
		, lcNoSide
		, lcQualifier
	};
}


IMPLEMENT_DYNAMIC(CEmrActionFilterAnatomyDetailDlg, CNxDialog)

CEmrActionFilterAnatomyDetailDlg::CEmrActionFilterAnatomyDetailDlg(CWnd* pParent, const Emr::ActionFilter& filter, long anatomicLocationID, CString anatomicLocationName)
	: CNxDialog(CEmrActionFilterAnatomyDetailDlg::IDD, pParent, "CEmrActionFilterAnatomyDetailDlg")
	, m_filter(filter)
	, m_anatomicLocationID(anatomicLocationID)
	, m_anatomicLocationName(anatomicLocationName)
{

}

CEmrActionFilterAnatomyDetailDlg::~CEmrActionFilterAnatomyDetailDlg()
{
}

void CEmrActionFilterAnatomyDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEmrActionFilterAnatomyDetailDlg, CNxDialog)
END_MESSAGE_MAP()


// CEmrActionFilterAnatomyDetailDlg message handlers
BEGIN_EVENTSINK_MAP(CEmrActionFilterAnatomyDetailDlg, CNxDialog)
	ON_EVENT(CEmrActionFilterAnatomyDetailDlg, IDC_LIST, 10, CEmrActionFilterAnatomyDetailDlg::EditingFinishedList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrActionFilterAnatomyDetailDlg, IDC_LIST, 6, CEmrActionFilterAnatomyDetailDlg::RButtonDownList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrActionFilterAnatomyDetailDlg, IDC_LIST, 32, CEmrActionFilterAnatomyDetailDlg::ShowContextMenuList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4 VTS_PBOOL)
END_EVENTSINK_MAP()

namespace {
	// (a.walling 2014-08-13 15:02) - PLID 62691 - Ensure the row is consistent in terms of bilateral and left/right
	void UpdateRow(Emr::ActionFilter& filter, long anatomicLocationID, NXDATALIST2Lib::IRowSettingsPtr pRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue)
	{

		if (!pRow) {
			return;
		}

		if (nCol < lcBilateral || nCol > lcNoSide) {
			return;
		}

		ASSERT(varOldValue.vt == VT_BOOL);
		ASSERT(varNewValue.vt == VT_BOOL);

		if (nCol == lcBilateral) {
			if (VarBool(varNewValue)) {
				pRow->Value[lcLeft] = true;
				pRow->Value[lcRight] = true;
			}
			else {
				pRow->Value[lcLeft] = false;
				pRow->Value[lcRight] = false;
			}
		}
		else if (nCol == lcLeft || nCol == lcRight) {
			if (VarBool(pRow->Value[lcLeft]) && VarBool(pRow->Value[lcRight])) {
				pRow->Value[lcBilateral] = true;
			}
			else {
				pRow->Value[lcBilateral] = false;
			}
		}

		long side = 0;

		if (VarBool(pRow->Value[lcBilateral])) {
			side |= Emr::sideBilateral;
		}
		if (VarBool(pRow->Value[lcLeft])) {
			side |= Emr::sideLeft;
		}
		if (VarBool(pRow->Value[lcRight])) {
			side |= Emr::sideRight;
		}
		if (VarBool(pRow->Value[lcNoSide])) {
			side |= Emr::sideNone;
		}

		/*	pRow->Value[lcBilateral] = !!((side & Emr::sideBilateral) == Emr::sideBilateral);
		pRow->Value[lcLeft] = !!(side & Emr::sideLeft);
		pRow->Value[lcRight] = !!(side & Emr::sideRight);
		pRow->Value[lcNoSide] = !!(side & Emr::sideNone);	*/

		long id = VarLong(pRow->Value[lcID]);

		if (!side) {
			filter.anatomicLocationFilters.erase({ anatomicLocationID, id });
			return;
		}

		auto ret = filter.anatomicLocationFilters.emplace(anatomicLocationID, id, side);
		if (!ret.second) {
			// already exists
			ret.first->anatomySide = side;
		}
	}
}

void CEmrActionFilterAnatomyDetailDlg::EditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		UpdateRow(m_filter, m_anatomicLocationID, lpRow, nCol, varOldValue, varNewValue);
	} NxCatchAll(__FUNCTION__);
}

void CEmrActionFilterAnatomyDetailDlg::RButtonDownList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;

		if (!pRow) {
			return;
		}

		if (pRow->Selected) {
			return;
		}

		// (a.walling 2014-08-13 15:02) - PLID 62691 - Add or set the selection upon rbuttondown
		if (nFlags & (MK_CONTROL | MK_SHIFT)) {
			pRow->Selected = VARIANT_TRUE;
		}
		else {
			m_pList->CurSel = pRow;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2014-08-13 15:02) - PLID 62691 - Laterality - CEmrActionAnatomyFilterDetailDlg - Context menu for (un)?check selected|all
void CEmrActionFilterAnatomyDetailDlg::ShowContextMenuList(LPDISPATCH lpRow, short nCol, long x, long y, long hwndFrom, BOOL* pbContinue)
{
	try {
		if (nCol < lcBilateral || nCol > lcNoSide) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;

		*pbContinue = FALSE;

		CString type = (const char*)m_pList->GetColumn(nCol)->ColumnTitle;

		CMenu mnu;
		mnu.CreatePopupMenu();

		enum MenuItem
		{
			miCheckAll = 1000,
			miUncheckAll,
			miCheckSelected,
			miUncheckSelected,
		};

		//TES 10/29/2015 - PLID 67313 - Don't allow them to Check All or Check Selected, they should just be selecting a few (or <Any>)
		//mnu.AppendMenu(MF_ENABLED, miCheckAll, FormatString("Check All %s", type));
		mnu.AppendMenu(MF_ENABLED, miUncheckAll, FormatString("Uncheck All %s", type));

		if (m_pList->CurSel != nullptr || pRow) {
			//TES 10/29/2015 - PLID 67313 - Don't allow them to Check All or Check Selected, they should just be selecting a few (or <Any>)
			//mnu.AppendMenu(MF_ENABLED, miCheckSelected, FormatString("Check Selected %s", type));
			mnu.AppendMenu(MF_ENABLED, miUncheckSelected, FormatString("Uncheck Selected %s", type));
		}

		UINT ret = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, x, y, this);

		switch (ret) {
		case miCheckAll:
		case miUncheckAll:
		{
			for (auto row = m_pList->GetFirstRow(); row; row = row->GetNextRow()) {
				auto varOldValue = row->Value[nCol];
				row->Value[nCol] = (ret == miCheckAll);
				UpdateRow(m_filter, m_anatomicLocationID, row, nCol, varOldValue, row->Value[nCol]);
			}
		}
			break;
		case miCheckSelected:
		case miUncheckSelected:
		{
			for (auto row = m_pList->GetFirstSelRow(); row; row = row->GetNextSelRow()) {
				auto varOldValue = row->Value[nCol];
				row->Value[nCol] = (ret == miCheckSelected);
				UpdateRow(m_filter, m_anatomicLocationID, row, nCol, varOldValue, row->Value[nCol]);
			}
			if (pRow) {
				auto varOldValue = pRow->Value[nCol];
				pRow->Value[nCol] = (ret == miCheckSelected);
				UpdateRow(m_filter, m_anatomicLocationID, pRow, nCol, varOldValue, pRow->Value[nCol]);
			}
		}
			break;
		}
	} NxCatchAll(__FUNCTION__);
}


BOOL CEmrActionFilterAnatomyDetailDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDOK))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);

		CWnd* pDesc = GetDlgItem(IDC_DESCRIPTION);
		CString strDesc;
		pDesc->GetWindowText(strDesc);
		strDesc += m_anatomicLocationName;
		pDesc->SetWindowText(strDesc);

		{
			using namespace NXDATALIST2Lib;

			m_pList = BindNxDataList2Ctrl(IDC_LIST, false);

			m_pList->AllowMultiSelect = VARIANT_TRUE;

			AppendColumns(m_pList, {
					{ "", "ID", 0, csFixedWidth },
					{ "", "Bilateral", 60, csFixedWidth | csEditable, cftBoolCheckbox },
					{ "", "Left", 60, csFixedWidth | csEditable, cftBoolCheckbox },
					{ "", "Right", 60, csFixedWidth | csEditable, cftBoolCheckbox },
					{ "", "No Side", 60, csFixedWidth | csEditable, cftBoolCheckbox },
					{ "", "Qualifier", 0, csWidthAuto, cftTextSingleLine, 0 },
			});

			long anySide = m_filter.GetAnyQualifierSide(m_anatomicLocationID);
			long noSide = m_filter.GetNoQualifierSide(m_anatomicLocationID);

			auto range = m_filter.equal_range(m_anatomicLocationID);

			// (a.walling 2014-09-11 15:25) - PLID 62689 - Parent dialog handles any fanciness with the details; this just simply reflects what is in the map, as it should.


			m_pList->AddRowAtEnd(MakeNewRow(m_pList, {
				-1
				, !!((anySide & Emr::sideBilateral) == Emr::sideBilateral)
				, !!(anySide & Emr::sideLeft)
				, !!(anySide & Emr::sideRight)
				, !!(anySide & Emr::sideNone)
				, (const char*)"<any qualifier>"
			}), nullptr);

			if (noSide == -1) {
				noSide = 0;
			}
			m_pList->AddRowAtEnd(MakeNewRow(m_pList, {
				0
				, !!((noSide & Emr::sideBilateral) == Emr::sideBilateral)
				, !!(noSide & Emr::sideLeft)
				, !!(noSide & Emr::sideRight)
				, !!(noSide & Emr::sideNone)
				, (const char*)"<no qualifier>"
			}), nullptr);

			for (ADODB::_RecordsetPtr prs = CreateRecordsetStd("SELECT ID, Name, Inactive FROM AnatomyQualifiersT ORDER BY Name"); !prs->eof; prs->MoveNext()) {
				long id = AdoFldLong(prs, "ID");
				CString name = AdoFldString(prs, "Name");
				bool inactive = !!AdoFldBool(prs, "Inactive");

				long side = 0;

				auto it = m_filter.anatomicLocationFilters.find({ m_anatomicLocationID, id });

				if (it == m_filter.anatomicLocationFilters.end()) {
					if (inactive) {
						// inactive and does not exist in the filter, so ignore
						continue;
					}
				}
				else {
					side = it->anatomySide;
				}

				auto row = MakeNewRow(m_pList, {
					id
					, !!((side & Emr::sideBilateral) == Emr::sideBilateral)
					, !!(side & Emr::sideLeft)
					, !!(side & Emr::sideRight)
					, !!(side & Emr::sideNone)
					, (const char*)name
				});

				if (inactive) {
					row->ForeColor = HEXRGB(0x808080);
				}

				m_pList->AddRowAtEnd(row, nullptr);
			}
		}
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

