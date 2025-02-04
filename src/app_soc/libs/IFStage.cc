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

#include "IFStage.hh"

#include "CPU.hh"

IFStage::IFStage(const std::string& name, Register<if_stage_out>* _if_id_reg)
    : acalsim::SimModule(name), if_id_reg(_if_id_reg), flush(false), stall(false), exe_next_pc{false, 0} {
	this->pc_reg = new Register<uint32_t>(std::make_shared<uint32_t>(0));
}

IFStage::~IFStage() {}

void IFStage::init() { CLASS_INFO << "IFStage Initialization"; }

void IFStage::step() {}

void IFStage::execDataPath() {
	int index = *(this->pc_reg->get()) / 4;
	if (!this->flush && !this->stall) {
		const instr& fetch_instr = dynamic_cast<CPU*>(this->getSimulator())->fetchInstr(index);

		std::shared_ptr<if_stage_out> infoPtr =
		    std::make_shared<if_stage_out>(if_stage_out{.pc = current_pc, .inst = fetch_instr});

		this->if_id_reg->set(infoPtr);
	}
	if (this->exe_next_pc.first) {
		this->pc_reg->set(std::make_shared<uint32_t>(this->exe_next_pc.second));
	} else {
		this->pc_reg->set(std::make_shared<uint32_t>(this->current_pc + 4));
	}
}
