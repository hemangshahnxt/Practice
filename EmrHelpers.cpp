#include "StdAfx.h"
#include "EmrHelpers.h"

#include "EMN.h"

#include <NxUILib/WindowUtils.h>

namespace Emr
{

// (a.walling 2012-07-03 10:56) - PLID 51284 - Update the title (now requires pointer)
void AttachedEMNImpl::UpdateTitle(CWnd* pThis)
{
	if (m_pAttachedEMN) {
		COleDateTime dtEMN = m_pAttachedEMN->GetEMNDate();
		COleDateTime dtNow = COleDateTime::GetCurrentTime();

		CString strDescription = m_pAttachedEMN->GetDescription();
		strDescription.Replace("\r", "");
		strDescription.Replace("\n", " ");

		if (dtEMN.GetDay() != dtNow.GetDay() || (abs((dtNow - dtEMN).GetTotalHours()) > 24)) {
			strDescription.AppendFormat(" (%s)", FormatDateTimeForInterface(dtEMN));
		}

		WindowUtils::NxSetWindowText(pThis->GetParentFrame(), strDescription);
	}
}

}