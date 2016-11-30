// GlassesEMNPrescriptionList.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "GlassesEMNPrescriptionList.h"
#include "InventoryRc.h"
#include "VisionWebOrderDlg.h"
#include "InvVisionWebUtils.h"
#include "EMRPreviewPopupDlg.h"
// (s.dhole 2012-01-18 12:14) - PLID 47455 Added new Dialog
// CGlassesEMNPrescriptionList dialog
#include "GlassesContactLenseRxPrintSelectDlg.h" // (r.wilson 3/19/2012) PLID 48952
#include "ReportInfo.h" //(r.wilson 3/21/2012) PLID 48952
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "GlobalDataUtils.h"
#include "EMN.h"
#include "EMNProvider.h"
#include "HL7Utils.h"
#include <OpticalUtils.h>

using namespace ADODB;
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CGlassesEMNPrescriptionList, CNxDialog)

// (j.dinatale 2012-03-09 14:25) - PLID 48724 - contact lens Rx info!
// (s.dhole 2012-04-25 12:35) - PLID 47395 Added orderid
namespace PrevConLensRxList {
	enum PrevConLensRxColumns {
		EmnID = 0,
		LensRxID,
		LensDetailRxID,
		Date,
		RxDate,
		RxExpirationDate,
		ProviderID, //(r.wilson 6/4/2012) PLID 48952 - added field
		ProviderName, //r.wilson (6/13/2012) PLID 50915 - added field		
		RxIssueDate, 
		RxAction,
		Rxeye_site,
		PrescriptionSphere,
		Cylinder,
		CylinderAxis,
		AdditionValue,
		BC,
		Diameter,
		Note,	// (j.dinatale 2012-05-15 16:11) - PLID 50346 - add the note field!
		DocIns,	// (j.dinatale 2013-03-19 16:55) - PLID 55766 
		SortDate,
		ObjPtr, //TES 4/17/2012 - PLID 49746
		OrderID,	
		HL7ID, // (s.tullis 2015-10-19 16:18) - PLID 67263 
	};
};



// (j.dinatale 2012-03-12 12:12) - PLID 48724 - removed the type column
// (s.dhole 2012-04-25 12:35) - PLID 47395 Added orderid
enum EmnToEMnOrderListColumns {
	lvrxEmnID=0,
	lvrxLensRxID,
	lvrxLensDetailRxid,
	lvrxDate,
	lvrxRxDate,
	lvrxRxExpirationDate,
	lvrxProviderID, //(r.wilson 6/4/2012) PLID 50915 - added field
	lvrxProviderName, //r.wilson (6/18/2012) PLID 50915 - added field
	lvrxRxAction,
	lvrxeye_site,
	lvrxPrescriptionSphere,
	lvrxCylinder,
	lvrxCylinderAxis,
	lvrxAdditionValue,
	lvrxPrism1,
	lvrxBase1,
	lvrxPrism2,
	lvrxBase2,
	lvrxFarHalfPd,
	lvrxNearHalfPd,
	lvrxSegHeight,
	//lvrxRowID,
	lvrxSortDate,
	lvrxObjPtr, //TES 4/17/2012 - PLID 49746
	lvrxOrderID,
	lvrxHL7ID,// (s.tullis 2015-10-19 16:18) - PLID 67263
	};

//(r.wilson 5/14/2012) PLID 48952
namespace RxReportTypes {
	enum RxReportTypes {
		GlassesReport = 0,
		ContactLensReport,
		ContactAndGlassesReport
	};
};

CGlassesEMNPrescriptionList::CGlassesEMNPrescriptionList(CWnd* pParent /*=NULL*/)
	: CNxDialog(CGlassesEMNPrescriptionList::IDD, pParent)
{
	m_bCalledForContactLens = false;
	// (s.dhole 2012-04-25 12:35) - PLID 47395 
	m_bIsReadOnly =FALSE ; 
	m_bAllowOrder=FALSE ; 
	m_bAllowEmn=FALSE ; 
	m_pEMRPreviewPopupDlg=NULL;
	m_bHidePrintBtn = true;

}

CGlassesEMNPrescriptionList::~CGlassesEMNPrescriptionList()
{
		
}

void CGlassesEMNPrescriptionList::DoDataExchange(CDataExchange* pDX)
{

	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_CLOSE_GLASSES_EMN_PRESCRIPTIONS , m_btnCancel);
	DDX_Control(pDX, ID_SELECT_PRESCRIPTIONS , m_BtnSelect);
	DDX_Control(pDX, ID_CLOSE_GLASSES_EMN_PRESCRIPTIONS_CLOSE , m_BtnClose);
	DDX_Control(pDX, IDC_BUTTON_PRINT_RX , m_BtnPrint);	
		
}


BEGIN_MESSAGE_MAP(CGlassesEMNPrescriptionList, CNxDialog)
	ON_BN_CLICKED(ID_CLOSE_GLASSES_EMN_PRESCRIPTIONS , &CGlassesEMNPrescriptionList::OnBnClickedCancel)
	ON_BN_CLICKED(ID_SELECT_PRESCRIPTIONS, &CGlassesEMNPrescriptionList::OnBnClickedSelectPrescriptions)
	ON_BN_CLICKED(ID_CLOSE_GLASSES_EMN_PRESCRIPTIONS_CLOSE, &CGlassesEMNPrescriptionList::OnBnClickedCloseGlassesEmnPrescriptionsClose)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_PRINT_RX, &CGlassesEMNPrescriptionList::OnBnClickedButtonPrint)
END_MESSAGE_MAP()


