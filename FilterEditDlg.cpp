// FilterEditDlg.cpp : implementation file
//
#include "stdafx.h"
#include "PracProps.h"
#include "FilterEditDlg.h"
#include "NxException.h"
#include "Groups.h"
#include "WhereClause.h"
#include "FilterDlg.h"
#include "resource.h"
#include "AuditTrail.h"
#include "GlobalDataUtils.h"
#include "GlobalDrawingUtils.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define FILTER_NAME_NEW				"Enter a name for this filter"
#define FILTER_NAME_TEMPORARY		""

/////////////////////////////////////////////////////////////////////////////
// CFilterEditDlg dialog

CFilterEditDlg::CFilterEditDlg(CWnd* pParent /*=NULL*/, long nFilterType, BOOL (WINAPI *pfnIsActionSupported)(SupportedActionsEnum, long), BOOL (WINAPI* pfnCommitSubfilterAction)(SupportedActionsEnum, long, long&, CString&, CString&, CWnd*), BOOL (WINAPI* pfnGetNewFilterString)(long, long, CString&, LPCTSTR, CString&), LPCTSTR strTitle /*= NULL*/)
	: CNxDialog(CFilterEditDlg::IDD, pParent),
		m_dlgFilter(*(new CFilterDlg(FILTER_ID_NEW, NULL, nFilterType, pfnIsActionSupported, pfnCommitSubfilterAction, pfnGetNewFilterString)))
{
	//{{AFX_DATA_INIT(CFilterEditDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_strFilterString.Empty();
	m_strFilterName.Empty();

	m_strSqlFrom.Empty();
	m_strSqlWhere.Empty();

	m_nFilterType = nFilterType;
	
	m_strTitle = strTitle;

	m_pfnIsActionSupported = pfnIsActionSupported;
	m_pfnCommitSubfilterAction = pfnCommitSubfilterAction;

	m_bAuditForLookup = FALSE; // (z.manning 2009-06-02 15:13) - PLID 34430
}

CFilterEditDlg::~CFilterEditDlg()
{
	delete &m_dlgFilter;
}

void CFilterEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterEditDlg)
	DDX_Control(pDX, IDC_FILTER_NAME_EDIT, m_nxeditFilterNameEdit);
	DDX_Control(pDX, IDC_FILTER_BKG_LABEL, m_nxstaticFilterBkgLabel);
	DDX_Control(pDX, IDC_FILTER_BKG_LABEL2, m_nxstaticFilterBkgLabel2);
	DDX_Control(pDX, IDC_FILTER_BKG_LABEL3, m_nxstaticFilterBkgLabel3);
	DDX_Control(pDX, IDC_FILTER_BKG_LABEL4, m_nxstaticFilterBkgLabel4);
	DDX_Control(pDX, IDC_FILTER_BKG_LABEL5, m_nxstaticFilterBkgLabel5);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SAVE_FILTER_AS, m_btnSaveFilterAs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFilterEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFilterEditDlg)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_CLEAR_BTN, OnClearBtn)
	ON_BN_CLICKED(IDC_SAVE_FILTER_AS, OnSaveFilterAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterEditDlg message handlers
BOOL CFilterEditDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/25/2008) - PLID 29795 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnSaveFilterAs.SetIcon(IDI_SAVE);

	// Place things appropriately on screen
	CRect rt1, rt2;
	GetDlgItem(IDC_FILTER_BKG)->GetWindowRect(rt1);
	// I don't exactly know why the GetWindowRect gave screen 
	// coordinates, but since it did we have to convert them to client
	ScreenToClient(rt1);

	// Now placte the filter dialog in the appropriate place
	m_dlgFilter.GetWindowRect(rt2);
	m_dlgFilter.SetWindowPos(NULL, rt1.left + rt1.Width()/2 - rt2.Width()/2, 
		rt1.top + 90, 0, 0, SWP_NOZORDER|SWP_SHOWWINDOW|SWP_NOSIZE);
	
	//Find out what title we should use.
	if(m_strTitle != "")
		SetWindowText(m_strTitle);

	//OK, here's what we want.  We want to get a chain of parent windows until we find one that is NOT a child window.
	//When we find that window, we will first center ourselves against, and then, if necessary, move ourselves so  
	//that we are offset at least 20 pixels down and to the right of the "real" parent.
	CWnd *pAncestorWnd = GetParent();
	while(pAncestorWnd->GetStyle() & WS_CHILD) {
		pAncestorWnd = pAncestorWnd->GetParent();
	}
	//OK, we've got it at this point. Let's find the y-coord we want for the top.
	CRect rParent, rThis;
	GetWindowRect(rThis);
	pAncestorWnd->GetWindowRect(rParent);
	int nY = (rParent.top + (rParent.Height() / 2) ) - (rThis.Height() / 2);
	//Now it's centered.  Is it at least 20 pixels below the top?
	if(nY < rParent.top + 20) nY = rParent.top + 20;

	//The same for the x-coord.
	int nX = (rParent.left + (rParent.Width() / 2) ) - (rThis.Width() / 2);
	//Now it's centered.  Is it at least 20 pixels right of the left?
	if(nX < rParent.left + 20) nX = rParent.left + 20;

	//Now, put the window there.
	MoveWindow(nX, nY, rThis.Width(), rThis.Height());


	// Load based on the current filter variables
	bool bLoadSuccess = Load();

	if (bLoadSuccess && GetFilterId() == FILTER_ID_TEMPORARY) {
		// Subtract two from the item count, because item count 
		// is always includes one empty item, and it's zero-based.
		return !m_dlgFilter.SetDetailFocus(m_dlgFilter.m_nItemCount-2, CFilterDlg::dfpValue);
	} else {
		return TRUE;
	}
}

