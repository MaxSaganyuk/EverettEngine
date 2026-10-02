#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <array>
#include <optional>
#include <functional>
#include <string>
#include <memory>
#include <expected>

namespace MFCUtilities
{
	bool EditIsEmpty(const CEdit& edit);
	bool EditsAllEmpty(const std::vector<CEdit*>& edits);
	bool EditsAnyEmpty(const std::vector<CEdit*>& edits);

	class BitmapWrapper
	{
		using HBufferDeleter = decltype([](void* hBuffer) { if (hBuffer) { GlobalFree(hBuffer); } });
		using IStreamDeleter = decltype([](IStream* stream) { if (stream) { stream->Release(); } });

		std::unique_ptr<void, HBufferDeleter> hBuffer;
		std::unique_ptr<IStream, IStreamDeleter> stream;
		std::unique_ptr<Gdiplus::Bitmap> bitmap;
	public:
		BitmapWrapper() = default;
		BitmapWrapper(void* hbuffer, IStream* stream, Gdiplus::Bitmap* bitmap) noexcept;

		BitmapWrapper(const BitmapWrapper&) = delete;
		BitmapWrapper(BitmapWrapper&&) noexcept = default;
		BitmapWrapper& operator=(const BitmapWrapper&) = delete;
		BitmapWrapper& operator=(BitmapWrapper&&) noexcept = default;

		Gdiplus::Bitmap* GetBitmapPtr() const noexcept;

		explicit operator bool() const noexcept;
	};

	std::expected<BitmapWrapper, std::string> LoadPNGFromResource(
		HMODULE hModule, unsigned int resourceID, LPCTSTR resourceType
	);

	void OpenColorSelection(std::function<glm::vec3&()>&& colorVectorGetter);
};