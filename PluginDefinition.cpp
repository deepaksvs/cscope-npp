//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
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

#include <stdio.h>

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include <pluginMain.h>
#include <ShlObj.h>
#include <input.h>
#include <InputToJumpDlg.h>
#include <PosListDlg.h>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;
HINSTANCE hInst;
static bool projectOpened = false;
InputToJumpDlg dlgInputToJump;
PosListDlg dlgOutList;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	hInst = (HINSTANCE)hModule;  
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
	ShortcutKey		*kyOpenFile = new ShortcutKey;
	ShortcutKey		*kySymb		= new ShortcutKey;
	ShortcutKey		*kyDef		= new ShortcutKey;
	ShortcutKey		*kyCallee	= new ShortcutKey;
	ShortcutKey		*kyPrev		= new ShortcutKey;

	kyOpenFile->_isAlt = true;
	kyOpenFile->_isCtrl = false;
	kyOpenFile->_isShift = false;
	kyOpenFile->_key = 0x4F; /* alt + o */

	kyDef->_isAlt = false;
	kyDef->_isCtrl = true;
	kyDef->_isShift = false;
	kyDef->_key = 0x5D; /* ctrl + ] */

	kySymb->_isAlt = true;
	kySymb->_isCtrl = true;
	kySymb->_isShift = false;
	kySymb->_key = 0x5D; /* ctrl + alt + ] */

	kyCallee->_isAlt = true;
	kyCallee->_isCtrl = false;
	kyCallee->_isShift = false;
	kyCallee->_key = 0x67; /* crtl + , */
	
	kyPrev->_isAlt = true;
	kyPrev->_isCtrl = true;
	kyPrev->_isShift = false;
	kyPrev->_key = 0x74; /* crtl + t */

    setCommand(0, TEXT("Create Project"), CreateProject, NULL, false);
    setCommand(1, TEXT("Open Project"), OpenProject, NULL, false);
	setCommand(2, TEXT("Open File"), csOpenFile, kyOpenFile, false);
	setCommand(3, TEXT("Find Definition"), csFindDefinition, kyDef, false);
	setCommand(4, TEXT("Find Symbol"), csFindSymbol, kySymb, false);
	setCommand(5, TEXT("Find Callee"), csFindCallee, kyCallee, false);
	setCommand(6, TEXT("Prev History"), csPrevious, kyPrev, false);
	setCommand(7, TEXT("Call Tree"), csCallTree, NULL, false);
	setCommand(8, TEXT("Grep"), csEGrep, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
static TCHAR filename[MAX_PATH] = {0};
static char	file[MAX_PATH] = {0};
static char	source[MAX_PATH] = {0};
static commands_e gCommand = command_none;
int csFindCommon(commands_e command, bool error);
int jumpTo (const char *file, int lineNo);
int pushHistory (void);
void csOutputDiag (int count);

void showInput (void)
{
	dlgInputToJump.init((HINSTANCE)hInst, nppData);
	dlgInputToJump.doDialog(false);
}

void csGooo (TCHAR *text)
{
	int		refs = 0;
	char word[100];

	if (!projectOpened) {
		::MessageBox(NULL, TEXT("Project Not opened"), TEXT("SourceNav"), MB_OK);
	}

	wcstombs(word, text, wcslen(text) + 1);
	exec_command(gCommand, word);
	refs = buildCrossRefs();
	if (refs) {

		if (refs > 1) {
			pushHistory();
			csOutputDiag(refs);
		}
		else  {
			/* Jump to file and line number */
			pushHistory();
			jumpTo (refInfo[0].filename, refInfo[0].lineno - 1);
		}
	}
	else {
		::MessageBox(NULL, TEXT("No references found"), TEXT("SourceNav"), MB_OK);
	}
	//gCommand = command_none;
	gCommand = command_none;
}

void CreateProject()
{
	OPENFILENAME		brw;
    int which = -1;
	TCHAR cwd[MAX_PATH];

    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

	HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	GetCurrentDirectory(sizeof(cwd), cwd);

	memset (&brw, 0, sizeof(brw));
	brw.lStructSize = sizeof(brw);
	brw.hwndOwner = curScintilla;
	brw.lpstrFile = filename;
	brw.nMaxFile  = sizeof(filename) / sizeof(TCHAR);

	::MessageBox(NULL, TEXT("Enter Project Name"), TEXT("SourceNav"), MB_OK);
	::GetOpenFileName(&brw);

	wcstombs(file, filename, wcslen(filename) + 1);
	filename[0] = 0;
	filename[1] = 0;

	::MessageBox(NULL, TEXT("Select source Directory"), TEXT("SourceNav"), MB_OK);
	BROWSEINFO fldr;
	PIDLIST_ABSOLUTE var;

	memset (&fldr, 0, sizeof(fldr));
	fldr.hwndOwner = curScintilla;
	fldr.pszDisplayName = filename;
	var = ::SHBrowseForFolder(&fldr);
	::SHGetPathFromIDList(var, filename);
	wcstombs(source, filename, wcslen(filename) + 1);
	const char *ndir[] = {source};
	SetCurrentDirectory(cwd);
	createproject(file, 1, ndir);
	projectOpened = true;
}

void OpenProject()
{
	OPENFILENAME		brw;
    int which = -1;
	TCHAR cwd[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

	HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	memset (&brw, 0, sizeof(brw));
	brw.lStructSize = sizeof(brw);
	brw.hwndOwner = curScintilla;
	brw.lpstrFile = filename;
	brw.nMaxFile  = sizeof(filename) / sizeof(TCHAR);
	GetCurrentDirectory(sizeof(cwd), cwd);
	::GetOpenFileName(&brw);
	SetCurrentDirectory(cwd);
	wcstombs(file, filename, wcslen(filename) + 1);
	openproject(file);
	projectOpened = true;
}

static TCHAR		wcWord[250] = {0};
static crossRefs	his;

void getCurrentWord(char *word, int len) 
{
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTWORD, MAX_PATH, (LPARAM)wcWord);
	wcstombs(word, wcWord, wcslen(wcWord) + 1);
}

void getCurrentFile(char *word, int len) 
{
	::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, MAX_PATH, (LPARAM)wcWord);
	wcstombs(word, wcWord, wcslen(wcWord) + 1);
}

