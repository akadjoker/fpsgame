#pragma once
// #include "Config.hpp"
// #include <raylib.h>
// #include <raymath.h>
// #include <rlgl.h>
// #include <vector>
// #include <algorithm>
// #include <random>
// #include <functional>

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

// Enumeração para tipos de forma de emissão
enum class EmissionShape
{
    POINT,
    SPHERE,
    CONE,
    BOX,
    CIRCLE
};

// Estrutura de partícula mais completa
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

    // Função de interpolação de cor
    void UpdateColor()
    {
        float t = 1.0f - (life / maxLife);
        currentColor.r = (unsigned char)Lerp(startColor.r, endColor.r, t);
        currentColor.g = (unsigned char)Lerp(startColor.g, endColor.g, t);
        currentColor.b = (unsigned char)Lerp(startColor.b, endColor.b, t);
        currentColor.a = (unsigned char)Lerp(startColor.a, endColor.a, t);
    }
};

// Classe base para modificadores de partículas
class ParticleModifier {
public:
    virtual ~ParticleModifier() = default;
    virtual void Apply(Particle& particle, float deltaTime) = 0;
};

// Modificador de gravidade
class GravityModifier : public ParticleModifier {
private:
    Vector3 gravity;

public:
    GravityModifier(Vector3 gravity): gravity(gravity) {}

    void Apply(Particle& particle, float deltaTime) override
    {
        particle.acceleration = Vector3Add(particle.acceleration, gravity);
    }
};

// Modificador de turbulência
class TurbulenceModifier : public ParticleModifier {
private:
    float strength;
    float frequency;

public:
    TurbulenceModifier(float strength = 1.0f, float frequency = 0.5f)
        : strength(strength), frequency(frequency)
    {}

    void Apply(Particle& particle, float deltaTime) override
    {
        float time = GetTime() * frequency;
        Vector3 turbulence = { sin(time + particle.position.x) * strength,
                               cos(time + particle.position.y) * strength,
                               sin(time + particle.position.z) * strength };
        particle.acceleration = Vector3Add(particle.acceleration, turbulence);
    }
};

// Modificador de vórtice
class VortexModifier : public ParticleModifier {
private:
    Vector3 center;
    float strength;
    Vector3 axis;

public:
    VortexModifier(Vector3 center, float strength, Vector3 axis = { 0, 1, 0 })
        : center(center), strength(strength), axis(Vector3Normalize(axis))
    {}

    void Apply(Particle& particle, float deltaTime) override
    {
        Vector3 toCenter = Vector3Subtract(center, particle.position);
        float distance = Vector3Length(toCenter);

        if (distance > 0.1f)
        {
            Vector3 tangent = Vector3CrossProduct(axis, toCenter);
            tangent = Vector3Normalize(tangent);

            float force = strength / (distance * distance);
            Vector3 vortexForce = Vector3Scale(tangent, force);

            particle.acceleration =
                Vector3Add(particle.acceleration, vortexForce);
        }
    }
};

// Modificador de atração/repulsão
class AttractorModifier : public ParticleModifier {
private:
    Vector3 center;
    float strength;
    bool repulse;

public:
    AttractorModifier(Vector3 center, float strength, bool repulse = false)
        : center(center), strength(strength), repulse(repulse)
    {}

