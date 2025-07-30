 
#include "emitter.hpp"

 
void Particle::UpdateColor()
{
    float t = 1.0f - (life / maxLife);
    currentColor.r = (u8)Lerp((float)startColor.r, (float)endColor.r, t);
    currentColor.g = (u8)Lerp((float)startColor.g, (float)endColor.g, t);
    currentColor.b = (u8)Lerp((float)startColor.b, (float)endColor.b, t);
    currentColor.a = (u8)Lerp((float)startColor.a, (float)endColor.a, t);
}

 
GravityModifier::GravityModifier(Vector3 gravity) : gravity(gravity) {}

void GravityModifier::Apply(Particle& particle, float /*deltaTime*/)
{
    particle.acceleration = Vector3Add(particle.acceleration, gravity);
}

TurbulenceModifier::TurbulenceModifier(float strength, float frequency)
    : strength(strength), frequency(frequency) {}

void TurbulenceModifier::Apply(Particle& particle, float /*deltaTime*/)
{
    float time = (float)GetTime() * frequency;
    Vector3 turbulence = {
        sinf(time + particle.position.x) * strength,
        cosf(time + particle.position.y) * strength,
        sinf(time + particle.position.z) * strength
    };
    particle.acceleration = Vector3Add(particle.acceleration, turbulence);
}

VortexModifier::VortexModifier(Vector3 center, float strength, Vector3 axis)
    : center(center), strength(strength), axis(Vector3Normalize(axis)) {}

void VortexModifier::Apply(Particle& particle, float /*deltaTime*/)
{
    Vector3 toCenter = Vector3Subtract(center, particle.position);
    float distance = Vector3Length(toCenter);

    if (distance > 0.1f)
    {
        Vector3 tangent = Vector3CrossProduct(axis, toCenter);
        tangent = Vector3Normalize(tangent);

        float force = strength / (distance * distance);
        Vector3 vortexForce = Vector3Scale(tangent, force);

        particle.acceleration = Vector3Add(particle.acceleration, vortexForce);
    }
}

AttractorModifier::AttractorModifier(Vector3 center, float strength, bool repulse)
    : center(center), strength(strength), repulse(repulse) {}

void AttractorModifier::Apply(Particle& particle, float /*deltaTime*/)
{
    Vector3 toCenter = Vector3Subtract(center, particle.position);
    float distance = Vector3Length(toCenter);

    if (distance > 0.1f)
    {
        Vector3 direction = Vector3Normalize(toCenter);
        float force = strength / (distance * distance);

        if (repulse) direction = Vector3Negate(direction);

        Vector3 attractionForce = Vector3Scale(direction, force);
        particle.acceleration = Vector3Add(particle.acceleration, attractionForce);
    }
}
 
ParticleSystem::ParticleSystem() = default;

ParticleSystem::~ParticleSystem()
{
    for (ParticleModifier* modifier : modifiers) delete modifier;
    modifiers.clear();
}

void ParticleSystem::InitializePool()
{
    particles.resize(maxParticles);
    freeIndices.reserve(maxParticles);
    freeIndices.clear();

    for (int i = 0; i < maxParticles; ++i)
    {
        particles[i].poolIndex = i;
        particles[i].active = false;
        freeIndices.push_back(i);
    }
}

