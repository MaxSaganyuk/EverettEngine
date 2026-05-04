#include "IEverettEngine.h"

#define MainScriptLoop()                                                                                 \
extern "C" __declspec(dllexport) void Main()                                                             \

#define ScriptInit()                                                                                     \
void ImplScriptInit(IEverettEngine& engine);                                                             \
                                                                                                         \
extern "C" __declspec(dllexport) void ScriptInitFunc(void* object)                                       \
{                                                                                                        \
    ImplScriptInit(*reinterpret_cast<IEverettEngine*>(object));                                          \
}                                                                                                        \
                                                                                                         \
void ImplScriptInit(IEverettEngine& engine)

#define ScriptCleanUp()                                                                                  \
extern "C" __declspec(dllexport) void CleanUp()                                                          
