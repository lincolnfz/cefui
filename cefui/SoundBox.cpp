// reviewed by CR 2012.6.4
// 注意g_bCloseSound的共享访问是否可能存在线程同步问题？
// g_bCloseSound这个开关的状态只通过界面静音按钮进行切换，其它线程只做读取，我觉得不存在同步问题 - mark by wjc
//#include "stdafx.h"
#include <windows.h>
#include <dsound.h>
#include "detours.h"
#include "SoundBox.h"
#include <Mmreg.h>
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

#pragma comment(lib, "dsound.lib")

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

extern "C" {
	static MMRESULT (WINAPI *Real_midiStreamOut)(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh) = midiStreamOut;
	static MMRESULT (WINAPI *Real_waveOutWrite)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh) = waveOutWrite;
	static HRESULT (WINAPI *Real_DirectSoundBufferUnlock)(LPDIRECTSOUNDBUFFER lpThis, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
	static HRESULT (WINAPI *Real_AudioRenderClientReleaseBuffer)(IAudioRenderClient *pThis, UINT32 NumFramesWritten, DWORD dwFlags);
}

static BOOL g_bCloseSound = FALSE;

__forceinline HWND CreateDxWindow(HINSTANCE hInstance)
{
	HWND hWnd = CreateWindowW(L"Static", L"Static", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	return hWnd;
}

__forceinline VOID DestroyDxWindow(HWND hWnd)
{
	if (!IsWindow(hWnd))
		return;
	DestroyWindow(hWnd);
}

BOOL InitializeFunctionAddress()
{
	HWND hWnd = CreateDxWindow((HINSTANCE)GetModuleHandle(NULL));
// 	BYTE bCode[] = {0xb8, 0x00, 0x00, 0x00, 0x80, 0xc2, 0x0c, 0x00};
// 	PVOID pFun1 = (PVOID)GetProcAddress(GetModuleHandle("dsound.dll"), "DirectSoundCreate");
// 	WriteProcessMemory(GetCurrentProcess(), pFun1, bCode, sizeof(bCode), NULL);
// 	PVOID pFun2 = (PVOID)GetProcAddress(GetModuleHandle("dsound.dll"), "DirectSoundCreate8");
// 	WriteProcessMemory(GetCurrentProcess(), pFun2, bCode, sizeof(bCode), NULL);

	do
	{
		LPDIRECTSOUND lpDS = NULL;
		HRESULT hr = DirectSoundCreate(NULL, &lpDS, NULL);
		if (FAILED(hr)){
			OutputDebugStringW(L"--------DirectSoundCreate fail");
			break;
		}

		hr = lpDS->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
		if (FAILED(hr))
		{
			lpDS->Release();
			OutputDebugStringW(L"--------SetCooperativeLevel fail");
			break;
		}

		WAVEFORMATEX wave_format    = {0};
		wave_format.wFormatTag      = WAVE_FORMAT_PCM;
		wave_format.nChannels       = 1;
		wave_format.nSamplesPerSec  = 11025;    
		wave_format.wBitsPerSample  = 16;
		wave_format.nBlockAlign     = (wave_format.wBitsPerSample / 8) * wave_format.nChannels;
		wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;

		DSBUFFERDESC ds_buffer_desc  = {0};
		ds_buffer_desc.dwSize        = sizeof(DSBUFFERDESC); 
		ds_buffer_desc.dwFlags       = DSBCAPS_CTRLVOLUME;
		ds_buffer_desc.dwBufferBytes = wave_format.nAvgBytesPerSec * 2;
		ds_buffer_desc.lpwfxFormat   = &wave_format;

		LPDIRECTSOUNDBUFFER lpDSBuffer;
		hr = lpDS->CreateSoundBuffer(&ds_buffer_desc, &lpDSBuffer, NULL);
		if (FAILED(hr))
		{
			lpDS->Release();
			OutputDebugStringW(L"--------CreateSoundBuffer fail");
			break;
		}

		*(PVOID *)&Real_DirectSoundBufferUnlock = (*(PVOID **)lpDSBuffer)[19];

		lpDSBuffer->Release();
		lpDS->Release();
	} while (0);

	do
	{
		IMMDeviceEnumerator *pEnumerator = NULL;
		IMMDevice *pDevice = NULL;
		IAudioClient *pAudioClient = NULL;
		IAudioRenderClient *pRenderClient = NULL;
		WAVEFORMATEX *pwfx = NULL;
		REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

		HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void **)&pEnumerator);
		if (FAILED(hr))
			goto failed;

		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
		if (FAILED(hr))
			goto failed;

		hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void **)&pAudioClient);
		if (FAILED(hr))
			goto failed;

		hr = pAudioClient->GetMixFormat(&pwfx);
		if (FAILED(hr))
			goto failed;

		hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
		if (FAILED(hr))
			goto failed;

		hr = pAudioClient->GetService(IID_IAudioRenderClient, (void **)&pRenderClient);
		if (FAILED(hr))
			goto failed;

		*(PVOID *)&Real_AudioRenderClientReleaseBuffer = (*(PVOID **)pRenderClient)[4];

