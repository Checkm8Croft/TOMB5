#include "../tomb5/pch.h"
#include "dxshell.h"
#include "function_stubs.h"

long DDSCL_FLAGS[11] =	// for DXSetCooperativeLevel logging
{
	DDSCL_ALLOWMODEX,
	DDSCL_ALLOWREBOOT,
	DDSCL_CREATEDEVICEWINDOW,
	DDSCL_EXCLUSIVE,
	DDSCL_FPUSETUP,
	DDSCL_FULLSCREEN,
	DDSCL_MULTITHREADED,
	DDSCL_NORMAL,
	DDSCL_NOWINDOWCHANGES,
	DDSCL_SETDEVICEWINDOW,
	DDSCL_SETFOCUSWINDOW
};

const char* DDSCL_TEXT[11] =
{
	"allowmodex",
	"allowreboot",
	"createdevicewindow",
	"exclusive",
	"fpusetup",
	"fullscreen",
	"multithreaded",
	"normal",
	"nowindowchanges",
	"setdevicewindow",
	"setfocuswindow"
};

char tga_header[18] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 1, 0, 1, 16, 0 };

void DXReadKeyboard(char* KeyMap)
{
#ifndef GENERAL_FIXES	//Arsunt's fix for keyboard deadlock, reproduceable in TR5 when a debugger breakpoint is hit..
	HRESULT state;

	state = G_dxptr->Keyboard->GetDeviceState(256, KeyMap);

	if (FAILED(state))
	{
		if (state == DIERR_INPUTLOST)
			G_dxptr->Keyboard->Acquire();

		G_dxptr->Keyboard->GetDeviceState(256, KeyMap);
	}
#else
	while FAILED(G_dxptr->Keyboard->GetDeviceState(256, KeyMap))	//original only acquires keyboard at DIERR_INPUTLOST
	{
		if FAILED(G_dxptr->Keyboard->Acquire())
		{
			memset(KeyMap, 0, 256);
			break;
		}
	}
#endif
}

void DXBitMask2ShiftCnt(ulong mask, uchar* shift, uchar* count)
{
	uchar i;

	for (i = 0; !(mask & 1); i++)
		mask >>= 1;

	*shift = i;

	for (i = 0; mask & 1; i++)
		mask >>= 1;

	*count = i;
}

long DXAttempt(HRESULT r)
{
	if (SUCCEEDED(r))
		return DD_OK;

	Log(1, "ERROR : %s", DXGetErrorString(r));
	return DD_FALSE;
}

long DIAttempt(HRESULT r)
{
	if (SUCCEEDED(r))
		return DI_OK;

	Log(1, "ERROR : %s", DIGetErrorString(r));
	return r;
}

void* AddStruct(void* p, long num, long size)
{
	void* ptr;

	if (!num)
		ptr = MALLOC(size);
	else
		ptr = REALLOC(p, size * (num + 1));

	memset((char*)ptr + size * num, 0, size);
	return ptr;
}

long DXDDCreate(LPGUID pGuid, void** pDD4)
{
	LPDIRECTDRAW pDD;

	Log(2, "DXDDCreate");

	if (DXAttempt(DirectDrawCreate(pGuid, &pDD, 0)) != DD_OK)
	{
		Log(1, "DXDDCreate Failed");
		return 0;
	}

	DXAttempt(pDD->QueryInterface(IID_IDirectDraw4, pDD4));

	if (pDD)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "DirectDraw", pDD, pDD->Release());
		pDD = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "DirectDraw");

	Log(5, "DXDDCreate Successful");
	return 1;
}

long DXD3DCreate(LPDIRECTDRAW4 pDD4, void** pD3D)
{
	Log(2, "DXD3DCreate");

	if (DXAttempt(pDD4->QueryInterface(IID_IDirect3D3, pD3D)) != DD_OK)
	{
		Log(1, "DXD3DCreate Failed");
		return 0;
	}

	Log(5, "DXD3DCreate Successful");
	return 1;
}

long DXSetCooperativeLevel(LPDIRECTDRAW4 pDD4, HWND hwnd, long flags)
{
	char* ptr;
	char buf[1024];

	strcpy(buf, "DXSetCooperativeLevel - ");
	ptr = &buf[strlen(buf)];

	for (int i = 0; i < 11; i++)
	{
		if (DDSCL_FLAGS[i] & flags)
		{
			strcpy(ptr, DDSCL_TEXT[i]);
			ptr += strlen(DDSCL_TEXT[i]);
			*ptr++ = ' ';
			*ptr = '\0';
		}
	}

	*(ptr - 1) = '\0';
	Log(2, buf);

	if (DXAttempt(pDD4->SetCooperativeLevel(hwnd, flags)) != DD_OK)
		return 0;

	return 1;
}

HRESULT __stdcall DXEnumDisplayModes(DDSURFACEDESC2* lpDDSurfaceDesc2, LPVOID lpContext)
{
	DXDIRECTDRAWINFO* DDInfo;
	DXDISPLAYMODE* DM;
	long nDisplayModes;

	DDInfo = (DXDIRECTDRAWINFO*)lpContext;
	nDisplayModes = DDInfo->nDisplayModes;
	DDInfo->DisplayModes = (DXDISPLAYMODE*)AddStruct(DDInfo->DisplayModes, nDisplayModes, sizeof(DXDISPLAYMODE));
	DM = &DDInfo->DisplayModes[nDisplayModes];
	DM->w = lpDDSurfaceDesc2->dwWidth;
	DM->h = lpDDSurfaceDesc2->dwHeight;
	DM->bpp = lpDDSurfaceDesc2->ddpfPixelFormat.dwRGBBitCount;
	DM->bPalette = (lpDDSurfaceDesc2->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) >> 5;
	DM->RefreshRate = lpDDSurfaceDesc2->dwRefreshRate;
	memcpy(&DM->ddsd, lpDDSurfaceDesc2, sizeof(DM->ddsd));

	if (DM->bPalette)
		Log(3, "%d x %d - %d Bit - Palette", DM->w, DM->h, DM->bpp);
	else
	{
		DXBitMask2ShiftCnt(lpDDSurfaceDesc2->ddpfPixelFormat.dwRBitMask, &DM->rshift, &DM->rbpp);
		DXBitMask2ShiftCnt(lpDDSurfaceDesc2->ddpfPixelFormat.dwGBitMask, &DM->gshift, &DM->gbpp);
		DXBitMask2ShiftCnt(lpDDSurfaceDesc2->ddpfPixelFormat.dwBBitMask, &DM->bshift, &DM->bbpp);
		Log(3, "%d x %d - %d Bit - %d%d%d", DM->w, DM->h, DM->bpp, DM->rbpp, DM->gbpp, DM->bbpp);
	}

	DDInfo->nDisplayModes++;
	return DDENUMRET_OK;
}

