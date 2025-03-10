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

#ifndef SOC_INCLUDE_EVENT_EXECONEINSTREVENT_HH_
#define SOC_INCLUDE_EVENT_EXCONEINSTREVENT_HH_

#include "ACALSim.hh"
#include "DataStruct.hh"

class CPU;

class ExecOneInstrEvent : public acalsim::SimEvent {
public:
	ExecOneInstrEvent() = default;
	ExecOneInstrEvent(int _id, CPU* _cpu);
	virtual ~ExecOneInstrEvent() = default;

	void renew(int _id, CPU* _cpu);
	void process() override;

private:
	CPU* cpu;
};

#endif
