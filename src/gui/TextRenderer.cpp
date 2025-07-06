#include "TextRenderer.h"
#include "Renderer.h" 
#include <iostream>
#include <QVector2D>
#include <QMatrix4x4>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex;
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

TextRenderer::TextRenderer() : m_shaderProgram(nullptr), m_vao(0), m_vbo(0) {
}

TextRenderer::~TextRenderer()
{
    if (m_shaderProgram) {
        delete m_shaderProgram;
    }
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_ft);
}

void TextRenderer::cleanup(Renderer* renderer)
{
    renderer->glDeleteVertexArrays(1, &m_vao);
    renderer->glDeleteBuffers(1, &m_vbo);
}

void TextRenderer::initialize(Renderer* renderer, const std::string& fontPath, int fontSize)
{
    if (FT_Init_FreeType(&m_ft)) {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    loadFont(renderer, fontPath, fontSize);

    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_shaderProgram->link();

    renderer->glGenVertexArrays(1, &m_vao);
    renderer->glGenBuffers(1, &m_vbo);
    renderer->glBindVertexArray(m_vao);
    renderer->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    renderer->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    renderer->glEnableVertexAttribArray(0);
    renderer->glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    renderer->glBindBuffer(GL_ARRAY_BUFFER, 0);
    renderer->glBindVertexArray(0);
}

void TextRenderer::loadFont(Renderer* renderer, const std::string& fontPath, int fontSize)
{
    if (FT_New_Face(m_ft, fontPath.c_str(), 0, &m_face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(m_face, 0, fontSize);
    renderer->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(m_face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        unsigned int texture;
        renderer->glGenTextures(1, &texture);
        renderer->glBindTexture(GL_TEXTURE_2D, texture);
        renderer->glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            m_face->glyph->bitmap.width,
            m_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            m_face->glyph->bitmap.buffer
        );
        renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        renderer->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            QVector2D(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows),
            QVector2D(m_face->glyph->bitmap_left, m_face->glyph->bitmap_top),
            static_cast<unsigned int>(m_face->glyph->advance.x)
        };
        m_characters.insert(std::pair<char, Character>(c, character));
    }
    renderer->glBindTexture(GL_TEXTURE_2D, 0);
}

QRectF TextRenderer::getTextBounds(const std::string& text, float scale)
{
    float x = 0;
    float y = 0;
    float minX = 0, maxX = 0, minY = 0, maxY = 0;
    float line_height = (m_face->size->metrics.height >> 6) * scale;

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        if (*c == '\n') {
            y -= line_height;
            x = 0;
            continue;
        }

        Character ch = m_characters[*c];
        float xpos = x + ch.bearing.x() * scale;
        float ypos = y - (ch.size.y() - ch.bearing.y()) * scale;
        float w = ch.size.x() * scale;
        float h = ch.size.y() * scale;

        if (xpos < minX) minX = xpos;
        if (xpos + w > maxX) maxX = xpos + w;
        if (ypos < minY) minY = ypos;
        if (ypos + h > maxY) maxY = ypos + h;

        x += (ch.advance >> 6) * scale;
    }

    return QRectF(minX, minY, maxX - minX, maxY - minY);
}

void TextRenderer::renderText(Renderer* renderer, const std::string& text, float x, float y, float scale, const QVector3D& color, float windowWidth, float windowHeight)
{
    QMatrix4x4 projection;
    projection.ortho(0.0f, windowWidth, 0.0f, windowHeight, -1.0f, 1.0f);

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("projection", projection);
    m_shaderProgram->setUniformValue("textColor", color);
    renderer->glActiveTexture(GL_TEXTURE0);
    renderer->glBindVertexArray(m_vao);

    float initialX = x;
    float line_height = (m_face->size->metrics.height >> 6) * scale;

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        if (*c == '\n') {
            y -= line_height;
            x = initialX;
            continue;
        }

        Character ch = m_characters[*c];

        float xpos = x + ch.bearing.x() * scale;
        float ypos = y - (ch.size.y() - ch.bearing.y()) * scale;

        float w = ch.size.x() * scale;
        float h = ch.size.y() * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        renderer->glBindTexture(GL_TEXTURE_2D, ch.textureID);
        renderer->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        renderer->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        renderer->glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderer->glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance >> 6) * scale;
    }
    renderer->glBindVertexArray(0);
    renderer->glBindTexture(GL_TEXTURE_2D, 0);
    m_shaderProgram->release();
}