long CFilterEditDlg::GetFilterId()
{
	return m_dlgFilter.m_nFilterId;
}

void CFilterEditDlg::OnCancel()
{
	CDialog::OnCancel();
}

void CFilterEditDlg::OnOK() 
{
#ifdef _DEBUG
	// For Debugging show the clauses that were generated
	CString strFrom, strWhere;
	if(!m_dlgFilter.GetFromClause(strFrom)) {
		MsgBox("Could not save filter");
		return;
	}
	CMapStringToString mapUsedFilters;
	m_dlgFilter.GetWhereClause(strWhere, mapUsedFilters);
	if (MsgBox(MB_OKCANCEL, "SELECT * FROM %s\n\nWHERE %s", strFrom, strWhere) != IDOK) return;
#endif

	// Try to save it, and if successful, close the dialog
	if (Save()) {
		//PLID 17069 Adding filtersT table checker
		CClient::RefreshTable(NetUtils::FiltersT, m_dlgFilter.m_nFilterId);
		CDialog::OnOK();
	}
}

// (z.manning 2009-06-03 10:29) - PLID 34430 - Added optional audit transaction parameter
bool CFilterEditDlg::ValidateFilter(CAuditTransaction *pAuditTransaction /* = NULL */)
{
	if (m_dlgFilter.GetFilterString(m_strFilterString)) {
		if (!m_strFilterString.IsEmpty()) {
			// Got the filter string successfully
			// Now try to get the WHERE and FROM clauses
			CString strLocalWhere;
			CMapStringToString mapUsedFilters;
			// (z.manning 2009-06-03 10:34) - PLID 34430 - Only audit individual filters if this is a lookup.
			CAuditTransaction *pFilterDetailAudit = NULL;
			if(m_bAuditForLookup) {
				pFilterDetailAudit = pAuditTransaction;
			}
			if (m_dlgFilter.GetWhereClause(strLocalWhere, mapUsedFilters, FALSE, pFilterDetailAudit)) {
				CString strLocalFrom;
				if (m_dlgFilter.GetFromClause(strLocalFrom)) {

					//PLID 20664 - only run the query if there is an advanced filter anywhere in it
					if (m_dlgFilter.HasAdvancedFilter()) {

						// We successfully got the from and where clauses, so see if they can work to query the data
						try {
							// Validate the query by using an Execute statenent that returns no recordset
							CString strSql;
							strSql.Format("SELECT * FROM "
							"%s WHERE (%s)", strLocalFrom, strLocalWhere);
							ExecuteSqlStd(strSql, NULL, ADODB::adCmdText|ADODB::adExecuteNoRecords);
						
							// We were able to create it successfully, so we have a valid filter
							m_strSqlFrom = strLocalFrom;
							m_strSqlWhere = strLocalWhere;
							return true;
						} catch (_com_error e) {
							// Put it into a statement to present to the user
							Log("The filter you entered is invalid (%s %s): SELECT * FROM "
								"%s WHERE (%s)", (LPCTSTR)e.Description(), e.ErrorMessage(), strLocalFrom, strLocalWhere);
							MsgBox(MB_ICONINFORMATION|MB_OK, "The filter you entered is invalid:\n\n%s (%s)", (LPCTSTR)e.Description(), e.ErrorMessage());
							return false;
						} catch (CException *e) {
							// Unexpected error
							HandleException(e, "CFilterEditDlg::ValidateFilter Error 150:", __LINE__, __FILE__);
							return false;
						}
					}
					else {
						//we aren't validating, so do what it normally does if it successfully validates
						m_strSqlFrom = strLocalFrom;
						m_strSqlWhere = strLocalWhere;
						return true;
					}
				} else {
					// Could not obtain FROM clause
					MsgBox(MB_ICONINFORMATION|MB_OK, "CFilterEditDlg::ValidateFilter Error 250: Could not obtain FROM clause.");
					return false;
				}
			} else {
				// Could not obtain WHERE clause
				MsgBox(MB_ICONINFORMATION|MB_OK, "This filter cannot be saved.  Please confirm that each line has been entered correctly.");
				return false;
			}
		} else {
			// The filter string was empty
			MsgBox(MB_ICONINFORMATION|MB_OK, "You may not save a blank filter.");
			return false;
		}
	} else {
		// GetFilterString failed
		MsgBox(MB_ICONEXCLAMATION|MB_OK, "CFilterEditDlg::ValidateFilter Error 450: Could not obtain Filter String");
		return false;
	}
}

