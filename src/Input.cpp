#include "Input.h"

dtype KeyboardInput::get() &
{
	std::cout << "Input: ";
	int temp;
	std::cin >> temp;
	this->buffer = temp;
	return buffer;
}

FileInput::FileInput(const std::string& path)
{
	std::ifstream input;
	input.open(path);
	input.seekg(0, std::ios::end);
	this-> size = input.tellg();
	input.seekg(0, std::ios::beg);
	this->buffer =  new char[size];	
	input.read(buffer, size);	
	input.close();
}

dtype FileInput::get() &
{
	if (this->pos >= this->size)
	{
		throw std::string("Error: Out of bound in input file.\n");
	}
	return this->buffer[this->pos++];
}