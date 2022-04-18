#ifndef INCLUDEOUTPUT
#define INCLUDEOUTPUT

#include <iostream>

typedef unsigned char dtype;

class Output
{
public:
	virtual void write(dtype) & = 0;
private:
};

class TerminalOutput : public Output
{
public:
	void write(dtype) &;
};

#endif