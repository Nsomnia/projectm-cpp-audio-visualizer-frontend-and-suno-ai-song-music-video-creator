#include <QApplication>
#include "gui/MainWindow.h"
#include "cxxopts.hpp"
#include <iostream>
#include "core/Config.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Config config;

    cxxopts::Options options("AuroraVisualizer", "A highly-customizable audio visualizer.");
    options.add_options()
        ("d,default-preset", "Start with the default preset and disable shuffle", cxxopts::value<bool>()->default_value("false"))
        ("a,artist", "Set the artist name", cxxopts::value<std::string>()->default_value(""))
        ("u,url", "Set a static URL to display", cxxopts::value<std::string>()->default_value(""))
        ("f,font", "Set the font path", cxxopts::value<std::string>()->default_value(config.fontPath().toStdString()))
        ("font-size", "Set the font size", cxxopts::value<int>()->default_value(std::to_string(config.fontSize())))
        ("title-line-length", "Set the target line length for song titles", cxxopts::value<int>()->default_value(std::to_string(config.titleLineLengthTarget())))
        ("title-color-r", "Set the red component of the title color", cxxopts::value<float>()->default_value(std::to_string(config.titleColor().redF())))
        ("title-color-g", "Set the green component of the title color", cxxopts::value<float>()->default_value(std::to_string(config.titleColor().greenF())))
        ("title-color-b", "Set the blue component of the title color", cxxopts::value<float>()->default_value(std::to_string(config.titleColor().blueF())))
        ("title-opacity", "Set the opacity of the title", cxxopts::value<float>()->default_value(std::to_string(config.titleColor().alphaF())))
        ("fade-duration", "Set the fade duration for animations", cxxopts::value<float>()->default_value(std::to_string(config.animationFadeDuration())))
        ("bounce-duration", "Set the bounce duration for the title animation", cxxopts::value<float>()->default_value(std::to_string(config.animationBounceDuration())))
        ("target-alpha", "Set the target alpha for the title animation", cxxopts::value<float>()->default_value(std::to_string(config.animationTargetAlpha())))
        ("h,help", "Print usage")
    ;

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    bool use_default_preset = result["default-preset"].as<bool>();
    std::string artist = result["artist"].as<std::string>();
    std::string url = result["url"].as<std::string>();
    std::string font_path = result["font"].as<std::string>();

    MainWindow window(use_default_preset, artist, url, font_path, nullptr);
    window.show();
    return app.exec();
}
