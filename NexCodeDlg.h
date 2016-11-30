#pragma once

#include "PracticeRc.h"
#include "NxDataUtilitiesLib\SafeArrayUtils.h"
#include "NxAPI.h"

// CNexCodeDlg dialog
// (b.savon 2014-03-04 13:07) - PLID 60987 - Create a NexCode dialog that returns a DiagCode ID from the newly (or not) created ICD10 code in DiagCodes

class CNexCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexCodeDlg)

private:

	long	m_nDiagCodeID_ICD10;
	CString m_strDiagCode_ICD10;
	CString	m_strDiagCodeDescription_ICD10;
	
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnOK;
	// (s.dhole 2014-03-13 16:36) - PLID 60988
	CNxIconButton m_btnBack;
	NXDATALIST2Lib::_DNxDataListPtr  m_pCodeHierachyList;
	NxButton	m_radioICD10CM;
	NxButton	m_radioICD10PCS;
	CNxStatic	m_nxstaticCode;
	CNxStatic	m_nxstaticCodeDetail;

	CNxColor m_nxcTopBack;
	CNxColor m_nxcBottomBack;
	// (s.dhole 2014-03-14 08:57) - PLID 60989 
	BOOL m_bIsICD10CMSelected;
	BOOL IsICD10CMSelected(){ return m_bIsICD10CMSelected;}; 
	
public:
	CNexCodeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNexCodeDlg();
	
// (s.dhole 2014-03-13 16:21) - PLID 60988
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnCancel();
	inline long		GetDiagCodeID(){ return m_nDiagCodeID_ICD10; }
	inline CString	GetDiagCodeDescription(){ return m_strDiagCodeDescription_ICD10; }
	inline CString	GetDiagCode(){ return m_strDiagCode_ICD10; }

	void SaveToDiagCodes(); // (r.farnworth 2014-03-11 12:05) - PLID 60990

	afx_msg void OnBnClickedOk();

// Dialog Data
	enum { IDD = IDD_NEXCODE_DLG };
protected: 

// (s.dhole 2014-03-14 07:50) - PLID 60988
struct NexCodeHierarchy
{
	CString strMinCode;
	CString strMaxCode;
	CString strDescription;

	NexCodeHierarchy(){
		strMinCode = "";
		strMaxCode = "";
		strDescription = "";
	}
	
	NexCodeHierarchy(const CString _strDescription,const CString _strMinCode,const CString _strMaxCode){
		strDescription =_strDescription;
		strMinCode = _strMinCode;
		strMaxCode = _strMaxCode;
	}
	void operator=(const NexCodeHierarchy& NexCode)
	{
		strMinCode = NexCode.strMinCode;
		strMaxCode = NexCode.strMaxCode;
		strDescription = NexCode.strDescription;
	}
};

// (s.dhole 2014-03-14 07:50) - PLID 60988
struct NexCodeCharacter
{
	CString strCharacters;
	CString strDescription;

	NexCodeCharacter(){
		strCharacters = "";
		strDescription = "";
	}
	
	NexCodeCharacter(const CString _strCharacters,const CString _strDescription){
		strCharacters =_strCharacters;
		strDescription = _strDescription;
	}
	void operator=(const NexCodeCharacter& NexCode)
	{
		strCharacters= NexCode.strCharacters;
		strDescription = NexCode.strDescription;
		
	}
};



// (s.dhole 2014-03-14 07:50) - PLID 60988
struct NexCodeHistory
{
	long nIndex;
	CString strCode;
	CString strMenuChar;// (s.dhole 2014-03-13 16:15) - PLID 61369 
	CString strDescription;
	//BOOL bIsUseCharacter ;
	CArray<NexCodeHierarchy, NexCodeHierarchy> aCodeHierarchy;
	CArray<NexCodeHierarchy, NexCodeHierarchy> aCodeHierarchyResult;
	CArray<NexCodeCharacter, NexCodeCharacter> aCodeCharacters;
	CArray<NexCodeCharacter, NexCodeCharacter> aCodeCharacterResult;

	NexCodeHistory(){
		nIndex= 0;
		strMenuChar = strCode= strDescription = "";
		aCodeHierarchy.RemoveAll(); 
		aCodeCharacters.RemoveAll();
		aCodeHierarchyResult.RemoveAll();
		aCodeCharacterResult.RemoveAll();
		//bIsUseCharacter= FALSE;
	}

	// (s.dhole 2014-03-13 16:15) - PLID 61369 
	NexCodeHistory(const long _nIndex, const CString _sCode, const CString _sMenuChar,
		const CString _strDescription,  
		CArray<NexCodeHierarchy, NexCodeHierarchy> &_aCodeHierarchy,
		CArray<NexCodeHierarchy, NexCodeHierarchy> &_aCodeHierarchyResult,
		CArray<NexCodeCharacter, NexCodeCharacter>  &_aCodeCharacters,
		CArray<NexCodeCharacter, NexCodeCharacter> &_aCodeCharacterResult){
			nIndex = _nIndex;
			strCode = _sCode;
			strMenuChar = _sMenuChar;// (s.dhole 2014-03-13 16:15) - PLID 61369 
			//bIsUseCharacter = _bIsUseCharacter;
			strDescription = _strDescription;
			aCodeHierarchyResult.Copy(_aCodeHierarchyResult); 
			aCodeHierarchy.Copy( _aCodeHierarchy); 
			aCodeCharacters.Copy( _aCodeCharacters);
			aCodeCharacterResult.Copy( _aCodeCharacterResult);
	}

