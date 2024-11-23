void moe_get_bytes_to_fill(u32* byte_to_lock, u32* bytes_to_write){
    DWORD play_cursor;
	DWORD write_cursor;
	DWORD lock;
	DWORD target_cursor;
	DWORD write;
	DWORD status;
    
	HRESULT hr = IDirectSoundBuffer_GetCurrentPosition(win32_sound_buffer, &play_cursor, &write_cursor);
	if (hr != DS_OK) {
		if (hr == DSERR_BUFFERLOST) {
			hr = IDirectSoundBuffer_Restore(win32_sound_buffer);
		}
		*byte_to_lock = write_cursor;
		*bytes_to_write = ctx.sound.samples_per_second * ctx.sound.bytes_per_sample;
		if (!SUCCEEDED(hr)) {
			return;
		}
	}
    
    ctx.sound.last_cursor = write_cursor;
	
    IDirectSoundBuffer_GetStatus(win32_sound_buffer, &status);
	if (!(status & DSBSTATUS_PLAYING)) {
		hr = IDirectSoundBuffer_Play(win32_sound_buffer, 0, 0, DSBPLAY_LOOPING);
		if (!SUCCEEDED(hr)) {
			return;
		}
	}
    
    lock = (ctx.sound.running_sample_index * ctx.sound.bytes_per_sample) % ctx.sound.size;
    target_cursor = (write_cursor + ctx.sound.samples_per_second * ctx.sound.bytes_per_sample);
    if (target_cursor > (DWORD)ctx.sound.size) target_cursor %= ctx.sound.size;
    target_cursor = (DWORD)MOE_TRUNCATE_TO_NEAREST(target_cursor, 16);
    
    if (lock > target_cursor) {
        write = (ctx.sound.size - lock) + target_cursor;
    } else {
        write = target_cursor - lock;
    }
    
    *byte_to_lock = lock;
    *bytes_to_write = write;
}

typedef struct moe_playing_sound {
    u8 active;
    u8 looping;
    f32 pan;
    u32 position;
    moe_sound* sound;
} moe_playing_sound;

static moe_playing_sound playing_sounds[24];

int next_playing;
moe_playing_sound* play_sound(moe_sound* sound, u8 looping){
    moe_playing_sound *res = playing_sounds + next_playing++;
    
    if(next_playing >= ARRAY_COUNT(playing_sounds)){ next_playing = 0; }
    
    res->active = 1;
    res->looping = looping;
    res->position = 0;
    res->sound = sound;
    
    return res;
}

void update_game_audio(moe_sound_buffer* sound_buffer){
    u16* at = sound_buffer->samples;
    for(int i = 0; i < sound_buffer->samples_to_write; i++){
        u16 left_sample = 0;
        u16 right_sample = 0;
        
        for( moe_playing_sound *sound = playing_sounds; sound != playing_sounds + ARRAY_COUNT(playing_sounds); sound++){
            if(!sound->active) continue;
            
            left_sample += sound->sound->samples[sound->position++];
            right_sample += sound->sound->samples[sound->position++];
            
            if(sound->position >= sound->sound->sample_count){
                if(sound->looping) sound->position = 0;
                else sound->active = 0;
            }
        }
        *at++ = left_sample;
        *at++ = right_sample;
    }
    
}

void win32_fill_sound_buffer(moe_sound_buffer* sound_buffer, DWORD byte_to_lock, DWORD bytes_to_write) {
    
    void *region_1;
    DWORD region_1_size;
    void *region_2;
    DWORD region_2_size;
    
    if (SUCCEEDED(win32_sound_buffer->lpVtbl->Lock(win32_sound_buffer, byte_to_lock, bytes_to_write, &region_1, &region_1_size, &region_2, &region_2_size, 0))) {
        
        u16 *dest = region_1;
        u16 *source = sound_buffer->samples;
        DWORD region_1_sample_count = region_1_size/sound_buffer->bytes_per_sample;
        for (DWORD i = 0; i < region_1_sample_count; i++) {
            
            *dest++ = *source++;
            *dest++ = *source++;
            
            sound_buffer->running_sample_index++;
        }
        
        DWORD region_2_sample_count = region_2_size/sound_buffer->bytes_per_sample;
        dest = region_2;
        for (DWORD i = 0; i < region_2_sample_count; i++) {
            
            *dest++ = *source++;
            *dest++ = *source++;
            
            sound_buffer->running_sample_index++;
        }
        
        if (!SUCCEEDED(win32_sound_buffer->lpVtbl->Unlock(win32_sound_buffer, region_1, region_1_size, region_2, region_2_size))) {
        }
    }
    
}

void moe_update_audio(){
    u32 play_cursor, write_cursor;
    if(FAILED(IDirectSoundBuffer_GetCurrentPosition(win32_sound_buffer, &play_cursor, &write_cursor))){
        logp("Pos failed");
        return;
    }
    
    u32 bytes_to_write;
    u32 byte_to_lock;
    moe_get_bytes_to_fill(&byte_to_lock, &bytes_to_write);
    
    if(bytes_to_write){
        ctx.sound.samples_to_write = bytes_to_write/ctx.sound.bytes_per_sample;
        update_game_audio(&ctx.sound);
        win32_fill_sound_buffer(&ctx.sound, byte_to_lock, bytes_to_write);
    }
    
}

int main( void ){
    arena_init(&ctx.arena);
    moe_os_create_window(800, 600, str_lit("MoeGame"));
    //moe_sound wav = moe_load_audio(&ctx.arena, str_lit("test.wav"));
    //moe_sound wav2 = moe_load_audio(&ctx.arena, str_lit("sound.wav"));
    
    moe_init_audio();
    //play_sound(&wav, 1);
    //play_sound(&wav2, 1);
    
    for (;;)
    {
        moe_os_handle_messages();
        moe_update_audio();
        if(is_down(KEY_ESCAPE)){
            moe_os_destroy_window(&ctx.window);
        }
        
    }
}