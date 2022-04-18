#include "Output.h"

void TerminalOutput::write(dtype data) &
{
	std::cout << "Terminal output: " << (short int)data << "\n";
}