	NexCodeHistory(const long _nIndex, const CString _strCode, const CString _strDescription,
		 CArray<NexCodeHierarchy, NexCodeHierarchy> &_aCodeHierarchy,
		 CArray<NexCodeCharacter, NexCodeCharacter>  &_aCodeCharacters){
			nIndex = _nIndex;
			strCode = _strCode;
			strDescription = _strDescription;
			aCodeHierarchy.Copy( _aCodeHierarchy); 
			aCodeCharacters.Copy( _aCodeCharacters); 
	}
	NexCodeHistory(const long _nIndex, const CString _strCode,const CString _strMenuChar, const CString _strDescription){
			nIndex = _nIndex;
			strCode = _strCode;
			strMenuChar = _strMenuChar; 
			strDescription = _strDescription;
	
	}
public:
	NexCodeHistory(const NexCodeHistory&) {
	}
    void operator=(const NexCodeHistory& NexCode) { 
			nIndex = NexCode.nIndex;
			strCode = NexCode.strCode ;
			strMenuChar = NexCode.strMenuChar ;// (s.dhole 2014-03-13 16:15) - PLID 61369 
			//bIsUseCharacter = NexCode.bIsUseCharacter;
			strDescription = NexCode.strDescription;
			aCodeHierarchyResult.Copy(NexCode.aCodeHierarchyResult); 
			aCodeHierarchy.Copy(NexCode.aCodeHierarchy); 
			aCodeCharacters.Copy(NexCode.aCodeCharacters);
			aCodeCharacterResult.Copy(NexCode.aCodeCharacterResult);
		//return *this; 
	}

};

	// (s.dhole 2014-03-14 07:50) - PLID 60988
	CArray<NexCodeHistory, NexCodeHistory&> m_arryNexCodeHistory;
	BOOL CNexCodeDlg::LoadDataFromAPI(NexCodeHistory& oNexCode, const BOOL bAutoIncreament =  FALSE);
	BOOL GetDataFromAPI(NexCodeHistory &oNexCode,Nx::SafeArray<IUnknown *> &saryCodeHierarchy,
									 Nx::SafeArray<IUnknown *> &saryCodeCharacters,
									 Nx::SafeArray<BSTR> &saryLabel,
									 CString &strMenu, CString &strDescription);
	void GoBackAtHierarchy(NexCodeHistory& pNexCodeHistory); 

	NexTech_Accessor::_SubCodeRequest_PCSPtr  CNexCodeDlg::LoadCurrentCharacters(NexCodeHistory &oNexCode);

	NexTech_Accessor::_SubCodeRequest_CMPtr  CNexCodeDlg::LoadCurrentHierarchy(NexCodeHistory &oNexCode);
	// (s.dhole 2014-03-13 16:15) - PLID 61369 
	void ShowBackMenu(CMenu& menu, CMap<long, long, NexCodeHistory, NexCodeHistory&> &mapCodeHistory); 
	
	// (s.dhole 2014-03-14 08:57) - PLID 60989 
	BOOL CanChangeCodeType();
	void LoadData(NXDATALIST2Lib::IRowSettingsPtr pRow);
	CString GetCommSeparated(const CString strCode);
	void RemoveHistory(NexCodeHistory &NexCodeHistory);
	void RemoveAllHistory();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	// (s.dhole 2014-03-14 08:57) - PLID 60989
	void UpdateCodeInitialText();
	// (s.dhole 2014-03-14 08:58) - PLID  60989
	void UpdateCodeHeaderText(const CString strCode,const CString strCodeDetail,const  BOOL bShowDash = TRUE );
	// (s.dhole 2014-03-21 15:44) - PLID 60989
	CString GetFormatedUIstring(const CString strCode,const  BOOL bShowDash = TRUE);

	void GoBackOneStep() ;
		// (s.dhole 2014-03-13 16:15) - PLID 61369 check if we have to load back history menu	
	void LoadBackMenu(); 
	
	// (s.dhole 2014-03-21 15:31) - PLID 60988
	BOOL IsCodeBuildInProcess();
	// (s.dhole 2014-03-14 10:03) - PLID 61376
	void ShowHideCotrol(BOOL bShowList);
	// (s.dhole 2014-03-14 09:58) - PLID 60989
	void RestAllchages();
	// (s.dhole 2014-03-21 16:06) - PLID 61376
	void LoadHierarchy();
	DECLARE_MESSAGE_MAP()

public:
	
	DECLARE_EVENTSINK_MAP()
	void LeftClickNexcodeHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedNexcodeIcd10cmRedioBtn();
	afx_msg void OnBnClickedNexcodeIcd10pcsRedioBtn();
	afx_msg void OnBnClickedBtnNexcodeCancel();
	void LeftClickNexcodeHistoryHierarchyList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};
