#include <Front.h>

#define TH_INT      0x00000000001A7774
#define TH_UINT     0x000000000EBA7774
#define TH_UINT64   0x000003AE9DDD1B34
#define TH_UINT32   0x000003AE9DDD19B2
#define TH_UINT16   0x000003AE9DDD18B6
#define TH_UINT8    0x000000075D3BBA38
#define TH_INT64    0x000000069DDD1B34
#define TH_INT32    0x000000069DDD19B2
#define TH_INT16    0x000000069DDD18B6
#define TH_INT8     0x000000000D3BBA38
#define TH_BOOL     0x000000000C5BF7EC
#define TH_CHAR     0x000000000C7A30F2
#define TH_STRUCT   0x0000039F4E5D71F4
#define TH_UNION    0x000000075DDA77EE
#define TH_VOID     0x000000000EDBF4E4
#define TH_STATIC   0x0000039F4C3D34E3
#define TH_EXTERN   0x0000032F8E99796E
#define TH_RETURN   0x000003965E9D796E
#define TH_SWITCH   0x0000039F7D3D31E8
#define TH_IF       0x00000000000034E6
#define TH_ELSE     0x000000000CBB39E5
#define TH_DO       0x000000000000326F
#define TH_WHILE    0x000000077D1A7665
#define TH_FOR      0x000000000019B7F2
#define TH_BREAK    0x000000062E5970EB
#define TH_CONTINUE 0x00C7BF774D3BBAE5
#define TH_FOR      0x000000000019B7F2
#define TH_CASE     0x000000000C7879E5
#define TH_DEFAULT  0x0001932E6C3D7674


// All functions lex one token ahead.
// If you call ParseTerm() with "546 + 235" as source
// It will lex '+' too.


static struct Type *type_uint64 = &(struct Type) { .kind = KIND_UINT, .size = 8 };
static struct Type *type_uint32 = &(struct Type) { .kind = KIND_UINT, .size = 4 };
static struct Type *type_uint16 = &(struct Type) { .kind = KIND_UINT, .size = 2 };
static struct Type *type_uint8  = &(struct Type) { .kind = KIND_UINT, .size = 1 };
static struct Type *type_int64  = &(struct Type) { .kind = KIND_INT,  .size = 8 };
static struct Type *type_int32  = &(struct Type) { .kind = KIND_INT,  .size = 4 };
static struct Type *type_int16  = &(struct Type) { .kind = KIND_INT,  .size = 2 };
static struct Type *type_int8   = &(struct Type) { .kind = KIND_INT,  .size = 1 };
static struct Type *type_void   = &(struct Type) { .kind = KIND_VOID, .size = 0 };

static uint64 vstk_top[3] = { 0 };


static uint64 VarStackPush(uint8 stor, uint64 size)
{
	size = (size + 7) & ~0xF;

	vstk_top[stor] += size;
	return vstk_top[stor] - size;
}

static void VarStackReset(uint8 stor)
{
	vstk_top[stor] = 0;
}

