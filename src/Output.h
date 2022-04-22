#ifndef INCLUDEOUTPUT
#define INCLUDEOUTPUT
 
#include <iostream>
#include <vector>
#include <fstream>

typedef unsigned char dtype;

class Output
{
public:
	virtual void write(dtype) & = 0;
	virtual ~Output();
private:
};

class TerminalOutput : public Output
{
public:
	void write(dtype) &;
};

class FileOutput : public Output
{
public:
	
	FileOutput(const std::string&);
	~FileOutput();
	void write(dtype) &;
	
private:
	const std::string path;
	std::vector<dtype> buffer;

};

#endif