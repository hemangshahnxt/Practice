// FilterDetailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "FilterDlg.h"
#include "FilterDetailDlg.h"
#include "FilterDetail.h"
#include "FilterFieldInfo.h"
#include "FilterEditDlg.h"
#include "resource.h"
#include "letterwritingrc.h"

#include "GlobalDataUtils.h"
#include "nxmessagedef.h"
#include "InternationalUtils.h"
#include "GlobalDrawingUtils.h"
#include "MultiSelectDlg.h"

//This include should be "filterdetailcallback.h"
#include "Groups.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CFilterDetailDlg dialog

//Note: I'm starting these arbitrarily at 32767, because a couple of other places seem to arbitrarily start there,
//and no places seem to define this sort of thing non-arbitrarily.
// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_ADD		33767
#define ID_EDIT		33768
#define ID_DELETE	33769

CFilterDetailDlg::CFilterDetailDlg(CWnd* pParent, WPARAM nDetailData, long nFilterType, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&))
	: CNxDialog(CFilterDetailDlg::IDD, (CNxView*)pParent),
	CFilterDetail(nFilterType, pfnGetNewFilterString)
{
	//{{AFX_DATA_INIT(CFilterDetailDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nDetailData = -1;
	m_bUseOrAfter = true;
	m_pfnIsActionSupported = pfnIsActionSupported;
	m_pfnCommitSubfilterAction = pfnCommitSubfilterAction;

	m_strMultiItemValues = "";
}

void CFilterDetailDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterDetailDlg)
	DDX_Control(pDX, IDC_OPERATOR_COMBO, m_cboOperators);
	DDX_Control(pDX, IDC_MULTI_VALUE_LIST, m_nxlMultiValueListLabel);
	DDX_Control(pDX, IDC_VALUE_EDIT, m_nxeditValueEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFilterDetailDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFilterDetailDlg)
	ON_CBN_SELCHANGE(IDC_OPERATOR_COMBO, OnSelChangeOperatorCombo)
	ON_BN_CLICKED(IDC_USE_OR_BTN, OnClickOrDoubleClickUseOrBtn)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDC_FIELD_BTN, OnFieldBtn)
	ON_BN_CLICKED(IDC_EDIT_SUBFILTER, OnEditSubfilter)
	ON_COMMAND(ID_ADD, OnAdd)
	ON_COMMAND(ID_EDIT, OnEdit)
	ON_COMMAND(ID_DELETE, OnDelete)
	ON_BN_DOUBLECLICKED(IDC_USE_OR_BTN, OnClickOrDoubleClickUseOrBtn)
	ON_EN_KILLFOCUS(IDC_VALUE_EDIT, OnKillfocusValueEdit)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CFilterDetailDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFilterDetailDlg)
	ON_EVENT(CFilterDetailDlg, IDC_COMBO_FIELD, 16 /* SelChosen */, OnSelChosenField, VTS_I4)
	ON_EVENT(CFilterDetailDlg, IDC_VALUE_LIST, 16 /* SelChosen */, OnSelChosenValueList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDetailDlg message handlers

BOOL CFilterDetailDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	//these filters are only allowed under Nextech-only licenses
	m_nextechFilter.Add("Allow Support");
	m_nextechFilter.Add("Support Expires");
	m_nextechFilter.Add("Support Rating");
	m_nextechFilter.Add("Reference Rating");
	m_nextechFilter.Add("Reference Status");
	//(J.Camacho 2013-03-12 10:28) - PLID 53866 - Added License Key to the filter list.
	m_nextechFilter.Add("License Key");

	// (b.cardillo 2006-05-19 18:08) - PLID 20593 - No need to ensure the database since this 
	// datalist is not actually bound to data.  Just do regular com object init of the controls.
	m_dlFields = GetDlgItem(IDC_COMBO_FIELD)->GetControlUnknown();
	m_dlValueList = GetDlgItem(IDC_VALUE_LIST)->GetControlUnknown();
	
	// Empty the field combo (it will be filled automatically when set the detail
	////////////////////////////////////////////
	//m_cboFields.ResetContent();
	//m_cboFields.SetDroppedWidth(275);
	m_dlFields->Clear();
	m_dlValueList->Clear();
	////////////////////////////////////////////
	
	// Make it so no field is selected
	m_nDetailData = -1;
	CFilterDetail::SetDetail(-1, foInvalidOperator, NULL, false, -1);

	m_nxlMultiValueListLabel.SetColor(0x00FFFFFF);
	m_nxlMultiValueListLabel.SetText("");
	m_nxlMultiValueListLabel.SetType(dtsHyperlink);

#ifdef _DEBUG
//	m_cboFields.ShowWindow(SW_HIDE);
//	GetDlgItem(IDC_FIELD_BTN)->ShowWindow(SW_SHOW);
#endif	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFilterDetailDlg::SetPosition(long nIndexPosition)
{
	// Call base class implementation
	CFilterDetail::SetPosition(nIndexPosition);
	
	// Extra stuff for this derived class
	SetDlgCtrlID(nIndexPosition);
	m_nDetailData = nIndexPosition;
	CWnd *pWnd = GetParent();
	if (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CFilterDlg))) {
		SetWindowPos(NULL, 0, ((CFilterDlg *)pWnd)->m_nScrolledHeight + nIndexPosition * ITEM_SIZE_VERT, 0, 0, SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOSIZE);
	}
}

void CFilterDetailDlg::SetUseOr(bool bUseOr)
{
	// Call base class implementation
	CFilterDetail::SetUseOr(bUseOr);
	
	// Extra stuff for this derived class
	GetParent()->PostMessage(NXM_FILTER_USE_OR_CHANGED, m_nDetailData);
}

