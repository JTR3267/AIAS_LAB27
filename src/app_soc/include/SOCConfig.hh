
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

#ifndef SRC_APP_SOC_INCLUDE_SOCCONFIG_HH_
#define SRC_APP_SOC_INCLUDE_SOCCONFIG_HH_

#include <string>

#include "ACALSim.hh"
using namespace acalsim;

using json = nlohmann::json;

class SOCConfig : public SimConfig {
public:
	SOCConfig(const std::string& _name) : SimConfig(_name) {
		/* register all parameters into PETileConfig  */
		this->addParameter<int>("memory_size", 256, ParamType::INT);
		this->addParameter<std::string>("asm_file_path", "", ParamType::STRING);
		this->addParameter<Tick>("memory_read_latency", 1, ParamType::TICK);
		this->addParameter<Tick>("memory_write_latency", 1, ParamType::TICK);
	}
	~SOCConfig() {}
};

#endif  // SRC_APP_SOC_INCLUDE_SOCCONFIG_HH_
