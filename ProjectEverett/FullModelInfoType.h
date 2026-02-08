#pragma once

#include "LGLStructs.h"
#include "AnimSystem.h"

class FullModelInfo : public std::pair<std::weak_ptr<LGLStructs::ModelInfo>, std::weak_ptr<AnimSystem::ModelAnim>> {};