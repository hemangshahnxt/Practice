// NexWebApptInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practicerc.h"
#include "NexWebApptInfoDlg.h"
#include "NexwebImportDlg.h"
#include "MultiSelectDlg.h"
#include "NxMessageDef.h"
#include "commonschedutils.h"
#include "globalschedutils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CNexWebApptInfoDlg dialog


CNexWebApptInfoDlg::CNexWebApptInfoDlg(long nApptID, BOOL bIsNew, long nPersonID, ApptImport * pAppt, CWnd* pParent)
	: CNxDialog(CNexWebApptInfoDlg::IDD, pParent)
{
	m_nApptID = nApptID;
	m_bIsNew = bIsNew;
	m_nPersonID = nPersonID;
	m_pAppt = pAppt;
	
	//{{AFX_DATA_INIT(CNexWebApptInfoDlg)
	//}}AFX_DATA_INIT
}


void CNexWebApptInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebApptInfoDlg)
	DDX_Control(pDX, IDC_MULTI_PURPOSE_LIST, m_nxtPurposeLabel);
	DDX_Control(pDX, IDC_MULTI_RESOURCE_LIST, m_nxtResourceLabel);
	DDX_Control(pDX, IDC_EVENT_DATE_START, m_ApptDate);
	DDX_Control(pDX, IDC_NEXWEB_NOTES_BOX, m_nxeditNexwebNotesBox);
	DDX_Control(pDX, IDC_NEXWEB_CANCELLED_STATUS, m_nxstaticNexwebCancelledStatus);
	DDX_Control(pDX, IDC_IMPORT, m_btnOk);
	DDX_Control(pDX, IDC_DELETE, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebApptInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebApptInfoDlg)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_NEXWEB_CANCELLED_STATUS, OnNexwebCancelledStatus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// CNexWebApptInfoDlg message handlers

