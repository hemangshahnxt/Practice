#include "stdafx.h"

// (b.savon 2013-02-28 11:44) - PLID 54714 - Moved Colors to OMRUtils
#define OMR_ERROR		RGB(255,0,0)
#define OMR_SUCCESS		RGB(128,255,128)
#define OMR_EMPTY		RGB(255,140,0)

#define OMR_STANDARD_DETAIL		RGB(255,255,255)
#define OMR_ALTERNATE_DETAIL	RGB(245,245,245)

// (j.dinatale 2012-08-02 16:11) - PLID 51911 - created

namespace OMRUtils
{
	NexTech_COM::INxXmlGeneratorPtr ProcessXML(const CString &strNxXMLFilePath);

	// (b.spivey, August 20, 2012) - PLID 51721 - processing function for remark xml. 
	void RemarkOutputProcess(CString strPendingOMRPath, CString strRemarkXMLPath, DWORD dwBatchID, long nFormID); 

	// (j.dinatale 2012-08-09 16:59) - PLID 52060 - be able to commit NxXML via the API
	long CommitNxXMLToEMN(NexTech_COM::INxXmlGeneratorPtr pNxXml);

	// (j.dinatale 2012-08-21 16:52) - PLID 52284 - attach files to history
	bool AttachFilesFromNxXML(NexTech_COM::INxXmlGeneratorPtr pNxXml, long nEMNID);

	// (j.dinatale 2012-08-22 11:14) - PLID 52256 - be able to clean up our NxXML files, including the attached documents
	bool DeleteNxXMLFiles(const CString &strNxXMLPath, NexTech_COM::INxXmlGeneratorPtr pNxXml, bool bSkipAttachments = false);

	// (b.savon 2013-02-28 12:40) - PLID 54714 - Validation utility function
	OLE_COLOR GetOMRItemValidation(long nEMRDataType, long nCountSelected, bool &bWarning);

	// (b.savon 2013-02-28 13:42) - PLID 54714 - Color the alternating detail rows to easily distinguish what selection is made.
	inline OLE_COLOR GetOMRDetailRowColor(long nRow){ return (nRow%2==0) ? OMR_ALTERNATE_DETAIL : OMR_STANDARD_DETAIL; }
}