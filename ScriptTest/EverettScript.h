#include <string>

#include "ISolidSim.h"

#define ScriptFunc(Name)                                                  \
void Impl##Name(const std::string& name, ISolidSim& solid);               \
                                                                          \
extern "C" __declspec(dllexport) void Name(const char* name, void* solid) \
{                                                                         \
    Impl##Name(name, *reinterpret_cast<ISolidSim*>(solid));               \
}                                                                         \
                                                                          \
void Impl##Name(const std::string& name, ISolidSim& solid)                \
