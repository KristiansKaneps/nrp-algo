#ifndef EMBEDDED_RESOURCES_H
#define EMBEDDED_RESOURCES_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

const char *find_embedded_file(const char *file_name, size_t *size);

#ifdef __cplusplus
}
#endif

#endif // EMBEDDED_RESOURCES_H
