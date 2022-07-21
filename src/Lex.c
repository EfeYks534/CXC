#include <Symbol.h>
#include <Lex.h>


static struct LexState *lex = NULL;

static const char *tok_names[] = {
	"end-of-file", "identifier", "string", "integer", "endif"
};


static void LexNL()
{
	lex->line++;
	lex->col = 0;
}

static char LexCurCh() { return lex->text[lex->pos == -1ULL ? 0 : lex->pos]; }

static bool LexEOF() { return LexCurCh() == '\0'; }

static bool LexSOF() { return lex->pos == -1ULL; }

static void LexPutCh()
{
	if(lex->pos == -1ULL)
		return;

	lex->pos--;
	lex->col--;
}

static char LexCh()
{
	if(LexEOF())
		return '\0';

	lex->col++;

	return lex->text[++lex->pos];
}

static char LexStrCh()
{
	if(LexEOF())
		return '\0';

	char ch = LexCh();

	if(ch == '\\') {
		ch = LexCh();

		switch(ch)
		{
		case '"':  return '\"';
		case '\'': return '\'';
		case '0':  return '\0';
		case 'a':  return '\a';
		case 'b':  return '\b';
		case 't':  return '\t';
		case 'n':  return '\n';
		case 'v':  return '\v';
		case 'f':  return '\f';
		case 'r':  return '\r';
		case '\\': return '\\';
		case 'x': {
			ch = LexCh();
			uint8 num = 0;

			for(int i = 0; i < 2; i++) {
				if(!isxdigit(ch)) break;

				ch -= ch <= '9' ? '0' : (ch >= 'a' ? ('a'-10) : ('A'-10));

				num = (num << 4) | ch;

				ch = LexCh();
			}

			LexPutCh();
			return num;
		  }
		default:
			LexError("Invalid character");
		}
	}

	return ch;
}

static void LexComment()
{
	char ch = LexCurCh();

	while(!LexEOF()) {
		if(ch == '\n')
			break;

		ch = LexCh();
	}
}

static char *LexStr(char *end, bool eof)
{
	char *buf   = malloc(256);
	uint64 len  = 0;
	char ch     = LexCurCh();
	uint8 slash = 0;

	while(!LexEOF()) {
		for(int i = 0; end[i]; i++) {
			if(ch == end[i])
				ch = '\0';
		}

		slash = ch == '/' ? slash + 1 : 0;

		if(slash == 2) {
			len--;

			LexCh();
			LexComment();
			break;
		}

		if(ch == '\0')
			break;

		if(len >= 255)
			LexError("String too long");

		buf[len++] = ch;
		ch = LexCh();
	}

	if(eof && LexEOF())
		LexError("Unexpected end-of-file");

	buf[len] = '\0';

	return buf;
}

static char *LexIdent(char *end, bool eof)
{
	char *buf  = malloc(256);
	uint64 len = 0;
	char ch    = LexCurCh();
	while(!LexEOF()) {
		for(uint64 i = 0; end[i]; i++) {
			if(ch == end[i])
				ch = '\0';
		}

		if(ch == '\0')
			break;

		if((len == 0 ? !isalpha(ch) : !isalnum(ch)) && ch != '_')
			LexError("Identifier contains invalid character");

		if(len >= 255)
			LexError("Identifier too long");

		buf[len++] = ch;
		ch = LexCh();
	}

	if(eof && LexEOF())
		LexError("Unexpected end-of-file");

	buf[len] = '\0';

	return buf;
}

static void LexIncludeStr(const char *name, const char *text)
{
	if(text == lex->text)
		LexError("Recursive macros are not allowed");

	struct LexState *inc = AllocZ(sizeof(struct LexState));

	inc->name   = name;
	inc->text   = text;
	inc->pos    = -1ULL;
	inc->state1 = 1;

	LexStatePush(inc);
}


