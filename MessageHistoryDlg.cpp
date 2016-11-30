// MessageHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "MessageHistoryDlg.h"

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CMessageHistoryDlg dialog


CMessageHistoryDlg::CMessageHistoryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMessageHistoryDlg::IDD, pParent)
	, m_nOtherUserID(-1) // (a.walling 2010-11-16 16:37) - PLID 41510 - Initialize this value
{
	//{{AFX_DATA_INIT(CMessageHistoryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMessageHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageHistoryDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMessageHistoryDlg)
	ON_COMMAND(ID_GOTO_MESSAGE, OnGotoMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageHistoryDlg message handlers

BOOL CMessageHistoryDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnClose.AutoSet(NXB_CLOSE);
	
	m_pMessages = BindNxDataListCtrl(this, IDC_MESSAGE_LIST, GetRemoteData(), false);

	// (a.walling 2010-11-16 16:37) - PLID 41510 - Don't load until the OtherUserID is set. It's an uninitialized value anyway!
	//Load();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CMessageHistoryDlg::DoModal() 
{
	return CDialog::DoModal();
}	

BEGIN_EVENTSINK_MAP(CMessageHistoryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMessageHistoryDlg)
	ON_EVENT(CMessageHistoryDlg, IDC_MESSAGE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedMessageList, VTS_I2)
	ON_EVENT(CMessageHistoryDlg, IDC_MESSAGE_LIST, 3 /* DblClickCell */, OnDblClickCellMessageList, VTS_I4 VTS_I2)
	ON_EVENT(CMessageHistoryDlg, IDC_MESSAGE_LIST, 7 /* RButtonUp */, OnRButtonUpMessageList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMessageHistoryDlg::OnRequeryFinishedMessageList(short nFlags) 
{
	//MSC - Ideally we would want to stop the redraw before the requery, but
	//that causes other drawing problems, so this is quite acceptable
	m_pMessages->SetRedraw(FALSE);

	if(m_pMessages->GetRowCount() > 0) 
		SetMessageColors();

	m_pMessages->SetRedraw(TRUE);	
}

void CMessageHistoryDlg::SetMessageColors()
{
	long pCurRowEnum = m_pMessages->GetFirstRowEnum();
	while(pCurRowEnum != 0){
		IRowSettingsPtr pRow;
		{
			IDispatch *lpDisp;
			m_pMessages->GetNextRowEnum(&pCurRowEnum, &lpDisp);
			pRow = lpDisp;
			lpDisp->Release();
			lpDisp = NULL;
		}

		ASSERT(pRow != NULL);
		_variant_t var = pRow->GetValue(1);

		long nCurRowSender = VarLong(var);
		OLE_COLOR color;
		if(nCurRowSender == GetCurrentUserID()){
			color = RGB( 255, 0, 0);
		}
		else{
			color = RGB(0,0,255);
		}
		pRow->PutForeColor(color);
	}
}

void CMessageHistoryDlg::SetOtherUserID(const IN long nOtherUserID)
{
	m_nOtherUserID = nOtherUserID;
	Load();
}

void CMessageHistoryDlg::Load()
{
	ASSERT(m_nOtherUserID != -1); // (a.walling 2010-11-16 16:37) - PLID 41510 - ASSERT for good measure

	CString strWhere;
	strWhere.Format("(MessagesT.RecipientID = %li AND MessagesT.SenderID = %li AND MessagesT.DeletedBySender = 0) "
		"OR (MessagesT.SenderID = %li AND MessagesT.RecipientID = %li AND MessagesT.DeletedByRecipient = 0)", m_nOtherUserID, GetCurrentUserID(), m_nOtherUserID, GetCurrentUserID());
	m_pMessages->WhereClause = (LPCTSTR)strWhere;
	m_pMessages->Requery();

	_RecordsetPtr rsOtherUser = CreateRecordset("SELECT Username FROM UsersT WHERE PersonID = %li", m_nOtherUserID);
	if(!rsOtherUser->eof){
		m_strOtherUserName = AdoFldString(rsOtherUser, "UserName");
		CString strDialogCaption;
		strDialogCaption.Format("Message History with %s", m_strOtherUserName);
		this->SetWindowText(strDialogCaption);
	}
}



void CMessageHistoryDlg::OnDblClickCellMessageList(long nRowIndex, short nColIndex) 
{
	OnGotoMessage();	
}

void CMessageHistoryDlg::OnRButtonUpMessageList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1) return;

	//Don't show this if this is the "Offline" row.
	if(VarLong(m_pMessages->GetValue(nRow, 0)) == -1)
		return;

	m_pMessages->CurSel = nRow;
	
	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();

	pMenu->InsertMenu(0, MF_BYPOSITION, ID_GOTO_MESSAGE, "Go to Message");
		
	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}

void CMessageHistoryDlg::OnGotoMessage()
{
	if(m_pMessages->CurSel != -1) {
		if(m_pMainYakDlg) {
			m_pMainYakDlg->PostMessage(NXM_GOTO_MESSAGE, VarLong(m_pMessages->GetValue(m_pMessages->CurSel, 0)));
		}
		CDialog::OnOK();
	}
}