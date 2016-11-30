// ZipChooser.cpp : implementation file
//

#include "stdafx.h"
#include "ZipChooser.h"
#include "RegUtils.h"
#include "ZipcodeUtils.h"
#include "CityZipUtils.h"

using namespace ADODB;
using namespace NXDATALISTLib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZipChooser dialog

CZipChooser::CZipChooser(CWnd* pParent)
	: CNxDialog(CZipChooser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CZipChooser)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CZipChooser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CZipChooser)
	DDX_Control(pDX, IDC_ZIP_TEXT, m_nxstaticZipText);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_MAKE_PRI, m_btnMakePrimary);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CZipChooser, CNxDialog)
	//{{AFX_MSG_MAP(CZipChooser)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZipChooser message handlers

void CZipChooser::OnOK() 
{
	//make sure something is selected
	long nCurSel = m_pZipCodes->CurSel;
	if(nCurSel == -1)
		return;

	if(IsDlgButtonChecked(IDC_MAKE_PRI)) {
		//write it into the data
		try {
			bool bStatic = true;

			CString strZip = VarString(m_pZipCodes->GetValue(nCurSel, 2),"");
			CString strCity = VarString(m_pZipCodes->GetValue(nCurSel, 3),"");			

			//remove anything that was previously primary
			ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 0 WHERE ZipCode = '%s'", strZip);
			//also remove any records that are all nulls (because they were probably only entered for the primary value)
			// (j.gruber 2009-10-06 16:20) - PLID 35607 - need to just check on state and areacode since we need to store city now also
			ExecuteSql("DELETE FROM ZipCodesT WHERE State IS NULL AND AreaCode IS NULL AND ZipCode = '%s'", strZip);

			long nID = -1;
			if(m_pZipCodes->GetValue(nCurSel, 1).vt == VT_I4) {
				nID = VarLong(m_pZipCodes->GetValue(nCurSel, 1));	//static id
				
				// (a.walling 2010-10-04 10:40) - PLID 40573 - Ensure ID is OK
				if (nID == 0xCCCCCCCC || nID == 0xCDCDCDCD) {
					ThrowNxException("Please ensure you are using the latest zip code database");
				}
			}

			if(nID == -1) {	//internal only code
				nID = VarLong(m_pZipCodes->GetValue(nCurSel, 0),-1);	//unique id
				bStatic = false;
				if(nID == -1) {
					//STILL not correct, this is bad
					AfxMessageBox("Error setting primary zip code.  Please refresh and try again.");
					return;
				}
			}

			if(bStatic) {	//we're working with a code which came from the static database
				if(ReturnsRecords("SELECT StaticID FROM ZipCodesT WHERE StaticID = %li", nID)) {
					//if the record exists, update the PrimaryZip column
					ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 1 WHERE StaticID = %li", nID);
				}
				else {
					//insert a new record
					// (j.gruber 2009-10-06 16:21) - PLID 35607 - save city also
					ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip) values (%li, %li, '%s', '%s', NULL, NULL, %li)", 
						NewNumber("ZipCodesT", "UniqueID"), nID, strZip, _Q(strCity), 1);
				}
			}
			else {	//we're working with a code which was inserted into practice by a user
				if(ReturnsRecords("SELECT UniqueID FROM ZipCodesT WHERE UniqueID = %li", nID)) {
					//if the record exists, update the PrimaryZip column
					ExecuteSql("UPDATE ZipCodesT SET PrimaryZip = 1 WHERE UniqueID = %li", nID);
				}
				else {
					//insert a new record
					// (j.gruber 2009-10-06 16:22) - PLID 35607 - add city also
					ExecuteSql("INSERT INTO ZipCodesT (UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip) values (%li, NULL, '%s', '%s', NULL, NULL, %li)", 
						NewNumber("ZipCodesT", "UniqueID"), strZip, _Q(strCity), 1);
				}
			}
			CClient::RefreshTable(NetUtils::PrimaryZipCode);
		} NxCatchAll("Error setting primary zip code");
	}

	m_strZip = VarString(m_pZipCodes->GetValue(nCurSel, 2),"");
	m_strCity = VarString(m_pZipCodes->GetValue(nCurSel, 3),"");
	m_strState = VarString(m_pZipCodes->GetValue(nCurSel, 4),"");
	m_strArea = VarString(m_pZipCodes->GetValue(nCurSel, 5),"");

	CDialog::OnOK();
}

