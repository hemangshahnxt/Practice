#pragma once


// CEditPharmacyStaffDlg dialog
//DRT 11/19/2008 - PLID 32093 - Created

class CEditPharmacyStaffDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditPharmacyStaffDlg)

public:
	CEditPharmacyStaffDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditPharmacyStaffDlg();

	//Must be filled by the caller
	CString m_strPharmacyName;		//LocationsT.Name
	long m_nLocationID;			//LocationsT.ID

// Dialog Data
	enum { IDD = IDD_EDIT_PHARMACY_STAFF_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	//Dialog functionality
	void DeletePharmacyStaffRecords(long nPersonID);

	//Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pPharmacistList;
	NXDATALIST2Lib::_DNxDataListPtr m_pStaffList;
	CNxStatic m_nxstaticPharmacyName;
	CNxIconButton m_btnAddPharmacist;
	CNxIconButton m_btnEditPharmacist;
	CNxIconButton m_btnRemovePharmacist;
	CNxIconButton m_btnAddStaff;
	CNxIconButton m_btnEditStaff;
	CNxIconButton m_btnRemoveStaff;
	CNxIconButton m_btnClose;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPharmStaffClose();
	afx_msg void OnBnClickedAddPharmacist();
	afx_msg void OnBnClickedEditPharmacist();
	afx_msg void OnBnClickedRemovePharmacist();
	afx_msg void OnBnClickedAddStaff();
	afx_msg void OnBnClickedEditStaff();
	afx_msg void OnBnClickedRemoveStaff();
	DECLARE_EVENTSINK_MAP()
	void OnDblClickPharmacistList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellPharmacyStaffList(LPDISPATCH lpRow, short nColIndex);
};
