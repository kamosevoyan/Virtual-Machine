#ifndef INCLUDEVIRTUALMACHINE
#define INCLUDEVIRTUALMACHINE

#include "Input.h"
#include "Output.h"

#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iostream>
#include <bitset>

typedef unsigned char dtype;
constexpr int REGISTERS = 12;
const int RAM_SIZE = 256;

void ui_handler(int, char*[]);

class VirtualMachine final
{	
	friend void ui_handler(int, char*[]);

public:

	const dtype HALT_OPCODE =     0b11111111;
	const dtype ram_addres =  	  0b00000101;
	const dtype program_counter = 0b00000110;
	const dtype input_output = 	  0b00000111;
	const dtype ram_pointer = 	  0b00001000;

private:

	enum class Operation {ALU_OR, ALU_AND, ALU_XOR, ALU_NOT, ALU_ADD, ALU_SUB, ALU_MUL, ALU_DIV, ALU_SHL, ALU_SHR,
						CND_EQUAL, CND_NOT_EQUAL, CND_LESS, CND_LESS_OR_EQUAL, CND_GREATHER, CND_GREATHER_OR_EQUAL};

	class Register
	{
	public:
	
		Register();		
		void set(dtype) &;
		dtype get() &;
			
	private:

		dtype data = 0;
	};
	
	class ALU
	{
	public:

		ALU();
		dtype compute(Operation, dtype, dtype) &;
	};
	
	class ControlUnit
	{
	public:
		ControlUnit(Register*);
		bool compute(Operation, dtype, dtype, dtype) &;
		
	private:
		Register* counter_ptr;
	};
					
	class Clock
	{
	public:
		Clock();
		void tick() &;

	private:
		size_t speed = 0;
	};
			
	class Ram
	{
	public:
		Ram();
		void write(dtype, dtype)&;
		dtype read(dtype)&;
		
	private:
		dtype data[RAM_SIZE];
	};

	Clock clock;
	ALU alu;
	ControlUnit control_unit;		
	Ram ram;		

	Register registers[REGISTERS];
	Register command_buffer[4];

	bool halt = false;
	bool pc_to_update = true;

	Input* input = nullptr;
	Output* output = nullptr;
				
	void fetch() &;
	void decode() &;
	void execute(Operation, dtype, dtype, dtype, dtype) &;
		
public:

	VirtualMachine();
	~VirtualMachine();
	VirtualMachine(const VirtualMachine&) = delete;
	VirtualMachine(VirtualMachine&&) = delete;
	VirtualMachine& operator=(const VirtualMachine&) = delete;		

	void load_program(char*, size_t) &;
	void run() &;
};

#endif