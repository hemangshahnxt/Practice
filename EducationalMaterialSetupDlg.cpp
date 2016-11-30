// EducationalMaterialSetupDlg.cpp : implementation file
//
// (c.haag 2010-09-22 15:54) - PLID 40629 - Initial implementation. This class is used by
// the user to configure patient educational "packets" which are really just a list of letter
// writing filters and templates.
//

#include "stdafx.h"
#include "Practice.h"
#include "EducationalMaterialSetupDlg.h"
#include "SingleSelectDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// CEducationalMaterialSetupDlg dialog

typedef enum {
	ecID = 0,
	ecFilterID = 1,
	ecMergeTemplateID = 2
} EColumns;

IMPLEMENT_DYNAMIC(CEducationalMaterialSetupDlg, CNxDialog)

CEducationalMaterialSetupDlg::CEducationalMaterialSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEducationalMaterialSetupDlg::IDD, pParent),
	m_ChangesTracker("EducationTemplatesT")
{
	m_bWordTemplatesEnumerated = FALSE;
}

CEducationalMaterialSetupDlg::~CEducationalMaterialSetupDlg()
{

}

// Populates m_astrWordTemplates with a list of all templates in the shared path
void CEducationalMaterialSetupDlg::PopulateWordTemplateArray(const CString& strPath)
{
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strPath ^ "*.*");
	CString strSharedPath = GetSharedPath() ^ "";
	strSharedPath.TrimRight('\\');
	while (bWorking) {
		bWorking = finder.FindNextFile();

		if (finder.IsDots()) {
			// Skip dots
		}
		else if(finder.IsDirectory()) {
			// Recurse
			PopulateWordTemplateArray(finder.GetFilePath());
		}
		else {
			// Add it to our list if it's a Word template. Being consistent with how other areas of the program do it.
			CString strFileName = finder.GetFilePath();
			strFileName.MakeUpper();
			if(strFileName.Right(4) == ".DOT" || (strFileName.Right(5) == ".DOTX") || (strFileName.Right(5) == ".DOTM")) 
			{
				// Cull the shared path out of the file name. We want the results to be consistent with the
				// content of MergeTemplatesT which begins with \TEMPLATES
				strFileName = strFileName.Right( strFileName.GetLength() - strSharedPath.GetLength() );
				m_astrWordTemplates.Add(strFileName);
			}
		}
	}
}

// Populates m_astrWordTemplates with a list of all templates in the shared path
void CEducationalMaterialSetupDlg::PopulateWordTemplateArray()
{
	if (!m_bWordTemplatesEnumerated) {
		CWaitCursor wc;
		PopulateWordTemplateArray(GetSharedPath() ^ "Templates");
		m_bWordTemplatesEnumerated = TRUE;
	}
}

void CEducationalMaterialSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE, m_btnDelete);
}


BEGIN_MESSAGE_MAP(CEducationalMaterialSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD, &CEducationalMaterialSetupDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_DELETE, &CEducationalMaterialSetupDlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDOK, &CEducationalMaterialSetupDlg::OnBnClickedOK)
END_MESSAGE_MAP()


// CEducationalMaterialSetupDlg message handlers

