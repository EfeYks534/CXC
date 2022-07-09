#include <Symbol.h>
#include <Types.h>
#include <Lex.h>


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

	struct Token tok;

	while(1) {
		Lex(&tok);

		if(tok.token == TK_INT)
			printf("%s %llX %llu\n", Tok2Str(tok.token), tok.num, tok.num);
		else if(tok.token == TK_IDENT)
			printf("%s %s %llu\n", Tok2Str(tok.token), tok.str, tok.len);
		else if(tok.token == TK_STRING)
			printf("%s %s %llu\n", Tok2Str(tok.token), tok.str, tok.len);
		else
			printf("%s\n", Tok2Str(tok.token));

		if(tok.token == TK_EOF)
			break;
	}

	return 0;
}
