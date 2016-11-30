#include "stdafx.h"
#include "MarkUpFormula.h"

CMarkUpFormula::CMarkUpFormula(void)
{
	m_nID = -1;
	m_strFormula = "";
	// (b.eyers 2016-03-14) - PLID 68590
	m_nRoundUpRule = 0;
}

CMarkUpFormula::CMarkUpFormula(long id, CString formula, long roundUp)
{
	m_nID = id;
	m_strFormula = formula;
	// (b.eyers 2016-03-14) - PLID 68590
	m_nRoundUpRule = roundUp;
}

CMarkUpFormula::~CMarkUpFormula(void)
{
}
