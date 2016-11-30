#if !defined(AFX_ADVHCFAPINSETUP_H__5091DC9F_9CAA_43DE_9FBB_04BDE2D5057D__INCLUDED_)
#define AFX_ADVHCFAPINSETUP_H__5091DC9F_9CAA_43DE_9FBB_04BDE2D5057D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvHCFAPinSetup.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CAdvHCFAPinSetup dialog

class CAdvHCFAPinSetup : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_ProvidersCombo, m_LocationsCombo;

	BOOL m_bLocationsLoaded;
	BOOL m_bProvidersLoaded;
	BOOL Save(); // (a.walling 2008-05-23 14:30) - PLID 27810 - Cancelable
	void Load();
	CAdvHCFAPinSetup(CWnd* pParent);   // standard constructor

	long m_HCFAGroup;

	// (j.jones 2007-04-05 09:06) - PLID 25496 - track these values to help us
	// determine whether to warn and save when the provider or location changes
	long m_ProviderID;
	long m_LocationID;
	BOOL m_bHasChanged;
	BOOL m_bHas33bQualChanged;	// (j.jones 2010-04-16 08:46) - PLID 38149
	BOOL m_bIsLoading;

	// (z.manning, 04/30/2008) - PLID 29850 - Added NxIconButtons
	// (j.jones 2008-09-10 11:02) - PLID 30788 - added SSN/EIN/None radio buttons
// Dialog Data
	//{{AFX_DATA(CAdvHCFAPinSetup)
	enum { IDD = IDD_ADV_HCFA_PIN_SETUP };
		// NOTE: the ClassWizard will add data members here
	NxButton	m_radioSSN;
	NxButton	m_radioEIN;
	NxButton	m_radioNoOverride;
	CNxEdit	m_nxeditAdvBox33Name;
	CNxEdit	m_nxeditAdvBox19;
	CNxEdit	m_nxeditBox24jQual;
	CNxEdit	m_nxeditAdvBox24j;
	CNxEdit	m_nxeditAdvBox24jNpi;
	CNxEdit	m_nxeditAdvEin;
	CNxEdit	m_nxeditAdvBox33aNpi;
	CNxEdit	m_nxeditBox33Qual;
	CNxEdit	m_nxeditPin;
	CNxEdit	m_nxeditAdvGrp;
	CNxStatic	m_nxstaticHcfaGroupName;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
	CNxEdit m_nxeditBox33Address1;
	CNxEdit m_nxeditBox33Address2;
	CNxEdit m_nxeditBox33City;
	CNxEdit m_nxeditBox33State;
	CNxEdit m_nxeditBox33Zip;
	// (j.jones 2011-09-23 17:40) - PLID 28441 - added named GRP label
	CNxStatic m_nxstaticGRPLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvHCFAPinSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// (a.walling 2008-05-19 17:32) - PLID 27810 - We don't warn unless the ID has been changed.
	CString m_strOriginal24JNPI;
	CString m_strOriginal33aNPI;

	CBrush m_brush;

	// (b.eyers 2015-04-09) - PLID 59169
	BOOL m_bLookupByCity;

	// Generated message map functions
	//{{AFX_MSG(CAdvHCFAPinSetup)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelChosenProvidersCombo(long nRow);
	afx_msg void OnSelChosenLocationsCombo(long nRow);
	afx_msg void OnRequeryFinishedProvidersCombo(short nFlags);
	afx_msg void OnRequeryFinishedLocationsCombo(short nFlags);
	afx_msg void OnKillfocusZipBox(); // (b.eyers 2015-04-09) - PLID 59169
	afx_msg void OnKillfocusCityBox(); // (b.eyers 2015-04-09) - PLID 59169
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVHCFAPINSETUP_H__5091DC9F_9CAA_43DE_9FBB_04BDE2D5057D__INCLUDED_)
