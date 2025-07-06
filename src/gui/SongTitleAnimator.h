#pragma once

#include <QObject>
#include <QTimer>
#include <QVector3D>
#include <string>
#include <QRectF>
#include "core/Config.h"

class TextRenderer;
class AudioEngine;

class SongTitleAnimator : public QObject
{
    Q_OBJECT
public:
    explicit SongTitleAnimator(QObject *parent = nullptr);

    void start(int screenWidth, int screenHeight, TextRenderer* textRenderer, AudioEngine* audioEngine);
    void update();
    void setScreenSize(int width, int height);

    const std::string& text() const { return m_wrappedText; }
    const QVector3D& position() const { return m_position; }
    float scale() const { return m_finalScale; }
    const QVector3D& color() const { return m_color; }

    void setText(const std::string& text);

private:
    enum class State {
        Hidden,
        FadeIn,
        Bouncing,
        FadeToTransparent,
        FadeToOpaque,
        ReturnToCenter
    };

    void setState(State newState);
    std::string wrapText(const std::string& text, int lineLengthTarget);

    State m_state;
    std::string m_text;
    std::string m_wrappedText;
    QVector3D m_position;
    QVector3D m_velocity;
    float m_scale;
    float m_finalScale;
    QVector3D m_color;
    QTimer m_timer;
    float m_totalTime;
    Config m_config;

    int m_screenWidth;
    int m_screenHeight;
    TextRenderer* m_textRenderer;
    AudioEngine* m_audioEngine;
    QRectF m_textBounds;
    float m_songDuration;
};