BOOL CZipChooser::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_pZipCodes = BindNxDataListCtrl(IDC_ZIP_LIST, false);
	CheckDlgButton(IDC_MAKE_PRI, true);	//should be checked by default

	//load everything into the datalist
	//this function is pretty much a modified Requery (that is much slower) for pulling records from both places

	m_pZipCodes->Clear();
	
	try{

		/*
		_ConnectionPtr pMDB;
		try{
			CString strPath = NxRegUtils::ReadString(GetRegistryBase() + "InstallPath");
			strPath = strPath ^ "ZipCodeList.mdb";
			pMDB.CreateInstance("ADODB.Connection");
			pMDB->Provider = "Microsoft.Jet.OLEDB.4.0";
			pMDB->Open((LPCTSTR)strPath,"", "",adConnectUnspecified);
		} NxCatchAll("Error opening Zip Code List");
		

		_RecordsetPtr rsMDB(__uuidof(Recordset));
		CString sql;
		sql.Format("SELECT ID, Zip, City, State, Area FROM ZipCodeList WHERE Zip LIKE '%s'", _Q(m_strZip));

		rsMDB->Open(_bstr_t(sql), _variant_t((IDispatch *) pMDB, true), adOpenStatic, adLockReadOnly, adCmdText);
		*/

		// (a.walling 2006-12-06 09:36) - PLID 23754 - Use our ZipUtils to get the list of records for this zip code.
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		BOOL bResult = FALSE;
		if (m_bSearchZip) {
			bResult = LoadFromZipFile();			
		}
		else {
			bResult = LoadFromCityFile();			
		}
		
		// pass INVALID_HANDLE_VALUE to prevent any synchronization

		if (bResult) {
			
			//now we need to lookup everything in the ZipCodesT in practice and see if any of those fit in as well
			CString strField;
			if (m_bSearchZip) {
				strField = "ZipCode";
			}
			else {
				strField = "City";
			}					

			_RecordsetPtr rs = CreateRecordset("SELECT UniqueID, StaticID, ZipCode, City, State, AreaCode, PrimaryZip, Deleted FROM ZipCodesT WHERE %s LIKE '%s'", strField, _Q(m_strSearchField));

			while(!rs->eof) {
				FieldsPtr fields = rs->Fields;
				bool bFound = false;
				long id = VarLong(fields->Item["StaticID"]->Value, -1);
				_variant_t varID, varZip, varCity, varState, varArea, varPri, varUnique, varDeleted;
				varID = fields->Item["StaticID"]->Value;
				varUnique = fields->Item["UniqueID"]->Value;
				varZip = fields->Item["ZipCode"]->Value;
				varCity = fields->Item["City"]->Value;
				varState = fields->Item["State"]->Value;
				varArea = fields->Item["AreaCode"]->Value;
				varPri = fields->Item["PrimaryZip"]->Value;
				varDeleted = fields->Item["Deleted"]->Value;

				if(id != -1) {	//row has a static id
					//find the row that matches this one
					for(int i = 0; i < m_pZipCodes->GetRowCount(); i++) {
						if(m_pZipCodes->GetValue(i, 1) == varID) {
							if(VarBool(varDeleted)) {
								//this item is deleted!
								m_pZipCodes->RemoveRow(i);
								bFound = true;
							}
							else {
								//not deleted, safe to update
								//we need to update this row
								if (m_bSearchZip) {
									if(varCity.vt != VT_NULL)
										m_pZipCodes->PutValue(i, 3, varCity);
								}
								else {
									if(varZip.vt != VT_NULL)
										m_pZipCodes->PutValue(i, 2, varZip);
								}

								if(varState.vt != VT_NULL)
									m_pZipCodes->PutValue(i, 4, varState);
								if(varArea.vt != VT_NULL)
									m_pZipCodes->PutValue(i, 5, varArea);
								if(VarBool(varPri, FALSE)) {
									//it is the primary, so highlight it
									IRowSettingsPtr pRow;
									pRow = m_pZipCodes->GetRow(i);
									pRow->ForeColor = RGB(255,0,0);
									pRow->ForeColorSel = RGB(255,0,0);
								}
								if(varUnique.vt != VT_NULL)
									m_pZipCodes->PutValue(i, 0, varUnique);
								bFound = true;
							}
						}
					}//end for
				}

				if(!bFound) {	//item wasn't found in the list
					//this static id isn't in the list, so we have to add it manually

					if(!VarBool(varDeleted)) {
						//not deleted, safe to add it
						IRowSettingsPtr pRow;
						pRow = m_pZipCodes->GetRow(-1);
						pRow->PutValue(2, varZip);	//zip
						pRow->PutValue(3, varCity);	//city
						pRow->PutValue(4, varState);	//state
						pRow->PutValue(5, varArea);	//area
						pRow->PutValue(1, varID);
						pRow->PutValue(0, varUnique);
						if(VarBool(varPri, FALSE)) {
							//it is the primary, so highlight it
							pRow->ForeColor = RGB(255,0,0);
							pRow->ForeColorSel = RGB(255,0,0);
						}
						//insert into the datalist
						m_pZipCodes->AddRow(pRow);
					}
				}
				bFound = false;

				rs->MoveNext();

			}//end while

			//turn drawing back on
			m_pZipCodes->SetRedraw(true);

			//close the rs
			rs->Close();
			//pMDB->Close();

			//now set the static ctrl
			SetDlgItemText(IDC_ZIP_TEXT, m_strZip);
		}		
		
	} NxCatchAll("Error loading zip codes");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CZipChooser::LoadFromZipFile() 
{
	// (d.singleton 2011-11-18 09:18) - PLID 18875 - need to pass a BOOL & into ListAll, so make one
	BOOL bFailure;
	ZipcodeUtils::ZipcodeSet *pZs = NULL;
	pZs = ZipcodeUtils::ListAll(bFailure, INVALID_HANDLE_VALUE, ZipcodeUtils::zfZip, m_strSearchField.GetBuffer(0));
	
	// pass INVALID_HANDLE_VALUE to prevent any synchronization

	if (pZs) {

		//turn off redrawing so it's faster
		m_pZipCodes->SetRedraw(false);

		CString str;

		for (int i = 0; i < pZs->GetSize(); i++) {
			IRowSettingsPtr pRow;
			pRow = m_pZipCodes->GetRow(-1);
			ZipcodeUtils::ZipcodeInfo* zi = pZs->GetAt(i);

			// (a.walling 2006-12-06 09:37) - PLID 23754 - Fill in the datalist with values from the ZipSet
			if (zi) {
				pRow->PutValue(2, zi->Zip);
				pRow->PutValue(3, zi->City);
				pRow->PutValue(4, zi->State);

				str.Format("%03li", zi->nArea);
				pRow->PutValue(5, (_bstr_t)str);
				pRow->PutValue(1, zi->nID);
				pRow->PutValue(0, g_cvarNull);

				m_pZipCodes->InsertRow(pRow, -2);
			}
		}

		ZipcodeUtils::ClearZipcodeSet(pZs); // (a.walling 2006-12-06 09:38) - PLID 23754 - free all of our data

		return TRUE;
	}
	else {
		return FALSE;
	}
}