static void PrintKeywordHashes()
{
	printf("#define TH_INT      0x%016llX\n", Hash("int"));
	printf("#define TH_UINT     0x%016llX\n", Hash("uint"));
	printf("#define TH_UINT64   0x%016llX\n", Hash("uint64"));
	printf("#define TH_UINT32   0x%016llX\n", Hash("uint32"));
	printf("#define TH_UINT16   0x%016llX\n", Hash("uint16"));
	printf("#define TH_UINT8    0x%016llX\n", Hash("uint8"));
	printf("#define TH_INT64    0x%016llX\n", Hash("int64"));
	printf("#define TH_INT32    0x%016llX\n", Hash("int32"));
	printf("#define TH_INT16    0x%016llX\n", Hash("int16"));
	printf("#define TH_INT8     0x%016llX\n", Hash("int8"));
	printf("#define TH_BOOL     0x%016llX\n", Hash("bool"));
	printf("#define TH_CHAR     0x%016llX\n", Hash("char"));
	printf("#define TH_STRUCT   0x%016llX\n", Hash("struct"));
	printf("#define TH_UNION    0x%016llX\n", Hash("union"));
	printf("#define TH_VOID     0x%016llX\n", Hash("void"));
	printf("#define TH_STATIC   0x%016llX\n", Hash("static"));
	printf("#define TH_EXTERN   0x%016llX\n", Hash("extern"));
	printf("#define TH_RETURN   0x%016llX\n", Hash("return"));
	printf("#define TH_SWITCH   0x%016llX\n", Hash("switch"));
	printf("#define TH_IF       0x%016llX\n", Hash("if"));
	printf("#define TH_ELSE     0x%016llX\n", Hash("else"));
	printf("#define TH_DO       0x%016llX\n", Hash("do"));
	printf("#define TH_WHILE    0x%016llX\n", Hash("while"));
	printf("#define TH_FOR      0x%016llX\n", Hash("for"));
	printf("#define TH_BREAK    0x%016llX\n", Hash("break"));
	printf("#define TH_CONTINUE 0x%016llX\n", Hash("continue"));
	printf("#define TH_FOR      0x%016llX\n", Hash("for"));
	printf("#define TH_CASE     0x%016llX\n", Hash("case"));
	printf("#define TH_DEFAULT  0x%016llX\n", Hash("default"));

}

static struct Node *ParseLocator(struct Token *tok)
{
	struct SymVar *sym = SymFind(tok->str, STT_VAR);

	if(sym == NULL)
		LexError("Symbol '%s' doesn't exist", tok->str);


	struct LValue *lval = AllocNode(NODE_LVALUE);

	lval->sym = &sym->hdr;

	struct Type *type = sym->type;

	struct Node *expr = &lval->node;

	while(Lex(tok) == '.' || tok->token == '->') {
		if(type->kind != KIND_STRUC && tok->token == '.')
			LexError("Tried to get member of non-struct type");

		if(type->kind != KIND_PTR && tok->token == '->')
			LexError("Tried to get member of non-pointer-to-struct type");

		if(type->kind == KIND_PTR && type->ptr->kind != KIND_STRUC)
			LexError("Tried to get member of non-pointer-to-struct type");

		bool dref = FALSE;
		if(type->kind == KIND_PTR) {
			type = TypeStrucOf(type->ptr->struc);
			dref = TRUE;
		}

		if(Lex(tok) != TK_IDENT)
			LexError("Expected member name, got %s", Tok2Str(tok->token));

		struct SymStruc *struc = type->struc;

		uint64 i = 0;

		for(; i < struc->membc; i++) {
			if(strcmp(struc->membn[i], tok->str) == 0)
				break;
		}

		if(i == struc->membc)
			LexError("struct %s does not contain field '%s'", struc->hdr.name, tok->str);

		if(dref) {
			struct Expr *dexpr = AllocNode(NODE_EXPR);

			dexpr->op   = EXPR_DREF;
			dexpr->type = struc->membt[i];

			struct Expr *mexpr = AllocNode(NODE_EXPR);
			mexpr->op   = EXPR_ADD;
			mexpr->type = TypePtrTo(struc->membt[i]);

			struct RValue *off = AllocNode(NODE_RVALUE);
			off->type = type_uint64;
			off->imm  = struc->membo[i];

			SET_LEFT(dexpr,  expr);
			SET_LEFT(mexpr, dexpr);
			SET_RIGHT(mexpr, off);

			expr = &mexpr->node;
		} else {
			struct Expr *mexpr = AllocNode(NODE_EXPR);
			mexpr->op   = EXPR_ADD;
			mexpr->type = struc->membt[i];

			struct RValue *off = AllocNode(NODE_RVALUE);
			off->type = type_uint64;
			off->imm  = struc->membo[i];

			SET_LEFT(mexpr,  expr);
			SET_RIGHT(mexpr, off);

			expr = &mexpr->node;
		}
		type = struc->membt[i];
	}

	if(expr->type != NODE_LVALUE) {
		struct LValue *lval = AllocNode(NODE_LVALUE);
		SET_LEFT(lval, expr);
		return &lval->node;
	}

	return expr;
}

static struct Type *ParseType(struct Token *tok)
{
	if(tok->token != TK_IDENT)
		return NULL;