    void Apply(Particle& particle, float deltaTime) override
    {
        Vector3 toCenter = Vector3Subtract(center, particle.position);
        float distance = Vector3Length(toCenter);

        if (distance > 0.1f)
        {
            Vector3 direction = Vector3Normalize(toCenter);
            float force = strength / (distance * distance);

            if (repulse)
            {
                direction = Vector3Negate(direction);
            }

            Vector3 attractionForce = Vector3Scale(direction, force);
            particle.acceleration =
                Vector3Add(particle.acceleration, attractionForce);
        }
    }
};

 
class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::vector<int> freeIndices;
    std::vector<ParticleModifier*> modifiers;

    int maxParticles;
    float timeAccumulator;
    Texture2D texture;

 
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> uniformDist;

    EmissionConfig config;
    EmissionShape shape;
    Batch batch;
    bool isActive;

 
    void InitializePool()
    {
        particles.resize(maxParticles);
        freeIndices.reserve(maxParticles);

        for (int i = 0; i < maxParticles; ++i)
        {
            particles[i].poolIndex = i;
            freeIndices.push_back(i);
        }
    }

    // Função para gerar posição baseada na forma
    Vector3 GenerateEmissionPosition()
    {
        switch (shape)
        {
            case EmissionShape::SPHERE: {
                float theta = uniformDist(gen) * 2.0f * PI;
                float phi = acos(1.0f - 2.0f * uniformDist(gen));
                float radius = pow(uniformDist(gen), 1.0f / 3.0f)
                    * config.directionVariance.x;

                return { config.position.x + radius * sin(phi) * cos(theta),
                         config.position.y + radius * sin(phi) * sin(theta),
                         config.position.z + radius * cos(phi) };
            }

            case EmissionShape::BOX: {
                return { config.position.x
                             + (uniformDist(gen) - 0.5f)
                                 * config.directionVariance.x,
                         config.position.y
                             + (uniformDist(gen) - 0.5f)
                                 * config.directionVariance.y,
                         config.position.z
                             + (uniformDist(gen) - 0.5f)
                                 * config.directionVariance.z };
            }

            case EmissionShape::CIRCLE: {
                float angle = uniformDist(gen) * 2.0f * PI;
                float radius =
                    sqrt(uniformDist(gen)) * config.directionVariance.x;

                return { config.position.x + radius * cos(angle),
                         config.position.y,
                         config.position.z + radius * sin(angle) };
            }

            case EmissionShape::CONE: {
                float angle = uniformDist(gen) * 2.0f * PI;
                float height = uniformDist(gen) * config.directionVariance.y;
                float radius = height * tan(config.directionVariance.x);

                return { config.position.x + radius * cos(angle),
                         config.position.y + height,
                         config.position.z + radius * sin(angle) };
            }

            default: // POINT
                return config.position;
        }
    }

    Vector3 GenerateRandomDirection()
    {
        Vector3 baseDir = config.direction;
        Vector3 randomOffset = {
            (uniformDist(gen) - 0.5f) * config.directionVariance.x,
            (uniformDist(gen) - 0.5f) * config.directionVariance.y,
            (uniformDist(gen) - 0.5f) * config.directionVariance.z
        };

        return Vector3Normalize(Vector3Add(baseDir, randomOffset));
    }

public:
    ParticleSystem()
        :   timeAccumulator(0.0f),
          gen(rd()), uniformDist(0.0f, 1.0f), shape(EmissionShape::POINT),
          isActive(true)
    {
        
    }

    //  RobustParticleSystem(Texture2D texture, int maxParticles = 1000)
    //     : maxParticles(maxParticles), timeAccumulator(0.0f), texture(texture),
    //       gen(rd()), uniformDist(0.0f, 1.0f), shape(EmissionShape::POINT), isActive(true) {
    //     InitializePool();
    // }
    
    void Init(Texture2D texture, int maxParticles = 1000)
    {
        this->texture = texture;
        this->maxParticles = maxParticles;
        batch.Init(maxParticles);
        InitializePool();

    }

    void Release() 
    {
        batch.Release();    
    }



    ~ParticleSystem()
    {
        
        for (ParticleModifier* modifier : modifiers)
        {
            delete modifier;
        }
        modifiers.clear();
    }

 
    void SetEmissionConfig(const EmissionConfig& newConfig)
    {
        config = newConfig;
    }

    void SetEmissionShape(EmissionShape newShape) { shape = newShape; }

    void SetActive(bool active) { isActive = active; }

    // Adicionar modificadores
    void AddModifier(ParticleModifier* modifier)
    {
        modifiers.push_back(modifier);
    }

    void ClearModifiers()
    {
        for (ParticleModifier* modifier : modifiers)
        {
            delete modifier;
        }
        modifiers.clear();
    }

    // Emissão de partículas usando configuração padrão
    void EmitParticles(int count = -1)
    {
        if (count == -1) count = config.burstCount;

        for (int i = 0; i < count && !freeIndices.empty(); ++i)
        {
            int index = freeIndices.back();
            freeIndices.pop_back();

            Particle& p = particles[index];

            // Configurar partícula
            p.position = GenerateEmissionPosition();
            p.velocity = Vector3Scale(GenerateRandomDirection(),
                                      config.speed
                                          + (uniformDist(gen) - 0.5f)
                                              * config.speedVariance);

            p.acceleration = { 0, 0, 0 };
            p.maxLife =
                config.life + (uniformDist(gen) - 0.5f) * config.lifeVariance;
            p.life = p.maxLife;
            p.size =
                config.size + (uniformDist(gen) - 0.5f) * config.sizeVariance;
            p.rotation = uniformDist(gen) * 360.0f;
            p.angularVelocity = (uniformDist(gen) - 0.5f) * 180.0f;

            p.startColor = config.startColor;
            p.endColor = config.endColor;
            p.currentColor = config.startColor;

            p.active = true;
        }
    }

    // Emissão específica em posição e direção (para efeitos de bala/impacto)
    void EmitAt(Vector3 position, Vector3 direction, int count = 10,
                float spread = 0.3f, float speed = 5.0f, Color color = WHITE)
    {

        for (int i = 0; i < count && !freeIndices.empty(); ++i)
        {
            int index = freeIndices.back();
            freeIndices.pop_back();

            Particle& p = particles[index];

            // Posição específica
            p.position = position;

            // Direção com spread aleatório
            Vector3 randomOffset = { (uniformDist(gen) - 0.5f) * spread,
                                     (uniformDist(gen) - 0.5f) * spread,
                                     (uniformDist(gen) - 0.5f) * spread };

            Vector3 finalDirection = Vector3Add(direction, randomOffset);
            p.velocity = Vector3Scale(
                Vector3Normalize(finalDirection),
                speed + (uniformDist(gen) - 0.5f) * (speed * 0.3f));

            p.acceleration = { 0, 0, 0 };
            p.maxLife =
                config.life + (uniformDist(gen) - 0.5f) * config.lifeVariance;
            p.life = p.maxLife;
            p.size =
                config.size + (uniformDist(gen) - 0.5f) * config.sizeVariance;
            p.rotation = uniformDist(gen) * 360.0f;
            p.angularVelocity = (uniformDist(gen) - 0.5f) * 180.0f;

            p.startColor = color;
            p.endColor = config.endColor;
            p.currentColor = color;

            p.active = true;
        }
    }

    // Explosão de barril (efeito completo multi-camadas)