HRESULT __stdcall DXEnumZBufferFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
{
	DXD3DDEVICE* d3d;
	DXZBUFFERINFO* zbuffer;
	long nZBufferInfos;

	d3d = (DXD3DDEVICE*)lpContext;
	nZBufferInfos = d3d->nZBufferInfos;
	d3d->ZBufferInfos = (DXZBUFFERINFO*)AddStruct(d3d->ZBufferInfos, nZBufferInfos, sizeof(DXZBUFFERINFO));
	zbuffer = &d3d->ZBufferInfos[nZBufferInfos];
	memcpy(&zbuffer->ddpf, lpDDPixFmt, sizeof(DDPIXELFORMAT));
	zbuffer->bpp = lpDDPixFmt->dwRGBBitCount;
	Log(3, "%d Bit", zbuffer->bpp);
	d3d->nZBufferInfos++;
	return D3DENUMRET_OK;
}

HRESULT __stdcall DXEnumTextureFormats(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext)
{
	DXD3DDEVICE* d3d;
	DXTEXTUREINFO* tex;
	long nTextureInfos;

	if (!(lpDDPixFmt->dwFlags & DDPF_ALPHAPIXELS) || !(lpDDPixFmt->dwFlags & DDPF_RGB))
		return DDENUMRET_OK;

	d3d = (DXD3DDEVICE*)lpContext;
	nTextureInfos = d3d->nTextureInfos;
	d3d->TextureInfos = (DXTEXTUREINFO*)AddStruct(d3d->TextureInfos, nTextureInfos, sizeof(DXTEXTUREINFO));
	tex = &d3d->TextureInfos[nTextureInfos];
	memcpy(&tex->ddpf, lpDDPixFmt, sizeof(DDPIXELFORMAT));

	if (lpDDPixFmt->dwFlags & DDPF_PALETTEINDEXED8)
	{
		tex->bPalette = 1;
		tex->bpp = 8;
		Log(3, "8 Bit");
	}
	else if (lpDDPixFmt->dwFlags & DDPF_PALETTEINDEXED4)
	{
		tex->bPalette = 1;
		tex->bpp = 4;
		Log(3, "4 Bit");
	}
	else
	{
		tex->bPalette = 0;
		tex->bpp = lpDDPixFmt->dwRGBBitCount;

		if (lpDDPixFmt->dwFlags & DDPF_RGB)
		{
			if (lpDDPixFmt->dwFlags & DDPF_ALPHAPIXELS)
			{
				DXBitMask2ShiftCnt(lpDDPixFmt->dwRBitMask, &tex->rshift, &tex->rbpp);
				DXBitMask2ShiftCnt(lpDDPixFmt->dwGBitMask, &tex->gshift, &tex->gbpp);
				DXBitMask2ShiftCnt(lpDDPixFmt->dwBBitMask, &tex->bshift, &tex->bbpp);
				DXBitMask2ShiftCnt(lpDDPixFmt->dwRGBAlphaBitMask, &tex->ashift, &tex->abpp);
				Log(3, "%d Bit %d%d%d%d RGBA", tex->bpp, tex->rbpp, tex->gbpp, tex->bbpp, tex->abpp);
			}
			else
			{
				DXBitMask2ShiftCnt(lpDDPixFmt->dwRBitMask, &tex->rshift, &tex->rbpp);
				DXBitMask2ShiftCnt(lpDDPixFmt->dwGBitMask, &tex->gshift, &tex->gbpp);
				DXBitMask2ShiftCnt(lpDDPixFmt->dwBBitMask, &tex->bshift, &tex->bbpp);
				Log(3, "%d Bit %d%d%d RGB", tex->bpp, tex->rbpp, tex->gbpp, tex->bbpp);
			}
		}
	}

	d3d->nTextureInfos++;
	return D3DENUMRET_OK;
}

long BPPToDDBD(long BPP)
{
	switch (BPP)
	{
	case 1:
		return DDBD_1;

	case 2:
		return DDBD_2;

	case 4:
		return DDBD_4;

	case 8:
		return DDBD_8;

	case 16:
		return DDBD_16;

	case 24:
		return DDBD_24;

	case 32:
		return DDBD_32;

	default:
		return 0;
	}
}

long DXCreateD3DDevice(LPDIRECT3D3 d3d, GUID guid, LPDIRECTDRAWSURFACE4 surf, LPDIRECT3DDEVICE3* device)
{
	Log(2, "DXCreateD3DDevice");

	if (DXAttempt(d3d->CreateDevice(guid, surf, device, 0)) != DD_OK)
	{
		Log(1, "DXCreateD3DDevice Failed");
		return 0;
	}
	else
	{
		Log(2, "DXCreateD3DDevice Successful");
		return 1;
	}
}