void CFilterDetailDlg::RefreshFieldType()
{
	// Make sure the field dropdown contains the field list
	////////////////////////////////////////////
	//if (m_cboFields.GetCount() == 0) {
	//	m_cboFields.ResetContent();
	////////////////////////////////////////////
	if (m_dlFields->GetRowCount() == 0) {
		m_dlFields->Clear();
		for (long i=0; i<g_nFilterFieldCount; i++) {
			////////////////////////////////////////////
			//m_cboFields.InsertString(i, g_FilterFields[i].GetFieldNameApparent());
			
			

			
			if(AllowFilter(g_FilterFields[i].GetFieldNameApparent())  //DRT 7/3/02 - Check to see if the filter being added is allowed to work with this current license structure.
				&& (g_FilterFields[i].m_nFilterType == m_nFilterType) //TES 3/18/03 - Only add it if it is one of the fields that's based on the same thing we are.
				&& g_FilterFields[i].IsVisible() ) {				  //Don't add it if it's marked as "invisible."

				//Now, is this a dynamic field?
				if(g_FilterFields[i].GetFieldNameApparent().Left(13) == "DYNAMIC_FIELD") {
					//Aha!  Let's loop through and add all the appropriate fields.
					_RecordsetPtr rsDynamicList = CreateRecordsetStd(g_FilterFields[i].GetFieldNameApparent().Mid(14));
					while(!rsDynamicList->eof) {
						IRowSettingsPtr pRow = m_dlFields->Row[-1];
						pRow->Value[0] = i;
						pRow->Value[1] = rsDynamicList->Fields->GetItem("Name")->Value;
						pRow->Value[2] = rsDynamicList->Fields->GetItem("ID")->Value;
						m_dlFields->AddRow(pRow);
						rsDynamicList->MoveNext();
					}					
				}
				else {
					//OK, we'll just add this row.
					IRowSettingsPtr pRow = m_dlFields->Row[-1];
					pRow->Value[0] = i;
					CString strFieldName = g_FilterFields[i].GetFieldNameApparent();
					((CFilterDlg*)m_pParent)->FormatFieldNameApparent(strFieldName);
					pRow->Value[1] = (LPCTSTR)strFieldName;
					_variant_t varNull;
					varNull.vt = VT_NULL;
					pRow->Value[2] = varNull;//There's no dynamic index, since this isn't dynamic.
					m_dlFields->AddRow(pRow);
				}
			}

			//m_dlFields->InsertRow(pRow, i);
			////////////////////////////////////////////
		}
		//The very first record is always the "None" record.
		IRowSettingsPtr pNoneRow = m_dlFields->Row[-1];
		pNoneRow->Value[0] = (long)-1;
		pNoneRow->Value[1] = _bstr_t("{ None }");
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pNoneRow->Value[2] = varNull;
		m_dlFields->InsertRow(pNoneRow, 0);
	}	

	if (m_nFieldIndex == -1) {
		////////////////////////////////////////////
		// Unselect everything from the combos
		//m_cboFields.Clear();
		m_dlFields->PutCurSel(-1);
		////////////////////////////////////////////
	} else {
		if(g_FilterFields[m_nFieldIndex].IsRemoved()) {
			//This field won't be in the list, but we need to show it.  Let's add it manually.
			//But first, let's make double-sure it's not already in there.
			if(m_dlFields->FindByColumn(0, m_nFieldIndex, 0, FALSE) == -1) {
				IRowSettingsPtr pRow = m_dlFields->GetRow(-1);
				pRow->PutValue(0, m_nFieldIndex);
				pRow->PutValue(1, _bstr_t(g_FilterFields[m_nFieldIndex].GetFieldNameApparent()));
				if(m_nDynamicRecordID == INVALID_DYNAMIC_ID) {
					_variant_t varNull;
					varNull.vt = VT_NULL;
					pRow->PutValue(2, varNull);
				}
				else {
					pRow->PutValue(2, m_nDynamicRecordID);
				}
				m_dlFields->AddRow(pRow);
			}
		}

		// Select the appropriate item from the field dropdown
		////////////////////////////////////////////
		//m_cboFields.SetCurSel(m_nFieldIndex);
		if(m_nDynamicRecordID != INVALID_DYNAMIC_ID) {
			//OK, we have to find the row with the right index and dynamicindex
			long p = m_dlFields->GetFirstRowEnum();
			int i = 0;
			
			while (p)
			{
				LPDISPATCH pDisp;
				m_dlFields->GetNextRowEnum(&p, &pDisp);
				IRowSettingsPtr pRow(pDisp);
				pDisp->Release();
			
				if(VarLong(pRow->GetValue(0)) == m_nFieldIndex) {
					_variant_t varDynamic = pRow->GetValue(2);
					if(varDynamic.vt == VT_I4) {
						if(VarLong(varDynamic) == m_nDynamicRecordID) {
							//TES: This isn't guaranteed safe according to the datalist. It needs a PutCurSelByEnum() function.
							//However, we know (because we share an office with Bob) that the datalist does happen
							//to put its rows in the linked list in the same order that they are on the screen, so this
							//is safe.
							m_dlFields->CurSel = i; 
							return;
						}
					}
				}				
				i++;
			}
		}
		m_dlFields->SetSelByColumn(0, m_nFieldIndex);
		////////////////////////////////////////////
	}

	
}

void CFilterDetailDlg::OnSelChangeOperatorCombo()
{
	// Get the currently selected operator
	long nCurOp = foInvalidOperator;
	if (m_cboOperators.GetCurSel() >= 0) {
		nCurOp = m_cboOperators.GetItemData(m_cboOperators.GetCurSel());
	}

	switch (nCurOp) {
	case foBlank:
	case foNotBlank:
		GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_VALUE_LIST)->EnableWindow(FALSE);
		break;
	default:
		GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_VALUE_LIST)->EnableWindow(TRUE);
		break;
	}
}

void CFilterDetailDlg::DoMultipleSelectDlg()
{
	// Init the dialog and fill with existing selections
	// (j.armen 2012-06-20 15:23) - PLID 49607 - This could be coming from many places, so we'll use a unique entry
	CMultiSelectDlg dlg(this, "CFilterDetailDlg::DoMultipleSelectDlg");

	CString strIDList = m_strMultiItemValues;
	long nID, nNumLength;
	while (strIDList.GetLength() != 0) {
		nNumLength = strIDList.Find(",");
		if (nNumLength == -1)
			nNumLength = strIDList.GetLength();
		nID = atoi(strIDList.Left(nNumLength));
		strIDList = strIDList.Right((strIDList.GetLength() - nNumLength - 1) <= strIDList.GetLength() ? (strIDList.GetLength() - nNumLength - 1) : (strIDList.GetLength() - nNumLength));

		dlg.PreSelect(nID);
	}

	// Get the name of the item type if this is a custom item
	CString strColTitle = "";
	strColTitle = g_FilterFields[m_nFieldIndex].GetFieldNameApparent();
	if (strColTitle.Find("REPLACE_FIELD_NAME") != -1) {
		CString strQuery;
		strQuery = strColTitle.Right(strColTitle.GetLength() - (strColTitle.Find("):") + 2));
		_RecordsetPtr rsName = CreateRecordset(strQuery);
		if(!rsName->eof) {		
			strColTitle = AdoFldString(rsName, "Name", "");
		}
	}
	dlg.m_strNameColTitle = strColTitle;

	// Get all of the parameters
	LPCTSTR pstrParamID = g_FilterFields[m_nFieldIndex].GetNextParam(NULL, TRUE), pstrParamName = NULL, pstrParamFrom = NULL, pstrParamWhere = NULL;
	if (pstrParamID && *pstrParamID)
		pstrParamName = g_FilterFields[m_nFieldIndex].GetNextParam(pstrParamID, TRUE);
	if (pstrParamName && *pstrParamName)
		pstrParamFrom = g_FilterFields[m_nFieldIndex].GetNextParam(pstrParamName, TRUE);
	if (pstrParamFrom && *pstrParamFrom)
		pstrParamWhere = g_FilterFields[m_nFieldIndex].GetNextParam(pstrParamFrom, TRUE);

	if (pstrParamID == NULL || pstrParamName == NULL || pstrParamFrom == NULL)
		// All of the parameters but the Where need to be set.  If this happens someone didn't give enough parameters for this multi select list
		ASSERT(FALSE);

	// Do the multi select dialog
	HRESULT hRes;
	hRes = dlg.Open(pstrParamFrom, pstrParamWhere, pstrParamID, pstrParamName, "Please select the items to be compared with the filter field.", 1);

	// Process the results of the dialog
	if (hRes == IDOK) {
		m_strMultiItemValues = dlg.GetMultiSelectIDString(",");
		if (m_strMultiItemValues.Find(",") == -1) {
			// This means only one item is selected, so lets just select it
			for (int k=0; k < m_dlValueList->GetRowCount(); k++) {
				if (VarString(m_dlValueList->GetValue(k, 0)) == m_strMultiItemValues)
					m_dlValueList->CurSel = k;
			}
			m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_SHOW);
		}
		else {
			// Multiple items were selected
			m_nxlMultiValueListLabel.SetText(" " + dlg.GetMultiSelectString());
			m_nxlMultiValueListLabel.Invalidate();
			m_nxlMultiValueListLabel.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_HIDE);
		}
		m_lCurSelIndex = m_dlValueList->CurSel;
	}
	else
		m_dlValueList->CurSel = m_lCurSelIndex;
}

