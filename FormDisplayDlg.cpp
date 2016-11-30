// FormDisplayDlg.cpp : implementation file
//
#include "stdafx.h"
#include "FormDisplayDlg.h"
#include "FormEdit.h"
#include "FormLine.h"
#include "FormDate.h"
#include "FormCheck.h"
#include "FormFormat.h"
#include "FormLayer.h"
#include "FormQuery.h"
#include "PracProps.h"
#include <PrintUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

double PRINT_X_SCALE;		//13.87//14.6//15.1
double PRINT_Y_SCALE;		//13.87//14.6//15.1
double PRINT_X_OFFSET;		//-350//-400//-100
double PRINT_Y_OFFSET;		//400//100
int	   FONT_SIZE;
int	   MINI_FONT_SIZE;

//#define magic 187

CString xxx (int i)
{
	CString x = "";
	while (i--)
		x+='x';
	return x;
}

// (j.jones 2007-06-22 09:28) - PLID 25665 - used to color the background of an edited field
#define EDITED_FIELD_COLOR	RGB(255,243,53)

/////////////////////////////////////////////////////////////////////////////
// CFormDisplayDlg dialog
CFormDisplayDlg::CFormDisplayDlg(int ID, CWnd* pParent /*=NULL*/)
	: CDialog(ID, pParent)
{
	//{{AFX_DATA_INIT(CFormdesignDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pFont = 0;
	m_pOnCommand = NULL;
	m_pOnKeyDown = NULL;
	m_pPrePrintFunc = NULL;
	m_ShowPrintDialog = TRUE;
	m_ChangeKey1 = m_ChangeKey2 = -1;
	m_rsHistory.CreateInstance(__uuidof(Recordset));
	m_bColorEditedFields = TRUE;
	// (j.jones 2007-06-25 10:38) - PLID 25663 - renamed this variable
	m_bFieldsEdited = FALSE;
}


CFormDisplayDlg::CFormDisplayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFormDisplayDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFormDisplayDlg)
	//}}AFX_DATA_INIT
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pFont = 0;
	m_pOnCommand = NULL;
	m_pOnKeyDown = NULL;
	m_pPrePrintFunc = NULL;
	m_ShowPrintDialog = TRUE;
	m_ChangeKey1 = m_ChangeKey2 = -1;
	m_rsHistory.CreateInstance(__uuidof(Recordset));
	m_bColorEditedFields = TRUE;
	// (j.jones 2007-06-25 10:38) - PLID 25663 - renamed this variable
	m_bFieldsEdited = FALSE;
}

CFormDisplayDlg::~CFormDisplayDlg()
{
	// (j.armen 2014-03-27 16:28) - PLID 60784 - These are arrays of smart pointers now
	m_ControlArray.clear();
	m_LayerArray.clear();
	m_pChangeArray.clear();
	if (m_rsHistory->State != adStateClosed)
		m_rsHistory->Close();
}

void CFormDisplayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormDisplayDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFormDisplayDlg, CDialog)
	//{{AFX_MSG_MAP(CFormDisplayDlg)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_KILLFOCUS()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormDisplayDlg message handlers
//#define scale_by 1//1.24
//#define scroll 400 * scale_by

BOOL CFormDisplayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_RegBGBrush.CreateSolidBrush(PaletteColor((COLORREF)0x00FFFFFF));
	// (j.jones 2007-06-22 09:28) - PLID 25665 - used to color the background of an edited field
	m_EditedBGBrush.CreateSolidBrush(PaletteColor(EDITED_FIELD_COLOR));

	m_bColorEditedFields = TRUE;
	// (j.jones 2007-06-25 10:38) - PLID 25663 - renamed this variable
	m_bFieldsEdited = FALSE;

	m_yscroll = 0;
	m_xscroll = 0;

	// (j.jones 2007-06-22 12:16) - PLID 25665 - cache whether we want to color edited fields
	m_bColorEditedFields = GetRemotePropertyInt("EnableEditedClaimFieldColoring", 1, 0, GetCurrentUserName(), true) == 1;

	SetScrollRange(SB_VERT, 1000, 1000, false);
	ScrollWindow (m_xscroll, m_yscroll, NULL, NULL);

	return TRUE;
}

void CFormDisplayDlg::UnPunctuate(CDWordArray *aryIDsToIgnore /* = NULL*/)
{
	for (unsigned int i = 0; i < m_LayerArray.size(); i++)
		m_LayerArray[i]->UnPunctuate(m_LayerArray[i]->m_formID, m_ControlArray, aryIDsToIgnore);
}