long DXSetVideoMode(LPDIRECTDRAW4 dd, long dwWidth, long dwHeight, long dwBPP)
{
	Log(2, "DXSetVideoMode");
	Log(5, "SetDisplayMode - %dx%dx%d", dwWidth, dwHeight, dwBPP);

	if (DXAttempt(dd->SetDisplayMode(dwWidth, dwHeight, dwBPP, 0, 0)) != DD_OK)
		return 0;

	return 1;
}

long DXCreateSurface(LPDIRECTDRAW4 dd, LPDDSURFACEDESC2 desc, LPDIRECTDRAWSURFACE4* surf)
{
	Log(2, "DXCreateSurface");

	if (DXAttempt(dd->CreateSurface(desc, surf, 0)) == DD_OK)
		return 1;

	Log(1, "DXCreateSurface Failed");
	return 0;
}

long DXCreateViewport(LPDIRECT3D3 d3d, LPDIRECT3DDEVICE3 device, long w, long h, LPDIRECT3DVIEWPORT3* viewport)
{
	D3DVIEWPORT2 vp2;

	Log(2, "DXCreateViewport");


	if (DXAttempt(d3d->CreateViewport(viewport, 0)) != DD_OK)
		return 0;

	if (DXAttempt(device->AddViewport(*viewport)) != DD_OK)
		return 0;

	memset(&vp2, 0, sizeof(D3DVIEWPORT2));
	vp2.dwSize = sizeof(D3DVIEWPORT2);
	vp2.dvClipWidth = (float)w;
	vp2.dvClipHeight = (float)h;
	vp2.dwX = 0;
	vp2.dwY = 0;
	vp2.dvClipX = 0;
	vp2.dvClipY = 0;
	vp2.dvMinZ = 0;
	vp2.dvMaxZ = 1;
	vp2.dwWidth = w;
	vp2.dwHeight = h;

	if (DXAttempt((*viewport)->SetViewport2(&vp2)) != DD_OK)
		return 0;

	DXAttempt(device->SetCurrentViewport(*viewport));
	return 1;
}

void DXSaveScreen(LPDIRECTDRAWSURFACE4 surf, const char* name)
{
	FILE* file;
	DDSURFACEDESC2 desc;
	short* pSurf;
	short* pDest;
	char* pM;
	ulong val;
	static long num = 0;
	long r, g, b;
	char buf[16];

	memset(&desc, 0, sizeof(DDSURFACEDESC2));
	desc.dwSize = sizeof(DDSURFACEDESC2);
	DXAttempt(surf->GetSurfaceDesc(&desc));
	DXAttempt(surf->Lock(0, &desc, DDLOCK_WAIT, 0));
	pSurf = (short*)desc.lpSurface;
	sprintf(buf, "%s%04d.tga", name, num);
	num++;
	file = OPEN(buf, "wb");

	if (file)
	{
		*(short*)&tga_header[12] = (short)desc.dwWidth;
		*(short*)&tga_header[14] = (short)desc.dwHeight;
		WRITE(tga_header, sizeof(tga_header), 1, file);
		pM = (char*)MALLOC(2 * desc.dwWidth * desc.dwHeight);
		pDest = (short*)pM;
		pSurf += desc.dwHeight * (desc.lPitch / 2);

		for (ulong h = 0; h < desc.dwHeight; h++)
		{
			for (ulong w = 0; w < desc.dwWidth; w++)
			{
				val = pSurf[w];

				if (desc.ddpfPixelFormat.dwRBitMask == 0xF800)
				{
					r = (val >> 11) & 0x1F;
					g = (val >> 6) & 0x1F;
					b = val & 0x1F;
					*pDest++ = short((r << 10) + (g << 5) + b);
				}
				else
					*pDest++ = (short)val;
			}

			pSurf -= desc.lPitch / 2;
		}

		WRITE(pM, 2 * desc.dwWidth * desc.dwHeight, 1, file);
		CLOSE(file);
		FREE(pM);
		buf[7]++;

		if (buf[7] > '9')
		{
			buf[7] = '0';
			buf[6]++;
		}
	}

	DXAttempt(surf->Unlock(0));
}

HRESULT DXShowFrame()
{
	if (keymap[DIK_APOSTROPHE])
		DXSaveScreen(App.dx.lpBackBuffer, "Tomb");

	if (G_dxptr->lpPrimaryBuffer->IsLost())
	{
		Log(3, "Restored Primary Buffer");
		DXAttempt(G_dxptr->lpPrimaryBuffer->Restore());
	}

	if (G_dxptr->lpBackBuffer->IsLost())
	{
		Log(3, "Restored Back Buffer");
		DXAttempt(G_dxptr->lpBackBuffer->Restore());
	}

	if (!(App.dx.Flags & 0x82))
		return 0;

	if (G_dxptr->Flags & 2)
		return DXAttempt(G_dxptr->lpPrimaryBuffer->Blt(&G_dxptr->rScreen, G_dxptr->lpBackBuffer, &G_dxptr->rViewport, DDBLT_WAIT, 0));
	else
		return DXAttempt(G_dxptr->lpPrimaryBuffer->Flip(0, DDFLIP_WAIT));
}

void DXMove(long x, long y)
{
	Log(2, "DXMove : x %d y %d", x, y);

	if (G_dxptr && !(G_dxptr->Flags & 1))
		SetRect(&G_dxptr->rScreen, x, y, x + G_dxptr->dwRenderWidth, y + G_dxptr->dwRenderHeight);
}

