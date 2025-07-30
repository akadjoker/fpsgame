#pragma once
#include "Config.hpp"
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <vector>
#include <algorithm>
#include <random>
#include <functional>
#include "batch.hpp" 
 
struct EmissionConfig
{
    Vector3 position = { 0, 0, 0 };
    Vector3 direction = { 0, 1, 0 };
    Vector3 directionVariance = { 0.5f, 0.5f, 0.5f };

    float speed = 2.0f;
    float speedVariance = 1.0f;

    float life = 2.0f;
    float lifeVariance = 0.5f;

    float size = 0.1f;
    float sizeVariance = 0.05f;

    Color startColor = WHITE;
    Color endColor = { 255, 255, 255, 0 };

    Vector3 gravity = { 0, -9.8f, 0 };
    float drag = 0.98f;

    int burstCount = 1;
    float emissionRate = 30.0f;
    bool continuous = true;
};

enum class EmissionShape
{
    POINT,
    SPHERE,
    CONE,
    BOX,
    CIRCLE
};
 
struct Particle
{
    Vector3 position = { 0, 0, 0 };
    Vector3 velocity = { 0, 0, 0 };
    Vector3 acceleration = { 0, 0, 0 };

    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 1.0f;
    float rotation = 0.0f;
    float angularVelocity = 0.0f;

    Color startColor = WHITE;
    Color currentColor = WHITE;
    Color endColor = { 255, 255, 255, 0 };

    bool active = false;
    int poolIndex = -1;

    void UpdateColor();
};

 
class ParticleModifier {
public:
    virtual ~ParticleModifier() = default;
    virtual void Apply(Particle& particle, float deltaTime) = 0;
};

class GravityModifier : public ParticleModifier {
private:
    Vector3 gravity;
public:
    explicit GravityModifier(Vector3 gravity);
    void Apply(Particle& particle, float deltaTime) override;
};

class TurbulenceModifier : public ParticleModifier {
private:
    float strength;
    float frequency;
public:
    TurbulenceModifier(float strength = 1.0f, float frequency = 0.5f);
    void Apply(Particle& particle, float deltaTime) override;
};

class VortexModifier : public ParticleModifier {
private:
    Vector3 center;
    float strength;
    Vector3 axis;
public:
    VortexModifier(Vector3 center, float strength, Vector3 axis = { 0, 1, 0 });
    void Apply(Particle& particle, float deltaTime) override;
};

class AttractorModifier : public ParticleModifier {
private:
    Vector3 center;
    float strength;
    bool repulse;
public:
    AttractorModifier(Vector3 center, float strength, bool repulse = false);
    void Apply(Particle& particle, float deltaTime) override;
};

 
class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::vector<int> freeIndices;
    std::vector<ParticleModifier*> modifiers;

    int maxParticles = 0;
    float timeAccumulator = 0.0f;
    Texture2D texture{};

    std::random_device rd;
    std::mt19937 gen{ rd() };
    std::uniform_real_distribution<float> uniformDist{ 0.0f, 1.0f };

    EmissionConfig config{};
    EmissionShape shape = EmissionShape::POINT;
    Batch batch;
    bool isActive = true;

    void InitializePool();
    Vector3 GenerateEmissionPosition();
    Vector3 GenerateRandomDirection();

public:
    ParticleSystem();
    ~ParticleSystem();

    void Init(Texture2D texture, int maxParticles = 1000);
    void Release();

    void SetEmissionConfig(const EmissionConfig& newConfig);
    void SetEmissionShape(EmissionShape newShape);
    void SetActive(bool active);

    void AddModifier(ParticleModifier* modifier);
    void ClearModifiers();

    void EmitParticles(int count = -1);
    void EmitAt(Vector3 position, Vector3 direction, int count = 10,
                float spread = 0.3f, float speed = 5.0f, Color color = WHITE);

    // Efeitos compostos
    void EmitBarrelExplosion(Vector3 position, float intensity = 1.0f);
    void EmitExplosionCore(Vector3 position, float intensity = 1.0f);
    void EmitFireball(Vector3 position, float intensity = 1.0f);
    void EmitDebris(Vector3 position, float intensity = 1.0f);
    void EmitExplosionSmoke(Vector3 position, float intensity = 1.0f);
    void EmitSparksAndEmbers(Vector3 position, float intensity = 1.0f);

    // Efeitos de bala
    void EmitBulletImpact(Vector3 position, Vector3 normal, int count = 15);
    void EmitBulletTrail(Vector3 startPos, Vector3 endPos, int count = 20);
    void EmitMuzzleFlash(Vector3 position, Vector3 direction, float intensity = 1.0f);
    void EmitExplosion(Vector3 position, Vector3 direction, float intensity = 1.0f);

    void Update(float deltaTime);
    void Draw(Camera3D camera);

    int GetActiveParticleCount() const;
    int GetMaxParticles() const;

    void Clear();

    // Presets
    static EmissionConfig CreateFireConfig();
    static EmissionConfig CreateSmokeConfig();
    static EmissionConfig CreateSparkConfig();
    static EmissionConfig CreateSnowConfig();
    static EmissionConfig CreateExplosionConfig();
    static EmissionConfig CreateBulletImpactConfig();
    static EmissionConfig CreateMuzzleFlashConfig();
    static EmissionConfig CreateBulletTrailConfig();
    static EmissionConfig CreateRicochetConfig();
};

// --------------------------------------------------------
// Efeitos “muzzle flash / shockwave” (billboards)
// --------------------------------------------------------
enum EffectType
{
    BILLBOARD,
    YUP_PLANE
};

struct Effect
{
    Vector3 position{};
    EffectType type = BILLBOARD;
    float rotation = 0.0f;
    float size = 0.0f;
    float maxSize = 0.0f;
    float life = 0.0f;
    float maxLife = 0.0f;
    Color color = WHITE;
    float speed = 0.0f;
    u32 id = 0;

    Effect(Vector3 pos, EffectType t, float rot, float lifeTime, float sz, float speed, u32 id = 0);
};

class EffectEmitter
{
private:
    std::vector<Effect> effects;
    Texture2D texture{};
    Batch batch;

public:
    EffectEmitter() = default;
    ~EffectEmitter() = default;

    void Init(Texture2D tex, u32 maxParticles = 10);
    void Release();

    void SetPosition(Vector3 pos);

    void CreateMuzzleFlash(Vector3 position, float rotation, float lifeTime, float size, u32 id = 0);
    void CreateShockwave(Vector3 position, float maxSize, float lifeTime, float sz, float speed, u32 id = 0);

    void ClearByID(u32 id);

    void Update(float deltaTime);
    void Draw(Camera3D camera);

    int GetEffectCount() const;
    void Clear();
};
