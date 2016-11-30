// ConfigPacketsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "nxstandard.h"
#include "letterwriting.h"
#include "ConfigPacketsDlg.h"
#include "EditPacketDlg.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigPacketsDlg dialog


CConfigPacketsDlg::CConfigPacketsDlg(CWnd* pParent)
	: CNxDialog(CConfigPacketsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigPacketsDlg)
		m_bShowProcSpecific = false;
	//}}AFX_DATA_INIT
}


void CConfigPacketsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigPacketsDlg)
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_ADD_COPY, m_btnAddCopy);
	DDX_Control(pDX, IDC_EDIT_PACKET, m_btnEditPacket);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnDone);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigPacketsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigPacketsDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_EDIT_PACKET, OnEdit)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_ADD_COPY, OnAddCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigPacketsDlg message handlers

using namespace ADODB;
void CConfigPacketsDlg::OnAdd() 
{
	try {
		CString strNewName;
		if(IDOK == InputBoxLimited(this, "Enter a name to identify this packet: ", strNewName, "",50,false,false,NULL)) {

			strNewName.TrimRight();
			
			if(strNewName == "") {
				MessageBox("You cannot use a blank name for this packet.");
				return;
			}		
			_RecordsetPtr rsDuplicate = CreateRecordset("SELECT ID,ProcedureRelated FROM PacketsT WHERE Name = '%s' AND Deleted = 0", _Q(strNewName));
			if (! rsDuplicate->eof) {
				BOOL bProcedure = AdoFldBool(rsDuplicate, "ProcedureRelated");
				if (bProcedure) {
					MsgBox("There is already a packet created from the PIC named %s.  Please choose a different name for this packet.", strNewName);
				}
				else {
					MsgBox("There is already a packet created from the letterwriting module named %s.  Please choose a different name for this packet.", strNewName);
				}
				return;
			}

			int nNewID = NewNumber("PacketsT", "ID");
			ExecuteSql("INSERT INTO PacketsT (ID, Name, ProcedureRelated) VALUES (%li, '%s', %li)", NewNumber("PacketsT", "ID"), _Q(strNewName), m_bShowProcSpecific ? 1 : 0);
			CEditPacketDlg dlg(this);
			dlg.m_nPacketID = nNewID;
			dlg.DoModal();
			m_pExistingPackets->Requery();
		}
	}NxCatchAll("Error in CConfigPacketsDlg::OnAdd()");
}

