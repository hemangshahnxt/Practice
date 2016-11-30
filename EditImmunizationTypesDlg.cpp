// EditImmunizationTypesDlg.cpp : implementation file
// (d.thompson 2009-05-18) - PLID 34232 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "EditImmunizationTypesDlg.h"
#include "WellnessDataUtils.h"

// (a.walling 2010-02-18 08:37) - PLID 37434 - Added CVX, Description
enum eListColumns {
	elcID = 0,
	elcCVX,
	elcType,
	elcDescription,
	elcDosage,
	elcDosageUnits, // (a.walling 2010-09-13 14:44) - PLID 40505 - Support dosage units for vaccinations
};

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CEditImmunizationTypesDlg dialog

IMPLEMENT_DYNAMIC(CEditImmunizationTypesDlg, CNxDialog)

CEditImmunizationTypesDlg::CEditImmunizationTypesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditImmunizationTypesDlg::IDD, pParent)
{

}

CEditImmunizationTypesDlg::~CEditImmunizationTypesDlg()
{
}

void CEditImmunizationTypesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD_IMM_TYPE, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_IMM_TYPE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_IMMTYPE_UCUM_LABEL, m_nxlabelUnits); // (a.walling 2010-09-13 14:44) - PLID 40505 - Support dosage units for vaccinations
}


