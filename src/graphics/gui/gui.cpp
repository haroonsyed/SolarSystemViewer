#include <GL/glew.h>
#include <ft2build.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <math.h>
#include FT_FREETYPE_H

#define GLFW_INCLUDE_NONE

#include "iostream"

#include "gui.h"
#include "../../config.h"
#include "../shader/shaderManager.h"


Gui::Gui()
{
    //GUI

    ShaderManager* shaderManager = ShaderManager::getInstance();
    shaderManager->bindShader(gui_vertShaderPath, gui_fragShaderPath);

    float vertices[] = {
        -0.68f,  1.00f, 0.0f,
        -0.68f,  0.70f, 0.0f,
        -1.00f,  0.70f, 0.0f,
        -1.00f,  1.00f, 0.0f 
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3   
    };

    glGenVertexArrays(1, &guiVAO);
    glGenBuffers(1, &guiVBO);
    glGenBuffers(1, &guiEBO);
    glBindVertexArray(guiVAO);

    glBindBuffer(GL_ARRAY_BUFFER, guiVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, guiEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float leftBound = -0.99;
    float rightBound = -0.69;
    float topBound = 0.99;
    float bottomBound = 0.71;
    glUniform1fv(glGetUniformLocation(shaderManager->getBoundShader(), "leftBound"), 1, &leftBound);
    glUniform1fv(glGetUniformLocation(shaderManager->getBoundShader(), "rightBound"), 1, &rightBound);
    glUniform1fv(glGetUniformLocation(shaderManager->getBoundShader(), "topBound"), 1, &topBound);
    glUniform1fv(glGetUniformLocation(shaderManager->getBoundShader(), "bottomBound"), 1, &bottomBound);



    //TEXT

    shaderManager->bindShader(text_vertShaderPath, text_fragShaderPath);
    textShaderProgram = shaderManager->getBoundShader();

    FT_Library ft;
    FT_Init_FreeType(&ft);

    // find path to font
    std::string font_name = "../assets/fonts/arial.ttf";
    // load font as face
    FT_Face face;
    FT_New_Face(ft, font_name.c_str(), 0, &face);
    // set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
     {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
         };
        Characters[c] = character;
     }

    glBindTexture(GL_TEXTURE_2D, 0);

    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}


void Gui::render(double frameRate, bool status, std::vector<std::string> info)
{
    if (status) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        std::string frameRateString = std::to_string(frameRate);
        if (frameRate < 100)
        {
            frameRateString = frameRateString.substr(0, 2);
        }
        else
        {
            frameRateString = frameRateString.substr(0, 3);
        }

        glUseProgram(textShaderProgram);
        Config* config = Config::getInstance();
        float width = config->getScreenWidth();
        float height = config->getScreenHeight();
        glm::mat4 projection = glm::mat4(1.0);
        projection = glm::ortho(0.0f, width, 0.0f, height);
        unsigned int projectionloc = glGetUniformLocation(textShaderProgram, "projection");
        glUniformMatrix4fv(projectionloc, 1, GL_FALSE, glm::value_ptr(projection));

        float textScale = 0.4;

        renderText("System: Sol", 20, height - 20, textScale, glm::vec3(0.5, 0.8f, 0.2f));
        renderText("Stars: 1", 20, height - 40, textScale, glm::vec3(0.5, 0.8f, 0.2f));
        renderText("Planets: 9", 20, height - 60, textScale, glm::vec3(0.5, 0.8f, 0.2f));
        renderText("Moons: 171", 20, height - 80, textScale, glm::vec3(0.5, 0.8f, 0.2f));
        renderText("FPS " + frameRateString, (width - 100), height - 30, textScale + 0.1, glm::vec3(0.5, 0.8f, 0.2f));

        float index = 20;

        for (int i = 0; i < info.size(); i++)
        {
            renderText(info[i], 20, height - 850 - index, textScale, glm::vec3(0.5, 0.8f, 0.2f));
            index = index + 20;
        }

        /*ShaderManager* shaderManager = ShaderManager::getInstance();
        shaderManager->bindShader(gui_vertShaderPath, gui_fragShaderPath);
        glBindVertexArray(guiVAO);
        projectionloc = glGetUniformLocation(shaderManager->getBoundShader(), "scale");
        glUniform1f(projectionloc, width / height);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/
        glDisable(GL_BLEND);
    }
}

unsigned int Gui::getShader()
{
    return textShaderProgram;
}


void Gui::renderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
