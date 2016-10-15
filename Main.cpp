#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

class ExecuteCommand
{
public:
	std::string input_file;
	std::string output_file;
	std::vector<std::vector<std::string>> commands;

	ExecuteCommand()
	{
		input_file = "";
		output_file = "";
	}
};

std::vector<std::string> command_parser(std::string command)
{
	std::vector<std::string> command_list;
	auto current = command.begin();
	bool done = false;
	while (!done)
	{
		auto next = std::find(current, command.end(), ' ');
		std::string new_command{ current, next };
		command_list.push_back(new_command);
		if (next != command.end()) current = next + 1;
		else done = true;
	}
	return command_list;
}

std::string remove_extra_spaces(std::string str)
{
	while (str[0] == ' ')
	{
		str = str.substr(1, str.size());

	}
	while (str[str.size()-1] == ' ')
	{
		str = str.substr(0, str.size()-1);
	}
	return str;
}

ExecuteCommand input_parser(std::string input)
{
	ExecuteCommand execute_command;
	std::vector<std::string> command_list;
	std::string unknown_word = "";
	char last_command = ' ';
;	for (int i = 0; i < input.size()+1; i++)
	{
		if (input[i] == '>')
		{
			if (last_command == '>')
			{
				if (unknown_word != "")
					execute_command.output_file = remove_extra_spaces(unknown_word);
			}
			else if (last_command == ' ')
			{
				if (unknown_word != "")
					execute_command.commands.push_back(command_parser(remove_extra_spaces(unknown_word)));
			}
			else if ('<')
			{
				if (unknown_word != "")
					execute_command.input_file = remove_extra_spaces(unknown_word);
			}
			unknown_word = "";
			last_command = '>';
		}
		else if (input[i] == '<')
		{
			if (last_command == '>')
			{
				if(unknown_word!="")
					execute_command.output_file = remove_extra_spaces(unknown_word);
			}
			else if (last_command == ' ')
			{
				if (unknown_word != "")
					execute_command.commands.push_back(command_parser(remove_extra_spaces(unknown_word)));
			}
			else if ('<')
			{
				if (unknown_word != "")
					execute_command.input_file = remove_extra_spaces(unknown_word);
			}
			unknown_word = "";
			last_command = '<';
		}
		else if (input[i] == '|' || i == input.size())
		{
			if (last_command == '>')
			{
				if (unknown_word != "")
					execute_command.output_file = remove_extra_spaces(unknown_word);
			}
			else if (last_command == ' ')
			{
				if (unknown_word != "")
					execute_command.commands.push_back(command_parser(remove_extra_spaces(unknown_word)));
			}
			else if ('<')
			{
				if (unknown_word != "")
					execute_command.input_file = remove_extra_spaces(unknown_word);
			}
			unknown_word = "";
			last_command = ' ';
		}
		else
		{
			unknown_word += input[i];
		}
	}
	return execute_command;
}

int main()
{

	std::cout << "--------------------" << std::endl;
	std::cout << "WELCOME" << std::endl;
	std::cout << "--------------------" << std::endl;
	std::cout << "ptime: prints executing time of last child process" << std::endl;
	std::cout << "history: print history of commands entered" << std::endl;
	std::cout << "^ [command index] : runs that command" << std::endl;
	std::cout << "exit : exit shell" << std::endl;
	std::cout << "--------------------" << std::endl;
	std::cout << std::endl;

	char command[200];
	std::vector < std::string > command_history;
	std::chrono::milliseconds ptime;

	std::cout << "> ";
	std::cin.getline(command, 200);

	while (strcmp(command, "exit") != 0)
	{
		command_history.push_back(command);
		ExecuteCommand command_to_execute = input_parser(command);

		//PTIME
		if (command_to_execute.commands[0][0] == "ptime")
		{
			std::cout << "Total Time (milliseconds) Executing Child Processes: " << ptime.count() << std::endl;

			std::cout << "> ";
			std::cin.getline(command, 200);
			continue;
		}

		//HISTORY
		else if (command_to_execute.commands[0][0] == "History" || command_to_execute.commands[0][0] == "history")
		{
			for (int i = 0; i < command_history.size(); i++)
			{
				std::cout << i << ":  " << command_history[i] << std::endl;
			}

			std::cout << "> ";
			std::cin.getline(command, 200);
			continue;
		}

		//RUN ^HISTORY
		else if (command_to_execute.commands[0][0] == "^")
		{
			int number = std::stoi(command_to_execute.commands[0][1]);
			command_to_execute = input_parser(command_history[number]);
		}

		int pipe_a[2];
		int pipe_b[2];
		pipe(pipe_a);
		pipe(pipe_b);

		for (int i = 0; i < command_to_execute.commands.size(); i++)
		{
			std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
			pid_t pid = fork();

			if (pid < 0)
			{
				// if we got here then something terrible happened
				perror("Error: ");
			}

			else if (pid > 0)
			{
				//this is the parent
				int * pid_ptr = &pid;
				wait(pid_ptr);
				std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
				ptime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
			}

			else
			{
				//this is the child process
				if (i == 0 && command_to_execute.input_file != "")
				{
					//if first command and input file exists, dupe file_descriptor of file to stdin
					std::string mode = "r";
					FILE * file_descriptor = fopen(command_to_execute.input_file.c_str(), mode.c_str());
					dup2(fileno(file_descriptor), fileno(stdin));
					// fclose(file_descriptor);
				}
				else if (i != 0)
				{
					if (i % 2 == 0)
					{
						// dup pibeb[1] to input
						dup2(pipe_b[1], fileno(stdin));
					}
					else
					{
						//dup pipe_a[1] to input
						dup2(pipe_a[1], fileno(stdin));
					}
				}
				if (i == command_to_execute.commands.size() - 1 && command_to_execute.output_file != "")
				{
					// if last command and output file exists, dup file_descriptor to stdout
					std::string mode = "w";
					FILE * file_descriptor = fopen(command_to_execute.output_file.c_str(), mode.c_str());
					dup2(fileno(file_descriptor), fileno(stdout));
					// fclose(file_descriptor);
				}
				else if (i != command_to_execute.commands.size() - 1)
				{
					if (i == 0 || i % 2 == 0)
					{
						// dup pipe_a[0] to output
						dup2(pipe_a[0], fileno(stdout));
					}
					else
					{
						// dup pipe_b[0] to output
						dup2(pipe_b[0], fileno(stdout));
					}
				}

				//package it correctly
				char* arguments[50];
				int j = 1;
				for (int g = 0; g < command_to_execute.commands[i].size(); i++)
				{
					const char * c_string = command_to_execute.commands[i][g].c_str();
					arguments[g] = strdup(c_string);
					j++;
				}
				arguments[j] = nullptr;

				execvp(arguments[0], arguments);
				perror("Error: ");
				exit(pid);
			}
		}

		std::cout << "> ";
		std::cin.getline(command, 200);
	}
}