void CFilterDetailDlg::OnSelChosenValueList(long nRow)
{
	try {

		if(m_dlValueList->CurSel == -1) {
			m_lCurSelIndex = -1;
			return;
		}

		// Only used for combo select lists
		if (VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0)) == "-2") {
			DoMultipleSelectDlg();
		}
		else {
			m_strMultiItemValues = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
			m_lCurSelIndex = m_dlValueList->CurSel;
		}

	}NxCatchAll("Error in CFilterDetailDlg::OnSelChosenValueList");
}

/*void CFilterDetailDlg::OnSelEndOkFieldCombo() 
{
	// Get the current value
	CString strLastValue;
	if (m_nFieldIndex >= 0) {
		if (g_FilterFields[m_nFieldIndex].m_ftFieldType & (ftText|ftDate|ftTime|ftNumber)) {
			// Last value was in the text box
			GetDlgItemText(IDC_VALUE_EDIT, strLastValue);
		} else if (g_FilterFields[m_nFieldIndex].m_ftFieldType & (ftComboValues|ftComboSelect)) {
			// Last value was in the combo so grab the actual apparent text
			GetDlgItemText(IDC_VALUE_COMBO, strLastValue);
		}
	}

	// Get the currently selected operator
	long nLastOp = foInvalidOperator;
	if (m_cboOperators.GetCurSel() >= 0) {
		nLastOp = m_cboOperators.GetItemData(m_cboOperators.GetCurSel());
	}
	
	// Now set the detail in the base class which will automatically call our refresh
	CFilterDetail::SetDetail(m_cboFields.GetCurSel(), nLastOp, strLastValue, m_bUseOr);

	// Tell our parent that we just changed the selection
	GetParent()->PostMessage(NXM_FIELD_SEL_CHANGED, m_nDetailData, (LPARAM)&g_FilterFields[m_nFieldIndex]);
}*/

void CFilterDetailDlg::OnSelChosenField(long nRow)
{
	try {

		CWnd nRow;

		long nCurSel = m_dlFields->GetCurSel();
		long nIndex = (nCurSel != -1) ? VarLong(m_dlFields->GetValue(nCurSel, 0)) : -1;

		if (nCurSel == -1 || nCurSel == 0) { //This is the (framework-maintained) {None} row.
			CFilterDlg *pParent = static_cast<CFilterDlg *>(GetParent());
			ASSERT(pParent);
			if (pParent && ((long)m_nDetailData == (pParent->m_nItemCount - 1))) { 
				m_dlFields->CurSel = -1;
				return;
			}
			GetParent()->PostMessage(NXM_FILTER_REMOVE_ITEM, m_nDetailData);
		}

		if(nCurSel == -1)
			return;

		// Get the current value
		CString strLastValue;
		if (m_nFieldIndex >= 0) {
			if (g_FilterFields[m_nFieldIndex].m_ftFieldType & (ftText|ftPhoneNumber|ftDate|ftTime|ftNumber|ftCurrency)) {
				// Last value was in the text box
				GetDlgItemText(IDC_VALUE_EDIT, strLastValue);				
			} else if (g_FilterFields[m_nFieldIndex].m_ftFieldType & (ftComboValues|ftComboSelect)) {
				// Last value was in the combo so grab the actual apparent text
				GetDlgItemText(IDC_VALUE_COMBO, strLastValue);
			}
			else if(g_FilterFields[m_nFieldIndex].m_ftFieldType & (ftSubFilter|ftSubFilterEditable)) {
				strLastValue = "";
			}
		}

		// Get the currently selected operator
		long nLastOp = foInvalidOperator;
		if (m_cboOperators.GetCurSel() >= 0) {
			nLastOp = m_cboOperators.GetItemData(m_cboOperators.GetCurSel());
		}

		//Get the dynamic ID
		_variant_t varDynamicID = m_dlFields->GetValue(nCurSel, 2);
		long nDynamicID = INVALID_DYNAMIC_ID;
		if(varDynamicID.vt == VT_I4) {
			nDynamicID = VarLong(varDynamicID);
		}

		if(g_FilterFields[nIndex].m_ftFieldType & ftPhoneNumber) {
			strLastValue = StripNonPhoneNumber(strLastValue);
		}
		if(g_FilterFields[nIndex].m_ftFieldType & ftCurrency) {
			strLastValue = FormatCurrencyForSql(ParseCurrencyFromInterface(strLastValue));
		}

		// Now set the detail in the base class which will automatically call our refresh
		CFilterDetail::SetDetail(nIndex, nLastOp, strLastValue, m_bUseOr, nDynamicID);

		// Tell our parent that we just changed the selection
		GetParent()->PostMessage(NXM_FIELD_SEL_CHANGED, m_nDetailData, (LPARAM)&g_FilterFields[m_nFieldIndex]);
	} NxCatchAll("CFilterDetailDlg::OnSelChosenField");
}

// Given a certain field (in g_FilterFields, specified by index), is the operator efo available?
#define HAS_OP(index, efo)					(((g_FilterFields[index].m_nAvailableOperators) & (efo)) != 0)

