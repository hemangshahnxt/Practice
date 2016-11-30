// CMassAssignSecurityCodesDlg.cpp : implementation file
//

//(s.dhole 12/5/2014 2:28 PM ) - PLID 64337 Added new dialog to show Mass Assign Security Codes from admin ->Activities
#include "stdafx.h"
#include "Practice.h"
#include "MassAssignSecurityCodesDlg.h"
#include "afxdialogex.h"
#include "AuditTrail.h"

using namespace ADODB;

// CMassAssignSecurityCodesDlg dialog

IMPLEMENT_DYNAMIC(CMassAssignSecurityCodesDlg, CNxDialog)

CMassAssignSecurityCodesDlg::CMassAssignSecurityCodesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMassAssignSecurityCodesDlg::IDD, pParent)
{

}

CMassAssignSecurityCodesDlg::~CMassAssignSecurityCodesDlg()
{
}

void CMassAssignSecurityCodesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDYES, m_btnYes);
	DDX_Control(pDX, IDNO, m_btnNo);
	DDX_Control(pDX, IDC_MASS_ASSIGN_SECURITY_CODES_BKG, m_nxclrBackground);
}


BEGIN_MESSAGE_MAP(CMassAssignSecurityCodesDlg, CNxDialog)
	ON_BN_CLICKED(IDYES, &CMassAssignSecurityCodesDlg::OnBnClickedYes)
	ON_BN_CLICKED(IDNO, &CMassAssignSecurityCodesDlg::OnBnClickedNo)
END_MESSAGE_MAP()


// CMassAssignSecurityCodesDlg message handlers

