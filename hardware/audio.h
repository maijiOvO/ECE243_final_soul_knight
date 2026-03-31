#ifndef AUDIO_H
#define AUDIO_H

void Audio_Init(void);
void Audio_Update(void);   // 放在主循环中每帧调用
void Play_Shoot_SFX(void); // 在开火时调用

void Play_Boss_BGM(void);      // 进入 Boss 房时调用
void Stop_Boss_BGM(void);      // 打败 Boss 后调用

#endif