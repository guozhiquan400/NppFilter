//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "Windows.h"
#include "PluginDefinition.h"
#include "ThemeRenderer.h"
#include "NppInterface.h"


const int nbFunc = 1;

FuncItem funcItem[nbFunc];
NppData nppData;

HINSTANCE  g_hInst = nullptr;
FolderDialog folderDialog;
CreateRuleDialog createRuleDlg;

const TCHAR NPP_PLUGIN_NAME[] = TEXT("Folder");

void UpdateThemeColor();

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD  reasonForCall, LPVOID /*lpReserved*/)
{
	g_hInst = hModule;
	try {

		switch (reasonForCall)
		{
			case DLL_PROCESS_ATTACH:
				ThemeRenderer::Create();
				break;

			case DLL_PROCESS_DETACH:
				pluginCleanUp();
				ThemeRenderer::Destroy();
				break;

			case DLL_THREAD_ATTACH:
				break;

			case DLL_THREAD_DETACH:
				break;
		}
	}
	catch (...) { return FALSE; }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	NppInterface::setNppData(notpadPlusData);

	nppData = notpadPlusData;
	commandMenuInit();
	UpdateThemeColor();
	folderDialog.init(g_hInst, nppData._nppHandle);
	createRuleDlg.init(g_hInst, nppData._nppHandle);
	folderDialog.VisibleChanged([](bool visible) {
		::SendMessage(nppData._nppHandle, NPPM_SETMENUITEMCHECK, funcItem[DOCKABLE_FILTER_INDEX]._cmdID, (LPARAM)visible);
	});
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch (notifyCode->nmhdr.code) 
	{
		case NPPN_SHUTDOWN:
		{
			commandMenuCleanUp();
		}
		break;

		case NPPN_READY:
			folderDialog.initFinish();
			UpdateThemeColor();
			break;
		case NPPN_WORDSTYLESUPDATED:
			UpdateThemeColor();
			break;
		case SCN_MARGINCLICK:
		{
			if (notifyCode->margin == 2) {
				UINT currentEdit;
				::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
				HWND currentSciHandle = (0 == currentEdit) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
				
				int lineClick = (int)::SendMessage(currentSciHandle, SCI_LINEFROMPOSITION, notifyCode->position, 0);
				int level = (int)::SendMessage(currentSciHandle, SCI_GETFOLDLEVEL, lineClick, 0);
				
				if (level & SC_FOLDLEVELHEADERFLAG) {
					::SendMessage(currentSciHandle, SCI_TOGGLEFOLD, lineClick, 0);
				}
			}
		}
		break;
		default:
			return;
	}
}

void UpdateThemeColor()
{
	auto IsDarkColor = [](COLORREF rgb) -> bool {
		uint8_t r = GetRValue(rgb);
		uint8_t g = GetGValue(rgb);
		uint8_t b = GetBValue(rgb);
		float brightness = (0.2126F * r + 0.7152F * g + 0.0722F * b) / 255.0F;
		return brightness < 0.5F;
	};

	auto nppColors = NppInterface::GetColors();

	ThemeColors colors{
		.body = nppColors.darkerText,
		.body_bg = nppColors.pureBackground,
		.secondary = NppInterface::getEditorDefaultForegroundColor(),
		.secondary_bg = NppInterface::getEditorDefaultBackgroundColor(),
		.border = nppColors.edge,
		.primary = nppColors.text,
		.primary_bg = nppColors.hotBackground,
		.primary_border = nppColors.hotEdge,
	};
	auto isDarkMode = IsDarkColor(colors.body_bg);
	ThemeRenderer::Instance().SetTheme(isDarkMode, colors);
}

// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// https://github.com/notepad-plus-plus/notepad-plus-plus/issues
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
