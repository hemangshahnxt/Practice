 #pragma once

//TES 9/23/2010 - PLID 40539 - Created
// CVisionWebOrderDlg dialog
#include "VisionWebOrderParametersDlg.h"
#include "InvVisionWebUtils.h"
#include "GlassesOrderServiceSelectDlg.h"

//TES 3/22/2011 - PLID 42762 - Moved PrescriptionNumberFormat to InvVisionWebUtils.h
enum PrescriptionNumberFormat;
struct GlassesOrder;
enum VisionWebOrderType;
enum GlassesOrderServiceType;



// (s.dhole 2012-04-24 10:20) - PLID 48821 Support multiple code link
// (s.dhole 2012-03-28 12:22) - PLID 43785 Added discount ammount data
struct LenseServiceInfo
	{
		long nRowID ;
		long nID ;
		long nServiceID ;
		long nSavedServiceID;
		CString  strCode ;
		CString strDescription;
		CString strSavedDescription;
		COleCurrency Cost;
		COleCurrency SavedCost;
		COleCurrency Price;
		COleCurrency SavedPrice;
		double nQty;
		double nSavedQty;
		long nType;
		long nSavedType;
		long nDesignID;
		BOOL bDesignBillPerLens;
		long nMaterialID;
		BOOL bMaterialBillPerLens;
		long nTreatmentID;
    	BOOL bTreatmentBillPerLens;
		BOOL bIsOD;
		BOOL bIsOS;
		BOOL bIsDelete;
		BOOL bIsDefault;
		BOOL bIsOffTheShelf;
		BOOL bIsOffTheShelfSaved;
		COleCurrency TotalAmount; // Qty * price
		COleCurrency Discount;
		COleCurrency LineTotal;
		COleCurrency VisionResp;
		COleCurrency SavedVisionResp;
		COleCurrency PatientResp;
		COleCurrency SavedPatientResp;
		struct DiscountList * DiscountList;
	};






class CVisionWebOrderDlg : public CNxDialog
{
	//TES 12/8/2010 - PLID 41715 - This dialog will update our parameter lists
	friend class CVisionWebOrderParametersDlg;

	DECLARE_DYNAMIC(CVisionWebOrderDlg)

public:
	CVisionWebOrderDlg(CWnd* pParent);   // standard constructor
	virtual ~CVisionWebOrderDlg();

	//TES 9/23/2010 - PLID 40539 - Set this to load an existing order, defaults to -1=New Order.
	long m_nOrderID;

	//TES 3/22/2011 - PLID 42761 - Set this to load a new order with data from the specified EMN
	long m_nEmnID;

	//TES 5/24/2011 - PLID 43737 - Call to specify the order type.
	// At the moment all that's allowed is vwotSpectacleLens (default) and vwotContactLensPatient
	VisionWebOrderType m_vwot;
	// (s.dhole 2012-04-09 11:58) - PLID 43785
	CArray<LenseServiceInfo,LenseServiceInfo&> m_arLenseServiceInfo;
// Dialog Data
	enum { IDD = IDD_VISION_WEB_ORDER_DLG };

protected:
	// (s.dhole 2011-03-29 18:43) -  PLID 43035
	// (j.dinatale 2012-04-09 19:27) - PLID 49536 - optician field
	CString m_strVisoinWebOrderID_DT;	
	NXDATALIST2Lib::_DNxDataListPtr m_pPatient, m_pLocation, m_pSupplier, m_pOrderType, m_pJobType, 
		m_pLensDesignL, m_pLensDesignR, m_pLensMaterialL, m_pLensMaterialR, m_pLensTreatmentsL, m_pLensTreatmentsR, 
		m_pFrameType, m_pThicknessType, m_pFrame, m_pVisionPlans, m_pProvider, m_pServiceCodeList, m_pProductList, 
		m_pServices, m_pCLSupplier, m_pCLProduct, m_pdlOpticianList;

	// (s.dhole 2011-05-16 12:12) - PLID 41986 
	HBITMAP m_hBitmap;

