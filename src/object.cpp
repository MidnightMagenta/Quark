#include "../include/object.hpp"

void qrk::Object::LoadObjectAsync(
        std::string path, std::promise<std::vector<GLfloat>> _promisedData) {
    //get data from file
    std::ifstream objFile(path);
    if (!objFile.is_open()) {
        qrk::Debug::Error("Failed to open file: " + path, 3);
    }
    qrk::Object::ObjectData object;
    std::string dataBuffer;
    while (std::getline(objFile, dataBuffer)) {
        int pos = 0;
        std::string breakBuffer;
        std::vector<std::string> buffer;

        std::stringstream data;
        data << dataBuffer;
        while (std::getline(data, breakBuffer, ' ')) {
            buffer.push_back(breakBuffer);
            breakBuffer.clear();
        }

        if (buffer[0] == "v") {
            qrk::vec4f bufferVertexVector({std::stof(buffer[1]),
                                           std::stof(buffer[2]),
                                           std::stof(buffer[3]), 1.f});
            object.vertices.push_back(bufferVertexVector);
        }
        if (buffer[0] == "vt") {
            qrk::vec2f bufferTextureVector(
                    {std::stof(buffer[1]), std::stof(buffer[2])});
            object.textures.push_back(bufferTextureVector);
        }
        if (buffer[0] == "vn") {
            qrk::vec3f bufferNormalVector({std::stof(buffer[1]),
                                           std::stof(buffer[2]),
                                           std::stof(buffer[3])});
            object.normals.push_back(bufferNormalVector);
        }
        if (buffer[0] == "f") {
            std::stringstream sBuffer;
            for (int i = 1; i < buffer.size(); i++) {
                sBuffer << buffer[i];
                std::string oBuffer;
                std::vector<std::string> osBuffer;
                while (std::getline(sBuffer, oBuffer, '/')) {
                    osBuffer.push_back(oBuffer);
                    oBuffer.clear();
                }
                object.vertexIndeces.push_back(std::stoi(osBuffer[0]));
                object.textureIndeces.push_back(std::stoi(osBuffer[1]));
                object.normalIndeces.push_back(std::stoi(osBuffer[2]));
                osBuffer.clear();
                sBuffer.clear();
            }
        }

        //clean up
        buffer.clear();
        breakBuffer.clear();
        data.clear();
        dataBuffer.clear();
    }

    objFile.close();

    //check if the data was loaded succesfully
    if (!(object.vertexIndeces.size() == object.textureIndeces.size()) &&
        !(object.vertexIndeces.size() == object.normalIndeces.size())) {
        std::string error = "Failed to load object at: " + path +
                            " Index arrays have different sizes";
        qrk::Debug::Error(error, 4);
    }
    for (unsigned int element : object.vertexIndeces) {
        if (element > object.vertices.size()) {
            std::string error = "Failed to load object at: " + path +
                                " Out of range exception (vertices)";
            qrk::Debug::Error(error, 5);
        }
    }
    for (unsigned int element : object.textureIndeces) {
        if (element > object.textures.size()) {
            std::string error = "Failed to load object at: " + path +
                                " Out of range exception (textures)";
            qrk::Debug::Error(error, 6);
        }
    }
    for (unsigned int element : object.normalIndeces) {
        if (element > object.normals.size()) {
            std::string error = "Failed to load object at: " + path +
                                " Out of range exception (normals)";
            qrk::Debug::Error(error, 7);
        }
    }

    //parse data into a 1d array of GLfloats
    std::vector<qrk::vec4f> alignedVertices;
    std::vector<qrk::vec2f> alignedTextures;
    std::vector<qrk::vec3f> alignedNormals;

    for (int i = 0; i < object.vertexIndeces.size(); i++) {
        alignedVertices.push_back(object.vertices[object.vertexIndeces[i] - 1]);
        alignedTextures.push_back(
                object.textures[object.textureIndeces[i] - 1]);
        alignedNormals.push_back(object.normals[object.normalIndeces[i] - 1]);
    }

    if (!(alignedVertices.size() == alignedTextures.size()) &&
        !(alignedVertices.size() == alignedNormals.size())) {
        std::string error = "Failed to load object at: " + path +
                            " Aligned arrays have different sizes";
        qrk::Debug::Error(error, 8);
    }

    std::vector<GLfloat> loadResult;

    for (int i = 0; i < alignedVertices.size(); i++) {
        loadResult.push_back(alignedVertices[i].x());
        loadResult.push_back(alignedVertices[i].y());
        loadResult.push_back(alignedVertices[i].z());
        loadResult.push_back(alignedVertices[i].w());

        loadResult.push_back(alignedTextures[i].x());
        loadResult.push_back(alignedTextures[i].y());

        loadResult.push_back(alignedNormals[i].x());
        loadResult.push_back(alignedNormals[i].y());
        loadResult.push_back(alignedNormals[i].z());
    }

    //clean up
    std::vector<qrk::vec4f>().swap(alignedVertices);
    std::vector<qrk::vec3f>().swap(alignedNormals);
    std::vector<qrk::vec2f>().swap(alignedTextures);
    object.Clear();

    //return data
    _promisedData.set_value(loadResult);
    std::vector<GLfloat>().swap(loadResult);
    loadFinished = true;
}

