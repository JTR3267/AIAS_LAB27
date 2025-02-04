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

#include "DataMemory.hh"

DataMemory::DataMemory(const std::string& name, size_t _size) : acalsim::CPPSimBase(name), BaseMemory(_size) {
	// Generate and Register MasterPorts

	// Generate and Register SlavePorts
}

DataMemory::~DataMemory() {}

void DataMemory::init() {
	CLASS_INFO << "DataMemory Initialization";
	this->registerModules();
}

void DataMemory::registerModules() {
	// Generate and Register Modules

	// Connect SimPort
}

void DataMemory::step() {}

void DataMemory::cleanup() {}
