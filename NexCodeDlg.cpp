// NexCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexCodeDlg.h"
#include "PracticeRc.h"
#include "GlobalStringUtils.h"
#include <foreach.h>

// CNexCodeDlg dialog
// (b.savon 2014-03-04 13:07) - PLID 60987 - Create a NexCode dialog that returns a DiagCode ID from the newly (or not) created ICD10 code in DiagCodes
using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CNexCodeDlg, CNxDialog)
// (s.dhole 2014-03-13 16:15) - PLID 61369
#define	IDM_HIERARCHY_LEVEL					40001 

// (s.dhole 2014-03-21 09:02) - PLID  61376
enum PreferredContactType
{
	hlCode=0,
	hlDescription,
	hlMin,
	hlMax,
	hlIsCharacter,
};


// (s.dhole 2014-03-14 10:33) - PLID 60989
CFont m_CodeFont;
CFont m_CodeFontTital;


CNexCodeDlg::CNexCodeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexCodeDlg::IDD, pParent)
{
	m_nDiagCodeID_ICD10 = -1;
	m_strDiagCodeDescription_ICD10 = "";
	m_strDiagCode_ICD10 = "";
	m_bIsICD10CMSelected = TRUE;
}


CNexCodeDlg::~CNexCodeDlg()
{
// (s.dhole 2014-03-14 10:33) - PLID 60989 Cleanup	
	m_CodeFont.DeleteObject(); 
	m_CodeFontTital.DeleteObject(); 
	m_arryNexCodeHistory.RemoveAll(); 
}


void CNexCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEXCODE_OK, m_btnOK);
	DDX_Control(pDX, ID_BTN_NEXCODE_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NXC_NEXCODE_TOP_BACK, m_nxcTopBack);
	DDX_Control(pDX, IDC_NXC_NEXCODE_BOTTOM_BACK, m_nxcBottomBack);
	DDX_Control(pDX, IDC_NEXCODE_BACK_BTN, m_btnBack);
	DDX_Control(pDX, IDC_NEXCODE_ICD10CM_REDIO_BTN, m_radioICD10CM);
	DDX_Control(pDX, IDC_NEXCODE_ICD10PCS_REDIO_BTN, m_radioICD10PCS);
	DDX_Control(pDX, IDC_NEXCODE_CODE_LABLE, m_nxstaticCode);
	DDX_Control(pDX, IDC_NEXCODE_CODE_DESCRIPTION_LABLE, m_nxstaticCodeDetail);
}



