#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include <QGuiApplication>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <sstream>

#include <libprojectM/projectM.hpp>
#include <libprojectM/PCM.hpp>

namespace {
    bool contains_ci(const std::string& str, const std::string& substr) {
        auto it = std::search(
            str.begin(), str.end(),
            substr.begin(), substr.end(),
            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
        );
        return (it != str.end());
    }
}

Renderer::Renderer(QWindow *window, bool use_default_preset, const std::string& artist, const std::string& url, const std::string& fontPath, int menuBarHeight)
    : QObject(nullptr), m_window(window),
      m_isInitialized(false), m_use_default_preset(use_default_preset),
      m_artist(artist), m_url(url), m_fontPath(fontPath), m_menuBarHeight(menuBarHeight)
{
    m_context = new QOpenGLContext(this);
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    m_context->setFormat(format);
    m_context->create();

    m_audioEngine = std::make_unique<AudioEngine>();
    m_textRenderer = std::make_unique<TextRenderer>();
    m_songTitleAnimator = std::make_unique<SongTitleAnimator>(this);
    m_pcmBuffer.resize(512 * 2);

    connect(&m_renderTimer, &QTimer::timeout, this, &Renderer::render);
    m_renderTimer.start(16);

    connect(m_window, &QWindow::widthChanged, this, &Renderer::onResize);
    connect(m_window, &QWindow::heightChanged, this, &Renderer::onResize);
}

Renderer::~Renderer() {
    m_textRenderer->cleanup(this);
}

void Renderer::initialize()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    projectM::Settings settings;
    settings.presetURL = "/usr/share/projectM/presets/";
    settings.windowWidth = m_window->width();
    settings.windowHeight = m_window->height();
    settings.shuffleEnabled = !m_use_default_preset;
    settings.presetDuration = 99999;

    m_projectM = std::make_unique<projectM>(settings);
    m_textRenderer->initialize(this, m_fontPath, m_config.fontSize());
}

void Renderer::selectRandomPreset()
{
    if (m_projectM) {
        unsigned int currentIndex = 0;
        if (m_projectM->selectedPresetIndex(currentIndex)) {
            m_presetHistory.push_back(currentIndex);
            if (m_presetHistory.size() > MAX_HISTORY_SIZE) {
                m_presetHistory.pop_front();
            }
        }
        m_projectM->selectRandom(true);
        if (m_logCatcher.checkForErrors()) {
            std::cerr << "Broken preset detected: " << getCurrentPresetPath() << std::endl;
        }
    }
}

void Renderer::reset()
{
    m_isSongPlaying = false;
    m_audioEngine->closeFile();
}

std::string Renderer::intelligentWordWrap(const std::string& text, int lineLengthTarget) {
    std::stringstream ss(text);
    std::string word;
    std::string result;
    std::string current_line;

    while (ss >> word) {
        if (current_line.length() + word.length() + 1 > lineLengthTarget && !current_line.empty()) {
            result += current_line + "\n";
            current_line = "";
        }
        if (!current_line.empty()) {
            current_line += " ";
        }
        current_line += word;
    }
    result += current_line;
    return result;
}

void Renderer::setSongTitle(const std::string& title)
{
    std::string sanitizedTitle = title;
    std::replace(sanitizedTitle.begin(), sanitizedTitle.end(), '_', ' ');
    std::replace(sanitizedTitle.begin(), sanitizedTitle.end(), '-', ' ');

    std::string final_title = intelligentWordWrap(sanitizedTitle, m_config.titleLineLengthTarget());

    if (!m_artist.empty() && !contains_ci(title, m_artist)) {
        final_title += "\n\n" + m_artist;
    }

    m_songTitleAnimator->setText(final_title);
    m_songTitleAnimator->start(m_window->width(), m_window->height() - m_menuBarHeight, m_textRenderer.get(), m_audioEngine.get());
}

void Renderer::selectPreviousPreset()
{
    if (m_projectM && !m_presetHistory.empty()) {
        unsigned int last_preset = m_presetHistory.back();
        m_presetHistory.pop_back();
        m_projectM->selectPreset(last_preset);
    }
}

void Renderer::onResize()
{
    if (m_projectM && m_isInitialized) {
        m_context->makeCurrent(m_window);
        m_projectM->projectM_resetGL(m_window->width(), m_window->height());
        m_songTitleAnimator->setScreenSize(m_window->width(), m_window->height() - m_menuBarHeight);
    }
}

void Renderer::render()
{
    if (!m_context->makeCurrent(m_window)) return;

    if (!m_isInitialized)
    {
        initialize();
        m_isInitialized = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_projectM) {
        size_t framesRead = m_audioEngine->getPCM(m_pcmBuffer.data(), 512);
        if (framesRead > 0)
        {
            if (!m_isSongPlaying) {
                m_isSongPlaying = true;
                if (!m_use_default_preset) {
                    selectRandomPreset();
                }
            }
            m_projectM->pcm()->addPCM16Data(m_pcmBuffer.data(), framesRead);
        }
        m_projectM->renderFrame();
    }

    m_songTitleAnimator->update();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_isSongPlaying) {
        QColor color = m_config.titleColor();
        color.setAlphaF(m_songTitleAnimator->color().x());
        m_textRenderer->renderText(this,
                                 m_songTitleAnimator->text(),
                                 m_songTitleAnimator->position().x(),
                                 m_songTitleAnimator->position().y(),
                                 m_songTitleAnimator->scale(),
                                 QVector3D(color.redF(), color.greenF(), color.blueF()),
                                 m_window->width(),
                                 m_window->height());
    }

    if (!m_url.empty()) {
        m_textRenderer->renderText(this, m_url, 10.0f, m_window->height() - 20.0f, 0.5f, QVector3D(1.0f, 1.0f, 1.0f), m_window->width(), m_window->height());
    }

    // Render lyrics
    if (!m_currentLyricsText.empty()) {
        m_textRenderer->renderText(this,
                                 m_currentLyricsText,
                                 m_window->width() / 2.0f, // Centered horizontally
                                 m_window->height() * 0.8f, // Towards the bottom
                                 0.7f, // Slightly larger scale for lyrics
                                 QVector3D(1.0f, 1.0f, 0.0f), // Yellow color for lyrics
                                 m_window->width(),
                                 m_window->height());
    }

    glDisable(GL_BLEND);

    m_context->swapBuffers(m_window);
}

bool Renderer::isPresetBroken()
{
    if (!m_projectM) return true;
    return !m_projectM->presetPositionValid();
}

std::string Renderer::getCurrentPresetPath()
{
    if (!m_projectM) return "";
    unsigned int index = 0;
    if (m_projectM->selectedPresetIndex(index)) {
        return m_projectM->getPresetURL(index);
    }
    return "";
}

void Renderer::selectNextPreset()
{
    if (m_projectM) {
        m_projectM->selectNext(true);
    }
}

void Renderer::setLyrics(const std::string& lyrics)
{
    m_currentLyricsText = intelligentWordWrap(lyrics, m_config.lyricsLineLengthTarget());
}

void Renderer::clearLyrics()
{
    m_currentLyricsText = "";
}