BOOL CNexWebApptInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
	
		//bind the data lists
		m_pLocationList = BindNxDataListCtrl(IDC_NEXWEB_APTLOCATION_COMBO, GetRemoteData(), true);
		//m_pStatusList = BindNxDataListCtrl(IDC_NEXWEB_LIST_STATUS, GetRemoteData(), true);
		m_pResourceList = BindNxDataListCtrl(IDC_NEXWEB_APTRESOURCE_COMBO, GetRemoteData(), true);
		m_pTypeList = BindNxDataListCtrl(IDC_NEXWEB_APTTYPE_COMBO, GetRemoteData(), true);
		m_pPurposeList = BindNxDataListCtrl(IDC_NEXWEB_APTPURPOSE_COMBO, GetRemoteData(), false);
		// (j.gruber 2007-02-23 10:58) - PLID 24767 - changed moveup from combo box to datalist2
		m_pMoveUpList = BindNxDataList2Ctrl(IDC_NEXWEB_MOVE_UP_LIST, GetRemoteData(), false);

		m_pStartTime = BindNxTimeCtrl(this, IDC_NEXWEB_START_TIME_BOX);
		m_pEndTime = BindNxTimeCtrl(this, IDC_NEXWEB_END_TIME_BOX);

		CFont fnt;
		fnt.CreateFont(
		27,                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,                 // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial"));                 // lpszFacename

		GetDlgItem(IDC_NEXWEB_CANCELLED_STATUS)->SetFont(&fnt);
		m_nxtPurposeLabel.SetColor(GetSysColor(COLOR_3DFACE));
		m_nxtPurposeLabel.SetType(dtsHyperlink);
		//m_nxtPurposeLabel.ModifyStyle(NULL, WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT|SS_NOTIFY);
		m_nxtResourceLabel.SetColor(GetSysColor(COLOR_3DFACE));
		m_nxtResourceLabel.SetType(dtsHyperlink);
		//m_nxtResourceLabel.ModifyStyle(NULL, WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT|SS_NOTIFY);

		// (j.gruber 2007-02-23 10:58) - PLID 24767 - changed moveup from combo box to datalist2
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = 	m_pMoveUpList->GetNewRow();
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, _variant_t("No"));
		m_pMoveUpList->AddRowAtEnd(pRow, NULL);
		
		pRow = 	m_pMoveUpList->GetNewRow();
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, _variant_t("Yes"));
		m_pMoveUpList->AddRowAtEnd(pRow, NULL);

		m_pMoveUpList->SetSelByColumn(0, (long)0);

		//enable them all if this is a new appt
		if (m_bIsNew) {
			InitializeControls(TRUE);
		}
		else {
			InitializeControls();
		}

		//fill all the information in
		FillBoxes();
		
	}NxCatchAll("Error in OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#define SET_CONTROL(strField) SetDlgItemText(pWnd->GetDlgCtrlID(), AdoFldString(rsAppt, strField, ""))

void CNexWebApptInfoDlg::InitializeControls(BOOL bEnabled /* = FALSE*/) {

	// (d.moore 2007-05-22 16:07) - PLID 4013 - The original MoveUp field for AppointmentsT is no longer
	//  valid. The WaitingListT table now handles similar functionality.
	_RecordsetPtr rsAppt = CreateRecordset(
		"SELECT AppointmentsT.ShowState, AppointmentsT.Confirmed, AppointmentsT.Ready, AppointmentsT.Date, "
			"AppointmentsT.StartTime, AppointmentsT.EndTime, AppointmentsT.LocationID, "
			"CONVERT(bit, CASE WHEN ISNULL(WaitingListT.ID, 0) > 0 THEN 1 ELSE 0 END) AS MoveUp, "
			"AppointmentsT.AptTypeID, AppointmentsT.Notes, AppointmentsT.Status "
		"FROM AppointmentsT LEFT JOIN WaitingListT ON AppointmentsT.ID = WaitingListT.AppointmentID "
		"WHERE AppointmentsT.ID = %li", m_nApptID);

	CWnd *pWnd;
	long i;
	//loop through all the controls on the dialog, fill them if possible, if not disable them
	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT)) {

		if (pWnd && pWnd->m_hWnd) {

			if ((pWnd->GetDlgCtrlID() != IDC_STATIC) && pWnd->GetDlgCtrlID() != IDC_IMPORT && pWnd->GetDlgCtrlID() != IDC_DELETE) {

				if (! rsAppt->eof) {

					switch (pWnd->GetDlgCtrlID()) {

						/*case IDC_NEXWEB_LIST_STATUS:
							m_pStatusList->SetSelByColumn(0, rsAppt->Fields->Item["ShowState"]->Value);
						break;*/

						case IDC_NEXWEB_MOVE_UP_LIST:
							//m_MoveUp.SetCurSel(AdoFldBool(rsAppt, "MoveUp"));

							// (j.gruber 2007-02-23 09:30) - PLID 24767 - change to be datalist2
							if (AdoFldBool(rsAppt, "MoveUp", FALSE)) {
								m_pMoveUpList->SetSelByColumn(0, (long)1);
							}
							else {
								m_pMoveUpList->SetSelByColumn(0, (long)0);
							}
						break;

						/*case IDC_NEXWEB_CONFIRMED_COMBO:
							m_Confirmed.SetCurSel(AdoFldLong(rsAppt, "Confirmed"));
						break;*/

						case IDC_EVENT_DATE_START:
							m_ApptDate.SetValue(rsAppt->Fields->Item["Date"]->Value);
						break;

						case IDC_NEXWEB_START_TIME_BOX:
							m_pStartTime->SetDateTime(AdoFldDateTime(rsAppt, "StartTime"));
						break;

						case IDC_NEXWEB_END_TIME_BOX:
							m_pEndTime->SetDateTime(AdoFldDateTime(rsAppt, "EndTime"));
						break;

						case IDC_NEXWEB_APTLOCATION_COMBO:
							m_pLocationList->SetSelByColumn(0, rsAppt->Fields->Item["LocationID"]->Value);
						break;

						case IDC_NEXWEB_APTRESOURCE_COMBO:  {
							//we have to do this one ourselves
							_RecordsetPtr rsResource = CreateRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %li", m_nApptID);
							while (! rsResource->eof) {
								m_ResourceList.Add(AdoFldLong(rsResource, "ResourceID"));
								rsResource->MoveNext();
							}
															}
						break;

						case IDC_NEXWEB_APTTYPE_COMBO:
							m_pTypeList->SetSelByColumn(0, rsAppt->Fields->Item["AptTypeID"]->Value);
						break;
 
						case IDC_NEXWEB_APTPURPOSE_COMBO: {
							//do this one ourselves
							_RecordsetPtr rsPurpose = CreateRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = %li", m_nApptID);
							while (! rsPurpose->eof) {
								m_PurposeList.Add(AdoFldLong(rsPurpose, "PurposeID"));
								rsPurpose->MoveNext();
							}
										}
						break;

						case IDC_NEXWEB_NOTES_BOX:
							SET_CONTROL("Notes");
						break;

						case IDC_NEXWEB_CANCELLED_STATUS: 
							if (AdoFldByte(rsAppt, "Status") == 4) {
								SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "CANCELLED");
							}
							else {
								SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "NOT CANCELLED");
							}
						break;

							
					}
				}
				pWnd->EnableWindow(bEnabled);
			}
			else {
				pWnd->EnableWindow(TRUE);
			}
			
		}
		else {
			pWnd->EnableWindow(TRUE);
		}
	}
	
	
	//set this up now and it will be changed if necessary
	SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "NOT CANCELLED");
	GetDlgItem(IDC_NEXWEB_CANCELLED_STATUS)->EnableWindow(TRUE);
			
}

