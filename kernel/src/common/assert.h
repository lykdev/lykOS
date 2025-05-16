#pragma once

#include <common/log.h>
#include <common/panic.h>

/**
 * @brief Make an assertion & panic on failure.
 */
#define ASSERT(ASSERTION)                         \
    if (!(ASSERTION))                             \
    {                                             \
        log("Return address: %p", __builtin_return_address(0)); \
        panic("Assertion `%s` failed.", #ASSERTION); \
    }

/**
 * @brief Make an assertion & panic with a comment on failure.
 */
#define ASSERT_C(ASSERTION, COMMENT)              \
    if (!(ASSERTION))                             \
    {                                             \
        log("Return address: %p", __builtin_return_address(0)); \
        panic("Assertion `%s` failed: %s.", #ASSERTION, COMMENT); \
    }