void EmitBarrelExplosion(Vector3 position, float intensity = 1.0f)
{
    // FASE 1: Flash inicial (muito rápido e brilhante)
    EmitExplosionCore(position, intensity);
    
    // FASE 2: Bola de fogo principal
    EmitFireball(position, intensity);
    
    // FASE 3: Fragmentos e detritos
    EmitDebris(position, intensity);
    
    // FASE 4: Fumaça densa
    EmitExplosionSmoke(position, intensity);
    
    // FASE 5: Faíscas e brasas
    EmitSparksAndEmbers(position, intensity);
}

// Flash inicial da explosão (core brilhante)
void EmitExplosionCore(Vector3 position, float intensity = 1.0f)
{
    Color coreColor = { 255, 255, 255, 255 }; // Branco puro
    int count = (int)(8 * intensity);
    
    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();
        
        Particle& p = particles[index];
        
        p.position = position;
        
        // Expansão rápida em todas as direções
        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f
        };
        
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), 15.0f * intensity);
        p.acceleration = Vector3Scale(p.velocity, -8.0f); // Desaceleração rápida
        
        p.maxLife = 0.1f + uniformDist(gen) * 0.05f; // Muito rápido
        p.life = p.maxLife;
        p.size = (0.3f + uniformDist(gen) * 0.2f) * intensity;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 1800.0f;
        
        p.startColor = coreColor;
        p.endColor = { 255, 100, 0, 0 }; // Para laranja e desaparece
        p.currentColor = coreColor;
        
        p.active = true;
    }
}

// Bola de fogo principal
void EmitFireball(Vector3 position, float intensity = 1.0f)
{
    Color fireColor = { 255, 120, 0, 255 }; // Laranja ardente
    int count = (int)(25 * intensity);
    
    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();
        
        Particle& p = particles[index];
        
        p.position = position;
        
        // Esfera de expansão
        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f,
            (uniformDist(gen) - 0.5f) * 2.0f
        };
        
        float speed = 8.0f + uniformDist(gen) * 6.0f;
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), speed * intensity);
        p.acceleration = { 0, 1.0f, 0 }; // Leve subida (calor)
        
        p.maxLife = 0.8f + uniformDist(gen) * 0.4f;
        p.life = p.maxLife;
        p.size = (0.15f + uniformDist(gen) * 0.1f) * intensity;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 360.0f;
        
        p.startColor = fireColor;
        p.endColor = { 80, 20, 20, 0 }; // Para vermelho escuro
        p.currentColor = fireColor;
        
        p.active = true;
    }
}

// Fragmentos e detritos
void EmitDebris(Vector3 position, float intensity = 1.0f)
{
    Color debrisColor = { 120, 80, 40, 255 }; // Marrom metálico
    int count = (int)(30 * intensity);
    
    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();
        
        Particle& p = particles[index];
        
        p.position = position;
        
        // Direção mais aleatória, com tendência para cima
        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            uniformDist(gen) * 1.5f + 0.2f, // Mais para cima
            (uniformDist(gen) - 0.5f) * 2.0f
        };
        
        float speed = 12.0f + uniformDist(gen) * 8.0f;
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), speed * intensity);
        p.acceleration = { 0, -9.8f, 0 }; // Gravidade
        
        p.maxLife = 2.0f + uniformDist(gen) * 1.5f; // Dura mais tempo
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

