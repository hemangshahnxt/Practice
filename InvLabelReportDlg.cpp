// InvLabelReportDlg.cpp : implementation file
//
// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
#include "stdafx.h"
#include "InvLabelReportDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"

using namespace ADODB;
using namespace NXTIMELib;

// CInvLabelReportDlg dialog

IMPLEMENT_DYNAMIC(CInvLabelReportDlg, CNxDialog)
typedef enum  
{
	eColID = 0,
	eColPrint,
	eColName,
	eColBarcode,			
	eColQuantity,	
	eColUnitDesc,
	eColPrice,
	eColVendor,
	eColStyleName, 
	eColColorDescription,
	eColColorCode,
	eColEye,
	eColBridge, 
	eColTemple,
	eColDBL,
	eColA,
	eColB,
	eColED,
	eColEDAngle,
	eColCircumference,
	eColYearIntroduced,
	eColLastCost,
	eColDateReceived, 

} ItemListColumn;

CInvLabelReportDlg::CInvLabelReportDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvLabelReportDlg::IDD, pParent)
{
	m_nOrderID = -1;
	m_nProductId = -1;
	m_bIsFramesReport = FALSE; 
}

CInvLabelReportDlg::~CInvLabelReportDlg()
{
}

CInvLabelReportDlg::CInvLabelReportDlg(CWnd* pParent /*=NULL*/, long nProductID)
	: CNxDialog(CInvLabelReportDlg::IDD, pParent)
{
	m_nProductId = nProductID;	
}

void CInvLabelReportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PRINT_LABEL, m_btnPrint);
	DDX_Control(pDX, IDC_PRINTPREVIEW, m_btnPrintPreview);
}


BEGIN_MESSAGE_MAP(CInvLabelReportDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PRINT_LABEL, OnPrint)
	ON_BN_CLICKED(IDC_PRINTPREVIEW, OnPrintPreview)
END_MESSAGE_MAP()

// CInvLabelReportDlg message handlers
BEGIN_EVENTSINK_MAP(CInvLabelReportDlg, CNxDialog)
	ON_EVENT(CInvLabelReportDlg, IDC_INV_LABEL_REPORT_LIST, 9, CInvLabelReportDlg::EditingFinishingInvLabelReportList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

BOOL CInvLabelReportDlg::OnInitDialog() 
{	
	try
	{
		CNxDialog::OnInitDialog();
		m_pOrderList= BindNxDataList2Ctrl(this, IDC_INV_LABEL_REPORT_LIST, GetRemoteData(), false);
		m_btnPrint.AutoSet(NXB_PRINT);
		m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);
		m_btnCancel.AutoSet(NXB_CANCEL);
		
		//r.wilson 3/7/2012 
		CString strWhere;
			// If the product ID is -1 then we know that its an order. So go inside
			if(m_nProductId == -1){
				strWhere = FormatString("  ID = %d", m_nOrderID );	
			}
			//Else If the Product Id is not -1 (which means it got set) then go inside here
			else
			{
				//r.wilson -> Change the dialog caption
				this->SetWindowTextA("Print Labels For Frame");

				//r.wilson -> Only get 1 product 
				strWhere = FormatString(" ID = %d",m_nProductId);
				
				//r.wilson PLID 48351 -> Query gets the Product Info and Frame Data (minus any order details from the initial query in the datalist resoure view)
				_bstr_t strQry = 
					"( "
					"	SELECT      "
					"		ProductT.ID, cast(( 1 ) as bit)  AS Select_Print,ServiceT.Name, ServiceT.Barcode, 1 AS QuantityOrdered  , "
					"		ProductT.UnitDesc,  ServiceT.Price, ProductT.LastCost, NULL AS DateReceived, FramesDataT.ManufacturerName,  "
					"		FramesDataT.StyleName, FramesDataT.ColorDescription, FramesDataT.ColorCode, FramesDataT.Eye, FramesDataT.Bridge,  "
					"		FramesDataT.Temple, FramesDataT.DBL, FramesDataT.A, FramesDataT.B, FramesDataT.ED, FramesDataT.EDAngle,  "
					"		FramesDataT.Circumference, FramesDataT.YearIntroduced   "
					"	FROM  "
					"		ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID  "
					"		LEFT JOIN FramesDataT ON ProductT.FramesDataID = FramesDataT.ID "
					") as Q";
								
				m_pOrderList->FromClause = strQry;	
			}
		
		m_pOrderList->WhereClause = _bstr_t(strWhere);
		m_pOrderList->Requery();

		
	} NxCatchAll("Error in OnInitDialog");
	return TRUE;  
}

