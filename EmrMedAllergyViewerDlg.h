#pragma once


// CEmrMedAllergyViewerDlg dialog
// (c.haag 2010-07-28 15:39) - PLID 38928 - Initial implementation. This dialog
// displays medications, allergies, and prescriptions for an EMR.

class CEmrMedAllergyViewerDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrMedAllergyViewerDlg)

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dlItems;

public:
	CEmrMedAllergyViewerDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrMedAllergyViewerDlg();

// Dialog Data
	enum { IDD = IDD_EMR_MED_ALLERGY_VIEWER_DLG };

private:
	long m_nPatientID;
	CString m_strPatientName;

private:
	BOOL m_bIncludeInactiveItems;

private: // Variables used to retain scroll position after a requery
	long m_nTopRowID;
	long m_nTopRecordID;

private:
	// (c.haag 2010-12-13 9:44) - PLID 41817 - Shows or hides a specified column
	void ShowColumn(short nCol, BOOL bShow);

public:
	long GetPatientID() const { return m_nPatientID; }
	void SetPatientID(long n) { m_nPatientID = n; }

	CString GetPatientName() const { return m_strPatientName; }
	void SetPatientName(const CString& str) { m_strPatientName = str; }

public:
	void Requery(); // Requery the visible list
	void UpdateButtonText(); // Update the button text

public:
	void SaveSize(); // Save the current size and position
	void RestoreSize(); // Restore the last saved size and position

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnToggleInactiveMedAllergies();
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedListMedallergyitems(short nFlags);
};
