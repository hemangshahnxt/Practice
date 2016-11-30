// UTSSearchDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "UTSSearchDlg.h"
#include "NxUTS.h"
#include "EditSingleCodeDlg.h"
#include "AdministratorRc.h"


// CUTSSearchDlg dialog

// (d.singleton 2013-10-09 13:49) - PLID 58882 - added dialog to access codes on the utc.  

// (a.walling 2013-10-18 10:51) - PLID 59096 - UTS search should support other vocabularies beyond SNOMEDCT
namespace {
	namespace VocabList {
		enum Columns {
			Name = 0,
			Family,
			Description,
		};
	}
	namespace CodeList {
		enum Columns {
			Code = 0,
			Name,
			Vocab,
		};
	}
}

IMPLEMENT_DYNAMIC(CUTSSearchDlg, CNxDialog)


CUTSSearchDlg::CUTSSearchDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUTSSearchDlg::IDD, pParent, "CUTSSearchDlg")
	, m_bManuallyEdited(false)
{

}

CUTSSearchDlg::~CUTSSearchDlg()
{
}

void CUTSSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUTSSearchDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SEARCH, &CUTSSearchDlg::OnBnClickedSearch)
	ON_BN_CLICKED(IDC_ADD, &CUTSSearchDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDOK, &CUTSSearchDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CUTSSearchDlg message handlers
BEGIN_EVENTSINK_MAP(CUTSSearchDlg, CNxDialog)
	ON_EVENT(CUTSSearchDlg, IDC_UTS_CODES, 1, CUTSSearchDlg::SelChangingUTSCodes, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CUTSSearchDlg, IDC_VOCAB_LIST, 1, CUTSSearchDlg::SelChangingVocabList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

BOOL CUTSSearchDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	try {
		CheckRadioButton(IDC_SEARCH_BY_WORDS, IDC_SEARCH_BY_CODE, IDC_SEARCH_BY_WORDS);
		
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDOK))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDCANCEL))->AutoSet(NXB_CANCEL);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_SEARCH))->AutoSet(NXB_INSPECT);

		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_ADD))->AutoSet(NXB_NEW);

		m_dlCodes = BindNxDataList2Ctrl(IDC_UTS_CODES, GetRemoteData(), false);
		m_dlVocab = BindNxDataList2Ctrl(IDC_VOCAB_LIST, GetRemoteData(), false);
		// (s.tullis 04/11/2016) NX-100079 - cache our Snomed Version
		g_propManager.CachePropertiesInBulk("UTSSearchDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'CurrentSnomedVersion' "
			")", _Q(GetCurrentUserName())
			);

		// (a.walling 2013-10-18 10:51) - PLID 59096 - Populate vocab list
		{
			using namespace Nx::UTS;
			for each (const Metadata::RootSource& source in Metadata::GetRootSources()) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlVocab->GetNewRow();
				pRow->Value[VocabList::Name] = (LPCTSTR)source.name;
				pRow->Value[VocabList::Family] = (LPCTSTR)source.family;
				pRow->Value[VocabList::Description] = (LPCTSTR)source.description;
				m_dlVocab->AddRowSorted(pRow, NULL);
			}

			// (a.walling 2013-10-18 10:51) - PLID 59096 - Select SNOMEDCT by default for now
			// (a.walling 2014-10-16 16:59) - PLID 62911 - Update for SNOMEDCT_US
			m_dlVocab->SetSelByColumn(VocabList::Name, "SNOMEDCT_US");
		}

		return TRUE;
	}NxCatchAll(__FUNCTION__);

	// (a.walling 2013-10-18 10:51) - PLID 59096 - In case of error, just add a SNOMEDCT default
	try {
		if (m_dlVocab) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlVocab->GetNewRow();
			// (a.walling 2014-10-16 16:59) - PLID 62911 - Update for SNOMEDCT_US
			pRow->Value[VocabList::Name] = "SNOMEDCT_US";
			pRow->Value[VocabList::Family] = "SNOMEDCT";
			pRow->Value[VocabList::Description] = "US Edition of SNOMED CT";
			m_dlVocab->CurSel = m_dlVocab->AddRowSorted(pRow, NULL);
		}
	}NxCatchAll(__FUNCTION__" - Override");

	return TRUE;
}