void CFormDisplayDlg::Capitalize()
{
	for (unsigned int i = 0; i < m_LayerArray.size(); i++)
		m_LayerArray[i]->Capitalize(m_LayerArray[i]->m_formID, m_ControlArray);
}

void CFormDisplayDlg::Refresh(int form)
{
	for (unsigned int i = 0; i < m_LayerArray.size(); i++)
	{
		if (m_LayerArray[i]->m_formID == form)
		{
			m_LayerArray[i]->m_prsHistory = m_rsHistory;
			m_LayerArray[i]->Refresh(form, m_ControlArray);
			return;
		}
	}
}

void CFormDisplayDlg::Load(int form, CString where, CString orderby, int *i, long nSetupGroupID /* = -1*/, CStringArray* pastrParams, VarAry* pavarParams)
{
	shared_ptr<FormLayer> pLayer = make_shared<FormLayer>(this, m_rsHistory);
	pLayer->m_prsHistory = m_rsHistory;
	pLayer->m_color = color;
	m_LayerArray.push_back(pLayer);
	pLayer->Load(form, where, orderby, m_ControlArray, i, nSetupGroupID, pastrParams, pavarParams);
}

void CFormDisplayDlg::OnPaint() 
{
	CDialog::OnPaint();

	CDC *pdc = GetDC();
	pdc->SetViewportOrg (m_xscroll, m_yscroll);
	for (unsigned int i = 0; i < m_ControlArray.size(); i++)
		m_ControlArray[i]->Draw(pdc);
	ReleaseDC(pdc);
}

void CFormDisplayDlg::Scroll(int x, int y)
{
	ScrollWindow(x - m_xscroll, y - m_yscroll);
	m_xscroll = x;
	m_yscroll = y;
}

