// Copyright (c) Sergey Lyubka, 2013.
// All rights reserved.
// Released under the MIT license.
//
// Modified by Kristians Kaneps on 15.05.2025.
// Changes include:
// Use relative resource path as name if input path contains "resources/". Added C++ support.
// Custom `strcasestr` implementation for older compilers.
//
// This program is used to embed arbitrary data into a C binary. It takes
// a list of files as an input, and produces a .c data file that contains
// contents of all these files as collection of char arrays.
// Usage:
//   1. Compile this file:
//      cc -o embed embed.c
//
//   2. Convert list of files into single .c:
//      ./embed file1.data file2.data > embedded_data.c
//
//   3. In your application code, you can access files using this function:
//
//      const char *find_embedded_file(const char *file_name, size_t *size);
//      size_t size;
//      const char *data = find_embedded_file("file1.data", &size);
//
//   4. Build your app with embedded_data.c:
//      cc -o my_app my_app.c embedded_data.c

// ReSharper disable CppDeprecatedEntity

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *code =
    "const char *find_embedded_file(const char *name, size_t *size) {\n"
    "  const struct embedded_file *p;\n"
    "  for (p = embedded_files; p->name != NULL; p++) {\n"
    "    if (!strcmp(p->name, name)) {\n"
    "      if (size != NULL) { *size = p->size; }\n"
    "      return (const char *) p->data;\n"
    "    }\n"
    "  }\n"
    "  return NULL;\n"
    "}\n";

static const char *my_strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack, *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (!*n) return haystack;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *fp;
    int i, j, ch;

    printf("%s\n", "#include \"embedded_resources.h\"");
    printf("%s\n", "#include <string.h>\n");

    printf("%s\n", "#ifdef __cplusplus\nextern \"C\" {\n#endif\n");

    for (i = 1; i < argc; i++) {
        if ((fp = fopen(argv[i], "rb")) == NULL) { exit(EXIT_FAILURE); } else {
            printf("static const unsigned char v%d[] = {", i);
            for (j = 0; (ch = fgetc(fp)) != EOF; j++) {
                if ((j % 12) == 0) { printf("%s", "\n "); }
                printf(" %#04x,", ch);
            }
            // Append zero-byte at the end to make text files appear in memory
            // as null-terminated strings.
            printf("%s", " 0x00\n};\n");
            fclose(fp);
        }
    }

    printf("%s", "\nconst struct embedded_file {\n");
    printf("%s", "  const char *name;\n");
    printf("%s", "  const unsigned char *data;\n");
    printf("%s", "  size_t size;\n");
    printf("%s", "} embedded_files[] = {\n");

    for (i = 1; i < argc; i++) {
        const char *path = argv[i];

        // Normalize backslashes to forward slashes (Windows)
        static char normalized[1024];
        strncpy(normalized, path, sizeof(normalized));
        normalized[sizeof(normalized) - 1] = '\0';
        for (char *p = normalized; *p; ++p) { if (*p == '\\') *p = '/'; }

        static const char *resourcesDir = "resources/";
        const char *base = my_strcasestr(normalized, resourcesDir);

        if (base != NULL) {
            base += strlen(resourcesDir);
            printf("  {\"%s\", v%d, sizeof(v%d) - 1},\n", base, i, i);
        } else {
            printf("  {\"%s\", v%d, sizeof(v%d) - 1},\n", argv[i], i, i);
        }
    }
    printf("%s", "  {NULL, NULL, 0}\n");
    printf("%s", "};\n\n");
    printf("%s\n", code);

    printf("%s\n", "#ifdef __cplusplus\n}\n#endif");

    return EXIT_SUCCESS;
}
