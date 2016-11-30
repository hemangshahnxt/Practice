// NxCoCPacketSheet.cpp : implementation file
//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer
//

#include "stdafx.h"
#include "Practice.h"
#include "NxCoCPacketSheet.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ePacketColumns {
	epcSelected = 0,
	epcID,
	epcName,
	epcProcRel,
	epcArrayIndex,
};

enum eDuplicateColumns {
	edcID = 0,
	edcName,
	edcProcRel,
	edcArrayIndex,
	edcOverwrite,
	edcRenameNew,
	edcRenameExisting,
	edcSkip,
};

// CNxCoCPacketSheet dialog

IMPLEMENT_DYNAMIC(CNxCoCPacketSheet, CNxCoCWizardSheet)

CNxCoCPacketSheet::CNxCoCPacketSheet(CNxCoCWizardMasterDlg* pParent /*=NULL*/)
	: CNxCoCWizardSheet(CNxCoCPacketSheet::IDD, pParent)
{

}

CNxCoCPacketSheet::~CNxCoCPacketSheet()
{
}

void CNxCoCPacketSheet::DoDataExchange(CDataExchange* pDX)
{
	CNxCoCWizardSheet::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CNxCoCPacketSheet, CNxCoCWizardSheet)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_PACKET_OVERWRITE, &CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketOverwrite)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_PACKET_RENAME_NEW, &CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketRenameNew)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_PACKET_RENAME_EXISTING, &CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketRenameExisting)
	ON_BN_CLICKED(IDC_NXCOC_SEL_ALL_PACKET_SKIP, &CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketSkip)
END_MESSAGE_MAP()


// CNxCoCPacketSheet message handlers

