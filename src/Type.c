#include <Symbol.h>
#include <Type.h>


struct Type *TypeMake()
{
	return AllocZ(sizeof(struct Type));
}

struct Type *TypePtrTo(struct Type *type)
{
	struct Type *ptr = TypeMake();

	ptr->kind = KIND_PTR;
	ptr->size = 8;
	ptr->ptr  = type;

	return ptr;
}

struct Type *TypeDerive(struct Type *type)
{
	struct Type *derived = TypeMake();
	*derived = *type;
	return derived;
}

struct Type *TypeStrucOf(struct SymStruc *struc)
{
	struct Type *type = TypeMake();

	type->kind  = KIND_STRUC;
	type->size  = struc->membo[struc->membc - 1] + struc->membt[struc->membc - 1]->size;
	type->struc = struc;

	return type;
}

struct Type *TypeFuncOf(struct SymFunc *func)
{
	return NULL;
}

bool TypeCompare(struct Type *a, struct Type *b)
{
	return FALSE;
}

bool TypeCompareWeak(struct Type *a, struct Type *b)
{
	return FALSE;
}
