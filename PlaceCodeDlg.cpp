// PlaceCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlaceCodeDlg.h"
#include "GlobalDataUtils.h"
#include "Client.h"
#include "AlbertaHLINKUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CPlaceCodeDlg dialog

CPlaceCodeDlg::CPlaceCodeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPlaceCodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPlaceCodeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPlaceCodeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPlaceCodeDlg)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPlaceCodeDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPlaceCodeDlg)
	ON_EVENT(CPlaceCodeDlg, IDC_PLACE_CODES, 9 /* EditingFinishing */, OnEditingFinishingPlaceCodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPlaceCodeDlg, IDC_PLACE_CODES, 10 /* EditingFinished */, OnEditingFinishedPlaceCodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CPlaceCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlaceCodeDlg)
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CPlaceCodeDlg message handlers

void CPlaceCodeDlg::OnDelete() 
{

	if(m_pCodes->GetCurSel() == -1)
		return;

	try
	{
		long nPOS = VarLong(m_pCodes->GetValue(m_pCodes->CurSel, 0), -1);

		
		if (ReturnsRecords("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ServiceLocationID = %li AND LineItemT.Deleted = 0", nPOS)) {

			// (j.jones 2012-07-24 15:39) - PLID 51736 - Reworded the prompt for Alberta.
			CString strPrompt = "There are bills with the Place of Service Designation Code that you are trying to delete. "
				"If you continue, these bills will have a blank Place of Service Designation Code.\n\n"
				"Are you sure you wish to delete this code?";
			if(UseAlbertaHLINK()) {
				strPrompt.Replace("Place of Service", "Functional Centre");
			}

			if (IDNO == MsgBox(MB_YESNO, strPrompt)) {
				return;
			}

		}

		// (c.haag 2010-10-15 11:09) - PLID 40352 - Check for Eligibility requests that use it
		if (ReturnsRecords("SELECT ID FROM EligibilityRequestsT WHERE PlaceOfServiceID = %li", nPOS)) 
		{
			// (j.jones 2012-07-24 15:39) - PLID 51736 - Reworded the prompt for Alberta (though they shouldn't be using eligibility)
			CString strPrompt = "There are eligibility requests with the Place of Service Designation Code that you are trying to delete. "
				"If you continue, these requests will have a blank Place of Service Designation Code.\n\n"
				"Are you sure you wish to delete this code?";
			if(UseAlbertaHLINK()) {
				strPrompt.Replace("Place of Service", "Functional Centre");
			}

			if (IDNO == MsgBox(MB_YESNO, strPrompt)) {
				return;
			}
		}

		// (c.haag 2010-10-15 11:09) - PLID 40352 - Remove from Eligibility requests
		ExecuteSql("UPDATE EligibilityRequestsT SET PlaceOfServiceID = NULL WHERE PlaceOfServiceID = %li", nPOS);
		ExecuteSql("UPDATE ChargesT SET ServiceLocationID = NULL WHERE ServiceLocationID = %li ", nPOS);
		ExecuteSql("UPDATE LocationsT SET POSID = NULL WHERE POSID = %li", nPOS);
		ExecuteSql("DELETE FROM POSLocationLinkT WHERE POSID = %li", nPOS);
		ExecuteSql("DELETE FROM PlaceOfServiceCodesT WHERE ID = %li ", nPOS);
		CClient::RefreshTable(NetUtils::PlaceOfServiceDesignation);
		m_pCodes->RemoveRow(m_pCodes->CurSel);

		
	}NxCatchAll("Error 100: CPlaceCodeDlg::OnDelete");
	

}

