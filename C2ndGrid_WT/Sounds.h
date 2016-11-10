#ifndef _SOUNDS_H_
#define _SOUNDS_H_

#ifdef __cplusplus 
extern "C"{
#endif 

#define SOUNDFILE_COUNT							18

#define SOUND_CANNONBALL_SPLASH_1				0
#define SOUND_CARTOON_OH_BOY_2					1
#define SOUND_CARTOON_UH_OH_2					2
#define SOUND_COMPUTER_BLEEP_2					3
#define SOUND_DING_1							4
#define SOUND_DING_SEC							5
#define SOUND_IM00								6
#define SOUND_JETPLANEF16						7
#define SOUND_MAGICAL_SHIMMER_1					8
#define SOUND_MEDIUM_CHEER_1					9
#define SOUND_PLUCK_1							10
#define SOUND_PSST_1							11
#define SOUND_PSST_2							12
#define SOUND_ROBOT_INCOMING_TRANSMISSION_1		13
#define SOUND_SONAR_1							14
#define SOUND_SUSPENSEFUL_NOTE_2				15
#define SOUND_TONE_1							16
#define SOUND_TONE_2							17

void PlayLocalSound(struct InstanceData* id, unsigned int Sound);


#ifdef __cplusplus 
}
#endif

#endif//_SOUNDS_H_