
/*
 *
 * sfxMixer.c - Sound effect mixer
 *
 */

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspaudio.h>
#include <pspaudiolib.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mp3player.h"

#include "sfxMixer.h"


// ---------------------------------------------------------------------------
// variables
// ---------------------------------------------------------------------------

// did we succeed loading the background mp3?
int soundGotMusic = NO;

// play music?
int soundMusic = YES;

// play SFX?
int soundSFX = YES;

// the SFX channels
struct sfxChannel sfxChannels[4*2];

// the SFXs
extern unsigned char sfx_flip_start[];
extern unsigned char sfx_hit_start[];
extern unsigned char sfx_remove_row_start[];

struct sfx sfxs[] = {
	{ sfx_flip_start, 13824/2 },
	{ sfx_hit_start, 12636/2 },
	{ sfx_remove_row_start, 82200/2 }
};

// ---------------------------------------------------------------------------
// functions
// ---------------------------------------------------------------------------

void soundSFXCallbackMixer(void *buf, unsigned int length, int sfxChannel) {

	short *out = (short *)buf;
	short *left, *right;
	int i, lengthLeft = 0, lengthRight = 0, *positionLeft = NULL, *positionRight = NULL;

	// set up the channel pointers
	i = sfxChannels[sfxChannel].sfxID;
	if (i == SFX_NONE)
		left = NULL;
	else {
		left = (short *)sfxs[i].data;
		lengthLeft = sfxs[i].length;
		positionLeft = &sfxChannels[sfxChannel].position;
	}

	i = sfxChannels[sfxChannel + 1].sfxID;
	if (i == SFX_NONE)
		right = NULL;
	else {
		right = (short *)sfxs[i].data;
		lengthRight = sfxs[i].length;
		positionRight = &sfxChannels[sfxChannel + 1].position;
	}

	// 16bit, stereo, 44100Hz
	i = 0;

	if (left == NULL && right == NULL) {
		// nothing to mix!
		unsigned int *o = (unsigned int *)out;

		while (i < length) {
			o[i++] = 0;
			o[i++] = 0;
		}
	}
	else {
		// mix the SFX
		length <<= 1;
		while (i < length) {
			// left
			if (left == NULL)
				out[i++] = 0;
			else {
				out[i++] = left[(*positionLeft)++];
				if (*positionLeft == lengthLeft) {
					left = NULL;
					sfxChannels[sfxChannel].sfxID = SFX_NONE;
				}
			}

			// right
			if (right == NULL)
				out[i++] = 0;
			else {
				out[i++] = right[(*positionRight)++];
				if (*positionRight == lengthRight) {
					right = NULL;
					sfxChannels[sfxChannel + 1].sfxID = SFX_NONE;
				}
			}
		}
	}
}

void soundSFXCallback0(void *buf, unsigned int length, void *userdata) {
	soundSFXCallbackMixer(buf, length, 0);
}
void soundSFXCallback1(void *buf, unsigned int length, void *userdata) {
	soundSFXCallbackMixer(buf, length, 2);
}
void soundSFXCallback2(void *buf, unsigned int length, void *userdata) {
	soundSFXCallbackMixer(buf, length, 4);
}
void soundSFXCallback3(void *buf, unsigned int length, void *userdata) {
	soundSFXCallbackMixer(buf, length, 6);
}

void soundInit(void) {

	int i;

	pspAudioInit();

	// mp3 playing will take channel 0
	MP3_Init(0);
	if (MP3_Load("music.mp3") == 0)
		soundGotMusic = NO;
	else {
		soundGotMusic = YES;
		MP3_Play();
	}

	// reset the SFX channels
	for (i = 0; i < 4*2; i++)
		sfxChannels[i].sfxID = SFX_NONE;

	// SFX will take channels 1-4 (two mono samples per channel -> 8 concurrent samples can be played)
	pspAudioSetChannelCallback(1, soundSFXCallback0, 0);
	pspAudioSetChannelCallback(2, soundSFXCallback1, 0);
	pspAudioSetChannelCallback(3, soundSFXCallback2, 0);
	pspAudioSetChannelCallback(4, soundSFXCallback3, 0);
}

void soundMusicSet(int status) {

	soundMusic = status;

	if (soundGotMusic == NO)
		return;

	if (soundMusic == YES)
		MP3_Play();
	else
		MP3_Stop();
}

void soundSFXSet(int status) {

	soundSFX = status;
}

void soundPlaySFX(int sfx) {

	int i;

	if (soundSFX == NO)
		return;

	// find a free channel
	for (i = 0; i < 4*2; i++) {
		if (sfxChannels[i].sfxID == SFX_NONE) {
			// reserve the channel
			sfxChannels[i].position = 0;
			sfxChannels[i].sfxID = sfx;
			return;
		}
	}
}
