#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "InputToJumpDlg.h"
#include "PluginDefinition.h"
#include "resource.h"

void InputToJumpDlg::doDialog(bool isRTL) {
	if (!isCreated())
		create(IDD_INPUTTOJUMP, isRTL);
  
	SetDlgItemText(_hSelf, IDC_DEFINITION, TEXT(""));
	::SetFocus(::GetDlgItem(_hSelf, IDC_DEFINITION));
	goToCenter();
};

BOOL CALLBACK InputToJumpDlg::run_dlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) 
	{
    case WM_INITDIALOG :
		{
      /* Change dialog lang */
			//NLChangeDialog(_hInst, _nppData._nppHandle, _hSelf, _T("Demo"));
			return TRUE;
		}
		case WM_COMMAND : 
		{
			switch (wParam)
			{
				case IDOK:
				{
					TCHAR input_text[100];
					GetDlgItemText(_hSelf, IDC_DEFINITION, input_text, sizeof(input_text));
					display(FALSE);
					if (wcslen(input_text)) {
						csGooo(input_text);
					}
					break;

				}

				case IDCANCEL:
						  display(FALSE);
						  return FALSE;

				default :
					  break;
			}
		}
	}
	return FALSE;
}
