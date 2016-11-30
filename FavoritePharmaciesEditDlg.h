#if !defined(AFX_FAVORITEPHARMACIESEDITDLG_H__E29AB405_43D1_4649_B847_6475DB09B040__INCLUDED_)
#define AFX_FAVORITEPHARMACIESEDITDLG_H__E29AB405_43D1_4649_B847_6475DB09B040__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// CFavoritePharmaciesEditDlg dialog

// (j.jones 2008-10-06 15:20) - PLID 31596 - created

class CFavoritePharmaciesEditDlg : public CNxDialog
{
public:
	CFavoritePharmaciesEditDlg(CWnd* pParent);   // standard constructor

	CString m_strPatientName;
	long m_nPatientID;

// Dialog Data
	enum { IDD = IDD_FAVORITE_PHARMACIES_EDIT_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnMoveUp;
	CNxIconButton	m_btnMoveDown;
	CNxColor		m_bkg;

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_PharmacyCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_FavoriteList;

	// (a.walling 2009-03-31 17:21) - PLID 33573 - Add the "< Add from Pharmacy Directory >" option to the pharmacy combo
	void AddPharmacyDirectoryOptionToCombo();

	
	// (a.walling 2009-03-31 17:21) - PLID 33573 - Help keep track of 'special' pharmacy IDs that link to commands
	enum ESpecialPharmacyIDs {
		espiAddFromDirectory = -2,
	};

	void EnableArrows();

	virtual BOOL OnInitDialog();
	afx_msg void OnBtnClosePharmacies();
	afx_msg void OnBnClickedBtnMovePharmUp();
	afx_msg void OnBnClickedBtnMovePharmDown();
	afx_msg void OnSelChosenPharmacyCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChangedFavoritePharmaciesList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRButtonDownPharmaciesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()	
};

#endif // !defined(AFX_FAVORITEPHARMACIESEDITDLG_H__E29AB405_43D1_4649_B847_6475DB09B040__INCLUDED_)