// CGlassesEMNPrescriptionList message handlers
BEGIN_EVENTSINK_MAP(CGlassesEMNPrescriptionList, CNxDialog)
ON_EVENT(CGlassesEMNPrescriptionList, IDC_CON_LEN_PREV_RX, 19, CGlassesEMNPrescriptionList::LeftClickContactPrescriptionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
ON_EVENT(CGlassesEMNPrescriptionList, IDC_GLASSES_ORDER_EMN_LIST, 19, CGlassesEMNPrescriptionList::LeftClickGlassesPrescriptionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

BOOL CGlassesEMNPrescriptionList::OnInitDialog() 
{
CNxDialog::OnInitDialog();
try {
	m_btnCancel.AutoSet(NXB_CANCEL );
	m_BtnSelect.AutoSet(NXB_OK);
	m_BtnPrint.AutoSet(NXB_PRINT_PREV);	
	
	//r.wilson 
	if(m_bHidePrintBtn){
		m_BtnPrint.ShowWindow(SW_HIDE);
	}
	else
	{
		m_btnCancel.ShowWindow(SW_SHOW);
	}

	// (s.dhole 2012-02-27 17:24) - PLID 48354 
	m_BtnClose.AutoSet(NXB_CLOSE );
		
	SetWindowText(m_strPatientName + " - Vision Prescriptions"   );
	

	IRowSettingsPtr pRxRow, pRxPRow ;
	m_GlassesEMNPrescriptionList= BindNxDataList2Ctrl(IDC_GLASSES_ORDER_EMN_LIST, false);
	// (s.dhole 2012-04-25 12:35) - PLID 47395 Change column proporty 
	IColumnSettingsPtr prRxCol = m_GlassesEMNPrescriptionList->GetColumn(lvrxRxAction);
	if (m_PrescriptionWindowDisplayType == pwShowRxList ){
			prRxCol->FieldType = cftTextSingleLineLink ;
			prRxCol->ColumnTitle = "";
			m_GlassesEMNPrescriptionList->GetColumn(lvrxRxAction)->PutForeColor(RGB(0,0,255));
		}
		else{
			prRxCol->ColumnTitle = "Type";
			prRxCol->FieldType = cftTextSingleLine;
		}
	m_oLensRx.ISRecordExist=FALSE; 
	CWaitCursor cuWait;

	// (s.dhole 2012-04-25 12:35) - PLID 47395 check perimission and   License
	IColumnSettingsPtr pServicesCol; 
	if(g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
		m_bAllowOrder=TRUE; 
	}

	// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
	if(!(g_pLicense->HasEMR(CLicense::cflrSilent) == 2) 
		|| !(GetCurrentUserPermissions(bioPatientEMR) & SPT__R_________ANDPASS)) 
	{
		m_bAllowEmn=FALSE; 
	} 
	else{
		m_bAllowEmn=TRUE; 
	}



	// (j.dinatale 2012-03-12 12:11) - PLID 48724 - filter out only glasses orders here
	//r.wilson (6/18/2012) PLID 50915 - Load Provider's name from database
	// (s.tullis 2015-10-19 16:18) - PLID 67263 - added HL7ID
	_RecordsetPtr rsRx = CreateParamRecordset( "Select * FROM ( "
		" SELECT  GlassesOrderT.LensRxID,null AS PerentID, NULL AS id, NULL AS eye_site, NULL AS PrescriptionSphere, NULL AS FarHalfPd, NULL AS CylinderValue,NULL AS CylinderAxis,  "
		" NULL AS AdditionValue, NULL AS PrismValue, NULL AS SecondaryPrismValue, NULL AS PrismAxis,NULL AS PrismAxisStr, NULL AS NearHalfPd, NULL AS SegHeight, NULL AS PrismAxis2,  NULL AS PrismAxisStr2 , "
		" LensRxT.RxDate, LensRxT.RxExpirationDate,  "
		" GlassesOrderT.ID AS GlassesOrderID, GlassesOrderT.GlassesOrderType, GlassesOrderT.Date AS OrderDate ,  "
		" GlassesOrderT.PersonID, GlassesOrderT.Date AS GlassesOrderDate, "
		" GlassesOrderT.ProviderID AS ProviderID, "
		" PersonProvider.Last + ', ' + PersonProvider.First AS ProviderName, "
		" HL7MessageID as HL7ID "
		" FROM GlassesOrderT  INNER JOIN LensRxT  "
		" ON LensRxT.id = GlassesOrderT.LensRxID "
		//TES 4/19/2012 - PLID 49819 - This was joining on RightLensDetailRxID, leading to an exception if you loaded the dialog with
		// any orders that had only a left lens.
		" INNER JOIN LensDetailRxT ON (LensRxT.RightLensDetailRxID = LensDetailRxT.id OR LensRxT.LeftLensDetailRxID = LensDetailRxT.ID) "
		//r.wilson (6/13/2012) PLID 50915 - Joined another PersonT to get provider's name
		" LEFT JOIN PersonT AS PersonProvider ON GlassesOrderT.ProviderID = PersonProvider.ID "
		" WHERE IsDelete=0 AND GlassesOrderT.PersonID ={INT}  AND GlassesOrderT.GlassesOrderType = {INT} " 
		" union  "
		" SELECT  GlassesOrderT.LensRxID,LensRxT.ID AS PerentID ,  LensDetailRxT.id, 'OD' as eye_site,  PrescriptionSphere, FarHalfPd, CylinderValue, CylinderAxis,  "
		" AdditionValue, PrismValue, SecondaryPrismValue, PrismAxis, PrismAxisStr, NearHalfPd, SegHeight, PrismAxis2, PrismAxisStr2 , "
		" NULL   aS RxDate, NULL AS RxExpirationDate, "
		" GlassesOrderT.ID AS GlassesOrderID, NULL  GlassesOrderType,  GlassesOrderT.Date AS OrderDate , "
		" GlassesOrderT.PersonID, NULL  AS GlassesOrderDate,  "
		" NULL AS ProviderID, "
		" NULL AS ProviderName, "
		" HL7MessageID as HL7ID "
		" FROM GlassesOrderT  INNER JOIN LensRxT  "
		" ON LensRxT.id = GlassesOrderT.LensRxID "
		" INNER JOIN LensDetailRxT  ON LensRxT.RightLensDetailRxID = LensDetailRxT.id "
		" WHERE IsDelete=0 AND GlassesOrderT.PersonID ={INT}  AND GlassesOrderT.GlassesOrderType = {INT} "
		" union  "
		" SELECT  GlassesOrderT.LensRxID,LensRxT.ID AS PerentID ,  NULL AS id, 'OD' as eye_site,  NULL AS PrescriptionSphere, NULL AS FarHalfPd, NULL AS CylinderValue, NULL AS CylinderAxis,  "
		" NULL AS AdditionValue, NULL AS PrismValue, NULL AS SecondaryPrismValue, NULL AS PrismAxis, NULL AS PrismAxisStr, NULL AS NearHalfPd, NULL AS SegHeight, NULL AS PrismAxis2, NULL AS PrismAxisStr2 , "
		" NULL   aS RxDate, NULL AS RxExpirationDate,  "
		" GlassesOrderT.ID AS GlassesOrderID, NULL  GlassesOrderType, GlassesOrderT.Date AS OrderDate , "
		" GlassesOrderT.PersonID, NULL  AS GlassesOrderDate,  "
		" NULL AS ProviderID, "
		" NULL AS ProviderName, "
		" HL7MessageID as HL7ID "
		" FROM GlassesOrderT  INNER JOIN LensRxT  "
		" ON LensRxT.id = GlassesOrderT.LensRxID "
		" INNER JOIN LensDetailRxT  ON LensRxT.leftLensDetailRxID = LensDetailRxT.id "
		" WHERE IsDelete=0  and LensRxT.RightLensDetailRxID IS NULL AND GlassesOrderT.PersonID ={INT} AND GlassesOrderT.GlassesOrderType = {INT} "
		" union  "
		" SELECT  GlassesOrderT.LensRxID,LensRxT.ID AS PerentID, LensDetailRxT.id, 'OS' as eye_site,  PrescriptionSphere, FarHalfPd, CylinderValue, CylinderAxis,  "
		" AdditionValue, PrismValue, SecondaryPrismValue, PrismAxis, PrismAxisStr, NearHalfPd, SegHeight, PrismAxis2, PrismAxisStr2 , "
		" NULL   aS RxDate, NULL AS RxExpirationDate,  "
		" GlassesOrderT.ID AS GlassesOrderID, NULL  GlassesOrderType, GlassesOrderT.Date AS OrderDate , "
		" GlassesOrderT.PersonID, NULL  AS GlassesOrderDate,  "
		" NULL AS ProviderID, "
		" NULL AS ProviderName, "
		" HL7MessageID as HL7ID "
		" FROM GlassesOrderT  INNER JOIN LensRxT  "
		" ON LensRxT.id = GlassesOrderT.LensRxID "
		" INNER JOIN LensDetailRxT  ON LensRxT.LeftLensDetailRxID = LensDetailRxT.id "
		" WHERE IsDelete=0 AND GlassesOrderT.PersonID ={INT}  AND GlassesOrderT.GlassesOrderType = {INT} "
		" union "
		" SELECT  GlassesOrderT.LensRxID,LensRxT.ID AS PerentID ,  NULL AS id, 'OS' as eye_site,  NULL AS PrescriptionSphere, NULL AS FarHalfPd, NULL AS CylinderValue, NULL AS CylinderAxis,  "
		" NULL AS AdditionValue, NULL AS PrismValue, NULL AS SecondaryPrismValue, NULL AS PrismAxis, NULL AS PrismAxisStr, NULL AS NearHalfPd, NULL AS SegHeight, NULL AS PrismAxis2, NULL AS PrismAxisStr2 , "
		" NULL   aS RxDate, NULL AS RxExpirationDate,  "
		" GlassesOrderT.ID AS GlassesOrderID, NULL  GlassesOrderType, GlassesOrderT.Date AS OrderDate , "
		" GlassesOrderT.PersonID, NULL  AS GlassesOrderDate,  "
		" NULL AS ProviderID, "
		" NULL AS ProviderName, "
		" HL7MessageID as HL7ID "
		" FROM GlassesOrderT  INNER JOIN LensRxT  "
		" ON LensRxT.id = GlassesOrderT.LensRxID "
		" INNER JOIN LensDetailRxT  ON LensRxT.RightLensDetailRxID  = LensDetailRxT.id "
		" WHERE IsDelete=0  and LensRxT.LeftLensDetailRxID IS NULL AND GlassesOrderT.PersonID ={INT} AND GlassesOrderT.GlassesOrderType = {INT} ) _RxDetailQ   "
		" ORDER BY LensRxID,eye_site"
		,m_nPatientID, vwotSpectacleLens, m_nPatientID, vwotSpectacleLens, m_nPatientID, vwotSpectacleLens, m_nPatientID, vwotSpectacleLens , m_nPatientID);
	
		while(!rsRx->eof) {
			if (AdoFldLong(rsRx,"PerentID",-1)==-1) 
			{
				if (AdoFldByte(rsRx, "GlassesOrderType",0)!=0)
				{
					pRxPRow = m_GlassesEMNPrescriptionList->GetNewRow();

					// (j.dinatale 2012-03-12 12:16) - PLID 48724 - no longer need the type column
					/*	VisionWebOrderType m_vwot =(VisionWebOrderType)AdoFldByte(rsRx, "GlassesOrderType");
					if (m_vwot ==vwotContactLensPatient)
					pRxPRow->PutValue(lvRxAction,_bstr_t("Contact Lens"));
					else if (m_vwot ==vwotSpectacleLens)
					pRxPRow->PutValue(lvRxAction,_bstr_t("Glasses"));*/
				}
				// (s.dhole 2012-04-25 12:35) - PLID 47395 Set value
				if (m_PrescriptionWindowDisplayType==pwShowRxList ){
					if (m_bAllowOrder==TRUE){
					pRxPRow->PutValue(lvrxRxAction , "Click to Open Order");
					}
					else
					{
						// nothing
					}
				}
				else if (m_PrescriptionWindowDisplayType==pwAllowRxSelection ){
					pRxPRow->PutValue(lvrxRxAction , "Order");
				}
				CGlassesEMNPrescriptionList::LoadRxRows (pRxPRow,rsRx);
				m_GlassesEMNPrescriptionList->AddRowSorted(pRxPRow,NULL); 
			}
			else
			{
				pRxRow = m_GlassesEMNPrescriptionList->GetNewRow();
				//pRxRow->PutValue(lvRxAction,g_cvarNull);	// (j.dinatale 2012-03-12 12:16) - PLID 48724 - no longer need the type column
				CGlassesEMNPrescriptionList::LoadRxRows (pRxRow,rsRx);
				m_GlassesEMNPrescriptionList->AddRowSorted(pRxRow,pRxPRow); 
				pRxPRow->PutExpanded (VARIANT_TRUE);
			}

			rsRx->MoveNext();
		}


		_RecordsetPtr rsEmnRx = CreateParamRecordset( " SELECT EMRMasterT.PatientID, EMRMasterT.ID AS EMNID, EmrMasterT.ModifiedDate AS EmnModifiedDate "
			" FROM EMRMasterT  "
			" INNER JOIN (SELECT EmrDetailsT.EmrID FROM EmrDetailsT INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID   "
			" INNER JOIN EMRTopicsT ON EMRTopicsT.EMRID= EmrDetailsT.EmrID   "
			" WHERE EmrInfoT.HasGlassesOrderData = 1 AND EmrDetailsT.Deleted=0  "
			" AND EMRTopicsT.Deleted=0    GROUP BY EmrDetailsT.EmrID ) AS OrderableEmnsQ ON EmrMAsterT.ID = OrderableEmnsQ.EmrID   "
			" WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.PatientID ={INT} ", m_nPatientID);
		long  nEMnLensID=-1;
		long nEmnID =-1;
		//long nRowID =-1;
		while(!rsEmnRx->eof)
		{
			nEmnID =AdoFldLong(rsEmnRx, "EMNID",-1);
			if (nEmnID >-1)
			{
				COleDateTime dtEmnModifiedDate = AdoFldDateTime(rsEmnRx, "EmnModifiedDate");
				CEMN* pEmn = new CEMN(NULL);
				pEmn->LoadFromEmnID(nEmnID);
				GlassesOrder go;
				// to iusee existing function passint this string
				CString strIgnoredData;
				if(pEmn->GetGlassesOrderData(go, strIgnoredData)) {
					//load header
					pRxPRow = m_GlassesEMNPrescriptionList->GetNewRow();
					//pRxPRow->PutValue(lvrxRowID, (long)nRowID )	;
					//pRxPRow->PutValue(lvRxAction,_bstr_t("EMN"));	// (j.dinatale 2012-03-12 12:16) - PLID 48724 - no longer need the type column
					pRxPRow->PutValue(lvrxEmnID,nEmnID);
					pRxPRow->PutValue(lvrxLensRxID,nEMnLensID );
					pRxPRow->PutValue(lvrxLensDetailRxid, g_cvarNull);
					pRxPRow->PutValue(lvrxDate, _variant_t(pEmn->GetEMNDate(), VT_DATE));
					pRxPRow->PutValue(lvrxRxDate, _variant_t(pEmn->GetEMNDate(), VT_DATE));
					pRxPRow->PutValue(lvrxRxExpirationDate, g_cvarNull);
					pRxPRow->PutValue(lvrxSortDate,_variant_t(pEmn->GetEMNDate(), VT_DATE));
					// (s.dhole 2012-04-25 12:35) - PLID 47395 set text
					if (m_PrescriptionWindowDisplayType==pwShowRxList   )
					{
						if  (nEmnID>0 && m_bAllowEmn==TRUE){
							pRxPRow->PutValue(lvrxRxAction , "Click to Open EMN");
						}else {
							// Nothing
						}
					}
					else if (m_PrescriptionWindowDisplayType==pwAllowRxSelection )
					{
						pRxPRow->PutValue(lvrxRxAction , "EMN");
					}

					CString strProviderNames;
					CString strProviderIDs;

					for(int i = 0; i < pEmn->GetProviderCount() ; i++)
					{
						if(i > 0){
							strProviderNames += ", ";
							strProviderIDs += "; ";
						}
						EMNProvider *o_EmnProv = pEmn->GetProvider(i);
						strProviderNames += o_EmnProv->strName;
						CString strTmp;
						strTmp.Format("%li",o_EmnProv->nID);
						strProviderIDs += strTmp;
					}

					//r.wilson (6/13/2012) PLID 50915 - ProviderID(s)
					pRxPRow->PutValue(lvrxProviderID,_bstr_t(strProviderIDs));	

					//r.wilson (6/13/2012) PLID 50915 - ProviderName(s)										
					pRxPRow->PutValue(lvrxProviderName,_bstr_t(strProviderNames));

					//find me (r.wilson)					
					//pRxPRow->PutValue(lvrxLocationID,variant_t(pEmn->GetLocationID()));

					m_GlassesEMNPrescriptionList->AddRowSorted(pRxPRow,NULL); 

					// Load Detail OD
					pRxRow = m_GlassesEMNPrescriptionList->GetNewRow();
					//pRxRow->PutValue(lvrxRowID, (long)nRowID )	;
					pRxRow->PutValue(lvrxEmnID,nEmnID);
					pRxRow->PutValue(lvrxLensRxID,nEMnLensID );
					pRxRow->PutValue(lvrxLensDetailRxid, nEMnLensID);
			
					pRxRow->PutValue(lvrxeye_site,_bstr_t("OD"));
					//TES 4/17/2012 - PLID 49746 - Copy the struct into a pointer that we can store in the datalist, so that any information
					// in it that isn't in the displayed datalist fields can be accessed later.
					GlassesOrderLensDetails *pGold = new GlassesOrderLensDetails;
					*pGold = go.golOD;
					CGlassesEMNPrescriptionList::LoadEMNRxRows (pRxRow, pGold);
					pRxRow->PutValue(lvrxSortDate,_variant_t(pEmn->GetEMNDate(), VT_DATE));
					m_GlassesEMNPrescriptionList->AddRowSorted(pRxRow,pRxPRow); 
					pRxPRow->PutExpanded (VARIANT_TRUE);

					// Load Detail OS
					pRxRow = m_GlassesEMNPrescriptionList->GetNewRow();
					//pRxRow->PutValue(lvrxRowID, (long)nRowID )	;
					pRxRow->PutValue(lvrxEmnID,nEmnID);
					pRxRow->PutValue(lvrxLensRxID,nEMnLensID );
					pRxRow->PutValue(lvrxLensDetailRxid, nEMnLensID);
					pRxRow->PutValue(lvrxeye_site,_bstr_t("OS"));
					//TES 4/17/2012 - PLID 49746 - Copy the struct into a pointer that we can store in the datalist, so that any information
					// in it that isn't in the displayed datalist fields can be accessed later.
					pGold = new GlassesOrderLensDetails;
					*pGold = go.golOS;
					CGlassesEMNPrescriptionList::LoadEMNRxRows (pRxRow, pGold);
					pRxRow->PutValue(lvrxSortDate,_variant_t(pEmn->GetEMNDate(), VT_DATE));					
					m_GlassesEMNPrescriptionList->AddRowSorted(pRxRow,pRxPRow); 
					pRxPRow->PutExpanded (VARIANT_TRUE);
				}
				delete pEmn;
			}
			rsEmnRx->MoveNext();
		}

		// (j.dinatale 2012-03-09 17:21) - PLID 48724 - load prev contact lens rx info in another list
		m_ContactLensRxList = BindNxDataList2Ctrl(IDC_CON_LEN_PREV_RX, false);
		// (s.dhole 2012-04-25 12:35) - PLID 47395 
		prRxCol = m_ContactLensRxList->GetColumn(PrevConLensRxList::RxAction );
		if (m_PrescriptionWindowDisplayType == pwShowRxList){
			prRxCol->FieldType = cftTextSingleLineLink ;
			prRxCol->ColumnTitle = "";
			m_ContactLensRxList->GetColumn(PrevConLensRxList::RxAction)->PutForeColor(RGB(0,0,255));
		}
		else{
			prRxCol->FieldType = cftTextSingleLine;
			prRxCol->ColumnTitle = "Type";
		}
		
		// (s.tullis 2015-10-19 16:18) - PLID 67263- Load HL7 glasses RX
		LoadHL7Glasses();
		
		LoadContactLensList();

		if (m_PrescriptionWindowDisplayType == pwShowRxList )
		{
			m_btnCancel.ShowWindow(SW_HIDE);
			m_BtnSelect.ShowWindow(SW_HIDE);
			
			GetDlgItem(IDC_LBL_RX_GLASSES)->ShowWindow(SW_HIDE);
			
			m_BtnClose.ShowWindow(SW_SHOW );
		}
		else
		{
			m_btnCancel.ShowWindow(SW_SHOW);
			m_BtnSelect.ShowWindow(SW_SHOW);
			m_BtnClose.ShowWindow(SW_HIDE);

			//TES 4/16/2012 - PLID 49368 - If we're selecting from a list, disable the list that we don't want to return.
			if(m_bCalledForContactLens) {
				m_GlassesEMNPrescriptionList->ReadOnly = g_cvarTrue;
				// (s.dhole 2012-05-24 16:51) - PLID 50634 
				GetDlgItem(IDC_LBL_RX_GLASSES)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_LBL_RX_CONTACT_LENS)->ShowWindow(SW_SHOW );
			}
			else {
				m_ContactLensRxList->ReadOnly = g_cvarTrue;
				// (s.dhole 2012-05-24 16:51) - PLID 50634 
				GetDlgItem(IDC_LBL_RX_CONTACT_LENS)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_LBL_RX_GLASSES)->ShowWindow(SW_SHOW );
			}
		}
		
		// (s.dhole 2012-04-20 13:14) - PLID 49728  Enable button
		EnableDlgItem(ID_SELECT_PRESCRIPTIONS, !m_bIsReadOnly);
		if (m_bIsReadOnly==TRUE)
		{
			//EnableDlgItem(IDC_GLASSES_ORDER_EMN_LIST, FALSE);
			m_GlassesEMNPrescriptionList->ReadOnly = g_cvarTrue;
			// (s.dhole 2012-04-23 11:50) - PLID 49893 make Cotact lens selection readonly
			m_ContactLensRxList->ReadOnly = g_cvarTrue;
			//EnableDlgItem(IDC_CON_LEN_PREV_RX, FALSE);
		}

		
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//TES 4/17/2012 - PLID 49746 - Changed to take a pointer to the GlassesOrderLensDetails struct, which will then be stored in the datalist.
void CGlassesEMNPrescriptionList::LoadEMNRxRows(IRowSettingsPtr pRxRow ,GlassesOrderLensDetails *prxDetail  )
{
	try {
		pRxRow->PutValue(lvrxDate, g_cvarNull);
		pRxRow->PutValue(lvrxRxDate, g_cvarNull);
		pRxRow->PutValue(lvrxRxExpirationDate, g_cvarNull);
		if (prxDetail->strSphere !=""){
			pRxRow->PutValue(lvrxPrescriptionSphere,_bstr_t(AsPrescriptionNumber(prxDetail->strSphere,pnfSignedFloat)));
		}
		//TES 4/18/2012 - PLID 49764 - Throughout this function, make sure and not leave any fields as VT_EMPTY
		else {
			pRxRow->PutValue(lvrxPrescriptionSphere, g_cvarNull);
		}
		if(prxDetail->strCylinder != "") {
			pRxRow->PutValue(lvrxCylinder,_bstr_t(AsPrescriptionNumber(prxDetail->strCylinder,pnfSignedFloat)));
		}
		else {
			pRxRow->PutValue(lvrxCylinder, g_cvarNull);
		}
		if(prxDetail->strAxis != "") {
			pRxRow->PutValue(lvrxCylinderAxis,_bstr_t( prxDetail->strAxis));
		}
		else {
			pRxRow->PutValue(lvrxCylinderAxis, g_cvarNull);
		}
		if(prxDetail->strAddition != "") {
			pRxRow->PutValue(lvrxAdditionValue,_bstr_t(AsPrescriptionNumber(prxDetail->strAddition,pnfUnsignedFloat)));
		}
		else {
			pRxRow->PutValue(lvrxAdditionValue, g_cvarNull);
		}
		if(prxDetail->strPrism != "") {
			pRxRow->PutValue(lvrxPrism1,_bstr_t(AsPrescriptionNumber(prxDetail->strPrism,pnfUnsignedFloat)));
		}
		else {
			pRxRow->PutValue(lvrxPrism1, g_cvarNull);
		}
		if(prxDetail->strBase != "") {
			pRxRow->PutValue(lvrxBase1,_bstr_t(AsPrescriptionNumber(prxDetail->strBase,pnfBase)));
		}
		else {
			pRxRow->PutValue(lvrxBase1, g_cvarNull);
		}
		if (prxDetail->strDistPD !="") {
			pRxRow->PutValue(lvrxFarHalfPd,_bstr_t(AsPrescriptionNumber(prxDetail->strDistPD,pnfNaturalFloat)));
		}
		else {
			pRxRow->PutValue(lvrxFarHalfPd, g_cvarNull);
		}
		if(prxDetail->strNearPD != "") {
			pRxRow->PutValue(lvrxNearHalfPd,_bstr_t(AsPrescriptionNumber(prxDetail->strNearPD,pnfNaturalFloat)));
		}
		else {
			pRxRow->PutValue(lvrxNearHalfPd, g_cvarNull);
		}
		if(prxDetail->strHeight != "") {
			pRxRow->PutValue(lvrxSegHeight,_bstr_t(AsPrescriptionNumber(prxDetail->strHeight,pnfNaturalFloat)));
		}
		else {
			pRxRow->PutValue(lvrxSegHeight, g_cvarNull);
		}
		//TES 4/17/2012 - PLID 49746 - Store the pointer
		pRxRow->PutValue(lvrxObjPtr, (long)prxDetail);
	}NxCatchAll(__FUNCTION__);
}

//TES 4/16/2012 - PLID 49369 - Added equivalent function for contact lens prescriptions
//TES 4/17/2012 - PLID 49746 - Changed to take a pointer to the ContactLensOrderLensDetails struct, which will then be stored in the datalist.
void CGlassesEMNPrescriptionList::LoadEMNRxRows(IRowSettingsPtr pRxRow , ContactLensOrderLensDetails *prxDetail )
{
	try {
		pRxRow->PutValue(lvrxDate, g_cvarNull);
		pRxRow->PutValue(lvrxRxDate, g_cvarNull);
		pRxRow->PutValue(lvrxRxExpirationDate, g_cvarNull);
		if (prxDetail->strSphere !=""){
			pRxRow->PutValue(PrevConLensRxList::PrescriptionSphere,_bstr_t(AsPrescriptionNumber(prxDetail->strSphere,pnfSignedFloat)));
		}
		//TES 4/18/2012 - PLID 49764 - Throughout this function, make sure and not leave any fields as VT_EMPTY
		else {
			pRxRow->PutValue(PrevConLensRxList::PrescriptionSphere, g_cvarNull);
		}
		if(prxDetail->strCylinder != "") {
			pRxRow->PutValue(PrevConLensRxList::Cylinder,_bstr_t(AsPrescriptionNumber(prxDetail->strCylinder,pnfSignedFloat)));
		}
		else {
			pRxRow->PutValue(PrevConLensRxList::Cylinder, g_cvarNull);
		}
		if(prxDetail->strAxis != "") {
			pRxRow->PutValue(PrevConLensRxList::CylinderAxis,_bstr_t( prxDetail->strAxis));
		}
		else {
			pRxRow->PutValue(PrevConLensRxList::CylinderAxis, g_cvarNull);
		}
		if(prxDetail->strAddition != "") {
			pRxRow->PutValue(PrevConLensRxList::AdditionValue,_bstr_t(AsPrescriptionNumber(prxDetail->strAddition,pnfUnsignedFloat)));
		}
		else {
			pRxRow->PutValue(PrevConLensRxList::AdditionValue, g_cvarNull);
		}
		if(prxDetail->strBC != "") {
			pRxRow->PutValue(PrevConLensRxList::BC,_bstr_t(AsPrescriptionNumber(prxDetail->strBC,pnfUnsignedFloat)));
		}
		else {
			pRxRow->PutValue(PrevConLensRxList::BC, g_cvarNull);
		}
		if(prxDetail->strDiameter != "") {
			pRxRow->PutValue(PrevConLensRxList::Diameter, _bstr_t(AsPrescriptionNumber(prxDetail->strDiameter,pnfUnsignedFloat)));
		}
		else {
			pRxRow->PutValue(PrevConLensRxList::Diameter, g_cvarNull);
		}

		// (j.dinatale 2013-03-20 14:23) - PLID 55766 - get the manufacturer, place it in the doc ins column
		if(!prxDetail->strManufacturer.IsEmpty()){
			CString strDocIns;
			strDocIns.Format("Manufacturer: %s", prxDetail->strManufacturer);
			pRxRow->PutValue(PrevConLensRxList::DocIns, _bstr_t(strDocIns));
		}else{
			pRxRow->PutValue(PrevConLensRxList::DocIns, g_cvarNull);
		}

		//TES 4/17/2012 - PLID 49746 - Store the pointer
		pRxRow->PutValue(PrevConLensRxList::ObjPtr, (long)prxDetail);
		
	}NxCatchAll(__FUNCTION__);
}

void CGlassesEMNPrescriptionList::LoadRxRows(IRowSettingsPtr pRxRow ,_RecordsetPtr rsRx  )
{ 
try {	
	//pRxRow->PutValue(lvrxRowID, AdoFldLong(rsRx, "RowID"))	;
	pRxRow->PutValue(lvrxEmnID,long(0));
	long nLensRxID =AdoFldLong(rsRx,"LensRxID",-1);
	
	pRxRow->PutValue(lvrxOrderID,(long)AdoFldLong(rsRx,"GlassesOrderID",-1));
	pRxRow->PutValue(lvrxLensRxID,nLensRxID );
	if (AdoFldLong(rsRx,"PerentID",-1)==-1) {
		pRxRow->PutValue(lvrxLensDetailRxid, g_cvarNull);
		if (m_bAllowOrder==FALSE){
			pRxRow->PutValue(lvrxRxAction ,_bstr_t("Click to Open Order"));
		}
	}
	else{
		pRxRow->PutValue(lvrxLensDetailRxid,long(AdoFldLong(rsRx,"PerentID")));
	}

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	COleDateTime dt = AdoFldDateTime(rsRx, "GlassesOrderDate", dtInvalid);
	if (COleDateTime::valid == dt.GetStatus()) {
		pRxRow->PutValue(lvrxDate, _variant_t(dt, VT_DATE));
	} else {
		pRxRow->PutValue(lvrxDate, g_cvarNull);
	}
	COleDateTime dtRx = AdoFldDateTime(rsRx, "RxDate", dtInvalid);
	if (COleDateTime::valid == dtRx.GetStatus()) {
		pRxRow->PutValue(lvrxRxDate, _variant_t(dtRx, VT_DATE));
	} else {
		pRxRow->PutValue(lvrxRxDate, g_cvarNull);
	}

	if (COleDateTime::valid == dtRx.GetStatus()) {
		pRxRow->PutValue(lvrxSortDate,_variant_t(dtRx, VT_DATE));
	}
	else{
		// (s.tullis 2015-10-19 16:18) - PLID 67263- Make sure we don't load bad dates
		//if we missing rx date than use order date
		if (AdoFldDateTime(rsRx, "OrderDate", dtInvalid).GetStatus() == COleDateTime::valid){
			pRxRow->PutValue(lvrxSortDate, _variant_t(AdoFldDateTime(rsRx, "OrderDate", dtInvalid), VT_DATE));
		}
	}

	COleDateTime dtRxExpirationDate = AdoFldDateTime(rsRx, "RxExpirationDate", dtInvalid);
	if (COleDateTime::valid == dtRxExpirationDate.GetStatus()) {
		pRxRow->PutValue(lvrxRxExpirationDate, _variant_t(dtRxExpirationDate, VT_DATE));
	} else {
		pRxRow->PutValue(lvrxRxExpirationDate, g_cvarNull);
	}
	
	if (AdoFldString (rsRx, "eye_site","") !=""){
		pRxRow->PutValue(lvrxeye_site,_bstr_t(AdoFldString (rsRx, "eye_site")));
	}
	//TES 4/18/2012 - PLID 49764 - Throughout this function, make sure and not leave any fields as VT_EMPTY
	else {
		pRxRow->PutValue(lvrxeye_site, g_cvarNull);
	}
	_variant_t varSphere =  rsRx->Fields->GetItem("PrescriptionSphere")->Value; 
	if(varSphere.vt != VT_NULL) {
		pRxRow->PutValue(lvrxPrescriptionSphere,_bstr_t(AsPrescriptionNumber(AsString(varSphere),pnfSignedFloat)));
	}
	else {
		pRxRow->PutValue(lvrxPrescriptionSphere, g_cvarNull);
	}

	_variant_t varCylinder = rsRx->Fields->GetItem("CylinderValue")->Value;
	if(varCylinder.vt != VT_NULL) {
		pRxRow->PutValue(lvrxCylinder,_bstr_t(AsPrescriptionNumber(AsString(varCylinder),pnfSignedFloat)));
	}
	else {
		pRxRow->PutValue(lvrxCylinder, g_cvarNull);
	}

	_variant_t varAxis = rsRx->Fields->GetItem("CylinderAxis")->Value;
	if(varAxis.vt != VT_NULL) {
		pRxRow->PutValue(lvrxCylinderAxis,_bstr_t( AsString(varAxis)));
	}
	else {
		pRxRow->PutValue(lvrxCylinderAxis, g_cvarNull);
	}

	_variant_t varAddition = rsRx->Fields->GetItem("AdditionValue")->Value;
	if(varAddition.vt != VT_NULL) {
		pRxRow->PutValue(lvrxAdditionValue,_bstr_t(AsPrescriptionNumber(AsString(varAddition),pnfUnsignedFloat)));
	}
	else {
		pRxRow->PutValue(lvrxAdditionValue, g_cvarNull);
	}

	_variant_t varPrism = rsRx->Fields->GetItem("PrismValue")->Value;
	if(varPrism.vt != VT_NULL) {
		pRxRow->PutValue(lvrxPrism1,_bstr_t(AsPrescriptionNumber(AsString(varPrism),pnfUnsignedFloat)));
	}
	else {
		pRxRow->PutValue(lvrxPrism1, g_cvarNull);
	}

	_variant_t varAxisVal = rsRx->Fields->GetItem("PrismAxis")->Value;
	if(varAxisVal.vt == VT_NULL) {
		pRxRow->PutValue(lvrxBase1,_bstr_t(AdoFldString(rsRx, "PrismAxisStr", "")));
	}
	else {
		pRxRow->PutValue(lvrxBase1,_bstr_t(AsString(varAxisVal)));
	}
		
	_variant_t varSecdPrism = rsRx->Fields->GetItem("SecondaryPrismValue")->Value;
	if(varSecdPrism .vt != VT_NULL) {
		pRxRow->PutValue(lvrxPrism2,_bstr_t(AsPrescriptionNumber(AsString(varSecdPrism ),pnfUnsignedFloat)));
	}
	else {
		pRxRow->PutValue(lvrxPrism2, g_cvarNull);
	}

	_variant_t varAxisVal2 = rsRx->Fields->GetItem("PrismAxis2")->Value;
	if(varAxisVal2.vt == VT_NULL) {
		pRxRow->PutValue(lvrxBase2,_bstr_t(AdoFldString(rsRx, "PrismAxisStr2", "")));
	}
	else {
		pRxRow->PutValue(lvrxBase2,_bstr_t(AsString(varAxisVal2)));
	}
		
	_variant_t varDistPD  = rsRx->Fields->GetItem("FarHalfPd")->Value;
	if(varDistPD.vt != VT_NULL) {
		pRxRow->PutValue(lvrxFarHalfPd,_bstr_t(AsPrescriptionNumber(AsString(varDistPD),pnfNaturalFloat)));
	}
	else {
		pRxRow->PutValue(lvrxFarHalfPd, g_cvarNull);
	}
	_variant_t varNearPD = rsRx->Fields->GetItem("NearHalfPd")->Value;
	if(varNearPD.vt != VT_NULL) {
		pRxRow->PutValue(lvrxNearHalfPd,_bstr_t(AsPrescriptionNumber(AsString(varNearPD),pnfNaturalFloat)));
	}
	else {
		pRxRow->PutValue(lvrxNearHalfPd, g_cvarNull);
	}
	
	_variant_t varHeight = rsRx->Fields->GetItem("SegHeight")->Value;
	if(varHeight.vt != VT_NULL) {
		pRxRow->PutValue(lvrxSegHeight,_bstr_t(AsPrescriptionNumber(AsString(varHeight),pnfNaturalFloat)));
	}
	else {
		pRxRow->PutValue(lvrxSegHeight, g_cvarNull);
	}	

	pRxRow->PutValue(lvrxObjPtr, g_cvarNull);

	/*r.wilson (6/18/2012) PLID 50915 - Add providerID to the row
		Note: The ProviderID column is converted to a  string because its column on the datalist is a string were 
		      it is possible to have multiple providerID's each serated by a ;   (semicolon)

	 */
	_variant_t varProviderID = rsRx->Fields->GetItem("ProviderID")->Value;
	if(varProviderID.vt != VT_NULL){
		long nTmpProviderID = AdoFldLong(rsRx,"ProviderID");
		CString strTmpProviderID = AsString(AdoFldLong(rsRx,"ProviderID",-1));		
		pRxRow->PutValue(lvrxProviderID, _bstr_t(strTmpProviderID));		
	}
	else {
		pRxRow->PutValue(lvrxProviderID, g_cvarNull);
	}

	//r.wilson (6/18/2012) PLID 50915 - Put Provider's name into it's correct column on the current row
	variant_t varProviderName = rsRx->Fields->GetItem("ProviderName")->Value;
	if(varProviderName.vt == VT_NULL) {
		pRxRow->PutValue(lvrxProviderName,_bstr_t(AdoFldString(rsRx, "ProviderName", "")));
	}
	else {
		pRxRow->PutValue(lvrxProviderName,_bstr_t(AsString(varProviderName)));
	}

	// (s.tullis 2015-10-19 16:18) - PLID 67263- Added HL7ID
	long nHl7ID = AdoFldLong(rsRx, "HL7ID", -1);
	pRxRow->PutValue(lvrxHL7ID, nHl7ID);


}NxCatchAll(__FUNCTION__);
}


void CGlassesEMNPrescriptionList::OnBnClickedCancel()
{
try {
	CNxDialog::OnCancel(); 
	}NxCatchAll(__FUNCTION__);
}

void CGlassesEMNPrescriptionList::OnBnClickedSelectPrescriptions()
{
	try {
		//TES 4/16/2012 - PLID 49368 - If we were called from a contact lens form, check the contact lens list.
		if(m_bCalledForContactLens) {
			IRowSettingsPtr pRow = m_ContactLensRxList->CurSel;
			IRowSettingsPtr pChildRow,pParentRow;
			if(pRow != NULL  ) 
			{
				//long nRowID=	VarLong( pRow->GetValue(lvrxRowID),-1);   
				// try to read parent 
				pRow = pRow->GetParentRow(); 	
				if (pRow == NULL){ 
					// current row is parent row
					pParentRow = m_ContactLensRxList->CurSel;
				}
				else{	
					// set parent row
					pParentRow=pRow ;
				}
				if (pParentRow != NULL)
				{
					// (s.dhole 2012-04-25 12:32) - PLID 49969 Added Emnid
					m_oContactLensRx.nEmnId = pParentRow->GetValue(PrevConLensRxList::EmnID);
					m_oContactLensRx.varRxDate  = pParentRow->GetValue(PrevConLensRxList::RxDate);
					m_oContactLensRx.varRxExpDate =  pParentRow->GetValue(PrevConLensRxList::RxExpirationDate);
					pChildRow = pParentRow ->GetFirstChildRow(); 
					if (pChildRow != NULL) {
						_variant_t vareye_site = pChildRow->GetValue(PrevConLensRxList::Rxeye_site); 
						if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
							SetLensRxDetail(pChildRow, m_oContactLensRx.clriOD);
						}
						else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
							SetLensRxDetail(pChildRow, m_oContactLensRx.clriOS);
						} 
					}
					pChildRow =  pChildRow ->GetNextRow (); 
					if (pChildRow != NULL) {
						_variant_t vareye_site =   pChildRow->GetValue(PrevConLensRxList::Rxeye_site); 
						if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
							SetLensRxDetail(pChildRow, m_oContactLensRx.clriOD);
						}
						else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
							SetLensRxDetail(pChildRow, m_oContactLensRx.clriOS);
						}
					}
					m_oContactLensRx.bRecordExists = TRUE;

				}
				CNxDialog::OnOK();
			}
			else {
				//TES 4/16/2012 - PLID 49732 - Let them know what they need to do.
				MsgBox("Please select a Contact Lens prescription, or select Cancel to return to the Contact Lens Order Form");
			}
		}
		else {
			IRowSettingsPtr pRow = m_GlassesEMNPrescriptionList->CurSel;
			IRowSettingsPtr pChildRow,pParentRow;
			if(pRow != NULL  ) 
			{
				//long nRowID=	VarLong( pRow->GetValue(lvrxRowID),-1);   
				// try to read parent 
				pRow = pRow->GetParentRow(); 	
				if (pRow == NULL){ 
					// current row is parent row
					pParentRow = m_GlassesEMNPrescriptionList->CurSel;
				}
				else{	
					// set parent row
					pParentRow=pRow ;
				}
				if (pParentRow != NULL)
				{
					m_oLensRx.strRxDate  = pParentRow->GetValue(lvrxRxDate)  ;
					if(m_oLensRx.strRxDate.vt == VT_EMPTY){
						m_oLensRx.strRxDate = _T("");
					}
					m_oLensRx.strRxExpDate =  pParentRow->GetValue(lvrxRxExpirationDate)  ;
					if(m_oLensRx.strRxDate.vt == VT_EMPTY){
						m_oLensRx.strRxDate = _T("");
					}
					// (s.dhole 2012-04-25 12:32) - PLID 49969 Added Emnid
					m_oLensRx.nEmnId =  pParentRow->GetValue(lvrxEmnID);
					pChildRow =  pParentRow ->GetFirstChildRow(); 
					if (pChildRow != NULL) {
						_variant_t vareye_site =   pChildRow->GetValue(lvrxeye_site); 
						if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
							SetLensRxDetail(pChildRow, m_oLensRx.LensRxOD);
						}
						else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
							SetLensRxDetail(pChildRow, m_oLensRx.LensRxOS);
						} 
					}
					pChildRow =  pChildRow ->GetNextRow (); 
					if (pChildRow != NULL) {
						_variant_t vareye_site =   pChildRow->GetValue(lvrxeye_site); 
						if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
							SetLensRxDetail(pChildRow, m_oLensRx.LensRxOD);
						}
						else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
							SetLensRxDetail(pChildRow, m_oLensRx.LensRxOS);
						}
					}
					m_oLensRx.ISRecordExist=TRUE;
				}
				CNxDialog::OnOK();
			}
			else {
				//TES 4/16/2012 - PLID 49732 - Let them know what they need to do.
				// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
				MsgBox("Please select a Prescription, or select Cancel to return to the Optical Order Form");
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CGlassesEMNPrescriptionList::SetLensRxDetail(IRowSettingsPtr pRow,OUT LensRxInfo &LensRxDetail )
{
	try
	{
		// added prism1, prism2 ,base1,base2 
		LensRxDetail.strSphere = AsString(pRow->GetValue(lvrxPrescriptionSphere));   
		LensRxDetail.strCylinder = AsString(pRow->GetValue(lvrxCylinder));   
		LensRxDetail.strAxis  = AsString(pRow->GetValue(lvrxCylinderAxis));   
		LensRxDetail.strAddition  = AsString(pRow->GetValue(lvrxAdditionValue));   
		LensRxDetail.strPrism1 = AsString(pRow->GetValue(lvrxPrism1));   
		LensRxDetail.strBase1  = AsString(pRow->GetValue(lvrxBase1));   
		LensRxDetail.strPrism2 = AsString(pRow->GetValue(lvrxPrism2));   
		LensRxDetail.strBase2  = AsString(pRow->GetValue(lvrxBase2));   
		LensRxDetail.strNearPD= AsString(pRow->GetValue(lvrxNearHalfPd ));   
		LensRxDetail.strDistPD  = AsString(pRow->GetValue(lvrxFarHalfPd));   
		LensRxDetail.strHeight= AsString(pRow->GetValue(lvrxSegHeight));
		//(r.wilson 5/14/2012) PLID 48952
		//LensRxDetail.strPrismAxisStr = AsString(pRow->GetValue());
		//TES 4/17/2012 - PLID 49746 - Copy the data from the stored pointer (which is about to be deleted)
		GlassesOrderLensDetails *pObj = (GlassesOrderLensDetails*)VarLong(pRow->GetValue(lvrxObjPtr),NULL);
		if(pObj) {
			LensRxDetail.pEmnInfo = new GlassesOrderLensDetails;
			*(LensRxDetail.pEmnInfo) = *pObj;
		}
	}NxCatchAll(__FUNCTION__);
}

//TES 4/16/2012 - PLID 49368 - Added equivalent function for contact lens prescriptions
void CGlassesEMNPrescriptionList::SetLensRxDetail(IRowSettingsPtr pRow, OUT ContactLensRxInfo &LensRxDetail )
{
	try
	{
		LensRxDetail.strSphere = AsString(pRow->GetValue(PrevConLensRxList::PrescriptionSphere));   
		LensRxDetail.strCylinder = AsString(pRow->GetValue(PrevConLensRxList::Cylinder));   
		LensRxDetail.strAxis  = AsString(pRow->GetValue(PrevConLensRxList::CylinderAxis));   
		LensRxDetail.strAddition  = AsString(pRow->GetValue(PrevConLensRxList::AdditionValue));   
		LensRxDetail.strBC = AsString(pRow->GetValue(PrevConLensRxList::BC));
		LensRxDetail.strDiameter = AsString(pRow->GetValue(PrevConLensRxList::Diameter));
		LensRxDetail.strDocIns = AsString(pRow->GetValue(PrevConLensRxList::DocIns));	// (j.dinatale 2013-03-21 12:41) - PLID 55752
		LensRxDetail.strNote = AsString(pRow->GetValue(PrevConLensRxList::Note)); //TES 12/2/2015 - PLID 67671 
		//TES 4/17/2012 - PLID 49746 - Copy the data from the stored pointer (which is about to be deleted)
		ContactLensOrderLensDetails *pObj = (ContactLensOrderLensDetails*)VarLong(pRow->GetValue(PrevConLensRxList::ObjPtr),NULL);
		if(pObj) {
			LensRxDetail.pEmnInfo = new ContactLensOrderLensDetails;
			*(LensRxDetail.pEmnInfo) = *pObj;
		}
	}NxCatchAll(__FUNCTION__);
}

void CGlassesEMNPrescriptionList::OnBnClickedCloseGlassesEmnPrescriptionsClose()
{
	try {
		CNxDialog::OnCancel(); 
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-03-09 16:42) - PLID 48724 - Can I haz previous rx?
void CGlassesEMNPrescriptionList::LoadContactLensList()
{
	try{
		CSqlFragment sqlContacts;
		// (j.dinatale 2012-05-15 16:13) - PLID 50346 - select the note fields for the OS and OD
		// (j.dinatale 2012-05-16 15:51) - PLID 50346 - get rid of an rows that have no rx info what so ever
		//r.wilson (6/13/2012) PLID 50915 - Add the field ProviderName
		// (j.dinatale 2013-03-19 17:01) - PLID 55766 - Added the Doc Instruction field
		// (s.tullis 2015-10-19 16:18) - PLID 67263 - Refactored
		sqlContacts.Create(
			R"(SELECT GlassesOrderT.ID AS OrderID, GlassesOrderT.PersonID AS PatientID, GlassesOrderT.Date AS OrderDate, 
GlassesOrderT.LensRxID AS LensRxID, 
LensRxT.RxDate AS ExamDate, LensRxT.RxExpirationDate AS ExpirationDate, LensRxT.RxIssueDate AS IssueDate, 
OSLensRxDetailsQ.ID AS LeftLensRxDetailID, OSLensRxDetailsQ.Sphere AS OSSphere, OSLensRxDetailsQ.Cyl AS OSCyl, OSLensRxDetailsQ.Axis AS OSAxis, 
OSLensRxDetailsQ.Addition AS OSAdd, OSLensRxDetailsQ.BC AS OSBC, OSLensRxDetailsQ.Diameter AS OSDiam, OSLensRxDetailsQ.Note AS OSNote, OSLensRxDetailsQ.DocIns AS OSDocIns, 
ODLensRxDetailsQ.ID AS RightLensRxDetailID, ODLensRxDetailsQ.Sphere AS ODSphere, ODLensRxDetailsQ.Cyl AS ODCyl, ODLensRxDetailsQ.Axis AS ODAxis, 
ODLensRxDetailsQ.Addition AS ODAdd, ODLensRxDetailsQ.BC AS ODBC, ODLensRxDetailsQ.Diameter AS ODDiam, ODLensRxDetailsQ.Note AS ODNote, ODLensRxDetailsQ.DocIns AS ODDocIns, 
GlassesOrderT.ProviderID AS ProviderID, 
PersonProvider.Last + ', ' + PersonProvider.First AS ProviderName,
HL7MessageID as HL7ID  
FROM 
GlassesOrderT 
LEFT JOIN LensRxT ON GlassesOrderT.LensRxID = LensRxT.ID 
LEFT JOIN ( 
SELECT ID, PrescriptionSphere AS Sphere, CylinderValue AS Cyl, CylinderAxis AS Axis, AdditionValue AS Addition, 
BC, Diameter, Color, Quantity, Note, DocIns FROM LensDetailRxT 
) OSLensRxDetailsQ ON LensRxT.LeftLensDetailRxID = OSLensRxDetailsQ.ID 
LEFT JOIN ( 
SELECT ID, PrescriptionSphere AS Sphere, CylinderValue AS Cyl, CylinderAxis AS Axis, AdditionValue AS Addition, 
BC, Diameter, Color, Quantity, Note, DocIns FROM LensDetailRxT 
) ODLensRxDetailsQ ON LensRxT.RightLensDetailRxID = ODLensRxDetailsQ.ID 
  LEFT JOIN PersonT AS PersonProvider ON PersonProvider.ID = GlassesOrderT.ProviderID  
WHERE IsDelete = 0 AND (OSLensRxDetailsQ.ID IS NOT NULL OR ODLensRxDetailsQ.ID IS NOT NULL) AND GlassesOrderT.PersonID = {INT} 
AND GlassesOrderT.GlassesOrderType = {INT})", m_nPatientID, vwotContactLensPatient
		);

		_RecordsetPtr rsContactRx = CreateParamRecordset(sqlContacts);
		BOOL bSetDocInsWidth = FALSE;
		while(!rsContactRx->eof){
			// (j.dinatale 2012-05-15 16:14) - PLID 50346 - load the note field
			// (j.dinatale 2013-03-19 17:01) - PLID 55766 - load the doctor's instruction field
			// (s.tullis 2015-11-06 09:03) - PLID 67263 - Moved to a fuction
			LoadContactListRow(rsContactRx,bSetDocInsWidth);
			rsContactRx->MoveNext();
		}
		// (s.tullis 2015-10-19 16:18) - PLID 67263 - Load HL7 Contacts
		LoadHL7Contacts(bSetDocInsWidth);

		//TES 4/16/2012 - PLID 49369 - Now load any EMNs for this patient with contact lens data associated.
		_RecordsetPtr rsEmnRx = CreateParamRecordset( " SELECT EMRMasterT.PatientID, EMRMasterT.ID AS EMNID, EmrMasterT.ModifiedDate AS EmnModifiedDate "
			" FROM EMRMasterT  "
			" INNER JOIN (SELECT EmrDetailsT.EmrID FROM EmrDetailsT INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID   "
			" INNER JOIN EMRTopicsT ON EMRTopicsT.EMRID= EmrDetailsT.EmrID   "
			" WHERE EmrInfoT.HasContactLensData = 1 AND EmrDetailsT.Deleted=0  "
			" AND EMRTopicsT.Deleted=0    GROUP BY EmrDetailsT.EmrID ) AS OrderableEmnsQ ON EmrMasterT.ID = OrderableEmnsQ.EmrID   "
			" WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.PatientID ={INT} ", m_nPatientID);
		long  nEmnLensID=-1;
		long nEmnID =-1;
		//long nRowID =-1;
		while(!rsEmnRx->eof) {
			nEmnID =AdoFldLong(rsEmnRx, "EMNID",-1);
			if (nEmnID >-1)
			{
				COleDateTime dtEmnModifiedDate = AdoFldDateTime(rsEmnRx, "EmnModifiedDate");
				CEMN* pEmn = new CEMN(NULL);
				pEmn->LoadFromEmnID(nEmnID);
				ContactLensOrder clo;
				// to iusee existing function passint this string
				CString strIgnoredData;
				if(pEmn->GetContactLensOrderData(clo, strIgnoredData)) {
					//load header
					
					IRowSettingsPtr pParentRow = m_ContactLensRxList->GetNewRow();					
										
					pParentRow->PutValue(PrevConLensRxList::EmnID,nEmnID);
					pParentRow->PutValue(PrevConLensRxList::LensRxID,nEmnLensID );
					pParentRow->PutValue(PrevConLensRxList::LensDetailRxID, g_cvarNull);
					pParentRow->PutValue(PrevConLensRxList::Date, _variant_t(pEmn->GetEMNDate(), VT_DATE));
					pParentRow->PutValue(PrevConLensRxList::RxDate, _variant_t(pEmn->GetEMNDate(), VT_DATE));
					pParentRow->PutValue(PrevConLensRxList::RxExpirationDate, g_cvarNull);
					pParentRow->PutValue(PrevConLensRxList::SortDate,_variant_t(pEmn->GetEMNDate(), VT_DATE));
					m_ContactLensRxList->AddRowSorted(pParentRow,NULL); 
					
					// (s.dhole 2012-04-25 12:35) - PLID 47395 set text
					if (m_PrescriptionWindowDisplayType==pwShowRxList){
						if  (nEmnID>0 && m_bAllowEmn==TRUE){
							pParentRow->PutValue(PrevConLensRxList::RxAction , "Click to Open EMN");
						}
					}
					else if (m_PrescriptionWindowDisplayType==pwAllowRxSelection){ 
						if  (nEmnID>0 ){
							pParentRow->PutValue(PrevConLensRxList::RxAction , "EMN");
						}
					}

					// Load Detail OD
					IRowSettingsPtr pLensRow = m_ContactLensRxList->GetNewRow();
					//pRxRow->PutValue(lvrxRowID, (long)nRowID )	;
					pLensRow->PutValue(PrevConLensRxList::EmnID,nEmnID);
					pLensRow->PutValue(PrevConLensRxList::LensRxID,nEmnLensID );
					pLensRow->PutValue(PrevConLensRxList::LensDetailRxID, nEmnLensID);
			
					pLensRow->PutValue(PrevConLensRxList::Rxeye_site,_bstr_t("OD"));
					//TES 4/17/2012 - PLID 49746 - Copy the lens detail information into a pointer that can be stored in the datalist
					ContactLensOrderLensDetails *pClold = new ContactLensOrderLensDetails;
					*pClold = clo.clolOD;
					CGlassesEMNPrescriptionList::LoadEMNRxRows(pLensRow, pClold);
					pLensRow->PutValue(PrevConLensRxList::SortDate,_variant_t(pEmn->GetEMNDate(), VT_DATE));														

					CString strProviderNames;
					CString strProviderIDs;

					for(int i = 0; i < pEmn->GetProviderCount() ; i++)
					{
						if(i > 0){
							strProviderNames += ", ";
							strProviderIDs += "; ";
						}
						EMNProvider *o_EmnProv = pEmn->GetProvider(i);
						strProviderNames += o_EmnProv->strName;
						CString strTmp;
						strTmp.Format("%li",o_EmnProv->nID);
						strProviderIDs += strTmp;
					}

					//r.wilson (6/13/2012) PLID 50915 - ProviderID(s)
					pParentRow->PutValue(PrevConLensRxList::ProviderID,_bstr_t(strProviderIDs));	

					//r.wilson (6/13/2012) PLID 50915 - ProviderName(s)										
					pParentRow->PutValue(PrevConLensRxList::ProviderName,_bstr_t(strProviderNames));

					m_ContactLensRxList->AddRowSorted(pLensRow,pParentRow); 
					pParentRow->PutExpanded (VARIANT_TRUE);

					// Load Detail OS
					pLensRow = m_ContactLensRxList->GetNewRow();
					//pRxRow->PutValue(lvrxRowID, (long)nRowID )	;
					pLensRow->PutValue(PrevConLensRxList::EmnID,nEmnID);
					pLensRow->PutValue(PrevConLensRxList::LensRxID,nEmnLensID );
					pLensRow->PutValue(PrevConLensRxList::LensDetailRxID, nEmnLensID);
					pLensRow->PutValue(PrevConLensRxList::Rxeye_site,_bstr_t("OS"));

					/*
					_variant_t varProviderID = pRow->Fields->GetItem("ProviderID")->Value;
					if(varProvider.vt != VT_NULL)
					{
						pRxRow->PutValue(PrevConLensRxList::ProviderID,long(AdoFldLong(rsRx,"ProviderID")));
						//pRxRow->PutValue(lvrxLensDetailRxid,long(AdoFldLong(rsRx,"PerentID")));
					}*/
					

					//TES 4/17/2012 - PLID 49746 - Copy the lens detail information into a pointer that can be stored in the datalist
					pClold = new ContactLensOrderLensDetails;
					*pClold = clo.clolOS;
					CGlassesEMNPrescriptionList::LoadEMNRxRows (pLensRow, pClold);
					pLensRow->PutValue(PrevConLensRxList::SortDate,_variant_t(pEmn->GetEMNDate(), VT_DATE));					
					m_ContactLensRxList->AddRowSorted(pLensRow,pParentRow); 
					pParentRow->PutExpanded (VARIANT_TRUE);

					// (j.dinatale 2013-03-19 17:03) - PLID 55766 - if either lens has manufacturer info, set the width of the column
					if(!clo.clolOD.strManufacturer.IsEmpty() || !clo.clolOS.strManufacturer.IsEmpty()){
						if(!bSetDocInsWidth){
							NXDATALIST2Lib::IColumnSettingsPtr pCol = m_ContactLensRxList->GetColumn(PrevConLensRxList::DocIns);
							if(pCol){
								pCol->StoredWidth = 150;
								bSetDocInsWidth = true;
							}
						}
					}
				}
				delete pEmn;
			}
			rsEmnRx->MoveNext();
		}
	}NxCatchAll(__FUNCTION__);
}

void CGlassesEMNPrescriptionList::OnDestroy()
{
	try {
		CNxDialog::OnDestroy();

		//TES 4/17/2012 - PLID 49746 - Go through all the pointers in the datalist and free up their memory (any being returned to the client
		// will already have had their values copied to a new object).
		IRowSettingsPtr pParentRow = m_GlassesEMNPrescriptionList->GetFirstRow();
		while(pParentRow) {
			IRowSettingsPtr pChild = pParentRow->GetFirstChildRow();
			while(pChild) {
				GlassesOrderLensDetails *pGold = (GlassesOrderLensDetails*)VarLong(pChild->GetValue(lvrxObjPtr),NULL);
				if(pGold) {
					delete pGold;
				}
				pChild = pChild->GetNextRow();
			}
			pParentRow = pParentRow->GetNextRow();
		}
		pParentRow = m_ContactLensRxList->GetFirstRow();
		while(pParentRow) {
			IRowSettingsPtr pChild = pParentRow->GetFirstChildRow();
			while(pChild) {
				ContactLensOrderLensDetails *pClold = (ContactLensOrderLensDetails*)VarLong(pChild->GetValue(PrevConLensRxList::ObjPtr), NULL);
				if(pClold) {
					delete pClold;
				}
				pChild = pChild->GetNextRow();
			}
			pParentRow = pParentRow->GetNextRow();
		}
		if (m_pEMRPreviewPopupDlg) {
			m_pEMRPreviewPopupDlg->DestroyWindow();
			delete m_pEMRPreviewPopupDlg;
			m_pEMRPreviewPopupDlg = NULL;
		}
	
	}NxCatchAll(__FUNCTION__);
}


void CGlassesEMNPrescriptionList::OnPreviewEmn(long nEmnID)
{
	try {
				
		//- Create a Preview Popup dialog for our EMN
		if(nEmnID  == -1) {
			//- This button should have been disabled!
			ASSERT(FALSE);
			return;
		}

		//- Make sure we have permission
		if(!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
			return;
		}

		//- Check if the EMN has been deleted.
		_RecordsetPtr rsDeleteInfo = CreateParamRecordset(
			"SELECT Deleted, DeletedBy, DeleteDate, EmnTabChartID, ModifiedDate \r\n"
			"FROM EmrMasterT \r\n"
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID \r\n"
			"WHERE ID = {INT} \r\n"
			, nEmnID );
		if(rsDeleteInfo->eof) {
			return;
		}
		if(AdoFldBool(rsDeleteInfo, "Deleted")) {
			MsgBox("This EMN was deleted by %s on %s, it cannot be previewed", AdoFldString(rsDeleteInfo, "DeletedBy"), 
				FormatDateTimeForInterface(AdoFldDateTime(rsDeleteInfo, "DeleteDate")));
			return;
		}

		// - Don't let them preview if they don't have permission to this EMN's chart.
		long nChartID = AdoFldLong(rsDeleteInfo, "EmnTabChartID", -1);
		if(nChartID != -1 && !CheckCurrentUserPermissions(bioEmrCharts, sptView, TRUE, nChartID)) {
			return;
		}

		COleDateTime dtEmnModifiedDate = AdoFldDateTime(rsDeleteInfo, "ModifiedDate");
		if (m_pEMRPreviewPopupDlg == NULL) {
			//- Create our dialog
			m_pEMRPreviewPopupDlg = new CEMRPreviewPopupDlg(this);
			m_pEMRPreviewPopupDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

			//- Remember the size for previewing from Glasses Orders
			m_pEMRPreviewPopupDlg->RestoreSize("GlassesOrder");
		}
		
		//- Now give it our patient ID and EMN ID
		// (z.manning 2012-09-10 14:17) - PLID 52543 - Use the new EmnPreviewPopup object
		EmnPreviewPopup emn(nEmnID, dtEmnModifiedDate);
		m_pEMRPreviewPopupDlg->SetPatientID(m_nPatientID, emn);
		//- Now preview the first (only) EMN that we gave it.
		m_pEMRPreviewPopupDlg->PreviewEMN(emn, 0);

		//- Now show the dialog, if we haven't already
		if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
			m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-04-25 12:35) - PLID 47395 
void CGlassesEMNPrescriptionList::LeftClickGlassesPrescriptionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
try {
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) 
		return;
		
	if (m_PrescriptionWindowDisplayType==pwShowRxList 
		&& nCol == lvrxRxAction ) {
		long nEmnID=-1;
		long nOrderID=-1;
		
		nEmnID =  pRow->GetValue(lvrxEmnID);
		nOrderID =  pRow->GetValue(lvrxOrderID);
		if (nEmnID>0){
			OnPreviewEmn(nEmnID);
		}
		else if (nOrderID>0){
			GetMainFrame()->OpenVisionWebOrderDlg(this, nOrderID);
		}
		else{
		// nothing
		}
	}

} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-04-25 12:35) - PLID 47395 
void CGlassesEMNPrescriptionList::LeftClickContactPrescriptionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
try {
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) 
		return;
	if (m_PrescriptionWindowDisplayType==pwShowRxList && 
		nCol == PrevConLensRxList::RxAction ){
			long nEmnID=-1;
			long nOrderID=-1;

			// try to read parent 
			nEmnID =  pRow->GetValue(PrevConLensRxList::EmnID);
			nOrderID =  pRow->GetValue(PrevConLensRxList::OrderID);
			if (nEmnID>0){
				OnPreviewEmn(nEmnID);
			}

			else if (nOrderID>0){
				GetMainFrame()->OpenContactLensOrderForm(this, nOrderID);
			}
			else{
			// nothing
			}
		}			

} NxCatchAll(__FUNCTION__);
}

