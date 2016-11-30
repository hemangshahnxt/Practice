// (c.haag 2008-11-26 13:45) - PLID 10776 - Iniital implementation

#pragma once
#include "afxwin.h"


// CInactiveProceduresDlg dialog

class CInactiveProceduresDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInactiveProceduresDlg)

public:
	CInactiveProceduresDlg(CWnd* pParent);   // standard constructor
	virtual ~CInactiveProceduresDlg();

// Dialog Data
	enum { IDD = IDD_INACTIVE_PROCEDURES };

private:
	// The number of procedures activated in this session
	int m_nActivatedProcedures;

private:
	NXDATALIST2Lib::_DNxDataListPtr m_dlProcedures;

public:
	int GetActivatedProcedureCount();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CNxIconButton m_btnClose;
	DECLARE_EVENTSINK_MAP()
	void DblClickCellInactiveTypes(LPDISPATCH lpRow, short nColIndex);
};
