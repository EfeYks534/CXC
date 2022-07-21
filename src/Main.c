#include <Symbol.h>
#include <Types.h>
#include <Front.h>
#include <Lex.h>


static const char *expr_name[] = {
	"<<", ">>", "*", "/", "%", "+", "-", "&", "^", "|", "<", "<=", ">", ">=",
	"==", "!=", "&&", "||", "ceil", "min", "max", "++", "--", "+", "-",
	"!", "~", "cast", "*", "&", "++", "--"
};

static struct Type *type_uint64 = &(struct Type) { .kind = KIND_UINT, .size = 8 };
static struct Type *type_uint32 = &(struct Type) { .kind = KIND_UINT, .size = 4 };
static struct Type *type_uint16 = &(struct Type) { .kind = KIND_UINT, .size = 2 };
static struct Type *type_uint8  = &(struct Type) { .kind = KIND_UINT, .size = 1 };
static struct Type *type_int64  = &(struct Type) { .kind = KIND_INT,  .size = 8 };
static struct Type *type_int32  = &(struct Type) { .kind = KIND_INT,  .size = 4 };
static struct Type *type_int16  = &(struct Type) { .kind = KIND_INT,  .size = 2 };
static struct Type *type_int8   = &(struct Type) { .kind = KIND_INT,  .size = 1 };

static void PrintNode(struct Node *node)
{
	if(node == NULL)
		return;

	switch(node->type)
	{
	case NODE_LVALUE: {
		struct LValue *lval = (struct LValue *) node;
		if(lval->sym != NULL) {
			printf("[%s]", lval->sym->name);
		} else {
			putchar('[');
			PrintNode(node->left);
			putchar(']');
		}
		break;
	  }
	case NODE_RVALUE: {
		struct RValue *rval = (struct RValue *) node;
		if(rval->type != NULL) {
			printf("%llu", rval->imm);
		} else {
			putchar('|');
			PrintNode(node->left);
			putchar('|');
		}
		break;
	  }
	case NODE_STMT: {
		PrintNode(node->left);
		printf(";\n");
		PrintNode(node->right);
		break;
	  }
	case NODE_ASSIGN: {
		PrintNode(node->left);
		printf(" = ");
		PrintNode(node->right);
		break;
	  }
	case NODE_EXPR: {
		struct Expr *expr = (struct Expr *) node;

		if(expr->op < EXPR_MIN) {
			putchar('(');
			PrintNode(node->left);
			printf(" %s ", expr_name[expr->op]);
			PrintNode(node->right);
			putchar(')');
			break;
		} else if(expr->op < EXPR_BADD) {
			printf("%s(", expr_name[expr->op]);
			PrintNode(node->left);
			printf(", ");
			PrintNode(node->right);
			putchar(')');
		} else if(expr->op < EXPR_PADD) {
			printf("%s", expr_name[expr->op]);
			PrintNode(node->left);
		} else if(expr->op <= EXPR_PSUB) {
			PrintNode(node->left);
			printf("%s", expr_name[expr->op]);
		}
	  }
	}
}

static struct Type *StrucCreate(const char *name, uint64 membc, const char **membn, struct Type **membt)
{
	struct SymStruc *struc = AllocZ(sizeof(struct SymStruc));
	struc->hdr.name = name;
	struc->membc = membc;

	uint64 offset = 0;

	for(uint64 i = 0; i < membc; i++) {
		struc->membo[i] = offset;

		struc->membn[i] = membn[i];
		struc->membt[i] = membt[i];

		offset += membt[i]->size;
	}

	SymInsert(&struc->hdr, STT_STRUC);

	struct Type *type = TypeMake();

	type->kind  = KIND_STRUC;
	type->size  = offset;
	type->struc = struc;

	return type;
}


int main(int argc, char *argv[])
{
	if(argc != 2)
		return 1;

	FILE *f = fopen(argv[1], "r");

	if(f == NULL)
		return 1;

	struct LexState lex = (struct LexState) { 0 };

	lex.name   = argv[1];
	lex.pos    = -1ULL;
	lex.state1 = 1;
	lex.state2 = 0;

	fseek(f, 0, SEEK_END);

	uint64 len = ftell(f);
	char *text = malloc(len + 1);

	fseek(f, 0, SEEK_SET);

	fread(text, 1, len, f);

	fclose(f);

	lex.text = text;

	LexStateSet(&lex);

	const char *field_names1[] = {"shit", "dumb", "far"};
	struct Type *field_types1[] = {type_int64, type_int32, TypePtrTo(type_uint32)};
	struct Type *struc1 = StrucCreate("DumbShit", 3, field_names1, field_types1);

	const char *field_names2[] = {"cock", "mock", "baz"};
	struct Type *field_types2[] = {type_int64, struc1, TypePtrTo(struc1)};
	struct Type *struc2 = StrucCreate("Cock", 3, field_names2, field_types2);


	struct SymVar *var = AllocZ(sizeof(struct SymVar));

	var->hdr.name = "cock_var";
	var->type     = struc2;
	var->stor     = STOR_STATIC;
	var->addr     = 0x7C0000;

	SymInsert(&var->hdr, STT_VAR);


	struct Token tok;
	Lex(&tok);

	while(1) {
		struct Node *node = ParseStmt(&tok);

		PrintNode(node);

		if(node == NULL)
			break;

		node = node->left;

		if(NodeConst(node))
			printf("%lld\n", NodeConstEval(node));
	}

	return 0;
}