void qrk::Object::LoadObject(const std::string &path) {
    //get data from file
    if (!std::filesystem::exists(path)) {
        std::string error = "Failed to find file: " + path;
        qrk::Debug::Error(error, 3);
    }
    std::ifstream objFile(path);
    if (!objFile.is_open()) {
        std::string error = "Failed to open file: " + path;
        qrk::Debug::Error(error, 4);
    }
    qrk::Object::ObjectData object;
    std::string dataBuffer;
    while (std::getline(objFile, dataBuffer)) {
        int pos = 0;
        std::string breakBuffer;
        std::vector<std::string> buffer;

        std::stringstream data;
        data << dataBuffer;
        while (std::getline(data, breakBuffer, ' ')) {
            buffer.push_back(breakBuffer);
            breakBuffer.clear();
        }

        if (buffer[0] == "v") {
            qrk::vec4f bufferVertexVector({std::stof(buffer[1]),
                                           std::stof(buffer[2]),
                                           std::stof(buffer[3]), 1.f});
            object.vertices.push_back(bufferVertexVector);
        }
        if (buffer[0] == "vt") {
            qrk::vec2f bufferTextureVector(
                    {std::stof(buffer[1]), std::stof(buffer[2])});
            object.textures.push_back(bufferTextureVector);
        }
        if (buffer[0] == "vn") {
            qrk::vec3f bufferNormalVector({std::stof(buffer[1]),
                                           std::stof(buffer[2]),
                                           std::stof(buffer[3])});
            object.normals.push_back(bufferNormalVector);
        }
        if (buffer[0] == "f") {
            std::stringstream sBuffer;
            for (int i = 1; i < buffer.size(); i++) {
                sBuffer << buffer[i];
                std::string oBuffer;
                std::vector<std::string> osBuffer;
                while (std::getline(sBuffer, oBuffer, '/')) {
                    osBuffer.push_back(oBuffer);
                    oBuffer.clear();
                }
                object.vertexIndeces.push_back(std::stoi(osBuffer[0]));
                object.textureIndeces.push_back(std::stoi(osBuffer[1]));
                object.normalIndeces.push_back(std::stoi(osBuffer[2]));
                osBuffer.clear();
                sBuffer.clear();
            }
        }
        //clean up
        buffer.clear();
        breakBuffer.clear();
        data.clear();
        dataBuffer.clear();
    }

    objFile.close();

    //check if data was loaded sucessfully
    if (!(object.vertexIndeces.size() == object.textureIndeces.size()) &&
        !(object.vertexIndeces.size() == object.normalIndeces.size())) {
        std::string error = "Failed to load object at: " + path +
                            " Index arrays have different sizes";
        qrk::Debug::Error(error, 5);
    }
    for (unsigned int element : object.vertexIndeces) {
        if (element > object.vertices.size()) {
            std::string error = "Failed to load object at: " + path +
                                " Out of range exception (vertices)";
            qrk::Debug::Error(error, 6);
        }
    }
    for (unsigned int element : object.textureIndeces) {
        if (element > object.textures.size()) {
            std::string error = "Failed to load object at: " + path +
                                " Out of range exception (textures)";
            qrk::Debug::Error(error, 7);
        }
    }
    for (unsigned int element : object.normalIndeces) {
        if (element > object.normals.size()) {
            std::string error = "Failed to load object at: " + path +
                                " Out of range exception (normals)";
            qrk::Debug::Error(error, 8);
        }
    }

    //parse data into a 1d array of GLfloats
    std::vector<qrk::vec4f> alignedVertices;
    std::vector<qrk::vec2f> alignedTextures;
    std::vector<qrk::vec3f> alignedNormals;

    for (int i = 0; i < object.vertexIndeces.size(); i++) {
        alignedVertices.push_back(object.vertices[object.vertexIndeces[i] - 1]);
        alignedTextures.push_back(
                object.textures[object.textureIndeces[i] - 1]);
        alignedNormals.push_back(object.normals[object.normalIndeces[i] - 1]);
    }

    if (!(alignedVertices.size() == alignedTextures.size()) &&
        !(alignedVertices.size() == alignedNormals.size())) {
        std::string error = "Failed to load object at: " + path +
                            " Aligned arrays have different sizes";
        qrk::Debug::Error(error, 15);
    }

    std::vector<GLfloat> loadResult;

    for (int i = 0; i < alignedVertices.size(); i++) {
        loadResult.push_back(alignedVertices[i].x());
        loadResult.push_back(alignedVertices[i].y());
        loadResult.push_back(alignedVertices[i].z());
        loadResult.push_back(alignedVertices[i].w());

        loadResult.push_back(alignedTextures[i].x());
        loadResult.push_back(alignedTextures[i].y());

        loadResult.push_back(alignedNormals[i].x());
        loadResult.push_back(alignedNormals[i].y());
        loadResult.push_back(alignedNormals[i].z());
    }

    //clean up
    std::vector<qrk::vec4f>().swap(alignedVertices);
    std::vector<qrk::vec3f>().swap(alignedNormals);
    std::vector<qrk::vec2f>().swap(alignedTextures);
    object.Clear();

    //return data and further clean up
    data = loadResult;
    std::vector<GLfloat>().swap(loadResult);
}

std::string qrk::Object::DumpObjectData(std::string path) {
    std::stringstream dataDump;
    //print data to the log file
    for (int i = 0; i < data.size(); i += 9) {
        dataDump << "Vertex " << (i / 9) + 1 << "\nVertex coordinates:\t "
                 << data[i] << " | " << data[i + 1] << " | " << data[i + 2]
                 << " | " << data[i + 3] << " \nTexture coordinates:\t "
                 << data[i + 4] << " | " << data[i + 5]
                 << " \nNormal coordinates:\t " << data[i + 6] << " | "
                 << data[i + 7] << " | " << data[i + 8] << " \n"
                 << std::endl;
    }

    return dataDump.str();
}

qrk::obj qrk::GLObject::GetDrawData() {
    qrk::obj returnData;
    returnData.VAO = this->VAO;
    returnData.VBO = this->VBO;
    if (textured) {
        returnData.texture = this->texture;
        returnData.textured = true;
    } else {
        returnData.texture = nullptr;
        returnData.textured = false;
    }
    returnData.vertexCount = this->objectData->vertexNumber;
    returnData.position = this->posMatrix;
    returnData.rotation = this->rotMatrix;
    returnData.scale = this->sclMatrix;
    returnData.color = this->color;
    return returnData;
}