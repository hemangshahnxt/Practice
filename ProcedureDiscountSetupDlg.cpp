// ProcedureDiscountSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProcedureDiscountSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CProcedureDiscountSetupDlg dialog


CProcedureDiscountSetupDlg::CProcedureDiscountSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CProcedureDiscountSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcedureDiscountSetupDlg)
		m_ProcedureID = -1;
		m_strProcedureName = "";
	//}}AFX_DATA_INIT
}


void CProcedureDiscountSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureDiscountSetupDlg)
	DDX_Control(pDX, IDC_BTN_ADD_DISCOUNT, m_btnAddDiscount);
	DDX_Control(pDX, IDC_BTN_DELETE_DISCOUNT, m_btnDeleteDiscount);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcedureDiscountSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProcedureDiscountSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_DISCOUNT, OnBtnAddDiscount)
	ON_BN_CLICKED(IDC_BTN_DELETE_DISCOUNT, OnBtnDeleteDiscount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcedureDiscountSetupDlg message handlers

BOOL CProcedureDiscountSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnAddDiscount.AutoSet(NXB_NEW);
	m_btnDeleteDiscount.AutoSet(NXB_DELETE);
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// (j.jones 2005-11-07 11:49) - PLID 16034 - check to see if this is a detail procedure
	_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM ProcedureT WHERE ID IN (SELECT MasterProcedureID FROM ProcedureT WHERE ID = %li)", m_ProcedureID);
	if(!rs->eof) {
		//this is indeed a detail procedure, so instead load this configuration for the master
		m_ProcedureID = AdoFldLong(rs, "ID");
		m_strProcedureName = AdoFldString(rs, "Name");
	}
	rs->Close();

	if(!m_strProcedureName.IsEmpty())
		SetWindowText("Recurring Procedure Discount Setup (" + m_strProcedureName + ")");
	
	m_List = BindNxDataListCtrl(this,IDC_DISCOUNT_LIST,GetRemoteData(),false);

	CString str;
	str.Format("ProcedureID = %li",m_ProcedureID);
	m_List->WhereClause = _bstr_t(str);
	m_List->Requery();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcedureDiscountSetupDlg::OnOK() 
{
	try {

		//first delete
		for(int i=0;i<m_dwaryOccurrencesToDelete.GetSize();i++) {
			
			ExecuteSql("DELETE FROM ProcedureDiscountsT WHERE ProcedureID = %li AND Occurrence = %li",m_ProcedureID,m_dwaryOccurrencesToDelete.GetAt(i));
		}

		//now add
		for(i=0;i<m_dwaryOccurrencesToAdd.GetSize();i++) {

			//using the Occurrence value, find the discount in the datalist
			long nOccurrence = m_dwaryOccurrencesToAdd.GetAt(i);
			long nPercentOff = 0;
			for(int j=0;j<m_List->GetRowCount();j++) {				
				if(VarLong(m_List->GetValue(j,0)) == nOccurrence) {
					nPercentOff = VarLong(m_List->GetValue(j,2),0);
				}
			}
			
			ExecuteSql("INSERT INTO ProcedureDiscountsT (ProcedureID, Occurrence, PercentOff) VALUES (%li, %li, %li)",m_ProcedureID,nOccurrence,nPercentOff);
		}

		//and update remaining percentages (which, yes, duplicates some of the above changes. so sue me.)
		for(i=0;i<m_List->GetRowCount();i++) {				
			long nOccurrence = VarLong(m_List->GetValue(i,0));
			long nPercentOff = VarLong(m_List->GetValue(i,2),0);
			ExecuteSql("UPDATE ProcedureDiscountsT SET PercentOff = %li WHERE ProcedureID = %li AND Occurrence = %li",nPercentOff,m_ProcedureID,nOccurrence);
		}
	
		CDialog::OnOK();

	}NxCatchAll("Error saving discount changes.");
}

void CProcedureDiscountSetupDlg::OnBtnAddDiscount() 
{
	try {

		CString strCount;
		strCount.Format("%li", 1);
		long nOccurrence = 0;
		if(InputBox(this, "Which occurrence of a bill would you like to apply this discount to?", strCount, "", false, false, NULL, TRUE) == IDOK) {
			nOccurrence = atol(strCount);
			
			//do not allow zero
			if(nOccurrence == 0) {
				AfxMessageBox("You must enter an occurrence greater than zero.");
				return;
			}

			//do not allow duplicates
			for(int i=0;i<m_List->GetRowCount();i++) {				
				if(VarLong(m_List->GetValue(i,0)) == nOccurrence) {
					AfxMessageBox("That occurrence has already been entered.");
					return;
				}
			}

			//warn if 1
			if(nOccurrence == 1) {
				if(IDNO == MessageBox("You entered 1, which means you want to automatically discount the\n"
					"first occurrence of a bill for this procedure.\n\n"
					"Are you sure you want to do this?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
					return;
				}
			}			
		}

		//see if it's already in the added list - which really shouldn't be possible
		BOOL bFound = FALSE;
		for(int i=0;i<m_dwaryOccurrencesToAdd.GetSize();i++) {
			if((long)m_dwaryOccurrencesToAdd.GetAt(i) == nOccurrence)
				bFound = TRUE;
		}

		if(!bFound) {
			
			CString strPercent;
			strPercent.Format("0");
			if(InputBox(this, "Enter the percent discount for this occurrence", strPercent, "", false, false, NULL, TRUE) == IDOK) {
				long nPercentOff = atol(strPercent);

				//allow zero, because perhaps they want to allow 5% off on the 5th occurrence but not afterwards
				if(nPercentOff == 0) {
					if(IDNO == MessageBox("You entered 0 for your percent off, which means that no discount will be made on this occurrence.\n"
						"Are you sure you want to do this?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
						return;
					}
				}

				if(nPercentOff > 100) {
					AfxMessageBox("You may not enter in a discount greater than 100%.");
					return;
				}

				m_dwaryOccurrencesToAdd.Add(nOccurrence);

				//determine the text to display
				CString str;
				str.Format("%li",nOccurrence);
				if(nOccurrence % 10 == 1 && nOccurrence != 11)
					str += "st";
				else if(nOccurrence % 10 == 2 && nOccurrence != 12)
					str += "nd";
				else if(nOccurrence % 10 == 3 && nOccurrence != 13)
					str += "rd";
				else
					str += "th";
				str += " Bill";

				//add to the list
				IRowSettingsPtr pRow = m_List->GetRow(-1);
				pRow->PutValue(0,(long)nOccurrence);
				pRow->PutValue(1,_bstr_t(str));
				pRow->PutValue(2,(long)nPercentOff);
				m_List->CurSel = m_List->AddRow(pRow);
			}
		}
		else {
			//since this shouldn't be possible, assert
			ASSERT(FALSE);
		}

	}NxCatchAll("Error adding new discount.");
}

void CProcedureDiscountSetupDlg::OnBtnDeleteDiscount() 
{
	try {

		if(m_List->CurSel == -1) {
			AfxMessageBox("Please select a discount occurrence before deleting.");
			return;
		}

		if(IDNO == MessageBox("Are you sure you wish to delete this discount occurrence?","Practice",MB_YESNO|MB_ICONQUESTION)) {
			return;
		}

		long nOccurrence = VarLong(m_List->GetValue(m_List->CurSel,0));

		//see if it's already in the deleted list, if so, it means they deleted one,
		//created a new one, and deleted that too, so who cares?
		BOOL bFound = FALSE;
		for(int i=0;i<m_dwaryOccurrencesToDelete.GetSize();i++) {
			if((long)m_dwaryOccurrencesToDelete.GetAt(i) == nOccurrence)
				bFound = TRUE;
		}

		if(!bFound)
			m_dwaryOccurrencesToDelete.Add(nOccurrence);

		//now see if it's already in the added list, if so, it means they intended to add it,
		//but then chose to delete it, so let's make sure we don't add it!
		for(i=m_dwaryOccurrencesToAdd.GetSize()-1;i>=0;i--) {
			if((long)m_dwaryOccurrencesToAdd.GetAt(i) == nOccurrence) {
				m_dwaryOccurrencesToAdd.RemoveAt(i);
			}
		}

		m_List->RemoveRow(m_List->CurSel);

	}NxCatchAll("Error deleting discount.");
}

BEGIN_EVENTSINK_MAP(CProcedureDiscountSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProcedureDiscountSetupDlg)
	ON_EVENT(CProcedureDiscountSetupDlg, IDC_DISCOUNT_LIST, 6 /* RButtonDown */, OnRButtonDownDiscountList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CProcedureDiscountSetupDlg, IDC_DISCOUNT_LIST, 9 /* EditingFinishing */, OnEditingFinishingDiscountList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProcedureDiscountSetupDlg::OnRButtonDownDiscountList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_List->CurSel = nRow;

	if(nRow == -1)
		return;

	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();
	pMenu->InsertMenu(-1, MF_BYPOSITION, IDC_BTN_DELETE_DISCOUNT, "Delete Discount Occurrence");

	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}

void CProcedureDiscountSetupDlg::OnEditingFinishingDiscountList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol){
	case 2:
		if(pvarNewValue->vt == VT_I4 && pvarNewValue->lVal < 0) {
			AfxMessageBox("You cannot enter a percent off that is less than 0.");
			*pbCommit = FALSE;
		}
		if(pvarNewValue->vt == VT_I4 && pvarNewValue->lVal > 100) {
			AfxMessageBox("You cannot enter a percent off that is greater than 100.");
			*pbCommit = FALSE;
		}
		break;
	}
}