void CNexWebApptInfoDlg::FillBoxes() {

	if (!m_pAppt->bHasBeenSaved) {
		//get the information we are going to need
		_RecordsetPtr rs = CreateRecordset("SELECT TransType, Field, Value FROM NexWebTransactionsT WHERE ObjectID = %li AND PersonID = %li ORDER BY ID ASC", m_nApptID, m_nPersonID);
		CString strValue;
		TransactionType ttFieldID;
	
		while (! rs->eof) {

			strValue = AdoFldString(rs, "Value");
			ttFieldID = (TransactionType)AdoFldLong(rs, "Field");

			ProcessField(ttFieldID, strValue);
		
			rs->MoveNext();
		}
	 
		//now that we have almost all the boxes filled, we need to process the resources and purposes
		RequeryApptPurposes();
		ProcessMultiSelectList(&m_PurposeList, &m_strMultiPurpose, m_pPurposeList, IDC_NEXWEB_APTPURPOSE_COMBO, &m_nxtPurposeLabel, "Name", "AptPurposeT");
		ProcessMultiSelectList(&m_ResourceList, &m_strMultiResource, m_pResourceList, IDC_NEXWEB_APTRESOURCE_COMBO, &m_nxtResourceLabel, "Item", "ResourceT");
	}
	else {
		//if we are doing it this way, enable all the boxes
		InitializeControls(TRUE);
		//get the information out of the struct
		// (j.gruber 2007-04-24 14:35) - PLID 24767 - fix the casting to a long
		ProcessField(transTypeApptMoveUp, (m_pAppt->bMoveUp) ? "1" : "0");
		ProcessField(transTypeApptDate, FormatDateTimeForInterface(m_pAppt->dtApptDate));
		ProcessField(transTypeApptStartTime, FormatDateTimeForInterface(m_pAppt->dtStartTime));
		ProcessField(transTypeApptEndTime, FormatDateTimeForInterface(m_pAppt->dtEndTime));
		ProcessField(transTypeApptLocation, AsString(_variant_t(m_pAppt->nLocationID)));
		ProcessField(transTypeApptType, AsString(_variant_t(m_pAppt->nTypeID)));
		ProcessField(transTypeApptNote, AsString(_variant_t(m_pAppt->strNotes)));
		if (m_pAppt->nStatus == 4) {
			SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "Cancelled");
		}
		else {
			SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "Not Cancelled");
		}
		FillDWordArray(m_ResourceList, m_pAppt->dwResourceList);
		FillDWordArray(m_PurposeList, m_pAppt->dwPurposeList);
		
		RequeryApptPurposes();
		ProcessMultiSelectList(&m_ResourceList, &m_strMultiResource, m_pResourceList, IDC_NEXWEB_APTRESOURCE_COMBO, &m_nxtResourceLabel, "Item", "ResourceT");
		ProcessMultiSelectList(&m_PurposeList, &m_strMultiPurpose, m_pPurposeList, IDC_NEXWEB_APTPURPOSE_COMBO, &m_nxtPurposeLabel, "Name", "AptPurposeT");
				
	}

	 
}

void CNexWebApptInfoDlg::ProcessMultiSelectList(CDWordArray *pAry, CString *strList, NXDATALISTLib::_DNxDataListPtr pDatalist, int nID, CNxLabel *pLbl, CString strField, CString strTable) 
{ 

	//first see whether to enable it or not
	BOOL bChanged = FALSE;
	for (int i = 0; i < m_IDsToSave.GetSize(); i++) {
		if (m_IDsToSave.GetAt(i) == nID) {
			bChanged = TRUE;
		}
	}
	if (bChanged) {
		GetDlgItem(nID)->EnableWindow(TRUE);
	}

	//now figure out whether we want the datalist or the label
	if (pAry->GetSize() == 1) {
		//hide the label
		pLbl->ShowWindow(FALSE);
		GetDlgItem(nID)->ShowWindow(TRUE);
		pDatalist->SetSelByColumn(0, (long)pAry->GetAt(0));
		*strList = "";
		
	}
	else if (pAry->GetSize() > 1) {

		//we need the label
		GetDlgItem(nID)->ShowWindow(FALSE);
		pLbl->ShowWindow(TRUE);

		CString strIDList = "";
		CString strTemp = "";
		//generate the list of values
		for (i = 0; i < pAry->GetSize(); i++ ) {
			strTemp.Format("%li, ", (long)pAry->GetAt(i));
			strIDList += strTemp;
		}

		//take the last , off
		strIDList.TrimLeft();
		strIDList.TrimRight();
		strIDList = strIDList.Left(strIDList.GetLength() - 1);

		// now we need to make a recordset to get the names out
		_RecordsetPtr rs = CreateRecordset("SELECT %s as Field FROM %s WHERE ID IN (%s)", strField, strTable, strIDList);

		CString strNameList = "";
		while (! rs->eof) {
			strTemp.Format("%s, ", AdoFldString(rs, "Field"));
			strNameList += strTemp;

			rs->MoveNext();
		}

		//take the last , off
		strNameList.TrimLeft();
		strNameList.TrimRight();
		strNameList = strNameList.Left(strNameList.GetLength() - 1);

		//now set the label
		pLbl->SetText(strNameList);
		*strList = strNameList;
	}
	else {
		//there is nothing in the array, so set the datalist to have no selection
		pLbl->ShowWindow(FALSE);
		GetDlgItem(nID)->ShowWindow(TRUE);
		pDatalist->CurSel = 0;
		*strList = "";
	}
}

