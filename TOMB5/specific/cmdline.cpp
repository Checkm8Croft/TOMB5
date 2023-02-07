#include "../tomb5/pch.h"
#include "cmdline.h"
#include "function_stubs.h"
#include "../game/gameflow.h"

#define nDDDevice	VAR_(0x0057A094, long)
#define nD3DDevice	VAR_(0x0057A084, long)

void CLSetup(char* cmd)
{
	Log(2, "CLSetup");

	if (cmd)
		start_setup = 0;
	else
		start_setup = 1;
}

void CLNoFMV(char* cmd)
{
	Log(2, "CLNoFMV");

	if (cmd)
		fmvs_disabled = 0;
	else
		fmvs_disabled = 1;
}

void InitDSDevice(HWND dlg, HWND hwnd)
{
	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

	for (int i = 0; i < App.DXInfo.nDSInfo; i++)
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)App.DXInfo.DSInfo[i].About);

	if (!App.DXInfo.nDSInfo)
	{
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)SCRIPT_TEXT(STR_NO_SOUND_CARD_INSTALLED));
		EnableWindow(GetDlgItem(dlg, 1018), 0);
		SendMessage(GetDlgItem(dlg, 1018), BM_SETCHECK, 1, 0);
		EnableWindow(hwnd, 0);
	}

	SendMessage(hwnd, CB_SETCURSEL, 0, 0);
}

void InitTFormats(HWND dlg, HWND hwnd)
{
	DXD3DDEVICE* device;
	DXTEXTUREINFO* tex;
	long bpp, r, g, b, a;
	char buffer[40];
	bool software;

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	EnableWindow(GetDlgItem(dlg, 1006), 1);
	software = SendMessage(GetDlgItem(dlg, 1011), BM_GETCHECK, 0, 0);
	device = &App.DXInfo.DDInfo[nDDDevice].D3DDevices[nD3DDevice];

	for (int i = 0; i < device->nTextureInfos; i++)
	{
		tex = &device->TextureInfos[i];
		bpp = tex->bpp;
		r = tex->rbpp;
		g = tex->gbpp;
		b = tex->bbpp;
		a = tex->abpp;

		wsprintf(buffer, "%d %s RGBA %d%d%d%d", bpp, SCRIPT_TEXT(STR_BIT), r, g, b, a);
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)buffer);

		if (software)
		{
			if (bpp == 32 && r == 8 && b == 8 && g == 8 && a == 8)
			{
				SendMessage(hwnd, CB_SETCURSEL, i, 0);
				EnableWindow(GetDlgItem(dlg, 1006), 0);
			}
		}
		else if (bpp == 16 && r == 5 && b == 5 && g == 5 && a == 1)
			SendMessage(hwnd, CB_SETCURSEL, i, 0);
		else
			SendMessage(hwnd, CB_SETCURSEL, 0, 0);
	}
}

void inject_cmdline(bool replace)
{
	INJECT(0x00495B70, CLSetup, replace);
	INJECT(0x00495BA0, CLNoFMV, replace);
	INJECT(0x00495C40, InitDSDevice, replace);
	INJECT(0x00495D50, InitTFormats, replace);
}
