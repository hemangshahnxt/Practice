 // r.wilson (2011-4-8) - PLID 43188 - CREATED BY ..
#pragma once
#include "InventoryRc.h"

const long cnAllLocations = -1;
const long cnAllCategories = -1;
const long cnAllOwners = -1;


// CInvInternalManagementDlg dialog

// (j.armen 2011-06-02 16:36) - PLID 43259 - Updated the enum to have hard coded values
enum ManagementStatusType {
	mstAll = -1,
	mstPending = 0,
	mstDenied = 1,
	mstApproved = 2 ,
	mstCheckedOut = 3
};

class CInvInternalManagementDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CInvInternalManagementDlg)

public:
	CInvInternalManagementDlg(CWnd* pParent);   // standard constructor
	void RequeryMainDataList();
	virtual ~CInvInternalManagementDlg();
	

	NXDATALIST2Lib::_DNxDataListPtr m_StatusList; //list of item statuses
	NXDATALIST2Lib::_DNxDataListPtr m_OwnerCombo;  //list of departments
	NXDATALIST2Lib::_DNxDataListPtr m_StatusCombo; //list of different statuses
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo; // list of different Categories
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo; // list of differnet locations

// Dialog Data
	enum { IDD = IDD_INV_INTERNAL_MANAGEMENT_DLG };

	NxButton m_MyRequests;
	NxButton m_Filters;
	// (j.luckoski 2012-04-02 14:50) - PLID 48195 - Added in m_History to handle show history checkbox events.
	NxButton m_History;
	// (j.fouts 2012-05-08 08:43) - PLID 50210 - Added a filter on indefinite items
	NxButton m_Indefinite;
	CNxIconButton m_CreateRequest;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	long nNEXTECH_PARENT_CATEGORY;

	BOOL bHasApproveRequest;
	BOOL bHasDenyRequest;
	BOOL bDragAllowed;
	BOOL bLoadedParentCategoryID; //PLID 43188 r.wilson 6/10/2011
	BOOL IsAdmin(long nProductID, long nCategoryID, BOOL& bIsAny); // (j.luckoski 2012-03-29 17:03) - PLID 49305 - Added in function to return IsAllowAdminFeatures;

	CString RefilterStatusList(BOOL bVersion2);

	// (j.armen 2011-06-02 16:36) - PLID 43259 - Build the where clause for filters
	CString BuildWhere();
	 /* r.wilson PLID 43353 5/13/2011 -> See the CPP file for description*/
	CString RefilterStatusListANY();
	void BuildDepartmentCombo();
	void BuildStatusCombo();
	
	virtual BOOL OnInitDialog();
	afx_msg void RButtonUpStatusList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnApproveRequest();
	afx_msg void OnDenyRequest();
	afx_msg void OnEditRequest();
	afx_msg void OnCheckOutItem();
	afx_msg void OnCheckItemIn();
	afx_msg void OnDeleteRequest();
	afx_msg void OnBnClickedBtnRequest();
	afx_msg void OnChangeMyRequests();
	// (j.luckoski 2012-04-02 12:23) - PLID 48195 - Provide a fucntion to handle show history button.
	afx_msg void OnShowHistory();
	afx_msg void OnSelChosenInvManagementOwnerCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenInvManagementCategoryCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenInvManagementLocationCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenInvManagementStatusCombo(LPDISPATCH lpRow);
	/*r.wilson   4/26/2011 (PLID  43470) */
	//afx_msg void EditingFinishedStatusDataList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void FireDragBegin(BOOL FAR* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags);
	afx_msg void OnRequeryFinishedStatusList(short nFlags);
	//void ToggleSelectedItemActiveInDatabase(LPDISPATCH lpRow);
	// (j.luckoski 2012-03-29 10:49) - PLID 49279 - Function OnRememberFilters to remember filters :) // (j.luckoski 2012-03-29 10:54) - PLID 
	afx_msg void OnRememberFilters();
	afx_msg void OnSignRequest();
	DECLARE_EVENTSINK_MAP()

	DECLARE_MESSAGE_MAP()

	
public:
	void DragEndInventoryStatusList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags);
	/* (r.wilson) 5/2/2011 PL ID = 43353 */
	BOOL CInvInternalManagementDlg::DoRequestDateTimesConflict(int nProductID, int nRequestID,COleDateTime dtFrom, COleDateTime dtTo, BOOL bIndefinite);
	afx_msg void OnBnClickedHideIndefinite();
};