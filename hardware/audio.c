#include "audio.h"
#include "../sources/audiodata.h"
#include <stdint.h>

int bgm_index = 0;
int bgm_playing = 0;
int sfx_index = 0;
int sfx_playing = 0;

// DE1-SoC 音频控制器基地址
#define AUDIO_BASE        0xFF203040
#define AUDIO_FIFOSPACE   0xFF203044
#define AUDIO_LEFTDATA    0xFF203048
#define AUDIO_RIGHTDATA   0xFF20304C

void Audio_Init(void) {
    bgm_index = 0;
    sfx_index = 0;
    sfx_playing = 0;
    bgm_playing = 0;
    
}

void Play_Shoot_SFX(void) {
    sfx_index = 0;
    sfx_playing = 1;
}

void Play_Boss_BGM(void) {
    bgm_index = 0; // 从头开始放
    bgm_playing = 1;
}

void Stop_Boss_BGM(void) {
    bgm_playing = 0;
}

void Audio_Update(void) {
    volatile int *fifospace_ptr = (int *)AUDIO_FIFOSPACE;
    volatile int *left_data_ptr = (int *)AUDIO_LEFTDATA;
    volatile int *right_data_ptr = (int *)AUDIO_RIGHTDATA;

    // 读取 FIFO 里还有多少空余空间 (提取 Bit 24-31)
    int fifospace = *fifospace_ptr;
    int words_available = (fifospace >> 24) & 0xFF; 

    // 有多少空位，我们就一口气填多少音频数据进去
    while (words_available > 0) {
        
        int32_t current_sfx = 0;
        int32_t current_bgm = 0;

        // 1. 获取音效采样点 (播完就停止)
        if (sfx_playing) {
            current_sfx = gunshot[sfx_index++];
            if (sfx_index >= gunshot_length) {
                sfx_playing = 0; 
            }
        }

        // 2. 获取 BGM 采样点 (开启时循环播放)
        if (bgm_playing) {
            current_bgm = boss_bgm[bgm_index++];
            if (bgm_index >= bossfight_bgm_length) {
                bgm_index = 0; // 播到尾部自动循环
            }
        }

        // 3. 💥 混音：直接将两个波形相加 💥
        int32_t mixed_sample = current_sfx + current_bgm;

        // 4. 防爆音处理 (Clipping，防止波形溢出 16-bit 范围产生杂音)
        if (mixed_sample > 32767) mixed_sample = 32767;
        if (mixed_sample < -32768) mixed_sample = -32768;

        // 5. 写入双声道硬件寄存器
        *left_data_ptr = mixed_sample << 16;
        *right_data_ptr = mixed_sample << 16;

        words_available--;
    }
}