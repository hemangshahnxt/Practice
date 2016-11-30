// EMRCodeEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EMRCodeEditorDlg.h"
#include "EMRTableCellCodes.h"
#include "UTSSEarchDlg.h"


#define ID_REMOVE_CODE        52345

enum CodeListColumn
{
	clcID = 0,
	clcVocab,
	clcCode,
	clcName,
};


using namespace NXDATALIST2Lib;




// CEMRCodeEditorDlg dialog

// (j.gruber 2013-10-01 13:41) - PLID 58675 - created for

IMPLEMENT_DYNAMIC(CEMRCodeEditorDlg, CNxDialog)

CEMRCodeEditorDlg::CEMRCodeEditorDlg(CEMRCodeArray *pAry, CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRCodeEditorDlg::IDD, pParent)
{
	m_paryCodes = pAry;
}

CEMRCodeEditorDlg::~CEMRCodeEditorDlg()
{
}

void CEMRCodeEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CEMRCodeEditorDlg, CNxDialog)
	ON_BN_CLICKED(IDC_OPEN_UMLS, &CEMRCodeEditorDlg::OnBnClickedOpenUmls)
	ON_BN_CLICKED(IDOK, &CEMRCodeEditorDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CEMRCodeEditorDlg message handlers
BEGIN_EVENTSINK_MAP(CEMRCodeEditorDlg, CNxDialog)
	ON_EVENT(CEMRCodeEditorDlg, IDC_CODE_SELECTED_LIST, 1, CEMRCodeEditorDlg::SelChangingCodeSelectedList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEMRCodeEditorDlg, IDC_CODE_SELECTED_LIST, 16, CEMRCodeEditorDlg::SelChosenCodeSelectedList, VTS_DISPATCH)
	ON_EVENT(CEMRCodeEditorDlg, IDC_CODE_LIST, 7, CEMRCodeEditorDlg::RButtonUpCodeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


BOOL CEMRCodeEditorDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
	
		m_pCodeList = BindNxDataList2Ctrl(IDC_CODE_LIST, GetRemoteData(), false);
		m_pSelectedList = BindNxDataList2Ctrl(IDC_CODE_SELECTED_LIST, GetRemoteData(), true);

		LoadCodeList();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEMRCodeEditorDlg::LoadCodeList()
{	
	//just loop through our array and add to the list
	for (int i = 0; i < m_paryCodes->GetSize(); i++) {
		
		CEMRCode code = m_paryCodes->GetAt(i);

		IRowSettingsPtr pRow = m_pCodeList->GetNewRow();
		if (pRow) 
		{
			pRow->PutValue(clcID, code.GetID());
			pRow->PutValue(clcVocab, _variant_t(code.GetVocab()));
			pRow->PutValue(clcCode, _variant_t(code.GetCode()));
			pRow->PutValue(clcName, _variant_t(code.GetName()));
			
			m_pCodeList->AddRowAtEnd(pRow, NULL);
		}
	}


}

void CEMRCodeEditorDlg::SelChangingCodeSelectedList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	// TODO: Add your message handler code here
}

void CEMRCodeEditorDlg::SelChosenCodeSelectedList(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);

		if (pRow)
		{
			//make sure the code isn't already in our list
			IRowSettingsPtr pRowFound = m_pCodeList->FindByColumn(clcID, pRow->GetValue(clcID), NULL, g_cvarFalse);
			if (pRowFound)
			{
				MsgBox("This code is already selected");
				return;
			}

			IRowSettingsPtr pNewCodeRow = m_pCodeList->GetNewRow();
			if (pNewCodeRow) {
				pNewCodeRow->PutValue(clcID, pRow->GetValue(clcID));
				pNewCodeRow->PutValue(clcCode, pRow->GetValue(clcCode));
				pNewCodeRow->PutValue(clcVocab, pRow->GetValue(clcVocab));
				pNewCodeRow->PutValue(clcName, pRow->GetValue(clcName));
					
				m_pCodeList->AddRowSorted(pNewCodeRow, NULL);
			}

		}

		//now clear the selection
		m_pSelectedList->CurSel = NULL;
	}NxCatchAll(__FUNCTION__);
}

void CEMRCodeEditorDlg::RButtonUpCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		CMenu Popup;
		Popup.m_hMenu = CreatePopupMenu();		
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {		
			m_pCodeList->CurSel = pRow;
			Popup.InsertMenu(3, MF_BYPOSITION, ID_REMOVE_CODE, "Delete Code");
		}		

		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* pwnd = GetDlgItem(IDC_CODE_LIST);
		if (pwnd != NULL) {
			pwnd->ClientToScreen(&pt);
			Popup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CEMRCodeEditorDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (wParam) {
		case ID_REMOVE_CODE:			
			try{
				IRowSettingsPtr pRow = m_pCodeList->CurSel;
				if (pRow) {
					if (IDYES == MsgBox(MB_YESNO, "Deleting this code will make it no longer exportable in the CCDA and possibly other places.  This may cause these exports not to function correctly.  Are you sure you wish to continue?"))
					{
						//remove it
						m_pCodeList->RemoveRow(pRow);
					}
				}
			}NxCatchAll(" Error in CEMRCodeEditorDlg::OnCommand - Cannot Remove Code");
		break;

		return TRUE;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}


void CEMRCodeEditorDlg::OnBnClickedOpenUmls()
{
	try{
		CUTSSearchDlg dlg;
		int nResult = dlg.DoModal();

		if (nResult == IDOK)
		{
			//requery our code list
			m_pSelectedList->Requery();
		}
	}NxCatchAll(__FUNCTION__);
}

void CEMRCodeEditorDlg::OnBnClickedOk()
{
	try {

		//we have to save our list

		//first clear the list
		m_paryCodes->RemoveAll();

		IRowSettingsPtr pRow = m_pCodeList->GetFirstRow();
		while (pRow) 
		{
			long nID = VarLong(pRow->GetValue(clcID));
			CString strVocab = VarString(pRow->GetValue(clcVocab));
			CString strCode = VarString(pRow->GetValue(clcCode));
			CString strName = VarString(pRow->GetValue(clcName));

			CEMRCode code(nID, strVocab, strCode, strName);
			m_paryCodes->Add(code);

			pRow = pRow->GetNextRow();
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}