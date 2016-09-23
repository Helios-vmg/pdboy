#include "CodeGenerator.h"
#include "CpuDefinition.h"
#include "InterpreterCodeGenerator.h"

int main(){
	auto definition = std::make_shared<CpuDefinition>();
	InterpreterCodeGenerator icg(definition, "GameboyCpu");
	icg.generate();
	icg.dump_function_definitions(std::cout);
}