Vector3 ParticleSystem::GenerateEmissionPosition()
{
    switch (shape)
    {
        case EmissionShape::SPHERE:
        {
            float theta = uniformDist(gen) * 2.0f * PI;
            float phi = acosf(1.0f - 2.0f * uniformDist(gen));
            float radius = powf(uniformDist(gen), 1.0f / 3.0f) * config.directionVariance.x;

            return {
                config.position.x + radius * sinf(phi) * cosf(theta),
                config.position.y + radius * sinf(phi) * sinf(theta),
                config.position.z + radius * cosf(phi)
            };
        }
        case EmissionShape::BOX:
        {
            return {
                config.position.x + (uniformDist(gen) - 0.5f) * config.directionVariance.x,
                config.position.y + (uniformDist(gen) - 0.5f) * config.directionVariance.y,
                config.position.z + (uniformDist(gen) - 0.5f) * config.directionVariance.z
            };
        }
        case EmissionShape::CIRCLE:
        {
            float angle = uniformDist(gen) * 2.0f * PI;
            float radius = sqrtf(uniformDist(gen)) * config.directionVariance.x;

            return { config.position.x + radius * cosf(angle),
                     config.position.y,
                     config.position.z + radius * sinf(angle) };
        }
        case EmissionShape::CONE:
        {
            float angle = uniformDist(gen) * 2.0f * PI;
            float height = uniformDist(gen) * config.directionVariance.y;
            float radius = height * tanf(config.directionVariance.x);

            return { config.position.x + radius * cosf(angle),
                     config.position.y + height,
                     config.position.z + radius * sinf(angle) };
        }
        default: // POINT
            return config.position;
    }
}

Vector3 ParticleSystem::GenerateRandomDirection()
{
    Vector3 baseDir = config.direction;
    Vector3 randomOffset = {
        (uniformDist(gen) - 0.5f) * config.directionVariance.x,
        (uniformDist(gen) - 0.5f) * config.directionVariance.y,
        (uniformDist(gen) - 0.5f) * config.directionVariance.z
    };
    return Vector3Normalize(Vector3Add(baseDir, randomOffset));
}

void ParticleSystem::Init(Texture2D tex, int maxParticles)
{
    this->texture = tex;
    this->maxParticles = maxParticles;
    batch.Init((u32)maxParticles);
    InitializePool();
}

void ParticleSystem::Release()
{
    batch.Release();
}

void ParticleSystem::SetEmissionConfig(const EmissionConfig& newConfig)
{
    config = newConfig;
}

void ParticleSystem::SetEmissionShape(EmissionShape newShape)
{
    shape = newShape;
}

void ParticleSystem::SetActive(bool active)
{
    isActive = active;
}

void ParticleSystem::AddModifier(ParticleModifier* modifier)
{
    modifiers.push_back(modifier);
}

void ParticleSystem::ClearModifiers()
{
    for (ParticleModifier* modifier : modifiers) delete modifier;
    modifiers.clear();
}

void ParticleSystem::EmitParticles(int count)
{
    if (count == -1) count = config.burstCount;

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();

        Particle& p = particles[index];

        p.position = GenerateEmissionPosition();
        p.velocity = Vector3Scale(GenerateRandomDirection(),
                                  config.speed + (uniformDist(gen) - 0.5f) * config.speedVariance);

        p.acceleration = { 0, 0, 0 };
        p.maxLife = config.life + (uniformDist(gen) - 0.5f) * config.lifeVariance;
        p.life = p.maxLife;
        p.size = config.size + (uniformDist(gen) - 0.5f) * config.sizeVariance;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 180.0f;

        p.startColor = config.startColor;
        p.endColor = config.endColor;
        p.currentColor = config.startColor;

        p.active = true;
    }
}

void ParticleSystem::EmitAt(Vector3 position, Vector3 direction, int count,
                            float spread, float speed, Color color)
{
    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();

        Particle& p = particles[index];

        p.position = position;

        Vector3 randomOffset = {
            (uniformDist(gen) - 0.5f) * spread,
            (uniformDist(gen) - 0.5f) * spread,
            (uniformDist(gen) - 0.5f) * spread
        };

        Vector3 finalDirection = Vector3Add(direction, randomOffset);
        p.velocity = Vector3Scale(Vector3Normalize(finalDirection),
                                  speed + (uniformDist(gen) - 0.5f) * (speed * 0.3f));

        p.acceleration = { 0, 0, 0 };
        p.maxLife = config.life + (uniformDist(gen) - 0.5f) * config.lifeVariance;
        p.life = p.maxLife;
        p.size = config.size + (uniformDist(gen) - 0.5f) * config.sizeVariance;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 180.0f;

        p.startColor = color;
        p.endColor = config.endColor;
        p.currentColor = color;

        p.active = true;
    }
}