	struct Type *type;

	switch(tok->hash)
	{
	case TH_INT:    type = type_int64;  break;
	case TH_UINT:   type = type_uint64; break;
	case TH_UINT64: type = type_uint64; break;
	case TH_UINT32: type = type_uint32; break;
	case TH_UINT16: type = type_uint16; break;
	case TH_UINT8:  type = type_uint8;  break;
	case TH_INT64:  type = type_int64;  break;
	case TH_INT32:  type = type_int32;  break;
	case TH_INT16:  type = type_int16;  break;
	case TH_INT8:   type = type_int8;   break;
	case TH_BOOL:   type = type_int32;  break;
	case TH_CHAR:   type = type_uint8;  break;
	case TH_VOID:   type = type_void;   break;
	case TH_STRUCT: type = NULL;        break;
	default:        return NULL;
	}

	if(type == NULL) {
		if(Lex(tok) != TK_IDENT)
			LexError("Expected struct name, got %s", Tok2Str(tok->token));

		struct SymStruc *struc = SymFind(tok->str, STT_STRUC);

		if(struc == NULL)
			LexError("struct %s does not exist", tok->str);

		type = TypeStrucOf(struc);
	}

	while(Lex(tok) == '*')
		type = TypePtrTo(type);

	if(tok->token == '[') {
		Lex(tok);

		struct Node *node;
		if(tok->token == ']') {
			node = AllocNode(NODE_RVALUE);
			((struct RValue *) node)->type = type_uint64;
			((struct RValue *) node)->imm  = 0;
		} else {
			node = ParseExpr(tok);
		}

		if(node == NULL || !NodeConst(node))
			LexError("Expected constant expression for array size");

		if(tok->token != ']')
			LexError("Expected ']', got %s", Tok2Str(tok->token));

		Lex(tok);

		struct Type *arr = TypeMake();
		arr->kind = KIND_ARR;
		arr->size = NodeConstEval(node);

		arr->ptr = type;
		type = arr;
	}

	return type;
}

static struct Node *ParseDecl(struct Token *tok)
{
	if(tok->token != TK_IDENT)
		return NULL;

	uint64 sc = STOR_AUTO;

	if(tok->hash == TH_STATIC)
		sc = STOR_STATIC;
	else if(tok->hash == TH_EXTERN)
		sc = STOR_EXTERN;

	if(sc != STOR_AUTO)
		Lex(tok);

	struct Type *type = ParseType(tok);

	if(type == NULL && sc != STOR_AUTO)
		LexError("Expected type, got %s", Tok2Str(tok->token));

	if(type == NULL)
		return NULL;

	if(tok->token != TK_IDENT)
		LexError("Expected identifier, got %s", Tok2Str(tok->token));

	char *name = tok->str;

	if(Lex(tok) == '(') {
		LexError("TODO: FUNC DECL");
	}

	struct SymVar *var = AllocZ(sizeof(struct SymVar));

	var->hdr.name = name;
	var->type = type;
	var->addr = VarStackPush(sc, type->size);
	var->stor = sc;

	if(!SymInsert(&var->hdr, STT_VAR))
		LexError("Symbol '%s' redefined", name);

	struct LValue *lval = AllocNode(NODE_LVALUE);

	lval->sym = &var->hdr;

	return &lval->node;
}

static struct Node *ParseAssign(struct Token *tok, struct Node *lhs)
{
	uint64 op = tok->token;
	if((op & 0xFF) != '=')
		return NULL;

	Lex(tok);

	struct Node *rhs = ParseExpr(tok);

	if(rhs == NULL)
		LexError("Expected right-hand side of assignment");

	if(lhs->type != NODE_LVALUE)
		LexError("Expected lvalue as left-hand side of assignment");

