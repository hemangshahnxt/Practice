// EmrProblemActionsDlg.cpp : implementation file
//
// (c.haag 2008-07-17 10:52) - PLID 30723 - Initial implementation
// (c.haag 2014-07-22) - PLID 62789 - Refactored new problem entry. Problems are now entered via modal dialog
//

#include "stdafx.h"
#include "administratorrc.h"
#include "EmrProblemActionsDlg.h"
#include "EMRProblemStatusDlg.h"
#include "EmrProblemNewActionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef enum {
	epalcID = 0,
	epalcStatus = 1,
	epalcDescription = 2,
	epalcAssocWith = 3,
	epalcSNOMEDCodeID = 4, // (c.haag 2014-07-22) - PLID 62789
	epalcDoNotShowOnCCDA=5,// (s.tullis 2015-02-24 11:31) - PLID 64724 
	epalcPrompt = 6, // (r.gonet 2015-03-10 14:48) - PLID 65013 - Column index enumeration value for the Prompt column.
} EProblemActionListCol;

typedef enum {
	epslcID = 0,
	epslcName = 1,
} EProblemStatusListCol;

/////////////////////////////////////////////////////////////////////////////
// CEmrProblemActionsDlg dialog


CEmrProblemActionsDlg::CEmrProblemActionsDlg(CProblemActionAry* paryProblemActions, EmrActionObject SourceType,
		EmrActionObject DestType, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrProblemActionsDlg::IDD, pParent),
	m_paryProblemActions(paryProblemActions),
	m_SourceType(SourceType),
	m_DestType(DestType)
{
	//{{AFX_DATA_INIT(CEmrProblemActionsDlg)
	//}}AFX_DATA_INIT
}


void CEmrProblemActionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrProblemActionsDlg)
	DDX_Control(pDX, IDC_BTN_DELETE_PROBLEM_ACTION, m_btnDeleteProblem);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_ADD_PROBLEM_ACTION, m_btnAddProblem);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrProblemActionsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrProblemActionsDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_PROBLEM_ACTION, OnBtnAddProblemAction)
	ON_BN_CLICKED(IDC_BTN_DELETE_PROBLEM_ACTION, OnBtnDeleteProblemAction)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrProblemActionsDlg message handlers