// Macros for entering all the operators that are available to 
// the g_FilterFields[index] field, into the combo box specified 
// by cbo; the integer cnt will tell how many were added
#define BEGIN_ADD_OP(cbo, index,cnt)	{ long &nScopeAddOpIndex = index; long &nScopeAddOpCount = cnt; CComboBox &cboScopeOperators = cbo; long nScopeOperatorDefault; if (m_foOperator != foInvalidOperator) nScopeOperatorDefault = m_foOperator; else nScopeOperatorDefault = g_FilterFields[nScopeAddOpIndex].m_foDefault; cboScopeOperators.ResetContent(); nScopeAddOpCount = 0;
#define ADD_OP(efo, txt)					if (HAS_OP(nScopeAddOpIndex, efo)) { cboScopeOperators.SetItemData(cboScopeOperators.InsertString(nScopeAddOpCount++, txt), efo); if ((efo) == nScopeOperatorDefault) cboScopeOperators.SetCurSel(nScopeAddOpCount-1); }
#define END_ADD_OP()							}

void CFilterDetailDlg::RefreshOperator()
{
	if (m_nFieldIndex == -1) {
		// Disable the operators combo
		m_cboOperators.Clear();
		m_cboOperators.EnableWindow(FALSE);
	} else {
		// Try to add each field
		long nCount;
		BEGIN_ADD_OP(m_cboOperators, m_nFieldIndex, nCount)
			ADD_OP(foEqual, "Is Equal To (=)");
			ADD_OP(foGreater, "Is Greater Than (>)");
			ADD_OP(foGreaterEqual, "Is Greater Than or Equal To (>=)");
			ADD_OP(foLess, "Is Less Than (<)");
			ADD_OP(foLessEqual, "Is Less Than or Equal To (<=)");
			ADD_OP(foNotEqual, "Is Not Equal To (<>)");
			ADD_OP(foLike, "Is Like");
			ADD_OP(foBeginsWith, "Begins With");
			ADD_OP(foEndsWith, "Ends With");
			ADD_OP(foContains, "Contains");
			ADD_OP(foIn, "Is In");
			ADD_OP(foNotIn, "Is NOT In");
			ADD_OP(foBlank, "Is Blank");
			ADD_OP(foNotBlank, "Is NOT Blank");
		END_ADD_OP();

		// Decide how to display the combo
		if (m_cboOperators.GetCount() == 0) {
			m_cboOperators.EnableWindow(FALSE);
		} else {
			m_cboOperators.EnableWindow(TRUE);
		}
	}
}

CString CFilterDetailDlg::CreateStringListFromIDs(CString strIDList)
{
	CArray<long, long> aryIDs;
	CArray<CString, CString> aryNames;
	CString strQuery = g_FilterFields[m_nFieldIndex].m_pstrParameters, strResult = "";
	_RecordsetPtr rsItems = CreateRecordset(strQuery);
	if(!rsItems->eof) {
		while (!rsItems->eof) {
			aryIDs.Add(AdoFldLong(rsItems, "ID", -1));
			aryNames.Add(AdoFldString(rsItems, "Name", ""));
			rsItems->MoveNext();
		}

		long nNumLength = 0, nID;
		strIDList.TrimLeft("\"");
		strIDList.TrimRight("\"");
		while (strIDList.GetLength() != 0) {
			nNumLength = strIDList.Find(",");
			if (nNumLength == -1)
				nNumLength = strIDList.GetLength();
			nID = atoi(strIDList.Left(nNumLength));
			strIDList = strIDList.Right((strIDList.GetLength() - nNumLength - 1) <= strIDList.GetLength() ? (strIDList.GetLength() - nNumLength - 1) : (strIDList.GetLength() - nNumLength));

			// Bring item with that ID over to selected list
			for (int i=0; i < aryIDs.GetSize(); i++) {
				if (aryIDs.GetAt(i) == nID)
					strResult += (aryNames.GetAt(i) + ", ");
			}
		}
		strResult.TrimRight(", ");
	}

	return strResult;		
}

