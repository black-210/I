#include "diagnostics.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

static const char *s_source = NULL;
static const char *s_filename = "<input>";
static int s_errors = 0;

void diag_init(const char *source, const char *filename) {
    s_source = source;
    s_filename = filename ? filename : "<input>";
    s_errors = 0;
}

void diag_report(Severity sev, const char *code, SourceLoc loc, const char *fmt, ...) {
    if (sev == SEV_ERROR) s_errors++;

    const char *sev_str = (sev == SEV_ERROR) ? "\033[1;31merror\033[0m" :
                          (sev == SEV_WARN)  ? "\033[1;33mwarning\033[0m" : "\033[1;36mnote\033[0m";

    fprintf(stderr, "%s:%d:%d: %s[%s]: ", loc.file ? loc.file : s_filename, loc.line, loc.col, sev_str, code);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    // Print source line if available
    if (s_source && loc.line > 0) {
        int cur_line = 1;
        const char *p = s_source;
        while (*p && cur_line < loc.line) {
            if (*p == '\n') cur_line++;
            p++;
        }
        if (*p) {
            const char *end = strchr(p, '\n');
            int len = end ? (int)(end - p) : (int)strlen(p);
            fprintf(stderr, "  %4d | %.*s\n", loc.line, len, p);
            fprintf(stderr, "       | ");
            for (int i = 1; i < loc.col; i++) fprintf(stderr, " ");
            fprintf(stderr, "\033[1;31m^\033[0m\n");
        }
    }
}

void diag_note(const char *fmt, ...) {
    fprintf(stderr, "       = \033[1;36mnote\033[0m: ");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

bool diag_has_errors(void) {
    return s_errors > 0;
}

int diag_error_count(void) {
    return s_errors;
}
