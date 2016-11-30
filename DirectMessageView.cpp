// DirectMessageView.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "DirectMessageView.h"


// CDirectMessageView dialog

IMPLEMENT_DYNAMIC(CDirectMessageView, CNxDialog)

CDirectMessageView::CDirectMessageView(CWnd* pParent /*=NULL*/)
	: CNxDialog(CDirectMessageView::IDD, pParent)
{

}

CDirectMessageView::~CDirectMessageView()
{
}

void CDirectMessageView::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDirectMessageView, CNxDialog)
END_MESSAGE_MAP()

void CDirectMessageView::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();
		
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CDirectMessageView::OnOK()
{
	try{
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

// CDirectMessageView message handlers
