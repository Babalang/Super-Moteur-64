#pragma once
#include <AL/al.h>
#include <AL/alc.h>

#include <string>
#include <fstream>
#include <cstring>
#include <map>

namespace Audio {

    static ALCdevice* device = nullptr;
    static ALCcontext* context = nullptr;
    static bool initialized = false;

    struct SoundInstance {
        ALuint source;
        ALuint buffer;
    };

    static std::map<std::string, ALuint> loadedBuffers;

    static std::unordered_map<std::string, ALuint> playingSources;

    static std::vector<SoundInstance> activeSounds;

    inline void init() {
        if (!alcIsExtensionPresent(device, "ALC_EXT_EFX")) {
            std::cerr << "EFX extension not supported!\n";
        }

        if (initialized) return;
        device = alcOpenDevice(nullptr);
        if (!device) { std::cerr << "Failed to open OpenAL device\n"; return; }
        context = alcCreateContext(device, nullptr);
        if (!context || !alcMakeContextCurrent(context)) {
            std::cerr << "Failed to set OpenAL context\n";
            return;
        }
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        initialized = true;
    }

        inline void stopAllSounds() {
        for (const auto& sound : activeSounds) {
            alSourceStop(sound.source);
            alDeleteSources(1, &sound.source);
            alDeleteBuffers(1, &sound.buffer);
        }
        activeSounds.clear();
    }
    inline void cleanup() {
        stopAllSounds();
        for (auto& pair : loadedBuffers) {
            alDeleteBuffers(1, &pair.second);
        }
        loadedBuffers.clear();

        alcMakeContextCurrent(nullptr);
        if (context) alcDestroyContext(context);
        if (device) alcCloseDevice(device);
        initialized = false;
    }


    inline bool loadWavFile(const std::string& filename, ALuint& outBuffer) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return false;

        char riff[4];
        file.read(riff, 4);
        if (strncmp(riff, "RIFF", 4) != 0) return false;

        file.ignore(4); // chunk size

        char wave[4];
        file.read(wave, 4);
        if (strncmp(wave, "WAVE", 4) != 0) return false;

        char fmt[4];
        file.read(fmt, 4);
        if (strncmp(fmt, "fmt ", 4) != 0) return false;

        uint32_t fmtSize;
        file.read(reinterpret_cast<char*>(&fmtSize), 4);

        uint16_t audioFormat, numChannels, blockAlign, bitsPerSample;
        uint32_t sampleRate, byteRate;

        file.read(reinterpret_cast<char*>(&audioFormat), 2);
        file.read(reinterpret_cast<char*>(&numChannels), 2);
        file.read(reinterpret_cast<char*>(&sampleRate), 4);
        file.read(reinterpret_cast<char*>(&byteRate), 4);
        file.read(reinterpret_cast<char*>(&blockAlign), 2);
        file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

        if (audioFormat != 1 || (numChannels != 1 && numChannels != 2) || bitsPerSample != 16) {
            std::cerr << "Only mono or stereo 16bit PCM WAV supported\n";
            return false;
        }


        if (fmtSize > 16) file.ignore(fmtSize - 16);

        // chercher chunk "data"
        char chunkId[4];
        uint32_t dataSize = 0;
        while (file.read(chunkId, 4)) {
            file.read(reinterpret_cast<char*>(&dataSize), 4);
            if (strncmp(chunkId, "data", 4) == 0) break;
            file.ignore(dataSize);
        }

        if (dataSize == 0) return false;

        std::vector<int16_t> data(dataSize / 2);
        file.read(reinterpret_cast<char*>(data.data()), dataSize);

        alGenBuffers(1, &outBuffer);
        ALenum format;
        if (numChannels == 1) {
            format = AL_FORMAT_MONO16;
        } else if (numChannels == 2) {
            format = AL_FORMAT_STEREO16;
        } else {
            std::cerr << "Unsupported number of channels: " << numChannels << "\n";
            return false;
        }

        alBufferData(outBuffer, format, data.data(), dataSize, sampleRate);


