#pragma once

class EMNProcedure {
public:
	long nID;
	BOOL bIsNew;
	CString strName;

	// (z.manning 2009-02-27 10:32) - PLID 33141 - Use the new source action info class
	// for all the source action/detail data.
	SourceActionInfo sai;

	EMNProcedure() {
		nID = -1;
		bIsNew = TRUE;
	}
};