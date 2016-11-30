#pragma once

// (j.armen 2014-03-26 11:27) - PLID 61517 - Forward declares for the DiagCodeInfo structure

struct DiagCodeInfo;

// (j.gruber 2014-02-18 16:28) - PLID 60878 - new whichCodes structure
//these longs are the Diag9ID and the Diag10ID, then the DiagCodeInfoPtr is just the DiagCodeInfoPtr that is on the bill
// (j.gruber 2014-03-21 12:52) - PLID 61494 - made this into a shared pointer
typedef boost::shared_ptr<DiagCodeInfo> DiagCodeInfoPtr;
typedef std::pair<long, long> CChargeWhichCodePair;
typedef std::map<CChargeWhichCodePair, DiagCodeInfoPtr>::iterator CChargeWhichCodesIterator;
typedef std::map<CChargeWhichCodePair, DiagCodeInfoPtr>::reverse_iterator CChargeWhichCodesReverseIterator;
typedef std::map<CChargeWhichCodePair, DiagCodeInfoPtr> CChargeWhichCodesMap;
typedef boost::shared_ptr<CChargeWhichCodesMap> CChargeWhichCodesMapPtr;