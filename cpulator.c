/* DISCLAIMER: This code was created with generative AI (gemini) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr) return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void process_file(const char *filename, FILE *out_fp) {
    FILE *in_fp = fopen(filename, "r");
    if (!in_fp) {
        fprintf(stderr, "Error: Could not open file '%s'. Skipping.\n", filename);
        return;
    }

    fprintf(out_fp, "// --- Start of %s ---\n", filename);

    char line[1024];

    while (fgets(line, sizeof(line), in_fp)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;

        if (strncmp(p, "#include \"", 10) == 0 || strncmp(p, "#include\"", 9) == 0) {
            fprintf(out_fp, "// Stripped local include: %s", line);
            continue;
        }

        if (strncmp(p, "#pragma once", 12) == 0) {
            fprintf(out_fp, "// Stripped pragma once: %s", line);
            continue;
        }

        fputs(line, out_fp);
    }

    fprintf(out_fp, "\n// --- End of %s ---\n\n", filename);
    fclose(in_fp);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1.c> <file2.h> ...\n", argv[0]);
        return 1;
    }

    const char *output_filename = "combined_output.c";
    FILE *out_fp = fopen(output_filename, "w");
    if (!out_fp) {
        perror("Error creating output file");
        return 1;
    }

    printf("Generating %s...\n", output_filename);

    for (int i = 1; i < argc; i++) {
        if (ends_with(argv[i], ".h")) {
            printf("Adding header file: %s\n", argv[i]);
            process_file(argv[i], out_fp);
        }
    }

    for (int i = 1; i < argc; i++) {
        if (ends_with(argv[i], ".c") || ends_with(argv[i], ".s")) {
            printf("Adding source file: %s\n", argv[i]);
            process_file(argv[i], out_fp);
        }
    }

    fclose(out_fp);
    printf("Success! All files combined into '%s'.\n", output_filename);

    return 0;
}