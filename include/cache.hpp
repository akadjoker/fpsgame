#pragma once
class Cache
{

    Cache() {}

public:
    Camera3D camera;
    Matrix viewMatrix;
    Matrix projectionMatrix;

    static Cache& getInstance()
    {
        static Cache instance;
        return instance;
    }

};

#define CACHE Cache::getInstance()