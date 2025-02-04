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

#include <memory>

#include "ACALSim.hh"
#include "SOCSimTop.hh"

int main(int argc, char** argv) {
	// Step 3. instantiate a top-level simulation instance
	acalsim::top = std::make_shared<SOCSimTop>("src/app_soc/configs.json");
	acalsim::top->init(argc, argv);
	acalsim::top->run();
	acalsim::top->finish();
	return 0;
}
