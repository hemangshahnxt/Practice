#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFormDisplayDlg dialog

typedef CArray<COleVariant, COleVariant> VarAry;

typedef struct {
	int key1, key2;
	CPtrArray adwID;
	CStringArray astrText;
//	CDWordArray adwType; // 0=Edit 1=Radio
} CHANGE_STRUCT;

class CFormDisplayDlg : public CDialog
{
// Construction
public:
	CPrintDialog *m_printDlg;
	CFormDisplayDlg(int ID, CWnd* pParent /*=NULL*/);
	CFormDisplayDlg(CWnd* pParent);	// standard constructor
	~CFormDisplayDlg();
	void Scroll (int x, int y);
	void Load (int form, CString where, CString orderby, int *i, long nSetupGroupID = -1, CStringArray* pastrParams = NULL, VarAry* pavarParams = NULL);
	void Refresh(int form);
	void Capitalize();
	void UnPunctuate(CDWordArray *aryIDsToIgnore = NULL);
	BOOL OnPrint(BOOL capitalize, CString strRegName, CDWordArray *aryIDsToIgnore = NULL, CDC *pPrintDC = NULL);
	void Save(int key1, int key2, int iDocumentID, BOOL boSaveAll);
	void ChangeParameter(int form, CString strParam, COleVariant var);

	void (*m_pOnCommand)(WPARAM wParam, LPARAM lParam, CDWordArray& dwChangedForms, int FormControlID, CFormDisplayDlg* dlg);
	BOOL (*m_pPrePrintFunc)(CDWordArray& dwChangedForms, CFormDisplayDlg* dlg);
	void (*m_pOnKeyDown)(CDialog* pFormDisplayDlg, MSG* pMsg);
	BOOL m_ShowPrintDialog;
	CWnd * GetControl (int identifier);//search by id fields!

	long color;

	// (j.armen 2014-03-27 16:28) - PLID 60784 - use vectors of shared_ptr instead of generic pointer array
	std::vector<shared_ptr<class FormControl>> m_ControlArray;
	std::vector<shared_ptr<class FormLayer>> m_LayerArray;

	// Used for tracking changes on a form
	void TrackChanges(int key1, int key2);
	void StopTrackingChanges();
	void ReapplyChanges(int key1, int key2);
	void UndoChanges();

	//////////////////////////////////////////////
	// For historical information
	void SetDocumentID(int iDocumentID);

	std::vector<shared_ptr<CHANGE_STRUCT>> m_pChangeArray;
	int m_ChangeKey1, m_ChangeKey2;
	ADODB::_RecordsetPtr m_rsHistory;

	// (j.jones 2007-06-22 13:25) - PLID 25665 - track if we've edited fields, and if we do,
	// tell the parent we've done so
	// (j.jones 2007-06-25 10:31) - PLID 25663 - changed name of this variable
	BOOL m_bFieldsEdited;

// Dialog Data
	//{{AFX_DATA(CFormDisplayDlg)
	enum { IDD = IDD_DIALOG3 };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormDisplayDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON		m_hIcon;
//	CPtrArray	EditArray,
//				LineArray,
//				DateArray,
//				CheckArray;
	CFont		*pFont, 
				*pStaticFont,
				*pItalicFont;
	int			m_xscroll;
	int			m_yscroll;
	// (j.jones 2007-06-22 09:28) - PLID 25665 - used to color the background of an edited field
	CBrush		m_RegBGBrush,
				m_EditedBGBrush;

	// (j.jones 2007-06-22 12:16) - PLID 25665 - cache whether we want to color edited fields
	BOOL m_bColorEditedFields;
	// (j.jones 2007-06-22 13:25) - PLID 25665 - track if we've edited fields, and if we do,
	// tell the parent we've done so
	// (j.jones 2007-06-25 10:31) - PLID 25663 - changed the name of this function
	void ReflectEditedFields();

	void SetPrintAlign(CDC *pDC, HDC hdcPrn);
	void FormatPage (CDC *pDC);
	// Generated message map functions
	//{{AFX_MSG(CFormDisplayDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};