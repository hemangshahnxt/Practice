// FavoritePharmaciesEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FavoritePharmaciesEditDlg.h"
#include "PharmacyDirectorySearchDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CFavoritePharmaciesEditDlg dialog

// (j.jones 2008-10-06 15:20) - PLID 31596 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum PharmacyComboColumns {

	pccID = 0,
	pccName,
	pccAddress,
	pccCity,
	pccState,
	pccZip,
	pccPhone,	// (j.jones 2011-06-24 16:26) - PLID 32313 - added Phone
};

enum FavoritePharmacyListColumns {

	fplcID = 0,
	fplcName,
	fplcAddress,
	fplcCity,
	fplcState,
	fplcZip,
	fplcOrderIndex,
	fplcPhone,	// (j.jones 2011-06-24 16:26) - PLID 32313 - added Phone
};


CFavoritePharmaciesEditDlg::CFavoritePharmaciesEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFavoritePharmaciesEditDlg::IDD, pParent)
{
	m_nPatientID = -1;
}


void CFavoritePharmaciesEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFaxSendDlg)
	DDX_Control(pDX, IDC_BTN_CLOSE_PHARMACIES, m_btnClose);
	DDX_Control(pDX, IDC_BTN_MOVE_PHARM_UP, m_btnMoveUp);
	DDX_Control(pDX, IDC_BTN_MOVE_PHARM_DOWN, m_btnMoveDown);
	DDX_Control(pDX, IDC_FAVPHARM_BKG, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFavoritePharmaciesEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE_PHARMACIES, OnBtnClosePharmacies)
	ON_BN_CLICKED(IDC_BTN_MOVE_PHARM_UP, OnBnClickedBtnMovePharmUp)
	ON_BN_CLICKED(IDC_BTN_MOVE_PHARM_DOWN, OnBnClickedBtnMovePharmDown)
END_MESSAGE_MAP()


// CFavoritePharmaciesEditDlg message handlers

BOOL CFavoritePharmaciesEditDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		// (b.savon 2013-01-24 16:38) - PLID 54842
		SetTitleBarIcon(IDI_ERX);

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnMoveUp.AutoSet(NXB_UP);
		m_btnMoveDown.AutoSet(NXB_DOWN);

		//set the background color to be the patient color, regardless of the patient's status
		m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		CString str;
		str.Format("Edit Favorite Pharmacies for %s", m_strPatientName);
		SetWindowText(str);

		m_PharmacyCombo = BindNxDataList2Ctrl(IDC_PHARMACY_COMBO, true);
		// (a.walling 2009-03-31 17:21) - PLID 33573 - Add a new pharmacy from directory
		AddPharmacyDirectoryOptionToCombo();

		m_FavoriteList = BindNxDataList2Ctrl(IDC_FAVORITE_PHARMACIES_LIST, false);

		str.Format("Active = 1 AND TypeID = 3 AND PatientID = %li", m_nPatientID);
		m_FavoriteList->WhereClause = _bstr_t(str);
		m_FavoriteList->Requery();

		EnableArrows();

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFavoritePharmaciesEditDlg::AddPharmacyDirectoryOptionToCombo() {
	// (a.walling 2009-03-31 17:21) - PLID 33573 - Add a new pharmacy from directory
	if (g_pLicense->CheckForLicense(CLicense::lcePrescribe, CLicense::cflrSilent)) {
		IRowSettingsPtr pRow = m_PharmacyCombo->GetNewRow();
		pRow->PutValue(pccID, (long)espiAddFromDirectory);
		pRow->PutValue(pccName, (LPCTSTR)"  < Add from Pharmacy Directory >");
		pRow->PutValue(pccAddress, (LPCTSTR)"");
		pRow->PutValue(pccCity, (LPCTSTR)"");
		pRow->PutValue(pccState, (LPCTSTR)"");
		pRow->PutValue(pccZip, (LPCTSTR)"");
		// (j.jones 2011-06-24 16:26) - PLID 32313 - added Phone
		pRow->PutValue(pccPhone, (LPCTSTR)"");

		m_PharmacyCombo->AddRowSorted(pRow, NULL);
	}
}


void CFavoritePharmaciesEditDlg::OnBtnClosePharmacies()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::OnBtnClosePharmacies");
}


