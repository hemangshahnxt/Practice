#include "stdafx.h"
#include "MUCalculator.h"
#include "MUMeasure_DEM_00.h"
#include "MUMeasure_CORE_01.h"
#include "MUMeasure_CORE_03.h"
#include "MUMeasure_CORE_04.h"
#include "MUMeasure_CORE_05.h"
#include "MUMeasure_CORE_06.h"
#include "MUMeasure_CORE_07.h"
#include "MUMeasure_CORE_08.h"
#include "MUMeasure_CORE_09.h"
#include "MUMeasure_CORE_12.h"
#include "MUMeasure_CORE_13.h"
#include "MUMeasure_MENU_02.h"
#include "MUMeasure_MENU_04.h"
#include "MUMeasure_MENU_05.h"
#include "MUMeasure_MENU_06.h"
#include "MUMeasure_MENU_07.h"
#include "MUMeasure_MENU_08.h"
#include "foreach.h"

#include "MUMeasure2_CORE_01A.h"
#include "MUMeasure2_CORE_01B.h"
#include "MUMeasure2_CORE_01C.h"
#include "MUMeasure2_CORE_02.h"
#include "MUMeasure2_CORE_03.h"
#include "MUMeasure2_CORE_04.h"
#include "MUMeasure2_CORE_05.h"
#include "MUMeasure2_CORE_07A.h"
#include "MUMeasure2_CORE_08.h"
#include "MUMeasure2_CORE_10.h"
#include "MUMeasure2_CORE_12.h"
#include "MUMeasure2_CORE_13.h"
#include "MUMeasure2_CORE_14.h"
#include "MUMeasure2_CORE_15A.h"
#include "MUMeasure2_CORE_15B.h"
#include "MUMeasure2_CORE_17.h"
#include "MUMeasure2_MENU_02.h"
#include "MUMeasure2_MENU_03.h"
#include "MUMeasure2_MENU_04.h"
#include "MUMeasure2_CORE_07B.h"

// (j.dinatale 2012-10-24 12:37) - PLID 53508 - threaded calculator so that way we can get measure data

namespace MU
{
	// collect all the measures we want to run
	std::vector<shared_ptr<CMUMeasureBase>> CCalculator::GetAllMeasuresToCalculate()
	{
		std::vector<shared_ptr<CMUMeasureBase>> Measures;

		// (r.farnworth 2013-10-15 11:44) - PLID 59011 - Allow the user to specifiy Stage 1 or 2 in the Meaningful Use Detailed Report dialog.
		
		//TES 10/15/2013 - PLID 58993 - Dem00 is a dummy measure that applies to all stages
		shared_ptr<CMUMeasureBase> Dem00 = shared_ptr<CMUMeasureBase>(new CMUMeasure_DEM_00(m_eMeaningfulUseStage));
		Dem00->m_filterMURange = m_filterMURange;
		Dem00->m_filterPatients = m_filterPatients;
		Measures.push_back(Dem00);
			
		if(m_eMeaningfulUseStage == MU::Stage1)
		{
			shared_ptr<CMUMeasureBase> Core01 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_01());
			Core01->m_filterMURange = m_filterMURange;
			Core01->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01);

