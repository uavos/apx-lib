#pragma once

#include "macro_expand.h"

#define _ENUM_STRINGIFY(X) #X
#define _ENUM_STRINGS(...) FOREACH(_ENUM_STRINGIFY, (__VA_ARGS__))

#define ENUM_MACRO(name, ...) \
    enum name { __VA_ARGS__ }; \
    static constexpr const char *name##_strings[] = {_ENUM_STRINGS(__VA_ARGS__)}; \
    static inline constexpr const char *name##_string(name value) { return name##_strings[value]; }
