/*do not sumbit, a quick skip safestring lib */

#include <string.h>

#define memset_s(a, s, b) memset(a, b, s)
//#define memcpy_s(a, s, b) memcpy(a, b, s)
#define memcpy_s(a, s1, b, s2) memcpy(a, b, s1)
#define memmove_s(a, s1, b, s2) memmove(a, b, s1)


