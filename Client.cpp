//-JMM 04-03  This file is used in nxweb and any changes made to it have to be copied there as well
// (j.armen 2011-10-26 15:06) - PLID 45795 - Cleaned up some unused dead code
#include "stdafx.h"
#include <stdarg.h>

// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "practice.h"
#include "Client.h"
#include "tcpsockets.h"
#include "RegUtils.h"
#include "backup.h"
//TES 10/20/03: NOTE: this include is not in nxweb, because it's only used by Yak stuff, which is
//commented out in the nxweb version of this file (which in fact is different).
#include "nxmessagedef.h"
// (c.haag 2005-10-31 15:15) - PLID 16595 - Added a dependency on NxPropManager
#include "nxpropmanager.h"
extern CNxPropManager g_propManager;
#include "NxPackets.h"
#include <NxPracticeSharedLib/PracData.h>
#include <NxPracticeSharedLib/PracDataEmr.h>

using namespace ADODB;

//DRT 6/15/2007 - PLID 25531 - Packets are no longer part of NetUtils namespace, removed numerous NetUtils::

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////
// CTableChecker class

CTableChecker::CTableChecker(NetUtils::ERefreshTable table, bool bDiscardIfDisconnected, bool bDiscardIfConnected)
{
	TableCheckerNode *p = new TableCheckerNode(this);
	
	m_Table = table;
	m_bDiscardIfDisconnected = bDiscardIfDisconnected;
	m_bDiscardIfConnected = bDiscardIfConnected;
	m_changed = false;
	m_bExpectingSelfRefresh = false;

	if (CClient::pHead == NULL)
		CClient::pHead = p;
	else
	{	TableCheckerNode *q = CClient::pHead;
		while (q->next)
			q = q->next;
		q->next = p;
	}
}

CTableChecker::~CTableChecker()
{
	TableCheckerNode *p = CClient::pHead, *q;

	//our current item still exists if we got here, so the list cannot be empty
	if (!p)
	{	ASSERT(FALSE);
		return;
	}

	if (p->data == this)//item is the head of the list
	{	CClient::pHead = p->next;
		delete p;
		return;
	}

	while (p->next)
	{	if (p->next->data == this)
		{	q = p->next;
			p->next = p->next->next;
			delete q;
			return;
		}
		p = p->next;
	}

	ASSERT(FALSE);//item not found
}

bool CTableChecker::Changed()
//returns whether or not the table was changed since we last called CTableChecker::Changed()
{
	bool result;

//	if (!CClient::isConnected)
//		return true;//if we can't tell if someone else changed it, assume they did

	result = m_changed;
	m_changed = false;
	return result;
}

// (a.walling 2010-10-13 11:15) - PLID 40908 - Checks the changed state without resetting
bool CTableChecker::PeekChanged() const
{
	return m_changed;
}

void CTableChecker::Refresh(long id /*=-1*/)
/*BVB added 7/31/2001
Call Refresh from the owner of the table checker
whenever the owner changes the data assosiated with the checker
This will refresh the other table checkers (including other machines)
And not mark us changed becuase of the change (change state stays the same)
*/

{
	if (!m_changed)
		m_bExpectingSelfRefresh = true;
	CClient::RefreshTable(m_Table, id);
}

//same as refresh, but to handle cases where we own mutiple checkers
void CTableChecker::MultiRefresh(CTableChecker **p, long id /*=-1*/)
{
	CTableChecker *q;

	if (!m_changed)
		m_bExpectingSelfRefresh = true;

	while (*p)
	{
		q = *p;
		if (!q->m_changed)
			q->m_bExpectingSelfRefresh = true;

		//we can only muti-refresh checkers of the same type
		ASSERT(q->m_Table == m_Table);

		p++;
	}

	CClient::RefreshTable(m_Table, id);
}

// (a.walling 2010-03-09 14:18) - PLID 37640 - Moved to Client.cpp / .h
CSafeTableChecker::CSafeTableChecker() :
	m_pChecker(NULL)
{
};

CSafeTableChecker::~CSafeTableChecker()
{
	if (m_pChecker != NULL) {
		delete m_pChecker;
	}
}

