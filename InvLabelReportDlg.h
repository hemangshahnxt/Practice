#pragma once

// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
// CInvLabelReportDlg dialog

class CInvLabelReportDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvLabelReportDlg)

public:
	CInvLabelReportDlg(CWnd* pParent);   // standard constructor
	CInvLabelReportDlg(CWnd* pParent /*=NULL*/, long nProductID); //Constructor for printing out a single product (One that is not on an order)
	virtual ~CInvLabelReportDlg();
void SetOrderID(long nOrderID)	{	m_nOrderID = nOrderID;	}
// (b.spivey, October 21, 2011) - PLID 46073 - mutator for m_bIsFramesReport 
void SetIsFramesLabel(BOOL bIsFramesReport)	{	m_bIsFramesReport = bIsFramesReport;	}
// Dialog Data
	enum { IDD = IDD_INV_LABEL_REPORT };
	
	CNxIconButton	m_btnPrint;
	CNxIconButton	m_btnPrintPreview;
	CNxIconButton	m_btnCancel;

	//r.wilson 3/7/2012 PLID 48351
	void SetProductID(long nProductId){m_nProductId = nProductId;};

protected:
	//r.wilson 3/7/2012 PLID 48351
	long m_nProductId;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	long m_nOrderID;
	BOOL m_bIsFramesReport; 
	NXDATALIST2Lib::_DNxDataListPtr m_pOrderList;			//Main datalist
	virtual BOOL OnInitDialog();
	virtual void OnPrint();
	virtual void OnPrintPreview();
	virtual void PrintRecord(BOOL IsPreview ) ;
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void EditingFinishingInvLabelReportList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};