BOOL CEmrProblemActionsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// Bind datalists
		m_dlProblems = BindNxDataList2Ctrl(IDC_LIST_ACTION_PROBLEMS, false);

		// Iconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddProblem.AutoSet(NXB_NEW);
		m_btnDeleteProblem.AutoSet(NXB_DELETE);

		if (eaoEmrDataItem == m_SourceType || eaoEmrItem == m_SourceType)
		{
			m_AssocMode = eSupportEitherAssoc;
		}
		else {
			// This must be a hotspot action. We only support associating the problem with the destination item
			m_AssocMode = eSupportDestAssocOnly;
		}

		// Set the embedded combo sources for the problem list
		CString strAssocComboSrc;
		switch (m_AssocMode) {
		case eSupportSourceAssocOnly:
			strAssocComboSrc.Format("%d;%s;", (long)m_SourceType, GetEmrActionObjectName(m_SourceType, TRUE));
			break;
		case eSupportDestAssocOnly:
			strAssocComboSrc.Format("%d;%s;", (long)m_DestType, GetEmrActionObjectName(m_DestType, TRUE));
			break;
		default:
			strAssocComboSrc.Format("%d;%s;%d;%s;", (long)m_SourceType, GetEmrActionObjectName(m_SourceType, TRUE),
				(long)m_DestType, GetEmrActionObjectName(m_DestType, TRUE));
			break;
		}

		// (c.haag 2008-07-24 17:18) - Set the combo sources. We need to include inactive but used statii.

		// Fill the problem list
		CArray<long,long> anUsedStatuses;
		// (a.walling 2014-07-01 15:28) - PLID 62697
		for (const EmrProblemAction& epa : *m_paryProblemActions) {
			anUsedStatuses.Add(epa.nStatus); // Doesn't matter if we have duplicates
		}
		CString strWhere = FormatString("ID IN (%s) OR (Inactive = 0 AND ID <> 2)", ArrayAsString(anUsedStatuses, false));
		m_dlProblems->GetColumn(epalcAssocWith)->ComboSource = _bstr_t(strAssocComboSrc);
		m_dlProblems->GetColumn(epalcStatus)->ComboSource = _bstr_t("SELECT ID, Name FROM EMRProblemStatusT WHERE " + strWhere);

		// (c.haag 2014-07-22) - PLID 62789 - We now have a SNOMED code column
		m_dlProblems->GetColumn(epalcSNOMEDCodeID)->ComboSource = _bstr_t("SELECT NULL AS ID, '<None>' AS Code UNION SELECT ID, CodesT.Code + '  ' + CodesT.Name FROM CodesT LEFT JOIN VocabFamilyT ON CodesT.Vocab = VocabFamilyT.Name WHERE VocabFamilyT.Family = 'SNOMEDCT' ORDER BY Code ASC");

		// Set default button enabled states
		GetDlgItem(IDC_BTN_DELETE_PROBLEM_ACTION)->EnableWindow(FALSE);

		// Fill the problem list
		// (a.walling 2014-07-01 15:28) - PLID 62697
		for (const EmrProblemAction& epa : *m_paryProblemActions) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlProblems->GetNewRow();
			pRow->Value[epalcID] = epa.nID;
			pRow->Value[epalcStatus] = epa.nStatus;
			pRow->Value[epalcDescription] = _bstr_t(epa.strDescription);
			pRow->Value[epalcAssocWith] = _variant_t((long)(epa.bSpawnToSourceItem ? m_SourceType : m_DestType));
			pRow->Value[epalcSNOMEDCodeID] = (-1 == epa.nSNOMEDCodeID) ? g_cvarNull : epa.nSNOMEDCodeID;
			pRow->Value[epalcDoNotShowOnCCDA] =_variant_t((long)epa.bDoNotShowOnCCDA);// (s.tullis 2015-02-24 11:31) - PLID 64724 
			// (r.gonet 2015-03-10 14:48) - PLID 65013 - Assign the opposite of the DoNotShowOnProblemPrompt flag, since this column says the opposite.
			pRow->Value[epalcPrompt] =  epa.bDoNotShowOnProblemPrompt ? _variant_t(1L, VT_I4) : _variant_t(0L, VT_I4); 
			m_dlProblems->AddRowSorted(pRow, NULL);
		}

		// (c.haag 2014-07-22) - PLID 62789 - Set focus to the Add Problem button since that's probably why the user came in here
		GetDlgItem(IDC_BTN_ADD_PROBLEM_ACTION)->SetFocus();
	}
	NxCatchAll("Error in CEmrProblemActionsDlg::OnInitDialog");
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEmrProblemActionsDlg::OnBtnAddProblemAction() 
{
	try {
		// (c.haag 2014-07-22) - PLID 62789 - Use the new problem action dialog for adding problem actions
		CEmrProblemNewActionDlg dlg(m_SourceType, m_DestType, this);
		if (IDOK == dlg.DoModal())
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlProblems->GetNewRow();
			pRow->Value[epalcID] = -1L;
			pRow->Value[epalcStatus] = dlg.m_selStatus;
			pRow->Value[epalcDescription] = dlg.m_selDescription;
			switch (m_AssocMode) {
			case eSupportSourceAssocOnly:
				pRow->Value[epalcAssocWith] = (long)m_SourceType;
				break;
			case eSupportDestAssocOnly:
				pRow->Value[epalcAssocWith] = (long)m_DestType;
				break;
			default:
				pRow->Value[epalcAssocWith] = dlg.m_selAssocWith;
				break;
			}
			pRow->Value[epalcSNOMEDCodeID] = dlg.m_selSNOMEDCode;
			pRow->Value[epalcDoNotShowOnCCDA] = dlg.m_selDoNotShowOnCCDA; // (s.tullis 2015-02-24 11:31) - PLID 64724 
			// (r.gonet 2015-03-10 14:48) - PLID 65013 - Assign back the opposite of the DoNotShowOnProblemPrompt flag.
			pRow->Value[epalcPrompt] = VarBool(dlg.m_selDoNotShowOnProblemPrompt, FALSE) ? _variant_t(1L, VT_I4) : _variant_t(0L, VT_I4); 
			m_dlProblems->AddRowSorted(pRow, NULL);
		}
	}
	NxCatchAll("Error in CEmrProblemActionsDlg::OnBtnAddProblemAction");
}