// Report print 4 labels per row.  Create temp table having 4 labels, 
//calculate how many row required for product and insert those records. 
//Than remain labels insert as individual labels. 
//To move next product keep last unfilled label count and adjust those with next product 
//and remaining label count follow same row count routine 
void CInvLabelReportDlg::PrintRecord(BOOL IsPreview ) 
{
	long nLabelCount =0;   // total label count per product
	long nId =1000; 
	long nEmptycount =0; // Store empty label count from previs row
	CString  strName ; 
	CString  strBarCode ; 
	CString  strUnitDesc ; 
	COleCurrency dblPrice ;
	CString strSql;

	// (b.spivey, October 10, 2011) - PLID 45882 - for framesdata fields. 
	CString  strVendor;
	CString  strStyleName;
	CString  strColorDescription;
	CString  strColorCode;
	CString  strEye;
	CString  strBridge;
	CString  strTemple;
	CString  strDBL;
	CString  strA;
	CString  strB;
	CString  strED;
	CString  strEDAngle;
	CString  strCircumference;
	CString  strYearIntroduced;
	COleCurrency cyLastCost;
	COleDateTime dtDateReceived; 

	BOOL IsRecordExist=FALSE;
	try
		{
			// (b.spivey, October 10, 2011) - PLID 45882 - Added the FramesDataT fields. 
			// (b.spivey, November 04, 2011) - PLID 46072 - Added a date received field for the frames version of the report. 
		strSql= "if (object_id(N'[tempdb]..[#tempBarcodeTabel]','local')  is not null)"
			" BEGIN "
			" drop table #tempBarcodeTabel"
			" END "
			" SET NOCOUNT ON "
			" CREATE TABLE #tempBarcodeTabel   \r\n"
			" (   \r\n"
			" ID int,  \r\n"
			" [Name0]  varchar(255),  \r\n"
			" [UnitDesc0] varchar(255),  \r\n"
			" [Barcode0] varchar(255),  \r\n"
			" price0 money,  \r\n"
			" Vendor0 varchar(50),  \r\n "
			" StyleName0 varchar(50),  \r\n "
			" ColorDescription0 varchar(50),  \r\n "
			" ColorCode0 varchar(20),  \r\n "
			" Eye0 varchar(3),  \r\n "
			" Bridge0 varchar(10),  \r\n "
			" Temple0 varchar(10),  \r\n "
			" DBL0 varchar(10),  \r\n "
			" A0 varchar(5),  \r\n "
			" B0 varchar(5),  \r\n "
			" ED0 varchar(5),  \r\n "
			" EDAngle0 varchar(6),  \r\n "
			" Circumference0 varchar(6),  \r\n "
			" YearIntroduced0 varchar(4),  \r\n "
			" LastCost0 money,  \r\n "
			" DateReceived0 datetime,  \r\n "
			" [Name1]  varchar(255),  \r\n"
			" [UnitDesc1] varchar(255),  \r\n"
			" [Barcode1] varchar(255),  \r\n"
			" price1 money,  \r\n"
			" Vendor1 varchar(50),  \r\n "
			" StyleName1 varchar(50),  \r\n "
			" ColorDescription1 varchar(50),  \r\n "
			" ColorCode1 varchar(20),  \r\n "
			" Eye1 varchar(3),  \r\n "
			" Bridge1 varchar(10),  \r\n "
			" Temple1 varchar(10),  \r\n "
			" DBL1 varchar(10),  \r\n "
			" A1 varchar(5),  \r\n "
			" B1 varchar(5),  \r\n "
			" ED1 varchar(5),  \r\n "
			" EDAngle1 varchar(6),  \r\n "
			" Circumference1 varchar(6),  \r\n "
			" YearIntroduced1 varchar(4),  \r\n "
			" LastCost1 money,  \r\n "
			" DateReceived1 datetime,  \r\n "
			" [Name2]  varchar(255),  \r\n"
			" [UnitDesc2] varchar(255),  \r\n"
			" [Barcode2] varchar(255),  \r\n"
			" price2 money,  \r\n"
			" Vendor2 varchar(50),  \r\n "
			" StyleName2 varchar(50),  \r\n "
			" ColorDescription2 varchar(50),  \r\n "
			" ColorCode2 varchar(20),  \r\n "
			" Eye2 varchar(3),  \r\n "
			" Bridge2 varchar(10),  \r\n "
			" Temple2 varchar(10),  \r\n "
			" DBL2 varchar(10),  \r\n "
			" A2 varchar(5),  \r\n "
			" B2 varchar(5),  \r\n "
			" ED2 varchar(5),  \r\n "
			" EDAngle2 varchar(6),  \r\n "
			" Circumference2 varchar(6),  \r\n "
			" YearIntroduced2 varchar(4),  \r\n "
			" LastCost2 money,  \r\n "
			" DateReceived2 datetime,  \r\n "
			" [Name3]  varchar(255),  \r\n"
			" [UnitDesc3] varchar(255),  \r\n"
			" [Barcode3] varchar(255),  \r\n"
			" price3 money,  \r\n"
			" Vendor3 varchar(50),  \r\n "
			" StyleName3 varchar(50),  \r\n "
			" ColorDescription3 varchar(50),  \r\n "
			" ColorCode3 varchar(20),  \r\n "
			" Eye3 varchar(3),  \r\n "
			" Bridge3 varchar(10),  \r\n "
			" Temple3 varchar(10),  \r\n "
			" DBL3 varchar(10),  \r\n "
			" A3 varchar(5),  \r\n "
			" B3 varchar(5),  \r\n "
			" ED3 varchar(5),  \r\n "
			" EDAngle3 varchar(6),  \r\n "
			" Circumference3 varchar(6),  \r\n "
			" YearIntroduced3 varchar(4),  \r\n "
			" LastCost3 money,  \r\n "
			" DateReceived3 datetime  \r\n "
			" )  \r\n"
			" Declare @intCount int ; \r\n"
			"  set @intCount=1;  \r\n"
			" Declare @intval int ; \r\n";

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOrderList->GetFirstRow();
		while(pRow) {
			if	( VarBool(pRow->GetValue(eColPrint)) ==TRUE){
				IsRecordExist=TRUE;
				CParamSqlBatch batch;	
				CString strSqlInsertExt;
				CString strSqlUpdateExt;
				CString strSqlInsert;
				nLabelCount  = VarLong(  pRow->GetValue( eColQuantity ));
				strName = VarString(  pRow->GetValue( eColName ),"");
				strName.Replace("'","''"  );
				strBarCode = VarString(  pRow->GetValue( eColBarcode ),"");
				strBarCode .Replace("'","''"  );
				strUnitDesc= VarString(  pRow->GetValue( eColUnitDesc ));
				strUnitDesc.Replace("'","''"  );
				dblPrice = VarCurrency(  pRow->GetValue(eColPrice ));
				// (b.spivey, October 10, 2011) - PLID 45882 - Added several fields for glasses orders. 
				strVendor = _Q(VarString(pRow->GetValue(eColVendor), "")); 
				strStyleName = _Q(VarString(pRow->GetValue(eColStyleName), "")); 
				strColorDescription = _Q(VarString(pRow->GetValue(eColColorDescription), "")); 
				strColorCode = _Q(VarString(pRow->GetValue(eColColorCode), "")); 
				strEye = _Q(VarString(pRow->GetValue(eColEye), "")); 
				strBridge = _Q(VarString(pRow->GetValue(eColBridge), "")); 
				strTemple = _Q(VarString(pRow->GetValue(eColTemple), "")); 
				strDBL = _Q(VarString(pRow->GetValue(eColDBL), "")); 
				strA = _Q(VarString(pRow->GetValue(eColA), "")); 
				strB = _Q(VarString(pRow->GetValue(eColB), "")); 
				strED = _Q(VarString(pRow->GetValue(eColED), "")); 
				strEDAngle = _Q(VarString(pRow->GetValue(eColEDAngle), "")); 
				strCircumference = _Q(VarString(pRow->GetValue(eColCircumference), "")); 
				strYearIntroduced = _Q(VarString(pRow->GetValue(eColYearIntroduced), "")); 
				cyLastCost = VarCurrency(pRow->GetValue(eColLastCost), COleCurrency(0, 0)); 
				// (b.spivey, November 10, 2011) - PLID 46072 - Use today as a default. 
				dtDateReceived = VarDateTime(pRow->GetValue(eColDateReceived), g_cdtNull); 

				// Case 1 less than 4  Quntity
				// 1) insert record and store remaining empty fields in nEmptycount
				// 2)  go to next product and check lesss then nEmptycount , update current row and inser rest of the next line
				{
					CString  strSqlUpdateDetailExt="";
					for (int nCount=4-nEmptycount ;  ((nCount<(4-nEmptycount) + nLabelCount   )  && nCount <4) ; nCount++  )
					{
						// (b.spivey, October 10, 2011) - PLID 45882 - Added the 15 framesdatat fields here to be used in the 0-3 sets.
						// (b.spivey, November 04, 2011) - PLID 46072 - Added DateReceived
						strSqlUpdateDetailExt.Format(  " Update #tempBarcodeTabel Set "
							" [Name%d] ='%s' ,[UnitDesc%d]  ='%s',[Barcode%d]  ='%s',price%d =%s,  \r\n"
							" Vendor%d = '%s', StyleName%d = '%s', ColorDescription%d = '%s', ColorCode%d = '%s', \r\n "
							" Eye%d = '%s', Bridge%d = '%s', Temple%d = '%s', DBL%d = '%s', A%d = '%s', B%d = '%s', \r\n "
							" ED%d = '%s', EDAngle%d = '%s', Circumference%d = '%s', YearIntroduced%d = '%s', \r\n "
							" LastCost%d = %s, DateReceived%d = '%s' \r\n "
							" WHERE  id=%d  \r\n",
							nCount, strName,nCount,strUnitDesc,nCount,strBarCode ,nCount,FormatCurrencyForSql(dblPrice),
							nCount, strVendor, nCount, strStyleName, nCount, strColorDescription, nCount, strColorCode, 
							nCount, strEye, nCount, strBridge, nCount, strTemple, nCount, strDBL, nCount, strA, nCount, strB, 
							nCount, strED, nCount, strEDAngle, nCount, strCircumference, nCount, strYearIntroduced, nCount, 
							FormatCurrencyForSql(cyLastCost), nCount, FormatDateTimeForSql(dtDateReceived), 
							nId);
						strSqlUpdateExt+=strSqlUpdateDetailExt;
						nLabelCount -=1;
						nEmptycount-=1;
					}
				}
				// Insert remaining less than 4 lable new row row 
				if (nEmptycount ==0)
					{
					// To insert extra labels (Row with less than 4) 
					switch (nLabelCount  % 4)
						{
							// (b.spivey, October 10, 2011) - PLID 45882 - There are up to three sets of data left. The 
							//	"values" section of the insert query will insert them row by row, i.e., set 0 is the first row, 
							//  set 1 is the second row. 
							// (b.spivey, November 04, 2011) - PLID 46072 - Added DateReceived
						case 3: 
							nId +=1;
							strSqlInsertExt.Format(  " insert into #tempBarcodeTabel (id,[Name0],[UnitDesc0],[Barcode0],price0, "
								" Vendor0, StyleName0, ColorDescription0, ColorCode0, Eye0, Bridge0, Temple0, DBL0, "
								" A0, B0, ED0, EDAngle0, Circumference0, YearIntroduced0, LastCost0, DateReceived0, "
								"[Name1],[UnitDesc1],[Barcode1],price1 , "
								" Vendor1, StyleName1, ColorDescription1, ColorCode1, Eye1, Bridge1, Temple1, DBL1, "
								" A1, B1, ED1, EDAngle1, Circumference1, YearIntroduced1, LastCost1, DateReceived1, "
								"[Name2],[UnitDesc2],[Barcode2],price2 , "
								" Vendor2, StyleName2, ColorDescription2, ColorCode2, Eye2, Bridge2, Temple2, DBL2, "
								" A2, B2, ED2, EDAngle2, Circumference2, YearIntroduced2, LastCost2, DateReceived2) \r\n"
								" values(%d, '%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', "
								"'%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', "
								"'%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s') ;\r\n",
								nId,strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
								strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
								strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
								FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived), 
								strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
								strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
								strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
								FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived), 
								strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
								strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
								strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
								FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived));
							nEmptycount =1; 
							nLabelCount  -=3;
						break;
						case 2 :
							nId +=1;
								  strSqlInsertExt.Format(  " insert into #tempBarcodeTabel (id, [Name0],[UnitDesc0],[Barcode0],price0, "
									  " Vendor0, StyleName0, ColorDescription0, ColorCode0, Eye0, Bridge0, Temple0, DBL0, "
									  " A0, B0, ED0, EDAngle0, Circumference0, YearIntroduced0, LastCost0, DateReceived0, "
									  "[Name1],[UnitDesc1],[Barcode1],price1, "
									  " Vendor1, StyleName1, ColorDescription1, ColorCode1, Eye1, Bridge1, Temple1, DBL1, "
									  " A1, B1, ED1, EDAngle1, Circumference1, YearIntroduced1, LastCost1, DateReceived1) \r\n"
									" values(%d, '%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', "
									" '%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s') ;\r\n",
									nId,strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
									strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
									strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
									FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived),
									strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
									strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
									strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
									FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived));
								nEmptycount =2;
								nLabelCount  -=2;
						break;
						case 1 :
							nId +=1;
							strSqlInsertExt.Format(  " insert into #tempBarcodeTabel (id, [Name0],[UnitDesc0],[Barcode0],price0, "
								" Vendor0, StyleName0, ColorDescription0, ColorCode0, Eye0, Bridge0, Temple0, DBL0, "
								" A0, B0, ED0, EDAngle0, Circumference0, YearIntroduced0, LastCost0, DateReceived0) \r\n"
								" values(%d, '%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s') \r\n",
								nId,strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
							strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
							strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
							FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived));
							nEmptycount =3;
							nLabelCount  -=1;
						break;
						default:
							nEmptycount =0;
						break;
						}
					}
				// Insert 4 lables per row
				if ((nLabelCount  / 4) >0) 
					{
					// Loop to insert record into temp table for each product /Quantity (ignore rows with  less than 4 label)
					// (b.spivey, October 10, 2011) - PLID 45882 - every row of values is for one label. 
					// (b.spivey, November 04, 2011) - PLID 46072 - Added DateReceived
					strSqlInsert.Format(" set @intval=1;  \r\n"
						" while  @intval<=%d  \r\n"
						" Begin \r\n"
						" insert into #tempBarcodeTabel (id, [Name0],[UnitDesc0],[Barcode0],price0, "
						" Vendor0, StyleName0, ColorDescription0, ColorCode0, Eye0, Bridge0, Temple0, DBL0, "
						" A0, B0, ED0, EDAngle0, Circumference0, YearIntroduced0, LastCost0, DateReceived0, "
						"[Name1],[UnitDesc1],[Barcode1],price1 , "
						" Vendor1, StyleName1, ColorDescription1, ColorCode1, Eye1, Bridge1, Temple1, DBL1, "
						" A1, B1, ED1, EDAngle1, Circumference1, YearIntroduced1, LastCost1, DateReceived1, "
						"[Name2],[UnitDesc2],[Barcode2],price2 , "
						" Vendor2, StyleName2, ColorDescription2, ColorCode2, Eye2, Bridge2, Temple2, DBL2, "
						" A2, B2, ED2, EDAngle2, Circumference2, YearIntroduced2, LastCost2, DateReceived2, "
						"[Name3],[UnitDesc3],[Barcode3],price3, "
						" Vendor3, StyleName3, ColorDescription3, ColorCode3, Eye3, Bridge3, Temple3, DBL3, "
						" A3, B3, ED3, EDAngle3, Circumference3, YearIntroduced3, LastCost3, DateReceived3) \r\n "
						" values(@intCount, '%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', "  
						"'%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', "  
						"'%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s', "  
						"'%s','%s','%s',%s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %s, '%s') ;\r\n" 
						" set @intval= @intval + 1; \r\n"
						" set @intCount= @intCount + 1; \r\n"
						" end \r\n"		,(nLabelCount  / 4),strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
						strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
						strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
						FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived), 
						strName,strUnitDesc,strBarCode ,FormatCurrencyForSql(dblPrice),
						strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
						strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
						FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived), 
						strName,strUnitDesc,strBarCode,FormatCurrencyForSql(dblPrice),
						strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
						strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
						FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived), 
						strName,strUnitDesc,strBarCode, FormatCurrencyForSql(dblPrice), 
						strVendor, strStyleName, strColorDescription, strColorCode, strEye, strBridge, strTemple,
						strDBL, strA, strB, strED, strEDAngle, strCircumference, strYearIntroduced, 
						FormatCurrencyForSql(cyLastCost), FormatDateTimeForSql(dtDateReceived)); 
					}
				strSql += strSqlInsert;
				strSql += strSqlInsertExt;
				strSql += strSqlUpdateExt;
				}
			pRow = pRow->GetNextRow();
			}
			// (b.spivey, October 10, 2011) - PLID 45882 - Updated select query with the extra fields. 
			// (b.spivey, November 04, 2011) - PLID 46072 - Added DateReceived
			strSql += " Select id, [Name0],[UnitDesc0],[Barcode0],price0, "
						" Vendor0, StyleName0, ColorDescription0, ColorCode0, Eye0, Bridge0, Temple0, DBL0, "
						" A0, B0, ED0, EDAngle0, Circumference0, YearIntroduced0, LastCost0, DateReceived0, "
					"[Name1],[UnitDesc1],[Barcode1],price1 , "
						" Vendor1, StyleName1, ColorDescription1, ColorCode1, Eye1, Bridge1, Temple1, DBL1, "
						" A1, B1, ED1, EDAngle1, Circumference1, YearIntroduced1, LastCost1, DateReceived1, "
					"[Name2],[UnitDesc2],[Barcode2],price2 , "
						" Vendor2, StyleName2, ColorDescription2, ColorCode2, Eye2, Bridge2, Temple2, DBL2, "
						" A2, B2, ED2, EDAngle2, Circumference2, YearIntroduced2, LastCost2, DateReceived2, "
					"[Name3],[UnitDesc3],[Barcode3],price3, "
						" Vendor3, StyleName3, ColorDescription3, ColorCode3, Eye3, Bridge3, Temple3, DBL3, "
						" A3, B3, ED3, EDAngle3, Circumference3, YearIntroduced3, LastCost3, DateReceived3 "
					"from #tempBarcodeTabel  \r\n"
					" drop table  #tempBarcodeTabel  \r\n";
		// when print without preview code expecting  CPrintInfo
		if (IsRecordExist==TRUE)
		{
		CPrintInfo prInfo;
		if (!IsPreview) {
			CPrintDialog* dlg;
			dlg = new CPrintDialog(FALSE);
			prInfo.m_bPreview = false;
			prInfo.m_bDirect = false;
			prInfo.m_bDocObject = false;
			if(prInfo.m_pPD != NULL) {
				delete prInfo.m_pPD;
				}
			prInfo.m_pPD = dlg;
			}

			if(m_bIsFramesReport){
				// (b.spivey, October 21, 2011) - PLID 46073 - 717 is the frames version of the avery report. 
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(717)]);
				infReport.strListBoxSQL=strSql;
				
				CPtrArray aryParams;
				RunReport(&infReport, &aryParams, IsPreview, (CWnd*)this, "Inventory Barcodes Frames Labels",  &prInfo);
				ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
			}
			else{
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(698)]);
				infReport.strListBoxSQL=strSql;
				
				CPtrArray aryParams;
				//(e.lally 2010-10-20) PLID 26807 - Changed spelling of Bar Codes to Barcodes
				RunReport(&infReport, &aryParams, IsPreview, (CWnd*)this, "Inventory Barcodes Avery 8167 Labels",  &prInfo);
				ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
			}
		}
		else{
			MessageBox(FormatString("You must select at least one product for %s labels.", IsPreview ? "previewing" : "printing"));
		}
	} NxCatchAll("Error in PrintRecord");
}