void CNexWebApptInfoDlg::ParseDateTimeOnly(CString strDateTime, COleDateTime &dt) {

	//the format with be YYYY-MM-DD HH:MM:SS:000
	CString strHour, strMinute, strSecond, strTemp;
	long nResult;
	nResult = strDateTime.Find(" ");
	if (nResult > 0) {
		strHour = strDateTime.Mid(nResult + 1, 2);
		strMinute = strDateTime.Mid(nResult + 4, 2);
		strSecond = strDateTime.Mid(nResult + 7, 2);
		
		//we don't care about the date here
		dt.SetTime(atoi(strHour), atoi(strMinute), atoi(strSecond));
	}
}

void CNexWebApptInfoDlg::ProcessField(long ttFieldID, CString strEntry) {

	switch (ttFieldID) {
		case transTypeAppointment:
		case transTypeApptPatient:
			//we don't really have to do anything here
		break;
		case transTypeApptType:
			GetDlgItem(IDC_NEXWEB_APTTYPE_COMBO)->EnableWindow(TRUE);
			m_pTypeList->SetSelByColumn(0, (long)atoi(strEntry));
			if (m_pTypeList->CurSel == -1) {
				//set it to be {No Type}
				m_pTypeList->SetSelByColumn(0, (long)-1);
			}
			m_IDsToSave.Add(IDC_NEXWEB_APTTYPE_COMBO);
		break;
		case transTypeApptPurposeAdd:
			//we are going to handle this later, but save our selection at the now
			m_PurposeList.Add(atoi(strEntry));
			m_IDsToSave.Add(IDC_NEXWEB_APTPURPOSE_COMBO);
		break;
		case transTypeApptPurposeRemove: {
			//first we need to find the item
			for (int i = 0; i < m_PurposeList.GetSize(); i++) {
				if ((long)(m_PurposeList.GetAt(i)) == atoi(strEntry)) {
					m_PurposeList.RemoveAt(i);
				}
			}		
			m_IDsToSave.Add(IDC_NEXWEB_APTPURPOSE_COMBO);
										 }
		break;
		case transTypeApptResourceAdd:
			//save it in the list, we will fill the box later
			m_ResourceList.Add(atoi(strEntry));
			m_IDsToSave.Add(IDC_NEXWEB_APTRESOURCE_COMBO);
		break;
		case transTypeApptResourceRemove: {
			for (int i = 0; i < m_ResourceList.GetSize(); i++) {
				if (((long)m_ResourceList.GetAt(i)) == atoi(strEntry)) {
					m_ResourceList.RemoveAt(i);
				}
			}		
			m_IDsToSave.Add(IDC_NEXWEB_APTRESOURCE_COMBO);
										  }
		break;
		case transTypeApptNote:
			GetDlgItem(IDC_NEXWEB_NOTES_BOX)->EnableWindow(TRUE);
			SetDlgItemText(IDC_NEXWEB_NOTES_BOX, strEntry);
			m_IDsToSave.Add(IDC_NEXWEB_NOTES_BOX);
		break;
		case transTypeApptStartTime: {
			COleDateTime dt;
			//dt.ParseDateTime(strEntry);
			//parse date time isn't working, so lets parse it ourselves
			ParseDateTimeOnly(strEntry, dt);
			GetDlgItem(IDC_NEXWEB_START_TIME_BOX)->EnableWindow(TRUE);
			CString strTest = FormatDateTimeForSql(dt);
			m_pStartTime->SetDateTime(dt);
			m_IDsToSave.Add(IDC_NEXWEB_START_TIME_BOX);
									 }
		break;
		case transTypeApptEndTime: {
			COleDateTime dt;
			ParseDateTimeOnly(strEntry, dt);
			GetDlgItem(IDC_NEXWEB_END_TIME_BOX)->EnableWindow(TRUE);
			m_pEndTime->SetDateTime(dt);
			m_IDsToSave.Add(IDC_NEXWEB_END_TIME_BOX);
								   }
		break;
		case transTypeApptConfirmed:
			/*GetDlgItem(IDC_NEXWEB_CONFIRMED_COMBO)->EnableWindow(TRUE);
			m_Confirmed.SetCurSel(atoi(strEntry));
			m_IDsToSave.Add(IDC_NEXWEB_CONFIRMED_COMBO);*/
		break;
		case transTypeApptDate:
			GetDlgItem(IDC_EVENT_DATE_START)->EnableWindow(TRUE);
			m_ApptDate.SetValue(_variant_t(strEntry));
			m_IDsToSave.Add(IDC_EVENT_DATE_START);
		break;
		case transTypeApptMoveUp:
			
			// (j.gruber 2007-02-23 09:33) - PLID 24767 - change to datalist2
			GetDlgItem(IDC_NEXWEB_MOVE_UP_LIST)->EnableWindow(TRUE);			
			m_pMoveUpList->SetSelByColumn(0, (long)atoi(strEntry));
			//m_MoveUp.SetCurSel(atoi(strEntry));
			m_IDsToSave.Add(IDC_NEXWEB_MOVE_UP_LIST);
		break;
		case transTypeApptCancel:
			GetDlgItem(IDC_NEXWEB_CANCELLED_STATUS)->EnableWindow(TRUE);
			SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "CANCELLED");
			m_IDsToSave.Add(IDC_NEXWEB_CANCELLED_STATUS);
		break;
		case transTypeApptLocation: 
			GetDlgItem(IDC_NEXWEB_APTLOCATION_COMBO)->EnableWindow(TRUE);
			m_pLocationList->SetSelByColumn(0, (long)atoi(strEntry));
			m_IDsToSave.Add(IDC_NEXWEB_APTLOCATION_COMBO);
		break;

	}


}

