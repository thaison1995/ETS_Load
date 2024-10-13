#include "Main.h"

static HFONT LoadingFont = NULL;

void CreateFolder(string path)
{
	if (!CreateDirectory(path.c_str(), NULL))
	{
		return;
	}
}

string GetFileExtension(const string& st)
{
	size_t pos = st.rfind('.');

	if (pos == string::npos)
		return "";

	return st.substr(pos + 1);
}

vector<string> get_all_files_names_within_folder(string folder)
{
	vector<string> names;
	string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
				names.push_back(fd.cFileName);
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

void __fastcall ShowMsgBox(bool bexit, char* Msg...)
{
	va_list arguments;
	va_start(arguments, Msg);

	int len = _vscprintf(Msg, arguments) + 1;
	char* text = new char[len];
	memset(text, 0, len);
	vsprintf_s(text, len, Msg, arguments);
	va_end(arguments);

	MessageBox(0, text, "Warning", MB_ICONINFORMATION);
	delete[] text;

	if (bexit)
	{
		exit(-1);
	}
}

namespace MPQ
{
	DWORD MpqLoadPriority = 15;
	bool OpenArchive(const string& fileName, HANDLE* mpqHandle)
	{
		if (!SFileOpenArchive(fileName.c_str(), MpqLoadPriority, 0, mpqHandle))
			return false;

		MpqLoadPriority++;

		return true;
	}
}

void ETS_Load()
{
	CreateFolder("ETS_Load");
	vector<string> listFiles = get_all_files_names_within_folder("ETS_Load");
	for (auto lFile : listFiles)
	{
		string szFileName = GetFileExtension(lFile);
		lFile = string("ETS_Load\\") + lFile;
		if (szFileName == "mpq")
		{
			HANDLE MPQHandleEx = 0;
			if (!MPQ::OpenArchive(lFile, &MPQHandleEx))
			{
				ShowMsgBox(1, "Cannot Load MPQ File: %s", lFile.c_str());
			}
		}
		else
		{
			if (LoadLibrary(lFile.c_str()) == NULL)
			{
				ShowMsgBox(1, "Cannot Load File: %s", lFile.c_str());
			}
		}
	}
}

BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hinstDLL);

			HWND win = GetActiveWindow();
			HDC dc;
			RECT rect;
			if (win && (dc = GetDC(win)) != NULL && TRUE == GetClientRect(win, &rect))
			{
				LoadingFont = CreateFont(
					14, //__in  int nHeight,
					0, //__in  int nWidth,
					0, //__in  int nEscapement,
					0, //  int nOrientation,
					FW_BOLD, //__in  int fnWeight,
					false, //__in  DWORD fdwItalic,
					false, //__in  DWORD fdwUnderline,
					false, //__in  DWORD fdwStrikeOut,
					ANSI_CHARSET, //__in  DWORD fdwCharSet,
					OUT_OUTLINE_PRECIS, //__in  DWORD fdwOutputPrecision,
					CLIP_DEFAULT_PRECIS, //__in  DWORD fdwClipPrecision,
					CLEARTYPE_QUALITY, //__in  DWORD fdwQuality,
					VARIABLE_PITCH, //__in  DWORD fdwPitchAndFamily,
					"Verdana"
				);
				UINT align = GetTextAlign(dc);
				HFONT font = (HFONT)GetCurrentObject(dc, OBJ_FONT);
				COLORREF color = SetTextColor(dc, RGB(0x2E, 0xB0, 0xDF));
				int bkmode = SetBkMode(dc, TRANSPARENT);

				SetTextAlign(dc, TA_CENTER | VTA_CENTER);
				SelectObject(dc, LoadingFont);
				const char* szText = "...Loading...";
				TextOut(dc, (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2, szText, strlen(szText));

				RedrawWindow(win, NULL, NULL, RDW_UPDATENOW);

				SetBkMode(dc, bkmode);
				SetTextColor(dc, color);
				SetTextAlign(dc, align);
				SelectObject(dc, font);
			}
			ETS_Load();
			break;
		}

		case DLL_PROCESS_DETACH:
		{
			if (LoadingFont)
			{
				DeleteObject(LoadingFont);
				LoadingFont = NULL;
			}
			break;
		}
	}

	return TRUE;
}