bool CFilterEditDlg::Save()
{
	try {
		// (z.manning 2009-06-03 10:32) - PLID 34430 - Use an audit transaction in this function.
		CAuditTransaction audit;
		// Get the filter string from the filter
		if (ValidateFilter(&audit)) {
			if (m_dlgFilter.m_nFilterId == FILTER_ID_TEMPORARY) {
				// This filter type is not allowed to be saved, so just make sure it's valid
				audit.Commit(); // (z.manning 2009-06-03 10:31) - PLID 34430
				return true;
			}
			// Get the name for the filter
			GetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
			m_strFilterName.TrimLeft();
			m_strFilterName.TrimRight();
			//First, make sure that it is not "Enter a name for this filter" (We, the framework, put that string in there,
			//so it's up to us to deal with it.)
			int nReturn = IDOK;
			while(nReturn == IDOK && (m_strFilterName == "Enter a name for this filter" || m_strFilterName.IsEmpty()) ){
				nReturn = InputBoxLimited(this, "Enter a name for this filter", m_strFilterName,"",50,false,false,NULL);
			}
			if(nReturn != IDOK) {
				//They canceled.
				return false;
			}

			// See if this is a new filter
			if (m_dlgFilter.m_nFilterId == FILTER_ID_NEW) {
				// It is a new filter
				if(m_pfnIsActionSupported(saAdd, m_nFilterType)) {
					if(m_pfnCommitSubfilterAction(saAdd, m_nFilterType, m_dlgFilter.m_nFilterId, m_strFilterName, m_strFilterString, this)) {
						//auditing
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "",audit, aeiFilterCreated, -1, "", m_strFilterName, aepMedium, aetCreated);
						audit.Commit(); // (z.manning 2009-06-03 10:31) - PLID 34430
						return true;
					}
					else {
						//Failed to save.
						return false;
					}
				}
				else {
					//Adding filters not supported.
					return false;
				}
			} else {
				if(m_pfnIsActionSupported(saEdit, m_nFilterType)) {
					if(m_pfnCommitSubfilterAction(saEdit, m_nFilterType, m_dlgFilter.m_nFilterId, m_strFilterName, m_strFilterString, this)) {
						//auditing
						if(m_strOldFilterName != m_strFilterName) {
							long nAuditID = BeginNewAuditEvent();
							AuditEvent(-1, "", audit, aeiFilter, -1, m_strOldFilterName, m_strFilterName, aepMedium, aetChanged);
						}
						// If we made it to here we have succeeded
						audit.Commit(); // (z.manning 2009-06-03 10:31) - PLID 34430
						return true;
					}
					else {
						//Failed to commit edit.
						return false;
					}
				}
				else {
					//Editing not supported.
					return false;
				}				
			}
			
		} else {
			// ValidateFilter failed
			return false;
		}
	} NxCatchAll("CFilterEditDlg::Save");
	return false;
}