void CFilterDetailDlg::RefreshValue()
{
	if (m_nFieldIndex == -1) {
		// Clear and disable the value box
		SetDlgItemText(IDC_VALUE_EDIT, "");
		GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(FALSE);
		GetDlgItem(IDC_VALUE_EDIT)->ShowWindow(SW_SHOW);
		// Make value combo invisible
		GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_HIDE);
		// Hide NxLabel
		m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
	} else {
		// Now affect the new change
		switch (g_FilterFields[m_nFieldIndex].m_ftFieldType) {
		case ftComboValues:
			{
			// Clear the value edit box text and disable and hide the box
			SetDlgItemText(IDC_VALUE_EDIT, "");
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VALUE_EDIT)->ShowWindow(SW_HIDE);
			//TES 3/26/2007 - PLID 25353 - Make sure that we hide the "..." button, if it's there.
			//Now, make sure that the combo box fills the whole area it should
			//Start one pixel to the right of the operator combo, go to the end, don't change the height.
			CRect rOperator, rDlg, rCombo;
			GetDlgItem(IDC_OPERATOR_COMBO)->GetWindowRect(rOperator);
			GetWindowRect(rDlg);
			GetDlgItem(IDC_VALUE_LIST)->GetWindowRect(rCombo);
			GetDlgItem(IDC_VALUE_LIST)->MoveWindow(rOperator.right + 1 - rDlg.left, 0, rDlg.right - (rOperator.right + 1), rCombo.Height());
			GetDlgItem(IDC_EDIT_SUBFILTER)->ShowWindow(SW_HIDE);

			//TES 3/26/2007 - PLID 20528 - We also need to hide the CreatedDate and ModifiedDate columns, as well as
			// the column headers, and reduce the dropdown width.
			m_dlValueList->HeadersVisible = g_cvarFalse;
			m_dlValueList->DropDownWidth = -1;
			m_dlValueList->GetColumn(2)->ColumnStyle = m_dlValueList->GetColumn(2)->ColumnStyle & (~csVisible);
			m_dlValueList->GetColumn(3)->ColumnStyle = m_dlValueList->GetColumn(3)->ColumnStyle & (~csVisible);

			// Reset the combo and fill it with the possible values specified by the fieldinfo parameters
			m_dlValueList->Clear();
			bool bDefaultSet = false;
			long nItem = 0;
			
			// If this detail has the { Multiple Items } option, add it
			if (g_FilterFields[m_nFieldIndex].m_pstrMultiSelectParams) {
				IRowSettingsPtr pRow = m_dlValueList->GetRow(-1);
				pRow->PutValue(0, _bstr_t("-2"));
				pRow->PutValue(1, _bstr_t("{ Multiple Items }"));
				m_dlValueList->AddRow(pRow);
				
				if (!bDefaultSet && m_strValue.Find(",") != -1) {
					// This means the default is multiple items
					m_dlValueList->CurSel = m_dlValueList->GetRowCount()-1;
					m_strMultiItemValues = m_strValue;
					m_nxlMultiValueListLabel.SetText(" " + CreateStringListFromIDs(m_strValue));
					m_nxlMultiValueListLabel.Invalidate();
					bDefaultSet = true;
				}
			}

			LPCTSTR pstrParamName = g_FilterFields[m_nFieldIndex].GetNextParam(NULL), pstrParamData;

			// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
			long i = 0;
			for (i=0; pstrParamName && *pstrParamName; i++) {
				pstrParamData = g_FilterFields[m_nFieldIndex].GetNextParam(pstrParamName);
				if (pstrParamData && *pstrParamData) {
					// Put the value in the dropdown
					IRowSettingsPtr pRow = m_dlValueList->GetRow(-1);
					pRow->PutValue(0, _bstr_t(pstrParamData));
					pRow->PutValue(1, _bstr_t(pstrParamName));
					m_dlValueList->AddRow(pRow);
					// Set the default if it was passed
					if (!bDefaultSet && (
						 (strcmp(pstrParamData, m_strValue) == 0) ||
						 (stricmp(pstrParamName, m_strValue) == 0))) {
						m_dlValueList->CurSel = g_FilterFields[m_nFieldIndex].m_pstrMultiSelectParams ? i + 1 : i;
						m_strMultiItemValues = m_strValue;
						bDefaultSet = true;
					}
				} else {
					// There wasn't a data parameter for this name parameter
					break;
				}
				// Find the next item
				pstrParamName = g_FilterFields[m_nFieldIndex].GetNextParam(pstrParamData);
			}
			// If a default hasn't been set, set it to the first value
			if (!bDefaultSet && i>0) {
				if (VarString(m_dlValueList->GetValue(0,0)) == "-2")
					m_dlValueList->CurSel = 1;
				else
					m_dlValueList->CurSel = 0;
				m_strMultiItemValues = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
			}
			if (VarString(m_dlValueList->GetValue(0,0)) == "-2") {
				GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_HIDE);
				m_nxlMultiValueListLabel.ShowWindow(SW_SHOW);
			}
			else {
				// Show the combo box
				GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_SHOW);
				// Hide NxLabel
				m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
			}
			}
			break;
		case ftComboSelect:
		case ftSubFilter: //Subfilter is a special kind of ComboSelect
			{
			// Clear the value edit box text and disable and hide the box
			SetDlgItemText(IDC_VALUE_EDIT, "");
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VALUE_EDIT)->ShowWindow(SW_HIDE);
			//Now, make sure that the combo box fills the whole area it should
			//Start one pixel to the right of the operator combo, go to the end, don't change the height.
			CRect rOperator, rDlg, rCombo;
			GetDlgItem(IDC_OPERATOR_COMBO)->GetWindowRect(rOperator);
			GetWindowRect(rDlg);
			GetDlgItem(IDC_VALUE_LIST)->GetWindowRect(rCombo);
			GetDlgItem(IDC_VALUE_LIST)->MoveWindow(rOperator.right + 1 - rDlg.left, 0, rDlg.right - (rOperator.right + 1), rCombo.Height());
			GetDlgItem(IDC_EDIT_SUBFILTER)->ShowWindow(SW_HIDE);

			//TES 3/26/2007 - PLID 20528 - We also need to hide the CreatedDate and ModifiedDate columns, as well as
			// the column headers, and reduce the dropdown width.
			m_dlValueList->HeadersVisible = g_cvarFalse;
			m_dlValueList->DropDownWidth = -1;
			m_dlValueList->GetColumn(2)->ColumnStyle = m_dlValueList->GetColumn(2)->ColumnStyle & (~csVisible);
			m_dlValueList->GetColumn(3)->ColumnStyle = m_dlValueList->GetColumn(3)->ColumnStyle & (~csVisible);

			// Reset the combo and fill it with the possible values specified by the fieldinfo parameters
			m_dlValueList->Clear();
			CString strId, strName;
			bool bDefaultSet = false;
			try {
				// If this detail has the { Multiple Items } option, add it
				if (g_FilterFields[m_nFieldIndex].m_pstrMultiSelectParams) {
					IRowSettingsPtr pRow = m_dlValueList->GetRow(-1);
					pRow->PutValue(0, _bstr_t("-2"));
					pRow->PutValue(1, _bstr_t("{ Multiple Items }"));
					m_dlValueList->AddRow(pRow);
					
					if (!bDefaultSet && m_strValue.Find(",") != -1) {
						// This means the default is multiple items
						m_dlValueList->CurSel = m_dlValueList->GetRowCount()-1;
						m_strMultiItemValues = m_strValue;
						m_nxlMultiValueListLabel.SetText(" " + CreateStringListFromIDs(m_strValue));
						m_nxlMultiValueListLabel.Invalidate();
						bDefaultSet = true;
					}
				}

				// Add of of the items to the combo
				CString strQuery = g_FilterFields[m_nFieldIndex].m_pstrParameters;
				if(m_nDynamicRecordID != INVALID_DYNAMIC_ID) {
					CString strDynamicID;
					strDynamicID.Format("%li", m_nDynamicRecordID);
					strQuery.Replace("{DYNAMIC_ID}", strDynamicID);
				}
				_RecordsetPtr prs = CreateRecordsetStd(strQuery);
				FieldsPtr flds = prs->Fields;
				FieldPtr fldID = flds->Item[_T("ID")];
				FieldPtr fldName = flds->Item[_T("Name")];
				// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
				long i = 0;
				for (i=0; !prs->eof; i++) {
					// Put the value in the dropdown
					strId = AsString(fldID->Value);
					strName = AsString(fldName->Value);
					IRowSettingsPtr pRow = m_dlValueList->GetRow(-1);
					pRow->PutValue(0, _bstr_t(strId));
					pRow->PutValue(1, _bstr_t(strName));
					m_dlValueList->AddRow(pRow);
					// Set the default if it was passed
					if (!bDefaultSet && (
						 (stricmp(strId, m_strValue) == 0) ||
						 (stricmp(strName, m_strValue) == 0))) {
						m_dlValueList->CurSel = g_FilterFields[m_nFieldIndex].m_pstrMultiSelectParams ? i + 1 : i;
						m_strMultiItemValues = m_strValue;
						bDefaultSet = true;
					}
					// Find the next item
					HR(prs->MoveNext());
				}
				HR(prs->Close());

				// If a default hasn't been set, set it to the first value
				if (!bDefaultSet && i>0) {
					if (VarString(m_dlValueList->GetValue(0,0)) == "-2")
						m_dlValueList->CurSel = 1;
					else
						m_dlValueList->CurSel = 0;
					m_strMultiItemValues = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
				}

				// (j.jones 2006-07-20 14:54) - PLID 21540 - no point in showing the
				// multiple items row if there aren't any items at all
				if(i==0) {
					m_dlValueList->Clear();
				}

				m_lCurSelIndex = m_dlValueList->CurSel;
			} NxCatchAll("CFilterDetailDlg::RefreshValue");
			if (m_dlValueList->CurSel != -1 && VarString(m_dlValueList->GetValue(m_dlValueList->CurSel,0)) == "-2") {
				GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_HIDE);
				m_nxlMultiValueListLabel.ShowWindow(SW_SHOW);
			}
			else {
				// Show the combo box
				GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_SHOW);
				// Hide NxLabel
				m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
			}
			}
			break;
		case ftSubFilterEditable:
			{
			// Hide NxLabel
			m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
			// Clear the value edit box text and disable and hide the box
			SetDlgItemText(IDC_VALUE_EDIT, "");
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VALUE_EDIT)->ShowWindow(SW_HIDE);
			//Now, make sure that the combo box fills the whole area it should
			//Start one pixel to the right of the operator combo, go to one pixel to the left of the ..., don't change the height.
			CRect rOperator, rDlg, rButton, rCombo;
			GetDlgItem(IDC_OPERATOR_COMBO)->GetWindowRect(rOperator);
			GetDlgItem(IDC_EDIT_SUBFILTER)->GetWindowRect(rButton);
			GetWindowRect(rDlg);
			GetDlgItem(IDC_VALUE_LIST)->GetWindowRect(rCombo);
			GetDlgItem(IDC_VALUE_LIST)->MoveWindow(rOperator.right + 1 - rDlg.left, 0, rButton.left - (rOperator.right + 1), rCombo.Height());
			GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_EDIT_SUBFILTER)->ShowWindow(SW_SHOW);

			//TES 3/26/2007 - PLID 20528 - We also need to show the CreatedDate and ModifiedDate columns, as well as
			// the column headers, and expand the dropdown width.
			m_dlValueList->HeadersVisible = g_cvarTrue;
			m_dlValueList->DropDownWidth = 400;
			m_dlValueList->GetColumn(2)->ColumnStyle = m_dlValueList->GetColumn(2)->ColumnStyle|csVisible;
			m_dlValueList->GetColumn(3)->ColumnStyle = m_dlValueList->GetColumn(3)->ColumnStyle|csVisible;
			// Reset the combo and fill it with the possible values specified by the fieldinfo parameters
			m_dlValueList->Clear();
			CString strId, strName;
			bool bDefaultSet = false;
			try {
				_RecordsetPtr prs = CreateRecordsetStd(g_FilterFields[m_nFieldIndex].m_pstrParameters);
				FieldsPtr flds = prs->Fields;
				FieldPtr fldID = flds->Item[_T("ID")];
				FieldPtr fldName = flds->Item[_T("Name")];
				//TES 3/26/2007 - PLID 20528 - Show the CreatedDate and ModifiedDate
				FieldPtr fldCreatedDate = flds->Item[_T("CreatedDate")];
				FieldPtr fldModifiedDate = flds->Item[_T("ModifiedDate")];
				_variant_t varNull;
				varNull.vt = VT_NULL;
				//Let's go ahead and throw in the "{One-time subfilter}" option.
				IRowSettingsPtr pRow = m_dlValueList->GetRow(-1);
				pRow->PutValue(0, _bstr_t("0-"));
				pRow->PutValue(1, _bstr_t("{One-time subfilter}"));
				pRow->PutValue(2, varNull);
				pRow->PutValue(3, varNull);
				m_dlValueList->InsertRow(pRow, 0);
				//Now, should we default to this?
				if(m_strValue.Left(2) == "0-") {
					m_dlValueList->CurSel = 0;
					m_dlValueList->PutValue(m_dlValueList->CurSel, 0, _bstr_t(m_strValue));
					bDefaultSet = true;
				}
				// (a.walling 2007-11-05 15:18) - PLID 27977 - VS2008 - for() loops
				long i = 1;
				for (i=1; !prs->eof; i++) {
					// Put the value in the dropdown
					strId = AsString(fldID->Value);
					strName = AsString(fldName->Value);
					IRowSettingsPtr pRow = m_dlValueList->GetRow(-1);
					pRow->PutValue(0, _bstr_t(strId));
					pRow->PutValue(1, _bstr_t(strName));
					pRow->PutValue(2, fldCreatedDate->Value);
					pRow->PutValue(3, fldModifiedDate->Value);
					m_dlValueList->AddRow(pRow);
					// Set the default if it was passed
					if (!bDefaultSet && (
						 (stricmp(strId, m_strValue) == 0) ||
						 (stricmp(strName, m_strValue) == 0))) {
						m_dlValueList->CurSel = i;
						bDefaultSet = true;
					}
					// Find the next item
					HR(prs->MoveNext());
				}
				HR(prs->Close());
				// If a default hasn't been set, set it to the first value
				if (!bDefaultSet && i>0) {
					m_dlValueList->CurSel = 0;
				}
			} NxCatchAll("CFilterDetailDlg::RefreshValue");
			// Show the combo box
			GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_SHOW);
			}
			break;

		case ftCurrency:
			SetDlgItemText(IDC_VALUE_EDIT, FormatCurrencyForInterface(ParseCurrencyFromSql(m_strValue)));
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_VALUE_EDIT)->ShowWindow(SW_SHOW);
			// Hide the combo box
			GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_SUBFILTER)->ShowWindow(SW_HIDE);
			// Hide NxLabel
			m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
			break;
		case ftText:
		case ftPhoneNumber:
		case ftDate:
		case ftNumber:
		case ftTime:
		default:
			// Set the text box to contain the appropriate value and enable the box
			SetDlgItemText(IDC_VALUE_EDIT, m_strValue);
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_VALUE_EDIT)->ShowWindow(SW_SHOW);
			// Hide the combo box
			GetDlgItem(IDC_VALUE_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_SUBFILTER)->ShowWindow(SW_HIDE);
			// Hide NxLabel
			m_nxlMultiValueListLabel.ShowWindow(SW_HIDE);
			break;
		}
	}

	// Enable/Disable the value box depending on the operator
	{
		// Get the currently selected operator
		long nCurOp = foInvalidOperator;
		if (m_cboOperators.GetCurSel() >= 0) {
			nCurOp = m_cboOperators.GetItemData(m_cboOperators.GetCurSel());
		}

		// If it's invalid or one of the "blank" operators, then the value field doesn't matter so it should be disabled
		switch (nCurOp) {
		case foBlank:
		case foNotBlank:
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_VALUE_LIST)->EnableWindow(FALSE);
			break;
		default:
			GetDlgItem(IDC_VALUE_EDIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_VALUE_LIST)->EnableWindow(TRUE);
			break;
		}
	}
}

