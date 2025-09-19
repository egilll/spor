#pragma once

#include <string>

// Simplified string field type used on wire:
// - On host builds, a TextField is a std::string (decoded inline stringz)
// - On target builds, a TextField is a const char* (encoded as stringz)

#ifdef SPOR_HOST
using TextField = std::string;
#else
using TextField = const char *;
#endif
