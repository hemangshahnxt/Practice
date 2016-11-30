// Client.h: interface for the CClient class.
//-JMM 04/03  Whenever this file changes, you also have to change the client.h file in nxweb
// (j.armen 2011-10-26 15:07) - PLID 45795 - Cleaned up unused dead code
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENT_H__C135601F_2FA0_4B0E_9DAE_07AD33871705__INCLUDED_)
#define AFX_CLIENT_H__C135601F_2FA0_4B0E_9DAE_07AD33871705__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "GlobalTodoUtils.h"
#include "NxPackets.h"
#include "TCPSockets.h"
#include <NxNetworkLib\TableCheckerDetails.h>	// (a.wilson 2014-08-13 10:41) - PLID XXXXX

//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace, removed numerous NetUtils::
//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"

class TableCheckerNode;
class TableChecker;

class CTableChecker
{
public:
	CTableChecker(NetUtils::ERefreshTable table, bool bDiscardIfDisconnected = false, bool bDiscardIfConnected = false);
	virtual ~CTableChecker();
	bool Changed();
	bool PeekChanged() const; // (a.walling 2010-10-13 11:15) - PLID 40908 - Checks the changed state without resetting
	void Refresh(long id = -1);//BVB added 7/31/2001, see declaration for definition

	void MultiRefresh(CTableChecker **p, long id /*=-1*/);

//these should be protected, do not access them outside where they are meant to be
	NetUtils::ERefreshTable m_Table;
	bool m_changed;
	bool m_bDiscardIfDisconnected;
	bool m_bDiscardIfConnected;
	bool m_bExpectingSelfRefresh;
};

class TableCheckerNode
{
public:
	TableCheckerNode(CTableChecker *checker)
	{	data = checker;
		next = NULL;
	}

	TableCheckerNode *next;
	CTableChecker *data;
};

// (a.walling 2007-06-06 13:24) - PLID 26238 - Safe Table Checker class to ensure cleanup occurs on destruction
// (a.walling 2010-03-09 14:18) - PLID 37640 - Moved to Client.cpp / .h
class CSafeTableChecker {
	public:
		CSafeTableChecker();
		~CSafeTableChecker();

		//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"
		void Attach(NetUtils::ERefreshTable t);

		bool Attached();

		// will always return TRUE if uninitialized.
		bool Changed();
		
	protected:
		CTableChecker* m_pChecker;
};

// (j.jones 2014-08-20 10:42) - PLID 63427 - converted this to a namespace, because it has always been used as such
namespace CClient  
{
//	void RefreshTable(DWORD id, NetUtils::ERefreshTable table, NetUtils::PacketType type);
//	Abolutely CANNOT EVER use a different packet type for this message
//  Also, pass the id as a second param, defaulting to all (-1)
	// (a.walling 2008-06-04 13:36) - PLID 22049 - Allow an explicit "don't send to local" flag
	void RefreshTable(NetUtils::ERefreshTable table, DWORD id, CTableCheckerDetails* pDetails, BOOL bSendLocal = TRUE);
	void RefreshTable(NetUtils::ERefreshTable table, DWORD id = -1);

	//APPOINTMENTS

	// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
	// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
	void RefreshAppointmentTable(long nResID, long nPatientID, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime, long nStatus, long nShowState,
		long nLocationID, CString strResourceIDsCSV);
	void RefreshAppointmentTable(long nResID);
	//END APPOINTMENTS
	
	//ROOMAPPOINTMENTS

	void RefreshRoomAppointmentTable(long nRoomAppointmentID, long nRoomID, long nAppointmentID, long nStatus);
	void RefreshRoomAppointmentTable(long nRoomAppointmentID);
	//END ROOMAPPOINTMENTS

	//LABS
	// (r.gonet 09/02/2014) - PLID 63221 - Function to send an ex table checker for LabsT
	void RefreshLabsTable(long nPersonID, long nLabID);
	// (r.gonet 09/03/2014) - PLID 63540 - Function to send an ex table checker for LabsT. Overload
	// to be able to send some identifying string for the sender and also dictate the receiver
	// to ignore the table checker message if it is the sender.
	void RefreshLabsTable(CString strSenderID, BOOL bIgnoreSelf, long nPersonID, long nLabID);
	//END LABS

	//MAILSENT

