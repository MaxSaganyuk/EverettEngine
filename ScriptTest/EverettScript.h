#include <string>

#include "IObjectSim.h"
#include "ISolidSim.h"
#include "ICameraSim.h"
#include "ILightSim.h"
#include "ISoundSim.h"
#include "IColliderSim.h"

#define MainScriptLoop()                                                           \
extern "C" __declspec(dllexport) void Main()                                       \

#define CameraObjectInit()                                                         \
void ImplCamera(ICameraSim& objectCamera);                                         \
                                                                                   \
extern "C" __declspec(dllexport) void Camera(void* object)                         \
{                                                                                  \
    ImplCamera(*dynamic_cast<ICameraSim*>(reinterpret_cast<IObjectSim*>(object))); \
}                                                                                  \
                                                                                   \
void ImplCamera(ICameraSim& objectCamera)                                          \

#define ScriptObjectInit(name, objectType)                                         \
void Impl##name(objectType& object##name);                                         \
                                                                                   \
extern "C" __declspec(dllexport) void name(void* object)                           \
{                                                                                  \
    Impl##name(*dynamic_cast<objectType*>(reinterpret_cast<IObjectSim*>(object))); \
}                                                                                  \
                                                                                   \
void Impl##name(objectType& object##name)                                          \

#define ScriptKeybindPressed(keyname)                                              \
void ImplKey##keyname##Pressed();                                                  \
                                                                                   \
extern "C" __declspec(dllexport) void Key##keyname##Pressed(void* placeholder)     \
{                                                                                  \
    ImplKey##keyname##Pressed();                                                   \
}                                                                                  \
                                                                                   \
void ImplKey##keyname##Pressed()                                                   \

#define ScriptKeybindReleased(keyname)                                             \
void ImplKey##keyname##Released();                                                 \
                                                                                   \
extern "C" __declspec(dllexport) void Key##keyname##Released(void* placeholder)    \
{                                                                                  \
    ImplKey##keyname##Released();                                                  \
}                                                                                  \
                                                                                   \
void ImplKey##keyname##Released()                                                  \

#define ScriptCleanUp()                                                            \
extern "C" __declspec(dllexport) void CleanUp()                                    \