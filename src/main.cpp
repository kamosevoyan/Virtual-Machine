#include <string>
#include "VirtualMachine.h"
#include <iostream>



int main(int argc, char* argv[])
{
	try
	{
		ui_handler(argc, argv);
	}
	catch(const std::string& report)
	{
		std::cerr << "\033[31m" << report << "\033[01";
		return 1;
	}
	
	return 0;
}