BOOL CMassAssignSecurityCodesDlg::OnInitDialog()
{
	try{

		CNxDialog::OnInitDialog();
		SetWindowText("Mass Assign Security Codes" );
		m_btnYes.AutoSet(NXB_OK);
		m_btnNo.AutoSet(NXB_CANCEL);
		m_nxclrBackground.SetColor(GetNxColor(GNC_ADMIN, 1));
		return TRUE;
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CMassAssignSecurityCodesDlg::OnBnClickedYes()
{
	try{
		CWaitCursor wc;
		if (ProcessSecuritycode() == FALSE)
		{
			MessageBox("Failed to assign security codes.", "Practice", MB_OK | MB_ICONSTOP);
		}
		else
		{
		 // Nothing	
		}
		// still exit
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}


void CMassAssignSecurityCodesDlg::OnBnClickedNo()
{
	try{
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__);
}
//(s.dhole 12/8/2014 9:48 AM ) - PLID 64338 We will update  all security code to all active  patients and prospects who are missing  NexWeb security code or NexWeb username(login), and uudiut this action

BOOL CMassAssignSecurityCodesDlg::ProcessSecuritycode()
{
	long nAuditTransactionID = -1;
	try 
	{ 
	// We will update  all security code to all active  patients and prospects who are missing  NexWeb security code or NexWeb username(login),
	//return list of updated patients and ther security code to audit
	// (v.maida 2015-06-05 16:58) - PLID 66178 - The InitialSecurityCodeCreationDate field is not being set when creating security codes en masse.
	// (e.frazier 2016-05-09 12:10) - PLID 66533 - The Mass Assign Security Codes functionality can skip patients in certain circumstances.
		CString strSQL = "SET XACT_ABORT ON \r\n"
			" SET NOCOUNT ON   \r\n"
			" BEGIN TRAN \r\n"
			" \r\n"
			"       DECLARE  @UpdatedPatients TABLE\r\n"
			"          ( \r\n"
			"           PersonID INT, \r\n"
			"           SecurityCode NVARCHAR(9) \r\n"
			"          ); \r\n"
			"       DECLARE @i INT \r\n"
			"       SET @i = 0 \r\n"
			"		WHILE(@i < 11) BEGIN \r\n"
			"		\r\n"
			"		; WITH PatientsFilterMissingSecurityCodeQ AS( "
			"      --patients of interest \r\n"
			"		SELECT  PatientsQ.PersonID \r\n"
			"		FROM PatientsT PatientsQ \r\n"
			"		INNER JOIN PersonT PersonQ ON PatientsQ.PersonID = PersonQ.ID \r\n"
			"		WHERE PersonQ.ID > 0 \r\n"
			"		AND PersonQ.Archived = 0 \r\n"
			"		AND PatientsQ.PersonID NOT IN(SELECT L.PersonID FROM NexWebLoginInfoT L) \r\n"
			"		AND PatientsQ.SecurityCode IS NULL \r\n"
			"		AND PatientsQ.CurrentStatus <> 4 \r\n"
			"		) \r\n"
			"		, PatientsFilterWithNewSecurityCodeQ  AS(--come up with a random security code for each one \r\n"
			"		SELECT PatientsFilterMissingSecurityCodeQ.PersonID, CASE WHEN LEN(NewSecurityCode) < 9 THEN LEFT('000000000', (9 - LEN(NewSecurityCode))) + NewSecurityCode \r\n"
			"		ELSE NewSecurityCode END AS NewSecurityCode \r\n"
			"		FROM PatientsFilterMissingSecurityCodeQ, (SELECT CONVERT(NVARCHAR(9), CONVERT(INT, RAND(CAST(NEWID() AS VARBINARY)) * 1000000000)) AS NewSecurityCode) X \r\n"
			"		) \r\n"
			"		, PatientsFilterWithNewSecurityCodeNoDuplicateQ AS(--group by the new security code to eliminate duplicates against self \r\n"
			"		SELECT MIN(PatientsFilterWithNewSecurityCodeQ.PersonID) AS PersonID, PatientsFilterWithNewSecurityCodeQ.NewSecurityCode \r\n"
			"		FROM PatientsFilterWithNewSecurityCodeQ \r\n"
			"		GROUP BY NewSecurityCode \r\n"
			"		) \r\n"
			"		, PatientsAllWithNewSecurityCodeNoDuplicateQ AS(--left join to patientst to eliminate duplicates against existing \r\n"
			"		SELECT PatientsFilterWithNewSecurityCodeNoDuplicateQ.PersonID, PatientsFilterWithNewSecurityCodeNoDuplicateQ.NewSecurityCode, GETDATE() AS NewSecurityCodeCreationDate \r\n"
			"		FROM PatientsFilterWithNewSecurityCodeNoDuplicateQ \r\n"
			"		LEFT JOIN PatientsT P ON PatientsFilterWithNewSecurityCodeNoDuplicateQ.NewSecurityCode = P.SecurityCode \r\n"
			"		WHERE P.SecurityCode IS NULL \r\n"
			"		) \r\n"
			"		--all set, no duplicates left, so drill them in there \r\n"
			"		--NOTICE: We write to all patients who are missing security codes, even if we have \r\n"
			"		--        failed to come up with a unique new code for them. This will let us know \r\n"
			"		--        to keep looping until we have nobody left to write to. \r\n"
			"		UPDATE PatientsT SET SecurityCode = PatientsAllWithNewSecurityCodeNoDuplicateQ.NewSecurityCode, \r\n"
			"       InitialSecurityCodeCreationDate = CASE WHEN InitialSecurityCodeCreationDate IS NULL THEN PatientsAllWithNewSecurityCodeNoDuplicateQ.NewSecurityCodeCreationDate ELSE InitialSecurityCodeCreationDate END, \r\n"
			"       SecurityCodeCreationDate = PatientsAllWithNewSecurityCodeNoDuplicateQ.NewSecurityCodeCreationDate \r\n"
			"		OUTPUT  inserted.PersonID, inserted.SecurityCode \r\n"
			"		INTO @UpdatedPatients  \r\n"
			"		FROM PatientsT \r\n"
			"		INNER JOIN PatientsFilterMissingSecurityCodeQ ON PatientsFilterMissingSecurityCodeQ.PersonID = PatientsT.PersonID \r\n"
			"		LEFT JOIN PatientsAllWithNewSecurityCodeNoDuplicateQ ON PatientsAllWithNewSecurityCodeNoDuplicateQ.PersonID = PatientsT.PersonID \r\n"
			"		-- if we had nothing to update, then job done \r\n"
			"		IF @@ROWCOUNT = 0 \r\n"
			"		BEGIN \r\n"
			"			COMMIT TRAN \r\n"
			"			BREAK \r\n"
			"		END \r\n"
			"		ELSE \r\n"
			"		BEGIN \r\n"
			"			SET @i = @i + 1 \r\n"
			"			-- if we did update, then we might need to update again because of eliminated duplicates, so loop back up; it’s extremely unlikely we’d do this more than one more time, but we’ll give it up to 10 just in case \r\n"
			"		END \r\n"
			"		--it should continue til 10th loop .. if we are at this level then we have issues and it should throw error \r\n"
			"		IF @i>10 \r\n"
			"		BEGIN \r\n"
			"			ROLLBACK TRAN \r\n"
			"			RAISERROR('FAILURE: Transaction rolled back, Failed to assing security code in 10th attempt', 16, 1) \r\n"
			"		BREAK \r\n"
			"		END \r\n"
			" END \r\n"
			" SET NOCOUNT OFF \r\n"
			" SELECT Q.PersonID, Q.SecurityCode, PersonT.FullName FROM @UpdatedPatients  Q INNER JOIN  PersonT ON PersonT.ID = Q.PersonID \r\n";
	ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();
	long nCount = 0;
	CIncreaseCommandTimeout noTimeout(pCon, 600);
	_RecordsetPtr rsUpdatedPatients = CreateRecordset(pCon, strSQL);
	if (!rsUpdatedPatients->eof) {
		if (nAuditTransactionID == -1)
		{
			nAuditTransactionID = BeginAuditTransaction();
		}
		
		while (!rsUpdatedPatients->eof) {
			long nPatitnID = AdoFldLong(rsUpdatedPatients, "PersonID");
			CString strSecurityCode = AdoFldString(rsUpdatedPatients, "SecurityCode", "");
			CString strPatientName = AdoFldString(rsUpdatedPatients, "FullName", "");
			AuditEvent(nPatitnID, strPatientName, nAuditTransactionID, aeiPatientSecurityCodeSet, nPatitnID, "", strSecurityCode, aepHigh, aetChanged);
			nCount++;
			rsUpdatedPatients->MoveNext();
		}

		
		CommitAuditTransaction(nAuditTransactionID);
		nAuditTransactionID = -1;
	}
	if (nCount > 0)
	{
		MessageBox(FormatString("%li security codes assigned.", nCount));
	}
	else
	{
		MessageBox("All patients have existing security codes. No new security codes were assigned.");
	}

	return TRUE;
	}NxCatchAllCall(__FUNCTION__, if (nAuditTransactionID == -1){ RollbackAuditTransaction(nAuditTransactionID); });
	return FALSE ;
}


