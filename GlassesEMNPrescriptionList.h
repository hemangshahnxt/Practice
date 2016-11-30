#pragma once
#include "InventoryRc.h"
#include "InvVisionWebUtils.h"

// (s.dhole 2012-01-18 12:14) - PLID 47455 Added new Dialog
// CGlassesEMNPrescriptionList dialog

class CGlassesEMNPrescriptionList : public CNxDialog
{
	DECLARE_DYNAMIC(CGlassesEMNPrescriptionList)

public:
	CGlassesEMNPrescriptionList(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGlassesEMNPrescriptionList();
	
	long m_nPatientID ;	
	long m_nOrderId, m_nEMNID;
	CString m_ProviderName;
	long m_nProviderID;
	//r.wilson
	bool m_bHidePrintBtn;
// Dialog Data
	// (s.dhole 2012-03-02 19:16) - PLID  48354 
	enum { IDD = IDD_GLASSES_EMN_PRESCRIPTIONS };
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_BtnSelect;
	CNxIconButton	m_BtnClose;
	CNxIconButton   m_BtnPrint;
	// (s.dhole 2012-03-06 16:43) - PLID 48354
	// (s.dhole 2012-04-25 12:35) - PLID 47395 Change name
	enum PrescriptionWindowType{
		pwAllowRxSelection=0,
		pwShowRxList,
	};
	// (s.dhole 2012-03-06 16:43) - PLID 48354
	// (s.dhole 2012-04-25 12:35) - PLID 47395 Change name
	PrescriptionWindowType  m_PrescriptionWindowDisplayType;
	// added prism1, prism2 ,base1,base2 
	struct LensRxInfo {
		CString strSphere;
		CString strCylinder;
		CString strAxis;
		CString strAddition;
		CString strPrism1;
		CString strPrismAxisStr;
		CString strBase1;
		CString strPrism2;
		CString strBase2;
		CString strDistPD;
		CString strNearPD;
		CString strHeight;
		GlassesOrderLensDetails *pEmnInfo; //TES 4/17/2012 - PLID 49746
		LensRxInfo() {
			pEmnInfo = NULL;
		}
	};
	// (s.dhole 2012-04-25 12:28) - PLID 49968 Added Emnid 
	struct LensRxDetailInfo{
		BOOL ISRecordExist;
		long nEmnId;
		_variant_t strRxDate;
		_variant_t strRxExpDate;
		LensRxInfo LensRxOD;
		LensRxInfo LensRxOS;
	};
	LensRxDetailInfo m_oLensRx;	
	CString m_strPatientName ;

	// (s.dhole 2012-04-20 13:14) - PLID 49728 if order is billed then we should not allow user to change few fields from order 
	BOOL m_bIsReadOnly;
	//TES 4/16/2012 - PLID 49368 - Set this to tell the dialog to return data in m_oContactLensRx instead of m_oLensRx
	bool m_bCalledForContactLens;

	bool m_bCalledForGlassesAndContacts;
	//TES 4/16/2012 - PLID 49368 - Structures to return contact lens prescription data
	struct ContactLensRxInfo {
		CString strSphere;
		CString strCylinder;
		CString strAxis;
		CString strAddition;
		CString strBC;
		CString strDiameter;
		CString strDocIns;	// (j.dinatale 2013-03-21 09:45) - PLID 55766
		CString strNote; //TES 12/2/2015 - PLID 67671
		ContactLensOrderLensDetails *pEmnInfo; //TES 4/17/2012 - PLID 49746
		ContactLensRxInfo() {
			pEmnInfo = NULL;
		}
	};
	// (s.dhole 2012-04-25 12:29) - PLID 49969 Added Emnid
	struct ContactLensRxDetailInfo {
		BOOL bRecordExists;
		long nEmnId;
		_variant_t varRxDate;
		_variant_t varRxExpDate;
		ContactLensRxInfo clriOD;
		ContactLensRxInfo clriOS;
	};
	ContactLensRxDetailInfo m_oContactLensRx;
protected:
	
	virtual BOOL OnInitDialog();
	NXDATALIST2Lib::_DNxDataListPtr m_GlassesEMNPrescriptionList;
	NXDATALIST2Lib::_DNxDataListPtr m_ContactLensRxList;	// (j.dinatale 2012-03-09 16:41) - PLID 48724	
	void LoadContactLensList();	// (j.dinatale 2012-03-09 16:41) - PLID 48724

	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// (s.tullis 2015-10-19 16:34) - PLID 67263 
	void LoadContactListRow(IN ADODB::_RecordsetPtr rsContactRx , IN OUT BOOL &bSetDocInsWidth, IN BOOL bHL7Record = FALSE);
	void LoadHL7Contacts(BOOL IN bSetDocInsWidth);
	void LoadHL7Glasses();
	CSqlFragment GetHL7GlassesSQL();
	CSqlFragment GetHL7ContactsSQL();
	void SetLensRxDetail(NXDATALIST2Lib::IRowSettingsPtr pRow ,OUT LensRxInfo  &LensRxDetail);
	//TES 4/16/2012 - PLID 49368 - Added equivalent function for contact lens prescriptions
	void SetLensRxDetail(NXDATALIST2Lib::IRowSettingsPtr pRow, OUT ContactLensRxInfo &LensRxDetail);
	void  LoadRxRows(NXDATALIST2Lib::IRowSettingsPtr pRxRow ,ADODB::_RecordsetPtr rsRx  );
	//TES 4/17/2012 - PLID 49746 - Changed to take a pointer to the GlassesOrderLensDetails struct, which will then be stored in the datalist.
	void LoadEMNRxRows(NXDATALIST2Lib::IRowSettingsPtr pRxRow ,GlassesOrderLensDetails *prxDetail  );
	//TES 4/16/2012 - PLID 49369 - Added equivalent function for contact lens prescriptions
	//TES 4/17/2012 - PLID 49746 - Changed to take a pointer to the ContactLensOrderLensDetails struct, which will then be stored in the datalist.
	void LoadEMNRxRows(NXDATALIST2Lib::IRowSettingsPtr pRxRow, ContactLensOrderLensDetails *prxDetail);
	void OnPreviewEmn(long nEmnID);
	DECLARE_MESSAGE_MAP()
	BOOL m_bAllowOrder,m_bAllowEmn;
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;
	BOOLEAN PrintRecord(BOOL IsPreview, BOOL bPrintGlassesRx, BOOL bPrintContactLensRx);
	//r.wilson
	CString ValidDoubleSQL(CString strDouble);
	CString ValidLongSQL(long strLong);
	CString ValidDateSQL(CString  strDate);
	CString ValidLongSQL(CString strLong);
	
public:
	DECLARE_EVENTSINK_MAP()
	
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedSelectPrescriptions();
	afx_msg void OnBnClickedCloseGlassesEmnPrescriptionsClose();
	void LeftClickContactPrescriptionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void LeftClickGlassesPrescriptionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	virtual void OnDestroy();
	afx_msg void OnBnClickedButtonPrint();
};
