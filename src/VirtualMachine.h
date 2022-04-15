#ifndef INCLUDEVIRTUALMACHINE
#define INCLUDEVIRTUALMACHINE

#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iostream>
#include <bitset>

typedef unsigned char dtype;

constexpr int REGISTERS = 12;

void ui_handler(int, char*[]);

class VirtualMachine final
{	

private:
	
	const int program_counter = REGISTERS / 2;

	enum class Operation {ALU_OR, ALU_AND, ALU_XOR, ALU_NOT, ALU_ADD, ALU_SUB, ALU_MUL, ALU_DIV,
						CND_EQUAL, CND_NOT_EQUAL, CND_LESS, CND_LESS_OR_EQUAL, CND_GREATHER, CND_GREATHER_OR_EQUAL};
	class Register
	{
		public:
			Register();		
			void set(dtype) &;
			dtype get() &;
				
		private:
			dtype data;
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
			void compute(Operation, dtype, dtype, dtype) &;
			
		private:
			Register* counter_ptr;
	};
					
	class Clock
	{
		public:
			Clock();
			void tick() &;

		private:
			size_t speed = 1000000000;
	};
			
	class Ram
	{
		public:
			Ram();
			void write(size_t, dtype)&;
			dtype read(size_t)&;
			
		private:
			dtype data[256];
	};
	
	Clock clock;
	ALU alu;
	ControlUnit control_unit;		
	Register registers[REGISTERS];
	Ram ram;		
	Register command_buffer[4];
	bool halt = false;
	bool pc_to_update = true;
					
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