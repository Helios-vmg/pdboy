#include "CodeGenerator.h"
#include "CpuDefinition.h"
#include "InterpreterCodeGenerator.h"

int main(){
	auto definition = std::make_shared<CpuDefinition>();
	InterpreterCodeGenerator icg(definition);
}