BOOL CFormDisplayDlg::OnPrint(BOOL capitalize, CString strRegName, CDWordArray *aryIDsToIgnore /* = NULL*/, CDC *pPrintDC /*=NULL*/) 
{
	// (j.dinatale 2010-07-22) - PLID 39692 - not needed anymore, the HDC is created in the if statement below if need be
	//HDC    hdcPrn;
	//CDC	   *pDC;

	// (j.dinatale 2010-07-22) - PLID 39692 - Keep track of whether the device context that was passed in was null
	bool bDC_WasNull = (pPrintDC == NULL);

	BOOL delPrintDlg = TRUE;

	// Instantiate a CPrintDialog.

	//
	//DRT 6/2/03 - We're going to save and load the settings of the printer for each form
	//		from the registry (like the global settings - actually they're saved as subkeys of them).
	HGLOBAL hSavedDevMode = NULL, hSavedDevNames = NULL;
	HGLOBAL hOldDevMode = NULL, hOldDevNames = NULL;	//for saving the old settings
	CString str;
	str.Format("PrintSettingsGlobal\\%s", strRegName);
	// (z.manning 2010-11-22 12:44) - PLID 40486 - Renamed this function
	LoadDevSettingsFromRegistryHKLM(str, hSavedDevNames, hSavedDevMode);
	/////
	//

	if (m_ShowPrintDialog) {
		// Display Windows print dialog box.       
		m_printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
		// Initialize some of the fields in PRINTDLG structure.
		m_printDlg->m_pd.nMinPage = m_printDlg->m_pd.nMaxPage = 1;
		m_printDlg->m_pd.nFromPage = m_printDlg->m_pd.nToPage = 1;

		//DRT
		hOldDevMode = m_printDlg->m_pd.hDevMode;
		hOldDevNames = m_printDlg->m_pd.hDevNames;
		m_printDlg->m_pd.hDevMode = hSavedDevMode;
		m_printDlg->m_pd.hDevNames = hSavedDevNames;
		//

		if(m_printDlg->DoModal() == IDCANCEL) {
			delete m_printDlg;
			m_printDlg = NULL;
			return FALSE;
		}
	}
	else {
		delPrintDlg = FALSE;
	}

	// (j.dinatale 2010-07-22) - PLID 39692 - moved outside of this dialog to the batchprintdlg, check if our print context member variable is null just to be safe..
	//		if it is, then we obtain the device context because we were not passed in a device context
	if(bDC_WasNull)
	{
		HDC    hdcPrn;

		// Obtain a handle to the device context.
		hdcPrn = m_printDlg->GetPrinterDC();
		if(!hdcPrn)
			hdcPrn = m_printDlg->CreatePrinterDC();

		// create a new device context
		pPrintDC = new CDC;

		// attach it and start a new document
		pPrintDC->Attach(hdcPrn);
		pPrintDC->StartDoc("Claim Form");
	}

	// (j.dinatale 2010-07-22) - PLID 39692 we no longer should check if the HDC is null, we create them locally if need be. 
	//		The real question is, after all the set up... do we really have a printer?
	if (pPrintDC != NULL) {

		// If this is not NULL, it means the function needs to do something
		// before a print. If it returns TRUE, it means another page is required.
		if (m_pPrePrintFunc)
		{	
			CDWordArray dwChangedForms;

			// (j.dinatale 2010-07-22) - PLID 39692 - no longer needed, the attach and detaching is occuring in another place entirely
			//		and the context is being passed in
			//pDC = new CDC;

			while (m_pPrePrintFunc(dwChangedForms, this)) {

				// Update all the changed forms
				for (int i=0; i < dwChangedForms.GetSize(); i++) {
					int form = dwChangedForms[i];
					Refresh(form);					
				}
				ReapplyChanges(m_ChangeKey1, m_ChangeKey2);
				if(capitalize)
					Capitalize();
				UnPunctuate(aryIDsToIgnore);

				dwChangedForms.RemoveAll();

				// (j.dinatale 2010-07-22) - PLID 39692 - these are no longer needed, the attach and detaching is occuring in another place entirely
				//		and the context is being passed in
				//pDC->Attach (hdcPrn);		// attach a printer DC
				//pDC->StartDoc("Claim Form");// begin a new print job for Win32 use CDC::StartDoc(LPDOCINFO) override

				pPrintDC->StartPage();			// begin a new page
				SetPrintAlign(pPrintDC, pPrintDC->GetSafeHdc());	// set the printing alignment
				FormatPage(pPrintDC);			//pDC->TextOut(10, 10, pbuf);	// write the string in pbuf
				pPrintDC->EndPage();				// end a page

				// (j.dinatale 2010-07-22) - PLID 39692 - these are no longer needed, the attach and detaching is occuring in another place entirely
				//		and the context is being passed in
				//pDC->EndDoc();				// end a print job
				//pDC->Detach();				// detach the printer DC
			}

			// (j.dinatale 2010-07-22) - PLID 39692 - no longer needed, the attach and detaching is occuring in another place entirely
			//		and the context is being passed in
			//delete pDC;      
	   } 
	  else	{
		  // (j.dinatale 2010-07-22) - PLID 39692 - no longer needed, the attach and detaching is occuring in another place entirely
		  //		and the context is being passed in
		  //pDC = new CDC;

		if(capitalize)
			Capitalize();
		UnPunctuate(aryIDsToIgnore);

		// (j.dinatale 2010-07-22) - PLID 39692 - no longer needed, the attach and detaching is occuring in another place entirely
		  //		and the context is being passed in
		//pDC->Attach (hdcPrn);		// attach a printer DC
		//pDC->StartDoc("Claim Form");// begin a new print job for Win32 use CDC::StartDoc(LPDOCINFO) override

		pPrintDC->StartPage();			// begin a new page
		SetPrintAlign(pPrintDC, pPrintDC->GetSafeHdc());	// set the printing alignment
		FormatPage(pPrintDC);			//pDC->TextOut(10, 10, pbuf);	// write the string in pbuf
		pPrintDC->EndPage();				// end a page

		// (j.dinatale 2010-07-22) - PLID 39692 - no longer needed, the attach and detaching is occuring in another place entirely
		  //		and the context is being passed in
		//pDC->EndDoc();				// end a print job
		//pDC->Detach();				// detach the printer DC          
		// (a.walling 2007-08-07 17:05) - PLID 26996 - Need to delete the device context we created
		//delete pDC;
	  }

	  // (j.dinatale 2010-07-22) - PLID 39692 - at this point, if the printer context was null then we need to end the document and detach
	  if(bDC_WasNull)
		{
			//end and detach
			pPrintDC->EndDoc();
			pPrintDC->Detach();

			// finally, delete the context
			delete pPrintDC;
		}
	}

	//DRT
	//save whatever the dialog knows
	hSavedDevMode = m_printDlg->m_pd.hDevMode;
	hSavedDevNames = m_printDlg->m_pd.hDevNames;

	// (z.manning 2010-11-22 12:44) - PLID 40486 - Renamed this function
	SaveDevSettingsToRegistryHKLM(str, hSavedDevNames, hSavedDevMode);
	//De-allocate our saved dev modes
	if (hSavedDevMode) {
		GlobalFree(hSavedDevMode);
		hSavedDevMode = NULL;
	}
	if (hSavedDevNames) {
		GlobalFree(hSavedDevNames);
		hSavedDevNames = NULL;
	}

	//Re-load the global settings here
	m_printDlg->m_pd.hDevNames = hOldDevNames;
	m_printDlg->m_pd.hDevMode = hOldDevMode;
	//

	if(delPrintDlg) {
		delete m_printDlg;
		m_printDlg = NULL;
	}
	
	return TRUE;
}

