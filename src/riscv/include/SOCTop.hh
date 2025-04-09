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
#ifndef HW2_INCLUDE_SOCTOP_HH_
#define HW2_INCLUDE_SOCTOP_HH_

#include <string>

#include "ACALSim.hh"
#include "CPU.hh"
#include "DMA.hh"
#include "DataMemory.hh"
#include "Emulator.hh"
#include "SystemConfig.hh"
#include "SystolicArray.hh"

/**
 * @class SOCTop
 * @brief Top-level System-on-Chip simulation class
 * @details Manages the complete SoC simulation environment, including configuration
 *          registration, CLI argument handling, and trace control. Inherits from
 *          STSim template class specialized for SOC type.
 */
class SOCTop : public acalsim::SimTop {
public:
	/**
	 * @brief Constructor for SOCTop simulation environment
	 * @param _name Name of the simulation instance (default: "SOCTop")
	 * @param _configFile Path to configuration file (default: empty string)
	 * @details Initializes the simulation and sets up trace container with
	 *          default path "soc/trace"
	 */
	SOCTop(const std::string _name = "SOCTop", const std::string _configFile = "")
	    : acalsim::SimTop(_name, _configFile) {
		this->traceCntr.run(0, &acalsim::SimTraceContainer::setFilePath, "trace", "soc/trace");
	}
	virtual ~SOCTop() {}

	/**
	 * @brief Registers configuration objects for the simulation
	 * @details Creates and registers two configuration objects:
	 *          1. EmulatorConfig: Configuration for the CPU emulator
	 *          2. SOCConfig: Configuration for SOC timing parameters
	 * @override Overrides base class method
	 */
	void registerConfigs() override {
		auto emuConfig = new EmulatorConfig("Emulator configuration");
		this->addConfig("Emulator", emuConfig);
		auto socConfig = new SOCConfig("SOC configuration");
		this->addConfig("SOC", socConfig);
	}

	/**
	 * @brief Registers command-line interface arguments
	 * @details Sets up CLI options for the simulation:
	 *          - --asm_file_path: Path to the assembly code file
	 * @override Overrides base class method
	 */
	void registerCLIArguments() override {
		this->addCLIOption<std::string>("--asm_file_path",                    // Option name
		                                "The file path of an assembly code",  // Description
		                                "Emulator",                           // Config section
		                                "asm_file_path"                       // Parameter name
		);
	}

