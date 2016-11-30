// DiagQuickListCopyFromUserDlg.cpp : implementation file
// (c.haag 2014-02-24) - PLID 60950 - Initial implementation. From here, users can choose which
// other users' QuickLists to copy items from.

#include "stdafx.h"
#include "Practice.h"
#include "DiagQuickListCopyFromUserDlg.h"
#include "DiagQuickListUtils.h"
#include "NxAPI.h"

using namespace NXDATALIST2Lib;

enum UserListColumns
{
	ulcID = 0,
	ulcChecked,
	ulcUsername,
};

// CDiagQuickListCopyFromUserDlg dialog

IMPLEMENT_DYNAMIC(CDiagQuickListCopyFromUserDlg, CNxDialog)

CDiagQuickListCopyFromUserDlg::CDiagQuickListCopyFromUserDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDiagQuickListCopyFromUserDlg::IDD, pParent)
{

}

CDiagQuickListCopyFromUserDlg::~CDiagQuickListCopyFromUserDlg()
{
}

void CDiagQuickListCopyFromUserDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CDiagQuickListCopyFromUserDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CDiagQuickListCopyFromUserDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CDiagQuickListCopyFromUserDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_UserList = GetDlgItemUnknown(IDC_QUICKLIST_USERLIST);
		
		// Populate the user list with other users who have QuickLists
		NexTech_Accessor::_DiagQuickListPtr quickList = DiagQuickListUtils::GetDiagQuickListForCurrentUser();
		Nx::SafeArray<IUnknown *> saryUsersWithQuickLists(quickList->UsersWithQuickLists);
		m_UserList->Clear();
		foreach(NexTech_Accessor::_UserPtr pUser, saryUsersWithQuickLists)
		{
			IRowSettingsPtr pRow = m_UserList->GetNewRow();
			pRow->Value[ulcID] = pUser->ID;
			pRow->Value[ulcChecked] = g_cvarFalse;
			pRow->Value[ulcUsername] = pUser->username;
			m_UserList->AddRowAtEnd(pRow, NULL);
		}
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
}

// CDiagQuickListCopyFromUserDlg message handlers

void CDiagQuickListCopyFromUserDlg::OnBnClickedOk()
{
	try
	{
		for (IRowSettingsPtr pRow = m_UserList->GetFirstRow(); NULL != pRow; pRow = pRow->GetNextRow())
		{
			if (VarBool(pRow->Value[ulcChecked]))
			{
				m_astrSelectedUserIDs.Add(VarString(pRow->Value[ulcID]));
			}
		}

		if (0 == m_astrSelectedUserIDs.GetCount())
		{
			AfxMessageBox("Please select at least one user to copy QuickList items from.");
		}
		else
		{
			__super::OnOK();
		}
	}
	NxCatchAll(__FUNCTION__);
}