BOOLEAN CGlassesEMNPrescriptionList::PrintRecord(BOOL IsPreview, BOOL bPrintGlassesRx, BOOL bPrintContactLensRx)
{	

	IRowSettingsPtr pRowParent = m_GlassesEMNPrescriptionList->GetCurSel();
	IRowSettingsPtr pRowParentCL = m_ContactLensRxList->GetCurSel();

	if(pRowParent == NULL && bPrintGlassesRx == TRUE){
		AfxMessageBox("No glasses prescription selected.");
		return FALSE;
	}

	if(pRowParentCL == NULL && bPrintContactLensRx == TRUE){
		AfxMessageBox("No contact lens prescription selected.");
		return FALSE;
	}
	
	if(bPrintGlassesRx == FALSE)
	{
		
	}

	if(bPrintContactLensRx == FALSE)
	{
		
	}

	if(pRowParent != NULL)
	{
		if(pRowParent->GetParentRow() != NULL)
		{
			pRowParent = pRowParent->GetParentRow();
		}
	}

	if(pRowParentCL != NULL)
	{
		if(pRowParentCL->GetParentRow() != NULL)
		{
			pRowParentCL = pRowParentCL->GetParentRow();
		}
	}

	long PersonID  = m_nPatientID;
	
	COleDateTime RxDateTime;
	
	long UserDefinedID = -1;
	CString PatientFirstName;
	CString PatientMiddleName;
	CString PatientLastName;
	CString PatientAddress1;
	CString PatientAddress2;
	CString PatientCity;
	CString PatientState;
	CString PatientZip;
	int PatientGender = -1;
	CString PatientHomePhone;
	CString PatientWorkPhone;
	CString PatientExtension;
	CString PatientCellPhone;
	COleDateTime PatientBirthDate;
	CString PatientSocialSecurity;
	
	long CLProviderID = -1;
	CString ProviderFirstName;
	CString ProviderMiddleName;
	CString ProviderLastName;
	CString ProviderPrefix;
	CString ProviderTitle;
	CString Location = "";
	CString LocationAddress1 = "";
	CString LocationAddress2 = "";
	CString LocationCity = "";
	CString LocationState = "";
	CString LocationZip = "";
	CString LocationPhone = "";
	CString LocationFax = "";

	// The CL prefix stands for Contact Lens
	CString CLProviderFirstName;
	CString CLProviderMiddleName;
	CString CLProviderLastName;
	CString CLProviderPrefix;
	CString CLProviderTitle;
	CString CLLocation;
	CString CLLocationAddress1;
	CString CLLocationAddress2;
	CString CLLocationCity;
	CString CLLocationState;
	CString CLLocationZip;
	CString CLLocationPhone;
	CString CLLocationFax;

	IRowSettingsPtr pRowGlasses = m_GlassesEMNPrescriptionList->GetCurSel();
	IRowSettingsPtr pRowContactLense = m_ContactLensRxList->GetCurSel();

	CString strPatientAndProviderQry = 
					" SELECT PersonT.ID ,UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Gender, HomePhone, WorkPhone, Extension, CellPhone, BirthDate, SocialSecurity"
					" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "	
					" WHERE PersonT.ID = {INT} ; \r\n" 					
					" SELECT TOP 1 PersonT.ID, First, Middle, Last , Prefix, Title "
					" FROM ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					" LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID "					
					" WHERE ProvidersT.PersonID = {INT} ; \r\n "
					" SELECT LocationsT.Name AS Location,LocationsT.Address1, LocationsT.Address2, LocationsT.City, LocationsT.State, LocationsT.Zip, LocationsT.Phone ,LocationsT.Fax "
					" FROM LocationsT "
					" WHERE LocationsT.ID = {INT} ";

	_RecordsetPtr rs = CreateParamRecordset(strPatientAndProviderQry,m_nPatientID,m_nProviderID,GetCurrentLocationID());	

	//Get Patient Information from recordset

	if(rs->RecordCount > 0)
	{		
		UserDefinedID = AdoFldLong(rs,"UserDefinedID",-1);
		PatientFirstName = AdoFldString(rs,"First","");
		PatientMiddleName = AdoFldString(rs, "Middle","");
		PatientLastName = AdoFldString(rs, "Last","");
		PatientAddress1 = AdoFldString(rs, "Address1", "");
		PatientAddress2 = AdoFldString(rs, "Address2" ,"");
		PatientCity = AdoFldString(rs, "City", "");
		PatientState = AdoFldString(rs, "State" ,"");
		PatientZip = AdoFldString(rs, "Zip", "");
		PatientGender = AdoFldByte(rs, "Gender", -1);
		PatientHomePhone = AdoFldString(rs, "HomePhone" ,"");
		PatientWorkPhone = AdoFldString(rs, "WorkPhone","");
		PatientExtension = AdoFldString(rs, "Extension" ,"");
		PatientCellPhone = AdoFldString(rs, "CellPhone","");

		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		PatientBirthDate = AdoFldDateTime(rs, "BirthDate",dtInvalid);
		PatientSocialSecurity = AdoFldString(rs,"SocialSecurity","");
	}	
	

	//Get Provider Information from recordset

	rs = rs->NextRecordset(NULL);

	if(rs->RecordCount > 0)
	{				
		 ProviderFirstName = AdoFldString(rs,"First","");
		 ProviderMiddleName = AdoFldString(rs,"Middle","");
		 ProviderLastName = AdoFldString(rs, "Last","");
		 ProviderPrefix = AdoFldString(rs, "Prefix","");
		 ProviderTitle = AdoFldString(rs, "Title","");					
	}
	else
	{
		 ProviderFirstName = "";
		 ProviderMiddleName = "";
		 ProviderLastName = "";
		 ProviderPrefix = "";
		 ProviderTitle = "";
	}

	rs = rs->NextRecordset(NULL);

	if(rs->RecordCount > 0)
	{
		 Location = AdoFldString(rs, "Location","");
		 LocationAddress1 = AdoFldString(rs, "Address1", "");
		 LocationAddress2 = AdoFldString(rs, "Address2", "");
		 LocationCity = AdoFldString(rs, "City","");
		 LocationState = AdoFldString(rs, "State", "");
		 LocationZip = AdoFldString(rs, "Zip" ,"");
		 LocationPhone = AdoFldString(rs, "Phone" ,"");
		 LocationFax = AdoFldString(rs,"Fax","");
	}

	
	//(r.wilson 6/4/2012) PLID 48952 - All peices of the query qill get added on this and this will hold the actual sql query that will be executed
	CString strQryFinal = "SELECT ";

	//(r.wilson 6/4/2012) PLID 48952 - String that holds the query for the patient's information (It will be the same no matter which Rx is chosen)
	CString strPatientInfoQry;
	strPatientInfoQry.Format(
			" %s AS PersonID, '%s' AS PatientFirstName, '%s' AS PatientMiddleName, '%s' AS PatientLastName, '%s' AS PatientAddress1, '%s' AS PatientAddress2, "
			" '%s' AS PatientCity, '%s' AS PatientState, '%s' AS PatientZip, '%s' AS PatientGender, '%s' AS PatientHomePhone, '%s' AS PatientWorkPhone, '%s' AS PatientExtension, " 
			" '%s' AS PatientCellPhone, %s AS PatientBirthDate, '%s' AS PatientSocialSecurity, ",
			/*PersonID*/ ValidLongSQL(PersonID), _Q(PatientFirstName), _Q(PatientMiddleName), _Q(PatientLastName), _Q(PatientAddress1), _Q(PatientAddress2),
			_Q(PatientCity),_Q(PatientState),_Q(PatientZip),ValidLongSQL(PatientGender),_Q(PatientHomePhone),_Q(PatientWorkPhone),_Q(PatientExtension),
			_Q(PatientCellPhone), ValidDateSQL(FormatDateTimeForSql(PatientBirthDate)),_Q(PatientSocialSecurity));		
	
	strQryFinal += strPatientInfoQry;


	//(r.wilson 6/4/2012) PLID 48952 - String to hold the glasses lens info for the query. This includes the provider who prescribed the Rx
	CString strQueryGlassesRx;
			
	strQueryGlassesRx.Format(
		" %s AS ID,  '%s' AS RxDate, " 
		" %s AS RightEyeNearPd, %s AS RightEyeSegHeight, %s AS RightEyeSphere, %s AS RightCylinder, %s AS RightAddition, %s AS RightPrism, " //row 2
		" %s AS RightPrismAxis, '%s' AS RightBase1 , '%s' AS RightBase2, %s AS RightDistPD, %s AS LeftEyeNearPd, %s AS LeftEyeSegHeight, %s AS LeftEyeSphere, %s AS LeftCylinder, %s AS LeftAddition,  %s AS LeftPrism, "// row 3
		" %s AS LeftPrismAxis, '%s' AS LeftBase1 , '%s' AS LeftBase2 , %s AS LeftDistPD ,%s AS UserDefinedID, "		
		" %s AS ProviderID, '%s' AS ProviderFirstName, '%s' AS ProviderMiddleName, " // row 6
		" '%s' AS ProviderLastName, '%s' AS ProviderPrefix, '%s' AS ProviderTitle, '%s' AS Location, '%s' AS LocationAddress1, '%s' AS LocationAddress2, '%s' AS LocationCity, " // row 7
		" '%s' AS LocationState, '%s' AS LocationZip, '%s' AS LocationPhone, '%s' AS LocationFax, '%s' AS RxExpirationDate, " // row 8
		" %s AS RightSecondaryPrism, %s AS LeftSecondaryPrism, ",  // row 9
		/*ID*/ ValidLongSQL(-1), /*RxDate*/ _Q(AsString(m_oLensRx.strRxDate)),
		/*RightEyeNearPd*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strNearPD), /*RightEyeSegHeight*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strHeight), /*RightEyeSphere*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strSphere), /*RightCylinder*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strCylinder), /*RightAddition*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strAddition),/*RightPrism*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strPrism1), //row 2
		/*RightPrismAxis*/ ValidLongSQL(m_oLensRx.LensRxOD.strAxis) , _Q(m_oLensRx.LensRxOD.strBase1), _Q(m_oLensRx.LensRxOD.strBase2), ValidDoubleSQL(m_oLensRx.LensRxOD.strDistPD), /*LeftEyeNearPd*/ ValidDoubleSQL(m_oLensRx.LensRxOS.strNearPD), /*LeftEyeSegHeight*/ ValidDoubleSQL(m_oLensRx.LensRxOS.strHeight), /*LeftEyeSphere*/ ValidDoubleSQL(m_oLensRx.LensRxOS.strSphere), /*LeftCylinder*/ ValidDoubleSQL(m_oLensRx.LensRxOS.strCylinder) , /*LeftAddition*/ ValidLongSQL(m_oLensRx.LensRxOS.strAddition), /*LeftPrism*/ ValidDoubleSQL(m_oLensRx.LensRxOS.strPrism1),
		/*LeftPrismAxis*/ ValidLongSQL(m_oLensRx.LensRxOS.strAxis), _Q(m_oLensRx.LensRxOS.strBase1),_Q(m_oLensRx.LensRxOS.strBase2), ValidDoubleSQL(m_oLensRx.LensRxOS.strDistPD),/* UserDefinedID*/ ValidLongSQL(UserDefinedID),		
		ValidLongSQL(m_nProviderID),_Q(ProviderFirstName),_Q(ProviderMiddleName),
		_Q(ProviderLastName), _Q(ProviderPrefix), _Q(ProviderTitle), _Q(Location), _Q(LocationAddress1), _Q(LocationAddress2), _Q(LocationCity),
		_Q(LocationState), _Q(LocationZip), _Q(LocationPhone), _Q(LocationFax) /*,  RightPrismAxisStr _Q(m_oLensRx.LensRxOD.strPrismAxisStr), /*LeftPrismAxisStr _Q(m_oLensRx.LensRxOS.strPrismAxisStr)*/, /*RxExpirationDate*/ _Q(AsString(m_oLensRx.strRxExpDate)),
		/*RightSecondaryPrism*/ ValidDoubleSQL(m_oLensRx.LensRxOD.strPrism2) , /*LeftSecondaryPrism*/ ValidDoubleSQL(m_oLensRx.LensRxOS.strPrism2)  );
		
	
	strQryFinal += strQueryGlassesRx;
			


	//(r.wilson 6/4/2012) PLID 48952 - String to hold the contact lens info for the query. This includes the provider who prescribed the Rx
	CString strConLensQuery;

	// (j.dinatale 2013-04-12 11:42) - PLID 55862 - include the Doc Ins fields for both lenses
	strConLensQuery.Format(
	" %s AS ConLensRxDate,"
	" %s AS ConLensRightDiameter, %s AS ConLensRightEyeSphere, %s AS ConLensRightCylinder, %s AS ConLensRightCylinderAxis , %s AS ConLensRightBC , %s AS ConLensRightAddition,  " //row 2
	" %s AS ConLensLeftDiameter, %s AS ConLensLeftEyeSphere, %s AS ConLensLeftCylinder, %s AS ConLensLeftCylinderAxis, %s AS ConLensLeftAddition , %s AS ConLensLeftBC, "// row 3					
	" %s AS ConLensRxExpirationDate, '%s' AS ConLensRxRightDocIns, '%s' AS ConLensRxLeftDocIns " ,	
	/*RxDate*/ ValidDateSQL(AsString(m_oContactLensRx.varRxDate)) ,
	/*[RightEyeSegHeight] RightDiameter*/ ValidDoubleSQL(m_oContactLensRx.clriOD.strDiameter), /*RightEyeSphere*/ ValidDoubleSQL(m_oContactLensRx.clriOD.strSphere), /*RightCylinder*/ ValidDoubleSQL(m_oContactLensRx.clriOD.strCylinder), /*RightCylinderAxis*/ ValidDoubleSQL(m_oContactLensRx.clriOD.strAxis),  ValidDoubleSQL(m_oContactLensRx.clriOD.strBC) ,/*RightAddition*/ ValidDoubleSQL(m_oContactLensRx.clriOD.strAddition), //row 2
	/*[LeftEyeSegHeight]LeftDiamter*/ ValidDoubleSQL(m_oContactLensRx.clriOS.strDiameter), /*LeftEyeSphere*/ ValidDoubleSQL(m_oContactLensRx.clriOS.strSphere), /*LeftCylinder*/ ValidDoubleSQL(m_oContactLensRx.clriOS.strCylinder), /*LeftAxis*/ ValidDoubleSQL(m_oContactLensRx.clriOS.strAxis), /*LeftAddition*/ ValidDoubleSQL(m_oContactLensRx.clriOS.strAddition), ValidDoubleSQL(m_oContactLensRx.clriOS.strBC),						
	/*RxExpirationDate*/ ValidDateSQL(AsString(m_oContactLensRx.varRxExpDate)), _Q(m_oContactLensRx.clriOD.strDocIns), _Q(m_oContactLensRx.clriOS.strDocIns)
	);				
	
	//(r.wilson 6/4/2012) PLID 48952 - Append contact lens portion of query onto the final query
	strQryFinal += strConLensQuery;	

	CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(737)]);
	infReport.strListBoxSQL = strQryFinal;

	CPrintInfo prInfo;
	
		CPrintDialog* dlg;
		dlg = new CPrintDialog(FALSE);
		
		prInfo.m_bPreview = IsPreview;
		
		prInfo.m_bDirect = false;
		prInfo.m_bDocObject = false;
		if(prInfo.m_pPD != NULL) {
			delete prInfo.m_pPD;
		}
		prInfo.m_pPD = dlg;
	

	//(r.wilson 6/4/2012) PLID 48952 - Initialize empty params and run the report
	CPtrArray aryParams;	
	RunReport(&infReport, &aryParams, IsPreview, (CWnd*)this, "Optical Prescriptions",  &prInfo);
	ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done

	return TRUE;

}
void CGlassesEMNPrescriptionList::OnBnClickedButtonPrint()
{
	try
	{
		GlassesContactLenseRxPrintSelectDlg PrePrintDialog(this);

		CString strProviderIDs;   //Glasses Provider IDs
		CString strCLProviderIDs; // Contact Lens Provider IDs
		CString strProviders;
		CString strCLProviders;
		CString strGlassesExamDate;
		CString strConLensExamDate;
		BOOL bPrintPreview = false;

		IRowSettingsPtr pRowParent = m_GlassesEMNPrescriptionList->GetCurSel();
		IRowSettingsPtr pRowParentCL = m_ContactLensRxList->GetCurSel();

		if(pRowParent != NULL){
			if(pRowParent->GetParentRow() != NULL)
			{
				pRowParent = pRowParent->GetParentRow();
			}
				_variant_t v_tmpProviderID = pRowParent->GetValue(lvrxProviderID);
				if(v_tmpProviderID.vt != VT_EMPTY)
				{
					strProviderIDs = VarString(v_tmpProviderID,"");
				}
				_variant_t v_tmpProviders = pRowParent->GetValue(lvrxProviderName);
				if(v_tmpProviders.vt != VT_EMPTY)
				{
					strProviders = VarString(v_tmpProviders,"");					
				}
				_variant_t v_tmpGlassesExamDate = pRowParent->GetValue(lvrxRxDate);
				if(v_tmpGlassesExamDate.vt != VT_NULL)
				{					
					COleDateTime dt_GlassesExamDate = VarDateTime(v_tmpGlassesExamDate);
					if(dt_GlassesExamDate.GetStatus() == COleDateTime::valid)
					{
						strGlassesExamDate = dt_GlassesExamDate.Format(_T("%m/%d/%Y"));
					}					
				}
			
		}

		if(pRowParentCL != NULL){
			if(pRowParentCL->GetParentRow() != NULL)
			{
				pRowParentCL = pRowParentCL->GetParentRow();
			}
				_variant_t v_tmpCLProviderID = pRowParentCL->GetValue(PrevConLensRxList::ProviderID);
				if(v_tmpCLProviderID.vt != VT_EMPTY)
				{
					strCLProviderIDs = VarString( v_tmpCLProviderID,"" );
				}
				_variant_t v_tmpCLProviders = pRowParentCL->GetValue(PrevConLensRxList::ProviderName);
				if(v_tmpCLProviders.vt != VT_NULL)
				{
					strCLProviders = VarString( v_tmpCLProviders, "");
				}
				_variant_t v_tmpConLensExamDate = pRowParentCL->GetValue(PrevConLensRxList::RxDate);
				if(v_tmpConLensExamDate.vt != VT_NULL)
				{					
					COleDateTime dt_ConLensExamDate = VarDateTime(v_tmpConLensExamDate);
					if(dt_ConLensExamDate.GetStatus() == COleDateTime::valid)
					{
						strConLensExamDate = dt_ConLensExamDate.Format(_T("%m/%d/%Y"));
					}
				}
		}
		
		PrePrintDialog.m_strProviderIDs = strProviderIDs;
		PrePrintDialog.m_strCLProviderIDs = strCLProviderIDs;
		PrePrintDialog.m_strProviders = strProviders;
		PrePrintDialog.m_strCLProviders = strCLProviders;	
		PrePrintDialog.m_strPatientName = m_strPatientName;
		PrePrintDialog.m_bPrintGlassesRx = (m_GlassesEMNPrescriptionList->GetCurSel()  == NULL ? FALSE:TRUE );
		PrePrintDialog.m_bPrintContactLensRx = (m_ContactLensRxList->GetCurSel()  == NULL ? FALSE:TRUE );
		PrePrintDialog.m_strGlassesExamDate = strGlassesExamDate;
		PrePrintDialog.m_strConLensExamDate = strConLensExamDate;

		if(m_GlassesEMNPrescriptionList->GetCurSel() == NULL 
		  && m_ContactLensRxList->GetCurSel() == NULL)
		{
			AfxMessageBox("Please select prescription(s) to print.");
			return ;
		}

		if(PrePrintDialog.DoModal() == IDCANCEL)
		{
			return;
		}

		

		BOOL bPrintGlassesRx = PrePrintDialog.m_bPrintGlassesRx;
		BOOL bPrintContactLensRx = PrePrintDialog.m_bPrintContactLensRx;
		m_nProviderID = PrePrintDialog.m_nSelectedProviderID;
		bPrintPreview = PrePrintDialog.m_bPrintPreview;

		if(bPrintGlassesRx == FALSE && bPrintContactLensRx == FALSE)
		{
			return;
		}

		if((m_bCalledForContactLens || m_bCalledForGlassesAndContacts) && bPrintContactLensRx) {
				IRowSettingsPtr pRow = m_ContactLensRxList->CurSel;
				IRowSettingsPtr pChildRow,pParentRow;
				if(pRow != NULL  ) 
				{				
					// try to read parent 
					pRow = pRow->GetParentRow(); 	
					if (pRow == NULL){ 
						// current row is parent row
						pParentRow = m_ContactLensRxList->CurSel;
					}
					else{	
						// set parent row
						pParentRow=pRow ;
					}
					if (pParentRow != NULL)
					{
						// (s.dhole 2012-04-25 12:32) - PLID 49969 Added Emnid
						m_oContactLensRx.nEmnId = pParentRow->GetValue(PrevConLensRxList::EmnID);
						m_oContactLensRx.varRxDate  = pParentRow->GetValue(PrevConLensRxList::RxDate);
						if(m_oContactLensRx.varRxDate.vt == VT_NULL){					
							m_oContactLensRx.varRxDate = _T("");
						}
						m_oContactLensRx.varRxExpDate =  pParentRow->GetValue(PrevConLensRxList::RxExpirationDate);
						if(m_oContactLensRx.varRxExpDate.vt == VT_NULL){
							m_oContactLensRx.varRxExpDate = _T("");
						}
						pChildRow = pParentRow ->GetFirstChildRow(); 
						if (pChildRow != NULL) {
							_variant_t vareye_site = pChildRow->GetValue(PrevConLensRxList::Rxeye_site); 
							if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
								SetLensRxDetail(pChildRow, m_oContactLensRx.clriOD);
							}
							else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
								SetLensRxDetail(pChildRow, m_oContactLensRx.clriOS);
							} 
						}
						pChildRow =  pChildRow ->GetNextRow (); 
						if (pChildRow != NULL) {
							_variant_t vareye_site =   pChildRow->GetValue(PrevConLensRxList::Rxeye_site); 
							if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
								SetLensRxDetail(pChildRow, m_oContactLensRx.clriOD);
							}
							else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
								SetLensRxDetail(pChildRow, m_oContactLensRx.clriOS);
							}
						}
						m_oContactLensRx.bRecordExists = TRUE;
						
					}
					
				}
			
			}

		if((!m_bCalledForContactLens || m_bCalledForGlassesAndContacts) && bPrintGlassesRx == TRUE) {
				IRowSettingsPtr pRow = m_GlassesEMNPrescriptionList->CurSel;
				IRowSettingsPtr pChildRow,pParentRow;
				if(pRow != NULL  ) 
				{				
					// try to read parent 
					pRow = pRow->GetParentRow(); 	
					if (pRow == NULL){ 
						// current row is parent row
						pParentRow = m_GlassesEMNPrescriptionList->CurSel;
					}
					else{	
						// set parent row
						pParentRow=pRow ;
					}
					if (pParentRow != NULL)
					{
						m_oLensRx.strRxDate  = pParentRow->GetValue(lvrxRxDate)  ;
						if(m_oLensRx.strRxDate.vt == VT_EMPTY){
							m_oLensRx.strRxDate = _T("");
						}
						m_oLensRx.strRxExpDate =  pParentRow->GetValue(lvrxRxExpirationDate)  ;
						if(m_oLensRx.strRxDate.vt == VT_EMPTY){
							m_oLensRx.strRxDate = _T("");
						}
						// (s.dhole 2012-04-25 12:32) - PLID 49969 Added Emnid
						m_oLensRx.nEmnId =  pParentRow->GetValue(lvrxEmnID);
						pChildRow =  pParentRow ->GetFirstChildRow(); 
						if (pChildRow != NULL) {
							_variant_t vareye_site =   pChildRow->GetValue(lvrxeye_site); 
							if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
								SetLensRxDetail(pChildRow, m_oLensRx.LensRxOD);
							}
							else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
								SetLensRxDetail(pChildRow, m_oLensRx.LensRxOS);
							} 
						}
						pChildRow =  pChildRow ->GetNextRow (); 
						if (pChildRow != NULL) {
							_variant_t vareye_site =   pChildRow->GetValue(lvrxeye_site); 
							if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OD")==0)) {
								SetLensRxDetail(pChildRow, m_oLensRx.LensRxOD);
							}
							else if ((AsString(vareye_site)!="") &&  (AsString(vareye_site).CollateNoCase("OS")==0)) {
								SetLensRxDetail(pChildRow, m_oLensRx.LensRxOS);
							}
						}
						m_oLensRx.ISRecordExist=TRUE;
					}
					
					//CNxDialog::OnOK();				
				}		
			}



		BOOLEAN bSuccessful = PrintRecord(bPrintPreview,bPrintGlassesRx,bPrintContactLensRx);

		if(bSuccessful == FALSE)
		{
			return;
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}