extern int FONT_SIZE;
void CFormDisplayDlg::FormatPage(CDC *pDC)
{
	CFont	*pFont, *oldFont;

	pFont = new CFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(pFont, FONT_SIZE * 100, "Arial");
	oldFont = (CFont *)pDC->SelectObject(pFont);
	for (unsigned int size = 0; size < m_ControlArray.size(); size++)
		m_ControlArray[size]->PrintOut(pDC);
	pDC->SelectObject(oldFont);
	delete pFont;
}

void CFormDisplayDlg::SetPrintAlign(CDC *pDC, HDC hdcPrn)
{
//	short	cxPage, 
//			cyPage;       
//	cxPage = ::GetDeviceCaps (hdcPrn, HORZRES);
//	cyPage = ::GetDeviceCaps (hdcPrn, VERTRES);
	pDC->SetMapMode (MM_TWIPS);
}

HBRUSH CFormDisplayDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	if (nCtlColor == CTLCOLOR_DLG) {
		return m_RegBGBrush;
	}

	// (j.jones 2007-06-22 09:28) - PLID 25665 - color the background of an edited field
	if (nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_BTN)
	{
		//check our control array for m_bIsEdited fields
		// (j.jones 2007-10-26 11:07) - PLID 25665 - ignore static fields
		for (unsigned int i = 0; i < m_ControlArray.size();i++)
		{
			if (m_ControlArray[i]->nID == pWnd->GetDlgCtrlID()
				&& m_ControlArray[i]->GetEdited()
				&& !(m_ControlArray[i]->format & STATIC))
			{
				//edited control

				// (j.jones 2007-06-25 10:30) - PLID 25663 - needs to call ReflectEditedFields()
				// even if we are not coloring fields
				ReflectEditedFields();

				// (j.jones 2007-06-22 12:16) - PLID 25665 - check the cached preference whether we want to color edited fields
				if(m_bColorEditedFields) {
					pDC->SetBkColor(PaletteColor(EDITED_FIELD_COLOR));
					return m_EditedBGBrush;
				}
			}
		}

		//double-check our change array for any remaining fields
		for (unsigned int i = 0; i < m_pChangeArray.size(); i++) {
			CHANGE_STRUCT* cs = m_pChangeArray[i].get();
			if (cs->key1 == m_ChangeKey1 && cs->key2 == m_ChangeKey2) {
				for (int j=0; j < cs->adwID.GetSize(); j++) {
					
					// (j.jones 2007-10-26 11:07) - PLID 25665 - ignore static fields,
					// and in fact, only run this on non-static editboxes and checkboxes

					BOOL bEditable = FALSE;

					CWnd *pTrackedWnd = (CWnd*)cs->adwID.GetAt(j);
					// (a.walling 2009-11-20 08:02) - PLID 36372 - This was casting pWnd as Form(Edit|Check) after checking
					// if pTrackedWnd is that kind of class. This is an obvious oversight; most of the time this would return
					// some garbage value, but sometimes it will end up accessing an invalid page, and therefore throw an
					// access violation. We now cast pTrackedWnd to the appropriate type instead.
					if (pTrackedWnd->IsKindOf(RUNTIME_CLASS( FormEdit ))) {
						bEditable = !(((FormEdit*)pTrackedWnd)->format & STATIC);
					}
					else  if (pTrackedWnd->IsKindOf(RUNTIME_CLASS( FormCheck ))) {
						bEditable = !(((FormCheck*)pTrackedWnd)->format & STATIC);
					}

					if (cs->adwID.GetAt(j) == pWnd
						&& bEditable) {
						
						//edited control

						// (j.jones 2007-06-25 10:30) - PLID 25663 - needs to call ReflectEditedFields()
						// even if we are not coloring fields
						ReflectEditedFields();

						// (j.jones 2007-06-22 11:31) - PLID 25665 - If we get here,
						// it means there is a changed field where m_bIsEdited is
						// not set to true, or we failed to access it above.
						// Unfortunately this happens because ancient and arguably
						// bad HCFA design has a few controls kept out of the
						// m_ControlArray.
						// (j.jones 2007-06-22 12:16) - PLID 25665 - check the cached preference whether we want to color edited fields
						if(m_bColorEditedFields) {
							pDC->SetBkColor(PaletteColor(EDITED_FIELD_COLOR));
							return m_EditedBGBrush;
						}
					}
				}
			}
		}

		//otherwise it is a normal control
		if(nCtlColor == CTLCOLOR_STATIC)
			pDC->SetTextColor(color);
		pDC->SetBkColor(PaletteColor(RGB(255,255,255)));
		return m_RegBGBrush;
	}

	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	return hbr;
}

