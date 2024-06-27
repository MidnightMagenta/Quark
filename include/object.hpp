#ifndef QRK_OBJECT
#define QRK_OBJECT

#include "../glad/glad.h"
#include "../include/color.hpp"
#include "../include/draw.hpp"
#include "../include/qrk_debug.hpp"
#include "../include/texture.hpp"
#include "../include/vector.hpp"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <future>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace qrk {
class Object {
public:
    Object() = delete;
    Object(const std::string &path, bool async = true)
        : loadFinished(false), asyncLoad(async), vertexNumber(NULL) {
        if (!std::filesystem::exists(path)) {
            qrk::Debug::Error("Failed to find file: " + path, 2);
        }
        if (async) {
            _path = path;
            futureData = promisedData.get_future();
            std::thread loadThread(std::bind(&qrk::Object::LoadObjectAsync,
                                             this, std::placeholders::_1,
                                             std::placeholders::_2),
                                   path, std::move(promisedData));
            loadThread.detach();
        } else {
            LoadObject(path);
        }
    }

    ~Object() {}

    bool WaitForLoad() {
        if (asyncLoad) {
            if (!data.empty()) { return false; }
            if (loadFinished) {
                data = futureData.get();
                vertexNumber = (GLsizei) data.size() / 9;
                return false;
            } else
                return true;
        } else
            return false;
    }

    void DeleteData() { std::vector<GLfloat>().swap(data); }

    std::string DumpObjectData(std::string path = "logs");

    std::vector<GLfloat> data;//vertex texture normals
    GLsizei vertexNumber;

private:
    void LoadObjectAsync(std::string path,
                         std::promise<std::vector<GLfloat>> _promisedData);
    void LoadObject(const std::string &path);

    std::atomic_bool loadFinished;
    bool asyncLoad;
    std::string _path;
    std::promise<std::vector<GLfloat>> promisedData;
    std::future<std::vector<GLfloat>> futureData;

    struct ObjectData {
        std::vector<qrk::vec4f> vertices;
        std::vector<qrk::vec2f> textures;
        std::vector<qrk::vec3f> normals;

        std::vector<int> vertexIndeces;
        std::vector<int> textureIndeces;
        std::vector<int> normalIndeces;

        void Clear() {
            std::vector<qrk::vec4f>().swap(vertices);
            std::vector<qrk::vec3f>().swap(normals);
            std::vector<qrk::vec2f>().swap(textures);

            std::vector<int>().swap(vertexIndeces);
            std::vector<int>().swap(textureIndeces);
            std::vector<int>().swap(normalIndeces);
        }
    };
};

class GLObject {
public:
    GLObject() = delete;
    GLObject(qrk::Object &_objectData)
        : objectData(&_objectData), position({0, 0, 0}), rotation({0, 0, 0}),
          scale({1, 1, 1}), color({1.f, 1.f, 1.f, 1.f}), texture(nullptr),
          textured(false) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, objectData->data.size() * sizeof(GLfloat),
                     objectData->data.data(), GL_STATIC_DRAW);
        qrk::mat4 identity = identity4();
        posMatrix = identity;
        rotMatrix = identity;
        sclMatrix = identity;
    }

    void SetPosition(float x, float y, float z) {
        position = qrk::vec3f({x, y, z});
        qrk::mat4 tempMatrix = qrk::CreateTranslationMatrix(x, y, z);
        posMatrix = tempMatrix;
    }
    void SetRotation(float x, float y, float z) {
        rotation = qrk::vec3f({x, y, z});
        qrk::mat4 tempMatrix = qrk::CreateRotationMatrix(x, y, z);
        rotMatrix = tempMatrix;
    }
    void SetScale(float x, float y, float z) {
        scale = qrk::vec3f({x, y, z});
        qrk::mat4 tempMatrix = qrk::CreateScaleMatrix(x, y, z);
        sclMatrix = tempMatrix;
    }
    void SetColor(qrk::Color _color) { color = qrk::ConvertToFloat(_color); }
    void SetColor(qrk::ColorF _color) { color = _color; }
    void SetTexture(qrk::Texture2D &_texture) {
        texture = &_texture;
        textured = true;
    }
    void RemoveTexture() {
        texture = nullptr;
        textured = false;
    }

    qrk::vec3f GetPosition() { return position; }
    qrk::vec3f GetRotation() { return rotation; }
    qrk::vec3f GetScale() { return scale; }

    qrk::obj GetDrawData();

private:
    qrk::Object *objectData;
    qrk::Texture2D *texture;
    bool textured;

    GLuint VAO;
    GLuint VBO;

    qrk::vec3f position;
    qrk::vec3f rotation;
    qrk::vec3f scale;
    qrk::mat4 posMatrix;
    qrk::mat4 rotMatrix;
    qrk::mat4 sclMatrix;
    qrk::ColorF color;
};
}// namespace qrk

#endif// !QRK_OBJECT