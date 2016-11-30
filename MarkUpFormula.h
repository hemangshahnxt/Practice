#pragma once
//r.wilson
//Class MarkUpFormula created for plid 46664 on 3/9/2012
class CMarkUpFormula
{
public:
	CMarkUpFormula(void);
	CMarkUpFormula(long id, CString formula, long roundUp);
	~CMarkUpFormula(void);

	//r.wilson 3/9/2012 PLID 46664
	long m_nID;
	CString m_strFormula;
	// (b.eyers 2016-03-14) - PLID 68590
	long m_nRoundUpRule;

};