        ALenum err = alGetError();
        if (err != AL_NO_ERROR) {
            std::cerr << "OpenAL error: " << err << std::endl;
            return false;
        }

        return true;
    }


    ALuint getBuffer(const std::string& filePath) {
        auto it = loadedBuffers.find(filePath);
        if (it != loadedBuffers.end()) {
            return it->second; // buffer déjà chargé
        }
        
        ALuint buffer;
        if (!loadWavFile(filePath, buffer)) {
            std::cerr << "Failed to load audio: " << filePath << std::endl;
            return 0;
        }
        loadedBuffers[filePath] = buffer;
        return buffer;
    }

    inline void playAudio(const std::string& filePath, const glm::vec3& position) {
        init();
        
        ALuint buffer = getBuffer(filePath);
        if (buffer == 0) return;
        
        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer);
        alSource3f(source, AL_POSITION, position.x, position.y, position.z);
        alSourcef(source, AL_GAIN, 0.4f);
        alSourcePlay(source);
        
        activeSounds.push_back({ source, buffer  }); 
    }


        inline void update() {
            std::vector<SoundInstance> stillPlaying;
            for (auto& sound : activeSounds) {
                ALint state;
                alGetSourcei(sound.source, AL_SOURCE_STATE, &state);
                if (state == AL_PLAYING) {
                    stillPlaying.push_back(sound);
                } else {
                    alDeleteSources(1, &sound.source);
                    // Ne pas supprimer le buffer ici, il est partagé !
                }
            }
            activeSounds = stillPlaying;
        }


    inline ALuint playBackgroundMusic(const std::string& filePath) {
        init();

        ALuint buffer;
        if (!loadWavFile(filePath, buffer)) {
            std::cerr << "Failed to load background music: " << filePath << std::endl;
            return 0;
        }

        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer);
        alSourcei(source, AL_LOOPING, AL_TRUE);  // 👈 boucle infinie
        alSourcef(source, AL_GAIN, 0.2f);
        alSourcePlay(source);

        activeSounds.push_back({ source, buffer });
        return source;
    }

    inline void switchBackgroundMusic(const std::string& filePath) {
        // On cherche la source de la musique de fond actuelle (en supposant qu'il y en a une seule)
        for (auto it = activeSounds.begin(); it != activeSounds.end(); ++it) {
            // Ici on peut supposer que la musique de fond est en boucle (AL_LOOPING == AL_TRUE)
            ALint looping = 0;
            alGetSourcei(it->source, AL_LOOPING, &looping);
            if (looping == AL_TRUE) {
                alSourceStop(it->source);
                alDeleteSources(1, &it->source);
                // On ne supprime pas le buffer, il est dans loadedBuffers pour réutilisation
                activeSounds.erase(it);
                break;
            }
        }

        // Puis on lance la nouvelle musique de fond
        playBackgroundMusic(filePath);
    }

    inline void playAudioOnce(const std::string& filePath, const glm::vec3& position) {
        auto it = playingSources.find(filePath);
        if (it != playingSources.end()) {
            ALint state;
            alGetSourcei(it->second, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING) {
                // Le son joue déjà, ne rien faire
                return;
            } else {
                // Son fini, on peut supprimer la source et l'enlever de la map
                alDeleteSources(1, &it->second);
                playingSources.erase(it);
            }
        }
        // Jouer le son normalement et mémoriser la source
        ALuint buffer = getBuffer(filePath);
        if (buffer == 0) return;

        ALuint source;
        alGenSources(1, &source);
        alSourcei(source, AL_BUFFER, buffer);
        alSource3f(source, AL_POSITION, position.x, position.y, position.z);
        alSourcef(source, AL_GAIN, 0.4f);
        alSourcePlay(source);

        playingSources[filePath] = source;
    }

    void stopAudio(const std::string& filePath) {
        auto it = playingSources.find(filePath);
        if (it != playingSources.end()) {
            ALuint source = it->second;
            alSourceStop(source);       // Arrête la lecture
            alDeleteSources(1, &source); // Supprime la source OpenAL
            playingSources.erase(it);    // Enlève de la map
        }
    }



}