// -------- Efeitos compostos de explosão ----------
void ParticleSystem::EmitBarrelExplosion(Vector3 position, float intensity)
{
    EmitExplosionCore(position, intensity);
    EmitFireball(position, intensity);
    EmitDebris(position, intensity);
    EmitExplosionSmoke(position, intensity);
    EmitSparksAndEmbers(position, intensity);
}

void ParticleSystem::EmitExplosionCore(Vector3 position, float intensity)
{
    Color coreColor = { 255, 255, 255, 255 };
    int count = (int)(8 * intensity);

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f
        };

        p.velocity = Vector3Scale(Vector3Normalize(randomDir), 15.0f * intensity);
        p.acceleration = Vector3Scale(p.velocity, -8.0f);

        p.maxLife = 0.1f + uniformDist(gen) * 0.05f;
        p.life = p.maxLife;
        p.size = (0.3f + uniformDist(gen) * 0.2f) * intensity;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 1800.0f;

        p.startColor = coreColor;
        p.endColor = { 255, 100, 0, 0 };
        p.currentColor = coreColor;

        p.active = true;
    }
}

void ParticleSystem::EmitFireball(Vector3 position, float intensity)
{
    Color fireColor = { 255, 120, 0, 255 };
    int count = (int)(25 * intensity);

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f
        };

        float speed = 8.0f + uniformDist(gen) * 6.0f;
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), speed * intensity);
        p.acceleration = { 0, 1.0f, 0 };

        p.maxLife = 0.8f + uniformDist(gen) * 0.4f;
        p.life = p.maxLife;
        p.size = (0.15f + uniformDist(gen) * 0.1f) * intensity;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 360.0f;

        p.startColor = fireColor;
        p.endColor = { 80, 20, 20, 0 };
        p.currentColor = fireColor;

        p.active = true;
    }
}

void ParticleSystem::EmitDebris(Vector3 position, float intensity)
{
    Color debrisColor = { 120, 80, 40, 255 };
    int count = (int)(30 * intensity);

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            uniformDist(gen) * 1.5f + 0.2f,
            (uniformDist(gen) - 0.5f) * 2.0f
        };

        float speed = 12.0f + uniformDist(gen) * 8.0f;
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), speed * intensity);
        p.acceleration = { 0, -9.8f, 0 };

        p.maxLife = 2.0f + uniformDist(gen) * 1.5f;
        p.life = p.maxLife;
        p.size = 0.04f + uniformDist(gen) * 0.06f;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 720.0f;

        p.startColor = debrisColor;
        p.endColor = { 60, 40, 20, 0 };
        p.currentColor = debrisColor;

        p.active = true;
    }
}

void ParticleSystem::EmitExplosionSmoke(Vector3 position, float intensity)
{
    Color smokeColor = { 40, 40, 40, 180 };
    int count = (int)(20 * intensity);

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = Vector3Add(position, { (uniformDist(gen) - 0.5f) * 0.5f,
                                            uniformDist(gen) * 0.3f,
                                            (uniformDist(gen) - 0.5f) * 0.5f });

        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 0.8f,
            0.5f + uniformDist(gen) * 0.5f,
            (uniformDist(gen) - 0.5f) * 0.8f
        };

        p.velocity = Vector3Scale(Vector3Normalize(randomDir), 3.0f + uniformDist(gen) * 2.0f);
        p.acceleration = { 0, 0.5f, 0 };

        p.maxLife = 3.0f + uniformDist(gen) * 2.0f;
        p.life = p.maxLife;
        p.size = (0.2f + uniformDist(gen) * 0.3f) * intensity;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 90.0f;

        p.startColor = smokeColor;
        p.endColor = { 80, 80, 80, 0 };
        p.currentColor = smokeColor;

        p.active = true;
    }
}

