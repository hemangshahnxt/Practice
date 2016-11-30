#ifndef IMPORTBATCHUTILS_H
#define IMPORTBATCHUTILS_H

#pragma once

// (b.savon 2015-03-20 15:10) - PLID 65153 - Add Race batch statement
void AddRaceStatementToBatch(CString &strSqlBatch, CString strFields, CString strValues);
// (b.savon 2015-03-23 11:55) - PLID 65158 - Add Referral Source Name batch statement for MultiReferralT
void AddMultiReferalSourceToBatch(CString &strSqlBatch, CString strFields, CString strValues);
// (b.savon 2015-03-25 08:22) - PLID 65144 - Add Custom Fields query batch statement
void AddCustomFieldsToBatch(CString &strSqlBatch, CString strQuery);

#endif