void DXClose()
{
	Log(2, "CloseDirectX");

	if (!G_dxptr)
		return;

	if (G_dxptr->lpViewport)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Viewport", G_dxptr->lpViewport, G_dxptr->lpViewport->Release());
		G_dxptr->lpViewport = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Viewport");

	if (G_dxptr->lpD3DDevice)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Direct3DDevice", G_dxptr->lpD3DDevice, G_dxptr->lpD3DDevice->Release());
		G_dxptr->lpD3DDevice = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Direct3DDevice");

	if (G_dxptr->lpZBuffer)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Z Buffer", G_dxptr->lpZBuffer, G_dxptr->lpZBuffer->Release());
		G_dxptr->lpZBuffer = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Z Buffer");

	if (G_dxptr->lpBackBuffer)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Back Buffer", G_dxptr->lpBackBuffer, G_dxptr->lpBackBuffer->Release());
		G_dxptr->lpBackBuffer = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Back Buffer");

	if (G_dxptr->lpPrimaryBuffer)
	{
		Log(4, "Released %s @ %x - RefCnt = %d", "Primary Buffer", G_dxptr->lpPrimaryBuffer, G_dxptr->lpPrimaryBuffer->Release());
		G_dxptr->lpPrimaryBuffer = 0;
	}
	else
		Log(1, "%s Attempt To Release NULL Ptr", "Primary Buffer");

	if (!(G_dxptr->Flags & 0x40))
	{
		if (G_dxptr->lpDD)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "DirectDraw", G_dxptr->lpDD, G_dxptr->lpDD->Release());
			G_dxptr->lpDD = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "DirectDraw");

		if (G_dxptr->lpD3D)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Direct3D", G_dxptr->lpD3D, G_dxptr->lpD3D->Release());
			G_dxptr->lpD3D = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Direct3D");
	}
}

long DXCreate(long w, long h, long bpp, long Flags, DXPTR* dxptr, HWND hWnd, long WindowStyle)
{
	DXDISPLAYMODE* dm;
	LPDIRECTDRAWCLIPPER clipper;
	HWND desktop;
	DEVMODE dev;
	HDC hDC;
	DDSURFACEDESC2 desc;
	RECT r;
	long flag, CoopLevel;

	flag = 0;
	Log(2, "DXCreate");
	G_dxptr = dxptr;
	G_dxptr->Flags = Flags;
	G_dxptr->hWnd = hWnd;
	G_dxptr->WindowStyle = WindowStyle;

	if (Flags & 0x40)
		flag = 1;

	DXClose();

	if (!flag)
	{
		if (!DXDDCreate(G_dxinfo->DDInfo[G_dxinfo->nDD].lpGuid, (void**)&G_dxptr->lpDD) || !DXD3DCreate(G_dxptr->lpDD, (void**)&G_dxptr->lpD3D))
		{
			DXClose();
			return 0;
		}
	}

	if (Flags & 1)
		CoopLevel = DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE;
	else
		CoopLevel = DDSCL_NORMAL;

	if (Flags & 0x20)
		CoopLevel |= DDSCL_FPUSETUP;

	G_dxptr->CoopLevel = CoopLevel;

	if (!DXSetCooperativeLevel(G_dxptr->lpDD, hWnd, CoopLevel))
	{
		DXClose();
		return 0;
	}

	if (Flags & 1)
	{
		dm = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode];
		DXSetVideoMode(G_dxptr->lpDD, dm->w, dm->h, dm->bpp);
	}
	else
	{
		desktop = GetDesktopWindow();
		hDC = GetDC(desktop);
		ReleaseDC(desktop, hDC);
		dev.dmBitsPerPel = G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode].bpp;
		dev.dmSize = 148;	//sizeof(DEVMODE) is 156????
		dev.dmFields = DM_BITSPERPEL;
		ChangeDisplaySettings(&dev, 0);
	}

	memset(&desc, 0, sizeof(DDSURFACEDESC2));
	desc.dwSize = sizeof(DDSURFACEDESC2);

	if (Flags & 1)
	{
		desc.dwBackBufferCount = 1;
		desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		desc.ddsCaps.dwCaps = DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;

		if (!(Flags & 0x80))
		{
			desc.dwBackBufferCount = 0;
			desc.dwFlags = DDSD_CAPS;
			desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
		}

		Log(3, "Create Primary Surface");

		if (!DXCreateSurface(G_dxptr->lpDD, &desc, &G_dxptr->lpPrimaryBuffer))
		{
			DXClose();
			return 0;
		}

		if (Flags & 0x80)
		{
			Log(3, "Get Attached Back Buffer");
			desc.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;
			G_dxptr->lpPrimaryBuffer->GetAttachedSurface(&desc.ddsCaps, &G_dxptr->lpBackBuffer);
		}
		else
			G_dxptr->lpBackBuffer = G_dxptr->lpPrimaryBuffer;

		dm = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode];
		G_dxptr->dwRenderWidth = dm->w;
		G_dxptr->dwRenderHeight = dm->h;
		G_dxptr->rViewport.top = 0;
		G_dxptr->rViewport.left = 0;
		G_dxptr->rViewport.right = dm->w;
		G_dxptr->rViewport.bottom = dm->h;
	}
	else
	{
		Log(5, "DXCreate: Windowed Mode");
		dm = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode];
		r.top = 0;
		r.left = 0;
		r.right = dm->w;
		r.bottom = dm->h;
		AdjustWindowRect(&r, WindowStyle, 0);
		SetWindowPos(hWnd, 0, 0, 0, r.right - r.left, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER);
		GetClientRect(hWnd, &G_dxptr->rViewport);
		GetClientRect(hWnd, &G_dxptr->rScreen);
		ClientToScreen(hWnd, (LPPOINT)&G_dxptr->rScreen);
		ClientToScreen(hWnd, (LPPOINT)&G_dxptr->rScreen.right);
		G_dxptr->dwRenderWidth = G_dxptr->rViewport.right;
		G_dxptr->dwRenderHeight = G_dxptr->rViewport.bottom;
		Log(5, "w %d h %d", G_dxptr->dwRenderWidth, G_dxptr->dwRenderHeight);
		desc.dwFlags = DDSD_CAPS;
		desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if (!DXCreateSurface(G_dxptr->lpDD, &desc, &G_dxptr->lpPrimaryBuffer))
		{
			DXClose();
			return 0;
		}

		if (DXAttempt(G_dxptr->lpDD->CreateClipper(0, &clipper, 0)) != DD_OK)
		{
			DXClose();
			return 0;
		}

		DXAttempt(clipper->SetHWnd(0, hWnd));
		DXAttempt(G_dxptr->lpPrimaryBuffer->SetClipper(clipper));

		if (clipper)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "Clipper", clipper, clipper->Release());
			clipper = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "Clipper");

		desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		desc.dwWidth = G_dxptr->dwRenderWidth;
		desc.dwHeight = G_dxptr->dwRenderHeight;
		desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;

		if (DXAttempt(G_dxptr->lpDD->CreateSurface(&desc, &G_dxptr->lpBackBuffer, 0)) != DD_OK)
		{
			DXClose();
			return 0;
		}
	}

	if (Flags & 0x10 && Flags & 0x80)
	{
		Log(3, "Creating ZBuffer");
		memset(&desc, 0, sizeof(DDSURFACEDESC2));
		desc.dwSize = sizeof(DDSURFACEDESC2);
		desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_ZBUFFER;
		desc.dwWidth = G_dxptr->dwRenderWidth;
		desc.dwHeight = G_dxptr->dwRenderHeight;
		memcpy(&desc.ddpfPixelFormat, &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].ZBufferInfos->ddpf, sizeof(DDPIXELFORMAT));

		if (DXAttempt(G_dxptr->lpDD->CreateSurface(&desc, &G_dxptr->lpZBuffer, 0)) != DD_OK)
		{
			DXClose();
			return 0;
		}

		DXAttempt(G_dxptr->lpBackBuffer->AddAttachedSurface(G_dxptr->lpZBuffer));
		Log(3, "ZBuffer Created %x", G_dxptr->lpZBuffer);
	}

	if (!DXCreateD3DDevice(G_dxptr->lpD3D, G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].Guid, G_dxptr->lpBackBuffer, &G_dxptr->lpD3DDevice))
	{
		DXClose();
		return 0;
	}

	G_dxptr->_lpD3DDevice = G_dxptr->lpD3DDevice;

	if (!DXCreateViewport(G_dxptr->lpD3D, G_dxptr->lpD3DDevice, G_dxptr->dwRenderWidth, G_dxptr->dwRenderHeight, &G_dxptr->lpViewport))
	{
		DXClose();
		return 0;
	}

	DXAttempt(G_dxptr->lpD3DDevice->SetRenderTarget(G_dxptr->lpBackBuffer, 0));

	if (!(G_dxptr->Flags & 0x80))
		CreateFakeD3D();

	return 1;
}

