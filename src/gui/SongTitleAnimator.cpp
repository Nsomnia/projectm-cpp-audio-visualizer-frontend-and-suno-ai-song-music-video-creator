#include "SongTitleAnimator.h"
#include "TextRenderer.h"
#include "core/audio/AudioEngine.h"
#include <QRandomGenerator>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <sstream>

SongTitleAnimator::SongTitleAnimator(QObject *parent)
    : QObject(parent), m_state(State::Hidden), m_scale(1.0f), m_finalScale(1.0f), m_totalTime(0.0f),
      m_screenWidth(800), m_screenHeight(600),
      m_textRenderer(nullptr), m_audioEngine(nullptr), m_songDuration(0.0f)
{
    connect(&m_timer, &QTimer::timeout, this, &SongTitleAnimator::update);
}

void SongTitleAnimator::start(int screenWidth, int screenHeight, TextRenderer* textRenderer, AudioEngine* audioEngine)
{
    m_textRenderer = textRenderer;
    m_audioEngine = audioEngine;
    m_songDuration = m_audioEngine->getSongDuration();
    setScreenSize(screenWidth, screenHeight);

    m_textBounds = m_textRenderer->getTextBounds(m_wrappedText, m_scale);

    m_position = QVector3D(m_screenWidth / 2.0f - m_textBounds.width() / 2.0f,
                           m_screenHeight / 2.0f + m_textBounds.height() / 2.0f,
                           0.0f);

    m_velocity = QVector3D(QRandomGenerator::global()->bounded(1.0f) - 0.5f,
                           QRandomGenerator::global()->bounded(1.0f) - 0.5f,
                           0.0f);
    m_velocity.normalize();
    m_velocity *= 2.0f;

    setState(State::FadeIn);
    m_timer.start(16);
}

void SongTitleAnimator::setState(State newState)
{
    m_state = newState;
}

void SongTitleAnimator::update()
{
    if (!m_audioEngine || !m_textRenderer) return;

    float currentTime = m_audioEngine->getCurrentPosition();
    float remainingTime = m_songDuration - currentTime;
    m_totalTime += 0.016f;

    float breath = sin(m_totalTime * 2.0f) * 0.025f + 1.0f;
    m_finalScale = m_scale * breath;

    auto move = [&](){
        m_position += m_velocity;

        float textWidth = m_textBounds.width() * m_finalScale;
        float textHeight = m_textBounds.height() * m_finalScale;

        if (m_position.x() < 0) {
            m_velocity.setX(std::abs(m_velocity.x()));
            m_position.setX(0);
        } else if (m_position.x() + textWidth > m_screenWidth) {
            m_velocity.setX(-std::abs(m_velocity.x()));
            m_position.setX(m_screenWidth - textWidth);
        }

        if (m_position.y() - textHeight < 0) {
            m_velocity.setY(std::abs(m_velocity.y()));
            m_position.setY(textHeight);
        } else if (m_position.y() > m_screenHeight) {
            m_velocity.setY(-std::abs(m_velocity.y()));
            m_position.setY(m_screenHeight);
        }
    };

    float fadeDuration = m_config.animationFadeDuration();
    float bounceDuration = m_config.animationBounceDuration();
    float targetAlpha = m_config.animationTargetAlpha();
    QColor titleColor = m_config.titleColor();

    switch (m_state)
    {
        case State::FadeIn:
            move();
            if (currentTime <= fadeDuration) {
                float alpha = currentTime / fadeDuration;
                m_color = QVector3D(titleColor.redF(), titleColor.greenF(), titleColor.blueF()) * alpha;
            } else {
                m_color = QVector3D(titleColor.redF(), titleColor.greenF(), titleColor.blueF());
                setState(State::Bouncing);
            }
            break;

        case State::Bouncing:
            move();
            if (currentTime > bounceDuration) {
                setState(State::FadeToTransparent);
            }
            break;

        case State::FadeToTransparent:
            move();
            {
                float time_in_state = currentTime - bounceDuration;
                if (time_in_state <= fadeDuration) {
                    float alpha = titleColor.alphaF() - (titleColor.alphaF() - targetAlpha) * (time_in_state / fadeDuration);
                    m_color = QVector3D(titleColor.redF(), titleColor.greenF(), titleColor.blueF()) * alpha;
                } else {
                    m_color = QVector3D(titleColor.redF(), titleColor.greenF(), titleColor.blueF()) * targetAlpha;
                    setState(State::Bouncing);
                }
            }
            if (remainingTime <= fadeDuration + bounceDuration) {
                 setState(State::FadeToOpaque);
            }
            break;

        case State::FadeToOpaque:
            move();
            {
                float time_until_end_fade = remainingTime - fadeDuration;
                if (time_until_end_fade <= fadeDuration) {
                    float alpha = targetAlpha + (titleColor.alphaF() - targetAlpha) * (1.0f - time_until_end_fade / fadeDuration);
                    m_color = QVector3D(titleColor.redF(), titleColor.greenF(), titleColor.blueF()) * alpha;
                } else {
                     m_color = QVector3D(titleColor.redF(), titleColor.greenF(), titleColor.blueF());
                     setState(State::ReturnToCenter);
                }
            }
            break;

        case State::ReturnToCenter:
            {
                QVector3D center(m_screenWidth / 2.0f - m_textBounds.width() / 2.0f,
                                 m_screenHeight / 2.0f + m_textBounds.height() / 2.0f,
                                 0.0f);
                if (remainingTime <= fadeDuration) {
                    float t = 1.0f - (remainingTime / fadeDuration);
                    m_position = m_position * (1.0f - t) + center * t;
                } else {
                    m_position = center;
                    setState(State::Hidden);
                    m_timer.stop();
                }
            }
            break;

        case State::Hidden:
            break;
    }
}

void SongTitleAnimator::setScreenSize(int width, int height)
{
    m_screenWidth = width;
    m_screenHeight = height;
}

void SongTitleAnimator::setText(const std::string& text)
{
    m_text = text;
    m_wrappedText = wrapText(text, m_config.titleLineLengthTarget());
    if (m_textRenderer) {
        m_textBounds = m_textRenderer->getTextBounds(m_wrappedText, m_scale);
    }
}

std::string SongTitleAnimator::wrapText(const std::string& text, int lineLengthTarget)
{
    std::stringstream ss(text);
    std::string word;
    std::string line;
    std::string wrappedText;

    while (ss >> word) {
        if (line.length() + word.length() + 1 > lineLengthTarget) {
            wrappedText += line + '\n';
            line = "";
        }
        if (!line.empty()) {
            line += " ";
        }
        line += word;
    }
    wrappedText += line;

    return wrappedText;
}
