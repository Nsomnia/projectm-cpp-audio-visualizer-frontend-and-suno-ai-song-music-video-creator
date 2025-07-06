#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <string>
#include <map>
#include <QRectF>

class Renderer;

class TextRenderer
{
public:
    TextRenderer();
    ~TextRenderer();

    void initialize(Renderer* renderer, const std::string& fontPath, int fontSize);
    void renderText(Renderer* renderer, const std::string& text, float x, float y, float scale, const QVector3D& color, float windowWidth, float windowHeight);
    QRectF getTextBounds(const std::string& text, float scale);
    void cleanup(Renderer* renderer);
private:
    struct Character {
        unsigned int textureID;
        QVector2D    size;
        QVector2D    bearing;
        unsigned int advance;
    };

    void loadFont(Renderer* renderer, const std::string& fontPath, int fontSize);

    FT_Library m_ft;
    FT_Face m_face;
    std::map<char, Character> m_characters;

    QOpenGLShaderProgram* m_shaderProgram;
    unsigned int m_vao, m_vbo;
};
