
/*

How to use:
Loaded_Sound my_sound = load_wav("file.wav"); //calls os_read_entire_file and doesn't free
or
Loaded_Sound my_sound = load_wav_from_memory(string);

file spec: .wav PCM 16-bits per sample, 1 or 2 channels (interleaved rlrlrlr)


*/
struct {
    char *data;
    u64 size;
} typedef String;

static void
os_free_file(String s) {
    VirtualFree(s.data, 0, MEM_RELEASE);
}

static String
os_read_entire_file(char *file_path) {
    String result = {0};
    
    HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle == INVALID_HANDLE_VALUE) {
        CloseHandle(file_handle);
        return result;
    }
    
    DWORD file_size = GetFileSize(file_handle, 0);
    result.size = file_size;
    result.data = VirtualAlloc(0, result.size,
                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    
    DWORD bytes_read;
    if (ReadFile(file_handle, result.data, file_size, &bytes_read, 0) && file_size == bytes_read) {
        // Success;
        
    } else {
        // @Incomplete: error message?
        assert(0);
    }
    
    CloseHandle(file_handle);
    return result;
}

void win32LoadEntireFile(const char* filename, void** data, uint32_t* numBytesRead)
{    
    HANDLE file = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);  
    if((file == INVALID_HANDLE_VALUE)) return;
    
    DWORD fileSize = GetFileSize(file, 0);
    if(!fileSize) return;
    
    *data = HeapAlloc(GetProcessHeap(), 0, fileSize+1);
    if(!*data) return;
    
    if(!ReadFile(file, *data, fileSize, (LPDWORD)numBytesRead, 0))
        return;
    
    CloseHandle(file);
    ((uint8_t*)*data)[fileSize] = 0;
}

////////
// Decls

typedef struct Loaded_Sound{
    int sample_count;
    int channel_count;
    
    u16* samples;
} Loaded_Sound;

#pragma pack(push, 1)
struct {
    
    u32 riff_id;
    u32 size;
    u32 wave_id;
    
} typedef Wav_Header;

#define RIFF_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

enum {
    
    Wav_chunk_id_fmt = RIFF_CODE('f', 'm', 't', ' '),
    Wav_chunk_id_riff = RIFF_CODE('R', 'I', 'F', 'F'),
    Wav_chunk_id_wave = RIFF_CODE('W', 'A', 'V', 'E'),
    Wav_chunk_id_data = RIFF_CODE('d', 'a', 't', 'a'),
};

struct {
    
    u32 id;
    u32 size;
} typedef Wav_Chunk;

struct {
    
    u16 format_tag;
    u16 num_channels;
    u32 samples_per_second;
    u32 avg_bytes_per_second;
    u16 block_align;
    u16 bits_per_sample;
    u16 cb_size;
    u16 valid_bits_sample;
    u32 dw_channel_mask;
    u8 sub_format[16];
    
} typedef Wav_Fmt;
#pragma pack(pop)


//////////
// API


struct {
    
    u8* at;
    u8* stop;
} typedef Riff_Iterator;

inline Riff_Iterator
parse_chunk_at(void *at, void* stop) {
    
    Riff_Iterator iter;
    
    iter.at = (u8*)at;
    iter.stop = (u8*)stop;
    
    return iter;
}

inline u32
is_riff_iterator_valid(Riff_Iterator iter) {
    
    return iter.at < iter.stop;
}

inline Riff_Iterator
next_chunk(Riff_Iterator iter) {
    
    Wav_Chunk *chunk = (Wav_Chunk*)iter.at;
    u32 size = (chunk->size+1) & ~1;
    iter.at += sizeof(Wav_Chunk) + size;
    return iter;
}

inline void *
get_chunk_data(Riff_Iterator iter) {
    
    return iter.at + sizeof(Wav_Chunk);
}

inline u32
get_type(Riff_Iterator iter) {
    
    Wav_Chunk *chunk = (Wav_Chunk*)iter.at;
    return chunk->id;
}

inline u32
get_chunk_data_size(Riff_Iterator iter) {
    
    Wav_Chunk *chunk = (Wav_Chunk*)iter.at;
    return chunk->size;
}

static Loaded_Sound
load_wav_from_memory(u8* s) {
    
    Loaded_Sound result = {0};
    
    Wav_Header *header = (Wav_Header*)s;
    assert(header->riff_id == Wav_chunk_id_riff);
    assert(header->wave_id == Wav_chunk_id_wave);
    
    u32 channel_count = 0;
    u32 sample_data_size = 0;
    u16* sample_data = 0;
    for (Riff_Iterator iter = parse_chunk_at(header+1, (u8*)(header+1) + header->size - 4); is_riff_iterator_valid(iter); iter = next_chunk(iter)) {
        
        switch(get_type(iter)) {
            
            case Wav_chunk_id_fmt: {
                
                Wav_Fmt *fmt = (Wav_Fmt*) get_chunk_data(iter);
                assert(fmt->format_tag == 1); //pcm
                assert(fmt->samples_per_second == 44100);
                assert(fmt->bits_per_sample == 16);
                //assert(fmt->block_align == (sizeof(u16)*fmt->num_channels));
                assert(fmt->num_channels == 1 || fmt->num_channels == 2);
                
                channel_count = fmt->num_channels;
                
            } break;
            
            case Wav_chunk_id_data: {
                
                sample_data = (u16*)get_chunk_data(iter);
                sample_data_size = get_chunk_data_size(iter);
            } break;
            
        }
    }
    
    assert(channel_count && sample_data && sample_data_size);
    result.channel_count = channel_count;
    u32 sample_count = sample_data_size / (channel_count*sizeof(u16));
    log_int(sample_data_size);
    if (channel_count == 1) {
        
        result.samples = sample_data;
        
    } else if (channel_count == 2) {
        
        result.samples = sample_data;
        
        /*
        for (u32 sample_index = 0; sample_index < sample_count; ++sample_index) {
        
            u16 source = sample_data[2*sample_index];
            sample_data[2*sample_index] = sample_data[sample_index];
            sample_data[sample_index] = source;
        }*/
        
    } else {
        
        assert(!"Invalid channel count in Wav file");
    }
    
    result.channel_count = channel_count;
    
    /* //This is for padding. If I were to do that, I have to have more data (because when opening in the pak, I ended up overwriting the next asset)
    for (u32 sample_index = sample_count; sample_index < (sample_count + 8); sample_index++) {
        result.samples[sample_index] = 0; 
    }*/
    
    result.sample_count = sample_count;
    
    return result;
}

static Loaded_Sound
load_wav(char *file_name) {
    String file = os_read_entire_file(file_name);
    return load_wav_from_memory(file.data);
}