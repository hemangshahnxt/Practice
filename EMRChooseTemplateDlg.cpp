// EMRChooseTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRChooseTemplateDlg.h"
#include "DontShowDlg.h"
#include "EmrColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEMRChooseTemplateDlg dialog


CEMRChooseTemplateDlg::CEMRChooseTemplateDlg(CWnd* pParent)
	: CNxDialog(CEMRChooseTemplateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEMRChooseTemplateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pList = NULL;
	m_nSelectedTemplateID = -1;
	m_bFilterOn = TRUE;
	m_bNoFilteredTemplates = FALSE;
	m_bSilentReload = FALSE;
}


void CEMRChooseTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRChooseTemplateDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEMRChooseTemplateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRChooseTemplateDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRChooseTemplateDlg message handlers

BOOL CEMRChooseTemplateDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-25 16:03) - PLID 29796 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//TES 11/12/2007 - PLID 28059 - This is a datalist2, not a datalist.
		m_pList = BindNxDataList2Ctrl(IDC_TEMPLATE_LIST, false);
		GetDlgItem(IDC_TEMPLATE_LIST)->SetFocus();

		CNxColor* pWnd = (CNxColor*)GetDlgItem(IDC_CONFIGURE_BACKGROUND);
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		pWnd->SetColor(EmrColors::Topic::PatientBackground());

		Reload();

		// TODO: Add extra initialization here
	}
	NxCatchAll("Error in CEMRChooseTemplateDlg::OnInitDialog");

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRChooseTemplateDlg::OnOK() 
{
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pList->GetCurSel();
	if(pCurSel == NULL) {
		AfxMessageBox("You must select a template before pressing OK.");
		return;
	}

	if(VarLong(pCurSel->GetValue(0)) == -2) {
		AfxMessageBox("You must select a template before pressing OK.");
		return;
	}

	m_nSelectedTemplateID = VarLong(pCurSel->GetValue(0));

	// (j.jones 2006-06-27 17:21) - used to tell the caller we selected the generic EMN
	if(m_nSelectedTemplateID == -1)
		m_nSelectedTemplateID = -2;

	CDialog::OnOK();
}

void CEMRChooseTemplateDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

//Call to add a procedure to our filter.  All procedures filtered will add templates to our
//	available list.
void CEMRChooseTemplateDlg::AddProcedureFilter(long nProcedureID)
{
	m_aryFilteredProcedures.Add(nProcedureID);

	//We should reload the list, in case this happened after OnInitDialog.  This method is a little
	//	slower (multiple requeries), so the calling function should attempt to find all procedures before
	//	calling DoModal.
	if(m_pList) {
		//Clear the "no templates" flag, we may have some now
		m_bNoFilteredTemplates = FALSE;
		Reload();
	}
}

