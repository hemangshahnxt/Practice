#pragma once

#include "PatientsRc.h"

// CConcTsMachinePoolDlg dialog
// (d.thompson 2011-10-05) - PLID 45791 - Created

class CConcTSMachine {
public:
	long nID;
	CString strName;
};

//Data class to track concurrent TS licensing machine pool
class CConcTSList {
public:
	void AddExistingMachine(long nID, CString strName, OPTIONAL OUT CConcTSMachine *pMachine);
	bool AddNewMachine(CString strName, OPTIONAL OUT CConcTSMachine *pMachine);
	bool RemoveMachineByID(long nID);
	bool RemoveMachineByName(CString strName);
	bool IsNameValid(CString strNewName, CString &strMessage);
	bool IsNameUnique(CString strName);
	void GetActiveMachineArray(OUT CArray<CConcTSMachine, CConcTSMachine&> *paryMachines);
	void GetDeletedMachineArray(OUT CArray<CConcTSMachine, CConcTSMachine&> *paryMachines);

protected:
	//List of all active machines
	CArray<CConcTSMachine, CConcTSMachine&> m_aryActiveMachines;
	//List of all machines deleted that need committed to data
	CArray<CConcTSMachine, CConcTSMachine&> m_aryDeletedMachines;

protected:
	//Helpers
	long FindMachineOrdinalByID(CArray<CConcTSMachine, CConcTSMachine&> *pAry, long nID);
	long FindMachineOrdinalByName(CArray<CConcTSMachine, CConcTSMachine&> *pAry, CString strName);
};

class CConcTSDataManipulation {
public:
	static void LoadFromData(CConcTSList *pList, long nClientID);
	static bool SaveToData(CConcTSList *pList, long nClientID);

protected:
	static CSqlFragment CConcTSDataManipulation::GenerateInsertQuery(CConcTSList *pList, long nClientID);
	static CSqlFragment CConcTSDataManipulation::GenerateDeleteQuery(CConcTSList *pList);
};

//UI class to setup concurrent TS licensing machine pool
class CConcTsMachinePoolDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CConcTsMachinePoolDlg)

public:
	CConcTsMachinePoolDlg(CWnd* pParent);   // standard constructor
	virtual ~CConcTsMachinePoolDlg();

	//What client ID are we currently working with?
	long m_nClientID;
	bool m_bIsTestAccount;		//Follows implementation of 33989 feature
	// (b.eyers 2015-03-10) - PLID 65208 - Machine Pool dlg needs to know the bought and in use for Con. TS
	long m_nBConc;
	long m_nUConc;

// Dialog Data
	enum { IDD = IDD_SUPPORT_CONC_TS_MACHINE_POOL_DLG };
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnRemove;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;

	// (b.eyers 2015-03-10) - PLID 65208 - Machine Pool dlg needs to know the bought and in use for Con. TS
	void SetBConc(long nConc);
	void SetUConc(long nConc);

	//member data
protected:
	CConcTSList m_data;

	//Helper functions
protected:
	bool AddMachineByName(CString strName);
	bool RemoveMachine(long nID, CString strName);
	bool IsValidName(CString strName);
	void AddMachineToUI(CConcTSMachine *pMachine);
	void AddActiveMachinesToInterface();
	void ReflectUIPermissions();
	void LoadSourceData();
	bool SaveToData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddMachine();
	afx_msg void OnBnClickedRemoveMachine();
};