void CEmrProblemActionsDlg::OnBtnDeleteProblemAction() 
{
	try {
		// (c.haag 2008-07-17 12:17) - Delete the selected problem
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlProblems->CurSel;
		if (NULL != pRow) {
			if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to delete this problem?")) {
				return;
			}
			m_dlProblems->RemoveRow(pRow);
		}
		else {
			// Should never happen -- the button should be disabled at this point
		}
	}
	NxCatchAll("Error in CEmrProblemActionsDlg::OnBtnDeleteProblemAction");
}

void CEmrProblemActionsDlg::OnOK() 
{
	try {
		// (c.haag 2008-07-17 12:18) - Save the problem list to the member array. We do this by clearing out
		// the array and adding all the items from the list into it
		m_paryProblemActions->clear();

		// Populate the array
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlProblems->GetFirstRow();
		while (NULL != pRow) {
			EmrProblemAction epa;
			epa.nID = VarLong(pRow->Value[epalcID]);
			epa.bSpawnToSourceItem = (VarLong(pRow->Value[epalcAssocWith]) == (long)m_SourceType) ? TRUE : FALSE;
			epa.nStatus = VarLong(pRow->Value[epalcStatus]);
			epa.strDescription = VarString(pRow->Value[epalcDescription]);
			epa.nSNOMEDCodeID = VarLong(pRow->Value[epalcSNOMEDCodeID], -1); // (c.haag 2014-07-22) - PLID 62789
			epa.bDoNotShowOnCCDA = AsBool(pRow->GetValue(epalcDoNotShowOnCCDA));// (s.tullis 2015-02-24 11:31) - PLID 64724 
			// (r.gonet 2015-03-17 11:23) - PLID 65013 - Read back the selection in the embedded dropdown. Note that the column is opposite what the flag actually is.
			epa.bDoNotShowOnProblemPrompt = (VarLong(pRow->GetValue(epalcPrompt), 0) == 1 ? TRUE : FALSE);
			m_paryProblemActions->push_back(epa);
			pRow = pRow->GetNextRow();
		}
	
		CNxDialog::OnOK();
	}
	NxCatchAll("Error in CEmrProblemActionsDlg::OnOK");
}

BEGIN_EVENTSINK_MAP(CEmrProblemActionsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrProblemActionsDlg)
	ON_EVENT(CEmrProblemActionsDlg, IDC_LIST_ACTION_PROBLEMS, 2 /* SelChanged */, OnSelChangedListActionProblems, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
//	ON_EVENT(CEmrProblemActionsDlg, IDC_LIST_ACTION_PROBLEMS, 10, CEmrProblemActionsDlg::EditingFinishedListActionProblems, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CEmrProblemActionsDlg, IDC_LIST_ACTION_PROBLEMS, 9, CEmrProblemActionsDlg::EditingFinishingListActionProblems, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CEmrProblemActionsDlg::OnSelChangedListActionProblems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		// (c.haag 2008-07-17 13:10) - Enable the buttons if a row is selected; or disable if no row is selected
		NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewSel);
		BOOL bEnable = (NULL != pNewRow) ? TRUE : FALSE;
		GetDlgItem(IDC_BTN_DELETE_PROBLEM_ACTION)->EnableWindow(bEnable);
	}
	NxCatchAll("Error in CEmrProblemActionsDlg::OnSelChangedListActionProblems");
}

void CEmrProblemActionsDlg::EditingFinishingListActionProblems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if (epalcSNOMEDCodeID == nCol)
		{
			// (c.haag 2014-07-22) - PLID 62789 - A selection of NULL is treated by the datalist as a zero.
			// We need to check for this condition and ensure the value is set to NULL.
			if (NULL != pvarNewValue
				&& VT_I4 == pvarNewValue->vt
				&& 0 == VarLong(*pvarNewValue)
				)
			{
				*pvarNewValue = g_cvarNull;
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}