void CFavoritePharmaciesEditDlg::OnBnClickedBtnMovePharmUp()
{
	try {
		
		IRowSettingsPtr pCurRow = m_FavoriteList->GetCurSel();
		if(pCurRow == NULL) {
			//why is this null? Disable this button.
			EnableArrows();
			return;
		}

		IRowSettingsPtr pPrevRow = pCurRow->GetPreviousRow();
		if(pPrevRow == NULL) {
			//why is this null? Disable this button.
			EnableArrows();
			return;
		}

		//we have two valid rows, so let's swap them
		long nPrevOrder = VarLong(pPrevRow->GetValue(fplcOrderIndex));
		long nCurOrder = VarLong(pCurRow->GetValue(fplcOrderIndex));

		long nPrevID = VarLong(pPrevRow->GetValue(fplcID));
		long nCurID = VarLong(pCurRow->GetValue(fplcID));

		//swap in data
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "UPDATE FavoritePharmaciesT SET OrderIndex = %li WHERE PatientID = %li AND PharmacyID = %li",
			nCurOrder, m_nPatientID, nPrevID);
		AddStatementToSqlBatch(strSqlBatch, "UPDATE FavoritePharmaciesT SET OrderIndex = %li WHERE PatientID = %li AND PharmacyID = %li",
			nPrevOrder, m_nPatientID, nCurID);
		ExecuteSqlBatch(strSqlBatch);

		//swap in the list
		pPrevRow->PutValue(fplcOrderIndex, nCurOrder);
		pCurRow->PutValue(fplcOrderIndex, nPrevOrder);

		//and re-sort
		m_FavoriteList->Sort();

		EnableArrows();

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::OnBnClickedBtnMovePharmUp");
}

void CFavoritePharmaciesEditDlg::OnBnClickedBtnMovePharmDown()
{
	try {

		IRowSettingsPtr pCurRow = m_FavoriteList->GetCurSel();
		if(pCurRow == NULL) {
			//why is this null? Disable this button.
			EnableArrows();
			return;
		}

		IRowSettingsPtr pNextRow = pCurRow->GetNextRow();
		if(pNextRow == NULL) {
			//why is this null? Disable this button.
			EnableArrows();
			return;
		}

		//we have two valid rows, so let's swap them
		long nNextOrder = VarLong(pNextRow->GetValue(fplcOrderIndex));
		long nCurOrder = VarLong(pCurRow->GetValue(fplcOrderIndex));

		long nNextID = VarLong(pNextRow->GetValue(fplcID));
		long nCurID = VarLong(pCurRow->GetValue(fplcID));

		//swap in data
		CString strSqlBatch;
		AddStatementToSqlBatch(strSqlBatch, "UPDATE FavoritePharmaciesT SET OrderIndex = %li WHERE PatientID = %li AND PharmacyID = %li",
			nCurOrder, m_nPatientID, nNextID);
		AddStatementToSqlBatch(strSqlBatch, "UPDATE FavoritePharmaciesT SET OrderIndex = %li WHERE PatientID = %li AND PharmacyID = %li",
			nNextOrder, m_nPatientID, nCurID);
		ExecuteSqlBatch(strSqlBatch);

		//swap in the list
		pNextRow->PutValue(fplcOrderIndex, nCurOrder);
		pCurRow->PutValue(fplcOrderIndex, nNextOrder);

		//and re-sort
		m_FavoriteList->Sort();

		EnableArrows();

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::OnBnClickedBtnMovePharmDown");
}
BEGIN_EVENTSINK_MAP(CFavoritePharmaciesEditDlg, CNxDialog)
	ON_EVENT(CFavoritePharmaciesEditDlg, IDC_PHARMACY_COMBO, 16, OnSelChosenPharmacyCombo, VTS_DISPATCH)
	ON_EVENT(CFavoritePharmaciesEditDlg, IDC_FAVORITE_PHARMACIES_LIST, 2, OnSelChangedFavoritePharmaciesList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CFavoritePharmaciesEditDlg, IDC_FAVORITE_PHARMACIES_LIST, 6, OnRButtonDownPharmaciesList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CFavoritePharmaciesEditDlg::OnSelChosenPharmacyCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nPharmacyID = VarLong(pRow->GetValue(pccID));		

		// (a.walling 2009-03-31 17:21) - PLID 33573 - Add a new pharmacy from directory
		if (nPharmacyID == espiAddFromDirectory) {
			CPharmacyDirectorySearchDlg dlg(this);
			dlg.m_bMultiMode = FALSE;

			if (IDOK == dlg.DoModal()) {
				long nSelectedPharmacyID = dlg.m_nSelectedID;

				if (nSelectedPharmacyID != -1) {
					nPharmacyID = nSelectedPharmacyID;
					m_PharmacyCombo->Requery(); 					
					AddPharmacyDirectoryOptionToCombo();
					m_PharmacyCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);

					pRow = m_PharmacyCombo->FindByColumn(pccID, nPharmacyID, NULL, VARIANT_TRUE);

					if (pRow == NULL) {
						MessageBox("Could not find new pharmacy!", NULL, MB_ICONEXCLAMATION);
						return;
					}
				} else {
					return;
				}
			} else {
				return;
			}
		}

		//see if the pharmacy is in the list, and if so, warn and leave
		IRowSettingsPtr pFoundRow = m_FavoriteList->FindByColumn(fplcID, (long)nPharmacyID, m_FavoriteList->GetFirstRow(), VARIANT_TRUE);
		if(pFoundRow) {

			//we changed the selection, so enable the arrows
			EnableArrows();

			AfxMessageBox("The selected pharmacy is already in this patient's favorites list. "
				"You may use the arrows to increase the priority of this pharmacy if necessary.");
			return;
		}

		//otherwise, we need to add to the list with the highest priority, they can change it if they wish		

		long nOrderIndex = m_FavoriteList->GetRowCount() + 1;

		ExecuteParamSql("INSERT INTO FavoritePharmaciesT (PatientID, PharmacyID, OrderIndex) "
			"VALUES ({INT}, {INT}, {INT})", m_nPatientID, nPharmacyID, nOrderIndex);

		IRowSettingsPtr pNewRow = m_FavoriteList->GetNewRow();
		pNewRow->PutValue(fplcID, nPharmacyID);
		pNewRow->PutValue(fplcName, pRow->GetValue(pccName));
		pNewRow->PutValue(fplcAddress, pRow->GetValue(pccAddress));
		pNewRow->PutValue(fplcCity, pRow->GetValue(pccCity));
		pNewRow->PutValue(fplcState, pRow->GetValue(pccState));
		pNewRow->PutValue(fplcZip, pRow->GetValue(pccZip));
		pNewRow->PutValue(fplcOrderIndex, nOrderIndex);
		// (j.jones 2011-06-24 16:26) - PLID 32313 - added Phone
		pNewRow->PutValue(fplcPhone, pRow->GetValue(pccPhone));
		m_FavoriteList->AddRowSorted(pNewRow, NULL);

		EnableArrows();

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::OnSelChosenPharmacyCombo");
}