	void registerSimulators() override {
		size_t mem_size          = acalsim::top->getParameter<int>("Emulator", "memory_size");
		size_t sa_size           = acalsim::top->getParameter<int>("SOC", "systolic_array_size");
		size_t sa_reg_base_addr  = acalsim::top->getParameter<int>("SOC", "accel_reg_base_addr");
		size_t sa_reg_size       = acalsim::top->getParameter<int>("SOC", "accel_reg_size");
		size_t sa_buf_base_addr  = acalsim::top->getParameter<int>("SOC", "accel_buf_base_addr");
		size_t sa_buf_size       = acalsim::top->getParameter<int>("SOC", "accel_buf_size");
		size_t dma_reg_base_addr = acalsim::top->getParameter<int>("SOC", "dma_reg_base_addr");
		size_t dma_reg_size      = acalsim::top->getParameter<int>("SOC", "dma_reg_size");

		this->bus         = new acalsim::crossbar::CrossBar("Bus", 6, 9);
		this->dma         = new DMA("DMA", dma_reg_base_addr, dma_reg_size);
		this->dmem        = new DataMemory("Data Memory", mem_size);
		this->isaEmulator = new Emulator("RISCV RV32I Emulator");
		this->cpu         = new CPU("Single-Cycle CPU Model", this->isaEmulator);
		this->systolicArray =
		    new SystolicArray("Systolic Array", sa_size, sa_reg_base_addr, sa_reg_size, sa_buf_base_addr, sa_buf_size);

		this->addSimulator(this->cpu);
		this->addSimulator(this->bus);
		this->addSimulator(this->dma);
		this->addSimulator(this->dmem);
		this->addSimulator(this->systolicArray);

		this->cpu->addDownStream(this->dmem, "DSmem");

		this->cpu->addSlavePort("bus2cpu-resp-s", 1);
		this->cpu->addSlavePort("bus2cpu-wresp-s", 1);
		this->cpu->addSlavePort("bus2cpu-wdresp-s", 1);
		this->dma->addSlavePort("bus2dma-rr-s", 1);
		this->dma->addSlavePort("bus2dma-wr-s", 1);
		this->dma->addSlavePort("bus2dma-wd-s", 1);
		this->dma->addSlavePort("bus2dma-resp-s", 1);
		this->dma->addSlavePort("bus2dma-wresp-s", 1);
		this->dma->addSlavePort("bus2dma-wdresp-s", 1);
		this->dmem->addSlavePort("bus2mem-rr-s", 1);
		this->dmem->addSlavePort("bus2mem-wr-s", 1);
		this->dmem->addSlavePort("bus2mem-wd-s", 1);
		this->systolicArray->addSlavePort("bus2sa-rr-s", 1);
		this->systolicArray->addSlavePort("bus2sa-wr-s", 1);
		this->systolicArray->addSlavePort("bus2sa-wd-s", 1);

		// Register PRMasterPort to MasterTBSim for the request channel
		this->cpu->addPRMasterPort("cpu2bus-rr-m", this->bus->getPipeRegister("Req", 0));
		this->cpu->addPRMasterPort("cpu2bus-wr-m", this->bus->getPipeRegister("Req", 1));
		this->cpu->addPRMasterPort("cpu2bus-wd-m", this->bus->getPipeRegister("Req", 2));
		this->dma->addPRMasterPort("dma2bus-rr-m", this->bus->getPipeRegister("Req", 3));
		this->dma->addPRMasterPort("dma2bus-wr-m", this->bus->getPipeRegister("Req", 4));
		this->dma->addPRMasterPort("dma2bus-wd-m", this->bus->getPipeRegister("Req", 5));

		// Simport Connection (Bus <> SlavePort at Devices)
		for (auto mp : bus->getMasterPortsBySlave("Req", 0)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dmem, mp->getName(), "bus2mem-rr-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 1)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dmem, mp->getName(), "bus2mem-wr-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 2)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dmem, mp->getName(), "bus2mem-wd-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 3)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dma, mp->getName(), "bus2dma-rr-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 4)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dma, mp->getName(), "bus2dma-wr-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 5)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dma, mp->getName(), "bus2dma-wd-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 6)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->systolicArray, mp->getName(), "bus2sa-rr-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 7)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->systolicArray, mp->getName(), "bus2sa-wr-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Req", 8)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->systolicArray, mp->getName(), "bus2sa-wd-s");
		}

		// Register PRMasterPort to SlaveTBSim for the response channel
		this->dmem->addPRMasterPort("mem2bus-resp-m", this->bus->getPipeRegister("Resp", 0));
		this->dma->addPRMasterPort("dma2bus-resp-m", this->bus->getPipeRegister("Resp", 3));
		this->systolicArray->addPRMasterPort("sa2bus-resp-m", this->bus->getPipeRegister("Resp", 6));

		// Simport Connection (Bus <> SlavePort at Devices)
		for (auto mp : bus->getMasterPortsBySlave("Resp", 0)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->cpu, mp->getName(), "bus2cpu-resp-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Resp", 1)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->cpu, mp->getName(), "bus2cpu-wresp-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Resp", 2)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->cpu, mp->getName(), "bus2cpu-wdresp-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Resp", 3)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dma, mp->getName(), "bus2dma-resp-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Resp", 4)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dma, mp->getName(), "bus2dma-wresp-s");
		}
		for (auto mp : bus->getMasterPortsBySlave("Resp", 5)) {
			acalsim::SimPortManager::ConnectPort(this->bus, this->dma, mp->getName(), "bus2dma-wdresp-s");
		}
	}

	void registerPipeRegisters() override {
		this->SimTop::registerPipeRegisters();
		for (auto reg : bus->getAllPipeRegisters("Req")) this->getPipeRegisterManager()->addPipeRegister(reg);
		for (auto reg : bus->getAllPipeRegisters("Resp")) this->getPipeRegisterManager()->addPipeRegister(reg);
	}

private:
	CPU*                         cpu;
	acalsim::crossbar::CrossBar* bus;
	DMA*                         dma;
	DataMemory*                  dmem;
	Emulator*                    isaEmulator;
	SystolicArray*               systolicArray;
};

#endif
