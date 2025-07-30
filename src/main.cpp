
#include "pch.h"
#include "Config.hpp"
#include "screen.hpp"
#include "cache.hpp"
#include "batch.hpp"
#include "bsp.hpp"
#include "collision.hpp"
#include "frustum.hpp"
#include "camera.hpp"
#include "md3.hpp"
#include "assets.hpp"
#include "decal.hpp"
#include "emitter.hpp"
#include "scene.hpp"
#include "animation.hpp"
#include "frustum.hpp"

float bobbingTime = 0.0f;
float walkBobbingSpeed = 8.0f;
float walkBobbingAmount = 0.02f;
float idleBobbingSpeed = 2.0f;
float idleBobbingAmount = 0.005f;
bool isPlayerWalking = false;


Vector3 CalculateWeaponBobbing(float deltaTime)
{
    bobbingTime += deltaTime;

    Vector3 offset = { 0.0f, 0.0f, 0.0f };

    if (isPlayerWalking)
    {
        offset.y = sin(bobbingTime * walkBobbingSpeed) * walkBobbingAmount;
        offset.x = cos(bobbingTime * walkBobbingSpeed * 0.3f)
            * walkBobbingAmount * 0.2f;
    }
    else
    {
        offset.y = sin(bobbingTime * idleBobbingSpeed) * idleBobbingAmount;
    }

    return offset;
}



class Player {
    LoadMD3 lower;
    LoadMD3 head;
    LoadMD3 upper;
    LoadMD3 weapon;
    MD3Animator* lowerAnimator;
    MD3Animator* upperAnimator;
    Texture2D skin;
    Vector3 positioncenter;
    float radius = 2.8f;
    
    bool active = true;
    
    public:
    Node3D transform;


    Player() {}

    void SetActive(bool active)
    {
        this->active = active;
    }

    bool pick(Ray& ray)
    {
        if (!active)
        {
            return false;
        }
        RayCollision global = GetRayCollisionSphere(ray, positioncenter, radius);
        if (!global.hit)
        {
            return false;
        }
        bool lowerHit = lower.pick(ray);
        bool headHit  = head.pick(ray);
        bool upperHit = upper.pick(ray);

        return lowerHit || headHit || upperHit;
    }

    void Kill()
    {
        active = false;
        lowerAnimator->PlayAnimation("DEATH_BLOWBACK");
        upperAnimator->PlayAnimation("DEATH_BLOWBACK");
    }