CString CGlassesEMNPrescriptionList::ValidDoubleSQL(CString strDouble)
{
	CString strOutput = "NULL";	
	if(!strDouble.IsEmpty()){
		double nTmpValue = atof(strDouble);
		strOutput.Format("%f",nTmpValue);
		return _Q(strOutput);
	}

	return _Q(strOutput);
}

CString CGlassesEMNPrescriptionList::ValidLongSQL(long nLong)
{
	CString strOutput = "NULL";		
	strOutput.Format("%d",nLong);
	return strOutput;
}

CString  CGlassesEMNPrescriptionList::ValidDateSQL(CString  strDate   )
{
	CString strTemp = "NULL";
	if (!strDate.IsEmpty()){
		COleDateTime dt_Tmp;
		if(dt_Tmp.ParseDateTime(strDate) == true)
		{
			strTemp = "CAST('" + _Q(strDate) + "' AS DATETIME)";
		}
	}
	return strTemp;	

}

CString CGlassesEMNPrescriptionList::ValidLongSQL(CString strLong)
{
	CString strOutput = "NULL";	
	if(!strLong.IsEmpty()){
		double nTmpValue = atof(strLong);
		strOutput.Format("%f",nTmpValue);
		return _Q(strOutput);
	}

	return _Q(strOutput);
}


