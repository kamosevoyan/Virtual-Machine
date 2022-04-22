#ifndef INCLUDEINPUT
#define INCLUDEINPUT

#include <iostream>
#include <fstream>

typedef unsigned char dtype;


class Input
{ 
public:
	virtual dtype get() & = 0;
private:
};

class KeyboardInput : public Input
{
public:
	dtype get() &;
private:
	dtype buffer;
};

class FileInput : public Input
{
public:
	FileInput(const std::string&);
	dtype get() &;
private:
	char* buffer;
	size_t size;
	size_t pos = 0;
};

#endif