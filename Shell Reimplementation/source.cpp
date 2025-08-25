
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
#include <windows.h>
#include <aclapi.h>

namespace fs = std::filesystem;

std::optional<fs::path> searchThroughPATH(std::string&, std::string&);
bool hasExecutePermissions(const fs::path&);
bool queryEnvironmentVariable(const std::string&, std::string&);
void substituteEnvironmentVariablesInString(std::string&);
void output(std::string&, std::ostream, std::vector<std::string>);

int main()
{


	std::cout << std::unitbuf;
	std::cerr << std::unitbuf;

	std::vector<std::string> prefixes{
		{
			"echo",
			"cd",
			"exit",
			"type",
			"path",
			"pwd"
		}
	};		

	std::string path{};
	queryEnvironmentVariable("PATH", path);

	std::cout << "A Command Line Reimplementation by 56dev_. (2025)" << "\n";
	std::cout << "ONLY FOR WINDOWS!\n" << std::endl;

	std::cout << "NOTICE: this is a developer build!" << std::endl;

	#ifndef _WIN32
		std::cout << "This machine is not on Windows. You cannot use the shell." << std::endl;
		return 0;
	#endif

	while (true)
	{
		
		std::cout << "$ ";

		//INPUT PROCESSING
		std::string commandLine{};
		std::vector<std::string> redirectorsCout{};
		std::getline(std::cin, commandLine);

		if (commandLine.empty())
			continue;

		std::size_t rediri = commandLine.find('>');

		if (rediri != std::string::npos)
		{			
			std::string temporary{};
			std::vector<int> indicesWhereRedirectionOperatorPresent{};

			while (true)
			{
				indicesWhereRedirectionOperatorPresent.push_back(static_cast<int>(rediri));
				std::size_t nextSubRediri = commandLine.substr(rediri + 1).find('>');
				if (nextSubRediri == std::string::npos)
					break;
				rediri += 1 + nextSubRediri;
			}

			int previousI{0};
			for (int i : indicesWhereRedirectionOperatorPresent)
			{
			
				if (commandLine.find_first_not_of(" ", i + 1) >= commandLine.size())
					continue;

				std::string t{};
				int x{ static_cast<int>(commandLine.find_first_not_of("> ", i)) };
				std::string cmds{ commandLine.substr(x)};
				int quoteCount{0};
				if (cmds[0] != '\"')
				{
					std::stringstream cmdss{ cmds };
					std::getline(cmdss, t, ' ');
				}
				else
					
				{	
					++quoteCount;
					for (int j {1}; j < cmds.size(); ++j)
					{
						if (cmds[j] == '\"')
						{
							++quoteCount;
							break;
						}
						t.push_back(cmds[j]);
					}
				}
				redirectorsCout.push_back(t);
				if (previousI > commandLine.size() || i - previousI > commandLine.size())
				{
					break;
				}
				temporary.append(commandLine.substr(previousI, i - previousI));
				previousI = /* i + */ x /* - i */ + quoteCount + static_cast<int>(t.size());
			}
			if(previousI < commandLine.size())
				temporary.append(commandLine.substr(previousI, commandLine.size() - previousI - 1));
			
			commandLine = temporary;
		}

		if (commandLine.empty())
			continue;

		std::vector<std::string> userInputVector{};
		std::stringstream     sm{ commandLine };
		std::string              temporary{};

		

		while (sm >> std::quoted(temporary)) 
			userInputVector.push_back(temporary);

		
		


		//ECHO-LIKE PARAMETER PROCESSING
		//		User input split into: command and argument, separated by either 
		//		1 or an arbitrary amount of whitespace
		//echo
		if (userInputVector[0] == prefixes[0])
		{
			if (commandLine.size() <= 5)
				std::cout << "" << std::endl;
			else
			{
				std::string output = commandLine.substr(5);
				substituteEnvironmentVariablesInString(output);
				std::cout << output << "\n";
			}
			continue;
		}
		//cd
		else if (userInputVector[0] == prefixes[1])
		{
			if (commandLine.size() <= 3)
				continue;
						
			std::string directory{ commandLine.substr(3) };

			substituteEnvironmentVariablesInString(directory);
			std::cout << directory << std::endl;
			directory.erase(std::remove(directory.begin(), directory.end(), '\"'), directory.end());

			auto first = directory.find_first_not_of(' ');

			if (!directory.size() == 0 && first != std::string::npos)
			{
				directory = directory.substr(first);
				std::cout << directory << std::endl;
				if (!fs::exists(directory))
					std::cout << "System cannot find the path specified!" << std::endl;
				else if (!fs::is_directory(directory))
					std::cout << "Invalid directory name." << std::endl;
				else
					fs::current_path(directory);
			}		

			continue;
		}

		//EXE-LIKE PARAMETER PROCESSING
		//		User input split into: command, argument-list
		//		arbitrary amount of whitespace between arguments.
		//		white-space preserving arguments ==> wrap in quotes.
		//exit
		else if (userInputVector[0] == prefixes[2])
			return 0;
		//type
		else if (userInputVector[0] == prefixes[3])
		{

			if (std::find(prefixes.begin(), prefixes.end(), userInputVector[1]) != std::end(prefixes))
				std::cout << userInputVector[1] << " is a builtin command." << std::endl;
			else
			{
				std::optional<fs::path> foundPath{ searchThroughPATH(path, userInputVector[1]) };

				if (foundPath)
					std::cout << userInputVector[1] << " is in " << foundPath.value().replace_filename("").string() << std::endl;
				else
					std::cout << userInputVector[1] << " is not recognized as a valid command!" << std::endl;
			}			
			
		}
		//path
		else if (userInputVector[0] == prefixes[4])
		{
			std::stringstream ss{ path };
			std::string pathTemporary{};
			while (std::getline(ss, pathTemporary, ';'))
				std::cout << pathTemporary << "\n";

			std::cout << std::endl;			
		}
		//pwd
		else if (userInputVector[0] == prefixes[5])
			std::cout << fs::current_path().string() << std::endl;


		//EXTERNAL EXE PARAMETER PROCESSING; AND INVALIDATION
		//		Entire line of user input passed into an EXE if it exists.
		//		Else, invalidated.
		else
		{			
			std::optional<fs::path> foundPath{ searchThroughPATH(path, userInputVector[0]) };

			if (foundPath)
			{
				STARTUPINFOA startupInfo{ {sizeof(startupInfo)} };
				PROCESS_INFORMATION processInfo{};
				
				if (CreateProcessA(foundPath.value().string().c_str(), &commandLine.front(), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
				{
					WaitForSingleObject(processInfo.hProcess, INFINITE);
					CloseHandle(processInfo.hThread);
					CloseHandle(processInfo.hProcess);
				}
			}
			else
				std::cout << "\"" << userInputVector[0] << "\"" << " not recognized as a valid command!" << std::endl;
		}
		
	}

	return 0;
}

std::optional<fs::path> searchThroughPATH(std::string& path, std::string& param)
{
	std::stringstream stream{ path };
	std::string directoryString{};
	
	while (getline(stream, directoryString, ';'))
	{
		std::error_code ec{};

		if (!fs::is_directory(directoryString, ec))
			continue;
		
		for (const auto& entry : fs::directory_iterator{ directoryString, fs::directory_options::skip_permission_denied })
		{				

			if (param != entry.path().stem().string())
				continue;
			std::error_code ec1{};

			if (!entry.is_regular_file(ec1) || ec1)
				continue;
				
			if (entry.path().extension() != ".exe")
				continue;

			if (hasExecutePermissions(entry.path()))
				return entry.path();
						
		}	
	}
	return std::nullopt;
}

bool hasExecutePermissions(const fs::path& path)
{
	PSECURITY_DESCRIPTOR pSD{ nullptr};

	DWORD result = GetNamedSecurityInfo(
		path.c_str(),
		SE_FILE_OBJECT,
		OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&pSD
	);
	

	if (result != ERROR_SUCCESS)
	{
		std::cerr << "Checking Security Permissions: Unable to get security info for path:" << path << std::endl;
		LocalFree(pSD);
		return false;
	}
	

	if (!ImpersonateSelf(SecurityImpersonation))
	{
		std::cerr << "Checking Security Permissions: Unable to impersonate" << std::endl;
		LocalFree(pSD);
		return false;
	}

	GENERIC_MAPPING mapping{ {} };
	mapping.GenericRead = FILE_GENERIC_READ;
	mapping.GenericWrite = FILE_GENERIC_WRITE;
	mapping.GenericExecute = FILE_GENERIC_EXECUTE;
	mapping.GenericAll = FILE_ALL_ACCESS;;

	PRIVILEGE_SET privileges{ {} };
	DWORD privilegeSetLength = sizeof(privileges);

	ACCESS_MASK grantedAccess{ 0 };
	BOOL accessStatus{ FALSE };

	if (!AccessCheck(pSD, GetCurrentThreadToken(), FILE_EXECUTE, &mapping, &privileges, &privilegeSetLength, &grantedAccess, &accessStatus))
	{
		std::cerr << "Checking Security Permissions: Access check was unable to succeed for path " << path << ": error " << GetLastError() << std::endl;
		accessStatus = FALSE;
	}

	RevertToSelf();
	LocalFree(pSD);

	return accessStatus != FALSE;
}

//if query fails, then outstr is set to an arbitrary number of null-terminator characters.
//the null-terminator from GetEnvironmentVariableA is automatically stripped out. 
bool queryEnvironmentVariable(const std::string& variableName, std::string& outstr)
{
	
	if (variableName.empty())
	{
		outstr = std::string{ '\0' };
		return false;
	}
	
	DWORD size = GetEnvironmentVariableA(variableName.c_str(), nullptr, 0);

	outstr = std::string(size, '\0');
	if (GetEnvironmentVariableA(variableName.c_str(), outstr.data(), size))
	{
		//this is needed to remove the null-terminator from the string. 
		outstr.erase(std::find(outstr.begin(), outstr.end(), '\0'), outstr.end());
		return true;
	}

	return false;
}

//string manipulation bullshit
//works in-place
void substituteEnvironmentVariablesInString(std::string& str)
{
	std::size_t i{ 0 };
	while (str.find('%', i) != std::string::npos)
	{
		std::size_t percentPos1 = str.find('%', i);

		if (std::size_t percentPos2 = str.find('%', percentPos1 + 1); percentPos2 != std::string::npos)
		{
			std::string replacement{};
			if (queryEnvironmentVariable(str.substr(percentPos1 + 1, percentPos2 - percentPos1 - 1), replacement))
			{				
				str.replace(percentPos1, percentPos2 - percentPos1 + 1, replacement);
				i = percentPos1 + replacement.size();
			}
			else
				i = percentPos2 + 1;
		}
		else
			break;
		
	}
}
