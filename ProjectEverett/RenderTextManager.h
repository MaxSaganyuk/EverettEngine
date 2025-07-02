#pragma once

#include <map>
#include <string>
#include <vector>

#include "LGLStructs.h"

#define ForwardDeclarePtrTo(Type) \
struct Type##Rec_; \
using Type = Type##Rec_*;

ForwardDeclarePtrTo(FT_Library)
ForwardDeclarePtrTo(FT_Face)

class RenderTextManager
{
public:
	RenderTextManager();
	~RenderTextManager();

	std::string LoadFontFromPath(const std::string& fontPath, int fontSize);

	LGLStructs::GlyphTexture GetGlyphTextureOf(const std::string& fontName, char c);
	LGLStructs::GlyphInfo& GetAllGlyphTextures(const std::string& fontName);

	void FreeFaceInfoByFont(const std::string& fontName, bool keepGlyphData = false);

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

#undef ForwardDeclarePtrTo