	CNxIconButton m_nxbOK, m_nxbCancel, m_nxbCatalogSetup, m_nxbCopyPrescription, m_nxbCopyLens, m_nxbCopyLensDesign, m_nxbCustomize
		, m_nxbPreviewEmn;
		//m_nxbFilterFrames
	// (s.dhole 2012-01-18 13:24) - PLID 47455 
	CNxIconButton  m_nxbSelectPrescriptions;
	/// (s.dhole 2011-03-22 13:22) - PLID 42898 Print Preview Button
	// (s.dhole 2011-05-04 12:55) -  PLID 42953 - Add Glasses Rx Report
	// (b.spivey, October 17, 2011) - PLID 44918 - Added Secondary prism fields. 
	// (s.dhole, 2011-12-07 12:29) - PLID 46883 - Added Base2R , Base2L fields. 
	CNxIconButton m_nxbPrintPreview,m_nxbPrintPreviewRx;
	CNxEdit m_nxeSphereL, m_nxeSphereR, m_nxeCylinderL, m_nxeCylinderR, m_nxeAxisL, m_nxeAxisR, m_nxeAddnL, m_nxeAddnR,
		m_nxePrismL, m_nxePrismR, m_nxeSecdPrismL, m_nxeSecdPrismR, m_nxeBaseL, m_nxeBaseR,
		m_nxeDistPDL, m_nxeDistPDR, m_nxeNearPDL, m_nxeNearPDR, m_nxeHeightL, m_nxeHeightR,
		m_nxeTreatmentCommentsL, m_nxeTreatmentCommentsR, m_nxeABox, m_nxeBBox, m_nxeDbl, m_nxeED,  
		m_nxeThicknessL, m_nxeThicknessR, m_nxeManufacturer, m_nxeStyle, m_nxeColor, m_nxeTempleLength,
		m_nxeEyeSize, m_nxeSpecialInstructions, m_nxeDescription, m_nxeCLQuantity, m_nxeCLNotes, 
		m_nxeLensCost, m_nxeFrameCost, m_nxeCLCost, m_nxeBase2R, m_nxeBase2L;
	CNxStatic m_nxsCreatedFromEmn;
	// (s.dhole 2012-05-07 18:15) - PLID 50131
	CNxEdit m_nxGlassesInvoiceNo;
	CString m_strGlassesInvoiceNo;

	CNxLabel m_nxlTreatmentsL, m_nxlTreatmentsR;

	NxButton m_nxbSubmitToVisionWeb, m_nxbProductsOnly, m_nxbServiceCodes, m_nxbProducts;

	//TES 4/8/2011 - PLID 43058
	NXTIMELib::_DNxTimePtr m_pDate;
	// (s.dhole 2011-04-27 10:48) - PLID 43451 
	// (s.dhole 2011-06-15 17:14) - PLID 43813  
	NXTIMELib::_DNxTimePtr m_pRxDate,m_pRxExpirationDate;

	//TES 9/23/2010 - PLID 40539 - Load an existing order's data onto this screen.
	void LoadExistingOrder();

	//TES 5/16/2011 - PLID 43701 - This function checks if there's a patient selected that has a previous order, 
	// the prescription information is empty, and the preference to copy prescription information is on, and if
	// all that's true, it will load the information from the previous order onto the current order.
	void LoadLastPatientRx();

	//TES 5/24/2011 - PLID 43737 - Show the appropriate controls for our order type
	void ReflectOrderType();

	//TES 5/25/2011 - PLID 43737 - Refresh the list of products when the supplier changes
	//void RequeryContactLensProducts();
	//TES 5/25/2011 - PLID 43737 - Track our ContactLensOrderInfoID
	//long m_nContactLensOrderInfoID;
	//TES 5/25/2011 - PLID 43737 - Track our Contact Lens product
	//long m_nCLProductID;
	//TES 5/25/2011 - PLID 43737 - Fields for Contact Lens Order auditing
	//CString m_strCLProductName, m_strSavedCLProductName;
	//long m_nSavedCLQuantity;
	//CString m_strSavedCLNotes;
	//TES 5/25/2011 - PLID 43737 - Track when the user edits the quantity
	//bool m_bChangedQuantity;

