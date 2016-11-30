// EditPacketDlg.cpp : implementation file
//

#include "stdafx.h"
#include "letterwriting.h"
#include "EditPacketDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEditPacketDlg dialog


CEditPacketDlg::CEditPacketDlg(CWnd* pParent)
	: CNxDialog(CEditPacketDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditPacketDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditPacketDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditPacketDlg)
	DDX_Control(pDX, IDC_COMPONENT_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_COMPONENT_UP, m_btnUp);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnDone);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditPacketDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditPacketDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_COMPONENT_UP, OnComponentUp)
	ON_BN_CLICKED(IDC_COMPONENT_DOWN, OnComponentDown)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditPacketDlg message handlers

BOOL CEditPacketDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/25/2008) - PLID 29795 - Set button styles
	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnDone.AutoSet(NXB_CLOSE);
	
	m_pComponentList = BindNxDataListCtrl(IDC_COMPONENT_LIST, false);
	
	if(!ReturnsRecords("SELECT ID FROM PacketsT WHERE ID = %li AND ProcedureRelated = 1 AND Deleted = 0", m_nPacketID)) {
		m_pComponentList->GetColumn(3)->PutStoredWidth(0);
		m_pComponentList->GetColumn(3)->ColumnStyle = csFixedWidth;
	}

	CString strWhere;
	strWhere.Format("PacketComponentsT.PacketID = %li", m_nPacketID);
	m_pComponentList->WhereClause = _bstr_t(strWhere);
	m_pComponentList->Requery();

	CString strCaption;
	_RecordsetPtr rsName = CreateRecordset("SELECT Name FROM PacketsT WHERE ID = %li", m_nPacketID);
	strCaption = AdoFldString(rsName, "Name");
	SetWindowText(strCaption);

	m_btnUp.AutoSet(NXB_UP);
	m_btnDown.AutoSet(NXB_DOWN);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditPacketDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CNxDialog::OnOK();
}

void CEditPacketDlg::OnAdd() 
{
	try {
		// (a.walling 2007-06-14 16:02) - PLID 26342 - Support Word 2007 templates
		// Always support Word 2007 templates
		// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
		CString strFilter = "Microsoft Word Templates|*.dot;*.dotx;*.dotm||";

		CFileDialog dlg(TRUE, "dot", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);

		// set up open document dialog
		CString strTemplatePath = GetTemplatePath();
		dlg.m_ofn.lpstrInitialDir = strTemplatePath;
		dlg.m_ofn.lpstrTitle = "Select a merge template";

		if (dlg.DoModal() == IDOK) {	
			long nNewTemplateID;
			CString strPath = dlg.GetPathName();
			if(strPath.Left(GetSharedPath().GetLength()).CompareNoCase(GetSharedPath()) == 0) {
				strPath = strPath.Mid(GetSharedPath().GetLength());
				//Since this is presumed to be relative to the shared path, make sure it starts with exactly one '\'.
				if(strPath.Left(1) != "\\") {
					strPath = "\\" + strPath;
				}
			}
			
			//Either way, let's make it all upper case
			strPath.MakeUpper();
			//OK, strPath is now a valid record.
			//TES 12/18/2006 - PLID 23917 - We will load the default scope for this template, if it has one.
			long nDefaultScope = 0; //The "global" default is 0, which is one-per-PIC.
			_RecordsetPtr rsTemplateID = CreateRecordset("SELECT ID, DefaultScope FROM MergeTemplatesT WHERE Path = '%s'", _Q(strPath));
			if(rsTemplateID->eof) {
				//OK, we need to store it.
				nNewTemplateID = NewNumber("MergeTemplatesT", "ID");
				ExecuteSql("INSERT INTO MergeTemplatesT (ID, Path) VALUES (%li, '%s')", nNewTemplateID, _Q(strPath));
			}
			else {
				//It already has been stored.
				nNewTemplateID = AdoFldLong(rsTemplateID, "ID");
				nDefaultScope = AdoFldLong(rsTemplateID, "DefaultScope", 0);
				
				//Check whether this is already in the packet.
				if(ReturnsRecords("SELECT PacketID FROM PacketComponentsT WHERE PacketID = %li AND MergeTemplateID = %li", m_nPacketID, nNewTemplateID) ) {
					if(IDYES != MsgBox(MB_YESNO, "The selected template is already part of this packet.  Are you sure you wish to add it again?"))
						return;
				}
			}
			
			//Now add this new template to our packet.
			//Get the highest "order" in this packet.
			_RecordsetPtr rsOrder = CreateRecordset("SELECT CASE WHEN EXISTS (SELECT ComponentOrder FROM PacketComponentsT WHERE PacketID = %li) THEN (SELECT Max(ComponentOrder) FROM PacketComponentsT WHERE PacketID = %li) ELSE 0 END AS HighestOrder", m_nPacketID, m_nPacketID);
			ExecuteSql("INSERT INTO PacketComponentsT (PacketID, MergeTemplateID, ComponentOrder, Scope) VALUES (%li, %li, %li+1, %li)", m_nPacketID, nNewTemplateID, AdoFldLong(rsOrder, "HighestOrder"), nDefaultScope);

			m_pComponentList->Requery();
		}
	}NxCatchAll("Error in CEditPacketDlg::OnAdd()");
}

