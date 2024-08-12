MOE_LIB void 
create_z_array(moe_string str, u64 *z)
{
	u64 n = str.size;
	u64 left, right, k;
	left = right = 0;

	for(u64 i = 1; i<n;i++)
	{
		if(i > right)
		{
			left = right = i;
			while (right < n && str.str[right - left] == str.str[right])
			{
				right++;
			}
			z[i] = right - left;
			right--;
		} else {
			k = i-left;
			if(z[k] < right-i+1)
			{
				z[i] = z[k];
			} else {
				left = i;
				while(right<n && str.str[right-left] == str.str[right])
				{
					right++;
				}
				z[i] = right-left;
				right--;
			}
		}
	}
}

moe_string str_concat(moe_arena* arena, moe_string a, moe_string b)
{
	moe_string final = {0};
	final.size = a.size + b.size;
	final.str = (u8*)arena_alloc(arena, final.size+1);
	memcpy(final.str, a.str, a.size);
	memcpy(final.str + a.size, b.str, b.size);
	final.str[final.size] = (u8)"\0";
	return final;
}

u64 str_count_substr(moe_arena* arena, moe_string text, moe_string pattern)
{
	u64 count = 0;
	moe_string concat = str_concat(arena, pattern, str_lit("$"));
	concat = str_concat(arena, concat, text);
	
	u64 l = concat.size;
	u64 *z = arena_alloc_array(arena, sizeof(u64), l);
	
	create_z_array(concat, z);
	for(u64 i = 0; i < l; ++i)
	{
		if(z[i] == pattern.size)
		{
			count++;
			printf("Pattern matched @ %lld\n", i - pattern.size -1);
		}
	}
	return count;
}

moe_bool str_find_first(moe_arena* arena, moe_string text, moe_string pattern)
{
	moe_string concat = str_concat(arena, pattern, str_lit("$"));
	concat = str_concat(arena, concat, text);
	
	u64 l = concat.size;
	u64 *z = arena_alloc_array(arena, sizeof(u64), l);
	
	create_z_array(concat, z);
	for(u64 i = 0; i < l; ++i)
	{
		if(z[i] == pattern.size)
		{
			printf("Pattern matched @ %lld\n", i - pattern.size -1);
			return 1;
			break;
		}
	}
	return 0;
}