bool CFilterEditDlg::Load()
{
	if (m_dlgFilter.m_nFilterId == FILTER_ID_NEW) {
		// This is a new filter so try to use the current filter string
		if (m_strFilterString.IsEmpty()) {
			m_dlgFilter.RemoveAll();
			m_dlgFilter.AddDetail();
			m_strFilterName = FILTER_NAME_NEW;
			GetDlgItem(IDC_FILTER_BKG_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_SHOW);
			SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
			return true;
		} else {
			if (m_dlgFilter.SetFilterString(m_strFilterString)) {
				m_dlgFilter.AddDetail();
				GetDlgItem(IDC_FILTER_BKG_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_SHOW);
				SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
				return true;
			} else {
				// Could not load the filter because it couldn't be parsed
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "CFilterEditDlg::Load Error 65: Could not parse filter %s", m_strFilterString);
				m_dlgFilter.RemoveAll();
				m_dlgFilter.AddDetail();
				m_strFilterName = FILTER_NAME_NEW;
				GetDlgItem(IDC_FILTER_BKG_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_SHOW);
				SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
				return false;
			}
		}
	} else if (m_dlgFilter.m_nFilterId == FILTER_ID_TEMPORARY) {
		// This is a filter that we won't be saving, just generating a filter clause (FROM and WHERE)
		if (m_strFilterString.IsEmpty()) {
			m_dlgFilter.RemoveAll();
			m_dlgFilter.AddDetail();
			GetDlgItem(IDC_FILTER_BKG_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_HIDE);
			m_strFilterName = FILTER_NAME_TEMPORARY;
			SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
			return true;
		} else {
			if (m_dlgFilter.SetFilterString(m_strFilterString)) {
				m_dlgFilter.AddDetail();
				GetDlgItem(IDC_FILTER_BKG_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_HIDE);
				SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
				return true;
			} else {
				// Could not load the filter because it couldn't be parsed
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "CFilterEditDlg::Load Error 65: Could not parse filter %s", m_strFilterString);
				m_dlgFilter.RemoveAll();
				m_dlgFilter.AddDetail();
				m_strFilterName = FILTER_NAME_TEMPORARY;
				GetDlgItem(IDC_FILTER_BKG_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_HIDE);
				SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
				return false;
			}
		}
	} else {
		try {
			// Load the filter string from the database
			_RecordsetPtr prs = CreateRecordset(
				"SELECT Name, Filter FROM FiltersT WHERE ID = %li", 
				m_dlgFilter.m_nFilterId);
			if (!prs->eof) {
				// Found filter so load it
				FieldsPtr flds = prs->Fields;
				m_strFilterName = m_strOldFilterName = AdoFldString(flds, "Name");
				m_strFilterString = AdoFldString(flds, "Filter");
				HR(prs->Close());
				if (m_dlgFilter.SetFilterString(m_strFilterString)) {
					// Success!
					m_dlgFilter.AddDetail();
					GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_SHOW);
					SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
					return true;
				} else {
					// Could load the filter from data, but could not show it on screen for some reason
					MsgBox(MB_ICONEXCLAMATION|MB_OK, "CFilterEditDlg::Load Error 125: Could not display filter %li\n\n%s", m_dlgFilter.m_nFilterId, m_strFilterString);
					m_dlgFilter.RemoveAll();
					m_dlgFilter.AddDetail();
					m_strFilterName = FILTER_NAME_NEW;
					GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_HIDE);
					SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
					return false;
				}
			} else {
				HR(prs->Close());
				// Couldn't find the filter
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "CFilterEditDlg::Load Error 250: Could not find filter %li", m_dlgFilter.m_nFilterId);
				m_dlgFilter.RemoveAll();
				m_dlgFilter.AddDetail();
				m_strFilterName = FILTER_NAME_NEW;
				GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_SHOW);
				SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
				return false;
			}

		} catch (_com_error e) {
			// I would have loved to use NxCatch but if you do that while a 
			// modal dialog is initializing it, amazingly it becomes modeless!!
			MsgBox(MB_ICONINFORMATION|MB_OK, 
				"Could not load filter \"%s\" because of the following error:\n\n%s (%s)", 
				m_strFilterName, (LPCTSTR)e.Description(), e.ErrorMessage());
		}
		// AddDetail with no parameters adds a blank one to the 
		// bottom, but only if there isn't already one there
		m_dlgFilter.AddDetail();
		GetDlgItem(IDC_FILTER_NAME_EDIT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SAVE_FILTER_AS)->ShowWindow(SW_SHOW);
		SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
		return false;
	}
}

int CFilterEditDlg::NewFilter()
{
	return EditFilter(FILTER_ID_NEW);
}

