#pragma once
#include "MUMeasureBase.h"
#include "MUMeasureFilter.h"
#include <NxSystemUtilitiesLib\NxThread.h>

// (j.dinatale 2012-10-24 12:37) - PLID 53508 - threaded calculator so that way we can get measure data

namespace MU
{
	class CCalculator
	{
	public:
		CCalculator(void)
		{}
		~CCalculator(void)
		{}

		// filters to apply to our measures.
		MUMeasureFilter m_filterPatients;
		MUMeasureFilter m_filterMURange;

		bool IsRunning()
		{
			return m_thread;
		}

		void Interrupt()
		{
			if(IsRunning()){
				m_thread->Interrupt();
			}
		}

		void Join()
		{
			if(IsRunning()){
				m_thread->Join();
			}
		}

		void Run(HWND hwndNotify)
		{
			if (m_thread) { // already running
				ASSERT(FALSE);
				return;
			}

			// this will interrupt and wait for the old thread to complete if it exists
			m_thread = NxThread(boost::bind(&CCalculator::CalculateMeasures, this, hwndNotify));
		}

		std::vector<MU::MeasureData> m_MeasureData;
		void CalculateMeasures(HWND hwndNotify);

		MU::Stage m_eMeaningfulUseStage; // (r.farnworth 2013-10-15 11:25) - PLID 59011
	protected:
		NxThread m_thread;
		std::vector<shared_ptr<CMUMeasureBase>> GetAllMeasuresToCalculate();
	};
};