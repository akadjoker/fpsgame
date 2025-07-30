
#include "camera.hpp"

void CameraFPS::Init(Vector3 startPos)
{
    translation = startPos;
    lastPosition = startPos;
    oldMousePos = GetMousePosition();
    firstUpdate = true;
    
    camera.target = (Vector3){0.0f, 2.0f, 0.0f};
    camera.position = Vector3Add(translation, { 0, baseEyeHeight, 0 });
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
}

void CameraFPS::Update(float deltaTime, Collider& world)
{
 

 
    if (IsKeyPressed(KEY_ENTER))
    {
        if (IsCursorHidden())
            EnableCursor();
        else
            DisableCursor();
    }
 
    if (firstUpdate)
    {
        lastPosition = translation;
        falling = false;
        lastTime = (u32)(GetTime() * 1000.0);
        fallingVelocity = { 0.0f, 0.0f, 0.0f };
        firstUpdate = false;
    }

    u32 currentTime = (u32)(GetTime() * 1000.0);
    u32 diff = currentTime - lastTime;
    lastTime = currentTime;
    float deltaTimeMs = (float)diff * 0.001f;

    // Rato
    Vector2 deltaMouse = Vector2Subtract(GetMousePosition(), oldMousePos);
    oldMousePos = GetMousePosition();

    Matrix rotationY = MatrixRotateY(-deltaMouse.x * mouseSensitivity);
    Vector3 forward = Vector3Transform(GetCameraForward(camera), rotationY);
    Vector3 right = Vector3Transform(GetCameraRight(camera), rotationY);

    pitch += deltaMouse.y * mouseSensitivity;
    pitch = Clamp(pitch, -1.5f, 1.5f);

    Vector3 forwardHorizontal =Vector3Normalize((Vector3){ forward.x, 0, forward.z });
    Vector3 newForward =Vector3Add(forwardHorizontal, Vector3Scale({ 0, 1, 0 }, sinf(pitch)));
    newForward = Vector3Normalize(newForward);

    // Input movimento
    Vector3 moveInput = { 0 };
    if (IsKeyDown(KEY_W)) moveInput = Vector3Add(moveInput, forwardHorizontal);
    if (IsKeyDown(KEY_S)) moveInput = Vector3Subtract(moveInput, forwardHorizontal);
    if (IsKeyDown(KEY_D)) moveInput = Vector3Add(moveInput, right);
    if (IsKeyDown(KEY_A)) moveInput = Vector3Subtract(moveInput, right);

    Vector3 vel = Vector3Subtract(translation, lastPosition);

    if (Vector3Length(moveInput) > 0.0f)
    {
        moveInput = Vector3Normalize(moveInput);
        moveInput = Vector3Scale(moveInput, walkSpeed * deltaTimeMs);
        vel = Vector3Add(vel, moveInput);
        isMoving = true;
    }
    else
    {
        isMoving = false;
    }

    if (IsKeyPressed(KEY_SPACE) && !falling) fallingVelocity.y = jumpForce;

    fallingVelocity =Vector3Add(fallingVelocity, Vector3Scale(gravity, deltaTimeMs));

    

    Vector3 result = world.collideEllipsoidWithWorld(
        lastPosition, 
        ellipsoidRadius, 
        vel, 
        slidingSpeed, 
        fallingVelocity,
        hitTriangle,
        hitPosition,
         outFalling, 
         collision);

    if (outFalling)
    {
        falling = true;
    }
    else
    {
        falling = false;
        fallingVelocity = { 0.0f, 0.0f, 0.0f };
    }

    translation = result;
    lastPosition = translation;
    camera.position = Vector3Add(translation, (Vector3){ 0, baseEyeHeight, 0 });

    if (isShaking)
    {
        shakeTimer -= deltaTime;
        
        if (shakeTimer <= 0.0f)
        {
            // Shake terminado
            isShaking = false;
            shakeOffset = { 0.0f, 0.0f, 0.0f };
        }
        else
        {
           
            float currentIntensity = shakeIntensity * (shakeTimer / shakeDuration);
            
          
            shakeOffset.x = (float)(rand() % 200 - 100) / 100.0f * currentIntensity;
            shakeOffset.y = (float)(rand() % 200 - 100) / 100.0f * currentIntensity;
            shakeOffset.z = (float)(rand() % 200 - 100) / 100.0f * currentIntensity;
        }
        camera.position = Vector3Add(camera.position, shakeOffset);
    }
    
 



    
    camera.target = Vector3Add(camera.position, newForward);
}

void CameraFPS::Stats() 
{
            DrawText(TextFormat("Posição: %.1f, %.1f, %.1f", translation.x, translation.y, translation.z), 10, 130, 16, DARKGRAY);
            DrawText(TextFormat("Velocidade: %.1f, %.1f, %.1f",  velocity.x, velocity.y, velocity.z), 10, 150, 16, DARKGRAY);
            DrawText(TextFormat("Collide: %s",collision ? "yes" : "no"), 10, 60, 16, DARKGRAY);
            DrawText(TextFormat("Falling: %s", falling ? "yes" : "no"), 10, 20, 16, DARKGRAY);
            DrawText(TextFormat("isMoving: %s", isMoving ? "yes" : "no"), 10, 40, 16, DARKGRAY);
}

void CameraFPS::StartShake(float duration, float intensity) 
{
     isShaking = true;
        shakeDuration = duration;
        shakeTimer = duration;
        shakeIntensity = intensity;
        originalPosition = camera.position;
}
