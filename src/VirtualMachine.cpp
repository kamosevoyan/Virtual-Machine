#include "VirtualMachine.h"


void ui_handler(int argc, char* argv[])
{
	if (argc < 3)
	{
		throw std::string("Error: missing inputs: Use following pattern: /VirtualMachine input_file_name\toutput type\n");
	}
	
	if (argc > 3)
	{
		throw std::string("Error: to many inputs: Use following pattern: /VirtualMachine input_file_name\toutput type\n");
	}
	
	std::ifstream program;
	program.open(argv[1], std::ios::binary);
	if (program.fail())
	{
		throw std::string("Error: invalid input file name or directory\n");
	}
	
	VirtualMachine virtual_machine;
	char* buffer;
	program.seekg(0, std::ios::end);
	size_t size = program.tellg();
	program.seekg(0, std::ios::beg);
	buffer = new char[size];	
	program.read(buffer, size);	
	program.close();
	
	if (size > 256)
	{
		delete[] buffer;
		throw std::string("Error: maximum ram size exceeded\n");
	}	

	virtual_machine.load_program(buffer, size);
	delete[] buffer;
	virtual_machine.run();
	
}

void VirtualMachine::load_program(char* buffer, size_t size) &
{
	for (int i = 0; i < size; ++i)
	{
		this->ram.write	(i, (dtype)buffer[i]);
	}	
}


VirtualMachine::VirtualMachine():control_unit(&this->registers[this->program_counter])
{
	this->registers[this->program_counter].set(0);
}

VirtualMachine::~VirtualMachine()
{
	
}

void VirtualMachine::run() &
{
	while(!this->halt)
	{
		std::bitset<8> bitset(this->registers[this->program_counter].get());
		std::cout << "pc = " << bitset << "\n";
		this->clock.tick();
		this->fetch();
		this->decode();	

		if (this->pc_to_update)
		{
			this->registers[this->program_counter].set(this->registers[this->program_counter].get() + 4);
		}
	}	
}
				
void VirtualMachine::fetch() &
{
	this->command_buffer[0].set(this->ram.read(this->registers[this->program_counter].get() + 0));
	this->command_buffer[1].set(this->ram.read(this->registers[this->program_counter].get() + 1));
	this->command_buffer[2].set(this->ram.read(this->registers[this->program_counter].get() + 2));
	this->command_buffer[3].set(this->ram.read(this->registers[this->program_counter].get() + 3));
}

void VirtualMachine::decode() &
{
	dtype temp = this->command_buffer[0].get();
	dtype first_immediate_mask = 128;
	dtype second_immediate_mask = 64;
	dtype control_unit_mask = 32;
	dtype operation_mask = 7;
	dtype module;
	dtype operand1, operand2, destination;
	destination = this->command_buffer[3].get();	
	Operation operation;
		
	if (temp & first_immediate_mask)
	{
		operand1 = this->command_buffer[1].get();
	}
	else
	{
		if (this->command_buffer[1].get() >= REGISTERS)
		{
			throw std::string("Error: the register does not exist\n");
		}
		operand1 = this->registers[this->command_buffer[1].get()].get();
	}
	
	if (temp & second_immediate_mask)
	{
		operand2 = this->command_buffer[2].get();
	}
	else
	{
		if (this->command_buffer[2].get() >= REGISTERS)
		{
			throw std::string("Error: the register does not exist\n");
		}		
		operand2 = this->registers[this->command_buffer[2].get()].get();
	}
	
	if (!(temp & control_unit_mask))
	{
		temp = temp & operation_mask;

		switch(temp)
		{
			case 0:
				operation = Operation::ALU_ADD;
			break;
			
			case 1:
				operation = Operation::ALU_SUB;
			break;
			
			case 2:
				operation = Operation::ALU_AND;
			break;
			
			case 3:
				operation = Operation::ALU_OR;
			break;
			
			case 4:
				operation = Operation::ALU_NOT;
			break;
			
			case 5:
				operation = Operation::ALU_XOR;
			break;				
			
			case 6:
				operation = Operation::ALU_MUL;
			break;
			
			case 7:
				operation = Operation::ALU_DIV	;
			break;	

			default:
				throw std::string("Error: Unknown opcode\n");
		}
		
		module = 0;
	}
	else
	{
		temp = temp & operation_mask;
		
		switch(temp)
		{
			case 0:
				operation = Operation::CND_EQUAL;
			break;
			
			case 1:
				operation = Operation::CND_NOT_EQUAL;
			break;
			
			case 2:
				operation = Operation::CND_LESS;
			break;
			
			case 3:
				operation = Operation::CND_LESS_OR_EQUAL;
			break;
			
			case 4:
				operation = Operation::CND_GREATHER;
			break;
			
			case 5:
				operation = Operation::CND_GREATHER_OR_EQUAL;
			break;	

			default:
				throw std::string("Error: Unknown opcode\n");					
		}
		module = 1;
	}
	
	this->execute(operation, operand1, operand2, destination, module);
	
}


