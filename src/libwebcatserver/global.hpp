#pragma once

#include <webcat/global.hpp>

#if defined(WEBCATSERVER_LIBRARY)
#define WEBCATSERVERSHARED_EXPORT WEBCAT_EXPORT
#else
#define WEBCATSERVERSHARED_EXPORT WEBCAT_IMPORT
#endif

