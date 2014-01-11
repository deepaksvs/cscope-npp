#include <StaticDialog.h>
#include <windows.h>
#include <PluginInterface.h>
#include <resource.h>

class inputWindow: public StaticDialog
{
public:
	inputWindow() {}
	~inputWindow() {}

	void init (NppData npp, HINSTANCE hInst)
	{
		nppData = npp;
		Window::init(hInst, nppData._nppHandle);
	}

	void setParent (HWND parent)
	{
		_hParent = parent;
	}

#define INPUT_DIAG_RES IDD_PLUGINGOLINE_DEMO

	void showDialog (bool isRTL = false)
	{
		if (!isCreated())
			create(INPUT_DIAG_RES, isRTL);
		SetDlgItemText(_hSelf, INPUT_DIAG_RES, TEXT(""));
		::SetFocus(::GetDlgItem(_hSelf, INPUT_DIAG_RES));
		goToCenter();
	}

protected:
	virtual BOOL CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message) 
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
						return TRUE;
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

private:
	/* Handles */
	NppData			nppData;
	HWND			hSource;
};