// Fumaça densa da explosão
void EmitExplosionSmoke(Vector3 position, float intensity = 1.0f)
{
    Color smokeColor = { 40, 40, 40, 180 }; // Cinza escuro
    int count = (int)(20 * intensity);
    
    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();
        
        Particle& p = particles[index];
        
        // Posição ligeiramente elevada
        p.position = Vector3Add(position, { (uniformDist(gen) - 0.5f) * 0.5f, 
                                           uniformDist(gen) * 0.3f, 
                                           (uniformDist(gen) - 0.5f) * 0.5f });
        
        // Movimento lento para cima
        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 0.8f,
            0.5f + uniformDist(gen) * 0.5f, // Principalmente para cima
            (uniformDist(gen) - 0.5f) * 0.8f
        };
        
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), 3.0f + uniformDist(gen) * 2.0f);
        p.acceleration = { 0, 0.5f, 0 }; // Leve subida
        
        p.maxLife = 3.0f + uniformDist(gen) * 2.0f; // Dura bastante
        p.life = p.maxLife;
        p.size = (0.2f + uniformDist(gen) * 0.3f) * intensity;
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 90.0f;
        
        p.startColor = smokeColor;
        p.endColor = { 80, 80, 80, 0 }; // Para cinza claro e transparente
        p.currentColor = smokeColor;
        
        p.active = true;
    }
}