//DRT 6/15/2007 - PLID 25531 - We now use the more descriptive ERefreshTable instead of just "Table"
void CSafeTableChecker::Attach(NetUtils::ERefreshTable t)
{
	ASSERT(m_pChecker == NULL);
	
	if (m_pChecker == NULL)
		m_pChecker = new CTableChecker(t);
};

bool CSafeTableChecker::Attached()
{
	return m_pChecker != NULL;
};

// will always return TRUE if uninitialized.
bool CSafeTableChecker::Changed()
{
	if (m_pChecker == NULL)
		return TRUE;
	else
		return m_pChecker->Changed();
};

///////////////////////////////////////////////////////////////////////////////////////
//CClient namespace

// (j.jones 2014-08-20 10:42) - PLID 63427 - converted this to a namespace, because it has always been used as such

namespace CClient
{

	///////////////////////////////////////////////////////////////////////////////////////
	//static data members
	bool isConnected = false;	
	TableCheckerNode *pHead = NULL;
	//DWORD pData[TCP_MAX_DEFAULT_DATASIZE];
	//DWORD serverIP = 0;

	///////////////////////////////////////////////////////////////////////////////////////
	//functions

	// (a.walling 2008-06-04 13:36) - PLID 22049 - Allow an explicit "don't send to local" flag
	void RefreshTable(NetUtils::ERefreshTable table, DWORD id, CTableCheckerDetails* pDetails, BOOL bSendLocal)
	{
		// (b.cardillo 2006-07-05 17:43) - PLID 21336 - Allocate a block of memory of the 
		// approriate size now that the framework supports packets of unlimited size
		char *szData = new char[sizeof(_NxHeader) + sizeof(PACKET_REFRESH_TABLE) + (pDetails ? pDetails->GetFormattedPacketDataSize() : 0)];
		try {
			_NxHeader* pHdr = (_NxHeader*)szData;
			PACKET_REFRESH_TABLE* pPkt = (PACKET_REFRESH_TABLE*)(pHdr + 1);
			void* pDetailData = (void*)(pPkt + 1);

			pHdr->dwVer = g_dwCurrentNxPacketVersion;
			pHdr->dwKey = 0;
			pHdr->wSize = sizeof(_NxHeader) + sizeof(PACKET_REFRESH_TABLE) +
				(pDetails ? pDetails->GetFormattedPacketDataSize() : 0);
			pHdr->type = PACKET_TYPE_REFRESH_TABLE;
			if (pDetails) memcpy(pDetailData, pDetails->GetFormattedPacketData(), pDetails->GetFormattedPacketDataSize());

			pPkt->ip = GetHostIP().S_un.S_addr;
			pPkt->table = table;
			pPkt->id = id;

			// Process locally if you're not connected to the server
			//TES 10/8/2007 - PLID 27641 - New plan: Just go ahead and process it locally before trying to send it to the server.
			// After all, why make that extra round trip if we don't have to?
			if (bSendLocal) {
				// (c.haag 2008-09-11 10:05) - PLID 31333 - Set the last parameter to FALSE so it will not send a packet to NxServer when finished
				OnRefreshTable(0, pPkt, sizeof(_NxHeader) + sizeof(PACKET_REFRESH_TABLE) + (pDetails ? pDetails->GetFormattedPacketDataSize() : 0), FALSE);//process locally - without sending over the net
			}
			Send(PACKET_TYPE_REFRESH_TABLE, pPkt, sizeof(PACKET_REFRESH_TABLE) + (pDetails ? pDetails->GetFormattedPacketDataSize() : 0));

		}
		catch (...) {
			// First free the memory we allocated
			delete[] szData;
			// Then let the exception fly
			throw;
		}

		// Free the memory we allocated
		delete[] szData;
	}

	void RefreshTable(NetUtils::ERefreshTable table, DWORD id /*=-1*/)
	{
		RefreshTable(table, id, NULL);
	}

	// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
	// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
	void RefreshAppointmentTable(long nResID, long nPatientID, const COleDateTime& dtStartTime, const COleDateTime& dtEndTime, long nStatus, long nShowState,
		long nLocationID, CString strResourceIDs)
	{
		// (j.jones 2014-08-05 10:34) - PLID 63167 - this now uses an enumeration
		CTableCheckerDetails dtls;
		dtls.AddDetail(TableCheckerDetailIndex::adiAppointmentID, nResID);
		dtls.AddDetail(TableCheckerDetailIndex::adiStartTime, dtStartTime);
		dtls.AddDetail(TableCheckerDetailIndex::adiEndTime, dtEndTime);
		dtls.AddDetail(TableCheckerDetailIndex::adiStatus, nStatus);
		dtls.AddDetail(TableCheckerDetailIndex::adiShowState, nShowState);
		dtls.AddDetail(TableCheckerDetailIndex::adiPatientID, nPatientID);
		dtls.AddDetail(TableCheckerDetailIndex::adiLocationID, nLocationID);
		//this is space-delimited, not CSV, to represent dbo.GetResourceIDString,
		//so replace commas now incase somebody mistakenly sent them
		strResourceIDs.Replace(",", " ");
		dtls.AddDetail(TableCheckerDetailIndex::adiResourceIDs, strResourceIDs);
		RefreshTable(NetUtils::AppointmentsT, nResID, &dtls);
	}

	void RefreshAppointmentTable(long nResID)
	{
		// (a.walling 2013-06-18 11:14) - PLID 57197 - Parameterize CClient's RefreshAppointmentTable and RefreshRoomAppointmentTable extended table checker notifiers
		// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
		_RecordsetPtr prs = CreateParamRecordset("SELECT PatientID, StartTime, EndTime, Status, ShowState, "
			"LocationID, dbo.GetResourceIDString(ID) AS ResourceIDs FROM AppointmentsT WHERE ID = {INT}", nResID);
		if (!prs->eof) {
			// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
			// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
			RefreshAppointmentTable(nResID, AdoFldLong(prs, "PatientID"), AdoFldDateTime(prs, "StartTime"), AdoFldDateTime(prs, "EndTime"), AdoFldByte(prs, "Status"),
				AdoFldLong(prs, "ShowState"), AdoFldLong(prs, "LocationID"), AdoFldString(prs, "ResourceIDs", ""));
		}
		else {
			// (a.wilson 2014-08-13 09:30) - PLID 63199 - this should never happen since we changed all tablecheckers for appointmentst to extablecheckers.
			// The only time the appointment wouldn't exist is if it was deleted in which case we changed all places that delete appointments to send all necessary information
			// and skips this function.
			ASSERT(FALSE);
			//RefreshTable(NetUtils::AppointmentsT, nResID);
		}
	}

	void RefreshRoomAppointmentTable(long nRoomAppointmentID, long nRoomID, long nAppointmentID, long nStatus)
	{
		// (j.jones 2014-08-05 10:34) - PLID 63167 - this now uses an enumeration
		CTableCheckerDetails dtls;
		dtls.AddDetail(TableCheckerDetailIndex::radiRoomAppointmentID, nRoomAppointmentID);
		dtls.AddDetail(TableCheckerDetailIndex::radiRoomID, nRoomID);
		dtls.AddDetail(TableCheckerDetailIndex::radiAppointmentID, nAppointmentID);
		dtls.AddDetail(TableCheckerDetailIndex::radiStatus, nStatus);
		RefreshTable(NetUtils::RoomAppointmentsT, nRoomAppointmentID, &dtls);
	}

	void RefreshRoomAppointmentTable(long nRoomAppointmentID)
	{
		// (a.walling 2013-06-18 11:14) - PLID 57197 - Parameterize CClient's RefreshAppointmentTable and RefreshRoomAppointmentTable extended table checker notifiers
		_RecordsetPtr prs = CreateParamRecordset("SELECT RoomID, AppointmentID, StatusID FROM RoomAppointmentsT WHERE ID = {INT}", nRoomAppointmentID);
		if (!prs->eof) {
			RefreshRoomAppointmentTable(nRoomAppointmentID, AdoFldLong(prs, "RoomID"), AdoFldLong(prs, "AppointmentID"), AdoFldLong(prs, "StatusID"));
		}
		else {
			RefreshTable(NetUtils::RoomAppointmentsT, nRoomAppointmentID);
		}
	}