void CFilterDetailDlg::RefreshUseOr()
{
	if (m_bUseOr) {
		SetDlgItemText(IDC_USE_OR_BTN, "or");
	} else {
		SetDlgItemText(IDC_USE_OR_BTN, "and");
	}
}

void CFilterDetailDlg::Refresh()
{
	// Select the appropriate item from the field dropdown
	RefreshFieldType();

	// Enable and fill the operators combo appropriately
	RefreshOperator();
	
	// Enable and default the value edit box
	RefreshValue();
	
	// Decide whether this is an AND or an OR
	RefreshUseOr();
}

bool CFilterDetailDlg::Store()
{
	if (m_nFieldIndex >= 0) {
		if(g_FilterFields[m_nFieldIndex].IsRemoved()) {
			//You can't store a field that's been removed.
			MsgBox("Warning: Obsolete field '%s' encountered.", g_FilterFields[m_nFieldIndex].GetFieldNameApparent());
			return false;
		}
		// Get the index of the currently selected item
		long nOpIndex = m_cboOperators.GetCurSel();
		// See if we got a valid index
		if (nOpIndex >= 0) {
			// If so, get the operator that index refers to
			m_foOperator = (FieldOperatorEnum)m_cboOperators.GetItemData(nOpIndex);
		} else {
			// If not, use the invalid operator
			m_foOperator = foInvalidOperator;
		}

		// Get the value
		switch (g_FilterFields[m_nFieldIndex].m_ftFieldType) {
		case ftCurrency:
			//m_strValue should at all times be formatted for sql.
			{
				CString strTmp;
				GetDlgItemText(IDC_VALUE_EDIT, strTmp);
				m_strValue = FormatCurrencyForSql(ParseCurrencyFromInterface(strTmp));
			}
			break;
		case ftText:
		case ftPhoneNumber:
		case ftNumber:
		case ftDate:
		case ftTime:
		case ftAdvanced:
			// Text, numbers, currencies and dates are typed into the edit box
			GetDlgItemText(IDC_VALUE_EDIT, m_strValue);
			break;
		case ftComboValues:
			// Value lists and select lists are entered via combo boxes
			if (m_dlValueList->CurSel >= 0)
				m_strValue = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
			else 
				m_strValue = "";
			break;
		case ftComboSelect:
		case ftSubFilter:
		case ftSubFilterEditable:
			// Value lists and select lists are entered via combo boxes
			{
				long nCurSel = m_dlValueList->CurSel;
				if (nCurSel >= 0) {
					if (VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0)) == "-2") {
						// Multiple items are selected
						if (m_strMultiItemValues == "")
							// This shouldn't be possible
							ASSERT(FALSE);
						m_strValue = m_strMultiItemValues;
					}
					else
						m_strValue = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
				} 
				else {
					m_strValue = "";
				}
			}
			break;
		default:
			// Unhandled
			return false;
			break;
		}
	}
	return true;
}

