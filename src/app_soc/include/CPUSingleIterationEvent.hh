/*
 * Copyright 2023-2024 Playlab/ACAL
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_APP_SOC_INCLUDE_CPU_SINGLE_ITERATION_EVENT_HH_
#define SRC_APP_SOC_INCLUDE_CPU_SINGLE_ITERATION_EVENT_HH_

#include "ACALSim.hh"
using namespace acalsim;

#include "CPU.hh"

class CPUSingleIterationEvent : public SimEvent {
public:
	CPUSingleIterationEvent(SimBase* _sim, instr& _instr) : SimEvent("CPUSingleIterationEvent"), sim(_sim) {
		this->clearFlags(Managed);
	}
	~CPUSingleIterationEvent() {}
	CPUSingleIterationEvent() {}

	void renew(SimBase* _sim) { this->sim = _sim; }
	void process() override;

private:
	SimBase* sim;
};

#endif