moe_sound moe_load_audio(moe_arena* arena, moe_string file_name){
    
    u8* file = moe_os_read_binary_file(arena, file_name);
    moe_wav_file* wav = (moe_wav_file*) file;
    
    assert(wav->riff_id == 1179011410);
    assert(wav->wave_id == 1163280727);
    assert(wav->data_id = 1635017060);
    assert(wav->fmt_id = 544501094);
    
    assert(wav->format_code == 1);
    assert(wav->channels == 1 || wav->channels == 2);
    assert(wav->sample_rate == 44100);
    assert(wav->bits_per_sample == 16);
    
    assert(wav->block_align == wav->channels * wav->bits_per_sample / 8);
    assert(wav->byte_rate == wav->sample_rate * wav->block_align);
    log_int(wav->data_chunk_size / (wav->channels * sizeof(u16)));
    
    return (moe_sound){
        .samples = wav->samples,
        .sample_count = wav->data_chunk_size / (wav->channels * sizeof(u16))
    };
}

void moe_init_audio(){
    // ctx.sound needs to be set before init
    ctx.sound.channels = 2;
    ctx.sound.samples_per_second = 44100;
    ctx.sound.bytes_per_sample = ctx.sound.channels * sizeof(u16);
    ctx.sound.size = ctx.sound.samples_per_second * ctx.sound.bytes_per_sample;
    
    ctx.sound.samples = arena_alloc(&ctx.arena, ctx.sound.size);
    
    moe_os_init_sound();
}