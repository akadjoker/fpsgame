
#include "Config.hpp"
#include "pch.h"
#include "cache.hpp"
#include "batch.hpp"
#include "bsp.h"
#include "collision.hpp"
#include "frustum.hpp"
#include "camera.hpp"
#include "md3.hpp"
#include "assets.hpp"
#include "decal.hpp"
#include "emitter.hpp"


float bobbingTime = 0.0f;
float walkBobbingSpeed = 8.0f;
float walkBobbingAmount = 0.02f;
float idleBobbingSpeed = 2.0f;
float idleBobbingAmount = 0.005f;
bool isPlayerWalking = false;

 
Vector3 CalculateWeaponBobbing(float deltaTime) 
{
    bobbingTime += deltaTime;
    
    Vector3 offset = {0.0f, 0.0f, 0.0f};
    
    if (isPlayerWalking) 
    {
        offset.y = sin(bobbingTime * walkBobbingSpeed) * walkBobbingAmount;
        offset.x = cos(bobbingTime * walkBobbingSpeed * 0.3f) * walkBobbingAmount * 0.2f;
    } 
    else 
    {
        offset.y = sin(bobbingTime * idleBobbingSpeed) * idleBobbingAmount;
    }
    
    return offset;
}




int main()
{

  //  SetTraceLogLevel(LOG_NONE);

    InitWindow(1024, 768, "Sistema de Colisão FPS - Raylib");
    InitAudioDevice();
    SetTargetFPS(60);
    DisableCursor();

    ViewFrustum frustum;

    LOAD_SOUND("sounds/mp5k.wav", "shoot");
    LOAD_SOUND("sounds/clipin.wav", "clipin");
    LOAD_SOUND("sounds/clipout.wav", "clipout");
    LOAD_SOUND("sounds/cock1.wav", "cock1");
    LOAD_SOUND("sounds/cock2.wav", "cock2");




    Model barrel = LoadModel("models/barrel/barrel.glb");
    Texture2D texture = LOAD_TEXTURE("models/barrel/barrel_d.png", "barrel");
    barrel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;            // Set map diffuse texture
    barrel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    Scene scene;

    scene.AddNode(&barrel, {37.5f, 26.0f ,6.6f}, {0.04f, 0.04f, 0.04f});
    scene.AddNode(&barrel, {36.5f, 26.0f ,9.6f}, {0.04f, 0.04f, 0.04f});
    scene.AddNode(&barrel, {-18.7f, 13.4f ,10.4f}, {0.04f, 0.04f, 0.04f});
    scene.AddNode(&barrel, {39.5f, 26.0f, 8.4f}, {0.04f, 0.04f, 0.04f});
    scene.AddNode(&barrel, {-1.1f, 26.0f, 7.6f}, {0.04f, 0.04f, 0.04f});






    Shader shader = LoadShader("shaders/lightmap.vs", "shaders/lightmap.fs");
    int blendLoc = GetShaderLocation(shader, "lightmapBlend");

    BSP map;




    map.loadFromFile("maps/oa_rpg3dm2.bsp");
   //     map.loadFromFile("maps/egyptians.bsp");



         const std::vector<BSPSurface>  surfaces = map.getSurfaces();

         Octree quad(map.getBounds());
         Collider world;


    for (u32 i = 0; i < surfaces.size(); i++)
    {
        const BSPSurface& surface = surfaces[i];
        const std::vector<Vector3>& verts = surface.vertices;
        const std::vector<u16>& indices = surface.indices;


        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
           quad.addTriangle(verts[indices[i + 0]], verts[indices[i + 1]], verts[indices[i + 2]]);
        }





    }

