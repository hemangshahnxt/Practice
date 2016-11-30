#pragma once
#include "PracticeRc.h"
#include "EmrOmrMap.h" 

// (b.spivey, August 02, 2012) - PLID 51928 - Created
// CEMROMRMapperDlg dialog

class CEMROMRMapperDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEMROMRMapperDlg)

public:
	CEMROMRMapperDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEMROMRMapperDlg();

// Dialog Data
	enum { IDD = IDD_EMR_OMR_FORM_MAPPER };



private:

	//For lists
	enum EmrOmrMapEnum {
		eomeID = 0, 
		eomeName, 
		eomeOmrID,
	};

	//to switch from form or template
	enum EmrOmrMapLoaderEnum {
		eomleForm = 0,
		eomleTemplate,
	};



	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	void LoadEmrOmrMapDatalist(); 

	void OnBtnOmrNew(); 
	void OnBtnOmrDelete();
	void OnBtnOmrRename();
	void OnBtnOmrSave(); 
	void OnBtnOmrClose(); 
	void OnBtnOmrBrowse();
	void OnKillFocusFormLocation(); 

	
	NXDATALIST2Lib::_DNxDataListPtr m_dlEmrOmrMapList; 
	NXDATALIST2Lib::_DNxDataListPtr m_dlFormSelectList; 
	NXDATALIST2Lib::_DNxDataListPtr m_dlTemplateSelectList; 
	EmrOmrMap m_EmrOmrMap; 

	// (b.spivey, August 31, 2012) - PLID 51928 - Button controls. 
	CNxIconButton m_nxbtnSave;
	CNxIconButton m_nxbtnClose;
	CNxIconButton m_nxbtnNew;
	CNxIconButton m_nxbtnRename;
	CNxIconButton m_nxbtnDelete; 

	// (b.spivey, September 06, 2012) - PLID 51928 - NxColor control
	CNxColor m_nxclrBackground;

	CNxEdit m_nxeditFormLocation; 

	DECLARE_MESSAGE_MAP()

	DECLARE_EVENTSINK_MAP()

	// (b.spivey, September 19, 2012) - PLID 51928 - there was no reason for this to be public. 
	void SelChangingFormSelectList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingEmrTemplateSelect(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnEditingStartingEmrOmrMap(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingEmrOmrMap(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	void OnSelChosenEmrTemplateSelect(LPDISPATCH lpRow);
	void OnSelChosenFormSelectList(LPDISPATCH lpRow); 
};
