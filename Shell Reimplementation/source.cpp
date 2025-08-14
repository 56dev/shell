
//for getenv
//be careful
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <sstream>
#include <filesystem>



int main()
{


	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	std::vector<std::string> prefixes{
		{
			"exit",
			"echo",
			"type",
			"path"
		}
	};
	
	

	//std::string path = getenv("PATH");

	//for security and convenience purposes, using a local path
	//instead of the system path.
	//PLEASE UNCOMMENT ABOVE CODE LINE IF NEED SYSTEM PATH!
	std::string path = "\
C:\\MinGW\\bin;\
C:\\Games;";
	std::cout << "A Shell Reimplementation by 56dev_. (2025)" << std::endl;
	std::cout << "ONLY FOR WINDOWS!\n" << std::endl;
	std::string input{};
	while (true)
	{

		std::cout << "$ ";

		std::getline(std::cin, input);

		//exit
		if (input.find(prefixes[0]) == 0)
			return 0;
		//echo
		else if (input.find(prefixes[1]) == 0)
		{
			std::cout << input.substr(5) << std::endl;
			
		}
		//type
		else if (input.find(prefixes[2]) == 0)
		{
			std::string param = input.substr(5);

			if (std::find(prefixes.begin(), prefixes.end(), param) != std::end(prefixes))
				std::cout << param << " is a builtin command." << std::endl;
			else
			{

				auto dir_it = std::filesystem::directory_iterator("C:\\MinGW\\bin");

				
				std::stringstream stream{ path };

				std::string directoryString{};
				bool hasFoundMatchingExecutable{ false };
				std::string foundPath;
				while (getline(stream, directoryString, ';'))
				{
					for (auto const& entry : std::filesystem::directory_iterator{ directoryString })
					{
						//a hilarious way to check if the extension is an exe
						//so hilarious, there's probably a better way
						if (entry.path().filename().replace_extension("exe") != entry.path().filename())
						{
							continue;
						}
						if (param == entry.path().filename().replace_extension(""))
						{
							hasFoundMatchingExecutable = true;
							//for some reason, directoryString is empty when going out of this loop.
							//maybe getline isn't so straightforward...
							foundPath = directoryString;
							break;
						}

					}

				}
				
				

				if (hasFoundMatchingExecutable)
					std::cout << param << " is in " << foundPath << std::endl;
				else
					std::cout << param << " is not recognized as a valid command!" << std::endl;

			}
			
			
		}
		//path
		else if (input.find(prefixes[3]) == 0)
		{
			std::stringstream ss{ path };
			std::string temporary{};
			while (std::getline(ss, temporary, ';'))
			{
				std::cout << temporary << "\n";
			}
			std::cout << std::endl;
			
		}
		else
			std::cout << "\"" << input << "\"" << " not recognized as a valid command!" << std::endl;

		
		

	}

	return 0;
}