void VirtualMachine::execute(Operation operation, dtype operand1, dtype operand2, dtype destination, dtype module) &
{
	switch (module)
	{
		case 0:

			this->registers[destination].set(this->alu.compute(operation, operand1, operand2));
			this->pc_to_update = (destination != 6);

		break;

		case 1:

			this->control_unit.compute(operation, operand1, operand2, destination);

		break;
		
		default:

			throw std::string("Unknown module.\n");
	}
}
		

VirtualMachine::Ram::Ram()
{
	
}

void VirtualMachine::Ram::write(size_t address,  dtype value) &
{
	if (address > 256)
	{
		throw std::string("Error:Ram size exceeded.\n");
	}
	
	this->data[address] = value;
	
}

dtype VirtualMachine::Ram::read(size_t address) &
{
	if (address > 256)
	{
		throw std::string("Error:Ram size exceeded.\n");
	}
	
	return this->data[address];	
}

VirtualMachine::Register::Register()
{

}

void VirtualMachine::Register::set(dtype data) &
{
	this->data = data;
}

dtype VirtualMachine::Register::get() &
{
	return this->data;
}

VirtualMachine::ALU::ALU()
{
	
}

dtype VirtualMachine::ALU::compute(Operation operation, dtype operand1, dtype operand2) &
{
	switch(operation)
	{
		case Operation::ALU_ADD:

			return operand1 + operand2;

		break;
		
		case Operation::ALU_SUB:

			return operand1 - operand2;

		break;			
		
		case Operation::ALU_AND:

			return operand1  & operand2;

		break;
		
		case Operation::ALU_OR:

			return operand1  | operand2;

		break;
		
		case Operation::ALU_XOR:

			return operand1  ^ operand2;

		break;			
		
		case Operation::ALU_MUL:

			return operand1  * operand2;

		break;			

		case Operation::ALU_DIV:

			if (operand2 == 0)
			{
				throw std::string("Error: zero division\n");
			}			
			return operand1  / operand2;

		break;				
	}
	
}

VirtualMachine::ControlUnit::ControlUnit(Register* counter)
{
	this->counter_ptr = counter;
}

void VirtualMachine::ControlUnit::compute(Operation operation, dtype operand1, dtype operand2, dtype destination) &
{
	bool output = false;
	
	switch (operation)
	{
		case Operation::CND_EQUAL:

			output = (operand1 == operand2);

		break;
		
		case Operation::CND_NOT_EQUAL:

			output = (operand1 != operand2);
			
		break;		
		
		case Operation::CND_LESS:

			output = (operand1 < operand2);
			
		break;
		
		case Operation::CND_LESS_OR_EQUAL:
		
			output = (operand1 <= operand2);

		break;		
		
		case Operation::CND_GREATHER:
		
			output = (operand1 > operand2);

		break;
		
		case Operation::CND_GREATHER_OR_EQUAL:
		
			output = (operand1 >= operand2);

		break;			
	}
	
	if (output == true)
	{
		this->counter_ptr->set(destination);
	}
	
}

VirtualMachine::Clock::Clock()
{
	
}

void VirtualMachine::Clock::tick() &
{	
    std::this_thread::sleep_for(std::chrono::nanoseconds(this->speed));
	std::cout << "tick \n";	
}