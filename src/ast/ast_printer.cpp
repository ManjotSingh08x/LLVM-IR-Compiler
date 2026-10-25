#include "ast/ast_printer.h"

namespace ast {

namespace {

const char *kindName(ASTKind kind) {
    switch (kind) {
    case ASTKind::Program:            return "Program";
    case ASTKind::TypeSpec:           return "TypeSpec";
    case ASTKind::VarDecl:            return "VarDecl";
    case ASTKind::ParamDecl:          return "ParamDecl";
    case ASTKind::FunctionDecl:       return "FunctionDecl";
    case ASTKind::ConstructorDecl:    return "ConstructorDecl";
    case ASTKind::DestructorDecl:     return "DestructorDecl";
    case ASTKind::RecordDecl:         return "RecordDecl";
    case ASTKind::AccessSpecDecl:     return "AccessSpecDecl";
    case ASTKind::TypedefDecl:        return "TypedefDecl";
    case ASTKind::CompoundStmt:       return "CompoundStmt";
    case ASTKind::DeclStmt:           return "DeclStmt";
    case ASTKind::ExprStmt:           return "ExprStmt";
    case ASTKind::IfStmt:             return "IfStmt";
    case ASTKind::SwitchStmt:         return "SwitchStmt";
    case ASTKind::WhileStmt:          return "WhileStmt";
    case ASTKind::DoStmt:             return "DoStmt";
    case ASTKind::ForStmt:            return "ForStmt";
    case ASTKind::ReturnStmt:         return "ReturnStmt";
    case ASTKind::BreakStmt:          return "BreakStmt";
    case ASTKind::ContinueStmt:       return "ContinueStmt";
    case ASTKind::GotoStmt:           return "GotoStmt";
    case ASTKind::LabelStmt:          return "LabelStmt";
    case ASTKind::CaseStmt:           return "CaseStmt";
    case ASTKind::DefaultStmt:        return "DefaultStmt";
    case ASTKind::IntLiteral:         return "IntLiteral";
    case ASTKind::FloatLiteral:       return "FloatLiteral";
    case ASTKind::CharLiteral:        return "CharLiteral";
    case ASTKind::StringLiteral:      return "StringLiteral";
    case ASTKind::BoolLiteral:        return "BoolLiteral";
    case ASTKind::NullLiteral:        return "NullLiteral";
    case ASTKind::ThisExpr:           return "ThisExpr";
    case ASTKind::DeclRefExpr:        return "DeclRefExpr";
    case ASTKind::ParenExpr:          return "ParenExpr";
    case ASTKind::UnaryExpr:          return "UnaryExpr";
    case ASTKind::BinaryExpr:         return "BinaryExpr";
    case ASTKind::AssignExpr:         return "AssignExpr";
    case ASTKind::ConditionalExpr:    return "ConditionalExpr";
    case ASTKind::CallExpr:           return "CallExpr";
    case ASTKind::ArraySubscriptExpr: return "ArraySubscriptExpr";
    case ASTKind::MemberExpr:         return "MemberExpr";
    case ASTKind::NewExpr:            return "NewExpr";
    case ASTKind::DeleteExpr:         return "DeleteExpr";
    case ASTKind::InitListExpr:       return "InitListExpr";
    case ASTKind::LambdaExpr:         return "LambdaExpr";
    case ASTKind::ImplicitCastExpr:   return "ImplicitCastExpr";
    }
    return "Unknown";
}

const char *opSpelling(UnaryOp op) {
    switch (op) {
    case UnaryOp::PreInc:     return "++";
    case UnaryOp::PreDec:     return "--";
    case UnaryOp::PostInc:    return "++";
    case UnaryOp::PostDec:    return "--";
    case UnaryOp::Plus:       return "+";
    case UnaryOp::Minus:      return "-";
    case UnaryOp::LogicalNot: return "!";
    case UnaryOp::BitNot:     return "~";
    case UnaryOp::Deref:      return "*";
    case UnaryOp::AddrOf:     return "&";
    }
    return "?";
}

const char *opSpelling(BinaryOp op) {
    switch (op) {
    case BinaryOp::LogicalOr:  return "||";
    case BinaryOp::LogicalAnd: return "&&";
    case BinaryOp::BitOr:      return "|";
    case BinaryOp::BitXor:     return "^";
    case BinaryOp::BitAnd:     return "&";
    case BinaryOp::Eq:         return "==";
    case BinaryOp::Ne:         return "!=";
    case BinaryOp::Lt:         return "<";
    case BinaryOp::Gt:         return ">";
    case BinaryOp::Le:         return "<=";
    case BinaryOp::Ge:         return ">=";
    case BinaryOp::Shl:        return "<<";
    case BinaryOp::Shr:        return ">>";
    case BinaryOp::Add:        return "+";
    case BinaryOp::Sub:        return "-";
    case BinaryOp::Mul:        return "*";
    case BinaryOp::Div:        return "/";
    case BinaryOp::Rem:        return "%";
    }
    return "?";
}

const char *opSpelling(AssignOp op) {
    switch (op) {
    case AssignOp::Assign:    return "=";
    case AssignOp::AddAssign: return "+=";
    case AssignOp::SubAssign: return "-=";
    case AssignOp::MulAssign: return "*=";
    case AssignOp::DivAssign: return "/=";
    case AssignOp::RemAssign: return "%=";
    }
    return "?";
}

const char *accessName(AccessKind access) {
    switch (access) {
    case AccessKind::Public:    return "public";
    case AccessKind::Private:   return "private";
    case AccessKind::Protected: return "protected";
    case AccessKind::None:      break;
    }
    return "";
}

std::string typeString(const TypeSpec *type, const std::string &inside);

bool isArrayOrFunction(const TypeSpec *type) {
    return type != nullptr
        && (type->specKind == TypeSpecKind::Array || type->specKind == TypeSpecKind::Function);
}

std::string paramList(const TypeSpec *fn) {
    std::string out = "(";
    for (size_t i = 0; i < fn->params.size(); i++) {
        if (i > 0) out += ", ";
        out += typeString(fn->params[i]->type.get(), "");
    }
    return out + ")";
}

//spells a written type C style, built outside in: int **(*)[10], int (*)(int)
std::string typeString(const TypeSpec *type, const std::string &inside) {
    if (type == nullptr) return "?" + inside;
    switch (type->specKind) {
    case TypeSpecKind::Builtin:
    case TypeSpecKind::Named:
    case TypeSpecKind::Struct: {
        std::string name = type->specKind == TypeSpecKind::Struct ? "struct " + type->name : type->name;
        if (inside.empty()) return name;
        if (inside[0] == '*' || inside[0] == '&') return name + inside;
        return name + " " + inside;
    }
    case TypeSpecKind::Pointer:
    case TypeSpecKind::Reference: {
        std::string mark = type->specKind == TypeSpecKind::Pointer ? "*" : "&";
        if (isArrayOrFunction(type->inner.get())) return typeString(type->inner.get(), "(" + mark + inside + ")");
        return typeString(type->inner.get(), mark + inside);
    }
    case TypeSpecKind::Array: {
        std::string size;
        if (type->size && type->size->getKind() == ASTKind::IntLiteral) {
            size = static_cast<const IntLiteral *>(type->size.get())->spelling;
        } else if (type->size) {
            size = "expr";
        }
        return typeString(type->inner.get(), inside + "[" + size + "]");
    }
    case TypeSpecKind::Function:
        return typeString(type->inner.get(), inside + paramList(type));
    }
    return "?";
}

std::string typeString(const TypeSpec *type) { return typeString(type, ""); }

const char *initStyleName(InitStyle style) {
    switch (style) {
    case InitStyle::None:     return "";
    case InitStyle::Copy:     return "copy";
    case InitStyle::Brace:    return "brace";
    case InitStyle::CtorArgs: return "ctor";
    }
    return "";
}

const char *newFormName(NewForm form) {
    switch (form) {
    case NewForm::Plain:     return "plain";
    case NewForm::Args:      return "args";
    case NewForm::InitList:  return "initlist";
    case NewForm::Array:     return "array";
    case NewForm::ArrayInit: return "array+initlist";
    }
    return "";
}

const char *castKindName(CastKind kind) {
    switch (kind) {
    case CastKind::IntegralCast:       return "integral";
    case CastKind::IntegralToFloating: return "integral-to-floating";
    case CastKind::FloatingToIntegral: return "floating-to-integral";
    case CastKind::FloatingCast:       return "floating";
    case CastKind::ToBool:             return "to-bool";
    case CastKind::ArrayToPointer:     return "array-to-pointer";
    case CastKind::FunctionToPointer:  return "function-to-pointer";
    case CastKind::NullToPointer:      return "null-to-pointer";
    case CastKind::DerivedToBase:      return "derived-to-base";
    case CastKind::PointerToVoid:      return "pointer-to-void";
    }
    return "";
}

std::string quoted(const std::string &text) { return "'" + text + "'"; }

} // namespace

void ASTPrinter::line(const ASTNode &node, const std::string &attrs) {
    out << std::string(depth * 2, ' ');
    if (role != nullptr) out << role << ": ";
    role = nullptr;
    out << kindName(node.getKind());
    if (!attrs.empty()) out << " " << attrs;
    out << " <" << node.line << ":" << node.col << ">\n";
}

void ASTPrinter::child(const char *childRole, ASTNode *node) {
    if (node == nullptr) return;
    depth++;
    role = childRole;
    visit(node);
    depth--;
}

void ASTPrinter::typeChildren(const TypeSpec *type) {
    for (; type != nullptr; type = type->inner.get()) {
        if (type->specKind == TypeSpecKind::Array) {
            if (type->size) {
                child("dim", type->size.get());
            } else {
                out << std::string((depth + 1) * 2, ' ') << "dim: <unsized>\n";
            }
        }
        if (type->specKind == TypeSpecKind::Function) children("param", type->params);
    }
}

void ASTPrinter::initChildren(VarDecl *node) {
    child("init", node->init.get());
    children("arg", node->ctorArgs);
}

//program and type

void ASTPrinter::visitProgram(Program *node) {
    line(*node, "");
    children(nullptr, node->decls);
}

void ASTPrinter::visitTypeSpec(TypeSpec *node) {
    line(*node, quoted(typeString(node)));
}

//declarations

void ASTPrinter::visitVarDecl(VarDecl *node) {
    std::string attrs = node->name.str() + " " + quoted(typeString(node->type.get()));
    if (node->isStatic) attrs += " static";
    if (node->initStyle != InitStyle::None) {
        attrs += std::string(" init=") + initStyleName(node->initStyle);
    }
    line(*node, attrs);
    typeChildren(node->type.get());
    initChildren(node);
}

void ASTPrinter::visitParamDecl(ParamDecl *node) {
    std::string name = node->name.empty() ? "<unnamed>" : node->name;
    line(*node, name + " " + quoted(typeString(node->type.get())));
    typeChildren(node->type.get());
}

void ASTPrinter::functionBody(FunctionDecl *node) {
    children("param", node->params);
    child("body", node->body.get());
}

void ASTPrinter::visitFunctionDecl(FunctionDecl *node) {
    std::string attrs = node->name.str() + " returns " + quoted(typeString(node->returnType.get()));
    if (node->isStatic) attrs += " static";
    if (!node->isDefinition()) attrs += " declaration";
    line(*node, attrs);
    functionBody(node);
}

void ASTPrinter::visitConstructorDecl(ConstructorDecl *node) {
    line(*node, node->name.str() + (node->isDefinition() ? "" : " declaration"));
    functionBody(node);
}

void ASTPrinter::visitDestructorDecl(DestructorDecl *node) {
    line(*node, node->name.str() + (node->isDefinition() ? "" : " declaration"));
    functionBody(node);
}

void ASTPrinter::visitRecordDecl(RecordDecl *node) {
    std::string attrs = std::string(node->isClass ? "class " : "struct ") + node->name;
    for (size_t i = 0; i < node->bases.size(); i++) {
        attrs += i == 0 ? " : " : ", ";
        if (node->bases[i].access != AccessKind::None) {
            attrs += std::string(accessName(node->bases[i].access)) + " ";
        }
        attrs += node->bases[i].name;
    }
    line(*node, attrs);
    children("member", node->members);
}

void ASTPrinter::visitAccessSpecDecl(AccessSpecDecl *node) {
    line(*node, accessName(node->access));
}

void ASTPrinter::visitTypedefDecl(TypedefDecl *node) {
    line(*node, node->name + " " + quoted(typeString(node->type.get())));
    typeChildren(node->type.get());
}

//statements

void ASTPrinter::visitCompoundStmt(CompoundStmt *node) {
    line(*node, "");
    children(nullptr, node->body);
}

void ASTPrinter::visitDeclStmt(DeclStmt *node) {
    line(*node, "");
    children(nullptr, node->decls);
}

void ASTPrinter::visitExprStmt(ExprStmt *node) {
    line(*node, node->expr ? "" : "<empty>");
    child(nullptr, node->expr.get());
}

void ASTPrinter::visitIfStmt(IfStmt *node) {
    line(*node, node->elseStmt ? "has_else" : "");
    child("cond", node->cond.get());
    child("then", node->thenStmt.get());
    child("else", node->elseStmt.get());
}

void ASTPrinter::visitSwitchStmt(SwitchStmt *node) {
    line(*node, "");
    child("cond", node->cond.get());
    child("body", node->body.get());
}

void ASTPrinter::visitWhileStmt(WhileStmt *node) {
    line(*node, node->isUntil ? "until" : "");
    child("cond", node->cond.get());
    child("body", node->body.get());
}

void ASTPrinter::visitDoStmt(DoStmt *node) {
    line(*node, node->isUntil ? "until" : "");
    child("body", node->body.get());
    child("cond", node->cond.get());
}

void ASTPrinter::visitForStmt(ForStmt *node) {
    line(*node, "");
    child("init", node->init.get());
    child("cond", node->cond.get());
    child("inc", node->inc.get());
    child("body", node->body.get());
}

void ASTPrinter::visitReturnStmt(ReturnStmt *node) {
    line(*node, "");
    child(nullptr, node->value.get());
}

void ASTPrinter::visitBreakStmt(BreakStmt *node) { line(*node, ""); }

void ASTPrinter::visitContinueStmt(ContinueStmt *node) { line(*node, ""); }

void ASTPrinter::visitGotoStmt(GotoStmt *node) { line(*node, node->label); }

void ASTPrinter::visitLabelStmt(LabelStmt *node) {
    line(*node, node->label);
    child(nullptr, node->sub.get());
}

void ASTPrinter::visitCaseStmt(CaseStmt *node) {
    line(*node, "");
    child("value", node->value.get());
    child(nullptr, node->sub.get());
}

void ASTPrinter::visitDefaultStmt(DefaultStmt *node) {
    line(*node, "");
    child(nullptr, node->sub.get());
}

//expressions

void ASTPrinter::visitIntLiteral(IntLiteral *node) { line(*node, node->spelling); }

void ASTPrinter::visitFloatLiteral(FloatLiteral *node) { line(*node, node->spelling); }

void ASTPrinter::visitCharLiteral(CharLiteral *node) { line(*node, node->spelling); }

void ASTPrinter::visitStringLiteral(StringLiteral *node) {
    std::string escaped;
    for (char c : node->spelling) {
        if (c == '\n') escaped += "\\n";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }
    line(*node, "\"" + escaped + "\"");
}

void ASTPrinter::visitBoolLiteral(BoolLiteral *node) { line(*node, node->value ? "true" : "false"); }

void ASTPrinter::visitNullLiteral(NullLiteral *node) { line(*node, ""); }

void ASTPrinter::visitThisExpr(ThisExpr *node) { line(*node, ""); }

void ASTPrinter::visitDeclRefExpr(DeclRefExpr *node) { line(*node, node->name); }

void ASTPrinter::visitParenExpr(ParenExpr *node) {
    line(*node, "");
    child(nullptr, node->sub.get());
}

void ASTPrinter::visitUnaryExpr(UnaryExpr *node) {
    line(*node, quoted(opSpelling(node->op)) + (node->isPostfix() ? " postfix" : " prefix"));
    child(nullptr, node->operand.get());
}

void ASTPrinter::visitBinaryExpr(BinaryExpr *node) {
    line(*node, quoted(opSpelling(node->op)));
    child(nullptr, node->lhs.get());
    child(nullptr, node->rhs.get());
}

void ASTPrinter::visitAssignExpr(AssignExpr *node) {
    line(*node, quoted(opSpelling(node->op)));
    child("target", node->target.get());
    child("value", node->value.get());
}

void ASTPrinter::visitConditionalExpr(ConditionalExpr *node) {
    line(*node, "");
    child("cond", node->cond.get());
    child("true", node->trueExpr.get());
    child("false", node->falseExpr.get());
}

void ASTPrinter::visitCallExpr(CallExpr *node) {
    line(*node, std::to_string(node->args.size()) + " arg(s)");
    child("callee", node->callee.get());
    children("arg", node->args);
}

void ASTPrinter::visitArraySubscriptExpr(ArraySubscriptExpr *node) {
    line(*node, "");
    child("base", node->base.get());
    child("index", node->index.get());
}

void ASTPrinter::visitMemberExpr(MemberExpr *node) {
    line(*node, std::string(node->isArrow ? "->" : ".") + node->member);
    child("base", node->base.get());
}

void ASTPrinter::visitNewExpr(NewExpr *node) {
    line(*node, quoted(typeString(node->type.get())) + " " + newFormName(node->form));
    children("dim", node->dims);
    children("arg", node->args);
    child("init", node->init.get());
}

void ASTPrinter::visitDeleteExpr(DeleteExpr *node) {
    line(*node, node->isArray ? "array" : "");
    child(nullptr, node->operand.get());
}

void ASTPrinter::visitInitListExpr(InitListExpr *node) {
    line(*node, std::to_string(node->inits.size()) + " item(s)");
    children(nullptr, node->inits);
}

void ASTPrinter::visitLambdaExpr(LambdaExpr *node) {
    std::string captures;
    if (node->captureDefault == CaptureDefault::ByCopy) captures = "=";
    if (node->captureDefault == CaptureDefault::ByRef) captures = "&";
    for (size_t i = 0; i < node->captures.size(); i++) {
        if (i > 0) captures += ", ";
        const LambdaCapture &capture = node->captures[i];
        if (capture.kind == LambdaCapture::Kind::This) captures += "this";
        else captures += (capture.kind == LambdaCapture::Kind::ByRef ? "&" : "") + capture.name;
    }
    line(*node, "[" + captures + "]");
    children("param", node->params);
    child("body", node->body.get());
}

void ASTPrinter::visitImplicitCastExpr(ImplicitCastExpr *node) {
    line(*node, castKindName(node->castKind));
    child(nullptr, node->sub.get());
}

} // namespace ast