    bool Load(const std::string& szFileName,
              const std::string& szWeaponFileName, float scale = 0.1f)
    {
        std::string lowerName = szFileName + "lower.md3";
        if (!lower.Load(lowerName.c_str(), scale))
        {
            LogError("load md3 file: %s", lowerName.c_str());
            return false;
        }

        std::string headName = szFileName + "head.md3";
        if (!head.Load(headName.c_str(), scale))
        {
            LogError("load md3 file: %s", headName.c_str());
            return false;
        }

        std::string upperName = szFileName + "upper.md3";
        if (!upper.Load(upperName.c_str(), scale))
        {
            LogError("load md3 file: %s", upperName.c_str());
            return false;
        }

        std::string weaponName = szWeaponFileName;
        if (!weapon.Load(weaponName.c_str(), scale))
        {
            LogError("load md3 file: %s", weaponName.c_str());
            return false;
        }

        lowerAnimator = lower.getAnimator();
        upperAnimator = upper.getAnimator();

        lowerAnimator->AddAnimation("all", 0, 836, 30.0f, true);
        lowerAnimator->AddAnimation("IDLE", 643, 1, 30.0f, false);
        lowerAnimator->AddAnimation("WALK", 385, 385 + 20, 21.0f, true);
        lowerAnimator->AddAnimation("DEATH_BLOWAHEAD", 146, 146 + 33, 15.0f,false);
        lowerAnimator->AddAnimation("DEATH_CHEST", 61, 61 + 31, 15.0f, false);
        lowerAnimator->AddAnimation("DEATH_BLOWBACK", 1, 60, 15.0f, false);
        lowerAnimator->AddAnimation("DEATH_HEADFRONT", 61, 61 + 30, 15.0f,false);
        lowerAnimator->AddAnimation("DEAD_BACK", 231, 231 + 31, 15.0f, true);

        lowerAnimator->PlayAnimation("IDLE");


        upperAnimator->AddAnimation("all", 0, 979, 30.0f, true);
        upperAnimator->AddAnimation("STAND_PISTOL", 458, 458, 15, true);
        upperAnimator->AddAnimation("STAND_RIFLE", 470, 470, 15, true);
        upperAnimator->AddAnimation("STAND_WALK", 938, 938 + 21, 15, true);
        upperAnimator->AddAnimation("ATTACK_RIFLE", 470, 470 + 11, 30, true);
        upperAnimator->AddAnimation("ATTACK_PISTOL", 458, 458 + 11, 30, true);
        upperAnimator->AddAnimation("DEATH_BLOWAHEAD", 146, 146 + 33, 30.0f,false);
        upperAnimator->AddAnimation("DEATH_CHEST", 61, 61 + 31, 15.0f, false);
        upperAnimator->AddAnimation("DEATH_BLOWBACK", 1, 60, 15.0f, false);
        upperAnimator->AddAnimation("DEATH_HEADFRONT", 61, 61 + 30, 15.0f,false);
        upperAnimator->AddAnimation("DEAD_BACK", 231, 231 + 31, 15.0f, true);
        upperAnimator->AddAnimation("POINT", 653, 653 + 38, 15.0f, true);
        upperAnimator->AddAnimation("RELOAD_PISTOL", 572, 572 + 20, 7.0f, true);
        upperAnimator->AddAnimation("RELOAD_RIFLE", 592, 592 + 60, 20.0f,false);
        upperAnimator->AddAnimation("WEAPON_LOWER", 556, 556 + 8, 15.0f, false);
        upperAnimator->AddAnimation("WEAPON_RAISE", 564, 564 + 8, 15.0f, false);
        upperAnimator->AddAnimation("BANDAGE", 421, 421 + 36, 15.0f, false);


        upperAnimator->PlayAnimation("STAND_RIFLE");

        std::string skinName = szFileName + "lower_free_b.png";
        skin = LOAD_TEXTURE(skinName.c_str(), "orion_lower_skin");

        lower.SetTexture(0, skin.id);
        lower.SetTexture(1, skin.id);

        skinName = szFileName + "upper_free_b.png";
        skin = LOAD_TEXTURE(skinName.c_str(), "orion_upper_skin");
        upper.SetTexture(1, skin.id);

        skinName = szFileName + "arms_free_b.png";
        skin = LOAD_TEXTURE(skinName.c_str(), "orion_arms_skin");
        upper.SetTexture(0, skin.id);
        upper.SetTexture(2, skin.id);
        upper.SetTexture(3, skin.id);
        upper.SetTexture(4, skin.id);


        skinName = szFileName + "head_free_b.png";
        skin = LOAD_TEXTURE(skinName.c_str(), "orion_head_skin");

        head.SetTexture(0, skin.id);
        head.SetTexture(0, skin.id);


        skin = LOAD_TEXTURE("models/mp5k/mp5k01.png", "mkp5k1");
        weapon.SetTexture(0, skin.id);
        skin = LOAD_TEXTURE("models/mp5k/mp5k02.png", "mkp5k2");
        weapon.SetTexture(1, skin.id);


        transform.Rotate(Vector3{ -90, 0, 0 });

        lower.setLink("tag_torso", &upper);

        upper.setLink("tag_head", &head);

        upper.setLink("tag_weapon", &weapon);


        return true;
    }


    void Update(float dt)
    {
        positioncenter = transform.GetWorldPosition();
        lower.update(dt);
        upper.update(dt);
    }

    void Render(Shader& shader)
    {


        lower.render(shader, transform.GetWorldMatrix(), MatrixIdentity());
     //   DrawSphere(positioncenter, radius, RED);
    }
};

Scene scene;
ViewFrustum frustum;
Collider world;
Octree quad;
BSP map;
Decal3D decals;
ParticleSystem particleSystem;
EffectEmitter muzzleFlash;
EffectEmitter shockWave;
Model barrel;

struct MainScreen : public Screen
{


    ViewFrustum frustum;
    CameraFPS camera;


    Texture2D decal;

    LoadMD3 weapon;
    LoadMD3 wlinks[8];
    Player player;


    Shader modelShader, shaderParticles, mapShader;

    int blendLoc;
    float blend;

