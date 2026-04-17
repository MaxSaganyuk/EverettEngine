#pragma once

#include "external/EverettException.h"

#define ThrowException                                     throw EverettException(std::source_location::current())
#define ThrowExceptionWMessage(message)                    throw EverettException(std::source_location::current(), message)
#define CheckAndThrowExceptionWMessage(condition, message) if(!condition) throw EverettException(std::source_location::current(), message)