	// (j.dinatale 2012-11-28 13:07) - PLID 53913 - need to move this to its own function, this can happen in more than one place.
	void CalcNewExpirationDate();
			

	//TES 5/25/2011 - PLID 43737 - Update the Items To Bill list when the Contact Lens product changes
	//void UpdateContactLensProduct(long nPreviousProductID);

	//Fill the Frame-related fields based on the selected frame
	void ReflectFrame(BOOL IsExistData =FALSE);// (s.dhole 2010-11-15 16:44) - PLID 
	//Enable/disable the thickness fields.
	void ReflectThicknessType();

	BOOL OnMultiSelectTreatmentsL();
	BOOL OnMultiSelectTreatmentsR();

	//The various IDs that go into this order.
	long m_nLensRxID, m_nLeftRxDetailID, m_nRightRxDetailID, m_nLeftLensOtherInfoID, m_nRightLensOtherInfoID;

	//Keep track of which treatments are selected.
	//TES 5/22/2011 - PLID 43790 - This is now a struct that includes associated billing information
	

	
	// (s.dhole 2012-03-06 13:06) - PLID 47398  
	//TES 5/22/2011 - PLID 43790 - Pass in the "current" right lens treatments, they may not be identical to our member variable
	// if we're copying the treatments from the right lens.
	
	//TES 6/8/2011 - PLID 43790 - Consolidated the code for handline when materials change
	void UpdateRightMaterialID(long nNewMaterialID, NXDATALIST2Lib::IRowSettingsPtr pMaterialRow);
	void UpdateLeftMaterialID(long nNewMaterialID, NXDATALIST2Lib::IRowSettingsPtr pMaterialRow);

	//Re-filter all the VisionWeb catalog lists, based on the currently selected supplier.
	void RefreshCatalogLists();

	long m_nLeftDesignID, m_nRightDesignID;
	//TES 5/20/2011 - PLID 43790 - Track the billing information associated with each design
	long m_nLeftDesignCptID, m_nRightDesignCptID;

	// (s.dhole 2012-03-15 15:35) - PLID 48926
	struct TreatmentInformation {
		long nTreatmentID;
		BOOL bIsOD ;

		BOOL bBillPerLens;
		TreatmentInformation& operator=( const TreatmentInformation &s )  // assignment operator
		{ nTreatmentID = s.nTreatmentID;
			bIsOD= s.bIsOD;
			//nCptID = s.nCptID;
			bBillPerLens= s.bBillPerLens;
		return *this; }
	};
	CArray<TreatmentInformation,TreatmentInformation&> m_arLeftLensTreatments, m_arRightLensTreatments;

	//TES 5/22/2011 - PLID 43790 - Updates service codes after treatments are changed
	void UpdateTreatmentServiceCodesR(CArray<TreatmentInformation,TreatmentInformation&> &arOldTreatments);
	//CArray<long,long> m_arLeftDesignCptID,m_arRightDesignCptID;
	BOOL m_bLeftDesignBillPerLens, m_bRightDesignBillPerLens;

	void RefreshSupplierList();
	//TES 12/9/2010 - PLID 40539 - The design lists are based on the location, not the supplier, so they need their own Refresh()
	void RefreshDesignLists();

	//TES 4/13/2011 - PLID 43248 - Refresh the list of available products when the location changes
	void RefreshProducts();
	// (s.dhole 2012-03-15 15:38) - PLID 48926
	void UpdateTreatmentServiceCodesL(CArray<TreatmentInformation,TreatmentInformation&> &arOldTreatments, CArray<TreatmentInformation,TreatmentInformation&> &arRightLensTreatments);
	//TES 5/22/2011 - PLID 43790 - Takes a treatment ID, returns a struct with the billing information
	TreatmentInformation GetTreatmentInformation(long nTreatmentID);