BEGIN_EVENTSINK_MAP(CNexWebApptInfoDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebApptInfoDlg)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTLOCATION_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedNexwebAptlocationCombo, VTS_I2)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTPURPOSE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedNexwebAptpurposeCombo, VTS_I2)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTPURPOSE_COMBO, 16 /* SelChosen */, OnSelChosenNexwebAptpurposeCombo, VTS_I4)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTRESOURCE_COMBO, 16 /* SelChosen */, OnSelChosenNexwebAptresourceCombo, VTS_I4)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTRESOURCE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedNexwebAptresourceCombo, VTS_I2)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTTYPE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedNexwebApttypeCombo, VTS_I2)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_APTTYPE_COMBO, 16 /* SelChosen */, OnSelChosenNexwebApttypeCombo, VTS_I4)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_MOVE_UP_LIST, 16 /* SelChosen */, OnSelChosenNexwebMoveUpList, VTS_DISPATCH)
	ON_EVENT(CNexWebApptInfoDlg, IDC_NEXWEB_MOVE_UP_LIST, 1 /* SelChanging */, OnSelChangingNexwebMoveUpList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexWebApptInfoDlg::OnRequeryFinishedNexwebAptlocationCombo(short nFlags) 
{
	// TODO: Add your control notification handler code here
	
}

void CNexWebApptInfoDlg::OnRequeryFinishedNexwebAptpurposeCombo(short nFlags) 
{
	IRowSettingsPtr pRow = m_pPurposeList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, "{No Purpose}");
	m_pPurposeList->InsertRow(pRow, 0);

	pRow = m_pPurposeList->GetRow(-1);
	pRow->PutValue(0, (long)-2);
	pRow->PutValue(1, "{Multiple Purposes}");
	m_pPurposeList->InsertRow(pRow, 1);

	//check to see if there is nothing in the purpose list
	if (m_PurposeList.GetSize() == 0) {
		m_pPurposeList->CurSel = 0;
	}
	
}

void CNexWebApptInfoDlg::OnSelChosenNexwebAptpurposeCombo(long nRow) 
{
	long nValue = VarLong(m_pPurposeList->GetValue(nRow, 0));

	if (nValue == -2) {

		//multi purpose list
		OnMultiSelectPurposeList();
	}
	else {
		//clear the list
		m_PurposeList.RemoveAll();
		//don't add a {No Purpose} Value
		if (nValue != -1) {
			m_PurposeList.Add(nValue);
		}
	}
	
}


