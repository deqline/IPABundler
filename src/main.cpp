#include <fstream>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <string>
#include <conio.h>
#include <process.h>
#include "../shared/zip.h"
#include "../shared/unzip.h"
#include <algorithm>

using std::cout;
using std::endl;
using std::string;

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	string cmd;
	HZIP hz = NULL;
	bool deleteRoot = false;

	if (argc <= 1 || !argv[1]) {
		cout << "Usage: apptoipa <file>" << endl;
		return EXIT_FAILURE;
	}

	fs::path app = argv[1];
	fs::path oldApp = app;
	string outputName = (app.stem().string()).append(".zip");
	fs::path current_path = std::filesystem::current_path();
	string root_path = current_path.string();
	root_path += "\\";

	fs::path tempPath = fs::temp_directory_path();
	tempPath += outputName;
	DWORD file = GetFileAttributesA(app.string().c_str());

	if (file == INVALID_FILE_ATTRIBUTES)
	{
		cout << "Failed. Does the file/folder exist?" << endl;
		return EXIT_FAILURE;
	}

	if (app.extension() == ".app") {
		for (size_t i = 0; i < argc; i++)
		{
			if (argv[i][0] == '-') {
				if (argv[i][1] == 'd') {
					deleteRoot = true;
				}
			}
		}
		cout << "[+] Converting .APP to .IPA " << endl;
		hz = CreateZip(tempPath.string().c_str(), 0);
		if (!hz)
		{
			cout << "[-] An error occured during conversion" << endl;
			return EXIT_FAILURE;
		}
		for (auto& p : fs::recursive_directory_iterator(app))
		{
			string path = p.path().string();
			auto index = path.find("\\");
			if (index <= path.length())
				path.erase(0, index + 1);
			ZipAdd(hz, path.c_str(), p.path().string().c_str());
		}
		CloseZip(hz);
		app = outputName;
		app.replace_extension(".ipa");
		cmd.append("move /y ").append(tempPath.string()).append(" ").append(tempPath.replace_extension(".ipa").string()).append(" > nul && move ")
			.append(tempPath.string()).append(" ").append(root_path.append(app.string())).append(" > nul");
		system(cmd.c_str());
		if (deleteRoot) {
			cout << "[+] Cleaning up..." << endl;
			cmd.clear();
			cmd.append("rmdir /Q /S ").append(current_path.string().append("\\")).append(oldApp.string()).append(" > nul");
			system(cmd.c_str());
		}
	}
	else if (app.extension() == ".ipa")
	{
		cout << "[+] Converting .IPA to .APP " << endl;
		cmd.append("move ").append(app.string()).append(" ").append(app.replace_extension(".zip").string()).append(" > nul");
		system(cmd.c_str());

		hz = OpenZip(app.string().c_str(), 0);
		if (!hz)
		{
			cout << "[-] An error occured during conversion" << endl;
			return EXIT_FAILURE;
		}

		ZIPENTRY ze; GetZipItem(hz, -1, &ze); int numitems = ze.index;

		for (int zi = 0; zi < numitems; zi++)
		{
			ZIPENTRY ze; GetZipItem(hz, zi, &ze);
			const char* fileName = string(app.stem().string().append(".app/").append(ze.name)).c_str();
			UnzipItem(hz, zi, fileName);
		}
		CloseZip(hz);
		cout << "[+] Cleaning up..." << endl;
		cmd.clear();
		cmd.append("del /F /Q ").append(root_path).append(app.string()).append(" > nul");
		system(cmd.c_str());
		app.replace_extension(".app");
		root_path.append(app.string());
	}
	else {
		cout << "You must provide a .app folder or a .ipa for convertion!" << endl;
		return EXIT_FAILURE;
	}
	cout << "[+] Finishing up..." << endl;
	cout << "[+] Success! Output to " << root_path << endl;
	return EXIT_SUCCESS;
}