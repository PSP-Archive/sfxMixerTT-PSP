
#ifndef SOUND_H
#define SOUND_H

// the SFXs
#define SFX_NONE       -1
#define SFX_FLIP        0
#define SFX_HIT         1
#define SFX_REMOVE_ROW  2

struct sfx {
	unsigned char *data; // 16bit, mono, 44100Hz, raw
	int length;          // the sample length
};

struct sfxChannel {
	int sfxID;    // the SFX we are currently playing on this channel
	int position; // the position in the SFX
};

// did we succeed loading the background mp3?
extern int soundGotMusic;

// play music?
extern int soundMusic;

// play SFX?
extern int soundSFX;

void soundInit(void);
void soundPlaySFX(int sfx);
void soundMusicSet(int status);
void soundSFXSet(int status);

#endif