void CPlaceCodeDlg::OnAdd() 
{
	try {

		CString code, name;
		_RecordsetPtr rs;

		// (j.jones 2012-07-24 15:39) - PLID 51736 - The limit is now 4 instead of 3.
		// Also added a different prompt for Alberta.
		CString strPrompt = "Enter new Place Of Service Code";
		if(UseAlbertaHLINK()) {
			strPrompt = "Enter new Functional Centre Code";
		}
		int nResult = InputBoxLimited(this, strPrompt, code, "",4,false,false,NULL);
		if (nResult == IDOK && code != "")
		{	
			if(code.GetLength() > 4) {
				CString strPrompt = "The Place Of Service Code cannot be more than 4 characters long.";
				if(UseAlbertaHLINK()) {
					strPrompt = "The Functional Centre Code cannot be more than 4 characters long.";
				}
				MsgBox(strPrompt);
				return;
			}

			rs = CreateRecordset("SELECT Count(PlaceCodes) AS CodeCount FROM PlaceOfServiceCodesT WHERE PlaceCodes = '%s'", _Q(code));
			if(AdoFldLong(rs, "CodeCount", 0) != 0){
				MsgBox(RCS(IDS_PLACE_CODE_DUP));
				return;
			}

			nResult = InputBoxLimited(this, "Enter a description for this new code", name, "",255,false,false,NULL);
			if (nResult == IDOK && name != "")
			{	
				try
				{
					long nPOS = NewNumber("PlaceofServiceCodesT", "ID");
					ExecuteSql("INSERT INTO PlaceOfServiceCodesT "
					"(ID, PlaceCodes, PlaceName) VALUES (%li, '%s', '%s')", nPOS, _Q(code), _Q(name));
					CClient::RefreshTable(NetUtils::PlaceOfServiceDesignation);
				}NxCatchAll("Error 100: CPlaceCodeDlg::OnAdd");
				m_pCodes->Requery();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CPlaceCodeDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();
		m_pCodes = BindNxDataListCtrl(this, IDC_PLACE_CODES, GetRemoteData(), true);

		// (z.manning, 05/01/2008) - PLID 29864 - Set button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		// (j.jones 2012-03-21 12:33) - PLID 48155 - added caching
		g_propManager.CachePropertiesInBulk("CPlaceCodeDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'UseAlbertaHLINK' "
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2012-03-21 12:33) - PLID 48155 - if the Alberta preference is on,
		// rename the window to say Functional Centre Setup
		if(UseAlbertaHLINK()) {
			SetWindowText("Functional Centre Setup");
		}
		else {
			SetWindowText("Place of Service Designations");
		}

		GetDlgItem(IDC_ADD)->SetFocus();

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CPlaceCodeDlg::OnEditingFinishingPlaceCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	CString strEntered;
	switch(nCol){
	case 1:
		strEntered = strUserEntered;

		// (j.jones 2012-07-24 15:39) - PLID 51736 - The limit is now 4 instead of 3.
		// Also added a different prompt for Alberta.
		if(strEntered.GetLength() > 4) {
			CString strPrompt = "The Place Of Service Code cannot be more than 4 characters long.";
			if(UseAlbertaHLINK()) {
				strPrompt = "The Functional Centre Code cannot be more than 4 characters long.";
			}
			MsgBox(strPrompt);
			*pbCommit = FALSE;
			return;
		}

		SetVariantString(*pvarNewValue, (LPCTSTR)strEntered);
		_RecordsetPtr rs = CreateRecordset("SELECT Count(PlaceCodes) AS CodeCount FROM PlaceOfServiceCodesT WHERE PlaceCodes = '%s'", _Q(VarString(pvarNewValue)));
		if(AdoFldLong(rs, "CodeCount", 0) != 0 && VarString(varOldValue) != VarString(*pvarNewValue)){ //This place code exists, and it isn't this record.
			MsgBox(RCS(IDS_PLACE_CODE_DUP));
			*pbCommit = FALSE;
		}
		break;
	}
}

void CPlaceCodeDlg::OnEditingFinishedPlaceCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try{

		if(_variant_t(varOldValue) == _variant_t(varNewValue))
			return;

		switch(nCol){
		case 1:
			if(bCommit)	{
				long nPOS = VarLong(m_pCodes->GetValue(nRow, 0));
				if (ReturnsRecords("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ServiceLocationID = %li AND LineItemT.Deleted = 0", nPOS)) {

					// (j.jones 2012-07-24 15:39) - PLID 51736 - Reworded the prompt for Alberta.
					CString strPrompt = "There are bills with the Place of Service Designation Code that you are trying to change.\n"
						"If you continue, these bills will use the new Place of Service Designation Code.\n\n"
						"Are you sure you wish to change this code?";
					if(UseAlbertaHLINK()) {
						strPrompt.Replace("Place of Service", "Functional Centre");
					}

					if (IDNO == MsgBox(MB_YESNO, strPrompt)) {

						//they said no, change it back
						m_pCodes->PutValue(nRow, nCol, varOldValue);
						return;
					}
				}

				ExecuteSql("UPDATE PlaceOfServiceCodesT SET PlaceCodes = '%s' WHERE ID = %li;", _Q(VarString(varNewValue)), nPOS);
			}
			CClient::RefreshTable(NetUtils::PlaceOfServiceDesignation);
			break;
		case 2:
			long nPOS = VarLong(m_pCodes->GetValue(nRow, 0));
			ExecuteSql("UPDATE PlaceOfServiceCodesT SET PlaceName = '%s' WHERE ID = %li;", _Q(VarString(varNewValue)), nPOS);
			CClient::RefreshTable(NetUtils::PlaceOfServiceDesignation);
			break;
		}
	}NxCatchAll("Error 100: CPlaceCodeDlg::OnEditingFinshedPlaceCodes");
}