long DXChangeVideoMode()
{
	long val;

	Log(2, "DXChangeVideoMode");
	G_dxptr->Flags |= 0x40;
	G_dxptr->lpD3D->EvictManagedTextures();
	val = DXCreate(0, 0, 0, G_dxptr->Flags, G_dxptr, G_dxptr->hWnd, G_dxptr->WindowStyle);
	G_dxptr->Flags ^= 0x40;
	Log(2, "Exited DXChangeVideoMode %d", val);
	return val;
}

long DXToggleFullScreen()
{
	DXDISPLAYMODE* dm;

	Log(2, "DXToggleFullScreen");

	if (G_dxptr->Flags & 2)
	{
		Log(5, "Switching To Full Screen");
		G_dxptr->Flags ^= 2;
		G_dxptr->Flags |= 65;
		G_dxptr->Flags |= 64;	//mhm
	}
	else
	{
		Log(5, "Switching To A Window");
		G_dxptr->Flags ^= 1;
		G_dxptr->Flags |= 66;
	}

	G_dxptr->lpD3D->EvictManagedTextures();
	dm = &G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode];
	DXCreate(dm->w, dm->h, dm->bpp, G_dxptr->Flags, G_dxptr, G_dxptr->hWnd, G_dxptr->WindowStyle);
	G_dxptr->Flags ^= 64;
	return 1;
}

