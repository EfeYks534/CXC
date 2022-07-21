#pragma once

#include <Symbol.h>
#include <Types.h>
#include <Type.h>
#include <Lex.h>

#define SET_LEFT(node, child)  do { ((struct Node *) node)->left  = ((struct Node *) child); ((struct Node *) child)->parent = (struct Node *) node; } while(0)
#define SET_RIGHT(node, child) do { ((struct Node *) node)->right = ((struct Node *) child); ((struct Node *) child)->parent = (struct Node *) node; } while(0)

#define NODE_LVALUE 0x00
#define NODE_RVALUE 0x01
#define NODE_EXPR   0x02
#define NODE_STMT   0x03
#define NODE_ASSIGN 0x04

#define EXPR_SHL   0x00
#define EXPR_SHR   0x01
#define EXPR_MUL   0x02
#define EXPR_DIV   0x03
#define EXPR_MOD   0x04
#define EXPR_ADD   0x05
#define EXPR_SUB   0x06
#define EXPR_AND   0x07
#define EXPR_XOR   0x08
#define EXPR_OR    0x09
#define EXPR_LT    0x0A
#define EXPR_LTE   0x0B
#define EXPR_GT    0x0C
#define EXPR_GTE   0x0D
#define EXPR_EQU   0x0E
#define EXPR_NEQ   0x0F
#define EXPR_LAND  0x10
#define EXPR_LOR   0x11
#define EXPR_CEIL  0x12
#define EXPR_MIN   0x13
#define EXPR_MAX   0x14
#define EXPR_BADD  0x15
#define EXPR_BSUB  0x16
#define EXPR_PLUS  0x17
#define EXPR_MINUS 0x18
#define EXPR_LNOT  0x19
#define EXPR_NOT   0x1A
#define EXPR_CAST  0x1B
#define EXPR_DREF  0x1C
#define EXPR_LEA   0x1D
#define EXPR_PADD  0x1E
#define EXPR_PSUB  0x1F

#define PREC_SHIFT 0x00
#define PREC_MUL   0x01
#define PREC_ADD   0x02
#define PREC_AND   0x03
#define PREC_XOR   0x04
#define PREC_OR    0x05
#define PREC_CMP   0x06
#define PREC_EQU   0x07
#define PREC_LAND  0x08
#define PREC_LOR   0x09
#define PREC_NONE  0xFF


struct Node
{
	struct Node *parent;
	struct Node   *left;
	struct Node  *right;
	uint64         type;
};

struct LValue
{
	struct Node   node;
	struct Symbol *sym; // If this is NULL, it's rvalue turned to lvalue
};

struct RValue
{
	struct Node   node;
	struct Type  *type; // If this is NULL, it's lvalue turned to rvalue
	uint64         imm;
};

struct Expr
{
	struct Node  node;
	struct Type *type;
	uint64         op;
};


void *AllocNode(uint64 type);

struct Node *ParseStmt(struct Token *tok);

struct Node *ParseExpr(struct Token *tok);

uint64 NodeConstEval(struct Node *node);

bool NodeConst(struct Node *node);