	// (r.gonet 09/02/2014) - PLID 63221 - Function to send an ex table checker for LabsT
	void RefreshLabsTable(long nPersonID, long nLabID)
	{
		CTableCheckerDetails details;
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::SenderID, _variant_t(_bstr_t("")));
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::IgnoreSelf, _variant_t(0L, VT_I4));
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::PatientID, _variant_t(nPersonID, VT_I4));
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::LabID, _variant_t(nLabID, VT_I4));
		RefreshTable(NetUtils::LabsT, nLabID, &details);
	}

	// (r.gonet 09/02/2014) - PLID 63540 - Function to send an ex table checker for LabsT. Overload
	// to be able to send some identifying string for the sender and also dictate the receiver
	// to ignore the table checker message if it is the sender.
	void RefreshLabsTable(CString strSenderID, BOOL bIgnoreSelf, long nPersonID, long nLabID)
	{
		CTableCheckerDetails details;
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::SenderID, _variant_t(_bstr_t(strSenderID)));
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::IgnoreSelf, (bIgnoreSelf ? _variant_t(1L, VT_I4) : _variant_t(0L, VT_I4)));
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::PatientID, _variant_t(nPersonID, VT_I4));
		details.AddDetail((short)TableCheckerDetailIndex::Labs_DetailIndex::LabID, _variant_t(nLabID, VT_I4));
		RefreshTable(NetUtils::LabsT, nLabID, &details);
	}
	

	// (j.jones 2014-08-04 11:29) - PLID 63141 - This version of RefreshMailSentTable sends
	// an EX tablechecker when we have an enumeration stating that it is/is not a photo or
	// is unknown. Use this when we know it's never a photo, such as when saving reports to history.
	// Defaults to non-photo since most uses are not for photos.
	void RefreshMailSentTable(long nPersonID, long nMailSentID, TableCheckerDetailIndex::MailSent_PhotoStatus ePhotoStatus /*= mspsIsNonPhoto*/)
	{
		CTableCheckerDetails dtls;
		dtls.AddDetail(TableCheckerDetailIndex::msdiPatientID, nPersonID);
		dtls.AddDetail(TableCheckerDetailIndex::msdiMailSentID, nMailSentID);
		dtls.AddDetail(TableCheckerDetailIndex::msdiPhotoStatus, (long)ePhotoStatus);
		RefreshTable(NetUtils::MailSent, nMailSentID, &dtls);
	}

	// (j.jones 2014-08-04 11:29) - PLID 63141 - This version of RefreshMailSentTable sends
	// an EX tablechecker when we have the value of MailSent.IsPhoto in varIsPhoto.
	// varIsPhoto is VARIANT_TRUE if we know it is, VARIANT_FALSE if we know it is not,
	// and NULL if we are not sure.
	void RefreshMailSentTable(long nPersonID, long nMailSentID, _variant_t varIsPhoto)
	{
		//varIsPhoto is our internal bit that is either true, false, or null if unknown.
		//Convert this to our local enumeration.
		TableCheckerDetailIndex::MailSent_PhotoStatus ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsUnknown;
		if (varIsPhoto.vt == VT_BOOL) {
			if (VarBool(varIsPhoto)) {
				ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto;
			}
			else {
				ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto;
			}
		}
		else if (varIsPhoto.vt == VT_I4) {
			if (VarLong(varIsPhoto) == 1) {
				ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsPhoto;
			}
			else {
				ePhotoStatus = TableCheckerDetailIndex::MailSent_PhotoStatus::mspsIsNonPhoto;
			}
		}

		RefreshMailSentTable(nPersonID, nMailSentID, ePhotoStatus);
	}

	// (j.jones 2014-08-04 11:29) - PLID 63141 - This version of RefreshMailSentTable sends an EX
	// tablechecker only after it loads the PersonID and IsPhoto fields from MailSent.
	// Use of this version should be avoided.
	void RefreshMailSentTable(long nMailSentID)
	{
		_RecordsetPtr prs = CreateParamRecordset("SELECT PersonID, IsPhoto FROM MailSent WHERE MailID = {INT}", nMailSentID);
		if (!prs->eof) {
			RefreshMailSentTable(AdoFldLong(prs, "PersonID"), nMailSentID, prs->Fields->Item["IsPhoto"]->Value);
		}
		else {
			RefreshTable(NetUtils::MailSent, nMailSentID);
		}
		prs->Close();
	}

	// (b.cardillo 2006-07-06 16:16) - PLID 20737 - Sends an extended NetUtils::CustomFielName.
	void RefreshTable_CustomFieldName(long nCustomFieldID, const CString &strNewName)
	{
		// Set up the payload to contain the field id and the new name
		CTableCheckerDetails dtls;
		dtls.AddDetail(1, (long)nCustomFieldID);
		dtls.AddDetail(2, (LPCTSTR)strNewName);
		// And send the extended message
		RefreshTable(NetUtils::CustomFieldName, nCustomFieldID, &dtls);
	}

	//TES 8/13/2014 - PLID 63194 - Added EX tablecheckers for PatCombo. These are only used when a change is made that could affect a row's visibility in the combo,
	// meaning it was added or deleted, or the Active checkbox was changed, or the patient/prospect status was changed
	void RefreshPatCombo(long nPersonID, bool bIsDeleted, PatCombo_ActiveType pcatActiveFlag, PatCombo_StatusType pcstStatus)
	{
		CTableCheckerDetails dtls;
		dtls.AddDetail(pcdiPersonID, nPersonID);
		dtls.AddDetail(pcdiDeleted, bIsDeleted ? (long)1 : (long)0);
		dtls.AddDetail(pcdiActive, (long)pcatActiveFlag);
		dtls.AddDetail(pcdiStatus, (long)pcstStatus);
		RefreshTable(NetUtils::PatCombo, nPersonID, &dtls);
	}

	// (s.tullis 2014-08-19 17:06) - PLID 63344, 63674 - Need a Fuction to send EX Todos