	if((op & ~0xFF) != 0) {
		struct RValue *rval = AllocNode(NODE_RVALUE);
		SET_LEFT(rval, lhs);

		struct Expr *rexpr = AllocNode(NODE_EXPR);

		SET_LEFT(rexpr, rval);
		SET_RIGHT(rexpr, rhs);

		switch(op)
		{
		case '+=':  rexpr->op = EXPR_ADD; break;
		case '-=':  rexpr->op = EXPR_SUB; break;
		case '>>=': rexpr->op = EXPR_SHR; break;
		case '<<=': rexpr->op = EXPR_SHL; break;
		case '*=':  rexpr->op = EXPR_MUL; break;
		case '/=':  rexpr->op = EXPR_DIV; break;
		case '%=':  rexpr->op = EXPR_MOD; break;
		case '&=':  rexpr->op = EXPR_AND; break;
		case '^=':  rexpr->op = EXPR_XOR; break;
		case '|=':  rexpr->op = EXPR_OR;  break;
		default:
			LexError("Expected valid assignment operator, got %s", Tok2Str(tok->token));
		}

		rhs = &rexpr->node;
	}

	struct Node *assign = AllocNode(NODE_ASSIGN);

	SET_LEFT(assign, lhs);
	SET_RIGHT(assign, rhs);

	return assign;
}

static struct Node *ParsePrimaryTerm(struct Token *tok)
{
	switch(tok->token)
	{
	case TK_INT: {
		struct RValue *rval = AllocNode(NODE_RVALUE);
		rval->type = type_uint64;
		rval->imm  = tok->num;

		Lex(tok);
		return &rval->node;
	  }
	case TK_IDENT: {
		struct Node *node = ParseLocator(tok);

		if(tok->token == '++' || tok->token == '--') {
			struct Expr *inc = AllocNode(NODE_EXPR);
			inc->op = tok->token == '--' ? EXPR_PSUB : EXPR_PADD;

			SET_LEFT(inc, node);

			Lex(tok);
			return &inc->node;
		}

		struct RValue *rval = AllocNode(NODE_RVALUE);
		SET_LEFT(rval, node);

		return &rval->node;
	  }
	case '(': {
		Lex(tok);

		struct Node *node = ParseExpr(tok);

		if(node == NULL)
			LexError("Expected expression");

		
		if(tok->token != ')')
			LexError("Expected ')'");

		Lex(tok);
		return node;
	  }
	case '--':
	case '++': {
		struct Expr *inc = AllocNode(NODE_EXPR);
		inc->op = tok->token == '--' ? EXPR_BSUB : EXPR_BADD;

		Lex(tok);
		struct Node *node = ParsePrimaryTerm(tok);

		if(node == NULL)
			LexError("Expected lvalue");

		if(node->type == NODE_RVALUE && ((struct RValue *) node)->type == NULL)
			node = node->left;

		if(node->type != NODE_LVALUE)
			LexError("Expected lvalue");


		SET_LEFT(inc, node);
		return &inc->node;
	  }
	default:
		return NULL;
	}
}

static struct Node *ParseUnaryPostfix(struct Token *tok)
{
	struct Node *node = ParsePrimaryTerm(tok);

	if(node == NULL)
		return NULL;

	while(tok->token == '[') {
		Lex(tok);

		struct Node *idx = ParseExpr(tok);

		if(idx == NULL)
			LexError("Expected array index");
	
		if(tok->token != ']')
			LexError("Expected ']'");

		Lex(tok);


		struct Expr *expr = AllocNode(NODE_EXPR);
	
		expr->op = EXPR_ADD;
		SET_LEFT(expr, node);
		SET_RIGHT(expr, idx);

		struct Expr *dref = AllocNode(NODE_EXPR);

		dref->op = EXPR_DREF;
		SET_LEFT(dref, expr);

		node = &dref->node;
	}

	return node;
}

static struct Node *ParseTerm(struct Token *tok)
{
	uint64 op = 0;
	switch(tok->token)
	{
	case '+':  op = EXPR_PLUS;  break;
	case '-':  op = EXPR_MINUS; break;
	case '!':  op = EXPR_LNOT;  break;
	case '~':  op = EXPR_NOT;   break;
	case '&':  op = EXPR_LEA;   break;
	default:  return ParseUnaryPostfix(tok);
	}

	Lex(tok);

	struct Expr *expr = AllocNode(NODE_EXPR);
	expr->op = op;

	struct Node *node = ParseTerm(tok);
	if(node == NULL)
		LexError("Expected term");

