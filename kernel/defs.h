#pragma once

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

// null pointer
#define NULL 0

// exit codes
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// standard streams
#define STD_IN  0
#define STD_OUT 1
#define STD_ERR 2
