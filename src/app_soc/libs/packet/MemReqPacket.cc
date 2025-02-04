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

#include "packet/MemReqPacket.hh"

#include "DataMemory.hh"

void MemReqPacket::visit(acalsim::Tick _when, acalsim::SimModule& _module) {
	CLASS_ERROR << "void MemReqPacket::visit(SimModule& _module)is not implemented yet!";
}

void MemReqPacket::visit(acalsim::Tick _when, acalsim::SimBase& _simulator) {
	if (auto mem = dynamic_cast<DataMemory*>(&_simulator)) {
		// mem->processReqPkt(this->addr, this->data, this->type);
		mem->processReqPkt();
	} else {
		CLASS_ERROR << "Invalid module type for MemRespPacket";
	}
}
