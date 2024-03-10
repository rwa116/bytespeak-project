// Playback sounds in real time, allowing multiple simultaneous wave files
// to be mixed together and played without jitter.
#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h> // needed for mixer
#include <thread>

#define DEFAULT_VOLUME 80

#define SAMPLE_RATE 22050
#define NUM_CHANNELS 1
#define SAMPLE_SIZE (sizeof(short)) // bytes per sample

#define MAX_SOUND_BITES 30

typedef struct {
	int numSamples;
	short *pData;
} wavedata_t;

typedef struct {
	// A pointer to a previously allocated sound bite (wavedata_t struct).
	// Note that many different sound-bite slots could share the same pointer
	// (overlapping cymbal crashes, for example)
	wavedata_t *pSound;

	// The offset into the pData of pSound. Indicates how much of the
	// sound has already been played (and hence where to start playing next).
	int location;
} playbackSound_t;

#define AUDIOMIXER_MAX_VOLUME 100

class AudioMixer {
public:
	// init() must be called before any other functions,
	// cleanup() must be called last to stop playback threads and free memory.
	AudioMixer(void);
	~AudioMixer(void);

	// Read the contents of a wave file into the pSound structure. Note that
	// the pData pointer in this structure will be dynamically allocated in
	// readWaveFileIntoMemory(), and is freed by calling freeWaveFileData().
	void readWaveFileIntoMemory(char *fileName, wavedata_t *pSound);
	void freeWaveFileData(wavedata_t *pSound);

	// Queue up another sound bite to play as soon as possible.
	void queueSound(wavedata_t *pSound);

	// Get/set the volume.
	// setVolume() function posted by StackOverflow user "trenki" at:
	// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
	int  getVolume(void);
	void setVolume(int newVolume);
private:
	snd_pcm_t *handle;
	unsigned long playbackBufferSize = 0;
	short *playbackBuffer = NULL;

	playbackSound_t soundBites[MAX_SOUND_BITES];

	// Playback threading
	void* playbackThread(void* arg);
	bool stopping = false;
	std::thread playbackThreadId;
	pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;

	int volume = 0;

	void fillPlaybackBuffer(short *buff, int size);
};

#endif