	//TES 4/14/2011 - PLID 43288 - Moved the code for adding a product to the list of selected services into its own function
	//TES 5/25/2011 - PLID 43737 - Added quantity
	//TES 6/29/2011 - PLID 44381 - Added Type
	//void AddProduct(NXDATALIST2Lib::IRowSettingsPtr pProductRow, long nQuantity = 1, GlassesOrderServiceType gost = (GlassesOrderServiceType)0/*gostOther*/);
	//TES 5/20/2011 - PLID 43790 - Similarly for ServiceCodes
	//enum ListType
	//{
	//	lstDesign = 0,
	//	lstMaterial = 1,
	//	lstTreatment = 2,
	//	lstProduct = 3,
	//	lstService = 4,
	//};
	void RemoveDeletedRowFromServiceList(); 
	//void AddServiceCode(NXDATALIST2Lib::IRowSettingsPtr pServiceRow, GlassesOrderServiceType gost = (GlassesOrderServiceType)0/*gostOther*/);
		
	//TES 4/22/2011 - PLID 43389 - Used for auditing
	//CStringArray m_saDeletedServiceDescriptions;
	
	//Ensure that everything's filled out correctly, outputs which lenses are being saved (they may only have a prescription
	// for one eye, or neither if it's just a frame order).
	BOOL Validate(OUT BOOL &bSaveLeftLensRx, OUT BOOL &bSaveRightLensRx);

	void Save(BOOL bSaveLeftLens, BOOL bSaveRightLens);

	//TES 11/17/2010 - PLID 41528 - Checks the "Submit To VisionWeb" box, assuming the preference to do so is turned on
	// turned on, check
	void AutoCheckSubmitBox();

	void FormatPrescriptionNumber(UINT nID, PrescriptionNumberFormat pnf);

	void ReflectUsedLenses();

	//TES 9/23/2010 - PLID 40539 - Used for TrySetSel handling.
	long m_nPendingPatientID;
	//TES 12/10/2010 - PLID 40879 - Used to preserve the selection when filtering/unfiltering the frames list.
	//TES 4/20/2011 - PLID 43275 - Replaced m_nPendingFrameID with m_strPendingFrameFPC
	CString m_strPendingFrameFPC;
	//TES 6/7/2011 - PLID 43275 - Need to track whether the pending FPC is being loaded from data, if so, we need to not update the boxes
	bool m_bLoadingFrameFromData;

	//TES 4/15/2011 - PLID 43288 - Track which product the currently selected frame is associated with, so we can
	// remove it from the list if a different frame is selected.
	//long m_nCurrentFrameProductID;

	//TES 4/8/2011 - PLID 43058
	long m_nProviderID;
	long m_nPendingProviderID;

	bool m_bFormattingField;

	long m_nSupplierID;

	long m_nFrameTypeID;
	long m_nLeftMaterialID;
	long m_nRightMaterialID;
	//TES 5/22/2011 - PLID 43790 - Track the billing information associated with each material
	long m_nLeftMaterialCptID, m_nRightMaterialCptID;
	BOOL m_bLeftMaterialBillPerLens, m_bRightMaterialBillPerLens;

	//TES 10/29/2010 - PLID 41197 - Do we want to update the description if anything changes?
	bool m_bAutoGenerateDescription;

	//TES 12/6/2010 - PLID 41730 - Track whether the lens information is identical, in which case we want to auto-copy it as its entered
	// for the OD lens.
	bool m_bLensesIdentical;

	//TES 12/8/2010 - PLID 41715 - Does our current design have any custom parameters associated?
	bool m_bDesignHasParameters;
	//TES 12/8/2010 - PLID 41715 - The parameters associated with our current design.
	CArray<VisionWebCustomParam,VisionWebCustomParam&> m_arCustomParams;
	//TES 12/8/2010 - PLID 41715 - Is m_arCustomParams up to date?
	bool m_bParameterListLoaded;
	//TES 12/8/2010 - PLID 41715 - What values have been entered for our parameters?
	CMapStringToString m_mapParamValues, m_mapSavedParamValues;
	//TES 12/8/2010 - PLID 41715 - Make sure that m_arCustomParams is up to date.
	void EnsureParameterList();

