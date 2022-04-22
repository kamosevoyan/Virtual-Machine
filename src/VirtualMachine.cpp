#include "VirtualMachine.h"


void ui_handler(int argc, char* argv[])
{

	VirtualMachine virtual_machine;
	std::ifstream test;
	
	if (argc < 4)
	{
		throw std::string(R"(Error: Misiing inputs. Use the following pattern.
./VirtualMachine program input_mode output_mode
)");
	}
	
	if (std::string(argv[2]) == "-k")
	{
		virtual_machine.input = new KeyboardInput;
		
		if (std::string(argv[3]) == "-t")
		{
			virtual_machine.output = new TerminalOutput;
		}
		else
		if (std::string(argv[3]) == "-f")
		{
			if (argc < 5)
			{
						throw std::string(R"(Error: Misiing inputs. Use the following pattern.
./VirtualMachine -k -f  output_file_name
)");
			}		 
			test.open(argv[4]);
			if (test.fail())
			{
				throw std::string("Error: incorrect output file name or directory.\n");
			}
			test.close();	

			virtual_machine.output = new FileOutput(argv[4]);
		}
	} 
	else
	if (std::string(argv[2]) == "-f")
	{
			if (std::string(argv[4]) == "-t")
			{
				if (argc < 5)
				{
							throw std::string(R"(Error: Misiing inputs. Use the following pattern.
./VirtualMachine program -f input_file_name output_mode
)");
				}
				
				test.open(argv[3]);
				if (test.fail())
				{
					throw std::string("Error: incorrect input file name or directory.\n");
				}
				
				if (test.peek() == std::ifstream::traits_type::eof())
				{
					throw std::string("Error: empty input file\n");
				}
				
				test.close();
				
				virtual_machine.input = new FileInput(argv[3]);
				virtual_machine.output = new TerminalOutput;
			}
			else
			if (std::string(argv[4]) == "-f")
			{
				if (argc < 6)
				{
							throw std::string(R"(Error: Misiing inputs. Use the following pattern.
./VirtualMachine program -f input_file_name -f output_file_name
)");					
				}
				
				
				test.open(argv[3]);
				if (test.fail())
				{
					throw std::string("Error: incorrect input file name or directory.\n");
				}
									
				if (test.peek() == std::ifstream::traits_type::eof())
				{
					throw std::string("Error: empty input file\n");
				}
				test.close();
				
				
				test.open(argv[5]);
				if (test.fail())
				{
					throw std::string("Error: incorrect output file name or directory.\n");
				}
				
				test.close();
				
				virtual_machine.input = new FileInput(argv[3]);	
				virtual_machine.output = new FileOutput(argv[5]);	
			}
			else
			{
				throw std::string(R"(Error: Misiing inputs. Use the following pattern.
./VirtualMachine program -f input_file_name output_mode
)");
			}
	}
	else
	{
		throw std::string("Error: unknown input mode. Use -k or -f.\n");
	}
	
	if ((virtual_machine.input == nullptr) || (virtual_machine.output == nullptr))
	{
		throw std::string("WTF");
	}
	
	
	std::ifstream program;
	program.open(argv[1], std::ios::binary);
	if (program.fail())
	{
		throw std::string("Error: invalid input file name or directory\n");
	}
	
	program.seekg(0, std::ios::end);
	size_t size = program.tellg();
	program.seekg(0, std::ios::beg);
	char* buffer = new char[size];	
	program.read(buffer, size);	
	program.close();
	
	if (size > RAM_SIZE)
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
		this->ram.write	(i, buffer[i]);
	}	
}

// to give access control unit to program counter
VirtualMachine::VirtualMachine():control_unit(&this->registers[this->program_counter])
{
	this->registers[this->program_counter].set(0);
}

VirtualMachine::~VirtualMachine()
{
	if (this->input != nullptr)
		delete this->input;

	
	if (this->output != nullptr)
		delete this->output;
}

void VirtualMachine::run() &
{
	while(!this->halt)
	{
		std::bitset<8*sizeof(dtype)> bitset(this->registers[this->program_counter].get());
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

	if (temp == this->HALT_OPCODE)
	{
		this->halt = true;
		return;
	}

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
		if (this->command_buffer[1].get() == this->input_output)
		{
			operand1 = this->input->get();
		}
		else if (this->command_buffer[1].get() == this->ram_pointer)
		{
			operand1 = this->ram.read(this->registers[this->ram_addres].get());
		}
		else
		{
			operand1 = this->registers[this->command_buffer[1].get()].get();
		}
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
		if (this->command_buffer[2].get() == this->input_output)
		{
			operand2 = this->input->get();
		}
		else if (this->command_buffer[2].get() == this->ram_pointer)
		{
			operand2 = this->ram.read(this->registers[this->ram_addres].get());
		}
		else
		{			
			operand2 = this->registers[this->command_buffer[2].get()].get();
		}
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
				operation = Operation::ALU_DIV;
			break;	

			case 8:
				operation = Operation::ALU_SHL;
			break;	

			case 9:
				operation = Operation::ALU_SHR;
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
	dtype temp;

	switch (module)
	{
		case 0:
			if (destination == this->input_output)
			{
				temp = this->alu.compute(operation, operand1, operand2);
				this->output->write(temp);
			}
			else 
			if (destination == this->ram_pointer)
			{
				temp = this->alu.compute(operation, operand1, operand2);
				this->ram.write(this->registers[this->ram_addres].get(), temp);
			}
			else
			{
				temp = this->alu.compute(operation, operand1, operand2);
				this->registers[destination].set(temp);
			}

			this->pc_to_update = (destination != this->program_counter);

		break;

		case 1:

			if(this->control_unit.compute(operation, operand1, operand2, destination))
			{
				this->pc_to_update = false;
			}
			else
			{
				this->pc_to_update = true;
			}

		break;
		
		default:

			throw std::string("Unknown module.\n");
	}
}
		
VirtualMachine::Ram::Ram()
{
	
}

void VirtualMachine::Ram::write(dtype address,  dtype value) &
{
	if (address > RAM_SIZE)
	{
		throw std::string("Error:Ram size exceeded.\n");
	}
	
	this->data[address] = value;
}

dtype VirtualMachine::Ram::read(dtype address) &
{
	if (address > RAM_SIZE)
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

		case Operation::ALU_SHL:
			
			return operand1 << operand2;

		break;

		case Operation::ALU_SHR:
			
			return operand1 >> operand2;
			
		break;			
	}
	
}

VirtualMachine::ControlUnit::ControlUnit(Register* counter)
{
	this->counter_ptr = counter;
}

bool VirtualMachine::ControlUnit::compute(Operation operation, dtype operand1, dtype operand2, dtype destination) &
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

	return output;
	
}

VirtualMachine::Clock::Clock()
{
	
}

void VirtualMachine::Clock::tick() &
{	
    std::this_thread::sleep_for(std::chrono::nanoseconds(this->speed));
	std::cout << "tick \n";	
}
