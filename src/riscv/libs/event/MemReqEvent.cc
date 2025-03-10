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

#include "event/MemReqEvent.hh"

#include "DataMemory.hh"

MemReqEvent::MemReqEvent(DataMemory* _callee, acalsim::SimPacket* _memReqPkt)
    : acalsim::SimEvent("MemReqEvent"), callee(_callee), memReqPkt(_memReqPkt) {}

void MemReqEvent::renew(DataMemory* _callee, acalsim::SimPacket* _memReqPkt) {
	this->acalsim::SimEvent::renew();
	this->callee    = _callee;
	this->memReqPkt = _memReqPkt;
}

void MemReqEvent::process() { this->callee->accept(acalsim::top->getGlobalTick(), *this->memReqPkt); }
