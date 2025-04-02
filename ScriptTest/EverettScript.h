#include <string>

#include "ISolidSim.h"
#include "ICameraSim.h"
#include "ILightSim.h"
#include "ISoundSim.h"

#define CameraScriptLoop()                                                \
void ImplCamera(ICameraSim& camera);                                      \
                                                                          \
extern "C" __declspec(dllexport) void Camera(void* camera)                \
{                                                                         \
    ImplCamera(*reinterpret_cast<ICameraSim*>(camera));                   \
}                                                                         \
                                                                          \
void ImplCamera(ICameraSim& camera)                                       \

#define ScriptObjectInit(name, objectType)                                \
void Impl##name(I##objectType##Sim& object##name);                        \
                                                                          \
extern "C" __declspec(dllexport) void name(void* object)                  \
{                                                                         \
    Impl##name(*reinterpret_cast<I##objectType##Sim*>(object));           \
}                                                                         \
void Impl##name(I##objectType##Sim& object##name)                         \