BOOL CFormDisplayDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	//GetParent()->SendMessage(WM_COMMAND, wParam, lParam);
	CDWordArray dwChangedForms;

	// Added by Chris
	if (m_pOnCommand) {
		int i, control_id = -1;

		for (unsigned int j = 0; j < m_ControlArray.size(); j++)
		{
			if (m_ControlArray[j]->nID == LOWORD(wParam))
			{
				control_id = m_ControlArray[j]->id;
				break;
			}
		}
		m_pOnCommand(wParam, lParam, dwChangedForms, control_id, this);
	
		if (dwChangedForms.GetSize() > 0) {
			BeginWaitCursor();
			for (i=0; i < dwChangedForms.GetSize(); i++) {
				int form = dwChangedForms[i];
				Refresh(form);
			}
			EndWaitCursor();
			
			ReapplyChanges(m_ChangeKey1, m_ChangeKey2);
			TrackChanges(m_ChangeKey1, m_ChangeKey2);
		}
	}

	return CDialog::OnCommand(wParam, lParam);
}

void CFormDisplayDlg::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
}

void CFormDisplayDlg::OnOK() 
{
}

void CFormDisplayDlg::OnCancel() 
{
}

void CFormDisplayDlg::TrackChanges(int key1, int key2)
{
	m_ChangeKey1 = key1;
	m_ChangeKey2 = key2;

	// (j.armen 2014-03-27 16:28) - PLID 60784 - changes are kept in a smart pointer now
	shared_ptr<CHANGE_STRUCT> cs;
	for (unsigned int i=0; i < m_pChangeArray.size(); i++)
	{
		cs = m_pChangeArray[i];
		if (cs->key1 == m_ChangeKey1 && cs->key2 == m_ChangeKey2)
			return;
	}

	cs = make_shared<CHANGE_STRUCT>();
	cs->key1 = key1;
	cs->key2 = key2;
	m_pChangeArray.push_back(cs);
}

void CFormDisplayDlg::StopTrackingChanges()
{
	m_ChangeKey1 = m_ChangeKey2 = -1;
}

void CFormDisplayDlg::ReapplyChanges(int key1, int key2)
{
	for (unsigned int i = 0; i < m_pChangeArray.size(); i++)
	{
		CHANGE_STRUCT* cs = m_pChangeArray[i].get();
		if (cs->key1 == key1 && cs->key2 == key2) {
			for (int j=0; j < cs->adwID.GetSize(); j++) {
				CWnd* pWnd = (CWnd*)cs->adwID.GetAt(j);

				if (pWnd && !pWnd->IsKindOf(RUNTIME_CLASS( FormCheck )))
					pWnd->SetWindowText(cs->astrText.GetAt(j));
				else if (pWnd){
					((FormCheck*)pWnd)->SetCheck(atoi(cs->astrText.GetAt(j)));
					((FormCheck*)pWnd)->check = atoi(cs->astrText.GetAt(j));
				}
			}
			break;
		}
	}
}

//find a child window with a given ID field
CWnd * CFormDisplayDlg::GetControl (int identifier)
{
	for (unsigned int i = 0; i < m_LayerArray.size(); i++)
	{
		CWnd* pLayerWnd = m_LayerArray[i]->m_pWnd;
		CWnd* tmpWnd = pLayerWnd->GetWindow(GW_CHILD);
		while(tmpWnd)
		{	if (tmpWnd && ::IsWindow(tmpWnd->m_hWnd)) 
				if (tmpWnd->IsKindOf(RUNTIME_CLASS(FormControl)))
				{	if (tmpWnd->IsKindOf(RUNTIME_CLASS(FormEdit)))
						if (((FormEdit *)tmpWnd)->id == identifier)
							return tmpWnd;
					else if (tmpWnd->IsKindOf(RUNTIME_CLASS(FormCheck)))
						if (((FormCheck *)tmpWnd)->id == identifier)
							return tmpWnd;
					//else etc.  This is not finished
				}
			tmpWnd = tmpWnd->GetWindow(GW_HWNDNEXT);
		}
	}
	return NULL;
}