void ParticleSystem::EmitSparksAndEmbers(Vector3 position, float intensity)
{
    Color sparkColor = { 255, 200, 50, 255 };
    int count = (int)(40 * intensity);

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            uniformDist(gen) * 1.0f + 0.3f,
            (uniformDist(gen) - 0.5f) * 2.0f
        };

        float speed = 15.0f + uniformDist(gen) * 10.0f;
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), speed * intensity);
        p.acceleration = { 0, -9.8f, 0 };

        p.maxLife = 1.2f + uniformDist(gen) * 0.8f;
        p.life = p.maxLife;
        p.size = 0.02f + uniformDist(gen) * 0.03f;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 1440.0f;

        p.startColor = sparkColor;
        p.endColor = { 150, 50, 50, 0 };
        p.currentColor = sparkColor;

        p.active = true;
    }
}

// -------- Efeitos de bala ----------
void ParticleSystem::EmitBulletImpact(Vector3 position, Vector3 normal, int count)
{
    Color sparkColor = { 255, 255, 100, 255 };
    float impactSpeed = 8.5f;
    float impactSpread = 0.8f;

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            normal.x + (uniformDist(gen) - 0.5f) * impactSpread,
            normal.y + (uniformDist(gen) - 0.5f) * impactSpread,
            normal.z + (uniformDist(gen) - 0.5f) * impactSpread
        };

        p.velocity = Vector3Scale(Vector3Normalize(randomDir),
                                  impactSpeed + (uniformDist(gen) - 0.5f) * 4.0f);

        p.acceleration = { 0, -0.1f, 0 };
        p.maxLife = 0.2f + uniformDist(gen) * 0.4f;
        p.life = p.maxLife;
        p.size = 0.05f + uniformDist(gen) * 0.03f;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 360.0f;

        p.startColor = sparkColor;
        p.endColor = { 255, 50, 50, 0 };
        p.currentColor = sparkColor;

        p.active = true;
    }
}

void ParticleSystem::EmitBulletTrail(Vector3 startPos, Vector3 endPos, int count)
{
    Color trailColor = { 255, 255, 200, 150 };
    Vector3 direction = Vector3Normalize(Vector3Subtract(endPos, startPos));

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        float t = uniformDist(gen);
        p.position = Vector3Lerp(startPos, endPos, t);

        Vector3 perpendicular = { -direction.z, 0, direction.x };
        if (Vector3Length(perpendicular) < 0.1f) perpendicular = { 0, 1, 0 };

        Vector3 randomPerp = Vector3Scale(Vector3Normalize(perpendicular),
                                          (uniformDist(gen) - 0.5f) * 2.0f);
        p.velocity = randomPerp;

        p.acceleration = { 0, 0, 0 };
        p.maxLife = 0.3f + uniformDist(gen) * 0.2f;
        p.life = p.maxLife;
        p.size = 0.02f + uniformDist(gen) * 0.01f;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 180.0f;

        p.startColor = trailColor;
        p.endColor = { 255, 255, 200, 0 };
        p.currentColor = trailColor;

        p.active = true;
    }
}

void ParticleSystem::EmitMuzzleFlash(Vector3 position, Vector3 direction, float intensity)
{
    Color flashColor = { 255, 200, 100, 255 };
    int count = (int)(15 * intensity);
    float speed = 6.0f * intensity;

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            direction.x + (uniformDist(gen) - 0.5f) * 0.4f,
            direction.y + (uniformDist(gen) - 0.5f) * 0.4f,
            direction.z + (uniformDist(gen) - 0.5f) * 0.4f
        };

        p.velocity = Vector3Scale(Vector3Normalize(randomDir),
                                  speed + (uniformDist(gen) - 0.5f) * 3.0f);

        p.acceleration = { 0, 0, 0 };
        p.maxLife = 0.15f + uniformDist(gen) * 0.1f;
        p.life = p.maxLife;
        p.size = 0.08f + uniformDist(gen) * 0.04f;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 720.0f;

        p.startColor = flashColor;
        p.endColor = { 100, 50, 50, 0 };
        p.currentColor = flashColor;

        p.active = true;
    }
}

