from acalsim import SimTop, SimBase, SimModule, CodeGen

# Create SimTop instance
sim_top = SimTop("SOCSimTop", "app_soc")

# Create SimBase instance: CPU
base_cpu = SimBase("CPU", shared=False, group="", app="app_soc")
# Create SimModule instance: IFStage
module_ifstage = SimModule("IFStage", shared=False, group="", app=base_cpu.app)
base_cpu.addModule(module_ifstage)
# Create SimModule instance: IDStage
module_idstage = SimModule("IDStage", shared=False, group="", app=base_cpu.app)
base_cpu.addModule(module_idstage)
# Create SimModule instance: EXEStage
module_exestage = SimModule("EXEStage", shared=False, group="", app=base_cpu.app)
base_cpu.addModule(module_exestage)
# Create SimModule instance: MEMStage
module_memstage = SimModule("MEMStage", shared=False, group="", app=base_cpu.app)
base_cpu.addModule(module_memstage)
# Create SimModule instance: WBStage
module_wbstage = SimModule("WBStage", shared=False, group="", app=base_cpu.app)
base_cpu.addModule(module_wbstage)
sim_top.addSimulator(base_cpu)
codegen = CodeGen(sim_top, genProj=True, projectName="SOCSimulator")

# Create SimBase instance: DataMemory
base_datamemory = SimBase("DataMemory", shared=False, group="", app="app_soc")
sim_top.addSimulator(base_datamemory)
codegen = CodeGen(sim_top, genProj=True, projectName="SOCSimulator")
