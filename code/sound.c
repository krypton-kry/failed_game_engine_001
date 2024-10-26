#include <dsound.h>

#define log_int(x) log_info("%d", x)

typedef struct WavFile {
    // RIFF Chunk
    u32 riff_id;
    u32 riff_chunk_size;
    u32 wave_id;
    
    // fmt Chunk
    u32 fmt_id;
    u32 fmt_chunk_size;
    u16 format_code;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
    
    // data Chunk
    u32 data_id;
    u32 data_chunk_size;
    u16 samples[]; // actual samples start here
} WavFile;

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
        .dwBufferBytes = WaveFormat.nAvgBytesPerSec * 1, // so we have a 2 sec buffer
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
    
    if(FAILED(IDirectSoundBuffer_Lock(*Buffer, 0, BufferDesc.dwBufferBytes, &region1, &size1, 0, 0, DSBLOCK_ENTIREBUFFER))) {
        logp("CreateSoundBuffer -> Locking failed");
        return;
    }
    
    memset(region1, 0, size1);
    IDirectSoundBuffer_Unlock(*Buffer, region1, size1, 0, 0);
}

void log_wav(WavFile* wav){
    
    log_info("riff_id : %d", wav->riff_id);
    log_info("riff_chunk_size: %d", wav->riff_chunk_size);
    log_info("wave_id: %d", wav->wave_id);
    
    // fmt Chunk
    log_info("fmt_id: %d", wav->fmt_id);
    log_info("fmt_chunk_size: %d", wav->fmt_chunk_size);
    log_info("format_code: %d", wav->format_code);
    log_info("num_channels: %d", wav->num_channels);
    log_info("sample_rate: %d", wav->sample_rate);
    log_info("byte_rate: %d", wav->byte_rate);
    log_info("block_align: %d", wav->block_align);
    log_info("bits_per_sample: %d", wav->bits_per_sample);
    
    // data Chunk
    log_info("data_id: %d", wav->data_id);
    log_info("data_chunk_size: %d", wav->data_chunk_size);
    
}

void FillDSBuffer(LPDIRECTSOUNDBUFFER SecondaryBuffer, WavFile *wav, uint16_t* samples)
{
    u32 w_pos, p_pos;
    IDirectSoundBuffer_GetCurrentPosition(SecondaryBuffer, &p_pos, &w_pos);
    
    void *region1, *region2;
    u32 size1, size2;
    
    if(FAILED(IDirectSoundBuffer_Lock(SecondaryBuffer, w_pos, 88200, &region1, &size1, &region2, &size2, 0))) {
        logp("> Locking failed");
        return;
    }
    
    u16* sample = (u16*)region1;
    
    u32 sample_count = size1 / (wav->bits_per_sample);
    memcpy(sample, samples, sample_count * sizeof(u16));
    IDirectSoundBuffer_Unlock(SecondaryBuffer, region1, size1, region2, size2);
}

int main(void){
    arena_init(&ctx.arena);
    moe_os_create_window(800, 600, str_lit("MoeGame"));
    
    u8* file = moe_os_read_binary_file(&ctx.arena, str_lit("laser1.wav"));
    WavFile* wav = (WavFile*) file;
    uint16_t* wavSamples = wav->samples;
    InitDSound(ctx.window.handle);
    
    LPDIRECTSOUNDBUFFER SecondaryBuffer = 0;
    CreateSecondaryBuffer(&SecondaryBuffer, wav->num_channels, wav->sample_rate, wav->bits_per_sample);
    u32 status = 0;
    
    FillDSBuffer(SecondaryBuffer, wav, wavSamples);
    for (;;)
    {
        moe_os_handle_messages();
        IDirectSoundBuffer_Play(SecondaryBuffer, 0, 0 ,DSBPLAY_LOOPING);
        //IDirectSoundBuffer_GetStatus(SecondaryBuffer, &status);
        
        if(is_down(KEY_ESCAPE)){
            moe_os_destroy_window(&ctx.window);
        }
        
    }
}