BOOL CConfigPacketsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (z.manning, 04/25/2008) - PLID 29795 - Set button styles
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnAddCopy.AutoSet(NXB_NEW);
		m_btnEditPacket.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnDone.AutoSet(NXB_CLOSE);

		m_pExistingPackets = BindNxDataListCtrl(IDC_EXISTING_PACKETS, false);

		if(m_bShowProcSpecific) {
			m_pExistingPackets->WhereClause = _bstr_t("Deleted = 0 AND ProcedureRelated = 1");
			//(e.lally 2010-01-15) PLID 9932 - When opened with procedure specific, call it PIC Packets for clarity
			SetWindowText("Configure PIC Packets");
		}
		else {
			m_pExistingPackets->WhereClause = _bstr_t("Deleted = 0 AND ProcedureRelated = 0");
			//(e.lally 2010-01-15) PLID 9932 - When opened normally, call it Letter Writing Packets for clarity
			SetWindowText("Configure Letter Writing Packets");
		}
		m_pExistingPackets->Requery();

	} NxCatchAll("Error in CConfigPacketsDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigPacketsDlg::OnEdit() 
{
	try {
		int nSel = m_pExistingPackets->CurSel;
		if(nSel != -1) {
			CEditPacketDlg dlg(this);
			dlg.m_nPacketID = VarLong(m_pExistingPackets->GetValue(nSel, 0));
			dlg.DoModal();
			m_pExistingPackets->Requery();
		}
	}NxCatchAll("Error in CConfigPacketsDlg::OnEdit()");
}

void CConfigPacketsDlg::OnDelete() 
{
	try {
		int nSel = m_pExistingPackets->CurSel;
		if(nSel != -1) {
			if(MessageBox("Are you sure you want to delete this packet?", NULL, MB_YESNO) == IDYES) {
				int nPacketID = VarLong(m_pExistingPackets->GetValue(nSel, 0));
				//ExecuteSql("DELETE FROM PacketComponentsT WHERE PacketID = %li", nPacketID);
				ExecuteSql("UPDATE PacketsT SET Deleted = 1 WHERE ID = %li", nPacketID);
				m_pExistingPackets->Requery();
			}
		}
	}NxCatchAll("Error in CConfigPacketsDlg::OnDelete()");
}

void CConfigPacketsDlg::OnOK() 
{
	
	CNxDialog::OnOK();
}

void CConfigPacketsDlg::OnAddCopy() 
{
	try {
		int nSel = m_pExistingPackets->CurSel;

		if(nSel == -1) {
			MessageBox("Please select a packet to copy.");
		}
		else {
			// a.walling 4/27/06 PLID 20322 Get the catID to insert later
			long nCatID = VarLong(m_pExistingPackets->GetValue(nSel, 2), -1);

			CString strNewName;
			CString strNewCatID;
			if (nCatID == -1) {
				strNewCatID = "NULL";
			}
			else {
				strNewCatID.Format("%li", nCatID);
			}

			if(IDOK == InputBoxLimited(this, "Enter a name for the new copy: ", strNewName, "", 50, false, false, NULL)) {

				strNewName.TrimRight();
				
				if(strNewName == "") {
					MessageBox("You cannot use a blank name for a packet.");
					return;
				}		
				if(ReturnsRecords("SELECT ID FROM PacketsT WHERE Name = '%s' AND Deleted = 0", _Q(strNewName))) {
					MsgBox("There is already a packet named %s.  Please choose a different name for this packet.", strNewName);
					return;
				}

				int nNewID = NewNumber("PacketsT", "ID");
				ExecuteSql("INSERT INTO PacketsT (ID, Name, ProcedureRelated, PacketCategoryID) VALUES (%li, '%s', %li, %s)", nNewID, _Q(strNewName), m_bShowProcSpecific ? 1 : 0, strNewCatID);
				ExecuteSql("INSERT INTO PacketComponentsT (PacketID, MergeTemplateID, ComponentOrder, Scope) SELECT %li, MergeTemplateID, ComponentOrder, Scope FROM PacketComponentsT WHERE PacketID = %li", nNewID, VarLong(m_pExistingPackets->GetValue(nSel, 0)));
				m_pExistingPackets->Requery();
			}
		}
	}NxCatchAll("Error in CConfigPacketsDlg::OnAddCopy()");

}

void CConfigPacketsDlg::OnCancel() 
{	
	CNxDialog::OnCancel();
}


BEGIN_EVENTSINK_MAP(CConfigPacketsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigPacketsDlg)
	ON_EVENT(CConfigPacketsDlg, IDC_EXISTING_PACKETS, 9 /* EditingFinishing */, OnEditingFinishingExistingPackets, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CConfigPacketsDlg, IDC_EXISTING_PACKETS, 10 /* EditingFinished */, OnEditingFinishedExistingPackets, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CConfigPacketsDlg::OnEditingFinishingExistingPackets(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		switch(nCol) {
			case 1:  //Name of the packet
				if(varOldValue.vt == VT_BSTR && pvarNewValue->vt == VT_BSTR){
					if(VarString(varOldValue) == VarString(pvarNewValue)){
						// the values are the same, don't do anything
						return;
					}

					//(e.lally 2008-10-13) PLID 31665 - Put format string inside ReturnsRecords params.
					if(ReturnsRecords("SELECT Name FROM PacketsT WHERE Name = '%s'", _Q(VarString(pvarNewValue)))) {
						AfxMessageBox("There is already a packet with this name.");
						*pbCommit = FALSE;
						return;
					}
				}
				break;

			case 2:  //Category of the Packet a.walling 4/28/06 PLID 20322
				if (pvarNewValue->vt == VT_EMPTY || varOldValue.vt == VT_EMPTY ) {
					*pbCommit = FALSE;
					return;
				}

				long nNewVal = VarLong(varOldValue, -1), nOldVal = VarLong(pvarNewValue, -1);

				if (nNewVal == nOldVal) { // values are the same;
					*pbCommit = FALSE;
					return;
				}
				else {
					
					return;
				}
		}
	}NxCatchAll("CConfigPacketsDlg::OnEditingFinishingExistingPackets()");
}

void CConfigPacketsDlg::OnEditingFinishedExistingPackets(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if(bCommit){
			CString strSql;
			long ID = VarLong(m_pExistingPackets->GetValue(nRow, 0));

			// a.walling 4/27/06 PLID 20322 Set the category for the packet
			if (nCol == 1) { // Name 
				//(e.lally 2008-10-13)PLID 31665 - Use ExecuteSqlStd for pre-formatted strings
				strSql.Format("UPDATE PacketsT SET Name = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), ID);
				ExecuteSqlStd(strSql);
			}
			else if (nCol == 2) { // Category
				CString strNewVal;
				long nNewVal;

				nNewVal = VarLong(varNewValue);
				if (nNewVal == -1) {
					strNewVal = "NULL";
				}
				else {
					strNewVal.Format("%li", nNewVal);
				}

				strSql.Format("UPDATE PacketsT SET PacketCategoryID = %s WHERE ID = %li", strNewVal, ID);
				//(e.lally 2008-10-13)PLID 31665 - Use ExecuteSqlStd for pre-formatted strings
				ExecuteSqlStd(strSql);
			}
		}	
	}NxCatchAll("CConfigPacketsDlg::OnEditingFinishedExistingPackets()");
}