uint64 Lex(struct Token *tok)
{
	char ch = LexCh();

	while(isspace(ch)) {
		if(ch == '\n')
			LexNL();
		ch = LexCh();
	}

	switch(ch)
	{
	case '\0':
		if(lex->parent != NULL) {
			LexStatePop();
			return Lex(tok);
		}

		tok->token = TK_EOF;
		break;
	case '0': {
		if(LexCh() == 'x') {
			tok->token = TK_INT;
			tok->num = 0;

			ch = LexCh();

			while(1) {
				if(isalpha(ch) && !isxdigit(ch)) LexError("Expected hexit");
				if(!isxdigit(ch)) break;

				ch -= ch <= '9' ? '0' : (ch >= 'a' ? ('a'-10) : ('A'-10));

				tok->num = (tok->num << 4) | ch;

				ch = LexCh();
			}

			LexPutCh();
			break;
		}

		LexPutCh();
	case '1'...'9':
		tok->token = TK_INT;
		tok->num   = 0;

		while(1) {
			if(isalpha(ch)) LexError("Expected digit");
			if(!isdigit(ch)) break;

			tok->num = tok->num * 10 + (ch - '0');

			ch = LexCh();
		}

		LexPutCh();
		break;
	  }
	case '_':
	case 'a'...'z':
	case 'A'...'Z': {
		tok->token = TK_IDENT;
		tok->str = Alloc(256);
		tok->len = 0;

		uint32 notmacro = 0;

		while(1) {
			if(!isalnum(ch) && ch != '_')
				break;

			if(tok->len >= 255)
				LexError("Identifier too long");

			if(isalpha(ch) && !isupper(ch))
				notmacro++;

			tok->str[tok->len++] = ch;
			ch = LexCh();
		}

		tok->str[tok->len] = '\0';

		LexPutCh();

		if(!notmacro) {
			struct SymMacro *macro = SymFind(tok->str, STT_MACRO);

			if(macro == NULL)
				break;

			LexIncludeStr(macro->hdr.name, macro->text);

			return Lex(tok);
		}

		tok->hash = Hash(tok->str);

		break;
	  }
	case '"': {
		tok->token = TK_STRING;
		tok->str   = Alloc(256);
		tok->len   = 0;

		while(1) {
			if(LexCh() == '"')
				break;
			else
				LexPutCh();

			if(tok->len >= 255)
				LexError("String literal too long");

			char c = LexStrCh();
			tok->str[tok->len++] = c;
		}

		tok->str[tok->len] = '\0';

		break;
	  }
	case '\'': {
		tok->token = TK_INT;
		tok->num   = 0;

		while(1) {
			if(LexCh() == '\'')
				break;
			else
				LexPutCh();

			uint64 c = LexStrCh();
			tok->num = (tok->num << 8) | c;
		}

		break;
	  }
	case '#': {
		LexCh();

		const char *ppd = &lex->text[lex->pos];

		if(strncmp(ppd, "define ", 7) == 0) {
			lex->pos += 7;
			lex->col += 7;

			char *name = LexIdent(" \n", TRUE);

			char *text = NULL;

			if(LexCurCh() == ' ') {
				LexCh();
				text = LexStr("\n", TRUE);
			}

			struct SymMacro *macro = Alloc(sizeof(struct SymMacro));

			for(int i = 0; name[i]; i++) {
				if(isalpha(name[i]) && !isupper(name[i]))
					LexError("Macro names have to UPPERCASE");
			}

			macro->hdr.name = name;
			macro->text = text;
			macro->line = lex->line;

			if(!lex->state2 && !SymInsert(&macro->hdr, STT_MACRO))
				LexError("Redefined macro '%s'", name);

			LexPutCh();
			return Lex(tok);
		} else if(strncmp(ppd, "undef ", 6) == 0) {
			lex->pos += 6;
			lex->col += 6;

			char *name = LexIdent(" \n", TRUE);

			if(LexCurCh() == '\n')
				LexPutCh();

			if(!lex->state2)
				SymErase(name, STT_MACRO);

			return Lex(tok);
		} else if(strncmp(ppd, "include \"", 9) == 0) {
			lex->pos += 9;
			lex->col += 9;

			char *name = LexStr("\"", FALSE);

			if(!lex->state2) {
				FILE *f = fopen(name, "r");

				if(f == NULL)
					LexError("Can't include file '%s'", name);

				fseek(f, 0, SEEK_END);

				uint64 len = ftell(f);
				char *text = malloc(len + 1);

				fseek(f, 0, SEEK_SET);

				if(fread(text, 1, len, f) != len)
					LexError("Can't read file '%s' for inclusion", name);

				fclose(f);

				text[len] = 0;

				LexIncludeStr(name, text);
			}

			return Lex(tok);
		} else if(strncmp(ppd, "ifdef ", 6) == 0) {
			lex->pos += 6;
			lex->col += 6;

			char *name = LexIdent(" \n", FALSE);

			if(LexCurCh() == '\n')
				LexPutCh();

			struct SymMacro *macro = SymFind(name, STT_MACRO);

			lex->state1++;

			if(macro == NULL && !lex->state2) {
				lex->state2 = lex->state1;

				uint64 token = TK_INT;
				while(token != TK_ENDIF && token != TK_EOF)
					token = Lex(tok);

				if(token == TK_EOF)
					LexError("Expected #endif, got end-of-file");
			}

			return Lex(tok);
		} else if(strncmp(ppd, "else", 4) == 0) {
			lex->pos += 3;
			lex->col += 3;

			if(lex->state2 == lex->state1) {
				lex->state2 = 0;
				return TK_ENDIF;
			} else if(lex->state2) {
				return Lex(tok);
			}

			lex->state2 = lex->state1;

			uint64 token = TK_INT;
			while(token != TK_ENDIF)
				token = Lex(tok);

			return Lex(tok);
		} else if(strncmp(ppd, "ifndef ", 7) == 0) {
			lex->pos += 7;
			lex->col += 7;

			char *name = LexIdent(" \n", FALSE);

			if(LexCurCh() == '\n')
				LexPutCh();

			struct SymMacro *macro = SymFind(name, STT_MACRO);

			lex->state1++;

			if(macro != NULL && !lex->state2) {
				lex->state2 = lex->state1;

				uint64 token = TK_INT;
				while(token != TK_ENDIF && token != TK_EOF)
					token = Lex(tok);

				if(token == TK_EOF)
					LexError("Expected #endif, got end-of-file");
			}

			return Lex(tok);
		} else if(strncmp(ppd, "endif", 5) == 0) {
			lex->pos += 4;
			lex->col += 4;

			if(lex->state1 == 1)
				LexError("Stray #endif");

			if(lex->state2 == lex->state1--) {
				lex->state2 = 0;
				return TK_ENDIF;
			}

			return Lex(tok);
		}

		LexError("Invalid preprocessor directive");
		break;
	  }
	case '/':
		if(LexCh() == '/') {
			LexCh();
			LexComment();
			LexPutCh();

			return Lex(tok);
		} else {
			LexPutCh();
		}
	default: {
		if(!ispunct(ch))
			LexError("Invalid character");

		tok->token = ch;

		ch = LexCh();

		switch((tok->token << 8) | ch)
		{
		case '<<':case '>>':
			tok->token = (tok->token << 8) | ch << 8;

			if(LexCh() == '=')
				tok->token = (tok->token << 8) | '=';
			else
				LexPutCh();
			break;
		case '+=':case '-=':case '*=':case '/=':
		case '%=':case '&=':case '^=':case '|=':
		case '++':case '--':case '%%':case '||':
		case '<=':case '>=':case '==':case '->':
		case '!=':
			tok->token = (tok->token << 8) | ch;
			break;
		default:
			LexPutCh();
			break;
		}

		break;
	  }
	}

	return tok->token;
}