HRESULT __stdcall DXEnumDirect3D(LPGUID lpGuid, LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
	DXDIRECTDRAWINFO* ddi;
	DXD3DDEVICE* device;
	LPDIRECT3DDEVICE3 d3dDevice;
	DXDISPLAYMODE* dm;
	LPDIRECTDRAWSURFACE4 surf;
	DDSURFACEDESC2 desc;
	long nD3DDevices;

	ddi = (DXDIRECTDRAWINFO*)lpContext;
	nD3DDevices = ddi->nD3DDevices;
	ddi->D3DDevices = (DXD3DDEVICE*)AddStruct(ddi->D3DDevices, nD3DDevices, sizeof(DXD3DDEVICE));
	device = &ddi->D3DDevices[nD3DDevices];

	if (lpGuid)
	{
		device->lpGuid = &device->Guid;
		device->Guid = *lpGuid;
	}
	else
		device->lpGuid = 0;

	lstrcpy(device->About, lpDeviceDescription);
	lstrcpy(device->Name, lpDeviceName);
	Log(5, "Found - %s", lpDeviceDescription);

	if (lpHWDesc->dwFlags)
	{
		device->bHardware = 1;
		memcpy(&device->DeviceDesc, lpHWDesc, sizeof(D3DDEVICEDESC));
	}
	else
	{
		device->bHardware = 0;
		memcpy(&device->DeviceDesc, lpHELDesc, sizeof(D3DDEVICEDESC));

		if (!App.mmx)
			strcpy(device->About, "Core Design Hardware Card Emulation");
		else
			strcpy(device->About, "Core Design MMX Hardware Card Emulation");
	}

	Log(5, "Finding Compatible Display Modes");
	device->nDisplayModes = 0;

	for (int i = 0; i < ddi->nDisplayModes; i++)
	{
		if (BPPToDDBD(ddi->DisplayModes[i].bpp) & device->DeviceDesc.dwDeviceRenderBitDepth)
		{
			device->DisplayModes = (DXDISPLAYMODE*)AddStruct(device->DisplayModes, ddi->nDisplayModes, sizeof(DXDISPLAYMODE));
			dm = &device->DisplayModes[device->nDisplayModes];
			memcpy(dm, &ddi->DisplayModes[i], sizeof(DXDISPLAYMODE));

			if (dm->bPalette)
				Log(3, "%d x %d - %d Bit - Palette", dm->w, dm->h, dm->bpp);
			else
				Log(3, "%d x %d - %d Bit - %d%d%d", dm->w, dm->h, dm->bpp, dm->rbpp, dm->gbpp, dm->bbpp);

			device->nDisplayModes++;
		}
	}

	Log(5, "Enumerate Texture Formats");
	memset(&desc, 0, sizeof(DDSURFACEDESC2));
	desc.dwSize = sizeof(DDSURFACEDESC2);
	desc.dwFlags = DDSD_CAPS;
	desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;
	DXSetCooperativeLevel(G_ddraw, G_hwnd, DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES | DDSCL_EXCLUSIVE);
	DXCreateSurface(G_ddraw, &desc, &surf);

	if (surf)
	{
		DXCreateD3DDevice(G_d3d, device->Guid, surf, &d3dDevice);

		if (!d3dDevice)	//fail
		{
			Log(1, "Create D3DDevice Failed");

			//release surface, recreate it and try again

			if (surf)
			{
				Log(4, "Released %s @ %x - RefCnt = %d", "DirectDrawSurface", surf, surf->Release());
				surf = 0;
			}
			else
				Log(1, "%s Attempt To Release NULL Ptr", "DirectDrawSurface");

			DXSetVideoMode(G_ddraw, device->DisplayModes->w, device->DisplayModes->h, device->DisplayModes->bpp);
			DXCreateSurface(G_ddraw, &desc, &surf);

			if (surf)
				DXCreateD3DDevice(G_d3d, device->Guid, surf, &d3dDevice);
		}

		if (d3dDevice)	//did it work?
		{
			device->nTextureInfos = 0;
			Log(2, "DXEnumTextureFormats");
			DXAttempt(d3dDevice->EnumTextureFormats(DXEnumTextureFormats, (void*)device));

			if (d3dDevice)
			{
				Log(4, "Released %s @ %x - RefCnt = %d", "D3DDevice", d3dDevice, d3dDevice->Release());
				d3dDevice = 0;
			}
			else
				Log(1, "%s Attempt To Release NULL Ptr", "D3DDevice");
		}

		if (surf)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "DirectDrawSurface", surf, surf->Release());
			surf = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "DirectDrawSurface");
	}

	DXSetCooperativeLevel(G_ddraw, G_hwnd, DDSCL_NORMAL);
	Log(5, "Enumerating ZBuffer Formats");
	Log(2, "DXEnumZBufferFormats");
	DXAttempt(G_d3d->EnumZBufferFormats(device->Guid, DXEnumZBufferFormats, (void*)device));
	ddi->nD3DDevices++;
	return D3DENUMRET_OK;
}

BOOL __stdcall DXEnumDirectDraw(GUID FAR* lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
	DXINFO* dxinfo;
	DXDIRECTDRAWINFO* DDInfo;
	long nDDInfo;

	Log(2, "DXEnumDirectDraw");
	dxinfo = (DXINFO*)lpContext;
	nDDInfo = dxinfo->nDDInfo;
	dxinfo->DDInfo = (DXDIRECTDRAWINFO*)AddStruct(dxinfo->DDInfo, dxinfo->nDDInfo, sizeof(DXDIRECTDRAWINFO));
	DDInfo = &dxinfo->DDInfo[nDDInfo];

	if (lpGUID)
	{
		DDInfo->lpGuid = &DDInfo->Guid;
		DDInfo->Guid = *lpGUID;
	}
	else
		DDInfo->lpGuid = 0;

	lstrcpy(DDInfo->About, lpDriverDescription);
	lstrcpy(DDInfo->Name, lpDriverName);
	Log(5, "Obtaining Information For %s", lpDriverDescription);

	if (DXDDCreate(lpGUID, (void**)&G_ddraw))
	{
		DXAttempt(G_ddraw->GetDeviceIdentifier(&DDInfo->DDIdentifier, 0));

		Log(5, "Found - %s\nDriver %s Version %d.%d.%d.%d",
			DDInfo->DDIdentifier.szDescription,
			DDInfo->DDIdentifier.szDriver,
			HIWORD(DDInfo->DDIdentifier.liDriverVersion.HighPart),
			LOWORD(DDInfo->DDIdentifier.liDriverVersion.HighPart),
			HIWORD(DDInfo->DDIdentifier.liDriverVersion.LowPart),
			LOWORD(DDInfo->DDIdentifier.liDriverVersion.LowPart));

		memset(&DDInfo->DDCaps, 0, sizeof(DDInfo->DDCaps));
		DDInfo->DDCaps.dwSize = sizeof(DDCAPS);

		Log(5, "Getting Device Capabilities");
		DXAttempt(G_ddraw->GetCaps(&DDInfo->DDCaps, 0));

		Log(5, "Enumerating Display Modes");
		DXSetCooperativeLevel(G_ddraw, 0, DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES | DDSCL_NORMAL | DDSCL_ALLOWMODEX);

		Log(2, "DXEnumDisplayModes");
		DXAttempt(G_ddraw->EnumDisplayModes(0, 0, (void*)DDInfo, DXEnumDisplayModes));

		if (DXD3DCreate(G_ddraw, (void**)&G_d3d))
		{
			Log(2, "DXEnumDirect3D");
			DXAttempt(G_d3d->EnumDevices(DXEnumDirect3D, (void*)DDInfo));

			if (G_d3d)
			{
				Log(4, "Released %s @ %x - RefCnt = %d", "Direct3D", G_d3d, G_d3d->Release());
				G_d3d = 0;
			}
			else
				Log(1, "%s Attempt To Release NULL Ptr", "Direct3D");
		}

		DXSetCooperativeLevel(G_ddraw, 0, DDSCL_NORMAL);

		if (G_ddraw)
		{
			Log(4, "Released %s @ %x - RefCnt = %d", "DirectDraw", G_ddraw, G_ddraw->Release());
			G_ddraw = 0;
		}
		else
			Log(1, "%s Attempt To Release NULL Ptr", "DirectDraw");
	}

	dxinfo->nDDInfo++;
	return DDENUMRET_OK;
}

