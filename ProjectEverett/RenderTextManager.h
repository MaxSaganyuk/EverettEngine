#pragma once

#include <map>
#include <string>
#include <vector>

#include "LGLStructs.h"

#define ForwardDeclarePtrTo(type, usingType) \
struct type; \
using usingType = type*;

ForwardDeclarePtrTo(FT_LibraryRec_, FT_Library)
ForwardDeclarePtrTo(FT_FaceRec_, FT_Face)

class RenderTextManager
{
public:
	void Init();
	std::string LoadFontFromPath(const std::string& fontPath, int fontSize);

	LGLStructs::GlyphTexture GetGlyphTextureOf(const std::string& fontName, char c);
	LGLStructs::GlyphInfo& GetAllGlyphTextures(const std::string& fontName);

private:
	struct FaceInfo
	{
		FT_Face face;
		std::map<char, std::vector<unsigned char>> glyphData;
		LGLStructs::GlyphInfo glyphInfo;
	};

	LGLStructs::GlyphTexture GetGlyphTextureOfImpl(FaceInfo& face, char c);

	FT_Library ft;
	std::map<std::string, FaceInfo> fontToFaceMap;
};