BEGIN_MESSAGE_MAP(CEditImmunizationTypesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_IMM_TYPE, OnAddType)
	ON_BN_CLICKED(IDC_DELETE_IMM_TYPE, OnDeleteType)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, &CEditImmunizationTypesDlg::OnLabelClick)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEditImmunizationTypesDlg, CNxDialog)
	ON_EVENT(CEditImmunizationTypesDlg, IDC_IMM_TYPE_LIST, 10 /* EditingFinished */, OnEditingFinishedList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditImmunizationTypesDlg, IDC_IMM_TYPE_LIST, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// CEditImmunizationTypesDlg message handlers
BOOL CEditImmunizationTypesDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//setup interface controls
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_pList = BindNxDataList2Ctrl(IDC_IMM_TYPE_LIST, true);

		// (a.walling 2010-09-13 14:44) - PLID 40505 - Support dosage units for vaccinations
		CString strOriginalUnitLabelText;
		m_nxlabelUnits.GetWindowText(strOriginalUnitLabelText);
		m_nxlabelUnits.SetText(strOriginalUnitLabelText);
		m_nxlabelUnits.SetType(dtsHyperlink);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CEditImmunizationTypesDlg::OnAddType()
{
	try {
		CString strName;
		if(InputBox(this, "Enter a type name", strName, "") == IDOK) {
			//max 255 chars
			if(strName.GetLength() > 255) {
				AfxMessageBox("You cannot enter a name greater than 255 characters.");
				return;
			}

			//confirm it doesn't already exist
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 * FROM ImmunizationsT WHERE Type = {STRING}", strName);
			if(!prs->eof) {
				AfxMessageBox("This name already exists, please try another.");
				return;
			}
			prs->Close();

			//All set, create it
			ExecuteParamSql("INSERT INTO ImmunizationsT (Type, DefaultDosage) values ({STRING}, '');", strName);

			//refresh
			m_pList->Requery();
		}

	} NxCatchAll("Error in OnAddType");
}

void CEditImmunizationTypesDlg::OnDeleteType()
{
	try {
		IRowSettingsPtr pRow = m_pList->CurSel;
		if(pRow == NULL) {
			return;
		}

		long nID = VarLong(pRow->GetValue(elcID));

		// (a.walling 2010-02-18 08:37) - PLID 37434
		CString strCVX = VarString(pRow->GetValue(elcCVX), "");

		if (!strCVX.IsEmpty()) {
			if(MessageBox("This immunization is linked to a standard CVX code. Are you sure you wish to delete this immunization?", NULL, MB_YESNO) != IDYES) {
				return;
			}
		}

		//make sure it's not in use
		_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 * FROM PatientImmunizationsT WHERE ImmunizationID = {INT};", nID);
		if(!prs->eof) {
			MessageBox("This immunization is in use and cannot be deleted.");
			return;
		}

		//TES 6/8/2009 - PLID 34525 - Make sure this isn't used by any Wellness tables.
		prs = CreateParamRecordset(FormatString("SELECT TOP 1 ID FROM WellnessTemplateCriterionT WHERE Type = %i AND RecordID = {INT} "
			"UNION SELECT TOP 1 ID FROM WellnessTemplateCompletionItemT WHERE RecordType = %i AND RecordID = {INT} "
			"UNION SELECT TOP 1 ID FROM PatientWellnessCriterionT WHERE Type = %i AND RecordID = {INT} "
			"UNION SELECT TOP 1 ID FROM PatientWellnessCompletionItemT WHERE RecordType = %i AND RecordID = {INT}",
			wtctImmunization, cirtImmunization, wtctImmunization, cirtImmunization),
			nID, nID, nID, nID);
		if(!prs->eof) {
			MessageBox("This immunization is used by Patient Wellness templates or alerts, and cannot be deleted.");
			return;
		}

		//confirm
		if(MessageBox("Are you sure you wish to delete this immunization?", NULL, MB_YESNO) != IDYES) {
			return;
		}

		ExecuteParamSql("DELETE FROM ImmunizationsT WHERE ID = {INT};", nID);

		//refresh it
		m_pList->Requery();

	} NxCatchAll("Error in OnDeleteType");
}

void CEditImmunizationTypesDlg::OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(elcID));

		switch(nCol) {
			// (a.walling 2010-02-18 08:38) - PLID 37434
			case elcCVX:
				{
					// (a.walling 2013-11-15 12:44) - PLID 59511 - CVX should be a string column to allow for leading zeros
					CString strNewCVX = VarString(varNewValue, "");
					_variant_t varNewCVX = g_cvarNull;
					if (!strNewCVX.IsEmpty()) {
						varNewCVX = (const char*)strNewCVX;
					}
					ExecuteParamSql("UPDATE ImmunizationsT SET CVX = {VT_BSTR} WHERE ID = {INT};", varNewCVX, nID);
				}
				break;
			case elcType:
				{
					//we already checked size, just save
					ExecuteParamSql("UPDATE ImmunizationsT SET Type = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;
			// (a.walling 2010-02-18 08:38) - PLID 37434
			case elcDescription:
				{
					//we already checked size, just save
					ExecuteParamSql("UPDATE ImmunizationsT SET Description = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;
			case elcDosage:
				{
					//we already checked size, just save
					ExecuteParamSql("UPDATE ImmunizationsT SET DefaultDosage = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;
			case elcDosageUnits: // (a.walling 2010-09-13 14:44) - PLID 40505 - Support dosage units for vaccinations
				{
					//we already checked size, just save
					ExecuteParamSql("UPDATE ImmunizationsT SET DefaultDosageUnits = {STRING} WHERE ID = {INT};", VarString(varNewValue), nID);
				}
				break;
		}

	} NxCatchAll("Error in OnEditingFinishedList");
}

void CEditImmunizationTypesDlg::OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	try {
		if(lpRow == NULL) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(elcID));

		switch(nCol) {
			// (a.walling 2010-02-18 08:38) - PLID 37434
			case elcCVX:
				{
					// (a.walling 2013-11-15 12:44) - PLID 59511 - CVX should be a string column to allow for leading zeros

					CString userEntered(strUserEntered);
					// (a.walling 2010-02-18 08:37) - PLID 37434
					CString strNewCVX = pvarNewValue ? VarString(*pvarNewValue, "") : "";
					if (userEntered.IsEmpty()) {
						strNewCVX.Empty();
						if (pvarNewValue) {
							*pvarNewValue = g_cvarNull;
						}
					}

					if (!strNewCVX.IsEmpty()) {		
						// see if anywhere else has this value				
						_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 * FROM ImmunizationsT WHERE CVX = {STR} AND ID <> {INT}", strNewCVX, nID);
						if(!prs->eof) {
							MessageBox("This CVX code is already linked to a standard immunization.");
							//*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
					}

					CString strOldCVX = VarString(varOldValue, "");

					// (a.walling 2010-02-26 11:10) - PLID 37434 - Don't prompt if no change
					if (!strOldCVX.IsEmpty() && strOldCVX != strNewCVX) {
						if(MessageBox("This immunization is already linked to a standard CVX code. Are you sure you wish to change this immunization's CVX code?", NULL, MB_YESNO) != IDYES) {
							*pbCommit = FALSE;
							return;
						}
					}
				}
				break;
			case elcType:
				{
					//constrain to 255 characters
					CString strValue = VarString(*pvarNewValue);
					if(strValue.GetLength() > 255) {
						MessageBox("You may not enter names greater than 255 characters.");
						//*pbContinue = FALSE;
						*pbCommit = FALSE;
						return;
					}

					//also ensure this is not a duplicate name
					_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 * FROM ImmunizationsT WHERE Type = {STRING} AND ID <> {INT}", strValue, nID);
					if(!prs->eof) {
						MessageBox("This name already exists, please try another.");
						//*pbContinue = FALSE;
						*pbCommit = FALSE;
					}
					prs->Close();
				}
				break;
			// (a.walling 2010-02-18 08:38) - PLID 37434
			case elcDescription:
				{
					//constrain to 512 characters
					CString strValue = VarString(*pvarNewValue);
					if(strValue.GetLength() > 512) {
						MessageBox("You may not enter descriptions greater than 512 characters.");
						//*pbContinue = FALSE;
						*pbCommit = FALSE;
						return;
					}

					// duplicate descriptions are acceptable.
				}
				break;
			case elcDosage:
				{
					//constrain to 50 characters
					CString strValue = VarString(*pvarNewValue);
					if(strValue.GetLength() > 50) {
						MessageBox("You may not enter dosages greater than 50 characters.");
						//*pbContinue = FALSE;
						*pbCommit = FALSE;
					}
				}
				break;
			case elcDosageUnits: // (a.walling 2010-09-13 14:44) - PLID 40505 - Support dosage units for vaccinations
				{
					//constrain to 20 characters
					CString strValue = VarString(*pvarNewValue);
					if(strValue.GetLength() > 20) {
						MessageBox("You may not enter dosage units greater than 20 characters.");
						//*pbContinue = FALSE;
						*pbCommit = FALSE;
					}
				}
				break;
		}

	} NxCatchAll("Error in OnEditingFinishingList");
}

// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
BOOL CEditImmunizationTypesDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		
		CRect rc;
		m_nxlabelUnits.GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (a.walling 2010-09-13 14:31) - PLID 40505 - Support dosage units for vaccinations
LRESULT CEditImmunizationTypesDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		// open it up!
		//ShellExecute(NULL, NULL, "http://www.unitsofmeasure.org/", NULL, NULL, SW_SHOW);
		// (a.walling 2010-09-14 10:22) - PLID 40505 - Need to use ANSI+ instead of UCUM for HL7 v2.5.1 compatibility
		CEditImmunizationTypesDlg::PopupANSIPlusUnitsDescription(this);
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (a.walling 2010-09-14 10:12) - PLID 40505 - Popup some examples of ANSI+ units
void CEditImmunizationTypesDlg::PopupANSIPlusUnitsDescription(CWnd* pParent)
{
	ASSERT(pParent->GetSafeHwnd());

	LPCTSTR szExamples = 
		"Common Examples of ANSI+ Units:\r\n"
		"\r\n"
		"m\tmeter\r\n"
		"cm\tcentimeter\r\n"
		"cm3\tcubic centimeter\r\n"
		"\r\n"
		"kg\tkilogram\r\n"
		"mg\tmilligram\r\n"
		"\r\n"
		"L\tliter\r\n"
		"mL\tmilliliter\r\n"
		"\r\n"
		"ft\tfoot\r\n"
		"in\tinch\r\n"
		"sqf\tsquare foot\r\n"
		"sin\tsquare inch\r\n"
		"cft\tcubic foot\r\n"
		"cin\tcubic inch\r\n"
		"\r\n"
		"lb\tpound\r\n"
		"oz\tounce (weight)\r\n"
		"\r\n"
		"gal\tgallon\r\n"
		"qt\tquart\r\n"
		"pt\tpint\r\n"
		"foz\tounce (fluid)\r\n"
		"tbs\ttablespoon\r\n"
		"tsp\tteaspoon";

	pParent->MessageBox(szExamples, "ANSI+ Units", MB_ICONINFORMATION);
}