void ParticleSystem::EmitExplosion(Vector3 position, Vector3 direction, float intensity)
{
    Color flashColor = { 255, 200, 100, 255 };
    int count = (int)(15 * intensity);
    float speed = 6.0f * intensity;

    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back(); freeIndices.pop_back();
        Particle& p = particles[index];

        p.position = position;

        Vector3 randomDir = {
            direction.x + (uniformDist(gen) - 0.5f) * 0.4f,
            direction.y + (uniformDist(gen) - 0.5f) * 0.4f,
            direction.z + (uniformDist(gen) - 0.5f) * 0.4f
        };

        p.velocity = Vector3Scale(Vector3Normalize(randomDir),
                                  speed + (uniformDist(gen) - 0.5f) * 3.0f);

        p.acceleration = { 0, 0, 0 };
        p.maxLife = 0.15f + uniformDist(gen) * 0.1f;
        p.life = p.maxLife;
        p.size = 0.08f + uniformDist(gen) * 0.04f;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 720.0f;

        p.startColor = flashColor;
        p.endColor = { 100, 50, 50, 0 };
        p.currentColor = flashColor;

        p.active = true;
    }
}

void ParticleSystem::Update(float deltaTime)
{
    if (!isActive) return;

    if (config.continuous && config.emissionRate > 0.0f)
    {
        timeAccumulator += deltaTime;
        float emissionInterval = 1.0f / config.emissionRate;

        while (timeAccumulator >= emissionInterval)
        {
            EmitParticles(1);
            timeAccumulator -= emissionInterval;
        }
    }

    for (auto& p : particles)
    {
        if (!p.active) continue;

        p.life -= deltaTime;
        if (p.life <= 0.0f)
        {
            p.active = false;
            freeIndices.push_back(p.poolIndex);
            continue;
        }

        p.acceleration = { 0, 0, 0 };

        for (ParticleModifier* modifier : modifiers)
            modifier->Apply(p, deltaTime);

        p.velocity = Vector3Add(p.velocity, Vector3Scale(p.acceleration, deltaTime));
        p.velocity = Vector3Scale(p.velocity, config.drag);
        p.position = Vector3Add(p.position, Vector3Scale(p.velocity, deltaTime));

        p.rotation += p.angularVelocity * deltaTime;

        p.UpdateColor();
    }
}

void ParticleSystem::Draw(Camera3D camera)
{
    batch.Clear();

    Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
    Vector3 cameraRight = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
    Vector3 cameraUp    = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };

    for (const auto& p : particles)
    {
        if (!p.active) continue;

        float rad = p.rotation * DEG2RAD;
        float cosR = cosf(rad);
        float sinR = sinf(rad);

        Vector3 rotatedRight = {
            cameraRight.x * cosR - cameraUp.x * sinR,
            cameraRight.y * cosR - cameraUp.y * sinR,
            cameraRight.z * cosR - cameraUp.z * sinR
        };
        Vector3 rotatedUp = {
            cameraRight.x * sinR + cameraUp.x * cosR,
            cameraRight.y * sinR + cameraUp.y * cosR,
            cameraRight.z * sinR + cameraUp.z * cosR
        };

        Vector3 right = Vector3Scale(rotatedRight, p.size * 0.5f);
        Vector3 up    = Vector3Scale(rotatedUp,    p.size * 0.5f);

        Vector3 v1 = Vector3Add(Vector3Add(p.position, right), up);
        Vector3 v2 = Vector3Add(Vector3Subtract(p.position, right), up);
        Vector3 v3 = Vector3Subtract(Vector3Subtract(p.position, right), up);
        Vector3 v4 = Vector3Subtract(Vector3Add(p.position, right), up);

        u8 r = p.currentColor.r, g = p.currentColor.g, b = p.currentColor.b, a = p.currentColor.a;

        batch.BeginQuad();
        batch.AddVertex(v1.x, v1.y, v1.z, 1.0f, 1.0f, r, g, b, a);
        batch.AddVertex(v2.x, v2.y, v2.z, 1.0f, 0.0f, r, g, b, a);
        batch.AddVertex(v3.x, v3.y, v3.z, 0.0f, 0.0f, r, g, b, a);
        batch.AddVertex(v4.x, v4.y, v4.z, 0.0f, 1.0f, r, g, b, a);
        batch.EndQuad();
    }

    batch.Render(texture.id);
    batch.Clear();
}