// Faíscas e brasas
void EmitSparksAndEmbers(Vector3 position, float intensity = 1.0f)
{
    Color sparkColor = { 255, 200, 50, 255 }; // Amarelo brilhante
    int count = (int)(40 * intensity);
    
    for (int i = 0; i < count && !freeIndices.empty(); ++i)
    {
        int index = freeIndices.back();
        freeIndices.pop_back();
        
        Particle& p = particles[index];
        
        p.position = position;
        
        // Arco balístico realista
        Vector3 randomDir = {
            (uniformDist(gen) - 0.5f) * 2.0f,
            uniformDist(gen) * 1.0f + 0.3f, // Para cima
            (uniformDist(gen) - 0.5f) * 2.0f
        };
        
        float speed = 15.0f + uniformDist(gen) * 10.0f;
        p.velocity = Vector3Scale(Vector3Normalize(randomDir), speed * intensity);
        p.acceleration = { 0, -9.8f, 0 }; // Gravidade completa
        
        p.maxLife = 1.2f + uniformDist(gen) * 0.8f;
        p.life = p.maxLife;
        p.size = 0.02f + uniformDist(gen) * 0.03f; // Bem pequenas
        p.rotation = uniformDist(gen) * 360.0f;
        p.angularVelocity = (uniformDist(gen) - 0.5f) * 1440.0f;
        
        p.startColor = sparkColor;
        p.endColor = { 150, 50, 50, 0 }; // Para vermelho escuro
        p.currentColor = sparkColor;
        
        p.active = true;
    }
}
    // Emissão de impacto de bala (burst rápido)
    void EmitBulletImpact(Vector3 position, Vector3 normal, int count = 15)
    {
        Color sparkColor = { 255, 255, 100, 255 };
        float impactSpeed = 8.5f;
        float impactSpread = 0.8f;

        for (int i = 0; i < count && !freeIndices.empty(); ++i)
        {
            int index = freeIndices.back();
            freeIndices.pop_back();

            Particle& p = particles[index];

            p.position = position;

            // Direção baseada na normal + aleatoriedade
            Vector3 randomDir = {
                normal.x + (uniformDist(gen) - 0.5f) * impactSpread,
                normal.y + (uniformDist(gen) - 0.5f) * impactSpread,
                normal.z + (uniformDist(gen) - 0.5f) * impactSpread
            };

            p.velocity =Vector3Scale(Vector3Normalize(randomDir),impactSpeed + (uniformDist(gen) - 0.5f) * 4.0f);

            p.acceleration = { 0, -0.1, 0 };
            p.maxLife =0.2f + uniformDist(gen) * 0.4f; 
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

    // Emissão de trilha de bala
    void EmitBulletTrail(Vector3 startPos, Vector3 endPos, int count = 20)
    {
        Color trailColor = { 255, 255, 200, 150 };
        Vector3 direction = Vector3Normalize(Vector3Subtract(endPos, startPos));
      //  float distance = Vector3Distance(startPos, endPos);

        for (int i = 0; i < count && !freeIndices.empty(); ++i)
        {
            int index = freeIndices.back();
            freeIndices.pop_back();

            Particle& p = particles[index];

            // Posição ao longo da linha da bala
            float t = uniformDist(gen);
            p.position = Vector3Lerp(startPos, endPos, t);

            // Velocidade perpendicular à direção da bala
            Vector3 perpendicular = { -direction.z, 0, direction.x };
            if (Vector3Length(perpendicular) < 0.1f)
            {
                perpendicular = { 0, 1, 0 };
            }

            Vector3 randomPerp = Vector3Scale(Vector3Normalize(perpendicular),
                                              (uniformDist(gen) - 0.5f) * 2.0f);
            p.velocity = randomPerp;

            p.acceleration = { 0, 0, 0 };
            p.maxLife = 0.3f + uniformDist(gen) * 0.2f; // Muito rápido
            p.life = p.maxLife;
            p.size = 0.02f + uniformDist(gen) * 0.01f; // Bem pequenas
            p.rotation = uniformDist(gen) * 360.0f;
            p.angularVelocity = (uniformDist(gen) - 0.5f) * 180.0f;

            p.startColor = trailColor;
            p.endColor = { 255, 255, 200, 0 };
            p.currentColor = trailColor;

            p.active = true;
        }
    }

    // Emissão de chamas de cano (muzzle flash)
    void EmitMuzzleFlash(Vector3 position, Vector3 direction,
                         float intensity = 1.0f)
    {
        Color flashColor = { 255, 200, 100, 255 };
        int count = (int)(15 * intensity);
        float speed = 6.0f * intensity;

        for (int i = 0; i < count && !freeIndices.empty(); ++i)
        {
            int index = freeIndices.back();
            freeIndices.pop_back();

            Particle& p = particles[index];

            p.position = position;

            // Cone frontal
            Vector3 randomDir = {
                direction.x + (uniformDist(gen) - 0.5f) * 0.4f,
                direction.y + (uniformDist(gen) - 0.5f) * 0.4f,
                direction.z + (uniformDist(gen) - 0.5f) * 0.4f
            };

            p.velocity = Vector3Scale(Vector3Normalize(randomDir),
                                      speed + (uniformDist(gen) - 0.5f) * 3.0f);

            p.acceleration = { 0, 0, 0 };
            p.maxLife = 0.15f + uniformDist(gen) * 0.1f; // Muito rápido
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

    void EmitExplosion(Vector3 position, Vector3 direction,float intensity = 1.0f)
    {
        Color flashColor = { 255, 200, 100, 255 };
        int count = (int)(15 * intensity);
        float speed = 6.0f * intensity;

        for (int i = 0; i < count && !freeIndices.empty(); ++i)
        {
            int index = freeIndices.back();
            freeIndices.pop_back();

            Particle& p = particles[index];

            p.position = position;

            // Cone frontal
            Vector3 randomDir = {
                direction.x + (uniformDist(gen) - 0.5f) * 0.4f,
                direction.y + (uniformDist(gen) - 0.5f) * 0.4f,
                direction.z + (uniformDist(gen) - 0.5f) * 0.4f
            };

            p.velocity = Vector3Scale(Vector3Normalize(randomDir),
                                      speed + (uniformDist(gen) - 0.5f) * 3.0f);

            p.acceleration = { 0, 0, 0 };
            p.maxLife = 0.15f + uniformDist(gen) * 0.1f; // Muito rápido
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
 
    void Update(float deltaTime)
    {
        if (!isActive) return;

     
        if (config.continuous && config.emissionRate > 0)
        {
            timeAccumulator += deltaTime;
            float emissionInterval = 1.0f / config.emissionRate;

            while (timeAccumulator >= emissionInterval)
            {
                EmitParticles(1);
                timeAccumulator -= emissionInterval;
            }
        }

        // Atualizar partículas ativas
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
            {
                modifier->Apply(p, deltaTime);
            }

            p.velocity =Vector3Add(p.velocity, Vector3Scale(p.acceleration, deltaTime));
            p.velocity = Vector3Scale(p.velocity, config.drag); // Arrasto
            p.position =Vector3Add(p.position, Vector3Scale(p.velocity, deltaTime));

            p.rotation += p.angularVelocity * deltaTime;


            p.UpdateColor();
        }
    }


    // Renderização
    void Draw(Camera3D camera)
    {
        batch.Clear();
//        Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
        
        Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
        Vector3 cameraRight = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
        Vector3 cameraUp = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };

     //   LogInfo("Particulas: %d", particles.size());
        
        for (const auto& p : particles)
        {
            if (!p.active) continue;

                Vector3 right, up;

                float rad = p.rotation * DEG2RAD;
                float cosR = cos(rad);
                float sinR = sin(rad);

                // Aplica rotação aos vetores da câmera
                Vector3 rotatedRight = {
                    cameraRight.x * cosR - cameraUp.x * sinR,
                    cameraRight.y * cosR - cameraUp.y * sinR,
                    cameraRight.z * cosR - cameraUp.z * sinR
                };
                Vector3 rotatedUp = { cameraRight.x * sinR + cameraUp.x * cosR,
                                      cameraRight.y * sinR + cameraUp.y * cosR,
                                      cameraRight.z * sinR
                                          + cameraUp.z * cosR };

                right = Vector3Scale(rotatedRight, p.size * 0.5f);
                up = Vector3Scale(rotatedUp, p.size * 0.5f);



            Vector3 v1 = Vector3Add(Vector3Add(p.position, right), up);
            Vector3 v2 =
                Vector3Add(Vector3Subtract(p.position, right), up);
            Vector3 v3 =
                Vector3Subtract(Vector3Subtract(p.position, right), up);
            Vector3 v4 =
                Vector3Subtract(Vector3Add(p.position, right), up);

            u8 r = p.currentColor.r, g = p.currentColor.g, b = p.currentColor.b, a = p.currentColor.a;
            batch.BeginQuad();

                batch.AddVertex(v1.x, v1.y, v1.z, 1.0f, 1.0f,r,g,b,a);
                batch.AddVertex(v2.x, v2.y, v2.z, 1.0f, 0.0f,r,g,b,a);
                batch.AddVertex(v3.x, v3.y, v3.z, 0.0f, 0.0f,r,g,b,a);
                batch.AddVertex(v4.x, v4.y, v4.z, 0.0f, 1.0f,r,g,b,a);
            
            batch.EndQuad();
  
            // if (p.rotation != 0.0f)
            // {
            //        //  void DrawQuadBillboard(Matrix& matView,Texture& texture, Vector3 position,Vector2 size,float angle, Color tint)

            //     batch.DrawQuadBillboard(viewMatrix, texture, 
            //         p.position,  
            //         { p.size, p.size },  p.rotation, p.currentColor);

            //     // DrawBillboardPro(
            //     //     camera, texture,
            //     //     { 0, 0, (float)texture.width, (float)texture.height },
            //     //     p.position, { 0, 1, 0 }, { p.size, p.size }, { 0, 0 },
            //     //     p.rotation, p.currentColor);
            // }
            // else
            // {
            //     batch.DrawQuadBillboard(viewMatrix, texture,  p.position, p.size , p.currentColor);
            //     //DrawBillboard(camera, texture, p.position, p.size,p.currentColor);
            // }
        }

        batch.Render(texture.id);
         batch.Clear();
    }

    // Estatísticas
    int GetActiveParticleCount() const
    {
        return maxParticles - (int)freeIndices.size();
    }

    int GetMaxParticles() const { return maxParticles; }

    // Limpar todas as partículas
    void Clear()
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

    // Criar efeitos pré-definidos
    static EmissionConfig CreateFireConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 1, 0 };
        config.directionVariance = { 0.3f, 0.2f, 0.3f };
        config.speed = 3.0f;
        config.speedVariance = 1.0f;
        config.life = 2.0f;
        config.lifeVariance = 0.5f;
        config.startColor = { 255, 100, 50, 255 };
        config.endColor = { 255, 255, 100, 0 };
        config.emissionRate = 50.0f;
        config.drag = 0.95f;
        return config;
    }

    static EmissionConfig CreateSmokeConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 1, 0 };
        config.directionVariance = { 0.5f, 0.1f, 0.5f };
        config.speed = 1.0f;
        config.speedVariance = 0.5f;
        config.life = 4.0f;
        config.lifeVariance = 1.0f;
        config.size = 0.2f;
        config.startColor = { 100, 100, 100, 180 };
        config.endColor = { 200, 200, 200, 0 };
        config.emissionRate = 20.0f;
        config.drag = 0.98f;
        return config;
    }

    static EmissionConfig CreateSparkConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 0, 0 };
        config.directionVariance = { 1.0f, 1.0f, 1.0f };
        config.speed = 5.0f;
        config.speedVariance = 3.0f;
        config.life = 1.0f;
        config.lifeVariance = 0.3f;
        config.size = 0.05f;
        config.startColor = { 255, 255, 100, 255 };
        config.endColor = { 255, 50, 50, 0 };
        config.emissionRate = 100.0f;
        config.drag = 0.90f;
        config.continuous = false;
        config.burstCount = 50;
        return config;
    }

    static EmissionConfig CreateSnowConfig()
    {
        EmissionConfig config;
        config.direction = { 0, -1, 0 };
        config.directionVariance = { 0.2f, 0.1f, 0.2f };
        config.speed = 1.0f;
        config.speedVariance = 0.3f;
        config.life = 8.0f;
        config.lifeVariance = 2.0f;
        config.size = 0.1f;
        config.sizeVariance = 0.05f;
        config.startColor = { 255, 255, 255, 200 };
        config.endColor = { 255, 255, 255, 50 };
        config.emissionRate = 15.0f;
        config.drag = 0.99f;
        return config;
    }

    static EmissionConfig CreateExplosionConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 0, 0 };
        config.directionVariance = { 1.0f, 1.0f, 1.0f };
        config.speed = 8.0f;
        config.speedVariance = 4.0f;
        config.life = 1.5f;
        config.lifeVariance = 0.5f;
        config.size = 0.15f;
        config.sizeVariance = 0.1f;
        config.startColor = { 255, 150, 50, 255 };
        config.endColor = { 100, 50, 50, 0 };
        config.emissionRate = 0;
        config.drag = 0.85f;
        config.continuous = false;
        config.burstCount = 100;
        return config;
    }

    // Configurações específicas para efeitos de bala
    static EmissionConfig CreateBulletImpactConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 1, 0 }; // Normal padrão
        config.directionVariance = { 0.8f, 0.8f, 0.8f };
        config.speed = 8.0f;
        config.speedVariance = 4.0f;
        config.life = 0.8f;
        config.lifeVariance = 0.4f;
        config.size = 0.05f;
        config.sizeVariance = 0.03f;
        config.startColor = { 255, 255, 100, 255 };
        config.endColor = { 255, 50, 50, 0 };
        config.emissionRate = 0;
        config.drag = 0.88f;
        config.continuous = false;
        config.burstCount = 15;
        return config;
    }

    static EmissionConfig CreateMuzzleFlashConfig()
    {
        EmissionConfig config;
        config.direction = { 1, 0, 0 }; // Frente da arma
        config.directionVariance = { 0.4f, 0.4f, 0.4f };
        config.speed = 6.0f;
        config.speedVariance = 3.0f;
        config.life = 0.15f;
        config.lifeVariance = 0.1f;
        config.size = 0.08f;
        config.sizeVariance = 0.04f;
        config.startColor = { 255, 200, 100, 255 };
        config.endColor = { 100, 50, 50, 0 };
        config.emissionRate = 0;
        config.drag = 0.92f;
        config.continuous = false;
        config.burstCount = 15;
        return config;
    }

    static EmissionConfig CreateBulletTrailConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 0, 0 }; // Direção perpendicular
        config.directionVariance = { 0.1f, 0.1f, 0.1f };
        config.speed = 2.0f;
        config.speedVariance = 1.0f;
        config.life = 0.3f;
        config.lifeVariance = 0.2f;
        config.size = 0.02f;
        config.sizeVariance = 0.01f;
        config.startColor = { 255, 255, 200, 150 };
        config.endColor = { 255, 255, 200, 0 };
        config.emissionRate = 0;
        config.drag = 0.95f;
        config.continuous = false;
        config.burstCount = 20;
        return config;
    }

    static EmissionConfig CreateRicochetConfig()
    {
        EmissionConfig config;
        config.direction = { 0, 1, 0 };
        config.directionVariance = { 1.2f, 0.6f, 1.2f };
        config.speed = 12.0f;
        config.speedVariance = 6.0f;
        config.life = 1.2f;
        config.lifeVariance = 0.6f;
        config.size = 0.03f;
        config.sizeVariance = 0.02f;
        config.startColor = { 255, 255, 150, 255 };
        config.endColor = { 255, 100, 50, 0 };
        config.emissionRate = 0;
        config.drag = 0.82f;
        config.continuous = false;
        config.burstCount = 8;
        return config;
    }
};


