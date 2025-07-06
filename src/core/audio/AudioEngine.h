#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include "miniaudio.h"

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool loadFile(const std::string& filePath);
    void closeFile();
    size_t getPCM(short* buffer, size_t frames);
    float getSongDuration();
    float getCurrentPosition();
    bool isPlaying();
    void play();
    void pause();
    int getSampleRate() const;

private:
    static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    void processAndStore(const short* pcmData, ma_uint32 frameCount);

    ma_decoder m_decoder;
    ma_device m_device;
    bool m_isInitialized = false;

    static const size_t VIZ_BUFFER_FRAMES = 1024;
    std::vector<short> m_vizBuffer;
    std::atomic<size_t> m_vizBufferWritePos{0};
    std::mutex m_vizBufferMutex;
};