//     {
//     std::vector<Triangle> triangles = scene.GetNode(0)->GetTriangles(true);
//     for (u32 i = 0; i < triangles.size(); i++)
//     {
//         quad.addTriangle(triangles[i].pointA, triangles[i].pointB, triangles[i].pointC);
//     }
//     }
// {
//     std::vector<Triangle> triangles = scene.GetNode(1)->GetTriangles(true);
//     for (u32 i = 0; i < triangles.size(); i++)
//     {
//         quad.addTriangle(triangles[i].pointA, triangles[i].pointB, triangles[i].pointC);
//     }
//     }
    quad.rebuild();

    world.setCollisionSelector(&quad);


    float blend = 0.5f;
    SetShaderValue(shader, blendLoc, &blend, SHADER_UNIFORM_FLOAT);
    map.SetShader(&shader);


    LoadMD3 weapon;
    LoadMD3 wlinks[8];
    weapon.Load("models/mp5k/mp5k_hold.MD3");

    texture =LOAD_TEXTURE("models/hand_brown.png", "hand_brown");

    weapon.SetTexture(0, texture.id);
 
    weapon.getAnimator()->AddAnimation("all", 0, 85, 15.0f, true);
    weapon.getAnimator()->AddAnimation("idle", 0, 1, 15.0f, false);
    weapon.getAnimator()->AddAnimation("fire", 0, 10, 60.0f, false);
    weapon.getAnimator()->AddAnimation("reload", 11, 85, 33.0f, false);
    weapon.getAnimator()->PlayAnimation("idle", true);

    LogInfo("Loading beretta...");
    const float FACTOR = 0.027f;
    wlinks[0].Load("models/mp5k/mp5k_view_main.MD3",FACTOR);
    weapon.setLink("tag_main", &wlinks[0]);

    

   
    
    
     wlinks[1].Load("models/mp5k/mp5k_view_slide.MD3",FACTOR);
     wlinks[2].Load("models/mp5k/mp5k_view_clip.MD3",FACTOR);
     wlinks[3].Load("models/mp5k/mp5k_view_trigger.MD3",FACTOR);
     wlinks[4].Load("models/mp5k/mp5k_view_bolt.MD3",FACTOR);;
     wlinks[5].Load("models/mp5k/mp5k_view_mode.MD3",FACTOR);;
    weapon.setLink("tag_slide", &wlinks[1]);
    weapon.setLink("tag_clip", &wlinks[2]);
    weapon.setLink("tag_trigger", &wlinks[3]);
    weapon.setLink("tag_bolt", &wlinks[4]);
    weapon.setLink("tag_mode", &wlinks[5]);


    texture =LOAD_TEXTURE("models/mp5k/mp5k01.png", "mp5k01");

   
    

    wlinks[0].SetTexture(0, texture.id);
    
    texture =LOAD_TEXTURE("models/mp5k/mp5k02.png", "mp5k02");
    
    wlinks[0].SetTexture(1, texture.id);
    wlinks[1].SetTexture(0, texture.id);
    wlinks[2].SetTexture(0, texture.id);
    wlinks[3].SetTexture(0, texture.id);
    wlinks[4].SetTexture(0, texture.id);
    wlinks[5].SetTexture(0, texture.id);

    CameraFPS camera;
    camera.Init((Vector3){5.0f, 30.0f, -5.0f});
    Shader modelShader = LoadShader("shaders/md3.vs", "shaders/md3.fs");
    float rotate = -4.7f;

    std::vector<Vector3> points;

    Texture2D decal =LOAD_TEXTURE("images/bulletdecal.png", "decal");
    LOAD_TEXTURE("images/Flash1_1.png", "flash_a");
    LOAD_TEXTURE("images/Flash1_2.png", "flash_b");
    LOAD_TEXTURE("images/pwave.png", "pwave");

    Decal3D decals;

    

    // Criar sistema
    ParticleSystem particleSystem;
    particleSystem.Init(LOAD_TEXTURE("images/fire_particle.png", "particle"), 500);

    EffectEmitter muzzleFlash;
    muzzleFlash.Init(LOAD_TEXTURE("images/Flash1_1.png", "flash_a"), 50);
    EffectEmitter shockWave;
    shockWave.Init(LOAD_TEXTURE("images/pwave.png", "pwave"), 50);

    Shader shaderParticles = LoadShader("shaders/particles.vs", "shaders/particles.fs");
    int textureLoc = GetShaderLocation(shaderParticles, "texture0");
    int index =0 ;
    SetShaderValue(shaderParticles, textureLoc, &index, SHADER_UNIFORM_FLOAT);



    // 37.5 28.4 6.6

    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_P)) blend += 0.01f;
        if (IsKeyDown(KEY_I)) blend -= 0.01f;

        blend = Clamp(blend, 0.0f, 1.0f);
        SetShaderValue(shader, blendLoc, &blend, SHADER_UNIFORM_FLOAT);

        float dt = GetFrameTime();

        camera.Update(dt, world);
        shockWave.Update(dt);
        muzzleFlash.Update(dt);
        particleSystem.Update(dt);
        isPlayerWalking = camera.IsMoving();


        
        
        
        
        BeginDrawing();
        ClearBackground(SKYBLUE);
        
        //    UpdateCamera(&camera, CAMERA_FREE);
        
        BeginMode3D(camera.camera);

         rlEnableBackfaceCulling();
         rlEnableDepthTest();
 
        
        
        
        
        frustum.update();
        
        map.render(frustum);

        // DrawModel(barrel, {37.5f, 24.4f ,6.6f}, 0.04f, WHITE); 
     //   DrawModel(barrel, {-18.7f, 13.4f ,10.4f}, 0.04f, WHITE); 
        scene.Render();

        
        weapon.update(dt);

        int w = GetScreenWidth() / 2;
        int h = GetScreenHeight() / 2;
        

        Ray ray = GetMouseRay((Vector2){(float)w, (float)h}, camera.camera);


        Matrix matView = rlGetMatrixModelview();
        Matrix scaleMat = MatrixScale(0.090f,0.090f,0.090f);
        Matrix rotMat   = QuaternionToMatrix(QuaternionFromEuler(80,rotate,0));
        Vector3 bobbingOffset = CalculateWeaponBobbing(GetFrameTime());
            
        Matrix transMat = MatrixTranslate(
                0.0f + bobbingOffset.x,   
                0.0f + bobbingOffset.y,     
                -0.2f + bobbingOffset.z      
            );

        Matrix local = MatrixMultiply(MatrixMultiply(scaleMat, rotMat), transMat);
        Matrix cano = MatrixMultiply(local, MatrixInvert(matView));
        Vector3 tag = Vector3Transform(weapon.GetTagPosition(0),cano);
        muzzleFlash.SetPosition(tag);
 
        if(IsKeyDown(KEY_M))
        {
            Ray pick = GetMouseRay(GetMousePosition(), camera.camera);
            std::vector<const Triangle*> tris = quad.getCandidates(pick, 1000.0f);

            for (const Triangle* tri : tris)
            {
                    RayCollision status = GetRayCollisionTriangle(ray, tri->pointA, tri->pointB, tri->pointC);
                    if (status.hit && status.distance < 100)
                    {
                        DrawSphere(status.point, 0.1f, RED);
                        LogInfo("Picked: %f %f %f", status.point.x, status.point.y, status.point.z);
                    }
                
            }
        }



       rlEnableShader(modelShader.id);

        MD3Animator* animator = weapon.getAnimator();
 
    
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !animator->IsPlaying())
            {
                animator->PlayAnimationThen("fire", "idle");
                PLAY_SOUND("shoot");


                //  particleSystem.EmitMuzzleFlash(tag, {0, 1, 0}, 0.09f);
                //  particleSystem.EmitBulletImpact(tag, {0, 1, 0}, 20);

                muzzleFlash.CreateMuzzleFlash(tag, GetRandomValue(0, 360),
                                              0.18f, 0.1f);
                muzzleFlash.CreateMuzzleFlash(tag, GetRandomValue(0, 360), 0.1f,
                                              0.2f);

                PickData data;
                 if (scene.collide(ray,100.0f, &data))
                 {
                     //decals.AddDecal(data.intersectionPoint, data.intersectionTriangle.normal, &data.intersectionTriangle, decal, 0.2f,WHITE);
                     particleSystem.EmitBulletImpact(data.intersectionPoint, data.intersectionTriangle.normal, 20);
                   ////  scene.AddToRemove(data.node);
                   // camera.StartShake(0.05f, 0.1f);

                   
                   
                   data.node->tag++;
                   if (data.node->tag == 3)
                   {
                       data.node->SetVisible(false);
                       shockWave.CreateShockwave(Vector3Add(data.node->localPosition, {0,0.9,0}), 8.0f, 2.5f, 0.5f, 10.0f);
                       camera.StartShake(0.1f, 0.3f);
                       particleSystem.EmitBarrelExplosion(data.intersectionPoint, 1.0f);
                     }
                 }
         

                    float closestDistance = FLT_MAX;
                    const Triangle* closestTri = nullptr;
                    RayCollision closestHit = {0};
                    std::vector<const Triangle*> tris = quad.getCandidates(ray, 1000.0f);
                    
                    // Encontra o triângulo mais próximo
                    for (const Triangle* tri : tris)
                    {
                        RayCollision status = GetRayCollisionTriangle(ray, tri->pointA, tri->pointB, tri->pointC);
                        if (status.hit && status.distance < closestDistance)
                        {
                            closestDistance = status.distance;
                            closestTri = tri;
                            closestHit = status;
                        }
                    }
                    
                    if (closestTri)
                    {
                        if (Vector3DotProduct(closestHit.normal, ray.direction) > 0) 
                        {
                            closestHit.normal = Vector3Negate(closestHit.normal);
                        }

                      
                          decals.AddDecal(closestHit.point,closestHit.normal, closestTri, decal, 0.2f,WHITE);
               
                          particleSystem.EmitBulletImpact(closestHit.point, closestHit.normal, 20);


                          
                           // shockWave.CreateShockwave(closestHit.point, 8.0f, 1.5f);


                        
                        //decals.AddDecalAtPosition(closestHit.point, closestHit.normal, decal, 1.0f, WHITE);
                       // LogInfo("HIT at distance: %.2f", closestDistance);
                    }
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !animator->IsPlaying())
            {
               animator->PlayAnimationThen("reload", "idle");
            }
            
            if (animator->IsPlaying())
            {
                if (animator->GetFrame() == 40 && !IS_SOUND_PLAYING("clipout"))
                {
                    PLAY_SOUND("clipout");
                }
                if (animator->GetFrame() == 65 && !IS_SOUND_PLAYING("clipin"))
                {
                    PLAY_SOUND("clipin");
                }
                 if (animator->GetFrame() == 77 && !IS_SOUND_PLAYING("cock2"))
                {
                    PLAY_SOUND("cock2");
                }
                  if (animator->GetFrame() == 20 && !IS_SOUND_PLAYING("cock1"))
                {
                    PLAY_SOUND("cock1");
                }
            }
            
            
 

        weapon.render(modelShader,local, MatrixInvert(matView));
        Matrix matProjection = rlGetMatrixProjection();
        Matrix matModelView = MatrixMultiply(MatrixIdentity(), matView);
        Matrix matModelViewProjection = MatrixMultiply(matModelView, matProjection);

     


        rlEnableShader(shaderParticles.id);
	    int locIndex = GetShaderLocation(shaderParticles, "mvp");
        rlSetUniformMatrix(locIndex, matModelViewProjection);
 

        rlDisableBackfaceCulling();
        shockWave.Draw(camera.camera);
      
      
       // rlDisableDepthTest();
        rlDisableDepthMask(); 
        particleSystem.Draw(camera.camera);
        muzzleFlash.Draw(camera.camera);

        rlEnableDepthMask();
      //  rlEnableDepthTest();
        
        decals.Draw();  
        
        rlDisableShader();
        
        

 
      // DrawCube(tag, 0.1f, 0.1f, 0.1f, RED);

        EndMode3D();


       camera.Stats();
       DrawText(TextFormat("Cound: %d",map.getViewCount()), 10, 80, 16, DARKGRAY);
       DrawText(TextFormat("Frame: %d",animator->GetFrame()), 10, 100, 16, DARKGRAY);




        if (IsCursorHidden())
        {
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();
            DrawLine(screenWidth/2 - 10, screenHeight/2, screenWidth/2 + 10, screenHeight/2, WHITE);
            DrawLine(screenWidth/2, screenHeight/2 - 10, screenWidth/2, screenHeight/2 + 10, WHITE);
        }

        EndDrawing();
    }

    scene.Clear();

    muzzleFlash.Release();
    shockWave.Release();
    particleSystem.Release();
    UnloadModel(barrel);
    ASSETS.unloadAllAssets();


    map.clear();
    UnloadShader(shader);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}


