#ifndef I_SEMANTICS_H
#define I_SEMANTICS_H

#include "ast.h"
#include <stdbool.h>

bool semantics_analyze(AstProgram *prog);

#endif
