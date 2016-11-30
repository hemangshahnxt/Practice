// ApptChooseMoreInsuredDlg.cpp : implementation file
//

// (j.gruber 2012-08-03 14:23) - PLID 51896 - created for
#include "stdafx.h"
#include "SchedulerRc.h"
#include "ApptChooseMoreInsuredDlg.h"


#define IDM_REMOVE_INS_PARTY 33476

// CApptChooseMoreInsuredDlg dialog
enum InsOrderListColumn{
	iolcID = 0,
	iolcName,
	iolcRespType,
	iolcPlace,
};

enum InsListColumn {
	ilcID,
	ilcName,
	ilcRespType,
};

IMPLEMENT_DYNAMIC(CApptChooseMoreInsuredDlg, CNxDialog)

CApptChooseMoreInsuredDlg::CApptChooseMoreInsuredDlg(long nCurPatientID, CNewPatientInsuredParty patientInsInfo,
	AppointmentInsuranceMap *pmapInsuranceInfo, CWnd* pParent /*=NULL*/)
	: CNxDialog(CApptChooseMoreInsuredDlg::IDD, pParent),
	m_nCurPatientID(nCurPatientID), m_patientInsInfo(patientInsInfo)
{
	m_pmapInsuranceInfo = pmapInsuranceInfo;

}

CApptChooseMoreInsuredDlg::~CApptChooseMoreInsuredDlg()
{
}

void CApptChooseMoreInsuredDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RES_INS_MOVE_UP, m_btnUp);
	DDX_Control(pDX, IDC_RES_INS_MOVE_DOWN, m_btnDown);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CApptChooseMoreInsuredDlg, CNxDialog)
	ON_BN_CLICKED(IDC_RES_INS_MOVE_UP, &CApptChooseMoreInsuredDlg::OnBnClickedResInsMoveUp)
	ON_BN_CLICKED(IDC_RES_INS_MOVE_DOWN, &CApptChooseMoreInsuredDlg::OnBnClickedResInsMoveDown)
	ON_BN_CLICKED(IDOK, &CApptChooseMoreInsuredDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CApptChooseMoreInsuredDlg::OnBnClickedCancel)	
END_MESSAGE_MAP()


// CApptChooseMoreInsuredDlg message handlers

