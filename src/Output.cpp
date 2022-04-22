#include "Output.h"
 
 
Output::~Output()
{
	
}

void TerminalOutput::write(dtype data) &
{
	std::cout << "Terminal output: " << (short int)data << "\n";
}

FileOutput::FileOutput(const std::string& path) : path(path)
{
	
}

FileOutput::~FileOutput()
{
	std::ofstream file;
	file.open(this->path, std::ios::binary);
	
	for (auto data: this->buffer)
	{
		file << data;
	}
	
	file.close();
}

void FileOutput::write(dtype data)&
{
	this->buffer.push_back(data);
}