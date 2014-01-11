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

#include <stdio.h>
#include <tchar.h>
#include <stack>

#include "PosListDlg.h"

extern int jumpTo (const char *file, int lineNo);

void PosListDlg::_jumpToSelected()
{
	/* Jump to the selected file and location*/
	int			selected;
	int			which = -1;
	crossRefs	*reference;

	selected = ::SendMessage(::GetDlgItem(_hSelf, IDC_LIST), LB_GETCURSEL, 0, 0);
	display(false);
	reference = m_data +selected;
	jumpTo (reference->filename, reference->lineno - 1);
}

void PosListDlg::doDialog(bool isRTL, bool bTrace)
{


	if (!isCreated())
		create(IDD_POSLIST, isRTL, false);
	else
		display(true);

#if 1
	int		each;
	TCHAR	buf[MAX_PATH];
	LRESULT	res;

	res = SendDlgItemMessage(_hSelf, IDC_LIST, LB_RESETCONTENT, 0, 0);

	for(each = 0; each < m_datasize; each++) {
		mbstowcs(buf, m_data[each].filename, strlen(m_data[each].filename) + 1);
		res = SendDlgItemMessage(_hSelf, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)buf);
    }

	HWND listHandle = ::GetDlgItem(_hSelf, IDC_LIST);
	::SetFocus(listHandle);
	res = SendDlgItemMessage(_hSelf, IDC_LIST, LB_SETCURSEL, (WPARAM)m_datasize, (LPARAM)-1);
    goToCenter();
#endif
};

BOOL CALLBACK PosListDlg::run_dlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{

	case WM_INITDIALOG:
		{
#if 0
			int		each;
			TCHAR	buf[MAX_PATH];
			HWND listHandle = ::GetDlgItem(_hSelf, IDC_LIST);
			if (listHandle) {
				for(each = 0; each < m_datasize; each++) {
					mbstowcs(buf, m_data[each].filename, strlen(m_data[each].filename) + 1);
					SendMessage(listHandle, LB_ADDSTRING, 0, (WPARAM)buf);
				}
				SetFocus(listHandle);
				SendMessage(listHandle, LB_GETCURSEL, 0, 0);
				goToCenter();
			}
			break;
#endif
		}
	case WM_COMMAND : 
		{
			switch(LOWORD(wParam))
			{
			case IDCANCEL:
				display(false);
				return FALSE;
			case IDOK:
				{
					_jumpToSelected();
					break;
				}
			case IDC_LIST:
				{
					int low = LOWORD(wParam);
					int high = HIWORD(wParam);
					switch(HIWORD(wParam))
					{
					case LBN_DBLCLK:
						{
							_jumpToSelected();
							break;
						}
					}
					return TRUE;
				}
			}
			return FALSE;
		}

	case WM_VKEYTOITEM:
		{
			int index;
			int count;

			index = ::SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
			count = ::SendMessage((HWND)lParam, LB_GETCOUNT, 0, 0);

			switch(LOWORD(wParam))
			{
			case VK_UP:
				if(index>0)
					return index-1;
				return 0;
			case VK_DOWN:
				if(index<count-1)
					return index+1;
				return count-1;
			case VK_ESCAPE:
				display(FALSE);
				return index;
			case VK_RETURN:
				_jumpToSelected();
				return index;
			}
		}
		break;

	default :
		break;
	}
	return FALSE;
}
