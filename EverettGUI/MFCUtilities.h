#include "afxdialogex.h"
#include <vector>

class MFCUtilities
{
public:
	static bool EditIsEmpty(const CEdit& edit)
	{
		return edit.SendMessage(WM_GETTEXTLENGTH) == 0;
	}

	static bool EditsAllEmpty(const std::vector<CEdit*>& edits)
	{
		bool res = true;

		for (const CEdit* edit : edits)
		{
			res &= EditIsEmpty(*edit);

			if (!res)
			{
				return false;
			}
		}

		return true;
	}

	static bool EditsAnyEmpty(const std::vector<CEdit*>& edits)
	{
		bool res = false;

		for (const CEdit* edit : edits)
		{
			res |= EditIsEmpty(*edit);

			if (res)
			{
				return true;
			}
		}

		return res;
	}

	// Yields owner, must be deleted
	static Gdiplus::Bitmap* LoadPNGFromResource(HMODULE hModule, unsigned int resourceID, LPCTSTR resourceType)
	{
		HRSRC hResource = FindResourceW(hModule, MAKEINTRESOURCE(resourceID), resourceType);
		if (!hResource) return nullptr;

		DWORD imageSize = SizeofResource(hModule, hResource);
		if (!imageSize) return nullptr;

		HGLOBAL hMemory = LoadResource(hModule, hResource);
		if (!hMemory) return nullptr;

		void* pResourceData = LockResource(hMemory);
		if (!pResourceData) return nullptr;

		HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
		if (!hBuffer) return nullptr;

		void* pBuffer = GlobalLock(hBuffer);
		memcpy(pBuffer, pResourceData, imageSize);
		GlobalUnlock(hBuffer);

		IStream* pStream = nullptr;
		if (FAILED(CreateStreamOnHGlobal(hBuffer, true, &pStream)))
		{
			GlobalFree(hBuffer);
			return nullptr;
		}

		Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromStream(pStream);
		pStream->Release();

		return pBitmap;
	}
};