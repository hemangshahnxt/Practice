// Rewards.h: interface for the Rewards namespace.
//
//////////////////////////////////////////////////////////////////////

// (a.walling 2007-05-21 14:24) - PLID 20838 - Namespace of utility functions for applying and voiding reward points

#if !defined(AFX_REWARDS_H__B0D58128_43DC_4CB6_8F54_E0BE23BD6D52__INCLUDED_)
#define AFX_REWARDS_H__B0D58128_43DC_4CB6_8F54_E0BE23BD6D52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace Rewards  
{
	static const COleCurrency m_cyZero(0, 0);

	// these values are written to data! I removed the Check() constraint on them, however, for easier future dev.
	enum ERewardSource {
		ersRedeemedByCharge = -1,	// points traded in for some charge
		ersPerBill = 0,			// bill ID
		ersPerBillDollars,		// bill ID
		ersPerBillCharges,		// bill ID
		ersPerRefPatient,		// patient ID
		ersPerRefBill,			// bill ID
		ersPerRefBillDollars,	// bill ID
		ersPerRefBillPoints,	// bill ID
		// (z.manning 2010-07-20 12:22) - PLID 30127 - Added ability to manually adjust reward points
		ersManualAdjustment,	// ID n/a
	};

	// these values are written to data! I removed the Check() constraint on them, however, for easier future dev.
	enum ERewardsDeletedReason {
		erdrNotDeleted = 0,
		erdrRemoved,
		erdrChanged
	};
	
	
	void Initialize();
	void Refresh();
	
	long GetReferredByPatientID(long nPatientID);

	void AddTotalPointSqlToBatch(IN OUT CParamSqlBatch &sqlBatch, long nPatientID);
	COleCurrency GetTotalPoints(long nPatientID);
	COleCurrency GetBillPoints(long nPatientID, long nBillID);
	
	////////////////////////
	// Apply reward points
	
	COleCurrency ApplyBillAll(long nPatientID, long nBillID, BOOL bUpdate = FALSE);
	COleCurrency UpdateBillAll(long nPatientID, long nBillID);
	
	// points are applied to the ReferredByPatient, not the RefPatient
	COleCurrency ApplyRefPatient(long nReferredPatientID); // looks up nPatientID in query
	COleCurrency ApplyRefPatient(long nReferredPatientID, long nPatientID); // nPatientID explicitly set
	
	////////////////////////
	// Unapply reward points
	
	void UnapplyBillAll(long nBillID, ERewardsDeletedReason erdrReason);
	// UPDATE RewardHistoryT SET Deleted = 1, DeletedDate = GetDate() WHERE Deleted = 0 AND BillID = nBillID AND Source in (ersPerBill, ersPerBillDollars, ersPerCharge)
	
	// points are unapplied from the ReferredByPatient
	void UnapplyRefPatient(long nPatientID, ERewardsDeletedReason erdrReason);
	// a patient can be referred by only one patient.
	// UPDATE RewardHistoryT SET Deleted = 1, DeletedDate = GetDate() WHERE Deleted = 0 AND Source = ersPerRefPatient AND RefPatientID = nPatientID

	// (z.manning 2010-07-20 14:07) - PLID 30127
	void ManuallyAdjustRewardPoints(const long nPatientID, const COleCurrency cyOldPoints, const COleCurrency cyNewPoints);

	////////////////////////

	BOOL GetUsePriceWhenNoPoints();

	COleCurrency GetBillPoints();
	COleCurrency GetRefBillPoints();
	COleCurrency GetPointsPerBillDollars(const COleCurrency &cyValue);
	COleCurrency GetPointsPerRefBillDollars(const COleCurrency &cyValue);
	COleCurrency GetPointsPerRefBillPoints(const COleCurrency &cyValue);
	COleCurrency GetPointsPerChargeDollars(const COleCurrency &cyValue);
	COleCurrency GetPointsPerReferral();

};

#endif // !defined(AFX_REWARDS_H__B0D58128_43DC_4CB6_8F54_E0BE23BD6D52__INCLUDED_)
