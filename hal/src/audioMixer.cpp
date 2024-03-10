// Incomplete implementation of an audio mixer. Search for "REVISIT" to find things
// which are left as incomplete.
// Note: Generates low latency audio on BeagleBone Black; higher latency found on host.
#include <fstream>
#include <iostream>

#include "hal/audioMixer.hpp"

AudioMixer::AudioMixer() {
	setVolume(DEFAULT_VOLUME);

	// Initialize the currently active sound-bites being played
	// REVISIT:- Implement this. Hint: set the pSound pointer to NULL for each
	//     sound bite.
    for (int snd = 0; snd < MAX_SOUND_BITES; snd++) {
        soundBites[snd].pSound = NULL;
        soundBites[snd].location = -1;
    }

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		std::cerr << "Playback open error: " << snd_strerror(err) << std::endl;
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		std::cerr << "Playback open error: " << snd_strerror(err) << std::endl;
		exit(EXIT_FAILURE);
	}

	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = new short[playbackBufferSize];

	// Launch playback thread:
	playbackThreadId = std::thread([this]() {
        this->playbackThread(nullptr);
    });
}


// Client code must call AudioMixer_freeWaveFileData to free dynamically allocated data.
void AudioMixer::readWaveFileIntoMemory(char *fileName, wavedata_t *pSound)
{
	assert(pSound);

	// The PCM data in a wave file starts after the header:
	const int PCM_DATA_OFFSET = 44;

	std::fstream file(fileName, std::ios::in);

	if(!file.is_open()) {
		std::cerr << "ERROR: Unable to open file " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}

	file.seekg(0, std::ios::end);
	std::streampos sizeInBytes = file.tellg();
	file.seekg(0, std::ios::beg);
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	file.seekg(PCM_DATA_OFFSET);
	std::streampos pcmDataSize = sizeInBytes - static_cast<std::streampos>(PCM_DATA_OFFSET);
	pSound->numSamples = pcmDataSize / SAMPLE_SIZE;

	// Allocate space to hold all PCM data
	pSound->pData = new short[pSound->numSamples];
	if (!pSound->pData) {
		std::cerr << "ERROR: Unable to allocate " << sizeInBytes << " bytes for file "
			<< fileName << "." << std::endl;
		exit(EXIT_FAILURE);
	}

	file.read(reinterpret_cast<char*>(pSound->pData), SAMPLE_SIZE * pSound->numSamples);
	if (!file) {
		std::cerr << "ERROR: Unable to read " << pSound->numSamples << " samples from file "
			<< fileName << "." << std::endl;
		exit(EXIT_FAILURE);
	}

	file.close();
}

void AudioMixer::freeWaveFileData(wavedata_t *pSound)
{
	pSound->numSamples = 0;
	delete[] pSound->pData;
	pSound->pData = NULL;
}

void AudioMixer::queueSound(wavedata_t *pSound)
{
	// Check if we are stopping, if we are, cannot queue sound
	if(stopping) {
		return;
	}
	// Ensure we are only being asked to play "good" sounds:
	assert(pSound->numSamples > 0);
	assert(pSound->pData);

	// Insert the sound by searching for an empty sound bite spot
	/*
	 * 1. Since this may be called by other threads, and there is a thread
	 *    processing the soundBites[] array, we must ensure access is threadsafe.
	 */
    pthread_mutex_lock(&audioMutex);
    /*
     * 2. Search through the soundBites[] array looking for a free slot.
    */
    for (int snd = 0; snd < MAX_SOUND_BITES; snd++) {
        /*
        * 3. If a free slot is found, place the new sound file into that slot.
	    *    Note: You are only copying a pointer, not the entire data of the wave file!
        */
        if (soundBites[snd].pSound == NULL) { // Free slot discovered
            soundBites[snd].pSound = pSound;
            soundBites[snd].location = 0;
            pthread_mutex_unlock(&audioMutex);
            return;
        }
    }

    /*
     * 4. After searching through all slots, if no free slot is found then print
	 *    an error message to the console (and likely just return vs asserting/exiting
	 *    because the application most likely doesn't want to crash just for
	 *    not being able to play another wave file.
    */
    perror("No free slot available:");
    pthread_mutex_unlock(&audioMutex);
    return;
}