// (s.tullis 2015-10-19 16:18) - PLID 67263- Glasses Query
CSqlFragment CGlassesEMNPrescriptionList::GetHL7GlassesSQL()
{

	CSqlFragment HL7Glasses;

	HL7Glasses.Create(
R"(
Select * FROM 
(

SELECT  LensRxT.ID as LensRxID,null AS PerentID, NULL AS id, NULL AS eye_site, NULL AS PrescriptionSphere, NULL AS FarHalfPd, NULL AS CylinderValue,NULL AS CylinderAxis,  
NULL AS AdditionValue, NULL AS PrismValue, NULL AS SecondaryPrismValue, NULL AS PrismAxis,NULL AS PrismAxisStr, NULL AS NearHalfPd, NULL AS SegHeight, NULL AS PrismAxis2,  NULL AS PrismAxisStr2 , 
LensRxT.RxDate, LensRxT.RxExpirationDate,  
NULL AS GlassesOrderID, NULL as GlassesOrderType, NULL AS OrderDate ,  
LensRxT.PersonID, NULL AS GlassesOrderDate, 
NULL AS ProviderID, 
NULL AS ProviderName,
RxType,HL7MessageID as HL7ID 
FROM   LensRxT  
INNER JOIN LensDetailRxT ON (LensRxT.RightLensDetailRxID = LensDetailRxT.id OR LensRxT.LeftLensDetailRxID = LensDetailRxT.ID) 

UNION 
 
SELECT  LensRxT.ID as LensRxID,LensRxT.ID AS PerentID ,  LensDetailRxT.id, 'OD' as eye_site,  PrescriptionSphere, FarHalfPd, CylinderValue, CylinderAxis,  
AdditionValue, PrismValue, SecondaryPrismValue, PrismAxis, PrismAxisStr, NearHalfPd, SegHeight, PrismAxis2, PrismAxisStr2 , 
NULL   aS RxDate, NULL AS RxExpirationDate, 
NULL  AS GlassesOrderID, NULL  GlassesOrderType,  NULL AS OrderDate , 
LensRxT.PersonID, NULL  AS GlassesOrderDate,  
NULL AS ProviderID, 
NULL AS ProviderName,
RxType,HL7MessageID as HL7ID 
FROM LensRxT   
INNER JOIN LensDetailRxT  ON LensRxT.RightLensDetailRxID = LensDetailRxT.id 
 
UNION

SELECT LensRxT.ID as LensRxID,LensRxT.ID AS PerentID, LensDetailRxT.id, 'OS' as eye_site,  PrescriptionSphere, FarHalfPd, CylinderValue, CylinderAxis,  
AdditionValue, PrismValue, SecondaryPrismValue, PrismAxis, PrismAxisStr, NearHalfPd, SegHeight, PrismAxis2, PrismAxisStr2 , 
NULL   aS RxDate, NULL AS RxExpirationDate,  
Null AS GlassesOrderID, NULL  GlassesOrderType, NULL AS OrderDate , 
LensRxT.PersonID, NULL  AS GlassesOrderDate,  
NULL AS ProviderID, 
NULL AS ProviderName, 
RxType,HL7MessageID as HL7ID 
FROM LensRxT  
INNER JOIN LensDetailRxT  ON LensRxT.LeftLensDetailRxID = LensDetailRxT.id 

UNION

SELECT  LensRxT.ID  as LensRxID,LensRxT.ID AS PerentID ,  NULL AS id, 'OS' as eye_site,  NULL AS PrescriptionSphere, NULL AS FarHalfPd, NULL AS CylinderValue, NULL AS CylinderAxis,  
NULL AS AdditionValue, NULL AS PrismValue, NULL AS SecondaryPrismValue, NULL AS PrismAxis, NULL AS PrismAxisStr, NULL AS NearHalfPd, NULL AS SegHeight, NULL AS PrismAxis2, NULL AS PrismAxisStr2 , 
NULL   aS RxDate, NULL AS RxExpirationDate,  
NULL AS GlassesOrderID, NULL  GlassesOrderType, NULL AS OrderDate , 
LensRxT.PersonID, NULL  AS GlassesOrderDate,  
NULL AS ProviderID, 
NULL AS ProviderName,
RxType, HL7MessageID as HL7ID 
FROM  LensRxT   
INNER JOIN LensDetailRxT  ON LensRxT.RightLensDetailRxID  = LensDetailRxT.id 
WHERE LensRxT.LeftLensDetailRxID IS NULL 
) HL7GlassesRxQ
Left Join 
GlassesOrderT ON HL7GlassesRxQ.LensRxID = GlassesOrderT.LensRxID
WHERE  HL7GlassesRxQ.RxType = {CONST_INT} AND HL7GlassesRxQ.PersonID ={INT} AND HL7GlassesRxQ.HL7ID  IS NOT NULL AND GlassesOrderT.ID IS NULL
ORDER BY HL7GlassesRxQ.LensRxID,HL7GlassesRxQ.eye_site	

)"
, LensRxType::RxTypeGlasses, m_nPatientID);

	return HL7Glasses;
}