void CEMRChooseTemplateDlg::Reload()
{
	try {
		//Clear out anything already loaded
		m_pList->Clear();

		if(m_bFilterOn) {
			//We are filtering on specific procedures.  Setup our query to only get those templates
			//	which are related to those procedures.

			//Make an "IN" clause for our available templates
			CString strClause;
			for(int i = 0; i < m_aryFilteredProcedures.GetSize(); i++) {
				CString str;
				str.Format("%li, ", m_aryFilteredProcedures.GetAt(i));
				strClause += str;
			}
			strClause.TrimRight(", ");

			//if no procedures, ensure the query will still run
			if(strClause.IsEmpty())
				strClause = "-1";

			//Formulate a query that will list all available templates for our current procedure filter.
			CString strQuery;
			strQuery.Format("EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
				"LEFT JOIN EMRTemplateProceduresT ON EMRTemplateT.ID = EMRTemplateProcedurest.EMRTemplateID");
			m_pList->PutFromClause(_bstr_t(strQuery));

			//DRT 1/24/2006 - This query is modified from the "filtered to this EMR" query that used to be in the PIC,
			//	but modified because we already have a procedure, not a ProcInfo ID.  I also got rid of the "Add Once"
			//	part of the WHERE clause, since this list is always used to start a new EMR, you obviously cannot add
			//	something a second time.
			strQuery.Format("EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND "
				"EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT) "
				"AND EMRTemplateProceduresT.ProcedureID IN (%s)", strClause);
			m_pList->PutWhereClause(_bstr_t(strQuery));
			m_pList->Requery();
		}
		else {
			//We are not doing any filters, just show everything.
			// (z.manning, 02/20/2007) - PLID 23331 - Don't show templates from inactive collections.
			m_pList->PutWhereClause(_bstr_t("EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 "));
			m_pList->Requery();
		}

	} NxCatchAll("Error in Reload");
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CEMRChooseTemplateDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRChooseTemplateDlg)
	ON_EVENT(CEMRChooseTemplateDlg, IDC_TEMPLATE_LIST, 18 /* RequeryFinished */, OnRequeryFinishedTemplateList, VTS_I2)
	ON_EVENT(CEMRChooseTemplateDlg, IDC_TEMPLATE_LIST, 3 /* DblClickCell */, OnDblClickCellTemplateList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEMRChooseTemplateDlg::OnRequeryFinishedTemplateList(short nFlags) 
{
	try {
		//If there are no rows in the list at all, then we must have nothing for our procedure.  Instead of
		//	giving a silly empty list with an option to show everything... just show everything.
		if(m_bFilterOn && m_pList->GetRowCount() == 0) {
			if(!m_bNoFilteredTemplates) {
				//PLID 19671 - If this flag is FALSE, we've never hit this before - so we are in the initial loading of the dialog.
				//	To clear confusion about what is displayed, pop up a message box informing the user what is
				//	going on.
				if(!m_bSilentReload) {
					DontShowMeAgain(this, "Your chosen procedures do not have any related EMN templates.  This list will be all EMN templates available in the system.", 
						"EMRChooseTemplateNoTemplateWarning", "No Templates Found", FALSE, FALSE);
				}
			}

			m_bFilterOn = FALSE;

			//Additionally, we need to set a flag that there are no filtered templates.  Otherwise you get the "<Filter Templates>"
			//	option, but that sends you back into this loop, which requeries again for everything.
			//Note that once we set this, the only way to unset it is to have the calling function call AddProcedureFilter, or 
			//	close and reopen this dialog.
			m_bNoFilteredTemplates = TRUE;

			//do the reload, then quit.  We'll get here again when it finishes to do the below stuff.
			Reload();
			return;
		}


		//When the requery finishes, we need to add options for "<Generic EMN>" at the top, and 
		//	"<More Templates>" at the bottom, in case they want something else.
		CString strName = "<Generic EMN>";

		//Add a new row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t(strName));
		pRow->PutValue(2, _bstr_t(""));
		//Insert this as the first row in the list
		m_pList->AddRowBefore(pRow, m_pList->GetFirstRow());


		if(m_bFilterOn) {
			//Now add the "More Templates" at the end
			strName = "<More Templates>";
		}
		else {
			//We're in the "more", so offer to "Filter"
			strName = "<Filter Templates>";
		}

		pRow = m_pList->GetNewRow();
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(strName));
		pRow->PutValue(2, _bstr_t(""));
		//Insert at end
		m_pList->AddRowAtEnd(pRow, NULL);

	} NxCatchAll("Error in OnRequeryFinished");
}

void CEMRChooseTemplateDlg::OnDblClickCellTemplateList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		if(!lpRow)
			return;

		//2 options -- they hit "More Info", in which case we need to change our filter, or
		//	a real option, in which case we just say OK.

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nID = VarLong(pRow->GetValue(0));
		if(nID != -2) {
			//"More Info" is always -2
			OnOK();
			return;
		}

		//Otherwise, we are on the more info (or "filtered templates") and need to revert our filter.
		//	Reverse our filter and reload
		m_bFilterOn = !m_bFilterOn;
		Reload();


	} NxCatchAll("Error in OnDblClickCell");
}