BOOL CNxCoCPacketSheet::OnInitDialog()
{
	try {
		m_pPacketList = BindNxDataList2Ctrl(IDC_NXCOC_PACKET_LIST, false);
		m_pDuplicateList = BindNxDataList2Ctrl(IDC_NXCOC_DUPLICATE_PACKET_LIST, false);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

void CNxCoCPacketSheet::Load()
{
	//Assume that if there's nothing in either list, we're on an initial load.  Otherwise they've gone forward/back.
	bool bInitialLoad = false;

	if(m_pPacketList->GetRowCount() == 0 && m_pDuplicateList->GetRowCount() == 0) {
		bInitialLoad = true;
	}

	if(bInitialLoad) {
		//We need to pull all the array of packets into the datalist
		LoadPacketsToDatalist();
	}
}

//Validation is speed-optimized to assume that most of the packets will be selected.
BOOL CNxCoCPacketSheet::Validate()
{
	//When they hit next, we need to iterate through all the rows in the datalist, and confirm the import flag on each
	//	CNxCoCPacket object.
	CArray<CNxCoCPacket, CNxCoCPacket&> *paryPackets = m_pdlgMaster->m_ImportInfo.GetPacketArrayPtr();


	//However, before they get started, we need to flag every template as unknown status.  This is faster than looping
	//	for every template in the below loop.
	CArray<CNxCoCTemplate, CNxCoCTemplate&> *paryTemplates = m_pdlgMaster->m_ImportInfo.GetTemplateArrayPtr();
	for(int nTemplateIdx = 0; nTemplateIdx < paryTemplates->GetSize(); nTemplateIdx++) {
		CNxCoCTemplate tmp = paryTemplates->GetAt(nTemplateIdx);

		if(tmp.eImportOption != ehiooUnknown) {
			tmp.eImportOption = ehiooUnknown;
			//reset the array index
			paryTemplates->SetAt(nTemplateIdx, tmp);
		}
	}


	//first, go through our "new" list and see which ones the user manually removed
	IRowSettingsPtr pRow = m_pPacketList->GetFirstRow();
	while(pRow != NULL) {

		//We saved the array index when loading the datalist, so use that to do a lookup.
		long nIdx = VarLong(pRow->GetValue(epcArrayIndex));
		CNxCoCPacket pkt = paryPackets->GetAt(nIdx);

		//If the row is checked, set the import option for importing newly
		if(VarBool(pRow->GetValue(epcSelected))) {
			pkt.eImportOption = ehiooImportNew;
		}
		else {
			pkt.eImportOption = ehiooSkip;

			//And if it's false, we need to iterate through all the templates, and tell each one that
			//	is in this packet not to import.
			for(int i = 0; i < paryTemplates->GetSize(); i++) {
				CNxCoCTemplate tmp = paryTemplates->GetAt(i);
				if(tmp.nPacketID == pkt.nID) {
					//This template is in this packet -- don't import it at all
					tmp.eImportOption = ehiooSkipTemplateEntirelyNoPacket;

					//update array
					paryTemplates->SetAt(i, tmp);
				}
			}
		}

		//Reload the structure in the array
		paryPackets->SetAt(nIdx, pkt);
		pRow = pRow->GetNextRow();
	}


	//next, go through all our "duplicate" list, and update the packet accordingly
	pRow = m_pDuplicateList->GetFirstRow();
	while(pRow != NULL) {
		//Get the index back out of the datalist
		long nIdx = VarLong(pRow->GetValue(edcArrayIndex));
		CNxCoCPacket pkt = paryPackets->GetAt(nIdx);

		//Go through each of the checkbox options, and flag our packet accordingly
		if(VarBool(pRow->GetValue(edcOverwrite))) {
			pkt.eImportOption = ehiooOverwrite;
		}
		else if(VarBool(pRow->GetValue(edcRenameNew))) {
			pkt.eImportOption = ehiooRenameNew;
		}
		else if(VarBool(pRow->GetValue(edcRenameExisting))) {
			pkt.eImportOption = ehiooRenameExisting;
		}
		else if(VarBool(pRow->GetValue(edcSkip))) {
			pkt.eImportOption = ehiooSkip;

			//And if it's skip, we need to iterate through all the templates, and tell each one that
			//	is in this packet not to import.
			for(int i = 0; i < paryTemplates->GetSize(); i++) {
				CNxCoCTemplate tmp = paryTemplates->GetAt(i);
				if(tmp.nPacketID == pkt.nID) {
					//This template is in this packet -- don't import it
					tmp.eImportOption = ehiooSkipTemplateEntirelyNoPacket;

					//update array
					paryTemplates->SetAt(i, tmp);
				}
			}
		}
		else {
			//One of those options must be selected or we must fail
			AfxMessageBox("You must select a method of handling all duplicate packets.  Please check your list and try again.");
			return FALSE;
		}

		//Update our array
		paryPackets->SetAt(nIdx, pkt);
		pRow = pRow->GetNextRow();
	}

	return TRUE;
}

//Function to do a 1-time load from the arrays into datalist objects.  This should only happen once per dialog, after the
//	arrays have been loaded, and before the user is able to interact with the interface.
void CNxCoCPacketSheet::LoadPacketsToDatalist()
{
	_variant_t varFalse(VARIANT_FALSE, VT_BOOL);
	_variant_t varTrue(VARIANT_TRUE, VT_BOOL);

	CArray<CNxCoCPacket, CNxCoCPacket&> *paryPackets = m_pdlgMaster->m_ImportInfo.GetPacketArrayPtr();

	//First, we need to get a map of all the existing packet names in the database.  We'll put these in a map for
	//	ease of lookup, and can then compare for duplicates as we load.  To ensure proper string comparisons, all
	//	values in the map will be keyed on the template name in all upper case.
	CMap<CString, LPCTSTR, BOOL, BOOL> mapExistingPackets;
	{
		_RecordsetPtr prs = CreateParamRecordset("SELECT Name FROM PacketsT WHERE Deleted = 0");
		while(!prs->eof) {
			CString strName = AdoFldString(prs, "Name");
			strName.MakeUpper();

			mapExistingPackets.SetAt(strName, TRUE);
			prs->MoveNext();
		}
	}

	//Now loop over our array of packets, and put them in the proper list
	for(int i = 0; i < paryPackets->GetSize(); i++) {
		CNxCoCPacket pkt = paryPackets->GetAt(i);

		//Determine which list it goes into by checking for a duplicate name
		CString strNameToTest = pkt.strName;
		strNameToTest.MakeUpper();

		BOOL bExists = FALSE;
		if(mapExistingPackets.Lookup((LPCTSTR)strNameToTest, bExists)) {
			//This is a duplicate packet name
			IRowSettingsPtr pRow = m_pDuplicateList->GetNewRow();
			pRow->PutValue(edcID, (long)pkt.nID);
			pRow->PutValue(edcName, _bstr_t(pkt.strName));
			pRow->PutValue(edcProcRel, pkt.bProcRelated ? varTrue : varFalse);
			//Track the array index for future looks back
			pRow->PutValue(edcArrayIndex, (long)i);

			//We will default everything to Overwrite, because these are just packets, it's not "real"
			//	content or anything important.  
			pRow->PutValue(edcOverwrite, varTrue);
			pRow->PutValue(edcRenameNew, varFalse);
			pRow->PutValue(edcRenameExisting, varFalse);
			pRow->PutValue(edcSkip, varFalse);

			m_pDuplicateList->AddRowSorted(pRow, NULL);
		}
		else {
			//This is a new packet
			IRowSettingsPtr pRow = m_pPacketList->GetNewRow();
			pRow->PutValue(epcSelected, varTrue);
			pRow->PutValue(epcID, (long)pkt.nID);
			pRow->PutValue(epcName, _bstr_t(pkt.strName));
			pRow->PutValue(epcProcRel, pkt.bProcRelated ? varTrue : varFalse);

			//Because of the nature of our importer, the m_ImportInfo is loaded once and only once.  This means
			//	that the array of packets is static and will not change.  We can safely record the index where
			//	this packet exists, and reference it later to gain speed.
			pRow->PutValue(epcArrayIndex, (long)i);
			m_pPacketList->AddRowSorted(pRow, NULL);

			//Set this packet's import option to "Overwrite", because it's new.
			pkt.eImportOption = ehiooOverwrite;
			paryPackets->SetAt(i, pkt);
		}
	}
}
BEGIN_EVENTSINK_MAP(CNxCoCPacketSheet, CNxCoCWizardSheet)
	ON_EVENT(CNxCoCPacketSheet, IDC_NXCOC_DUPLICATE_PACKET_LIST, 10, CNxCoCPacketSheet::EditingFinishedNxcocDuplicatePacketList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CNxCoCPacketSheet::EditingFinishedNxcocDuplicatePacketList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		//Ensure a valid row
		if(lpRow == NULL) {
			return;
		}
		IRowSettingsPtr pRow(lpRow);

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
						SelectSingleColumn(pRow, nCol);
					}
				}
				break;
		}
	} NxCatchAll("Error in EditingFinishedNxcocDuplicatePacketList");
}

void CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketOverwrite()
{
	try {
		//Iterate through all the rows and select the 'overwrite' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcOverwrite);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllPacketOverwrite");
}

void CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketRenameNew()
{
	try {
		//Iterate through all the rows and select the 'rename new' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcRenameNew);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllPacketRenameNew");
}

void CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketRenameExisting()
{
	try {
		//Iterate through all the rows and select the 'rename existing' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcRenameExisting);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllPacketRenameExisting");
}

void CNxCoCPacketSheet::OnBnClickedNxcocSelAllPacketSkip()
{
	try {
		//Iterate through all the rows and select the 'skip' column, unselecting the other columns
		IRowSettingsPtr pRow = m_pDuplicateList->GetFirstRow();
		while(pRow != NULL) {
			SelectSingleColumn(pRow, edcSkip);

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll("Error in OnBnClickedNxcocSelAllPacketSkip");
}

//Helper function to select a single column in the duplicate list.  Given a row, and the column you want selected, 
//	this function will ensure that column is selected, and none of the other editable columns are checked.
void CNxCoCPacketSheet::SelectSingleColumn(IRowSettingsPtr pRow, short nCol)
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