BOOL CNexWebApptInfoDlg::OnMultiSelectPurposeList()  {
	
	CString strResources, strWhere;
	long nTypeID;
	if (m_pTypeList->CurSel != -1) {
		nTypeID = m_pTypeList->GetValue(m_pTypeList->CurSel, 0);
	}
	else {
		nTypeID = -1;
	}
	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "AptPurposeT");
	dlg.PreSelect(m_PurposeList);
	if (m_ResourceList.GetSize() > 0)
	{
		for (long i=0; i < m_ResourceList.GetSize(); i++)
		{
			CString str;
			str.Format(",%d ", m_ResourceList.GetAt(i));
			strResources += str;
		}
		strResources = strResources.Right(strResources.GetLength() - 1);
		//TES 7/15/2004 - PLID 13466 - The purpose needs to be valid both by resource AND type.
		// (c.haag 2009-01-07 13:15) - PLID 32643 - Filter out inactive procedures
		strWhere.Format("AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
			" AND AptPurposeT.ID IN (select aptpurposeid from (select resourceid, aptpurposeid from resourcepurposetypet where apttypeid = %d group by resourceid, aptpurposeid) SubQ where SubQ.ResourceID in (%s) "
			" AND AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) "
			"group by aptpurposeid having count(aptpurposeid) = %d)", 
			nTypeID,
			strResources, 			
			nTypeID, m_ResourceList.GetSize());
	}
	else
	{
		// (c.haag 2004-04-01 16:12) - This should never happen but we should have safeguards.
		// (c.haag 2009-01-07 13:15) - PLID 32643 - Filter out inactive procedures
		strWhere.Format(
			"AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) AND "
			"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li)", 
			nTypeID);
	}

	if(IDOK == dlg.Open("AptPurposeT", strWhere, "AptPurposeT.ID", "AptPurposeT.Name", "Please select the procedures to associate with this appointment.", 1)) {
		dlg.FillArrayWithIDs(m_PurposeList);
		if(m_PurposeList.GetSize() > 1) {
			ShowDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO, SW_HIDE);
			m_strMultiPurpose = dlg.GetMultiSelectString();
			m_nxtPurposeLabel.SetText(m_strMultiPurpose);
			m_nxtPurposeLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MULTI_PURPOSE_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_MULTI_PURPOSE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO);
		}
		else if(m_PurposeList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO, SW_SHOW);
			ShowDlgItem(IDC_MULTI_PURPOSE_LIST, SW_HIDE);
			m_pPurposeList->SetSelByColumn(0, (long)m_PurposeList.GetAt(0));
			InvalidateDlgItem(IDC_MULTI_PURPOSE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO);
		}
		else {
			//They didn't select any.  But we told multiselect dlg they had to pick at least one!
			ASSERT(FALSE);
		}
		return TRUE;
		
	}
	else {
		//Check if they have "multiple" selected
		if(m_PurposeList.GetSize() > 1) {
			ShowDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO, SW_HIDE);
			m_nxtPurposeLabel.SetText(m_strMultiPurpose);
			m_nxtPurposeLabel.SetType(dtsHyperlink);
			
			ShowDlgItem(IDC_MULTI_PURPOSE_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_MULTI_PURPOSE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO);
		}
		else if(m_PurposeList.GetSize() == 1) {
			//They selected exactly one
			m_strMultiPurpose = "";
			ShowDlgItem(IDC_MULTI_PURPOSE_LIST, SW_HIDE);
			ShowDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO, SW_SHOW);
			m_pPurposeList->SetSelByColumn(0, (long)m_PurposeList.GetAt(0));
			InvalidateDlgItem(IDC_MULTI_PURPOSE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO);
		}
		else if(m_PurposeList.GetSize() == 0) {
			//They selected "<All Providers>"
			ShowDlgItem(IDC_MULTI_PURPOSE_LIST, SW_HIDE);
			ShowDlgItem(IDC_NEXWEB_APTPURPOSE_COMBO, SW_SHOW);
			m_strMultiPurpose = "";
			m_pPurposeList->SetSelByColumn(0, (long)-1);
		}
	}
	return FALSE;
}

BOOL CNexWebApptInfoDlg::OnMultiSelectResourceList()  {

	// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
	CMultiSelectDlg dlg(this, "ResourceT");
	dlg.PreSelect(m_ResourceList);

	CString strWhere = "Inactive = 0 OR ID IN (";
	if (m_ResourceList.GetSize() == 0) {
		strWhere += "-1";
	}
	for (int i=0; i < m_ResourceList.GetSize(); i++) {
		CString str;
		str.Format("%d,", m_ResourceList[i]);
		strWhere += str;
	}
	strWhere.TrimRight(',');
	strWhere += ")";
	if(IDOK == dlg.Open("ResourceT", strWhere, "ID", "Item", "Please select the resources to associate with this appointment.", 1)) {
		dlg.FillArrayWithIDs(m_ResourceList);
		if(m_ResourceList.GetSize() > 1) {
			ShowDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO, SW_HIDE);
			m_strMultiResource = dlg.GetMultiSelectString();
			m_nxtResourceLabel.SetText(m_strMultiResource);
			m_nxtResourceLabel.SetType(dtsHyperlink);
			ShowDlgItem(IDC_MULTI_RESOURCE_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_MULTI_RESOURCE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO);
		}
		else if(m_ResourceList.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO, SW_SHOW);
			ShowDlgItem(IDC_MULTI_RESOURCE_LIST, SW_HIDE);
			m_pResourceList->SetSelByColumn(0, (long)m_ResourceList.GetAt(0));
			InvalidateDlgItem(IDC_MULTI_RESOURCE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO);
		}
		else {
			//They didn't select any.  But we told multiselect dlg they had to pick at least one!
			ASSERT(FALSE);
		}
	
		return TRUE;
	}
	else {
		//Check if they have "multiple" selected
		if(m_ResourceList.GetSize() > 1) {
			ShowDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO, SW_HIDE);
			m_nxtResourceLabel.SetText(m_strMultiResource);
			m_nxtResourceLabel.SetType(dtsHyperlink);
			
			ShowDlgItem(IDC_MULTI_RESOURCE_LIST, SW_SHOW);
			InvalidateDlgItem(IDC_MULTI_RESOURCE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO);
		}
		else if(m_ResourceList.GetSize() == 1) {
			//They selected exactly one
			m_strMultiResource = "";
			ShowDlgItem(IDC_MULTI_RESOURCE_LIST, SW_HIDE);
			ShowDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO, SW_SHOW);
			m_pResourceList->SetSelByColumn(0, (long)m_ResourceList.GetAt(0));
			InvalidateDlgItem(IDC_MULTI_RESOURCE_LIST);
			InvalidateDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO);
		}
		else if(m_ResourceList.GetSize() == 0) {
			//They selected "<All Providers>"
			ShowDlgItem(IDC_MULTI_RESOURCE_LIST, SW_HIDE);
			ShowDlgItem(IDC_NEXWEB_APTRESOURCE_COMBO, SW_SHOW);
			m_strMultiResource = "";
			m_pResourceList->SetSelByColumn(0, (long)-1);
		}
	}

	return FALSE;
}