void CEditPacketDlg::OnDelete() 
{
	try {
		if(m_pComponentList->CurSel != -1) {
			if(IDYES == MessageBox("Are you sure you want to remove this template from this packet?", NULL, MB_YESNO)) {
				ExecuteSql("DELETE FROM PacketComponentsT WHERE ID = %li", VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 4)));
				m_pComponentList->Requery();
			}
		}
	}NxCatchAll("Error in CEditPacketDlg::OnDelete()");
}

void CEditPacketDlg::OnComponentUp() 
{
	try {
		//If there is nothing selected, or the top one is selected, then do nothing.
		if(m_pComponentList->CurSel > 0) {
			//Get the order of the currently selected one.
			long nThisOrder, nNextLowestOrder;
			nThisOrder = VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 0));

			//Get the next-lowest order.
			nNextLowestOrder = VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel-1, 0));

			//Set this one's order to the next lowest.
			ExecuteSql("UPDATE PacketComponentsT SET ComponentOrder = %li WHERE ID = %li", nNextLowestOrder, VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 4)));

			//Set the next lowest one's order to this one.
			ExecuteSql("UPDATE PacketComponentsT SET ComponentOrder = %li WHERE ID = %li", nThisOrder, VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel-1, 4)));

			//Requery
			m_nNextSel = m_pComponentList->CurSel-1;
			m_pComponentList->Requery();
		}
	}NxCatchAll("Error in CEditPacketDlg::OnComponentUp()");
}

BEGIN_EVENTSINK_MAP(CEditPacketDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditPacketDlg)
	ON_EVENT(CEditPacketDlg, IDC_COMPONENT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedComponentList, VTS_I2)
	ON_EVENT(CEditPacketDlg, IDC_COMPONENT_LIST, 10 /* EditingFinished */, OnEditingFinishedComponentList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditPacketDlg::OnRequeryFinishedComponentList(short nFlags) 
{
	m_pComponentList->CurSel = m_nNextSel;
}

void CEditPacketDlg::OnComponentDown() 
{
	try {
		//If there is nothing selected, or the bottom one is selected, then do nothing.
		if(m_pComponentList->CurSel != -1 && m_pComponentList->CurSel != m_pComponentList->GetRowCount()-1) {
			//Get the order of the currently selected one.
			long nThisOrder, nNextHighestOrder;
			nThisOrder = VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 0));

			//Get the next-highest order.
			nNextHighestOrder = VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel+1, 0));

			//Set this one's order to the next lowest.
			ExecuteSql("UPDATE PacketComponentsT SET ComponentOrder = %li WHERE ID = %li", nNextHighestOrder, VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 4)));

			//Set the next lowest one's order to this one.
			ExecuteSql("UPDATE PacketComponentsT SET ComponentOrder = %li WHERE ID = %li", nThisOrder, VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel+1, 4)));

			//Requery
			m_nNextSel = m_pComponentList->CurSel+1;
			m_pComponentList->Requery();
		}
	}NxCatchAll("Error in CEditPacketDlg::OnComponentDown()");
	
}

void CEditPacketDlg::OnEditingFinishedComponentList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if(bCommit) {
			switch(nCol) {
			case 3:
				{
					//TES 12/19/2006 - PLID 23917 - Remember this value for next time.
					long nNewScope = VarLong(varNewValue);
					long nComponentID = VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 4));
					long nTemplateID = VarLong(m_pComponentList->GetValue(m_pComponentList->CurSel, 1));
					ExecuteSql("UPDATE PacketComponentsT SET Scope = %li WHERE ID = %li", nNewScope, nComponentID);
					ExecuteSql("UPDATE MergeTemplatesT SET DefaultScope = %li WHERE ID = %li", nNewScope, nTemplateID);
				}
				break;
			default:
				break;
			}
		}
	}NxCatchAll("Error in CEditPacketDlg::OnEditingFinishedComponentList()");
}

void CEditPacketDlg::OnClose() 
{
	CNxDialog::OnClose();
}

void CEditPacketDlg::OnCancel() 
{	
	CNxDialog::OnCancel();
}
