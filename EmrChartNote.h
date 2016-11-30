//EmrChartNote.h

#ifndef EMRCHARTNOTE_H
#define EMRCHARTNOTE_H

#pragma once

#include "EmrCategoriesDlg.h"

class CEmrChartNote {
public:
	CEmrChartNote();
	CEmrChartNote(long nEmrID);
	virtual ~CEmrChartNote();
	void OutputToWord();

	// (c.haag 2004-04-30 13:14) - Generate a single HTML file for every
	// individual EMR category so they can be merged into a Word template.
	void GenerateTempMergeFiles();
	void RemoveTempMergeFiles();

protected:
	CStringArray m_astrTempFiles;
	long m_nEmrID;
	CString GetParagraph(long nCategoryID, EmrCategoryFormat Format);
	void GenerateTempMergeFile(long nCategoryID, EmrCategoryFormat fmt, const CString& strCatName);
};

#endif