int ParticleSystem::GetActiveParticleCount() const
{
    return maxParticles - (int)freeIndices.size();
}

int ParticleSystem::GetMaxParticles() const
{
    return maxParticles;
}

void ParticleSystem::Clear()
{
    for (auto& p : particles)
    {
        if (p.active)
        {
            p.active = false;
            freeIndices.push_back(p.poolIndex);
        }
    }
}

// -------- Presets ----------
EmissionConfig ParticleSystem::CreateFireConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 1, 0 };
    cfg.directionVariance = { 0.3f, 0.2f, 0.3f };
    cfg.speed = 3.0f;
    cfg.speedVariance = 1.0f;
    cfg.life = 2.0f;
    cfg.lifeVariance = 0.5f;
    cfg.startColor = { 255, 100, 50, 255 };
    cfg.endColor = { 255, 255, 100, 0 };
    cfg.emissionRate = 50.0f;
    cfg.drag = 0.95f;
    return cfg;
}

EmissionConfig ParticleSystem::CreateSmokeConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 1, 0 };
    cfg.directionVariance = { 0.5f, 0.1f, 0.5f };
    cfg.speed = 1.0f;
    cfg.speedVariance = 0.5f;
    cfg.life = 4.0f;
    cfg.lifeVariance = 1.0f;
    cfg.size = 0.2f;
    cfg.startColor = { 100, 100, 100, 180 };
    cfg.endColor = { 200, 200, 200, 0 };
    cfg.emissionRate = 20.0f;
    cfg.drag = 0.98f;
    return cfg;
}

EmissionConfig ParticleSystem::CreateSparkConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 0, 0 };
    cfg.directionVariance = { 1.0f, 1.0f, 1.0f };
    cfg.speed = 5.0f;
    cfg.speedVariance = 3.0f;
    cfg.life = 1.0f;
    cfg.lifeVariance = 0.3f;
    cfg.size = 0.05f;
    cfg.startColor = { 255, 255, 100, 255 };
    cfg.endColor = { 255, 50, 50, 0 };
    cfg.emissionRate = 100.0f;
    cfg.drag = 0.90f;
    cfg.continuous = false;
    cfg.burstCount = 50;
    return cfg;
}

EmissionConfig ParticleSystem::CreateSnowConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, -1, 0 };
    cfg.directionVariance = { 0.2f, 0.1f, 0.2f };
    cfg.speed = 1.0f;
    cfg.speedVariance = 0.3f;
    cfg.life = 8.0f;
    cfg.lifeVariance = 2.0f;
    cfg.size = 0.1f;
    cfg.sizeVariance = 0.05f;
    cfg.startColor = { 255, 255, 255, 200 };
    cfg.endColor = { 255, 255, 255, 50 };
    cfg.emissionRate = 15.0f;
    cfg.drag = 0.99f;
    return cfg;
}

EmissionConfig ParticleSystem::CreateExplosionConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 0, 0 };
    cfg.directionVariance = { 1.0f, 1.0f, 1.0f };
    cfg.speed = 8.0f;
    cfg.speedVariance = 4.0f;
    cfg.life = 1.5f;
    cfg.lifeVariance = 0.5f;
    cfg.size = 0.15f;
    cfg.sizeVariance = 0.1f;
    cfg.startColor = { 255, 150, 50, 255 };
    cfg.endColor = { 100, 50, 50, 0 };
    cfg.emissionRate = 0.0f;
    cfg.drag = 0.85f;
    cfg.continuous = false;
    cfg.burstCount = 100;
    return cfg;
}

