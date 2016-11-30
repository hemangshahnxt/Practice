// NxCoCTemplateSheet.cpp : implementation file
//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
//

#include "stdafx.h"
#include "Practice.h"
#include "NxCoCTemplateSheet.h"
#include "FileUtils.h"

using namespace NXDATALIST2Lib;

// CNxCoCTemplateSheet dialog
enum eTemplateColumns {
	etcSelected = 0,
	etcFullPath,
	etcDisplayName,
	etcArrayIndex,		//Index of the m_aryTemplates array
};

enum eDuplicateColumns {
	edcFullPath = 0,
	edcDisplayName,
	edcArrayIndex,
	edcOverwrite,
	edcRenameNew,
	edcRenameExisting,
	edcSkip,
};

//Helper function that pulls the \Templates\ section off the front of a path, to 
//	help make an easier-to-read display for the user.
CString TrimTemplateTextFromPath(CString strIn)
{
	if(strIn.GetLength() > 11 && strIn.Left(11).CompareNoCase("\\Templates\\") == 0) {
		strIn = strIn.Mid(11);
	}

	return strIn;
}

IMPLEMENT_DYNAMIC(CNxCoCTemplateSheet, CNxCoCWizardSheet)

CNxCoCTemplateSheet::CNxCoCTemplateSheet(CNxCoCWizardMasterDlg* pParent /*=NULL*/)
	: CNxCoCWizardSheet(CNxCoCTemplateSheet::IDD, pParent)
{

}

CNxCoCTemplateSheet::~CNxCoCTemplateSheet()
{
}

void CNxCoCTemplateSheet::DoDataExchange(CDataExchange* pDX)
{
	CNxCoCWizardSheet::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNxCoCTemplateSheet, CNxCoCWizardSheet)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_TMP_OVERWRITE, &CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpOverwrite)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_TMP_RENAME_NEW, &CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpRenameNew)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_TMP_RENAME_EXISTING, &CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpRenameExisting)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_TMP_SKIP, &CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpSkip)
END_MESSAGE_MAP()


