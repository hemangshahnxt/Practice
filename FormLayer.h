#pragma once

typedef CArray<COleVariant, COleVariant> VarAry;

extern class CFormQuery g_aryFormQueries[];

//deletes fonts on closing, but doesn't delete the window!
class FormLayer  
{
public: 	// (j.armen 2014-03-27 16:28) - PLID 60784 - controls are now passed via a typed array
	void Load (int form, CString &where, CString &orderby, std::vector<shared_ptr<class FormControl>>& controls, int *i, long nSetupGroupID /* = -1*/, CStringArray* pastrParams = NULL, VarAry* pavarParams = NULL);
	void Refresh(int form, std::vector<shared_ptr<class FormControl>>& controls);
	void Save(int iDocumentID, int form, std::vector<shared_ptr<class FormControl>>& controls);
	void ChangeParameter(CString strParam, COleVariant var);
	void Capitalize(int form, std::vector<shared_ptr<class FormControl>>& controls);
	void UnPunctuate(int form, std::vector<shared_ptr<class FormControl>>& controls, CDWordArray *aryIDsToIgnore = NULL);
//	CDaoQueryDef *m_pQueryDef;
	FormLayer(CWnd *wnd, ADODB::_RecordsetPtr prsHistory);
	virtual			~FormLayer();
//	virtual void	DeleteThis(void){delete this;}
	CFont			*m_pFont;
	CFont			*m_pStaticFont;
	CFont			*m_pItalicFont;
	CFont			*m_pMiniFont;
	long			m_color;
	ADODB::_RecordsetPtr	m_rs;

	CWnd			*m_pWnd;

	int				m_formID;

	CStringArray	m_astrParams;	// Parameter strings for the SQL
	CString m_strWhere, m_strOrderBy;
	VarAry m_avarParams;	// Parameters for the SQL
	ADODB::_RecordsetPtr m_prsHistory;
};

// Only show the source field (to generate a name map)
//#define SHOW_SOURCE_ONLY