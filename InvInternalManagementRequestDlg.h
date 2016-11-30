//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
#pragma once

#include "PatientsRc.h"
#include "InventoryRc.h"

class CInvInternalManagementRequestDlg :
	public CNxDialog
{

	DECLARE_DYNAMIC(CInvInternalManagementRequestDlg)
public:
	CInvInternalManagementRequestDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvInternalManagementRequestDlg();

	//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
	virtual BOOL OnInitDialog();

	// Dialog Data
	enum { IDD = IDD_INV_INTERNAL_MANAGEMENT_REQUEST_DLG };

	//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
	void SetRequest(long);			// If -1, start a new request
	void SetCategory(long);			// If -1, no default category

protected:

	//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
	CDateTimePicker	m_DateFrom;
	CDateTimePicker	m_DateTo;
	// (j.fouts 2012-05-08 08:43) - PLID 50210 - Added indefinite items checkbox
	NxButton m_Indefinite;

	enum rtRequestTypes {
		rtNewRequest = -1
	};

	enum CatTypes {
		ctNoCategory = -1
	};

	enum ItemTypes {
		itNoItem = -1
	};

	enum RequestedByTypes {
		rbNoRequestedBy = -1
	};

	enum RecipientTypes {
		rtNoRecipient = -1
	};

	enum DefaultProductFilter {
		dpfAnyItem = -1
	};

	enum ESpecialRowIndex
	{
		sriNoRow = -1,
		sriNoRowYet_WillFireEvent = -2,
		sriGetNewRow = -1
	};

	enum UserList
	{
		ulID = 0,
		ulTrueUserName = 1,
		ulUser = 2
	};

	enum CategoryList
	{
		clID = 0,
		clName = 1
	};

	enum ItemList
	{
		ilID = 0,
		ilName = 1
	};

	enum DialogReturnTypes {
		drtDialogErrorNoReadExistingData = -1
	};

	NXDATALIST2Lib::_DNxDataListPtr m_UserCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_ProductCombo;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnBnClickedBtnCancel();
	
	DECLARE_MESSAGE_MAP()

	long m_nItemID;
	long m_nRequestID;
	long m_nCategoryID;
	long m_nExistingRecipient;
	long m_nServiceTCategory;
	long m_nItemStatus; // (j.luckoski 2012-04-23 09:04) - PLID 49305 - Store item status

	//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
	void SelChosenCategoryList(LPDISPATCH lpRow);
	void SelChosenInvRequestCategoryCombo(LPDISPATCH lpRow);
	void UpdateItemNotes();
	void SelChosenInvRequestSelectedItemCombo(LPDISPATCH lpRow);
	void AddAnyItemEntry();
	void RemoveAnyItemEntry();
	void ApplyPermissions(long nRequestedBy);
	void InitialQueryProductCombo();

public:
	afx_msg void OnBnClickedInvManagementMakeRequestButton();
	DECLARE_EVENTSINK_MAP()

	void RequeryFinishedInvRequestUserCombo(short nFlags);
	void RequeryFinishedInvRequestSelectedItemCombo(short nFlags);
	void RequeryFinishedInvRequestCategoryCombo(short nFlags);
	afx_msg void OnBnClickedIndefinite();
};
