#include <dsound.h>

#define log_int(x) log_info(#x " -> %d", x)

LPDIRECTSOUND DirectSoundObj = 0;

void InitDSound(HWND hwnd){
    
    if(FAILED(DirectSoundCreate(0, &DirectSoundObj, 0))){
        logp("DirectSoundCreate failed");
        return;
    }
    
    if(FAILED(IDirectSound_SetCooperativeLevel(DirectSoundObj, hwnd, DSSCL_PRIORITY))){
        logp("SetCooperativeLevel failed");
        return;
    }
    
    LPDIRECTSOUNDBUFFER PrimaryBuffer = 0;
    DSBUFFERDESC BufferDesc ={0}; 
    
    BufferDesc = (DSBUFFERDESC){
        .dwSize = sizeof(DSBUFFERDESC),
        .dwFlags = DSBCAPS_PRIMARYBUFFER
    };
    
    if(FAILED(IDirectSound_CreateSoundBuffer(DirectSoundObj, &BufferDesc, &PrimaryBuffer, 0)))
    {
        logp("CreateSoundBuffer (p) failed");
        return;
    }
    
    IDirectSound_Release(PrimaryBuffer);
}

void CreateSecondaryBuffer(LPDIRECTSOUNDBUFFER *Buffer, u32 channels, u32 sample_rate, u32 bits_per_sample){
    
    DSBUFFERDESC BufferDesc = {0};
    WAVEFORMATEX WaveFormat = (WAVEFORMATEX){
        .wFormatTag = WAVE_FORMAT_PCM,
        .nChannels = channels,
        .nSamplesPerSec = sample_rate,
        .wBitsPerSample = bits_per_sample,
        .nBlockAlign = channels * (bits_per_sample / 8),
        .nAvgBytesPerSec = sample_rate  * (channels * (bits_per_sample / 8)), // sample_rate * nBlockAlign
        .cbSize = 0,
    };
    
    BufferDesc = (DSBUFFERDESC){
        .dwSize = sizeof(DSBUFFERDESC),
        .dwBufferBytes = sample_rate * sizeof(u16) * channels, // so we have a 1 sec buffer
        .lpwfxFormat = &WaveFormat,
    };
    
    if(FAILED(IDirectSound_CreateSoundBuffer(DirectSoundObj, &BufferDesc, Buffer, 0)))
    {
        logp("CreateSoundBuffer (s) failed");
        return;
    }
    
    // silence the buffer 
    void *region1;
    u32 size1;
    void *region2;
    u32 size2;
    
    if(FAILED(IDirectSoundBuffer_Lock(*Buffer, 0, BufferDesc.dwBufferBytes, &region1, &size1, &region2, &size2, DSBLOCK_ENTIREBUFFER))) {
        logp("CreateSoundBuffer -> Locking failed");
        return;
    }
    
    memset(region1, 0, size1);
    memset(region2, 0, size2);
    
    IDirectSoundBuffer_Unlock(*Buffer, region1, size1, region2, size2);
}
#define CUTE_SOUND_TRUNC(X, Y) ((size_t)(X) & ~((Y) - 1))
#include "wav_importer.h"
static void cs_dsound_get_bytes_to_fill(LPDIRECTSOUNDBUFFER SecondaryBuffer, u32 size, u32 bps,u32 running_index, int* byte_to_lock, int* bytes_to_write, u32* last_cursor)
{
	DWORD play_cursor;
	DWORD write_cursor;
	DWORD lock;
	DWORD target_cursor;
	DWORD write;
	DWORD status;
    
	HRESULT hr = IDirectSoundBuffer_GetCurrentPosition(SecondaryBuffer, &play_cursor, &write_cursor);
	if (hr != DS_OK) {
		if (hr == DSERR_BUFFERLOST) {
			hr = IDirectSoundBuffer_Restore(SecondaryBuffer);
		}
		*byte_to_lock = write_cursor;
		*bytes_to_write = 44100 * bps;
		if (!SUCCEEDED(hr)) {
			return;
		}
	}
    
    *last_cursor = write_cursor;
	
    IDirectSoundBuffer_GetStatus(SecondaryBuffer, &status);
	if (!(status & DSBSTATUS_PLAYING)) {
		hr = IDirectSoundBuffer_Play(SecondaryBuffer, 0, 0, DSBPLAY_LOOPING);
		if (!SUCCEEDED(hr)) {
			return;
		}
	}
    
	lock = (running_index * bps) % size;
	target_cursor = (write_cursor + 44100 * bps);
	if (target_cursor > (DWORD)size) target_cursor %= size;
	target_cursor = (DWORD)CUTE_SOUND_TRUNC(target_cursor, 16);
    
	if (lock > target_cursor) {
		write = (size - lock) + target_cursor;
	} else {
		write = target_cursor - lock;
	}
    
	*byte_to_lock = lock;
	*bytes_to_write = write;
}

