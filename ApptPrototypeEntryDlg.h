#pragma once
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes

#include "AdministratorRc.h"

class CApptPrototypePropertySetMap : public CMap<LONG, LONG, class CApptPrototypePropertySet *, class CApptPrototypePropertySet *>
{
public:
	CApptPrototypePropertySetMap();
	~CApptPrototypePropertySetMap();
	void RemoveAll();
	void Copy(const CApptPrototypePropertySetMap &src);
};

// CApptPrototypeEntryDlg dialog
class CApptPrototypeEntryDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptPrototypeEntryDlg)

public:
	CApptPrototypeEntryDlg(CWnd* pParent);   // standard constructor
	virtual ~CApptPrototypeEntryDlg();

// Dialog Data
	enum { IDD = IDD_APPTPROTOTYPE_ENTRY_DLG };
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

public:
	// For caller to get info after we return from domodal.  Only valid if we return IDOK.
	LONG GetCommittedPrototypeID();
	CString GetCommittedPrototypeName();
	CString GetCommittedPrototypeDescription();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	INT m_appointmentPrototypeID;
	const CStringArray *m_parystrOtherPrototypeNames;
	bool m_readyToDoModal;
	LONG m_nextNegativeID;
	LONG m_nNextNegativeNamedSetID;

protected:
	BOOL Validate();
	BOOL Save();
	void ReflectEnabledControls(BOOL bIsRowSelected);
	void ReflectEnabledControls();

private:
	CString m_strOriginalName;
	CString m_strOriginalDescription;
	CApptPrototypePropertySetMap m_mapOrigPropSets;
	CApptPrototypePropertySetMap m_mapCurrentPropSets;

private:
	CApptPrototypePropertySet *GetPropertySetFromRow(LPDISPATCH lpRow);

	DECLARE_MESSAGE_MAP()
public:
	virtual INT_PTR DoModal();
	virtual INT_PTR DoModal(INT appointmentPrototypeID, const CStringArray &arystrOtherPrototypeNames);
	afx_msg void OnBnClickedOk();
	virtual BOOL OnInitDialog();
	DECLARE_EVENTSINK_MAP()
	void LeftClickApptprototypePropertysetsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnChangePropertySetColumn_Available(LPDISPATCH lpRow);
	void OnChangePropertySetColumn_Types(LPDISPATCH lpRow);
	void OnChangePropertySetColumn_PurposeSets(LPDISPATCH lpRow);
	void OnChangePropertySetColumn_ResourceSets(LPDISPATCH lpRow);
	void OnChangePropertySetColumn_Locations(LPDISPATCH lpRow);
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedDeleteBtn();
	void EditingFinishingApptprototypePropertysetsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedApptprototypePropertysetsList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void SelChangedApptprototypePropertysetsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
};
