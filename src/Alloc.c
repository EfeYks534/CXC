#include <Types.h>
#include <Front.h>
#include <sys/mman.h>


static uint8 *data_top = NULL;

static uint64 data_size = 0;


void *Alloc(uint64 size)
{
	size = (size + 0xF) & ~0xF;

	if(data_size < size) {
		data_top  = mmap(NULL, 0x4000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		data_size = 0x4000;
	}

	uint8 *mem = data_top;

	data_top  += size;
	data_size -= size;

	return mem;
}

void *AllocZ(uint64 size)
{
	return memset(Alloc(size), 0, size);
}

void *AllocNode(uint64 type)
{
	struct Node *node;
	switch(type)
	{
	case NODE_LVALUE: node = AllocZ(sizeof(struct LValue)); break;
	case NODE_RVALUE: node = AllocZ(sizeof(struct RValue)); break;
	case NODE_EXPR:   node = AllocZ(sizeof(struct Expr));   break;
	case NODE_STMT:   node = AllocZ(sizeof(struct Node));   break;
	case NODE_ASSIGN: node = AllocZ(sizeof(struct Node));   break;
	default:          assert(0);
	}

	node->type = type;
	return node;
}