int getCurrentLine(void) 
{
	int curLine = 0;
	curLine = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLINE, MAX_PATH, (LPARAM)&curLine);
	return curLine;
}

int pushHistory (void)
{
	his.lineno = getCurrentLine();
	getCurrentFile(his.filename, sizeof(his.filename));
	return add_history(&his);
}

int jumpTo (const char *file, int lineNo)
{
    int				which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return which;

	HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	mbstowcs(wcWord, file, strlen(file) + 1);
	::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)wcWord);
	::SendMessage(curScintilla, SCI_GOTOLINE, lineNo, 0);
	return 1;
}

int csFindCommon(commands_e command, bool error)
{
	char	word[100] = {0};
	int		refs = 0;

	if (!projectOpened) {
		::MessageBox(NULL, TEXT("Project Not opened"), TEXT("SourceNav"), MB_OK);
		return refs;
	}

	getCurrentWord(word, sizeof(word));
	exec_command(command, word);
	refs = buildCrossRefs();
	if (refs) {

		if (refs > 1) {
			pushHistory();
			csOutputDiag(refs);
		}
		else  {
			/* Jump to file and line number */
			pushHistory();
			jumpTo (refInfo[0].filename, refInfo[0].lineno - 1);
		}
	}
	else {
		if (error)
			::MessageBox(NULL, TEXT("No references found"), TEXT("SourceNav"), MB_OK);
	}
	//gCommand = command_none;
	return refs;
}

void csOutputDiag (int count)
{
    dlgOutList.init(hInst, NULL);
    dlgOutList.setData(refInfo, count);
    dlgOutList.setParent(nppData._nppHandle);
    dlgOutList.doDialog(false, false);

}

void csFindDefinition()
{
	if (!csFindCommon(find_def, false)) {
		(void) csFindSymbol();
	}
}

void csOpenFile()
{
	gCommand = find_file;
	showInput();
	//(void) csFindCommon(find_file, true);
}

void csFindCallee()
{
	//(void) csFindCommon(find_calling, true);
	gCommand = find_calling;
	showInput();
}

void csFindSymbol()
{
	//(void) csFindCommon(find_symbol, true);
	gCommand = find_symbol;
	showInput();
}

void csPrevious()
{
	if (pop_history (&his)) {
		jumpTo(his.filename, his.lineno);
	}
	return;
}

void csCallTree()
{
	/*
	char	word[100];
	callh	*tree;
	FILE	*fp = fopen("D:\\tree.out", "w");

	getCurrentWord(word, sizeof(word));
	tree = build_call_tree(word);
	display_call_tree(tree, fp);
	fclose (fp);
	free_call_tree (tree);
	*/
}

void csEGrep()
{
	//(void) csFindCommon(find_regex, true);
	gCommand = find_regex;
	showInput();
}