// (s.tullis 2015-10-19 16:18) - PLID 67263- Contacts Query
CSqlFragment CGlassesEMNPrescriptionList::GetHL7ContactsSQL()
{

	CSqlFragment HL7Contacts;

	HL7Contacts.Create(
R"(
SELECT NULL AS OrderID, LensRxT.PersonID AS PatientID, NULL AS OrderDate, 
LensRxT.ID AS LensRxID, 
LensRxT.RxDate AS ExamDate, LensRxT.RxExpirationDate AS ExpirationDate, LensRxT.RxIssueDate AS IssueDate, 
OSLensRxDetailsQ.ID AS LeftLensRxDetailID, OSLensRxDetailsQ.Sphere AS OSSphere, OSLensRxDetailsQ.Cyl AS OSCyl, OSLensRxDetailsQ.Axis AS OSAxis, 
OSLensRxDetailsQ.Addition AS OSAdd, OSLensRxDetailsQ.BC AS OSBC, OSLensRxDetailsQ.Diameter AS OSDiam, OSLensRxDetailsQ.Note AS OSNote, OSLensRxDetailsQ.DocIns AS OSDocIns, 
ODLensRxDetailsQ.ID AS RightLensRxDetailID, ODLensRxDetailsQ.Sphere AS ODSphere, ODLensRxDetailsQ.Cyl AS ODCyl, ODLensRxDetailsQ.Axis AS ODAxis, 
ODLensRxDetailsQ.Addition AS ODAdd, ODLensRxDetailsQ.BC AS ODBC, ODLensRxDetailsQ.Diameter AS ODDiam, ODLensRxDetailsQ.Note AS ODNote, ODLensRxDetailsQ.DocIns AS ODDocIns, 
NULL AS ProviderID, 
NULL AS ProviderName,
HL7MessageID as HL7ID 
FROM 
LensRxT
LEFT JOIN GLassesOrderT
ON  GLassesOrderT.LensRxID = LensRxT.ID
LEFT JOIN ( 
SELECT ID, PrescriptionSphere AS Sphere, CylinderValue AS Cyl, CylinderAxis AS Axis, AdditionValue AS Addition, 
BC, Diameter, Color, Quantity, Note, DocIns FROM LensDetailRxT 
) OSLensRxDetailsQ ON LensRxT.LeftLensDetailRxID = OSLensRxDetailsQ.ID 
LEFT JOIN ( 
SELECT ID, PrescriptionSphere AS Sphere, CylinderValue AS Cyl, CylinderAxis AS Axis, AdditionValue AS Addition, 
BC, Diameter, Color, Quantity, Note, DocIns FROM LensDetailRxT 
) ODLensRxDetailsQ ON LensRxT.RightLensDetailRxID = ODLensRxDetailsQ.ID 
WHERE  (OSLensRxDetailsQ.ID IS NOT NULL OR ODLensRxDetailsQ.ID IS NOT NULL) AND LensRxT.RxType = {CONST_INT} AND LensRxT.PersonID = {INT}  AND LensRxT.HL7MessageID IS NOT NULL AND GlassesOrderT.ID IS NULL

)"
, LensRxType::RxTypeContacts, m_nPatientID);

	return HL7Contacts;
}

