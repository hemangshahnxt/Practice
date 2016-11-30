// ImplementationEditSpecialistDlg.cpp : implementation file
//

// (j.gruber 2008-04-02 16:50) - PLID 28979 - Created For
#include "stdafx.h"
#include "patientsRc.h"
#include "ImplementationEditSpecialistDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImplementationEditSpecialistDlg dialog


CImplementationEditSpecialistDlg::CImplementationEditSpecialistDlg(CWnd* pParent)
	: CNxDialog(CImplementationEditSpecialistDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImplementationEditSpecialistDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CImplementationEditSpecialistDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImplementationEditSpecialistDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CImplementationEditSpecialistDlg, CNxDialog)
	//{{AFX_MSG_MAP(CImplementationEditSpecialistDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImplementationEditSpecialistDlg message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CImplementationEditSpecialistDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CImplementationEditSpecialistDlg)
	ON_EVENT(CImplementationEditSpecialistDlg, IDC_USERS_SPECIALIST_LIST, 10 /* EditingFinished */, OnEditingFinishedUsersList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CImplementationEditSpecialistDlg, IDC_USERS_SPECIALIST_LIST, 9 /* EditingFinishing */, OnEditingFinishingUsersList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CImplementationEditSpecialistDlg::OnEditingFinishedUsersList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		if (bCommit) {

			switch (nCol) {

				case 1:
					{

						NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

						if (pRow) {
							long nUserID = VarLong(pRow->GetValue(0));
							
							if (VarBool(varNewValue)) {

								ExecuteParamSql("INSERT INTO EMRSpecialistT (UserID) VALUES ({INT})", nUserID);
							}
							else {

								// if we got this far, we must be ok
								ExecuteParamSql("DELETE FROM EMRSpecialistT WHERE UserID = {INT}", nUserID);
							}
						}
						else {

							ThrowNxException("Could not get row!");
						}
					}
				break;
			}
		}
	

	}NxCatchAll("Error in CImplementationEditSpecialistDlg::OnEditingFinishedUsersList");
	
}

void CImplementationEditSpecialistDlg::OnEditingFinishingUsersList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		switch (nCol) {

			case 1: //checkbox, the only column that is editable

				//see if they are unselecting or selecting
				if (VarBool(pvarNewValue)) {

					//they are selecting, we don't need to check anything
				}
				else {

					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

					if (pRow) {

						long nSpecialistID = VarLong(pRow->GetValue(0));
						
						//they are unselecting we have to check if there is anyone with this specialist
						// (j.gruber 2010-07-29 15:40) - PLID 39878 - check secondary also
						ADODB::_RecordsetPtr rsCheck = CreateParamRecordset("SELECT top 1 PersonID FROM NxClientsT WHERE EmrSpecialistID = {INT} OR SecondaryEMRSpecialistID = {INT}", nSpecialistID, nSpecialistID);
						if (! rsCheck->eof) {

							MsgBox("There are clients with this specialist, you may not uncheck it");
							*pbCommit = FALSE;
							*pbContinue = FALSE;
						}
					}
					else {
						ThrowNxException("Could not find row!");
					}
				}
			break;
		}

	}NxCatchAll("Error in CImplementationEditSpecialistDlg::OnEditingFinishingUsersList");
	
}

BOOL CImplementationEditSpecialistDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();


	try {

		m_btnClose.AutoSet(NXB_CLOSE);

		// (j.gruber 2010-07-29 15:11) - PLID 39906 - fix the bind to stop throwing an error
		m_pUsersList = BindNxDataList2Ctrl(IDC_USERS_SPECIALIST_LIST, false);

		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT UsersT.PersonID, UserName, "
			" CASE WHEN EmrSpecialistT.UserID IS NULL then 0 else 1 END as IsSpecialist "
			" FROM UsersT LEFT JOIN EMRSpecialistT ON UsersT.PersonID = EMRSpecialistT.UserID "
			" LEFT JOIN PersonT ON UsersT.PersonID = PersonT.ID "
			" WHERE (EmrSpecialistT.UserID IS NULL AND PersonT.Archived = 0) OR (EMRSpecialistT.UserID IS NOT NULL) ");

		while (! rs->eof) {

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pUsersList->GetNewRow();

			bool bSpec = AdoFldLong(rs, "IsSpecialist") == 0 ? false : true;

			pRow->PutValue(0, AdoFldLong(rs, "PersonID"));
			pRow->PutValue(1, _variant_t(bSpec));
			pRow->PutValue(2, _variant_t(AdoFldString(rs, "Username")));
			m_pUsersList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}


	}NxCatchAll("Error in CImplementationEditSpecialistDlg::OnInitDialog");
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CImplementationEditSpecialistDlg::OnOK() {

	CDialog::OnOK();
}


void CImplementationEditSpecialistDlg::OnCancel() {

	CDialog::OnOK();
}