void CFilterDetailDlg::OnClickOrDoubleClickUseOrBtn() 
{
	// Only flip AND/OR if we're not at the top and we're not an empty detail
	if ((m_nFieldIndex != -1) && (m_nDetailData != 0)) {
		m_bUseOr = !m_bUseOr;
		RefreshUseOr();
		GetParent()->PostMessage(NXM_FILTER_USE_OR_CHANGED, m_nDetailData);
	}
}

void CFilterDetailDlg::OnDrawUseOrBtn(CDC *pdc, UINT nAction, UINT nState, HWND hWnd)
{
	switch (nAction) {
	case ODA_DRAWENTIRE:	// Redraw the whole thing
		{
			// First get our drawing area
			CRect rc;
			::GetClientRect(hWnd, rc);
			
			// Set the basic background color to light grey
			pdc->SetBkColor(GetSysColor(COLOR_BTNFACE));
			pdc->FillSolidRect(rc, GetSysColor(COLOR_BTNFACE));

			// Deflate the rectangle by one point on all sides
			//rc.DeflateRect(1, 1, 1, 1);

			// Then calculate the point at the vertical center and off horizontal center a quarter to the right
			POINT ptPivot;
			ptPivot.x = rc.right * 3 / 4;
			ptPivot.y = rc.bottom / 2;

			// If we're not on an empty detail, draw the lines and the text
			if (m_nFieldIndex != -1) {
				// Draw the connecting lines
				if (!m_bUseOr) {
					// For AND we only draw the vertical bar upward if we're not at the top
					if (m_nDetailData != 0) {
						pdc->MoveTo(ptPivot.x, 0);
						pdc->LineTo(ptPivot);
					}
				}

				// Draw the vertical bar downward iff the next detail is going to be an AND
				if (!m_bUseOrAfter) {
					pdc->MoveTo(ptPivot);
					pdc->LineTo(ptPivot.x, rc.bottom);
				}

				// Always draw the horizontal line
				pdc->MoveTo(ptPivot);
				pdc->LineTo(rc.right, ptPivot.y);
				
				// If we're not on the first detail we draw the lines
				if (m_nDetailData != 0) {
					// Calc the rect that we will draw our box in
					rc.DeflateRect(3, 3, rc.right-ptPivot.x+3, 3);
					
					// Use the app's palette
					extern CPracticeApp theApp;
					pdc->SelectPalette(&theApp.m_palette, FALSE);
					pdc->RealizePalette();

					// Decide the background color (using a palette color if we're in 256 color mode)
					COLORREF clrBack = PaletteColor(GetNxColor(GNC_FILTER_USE_OR, m_bUseOr));

					// Draw the background
					{
						CBrush br(clrBack);
						CPen pn(PS_SOLID, 1, RGB(0,0,0));
						CBrush *pOldBrush = pdc->SelectObject(&br);
						CPen *pOldPen = pdc->SelectObject(&pn);
						pdc->Rectangle(rc);
						pdc->SelectObject(&pOldPen);
						pdc->SelectObject(&pOldBrush);
					}

					// Draw the text
					{
						pdc->SetBkColor(clrBack);
						pdc->DrawText(m_bUseOr ? "or" : "and", rc, DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_NOCLIP);
					}
				}
			}
		}
		break;
	case ODA_SELECT:
		break;
	default:
		// do nothing
		break;
	}
}

void CFilterDetailDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	switch (nIDCtl) {
	case IDC_USE_OR_BTN:
		{
			CDC dc;
			dc.Attach(lpDrawItemStruct->hDC);
			OnDrawUseOrBtn(&dc, lpDrawItemStruct->itemAction, lpDrawItemStruct->itemState, lpDrawItemStruct->hwndItem);
			dc.Detach();
		}
		break;
	default:
		CNxDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
		break;
	}
}

void CFilterDetailDlg::OnFieldBtn() 
{
	CRect rectButton;
	GetDlgItem(IDC_FIELD_BTN)->GetWindowRect(rectButton);

	CMenu mnu;
	mnu.CreatePopupMenu();
	mnu.AppendMenu(MF_STRING, 1, "Test");
	mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rectButton.right, rectButton.bottom, this, NULL);
}

