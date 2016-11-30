#pragma once

// (j.jones 2009-03-20 09:30) - PLID 9729 - used in m_arypMoreDiagCodes;
// (j.gruber 2014-03-21 12:51) - PLID 61494 - removed unused fields
// (j.armen 2014-03-26 11:27) - PLID 61517 - Moved to it's own file
struct DiagCodeInfo {

	// (b.savon 2014-04-01 06:46) - PLID 61613 - DiagCodeInfo struct members need to be initialized
	DiagCodeInfo(){
		nID = -1;
		nDiagCode9ID = -1;
		nDiagCode10ID = -1;
		nOrderIndex = -1;
	}

	long nID;	//the ID in BillExtraDiagCodesT	
	long nDiagCode9ID;
	long nDiagCode10ID;	
	CString strDiagCode9Code;
	CString strDiagCode10Code;	
	CString strDiagCode9Desc;
	CString strDiagCode10Desc;	
	long nOrderIndex;
};