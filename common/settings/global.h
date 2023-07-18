#ifndef GLOBAL_H
#define GLOBAL_H

#include "settings.h"

#if !defined(APP_NAMESPACE) /* app namespace */

# define APP_PREPEND_NAMESPACE(name) ::name
# define APP_USE_NAMESPACE
# define APP_BEGIN_NAMESPACE
# define APP_END_NAMESPACE

#else /* app namespace */

# define APP_PREPEND_NAMESPACE(name) ::APP_NAMESPACE::name
# define APP_USE_NAMESPACE using namespace ::APP_NAMESPACE;
# define APP_BEGIN_NAMESPACE namespace APP_NAMESPACE {
# define APP_END_NAMESPACE }

namespace APP_NAMESPACE {}

#endif /* app namespace */

#endif // GLOBAL_H
