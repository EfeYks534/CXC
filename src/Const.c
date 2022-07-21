#include <Front.h>


static uint64 Ceil(uint64 a, uint64 b) { return (a + b - 1) / b; }
static uint64 Min(uint64 a, uint64 b)  { return a < b ? a : b; }
static uint64 Max(uint64 a, uint64 b)  { return a < b ? b : a; }


bool NodeConst(struct Node *node)
{
	if(node->type != NODE_RVALUE && node->type != NODE_EXPR)
		return FALSE;

	if(node->type == NODE_RVALUE && ((struct RValue *) node)->type == NULL)
		return FALSE;

	if(node->type != NODE_EXPR)
		return TRUE;

	struct Expr *expr = (struct Expr *) node;

	if(expr->op == EXPR_CAST || expr->op == EXPR_DREF || expr->op == EXPR_LEA)
		return FALSE;

	if(expr->op < EXPR_PADD)
		return NodeConst(node->left) && NodeConst(node->right);

	return NodeConst(node->left);
}

uint64 NodeConstEval(struct Node *node)
{
	if(node->type == NODE_RVALUE)
		return ((struct RValue *) node)->imm;

	struct Expr *expr = (struct Expr *) node;

	switch(expr->op)
	{
	case EXPR_SHL:   return NodeConstEval(node->left) << NodeConstEval(node->right);
	case EXPR_SHR:   return NodeConstEval(node->left) >> NodeConstEval(node->right);
	case EXPR_MUL:   return NodeConstEval(node->left) *  NodeConstEval(node->right);
	case EXPR_DIV:   return NodeConstEval(node->left) /  NodeConstEval(node->right);
	case EXPR_MOD:   return NodeConstEval(node->left) %  NodeConstEval(node->right);
	case EXPR_ADD:   return NodeConstEval(node->left) +  NodeConstEval(node->right);
	case EXPR_SUB:   return NodeConstEval(node->left) -  NodeConstEval(node->right);
	case EXPR_AND:   return NodeConstEval(node->left) &  NodeConstEval(node->right);
	case EXPR_XOR:   return NodeConstEval(node->left) ^  NodeConstEval(node->right);
	case EXPR_OR:    return NodeConstEval(node->left) |  NodeConstEval(node->right);
	case EXPR_LT:    return NodeConstEval(node->left) < NodeConstEval(node->right);
	case EXPR_LTE:   return NodeConstEval(node->left) <= NodeConstEval(node->right);
	case EXPR_GT:    return NodeConstEval(node->left) > NodeConstEval(node->right);
	case EXPR_GTE:   return NodeConstEval(node->left) >= NodeConstEval(node->right);
	case EXPR_EQU:   return NodeConstEval(node->left) == NodeConstEval(node->right);
	case EXPR_NEQ:   return NodeConstEval(node->left) != NodeConstEval(node->right);
	case EXPR_LAND:  return NodeConstEval(node->left) && NodeConstEval(node->right);
	case EXPR_LOR:   return NodeConstEval(node->left) || NodeConstEval(node->right);
	case EXPR_CEIL:  return Ceil(NodeConstEval(node->left), NodeConstEval(node->right));
	case EXPR_MIN:   return Min(NodeConstEval(node->left),  NodeConstEval(node->right));
	case EXPR_MAX:   return Max(NodeConstEval(node->left),  NodeConstEval(node->right));
	case EXPR_PLUS:  return  NodeConstEval(node->left);
	case EXPR_MINUS: return -NodeConstEval(node->left);
	case EXPR_LNOT:  return !NodeConstEval(node->left);
	case EXPR_NOT:   return ~NodeConstEval(node->left);
	}

	return 0;
}
