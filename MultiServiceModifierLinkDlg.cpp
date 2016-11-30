// MultiServiceModifierLinkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiServiceModifierLinkDlg.h"
#include "GlobalUtils.h"

// (j.jones 2007-07-03 12:54) - PLID 26098 - created

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum MultiServiceModifierServiceList {

	msmslID	= 0,
	msmslCode = 1,
	msmslSub = 2,
	msmslDesc = 3,
};

enum MultiServiceModifierLinkedList {

	msmllID	= 0,
	msmllCode = 1,
	msmllSub = 2,
	msmllDesc = 3,
};

/////////////////////////////////////////////////////////////////////////////
// CMultiServiceModifierLinkDlg dialog


CMultiServiceModifierLinkDlg::CMultiServiceModifierLinkDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMultiServiceModifierLinkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMultiServiceModifierLinkDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMultiServiceModifierLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiServiceModifierLinkDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiServiceModifierLinkDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMultiServiceModifierLinkDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiServiceModifierLinkDlg message handlers

BOOL CMultiServiceModifierLinkDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		// (z.manning, 05/01/2008) - PLID 29864 - Set button styles
		m_btnClose.AutoSet(NXB_CLOSE);

		m_ModifierCombo = BindNxDataList2Ctrl(this, IDC_MODIFIER_COMBO, GetRemoteData(), true);
		m_ServiceCombo = BindNxDataList2Ctrl(this, IDC_SERVICE_COMBO, GetRemoteData(), true);
		m_LinkedList = BindNxDataList2Ctrl(this, IDC_SERVICE_LIST, GetRemoteData(), false);

	}NxCatchAll("Error in CMultiServiceModifierLinkDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMultiServiceModifierLinkDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMultiServiceModifierLinkDlg)
	ON_EVENT(CMultiServiceModifierLinkDlg, IDC_MODIFIER_COMBO, 16 /* SelChosen */, OnSelChosenModifierCombo, VTS_DISPATCH)
	ON_EVENT(CMultiServiceModifierLinkDlg, IDC_SERVICE_LIST, 6 /* RButtonDown */, OnRButtonDownServiceList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiServiceModifierLinkDlg, IDC_SERVICE_COMBO, 16 /* SelChosen */, OnSelChosenServiceCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMultiServiceModifierLinkDlg::OnSelChosenModifierCombo(LPDISPATCH lpRow) 
{
	try {

		//requery the service list with the new modifier

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			m_LinkedList->Clear();
			return;
		}

		CString strModifier = VarString(pRow->GetValue(0),"");

		CString strWhere;
		strWhere.Format("MultiServiceModifierLinkT.Modifier = '%s'", _Q(strModifier));
		m_LinkedList->PutWhereClause(_bstr_t(strWhere));
		m_LinkedList->Requery();

	}NxCatchAll("Error in CMultiServiceModifierLinkDlg::OnSelChosenModifierCombo");
}

void CMultiServiceModifierLinkDlg::OnRButtonDownServiceList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			m_LinkedList->PutCurSel(pRow);

			enum {
				eRemoveLink = -1,
			};

			CMenu mnu;
			mnu.CreatePopupMenu();
			mnu.InsertMenu(0, MF_BYPOSITION, eRemoveLink, "Remove Link To Modifier");
			CPoint pt;
			GetCursorPos(&pt);
			long nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, this);

			if (nRet == eRemoveLink) {
				//remove the link between this service and the current modifier
				long nServiceID = VarLong(pRow->GetValue(msmllID));
				ExecuteSql("DELETE FROM MultiServiceModifierLinkT WHERE ServiceID = %li", nServiceID);

				//and remove from the list
				m_LinkedList->RemoveRow(pRow);

				//send a modifier tablechecker
				CClient::RefreshTable(NetUtils::CPTModifierT);
			}
		}

	}NxCatchAll("Error in CMultiServiceModifierLinkDlg::OnRButtonDownServiceList");
}

void CMultiServiceModifierLinkDlg::OnSelChosenServiceCombo(LPDISPATCH lpRow) 
{
	try {

		//add to our list

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		IRowSettingsPtr pCurModifierRow = m_ModifierCombo->GetCurSel();

		if(pCurModifierRow == NULL) {
			AfxMessageBox("You must first select a modifier with which to link this service code.");
			return;
		}

		long nServiceID = VarLong(pRow->GetValue(msmslID),-1);
		CString strCode = VarString(pRow->GetValue(msmslCode),"");
		CString strSub = VarString(pRow->GetValue(msmslSub),"");;
		CString strDesc = VarString(pRow->GetValue(msmslDesc),"");

		CString strCurModifier = VarString(pCurModifierRow->GetValue(0),"");

		//is it already linked to a modifier?
		_RecordsetPtr rs = CreateRecordset("SELECT Modifier FROM MultiServiceModifierLinkT WHERE ServiceID = %li", nServiceID);
		if(!rs->eof) {

			CString strModifier = AdoFldString(rs, "Modifier","");

			if(strModifier == strCurModifier) {
				AfxMessageBox("This service code is already linked to the selected modifier.");
				//clear the selection
				m_ServiceCombo->PutCurSel(NULL);
				return;
			}
			else {
				CString str;
				str.Format("This service code is already linked to modifier %s.\n"
					"It must first have that link removed before it can link to a new modifier.\n\n"
					"Would you like to remove the link to modifier %s and instead link this\n"
					"service to modifier %s?", strModifier, strModifier, strCurModifier);
				if(IDNO == MessageBox(str, "Practice", MB_ICONQUESTION|MB_YESNO)) {
					//clear the selection
					m_ServiceCombo->PutCurSel(NULL);
					return;
				}
				else {
					//unlink from the existing modifier
					ExecuteSql("DELETE FROM MultiServiceModifierLinkT WHERE ServiceID = %li", nServiceID);

					//send a modifier tablechecker
					CClient::RefreshTable(NetUtils::CPTModifierT);
				}
			}
		}
		rs->Close();

		//now create the link
		ExecuteSql("INSERT INTO MultiServiceModifierLinkT (ServiceID, Modifier) VALUES (%li, '%s')", nServiceID, _Q(strCurModifier));

		//and add to our list
		IRowSettingsPtr pNewRow = m_LinkedList->GetNewRow();
		pNewRow->PutValue(msmllID, (long)nServiceID);
		pNewRow->PutValue(msmllCode, _bstr_t(strCode));
		pNewRow->PutValue(msmllSub, _bstr_t(strSub));
		pNewRow->PutValue(msmllDesc, _bstr_t(strDesc));
		m_LinkedList->AddRowSorted(pRow, NULL);

		//once we've added to the linked list, clear out the selection
		m_ServiceCombo->PutCurSel(NULL);

		//send a modifier tablechecker
		CClient::RefreshTable(NetUtils::CPTModifierT);

	}NxCatchAll("Error in CMultiServiceModifierLinkDlg::OnSelChosenServiceCombo");
}
