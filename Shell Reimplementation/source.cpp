
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
#include <winbase.h>
#include <shellapi.h>
#include <aclapi.h>


std::optional<std::filesystem::path> searchThroughPATH(std::string&, std::string&);
bool hasExecutePermissions(const std::filesystem::path&);

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
	

	std::string path{ getenv("PATH")};
//	std::string path = "\
//C:\\MinGW\\bin;\
//%USERPROFILE%;C:\\Games;";

	std::cout << "A Shell Reimplementation by 56dev_. (2025)" << "\n";
	std::cout << "ONLY FOR WINDOWS!\n" << std::endl;

	std::cout << "NOTICE: this is a developer build!" << std::endl;

	#ifndef _WIN32
		std::cout << "This machine is not on Windows. You cannot use the shell." << std::endl;
		return 0;
	#endif

	while (true)
	{

		std::cout << "$ ";

		std::string               userLine{};

		std::getline(std::cin, userLine);

		if (userLine.empty())
		{
			continue;
		}

		//echo processes parameters differently
		if (userLine.substr(0, 4) == prefixes[1])
		{
			std::cout << userLine << "\n";
			continue;

		}

		std::vector<std::string> userInput{};
		std::stringstream     sm{ userLine };
		std::string              temporary{};

		while (sm >> temporary)
			userInput.push_back(temporary);

		//exit
		if (userInput[0] == prefixes[0])
			return 0;
		//type
		else if (userInput[0] == prefixes[2])
		{

			if (std::find(prefixes.begin(), prefixes.end(), userInput[1]) != std::end(prefixes))
				std::cout << userInput[1] << " is a builtin command." << std::endl;
			else
			{
				std::optional<std::filesystem::path> foundPath{ searchThroughPATH(path, userInput[1]) };

				if (foundPath)
					std::cout << userInput[1] << " is in " << foundPath.value().replace_filename("") << std::endl;
				else
					std::cout << userInput[1] << " is not recognized as a valid command!" << std::endl;
			}			
			
		}
		//path
		else if (userInput[0] == prefixes[3])
		{
			std::stringstream ss{ path };
			std::string pathTemporary{};
			while (std::getline(ss, pathTemporary, ';'))
				std::cout << pathTemporary << "\n";

			std::cout << std::endl;			
		}
		else
		{			
			std::optional<std::filesystem::path> foundPath{ searchThroughPATH(path, userInput[0]) };
						
			if (foundPath)
			{
				std::string cmd{};
				for (int i{ 0 }; i < userInput.size(); ++i)
				{
					cmd.append(userInput[i]).append(" ");
				}
				STARTUPINFOA startupInfo{ {sizeof(startupInfo)} };
				PROCESS_INFORMATION processInfo{};

				if (CreateProcessA(foundPath.value().string().c_str(), &cmd.front(), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
				{
					WaitForSingleObject(processInfo.hProcess, INFINITE);
					CloseHandle(processInfo.hThread);
					CloseHandle(processInfo.hProcess);
				}
			}
			else
				std::cout << "\"" << userInput[0] << "\"" << " not recognized as a valid command!" << std::endl;
		}
		
	}

	return 0;
}

std::optional<std::filesystem::path> searchThroughPATH(std::string& path, std::string& param)
{
	std::stringstream stream{ path };

	std::string directoryString{};
	
	while (getline(stream, directoryString, ';'))
	{
		std::error_code ec{};

		if (!std::filesystem::is_directory(directoryString, ec))
			continue;

		if (ec)
			std::cerr << ec << std::endl;
		ec.clear();
		
		for (const auto& entry : std::filesystem::directory_iterator{ directoryString, std::filesystem::directory_options::skip_permission_denied })
		{			
			

			if (param != entry.path().stem().string())
				continue;
			std::error_code ec1{};

			if (!entry.is_regular_file(ec1) || ec1)
			{
				
				std::cout << "entry is not a file, or access failed!" << std::endl;
				continue;
			}
			
			if (entry.path().extension() != ".exe")
			{
				
				std::cout << "file is not an executable!" << std::endl;
				continue;
			}

			if (!hasExecutePermissions(entry.path()))
			{
				continue;
			}
			
			return entry.path();
			
		}	
	}
	return std::nullopt;
}

// (a lot of bullshit)
bool hasExecutePermissions(const std::filesystem::path& path)
{
	PSECURITY_DESCRIPTOR pSD{ nullptr};
	PACL pDACL{ nullptr };

	DWORD result = GetNamedSecurityInfo(
		path.c_str(),
		SE_FILE_OBJECT,
		OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
		nullptr,
		nullptr,
		&pDACL,
		nullptr,
		&pSD
	);
	

	if (result != ERROR_SUCCESS)
	{
		std::cout << "unable to get sec info" << std::endl;
		return false;
	}
	
	HANDLE hToken{ nullptr };
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE, &hToken))
	{
		LocalFree(pSD);
		std::cout << "unable to open process token" << std::endl;
		return false;
	}

	HANDLE hImpersonatedToken{ nullptr };
	if (!DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken))
	{
		CloseHandle(hToken);
		LocalFree(pSD);
		std::cout << "unable to duplicate token" << std::endl;
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

	if (!AccessCheck(pSD, hImpersonatedToken, FILE_EXECUTE, &mapping, &privileges, &privilegeSetLength, &grantedAccess, &accessStatus))
	{
		std::cout << "access check was unable to succeed" << std::endl;
		std::cout << GetLastError() << std::endl;
		accessStatus = FALSE;
	}

	CloseHandle(hImpersonatedToken);
	CloseHandle(hToken);
	LocalFree(pSD);

	if (!accessStatus)
		std::cout << "access check rejected the request" << std::endl;

	return accessStatus != FALSE;
}





