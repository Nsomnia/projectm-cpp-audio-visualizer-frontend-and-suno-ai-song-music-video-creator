#pragma once

#include <string>
#include <QColor>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

class Config
{
public:
    Config()
    {
        QString userConfigPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        QString userConfigFile = userConfigPath + "/aurora-visualizer/config.ini";
        QString localConfigFile = QCoreApplication::applicationDirPath() + "/config.ini";

        if (QFile::exists(userConfigFile)) {
            m_settings = new QSettings(userConfigFile, QSettings::IniFormat);
        } else if (QFile::exists(localConfigFile)) {
            m_settings = new QSettings(localConfigFile, QSettings::IniFormat);
        } else {
            // Fallback to a dummy QSettings object if no config file is found
            m_settings = new QSettings("dummy", QSettings::IniFormat);
        }
    }

    ~Config()
    {
        delete m_settings;
    }

    QString fontPath() const { return value("Font/path", "/usr/share/fonts/TTF/DejaVuSans.ttf").toString(); }
    int fontSize() const { return value("Font/size", 48).toInt(); }

    int titleLineLengthTarget() const { return value("Title/line_length_target", 20).toInt(); }
    int lyricsLineLengthTarget() const { return value("Lyrics/line_length_target", 40).toInt(); }
    QColor titleColor() const
    {
        return QColor::fromRgbF(
            value("Title/color_r", 1.0).toFloat(),
            value("Title/color_g", 1.0).toFloat(),
            value("Title/color_b", 1.0).toFloat(),
            value("Title/opacity", 1.0).toFloat()
        );
    }

    float animationFadeDuration() const { return value("Animation/fade_duration", 3.0f).toFloat(); }
    float animationBounceDuration() const { return value("Animation/bounce_duration", 10.0f).toFloat(); }
    float animationTargetAlpha() const { return value("Animation/target_alpha", 0.4f).toFloat(); }

private:
    QVariant value(const QString& key, const QVariant& defaultValue) const
    {
        return m_settings->value(key, defaultValue);
    }

    QSettings* m_settings;
};