void CUTSSearchDlg::SelChangingUTSCodes(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CUTSSearchDlg::OnBnClickedSearch()
{
	using namespace Nx::UTS;
	try {
		CString searchType;
		CString searchTarget;
		if (IsDlgButtonChecked(IDC_SEARCH_BY_CODE)) {
			searchType = "exact";
			searchTarget = "code";
		} else if (IsDlgButtonChecked(IDC_SEARCH_BY_WORDS)) {
			searchType = "words";
			searchTarget = "atom";
		} else {
			searchType = "approximate";
			searchTarget = "atom";
		}

		CWaitCursor wc;		
		//clear our datalist
		m_dlCodes->Clear();
		//search for codes
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		CString strSearchText, strCode, strName;
		GetDlgItemText(IDC_UTS_EDIT, strSearchText);
		// (a.walling 2013-10-18 10:51) - PLID 59096 - Include vocab in search results
		CString source = GetSelectedVocab();
		for each(Finder::Code result in Finder::FindSourceConcepts(strSearchText, source, searchType, searchTarget)) {
			//add rows to datalist
			pRow = m_dlCodes->GetNewRow();
			pRow->PutValue(CodeList::Code, _bstr_t(result.ui));
			pRow->PutValue(CodeList::Name, _bstr_t(result.label));
			pRow->PutValue(CodeList::Vocab, (LPCTSTR)source);
			m_dlCodes->AddRowAtEnd(pRow, NULL);
		}		
	}NxCatchAll(__FUNCTION__);
}

void CUTSSearchDlg::OnBnClickedOk()
{
	try {
		CWaitCursor wc;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlCodes->GetFirstSelRow();
		std::vector<CString> ExistingCodes;
		if(pRow) {
			while(pRow) {
				CString strCode = VarString(pRow->GetValue(CodeList::Code), "");
				CString strName = VarString(pRow->GetValue(CodeList::Name), "");
				CString strVocab = VarString(pRow->GetValue(CodeList::Vocab), "");
				CString strConceptId = Nx::UTS::Content::LookupConceptID(strCode, strVocab);
				// (a.walling 2013-10-18 10:51) - PLID 59096 - Codes are only unique within their vocabulary
				if(ReturnsRecordsParam("SELECT * FROM CodesT where Code = {STRING} AND Vocab = {STRING}", strCode, strVocab)) {					
					ExistingCodes.push_back(strCode);
					pRow = pRow->GetNextSelRow();
					continue;
				}
				else {
					// (a.walling 2013-10-18 10:51) - PLID 59096 - Insert vocab as well
					ExecuteParamSql("INSERT INTO CodesT(Code, Vocab, Name, ConceptID) VALUES({STRING}, {STRING}, {STRING}, {STRING});", 
						strCode, strVocab, strName, strConceptId);
					pRow = pRow->GetNextSelRow();
					continue;
				}				
			}
		}
		else {
			MessageBox("Please select a code(s) before trying to import.");
			return;
		}

		//build and display warning message
		if(ExistingCodes.size() > 0) {
			CString strWarning = "The following codes already exist in NexTech and will not be imported:\r\n\r\n";
			for each(CString code in ExistingCodes) {
				strWarning += code + ", ";
			}
			//take off last ", "
			strWarning = strWarning.Left(strWarning.ReverseFind(','));
			MessageBox(strWarning);
		}
	}NxCatchAll(__FUNCTION__);
	OnOK();
}

void CUTSSearchDlg::SelChangingVocabList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-10-18 10:51) - PLID 59096 - Will return selected vocab, or SNOMEDCT by default
CString CUTSSearchDlg::GetSelectedVocab()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlVocab->CurSel;
	if (pRow) {
		return VarString(pRow->Value[VocabList::Name]);
	} else {
		return "SNOMEDCT_US";
	}
}

// (a.walling 2013-10-21 10:19) - PLID 59113 - Dialog to manually add / edit a SNOMED / UTS code.
void CUTSSearchDlg::OnBnClickedAdd()
{
	try {
		if (IDOK == CEditSingleCodeDlg(this).DoModal()) {
			m_bManuallyEdited = true;
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-10-21 10:19) - PLID 59096 - Search when hitting enter if focus is in edit box
BOOL CUTSSearchDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus() == GetDlgItem(IDC_UTS_EDIT)) {
		PostMessage(WM_COMMAND, IDC_SEARCH);
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}

int CUTSSearchDlg::DoModal() 
{
	// this doesn't need an exception handler, don't be ridiculous
	int result = __super::DoModal();
	if (m_bManuallyEdited) {
		return IDOK;
	} else {
		return result;
	}
}

