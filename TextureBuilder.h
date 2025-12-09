#pragma once
#define GLUT_DISABLE_ATEXIT_HACK
#include <glew.h>      // or "glew.h" depending on your setup
#include <glut.h>      // or <GL/freeglut.h> if you use freeglut
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <stdlib.h>
// ================================================================
// LoadBMP – fully self-contained, no glaux, works 100 %
// ================================================================
static bool loadBMP(GLuint* tex, const char* filename, bool wrap = true)
{
    FILE* f = nullptr;
    if (fopen_s(&f, filename, "rb") || !f) return false;

    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54 || header[0] != 'B' || header[1] != 'M') {
        fclose(f);
        return false;
    }

    unsigned int dataPos = *(unsigned int*)&header[0x0A];
    unsigned int imageSize = *(unsigned int*)&header[0x22];
    unsigned int width = *(unsigned int*)&header[0x12];
    unsigned int height = *(unsigned int*)&header[0x16];

    if (imageSize == 0)    imageSize = width * height * 3;
    if (dataPos == 0)      dataPos = 54;

    unsigned char* data = new unsigned char[imageSize];
    fseek(f, dataPos, SEEK_SET);
    fread(data, 1, imageSize, f);
    fclose(f);

    // Convert BGR → RGB
    for (size_t i = 0; i < imageSize; i += 3) {
        unsigned char tmp = data[i];
        data[i] = data[i + 2];
        data[i + 2] = tmp;
    }

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    delete[] data;
    return true;
}

// ================================================================
// LoadPPM – simple raw PPM (P6) loader – no glaux, no external libs
// This is what 99 % of old OpenGL tutorials actually used
// ================================================================
static bool LoadPPM(GLuint* tex, const char* filename, bool wrap = true)
{
    FILE* f = nullptr;
    if (fopen_s(&f, filename, "rb") || !f) return false;

    char header[256];
    fgets(header, sizeof(header), f);
    if (header[0] != 'P' || header[1] != '6') { fclose(f); return false; }

    // Skip comments and read width, height, maxval
    int width, height, maxval;
    do {
        fgets(header, sizeof(header), f);
    } while (header[0] == '#');
    if (sscanf_s(header, "%d %d", &width, &height) != 2) { fclose(f); return false; }
    do {
        fgets(header, sizeof(header), f);
    } while (header[0] == '#');
    sscanf_s(header, "%d", &maxval);

    if (maxval != 255) { fclose(f); return false; }  // only 8-bit supported

    unsigned char* data = new unsigned char[width * height * 3];
    if (fread(data, 1, width * height * 3, f) != width * height * 3) {
        delete[] data; fclose(f); return false;
    }
    fclose(f);

    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    delete[] data;
    return true;
}