LRESULT CFormDisplayDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{

	if (message == WM_COMMAND && m_ChangeKey1 != -1 && m_ChangeKey2 != -1) {
		CWnd* pCtrl = GetDlgItem(LOWORD(wParam));
		CWnd* pWnd;
		CHANGE_STRUCT* cs;
		CString str;

		pWnd = pCtrl;

		if (!pWnd || !pWnd->IsKindOf(RUNTIME_CLASS( FormCheck ))) {//always check null pointers - BVB
			//TES 4/30/2008 - PLID 26475 - We used to do nothing here.  Now, however, we will check for the EN_CHANGE
			// notification, and if so, execute all the code that we used to execute in the WM_CHAR handler.  EN_CHANGE
			// is vastly preferable.  The old way, for example, hitting Ctrl+C would cause a field to be flagged as edited,
			// whereas right-clicking and pasting text would not.
			if(HIWORD(wParam) == EN_CHANGE) {
				if(m_ChangeKey1 != -1 && m_ChangeKey2 != -1) {
					CHANGE_STRUCT* cs;
					CString str;

					pCtrl->GetWindowText(str);

					for (unsigned int i=0; i < m_pChangeArray.size(); i++) {
						cs = m_pChangeArray[i].get();
						if (cs->key1 == m_ChangeKey1 && cs->key2 == m_ChangeKey2) {

							// (j.jones 2007-06-22 09:28) - PLID 25665 - mark the field as edited
							//TES 4/30/2008 - PLID 26475 - Check whether it actually gets marked as edited.
							bool bMarkedEdited = true;
							if (pCtrl->IsKindOf(RUNTIME_CLASS( FormEdit ))) {
								for (unsigned int j = 0; j < m_ControlArray.size(); j++) {
									// (j.jones 2007-10-26 10:47) - PLID 25665 - Only consider it edited if it is non-static.
									if (((FormEdit*)m_ControlArray[j].get())->nID == pCtrl->GetDlgCtrlID()) {

										if(!(((FormEdit*)m_ControlArray[j].get())->format & STATIC)) {
											// (a.walling 2007-07-27 14:28) - This is extremely useful, especially to quickly figure out
											// what FormControlsT.ID a control actually is. Also for verifying the output from the Forms app.
											// Using TRACE since this should never actually go in the log, but useful for developers.
											if (((FormEdit*)m_ControlArray[j].get())->GetEdited() == FALSE)
												TRACE("Edited FormControlsT.ID(%li)\r\n", ((FormEdit*)m_ControlArray[j].get())->id);

											((FormEdit*)m_ControlArray[j].get())->SetEdited(TRUE);
											if(!((FormEdit*)m_ControlArray[j].get())->GetEdited()) {
												//TES 4/30/2008 - PLID 26475 - We did not actually mark it as edited.
												bMarkedEdited = false;
											}
											break;
										}
										else {
											// (j.jones 2010-01-05 10:55) - PLID 36525 - if a static item,
											//we don't need to track changes
											bMarkedEdited = false;
										}
									}
								}
							}

							if(!bMarkedEdited) {
								//TES 4/30/2008 - PLID 26475 - This means that the control was in "self-editing" mode,
								// meaning that this EN_CHANGE notification was fired as a result of the control setting
								// its own text, not a user action.  Therefore, we don't want to add this control to our
								// list of changes, as it hasn't really changed.
								break;
							}

							for (int j=0; j < cs->adwID.GetSize(); j++) {
								if (cs->adwID.GetAt(j) == pCtrl) {
									cs->astrText.SetAt(j, str);
									break;
								}
							}
							if (j == cs->adwID.GetSize()) {
								cs->adwID.Add(pCtrl);
								cs->astrText.Add(str);
							}

							// (j.jones 2007-06-22 11:15) - PLID 25665 - ensure we repaint
							CRect rc;
							pCtrl->GetWindowRect(&rc);
							ScreenToClient(&rc);
							InvalidateRect(&rc);

							break;
						}
					}
					return CDialog::WindowProc(message, wParam, lParam);
				}
			}
			else {
				return CDialog::WindowProc(message, wParam, lParam);
			}
		}

		do {
			if (pWnd->IsKindOf(RUNTIME_CLASS( FormCheck ))) {
				for (unsigned int i = 0; i < m_pChangeArray.size(); i++) {
					cs = m_pChangeArray[i].get();
					if (cs->key1 == m_ChangeKey1 && cs->key2 == m_ChangeKey2) {

						// (j.jones 2007-10-26 10:47) - PLID 25665 - Only consider it edited if it is non-static.
						if(!(((FormCheck*)pCtrl)->format & STATIC)) {

							// (a.walling 2007-07-27 14:28) - This is extremely useful, especially to quickly figure out
							// what FormControlsT.ID a control actually is. Also for verifying the output from the Forms app.
							// Using TRACE since this should never actually go in the log, but useful for developers.
							if (((FormCheck*)pCtrl)->GetEdited() == FALSE)
								TRACE("Edited FormControlsT.ID(%li)\r\n", ((FormCheck*)pCtrl)->id);

							// (j.jones 2007-06-22 09:28) - PLID 25665 - mark the field as edited
							((FormCheck*)pCtrl)->SetEdited(TRUE);
						}
						
						if (pWnd == pCtrl)
							((FormCheck*)pWnd)->check = 1;
						else
							((FormCheck*)pWnd)->check = 0;

						for (int j=0; j < cs->adwID.GetSize(); j++) {
							if (cs->adwID.GetAt(j) == pWnd) {

								if (pWnd == pCtrl)
									cs->astrText.SetAt(j, "1");
								else
									cs->astrText.SetAt(j, "0");
								break;
							}
						}
						if (j == cs->adwID.GetSize()) {
							cs->adwID.Add(pWnd);
							if (pWnd == pCtrl)
								cs->astrText.Add("1");
							else
								cs->astrText.Add("0");
						}

						// (j.jones 2007-06-22 11:15) - PLID 25665 - ensure we repaint
						CRect rc;
						pWnd->GetWindowRect(&rc);
						ScreenToClient(&rc);
						InvalidateRect(&rc);

						break;
					}
				}				
			} 
			pWnd = GetNextDlgGroupItem(pWnd);
		} while (pCtrl != pWnd);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CFormDisplayDlg::PreTranslateMessage(MSG* pMsg) 
{
	CWnd* pCtrl = GetFocus();
	BOOL res = CDialog::PreTranslateMessage(pMsg);

	if (pMsg->message == WM_CHAR && m_pOnKeyDown) {
		m_pOnKeyDown(this, pMsg);
	}

	//TES 4/30/2008 - PLID 26475 - This was a subpar way of detecting changes.  With this method, hitting Ctrl+C would
	// count as a change, but right-clicking and pasting would not.  I moved all this code up into an EN_CHANGE handler
	// in WindowProc().
	/*if (pMsg->message == WM_CHAR || 
		(pMsg->message == WM_KEYDOWN && ((GetAsyncKeyState(VK_DELETE) & 0x80000000)))) {
		if(m_ChangeKey1 != -1 && m_ChangeKey2 != -1) {
			CHANGE_STRUCT* cs;
			CString str;

			pCtrl->GetWindowText(str);

			for (int i=0; i < m_pChangeArray.GetSize(); i++) {
				cs = (CHANGE_STRUCT*)m_pChangeArray.GetAt(i);
				if (cs->key1 == m_ChangeKey1 && cs->key2 == m_ChangeKey2) {

					// (j.jones 2007-06-22 09:28) - PLID 25665 - mark the field as edited
					if (pCtrl->IsKindOf(RUNTIME_CLASS( FormEdit ))) {
						for (int j=0;j<m_ControlArray.GetSize();j++) {
							// (j.jones 2007-10-26 10:47) - PLID 25665 - Only consider it edited if it is non-static.
							if (((FormEdit*)m_ControlArray.GetAt(j))->nID == pCtrl->GetDlgCtrlID()
								&& !(((FormEdit*)m_ControlArray.GetAt(j))->format & STATIC)) {
								// (a.walling 2007-07-27 14:28) - This is extremely useful, especially to quickly figure out
								// what FormControlsT.ID a control actually is. Also for verifying the output from the Forms app.
								// Using TRACE since this should never actually go in the log, but useful for developers.
								if (((FormEdit*)m_ControlArray.GetAt(j))->m_bIsEdited == FALSE)
									TRACE("Edited FormControlsT.ID(%li)\r\n", ((FormEdit*)m_ControlArray.GetAt(j))->id);

								((FormEdit*)m_ControlArray.GetAt(j))->m_bIsEdited = TRUE;
								break;
							}
						}
					}

					for (int j=0; j < cs->adwID.GetSize(); j++) {
						if (cs->adwID.GetAt(j) == pCtrl) {
							cs->astrText.SetAt(j, str);
							break;
						}
					}
					if (j == cs->adwID.GetSize()) {
						cs->adwID.Add(pCtrl);
						cs->astrText.Add(str);
					}

					// (j.jones 2007-06-22 11:15) - PLID 25665 - ensure we repaint
					CRect rc;
					pCtrl->GetWindowRect(&rc);
					ScreenToClient(&rc);
					InvalidateRect(&rc);

					break;
				}
			}
		}
	}*/

	return res;
}

void CFormDisplayDlg::Save(int key1, int key2, int iDocumentID, BOOL boSaveAll)
{
	CWnd* pWnd;

	///////////////////////////////////////////////
	// If we don't save everything, just save
	// the changed fields.
	if (!boSaveAll) {
		for (unsigned int i = 0; i < m_pChangeArray.size(); i++) {
			CHANGE_STRUCT* cs = m_pChangeArray[i].get();
			if (cs->key1 == key1 && cs->key2 == key2) {
				for (int j=0; j < cs->adwID.GetSize(); j++) {
					pWnd = (CWnd*)cs->adwID.GetAt(j);
					if (pWnd->IsKindOf(RUNTIME_CLASS( FormEdit )))
						((FormEdit*)pWnd)->Save(iDocumentID);
					else if (pWnd->IsKindOf(RUNTIME_CLASS( FormCheck )))
						((FormCheck*)pWnd)->Save(iDocumentID);
					
					/*if (pCtrl->format & EDIT || pCtrl->format & STATIC) {
						((FormEdit*)pCtrl)->Save(iDocumentID);
					}
					else if (pCtrl->format & DATE) {
						((FormDate*)pCtrl)->Save(iDocumentID);
					}
					else if (pCtrl->format & CHECK)
						((FormCheck*)pCtrl)->Save(iDocumentID);*/
				}
				break;
			}
		}
	}
	///////////////////////////////////////////////
	// Otherwise, save EVERY editable field
	else {
		for (unsigned int i = 0; i < m_LayerArray.size(); i++)
		{
			m_LayerArray[i]->Save(iDocumentID, 0, m_ControlArray);
		}
	}
}

void CFormDisplayDlg::SetDocumentID(int iDocumentID)
{
	try{
	if (m_rsHistory->State != adStateClosed)
		m_rsHistory->Close();

	if (iDocumentID != -1) {
		m_rsHistory = CreateRecordset("SELECT * FROM FormHistoryT WHERE DocumentID = %d", iDocumentID);
	}
	} NxCatchAll("Error in SetDocumentID");
}

void CFormDisplayDlg::ChangeParameter(int form, CString strParam, COleVariant var)
{
	for (unsigned int i = 0; i < m_LayerArray.size(); i++)
	{
		if (m_LayerArray[i]->m_formID == form)
		{
			m_LayerArray[i]->ChangeParameter(strParam, var);
			return;
		}
	}
}

// This function will clear all user-made changes
// and refresh the historical data table
void CFormDisplayDlg::UndoChanges()
{
	// (j.armen 2014-03-27 16:28) - PLID 60784 - Changes kept in shared_ptr, we can just clear
	m_pChangeArray.clear();
	// Refresh historical data anticipating deletions
	// made by CHCFADlg
	if (m_rsHistory->State != adStateClosed)
		m_rsHistory->Requery(adAsyncExecute);
}

// (j.jones 2007-06-22 13:25) - PLID 25665 - track if we've highlighted fields, and if we do,
// tell the parent we've done so
// (j.jones 2007-06-25 10:31) - PLID 25663 - changed the name of this function, to accomodate
// telling the parent that something changed, not necessarily that it was colored
void CFormDisplayDlg::ReflectEditedFields()
{
	//this notification function is independent of the field coloring preference

	//if we haven't already notified our parent that we edited fields,
	//do so, and set the boolean so we don't need to do it again
	if(!m_bFieldsEdited) {

		m_bFieldsEdited = TRUE;

		if(GetParent())
			GetParent()->PostMessage(NXM_CLAIM_FORM_FIELDS_EDITED);
	}
}
