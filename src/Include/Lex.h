#pragma once

#include <Types.h>

#define TK_EOF    0xCD000000
#define TK_IDENT  0xCD000001
#define TK_STRING 0xCD000002
#define TK_INT    0xCD000003
#define TK_ENDIF  0xCD000004


struct Token
{
	uint64 token;

	union
	{
		struct
		{
			char   *str;
			uint64  len;
			uint64 hash;
		};

		uint64 num;
	};
};

struct LexState
{
	struct LexState *parent;

	const char *name;
	const char *text;
	uint64       pos;
	uint32      line;
	uint32       col;
	uint16    state1;
	uint16    state2;
};


uint64 Lex(struct Token *tok);

void LexError(const char *fmt, ...);

struct LexState *LexStateGet();

void LexStateSet(struct LexState *state);

void LexStatePush(struct LexState *state);

void LexStatePop();

const char *Tok2Str(uint64 token);
