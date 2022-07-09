#include <Symbol.h>


static struct Symbol *symtab_var[ST_SIZE]   = { NULL };
static struct Symbol *symtab_func[ST_SIZE]  = { NULL };
static struct Symbol *symtab_struc[ST_SIZE] = { NULL };
static struct Symbol *symtab_macro[ST_SIZE] = { NULL };

static struct Symbol **symtabs[4] = {
	symtab_var, symtab_func, symtab_struc, symtab_macro
};


static uint64 Hash(const char *name)
{
	uint64 hash = 0;

	while(*name) {
		hash = (hash << 8) | (hash >> 56);
		hash |= *name;

		name++;
	}

	return hash;
}


static bool _SymInsert(struct Symbol **symtab, struct Symbol *sym)
{
	uint64 key = Hash(sym->name) & (ST_SIZE - 1);

	struct Symbol *victim = symtab[key];

	if(victim == NULL) {
		symtab[key] = sym;
		sym->next = NULL;
		return TRUE;
	}

	while(1) {
		if(strcmp(victim->name, sym->name) == 0)
			return FALSE;

		if(victim->next == NULL)
			break;

		victim = victim->next;
	}

	victim->next = sym;
	sym->next = NULL;
	return TRUE;
}

static struct Symbol *_SymFinds(struct Symbol **symtab, const char *name)
{
	uint64 key = Hash(name) & (ST_SIZE - 1);

	struct Symbol *sym = symtab[key];

	if(sym == NULL)
		return NULL;

	while(1) {
		if(strcmp(sym->name, name) == 0)
			return sym;

		if(sym->next == NULL)
			return NULL;

		sym = sym->next;
	}
}

static void *_SymFind(struct Symbol **symtab, const char *name)
{
	return (void *) _SymFinds(symtab, name);
}

static bool _SymErase(struct Symbol **symtab, const char *name)
{
	uint64 key = Hash(name) & (ST_SIZE - 1);

	struct Symbol *sym = symtab[key];

	if(sym == NULL)
		return FALSE;

	struct Symbol *last = NULL;

	while(1) {
		if(strcmp(sym->name, name) == 0) {
			if(last == NULL)
				symtab[key] = sym->next;
			else
				last->next = sym->next;

			sym->next = NULL;

			return TRUE;
		}

		if(sym->next == NULL)
			return FALSE;

		sym = sym->next;
	}
}

bool SymInsert(struct Symbol *sym, uint8 type)
{
	return _SymInsert(symtabs[type], sym);
}

bool SymErase(const char *name, uint8 type)
{
	return _SymErase(symtabs[type], name);
}

struct Symbol *SymFinds(const char *name, uint8 type)
{
	return _SymFinds(symtabs[type], name);
}

void *SymFind(const char *name, uint8 type)
{
	return _SymFind(symtabs[type], name);
}