int main(void){
    arena_init(&ctx.arena);
    moe_os_create_window(800, 600, str_lit("MoeGame"));
    
    InitDSound(ctx.window.handle);
    
    LPDIRECTSOUNDBUFFER SecondaryBuffer = 0;
    CreateSecondaryBuffer(&SecondaryBuffer, 2, 44100, 16);
    
    u32 status = 0;
    Loaded_Sound laser = load_wav("test.wav");
    // so right now what happens is the fill buffer just copies the first second of the wav and it keeps repeating the first second
    // figure out a way to keep writing to the buffer
    
    //aight we are not handling the audio properly
    u32 running_index = 1;
    u32 bytes_per_sample = sizeof(u16) * 2;
    u32 secondary_buffer_size = 44100 * bytes_per_sample;
    
    u16* samples = malloc(34953660);
    memcpy(samples, laser.samples, 34953660);
    int pos = 0;
    for (;;)
    {
        
        moe_os_handle_messages();
        u32 play_cursor, write_cursor;
        if(FAILED(IDirectSoundBuffer_GetCurrentPosition(SecondaryBuffer, &play_cursor, &write_cursor))){
            logp("Pos failed");
            return;
        }
        /*
        u32 byte_to_lock = running_index*bytes_per_sample % secondary_buffer_size;
        u32 bytes_to_write;
        if(byte_to_lock > play_cursor){
            bytes_to_write = (secondary_buffer_size - byte_to_lock) + play_cursor;
        } else {
            bytes_to_write = play_cursor - byte_to_lock;
        }
        */
        u32 bytes_to_write;
        u32 byte_to_lock;
        u32 last_cursor;
        cs_dsound_get_bytes_to_fill(SecondaryBuffer, secondary_buffer_size, bytes_per_sample, running_index, &byte_to_lock, &bytes_to_write, &last_cursor);
        
        /*
                DWORD safety_bytes = (int)((f32)(44100*bytes_per_sample)*0.166*1);
                safety_bytes -= safety_bytes % bytes_per_sample;
                
                
                DWORD byte_to_lock = (running_index*bytes_per_sample) % secondary_buffer_size;
                
                DWORD expected_bytes_per_tick = (DWORD)((f32)(44100*bytes_per_sample)*0.166);
                expected_bytes_per_tick -= expected_bytes_per_tick % bytes_per_sample;
                DWORD expected_boundary_byte = play_cursor + expected_bytes_per_tick;
                
                DWORD safe_write_buffer = write_cursor;
                if (safe_write_buffer < play_cursor) {
                    safe_write_buffer += secondary_buffer_size;
                } else {
                    safe_write_buffer += safety_bytes;
                }
                
                DWORD target_cursor;
                if (safe_write_buffer < expected_boundary_byte) {
                    target_cursor = expected_boundary_byte + expected_bytes_per_tick;
                } else {
                    target_cursor = write_cursor + expected_bytes_per_tick + safety_bytes;
                }
                target_cursor %= secondary_buffer_size;
                
                DWORD bytes_to_write;
                if (byte_to_lock > target_cursor) {
                    bytes_to_write = secondary_buffer_size - byte_to_lock + target_cursor;
                } else {
                    bytes_to_write = target_cursor - byte_to_lock;
                }*/
        
        u32 samples_to_write = bytes_to_write/bytes_per_sample;
        if(bytes_to_write){
            VOID* region1;
            VOID* region2;
            DWORD size1;
            DWORD size2;
            HRESULT hr;
            hr = IDirectSoundBuffer_Lock(SecondaryBuffer, byte_to_lock, bytes_to_write, &region1, &size1, &region2, &size2, 0);
            
            if(hr == DSERR_BUFFERLOST){
                logp("DSERR_BUFFERLOST");
            } else if(hr == DSERR_INVALIDCALL){
                logp("DSERR_INVALIDCALL");
            } else if(hr == DSERR_INVALIDPARAM){
                logp("DSERR_INVALIDPARAM");
            } else if(hr == DSERR_PRIOLEVELNEEDED){
                logp("DSERR_PRIOLEVELNEEDED");
            }
            
            u16* dest = region1;
            u16* source = samples;
            
            u32 r1_sample_count = size1/bytes_per_sample;
            memcpy(dest, samples, r1_sample_count * 4);
            samples += r1_sample_count * 2; 
            running_index += r1_sample_count;
            
            dest = region2;
            u32 r2_sample_count = size2/bytes_per_sample;
            memcpy(dest, samples, r2_sample_count * 4);
            running_index += r2_sample_count;
            samples += r2_sample_count * 2;
            DWORD status;
            DWORD cursor;
            DWORD junk;
            
            hr = IDirectSoundBuffer_GetCurrentPosition(SecondaryBuffer, &junk, &cursor);
            if (hr != DS_OK) {
                if (hr == DSERR_BUFFERLOST) {
                    IDirectSoundBuffer_Restore(SecondaryBuffer);
                }
                return;
            }
            
            // Prevent mixing thread from sending samples too quickly.
            while (cursor == last_cursor) {
                Sleep(1);
                IDirectSoundBuffer_GetStatus(SecondaryBuffer, &status);
                if ((status & DSBSTATUS_BUFFERLOST)) {
                    IDirectSoundBuffer_Restore(SecondaryBuffer);
                    IDirectSoundBuffer_GetStatus(SecondaryBuffer, &status);
                    if ((status & DSBSTATUS_BUFFERLOST)) {
                        break;
                    }
                }
                
                hr = IDirectSoundBuffer_GetCurrentPosition(SecondaryBuffer, &junk, &cursor);
                if (hr != DS_OK) {
                    // Eek! Not much to do here I guess.
                    return;
                }
            }
            /*
            u16* at = samples;
            
            for(int i =0;i<samples_to_write;i++){
                *at++ = laser.samples[pos];
                *at++ = laser.samples[pos+1];
                pos +=2;
                if(pos > laser.sample_count) pos = 0;
            }*/
            IDirectSoundBuffer_Unlock(SecondaryBuffer, region1, size1, region2, size2);
        }
        
        IDirectSoundBuffer_GetStatus(SecondaryBuffer, &status);
        if(!(status & DSBSTATUS_PLAYING)){
            IDirectSoundBuffer_Play(SecondaryBuffer, 0, 0 ,DSBPLAY_LOOPING);
        }
        
        //IDirectSoundBuffer_GetStatus(SecondaryBuffer, &status);
        
        if(is_down(KEY_ESCAPE)){
            moe_os_destroy_window(&ctx.window);
        }
        
    }
}