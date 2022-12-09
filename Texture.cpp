#include <iostream>
#include "Texture.h"
#include <cstring>
#include <opencv2/opencv.hpp>
using namespace std;

Texture::Texture(const std::string& path)
:m_filePath(path), m_localBuffer(nullptr),
m_Width(0), m_Height(0), m_BPP(0)
{
    cv::Mat texture = cv::imread(path, cv::IMREAD_UNCHANGED);
    cv::cvtColor(texture, texture, cv::COLOR_BGR2RGBA);
    m_Width = texture.cols;
    m_Height = texture.rows;
    m_localBuffer = static_cast<unsigned char*>(std::malloc(m_Height * texture.step));
    memcpy(m_localBuffer, texture.data, m_Height * texture.step);
    
	glGenTextures(1, &m_textureID); 
	glBindTexture(GL_TEXTURE_2D, m_textureID );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_localBuffer);
	glBindTexture(GL_TEXTURE_2D, 0);

    free(m_localBuffer);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_textureID);
}

void Texture::Bind(int slot)const 
{
	glActiveTexture(slot);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::Unbind() const 
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
