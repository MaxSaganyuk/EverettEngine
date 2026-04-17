#include <string>

#include "IEverettEngine.h"
#include "IObjectSim.h"
#include "ISolidSim.h"
#include "ICameraSim.h"
#include "ILightSim.h"
#include "ISoundSim.h"
#include "IColliderSim.h"

// This macro system can be simplified since we are getting engine interface now.
#define CONCAT(a, b, c, d) a##_##b##_##c##_##d##_
#define RESOLVE_AND_CONCAT(a, b, c, d) CONCAT(a, b, c, d)

#define MainScriptLoop()                                                                                 \
extern "C" __declspec(dllexport) void Main()                                                             \

#define EngineInterfaceInit()                                                                            \
void ImplEngineInterInit(IEverettEngine& engine);                                                        \
                                                                                                         \
extern "C" __declspec(dllexport) void RESOLVE_AND_CONCAT(S, __LINE__, EngineInter, 0)(void* object)      \
{                                                                                                        \
    ImplEngineInterInit(*reinterpret_cast<IEverettEngine*>(object));                                     \
}                                                                                                        \
                                                                                                         \
void ImplEngineInterInit(IEverettEngine& engine)

#define CameraObjectInit()                                                                               \
void ImplCamera(ICameraSim& objectCamera);                                                               \
                                                                                                         \
extern "C" __declspec(dllexport) void RESOLVE_AND_CONCAT(S, __LINE__, Camera, ICameraSim)(void* object)  \
{                                                                                                        \
    ImplCamera(*dynamic_cast<ICameraSim*>(reinterpret_cast<IObjectSim*>(object)));                       \
}                                                                                                        \
                                                                                                         \
void ImplCamera(ICameraSim& objectCamera)                                                                \

#define ScriptObjectInit(name, objectType)                                                               \
void Impl##name(objectType& object##name);                                                               \
                                                                                                         \
extern "C" __declspec(dllexport) void RESOLVE_AND_CONCAT(S, __LINE__, name, objectType)(void* object)    \
{                                                                                                        \
    Impl##name(*dynamic_cast<objectType*>(reinterpret_cast<IObjectSim*>(object)));                       \
}                                                                                                        \
                                                                                                         \
void Impl##name(objectType& object##name)                                                                \

#define ScriptKeybindPressed(keyname, holdable)                                                          \
void ImplKey##keyname##Pressed();                                                                        \
                                                                                                         \
extern "C" __declspec(dllexport) void RESOLVE_AND_CONCAT(K, holdable, keyname, 0)(void*)                 \
{                                                                                                        \
    ImplKey##keyname##Pressed();                                                                         \
}                                                                                                        \
                                                                                                         \
void ImplKey##keyname##Pressed()                                                                         \

#define ScriptKeybindReleased(keyname)                                                                   \
void ImplKey##keyname##Released();                                                                       \
                                                                                                         \
extern "C" __declspec(dllexport) void RESOLVE_AND_CONCAT(K, 0, keyname, 1)(void*)                        \
{                                                                                                        \
    ImplKey##keyname##Released();                                                                        \
}                                                                                                        \
                                                                                                         \
void ImplKey##keyname##Released()                                                                        \

#define ScriptMouseScroll()                                                                              \
void ImplScroll(double value);                                                                           \
                                                                                                         \
extern "C" __declspec(dllexport) void RESOLVE_AND_CONCAT(K, 0, MouseScroll, 0)(void* value)              \
{                                                                                                        \
    ImplScroll(*reinterpret_cast<double*>(value));                                                       \
}                                                                                                        \
                                                                                                         \
void ImplScroll(double value)

#define ScriptCleanUp()                                                                                  \
extern "C" __declspec(dllexport) void CleanUp()                                                          