BOOL __stdcall DXEnumDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
	DXINFO* dxinfo;
	DXDIRECTSOUNDINFO* DSInfo;
	long nDSInfo;

	Log(2, "DXEnumDirectSound");
	dxinfo = (DXINFO*)lpContext;
	nDSInfo = dxinfo->nDSInfo;
	dxinfo->DSInfo = (DXDIRECTSOUNDINFO*)AddStruct(dxinfo->DSInfo, nDSInfo, sizeof(DXDIRECTSOUNDINFO));
	DSInfo = &dxinfo->DSInfo[nDSInfo];

	if (lpGuid)
	{
		DSInfo->lpGuid = &DSInfo->Guid;
		DSInfo->Guid = *lpGuid;
	}
	else
		DSInfo->lpGuid = 0;

	lstrcpy(DSInfo->About, lpcstrDescription);
	lstrcpy(DSInfo->Name, lpcstrModule);
	Log(5, "Found - %s %s", lpcstrDescription, lpcstrModule);
	dxinfo->nDSInfo++;
	return DDENUMRET_OK;
}

long DXGetInfo(DXINFO* dxinfo, HWND hwnd)
{
	Log(2, "DXInitialise");
	G_hwnd = hwnd;
	Log(5, "Enumerating DirectDraw Devices");
	DXAttempt(DirectDrawEnumerate(DXEnumDirectDraw, dxinfo));
	DXAttempt(DirectSoundEnumerate(DXEnumDirectSound, dxinfo));
	G_dxinfo = dxinfo;
	return 1;
}

void DXFreeInfo(DXINFO* dxinfo)
{
	DXDIRECTDRAWINFO* DDInfo;
	DXD3DDEVICE* d3d;

	Log(2, "DXFreeInfo");

	for (int i = 0; i < dxinfo->nDDInfo; i++)
	{
		DDInfo = &dxinfo->DDInfo[i];

		for (int j = 0; j < DDInfo->nD3DDevices; j++)
		{
			d3d = &DDInfo->D3DDevices[j];
			FREE(d3d->DisplayModes);
			FREE(d3d->TextureInfos);
			FREE(d3d->ZBufferInfos);
		}

		FREE(DDInfo->D3DDevices);
		FREE(DDInfo->DisplayModes);
	}

	FREE(dxinfo->DDInfo);
	FREE(dxinfo->DSInfo);
}

void DXJoyAcquisition(long acquire)
{
	if (!G_dxptr->Joystick)
		return;

	if (acquire)
		G_dxptr->Joystick->Acquire();
	else
		G_dxptr->Joystick->Unacquire();
}

void DXSize(long x, long y)
{
	Log(2, "DXSize : x %d y %d", x, y);

	if (G_dxptr)
	{
		if (!(G_dxptr->Flags & 1))
		{
			G_dxptr->rScreen.right = x + G_dxptr->rScreen.left;
			G_dxptr->rScreen.bottom = y + G_dxptr->rScreen.top;
		}
	}
}

long DXFindTextureFormat(long r, long g, long b, long a)
{
	DXD3DDEVICE* d;
	DXTEXTUREINFO* t;

	Log(5, "DXFindTextureFormat %d%d%d%d", r, g, b, a);

	d = &G_dxinfo->DDInfo[G_dxinfo->nDDInfo].D3DDevices[G_dxinfo->nD3D];

	for (int i = 0; i < d->nTextureInfos; i++)
	{
		t = &d->TextureInfos[i];

		if (t->rbpp == r && t->gbpp == g && t->bbpp == b && t->abpp == a)
		{
			Log(5, "Found Format");
			return i;
		}
	}

	Log(1, "Format Not Found");
	return -1;
}

void FlashLEDs()
{
	DIDEVICEOBJECTDATA obj[3];
	static long data[4] = { 1, 2, 4, 2 };
	static long a, b, c, lp;
	ulong nOf;
	long n;

	nOf = 3;
	memset(obj, 0, sizeof(obj));
	obj[0].dwOfs = a;
	obj[1].dwOfs = b;
	obj[2].dwOfs = c;
	n = data[lp];
	obj[0].dwData = (n & 1) << 7;
	obj[1].dwData = (n & 2) << 6;
	obj[2].dwData = (n & 4) << 5;
	lp = (lp + 1) % 4;
	DXAttempt(G_dxptr->Keyboard->SendDeviceData(sizeof(DIDEVICEOBJECTDATA), obj, &nOf, 0));
}