	long m_nLocationID;
	//TES 12/10/2010 - PLID 45039 - Update the screen to reflect m_nLocationID and strLocationName.
	//TES 6/2/2011 - PLID 43939 - Added bRequeryFinished, which will be set to true if this is called from OnRequeryFinished
	void SetLocation(const CString &strLocationName, bool bRequeryFinished = false);
	CString m_strPendingLocationName;

	//TES 3/23/2011 - PLID 42975 - Cache the description of the EMN we were created from
	CString m_strEmnDescription;

	//TES 3/23/2011 - PLID 42975 - Show/Hide the controls related to the EMN this order was created from
	void ReflectEmn();

	//TES 3/23/2011 - PLID 42975 - Our dialog for previewing the EMN this order was created from
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;

	//TES 12/10/2010 - PLID 45039 - Update the screen to reflect m_nSupplierID, warn the user if that supplier is no longer valid.
	//TES 12/15/2010 - PLID 45039 - Added bWaitForRequery
	void SetSupplier(bool bWaitForRequery = false);

	//TES 3/23/2011 - PLID 42975 - The same code was getting called a few different places, I consolidated
	void CloseCleanup(UINT nReturn);

	//TES 12/16/2010 - PLID 41867 - Track whether the user has selected a different frame.
	bool m_bFrameChanged;
	// (s.dhole 2012-03-12 11:24) - PLID 48811 Frame To be ordered
	BOOL  m_bSavedFrameToBeOrdered;

	// (s.dhole 2010-12-20 12:04) - PLID 40538 Check if order imnformation has been change
	bool m_bOrderChanged;

	//TES 6/30/2011 - PLID 44166 - Track when the user has changed the costs, and when we're changing them ourselves.
	bool m_bFrameCostChangedByUser, m_bLensCostChangedByUser, m_bCLCostChangedByUser;
	bool m_bChangingFrameCost, m_bChangingLensCost, m_bChangingCLCost;
	//TES 6/30/2011 - PLID 44166 - Goes through the selected services, and updates the cost fields to match (except cost fields which 
	// have already been edited manually).
	
	// (s.dhole 2012-04-06 09:21) - PLID  49518 new frame
	long m_nFrameID ,m_nSavedFrameID, m_nFrameProductID,m_nSavedFrameProductID;	
	//TES 11/24/2010 - PLID 40864 - Cached values for auditing
	// (b.spivey, October 17, 2011) - PLID 44918 - Added secondary prism value 
	CString m_strSavedPatientName, m_strSavedLocationName, m_strSavedDescription, m_strSavedSupplierName, m_strSavedProviderName;
	double m_dSavedSphereL, m_dSavedSphereR, m_dSavedCylL, m_dSavedCylR, m_dSavedAdditionL, m_dSavedAdditionR,
		m_dSavedPrismL, m_dSavedPrismR, m_dSavedSecdPrismL, m_dSavedSecdPrismR, m_dSavedDistPdL, m_dSavedDistPdR, m_dSavedNearPdL, m_dSavedNearPdR,
		m_dSavedHeightL, m_dSavedHeightR, m_dSavedThicknessL, m_dSavedThicknessR, m_dSavedBoxA, m_dSavedBoxB,
		m_dSavedED, m_dSavedDBL;
	long m_nSavedAxisL, m_nSavedAxisR;
	CString m_strSavedBaseL, m_strSavedBaseR;
	// (s.dhole, 2011-12-07 12:29) - PLID 46883 - Added Base2. 
	CString m_strSavedBaseL2, m_strSavedBaseR2;
	// (j.dinatale 2012-05-11 12:47) - PLID 50254 - save the optician name
	CString m_strSavedDesignNameL, m_strSavedDesignNameR, m_strSavedMaterialNameL, m_strSavedMaterialNameR,
		m_strSavedTreatmentsL, m_strSavedTreatmentsR, m_strSavedTreatmentCommentsL, m_strSavedTreatmentCommentsR,
		m_strSavedJobType, m_strSavedJobNote, m_strSavedThicknessType, m_strSavedFrame, m_strSavedFrameType, m_strSavedOpticianName;
	COleDateTime m_dtSavedDate;
	//(r.wilson 4/19/2013) pl PLID 56307
	CString m_strLoadedTempleLength, m_strLoadedEyeSize;
	// (s.dhole 2011-04-27 10:48) - PLID 43451 
	// (s.dhole 2011-06-15 17:15) - PLID 43813
	COleDateTime m_dtSavedRxDate,m_dtSavedRxExpiration;
	//TES 6/29/2011 - PLID 44166 - Store costs for auditing
	COleCurrency m_cySavedFrameCost, m_cySavedLensCost, m_cySavedCLCost;