    MainScreen(): Screen("Main", false)
    {


        barrel = LoadModel("models/barrel/barrel.glb");
        Texture2D texture =
            LOAD_TEXTURE("models/barrel/barrel_d.png", "barrel");
        barrel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
            texture; // Setmap diffuse texture
        barrel.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = texture;


        scene.AddNode(&barrel, { 37.5f, 26.0f, 6.6f }, { 0.03f, 0.03f, 0.03f });
        scene.AddNode(&barrel, { 36.5f, 26.0f, 9.6f }, { 0.03f, 0.03f, 0.03f });
        scene.AddNode(&barrel, { -18.7f, 13.4f, 10.4f },{ 0.03f, 0.03f, 0.03f });
        scene.AddNode(&barrel, { 39.5f, 26.0f, 8.4f }, { 0.03f, 0.03f, 0.03f });
        scene.AddNode(&barrel, { -1.1f, 26.0f, 7.6f }, { 0.03f, 0.03f, 0.03f });


        mapShader = LOAD_SHADER("lightmap", "shaders/lightmap.vs",
                                "shaders/lightmap.fs");
        int blendLoc = GetShaderLocation(mapShader, "lightmapBlend");


        map.loadFromFile("maps/oa_rpg3dm2.bsp");
        //     map.loadFromFile("maps/egyptians.bsp");


        const std::vector<BSPSurface> surfaces = map.getSurfaces();


        quad.setWorldBounds(map.getBounds());


        for (u32 i = 0; i < surfaces.size(); i++)
        {
            const BSPSurface& surface = surfaces[i];
            const std::vector<Vector3>& verts = surface.vertices;
            const std::vector<u16>& indices = surface.indices;


            for (size_t i = 0; i + 2 < indices.size(); i += 3)
            {
                quad.addTriangle(verts[indices[i + 0]], verts[indices[i + 1]],
                                 verts[indices[i + 2]]);
            }
        }

        quad.rebuild();

        world.setCollisionSelector(&quad);
        world.setScene(&scene);


        float blend = 0.5f;
        SetShaderValue(mapShader, blendLoc, &blend, SHADER_UNIFORM_FLOAT);


        weapon.Load("models/mp5k/mp5k_hold.MD3");

        texture = LOAD_TEXTURE("models/hand_brown.png", "hand_brown");

        weapon.SetTexture(0, texture.id);

        weapon.getAnimator()->AddAnimation("all", 0, 85, 15.0f, true);
        weapon.getAnimator()->AddAnimation("idle", 0, 1, 15.0f, false);
        weapon.getAnimator()->AddAnimation("fire", 0, 10, 60.0f, false);
        weapon.getAnimator()->AddAnimation("reload", 11, 85, 33.0f, false);
        weapon.getAnimator()->PlayAnimation("idle", true);

        LogInfo("Loading beretta...");
        const float FACTOR = 0.027f;
        wlinks[0].Load("models/mp5k/mp5k_view_main.MD3", FACTOR);
        weapon.setLink("tag_main", &wlinks[0]);


        wlinks[1].Load("models/mp5k/mp5k_view_slide.MD3", FACTOR);
        wlinks[2].Load("models/mp5k/mp5k_view_clip.MD3", FACTOR);
        wlinks[3].Load("models/mp5k/mp5k_view_trigger.MD3", FACTOR);
        wlinks[4].Load("models/mp5k/mp5k_view_bolt.MD3", FACTOR);
        wlinks[5].Load("models/mp5k/mp5k_view_mode.MD3", FACTOR);
        weapon.setLink("tag_slide", &wlinks[1]);
        weapon.setLink("tag_clip", &wlinks[2]);
        weapon.setLink("tag_trigger", &wlinks[3]);
        weapon.setLink("tag_bolt", &wlinks[4]);
        weapon.setLink("tag_mode", &wlinks[5]);


        texture = LOAD_TEXTURE("models/mp5k/mp5k01.png", "mp5k01");


        wlinks[0].SetTexture(0, texture.id);

        texture = LOAD_TEXTURE("models/mp5k/mp5k02.png", "mp5k02");

        wlinks[0].SetTexture(1, texture.id);
        wlinks[1].SetTexture(0, texture.id);
        wlinks[2].SetTexture(0, texture.id);
        wlinks[3].SetTexture(0, texture.id);
        wlinks[4].SetTexture(0, texture.id);
        wlinks[5].SetTexture(0, texture.id);

        camera.Init((Vector3){ 5.0f, 30.0f, -5.0f });
        modelShader = LOAD_SHADER("models", "shaders/md3.vs", "shaders/md3.fs");


        decal = LOAD_TEXTURE("images/bulletdecal.png", "decal");
        LOAD_TEXTURE("images/Flash1_1.png", "flash_a");
        LOAD_TEXTURE("images/Flash1_2.png", "flash_b");
        LOAD_TEXTURE("images/pwave.png", "pwave");


        particleSystem.Init(
            LOAD_TEXTURE("images/fire_particle.png", "particle"), 500);
        muzzleFlash.Init(LOAD_TEXTURE("images/Flash1_1.png", "flash_a"), 50);
        shockWave.Init(LOAD_TEXTURE("images/pwave.png", "pwave"), 50);

        shaderParticles = LOAD_SHADER("partciles", "shaders/particles.vs",
                                      "shaders/particles.fs");
        int textureLoc = GetShaderLocation(shaderParticles, "texture0");
        int index = 0;
        SetShaderValue(shaderParticles, textureLoc, &index,
                       SHADER_UNIFORM_FLOAT);

        player.Load("models/orion/","models/p90/p90_1.md3");
        player.transform.SetLocalPosition(Vector3{  -28.7f, 15.4f, 10.4f });
        float offSet = 2.9f;
        player.transform.SetLocalPosition(Vector3{ 29.86f, 17.6f + offSet, 103.75f});


        player.transform.SetLocalScale(Vector3{ 0.6f, 0.6f, 0.6f });
    }
    void OnEnter() override { LogInfo("Entrou em %s", name.c_str()); }
    void OnExit() override { LogInfo("Saiu de %s", name.c_str()); }
    void Update(float dt) override
    {
        if (IsKeyDown(KEY_P)) blend += 0.01f;
        if (IsKeyDown(KEY_I)) blend -= 0.01f;


        camera.Update(dt, world);
        player.Update(dt);
        shockWave.Update(dt);
        muzzleFlash.Update(dt);
        particleSystem.Update(dt);
        weapon.update(dt);
        isPlayerWalking = camera.IsMoving();
    }
    void Render() override
    {

        ClearBackground(SKYBLUE);


        BeginMode3D(camera.camera);

        rlEnableBackfaceCulling();
        rlEnableDepthTest();


        frustum.update();
        blend = Clamp(blend, 0.0f, 1.0f);
        rlEnableShader(mapShader.id);
        SetShaderValue(mapShader, blendLoc, &blend, SHADER_UNIFORM_FLOAT);
        map.render(frustum, mapShader);

        scene.Render();


        int w = GetScreenWidth() / 2;
        int h = GetScreenHeight() / 2;


        Ray ray = GetMouseRay((Vector2){ (float)w, (float)h }, camera.camera);

        float rotate = -4.7f;

        Matrix matView = rlGetMatrixModelview();
        Matrix scaleMat = MatrixScale(0.090f, 0.090f, 0.090f);
        Matrix rotMat = QuaternionToMatrix(QuaternionFromEuler(80, rotate, 0));
        Vector3 bobbingOffset = CalculateWeaponBobbing(GetFrameTime());

        Matrix transMat =
            MatrixTranslate(0.0f + bobbingOffset.x, 0.0f + bobbingOffset.y,
                            -0.2f + bobbingOffset.z);

        Matrix local =
            MatrixMultiply(MatrixMultiply(scaleMat, rotMat), transMat);
        Matrix cano = MatrixMultiply(local, MatrixInvert(matView));
        Vector3 tag = Vector3Transform(weapon.GetTagPosition(0), cano);
        muzzleFlash.SetPosition(tag);

        if (IsKeyDown(KEY_M))
        {
            Ray pick = GetMouseRay(GetMousePosition(), camera.camera);
            std::vector<const Triangle*> tris =
                quad.getCandidates(pick, 1000.0f);

            for (const Triangle* tri : tris)
            {
                RayCollision status = GetRayCollisionTriangle(
                    ray, tri->pointA, tri->pointB, tri->pointC);
                if (status.hit && status.distance < 100)
                {
                    DrawSphere(status.point, 0.1f, RED);
                    LogInfo("Picked: %f %f %f", status.point.x, status.point.y,
                            status.point.z);
                }
            }
        }


        rlEnableShader(modelShader.id);

        
 
        player.Render(modelShader);
        MD3Animator* animator = weapon.getAnimator();

      

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !animator->IsPlaying())
        {
            animator->PlayAnimationThen("fire", "idle");
            PLAY_SOUND("shoot");

            particleSystem.EmitBulletTrail(tag, Vector3Scale(ray.direction, 1000.0f), 200);
              

            muzzleFlash.CreateMuzzleFlash(tag, GetRandomValue(0, 360), 0.18f,
                                          0.1f);
            muzzleFlash.CreateMuzzleFlash(tag, GetRandomValue(0, 360), 0.1f,
                                          0.2f);
 

            if (player.pick(ray))
            {
               
                player.Kill();
            }


            PickData data;
            if (scene.collide(ray, 100.0f, &data))
            {
                decals.AddDecal(data.intersectionPoint,
                                data.intersectionTriangle.normal,
                                &data.intersectionTriangle, decal, 0.2f, WHITE,
                                data.node->GetID());
                particleSystem.EmitBulletImpact(
                    data.intersectionPoint, data.intersectionTriangle.normal,
                    20);


                data.node->tag++;
                if (data.node->tag == 3)
                {
                    PLAY_SOUND("explosion_1");
                    particleSystem.EmitBarrelExplosion(data.intersectionPoint,1.0f);
                    shockWave.CreateShockwave(
                        Vector3Add(data.node->localPosition, { 0, 0.9, 0 }),
                        8.0f, 2.5f, 0.5f, 10.0f);
                    camera.StartShake(0.1f, 0.3f);
                    data.node->SetVisible(false);
                    decals.ClearByID(data.node->GetID());
                }
            }


            float closestDistance = FLT_MAX;
            const Triangle* closestTri = nullptr;
            RayCollision closestHit = { 0 };
            std::vector<const Triangle*> tris =quad.getCandidates(ray, 1000.0f);

            // Encontra o triângulo mais próximo
            for (const Triangle* tri : tris)
            {
                RayCollision status = GetRayCollisionTriangle(
                    ray, tri->pointA, tri->pointB, tri->pointC);
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


                decals.AddDecal(closestHit.point, closestHit.normal, closestTri,
                                decal, 0.2f, WHITE);

                particleSystem.EmitBulletImpact(closestHit.point,
                                                closestHit.normal, 20);


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


        weapon.render(modelShader, local, MatrixInvert(matView));
        Matrix matProjection = rlGetMatrixProjection();
        Matrix matModelView = MatrixMultiply(MatrixIdentity(), matView);
        Matrix matModelViewProjection =
            MatrixMultiply(matModelView, matProjection);


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
        DrawText(TextFormat("Cound: %d", map.getViewCount()), 10, 80, 16,
                 DARKGRAY);
        DrawText(TextFormat("Frame: %d", animator->GetFrame()), 10, 100, 16,
                 DARKGRAY);


        if (IsCursorHidden())
        {
            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();
            DrawLine(screenWidth / 2 - 10, screenHeight / 2,
                     screenWidth / 2 + 10, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, screenHeight / 2 - 10, screenWidth / 2,
                     screenHeight / 2 + 10, WHITE);
        }
    }
};


struct MenuPopup : public Screen
{
    MenuPopup(): Screen("MenuPause", true) {}
    void Render() override
    {
        DrawRectangle(200, 200, GetScreenWidth() - 400, GetScreenHeight() - 400,
                      Fade(BLACK, 0.5f));
        DrawText("PAUSE", (GetScreenWidth()) / 2, (GetScreenHeight()) / 2, 40,
                 RAYWHITE);
    }
};


int main()
{
    InitWindow(1280, 720, "ScreenManager Sample");
    InitAudioDevice();
    SetTargetFPS(60);
    DisableCursor();


    LOAD_SOUND("sounds/mp5k.wav", "shoot");
    LOAD_SOUND("sounds/clipin.wav", "clipin");
    LOAD_SOUND("sounds/clipout.wav", "clipout");
    LOAD_SOUND("sounds/cock1.wav", "cock1");
    LOAD_SOUND("sounds/cock2.wav", "cock2");
    LOAD_SOUND("sounds/Explo_1.wav", "explosion_1");


    auto* jogo = new MainScreen();
    auto* pause = new MenuPopup();

    auto& SM = ScreenManager::Get();
    SM.SetOwnScreens(true); // opcional: o manager destrói as screens no fim
    SM.Register(jogo);
    SM.Register(pause);

    SM.Set("Main", true, 0.4f);

    while (!WindowShouldClose())
    {
        // Input global
     //   if (IsKeyPressed(KEY_P)) SM.Push("MenuPause", true, 0.25f);
      //  if (IsKeyPressed(KEY_O)) SM.Pop(true, 0.25f);

        SM.HandleInput();
        SM.Update(GetFrameTime());

        BeginDrawing();
        SM.Render();

        DrawFPS(10, 10);
        EndDrawing();
    }

    scene.Clear();
    muzzleFlash.Release();
    shockWave.Release();
    particleSystem.Release();
    UnloadModel(barrel);
    CloseAudioDevice();
    scene.Clear();
    UNLOAD_ASSETS();
    CloseWindow();
    return 0;
}
