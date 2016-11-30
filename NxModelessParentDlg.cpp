// NxModelessParentDlg.cpp : implementation file
// (a.wilson 2013-01-09 14:00) - PLID 54535 - created

#include "stdafx.h"
#include "Practice.h"
#include "NxModelessParentDlg.h"


// CNxModelessParentDlg dialog

IMPLEMENT_DYNAMIC(CNxModelessParentDlg, CNxDialog)

// (a.walling 2015-01-12 10:28) - PLID 64558 - Rescheduling Queue - handle size and pos memory, custom styles
CNxModelessParentDlg::CNxModelessParentDlg(CWnd* pParent, const CString& strSizeAndPositionConfigRT,
	ParentDialogButtonOptions pdbs /* = bsNone */, DWORD styleRemove /* = -1*/, DWORD styleAdd /*= -1*/)
	: CNxDialog(CNxModelessParentDlg::IDD, pParent, strSizeAndPositionConfigRT)
	, m_styleRemove(styleRemove)
	, m_styleAdd(styleAdd)
{
	m_pdbs = pdbs;
	m_nButtonBuffer = 0;
	m_nChildIDD = 0;
}

CNxModelessParentDlg::~CNxModelessParentDlg() {}

void CNxModelessParentDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CNxModelessParentDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNxModelessParentDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNxModelessParentDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
END_MESSAGE_MAP()

//creates the child within the parent dialog.
void CNxModelessParentDlg::CreateWithChildDialog(std::unique_ptr<CNxDialog> pChild, int IDD_CHILD, int IDR_ICON, const CString & strTitle, CWnd* pParent)
{
	//set new child.
	m_nChildIDD = IDD_CHILD;
	m_pChild = std::move(pChild);
	Create(IDD_NX_MODELESS_PARENT_DLG, pParent);

	//set the dialog title
	SetTitleBarIcon(IDR_ICON);
	SetWindowText(strTitle);
}

BOOL CNxModelessParentDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		if (m_styleAdd || m_styleRemove) {
			ModifyStyle(m_styleRemove, m_styleAdd);
		}

		//create new child
		m_pChild->Create(m_nChildIDD, this);

		//display the appropriate buttons based on pdbs
		switch (m_pdbs)
		{
			case bsNone:
				m_btnOK.ShowWindow(SW_HIDE);
				m_btnCancel.ShowWindow(SW_HIDE);
				m_nButtonBuffer = 0;
				break;
			case bsOk:	//disable cancel button and move ok button to the center.
				m_btnCancel.ShowWindow(SW_HIDE);
			case bsOkCancel:
				m_nButtonBuffer = 50;
				break;
		}

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		ResizeContents();
		CenterWindow();
		m_pChild->ShowWindow(SW_SHOW);
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CNxModelessParentDlg::OnBnClickedOk()
{
	if (m_pChild && m_pChild->GetSafeHwnd())
		m_pChild->OnOK();	

	CNxDialog::OnOK();
}

void CNxModelessParentDlg::OnBnClickedCancel()
{
	if (m_pChild && m_pChild->GetSafeHwnd())
		m_pChild->OnCancel();

	CNxDialog::OnCancel();
}

void CNxModelessParentDlg::OnSize(UINT nType, int cx, int cy)
{
	CNxDialog::OnSize(nType, cx, cy);
	
	try {
		ResizeContents();

	} NxCatchAll(__FUNCTION__);
}

void CNxModelessParentDlg::ResizeContents()
{
	//if the child is active, resize its contents and parent buttons to fit the parent dialog.
	if (!(m_pChild && m_pChild->GetSafeHwnd())) {
		return;
	}
	CRect rcParent;
	GetClientRect(&rcParent);
	//resituate the ok button for this case.
	if (m_pdbs == bsOk) {
		GetDlgItem(IDOK)->MoveWindow(CRect(((rcParent.right / 2) - 100), (rcParent.bottom - 44), 
			((rcParent.right / 2) + 100), rcParent.bottom - 6), TRUE);
	} else if (bsOkCancel) {
		GetDlgItem(IDOK)->MoveWindow(CRect(((rcParent.right / 2) - 140), (rcParent.bottom - 44), 
			((rcParent.right / 2) -20), rcParent.bottom - 6), TRUE);
		GetDlgItem(IDCANCEL)->MoveWindow(CRect(((rcParent.right / 2) + 20), (rcParent.bottom - 44), 
			((rcParent.right / 2) + 140), rcParent.bottom - 6), TRUE);
	}
	rcParent.DeflateRect(0, 0, 0, m_nButtonBuffer);
	m_pChild->MoveWindow(rcParent, TRUE);
	m_pChild->SetControlPositions();
}

// (j.fouts 2013-04-22 16:03) - PLID 54719 - Send Hide/Show messages to child
void CNxModelessParentDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	try
	{
		m_pChild->ShowWindow(bShow? SW_SHOW : SW_HIDE);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-04-22 16:03) - PLID 54719 - Send Hide/Show messages to child
void CNxModelessParentDlg::OnClose()
{
	try
	{
		m_pChild->ShowWindow(SW_HIDE);
	}
	NxCatchAll(__FUNCTION__);

	CNxDialog::OnClose();
}