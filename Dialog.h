#ifndef _INPUTTOJUMP_H_
#define _INPUTTOJUMP_H_

#include <windows.h>
#include <staticdialog.h>
#include <PluginInterface.h>
#include <resource.h>

class InputToJumpDlg : public StaticDialog
{
public:
  InputToJumpDlg(void) {};
  ~InputToJumpDlg(void) {};
  void init(HINSTANCE hInst, NppData nppData)
    {
        _nppData = nppData;
        Window::init(hInst, nppData._nppHandle);
    };

  void setParent(HWND parent)
  {
    _hParent = parent;
  }

  void doDialog(bool isRTL = false);

protected :
	virtual BOOL CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	/* Handles */
	NppData			_nppData;
  HWND			_HSource;
};

#endif //_INPUTTOJUMP_H_