// (s.tullis 2015-10-19 16:18) - PLID 67263 - Load Glasses records into the Datalist
void CGlassesEMNPrescriptionList::LoadHL7Glasses()
{
	try{
		IRowSettingsPtr pRxRow, pRxPRow;
		_RecordsetPtr rsRx = CreateParamRecordset(GetHL7GlassesSQL());

		while (!rsRx->eof) {
			if (AdoFldLong(rsRx, "PerentID", -1) == -1)
			{
				
				pRxPRow = m_GlassesEMNPrescriptionList->GetNewRow();

				if (m_PrescriptionWindowDisplayType == pwShowRxList){
					if (m_bAllowOrder == TRUE){
						NXDATALIST2Lib::IFormatSettingsPtr pNoLink(__uuidof(NXDATALIST2Lib::FormatSettings));
						pNoLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
						pRxPRow->PutRefCellFormatOverride(lvrxRxAction, pNoLink);
						pRxPRow->PutValue(lvrxRxAction, "HL7 Message");
						pRxPRow->PutCellForeColor(lvrxRxAction , RGB(0, 0, 0));
						pRxPRow->PutCellLinkStyle(lvrxRxAction , dlLinkStyleFalse);
					}
					else
					{
						// nothing
					}
				}
				else if (m_PrescriptionWindowDisplayType == pwAllowRxSelection){
					pRxPRow->PutValue(lvrxRxAction, "HL7");
				}
				CGlassesEMNPrescriptionList::LoadRxRows(pRxPRow, rsRx);
				m_GlassesEMNPrescriptionList->AddRowSorted(pRxPRow, NULL);
			}
			else
			{
				pRxRow = m_GlassesEMNPrescriptionList->GetNewRow();
				CGlassesEMNPrescriptionList::LoadRxRows(pRxRow, rsRx);
				m_GlassesEMNPrescriptionList->AddRowSorted(pRxRow, pRxPRow);
				pRxPRow->PutExpanded(VARIANT_TRUE);
			}

			rsRx->MoveNext();
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2015-10-19 16:18) - PLID 67263 - Load Contacts data list
void CGlassesEMNPrescriptionList::LoadHL7Contacts(BOOL bSetDocInsWidth)
{
	try{
		_RecordsetPtr rsContactRx = CreateParamRecordset(GetHL7ContactsSQL());

		while (!rsContactRx->eof){

			LoadContactListRow(rsContactRx, bSetDocInsWidth,TRUE);
			rsContactRx->MoveNext();
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2015-10-19 16:18) - PLID 67263 - Refactored loading contacts row so we can use it for HL7 Contacts
void CGlassesEMNPrescriptionList::LoadContactListRow(_RecordsetPtr rsContactRx, BOOL &bSetDocInsWidth,BOOL bHL7Record)
{
	try{
		// (j.dinatale 2013-04-16 16:09) - PLID 55766 - keep track of if we set the doc's ins col width
		
		long nGlassesOrderID = AdoFldLong(rsContactRx, "OrderID",-1);
		long nLensRxID = AdoFldLong(rsContactRx, "LensRxID", -1);
		long nHL7ID = AdoFldLong(rsContactRx, "HL7ID", -1);
		long nLeftLensDetailRxID = AdoFldLong(rsContactRx, "LeftLensRxDetailID", -1);
		long nRightLensDetailRxID = AdoFldLong(rsContactRx, "RightLensRxDetailID", -1);
		COleDateTime dtOrderDate = AdoFldDateTime(rsContactRx, "OrderDate", g_cdtNull);
		COleDateTime dtExamDate = AdoFldDateTime(rsContactRx, "ExamDate", g_cdtNull);
		COleDateTime dtExpirationDate = AdoFldDateTime(rsContactRx, "ExpirationDate", g_cdtNull);
		COleDateTime dtIssueDate = AdoFldDateTime(rsContactRx, "IssueDate", g_cdtNull);
		_variant_t vtOSSphere = rsContactRx->Collect["OSSphere"];
		_variant_t vtOSCyl = rsContactRx->Collect["OSCyl"];
		_variant_t vtOSAxis = rsContactRx->Collect["OSAxis"];
		_variant_t vtOSAdd = rsContactRx->Collect["OSAdd"];
		_variant_t vtOSBC = rsContactRx->Collect["OSBC"];
		_variant_t vtOSDiam = rsContactRx->Collect["OSDiam"];
		CString strOSNote = AdoFldString(rsContactRx, "OSNote", "");
		CString strOSDocIns = AdoFldString(rsContactRx, "OSDocIns", "");
		_variant_t vtODSphere = rsContactRx->Collect["ODSphere"];
		_variant_t vtODCyl = rsContactRx->Collect["ODCyl"];
		_variant_t vtODAxis = rsContactRx->Collect["ODAxis"];
		_variant_t vtODAdd = rsContactRx->Collect["ODAdd"];
		_variant_t vtODBC = rsContactRx->Collect["ODBC"];
		_variant_t vtODDiam = rsContactRx->Collect["ODDiam"];
		CString strODNote = AdoFldString(rsContactRx, "ODNote", "");
		CString strODDocIns = AdoFldString(rsContactRx, "ODDocIns", "");

		//r.wilson (6/18/2012) PLID 50915
		CString strProviderID;
		strProviderID.Format("%d", AdoFldLong(rsContactRx, "ProviderID", -1));
		CString strProviderName = AdoFldString(rsContactRx, "ProviderName", "");

		NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_ContactLensRxList->GetNewRow();

		if (pParentRow){
			pParentRow->PutValue(PrevConLensRxList::LensRxID, nLensRxID);
			pParentRow->PutValue(PrevConLensRxList::OrderID, (long)nGlassesOrderID);
			if (dtExamDate.GetStatus() == COleDateTime::valid){
				pParentRow->PutValue(PrevConLensRxList::RxDate, _variant_t(dtExamDate, VT_DATE));
				pParentRow->PutValue(PrevConLensRxList::SortDate, _variant_t(dtExamDate, VT_DATE));
			}
			//TES 4/18/2012 - PLID 49764 - Throughout this function, make sure and not leave any fields as VT_EMPTY
			else {
				pParentRow->PutValue(PrevConLensRxList::RxDate, g_cvarNull);
				pParentRow->PutValue(PrevConLensRxList::SortDate, g_cvarNull);
			}


			//r.wilson (6/18/2012) PLID 50915 - Add the [;] separated list of providerID's to the row as a string
			pParentRow->PutValue(PrevConLensRxList::ProviderID, _bstr_t(strProviderID));

			pParentRow->PutValue(PrevConLensRxList::ProviderName, _bstr_t(strProviderName));


			if (dtExpirationDate.GetStatus() == COleDateTime::valid){
				pParentRow->PutValue(PrevConLensRxList::RxExpirationDate, _variant_t(dtExpirationDate, VT_DATE));
			}
			else {
				pParentRow->PutValue(PrevConLensRxList::RxExpirationDate, g_cvarNull);
			}

			if (dtIssueDate.GetStatus() == COleDateTime::valid){
				pParentRow->PutValue(PrevConLensRxList::RxIssueDate, _variant_t(dtIssueDate, VT_DATE));
			}
			else {
				pParentRow->PutValue(PrevConLensRxList::RxIssueDate, g_cvarNull);
			}

			if (dtOrderDate.GetStatus() == COleDateTime::valid){
				pParentRow->PutValue(PrevConLensRxList::Date, _variant_t(dtOrderDate, VT_DATE));

				if (dtExamDate.GetStatus() != COleDateTime::valid){
					pParentRow->PutValue(PrevConLensRxList::SortDate, _variant_t(dtOrderDate, VT_DATE));
				}
				else {
					pParentRow->PutValue(PrevConLensRxList::SortDate, g_cvarNull);
				}
			}
			else {
				pParentRow->PutValue(PrevConLensRxList::Date, g_cvarNull);
				pParentRow->PutValue(PrevConLensRxList::SortDate, g_cvarNull);
			}
			// (s.tullis 2015-10-19 16:18) - PLID 67263- Load HL7 ID
			pParentRow->PutValue(PrevConLensRxList::HL7ID, nHL7ID);
			// (s.tullis 2015-10-19 16:18) - PLID 67263- Load HL7 Text if HL7 RX
			// (s.dhole 2012-04-25 12:35) - PLID 47395 Set value
			if (m_PrescriptionWindowDisplayType == pwShowRxList){
				if (m_bAllowOrder == TRUE){
					if (bHL7Record)
					{
						NXDATALIST2Lib::IFormatSettingsPtr pNoLink(__uuidof(NXDATALIST2Lib::FormatSettings));
						pNoLink->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
						pParentRow->PutRefCellFormatOverride(PrevConLensRxList::RxAction, pNoLink);
						pParentRow->PutCellForeColor(PrevConLensRxList::RxAction, RGB(0, 0, 0));
						pParentRow->PutCellLinkStyle(PrevConLensRxList::RxAction, dlLinkStyleFalse);
					}
					pParentRow->PutValue(PrevConLensRxList::RxAction, bHL7Record?"HL7 Message":"Click to Open Order");
				}
				else{
					// nothing
				}
			}
			else if (m_PrescriptionWindowDisplayType == pwAllowRxSelection){
				pParentRow->PutValue(PrevConLensRxList::RxAction, bHL7Record?"HL7":"Order");
			}
			m_ContactLensRxList->AddRowSorted(pParentRow, NULL);

			// (j.dinatale 2012-05-15 16:44) - PLID 50346 - prevent a blank row from showing
			if (nRightLensDetailRxID > 0){
				// (j.dinatale 2012-04-19 17:52) - PLID 48724 - add OD before OS! Opps!
				NXDATALIST2Lib::IRowSettingsPtr pODRow = m_ContactLensRxList->GetNewRow();
				if (pODRow){
					pODRow->PutValue(PrevConLensRxList::LensDetailRxID, nRightLensDetailRxID);
					pODRow->PutValue(PrevConLensRxList::OrderID, (long)nGlassesOrderID);
					pODRow->PutValue(PrevConLensRxList::Rxeye_site, "OD");

					if (vtODSphere.vt != VT_NULL){
						pODRow->PutValue(PrevConLensRxList::PrescriptionSphere, _bstr_t(AsPrescriptionNumber(AsString(vtODSphere), pnfSignedFloat)));
					}
					else {
						pODRow->PutValue(PrevConLensRxList::PrescriptionSphere, g_cvarNull);
					}

					if (vtODCyl.vt != VT_NULL){
						pODRow->PutValue(PrevConLensRxList::Cylinder, _bstr_t(AsPrescriptionNumber(AsString(vtODCyl), pnfSignedFloat)));
					}
					else {
						pODRow->PutValue(PrevConLensRxList::Cylinder, g_cvarNull);
					}

					if (vtODAxis.vt != VT_NULL){
						pODRow->PutValue(PrevConLensRxList::CylinderAxis, vtODAxis);
					}
					else {
						pODRow->PutValue(PrevConLensRxList::CylinderAxis, g_cvarNull);
					}

					if (vtODAdd.vt != VT_NULL){
						pODRow->PutValue(PrevConLensRxList::AdditionValue, _bstr_t(AsPrescriptionNumber(AsString(vtODAdd), pnfSignedFloat)));
					}
					else {
						pODRow->PutValue(PrevConLensRxList::AdditionValue, g_cvarNull);
					}

					if (vtODBC.vt != VT_NULL){
						pODRow->PutValue(PrevConLensRxList::BC, vtODBC);
					}
					else {
						pODRow->PutValue(PrevConLensRxList::BC, g_cvarNull);
					}

					if (vtODDiam.vt != VT_NULL){
						pODRow->PutValue(PrevConLensRxList::Diameter, vtODDiam);
					}
					else {
						pODRow->PutValue(PrevConLensRxList::Diameter, g_cvarNull);
					}

					pODRow->PutValue(PrevConLensRxList::ObjPtr, g_cvarNull);

					// (j.dinatale 2012-05-15 16:16) - PLID 50346 - load the OD note
					if (!strODNote.IsEmpty()){
						pODRow->PutValue(PrevConLensRxList::Note, _bstr_t(strODNote));
					}

					// (j.dinatale 2013-03-19 17:03) - PLID 55766 - doctor's instruction field, also fix the width of the col
					if (!strODDocIns.IsEmpty()){
						pODRow->PutValue(PrevConLensRxList::DocIns, _bstr_t(strODDocIns));
						if (!bSetDocInsWidth){
							NXDATALIST2Lib::IColumnSettingsPtr pCol = m_ContactLensRxList->GetColumn(PrevConLensRxList::DocIns);
							if (pCol){
								pCol->StoredWidth = 150;
								bSetDocInsWidth = TRUE;
							}
						}
					}

					m_ContactLensRxList->AddRowSorted(pODRow, pParentRow);
				}
			}

			// (j.dinatale 2012-05-15 16:44) - PLID 50346 - prevent a blank row from showing
			if (nLeftLensDetailRxID > 0){
				NXDATALIST2Lib::IRowSettingsPtr pOSRow = m_ContactLensRxList->GetNewRow();
				if (pOSRow){
					pOSRow->PutValue(PrevConLensRxList::LensDetailRxID, nLeftLensDetailRxID);
					pOSRow->PutValue(PrevConLensRxList::OrderID, (long)nGlassesOrderID);
					pOSRow->PutValue(PrevConLensRxList::Rxeye_site, "OS");

					if (vtOSSphere.vt != VT_NULL){
						pOSRow->PutValue(PrevConLensRxList::PrescriptionSphere, _bstr_t(AsPrescriptionNumber(AsString(vtOSSphere), pnfSignedFloat)));
					}
					else {
						pOSRow->PutValue(PrevConLensRxList::PrescriptionSphere, g_cvarNull);
					}

					if (vtOSCyl.vt != VT_NULL){
						pOSRow->PutValue(PrevConLensRxList::Cylinder, _bstr_t(AsPrescriptionNumber(AsString(vtOSCyl), pnfSignedFloat)));
					}
					else {
						pOSRow->PutValue(PrevConLensRxList::Cylinder, g_cvarNull);
					}

					if (vtOSAxis.vt != VT_NULL){
						pOSRow->PutValue(PrevConLensRxList::CylinderAxis, vtOSAxis);
					}
					else {
						pOSRow->PutValue(PrevConLensRxList::CylinderAxis, g_cvarNull);
					}

					if (vtOSAdd.vt != VT_NULL){
						pOSRow->PutValue(PrevConLensRxList::AdditionValue, _bstr_t(AsPrescriptionNumber(AsString(vtOSAdd), pnfSignedFloat)));
					}
					else {
						pOSRow->PutValue(PrevConLensRxList::AdditionValue, g_cvarNull);
					}

					if (vtOSBC.vt != VT_NULL){
						pOSRow->PutValue(PrevConLensRxList::BC, vtOSBC);
					}
					else {
						pOSRow->PutValue(PrevConLensRxList::BC, g_cvarNull);
					}

					if (vtOSDiam.vt != VT_NULL){
						pOSRow->PutValue(PrevConLensRxList::Diameter, vtOSDiam);
					}
					else {
						pOSRow->PutValue(PrevConLensRxList::Diameter, g_cvarNull);
					}

					pOSRow->PutValue(PrevConLensRxList::ObjPtr, g_cvarNull);

					// (j.dinatale 2012-05-15 16:16) - PLID 50346 - load the OS note
					if (!strOSNote.IsEmpty()){
						pOSRow->PutValue(PrevConLensRxList::Note, _bstr_t(strOSNote));
					}

					// (j.dinatale 2013-03-19 17:03) - PLID 55766 - doctor's instruction field, also fix the width of the col
					if (!strOSDocIns.IsEmpty()){
						pOSRow->PutValue(PrevConLensRxList::DocIns, _bstr_t(strOSDocIns));
						if (!bSetDocInsWidth){
							NXDATALIST2Lib::IColumnSettingsPtr pCol = m_ContactLensRxList->GetColumn(PrevConLensRxList::DocIns);
							if (pCol){
								pCol->StoredWidth = 150;
								bSetDocInsWidth = true;
							}
						}
					}

					m_ContactLensRxList->AddRowSorted(pOSRow, pParentRow);
				}
			}

			pParentRow->PutExpanded(VARIANT_TRUE);
		}
	}NxCatchAll(__FUNCTION__)
}
