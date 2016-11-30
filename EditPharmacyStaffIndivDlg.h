#pragma once


// CEditPharmacyStaffIndivDlg dialog
//DRT 11/19/2008 - PLID 32093 - Created

class CEditPharmacyStaffIndivDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditPharmacyStaffIndivDlg)

public:
	CEditPharmacyStaffIndivDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditPharmacyStaffIndivDlg();

	//Use this intead of DoModal to set the parameters
	//	nPharmacyID - The Pharmacy that the person belongs to
	//	nPersonID - If editing, a PersonT record for the staff we wish to edit.  -1 if new
	//	nType - 0 for Pharmacists, 1 for General Staff
	UINT EditStaff(long nPharmacyID, long nPersonID, long nType);

// Dialog Data
	enum { IDD = IDD_EDIT_PHARMACY_INDIV_STAFF_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	//Members
	long m_nPharmacyID;
	long m_nPersonID;
	long m_nType;

	//Dialog functionality
	void LoadFromData();
	bool SaveToData();

	//Controls
	NXDATALIST2Lib::_DNxDataListPtr m_pPrefixList;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxEdit m_editFirst;
	CNxEdit m_editMiddle;
	CNxEdit m_editLast;
	CNxEdit m_editSuffix;

	DECLARE_MESSAGE_MAP()
};
