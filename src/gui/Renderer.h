#pragma once

#include <QObject>
#include <QWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include <memory>
#include <deque>

#include "core/audio/AudioEngine.h"
#include "gui/TextRenderer.h"
#include "gui/SongTitleAnimator.h"
#include "core/Config.h"
#include "core/LogCatcher.h"

class projectM;

class Renderer : public QObject, public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit Renderer(QWindow *window, bool use_default_preset, const std::string& artist, const std::string& url, const std::string& fontPath, int menuBarHeight);
    ~Renderer();

    AudioEngine* getAudioEngine() { return m_audioEngine.get(); }
    void reset();
    void setSongTitle(const std::string& title);
    bool isPresetBroken();
    std::string getCurrentPresetPath();
    void selectNextPreset();
    void setLyrics(const std::string& lyrics);
    void clearLyrics();

public slots:
    void render();
    void selectRandomPreset();
    void selectPreviousPreset();
    void onResize();

private:
    std::string m_currentLyricsText;
    void initialize();
    std::string intelligentWordWrap(const std::string& text, int lineLengthTarget);

    QWindow* m_window;
    QOpenGLContext* m_context;
    bool m_isInitialized;

    std::unique_ptr<projectM> m_projectM;
    std::unique_ptr<AudioEngine> m_audioEngine;
    std::unique_ptr<TextRenderer> m_textRenderer;
    std::unique_ptr<SongTitleAnimator> m_songTitleAnimator;
    QTimer m_renderTimer;
    Config m_config;
    LogCatcher m_logCatcher;

    std::vector<short> m_pcmBuffer;
    bool m_use_default_preset;
    bool m_isSongPlaying{false};
    std::deque<unsigned int> m_presetHistory;
    static const size_t MAX_HISTORY_SIZE = 20;

    std::string m_artist;
    std::string m_url;
    std::string m_fontPath;
    int m_menuBarHeight;
};
