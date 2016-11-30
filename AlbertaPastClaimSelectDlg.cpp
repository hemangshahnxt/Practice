// AlbertaPastClaimSelectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlbertaPastClaimSelectDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2011-07-21 15:55) - PLID 44662 - created

// CAlbertaPastClaimSelectDlg dialog

enum ListColumns {

	lcID = 0,
	lcDate,
	lcClaimNumber,
};

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CAlbertaPastClaimSelectDlg, CNxDialog)

CAlbertaPastClaimSelectDlg::CAlbertaPastClaimSelectDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAlbertaPastClaimSelectDlg::IDD, pParent)
{
	m_nCurrentClaimHistoryDetailID = -1;
	m_nSelectedClaimHistoryDetailID = -1;
	m_nUserDefinedID = -1;
	m_nBillID = -1;
	m_dtBillDate = g_cdtInvalid;
}

CAlbertaPastClaimSelectDlg::~CAlbertaPastClaimSelectDlg()
{
}

void CAlbertaPastClaimSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ALBERTA_PAST_CLAIMS_LABEL_1, m_nxstaticLabel1);
	DDX_Control(pDX, IDC_ALBERTA_PAST_CLAIMS_LABEL_2, m_nxstaticLabel2);
	DDX_Control(pDX, IDC_ALBERTA_PAST_CLAIMS_LABEL_3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_ALBERTA_PAST_CLAIMS_LABEL_4, m_nxstaticLabel4);
}

BEGIN_MESSAGE_MAP(CAlbertaPastClaimSelectDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

// CAlbertaPastClaimSelectDlg message handlers

int CAlbertaPastClaimSelectDlg::DoModal(long nCurrentClaimHistoryDetailID, CString strActionType, long nUserDefinedID, CString strPatientName,
										long nBillID, COleDateTime dtBillDate, CString strBillDescription,
										CString strSubmitterPrefix, CString strSourceCode)
{
	m_nCurrentClaimHistoryDetailID = nCurrentClaimHistoryDetailID;
	m_strActionType = strActionType;
	m_nUserDefinedID = nUserDefinedID;
	m_strPatientName = strPatientName;
	m_nBillID = nBillID;
	m_dtBillDate = dtBillDate;
	m_strBillDescription = strBillDescription;
	m_strSubmitterPrefix = strSubmitterPrefix;
	m_strSourceCode = strSourceCode;

	return CNxDialog::DoModal();
}

BOOL CAlbertaPastClaimSelectDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_List = BindNxDataList2Ctrl(IDC_CLAIM_HISTORY_LIST, false);

		for(int i=0; i<m_aryClaimHistoryInfo.GetSize(); i++) {
			
			ClaimHistoryInfo chInfo = (ClaimHistoryInfo)m_aryClaimHistoryInfo.GetAt(i);

			long nClaimHistoryDetailID = chInfo.nID;

			//display the Alberta claim number, which is actually Submitter Prefix,
			//Current Year, Source Code, Sequence Number, Check Digit

			CString strSequenceNumber = AsString(nClaimHistoryDetailID);
			while(strSequenceNumber.GetLength() < 7) {
				//force it to be 7 digits long
				strSequenceNumber = "0" + strSequenceNumber;
			}

			CString strYear;
			strYear.Format("%li", chInfo.dtDate.GetYear());
			strYear = strYear.Right(2);

			// (j.dinatale 2013-01-21 16:54) - PLID 54419 - we can now be given claim numbers, we need to ensure that we dont need to calculate any of them
			CString strClaimNumber;
			if(chInfo.strClaimNumber.IsEmpty()){
				strClaimNumber = m_strSubmitterPrefix + strYear + m_strSourceCode
					+ strSequenceNumber + AsString(CalculateAlbertaClaimNumberCheckDigit(strSequenceNumber));
			}else{
				strClaimNumber = chInfo.strClaimNumber;
			}

			IRowSettingsPtr pRow = m_List->GetNewRow();
			pRow->PutValue(lcID, nClaimHistoryDetailID);
			pRow->PutValue(lcDate, _variant_t(chInfo.dtDate, VT_DATE));
			pRow->PutValue(lcClaimNumber, (LPCTSTR)strClaimNumber);
			m_List->AddRowSorted(pRow, NULL);
		}

		CString strLabel1;
		CString strActionTense;
		if(m_strActionType.Right(1) == "e") {
			strActionTense = m_strActionType + "d";
		}
		else {
			strActionTense = m_strActionType + "ed";
		}
		strLabel1.Format("The following claim is flagged to be %s:", strActionTense);
		m_nxstaticLabel1.SetWindowText(strLabel1);

		CString strLabel2;
		strLabel2.Format("Patient: %s, ID: %li", m_strPatientName, m_nUserDefinedID);
		m_nxstaticLabel2.SetWindowText(strLabel2);

		CString strLabel3;
		strLabel3.Format("Bill: %s, Date: %s, ID: %li", m_strBillDescription, FormatDateTimeForInterface(m_dtBillDate), m_nBillID);
		m_nxstaticLabel3.SetWindowText(strLabel3);

		CString strLabel4;
		strLabel4.Format("You must select a prior submission to %s from the list below.", m_strActionType);
		m_nxstaticLabel4.SetWindowText(strLabel4);

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CAlbertaPastClaimSelectDlg::OnOk()
{
	try {


		IRowSettingsPtr pRow = m_List->GetCurSel();
		if(pRow == NULL) {
			CString strWarning;
			strWarning.Format("You must select a previous submission to %s.\n\n"
				"If you wish to add this as a new claim instead, click Cancel on this screen.", m_strActionType);
			MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		m_nSelectedClaimHistoryDetailID = VarLong(pRow->GetValue(lcID));
		m_strSelectedClaimNumber = VarString(pRow->GetValue(lcClaimNumber));	// (j.dinatale 2013-01-18 16:57) - PLID 54419 - get the transaction number

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CAlbertaPastClaimSelectDlg::OnCancel()
{
	try {

		CString strWarning;
		strWarning.Format("In order to %s a claim, you must select a previous submission to %s.\n\n"
			"Do you wish to add this as a new claim instead?", m_strActionType, m_strActionType);								
		if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		m_nSelectedClaimHistoryDetailID = -1;
		m_strSelectedClaimNumber = "";	// (j.dinatale 2013-01-18 16:57) - PLID 54419 - get the transaction number

		CNxDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CAlbertaPastClaimSelectDlg, CNxDialog)
	ON_EVENT(CAlbertaPastClaimSelectDlg, IDC_CLAIM_HISTORY_LIST, 3, OnDblClickCellClaimHistoryList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CAlbertaPastClaimSelectDlg::OnDblClickCellClaimHistoryList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_List->PutCurSel(pRow);
			OnOk();
		}

	} NxCatchAll(__FUNCTION__);
}