	SET_LEFT(expr, node);

	return &expr->node;
}

static uint64 ParseOp(struct Token *tok, struct Node *left, struct Node **node, uint64 prec)
{
	uint64 op = tok->token;
	uint64 prec_new = 0;

	switch(op)
	{
	case '<<': op = EXPR_SHL;  prec_new = PREC_SHIFT; break;
	case '>>': op = EXPR_SHR;  prec_new = PREC_SHIFT; break;
	case '*':  op = EXPR_MUL;  prec_new = PREC_MUL;   break;
	case '/':  op = EXPR_DIV;  prec_new = PREC_MUL;   break;
	case '%':  op = EXPR_MOD;  prec_new = PREC_MUL;   break;
	case '+':  op = EXPR_ADD;  prec_new = PREC_ADD;   break;
	case '-':  op = EXPR_SUB;  prec_new = PREC_ADD;   break;
	case '&':  op = EXPR_AND;  prec_new = PREC_AND;   break;
	case '^':  op = EXPR_XOR;  prec_new = PREC_XOR;   break;
	case '|':  op = EXPR_OR;   prec_new = PREC_OR;    break;
	case '>':  op = EXPR_GT;   prec_new = PREC_CMP;   break;
	case '>=': op = EXPR_GTE;  prec_new = PREC_CMP;   break;
	case '<':  op = EXPR_LT;   prec_new = PREC_CMP;   break;
	case '<=': op = EXPR_LTE;  prec_new = PREC_CMP;   break;
	case '==': op = EXPR_EQU;  prec_new = PREC_EQU;   break;
	case '!=': op = EXPR_NEQ;  prec_new = PREC_EQU;   break;
	case '&&': op = EXPR_LAND; prec_new = PREC_LAND;  break;
	case '||': op = EXPR_LOR;  prec_new = PREC_LOR;   break;
	default:                   prec_new = PREC_NONE;  break;
	}

	if(prec_new > prec) {
		*node = left;
		return prec_new;
	}

	Lex(tok);

	uint64 prec_cur = prec_new;

	struct Expr *expr = AllocNode(NODE_EXPR);

	expr->op = op;
	SET_LEFT(expr, left);

	while(prec_new <= prec_cur)
	{
		if(expr->node.right != NULL) {
			struct Expr *parent = AllocNode(NODE_EXPR);
			parent->op = op;
			SET_LEFT(parent, expr);
			expr = parent;
		}

		left = ParseTerm(tok);

		if(left != NULL) {
			struct Node *right = NULL;
			prec_new = ParseOp(tok, left, &right, prec_new);

			SET_RIGHT(expr, right);
			*node = &expr->node;
		} else {
			expr->node.left->parent = NULL;
			*node = expr->node.left;
			prec_new = PREC_NONE;
		}
	}

	if(prec_new < prec)
		prec_new = ParseOp(tok, *node, node, prec);

	return prec_new;
}

struct Node *ParseExpr(struct Token *tok)
{
	struct Node *node = ParseTerm(tok);

	if(node == NULL)
		return NULL;

	if((tok->token & 0xFF) == '=') {
		if(node->type == NODE_RVALUE && ((struct RValue *) node)->type == NULL) {
			node = node->left;
			node->parent = NULL;
		}

		return ParseAssign(tok, node);
	}

	ParseOp(tok, node, &node, PREC_LOR);

	return node;
}

struct Node *ParseStmt(struct Token *tok)
{
	if(tok->token == ';') {
		Lex(tok);
		return ParseStmt(tok);
	}

	if(tok->token == TK_EOF)
		return NULL;

	struct Node *node = NULL;

	do {
		if((node = ParseDecl(tok)) != NULL) break;
		if((node = ParseExpr(tok)) != NULL) break;
	} while(0);

	struct Node *stmt = AllocNode(NODE_STMT);

	if(node != NULL)
		SET_LEFT(stmt, node);
	else
		stmt = NULL;

	if(tok->token != ';')
		LexError("Expected ';', got %s", Tok2Str(tok->token));

	Lex(tok);

	return stmt;
}