EmissionConfig ParticleSystem::CreateBulletImpactConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 1, 0 };
    cfg.directionVariance = { 0.8f, 0.8f, 0.8f };
    cfg.speed = 8.0f;
    cfg.speedVariance = 4.0f;
    cfg.life = 0.8f;
    cfg.lifeVariance = 0.4f;
    cfg.size = 0.05f;
    cfg.sizeVariance = 0.03f;
    cfg.startColor = { 255, 255, 100, 255 };
    cfg.endColor = { 255, 50, 50, 0 };
    cfg.emissionRate = 0.0f;
    cfg.drag = 0.88f;
    cfg.continuous = false;
    cfg.burstCount = 15;
    return cfg;
}

EmissionConfig ParticleSystem::CreateMuzzleFlashConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 1, 0, 0 };
    cfg.directionVariance = { 0.4f, 0.4f, 0.4f };
    cfg.speed = 6.0f;
    cfg.speedVariance = 3.0f;
    cfg.life = 0.15f;
    cfg.lifeVariance = 0.1f;
    cfg.size = 0.08f;
    cfg.sizeVariance = 0.04f;
    cfg.startColor = { 255, 200, 100, 255 };
    cfg.endColor = { 100, 50, 50, 0 };
    cfg.emissionRate = 0.0f;
    cfg.drag = 0.92f;
    cfg.continuous = false;
    cfg.burstCount = 15;
    return cfg;
}

EmissionConfig ParticleSystem::CreateBulletTrailConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 0, 0 };
    cfg.directionVariance = { 0.1f, 0.1f, 0.1f };
    cfg.speed = 2.0f;
    cfg.speedVariance = 1.0f;
    cfg.life = 0.3f;
    cfg.lifeVariance = 0.2f;
    cfg.size = 0.02f;
    cfg.sizeVariance = 0.01f;
    cfg.startColor = { 255, 255, 200, 150 };
    cfg.endColor = { 255, 255, 200, 0 };
    cfg.emissionRate = 0.0f;
    cfg.drag = 0.95f;
    cfg.continuous = false;
    cfg.burstCount = 20;
    return cfg;
}

EmissionConfig ParticleSystem::CreateRicochetConfig()
{
    EmissionConfig cfg;
    cfg.direction = { 0, 1, 0 };
    cfg.directionVariance = { 1.2f, 0.6f, 1.2f };
    cfg.speed = 12.0f;
    cfg.speedVariance = 6.0f;
    cfg.life = 1.2f;
    cfg.lifeVariance = 0.6f;
    cfg.size = 0.03f;
    cfg.sizeVariance = 0.02f;
    cfg.startColor = { 255, 255, 150, 255 };
    cfg.endColor = { 255, 100, 50, 0 };
    cfg.emissionRate = 0.0f;
    cfg.drag = 0.82f;
    cfg.continuous = false;
    cfg.burstCount = 8;
    return cfg;
}

// --------------------------------------------------------
// EffectEmitter
// --------------------------------------------------------
Effect::Effect(Vector3 pos, EffectType t, float rot, float lifeTime, float sz, float speed, u32 id)
    : position(pos), type(t), rotation(rot), size(sz), maxSize(sz),
      life(lifeTime), maxLife(lifeTime), color(WHITE), speed(speed), id(id) {}

void EffectEmitter::Init(Texture2D tex, u32 maxParticles)
{
    texture = tex;
    batch.Init(maxParticles);
}

void EffectEmitter::Release()
{
    batch.Release();
}

void EffectEmitter::SetPosition(Vector3 pos)
{
    for (auto& effect : effects) effect.position = pos;
}