enum EffectType
{
    BILLBOARD, // Billboard rotacionável
    YUP_PLANE // Plano Y-up que cresce
};

struct Effect
{
    Vector3 position;
    EffectType type;
    float rotation; // Rotação em graus (para muzzle flash)
    float size;
    float maxSize;
    float life;
    float maxLife;
    Color color;
    float speed;
    u32 id;
    

    Effect(Vector3 pos, EffectType t, float rot, float lifeTime, float sz, float speed, u32 id = 0)
        : position(pos), type(t), rotation(rot), size(sz), maxSize(sz),
          life(lifeTime), maxLife(lifeTime), color(WHITE), speed(speed), id(id) 
    {}
};

class EffectEmitter 
{
private:
    std::vector<Effect> effects;
    Texture2D texture;
    Batch batch;
 

  

public:
    EffectEmitter()=default;
    ~EffectEmitter() = default;

    void Init(Texture2D tex,u32 maxParticles = 10)
    {
        texture = tex;
        batch.Init(maxParticles);

    }

    void Release() 
    {
        batch.Release();    
    }


    void SetPosition(Vector3 pos) 
    {
        for (auto& effect : effects)
        {
            effect.position = pos;
        }
    }

 
    void CreateMuzzleFlash(Vector3 position, float rotation, float lifeTime, float size,u32 id = 0)
    {
       
        effects.emplace_back(position, BILLBOARD, rotation, lifeTime, size,0.0f,id);
    }

  
    void CreateShockwave(Vector3 position, float maxSize, float lifeTime, float sz,float speed,u32 id = 0)
    {
       
        Effect shock(position, YUP_PLANE, 0, lifeTime, sz, speed,id);
        shock.maxSize = maxSize;
        effects.push_back(shock);
    }