void CApptChooseMoreInsuredDlg::OnBnClickedResInsMoveUp()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsOrderList->CurSel;
		if (pRow) {
			//get the next row
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pRow->GetPreviousRow();
			if (pPrevRow) {
				//resset the placements
				long nOrigPlace = VarLong(pRow->GetValue(iolcPlace));
				pRow->PutValue(iolcPlace, pPrevRow->GetValue(iolcPlace));
				pPrevRow->PutValue(iolcPlace, nOrigPlace);
				m_pInsOrderList->Sort();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CApptChooseMoreInsuredDlg::OnBnClickedResInsMoveDown()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsOrderList->CurSel;
		if (pRow) {
			//get the next row
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();
			if (pNextRow) {
				//resset the placements
				long nOrigPlace = VarLong(pRow->GetValue(iolcPlace));
				pRow->PutValue(iolcPlace, pNextRow->GetValue(iolcPlace));
				pNextRow->PutValue(iolcPlace, nOrigPlace);
				m_pInsOrderList->Sort();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CApptChooseMoreInsuredDlg::OnBnClickedOk()
{
	//clear our map and then refill it with the information from the datalist

	try {
		ClearInsuranceMap();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsOrderList->GetFirstRow();
		while (pRow) {
			InsuranceInfo *pInfo = new InsuranceInfo();
			
			pInfo->nInsuredPartyID = VarLong(pRow->GetValue(iolcID));
			pInfo->strInsCoName = VarString(pRow->GetValue(iolcName));
			pInfo->strRespType = VarString(pRow->GetValue(iolcRespType));
			m_pmapInsuranceInfo->SetAt(VarLong(pRow->GetValue(iolcPlace)), pInfo);

			pRow = pRow->GetNextRow();
		}

		OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CApptChooseMoreInsuredDlg::OnBnClickedCancel()
{
	try {
		OnCancel();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CApptChooseMoreInsuredDlg, CNxDialog)
	ON_EVENT(CApptChooseMoreInsuredDlg, IDC_RES_INS_LIST, 1, CApptChooseMoreInsuredDlg::SelChangingResInsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CApptChooseMoreInsuredDlg, IDC_RES_INS_LIST, 16, CApptChooseMoreInsuredDlg::SelChosenResInsList, VTS_DISPATCH)
	ON_EVENT(CApptChooseMoreInsuredDlg, IDC_RES_INS_LIST, 18, CApptChooseMoreInsuredDlg::RequeryFinishedResInsList, VTS_I2)
	ON_EVENT(CApptChooseMoreInsuredDlg, IDC_RES_INS_ORDER_LIST, 7, CApptChooseMoreInsuredDlg::RButtonUpResInsOrderList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CApptChooseMoreInsuredDlg::SelChangingResInsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}		
	}NxCatchAll(__FUNCTION__);
}

void CApptChooseMoreInsuredDlg::SelChosenResInsList(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRowAdd(lpRow);
		if (pRowAdd){

			long nInsPartyID = VarLong(pRowAdd->GetValue(ilcID));
			if (nInsPartyID == -1) {
				//we are adding new				
				CString strInsName;
				CString strInsCategory;
				if (AddNewInsuredParty(this, m_patientInsInfo, m_nCurPatientID, GetExistingPatientName(m_nCurPatientID), nInsPartyID, strInsName, strInsCategory)) {

					pRowAdd = m_pInsList->GetNewRow();
					pRowAdd->PutValue(ilcID, nInsPartyID);
					pRowAdd->PutValue(ilcName, _variant_t(strInsName));				
					pRowAdd->PutValue(ilcRespType, _variant_t(strInsCategory));
					m_pInsList->AddRowSorted(pRowAdd, NULL);
				}
				else {
					return;
				}
			}			

			//first make sure the row isn't in the list already
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsOrderList->GetFirstRow();
			long nPlacement = 1;
			while (pRow) {
				long nInsID = VarLong(pRow->GetValue(iolcID));
				if (nInsID == nInsPartyID) {
					MsgBox("This insured party has already been selected for this appointment.");
					return;
				}
				pRow = pRow->GetNextRow();
				nPlacement++;
			}

			//now add the row
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pInsOrderList->GetNewRow();
			if (pNewRow) {
				pNewRow->PutValue(iolcID, nInsPartyID);
				pNewRow->PutValue(iolcName, pRowAdd->GetValue(ilcName));
				pNewRow->PutValue(iolcRespType, pRowAdd->GetValue(ilcRespType));
				pNewRow->PutValue(iolcPlace, nPlacement);

				m_pInsOrderList->AddRowAtEnd(pNewRow, NULL);

				ColorInsList();
			}
			
		}
		
	}NxCatchAll(__FUNCTION__);
}

void CApptChooseMoreInsuredDlg::RequeryFinishedResInsList(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsList->GetNewRow();
		if (pRow) {
			pRow->PutValue(ilcID, (long)-1);
			pRow->PutValue(ilcName, _variant_t("<New Insured Party>"));
			m_pInsList->AddRowBefore(pRow, m_pInsList->GetFirstRow());
		}

		ColorInsList();
	}NxCatchAll(__FUNCTION__);
}

BOOL CApptChooseMoreInsuredDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pInsList = BindNxDataList2Ctrl(IDC_RES_INS_LIST, false);	

		CString strFrom;
		strFrom.Format("(SELECT InsuredPartyT.PersonID as InsuredPartyID, InsuranceCoT.Name as InsCoName, RespTypeT.TypeName as RespTypeName "
			" FROM InsuredPartyT "
			" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			" WHERE RespTypeID <> -1 AND InsuredPartyT.PatientID = %li ) Q "
					, m_nCurPatientID);
		m_pInsList->FromClause = _bstr_t(strFrom);
		m_pInsList->Requery();

		m_pInsOrderList = BindNxDataList2Ctrl(IDC_RES_INS_ORDER_LIST, false);

		//loop through our passed in map and add
		POSITION pos = m_pmapInsuranceInfo->GetStartPosition();
		InsuranceInfo *pInsInfo;
		BOOL bHasPrimary = FALSE;
		BOOL bHasSecondary = FALSE;
		long placement;		
		while (pos != NULL) {				
			m_pmapInsuranceInfo->GetNextAssoc( pos, placement, pInsInfo);

			if (placement == 1 ){
				bHasPrimary = TRUE;
			}
			else if (placement == 2) {
				bHasSecondary = TRUE;
			}
			
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsOrderList->GetNewRow();
			pRow->PutValue(iolcID, pInsInfo->nInsuredPartyID);
			pRow->PutValue(iolcName, _variant_t(pInsInfo->strInsCoName));
			pRow->PutValue(iolcRespType, _variant_t(pInsInfo->strRespType));
			pRow->PutValue(iolcPlace, placement);

			m_pInsOrderList->AddRowSorted(pRow, NULL);			
		}	

		if (bHasSecondary && !bHasPrimary) {
			//give them a warning that this will be corrected for them
			MsgBox("You have chosen a secondary insurance on this appointment, but not a primary, this will be corrected for you.");
		}

		//run though the list and change the placements if necessary
		NXDATALIST2Lib::IRowSettingsPtr pRowCheck = m_pInsOrderList->GetFirstRow();
		long nPlace = 1;
		while (pRowCheck) {
			pRowCheck->PutValue(iolcPlace, nPlace);
			pRowCheck = pRowCheck->GetNextRow();
			nPlace++;
		}
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}


void CApptChooseMoreInsuredDlg::ClearInsuranceMap()
{
	POSITION pos = m_pmapInsuranceInfo->GetStartPosition();
	InsuranceInfo *pInsInfo;
	long nPlacement;
	while (pos != NULL) {				
		m_pmapInsuranceInfo->GetNextAssoc( pos, nPlacement, pInsInfo);
		if (pInsInfo) {
			delete pInsInfo;
		}		
	}
	m_pmapInsuranceInfo->RemoveAll();
}

void CApptChooseMoreInsuredDlg::RButtonUpResInsOrderList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_pInsOrderList->CurSel = pRow;
			CMenu mnu;
			mnu.CreatePopupMenu();			
			mnu.AppendMenu(MF_BYPOSITION, IDM_REMOVE_INS_PARTY, "Remove");			
			
			CPoint pt(x,y);
			CWnd* pWnd = GetDlgItem(IDC_RES_INS_ORDER_LIST);

			if (pWnd != NULL) {
				pWnd->ClientToScreen(&pt);

				int nCmdID = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);

				if (nCmdID == IDM_REMOVE_INS_PARTY) {
					m_pInsOrderList->RemoveRow(pRow);

					//run through and reorder the list
					NXDATALIST2Lib::IRowSettingsPtr pRowCheck = m_pInsOrderList->GetFirstRow();
					long nPlace = 1;
					while (pRowCheck) {

						pRowCheck->PutValue(iolcPlace, nPlace);

						pRowCheck = pRowCheck->GetNextRow();
						nPlace++;
					}

					//recolor the top list
					ColorInsList();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CApptChooseMoreInsuredDlg::ColorInsList()
{
	//run through the top datalist and grey out any used ones from the bottom list
	NXDATALIST2Lib::IRowSettingsPtr pRowTop = m_pInsList->GetFirstRow();
	while (pRowTop) {

		pRowTop->PutForeColor(0);

		long nInsPartyID = VarLong(pRowTop->GetValue(ilcID));
		
		if (nInsPartyID != -1) {
			
			NXDATALIST2Lib::IRowSettingsPtr pRowBottom = m_pInsOrderList->FindByColumn(iolcID, nInsPartyID, NULL, FALSE);
			//we found it in the bootom list, so set the top list's row
			if (pRowBottom) {
				pRowTop->ForeColor = 7303023;
			}			
		}
		pRowTop = pRowTop->GetNextRow();
	}

}