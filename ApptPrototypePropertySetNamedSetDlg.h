#pragma once
// (b.cardillo 2011-02-22 16:25) - PLID 40419 - Admin UI for managing Appointment Prototypes

#include "AdministratorRc.h"
#include "ApptPrototypeUtils.h"
// CApptPrototypePropertySetNamedSetDlg dialog

class CApptPrototypePropertySetNamedSetDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptPrototypePropertySetNamedSetDlg)

public:
	CApptPrototypePropertySetNamedSetDlg(CWnd* pParent);   // standard constructor
	virtual ~CApptPrototypePropertySetNamedSetDlg();

// Dialog Data
	enum { IDD = IDD_APPTPROTOTYPE_PROPERTYSET_NAMEDSET_DLG };
	CNxIconButton m_btnAdd;
	CNxIconButton m_btnDelete;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;

public:
	virtual INT_PTR DoModal(const CString &strFriendlySetName, CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> &dataSource, LONG nNextNegativeID, const CString &strAllPossibleDetailsSql);
	LONG GetNextNegativeID();

protected:
	struct CPossibleDetail {
		CPossibleDetail() : m_nID(-1), m_strName(_T("")), m_bInactive(FALSE) { }
		CPossibleDetail(LONG id, const CString &name, BOOL inactive) : m_nID(id), m_strName(name), m_bInactive(inactive) { }
		LONG m_nID;
		CString m_strName;
		BOOL m_bInactive;
	};

protected:
	BOOL m_bInitialized;
	CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> *m_pDataSource;
	CArray<CApptPrototypePropertySetNamedSet, CApptPrototypePropertySetNamedSet &> m_LocalCopy;
	CMap<LONG, LONG, CPossibleDetail, CPossibleDetail &> m_mapAllPossibleDetails;
	CString m_strAllPossibleDetailsSql;
	CString m_strFriendlySetName;
	virtual INT_PTR DoModal();
	CString CalculateDetailsText(const CMap<LONG,LONG,BYTE,BYTE> &details);
	LONG m_nNextNegativeID;
	CApptPrototypePropertySetNamedSet &FindAssociatedNamedSet(LPDISPATCH lpRow);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void LeftClickNamedsetList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void SelChangedNamedsetList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedDeleteBtn();
	void EditingFinishingNamedsetList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedNamedsetList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
};
