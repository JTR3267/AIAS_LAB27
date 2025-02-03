import json
import sys


# Function to load JSON data from a file
def load_json(file_path):
	with open(file_path, "r") as file:
		return json.load(file)


# Function to generate Python code from UML JSON data
def generate_python_code(uml_data):
	code_lines = [
	    "from acalsim import SimTop, SimBase, SimModule, CodeGen", "", "# Create SimTop instance",
	    f"sim_top = SimTop(\"{uml_data['SimTop']['ClassName']}\", \"{uml_data['SimTop']['AppName']}\")",
	    ""
	]

	sim_app = uml_data['SimTop']['AppName']
	for sim in uml_data["SimTop"]["SimBases"]:
		sim_name = sim["Name"]
		sim_classname = sim["ClassName"]
		sim_shared = sim["SharedLib"]
		sim_group = sim["SubFolder"]

		code_lines.append(f"# Create SimBase instance: {sim_name}")
		code_lines.append(
		    f"base_{sim_name.lower()} = SimBase(\"{sim_classname}\", shared={sim_shared}, group=\"{sim_group}\" ,app=\"{sim_app}\")"
		)

		for module in sim["SimModules"]:
			module_name = module["Name"]
			module_shared = module["SharedLib"]

			code_lines.append(f"# Create SimModule instance: {module_name}")
			code_lines.append(
			    f"module_{module_name.lower()} = SimModule(\"{module_name}\", shared={module_shared}, group=\"{sim_group}\",app=base_{sim_name.lower()}.app)"
			)
			code_lines.append(f"base_{sim_name.lower()}.addModule(module_{module_name.lower()})")

		code_lines.append(f"sim_top.addSimulator(base_{sim_name.lower()})")
		code_lines.append(
		    f"codegen = CodeGen(sim_top, genProj=True, projectName=\"{uml_data['ProjectName']}\")"
		)

		code_lines.append("")

	return "\n".join(code_lines)


if __name__ == "__main__":
	if len(sys.argv) < 2:
		print("Usage: python script.py <path_to_json_file>")
		sys.exit(1)

	uml_file_path = sys.argv[1]

	try:
		# Load UML JSON data from file
		uml_data = load_json(uml_file_path)

		# Generate Python code dynamically
		python_code = generate_python_code(uml_data)

		# Save the generated code to a file
		with open("codegen/gen.py", "w") as file:
			file.write(python_code)

		print("Dynamic Python code has been generated from the JSON file.")
	except Exception as e:
		print(f"An error occurred: {e}")
		sys.exit(1)
