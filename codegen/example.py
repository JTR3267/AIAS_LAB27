from acalsim import SimTop, SimBase, SimModule, CodeGen
import click


# Create and Register SimulatorA
def createSimulatorA(app: str) -> SimBase:
	sim = SimBase("SimulatorA", shared=False, group="sima", app=app)

	# Create SimModules in SimBaseA
	module_a = SimModule("ModuleA", shared=True, group="sima", app=app)
	module_b = SimModule("ModuleB", shared=True, group="sima", app=app)
	module_x = SimModule("ModuleX", shared=True, group="sima", app=app)

	# Register SimModules in SimBaseA
	sim.addModule(module_a)
	sim.addModule(module_b)
	sim.addModule(module_x)

	# Make Connection:SimPort
	# ┌----------------------------------------------------------------┐
	# |	Master Port                     Slave Port                     |
	# |	                                                               |
	# |	ModuleA ("ModuleA to X")-┐                                     |
	# |	                         |----> ModuleX ("ModuleX from A/B")   |
	# |	ModuleB ("ModuleB to X")-┘                                     |
	# |	                                                               |
	# |	ModuleX ("ModuleX to A")------> ModuleA ("ModuleA from X")     |
	# |	                                                               |
	# |	ModuleX ("ModuleX to B")------> ModuleB ("ModuleB from X")     |
	# └----------------------------------------------------------------┘
	sim.connectSimPort(module_a, module_x, "ModuleA to X", "ModuleX from A/B", 4)
	sim.connectSimPort(module_b, module_x, "ModuleB to X", "ModuleX from A/B", 4)
	sim.connectSimPort(module_x, module_a, "ModuleX to A", "ModuleA from X", 4)
	sim.connectSimPort(module_x, module_b, "ModuleX to B", "ModuleB from X", 4)
	return sim


def createSimTop(app: str) -> SimTop:
	simtop = SimTop("BaseSimTop", app)
	# Create/Register SimulatorA
	sim_a = createSimulatorA(app)
	simtop.addSimulator(sim_a)
	# Create/Register SimulatorB
	sim_b = SimBase("SimulatorB", shared=True, group="")
	simtop.addSimulator(sim_b)
	# Create/Register SimulatorX
	sim_x = SimBase("SimulatorX", shared=True, group="")
	simtop.addSimulator(sim_x)

	# Make Connection:SimPort
	# ┌--------------------------------------------------------------------------┐
	# |	Master Port                     Slave Port                               |
	# |	                                                                         |
	# |	SimulatorA ("SimulatorA to X")-┐                                         |
	# |	                         	   |----> SimulatorX ("SimulatorX from A/B") |
	# |	SimulatorB ("SimulatorB to X")-┘                                         |
	# |	                                                                         |
	# |	SimulatorX ("SimulatorX to A")------> SimulatorA ("SimulatorA from X")   |
	# |	                                                                         |
	# |	SimulatorX ("SimulatorX to B")------> SimulatorB ("SimulatorB from X")   |
	# └--------------------------------------------------------------------------┘
	simtop.connectSimPort(sim_a, sim_x, "SimulatorA to X", "SimulatorX from A/B", 4)
	simtop.connectSimPort(sim_b, sim_x, "SimulatorB to X", "SimulatorX from A/B", 4)
	simtop.connectSimPort(sim_x, sim_a, "SimulatorX to A", "SimulatorA from X")
	simtop.connectSimPort(sim_x, sim_b, "SimulatorX to B", "SimulatorB from X")

	# Make Connection:SimChannel
	# ┌---------------------------------------------------------------------------┐
	# |	Master Channel                     Slave Channel                          |
	# |	                                                                          |
	# |	SimulatorA ("from/to SimulatorX") <---> SimulatorX ("from/to SimulatorA") |
	# |	                                                                          |
	# |	SimulatorB ("from/to SimulatorX") <---> SimulatorX ("from/to SimulatorB") |
	# └---------------------------------------------------------------------------┘
	simtop.connectSimChannel(sim_a, sim_x, "to SimulatorX", "from SimulatorA")
	simtop.connectSimChannel(sim_b, sim_x, "to SimulatorX", "from SimulatorB")
	simtop.connectSimChannel(sim_x, sim_a, "to SimulatorA", "from SimulatorX")
	simtop.connectSimChannel(sim_x, sim_b, "to SimulatorB", "from SimulatorX")

	return simtop


@click.command()
@click.option('--app', prompt='CodegenApp', help='The application name for the simulation.')
@click.option(
    '--project-name', default="CodegenProj", help='The project name for the code generation.'
)
def main(app, project_name):
	base_simtop = createSimTop(app)
	codegen = CodeGen(base_simtop, genProj=True, projectName=project_name)
	click.echo(f"Simulation setup complete for app: {app} with project: {project_name}")


if __name__ == '__main__':
	main()