void CNexWebApptInfoDlg::OnSelChosenNexwebAptresourceCombo(long nRow) 
{
	long nValue = VarLong(m_pResourceList->GetValue(nRow, 0));


	if (nValue == -2) {
		//it takes care ofthe list for us

		//multi Resource list
		OnMultiSelectResourceList();
	}
	else {
		//first clear the resource list
		m_ResourceList.RemoveAll();
		//just add this one
		if (nValue != -1) {
			m_ResourceList.Add(nValue);
		}
	}

	
}

void CNexWebApptInfoDlg::OnRequeryFinishedNexwebAptresourceCombo(short nFlags) 
{
	IRowSettingsPtr pRow = m_pResourceList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, "{No Resource}");

	m_pResourceList->InsertRow(pRow, 0);

	pRow = m_pResourceList->GetRow(-1);
	pRow->PutValue(0, (long)-2);
	pRow->PutValue(1, "{Multiple Resources}");
	m_pResourceList->InsertRow(pRow, 1);
	
}

void CNexWebApptInfoDlg::OnRequeryFinishedNexwebApttypeCombo(short nFlags) 
{
	IRowSettingsPtr pRow = m_pTypeList->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, "{No Type}");

	m_pTypeList->InsertRow(pRow, 0);
	
}

void CNexWebApptInfoDlg::FillDWordArray(CDWordArray &aryToFill, CDWordArray &aryFillFrom) {

	//clear the list to be filled
	aryToFill.RemoveAll();

	for (int i=0; i < aryFillFrom.GetSize(); i++) {
		aryToFill.Add(aryFillFrom.GetAt(i));
	}

}

void CNexWebApptInfoDlg::OnImport() 
{
	try { 
		//make sure they have a resource
		// (a.walling 2007-11-06 17:35) - PLID 27998 - VS2008 - Er, we were checking if GetSize == 0, not GetSize()
		if (m_ResourceList.GetSize() == 0) {
			MsgBox("You must select at least one resource");
			return;
		}

		//make sure their start time is after their end time
		if (m_pEndTime->GetDateTime() <= m_pStartTime->GetDateTime()) {
			MsgBox("The end time must be after the start time");
			return;
		}

		//we need to save all the fields on the dialog to our pAppt value
		// (j.gruber 2007-02-23 09:34) - PLID 24767 - change to datalist2
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pMoveUpList->CurSel;
		if (pRow) {
			if (VarLong(pRow->GetValue(0)) == 1) {
				m_pAppt->bMoveUp = true;
			}
			else {
				m_pAppt->bMoveUp = false;
			}

		}
		m_pAppt->dtApptDate = COleDateTime(m_ApptDate.GetValue());
		m_pAppt->dtStartTime = m_pStartTime->GetDateTime();
		m_pAppt->dtEndTime = m_pEndTime->GetDateTime();
		FillDWordArray(m_pAppt->dwPurposeList, m_PurposeList);
		FillDWordArray(m_pAppt->dwResourceList, m_ResourceList);
		if (m_pLocationList->CurSel != -1) {
			m_pAppt->nLocationID = VarLong(m_pLocationList->GetValue(m_pLocationList->CurSel, 0), -1);
		}
		if (m_pTypeList->CurSel != -1) {
			m_pAppt->nTypeID = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel, 0));
		}
		m_pAppt->bHasBeenSaved = TRUE;
		GetDlgItemText(IDC_NEXWEB_NOTES_BOX, m_pAppt->strNotes);
		CString strCancelled;
		GetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, strCancelled);
		if (strCancelled.CompareNoCase("Cancelled") == 0) {
			m_pAppt->nStatus = 4;
		}
		else {
			m_pAppt->nStatus = 1;
		}
		
		CDialog::OnOK();
	}NxCatchAll("Error in CNexWebApptInfoDlg::OnImport");
			
}

void CNexWebApptInfoDlg::OnDelete() 
{
	//for now
	CDialog::OnCancel();
	
}

