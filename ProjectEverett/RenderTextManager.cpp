#include "RenderTextManager.h"

#include <iostream>
#include <cassert>

#include <ft2build.h>
#include FT_FREETYPE_H  

RenderTextManager::RenderTextManager()
{
	assert(!FT_Init_FreeType(&ft) && "Cannot init FreeType lib");
}

RenderTextManager::~RenderTextManager()
{
	for (auto& faceInfo : fontToFaceMap)
	{
		FreeFaceInfoByFont(faceInfo.first);
	}
	fontToFaceMap.clear();

	FT_Done_FreeType(ft);
}

std::string RenderTextManager::LoadFontFromPath(const std::string& fontPath, int fontSize)
{
	std::string fontName = fontPath.substr(fontPath.rfind('\\') + 1, std::string::npos);

	fontToFaceMap[fontName] = {};
	fontToFaceMap[fontName].face = nullptr;

	bool failure = FT_New_Face(ft, fontPath.c_str(), 0, &fontToFaceMap[fontName].face);

	assert(!failure && "Failed to load font");

	failure = !failure && FT_Set_Pixel_Sizes(fontToFaceMap[fontName].face, 0, fontSize);
	
	assert(!failure && "Cannot set font size");

	if (failure)
	{
		fontToFaceMap.erase(fontName);
		return "";
	}

	return fontName;
}

LGLStructs::GlyphTexture RenderTextManager::GetGlyphTextureOf(const std::string& fontName, char c)
{
	if (fontToFaceMap.contains(fontName))
	{
		return GetGlyphTextureOfImpl(fontToFaceMap[fontName], c);
	}

	assert(false && "Invalid font name");
}

LGLStructs::GlyphInfo& RenderTextManager::GetAllGlyphTextures(const std::string& fontName)
{
	if (fontToFaceMap.contains(fontName))
	{
		if (!fontToFaceMap[fontName].glyphInfo.glyphs.size())
		{
			fontToFaceMap[fontName].glyphInfo.fontName = fontName;
			for (char c = 32; c < 127; ++c)
			{
				fontToFaceMap[fontName].glyphInfo.glyphs.emplace(c, GetGlyphTextureOfImpl(fontToFaceMap[fontName], c));
			}
		}

		return fontToFaceMap[fontName].glyphInfo;
	}

	assert(false && "Invalid font name");
}

LGLStructs::GlyphTexture RenderTextManager::GetGlyphTextureOfImpl(FaceInfo& faceInfo, char c)
{
	if (!FT_Load_Char(faceInfo.face, c, FT_LOAD_RENDER))
	{
		auto glyphToUse = faceInfo.face->glyph;

		if (glyphToUse)
		{
			size_t bufferSize = glyphToUse->bitmap.width * glyphToUse->bitmap.rows;
			faceInfo.glyphData[c].resize(bufferSize);
			std::memcpy(faceInfo.glyphData[c].data(), glyphToUse->bitmap.buffer, bufferSize);

			return {
				c,
				faceInfo.glyphData[c].data(),
				static_cast<int>(glyphToUse->bitmap.width),
				static_cast<int>(glyphToUse->bitmap.rows),
				glyphToUse->bitmap_left,
				glyphToUse->bitmap_top,
				glyphToUse->advance.x
			};
		}
	}

	assert(false && "Could not load glyph");
}

void RenderTextManager::FreeFaceInfoByFont(const std::string& fontName, bool keepGlyphData)
{
	if (fontToFaceMap.contains(fontName))
	{
		FT_Done_Face(fontToFaceMap[fontName].face);
		fontToFaceMap[fontName].face = nullptr;

		if (!keepGlyphData)
		{
			fontToFaceMap[fontName].glyphData.clear();
		}

		return;
	}

	assert(false && "Invalid font name");
}