BOOL CEducationalMaterialSetupDlg::OnInitDialog()
{
	try 
	{
		CNxDialog::OnInitDialog();

		// Set up controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		// Populate the embedded dropdown of the template column with data using a query.
		// The reason we don't have it pull from MergeTemplatesT directly is because we are
		// liable to change the content on the fly, and we don't want to make a trip to the SQL
		// server every time.
		//
		// We also choose not to populate it with actual Windows folder paths because it will
		// easily overcrowd the dropdown and make selections difficult for users. By restricting
		// the list to templates that have been used before, they can more easily find those.
		//
		CString strCombo = ";;2;;-1;\"{ Select a template }\";1;-2;\"{ Show all templates... }\";1;", str;
		_RecordsetPtr prs = CreateRecordset("SELECT ID, Path FROM MergeTemplatesT ORDER BY Path");
		while (!prs->eof)
		{
			long nID = AdoFldLong(prs, "ID");
			CString strPath = AdoFldString(prs, "Path");
			str.Format("%d;\"%s\";1;", nID, strPath);
			GrowFastConcat(strCombo, str.GetLength(), str);
			m_mapEmbeddedMergeTemplateIDs.SetAt(nID,TRUE);
			prs->MoveNext();
		}
		prs->Close();

		m_dlList = BindNxDataList2Ctrl(IDC_LIST_EDUCATION, false);
		IColumnSettingsPtr(m_dlList->GetColumn(ecMergeTemplateID))->ComboSource = _bstr_t(strCombo);
		m_dlList->Requery();
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CEducationalMaterialSetupDlg::OnBnClickedAdd()
{
	try
	{
		// Add an empty item and have the user edit it
		CStringArray astrFields;
		CArray<_variant_t,_variant_t&> aValues;
		astrFields.Add("FilterID"); aValues.Add(_variant_t(g_cvarNull));
		astrFields.Add("MergeTemplateID"); aValues.Add(_variant_t(-1L));
		// Make sure the change is applied to data
		long nNewRecordID = m_ChangesTracker.SetCreated(astrFields, aValues);

		// Now update the list
		IRowSettingsPtr pRow = m_dlList->GetNewRow();
		pRow->Value[ecID] = nNewRecordID;
		pRow->Value[ecFilterID] = g_cvarNull;
		pRow->Value[ecMergeTemplateID] = -1L;
		m_dlList->StartEditing(m_dlList->AddRowAtEnd(pRow, NULL), ecFilterID);
	}
	NxCatchAll(__FUNCTION__);
}

void CEducationalMaterialSetupDlg::OnBnClickedDelete()
{
	try
	{
		// Try to delete the selected item
		if (NULL == m_dlList->CurSel) {
			AfxMessageBox("Please select an item to delete.");
		}
		else {
			// Make sure the change is applied to data
			IRowSettingsPtr pRow = m_dlList->CurSel;
			m_ChangesTracker.SetDeleted(VarLong(pRow->GetValue(ecID)));
			m_dlList->RemoveRow(pRow);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEducationalMaterialSetupDlg::OnBnClickedOK()
{
	try {
		// See if any filters are without a Word Document template.
		if (NULL != m_dlList->FindByColumn(ecMergeTemplateID, -1L, NULL, VARIANT_TRUE))
		{
			AfxMessageBox("You have at least one filter without a template selected. Please choose a template or delete the filter.", MB_ICONSTOP | MB_OK);
		}
		else {
			// Save changes
			m_ChangesTracker.DoDatabaseUpdate();

			// All done!
			__super::OnOK();
		}
	}
	NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEducationalMaterialSetupDlg, CNxDialog)
	ON_EVENT(CEducationalMaterialSetupDlg, IDC_LIST_EDUCATION, 10, CEducationalMaterialSetupDlg::EditingFinishedListEducation, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEducationalMaterialSetupDlg, IDC_LIST_EDUCATION, 7, CEducationalMaterialSetupDlg::RButtonUpListEducation, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEducationalMaterialSetupDlg, IDC_LIST_EDUCATION, 9, CEducationalMaterialSetupDlg::EditingFinishingListEducation, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CEducationalMaterialSetupDlg::EditingFinishingListEducation(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try
	{
		// If the user chose the option to show all templates, handle it here
		if (ecMergeTemplateID == nCol
			&& pbCommit && *pbCommit
			&& pvarNewValue && -2 ==VarLong(*pvarNewValue)
			)
		{
			// Get a list of all templates in the shared templates folder
			PopulateWordTemplateArray();
			// Now have the user select one
			CSingleSelectDlg dlg(this);
			if (VarLong(varOldValue,-1) > 0) {
				IRowSettingsPtr pRow(lpRow);
				if (NULL != pRow) {
					CString strTemplate = pRow->GetOutputValue(ecMergeTemplateID);
					dlg.PreSelect(strTemplate);
				}
			}			
			if (IDOK == dlg.Open(&m_astrWordTemplates, "Please select an existing Microsoft Word template with educational material.", true))
			{
				CString strTemplate = m_astrWordTemplates.GetAt( dlg.GetSelectedID() );

				// Ensure the template is in MergeTemplatesT. This query will do that, and return the ID of the template.
				_RecordsetPtr prs = CreateParamRecordset(
					"SET NOCOUNT ON \r\n"
					"DECLARE @ID INT \r\n"
					"IF ({STRING} NOT IN (SELECT Path FROM MergeTemplatesT)) BEGIN \r\n"
					"	BEGIN TRAN  \r\n"
					"	SET @ID = (SELECT COALESCE(MAX(ID),0) + 1 FROM MergeTemplatesT) \r\n"
					"	IF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END \r\n" 
					"	INSERT INTO MergeTemplatesT (ID, Path) VALUES (@ID, {STRING}) \r\n"
					"	COMMIT TRAN  \r\n"
					"END \r\n"
					"ELSE BEGIN  \r\n"
					"	SET @ID = (SELECT ID FROM MergeTemplatesT WHERE Path = {STRING})  \r\n"
					"END \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT @ID AS ResultID "
					,strTemplate, strTemplate, strTemplate);

				long nID = AdoFldLong(prs, "ResultID");
				BOOL bDummy;
				if (!m_mapEmbeddedMergeTemplateIDs.Lookup(nID, bDummy))
				{
					// If this is not in the combo source, then add it now
					CString str, strPath = strTemplate;
					str.Format("%d;\"%s\";1;", nID, strPath);
					IColumnSettingsPtr pCol(m_dlList->GetColumn(ecMergeTemplateID));
					pCol->ComboSource = pCol->ComboSource + _bstr_t(str);
					m_mapEmbeddedMergeTemplateIDs.SetAt(nID, TRUE);
				}
				else {
					// Already exists in the combo source
				}
				// Now assign the cell value
				_variant_t vValue = nID;
				VariantCopy(pvarNewValue, &vValue);
			}
			else {
				*pvarNewValue = varOldValue;
				*pbCommit = FALSE;
				*pbContinue = TRUE;
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CEducationalMaterialSetupDlg::EditingFinishedListEducation(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow)
		{
			// See if the filter value is empty. If it is, delete the row. This can only happen if a new row was
			// added but the user clicked off the dropdown.
			if (ecFilterID == nCol && (VT_EMPTY == varNewValue.vt || VT_NULL == varNewValue.vt)) {
				m_ChangesTracker.SetDeleted(VarLong(pRow->GetValue(ecID)));
				m_dlList->RemoveRow(pRow); // Although the event is for this row, the datalist still handles the removal gracefully
				return;
			}

			if (bCommit)
			{
				// Now search the list for duplicate filter-template pairs. If so, let the user either delete the row,
				// or reset the template
				const long nRecordID = VarLong(pRow->GetValue(ecID));
				const long nFilterID = VarLong(pRow->GetValue(ecFilterID));
				const long nMergeTemplateID = VarLong(pRow->GetValue(ecMergeTemplateID));
				if (nMergeTemplateID > -1) 
				{
					IRowSettingsPtr pRowCheck = m_dlList->GetFirstRow();
					while (NULL != pRowCheck) 
					{
						if (pRowCheck != pRow &&
							VarLong(pRowCheck->GetValue(ecFilterID)) == nFilterID &&
							VarLong(pRowCheck->GetValue(ecMergeTemplateID)) == nMergeTemplateID
							)
						{
							if (IDYES == AfxMessageBox("There is already an identical filter-template pair in this list. Would you like to remove this row?", MB_YESNO | MB_ICONQUESTION))
							{
								// Remove the row
								m_ChangesTracker.SetDeleted(nRecordID);
								m_dlList->RemoveRow(pRow); // Although the event is for this row, the datalist still handles the removal gracefully
							}
							else {
								// Reset the template
								m_ChangesTracker.SetModified(nRecordID, "MergeTemplateID", -1L);
								pRow->PutValue(ecMergeTemplateID, -1L);
							}
							return;
						}
						pRowCheck = pRowCheck->GetNextRow();
					}
				}

				// Now make sure the change is applied to data
				m_ChangesTracker.SetModified(nRecordID,
					(ecFilterID == nCol) ? "FilterID" : "MergeTemplateID", 
					(ecFilterID == nCol) ? nFilterID : nMergeTemplateID);
			}
			else {
				// Nothing to do; change was not committed
			}
		} // if (NULL != pRow)
	}
	NxCatchAll(__FUNCTION__);
}

void CEducationalMaterialSetupDlg::RButtonUpListEducation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) 
		{
			CMenu mnu;
			mnu.CreatePopupMenu();
			mnu.AppendMenu(MF_ENABLED, 1, "Delete");
			CPoint ptClicked(x, y);
			GetDlgItem(IDC_LIST_EDUCATION)->ClientToScreen(&ptClicked);
			m_dlList->CurSel = pRow;
			int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
			if(nResult == 1) 
			{
				// Make sure the change is applied to data
				m_ChangesTracker.SetDeleted(VarLong(pRow->GetValue(ecID)));
				// The user wants to delete the item
				m_dlList->RemoveRow(pRow);
			}
		}		
	}
	NxCatchAll(__FUNCTION__);
}