	// (b.spivey, November 14, 2012) - PLID 53422 - Cached property/flags.
	// (j.dinatale 2013-02-11 10:22) - PLID 55093 - we now keep track of the sold off shelf flag differently
	bool m_bAutoCheckSoldOffShelfNewOrder, m_bSoldOffShelfManualSet, m_bSoldOffShelfManualLastVal;

	// (j.dinatale 2013-04-30 11:21) - PLID 56458 - need to keep track if we are increasing the qty or not
	bool m_bIncreaseItemToBillQty;

	// (j.dinatale 2013-04-30 14:01) - PLID 56458 - need a function to encapsulate all the logic for service code additions
	void HandleServiceProcessing(OrderServiceListType::ServiceListType oslType, bool bIsOS, long nNewID, long nServiceID, BOOL bBillPerLens);
	
	//TES 10/29/2010 - PLID 41197 - Calculates what are automatically generated description should be, based on the values in the
	// controls on-screen.
	CString GetGeneratedDescription();
	//TES 10/29/2010 - PLID 41197 - Updates the description, if appropriate.
	void UpdateDescription();
	//TES 10/29/2010 - PLID 41197 - Updates the m_bAutoGenerateDescription variable.
	void CheckDescription();
	// (c.haag 2010-11-29) - PLID 41124 - Update the title bar text based on the selected patient
	//TES 12/14/2010 - PLID 41106 - Changed function name; this updates all patient-related data (which now includes vision plans).
	void ReflectCurrentPatient();
	void CheckForIdentical();// (s.dhole 2011-06-08 16:03) - PLID 43320
	long m_nPatientID;
	long m_nEMNPatientID, m_nLastEMN ;
	// (s.dhole 2012-04-25 17:05) - PLID 49992 
	long m_nInsuredPartyID;

	//(c.copits 2011-09-22) PLID 43743 - Add a warning if they attempt to enter two orders for the same patient on the same day
	bool CheckDoesPatientHaveOrderToday();

	//(c.copits 2011-09-27) PLID 45112 - Disable "Glasses Catalog" button for inactive suppliers
	void UpdateGlassesCatalogButton();

	// (b.savon 2012-01-10 14:30) - PLID 46663 - Column Widths
	void RestoreColumnWidths();
	void SaveColumnWidths();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (r.wilson 2012-20-2) PLID 43773
	void UpdateSupplierDataList();
	
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel(); // (c.haag 2010-11-16 9:53) - PLID 41124

	// (s.dhole 2012-02-02 10:15) - PLID 47396
	void AddFirstRowToFrame();
	// (s.dhole 2012-02-02 10:15) - PLID 47396
	void SetFrameFields(BOOL bIsRemoveData,BOOL bIsFieldEnable);

		//(r.wilson 3/5/2012 ) PLID 48294 - to handle barcode scanner data events.
	
	int GetServiceCodeFromBarcode(WPARAM wParam, LPARAM lParam);		
	int GetProductCodeFromBarcode(WPARAM wParam, LPARAM lParam);
	long m_nRowID;

	// (j.dinatale 2013-03-29 09:23) - PLID 55936 - Set up a prompt
	void RemoveLineItems(OrderServiceListType::ServiceListType slt, bool bIsOS);
	void CopyLineItems(OrderServiceListType::ServiceListType oslType, bool bIsOS, BOOL bBillPerLens);
	bool HandlePrompt(OrderServiceListType::ServiceListType oslType, bool bIsOS, long &nOldID, long nNewID, BOOL &bOldBillPerLens, BOOL bNewBillPerLens, bool bCopyServiceItemsToLens);

