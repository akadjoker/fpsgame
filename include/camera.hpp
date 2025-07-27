#pragma once
#include "Config.hpp"
#include "collision.hpp"


class CameraFPS {
public:
    Camera camera;
    Vector3 ellipsoidRadius = { 1.6f, 2.8f, 1.6f };

    void Init(Vector3 startPos);
    void Update(float deltaTime, Collider& world);

    void Stats();
    bool IsMoving() { return isMoving; }

    void StartShake(float duration, float intensity);

private:
    float mouseSensitivity = 0.003f;
    float walkSpeed = 25.0f;
    float jumpForce = 2.0f;
    float slidingSpeed = 0.00001f;
    float baseEyeHeight = 0.8f;

    Vector3 translation;
    Vector3 lastPosition;
    Vector3 velocity = { 0.0f, 0.0f, 0.0f };
    Vector3 fallingVelocity = { 0.0f, 0.0f, 0.0f };
    Vector3 gravity = { 0.0f, -9.0f, 0.0f };

    bool falling = false;
    bool outFalling = false;
    bool firstUpdate = true;
    bool collision = false;
    bool isMoving = false;

    Vector2 oldMousePos;
    float pitch = 0.0f;
    float blend = 0.0f;
    u32 lastTime = 0;

    Triangle hitTriangle;
    Vector3 hitPosition;

    bool useFreeCamera = false;


      bool isShaking = false;
    float shakeTimer = 0.0f;
    float shakeDuration = 0.0f;
    float shakeIntensity = 0.0f;
    Vector3 shakeOffset = { 0.0f, 0.0f, 0.0f };
    Vector3 originalPosition;
};
