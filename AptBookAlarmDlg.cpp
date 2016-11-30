// AptBookAlarmDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AptBookAlarmDlg.h"
#include "AptBookAlarmDetailDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmDlg dialog


CAptBookAlarmDlg::CAptBookAlarmDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAptBookAlarmDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAptBookAlarmDlg)		
	//}}AFX_DATA_INIT

	m_ID = -1;
	m_bIsNew = TRUE;

	m_nDaysBefore = 30;
	m_nDaysAfter = 30;
	m_rcDaysBeforeLabel.top = m_rcDaysBeforeLabel.bottom = m_rcDaysBeforeLabel.left = m_rcDaysBeforeLabel.right = 0;
	m_rcDaysBeforeNumber.top = m_rcDaysBeforeNumber.bottom = m_rcDaysBeforeNumber.left = m_rcDaysBeforeNumber.right = 0;
	m_rcDaysAfterLabel.top = m_rcDaysAfterLabel.bottom = m_rcDaysAfterLabel.left = m_rcDaysAfterLabel.right = 0;
	m_rcDaysAfterNumber.top = m_rcDaysAfterNumber.bottom = m_rcDaysAfterNumber.left = m_rcDaysAfterNumber.right = 0;

}


void CAptBookAlarmDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAptBookAlarmDlg)
	DDX_Control(pDX, IDC_CHECK_INCLUDE_NO_SHOWS, m_checkIncludeNoShows);
	DDX_Control(pDX, IDC_CHECK_ALLOW_SAVE, m_checkAllowSave);
	DDX_Control(pDX, IDC_CHECK_SAME_TYPE, m_checkSameType);
	DDX_Control(pDX, IDC_CHECK_SAME_PURPOSE, m_checkSamePurpose);
	DDX_Control(pDX, IDC_CHECK_SAME_RESOURCE, m_checkSameResource);
	DDX_Control(pDX, IDC_CHECK_APT_AFTER, m_checkAptAfter);
	DDX_Control(pDX, IDC_CHECK_APT_BEFORE, m_checkAptBefore);
	DDX_Control(pDX, IDC_EDIT_ALARM_DESCRIPTION, m_nxeditEditAlarmDescription);
	DDX_Control(pDX, IDC_LABEL_DAYS_BEFORE, m_nxstaticLabelDaysBefore);
	DDX_Control(pDX, IDC_LABEL_DAYS_AFTER, m_nxstaticLabelDaysAfter);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_APT_ALARM_DETAIL_BTN, m_btnAddAptAlarmDetail);
	DDX_Control(pDX, IDC_EDIT_APT_ALARM_DETAIL_BTN, m_btnEditAptAlarmDetail);
	DDX_Control(pDX, IDC_REMOVE_APT_ALARM_DETAIL_BTN, m_btnRemoveAptAlarmDetail);
	DDX_Control(pDX, IDC_WARN_OPTIONS_GROUPBOX, m_btnWarnOptionsGroupbox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAptBookAlarmDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAptBookAlarmDlg)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_CHECK_APT_BEFORE, OnCheckAptBefore)
	ON_BN_CLICKED(IDC_CHECK_APT_AFTER, OnCheckAptAfter)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ADD_APT_ALARM_DETAIL_BTN, OnAddAptAlarmDetailBtn)
	ON_BN_CLICKED(IDC_EDIT_APT_ALARM_DETAIL_BTN, OnEditAptAlarmDetailBtn)
	ON_BN_CLICKED(IDC_REMOVE_APT_ALARM_DETAIL_BTN, OnRemoveAptAlarmDetailBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAptBookAlarmDlg message handlers

BOOL CAptBookAlarmDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// Calculate hyperlink rectangles
	{
		CWnd *pWnd;

		pWnd = GetDlgItem(IDC_LABEL_DAYS_BEFORE);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the is hotlinks
			pWnd->GetWindowRect(m_rcDaysBeforeLabel);
			ScreenToClient(&m_rcDaysBeforeLabel);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
		
		pWnd = GetDlgItem(IDC_LABEL_DAYS_AFTER);
		if (pWnd->GetSafeHwnd()) {
			// Get the position of the is hotlinks
			pWnd->GetWindowRect(m_rcDaysAfterLabel);
			ScreenToClient(&m_rcDaysAfterLabel);

			// Hide the static text that was there
			pWnd->ShowWindow(SW_HIDE);
		}
	}

	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnAddAptAlarmDetail.AutoSet(NXB_NEW);
	m_btnEditAptAlarmDetail.AutoSet(NXB_MODIFY);
	m_btnRemoveAptAlarmDetail.AutoSet(NXB_DELETE);

	m_AlarmDetailList = BindNxDataListCtrl(this,IDC_APT_BOOK_ALARM_DETAIL_LIST,GetRemoteData(),false);

	m_checkAllowSave.SetCheck(TRUE);

	if(!m_bIsNew)
		Load();

	RefreshButtons();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAptBookAlarmDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CDialog::OnLButtonDown(nFlags, point);
}

void CAptBookAlarmDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	DoClickHyperlink(nFlags, point);
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CAptBookAlarmDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	if (IsDlgButtonChecked(IDC_CHECK_APT_BEFORE)) {
		if (m_rcDaysBeforeNumber.PtInRect(point)) {
			CString strCount;
			strCount.Format("%li", m_nDaysBefore);
			if (InputBox(this, "Days to check before the appointment", strCount, "", false, false, NULL, TRUE) == IDOK) {
				m_nDaysBefore = atol(strCount);
				InvalidateRect(m_rcDaysBeforeLabel);
			}
		}	
	}
	if (IsDlgButtonChecked(IDC_CHECK_APT_AFTER)) {
		if (m_rcDaysAfterNumber.PtInRect(point)) {
			CString strCount;
			strCount.Format("%li", m_nDaysAfter);
			if (InputBox(this, "Days to check after the appointment", strCount, "", false, false, NULL, TRUE) == IDOK) {
				m_nDaysAfter = atol(strCount);
				InvalidateRect(m_rcDaysAfterLabel);
			}
		}	
	}	
}

void CAptBookAlarmDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	DrawBeforeLabel(&dc);
	DrawAfterLabel(&dc);
}

void CAptBookAlarmDlg::DrawBeforeLabel(CDC *pdc)
{
	BOOL bSelectionEnabled = IsDlgButtonChecked(IDC_CHECK_APT_BEFORE);

	// Draw the day #
	{
		m_rcDaysBeforeNumber = m_rcDaysBeforeLabel;
		CString strCount;
		strCount.Format("%li", m_nDaysBefore);
		//DRT 4/29/2008 - PLID 29823 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcDaysBeforeNumber, strCount, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}

	// Draw the text: " days before the new appointment"
	CRect rc(m_rcDaysBeforeLabel);
	rc.left = m_rcDaysBeforeNumber.right;
	//DRT 4/29/2008 - PLID 29823 - Set background color to transparent
	DrawTextOnDialog(this, pdc, rc, " days before the new appointment", dtsText, true, DT_LEFT, true, false, 0);
}

void CAptBookAlarmDlg::DrawAfterLabel(CDC *pdc)
{
	BOOL bSelectionEnabled = IsDlgButtonChecked(IDC_CHECK_APT_AFTER);

	// Draw the day #
	{
		m_rcDaysAfterNumber = m_rcDaysAfterLabel;
		CString strCount;
		strCount.Format("%li", m_nDaysAfter);
		//DRT 4/29/2008 - PLID 29823 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcDaysAfterNumber, strCount, bSelectionEnabled?dtsHyperlink:dtsDisabledHyperlink, true, DT_LEFT, true, false, 0);
	}

	// Draw the text: " days after the new appointment"
	CRect rc(m_rcDaysAfterLabel);
	rc.left = m_rcDaysAfterNumber.right;
	//DRT 4/29/2008 - PLID 29823 - Set background color to transparent
	DrawTextOnDialog(this, pdc, rc, " days after the new appointment", dtsText, true, DT_LEFT, true, false, 0);
}

BOOL CAptBookAlarmDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	if (IsDlgButtonChecked(IDC_CHECK_APT_BEFORE)) {
		if (m_rcDaysBeforeNumber.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	if (IsDlgButtonChecked(IDC_CHECK_APT_AFTER)) {
		if (m_rcDaysAfterNumber.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}

	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CAptBookAlarmDlg::OnCheckAptBefore() 
{
	InvalidateRect(m_rcDaysBeforeLabel);	
}

void CAptBookAlarmDlg::OnCheckAptAfter() 
{
	InvalidateRect(m_rcDaysAfterLabel);	
}

void CAptBookAlarmDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
}

void CAptBookAlarmDlg::Load()
{
	try {

		_RecordsetPtr rs = CreateRecordset("SELECT Description, CheckBefore, DaysBefore, CheckAfter, DaysAfter, "
				"AllowSave, SameType, SamePurpose, SameResource, IncludeNoShows FROM AptBookAlarmT WHERE ID = %li",m_ID);

		if(!rs->eof) {

			//Description
			CString strDescription = AdoFldString(rs, "Description","");
			SetDlgItemText(IDC_EDIT_ALARM_DESCRIPTION,strDescription);

			//CheckBefore
			m_checkAptBefore.SetCheck(AdoFldBool(rs, "CheckBefore",FALSE));

			//DaysBefore
			m_nDaysBefore = AdoFldLong(rs, "DaysBefore",m_nDaysBefore);

			//CheckAfter
			m_checkAptAfter.SetCheck(AdoFldBool(rs, "CheckAfter",FALSE));

			//DaysAfter
			m_nDaysAfter = AdoFldLong(rs, "DaysAfter",m_nDaysAfter);
			
			//AllowSave
			m_checkAllowSave.SetCheck(AdoFldBool(rs, "AllowSave",TRUE));

			//SameType
			m_checkSameType.SetCheck(AdoFldBool(rs, "SameType",FALSE));

			//SamePurpose
			m_checkSamePurpose.SetCheck(AdoFldBool(rs, "SamePurpose",FALSE));

			//SameResource
			m_checkSameResource.SetCheck(AdoFldBool(rs, "SameResource",FALSE));

			//IncludeNoShows
			m_checkIncludeNoShows.SetCheck(AdoFldBool(rs, "IncludeNoShows",FALSE));

			CString str;
			str.Format("AptBookAlarmID = %li",m_ID);
			m_AlarmDetailList->PutWhereClause(_bstr_t(str));
			m_AlarmDetailList->Requery();
		}
		rs->Close();
		RefreshButtons();

	}NxCatchAll("Error loading alarm information.");
}

BOOL CAptBookAlarmDlg::Save()
{
	try {

		//Description
		CString strDescription;
		GetDlgItemText(IDC_EDIT_ALARM_DESCRIPTION,strDescription);

		//description cannot be blank and should not be a duplicate
		if(strDescription.GetLength() == 0 ||
			!IsRecordsetEmpty("SELECT ID FROM AptBookAlarmT WHERE Description = '%s' AND ID <> %li",_Q(strDescription),m_ID)) {
			AfxMessageBox("Please enter a unique description for this alarm.");
			return FALSE;
		}

		//CheckBefore
		long nCheckBefore = 0;
		if(m_checkAptBefore.GetCheck())
			nCheckBefore = 1;

		//DaysBefore
		//m_nDaysBefore

		//CheckAfter
		long nCheckAfter = 0;
		if(m_checkAptAfter.GetCheck())
			nCheckAfter = 1;

		if(nCheckBefore == 0 && nCheckAfter == 0) {
			AfxMessageBox("An alarm must check for appointments either before or after the new appointment.\n"
				"This alarm cannot be saved until one or both of these choices have been selected.");
			return FALSE;
		}

		//DaysAfter
		//m_nDaysAfter

		//AllowSave
		long nAllowSave = 0;
		if(m_checkAllowSave.GetCheck())
			nAllowSave = 1;

		//SameType
		long nSameType = 0;
		if(m_checkSameType.GetCheck())
			nSameType = 1;

		//SamePurpose
		long nSamePurpose = 0;
		if(m_checkSamePurpose.GetCheck())
			nSamePurpose = 1;

		//SameResource
		long nSameResource = 0;
		if(m_checkSameResource.GetCheck())
			nSameResource = 1;

		//IncludeNoShows
		long nIncludeNoShows = 0;
		if(m_checkIncludeNoShows.GetCheck())
			nIncludeNoShows = 1;

		if(m_ID == -1) {
			//save new
			ExecuteSql("INSERT INTO AptBookAlarmT (ID, Description, CheckBefore, DaysBefore, CheckAfter, DaysAfter, "
				"AllowSave, SameType, SamePurpose, SameResource, IncludeNoShows) "
				"VALUES (%li, '%s', %li, %li, %li, %li, %li, %li, %li, %li, %li)",m_ID = NewNumber("AptBookAlarmT","ID"),_Q(strDescription),
				nCheckBefore,m_nDaysBefore,nCheckAfter,m_nDaysAfter,nAllowSave,nSameType,nSamePurpose,nSameResource,nIncludeNoShows);

		}
		else {
			//update existing record

			ExecuteSql("UPDATE AptBookAlarmT SET Description = '%s', CheckBefore = %li, DaysBefore = %li, "
				"CheckAfter = %li, DaysAfter = %li, AllowSave = %li, SameType = %li, SamePurpose = %li, SameResource = %li, "
				"IncludeNoShows = %li WHERE ID = %li",_Q(strDescription), nCheckBefore,m_nDaysBefore,nCheckAfter,m_nDaysAfter,
				nAllowSave,nSameType,nSamePurpose,nSameResource,nIncludeNoShows,m_ID);

		}

		//in either case, wipe out the details and save them new, each time
		ExecuteSql("DELETE FROM AptBookAlarmDetailsT WHERE AptBookAlarmID = %li",m_ID);

		for(int i=0;i<m_AlarmDetailList->GetRowCount();i++) {
			long nAptTypeID = VarLong(m_AlarmDetailList->GetValue(i,1),-1);
			long nAptPurposeID = VarLong(m_AlarmDetailList->GetValue(i,3),-1);
			CString strAptTypeID = "NULL";
			CString strAptPurposeID = "NULL";
			if(nAptTypeID != -1)
				strAptTypeID.Format("%li",nAptTypeID);
			if(nAptPurposeID != -1)
				strAptPurposeID.Format("%li",nAptPurposeID);

			ExecuteSql("INSERT INTO AptBookAlarmDetailsT (ID, AptBookAlarmID, AptTypeID, AptPurposeID) "
				"VALUES (%li, %li, %s, %s)",NewNumber("AptBookAlarmDetailsT","ID"),m_ID,strAptTypeID,strAptPurposeID);
		}

		return TRUE;

	}NxCatchAll("Error saving alarm information.");

	return FALSE;
}

void CAptBookAlarmDlg::OnOK() 
{

	if(!Save())
		return;
	
	CDialog::OnOK();
}

void CAptBookAlarmDlg::OnAddAptAlarmDetailBtn() 
{
	try {

		CAptBookAlarmDetailDlg dlg(this);
		if(IDOK == dlg.DoModal()) {
			//add the record to the list			
			PostEditDetail(-1,dlg.m_AptTypeID,dlg.m_AptPurposeID);
		}
		RefreshButtons();
	}NxCatchAll("Error adding new detail.");
}

void CAptBookAlarmDlg::OnEditAptAlarmDetailBtn() 
{
	try {

		if(m_AlarmDetailList->GetCurSel() == -1) {
			AfxMessageBox("Please select an item from the list.");
			return;
		}

		CAptBookAlarmDetailDlg dlg(this);
		dlg.m_AptTypeID = VarLong(m_AlarmDetailList->GetValue(m_AlarmDetailList->GetCurSel(),1),-1);
		dlg.m_AptPurposeID = VarLong(m_AlarmDetailList->GetValue(m_AlarmDetailList->GetCurSel(),3),-1);
		if(IDOK == dlg.DoModal()) {
			//update the record in the list
			PostEditDetail(m_AlarmDetailList->GetCurSel(),dlg.m_AptTypeID,dlg.m_AptPurposeID);
		}
		RefreshButtons();
		
	}NxCatchAll("Error editing detail.");	
}

void CAptBookAlarmDlg::OnRemoveAptAlarmDetailBtn() 
{
	try {

		if(m_AlarmDetailList->GetCurSel() == -1) {
			AfxMessageBox("Please select an item from the list.");
			return;
		}

		if(IDYES == MessageBox("Are you sure you with to permanently delete this alarm detail?","Practice",MB_ICONQUESTION|MB_YESNO)) {
			m_AlarmDetailList->RemoveRow(m_AlarmDetailList->GetCurSel());
		}
		RefreshButtons();
		
	}NxCatchAll("Error deleting new detail.");
}

BEGIN_EVENTSINK_MAP(CAptBookAlarmDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAptBookAlarmDlg)
	ON_EVENT(CAptBookAlarmDlg, IDC_APT_BOOK_ALARM_DETAIL_LIST, 3 /* DblClickCell */, OnDblClickCellAptBookAlarmDetailList, VTS_I4 VTS_I2)
	ON_EVENT(CAptBookAlarmDlg, IDC_APT_BOOK_ALARM_DETAIL_LIST, 2 /* SelChanged */, OnSelChangedAptBookAlarmDetailList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAptBookAlarmDlg::OnDblClickCellAptBookAlarmDetailList(long nRowIndex, short nColIndex) 
{
	OnEditAptAlarmDetailBtn();
}

void CAptBookAlarmDlg::OnCancel() 
{	
	CDialog::OnCancel();
}

void CAptBookAlarmDlg::PostEditDetail(long nRow, long nAptTypeID, long nAptPurposeID)
{	
	try {

		BOOL bNewRow = FALSE;

		if(nRow == -1)
			bNewRow = TRUE;

		CString strAptTypeName, strAptPurposeName;
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM AptTypeT WHERE ID = %li",nAptTypeID);
		if(!rs->eof) {
			strAptTypeName = AdoFldString(rs, "Name","<All Types>");
		}
		else {
			strAptTypeName = "<All Types>";
		}
		rs->Close();

		rs = CreateRecordset("SELECT Name FROM AptPurposeT WHERE ID = %li",nAptPurposeID);
		if(!rs->eof) {
			strAptPurposeName = AdoFldString(rs, "Name","<All Purposes>");
		}
		else {
			strAptPurposeName = "<All Purposes>";
		}
		rs->Close();

		IRowSettingsPtr pRow = m_AlarmDetailList->GetRow(nRow);
		if(bNewRow)
			pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,nAptTypeID);
		pRow->PutValue(2,_bstr_t(strAptTypeName));
		pRow->PutValue(3,nAptPurposeID);
		pRow->PutValue(4,_bstr_t(strAptPurposeName));
		if(bNewRow)
			m_AlarmDetailList->AddRow(pRow);
		RefreshButtons();

	}NxCatchAll("Error in PostEditDetail()");
}

void CAptBookAlarmDlg::OnSelChangedAptBookAlarmDetailList(long nNewSel) 
{
	RefreshButtons();	
}

void CAptBookAlarmDlg::RefreshButtons()
{
	if(m_AlarmDetailList->GetCurSel() == sriNoRow){
		GetDlgItem(IDC_EDIT_APT_ALARM_DETAIL_BTN)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_APT_ALARM_DETAIL_BTN)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_EDIT_APT_ALARM_DETAIL_BTN)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_APT_ALARM_DETAIL_BTN)->EnableWindow(TRUE);
	}
}