	//void LoadServiceInfo();
	void AddItemsToList(long nCptID ,BOOL bIsOD,BOOL bIsOS ,long nDesignID ,BOOL bDesignBillPerLens ,
						long nMaterialID ,BOOL bMaterialBillPerLens,long nTreatmentID 
						,BOOL bTreatmentBillPerLens 
						,NXDATALIST2Lib::IRowSettingsPtr pServiceRow =NULL,
						GlassesOrderServiceType nType = gostOther ,
						BOOL bIsProduct=FALSE,
						BOOL bIsDefaultProduct =FALSE,
						BOOL bIsOffTheShelf = FALSE);
	
	void LoadServiceList();
	void RefreshServiceList();
	void CalculateTotalDiscount(DiscountList *pDiscountList, COleCurrency cyCurrentTotal, long &nPercentOff,  COleCurrency &cyTotalLineDiscount );
	// (s.dhole 2012-04-16 16:53) - PLID 49728 
	BOOL m_bIsGlassesBilled ;
	// (s.dhole 2012-04-16 12:59) - PLID 49729 
	BOOL m_bIsGlassesvisionPlanExist;
	// (s.dhole 2012-04-16 16:53) - PLID 49728 
	void ApplyBilledSetting();
	// (s.dhole 2012-04-16 12:59) - PLID 49729 
	void ApplyVisionPlanSetting();
	// (s.dhole 2012-05-18 14:22) - PLID 
	void CheckFrameProductBillable(long nProductID,long  nLocationID,BOOL  &bIsBillable,BOOL  &bIsTrackableStatus );
public:
	DECLARE_EVENTSINK_MAP()
	void OnTrySetSelFinishedVisionwebPatient(long nRowEnum, long nFlags);
	afx_msg void OnCatalogSetup();
	void OnSelChosenVisionwebSupplier(LPDISPATCH lpRow);
	void OnSelChosenVisionwebLensTreatmentL(LPDISPATCH lpRow);
	void OnSelChosenVisionwebLensTreatmentR(LPDISPATCH lpRow);
	void OnSelChosenFrame(LPDISPATCH lpRow);
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	void OnSelChosenVisionwebThicknessType(LPDISPATCH lpRow);
	afx_msg void OnCopyPrescription();
	afx_msg void OnCopyLens();
	afx_msg void OnCopyLensDesign();// (s.dhole 2010-11-15 15:09) - PLID 
	void OnSelChosenVisionwebPatient(LPDISPATCH lpRow);