long DXFindDevice(long w, long h, long bpp, long hw)
{
	DXDISPLAYMODE* dm;
	bool flag;

	for (int i = G_dxinfo->nDDInfo - 1; i >= 0; i--)
	{
		for (int j = 0; j < G_dxinfo->DDInfo[i].nD3DDevices; j++)
		{
			if (G_dxinfo->DDInfo[i].D3DDevices[j].bHardware == hw)
			{
				for (int k = 0; k < G_dxinfo->DDInfo[i].D3DDevices[j].nDisplayModes; k++)
				{
					dm = &G_dxinfo->DDInfo[i].D3DDevices[j].DisplayModes[k];

					if (dm->w == w && dm->h == h && dm->bpp == bpp)
					{
						flag = 1;

						if (G_dxptr->Flags & 2)
							flag = G_dxinfo->DDInfo[i].DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED;

						if (flag)
						{
							G_dxinfo->nDD = i;
							G_dxinfo->nD3D = j;
							G_dxinfo->nDisplayMode = k;
							Log(5, "Matching Device Found\n%s\n%s\n%dx%dx%d",
								G_dxinfo->DDInfo[G_dxinfo->nDD].DDIdentifier.szDescription,
								G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].About,
								G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode].w,
								G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode].h,
								G_dxinfo->DDInfo[G_dxinfo->nDD].D3DDevices[G_dxinfo->nD3D].DisplayModes[G_dxinfo->nDisplayMode].bpp);
							G_dxinfo->bHardware = hw != 0;
							return 1;
						}
					}
				}
			}
		}
	}

	Log(1, "No Matching Device Found");
	return 0;
}

BOOL CALLBACK EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	DIPROPRANGE range;

	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwHow = DIPH_BYOFFSET;
	range.diph.dwObj = lpddoi->dwOfs;
	range.lMin = -1000;
	range.lMax = 1000;

	if (SUCCEEDED(G_dxptr->Joystick->SetProperty(DIPROP_RANGE, &range.diph)))
		return DIENUM_CONTINUE;

	return DIENUM_STOP;
}

BOOL CALLBACK EnumJoysticksCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	if (SUCCEEDED(G_dxptr->lpDirectInput->CreateDeviceEx(lpddi->guidInstance, IID_IDirectInputDevice2, (LPVOID*)G_dxptr->Joystick, 0)))
		return DIENUM_STOP;

	return DIENUM_CONTINUE;
}

long DXUpdateJoystick()
{
	DIJOYSTATE state;
	HRESULT hr;
	long b;

	joystick_read = 0;
	joystick_read_x = 0;
	joystick_read_y = 0;
	joystick_read_fire = 0;

	if (!G_dxptr->Joystick)
		return 0;

	do
	{
		G_dxptr->Joystick->Poll();
		hr = G_dxptr->Joystick->GetDeviceState(sizeof(DIJOYSTATE), &state);

		if (hr == DIERR_INPUTLOST)
			hr = G_dxptr->Joystick->Acquire();

	} while (hr == DIERR_INPUTLOST);

	if (FAILED(hr))
		return 0;

	joystick_read = 1;
	joystick_read_x = state.lX;
	joystick_read_y = state.lY;
	b = 0;
	
	for (int i = 0; i < 32; i++)
	{
		if (state.rgbButtons[i] & 0x80)
			b |= 1 << i;
	}

	joystick_read_fire = b;
	return 1;
}

long DXGetKey()
{
	long b;
	char stash[64];

	if (!KeyCount)
		return 0;

	b = KeyBuffer[0];
	KeyCount--;

	memcpy(stash, KeyBuffer, 64);
	memset(KeyBuffer, 0, sizeof(KeyBuffer));
	memcpy(KeyBuffer, &stash[1], 63);
	return b;
}

void inject_dxshell(bool replace)
{
	INJECT(0x004A2880, DXReadKeyboard, replace);
	INJECT(0x0049F9C0, DXBitMask2ShiftCnt, replace);
	INJECT(0x0049F1C0, DXAttempt, replace);
	INJECT(0x0049F200, DIAttempt, replace);
	INJECT(0x0049F4C0, AddStruct, replace);
	INJECT(0x0049F530, DXDDCreate, replace);
	INJECT(0x0049F620, DXD3DCreate, replace);
	INJECT(0x004A0600, DXSetCooperativeLevel, replace);
	INJECT(0x0049FA10, DXEnumDisplayModes, replace);
	INJECT(0x004A0490, DXEnumZBufferFormats, replace);
	INJECT(0x004A0270, DXEnumTextureFormats, replace);
	INJECT(0x0049FB80, BPPToDDBD, replace);
	INJECT(0x004A0C30, DXCreateD3DDevice, replace);
	INJECT(0x004A0590, DXSetVideoMode, replace);
	INJECT(0x004A0520, DXCreateSurface, replace);
	INJECT(0x004A1F50, DXCreateViewport, replace);
	INJECT(0x004A23A0, DXSaveScreen, replace);
	INJECT(0x004A2080, DXShowFrame, replace);
	INJECT(0x004A21B0, DXMove, replace);
	INJECT(0x004A1C40, DXClose, replace);
	INJECT(0x004A0EB0, DXCreate, replace);
	INJECT(0x004A1990, DXChangeVideoMode, replace);
	INJECT(0x004A1A20, DXToggleFullScreen, replace);
	INJECT(0x0049FC40, DXEnumDirect3D, replace);
	INJECT(0x0049F6A0, DXEnumDirectDraw, replace);
	INJECT(0x0049F2C0, DXEnumDirectSound, replace);
	INJECT(0x0049F240, DXGetInfo, replace);
	INJECT(0x0049F390, DXFreeInfo, replace);
	INJECT(0x004A2DF0, DXJoyAcquisition, replace);
	INJECT(0x004A2220, DXSize, replace);
	INJECT(0x004A2290, DXFindTextureFormat, replace);
	INJECT(0x004A27A0, FlashLEDs, replace);
	INJECT(0x004A0CB0, DXFindDevice, replace);
	INJECT(0x004A2C80, EnumAxesCallback, replace);
	INJECT(0x004A2C40, EnumJoysticksCallback, replace);
	INJECT(0x004A2D00, DXUpdateJoystick, replace);
	INJECT(0x004A28F0, DXGetKey, replace);
}