			shared_ptr<CMUMeasureBase> Core03 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_03());
			Core03->m_filterMURange = m_filterMURange;
			Core03->m_filterPatients = m_filterPatients;
			Measures.push_back(Core03);

			shared_ptr<CMUMeasureBase> Core04 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_04());
			Core04->m_filterMURange = m_filterMURange;
			Core04->m_filterPatients = m_filterPatients;
			Measures.push_back(Core04);

			shared_ptr<CMUMeasureBase> Core05 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_05());
			Core05->m_filterMURange = m_filterMURange;
			Core05->m_filterPatients = m_filterPatients;
			Measures.push_back(Core05);

			shared_ptr<CMUMeasureBase> Core06 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_06());
			Core06->m_filterMURange = m_filterMURange;
			Core06->m_filterPatients = m_filterPatients;
			Measures.push_back(Core06);

			shared_ptr<CMUMeasureBase> Core07 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_07());
			Core07->m_filterMURange = m_filterMURange;
			Core07->m_filterPatients = m_filterPatients;
			Measures.push_back(Core07);

			shared_ptr<CMUMeasureBase> Core08 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_08());
			Core08->m_filterMURange = m_filterMURange;
			Core08->m_filterPatients = m_filterPatients;
			Measures.push_back(Core08);

			shared_ptr<CMUMeasureBase> Core09 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_09());
			Core09->m_filterMURange = m_filterMURange;
			Core09->m_filterPatients = m_filterPatients;
			Measures.push_back(Core09);

			// (b.savon 2014-06-06 14:13) - PLID 62343 - Delete the top & bottom columns in the stage 1 detailed report for "Request. Elec"
			/*shared_ptr<CMUMeasureBase> Core12 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_12());
			Core12->m_filterMURange = m_filterMURange;
			Core12->m_filterPatients = m_filterPatients;
			Measures.push_back(Core12);*/

			// (b.savon 2014-06-06 14:20) - PLID 62344 - Move the top & bottom columns in the stage 1 detailed report for "Elect. Access" between "Smoking Status" and "Clin. Summary".  Also update the percentage to 50.
			shared_ptr<CMUMeasureBase> Menu05 = shared_ptr<CMUMeasureBase>(new CMUMeasure_MENU_05());
			Menu05->m_filterMURange = m_filterMURange;
			Menu05->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu05);

			shared_ptr<CMUMeasureBase> Core13 = shared_ptr<CMUMeasureBase>(new CMUMeasure_CORE_13());
			Core13->m_filterMURange = m_filterMURange;
			Core13->m_filterPatients = m_filterPatients;
			Measures.push_back(Core13);

			shared_ptr<CMUMeasureBase> Menu02 = shared_ptr<CMUMeasureBase>(new CMUMeasure_MENU_02());
			Menu02->m_filterMURange = m_filterMURange;
			Menu02->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu02);

			shared_ptr<CMUMeasureBase> Menu04 = shared_ptr<CMUMeasureBase>(new CMUMeasure_MENU_04());
			Menu04->m_filterMURange = m_filterMURange;
			Menu04->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu04);

			shared_ptr<CMUMeasureBase> Menu06 = shared_ptr<CMUMeasureBase>(new CMUMeasure_MENU_06());
			Menu06->m_filterMURange = m_filterMURange;
			Menu06->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu06);

			shared_ptr<CMUMeasureBase> Menu07 = shared_ptr<CMUMeasureBase>(new CMUMeasure_MENU_07());
			Menu07->m_filterMURange = m_filterMURange;
			Menu07->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu07);

			shared_ptr<CMUMeasureBase> Menu08 = shared_ptr<CMUMeasureBase>(new CMUMeasure_MENU_08());
			Menu08->m_filterMURange = m_filterMURange;
			Menu08->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu08);
		}
		else if (m_eMeaningfulUseStage == MU::Stage2)
		{
			//Stage 2 measure functions will be placed here
			// (r.farnworth 2014-04-24 11:18) - Commenting all of these out until the report has been properly updated for 11400

			// (r.farnworth 2014-04-30 15:33) - PLID 59563 - Implement Detailed Reporting for MU.CORE.01.A for Stage 2
			shared_ptr<CMUMeasureBase> Core01A = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_01A());
			Core01A->m_filterMURange = m_filterMURange;
			Core01A->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01A);

			// (r.farnworth 2013-11-18 12:26) - PLID 59564 - Implement Detailed Reporting for MU.CORE.01.B for Stage 2
			shared_ptr<CMUMeasureBase> Core01B = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_01B());
			Core01B->m_filterMURange = m_filterMURange;
			Core01B->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01B);

			// (r.farnworth 2013-11-18 12:28) - PLID 59565 - Implement Detailed Reporting for MU.CORE.01.C for Stage 2
			shared_ptr<CMUMeasureBase> Core01C = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_01C());
			Core01C->m_filterMURange = m_filterMURange;
			Core01C->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01C);

			// (r.farnworth 2013-11-18 12:30) - PLID 59566 - Implement Detailed Reporting for MU.CORE.02 for Stage 2
			shared_ptr<CMUMeasureBase> Core02 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_02());
			Core02->m_filterMURange = m_filterMURange;
			Core02->m_filterPatients = m_filterPatients;
			Measures.push_back(Core02);

			//TES 10/15/2013 - PLID 59567 - MU.CORE.03
			shared_ptr<CMUMeasureBase> Core03 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_03());
			Core03->m_filterMURange = m_filterMURange;
			Core03->m_filterPatients = m_filterPatients;
			Measures.push_back(Core03);

			//TES 10/15/2013 - PLID 59568 - MU.CORE.04
			shared_ptr<CMUMeasureBase> Core04 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_04());
			Core04->m_filterMURange = m_filterMURange;
			Core04->m_filterPatients = m_filterPatients;
			Measures.push_back(Core04);

			//TES 10/16/2013 - PLID 59569 - MU.CORE.05
			//(s.dhole 05/06/2014) PLID 59569
			shared_ptr<CMUMeasureBase> Core05 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_05());
			Core05->m_filterMURange = m_filterMURange;
			Core05->m_filterPatients = m_filterPatients;
			Measures.push_back(Core05);

			// (r.farnworth 2013-11-13 11:27) - PLID 59571 - MU.CORE.07.A
			shared_ptr<CMUMeasureBase> Core07A = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_07A());
			Core07A->m_filterMURange = m_filterMURange;
			Core07A->m_filterPatients = m_filterPatients;
			Measures.push_back(Core07A);

			// (r.farnworth 2013-11-14 16:22) - PLID 59572 - MU.CORE.07.B
			// (s.dhole 2014-05-28 09:30) - PLID 59572
			shared_ptr<CMUMeasureBase> Core07B = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_07B());
			Core07B->m_filterMURange = m_filterMURange;
			Core07B->m_filterPatients = m_filterPatients;
			Measures.push_back(Core07B);

			// (r.farnworth 2013-10-15 15:45) - PLID 59573 - MU.CORE.08
			shared_ptr<CMUMeasureBase> Core08 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_08());
			Core08->m_filterMURange = m_filterMURange;
			Core08->m_filterPatients = m_filterPatients;
			Measures.push_back(Core08);

			// (r.farnworth 2013-10-16 13:41) - PLID 59574 - MU.CORE.10
			shared_ptr<CMUMeasureBase> Core10 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_10());
			Core10->m_filterMURange = m_filterMURange;
			Core10->m_filterPatients = m_filterPatients;
			Measures.push_back(Core10);

			// (r.farnworth 2013-10-16 13:41) - PLID 59575 - MU.CORE.12
			shared_ptr<CMUMeasureBase> Core12 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_12());
			Core12->m_filterMURange = m_filterMURange;
			Core12->m_filterPatients = m_filterPatients;
			Measures.push_back(Core12);

			// (r.farnworth 2013-10-21 10:15) - PLID 59576 - MU.CORE.13
			shared_ptr<CMUMeasureBase> Core13 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_13());
			Core13->m_filterMURange = m_filterMURange;
			Core13->m_filterPatients = m_filterPatients;
			Measures.push_back(Core13);

			// (r.farnworth 2013-10-30 15:14) - PLID 59577 - MU.CORE.14
			shared_ptr<CMUMeasureBase> Core14 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_14());
			Core14->m_filterMURange = m_filterMURange;
			Core14->m_filterPatients = m_filterPatients;
			Measures.push_back(Core14);

			// (r.farnworth 2013-11-14 11:17) - PLID 59578 - MU.CORE.15.A
			shared_ptr<CMUMeasureBase> Core15A = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_15A());
			Core15A->m_filterMURange = m_filterMURange;
			Core15A->m_filterPatients = m_filterPatients;
			Measures.push_back(Core15A);

			// (r.farnworth 2013-11-14 14:22) - PLID 59579 - MU.CORE.15.B
			shared_ptr<CMUMeasureBase> Core15B = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_15B());
			Core15B->m_filterMURange = m_filterMURange;
			Core15B->m_filterPatients = m_filterPatients;
			Measures.push_back(Core15B);

			// (r.farnworth 2013-10-24 16:04) - PLID 59580 - MU.CORE.17
			shared_ptr<CMUMeasureBase> Core17 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_17());
			Core17->m_filterMURange = m_filterMURange;
			Core17->m_filterPatients = m_filterPatients;
			Measures.push_back(Core17);

			// (r.farnworth 2013-10-29 11:03) - PLID 59581 -  MU.MENU.02
			// (s.dhole 2014-05-19 10:30) - PLID 59581 
			shared_ptr<CMUMeasureBase> Menu02 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_MENU_02());
			Menu02->m_filterMURange = m_filterMURange;
			Menu02->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu02);

			// (b.savon 2014-05-14 11:26) - PLID 59582 - Implement Detailed Reporting for MU.MENU.03 for Stage 2
			shared_ptr<CMUMeasureBase> Menu03 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_MENU_03());
			Menu03->m_filterMURange = m_filterMURange;
			Menu03->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu03);

			//TES 10/16/2013 - PLID 59583 - MU.MENU.04
			// (s.dhole 2014-05-19 10:30) - PLID 59583 
			shared_ptr<CMUMeasureBase> Menu04 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_MENU_04());
			Menu04->m_filterMURange = m_filterMURange;
			Menu04->m_filterPatients = m_filterPatients;
			Measures.push_back(Menu04);

		}
		else if (m_eMeaningfulUseStage == MU::ModStage2)
		{
			//Mod. Stage 2 measure functions will be placed here

			// Measure.3.1 (MU.CORE.01.A)
			shared_ptr<CMUMeasureBase> Core01A = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_01A());
			Core01A->m_filterMURange = m_filterMURange;
			Core01A->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01A);

			// Measure.3.2 (MU.CORE.01.C)
			shared_ptr<CMUMeasureBase> Core01C = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_01C());
			Core01C->m_filterMURange = m_filterMURange;
			Core01C->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01C);

			// Measure.3.3 (MU.CORE.01.B)
			shared_ptr<CMUMeasureBase> Core01B = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_01B());
			Core01B->m_filterMURange = m_filterMURange;
			Core01B->m_filterPatients = m_filterPatients;
			Measures.push_back(Core01B);

			// Measure.4 (MU.CORE.02)
			shared_ptr<CMUMeasureBase> Core02 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_02());
			Core02->m_filterMURange = m_filterMURange;
			Core02->m_filterPatients = m_filterPatients;
			Measures.push_back(Core02);

			// Measure.5 (MU.CORE.15.B)
			shared_ptr<CMUMeasureBase> Core15B = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_15B());
			Core15B->m_filterMURange = m_filterMURange;
			Core15B->m_filterPatients = m_filterPatients;
			Measures.push_back(Core15B);

			// Measure.6 (MU.CORE.13)
			shared_ptr<CMUMeasureBase> Core13 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_13());
			Core13->m_filterMURange = m_filterMURange;
			Core13->m_filterPatients = m_filterPatients;
			Measures.push_back(Core13);

			// Measure.7 (MU.CORE.14)
			shared_ptr<CMUMeasureBase> Core14 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_14());
			Core14->m_filterMURange = m_filterMURange;
			Core14->m_filterPatients = m_filterPatients;
			Measures.push_back(Core14);

			// Measure.8.1 (MU.CORE.07.A)
			shared_ptr<CMUMeasureBase> Core07A = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_07A());
			Core07A->m_filterMURange = m_filterMURange;
			Core07A->m_filterPatients = m_filterPatients;
			Measures.push_back(Core07A);

			// Measure.8.2 (MU.CORE.07.B)
			shared_ptr<CMUMeasureBase> Core07B = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_07B());
			Core07B->m_filterMURange = m_filterMURange;
			Core07B->m_filterPatients = m_filterPatients;
			Measures.push_back(Core07B);

			// Measure.9 (MU.CORE.17)
			shared_ptr<CMUMeasureBase> Core17 = shared_ptr<CMUMeasureBase>(new CMUMeasure2_CORE_17());
			Core17->m_filterMURange = m_filterMURange;
			Core17->m_filterPatients = m_filterPatients;
			Measures.push_back(Core17);
		}

		return Measures;
	}

	void CCalculator::CalculateMeasures(HWND hwndNotify)
	{
		try {
			// Load Measures here
			std::vector<shared_ptr<CMUMeasureBase>> Measures = GetAllMeasuresToCalculate();
			long nCurrMeasure = 1;

			// Preload complete, notify our hwnd
			if(!NxThread::This()->IsInterrupted()){
				::PostMessage(hwndNotify, WM_MEASURE_PRELOAD_COMPLETE, nCurrMeasure, Measures.size());
			}

			foreach(shared_ptr<CMUMeasureBase> measure, Measures){
				// loads our data
				MU::MeasureData data = measure->GetMeasureInfo();
				m_MeasureData.push_back(data);

				// while loading, we can notify our hwnd where we are
				if(!NxThread::This()->IsInterrupted()){
					::PostMessage(hwndNotify, WM_MEASURE_COMPLETE, ++nCurrMeasure, Measures.size());
				}else{
					break;
				}
			}

			// notify the dialog that we are done or if we were cancelled
			if(!NxThread::This()->IsInterrupted()){
				::PostMessage(hwndNotify, WM_ALL_MEASURES_COMPLETE, Measures.size(), Measures.size());
			}else{
				::PostMessage(hwndNotify, WM_MEASURE_LOAD_CANCEL, 0, 0);
			}
		}NxCatchAll(__FUNCTION__);
	}
};