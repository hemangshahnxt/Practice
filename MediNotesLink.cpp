// MediNotesLink.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "MediNotesLink.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CMediNotesLink dialog


CMediNotesLink::CMediNotesLink(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMediNotesLink::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMediNotesLink)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMediNotesLink::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMediNotesLink)
	DDX_Control(pDX, IDC_EXPORT_BTN, m_btnExport);
	DDX_Control(pDX, IDC_PRAC_REMOVE_ALL, m_btnPracRemoveAll);
	DDX_Control(pDX, IDC_PRAC_REMOVE, m_btnPracRemove);
	DDX_Control(pDX, IDC_PRAC_ADD, m_btnPracAdd);
	DDX_Control(pDX, IDC_NEXTECH_COUNT, m_nxstaticNextechCount);
	DDX_Control(pDX, IDC_NEXTECH_COUNT2, m_nxstaticNextechCount2);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMediNotesLink, CNxDialog)
	//{{AFX_MSG_MAP(CMediNotesLink)
	ON_BN_CLICKED(IDC_EXPORT_BTN, OnExportBtn)
	ON_BN_CLICKED(IDC_PRAC_ADD, OnPracAdd)
	ON_BN_CLICKED(IDC_PRAC_REMOVE, OnPracRemove)
	ON_BN_CLICKED(IDC_PRAC_REMOVE_ALL, OnPracRemoveAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMediNotesLink message handlers

BOOL CMediNotesLink::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_nextechList = BindNxDataListCtrl(this,IDC_NEXTECH,GetRemoteData(),true);
	m_exportList = BindNxDataListCtrl(this,IDC_EXPORT,GetRemoteData(),false);
	
	m_btnPracAdd.AutoSet(NXB_RIGHT);
	m_btnPracRemove.AutoSet(NXB_LEFT);
	m_btnPracRemoveAll.AutoSet(NXB_LLEFT);
	m_btnExport.AutoSet(NXB_EXPORT);
	m_btnClose.AutoSet(NXB_CLOSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CMediNotesLink, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CMediNotesLink)
	ON_EVENT(CMediNotesLink, IDC_NEXTECH, 3 /* DblClickCell */, OnDblClickCellNextech, VTS_I4 VTS_I2)
	ON_EVENT(CMediNotesLink, IDC_EXPORT, 3 /* DblClickCell */, OnDblClickCellExport, VTS_I4 VTS_I2)
	ON_EVENT(CMediNotesLink, IDC_NEXTECH, 18 /* RequeryFinished */, OnRequeryFinishedNextech, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMediNotesLink::OnDblClickCellNextech(long nRowIndex, short nColIndex) 
{
	m_nextechList->CurSel = nRowIndex;
	OnPracAdd();
}

void CMediNotesLink::OnDblClickCellExport(long nRowIndex, short nColIndex) 
{
	m_exportList->CurSel = nRowIndex;
	OnPracRemove();
}

void CMediNotesLink::OnExportBtn() 
{	
	try {

		if(m_exportList->GetRowCount() == 0) {
			AfxMessageBox("Please select one or more patients before exporting.");
			return;
		}

		// (j.armen 2011-10-25 14:07) - PLID 46137 - We are prompting the user where to save, so the Practice path is safe
		CFileDialog SaveAs(FALSE,NULL,"ImpPat.txt");
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		CString dir = GetEnvironmentDirectory();
		SaveAs.m_ofn.lpstrInitialDir = (LPCSTR)dir;
		SaveAs.m_ofn.lpstrFilter = "Text Files\0*.txt\0All Files\0*.*\0\0";
		if (SaveAs.DoModal() == IDCANCEL) return;
		CString strFileName = SaveAs.GetPathName();

		//open the file for writing

		CFile OutputFile;
		if(!OutputFile.Open(strFileName,CFile::modeCreate|CFile::modeWrite|CFile::shareExclusive)) {
			AfxMessageBox("The export file could not be created. Contact Nextech for assistance.");
			return;
		}

		CWaitCursor pWait;

		for(int i=0; i<m_exportList->GetRowCount(); i++) {
			long PatientID = m_exportList->GetValue(i,3).lVal;
			CString strExport = GetExportString(PatientID);
			OutputFile.Write(strExport,strExport.GetLength());
		}

		OutputFile.Close();

		AfxMessageBox("Export complete. You may now import the file '" + strFileName + "' into MediNotes.");

	}NxCatchAll("Error exporting patients to MediNotes.");
}

CString CMediNotesLink::GetExportString(long PatientID) {

	try {

		CString strExportString = "", str = "";
		_variant_t var;

		_RecordsetPtr rs = CreateRecordset("SELECT Last, First, Middle, UserDefinedID, Address1, Address2, City, State, Zip, "
			"SocialSecurity, HomePhone, WorkPhone, BirthDate, Gender "
			"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PersonT.ID = %li",PatientID);

		if(rs->eof) {
			rs->Close();
			return "";
		}

		//format this patient's data the way MediNotes requires

		//Field			Max. Size	Notes		
		
		//LastName		255			Patient's last name.  No commas or apostrophes.
		var = rs->Fields->Item["Last"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.Replace(","," ");
		str.Replace("'","");
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";
		
		
		//FirstName		255			Patient's first name.  No commas or apostrophes.
		var = rs->Fields->Item["First"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.Replace(","," ");
		str.Replace("'","");
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//MiddleName	255			Patient's middle name.
		var = rs->Fields->Item["Middle"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//ChartNo		255			Patient's chart number.  No commas or apostrophes but can be alphanumeric.
		var = rs->Fields->Item["UserDefinedID"]->Value;
		if(var.vt == VT_I4)
			str.Format("%li",var.lVal);
		else
			str = "";
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//Address		255			Patient's street address.
		var = rs->Fields->Item["Address1"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//City			255			Patient's city.
		var = rs->Fields->Item["City"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//State			255			Patient's state.
		var = rs->Fields->Item["State"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//ZipCode		255			Patient's zip code.
		var = rs->Fields->Item["Zip"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//SSN	Text	255			Patient's social security number.
		var = rs->Fields->Item["SocialSecurity"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//HomePhone		255			Patient's home phone (format:  000-000-0000).
		var = rs->Fields->Item["HomePhone"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.Replace("(","");
		str.Replace(")","-");
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//WorkPhone		255			Patient's work phone (format: 000-000-0000).
		var = rs->Fields->Item["WorkPhone"]->Value;
		if(var.vt == VT_BSTR)
			str = CString(var.bstrVal);
		else
			str = "";
		str.Replace("(","");
		str.Replace(")","-");
		str.TrimRight();
		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//BirthDate		10			Patient's date of birth (format:  mm-dd-yyyy).
		var = rs->Fields->Item["BirthDate"]->Value;
		if(var.vt == VT_DATE) {
			COleDateTime dt = var.date;
			str = dt.Format("%m-%d-%Y");
		}
		else
			str = "";
		if(str.GetLength()>10)
			str = str.Left(10);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//Sex			255			Patient's sex (male or female).
		var = rs->Fields->Item["Gender"]->Value;
		if(VarByte(var,0) == 1)
			str = "male";
		else if(VarByte(var,0) == 2)
			str = "female";
		else
			str = "";

		if(str.GetLength()>255)
			str = str.Left(255);
		strExportString += ("\"" + str + "\"");
		strExportString += ",";

		//Reserved (Weight)		255	Reserved for patient's weight.
		strExportString += "\"\",";

		//Reserved (Height)		255	Reserved for patient's height.
		strExportString += "\"\",";
		
		//Reserved (Race)		255	Reserved for patient's race.
		strExportString += "\"\",";

		//Reserved (EntryDate)	10	Reserved for entry date (format:  mm-dd-yyyy).
		strExportString += "\"\",";

		//Reserved (Comments)	-	Reserved user-defined with unlimited size.
		strExportString += "\"\",";

		//Reserved (PhyLast)	255	Reserved for family Doctor's last name.
		strExportString += "\"\",";

		//Reserved (PhyFirst)	255	Reserved for family Doctor's middle name.
		strExportString += "\"\",";

		//Reserved (PhyMiddle)	255	Reserved for family Doctor's first name.
		strExportString += "\"\",";

		//Reserved1		255			Reserved for future use.
		strExportString += "\"\",";

		//Reserved2		255			Reserved for future use.
		strExportString += "\"\",";

		//Reserved3		255			Reserved for future use.
		strExportString += "\"\"";
		
		strExportString += "\r\n";

		rs->Close();

		return strExportString;

	}NxCatchAll("Error generating patient export.");

	return "";
}

void CMediNotesLink::OnPracAdd() 
{
	int curSel = m_nextechList->CurSel;

	if (curSel != -1)
		m_exportList->TakeCurrentRow(m_nextechList);

	UpdateCount();
}

void CMediNotesLink::OnPracRemove() 
{
	int curSel = m_exportList->CurSel;

	if (curSel != -1)
		m_nextechList->TakeCurrentRow(m_exportList);

	UpdateCount();
}

void CMediNotesLink::OnPracRemoveAll() 
{
	try
	{
		long p = m_exportList->GetFirstRowEnum();
		LPDISPATCH pRow = NULL;
		
		while (p)
		{
			m_exportList->GetNextRowEnum(&p, &pRow);
			m_nextechList->TakeRow(pRow);
			pRow->Release();
		}
	}
	NxCatchAll("Could not remove all rows from export list");

	UpdateCount();
}

void CMediNotesLink::UpdateCount()
{
	CRect rect;
	CString str;

	if (!m_nextechList->IsRequerying())
	{
		str.Format("         %d", m_nextechList->GetRowCount());
		GetDlgItem(IDC_NEXTECH_COUNT)->SetWindowText(str);
	}

	str.Format("         %d", m_exportList->GetRowCount());
	GetDlgItem(IDC_NEXTECH_COUNT2)->SetWindowText(str);
}

void CMediNotesLink::OnRequeryFinishedNextech(short nFlags) 
{
	CString str;
	str.Format("         %d", m_nextechList->GetRowCount());
	GetDlgItem(IDC_NEXTECH_COUNT)->SetWindowText(str);
}