// Used for editing existing filters, as well as creating new ones
// Call with FILTER_ID_NEW ad the nFilterId to create a new filter
// Returns IDCANCEL immediately if called on a built-in filter, otherwise returns DoModal()
int CFilterEditDlg::EditFilter(long nFilterId, LPCTSTR strCurrentFilterString /*= NULL*/)
{
	if (nFilterId > 0) {
		// The caller passed the actual filter id so just load it like normal
		m_dlgFilter.m_nFilterId = nFilterId;
		return DoModal();
	} else if (nFilterId == FILTER_ID_NEW) {
		// The caller passed the "FILTER_ID_NEW" id
		if (strCurrentFilterString == NULL) {
			// The caller didn't specify a filter string so just load a blank filter
			m_dlgFilter.m_nFilterId = nFilterId;
			m_strFilterString.Empty();
			return DoModal();
		} else {
			// The caller specified a filter string so load it
			m_dlgFilter.m_nFilterId = nFilterId;
			m_strFilterString = strCurrentFilterString;
			m_strFilterName = FILTER_NAME_NEW;
			return DoModal();
		}
	} else if (nFilterId == FILTER_ID_TEMPORARY) {
		// The caller passed the "FILTER_ID_TEMPORARY" id
		if (strCurrentFilterString == NULL) {
			// The caller didn't specify a filter string so just load a blank filter
			m_dlgFilter.m_nFilterId = nFilterId;
			m_strFilterString.Empty();
			return DoModal();
		} else {
			// The caller specified a filter string so load it
			m_dlgFilter.m_nFilterId = nFilterId;
			m_strFilterString = strCurrentFilterString;
			m_strFilterName = FILTER_NAME_TEMPORARY;
			return DoModal();
		}
	} else {
		// Built-in filter
		MsgBox("Built-in filters may not be modified.");
		return IDCANCEL;
	}
}

int CFilterEditDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// (a.walling 2008-05-23 12:56) - PLID 30099
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_dlgFilter.Create(IDD_FILTER_DLG, this);
	
	return 0;
}

void CFilterEditDlg::OnClearBtn() 
{
	if (MsgBox(MB_ICONQUESTION|MB_YESNO, "Would you like to clear this filter?") == IDYES) {
		m_dlgFilter.DoScrollTo(SCROLL_TOP_POS);
		m_dlgFilter.RemoveAll();
		m_dlgFilter.AddDetail();
	}
}

void CFilterEditDlg::OnSaveFilterAs() 
{
	try {
		if(!ValidateFilter()) {
			//ValidateFilter() will have notified the user.
			return;
		}
		CString strEnteredName;
		GetDlgItemText(IDC_FILTER_NAME_EDIT, strEnteredName);
		if (IDOK == InputBoxLimited(this, "Enter a name for this filter", strEnteredName, "",50,false,false,NULL)) {
			_RecordsetPtr rsExisting = CreateRecordset("SELECT ID FROM FiltersT WHERE Name = '%s'", _Q(strEnteredName));
			if(rsExisting->eof) {
				//Save as new.
				if(m_pfnIsActionSupported(saAdd, m_nFilterType)) {
					m_strFilterName = strEnteredName;
					SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
					if(m_pfnCommitSubfilterAction(saAdd, m_nFilterType, m_dlgFilter.m_nFilterId, m_strFilterName, m_strFilterString, this)) {
						//auditing
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(-1, "",nAuditID, aeiFilterCreated, -1, "", m_strFilterName, aepMedium, aetCreated);
						return;
					}
					else {
						//Failed to save.
						return;
					}
				}
				else {
					//Adding filters not supported.
					MsgBox("This action is not supported.");
					return;
				}
			}
			else {
				//Do they want to overwrite?
				if(IDYES == MsgBox(MB_YESNO, "A filter with the name \"%s\" already exists.  Would you like to permanently overwrite it?", strEnteredName)) {
					//"Edit" the existing filter.
					m_dlgFilter.m_nFilterId = AdoFldLong(rsExisting, "ID");
					m_strFilterName = strEnteredName;
					SetDlgItemText(IDC_FILTER_NAME_EDIT, m_strFilterName);
					if(m_pfnIsActionSupported(saEdit, m_nFilterType)) {
						if(m_pfnCommitSubfilterAction(saEdit, m_nFilterType, m_dlgFilter.m_nFilterId, m_strFilterName, m_strFilterString, this)) {
							//auditing
							if(m_strOldFilterName != m_strFilterName) {
								long nAuditID = BeginNewAuditEvent();
								AuditEvent(-1, "", nAuditID, aeiFilter, -1, m_strOldFilterName, m_strFilterName, aepMedium, aetChanged);
							}
							// If we made it to here we have succeeded
							return;
						}
						else {
							//Failed to commit edit.
							return;
						}
					}
					else {
						//Editing not supported.
						MsgBox("This action is not supported");
						return;
					}
				}
			}
		}
	}NxCatchAll("Error in CFilterEditDlg::OnSaveFilterAs()");

}
