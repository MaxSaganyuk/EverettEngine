#include <string>

#include "IObjectSim.h"
#include "ISolidSim.h"
#include "ICameraSim.h"
#include "ILightSim.h"
#include "ISoundSim.h"

#define CameraScriptLoop()                                                         \
void ImplCamera(ICameraSim& camera);                                               \
                                                                                   \
extern "C" __declspec(dllexport) void Camera(void* camera)                         \
{                                                                                  \
    ImplCamera(*dynamic_cast<ICameraSim*>(reinterpret_cast<IObjectSim*>(camera))); \
}                                                                                  \
                                                                                   \
void ImplCamera(ICameraSim& camera)                                                \

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
void CleanUpImpl();                                                                \
                                                                                   \
extern "C" __declspec(dllexport) void CleanUp()                                    \
{                                                                                  \
    CleanUpImpl();                                                                 \
}                                                                                  \
                                                                                   \
void CleanUpImpl()                                                                 \