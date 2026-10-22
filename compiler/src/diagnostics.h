#ifndef I_DIAGNOSTICS_H
#define I_DIAGNOSTICS_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int line;
    int col;
    const char *file;
} SourceLoc;

typedef enum {
    SEV_ERROR,
    SEV_WARN,
    SEV_NOTE
} Severity;

void diag_init(const char *source, const char *filename);
void diag_report(Severity sev, const char *code, SourceLoc loc, const char *fmt, ...);
void diag_note(const char *fmt, ...);
bool diag_has_errors(void);
int diag_error_count(void);

#endif
