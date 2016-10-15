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

		int pipe_1[2]
		int pipe_2[2]
		pipe(pipe_1)
		pipe(pipe_2)

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
				if (i == 0)
				{
					if (command_to_execute.input_file != "")
					{
						int file_descriptor = fopen(command_to_execute.input_file, "r");
						dup2(file_descriptor, STDIN);
					}
				}
				if (i != command_to_execute.commands.size() - 1)
				{
					if (i % 2 != 0)
					{
						dup2(pipe_1[0], STDIN);
						dup2(pipe_2[1], STDOUT);
					}
					else
					{
						dup2(pipe_2[0], STDIN);
						dup2(pipe_1[1], STDOUT);
					}
				}
				if (i == command_to_execute.commands.size() - 1)
				{
					if (command_to_execute.output_file != "")
					{
						int file_descriptor = fopen(command_to_execute.output_file, "w");
						dup2(file_descriptor, STDOUT);
					}
				}

				//package it correctly
				for (int s = 0; s < command_to_execute.commands.size(); s++)
				{
					char* arguments[50];
					int j = 1;
					for (int i = 0; i < command_to_execute.commands[s].size(); i++)
					{
						const char * c_string = command_to_execute.commands[i][i].c_str();
						arguments[i] = strdup(c_string);
						j++;
					}
					arguments[j] = nullptr;
				}

				execvp(arguments[0], arguments);
				perror("Error: "); 	//someething went terribly wrong if we hit this point
				exit(pid); 	//don't break if bad command
			}

		}


//////////////////////////////////////////////
		// for (int s = 0; s < command_to_execute.commands.size(); s++)
		// {
		// 	char* arguments[50];
		// 	int j = 1;
		// 	for (int i = 0; i < command_to_execute.commands[s].size(); i++)
		// 	{
		// 		const char * c_string = command_to_execute.commands[i][i].c_str();
		// 		arguments[i] = strdup(c_string);
		// 		j++;
		// 	}
		// 	arguments[j] = nullptr;
		// }
		//
		// std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		// pid_t pid = fork();
		//
		// if (pid < 0)
		// {
		// 	perror("Error: ");
		// }
		//
		// else if (pid > 0)
		// {
		// 	//this is the parent
		// 	int * pid_ptr = &pid;
		// 	wait(pid_ptr);
		// 	std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
		// 	ptime = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		// }
		//
		// else
		// {
		// 	//this is the child process
		//
		// 	if (execute_command.input_file != "")
		// 	{
		// 		//have input file
		// 	}
		//
		// 	if (execute_command.output_file && )
		// 	///
		// 	execvp(arguments[0], arguments);
		// 	perror("Error: "); 	//someething went terribly wrong if we hit this point
		// 	exit(pid); 	//don't break if bad command
		// }

		std::cout << "> ";
		std::cin.getline(command, 200);
	}
}