	void OnSelChosenVisionwebOrderType(LPDISPATCH lpRow);
	void OnRequeryFinishedVisionwebPatient(short nFlags);
	afx_msg void OnKillfocusVisionwebOrderDescription();
	void OnSelChosenVisionwebLensDesignR(LPDISPATCH lpRow);
	void OnSelChosenVisionwebLensDesignL(LPDISPATCH lpRow);
	void OnRequeryFinishedVisionwebSupplier(short nFlags);
	void OnRequeryFinishedVisionwebFrameType(short nFlags);
	void OnRequeryFinishedVisionwebLensMaterialR(short nFlags);
	void OnRequeryFinishedVisionwebLensMaterialL(short nFlags);
	void OnRequeryFinishedVisionwebLensDesignR(short nFlags);
	void OnRequeryFinishedVisionwebLensDesignL(short nFlags);
	void OnRequeryFinishedVisionwebLensTreatmentR(short nFlags);
	void OnRequeryFinishedVisionwebLensTreatmentL(short nFlags);
	afx_msg void OnEnChangeSphereR();
	afx_msg void OnEnChangeSphereL();
	void OnRequeryFinishedVisionwebLocation(short nFlags);
	void OnSelChosenVisionwebLocation(LPDISPATCH lpRow);
	void OnSelChosenVisionwebLensMaterialR(LPDISPATCH lpRow);
	void OnSelChosenVisionwebLensMaterialL(LPDISPATCH lpRow);
	afx_msg void OnKillfocusTreatmentCommentsR();
	afx_msg void OnKillfocusTreatmentCommentsL();
	afx_msg void OnCustomize();
	//afx_msg void OnFilterFrames();
	void OnRequeryFinishedFrame(short nFlags);
	void OnSelChangingVisionwebLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingVisionPlan(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	void OnRequeryFinishedVisionPlans(short nFlags);
	void OnSelChangingVisionwebThicknessType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnBtnOredrPrintPreview();// (s.dhole 2011-03-18 08:56) - PLID 42898 Print Glasses order
	afx_msg void OnBtnOredrPrintPreviewRx();// (s.dhole 2011-05-04 12:59) - PLID 42953
	void OnSelChangingFrame(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChangingJobType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	afx_msg void OnPreviewEmn();
	//afx_msg void OnProductsOnly();
	void OnTrySetSelFinishedVisionwebProvider(long nRowEnum, long nFlags);
	void OnSelChosenVisionwebProvider(LPDISPATCH lpRow);
	afx_msg void OnSelectServiceCodes();
	afx_msg void OnSelectProducts();
	void OnSelChosenAvailableServiceCodes(LPDISPATCH lpRow);
	void OnSelChosenAvailableProducts(LPDISPATCH lpRow);
	void OnRButtonDownGlassesOrderServices(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnRequeryFinishedVisionwebProvider(short nFlags);
	void OnTrySetSelFinishedFrame(long nRowEnum, long nFlags);
	afx_msg void OnSetFocusCylinderR();
	afx_msg void OnSetFocusCylinderL();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKillfocusLensCost();
	afx_msg void OnKillfocusFrameCost();
	//afx_msg void OnKillfocusClCost();
	void OnEditingFinishedGlassesOrderServices(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	//afx_msg void OnChangeFrameCost();
	//afx_msg void OnChangeClCost();
	//afx_msg void OnChangeLensCost();
	afx_msg void OnBnClickedSelectPriscription();
	void KillFocusGlassesOrderRxDate(); // (r.wilson 3/12/2012) PLID 47609 
	void OnSelChosenVisionPlan(LPDISPATCH lpRow);

	afx_msg void OnBnClickedChkFrameToBeOrdered();
	afx_msg void LeftClickGlassesOrderServices(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void EditingStartingGlassesOrderServices(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void SelChosenGlassesOrderOptician(LPDISPATCH lpRow);

	afx_msg void OnEnKillfocusTempleLength();
	afx_msg void OnEnKillfocusEyeSize();
	//(s.dhole 2013-05-21 10:58) - PLID 56558 
	afx_msg void OnEnKillfocusThicknessR();
	afx_msg void OnEnKillfocusThicknessL();
};
	
// (s.dhole 2012-04-09 11:58) - PLID 43785
// (s.dhole 2012-04-09 12:03) - PLID 49447 Support Contact lens order
void AuditAndGenerateSaveStringForDiscounts(long nPatientID,BOOL bNewCharge,BOOL bIsContact,  LenseServiceInfo *pLenseServiceInfo, CString &strSaveString);
void GlassesAuditAndGenerateSaveStringForDiscounts(long nPatientID,BOOL bNewCharge,BOOL bIsContact,CString strPatientName,
							long GlassesOrderServiceID ,DiscountList  *pDiscountList, CParamSqlBatch &batch ,CSqlFragment &sqlSave );
void GlassesAuditAndGenerateSaveStringForNewDiscounts(long nPatientID,BOOL bNewCharge,BOOL bIsContact, CString strPatientName,
							long GlassesOrderServiceID ,DiscountList *pDiscountList, CParamSqlBatch &batch ,CSqlFragment &sqlSave ) ;
// (s.dhole 2012-04-24 10:27) - PLID 43785 load discount 
DiscountList  *LoadServiceDiscount(long GlassesOrderServiceID);