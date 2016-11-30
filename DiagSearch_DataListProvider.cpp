// (d.thompson 2014-01-31) - PLID 60607 - Created.  This is the base class and should not have much in it.  Please derive your own
//	implementations from here.
#include "StdAfx.h"
#include "DiagSearch_DataListProvider.h"
#include "PracticeRc.h"

using namespace ADODB;

// (j.jones 2014-03-03 15:01) - PLID 61136 - added the diag PCS subpreference
CDiagSearch_DataListProvider::CDiagSearch_DataListProvider(bool bIncludePCS)
	: m_bIncludePCS(bIncludePCS)
{
	m_hQuicklistIcon = NULL;
}

CDiagSearch_DataListProvider::~CDiagSearch_DataListProvider(void)
{
	// (z.manning 2014-03-05 10:56) - PLID 61140 - Handle the quicklist icon
	if (m_hQuicklistIcon != NULL) {
		DestroyIcon(m_hQuicklistIcon);
		m_hQuicklistIcon = NULL;
	}
}

// (z.manning 2014-03-05 10:56) - PLID 61140
HICON CDiagSearch_DataListProvider::GetQuicklistIcon()
{
	if (m_hQuicklistIcon == NULL) {
		// (z.manning 2014-03-03 13:49) - PLID 61140 - Set the quick list icon
		m_hQuicklistIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_STAR_FILLED), IMAGE_ICON, 16, 16, 0);
	}
	return m_hQuicklistIcon;
}