// PracticeDoc.cpp : implementation of the CPracticeDoc class
//

#include "stdafx.h"
#include "Practice.h"

//#include "PatientSet.h"
#include "PracticeDoc.h"
//#include "CntrItem.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPracticeDoc

IMPLEMENT_DYNCREATE(CPracticeDoc, COleDocument)

BEGIN_MESSAGE_MAP(CPracticeDoc, COleDocument)
	//{{AFX_MSG_MAP(CPracticeDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI(ID_OLE_VERB_FIRST, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_FILE_SEND_MAIL, OnFileSendMail)
	ON_UPDATE_COMMAND_UI(ID_FILE_SEND_MAIL, OnUpdateFileSendMail)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CPracticeDoc, COleDocument)
	//{{AFX_DISPATCH_MAP(CPracticeDoc)
	DISP_FUNCTION(CPracticeDoc, "AddPatient", AddPatient, VT_I4, VTS_DISPATCH)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IPractice to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {F2B94DA9-9A7D-11D1-B2C7-00001B4B970B}
static const IID IID_IPractice =
{ 0xf2b94da9, 0x9a7d, 0x11d1, { 0xb2, 0xc7, 0x0, 0x0, 0x1b, 0x4b, 0x97, 0xb } };

BEGIN_INTERFACE_MAP(CPracticeDoc, COleDocument)
	INTERFACE_PART(CPracticeDoc, IID_IPractice, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPracticeDoc construction/destruction

CPracticeDoc::CPracticeDoc()
{
	// Use OLE compound files
	EnableCompoundFile();

	// TODO: add one-time construction code here

	EnableAutomation();

	AfxOleLockApp();
}

CPracticeDoc::~CPracticeDoc()
{
	AfxOleUnlockApp();
}

BOOL CPracticeDoc::OnNewDocument()
{
/*
	char tmpPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, tmpPath);
	m_strDatabase = CString(tmpPath) ^ "Practice.mde";

	SetPracPath(m_strDatabase);
//*/

//	if (!m_rsActivePatient.IsOpen())
//		m_rsActivePatient.Close();									 
//	m_rsActivePatient.m_strSort = "[FullName]";
//	m_rsActivePatient.m_strFilter = "ID >=0";
//	m_rsActivePatient.Open();

	if (!COleDocument::OnNewDocument())
		return FALSE;

//	CMainFrame *tmpMainWnd = (CMainFrame *)AfxGetMainWnd();

//	if (tmpMainWnd) {
//		tmpMainWnd->m_pOpenDoc = this;
//		CString find;
//		find.Format ("ID = %i", tmpMainWnd->m_patToolBar.m_toolBarCombo.GetValue().lVal);
//		m_rsActivePatient.FindFirst(find);

		// Initialize the patient drop down
//		tmpMainWnd->m_patToolBar.Requery();
//		GetMainFrame()->GenerateFilters();
//		tmpMainWnd->m_patToolBar.m_toolBarCombo.SetValue((COleVariant)GetRemotePropertyInt("CurrentPatient", m_rsActivePatient.m_ID));
//		tmpMainWnd->m_patToolBar.SetActivePatientID(::GetRemotePropertyInt("CurrentPatient", m_rsActivePatient.m_ID, 0, ::GetCurrentUserName()));
//	}
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CPracticeDoc serialization

void CPracticeDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << CString("NexTech Practice");
		ar << (int)0; // Major
		ar << (int)1; // Minor
		ar << m_strDatabase;
	}
	else
	{
		CString appName;
		int vMajor, vMinor;
		
		ar >> appName;
		if (appName != "NexTech Practice") {
			MsgBox("This file, %s, is not a valid NexTech Practice document!", appName);
			//AfxMessageBox("This file is not a valid NexTech Practice document!");
			return;
		}
		
		ar >> vMajor;
		if (vMajor != 0) {
			AfxMessageBox("This file is from an older version of NexTech Practice!");
			return;
		}

		ar >> vMinor;
		ar >> m_strDatabase;

//		SetPracPath(m_strDatabase);
	}

	// Calling the base class COleDocument enables serialization
	//  of the container document's COleClientItem objects.
	COleDocument::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CPracticeDoc diagnostics

#ifdef _DEBUG
void CPracticeDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CPracticeDoc::Dump(CDumpContext& dc) const
{
	COleDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPracticeDoc commands

void CPracticeDoc::OnCloseDocument() 
{

	COleDocument::OnCloseDocument();
}

BOOL CPracticeDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!COleDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	

	return TRUE;
}

long CPracticeDoc::AddPatient(LPDISPATCH pNewPatient) 
{
	// TODO: Add your dispatch handler code here

	return 0;
}
