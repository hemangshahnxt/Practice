// NewPatientMatchInquiry.cpp : implementation file
//

#include "stdafx.h"
#include "patientsRc.h"
#include "NewPatientMatchInquiry.h"
#include "GlobalUtils.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewPatientMatchInquiry dialog

// (d.moore 2007-08-15) - PLID 25455 - This dialog lists inquiries that match the
//  values for m_strFirstName, m_strLastName, or m_strEmail. The user can then
//  select an inquiry from the list and the PersonID for that inquiry will be 
//  stored in m_nPersonID for retrieval from outside the dialog.

CNewPatientMatchInquiry::CNewPatientMatchInquiry(CWnd* pParent)
	: CNxDialog(CNewPatientMatchInquiry::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewPatientMatchInquiry)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNewPatientMatchInquiry::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewPatientMatchInquiry)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CNewPatientMatchInquiry, CNxDialog)
	//{{AFX_MSG_MAP(CNewPatientMatchInquiry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewPatientMatchInquiry message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CNewPatientMatchInquiry, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewPatientMatchInquiry)
	ON_EVENT(CNewPatientMatchInquiry, IDC_INQUIRY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedInquiryList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CNewPatientMatchInquiry::OnInitDialog() 
{
	CNxDialog::OnInitDialog(); // (a.walling 2011-01-14 16:44) - no PLID - Fix bad base class OnInitDialog call

	try {
		// (c.haag 2008-04-25 12:31) - PLID 29790 - NxIconified buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nPersonID = -1;
		m_pInquiryList = BindNxDataList2Ctrl(IDC_INQUIRY_LIST, false);

		CString strQuery = 
			"SELECT "
				"PersonT.ID AS PersonID, "
				"PatientsT.UserDefinedID, "
				"PersonT.[First], "
				"PersonT.Middle, "
				"PersonT.[Last], "
				"Persont.Email, "
				"'' AS Prodedures, "
				"ReferralSourceT.Name AS Referral "
			"FROM PersonT "
				"INNER JOIN PatientsT "
					"ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN MultiReferralsT "
					"ON PersonT.ID = MultiReferralsT.PatientID "
				"LEFT JOIN ReferralSourceT "
					"ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID "
			"WHERE PatientsT.CurrentStatus = 4";

		CString strWhere, strNameQuery, strTemp;
		bool bValidName = false;
		
		if (m_strFirstName.GetLength() > 0) {
			bValidName = true;
			// (z.manning, 11/27/2007) - PLID 28208 - Added _Q
			strNameQuery.Format("PersonT.[First] = '%s'", _Q(m_strFirstName));
		}

		if (m_strLastName.GetLength() > 0) {
			// (z.manning, 11/27/2007) - PLID 28208 - Added _Q
			strTemp.Format("PersonT.[Last] = '%s'", _Q(m_strLastName));
			if (bValidName) {
				strNameQuery = "(" + strNameQuery + " AND " + strTemp + ")";
			} else {
				strNameQuery = strTemp;
			}
			bValidName = true;
		}

		strWhere = strNameQuery;

		if (m_strEmail.GetLength()) {
			if (bValidName) {
				strWhere += " OR ";
			}
			// (z.manning, 11/27/2007) - PLID 28208 - Added _Q
			strTemp.Format("PersonT.Email = '%s'", _Q(m_strEmail));
			strWhere += strTemp;
		}

		if (strWhere.GetLength()) {
			strWhere = " AND (" + strWhere + ")";
		}
		
		strQuery = "(" + strQuery + strWhere + ") AS SubQ";
		
		m_pInquiryList->PutFromClause(_bstr_t(strQuery));
		m_pInquiryList->Requery();

	} NxCatchAll("Error In: CNewPatientMatchInquiry::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewPatientMatchInquiry::OnRequeryFinishedInquiryList(short nFlags) 
{
	// Get data for procedures and add it to the list of inquires.
	try {
		// If there aren't any rows in the list then we can't do anything here.
		if (m_pInquiryList->GetRowCount() <= 0) {
			return;
		}
		
		// Get the list of ID values from the list of inquiries.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInquiryList->GetFirstRow();
		CString strIdList, strID;
		long nPersonID;
		while (pRow != NULL) {
			nPersonID = VarLong(pRow->GetValue(eilPersonID));
			strID.Format("%li, ", nPersonID);
			strIdList += strID;
			pRow = pRow->GetNextRow();
		}
		strID = strID.Left(strID.GetLength() - 2);
		
		// Get the procedures associated with each inquiry. 
		// Some may have multiple procedures.
		CString strQuery;
		strQuery.Format(
			"SELECT ProcInfoT.PatientID, ProcedureT.Name "
			"FROM ProcInfoT "
				"INNER JOIN ProcInfoDetailsT "
					"ON ProcInfoT.ID = ProcInfoDetailsT.ProcInfoID "
				"INNER JOIN ProcedureT "
					"ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID "
			"WHERE ProcInfoT.PatientID IN (%s) "
			"ORDER BY PatientID, ProcedureT.Name", 
			strID);

		// Store the lists of procedures in arrays to make it easier to group them
		//  together based on the patient ID values.
		CArray<long, long> arIdList;
		CStringArray arProcedureList;
		CString strProcName;
		long nLastID = 0;
		long nIndex = -1;

		_RecordsetPtr rs = CreateRecordset(strQuery);
		while (!rs->eof) {
			nPersonID = AdoFldLong(rs, "PatientID");
			if (nPersonID != nLastID) {
				arIdList.Add(nPersonID);
				arProcedureList.Add("");
				nIndex++;
				nLastID = nPersonID;
			}
			strProcName.Format("%s, ", AdoFldString(rs, "Name", ""));
			arProcedureList[nIndex] += strProcName;
			rs->MoveNext();
		}
		rs->Close();

		// Now just match the patient ID values to the inquiry list and add
		//  the procedure lists to the NxDataList.
		long nArraySize = arIdList.GetSize();
		for (long i = 0; i < nArraySize; i++) {
			pRow = m_pInquiryList->FindByColumn(eilPersonID, arIdList[i], NULL, FALSE);
			if (pRow != NULL) {
				pRow->PutValue(eilProcedures, (_variant_t)arProcedureList[i]);
			}
		}

		// Set the first row as selected.
		m_pInquiryList->PutCurSel(m_pInquiryList->GetFirstRow());

	} NxCatchAll("Error In: CNewPatientMatchInquiry::OnRequeryFinishedInquiryList");
}

void CNewPatientMatchInquiry::OnOK() 
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInquiryList->CurSel;
	if (pRow == NULL) {
		m_nPersonID = -1;
		// Require a selection be made from the list.
		MessageBox("Please select an entry from the list, or click "
			"Cancel to proceed without converting an inquiry.");
	} else {
		m_nPersonID = VarLong(pRow->GetValue(eilPersonID), -1);
		CDialog::OnOK();
	}
}

void CNewPatientMatchInquiry::OnCancel() 
{
	try {
		m_nPersonID = -1;
		CDialog::OnCancel();
	} NxCatchAll("Error In: CNewPatientMatchInquiry::OnCancel");
}