    void ClearByID(u32 id)
    {
        for (auto it = effects.begin(); it != effects.end();)
        {
            if (it->id == id)
            {
                it = effects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Update(float deltaTime)
    {
        for (auto it = effects.begin(); it != effects.end();)
        {
            Effect& effect = *it;

            effect.life -= deltaTime;
            float lifeRatio = effect.life / effect.maxLife;

            if (effect.type == BILLBOARD)
            {
               
                effect.color.a = (unsigned char)(255 * lifeRatio);
            }
            else if (effect.type == YUP_PLANE)
            {
            
                float growthRatio = (1.0f - lifeRatio);
                // Speed aumenta conforme o efeito envelhece
                float dynamicSpeed = effect.speed * (2.0f - lifeRatio); // Fica mais rápido com o tempo
                effect.size += effect.maxSize * (growthRatio + (deltaTime * dynamicSpeed));
            

               // float growthRatio = (1.0f - lifeRatio);
                // Crescimento mais acelerado no início
                //float acceleratedGrowth = powf(growthRatio, 0.5f);  
                //effect.size = effect.maxSize * (acceleratedGrowth + (deltaTime * effect.speed));

               //     effect.size = effect.maxSize * (growthRatio + (deltaTime * effect.speed * 2.0f)); // 2x mais rápido
               //     effect.color.a = (unsigned char)(255 * lifeRatio);
                
                
                // float growthRatio = (1.0f - lifeRatio) ;
                // effect.size = effect.maxSize * (growthRatio  + (deltaTime * effect.speed));
                effect.color.a = (unsigned char)(255 * lifeRatio);
            }


            if (effect.life <= 0)
            {
                it = effects.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void Draw(Camera3D camera)
    {

        Matrix viewMatrix = MatrixLookAt(camera.position, camera.target, camera.up);
        Vector3 cameraRight = { viewMatrix.m0, viewMatrix.m4, viewMatrix.m8 };
        Vector3 cameraUp = { viewMatrix.m1, viewMatrix.m5, viewMatrix.m9 };

        batch.Clear();

        for (const auto& effect : effects)
        {
            Vector3 right, up;

            if (effect.type == BILLBOARD)
            {
                // Billboard com rotação
                float rad = effect.rotation * DEG2RAD;
                float cosR = cos(rad);
                float sinR = sin(rad);

                // Aplica rotação aos vetores da câmera
                Vector3 rotatedRight = {
                    cameraRight.x * cosR - cameraUp.x * sinR,
                    cameraRight.y * cosR - cameraUp.y * sinR,
                    cameraRight.z * cosR - cameraUp.z * sinR
                };
                Vector3 rotatedUp = { cameraRight.x * sinR + cameraUp.x * cosR,
                                      cameraRight.y * sinR + cameraUp.y * cosR,
                                      cameraRight.z * sinR
                                          + cameraUp.z * cosR };

                right = Vector3Scale(rotatedRight, effect.size * 0.5f);
                up = Vector3Scale(rotatedUp, effect.size * 0.5f);
            }
            else if (effect.type == YUP_PLANE)
            {
                // Plano Y-up (horizontal)
                right = Vector3Scale({ 1, 0, 0 }, effect.size * 0.5f);
                up = Vector3Scale({ 0, 0, 1 }, effect.size * 0.5f);
            }

   
            Vector3 v1 = Vector3Add(Vector3Add(effect.position, right), up);
            Vector3 v2 =Vector3Add(Vector3Subtract(effect.position, right), up);
            Vector3 v3 =Vector3Subtract(Vector3Subtract(effect.position, right), up);
            Vector3 v4 =Vector3Subtract(Vector3Add(effect.position, right), up);

            u8 r = effect.color.r, g = effect.color.g, b = effect.color.b, a = effect.color.a;
            batch.BeginQuad();

                batch.AddVertex(v1.x, v1.y, v1.z, 1.0f, 1.0f,r,g,b,a);
                batch.AddVertex(v2.x, v2.y, v2.z, 1.0f, 0.0f,r,g,b,a);
                batch.AddVertex(v3.x, v3.y, v3.z, 0.0f, 0.0f,r,g,b,a);
                batch.AddVertex(v4.x, v4.y, v4.z, 0.0f, 1.0f,r,g,b,a);
            
            batch.EndQuad();

        }

        batch.Render(texture.id);

        
    }

    int GetEffectCount() const { return effects.size(); }
    void Clear() { effects.clear(); }
};