#ifndef AST_AST_H
#define AST_AST_H

//the whole node hierarchy. each header includes the one before it:
//base.h <- statements.h <- declarations.h <- expressions.h
#include "ast/base.h"
#include "ast/statements.h"
#include "ast/declarations.h"
#include "ast/expressions.h"

#endif