// CNxCoCTemplateSheet message handlers
BOOL CNxCoCTemplateSheet::OnInitDialog()
{
	try {
		m_pTemplateList = BindNxDataList2Ctrl(IDC_NXCOC_TEMPLATE_LIST, false);
		m_pDuplicateList = BindNxDataList2Ctrl(IDC_NXCOC_DUPLICATE_TEMPLATE_LIST, false);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CNxCoCTemplateSheet::Load()
{
	//Because templates are directly dependant on packets, if they go here then back, we must
	//	reload the entire interface every time.
	m_pTemplateList->Clear();
	m_pDuplicateList->Clear();

	//We need to pull all the array of packets into the datalist
	LoadTemplatesToDatalist();
}

BOOL CNxCoCTemplateSheet::Validate()
{
	//The validation of the packets will flag all templates as bImport = true, then go through
	//	and turn off any templates for individual packets which are unselected.  So at this point we will
	//	already have the non-displayed templates flagged as Skip.
	//For template-specific validation, we only need iterate through all the datalist rows, and
	//	change the flag for any that were unselected.
	CArray<CNxCoCTemplate, CNxCoCTemplate&> *paryTemplates = m_pdlgMaster->m_ImportInfo.GetTemplateArrayPtr();
	IRowSettingsPtr pRow = m_pTemplateList->GetFirstRow();
	while(pRow != NULL) {
		//We save the array index in the datalist, use that for lookup
		long nIdx = VarLong(pRow->GetValue(etcArrayIndex));
		CNxCoCTemplate tmp = paryTemplates->GetAt(nIdx);

		if(VarBool(pRow->GetValue(etcSelected))) {
			//In the "new" list, we choose to import newly
			tmp.eImportOption = ehiooImportNew;
		}
		else {
			//Do not import
			tmp.eImportOption = ehiooSkip;
		}

		//Reload the structure into the array
		paryTemplates->SetAt(nIdx, tmp);
		pRow = pRow->GetNextRow();
	}

	//Then go through the duplicate list, and update the status appropriately
	pRow = m_pDuplicateList->GetFirstRow();
	while(pRow != NULL) {
		//Get the index from the datalist
		long nIdx = VarLong(pRow->GetValue(edcArrayIndex));
		CNxCoCTemplate tmp = paryTemplates->GetAt(nIdx);

		if(VarBool(pRow->GetValue(edcOverwrite))) {
			tmp.eImportOption = ehiooOverwrite;
		}
		else if(VarBool(pRow->GetValue(edcRenameNew))) {
			tmp.eImportOption = ehiooRenameNew;
		}
		else if(VarBool(pRow->GetValue(edcRenameExisting))) {
			tmp.eImportOption = ehiooRenameExisting;
		}
		else if(VarBool(pRow->GetValue(edcSkip))) {
			tmp.eImportOption = ehiooSkip;
		}
		else {
			//Nothing selected, but we must have something!
			AfxMessageBox("You must make a selection for every duplicated template record.  Please review the list and try again.");
			return FALSE;
		}

		//Update our array
		paryTemplates->SetAt(nIdx, tmp);
		pRow = pRow->GetNextRow();
	}

	return TRUE;
}

void CNxCoCTemplateSheet::LoadTemplatesToDatalist()
{
	CArray<CNxCoCTemplate, CNxCoCTemplate&> *paryTemplates = m_pdlgMaster->m_ImportInfo.GetTemplateArrayPtr();

	_variant_t varTrue(VARIANT_TRUE, VT_BOOL);
	_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

	for(int i = 0; i < paryTemplates->GetSize(); i++) {
		CNxCoCTemplate tmp = paryTemplates->GetAt(i);

		//For each template, there are a few options:
		//	a)  A template can be a "spare" template (-1 packet ID).  We need to check to see if
		//		this template already exists, and give the user the chance to overwrite / etc.
		//	b)  The template could also be spare, but not have a duplicate, so we just offer yes/no.
		//	c)  The template could be part of a packet, and be a duplicate.  In this case we must force
		//		the user to pick a method.
		//	d)  The template is part of a packet, but not a duplicate.  In this case we don't even list it,
		//		we must import the template for the packet to be complete.

		//We will default all duplicates to Rename Existing, so that no template content is lost.

		if(tmp.nPacketID == -1) {
			//a + b

			CString strFullTempPath = GetSharedPath() ^ tmp.strRelativePath;
			if(FileUtils::DoesFileOrDirExist(strFullTempPath)) {
				//a - This is a spare template, but already exists.  Load it into the duplicate list so the user
				//	can choose what they wish to do with it.
				IRowSettingsPtr pRow = m_pDuplicateList->GetNewRow();
				pRow->PutValue(edcFullPath, _bstr_t(tmp.strRelativePath));
				pRow->PutValue(edcDisplayName, _bstr_t(TrimTemplateTextFromPath(tmp.strRelativePath)));
				pRow->PutValue(edcArrayIndex, (long)i);
				pRow->PutValue(edcOverwrite, varFalse);
				pRow->PutValue(edcRenameNew, varFalse);
				pRow->PutValue(edcRenameExisting, varTrue);
				pRow->PutValue(edcSkip, varFalse);
				m_pDuplicateList->AddRowSorted(pRow, NULL);
			}
			else {
				//b - This is a spare template, and is new.  Just a yes/no to import
				IRowSettingsPtr pRow = m_pTemplateList->GetNewRow();
				pRow->PutValue(etcSelected, varTrue);
				pRow->PutValue(etcFullPath, _bstr_t(tmp.strRelativePath));
				pRow->PutValue(etcDisplayName, _bstr_t(TrimTemplateTextFromPath(tmp.strRelativePath)));
				pRow->PutValue(etcArrayIndex, (long)i);
				m_pTemplateList->AddRowSorted(pRow, NULL);

				//set the import method to overwrite
				tmp.eImportOption = ehiooOverwrite;
				paryTemplates->SetAt(i, tmp);
			}
		}
		else {
			//c + d
			//We must also check if the template is already flagged as 'skip'.  It's possible that we have a template in a packet
			//	that the user chose to skip in step 1, but that template already exists.  We want to make sure it does not show up
			//	in the available list to import.
			if(tmp.eImportOption == ehiooSkip || tmp.eImportOption == ehiooSkipTemplateEntirelyNoPacket) {
				//Don't do a thing, just leave it out of the list, and remain as 'skipped' status
			}
			else {
				CString strFullTempPath = GetSharedPath() ^ tmp.strRelativePath;
				if(FileUtils::DoesFileOrDirExist(strFullTempPath)) {
					//c - This file is part of a packet, and it's duplicated.  We must have a file, so make the user pick.
					IRowSettingsPtr pRow = m_pDuplicateList->GetNewRow();
					pRow->PutValue(edcFullPath, _bstr_t(tmp.strRelativePath));
					pRow->PutValue(edcDisplayName, _bstr_t(TrimTemplateTextFromPath(tmp.strRelativePath)));
					pRow->PutValue(edcArrayIndex, (long)i);
					pRow->PutValue(edcOverwrite, varFalse);
					pRow->PutValue(edcRenameNew, varFalse);
					pRow->PutValue(edcRenameExisting, varTrue);
					pRow->PutValue(edcSkip, varFalse);
					m_pDuplicateList->AddRowSorted(pRow, NULL);
				}
				else {
					//d - This template is part of a packet, and does not already exist.  These will just automatically import, 
					//	we don't want to clutter things up by listing them.

					//Update the import type
					tmp.eImportOption = ehiooImportNew;
					paryTemplates->SetAt(i, tmp);
				}
			}
		}
	}
}
BEGIN_EVENTSINK_MAP(CNxCoCTemplateSheet, CNxCoCWizardSheet)
	ON_EVENT(CNxCoCTemplateSheet, IDC_NXCOC_DUPLICATE_TEMPLATE_LIST, 10, CNxCoCTemplateSheet::EditingFinishedNxcocDuplicateTemplateList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CNxCoCTemplateSheet::EditingFinishedNxcocDuplicateTemplateList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		//Ensure a valid row
		if(lpRow == NULL) {
			return;
		}
		IRowSettingsPtr pRow(lpRow);
		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		switch(nCol) {
			case edcOverwrite:
			case edcRenameNew:
			case edcRenameExisting:
			case edcSkip:
				{
					//We need to "fake" radio button functionality in the datalist between the checkboxes.  So if the user
					//	selects 'Overwrite', we need to uncheck rename new / rename exist / skip.  They can never have multiple
					//	options selected.
					if(VarBool(varNewValue) == FALSE) {
						//They're unselecting, in which case we have no work to do.
					}
					else {
						//unselect all the other columns in this row
						SelectSingleColumn(pRow, nCol);
					}
				}
				break;
		}

	} NxCatchAll("Error in EditingFinishedNxcocDuplicateTemplateList");
}

void CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpOverwrite()
{
	try {
		//Iterate through all the rows and select the 'overwrite' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcOverwrite);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllTmpOverwrite");
}

void CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpRenameNew()
{
	try {
		//Iterate through all the rows and select the 'rename new' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcRenameNew);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllTmpRenameNew");
}

void CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpRenameExisting()
{
	try {
		//Iterate through all the rows and select the 'rename existing' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcRenameExisting);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllTmpRenameExisting");
}

void CNxCoCTemplateSheet::OnBnClickedNxcocSelAllTmpSkip()
{
	try {
		//Iterate through all the rows and select the 'skip' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcSkip);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllTmpSkip");
}

//Helper function to ensure only 1 of the editable columns is chosen.  Given a row and a column to select, this
//	function will ensure that all other columns are unselected, and the given column is selected.
void CNxCoCTemplateSheet::SelectSingleColumn(IRowSettingsPtr pRow, short nCol)
{
	_variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	_variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	//nCol is the one we want selected, so do not unselect it.  All else, go ahead.
	pRow->PutValue(nCol, varTrue);

	if(nCol != edcOverwrite) {
		pRow->PutValue(edcOverwrite, varFalse);
	}

	if(nCol != edcRenameNew) {
		pRow->PutValue(edcRenameNew, varFalse);
	}

	if(nCol != edcRenameExisting) {
		pRow->PutValue(edcRenameExisting, varFalse);
	}

	if(nCol != edcSkip) {
		pRow->PutValue(edcSkip, varFalse);
	}
}
