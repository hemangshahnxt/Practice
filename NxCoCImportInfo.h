#pragma once

//DRT 10/28/2008 - PLID 31789 - Created for NxCoC Importer

#include "NxCoCUtils.h"
#include "ShowConnectingFeedbackDlg.h"
#include "ShowProgressFeedbackDlg.h"

//Method of import to be used.  See descriptive text in resources for each.
enum eImportType {
	eitUnknown = -1,
	eUsePIC = 0,
	eUseLW,
};

class CNxCoCWizardMasterDlg;

//Generic data class for housing information regarding the CoC import data.  This does all parsing, holds the 
//	parsed data, and performs the input into the database.
class CNxCoCImportInfo
{
public:
	CNxCoCImportInfo(void);
	~CNxCoCImportInfo(void);


	//Loading functionality
	bool ReadVersionInfoFromNxCoCContentFile(CString strFile, long& nContentFileVersion, long& nNxCoCVersion, COleDateTime& dtDateWritten);
	bool LoadFromNxCoCContentFile(CString strFile, CShowProgressFeedbackDlg *pProgressDlg);

	//Public methods
	void SetType(eImportType eit);
	CArray<CNxCoCPacket, CNxCoCPacket&>* GetPacketArrayPtr()	{	return &m_aryPackets;	}
	CArray<CNxCoCTemplate, CNxCoCTemplate&>* GetTemplateArrayPtr()	{	return &m_aryTemplates;	}

	//Importing
	// (z.manning 2009-04-09 12:59) - PLID 33934 - Changed the parent to be a CNxCoCWizardMasterDlg
	bool DoNxCoCImport(CNxCoCWizardMasterDlg *pwndParent, CShowConnectingFeedbackDlg *pFeedback);
	void ClearTempPath();

	//Public members
	HANDLE m_hevDestroying;

protected:
	//Protected members
	long m_nContentFileVersion;
	long m_nNxCoCVersion;
	COleDateTime m_dtDateWritten;
	CString m_strTempPath;

	//Packet data
	CArray<CNxCoCPacket, CNxCoCPacket&> m_aryPackets;
	//Template data
	CArray<CNxCoCTemplate, CNxCoCTemplate&> m_aryTemplates;
	//Method of import
	eImportType m_eitImportType;

	//File Parsing Functionality
	bool ReadPackets(CFile *pFile, CArray<CNxCoCPacket, CNxCoCPacket&> *paryPackets);
	bool ReadTemplates(CFile *pFile, CArray<CNxCoCTemplate, CNxCoCTemplate&> *paryTemplates, CShowProgressFeedbackDlg *pProgressDlg);

};