void CFavoritePharmaciesEditDlg::OnSelChangedFavoritePharmaciesList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		EnableArrows();

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::OnSelChangedFavoritePharmaciesList");
}

void CFavoritePharmaciesEditDlg::EnableArrows()
{
	try {

		IRowSettingsPtr pRow = m_FavoriteList->GetCurSel();

		if(pRow == NULL || (m_FavoriteList->GetFirstRow() == m_FavoriteList->GetLastRow())) {
			m_btnMoveUp.EnableWindow(FALSE);
			m_btnMoveDown.EnableWindow(FALSE);
		}
		else if(pRow == m_FavoriteList->GetFirstRow()) {
			m_btnMoveUp.EnableWindow(FALSE);
			m_btnMoveDown.EnableWindow(TRUE);
		}
		else if(pRow == m_FavoriteList->GetLastRow()) {
			m_btnMoveUp.EnableWindow(TRUE);
			m_btnMoveDown.EnableWindow(FALSE);
		}
		else {
			m_btnMoveUp.EnableWindow(TRUE);
			m_btnMoveDown.EnableWindow(TRUE);
		}

	}NxCatchAll("Error in CFavoritePharmaciesEditDlg::EnableArrows");
}

void CFavoritePharmaciesEditDlg::OnRButtonDownPharmaciesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			return;
		}

		m_FavoriteList->PutCurSel(pRow);

		enum MenuItem
		{
			miDelete = 1,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();

		mnu.AppendMenu(MF_ENABLED, miDelete, "Remove Pharmacy");

		CPoint ptClicked(x, y);
		GetDlgItem(IDC_FAVORITE_PHARMACIES_LIST)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);

		switch(nResult)
		{
			case miDelete:
			{
				int nResult = MessageBox("Are you sure you want to remove the selected pharmacy from this patient's favorites?", "Practice", MB_YESNO|MB_ICONQUESTION);
				if(nResult == IDYES)
				{
					long nPharmacyIDToRemove = VarLong(pRow->GetValue(fplcID));

					//update all the remaining order indices
					CString strSqlBatch;
					IRowSettingsPtr pCursorRow = pRow->GetNextRow();
					while(pCursorRow) {
						long nPharmacyIDToUpdate = VarLong(pCursorRow->GetValue(fplcID));
						long nOrderIndex = VarLong(pCursorRow->GetValue(fplcOrderIndex));
						nOrderIndex--;

						pCursorRow->PutValue(fplcOrderIndex, nOrderIndex);

						AddStatementToSqlBatch(strSqlBatch, "UPDATE FavoritePharmaciesT SET OrderIndex = %li "
							"WHERE PatientID = %li AND PharmacyID = %li", nOrderIndex, m_nPatientID, nPharmacyIDToUpdate);						

						pCursorRow = pCursorRow->GetNextRow();
					}
					
					//now remove our pharmacy
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM FavoritePharmaciesT WHERE PatientID = %li AND PharmacyID = %li", m_nPatientID, nPharmacyIDToRemove);

					ExecuteSqlBatch(strSqlBatch);

					//remove the row
					m_FavoriteList->RemoveRow(pRow);
					m_FavoriteList->Sort();
					
					//update the arrows
					EnableArrows();
				}
			}
			break;
		}

	}NxCatchAll("CFavoritePharmaciesEditDlg::OnRButtonDownPharmaciesList");
}