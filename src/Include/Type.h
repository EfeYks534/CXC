#pragma once

#include <Types.h>

#define KIND_INT   0x00
#define KIND_UINT  0x01
#define KIND_PTR   0x02
#define KIND_ARR   0x03
#define KIND_STRUC 0x04
#define KIND_FUNC  0x05
#define KIND_VOID  0x06


struct SymStruc;
struct SymFunc;

struct Type
{
	uint32 kind;
	uint32 size;

	union
	{
		struct Type       *ptr;
		struct SymStruc *struc;
		struct SymFunc   *func;
	};
};


struct Type *TypeMake();

struct Type *TypePtrTo(struct Type *type);

struct Type *TypeDerive(struct Type *type);

struct Type *TypeStrucOf(struct SymStruc *struc);

struct Type *TypeFuncOf(struct SymFunc *func);

bool TypeCompare(struct Type *a, struct Type *b);

bool TypeCompareWeak(struct Type *a, struct Type *b);
