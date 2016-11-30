#pragma once

// (a.walling 2013-10-21 10:19) - PLID 59113 - Dialog to manually add / edit a SNOMED / UTS code.

// CEditSingleCodeDlg dialog

class CEditSingleCodeDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditSingleCodeDlg)

public:
	CEditSingleCodeDlg(CWnd* pParent);   // standard constructor
	virtual ~CEditSingleCodeDlg();

// Dialog Data
	enum { IDD = IDD_EDIT_SINGLE_CODE };

	long GetCodeID() const
	{ return m_nCodeID; }
	void SetCodeID(long nCodeID)
	{ m_nCodeID = nCodeID; }

protected:
	long m_nCodeID;

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
