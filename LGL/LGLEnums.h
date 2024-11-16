#pragma once

namespace LGLEnums
{
	enum DepthTestMode
	{
		Disable,
		Always,
		Never,
	    Less,
	    Greater,
	    Equal,
	    NotEqual,
	    LessOrEqual,
	    GreaterOrEqual
	};

	enum class TextureOverlayType
	{
		Repeat, 
		Mirrored,
		EdgeClamp, 
		BorderClamp
	};

}