void EffectEmitter::CreateMuzzleFlash(Vector3 position, float rotation, float lifeTime, float size, u32 id)
{
    effects.emplace_back(position, BILLBOARD, rotation, lifeTime, size, 0.0f, id);
}

void EffectEmitter::CreateShockwave(Vector3 position, float maxSize, float lifeTime, float sz, float speed, u32 id)
{
    Effect shock(position, YUP_PLANE, 0.0f, lifeTime, sz, speed, id);
    shock.maxSize = maxSize;
    effects.push_back(shock);
}

void EffectEmitter::ClearByID(u32 id)
{
    for (auto it = effects.begin(); it != effects.end(); )
    {
        if (it->id == id) it = effects.erase(it);
        else ++it;
    }
}

void EffectEmitter::Update(float deltaTime)
{
    for (auto it = effects.begin(); it != effects.end(); )
    {
        Effect& effect = *it;

        effect.life -= deltaTime;
        float lifeRatio = effect.life / effect.maxLife;

        if (effect.type == BILLBOARD)
        {
            effect.color.a = (u8)(255 * lifeRatio);
        }
        else if (effect.type == YUP_PLANE)
        {
            float growthRatio = (1.0f - lifeRatio);
            float dynamicSpeed = effect.speed * (2.0f - lifeRatio);
            effect.size += effect.maxSize * (growthRatio + (deltaTime * dynamicSpeed));
            effect.color.a = (u8)(255 * lifeRatio);
        }

        if (effect.life <= 0.0f) it = effects.erase(it);
        else ++it;
    }
}

void EffectEmitter::Draw(Camera3D camera)
{
    Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
    Vector3 cameraRight = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
    Vector3 cameraUp    = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };

    batch.Clear();

    for (const auto& effect : effects)
    {
        Vector3 right{}, up{};

        if (effect.type == BILLBOARD)
        {
            float rad = effect.rotation * DEG2RAD;
            float cosR = cosf(rad);
            float sinR = sinf(rad);

            Vector3 rotatedRight = {
                cameraRight.x * cosR - cameraUp.x * sinR,
                cameraRight.y * cosR - cameraUp.y * sinR,
                cameraRight.z * cosR - cameraUp.z * sinR
            };
            Vector3 rotatedUp = {
                cameraRight.x * sinR + cameraUp.x * cosR,
                cameraRight.y * sinR + cameraUp.y * cosR,
                cameraRight.z * sinR + cameraUp.z * cosR
            };

            right = Vector3Scale(rotatedRight, effect.size * 0.5f);
            up    = Vector3Scale(rotatedUp,    effect.size * 0.5f);
        }
        else // YUP_PLANE
        {
            right = Vector3Scale(Vector3{ 1, 0, 0 }, effect.size * 0.5f);
            up    = Vector3Scale(Vector3{ 0, 0, 1 }, effect.size * 0.5f);
        }

        Vector3 v1 = Vector3Add(Vector3Add(effect.position, right), up);
        Vector3 v2 = Vector3Add(Vector3Subtract(effect.position, right), up);
        Vector3 v3 = Vector3Subtract(Vector3Subtract(effect.position, right), up);
        Vector3 v4 = Vector3Subtract(Vector3Add(effect.position, right), up);

        u8 r = effect.color.r, g = effect.color.g, b = effect.color.b, a = effect.color.a;

        batch.BeginQuad();
        batch.AddVertex(v1.x, v1.y, v1.z, 1.0f, 1.0f, r, g, b, a);
        batch.AddVertex(v2.x, v2.y, v2.z, 1.0f, 0.0f, r, g, b, a);
        batch.AddVertex(v3.x, v3.y, v3.z, 0.0f, 0.0f, r, g, b, a);
        batch.AddVertex(v4.x, v4.y, v4.z, 0.0f, 1.0f, r, g, b, a);
        batch.EndQuad();
    }

    batch.Render(texture.id);
}

int EffectEmitter::GetEffectCount() const
{
    return (int)effects.size();
}

void EffectEmitter::Clear()
{
    effects.clear();
}