failed:
		if (pRenderClient)
			pRenderClient->Release();
		if (pAudioClient)
			pAudioClient->Release();
		if (pDevice)
			pDevice->Release();
		if (pEnumerator)
			pEnumerator->Release();

		OutputDebugStringW(L"------------Ini fun address fail");
	} while (0);

	DestroyDxWindow(hWnd);
	return TRUE;
}


static MMRESULT WINAPI Mine_midiStreamOut(HMIDISTRM hms, LPMIDIHDR pmh, UINT cbmh)
{
	if (g_bCloseSound && pmh)
	{
		memset(pmh->lpData, 0, pmh->dwBufferLength);
	}
	MMRESULT result = Real_midiStreamOut(hms, pmh, cbmh);
	return result;
}

static MMRESULT WINAPI Mine_waveOutWrite(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
	if (g_bCloseSound && pwh)
	{
		memset(pwh->lpData, 0, pwh->dwBufferLength);
	}
	MMRESULT result = Real_waveOutWrite(hwo, pwh, cbwh);
	return result;
}

static HRESULT WINAPI Mine_DirectSoundBufferUnlock(LPDIRECTSOUNDBUFFER lpThis, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	if (g_bCloseSound)
	{
		if (pvAudioPtr1)
			memset(pvAudioPtr1, 0, dwAudioBytes1);
		if (pvAudioPtr2)
			memset(pvAudioPtr2, 0, dwAudioBytes2);
	}
	HRESULT result = Real_DirectSoundBufferUnlock(lpThis, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
	return result;
}

static HRESULT WINAPI Mine_AudioRenderClientReleaseBuffer(IAudioRenderClient *pThis, UINT32 NumFramesWritten, DWORD dwFlags)
{
	if (g_bCloseSound)
		dwFlags |= 2;
	HRESULT result = Real_AudioRenderClientReleaseBuffer(pThis, NumFramesWritten, dwFlags);
	return result;
}

BOOL EnableSoundControl(BOOL bEnable)
{
	static bool initCom = false;
	if ( !initCom )
	{
		OleInitialize(NULL);
		initCom = true;
	}	

	static BOOL bEnabled = FALSE;
	if ((bEnable && bEnabled) || (!bEnable && !bEnabled))
		return FALSE;

	BOOL bSuccessed;
	if (bEnable)
	{
		InitializeFunctionAddress();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID&)Real_midiStreamOut, Mine_midiStreamOut);
		DetourAttach(&(PVOID&)Real_waveOutWrite, Mine_waveOutWrite);
		DetourAttach(&(PVOID&)Real_DirectSoundBufferUnlock, Mine_DirectSoundBufferUnlock);

		// VISTA以前的系统没有这个函数，不用管它
		if (Real_AudioRenderClientReleaseBuffer)
			DetourAttach(&(PVOID&)Real_AudioRenderClientReleaseBuffer, Mine_AudioRenderClientReleaseBuffer);

		bSuccessed = DetourTransactionCommit() == 0;
		bEnabled = TRUE;
	}
	else
	{
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

	    DetourDetach(&(PVOID&)Real_midiStreamOut, Mine_midiStreamOut);
		DetourDetach(&(PVOID&)Real_waveOutWrite, Mine_waveOutWrite);
		DetourDetach(&(PVOID&)Real_DirectSoundBufferUnlock, Mine_DirectSoundBufferUnlock);

		if (Real_AudioRenderClientReleaseBuffer)
			DetourDetach(&(PVOID&)Real_AudioRenderClientReleaseBuffer, Mine_AudioRenderClientReleaseBuffer);

		bSuccessed = DetourTransactionCommit() == 0;
		bEnabled = FALSE;
	}
	return bSuccessed;
}

void OpenSound()
{
	g_bCloseSound = FALSE;
}

void CloseSound()
{
	g_bCloseSound = TRUE;
}
