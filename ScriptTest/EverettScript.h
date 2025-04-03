#include <string>

#include "ISolidSim.h"
#include "ICameraSim.h"
#include "ILightSim.h"
#include "ISoundSim.h"

#define CameraScriptLoop()                                                        \
void ImplCamera(ICameraSim& camera);                                              \
                                                                                  \
extern "C" __declspec(dllexport) void Camera(void* camera)                        \
{                                                                                 \
    ImplCamera(*dynamic_cast<ICameraSim*>(reinterpret_cast<ISolidSim*>(camera))); \
}                                                                                 \
                                                                                  \
void ImplCamera(ICameraSim& camera)                                               \

#define ScriptObjectInit(name, objectType)                                        \
void Impl##name(objectType& object##name);                                        \
                                                                                  \
extern "C" __declspec(dllexport) void name(void* object)                          \
{                                                                                 \
    Impl##name(*dynamic_cast<objectType*>(reinterpret_cast<ISolidSim*>(object))); \
}                                                                                 \
                                                                                  \
void Impl##name(objectType& object##name)                                         \