void LexError(const char *fmt, ...)
{
	while(lex->parent != NULL) {
		printf("\x1B[30;1m%s:%u:%u: \x1B[0m", lex->name, lex->line+1, lex->col-1);
		printf("expanded from:\n");
		LexStatePop();
	}

	printf("\x1B[30;1m%s:%u:%u: \x1B[0m", lex->name, lex->line+1, lex->col-1);

	va_list ap;

	va_start(ap, fmt);

	vprintf(fmt, ap);

	va_end(ap);

	putchar('\n');

	uint64 pos = 0;

	for(uint64 line = 0; line < lex->line; line++) {
		for(;lex->text[pos] != '\n' && lex->text[pos];)
			pos++;
		pos++;
	}

	do
		putchar(lex->text[pos]);
	while(lex->text[pos++] != '\n' && !LexEOF());

	exit(1);
}

struct LexState *LexStateGet()
{
	return lex;
}

void LexStateSet(struct LexState *state)
{
	lex = state;
}

void LexStatePush(struct LexState *state)
{
	state->parent = lex;
	lex = state;
}

void LexStatePop()
{
	assert(lex->parent != NULL);

	lex = lex->parent;
}

const char *Tok2Str(uint64 token)
{
	if(token >= TK_EOF && token <= TK_ENDIF)
		return tok_names[token - TK_EOF];

	char *str = Alloc(11);
	str[0]  = '\'';
	str[9]  = '\'';
	str[10] = '\0';

	*(uint64 *) &str[1] = token;

	for(int i = 0; i < 10; i++) {
		if(str[i] == '\0') {
			str[i] = '\'';
			str[i+1] = 0;
			break;
		}
	}

	return str;
}
