MOE_LIB u64 
align_memory(u64 mem_location)
{
	u64 align = sizeof(char*);
	ASSERT(align > 0 && (align & (align - 1)) == 0); // Power of two
    
	u64 address = mem_location;
	u64 mod = address & (align - 1);
    
	if(mod != 0)
	{
		address += align - mod;
	}
	return address;
}

void* arena_alloc(moe_arena* arena, u64 size)
{
	char* memory = 0;
	size = align_memory(size);
    
	// commit memory 
	if( arena->alloc_position + size > arena->commit_position )
	{
		u64 commit_size = size;
		commit_size += ARENA_COMMIT_SIZE - 1;
		commit_size -= commit_size % ARENA_COMMIT_SIZE;
        
		if(arena->commit_position >= arena->max)
		{
			assert(0 && "moe_arena is out of memory");
		} else {
			moe_os_commit_memory((char*)arena->memory + arena->commit_position, commit_size);
			arena->commit_position += commit_size;
		}
	}
    
	memory = (char*)arena->memory + arena->alloc_position;
	arena->alloc_position += size;
	return memory;
}

void* arena_alloc_array_sized(moe_arena* arena, u64 elem_size, u64 count)
{
	return arena_alloc(arena, elem_size * count);
}

void arena_init(moe_arena* arena)
{
	// TODO(krypton): ZeroOut Memory
	arena->memory = moe_os_reserve_memory(DEFAULT_MAX_ARENA);
	arena->max = DEFAULT_MAX_ARENA;
	arena->commit_position = 0;
	arena->alloc_position = 0;
}

void 
arena_init_sized(moe_arena* arena, u64 size)
{
	// TODO(krypton): ZeroOut Memory
	arena->memory = moe_os_reserve_memory(size);
	arena->max = size;
	arena->commit_position = 0;
	arena->alloc_position = 0;
}


void arena_free(moe_arena* arena)
{
	moe_os_free_memory(arena->memory);
}

void arena_dealloc(moe_arena* arena, u64 size)
{
	if(size > arena->alloc_position)
	{
		size = arena->alloc_position;
	}
	arena->alloc_position -= size;
}