bool CFilterDetailDlg::AllowFilter(CString name) {

	for(int i = 0; i < m_nextechFilter.GetSize(); i++) {

		if(name == "Support Expires")
			Sleep(0);
		
		if(name == m_nextechFilter.GetAt(i)) {
			//the filter is one of the ones we're looking for

			//if internal, use it, otherwise don't
			if(IsNexTechInternal())
				return true;
			else
				return false;
		}
	}
	
	//wasn't found, so allow it
	return true;
}

void CFilterDetailDlg::OnEditSubfilter() 
{
	//First of all, are we on the "one time" subfilter? 
	CString strID = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
	bool bOneTime = false;
	if(strID.Left(strID.Find("-")) == "0") {
		bOneTime = true;
	}


	//OK, we need first off to find out what we support.
	CMenu mActions;
	long nIndex = 0;
	long nFilterType = g_FilterFields[m_nFieldIndex].m_nSubfilterType;
	if(m_pfnIsActionSupported(saAdd, nFilterType)) {
		if(!mActions.GetSafeHmenu()) mActions.m_hMenu = CreatePopupMenu();
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_ADD, "&Add");
	}
	if(bOneTime || m_pfnIsActionSupported(saEdit, nFilterType)) {//One-time subfilter can always be edited.
		if(!mActions.GetSafeHmenu()) mActions.m_hMenu = CreatePopupMenu();
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_EDIT, "&Edit");
	}
	if(bOneTime || m_pfnIsActionSupported(saDelete, nFilterType)) {//One-time subfilter always has grayed-out delete.
		if(!mActions.GetSafeHmenu()) mActions.m_hMenu = CreatePopupMenu();
		mActions.InsertMenu(nIndex++, MF_BYPOSITION, ID_DELETE, "&Delete");
		if(bOneTime) mActions.EnableMenuItem(ID_DELETE, MF_GRAYED);			
	}

	if(mActions.GetSafeHmenu()) {
		//OK, we have at least one option, so let's show the menu.
		CRect rBtn;
		GetDlgItem(IDC_EDIT_SUBFILTER)->GetWindowRect(rBtn);
		mActions.TrackPopupMenu(TPM_LEFTALIGN, rBtn.right, rBtn.top, this, NULL);
	}

}

void CFilterDetailDlg::OnAdd()
{
	CFilterEditDlg dlg(this, g_FilterFields[m_nFieldIndex].m_nSubfilterType, m_pfnIsActionSupported, m_pfnCommitSubfilterAction, m_pfnGetNewFilterString, g_FilterFields[m_nFieldIndex].m_strSubfilterDisplayName);
	
	if(dlg.EditFilter(FILTER_ID_NEW) == IDOK) {
		m_strValue.Format("%li", dlg.GetFilterId());
		RefreshValue();
	}
}

void CFilterDetailDlg::OnEdit()
{
	CFilterEditDlg dlg(this, g_FilterFields[m_nFieldIndex].m_nSubfilterType, m_pfnIsActionSupported, m_pfnCommitSubfilterAction, m_pfnGetNewFilterString, g_FilterFields[m_nFieldIndex].m_strSubfilterDisplayName);

	CString strValue = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
	long nFilterID;
	CString strFilter = "";
	if(strValue.Left(1) == "0") {
		nFilterID = 0;
		if(strValue.Find("-") < strValue.GetLength()-1) {//If there's anything after the hyphen.
			strFilter = strValue.Mid(strValue.Find("-")+1);
		}
	}
	else {
		//We need to look this up, it will be in our select clause.
		CString strQueryNoOrderBy = g_FilterFields[m_nFieldIndex].m_pstrParameters;
		long nOrderByStart, nOrderByEnd;
		FindClause(strQueryNoOrderBy, fcOrderBy, nOrderByStart, nOrderByEnd);
		if(nOrderByEnd >= strQueryNoOrderBy.GetLength()-1) {
			strQueryNoOrderBy = strQueryNoOrderBy.Left(nOrderByStart);
		}
		else {
			strQueryNoOrderBy = strQueryNoOrderBy.Left(nOrderByStart) + strQueryNoOrderBy.Mid(nOrderByEnd);
		}
		_RecordsetPtr rsFilter = CreateRecordset("SELECT Filter FROM (%s) SubQ WHERE SubQ.ID = %s", strQueryNoOrderBy, strValue);
		strFilter = AdoFldString(rsFilter, "Filter");
		nFilterID = atol(strValue);
	}

	if(nFilterID == 0) {
		if(strFilter == "") {
			if(dlg.EditFilter(FILTER_ID_TEMPORARY) == IDCANCEL) return;
		}
		else {
			if(dlg.EditFilter(FILTER_ID_TEMPORARY, strFilter) == IDCANCEL) return;
		}
		m_strValue.Format("0-%s", dlg.m_strFilterString);
		m_dlValueList->PutValue(m_dlValueList->CurSel, 0, _bstr_t(m_strValue));
	}
	else {
		if(dlg.EditFilter(nFilterID, strFilter) == IDCANCEL) return;
		m_strValue.Format("%li", dlg.GetFilterId());
	}

	//Also, the name might have changed, so let's refresh the list.
	RefreshValue();
}

void CFilterDetailDlg::OnDelete()
{
	CString strFilterName = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 1));
	
	CString strID = VarString(m_dlValueList->GetValue(m_dlValueList->CurSel, 0));
	long nFilterID = atol(strID);
	CString strFilter = ""; //The filter doesn't matter.
	if(m_pfnCommitSubfilterAction(saDelete, g_FilterFields[m_nFieldIndex].m_nSubfilterType, nFilterID, strFilterName, strFilter, this)) {
		RefreshValue();
	}
}


void CFilterDetailDlg::OnKillfocusValueEdit() 
{
	if (m_nFieldIndex >= 0 &&
		g_FilterFields[m_nFieldIndex].m_ftFieldType & ftPhoneNumber) {

		//if a phone number field, strip out characters that aren't
		//digits, parentheses, or dashes
		CString strIn, strOut;
		GetDlgItemText(IDC_VALUE_EDIT,strIn);
		strOut = StripNonPhoneNumber(strIn);
		SetDlgItemText(IDC_VALUE_EDIT,strOut);
	}
}

CString CFilterDetailDlg::StripNonPhoneNumber(CString strIn) {

	//if a phone number field, strip out characters that aren't
	//digits, parentheses, or dashes
	CString strOut;
	for (int i = 0; i < strIn.GetLength(); i++) {
		if ((strIn.GetAt(i) > 47 && strIn.GetAt(i) < 58) ||
			strIn.GetAt(i) == '(' || strIn.GetAt(i) == ')' || strIn.GetAt(i) == '-') 
			strOut += (CString)strIn.GetAt(i);
	}
	return strOut;
}

LRESULT CFilterDetailDlg::OnLabelClick(WPARAM wParam, LPARAM lParam) 
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_MULTI_VALUE_LIST:
			DoMultipleSelectDlg();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}