	// (j.jones 2014-08-04 11:29) - PLID 63141 - This version of RefreshMailSentTable sends
	// an EX tablechecker when we have an enumeration stating that it is/is not a photo or
	// is unknown. Use this when we know it's never a photo, such as when saving reports to history.
	// Defaults to non-photo since most uses are not for photos.
	void RefreshMailSentTable(long nPersonID, long nMailSentID, TableCheckerDetailIndex::MailSent_PhotoStatus ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto);
	// (j.jones 2014-08-04 11:29) - PLID 63141 - This version of RefreshMailSentTable sends
	// an EX tablechecker when we have the value of MailSent.IsPhoto in varIsPhoto.
	// varIsPhoto is VARIANT_TRUE if we know it is, VARIANT_FALSE if we know it is not,
	// and NULL if we are not sure.
	void RefreshMailSentTable(long nPersonID, long nMailSentID, _variant_t varIsPhoto);
	// (j.jones 2014-08-04 11:29) - PLID 63141 - This version of RefreshMailSentTable sends an EX
	// tablechecker only after it loads the PersonID and IsPhoto fields from MailSent.
	// Use of this version should be avoided.
	void RefreshMailSentTable(long nMailSentID);
	//END MAILSENT



	//TODO
	// (s.tullis 2014-09-08 09:10) - PLID 63344 -
	 void RefreshTodoTable(long nTaskID,long personID, long nUserAssignedID, long todoStatus, BOOL bSendLocal=TRUE);
	//END TODO

	//BEGIN PATCOMBO
	//TES 8/13/2014 - PLID 63194 - Added EX tablecheckers for PatCombo. These are only used when a change is made that could affect a row's visibility in the combo,
	// meaning it was added or deleted, or the Active checkbox was changed, or the patient/prospect status was changed
	//NOTE: If you change the values, update NxServer's TryAddPatientToData function, which uses the equivalent numeric literals
	enum PatCombo_DetailIndex
	{
		pcdiPersonID = 1,
		pcdiDeleted,
		pcdiActive,
		pcdiStatus,
	};

	//TES 8/13/2014 - PLID 63194 - For the Active status, pcatUnchanged for cases where only the status was changed
	//NOTE: If you change the values, update NxServer's TryAddPatientToData function, which uses the equivalent numeric literals
	enum PatCombo_ActiveType
	{
		pcatUnchanged = 1,
		pcatActive,
		pcatInactive
	};

	//TES 8/13/2014 - PLID 63194 - For the patient/prospect status, pcstUnchanged for cases where only the Active status was changed
	//NOTE: If you change the values, update NxServer's TryAddPatientToData function, which uses the equivalent numeric literals
	enum PatCombo_StatusType
	{
		pcstUnchanged = 1,
		pcstPatient,
		pcstProspect,
		pcstPatientProspect,
	};
	void RefreshPatCombo(long nPersonID, bool bIsDeleted, PatCombo_ActiveType pcatActiveFlag, PatCombo_StatusType pcstStatus);
	//END PATCOMBO

	// END EXTENDED TABLE CHECKER REFRESH TABLE MESSAGES
	/////////////////////////////////////////////////////////////////////////////////

	// (b.cardillo 2006-07-06 16:16) - PLID 20737 - Sends an extended NetUtils::CustomFielName.
	void RefreshTable_CustomFieldName(long nCustomFieldID, const CString &strNewName);

	bool Send(PacketType pkttype, void* pData, DWORD dwSize);
	// (j.gruber 2010-07-16 15:08) - PLID 39643 - added ThreadID
	void SendIM(DWORD dwIP, long nRecipientID, COleDateTime dt, long nRegardingID, long nPriority, long nMessageGroupID, CString strMessage, long nID, long nSentBy, CString strSentBy, long nThreadID);
	//TES 6/9/2008 - PLID 23243 - Added parameters for the appointment ID, and its resource IDs.
	void SendAlert(const char* szMsg, long nAppointmentID, const CDWordArray &dwaResourceIDs, EAlert alert = GenericAlert);	
	void ProcessMessage(WPARAM wParam, LPARAM lParam);

	extern bool isConnected;	
	extern TableCheckerNode *pHead;
	//extern DWORD pData[TCP_MAX_DEFAULT_DATASIZE];
	//extern DWORD serverIP;

	void OnReceived(DWORD ip, void * pData, unsigned short wSize);

	// (c.haag 2008-09-11 10:03) - PLID 31333 - Added a parameter to send an acknowledgement packet to NxServer
	void OnRefreshTable(DWORD ip, PACKET_REFRESH_TABLE * pData, unsigned short wSize, BOOL bAckToNxServer);

	// (d.thompson 2009-07-07) - PLID 34803 - Logging
	void PLID34803_AddTableChecker(NetUtils::ERefreshTable ertTable);
	CString PLID34803_GetPrintableCString();

	// (j.jones 2014-08-20 10:42) - PLID 63427 - added a list of tablecheckers
	// that should always be processed immediately, and never queued up in the stagger
	bool IsImmediateTablechecker(NetUtils::ERefreshTable ertTable);

};	//end namespace

#endif // !defined(AFX_CLIENT_H__C135601F_2FA0_4B0E_9DAE_07AD33871705__INCLUDED_)