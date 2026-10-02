#include "pch.h"
#include "MFCUtilities.h"

namespace MFCUtilities
{
	std::optional<glm::vec3> GetColorFromPickerDlg(const glm::vec3& initialColor)
	{
		CColorDialog colorDlg(RGB(initialColor[0] * 255, initialColor[1] * 255, initialColor[2] * 255), CC_FULLOPEN);

		std::optional<glm::vec3> colorRes;

		if (colorDlg.DoModal() == IDOK)
		{
			COLORREF colorRef = colorDlg.GetColor();

			colorRes = {
				GetRValue(colorRef) / 255.0f,
				GetGValue(colorRef) / 255.0f,
				GetBValue(colorRef) / 255.0f
			};
		}

		return colorRes;
	}

	bool EditIsEmpty(const CEdit& edit)
	{
		return edit.SendMessage(WM_GETTEXTLENGTH) == 0;
	}

	bool EditsAllEmpty(const std::vector<CEdit*>& edits)
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

	bool EditsAnyEmpty(const std::vector<CEdit*>& edits)
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

	BitmapWrapper::BitmapWrapper(void* hbuffer, IStream* stream, Gdiplus::Bitmap* bitmap) noexcept
	{
		this->hBuffer.reset(hbuffer);
		this->stream.reset(stream);
		this->bitmap.reset(bitmap);
	}

	Gdiplus::Bitmap* BitmapWrapper::GetBitmapPtr() const noexcept
	{
		return bitmap.get();
	}

	BitmapWrapper::operator bool() const noexcept
	{
		return hBuffer && stream && bitmap;
	}

	std::unexpected<std::string> FormLoadPNGError(std::string&& error)
	{
		constexpr static const char errorMes[] = "Failed to load PNG from Resource. Reason: ";

		return std::unexpected{ errorMes + error };
	}

	std::expected<BitmapWrapper, std::string> LoadPNGFromResource(
		HMODULE hModule, unsigned int resourceID, LPCTSTR resourceType
	)
	{
		HRSRC hResource = FindResourceW(hModule, MAKEINTRESOURCE(resourceID), resourceType);
		if (!hResource) return FormLoadPNGError("Can't find resource"); 

		DWORD imageSize = SizeofResource(hModule, hResource);
		if (!imageSize) return FormLoadPNGError("Can't get image size");

		HGLOBAL hMemory = LoadResource(hModule, hResource);
		if (!hMemory) return FormLoadPNGError("Can't load resource");

		void* pResourceData = LockResource(hMemory);
		if (!pResourceData) return FormLoadPNGError("Can't lock resource");

		HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
		if (!hBuffer) return FormLoadPNGError("Can't allocate global");

		void* pBuffer = GlobalLock(hBuffer);
		if (!pBuffer) return FormLoadPNGError("Can't lock global");

		memcpy(pBuffer, pResourceData, imageSize);
		GlobalUnlock(hBuffer);

		IStream* pStream = nullptr;
		if (FAILED(CreateStreamOnHGlobal(hBuffer, true, &pStream)))
		{
			GlobalFree(hBuffer);
			return FormLoadPNGError("Can't create stream");
		}

		Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromStream(pStream);
		if (!bitmap) return FormLoadPNGError("Can't get bitmap from stream");

		return BitmapWrapper{ hBuffer, pStream, bitmap };
	}

	void OpenColorSelection(std::function<glm::vec3& ()>&& colorVectorGetter)
	{
		glm::vec3& colorVectorAddr = colorVectorGetter();

		auto colorRaw = MFCUtilities::GetColorFromPickerDlg(colorVectorAddr);

		if (colorRaw)
		{
			colorVectorAddr = *colorRaw;
		}
	}
};