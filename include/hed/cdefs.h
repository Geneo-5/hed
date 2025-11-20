#ifndef _HED_CDEFS_H
#define _HED_CDEFS_H

#include <hed/priv/config.h>
#include <stroll/cdefs.h>

#if defined(CONFIG_HED_ASSERT_API) || defined(CONFIG_HED_ASSERT_INTERN)

#define __hed_nonull(_arg_index, ...)

#else  /* !defined(CONFIG_HED_ASSERT_API) */

#define __hed_nonull(_arg_index, ...) \
	__nonull(_arg_index, ## __VA_ARGS__)

#endif /* defined(CONFIG_HED_ASSERT_API) */

#if defined(CONFIG_HED_ASSERT_API)

#include <stroll/assert.h>

#define hed_assert_api(_expr) \
	stroll_assert("hed", _expr)

#else  /* !defined(CONFIG_HED_ASSERT_API) */

#define hed_assert_api(_expr)

#endif /* defined(CONFIG_HED_ASSERT_API) */

#if defined(CONFIG_HED_ASSERT_INTERN)

#include <stroll/assert.h>

#define hed_assert_intern(_expr) \
	stroll_assert("hed", _expr)

#else  /* !defined(CONFIG_HED_ASSERT_INTERN) */

#define hed_assert_intern(_expr)

#endif /* defined(CONFIG_HED_ASSERT_INTERN) */

#endif /* _HED_CDEFS_H */
