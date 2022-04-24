#include "textureManager.h"

TextureManager* TextureManager::m_instance = nullptr;
unsigned int TextureManager::m_boundTexture = 0;
std::unordered_map<std::string, unsigned int> TextureManager::m_textureMap;

TextureManager::TextureManager() {};

TextureManager* TextureManager::getInstance() {
	if (m_instance == nullptr) {
		m_instance = new TextureManager();
	}
	return m_instance;
}

unsigned int TextureManager::getBoundTexture() {
	return m_boundTexture;
}

std::string TextureManager::getMapFromUniformLocation(int location) {
	switch (location) {
		case 0:
			return "diffuseMap";
		case 1:
			return "normalMap";
		case 2:
			return "specularMap";
		case 3:
			return "emissiveMap";
		default:
			return ("custom"+std::to_string(location));
	}
}

void TextureManager::bindTextures(std::vector<Texture> & staticTextures, std::vector<Texture> dynamicTextures) {

	unsigned int shaderID = ShaderManager::getInstance()->getBoundShader();


	for (int i = 0; i < dynamicTextures.size(); i++) {

		unsigned int existsLoc = glGetUniformLocation(shaderID, (getMapFromUniformLocation(i) + "Exists").c_str());
		unsigned int exists = !dynamicTextures[i].path.empty();
		glUniform1i(existsLoc, exists);

		if (!exists) {
			// No texture, continue to next one
			continue; 
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		// Send the texture to gpu and generate mip maps
		if (staticTextures[i].channels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, dynamicTextures[i].width, dynamicTextures[i].height, 0, GL_RGB, GL_UNSIGNED_BYTE, dynamicTextures[i].data);
		}
		else if (staticTextures[i].channels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dynamicTextures[i].width, dynamicTextures[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dynamicTextures[i].data);
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		// Free memory from cpu
		stbi_image_free(dynamicTextures[i].data);

		// Bind the texture
		glUniform1i(
			glGetUniformLocation(shaderID, getMapFromUniformLocation(i).c_str()),
			(unsigned int)i
		);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
	}


	for (int i = 0; i < staticTextures.size(); i++) {

		unsigned int existsLoc = glGetUniformLocation(shaderID, (getMapFromUniformLocation(i) + "Exists").c_str());
		unsigned int exists = !staticTextures[i].path.empty();
		glUniform1i(existsLoc, exists);

		if (!exists) {
			// No texture, continue to next one
			continue; 
		}
	
		// Textures are cached so they aren't built every time
		if (m_textureMap.count(staticTextures[i].path) == 0) {

			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);

			// set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// Send the texture to gpu and generate mip maps
			if (staticTextures[i].channels == 3)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, staticTextures[i].width, staticTextures[i].height, 0, GL_RGB, GL_UNSIGNED_BYTE, staticTextures[i].data);
			}
			else if(staticTextures[i].channels == 4)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, staticTextures[i].width, staticTextures[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, staticTextures[i].data);
			}
			glGenerateMipmap(GL_TEXTURE_2D);

			// Add to map
			m_textureMap[staticTextures[i].path] = texture;

			// Free memory from cpu
			stbi_image_free(staticTextures[i].data);

		}

		// Bind the texture
		unsigned int texture = m_textureMap.at(staticTextures[i].path);
		glUniform1i(
			glGetUniformLocation(shaderID, getMapFromUniformLocation(i).c_str()),
			(unsigned int)i
		);
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, texture);
	}





}

