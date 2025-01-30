#pragma once

#define CEIL(VALUE, GRAN) (((VALUE) + (GRAN) - 1) / (GRAN) * (GRAN))

#define FLOOR(VALUE, GRAN) ((VALUE) / (GRAN) * (GRAN))