BEGIN_MESSAGE_MAP(CNexCodeDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NEXCODE_ICD10CM_REDIO_BTN, &CNexCodeDlg::OnBnClickedNexcodeIcd10cmRedioBtn)
	ON_BN_CLICKED(IDC_NEXCODE_ICD10PCS_REDIO_BTN, &CNexCodeDlg::OnBnClickedNexcodeIcd10pcsRedioBtn)
	ON_BN_CLICKED(ID_BTN_NEXCODE_CANCEL, &CNexCodeDlg::OnBnClickedBtnNexcodeCancel)
	ON_BN_CLICKED(IDC_NEXCODE_OK, &CNexCodeDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CNexCodeDlg message handlers


BEGIN_EVENTSINK_MAP(CNexCodeDlg, CNxDialog)
	ON_EVENT(CNexCodeDlg, IDC_NEXCODE_HISTORY_HIERARCHY_LIST, 19, CNexCodeDlg::LeftClickNexcodeHistoryHierarchyList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()



BOOL CNexCodeDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();
		m_nxcTopBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxcBottomBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (s.dhole 2014-03-13 16:34) - PLID 60988
		m_btnBack.AutoSet(NXB_LEFT); 
		m_pCodeHierachyList= BindNxDataList2Ctrl(IDC_NEXCODE_HISTORY_HIERARCHY_LIST,false);
		// (s.dhole 2014-03-14 10:33) - PLID 60989
		m_arryNexCodeHistory.RemoveAll(); 
		CreateCompatiblePointFont(&m_CodeFont, 220, "Arial Bold");
		m_nxstaticCode.SetFont(&m_CodeFont);
		CreateCompatiblePointFont(&m_CodeFontTital, 90, "Arial Bold");
		GetDlgItem(IDC_NEXCODE_CODE_RESULT_LABLE)->SetFont(&m_CodeFontTital);
		m_radioICD10CM.SetCheck(BST_CHECKED);
		m_radioICD10PCS.SetCheck(BST_UNCHECKED);
		m_bIsICD10CMSelected = TRUE;
		CWaitCursor cwait;
		UpdateCodeInitialText();
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}



// (s.dhole 2014-03-13 17:10) - PLID 60988
void CNexCodeDlg::GoBackAtHierarchy(NexCodeHistory& pNexCodeHistory) 
{
	try{
		int nSize = m_arryNexCodeHistory.GetSize() - 1;
		for(int nId=nSize; nId>0; --nId ){
			NexCodeHistory& pNexCode = m_arryNexCodeHistory[nId] ; 
			if (pNexCodeHistory.nIndex < pNexCode.nIndex ){
				m_arryNexCodeHistory.RemoveAt(nId) ;
			}
		}
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2014-03-14 09:58) - PLID 60989
void CNexCodeDlg::OnBnClickedNexcodeIcd10cmRedioBtn()
{
	try{
		if (IsICD10CMSelected() == FALSE){
			// (s.dhole 2014-03-14 08:57) - PLID 60989 
			// check if we can change code type
			if (CanChangeCodeType()== FALSE){
				return;
			}
			CWaitCursor cwait;
			RestAllchages();
			m_bIsICD10CMSelected = TRUE;
			UpdateCodeInitialText();
			m_radioICD10PCS.SetCheck(BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-03-14 09:58) - PLID 60989
void CNexCodeDlg::OnBnClickedNexcodeIcd10pcsRedioBtn()
{
	try{
		if (IsICD10CMSelected()){
			// (s.dhole 2014-03-14 08:57) - PLID 60989 
			// check if we can change code type
			if (CanChangeCodeType()== FALSE){
				return;
			}
			CWaitCursor cwait;
			RestAllchages();
			m_bIsICD10CMSelected = FALSE;
			UpdateCodeInitialText();
			m_radioICD10CM.SetCheck(BST_UNCHECKED);
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-03-21 16:01) - PLID 60989
BOOL CNexCodeDlg::CanChangeCodeType()
{
	// check if we start bulding a code
	if (IsCodeBuildInProcess() ==FALSE){
		return TRUE;
	}
	// check if we started bulding a code
	CString strMsg = "You have started building an ICD-10 code.\n\nAre you sure you wish to start over?";
	if (IDYES== MessageBox(strMsg ,"Practice", MB_YESNO | MB_ICONINFORMATION)){
		return TRUE;
	}
	else{
		//Reset selection
		if (IsICD10CMSelected()){
			m_radioICD10CM.SetCheck(BST_CHECKED);
			m_radioICD10PCS.SetCheck(BST_UNCHECKED);
			m_radioICD10CM.SetFocus(); 
		}
		else{
			m_radioICD10PCS.SetCheck(BST_CHECKED);
			m_radioICD10CM.SetCheck(BST_UNCHECKED);
			m_radioICD10PCS.SetFocus(); 
		}
		return FALSE;
	}
}


// (s.dhole 2014-03-14 08:58) - PLID  60989 Update selection text
void CNexCodeDlg::UpdateCodeInitialText()
{
	LoadDataFromAPI(NexCodeHistory(0,"","_",IsICD10CMSelected()? "ICD-10-CM":"ICD-10-PCS"));
	// (s.dhole 2014-03-14 10:03) - PLID 61376
	ShowHideCotrol(TRUE);
 	LoadHierarchy();
}

// (s.dhole 2014-03-14 08:58) - PLID  60989 Update selection text
void CNexCodeDlg::UpdateCodeHeaderText(const CString strCode,const CString strCodeDetail, const  BOOL bShowDash/*=TRUE*/)
{
	SetDlgItemText(IDC_NEXCODE_CODE_LABLE, GetFormatedUIstring(strCode,bShowDash));
	SetDlgItemText(IDC_NEXCODE_CODE_DESCRIPTION_LABLE, strCodeDetail);
}


void CNexCodeDlg::OnBnClickedOk()
{
	try{
		SaveToDiagCodes();
		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}


// (r.farnworth 2014-03-11 10:45) - PLID 60990 - Save the code to DiagCodes by calling the import API function upon completion of the new code
void CNexCodeDlg::SaveToDiagCodes()
{
	Nx::SafeArray<IUnknown *> aryCommits;
	NexTech_Accessor::_DiagnosisCodeCommitPtr pCode(__uuidof(NexTech_Accessor::DiagnosisCodeCommit));
	pCode->PutCode(AsBstr(m_strDiagCode_ICD10));
	pCode->Putdescription(AsBstr(m_strDiagCodeDescription_ICD10));
	// (r.farnworth 2014-03-24 14:25) - PLID 60990 - Need to include PCS
	pCode->PutPCS(IsICD10CMSelected()?VARIANT_FALSE:VARIANT_TRUE);
	
	pCode->PutICD10(VARIANT_TRUE);
	aryCommits.Add(pCode);
	NexTech_Accessor::_DiagnosisCodesPtr pCodesCreated = GetAPI()->CreateDiagnosisCodes(GetAPISubkey(), GetAPILoginToken(), aryCommits);
	if(pCodesCreated->Codes == NULL){
		ThrowNxException("SaveToDiagCodes:  Did not get results for code creation.  Please attempt building your code again.");
	}

	Nx::SafeArray<IUnknown *> saryImportedCodes = pCodesCreated->Codes;
	if( saryImportedCodes.GetCount() != aryCommits.GetCount() ){
		// (r.farnworth 2014-03-27 14:13) - PLID 60990 - It didn't exist in data either, so let's warn the user that something went wrong. This should never happen.
		ThrowNxException("SaveToDiagCodes:  Did not get any results for code creation.");
	}

	NexTech_Accessor::_DiagnosisCodePtr pImportedICD10 = saryImportedCodes.GetAt(0);
	m_nDiagCodeID_ICD10 = AsLong(pImportedICD10->ID);
}


// (s.dhole 2014-03-13 16:15) - PLID 61369  check if there mouse button on hold on back button
BOOL CNexCodeDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message)
	{
	case WM_LBUTTONDOWN:
		{
			if (::GetDlgCtrlID(pMsg->hwnd)==IDC_NEXCODE_BACK_BTN){
				// (s.dhole 2014-03-13 16:21) - PLID 60988 Go back one step
				GoBackOneStep();
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			if (::GetDlgCtrlID(pMsg->hwnd)==IDC_NEXCODE_BACK_BTN){
				LoadBackMenu();
			}
		}
		break;
	default:
		break;
	}
	return CNxDialog::PreTranslateMessage(pMsg);
}


// (s.dhole 2014-03-24 16:14) - PLID  61508
// load data whic we passing to api
NexTech_Accessor::_SubCodeRequest_PCSPtr  CNexCodeDlg::LoadCurrentCharacters(NexCodeHistory &oNexCode)
{
	NexTech_Accessor::_SubCodeRequest_PCSPtr  pSubCodeRequest(__uuidof(NexTech_Accessor::SubCodeRequest_PCS));
	// Load Characters
	Nx::SafeArray<IUnknown *> aryNexCodeCharacters;
	for(int i =0; i<oNexCode.aCodeCharacters.GetSize(); i++){
		NexTech_Accessor::_NexCodeCharacterPtr  pNexCodeCharacters(__uuidof(NexTech_Accessor::NexCodeCharacter));
		pNexCodeCharacters->Characters = AsBstr(oNexCode.aCodeCharacters[i].strCharacters); 
		pNexCodeCharacters->description  = AsBstr(oNexCode.aCodeCharacters[i].strDescription ); 
		aryNexCodeCharacters.Add(pNexCodeCharacters); 
	}
	pSubCodeRequest->CharactersUsed  = aryNexCodeCharacters;
	return pSubCodeRequest;
}

// (s.dhole 2014-03-21 16:06) - PLID 61376
// load data whic we passing to api
NexTech_Accessor::_SubCodeRequest_CMPtr  CNexCodeDlg::LoadCurrentHierarchy(NexCodeHistory &oNexCode)
{
	NexTech_Accessor::_SubCodeRequest_CMPtr  pSubCodeRequest(__uuidof(NexTech_Accessor::SubCodeRequest_CM));
	// Load Hierarchy
	Nx::SafeArray<IUnknown *> aryNexCodeHierarchy;
	for(int i =0; i<oNexCode.aCodeHierarchy.GetSize(); i++){
		NexTech_Accessor::_NexCodeHierarchyPtr  pNexCodeHierarchy(__uuidof(NexTech_Accessor::NexCodeHierarchy));
		pNexCodeHierarchy->MinCode = AsBstr(oNexCode.aCodeHierarchy[i].strMinCode); 
		pNexCodeHierarchy->MaxCode = AsBstr(oNexCode.aCodeHierarchy[i].strMaxCode); 
		pNexCodeHierarchy->description  = AsBstr(oNexCode.aCodeHierarchy[i].strDescription); 
		aryNexCodeHierarchy.Add(pNexCodeHierarchy); 
	}
	
	// Load Characters
	Nx::SafeArray<IUnknown *> aryNexCodeCharacters;
	for(int i =0; i<oNexCode.aCodeCharacters.GetSize(); i++){
		NexTech_Accessor::_NexCodeCharacterPtr  pNexCodeCharacters(__uuidof(NexTech_Accessor::NexCodeCharacter));
		pNexCodeCharacters->Characters = AsBstr(oNexCode.aCodeCharacters[i].strCharacters); 
		pNexCodeCharacters->description  = AsBstr(oNexCode.aCodeCharacters[i].strDescription); 
		aryNexCodeCharacters.Add(pNexCodeCharacters); 
	}
	pSubCodeRequest->CharactersUsed = aryNexCodeCharacters;
	pSubCodeRequest->HierarchiesUsed = aryNexCodeHierarchy;
	return pSubCodeRequest;
}


// (s.dhole 2014-03-24 16:05) - PLID 61508 load PCS or CM code
BOOL CNexCodeDlg::GetDataFromAPI(NexCodeHistory &oNexCode,Nx::SafeArray<IUnknown *> &saryCodeHierarchy,
		Nx::SafeArray<IUnknown *> &saryCodeCharacters,Nx::SafeArray<BSTR> &saryLabel, 
		CString &strMenu, CString &strDescription)
{
	BOOL bResult = FALSE;
	if (IsICD10CMSelected()){
		NexTech_Accessor::_SubCodeRequestResult_CMPtr pCodesCreated = GetAPI()->GetNexCodeSubCodes_CM(GetAPISubkey(), GetAPILoginToken(), LoadCurrentHierarchy(oNexCode));
			Nx::SafeArray<IUnknown *> saryLable;
		if (pCodesCreated){
			saryCodeHierarchy = pCodesCreated->GetHierarchies();
			saryCodeCharacters= pCodesCreated->GetCharacters();
			saryLabel= pCodesCreated->GetLabel();
			strDescription = VarString(pCodesCreated->Getdescription(),"");
			strMenu= VarString(pCodesCreated->GetMenuChar(),"");
			bResult = TRUE;
		}
	}
	else{
		NexTech_Accessor::_SubCodeRequestResult_PCSPtr pCodesCreated = GetAPI()->GetNexCodeSubCodes_PCS(GetAPISubkey(), GetAPILoginToken(), LoadCurrentCharacters(oNexCode));
		if (pCodesCreated ){
			saryCodeCharacters= pCodesCreated->GetCharacters();
			saryLabel= pCodesCreated->GetLabel();
			strDescription = VarString(pCodesCreated->Getdescription(),"");
			strMenu= VarString(pCodesCreated->GetMenuChar(),"");
			bResult = TRUE;
		}
	}
	return bResult;
}

// Call API and load data
// (s.dhole 2014-03-21 16:06) - PLID 61376
BOOL CNexCodeDlg::LoadDataFromAPI(NexCodeHistory& oNexCode, const BOOL bAutoIncreament /*FALSE*/)
{
	BOOL bEnd =  FALSE;
	try
	{
		// we got result back
		Nx::SafeArray<IUnknown *> saryCodeHierarchy;
		Nx::SafeArray<IUnknown *> saryCodeCharacters;
		Nx::SafeArray<BSTR> saryLable;
		CString sLable,sMenu,sDescription;	 
		// (s.dhole 2014-03-24 16:05) - PLID 61508 to support CM and PCS call
		if(GetDataFromAPI(oNexCode,saryCodeHierarchy,saryCodeCharacters,saryLable,sMenu,sDescription)){
			BOOL bHierarchyExist = FALSE;
			BOOL bCharacterExist = FALSE;
			int nIndex = m_arryNexCodeHistory.GetSize(); 
			CString strMin, strMax;
			CString strCode,strCodeDescription;
			for(int iCount =0; iCount < (int)saryLable.GetCount(); iCount++ ){
				sLable += saryLable.GetAt(iCount);
				sLable += ",";
			}
			// load Hierarchy result
			CArray<NexCodeHierarchy, NexCodeHierarchy> aCodeHierarchyResult;
			foreach(NexTech_Accessor::_NexCodeHierarchyPtr pHierarchy, saryCodeHierarchy){
				strMin = VarString(pHierarchy->MinCode,"");
				strMax = VarString(pHierarchy->MaxCode,"");
				strCodeDescription = VarString(pHierarchy->description,"");
				aCodeHierarchyResult.Add(NexCodeHierarchy(strCodeDescription,strMin,strMax)); 
				bHierarchyExist = TRUE;
			}

			// load Character result
			CArray<NexCodeCharacter, NexCodeCharacter> aCodeCharacterResult;
			foreach(NexTech_Accessor::_NexCodeCharacterPtr pCharacte, saryCodeCharacters){
				strCode = VarString(pCharacte->Characters,"");
				strCodeDescription = VarString(pCharacte->description,"");
				aCodeCharacterResult.Add(NexCodeCharacter(strCode, strCodeDescription)); 
				bCharacterExist = TRUE;
			}
			
			// we do not have new result. Finish selecting a code
			if ((m_arryNexCodeHistory.GetSize()>1) && bCharacterExist == FALSE && bHierarchyExist == FALSE){
				bEnd = TRUE;
			}
			if (m_arryNexCodeHistory.GetCount()==0){
				sLable = oNexCode.strCode ;
				sMenu = oNexCode.strMenuChar ;
				sDescription = oNexCode.strDescription ;
			}
			
			// NOW UPDATE MAIN ARRAY
			m_arryNexCodeHistory.Add(NexCodeHistory(nIndex,sLable,sMenu,sDescription
				,oNexCode.aCodeHierarchy,aCodeHierarchyResult
				,oNexCode.aCodeCharacters,aCodeCharacterResult));

			// (s.dhole 2014-03-27 12:22) - PLID 61571  if we return only one result(Character or Hierarchy ) then make recurrence call to api
			// if there is only on Character or Hierarchy  result the try to load next step
			
			if (bEnd == FALSE && bAutoIncreament == TRUE && 
				((aCodeCharacterResult.GetSize()==0 && aCodeHierarchyResult.GetSize()==1 )
				|| (aCodeCharacterResult.GetSize()==1 && aCodeHierarchyResult.GetSize()==0))){
				CArray<NexCodeHierarchy, NexCodeHierarchy> aHierarchy;
				CArray<NexCodeCharacter, NexCodeCharacter> aCharacters;
				aHierarchy.Copy(oNexCode.aCodeHierarchy);
				aCharacters.Copy(oNexCode.aCodeCharacters);
				if (aCodeHierarchyResult.GetCount()>0 
					){
					NexCodeHierarchy oHierarchy =aCodeHierarchyResult[0];
					aHierarchy.Add(NexCodeHierarchy(oHierarchy.strDescription,
						oHierarchy.strMinCode, oHierarchy.strMaxCode)); 
				}
				if (aCodeCharacterResult.GetCount()>0 ){
					NexCodeCharacter oCharacter = aCodeCharacterResult[0];
					aCharacters.Add(NexCodeCharacter(oCharacter.strCharacters,oCharacter.strDescription)); 
				}
				bEnd = LoadDataFromAPI(NexCodeHistory(0,sLable,sDescription, aHierarchy, aCharacters), TRUE);
			}
		}
	}NxCatchAll(__FUNCTION__);
	return bEnd;
}


// (s.dhole 2014-03-14 10:03) - PLID 61376 Load curren step data
void CNexCodeDlg::LoadHierarchy()
{
	try{
		int nSize = m_arryNexCodeHistory.GetSize();
		m_pCodeHierachyList->Clear();
		if (nSize>0){
			CString  strCode;
			// try to load Hierarchy  code			
			NexCodeHistory& pNexCode = m_arryNexCodeHistory[nSize - 1]; 
			NXDATALIST2Lib::IRowSettingsPtr pRow; 
			for(int nCount=0 ;nCount<pNexCode.aCodeHierarchyResult.GetSize(); nCount++){
				pRow = m_pCodeHierachyList->GetNewRow();
				strCode = FormatString("%s - %s",
					pNexCode.aCodeHierarchyResult[nCount].strMinCode,
					pNexCode.aCodeHierarchyResult[nCount].strMaxCode);
				pRow->PutValue(hlCode,AsBstr(strCode));
				pRow->PutValue(hlDescription,AsBstr(pNexCode.aCodeHierarchyResult[nCount].strDescription)); 
				pRow->PutValue(hlMin,AsBstr(pNexCode.aCodeHierarchyResult[nCount].strMinCode)); 
				pRow->PutValue(hlMax,AsBstr(pNexCode.aCodeHierarchyResult[nCount].strMaxCode)); 
				pRow->PutValue(hlIsCharacter,AsBool(FALSE)); 
				m_pCodeHierachyList->AddRowSorted(pRow, NULL);
			}
			
			for(int nCount=0 ; nCount<pNexCode.aCodeCharacterResult.GetSize();nCount++){
				pRow = m_pCodeHierachyList->GetNewRow();
				strCode = pNexCode.aCodeCharacterResult[nCount].strCharacters;
				pRow->PutValue(hlCode, AsBstr(strCode) );
				pRow->PutValue(hlDescription,AsBstr(pNexCode.aCodeCharacterResult[nCount].strDescription)); 
				pRow->PutValue(hlIsCharacter,AsBool( TRUE)); 
				m_pCodeHierachyList->AddRowSorted(pRow, NULL);
			}
			
			strCode = m_arryNexCodeHistory[nSize-1].strCode;
			CString strCodeDescription = m_arryNexCodeHistory[nSize-1].strDescription;
			UpdateCodeHeaderText(strCode, strCodeDescription);
		}
		//Set Back Button
		// (s.dhole 2014-03-21 15:31) - PLID 60988
		m_btnBack.EnableWindow((IsCodeBuildInProcess())?TRUE:FALSE);
		
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2014-03-13 16:21) - PLID 60988 Load Back step
void CNexCodeDlg::GoBackOneStep(){
	try{
		if  (m_arryNexCodeHistory.GetSize()>1){
			GoBackAtHierarchy(m_arryNexCodeHistory[m_arryNexCodeHistory.GetSize()-2]);
		}
		else{
			m_arryNexCodeHistory.RemoveAt(0) ;
			GoBackAtHierarchy(NexCodeHistory());
		}
		// (s.dhole 2014-03-14 10:03) - PLID 61376 
		LoadHierarchy();
		ShowHideCotrol(TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-03-14 08:57) - PLID 60989 Load formated dext
CString CNexCodeDlg::GetFormatedUIstring(const CString strCode,const  BOOL bShowDash){
	CString strTemp;
	CStringArray strArray;
	SplitString(strCode,"," ,&strArray);
	for(int i = 0; i < strArray.GetSize(); i++) {
		strTemp += strArray[i];
		strTemp += " "; 
	}
	if (bShowDash){
		strTemp += "_ ";     
	}
	return strTemp;
}


// (s.dhole 2014-03-13 16:15) - PLID 61369 show back menu  
void CNexCodeDlg::ShowBackMenu(CMenu& menu, 
							   CMap<long, long, NexCodeHistory, NexCodeHistory&> &mapCodeHistory){
	menu.CreatePopupMenu();
	long nIndex = 0;
	long nCurrentID = IDM_HIERARCHY_LEVEL;	
	long nCurrentRowPosition = 0;
	nCurrentRowPosition = 0;
	for(int nId=0; nId<m_arryNexCodeHistory.GetSize() - 1; nId++ ){
		NexCodeHistory& pNexCodeHistory = m_arryNexCodeHistory[nId] ; 
		nCurrentID++;
		nCurrentRowPosition++;
		CString strCode = pNexCodeHistory.strMenuChar;
		menu.AppendMenu(MF_STRING, nCurrentID, strCode);
		mapCodeHistory[nCurrentID] = pNexCodeHistory;
		nCurrentID++;
		menu.AppendMenu(MF_SEPARATOR, nCurrentID, "");
	}
	nCurrentID++;
	nCurrentRowPosition = 0;
	for(int nId=0; nId<m_arryNexCodeHistory.GetSize()  - 1; nId++){
		NexCodeHistory& pNexCodeHistory = m_arryNexCodeHistory[nId] ; 
		nCurrentID++;
		nCurrentRowPosition++;
		CString strMenuDescription = pNexCodeHistory.strDescription;
		strMenuDescription.Replace("&", "&&");
		UINT uFlags = MF_STRING;
		if (nCurrentRowPosition==1){
			uFlags = MF_MENUBREAK|MF_STRING;
		}
		menu.AppendMenu(uFlags, nCurrentID, strMenuDescription);
		mapCodeHistory[nCurrentID] = pNexCodeHistory;
		nCurrentID++;
		menu.AppendMenu(MF_SEPARATOR, nCurrentID, "");
	}
}

// (s.dhole 2014-03-13 16:15) - PLID 61369 Load Menu 
void CNexCodeDlg::LoadBackMenu(){
	try{
		CMenu menu;
		CMap<long, long, NexCodeHistory, NexCodeHistory&> mapCodeHistory;
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_NEXCODE_BACK_BTN);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			ShowBackMenu(menu,mapCodeHistory);
			int nResult = menu.TrackPopupMenuEx((!GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_LEFTALIGN : TPM_RIGHTALIGN)|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
			if (nResult > 0) {
				NexCodeHistory pNexCodeHistory;
				if (mapCodeHistory.Lookup(nResult, pNexCodeHistory)){
					GoBackAtHierarchy(pNexCodeHistory);
					// (s.dhole 2014-03-14 10:03) - PLID 61376 
					ShowHideCotrol(TRUE);
					// (s.dhole 2014-03-21 16:06) - PLID 61376
					LoadHierarchy();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-03-21 15:31) - PLID 60989
BOOL CNexCodeDlg::IsCodeBuildInProcess(){
	if (m_arryNexCodeHistory.GetSize()>1){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

// (s.dhole 2014-03-14 09:58) - PLID 60989
void CNexCodeDlg::RestAllchages(){
	// (s.dhole 2014-03-21 09:02) - PLID 61376
	m_pCodeHierachyList->Clear();
	m_arryNexCodeHistory.RemoveAll(); 
	m_btnBack.EnableWindow(FALSE);
	m_strDiagCode_ICD10 = ""; 
	m_strDiagCodeDescription_ICD10 = "";
}



// (s.dhole 2014-03-14 10:03) - PLID 61376
void CNexCodeDlg::ShowHideCotrol(BOOL bShowList)
{
	if (bShowList){
		GetDlgItem(IDC_NEXCODE_HISTORY_HIERARCHY_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEXCODE_CODE_RESULT_LABLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXCODE_CODE_RESULT_DETAIL_LABLE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXCODE_OK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXCODE_OK)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_NEXCODE_HISTORY_HIERARCHY_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEXCODE_CODE_RESULT_LABLE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEXCODE_CODE_RESULT_DETAIL_LABLE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEXCODE_OK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEXCODE_OK)->EnableWindow(TRUE);
	}
}

// (s.dhole 2014-03-14 12:37) - PLID 61379
void CNexCodeDlg::OnCancel()
{
	try{
		int nRet = IDYES; 
		if (IsCodeBuildInProcess()){
			nRet = AfxMessageBox("You have started building an ICD-10 code.\n\nAre you sure you wish to Exit?", MB_YESNO | MB_ICONINFORMATION); 
		}
		if (nRet == IDYES){
			CNxDialog::OnCancel();
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-03-14 12:37) - PLID 61379
void CNexCodeDlg::OnBnClickedBtnNexcodeCancel()
{
	try{
		OnCancel();
	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2014-03-21 09:02) - PLID 61376
void CNexCodeDlg::LeftClickNexcodeHistoryHierarchyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	CWaitCursor cwait;
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow){
			return;
		}
	
		if (nCol== hlDescription || nCol== hlCode){
			LoadData(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2014-03-21 09:02) - PLID 61376
void CNexCodeDlg::LoadData(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// TODO: Add your message handler code here
	int nSize = m_arryNexCodeHistory.GetSize();
	NexCodeHistory& pNexCodeHistory = m_arryNexCodeHistory[nSize -1];
	CArray<NexCodeHierarchy, NexCodeHierarchy> aCodeHierarchy;
	CArray<NexCodeCharacter, NexCodeCharacter> aCodeCharacters;
	aCodeHierarchy.Copy(pNexCodeHistory.aCodeHierarchy); 
	aCodeCharacters.Copy(pNexCodeHistory.aCodeCharacters); 
	// load data from selected row
	CString strCode=""; 
	CString strCodeDetail = VarString(pRow->GetValue(hlDescription),"");
	if (VarLong(pRow->GetValue(hlIsCharacter)) == FALSE){
		CString strInputCodeDetail = VarString(pRow->GetValue(hlDescription),"");
		CString strMin = VarString(pRow->GetValue(hlMin),"");
		CString strMax = VarString(pRow->GetValue(hlMax),"");
		strCode = FormatString("%s - %s",strMin,strMax);
		aCodeHierarchy.Add( NexCodeHierarchy(strCodeDetail,strMin ,strMax));   
	}
	else{
		strCode = VarString(pRow->GetValue(hlCode),"");
		aCodeCharacters.Add(NexCodeCharacter(strCode, strCodeDetail));   
	}
	
	
	if (LoadDataFromAPI(NexCodeHistory(nSize,strCode,strCodeDetail,aCodeHierarchy,aCodeCharacters),TRUE) == TRUE){
		nSize = m_arryNexCodeHistory.GetSize();
		// (s.dhole 2014-03-21 15:31) - PLID 60988
		m_btnBack.EnableWindow((IsCodeBuildInProcess())?TRUE:FALSE);
		// should not happen
		if(nSize >0){
			strCode = m_arryNexCodeHistory[nSize-1].strCode;  
			strCodeDetail = m_arryNexCodeHistory[nSize-1].strDescription;
		}
		
		// (s.dhole 2014-03-21 15:44) - PLID 60989
		UpdateCodeHeaderText(GetCommSeparated(strCode),strCodeDetail,FALSE);
		ShowHideCotrol(FALSE);
		m_strDiagCode_ICD10  = strCode; 
		m_strDiagCode_ICD10.Replace(",","") ; 
		m_strDiagCodeDescription_ICD10 = strCodeDetail;
	}
	else{
		// (s.dhole 2014-03-21 16:06) - PLID 61376
		LoadHierarchy();
	}
}

// (s.dhole 2014-03-21 15:44) - PLID 60989
CString CNexCodeDlg::GetCommSeparated(const CString strCode)
{
	CString strTemp= "";
	if(strCode.Find(',')>-1){
		strTemp = strCode;
	}
	else{
		for(int iCount=0; iCount<strCode.GetLength(); iCount++){
			strTemp += strCode.Mid(iCount,1);
			strTemp+= ",";
		}
	}
	return strTemp;
}
