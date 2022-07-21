#pragma once

#include <Types.h>
#include <Type.h>

#define ST_SIZE 0x400

#define STT_VAR   0x00
#define STT_FUNC  0x01
#define STT_STRUC 0x02
#define STT_MACRO 0x03

#define STOR_STATIC 0x00
#define STOR_EXTERN 0x01
#define STOR_AUTO   0x02


struct Symbol
{
	const char    *name;
	struct Symbol *next;
	uint8          type;
};

struct SymVar
{
	struct Symbol hdr;
	struct Type *type;
	uint64       addr;
	uint8        stor;
};

struct SymFunc
{
	struct Symbol hdr;
	uint64       argc;
	struct Type *argt[6];
	const char  *argn[6];
	struct Type *rett;
	uint64       addr;
	uint8        stor;
};

struct SymStruc
{
	struct Symbol  hdr;
	uint64       membc;
	uint64       membo[64];
	const char  *membn[64];
	struct Type *membt[64];
};

struct SymMacro
{
	struct Symbol hdr;
	const char  *text;
	uint64       line;
};


uint64 Hash(const char *name);

bool SymInsert(struct Symbol *sym, uint8 type);

bool SymErase(const char *name, uint8 type);

struct Symbol *SymFinds(const char *name, uint8 type);

void *SymFind(const char *name, uint8 type);
