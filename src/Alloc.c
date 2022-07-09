#include <Types.h>
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