void RefreshTodoTable(long nTaskID,long personID, long nUserAssignedID, long todoStatus, BOOL bSendLocal){


		CTableCheckerDetails dtls;
		dtls.AddDetail(TableCheckerDetailIndex::tddiTaskID, nTaskID);
		dtls.AddDetail(TableCheckerDetailIndex::tddiPersonID, personID);
		dtls.AddDetail(TableCheckerDetailIndex::tddiAssignedUser, nUserAssignedID);
		dtls.AddDetail(TableCheckerDetailIndex::tddiTodoStatus, todoStatus);
		RefreshTable(NetUtils::TodoList, nTaskID, &dtls, bSendLocal);

	}

	// (c.haag 2008-09-11 10:03) - PLID 31333 - Added a parameter to send an acknowledgement packet to NxServer
	void OnRefreshTable(DWORD ip /* IP address of NxServer, or 0 if this is called locally (See RefreshTable above) */,
		PACKET_REFRESH_TABLE * pData, unsigned short wSize,
		BOOL bAckToNxServer)
	{
		TableCheckerNode *p;
		p = pHead;

		while (p)
		{
			if (p->data->m_Table == (NetUtils::ERefreshTable)pData->table)
			{
				DWORD dwSrcClientIP = GetHostIP().S_un.S_addr;

				//if the client is connected
				if (NULL != GetMainFrame()->GetNxServerSocket())
				{
					//if it is from our own IP
					if (pData->ip == dwSrcClientIP)
					{
						//if we are expecting to refresh ourselves, don't mark changed
						if (p->data->m_bExpectingSelfRefresh)
							p->data->m_bExpectingSelfRefresh = false;
						//otherwise, mark it changed only if we want our own packets
						else if (!p->data->m_bDiscardIfConnected)
							p->data->m_changed = true;
					}
					//if it is from someone else, we changed
					else p->data->m_changed = true;
				}
				//if we aren't disconnected, mark changed if we don't disregard emulated packets
				else if (!p->data->m_bDiscardIfDisconnected)
					p->data->m_changed = true;
			}

			p = p->next;
		}

		// (d.thompson 2009-07-07) - PLID 34803 - Log the last 25 tablecheckers
		// PLID34803_AddTableChecker(pData->table);
		// (a.walling 2009-07-14 12:29) - PLID 33644 - Moved to the actual handling of the
		// table checkers, since that may be delayed on the client side from when we actually
		// receive it. See WM_TABLE_CHANGE(_EX) handlers in mainfrm.cpp

		//
		// (c.haag 2005-07-01 09:03) - If the packet size is bigger than the standard
		// table checker message, that means we have an extended table checker message
		// which actually has data in it that we can use for the purpose of not having
		// to query the database. In those cases, we send a WM_TABLE_CHANGED_EX message.
		//
		// (c.haag 2005-08-16 17:14) - On second thought, lets ignore the pData->id value.
		// By doing so, we don't have to force every table-checker ready application to
		// support extended table checker messages. Their convenience is only in the scope
		// of Practice.
		//
		// (a.walling 2009-07-10 09:43) - PLID 33644 - A way for the client to decide how to handle the table checker
		// without having to change everything that already exists. The client should translate the
		// WM_PENDING_TABLE_CHANGED(_EX) messages into a WM_TABLE_CHANGED(_EX) message when necessary.

		if (wSize > sizeof(_NxHeader) + sizeof(PACKET_REFRESH_TABLE))
		{
			CTableCheckerDetails* p = new CTableCheckerDetails(pData + 1, wSize - sizeof(_NxHeader) - sizeof(PACKET_REFRESH_TABLE));
			PostMessage(GetMainFrame()->GetSafeHwnd(), WM_PENDING_TABLE_CHANGED_EX, (WPARAM)pData->table | (bAckToNxServer ? 0x80000000 : 0), (LPARAM)p);
		}
		else {
			//needed for ToDoList, and other things
			PostMessage(GetMainFrame()->GetSafeHwnd(), WM_PENDING_TABLE_CHANGED, (WPARAM)pData->table | (bAckToNxServer ? 0x80000000 : 0), pData->id);

			// (a.walling 2012-04-06 13:51) - PLID 48990 - Invalidate cached lists
			{
				using namespace PracData;
				switch (pData->table) {
				case NetUtils::LocationsT:
					InvalidateCached<LocationsT>();
					break;
				case NetUtils::Providers:
					InvalidateCached<ProvidersT>();
					break;
				case NetUtils::Coordinators:
					InvalidateCached<UsersT>();
					break;
				case NetUtils::NoteCatsF:
					InvalidateCached<NoteCatsF>();
					break;
				case NetUtils::EMNTabCategoriesT:
					InvalidateCached<EmnTabCategoriesT>();
					break;
				case NetUtils::EMNTabChartsT:
					InvalidateCached<EmnTabChartsT>();
					break;
				case NetUtils::EmnTabChartCategoryLinkT:
					InvalidateCached<EmnTabChartCategoryLinkT>();
					break;
				// (b.eyers 2016-03-17) - PLID 68321
				case NetUtils::DischargeStatusT:
					InvalidateCached<EmrDischargeStatusT>();
					break;
				}
			}
		}
	}

	void OnReceived(DWORD ip, void * pData, unsigned short wSize)
	{
		// (c.haag 2006-05-25 13:45) - PLID 20812 - This code has been depreciated
	}

	bool Send(PacketType pkttype, void* pData, DWORD dwSize)
	{
		if (NULL == GetMainFrame()->GetNxServerSocket()) return false;

		// (b.cardillo 2006-07-05 17:41) - PLID 21336 - Allocate the memory dynamically now that the 
		// framework can support packets of unlimited size.
		char *data = new char[dwSize + sizeof(_NxHeader)];
		_NxHeader* pPacket = (_NxHeader*)data;

		pPacket->dwKey = 0;
		pPacket->dwVer = g_dwCurrentNxPacketVersion;
		pPacket->type = pkttype;
		pPacket->wSize = (WORD)dwSize + sizeof(_NxHeader); // TODO
		if (dwSize != 0 && pData != 0) //Make sure there's actually some data being sent.
			memcpy(data + sizeof(_NxHeader), pData, dwSize);

		// Process locally if you're not connected to the server
		//if (isConnected)
		//{
		//	TCP_Send(GetServerIP(), pPacket, sizeof(NetUtils::_NxHeader)+dwSize);
		//}

		// (c.haag 2006-05-25 12:37) - PLID 20812 - If TCP_Send failed, then nothing would
		// happen. We want this behavior to stay consistent for now. In a future release
		// we need to have Practice auto-reconnect to NxServer.
		try {
			NxSocketUtils::Send(GetMainFrame()->GetNxServerSocket(), (const _NxHeader*)pPacket);
			// Free the memory we allocated before returning
			delete[]data;
			return true;
		}
		catch (CNxException* e)
		{
			e->Delete();
		}
		catch (...)
		{
		}
		// Free the memory we allocated before returning
		delete[]data;
		return false;
	}

	// (j.gruber 2010-07-16 15:07) - PLID 39463 - added ThreadID
	void SendIM(DWORD dwIP, long nRecipientID, COleDateTime dt, long nRegardingID, long nPriority, long nMessageGroupID, CString strMessage, long nID, long nSentBy, CString strSentBy, long nThreadID)
	{
		_IM_MESSAGE immsg;

		//Add the other new fields, silly.
		immsg.ip = dwIP;
		immsg.dwSentTo = nRecipientID;
		immsg.date = dt.m_dt;
		immsg.dwRegardingID = nRegardingID;
		immsg.dwPriority = nPriority;
		immsg.dwMessageGroupID = nMessageGroupID;
		if (strMessage.GetLength() >= 3991) {
			immsg.bCheckData = TRUE;
		}
		else {
			strcpy(immsg.szMsg, strMessage);
			immsg.bCheckData = FALSE;
		}
		immsg.dwMessageID = nID;

		immsg.dwThreadID = nThreadID;

		immsg.dwSentBy = (DWORD)nSentBy;
		strcpy(immsg.szSentBy, strSentBy);

		Send(PACKET_TYPE_IM_MESSAGE, &immsg, sizeof(_IM_MESSAGE));

	}

	//TES 6/9/2008 - PLID 23243 - Added parameters for the appointment ID, and its resource IDs.
	void SendAlert(const char* szMsg, long nAppointmentID, const CDWordArray &dwaResourceIDs, EAlert alert /* = GenericAlert */)
	{
		_ALERT_MESSAGE *pamsg = (_ALERT_MESSAGE*)new BYTE[sizeof(_ALERT_MESSAGE) + strlen(szMsg) + 1];

		//TES 6/9/2008 - PLID 23243 - Fill the packet's resources with the ones we were passed in (maximum of 5).
		for (int i = 0; i < dwaResourceIDs.GetSize() && i < 5; i++) {
			pamsg->dwResourceIDs[i] = dwaResourceIDs[i];
		}
		//TES 6/9/2008 - PLID 23243 - 0 out any remaining entries in the fixed-length array.
		for (; i < 5; i++) {
			pamsg->dwResourceIDs[i] = 0;
		}
		//TES 6/9/2008 - PLID 23243 - If the appointment had more than 5 resources, than our recipients will need to pull the rest
		// from the database (that's an unlikely scenario).
		pamsg->bCheckData = (dwaResourceIDs.GetSize() > 5);
		//TES 6/9/2008 - PLID 23243 - If they do need to pull from data, they'll need the appointment's ID.
		pamsg->dwAppointmentID = (DWORD)nAppointmentID;

		//TES 6/9/2008 - PLID 30327 - For some reason, this was not only limiting the string to 507 bytes, but it wasn't even 
		// truncating, just dropping the message altogether!  Variable-length packets are perfectly fine, we use them all over the
		// place, and I have now done so for this function as well.
		/*if (strlen(szMsg) > 512 - 5)
			return;*/

		strcpy(pamsg->szMsg, szMsg);
		pamsg->ip = GetHostIP().S_un.S_addr;
		pamsg->eAlert = alert;
		Send(PACKET_TYPE_ALERT, pamsg, sizeof(_ALERT_MESSAGE) - 1 + strlen(szMsg) + 1);

		delete[] pamsg;
	}

	void ProcessMessage(WPARAM wParam, LPARAM lParam)
	{
		// (c.haag 2006-05-25 13:44) - PLID 20812 - This code has been depreciated
		/*
		DWORD ip;	// IP of the socket from which you got the packet from. This should always be the NxServer.
		WORD wSize = TCP_MAX_DEFAULT_DATASIZE;

		switch (TCP_HandleMessage(wParam, lParam, &ip, pData, &wSize))
		{
		case EServerDisconnected:
		if (Connect())//reconnect, changed this to ip later
		{	TCP_Destroy();
		g_propManager.SetNxServerSocket(INVALID_SOCKET);
		isConnected = false;
		}
		break;
		case EReceivedData:
		//OnReceived(ip, pData, wSize);
		break;
		case EServerAccepted:
		if(isConnected){//There may have been an error.
		// (c.haag 2003-09-30 10:56) - Send the name of our database to
		// NxServer so our database packets will only persist in the
		// scope of our database.
		CString strDefaultDatabase = (LPCTSTR)GetRemoteData()->GetDefaultDatabase();
		char* szDefaultDatabase = new char[strDefaultDatabase.GetLength()+1];
		strcpy(szDefaultDatabase, strDefaultDatabase);
		Send(NetUtils::PACKET_TYPE_DATABASE_NAME, (void*)szDefaultDatabase, strDefaultDatabase.GetLength()+1);
		delete szDefaultDatabase;

		GetMainFrame()->m_dlgMessager.SendMessage(NXM_NXSERVER_CONNECTED, NULL, NULL);
		}
		break;
		case EClientConnected:
		case EClientDisconnected:
		case EError:
		default:
		break;
		}*/
	}

	// (d.thompson 2009-07-07) - PLID 34803 - Track the last 25 table checkers so we can log them upon an error
	CList<NetUtils::ERefreshTable, NetUtils::ERefreshTable> l_aryPLID34803_Tables;
	void PLID34803_AddTableChecker(NetUtils::ERefreshTable ertTable)
	{
		try {
			//Limit to 25 to save space / readability
			if (l_aryPLID34803_Tables.GetCount() >= 25) {
				//We're at/over 25, need to remove the head
				l_aryPLID34803_Tables.RemoveHead();
			}

			//Add this new one to the end
			l_aryPLID34803_Tables.AddTail(ertTable);
		}
		catch (...) {}
	}
	//Returns a printable string of everything in the list
	CString PLID34803_GetPrintableCString()
	{
		CString strOutput;

		try {
			//http://msdn.microsoft.com/en-us/library/kyk38ey9(VS.80).aspx
			POSITION pos = l_aryPLID34803_Tables.GetHeadPosition();
			for (int i = 0; i < l_aryPLID34803_Tables.GetCount(); i++) {
				NetUtils::ERefreshTable ert = l_aryPLID34803_Tables.GetNext(pos);

				//We're just outputting numbers
				strOutput += FormatString("%li, ", (long)ert);
			}
			return strOutput;
		}
		catch (...) {
			return "Exception in PLID34803_GetPrintableCString";
		}

	}

	// (j.jones 2014-08-20 10:42) - PLID 63427 - added a list of tablecheckers
	// that should always be processed immediately, and never queued up in the stagger
	bool IsImmediateTablechecker(NetUtils::ERefreshTable ertTable)
	{
		//The following tablecheckers must always be processed immediately.
		//They should never be queued up for processing by the stagger.
		switch (ertTable) {
			case NetUtils::EMNAccessT:
			case NetUtils::EMNTemplateAccessT:
			case NetUtils::EMNForceAccessT:
				return true;
				break;
		}

		//if we get here, this is not a tablechecker that needs
		//to be processed immediately
		return false;
	}
};
