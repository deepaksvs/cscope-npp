//this file is part of notepad++
//Copyright (C)2003 Don HO ( donho@altern.org )
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

#ifndef POSTLIST_DLB_H
#define POSTLIST_DLB_H

#include "StaticDialog.h"
#include "PluginInterface.h"
#include "PluginDefinition.h"
#include "resource.h"
#include <pluginMain.h>

extern NppData nppData;

class PosListDlg : public StaticDialog
{
public :
    PosListDlg() : StaticDialog()
    {
    };

    void doDialog(bool isRTL=false, bool bTrace=true);
   
    void setParent(HWND parent2set){
		  _hParent = parent2set;
	  };

    void setData(crossRefs *data, int dsize)
    {
      m_data = data;
      m_datasize = dsize;
    }

protected :
	  virtual BOOL CALLBACK run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  void _jumpToSelected();
  crossRefs *m_data;
  int m_datasize;
  bool m_bTrace;
};

#endif //POSTLIST_DLB_H