void CInvLabelReportDlg::OnPrint() 
{
	try {
		PrintRecord(FALSE);
		CDialog::OnCancel();
	} NxCatchAll("Error in CInvLabelReportDlg::OnPrint()");

}
void CInvLabelReportDlg::OnPrintPreview() 
{
	try {
		PrintRecord(TRUE);
		CDialog::OnCancel();
	} NxCatchAll("Error in CInvLabelReportDlg::OnPrintPreview()");
}
void CInvLabelReportDlg::OnCancel() 
{
	try {
		CDialog::OnCancel();
	} NxCatchAll("Error in CInvLabelReportDlg::OnCancel()");
}



// We shiould not allew user to select 0 quantity 
void CInvLabelReportDlg::EditingFinishingInvLabelReportList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
try {
	if (*pbCommit == FALSE)//user hit escape
		return;
		if (eColQuantity == nCol &&  VarLong(*pvarNewValue)<1 ) 
		{
			MsgBox("You cannot have any quantity less than 1.");
			*pvarNewValue = varOldValue;
			*pbCommit = false;
			return;
		}
		else if (eColPrint  == nCol &&  VarBool(*pvarNewValue)==TRUE  ) 
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOrderList->CurSel ; 
			if (VarLong(  pRow ->GetValue( eColQuantity )) <1 )
			{
				MsgBox("You cannot have any quantity less than 1.");
				*pvarNewValue = varOldValue;
				*pbCommit = false;
				return;
			}
		}
	} NxCatchAll("Error in CInvLabelReportDlg::EditingFinishingInvLabelReportList()")
	// TODO: Add your message handler code here
}