AudioMixer::~AudioMixer() {
	std::cout << "Stopping audio..." << std::endl;

	for(int ind = 0; ind > MAX_SOUND_BITES; ind++) {
		if(soundBites[ind].pSound == NULL) {
			delete[] soundBites[ind].pSound;
		}
	}

	// Stop the PCM generation thread
	stopping = true;
	playbackThreadId.join();

	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	//  in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	delete[] playbackBuffer;
	playbackBuffer = NULL;

	std::cout << "Done stopping audio..." << std::endl;
	fflush(stdout);
}


int AudioMixer::getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void AudioMixer::setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME) {
		std::cout << "ERROR: Volume must be between 0 and 100." << std::endl;
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *volHandle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";

    snd_mixer_open(&volHandle, 0);
    snd_mixer_attach(volHandle, card);
    snd_mixer_selem_register(volHandle, NULL, NULL);
    snd_mixer_load(volHandle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(volHandle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(volHandle);
}


// Fill the `buff` array with new PCM values to output.
//    `buff`: buffer to fill with new PCM data from sound bites.
//    `size`: the number of values to store into playbackBuffer
void AudioMixer::fillPlaybackBuffer(short *buff, int size)
{
    // Set up a buffer of integers, so we can do calculations without worrying about overflow
	int *overflowBuff = new int[size];
    memset(overflowBuff, 0, sizeof(int)*size);

    memset(buff, 0, sizeof(short)*size);

    pthread_mutex_lock(&audioMutex);

    for (int snd = 0; snd < MAX_SOUND_BITES; snd++) {
        bool soundFinished = false;
        // Check if sound is waiting to be played
        if (soundBites[snd].pSound != NULL) {
            // Check if we have less samples left than requested buffer fill size
            int samplesLeft = soundBites[snd].pSound->numSamples - soundBites[snd].location;
            if(samplesLeft <= size) {
                for (int ind=0; ind<samplesLeft; ind++) {
                    overflowBuff[ind] += soundBites[snd].pSound->pData[soundBites[snd].location + ind];
                }
                soundFinished = true;
            }
            // Otherwise, copy full size amount, also <= above means we dont need to check for finished sound
            else {
                for (int ind=0; ind<size; ind++) {
                    overflowBuff[ind] += soundBites[snd].pSound->pData[soundBites[snd].location + ind];
                }
                soundBites[snd].location += size;
            }
            // If the sound is finished, "free" it
            if (soundFinished) {
                soundBites[snd].pSound = NULL;
                soundBites[snd].location = -1;
            }
        }
    }

    pthread_mutex_unlock(&audioMutex);

    // Now, convert int buffer to short buffer, using trimming for overflows
    for (int ind=0; ind<size; ind++) {
        int val = overflowBuff[ind];
        buff[ind] = val < SHRT_MIN ? SHRT_MIN : val > SHRT_MAX ? SHRT_MAX : val;
    }
    // Free the temporary buffer we made to handle overflows
	delete[] overflowBuff;
}


void* AudioMixer::playbackThread(void* arg)
{
    (void)arg;
	while (!stopping) {
		// Generate next block of audio
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);

		// Output the audio
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);

		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			std::cerr << "AudioMixer: writei() returned " << frames << std::endl;
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			std::cerr << "ERROR: Failed writing audio with snd_pcm_writei(): " 
				<< frames << std::endl;
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && (unsigned long)frames < playbackBufferSize) {
			std::cerr << "Short write (expected " << playbackBufferSize
				<< ", wrote " << frames << ")" << std::endl;
		}
	}

	return NULL;
}