BOOL CNexWebApptInfoDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);


	if (GetDlgItem(IDC_MULTI_PURPOSE_LIST)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_PURPOSE_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	if (GetDlgItem(IDC_MULTI_RESOURCE_LIST)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_RESOURCE_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}


LRESULT CNexWebApptInfoDlg::OnLabelClick(WPARAM wParam, LPARAM lParam) {

	UINT nIdc = (UINT)wParam;
	switch(nIdc) {
		case IDC_MULTI_PURPOSE_LIST:
			if (GetDlgItem(IDC_MULTI_PURPOSE_LIST)->IsWindowVisible()) {
				//it visible so we can handle it
				if (OnMultiSelectPurposeList()) {
					//refresh the screen
					if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
						CWaitCursor cWait;
						GetMainFrame()->GetActiveView()->UpdateView();
					}
				}
			}

		break;

		case IDC_MULTI_RESOURCE_LIST:
			if (GetDlgItem(IDC_MULTI_RESOURCE_LIST)->IsWindowVisible()) {
				//it visible so we can handle it
				if (OnMultiSelectResourceList()) {
					//refresh the screen
					if (GetMainFrame() && GetMainFrame()->GetActiveView()) {
						CWaitCursor cWait;
						GetMainFrame()->GetActiveView()->UpdateView();
					}
				}
			}

		break;
	}
	
	return 0;

}

void CNexWebApptInfoDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (GetDlgItem(IDC_MULTI_PURPOSE_LIST)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_PURPOSE_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			OnMultiSelectPurposeList();
		}
	}

	if (GetDlgItem(IDC_MULTI_RESOURCE_LIST)->IsWindowVisible()) {
		GetDlgItem(IDC_MULTI_RESOURCE_LIST)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			OnMultiSelectResourceList();
		}
	}

	
	if (GetDlgItem(IDC_NEXWEB_CANCELLED_STATUS)->IsWindowVisible()) {
		GetDlgItem(IDC_NEXWEB_CANCELLED_STATUS)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			OnNexwebCancelledStatus();
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}


//Ripped from ResEntry with minor modifications
void CNexWebApptInfoDlg::RequeryApptPurposes() {

	CString strWhere;
	CString strResources;
	if (m_pTypeList->CurSel == -1) {
		return;
	}
	long nTypeID = VarLong(m_pTypeList->GetValue(m_pTypeList->CurSel, 0), -1);
	if (m_ResourceList.GetSize() > 0)
	{
		// (c.haag 2009-01-07 13:17) - PLID 32643 - Filter out inactive procedures
		strWhere.Format("AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) AND "
			"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) ", nTypeID);

		//2)  Loop through all selected resource and generate a query for all purposes allowed
		//	by this Resource / Type combination.  This is the Allowed Purposes on the
		//	resource editor (NexSpa only, but applies to all)
		for (long i=0; i < m_ResourceList.GetSize(); i++)
		{
			CString str;
			str.Format(" AND AptPurposeT.ID IN "
				"(SELECT AptPurposeID FROM ResourcePurposeTypeT WHERE AptTypeID = %li and ResourceID = %li GROUP BY ResourceID, AptPurposeID) ", 
				nTypeID, m_ResourceList[i]);

			strWhere += str;
		}
	}
	else
	{
		// (c.haag 2004-04-01 16:12) - This should never happen but we should have safeguards.
		// (c.haag 2009-01-07 13:18) - PLID 32643 - Filter out inactive procedures
		strWhere.Format(
			"AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) AND "
			"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li)", 
			nTypeID );
	}
	m_pPurposeList->WhereClause = _bstr_t(strWhere);

	// Requery the datalist because that's what this function does
	m_pPurposeList->Requery();

	ProcessMultiSelectList(&m_PurposeList, &m_strMultiPurpose, m_pPurposeList, IDC_NEXWEB_APTPURPOSE_COMBO, &m_nxtPurposeLabel, "Name", "AptPurposeT");
}

void CNexWebApptInfoDlg::OnSelChosenNexwebApttypeCombo(long nRow) 
{
	RequeryApptPurposes();
	
}

void CNexWebApptInfoDlg::OnNexwebCancelledStatus() 
{
	//toggle the cancelled status
	CString strCancelled;
	GetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, strCancelled);

	if (strCancelled.CompareNoCase("CANCELLED") == 0) {

		//make it say un cancelled
		SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "Not Cancelled");
	}
	else {
		//make it say cancelled
		SetDlgItemText(IDC_NEXWEB_CANCELLED_STATUS, "CANCELLED");
	}

	// (j.gruber 2006-11-13 12:29) - PLID 23154
	//make it save!!
	m_IDsToSave.Add(IDC_NEXWEB_CANCELLED_STATUS);
	
}


void CNexWebApptInfoDlg::OnCancel()
{
	CDialog::OnCancel();
}

void CNexWebApptInfoDlg::OnSelChosenNexwebMoveUpList(LPDISPATCH lpRow) 
{
}

void CNexWebApptInfoDlg::OnSelChangingNexwebMoveUpList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CNexWebApptInfoDlg::OnSelChangingNexwebMoveUpList");
	
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CNexWebApptInfoDlg::OnOK()
{
	//Eat the message
}