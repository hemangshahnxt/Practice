#pragma once
#include "afxwin.h"


// CEmrProviderConfigDlg dialog
// (z.manning 2011-01-31 09:36) - PLID 42334 - Created

class CEmrProviderConfigDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEmrProviderConfigDlg)

public:
	CEmrProviderConfigDlg(CWnd* pParent);   // standard constructor
	virtual ~CEmrProviderConfigDlg();

// Dialog Data
	enum { IDD = IDD_EMR_PROVIDER_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	struct EmrProviderData
	{
		BOOL bFloat;
		long nFloatCount;
		long nFloatDays;

		EmrProviderData() {
			bFloat = FALSE;
			nFloatCount = 0;
			nFloatDays = 0;
		}

		bool operator==(const EmrProviderData& source) const {
			if(bFloat == source.bFloat && nFloatCount == source.nFloatCount && nFloatDays == source.nFloatDays) {
				return true;
			}
			return false;
		}

		bool operator!=(const EmrProviderData& source) const {
			return !(*this == source);
		}
	};

	NXDATALIST2Lib::_DNxDataListPtr m_pdlProviderCombo;
	enum EProviderComboColumns {
		pccID = 0,
		pccName,
		pccNewPointer,
		pccFloat,
		pccFloatCount,
		pccFloatDays,
	};

	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxColor m_nxcolor;
	CNxColor m_nxcolor2;

	void Load();
	void GetDataFromRow(LPDISPATCH lpRow, OUT EmrProviderData *pProvData);

	void HandleProviderChange();

	BOOL ValidateAndSave();

	void EnableControls(BOOL bEnable);

	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void RequeryFinishedEmrProviderConfigCombo(short nFlags);
	void SelChosenEmrProviderConfigCombo(LPDISPATCH lpRow);
	afx_msg void OnBnClickedEmrProviderFloatCheck();
	afx_msg void OnEnChangeEmrProviderFloatCount();
	afx_msg void OnEnChangeEmrProviderFloatDays();
	afx_msg void OnBnClickedEmrProviderManualUpdate(); // (z.manning 2011-04-19 16:47) - PLID 42337
	afx_msg void OnDestroy(); // (a.walling 2011-06-30 12:38) - PLID 44388
	CNxIconButton m_btnUpdateProviderFloatingSelections;
};
