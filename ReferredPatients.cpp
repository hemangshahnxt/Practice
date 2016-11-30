// ReferredPatients.cpp : implementation file
//

#include "stdafx.h"
#include "ReferredPatients.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define	IDM_GO_TO_PATIENT		50000

enum PatientListColumns {
	plcUserID = 0, 
	plcName = 1,
	plcID = 2,
};

/////////////////////////////////////////////////////////////////////////////
// CReferredPatients dialog


CReferredPatients::CReferredPatients(CWnd* pParent)
	: CNxDialog(CReferredPatients::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReferredPatients)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nPersonID = -25;
	m_bRefPhys = false;
}


void CReferredPatients::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReferredPatients)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_TYPE_TEXT, m_nxstaticTypeText);
	DDX_Control(pDX, IDC_NAME_LABEL, m_nxstaticNameLabel);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReferredPatients, CNxDialog)
	//{{AFX_MSG_MAP(CReferredPatients)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(IDM_GO_TO_PATIENT, GoToPatient)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReferredPatients message handlers

BOOL CReferredPatients::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 12:28) - PLID 29790 - NxIconified buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		
		//fill in the datalist
		m_pPatients = BindNxDataListCtrl(IDC_PATIENTS,false);

		// (m.hancock 2006-08-02 16:04) - PLID 21752 - If we're looking for referrals from a ref. physician,
		// change the query field to DefaultReferringPhyID.
		CString strField;
		if(m_bRefPhys)
			strField = "DefaultReferringPhyID";
		else //Otherwise look for referrals for a patient using ReferringPatientID field
			strField = "ReferringPatientID";
		CString temp;
		// (d.moore 2007-05-02 13:43) - PLID 23602 - PatientsT.CurrentStatus = 3 was previously always added 
		//  to the query. It should only be used when displaying Patients.
		if (m_nStatus == 1) {
			temp.Format("%s = %li AND (PatientsT.CurrentStatus = 1 OR PatientsT.CurrentStatus = 3)", strField, m_nPersonID);
		} else {
			temp.Format("%s = %li AND PatientsT.CurrentStatus = %li", strField, m_nPersonID, m_nStatus);
		}

		m_pPatients->WhereClause = _bstr_t(temp);
		m_pPatients->Requery();

		//setup the text field
		// (m.hancock 2006-08-02 16:06) - PLID 21752 - Changed the query so it always uses PersonID instead of PatientID so it could be adapted for use with referring physicians
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PersonName FROM PersonT WHERE ID = %li", m_nPersonID);
		if(!rs->eof) {
			_variant_t var = rs->Fields->Item["PersonName"]->Value;
			if(var.vt == VT_BSTR)
				SetDlgItemText(IDC_NAME_LABEL, CString(var.bstrVal));
			else
				SetDlgItemText(IDC_NAME_LABEL, "");
		}
		rs->Close();

		if(m_nStatus == 2) {
			SetDlgItemText(IDC_TYPE_TEXT, "All prospects referred by:");
			SetWindowText("Referred Prospects");
		}
	}
	NxCatchAll("Error in CReferredPatients::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReferredPatients::OnOK() 
{
	
	CDialog::OnOK();
}

void CReferredPatients::OnCancel()
{
	CDialog::OnCancel();
}

void CReferredPatients::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if(m_pPatients->GetCurSel() == -1)
		return;

	CMenu mnu;
	mnu.m_hMenu = CreatePopupMenu();

	if(m_nStatus == 2)
		mnu.InsertMenu(0, MF_BYPOSITION, IDM_GO_TO_PATIENT, "&Go To Prospect");
	else 
		mnu.InsertMenu(0, MF_BYPOSITION, IDM_GO_TO_PATIENT, "&Go To Patient");

	CPoint pt;
	GetCursorPos(&pt);
	mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);

}

void CReferredPatients::GoToPatient()
{
	//Copied this out of the ResEntry::OnGoToPatient, and made minor modifications as needed

	long nCurSel = m_pPatients->GetCurSel();
	if(nCurSel == -1)
		return;

	// TODO: There are other implementations of 
	// very similar functions in a number of other places in the code.  We need 
	// to consolidate these various implementations!

	try {
		long nPatientID;
		nPatientID = VarLong(m_pPatients->GetValue(nCurSel, plcID));
		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {
			//We don't have to set the Active patient because it is already done when you switch the
			//patient datalist

			// CAH 4/24: True or not, it does not work having
			// this commented out.
			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if(IDNO == AfxMessageBox(
					"This patient is not in the current lookup.  If you proceed, Practice will "
					"clear the lookup before switching to the Patients module.\r\n\r\n"
					"Would you like to clear the lookup and proceed?", MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {
				return;
			}

			//Now just flip to the patient's module and set the active Patient
			pMainFrame->FlipToModule(PATIENT_MODULE_NAME);

			CNxTabView *pView = pMainFrame->GetActiveView();
			if(pView)
				pView->UpdateView();

		}//end if MainFrame
		else {
			AfxThrowNxException("Cannot Open MainFrame");
		}//end else pMainFrame
	} NxCatchAll("Error in CReferredPatients::GoToPatient()");

	//close this dialog
	OnOK();

}

BEGIN_EVENTSINK_MAP(CReferredPatients, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CReferredPatients)
	ON_EVENT(CReferredPatients, IDC_PATIENTS, 6 /* RButtonDown */, OnRButtonDownPatients, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CReferredPatients::OnRButtonDownPatients(long nRow, short nCol, long x, long y, long nFlags) 
{
	//set the selection to the row we rclicked on
	m_pPatients->PutCurSel(nRow);
}