BOOL CZipChooser::LoadFromCityFile() 
{
	CityZipUtils::CityZipSet *pZs = NULL;
	pZs = CityZipUtils::ListAll(INVALID_HANDLE_VALUE, CityZipUtils::zfCity, m_strSearchField.GetBuffer(0));
	
	// pass INVALID_HANDLE_VALUE to prevent any synchronization

	if (pZs) {

		//turn off redrawing so it's faster
		m_pZipCodes->SetRedraw(false);

		CString str;

		for (int i = 0; i < pZs->GetSize(); i++) {
			IRowSettingsPtr pRow;
			pRow = m_pZipCodes->GetRow(-1);
			CityZipUtils::CityZipInfo* ci = pZs->GetAt(i);

			// (a.walling 2006-12-06 09:37) - PLID 23754 - Fill in the datalist with values from the ZipSet
			if (ci) {
				pRow->PutValue(2, ci->Zip);
				pRow->PutValue(3, ci->City);
				pRow->PutValue(4, ci->State);

				str.Format("%03li", ci->nArea);
				pRow->PutValue(5, (_bstr_t)str);
				pRow->PutValue(1, ci->nID);
				pRow->PutValue(0, g_cvarNull);

				m_pZipCodes->InsertRow(pRow, -2);
			}
		}

		CityZipUtils::ClearCityZipSet(pZs); // (a.walling 2006-12-06 09:38) - PLID 23754 - free all of our data

		return TRUE;
	}
	else {
		return FALSE;
	}
	
}



void CZipChooser::OnCancel() 
{
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CZipChooser, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CZipChooser)
	ON_EVENT(CZipChooser, IDC_ZIP_LIST, 3 /* DblClickCell */, OnDblClickCellZipList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CZipChooser::OnDblClickCellZipList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex != -1) {
		OnOK();
	}
}
