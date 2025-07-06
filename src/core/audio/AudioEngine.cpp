#define MA_IMPLEMENTATION
#include "AudioEngine.h"
#include <stdexcept>
#include <iostream>
#include <mutex>

#ifdef MA_ASSERT
#undef MA_ASSERT
#endif
#define MA_ASSERT(expr) \
    if (!(expr)) { \
        throw std::runtime_error("miniaudio assertion failed: " #expr); \
    }

AudioEngine::AudioEngine() {
    m_vizBuffer.resize(VIZ_BUFFER_FRAMES * 2);
}

AudioEngine::~AudioEngine() {
    closeFile();
}

int AudioEngine::getSampleRate() const {
    if (!m_isInitialized) {
        return 44100;
    }
    return m_decoder.outputSampleRate;
}

float AudioEngine::getSongDuration()
{
    if (!m_isInitialized) {
        return 0.0f;
    }
    ma_uint64 length_in_pcm_frames;
    ma_result result = ma_decoder_get_length_in_pcm_frames(&m_decoder, &length_in_pcm_frames);
    if (result != MA_SUCCESS) {
        return 0.0f;
    }
    return static_cast<float>(length_in_pcm_frames) / m_decoder.outputSampleRate;
}

float AudioEngine::getCurrentPosition()
{
    if (!m_isInitialized) {
        return 0.0f;
    }
    ma_uint64 cursor_in_pcm_frames;
    ma_result result = ma_decoder_get_cursor_in_pcm_frames(&m_decoder, &cursor_in_pcm_frames);
    if (result != MA_SUCCESS) {
        return 0.0f;
    }
    return static_cast<float>(cursor_in_pcm_frames) / m_decoder.outputSampleRate;
}

void AudioEngine::closeFile() {
    if (m_isInitialized) {
        ma_device_uninit(&m_device);
        ma_decoder_uninit(&m_decoder);
        m_isInitialized = false;
    }
}

bool AudioEngine::loadFile(const std::string& filePath) {
    closeFile();

    ma_result result = ma_decoder_init_file(filePath.c_str(), nullptr, &m_decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to open audio file: " << filePath << std::endl;
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = m_decoder.outputFormat;
    deviceConfig.playback.channels = m_decoder.outputChannels;
    deviceConfig.sampleRate        = m_decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = this;

    result = ma_device_init(nullptr, &deviceConfig, &m_device);
    if (result != MA_SUCCESS) {
        ma_decoder_uninit(&m_decoder);
        std::cerr << "Failed to open playback device." << std::endl;
        return false;
    }

    m_isInitialized = true;
    return true;
}

void AudioEngine::data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    auto* engine = static_cast<AudioEngine*>(pDevice->pUserData);
    MA_ASSERT(engine != nullptr);

    ma_uint64 framesRead = 0;
    ma_decoder_read_pcm_frames(&engine->m_decoder, pOutput, frameCount, &framesRead);

    engine->processAndStore(static_cast<const short*>(pOutput), framesRead);

    if (framesRead < frameCount) {
        // End of file
    }

    (void)pInput;
}

void AudioEngine::processAndStore(const short* pcmData, ma_uint32 frameCount) {
    std::lock_guard<std::mutex> lock(m_vizBufferMutex);
    size_t writePos = m_vizBufferWritePos.load();
    const size_t channels = 2;

    for (ma_uint32 i = 0; i < frameCount; ++i) {
        m_vizBuffer[writePos * channels] = pcmData[i * channels];
        m_vizBuffer[writePos * channels + 1] = pcmData[i * channels + 1];
        writePos = (writePos + 1) % VIZ_BUFFER_FRAMES;
    }
    m_vizBufferWritePos.store(writePos);
}

size_t AudioEngine::getPCM(short* pcmBuffer, size_t framesToRead) {
    if (!m_isInitialized) return 0;

    std::lock_guard<std::mutex> lock(m_vizBufferMutex);
    const size_t channels = 2;
    const size_t bufferSizeFrames = m_vizBuffer.size() / channels;
    size_t writePos = m_vizBufferWritePos.load();

    for (size_t i = 0; i < framesToRead; ++i) {
        size_t readPos = (writePos - 1 - i + bufferSizeFrames) % bufferSizeFrames;
        pcmBuffer[(framesToRead - 1 - i) * channels] = m_vizBuffer[readPos * channels];
        pcmBuffer[(framesToRead - 1 - i) * channels + 1] = m_vizBuffer[readPos * channels + 1];
    }

    return framesToRead;
}

bool AudioEngine::isPlaying() {
    if (!m_isInitialized) return false;
    return ma_device_is_started(&m_device) == MA_TRUE;
}

void AudioEngine::play() {
    if (!m_isInitialized) return;
    ma_device_start(&m_device);
}

void AudioEngine::pause() {
    if (!m_isInitialized) return;
    ma_device_stop(&m_device);
}