// InactiveProceduresDlg.cpp : implementation file
// (c.haag 2008-11-26 13:45) - PLID 10776 - Iniital implementation
//

#include "stdafx.h"
#include "Practice.h"
#include "InactiveProceduresDlg.h"
#include "AdministratorRc.h"
#include "AuditTrail.h"

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// CInactiveProceduresDlg dialog

IMPLEMENT_DYNAMIC(CInactiveProceduresDlg, CNxDialog)

CInactiveProceduresDlg::CInactiveProceduresDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInactiveProceduresDlg::IDD, pParent)
{
	m_nActivatedProcedures = 0;
}

CInactiveProceduresDlg::~CInactiveProceduresDlg()
{
}

int CInactiveProceduresDlg::GetActivatedProcedureCount()
{
	return m_nActivatedProcedures;
}

void CInactiveProceduresDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
}


BEGIN_MESSAGE_MAP(CInactiveProceduresDlg, CNxDialog)
END_MESSAGE_MAP()


// CInactiveProceduresDlg message handlers

BOOL CInactiveProceduresDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_dlProcedures = BindNxDataList2Ctrl(IDC_INACTIVE_PROCEDURES);
	}
	NxCatchAll("Error in CInactiveProceduresDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
BEGIN_EVENTSINK_MAP(CInactiveProceduresDlg, CNxDialog)
	ON_EVENT(CInactiveProceduresDlg, IDC_INACTIVE_PROCEDURES, 3, CInactiveProceduresDlg::DblClickCellInactiveTypes, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CInactiveProceduresDlg::DblClickCellInactiveTypes(LPDISPATCH lpRow, short nColIndex)
{
	try {
		if(NULL != lpRow) {
			IRowSettingsPtr pRow(lpRow);
			const long nID = VarLong(pRow->GetValue(0));
			const CString strProcedure = VarString(pRow->GetValue(1));
			if(IDYES == MsgBox(MB_YESNO, "Are you sure you wish to restore the procedure %s to active use?", strProcedure)) {
				ExecuteParamSql("UPDATE ProcedureT SET Inactive = 0, InactivatedDate = NULL, InactivatedBy = NULL WHERE ID = {INT}", nID);
				// (c.haag 2008-12-03 17:56) - PLID 10776 - Auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiSchedProcInactivated, nID, "<Inactivated>", strProcedure, aepHigh, aetChanged);
				m_nActivatedProcedures++;
				m_dlProcedures->RemoveRow(pRow);
			}
		}
	}NxCatchAll("Error in CInactiveProceduresDlg::DblClickCellInactiveTypes");
}
