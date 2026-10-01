#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <vector>
#include <array>
#include <optional>
#include <functional>

namespace MFCUtilities
{
	bool EditIsEmpty(const CEdit& edit);
	bool EditsAllEmpty(const std::vector<CEdit*>& edits);
	bool EditsAnyEmpty(const std::vector<CEdit*>& edits);

	// Yields owner, must be deleted
	Gdiplus::Bitmap* LoadPNGFromResource(HMODULE hModule, unsigned int resourceID, LPCTSTR resourceType);

	void OpenColorSelection(std::function<glm::vec3&()>&& colorVectorGetter);
};