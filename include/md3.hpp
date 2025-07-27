#pragma once

// #include <raylib.h>
// #include <raymath.h>
// #include <stdint.h>
#include "binaryfile.hpp"
#include "mat3.h"
#include "node.hpp"


#define kLower	0			// This stores the ID for the legs model
#define kUpper	1			// This stores the ID for the torso model
#define kHead	2			// This stores the ID for the head model
#define kWeapon	3			// This stores the ID for the weapon model

class MD3Animator;
class LoadMD3;


typedef enum 
{
	// If one model is set to one of the BOTH_* animations, the other one should be too,
	// otherwise it looks really bad and confusing.

	BOTH_DEATH1 = 0,		// The first twirling death animation
	BOTH_DEAD1,				// The end of the first twirling death animation
	BOTH_DEATH2,			// The second twirling death animation
	BOTH_DEAD2,				// The end of the second twirling death animation
	BOTH_DEATH3,			// The back flip death animation
	BOTH_DEAD3,				// The end of the back flip death animation

	// The next block is the animations that the upper body performs

	TORSO_GESTURE,			// The torso's gesturing animation
	
	TORSO_ATTACK,			// The torso's attack1 animation
	TORSO_ATTACK2,			// The torso's attack2 animation

	TORSO_DROP,				// The torso's weapon drop animation
	TORSO_RAISE,			// The torso's weapon pickup animation

	TORSO_STAND,			// The torso's idle stand animation
	TORSO_STAND2,			// The torso's idle stand2 animation

	// The final block is the animations that the legs perform

	LEGS_WALKCR,			// The legs's crouching walk animation
	LEGS_WALK,				// The legs's walk animation
	LEGS_RUN,				// The legs's run animation
	LEGS_BACK,				// The legs's running backwards animation
	LEGS_SWIM,				// The legs's swimming animation
	
	LEGS_JUMP,				// The legs's jumping animation
	LEGS_LAND,				// The legs's landing animation

	LEGS_JUMPB,				// The legs's jumping back animation
	LEGS_LANDB,				// The legs's landing back animation

	LEGS_IDLE,				// The legs's idle stand animation
	LEGS_IDLECR,			// The legs's idle crouching animation

	LEGS_TURN,				// The legs's turn animation

	MAX_ANIMATIONS			// The define for the maximum amount of animations
} eAnimations;

 
struct tMd3Header
{ 
	char	fileID[4];					// This stores the file ID - Must be "IDP3"
	int		version;					// This stores the file version - Must be 15
	char	strFile[68];				// This stores the name of the file
	int		numFrames;					// This stores the number of animation frames
	int		numTags;					// This stores the tag count
	int		numMeshes;					// This stores the number of sub-objects in the mesh
	int		numMaxSkins;				// This stores the number of skins for the mesh
	int		headerSize;					// This stores the mesh header size
	int		tagStart;					// This stores the offset into the file for tags
	int		tagEnd;						// This stores the end offset into the file for tags
	int		fileSize;					// This stores the file size
};

 
struct tMd3MeshInfo
{
	char	meshID[4];					// This stores the mesh ID (We don't care)
	char	strName[68];				// This stores the mesh name (We do care)
	int		numMeshFrames;				// This stores the mesh aniamtion frame count
	int		numSkins;					// This stores the mesh skin count
	int     numVertices;				// This stores the mesh vertex count
	int		numTriangles;				// This stores the mesh face count
	int		triStart;					// This stores the starting offset for the triangles
	int		headerSize;					// This stores the header size for the mesh
	int     uvStart;					// This stores the starting offset for the UV coordinates
	int		vertexStart;				// This stores the starting offset for the vertex indices
	int		meshSize;					// This stores the total mesh size
};

 
struct tMd3Tag
{
	char		strName[64];			// This stores the name of the tag (I.E. "tag_torso")
	Vector3  	position;				// This stores the translation that should be performed
 
    Vector3  	axes[3];
    float  rotation[9];					// This stores the rotation that should be performed
 
};

// This stores the bone information (useless as far as I can see...)
struct tMd3Bone
{
	float	mins[3];					// This is the min (x, y, z) value for the bone
	float	maxs[3];					// This is the max (x, y, z) value for the bone
	float	position[3];				// This supposedly stores the bone position???
	float	scale;						// This stores the scale of the bone
	char	creator[16];				// The modeler used to create the model (I.E. "3DS Max")
};


// This stores the normals and vertex indices 
struct tMd3Triangle
{
   signed short	 vertex[3];				// The vertex for this face (scale down by 64.0f)
   unsigned char normal[2];				// This stores some crazy normal values (not sure...)
};


// This stores the indices into the vertex and texture coordinate arrays
struct tMd3Face
{
   int vertexIndices[3];				
};


// This stores UV coordinates
struct tMd3TexCoord
{
   float textureCoord[2];
};


// This stores a skin name (We don't use this, just the name of the model to get the texture)
struct tMd3Skin 
{
	char strName[68];
};

struct Bone3D
{
    u16  id;
    char name[64];
    Vector3 position;
    Quaternion rotation;
    Matrix trasform;
    LoadMD3* link;
};

struct tMD3Surface
{
    std::string name;
    std::vector<Vector2> texCoords;
    std::vector<Vector3> vertices;
    std::vector<s16> indices;
    u32 tex{ 0 };

    u32 vertexCount { 0 };
    u32 numTriangles{ 0 }; 
    int numFrames{ 0 };
    unsigned int vaoId{ 0 };    
    unsigned int vboId[3] { 0, 0,0};    
 


    std::vector<Vector3> vertex;
    
    void init();
    void update();
    void render();
};

class LoadMD3  
{

public:
    Vector3 position;
    Quaternion rotation;
    Matrix trasform;


    LoadMD3();

    ~LoadMD3();


    bool Load(const char *szFileName,     float scale = 0.1f);

    void Debug();
    void update(float dtime);
    void update(int currentFrame, int nexFrame, float pol);
    void render( Shader shader, Matrix mat,Matrix parent);

    u32 getFrameCount() { return numFrames; }

    bool SetTexture(u32 index , u32 texture);
   
    Node3D* getBone(u32 index) { return m_bones[index]; }
    Node3D* getBone(const char* name);

    MD3Animator* getAnimator() { return m_animator; }

  //  void addLink(LoadMD3* link) { m_links.push_back(link); }

    void setLink(const char* name, LoadMD3* link);


    Vector3 GetTagPosition(u32 index);


private:
    void UpdateTags(int currentFrame, int nextFrame, float pol);

    int m_frame {0};
    int m_nextFrame {0};
    float m_poll {0.0f};


 

    MD3Animator* m_animator {nullptr};
 

    std::vector<tMD3Surface> m_surfaces;
    std::vector<Bone3D> m_links;
    std::vector<tMd3Tag> m_tags;
   


  //  tMd3Tag* pTags{nullptr};

    u32 numOfTags{0};
    u32 numFrames{0};
    
    
	tMd3Header				m_Header ;			 
 
	 


 

    std::vector<Node3D*> m_bones;
		 
};


// struct MD3Animation 
// {
//     std::string name;
//     int startFrame;
//     int endFrame;
//     float fps;
//     bool loop;
    
//     MD3Animation(const std::string& n, int start, int end, float f, bool l = true)
//         : name(n), startFrame(start), endFrame(end), fps(f), loop(l) {}
// };

// // Estados do animator
// enum AnimatorState 
// {
//     STOPPED,
//     PLAYING,
//     PAUSED,
//     TRANSITIONING
// };

// class MD3Animator 
// {
// private:
//     std::map<std::string, MD3Animation*> animations;
//     std::string currentAnimation;
//     std::string nextAnimation;
//     std::string trasitionName;
//     float currentTime;
//     float transitionTime;
//     float transitionDuration;
    
//     LoadMD3* md3Model;
//     AnimatorState state;
    
// public:
//     MD3Animator(LoadMD3* model) : currentTime(0.0f), transitionTime(0.0f), transitionDuration(0.3f), md3Model(model), state(STOPPED){}
//     ~MD3Animator() 
//     {
//         for (auto& animation : animations) 
//         {
//             delete animation.second;
//         }
//     }
    
    
//     void AddAnimation(const std::string& name, int startFrame, int endFrame, float fps, bool loop = true) 
//     {
//         if (animations.find(name) != animations.end()) 
//         {
//             LogError("Animation '%s' already exists!", name.c_str());
//             return;
//         }
 
//      animations[name] = new MD3Animation(name, startFrame, startFrame+endFrame, fps, loop);

//     }
    
 
  
    
 
//     void PlayAnimation(const std::string& name, bool forceRestart = false) 
//     {
//         if (animations.find(name) == animations.end()) 
//         {
//             LogError("Animation '%s' not found!", name.c_str());
//             return;
//         }
        
//         if (currentAnimation == name && !forceRestart && state == PLAYING) 
//         {
//             return; 
//         }
        
//         currentAnimation = name;
//         currentTime = 0.0f;
//         state = PLAYING;
//         transitionTime = 0.0f;
        
//         LogInfo("Playing animation: %s", name.c_str());
//     }
    
//     void TransitionToAnimation(const std::string& name, float duration = 0.3f)
//      {
//         if (animations.find(name) == animations.end()) 
//         {
//             LogError("Animation '%s' not found!", name.c_str());
//             return;
//         }
        
//         if (currentAnimation == name) return;
//         if (trasitionName == name) return;
        
//         nextAnimation = name;
//         transitionDuration = duration;
//         transitionTime = 0.0f;
//         state = TRANSITIONING;
//         trasitionName = name;
        
//         LogInfo("Transitioning to animation: %s", name.c_str());
//     }
    
 
//     void Stop() 
//     {
//         state = STOPPED;
//         currentTime = 0.0f;
//     }
    
 
//     void Pause() { state = PAUSED; }
//     void Resume() { if (state == PAUSED) state = PLAYING; }
    
 
//     void Update(float deltaTime) {
//         if (state == STOPPED || currentAnimation.empty()) return;
        
//         if (state == TRANSITIONING) {
//             UpdateTransition(deltaTime);
//         } else if (state == PLAYING) {
//             UpdateAnimation(deltaTime);
//         }
//     }
    
// private:
//     void UpdateAnimation(float deltaTime) 
//     {
//        if (animations.find(currentAnimation) == animations.end()) return;
        
//         MD3Animation* anim = animations[currentAnimation];
//         currentTime += deltaTime;
        
//         float frameDuration = 1.0f / anim->fps;
//         int totalFrames = anim->endFrame - anim->startFrame + 1;
//         float totalDuration = totalFrames * frameDuration;
        
//         if (anim->loop) 
//         {
//             if (currentTime >= totalDuration) 
//             {
//                 currentTime = fmod(currentTime, totalDuration);
//             }
//         } else {
//             if (currentTime >= totalDuration) 
//             {
//                 currentTime = totalDuration - frameDuration;
//                 state = STOPPED; // Para no final se não for loop
//             }
//         }
//     }
    
//     void UpdateTransition(float deltaTime) 
//     {
//         transitionTime += deltaTime;
        
//         if (transitionTime >= transitionDuration) 
//         {
//             currentAnimation = nextAnimation;
//             nextAnimation.clear();
//             currentTime = 0.0f;
//             state = PLAYING;
//             transitionTime = 0.0f;
//         }
//     }
    
// public:
//     // Obter frames atuais para renderização
//     void GetCurrentFrames(int& currentFrame, int& nextFrame, float& interpolation) 
//     {
//         if (state == STOPPED || currentAnimation.empty()) {
//             currentFrame = nextFrame = 0;
//             interpolation = 0.0f;
//             return;
//         }
        
//         if (state == TRANSITIONING && !currentAnimation.empty() && !nextAnimation.empty()) 
//         {
//             GetTransitionFrames(currentFrame, nextFrame, interpolation);
//             return;
//         }
        
//         MD3Animation* anim = animations[currentAnimation];
//         float frameDuration = 1.0f / anim->fps;
        
//         float frameFloat = currentTime / frameDuration;
//         int frameIndex = (int)frameFloat;
//         interpolation = frameFloat - frameIndex;
        
//         currentFrame = anim->startFrame + frameIndex;
//         nextFrame = currentFrame + 1;
        
//         // Verificar limites
//         if (nextFrame > anim->endFrame) 
//         {
//             if (anim->loop) 
//             {
//                 nextFrame = anim->startFrame;
//             } else {
//                 nextFrame = anim->endFrame;
//                 interpolation = 0.0f;
//             }
//         }
//     }
    
// private:
//     void GetTransitionFrames(int& currentFrame, int& nextFrame, float& interpolation) 
//     {
//         // Durante transição, mistura entre animação atual e próxima
//         float transitionRatio = transitionTime / transitionDuration;
        
//         MD3Animation* currentAnim = animations[currentAnimation];
//         MD3Animation* nextAnim = animations[nextAnimation];
        
//         // Frame da animação atual
//         float frameDuration = 1.0f / currentAnim->fps;
//         float frameFloat = currentTime / frameDuration;
//         int frameIndex = (int)frameFloat;
        
//         currentFrame = currentAnim->startFrame + frameIndex;
//         if (currentFrame > currentAnim->endFrame) 
//         {
//             if (currentAnim->loop) 
//             {
//                 currentFrame = currentAnim->startFrame + (frameIndex % (currentAnim->endFrame - currentAnim->startFrame + 1));
//             } else {
//                 currentFrame = currentAnim->endFrame;
//             }
//         }
        
//         // Frame da próxima animação (começa do início)
//         nextFrame = nextAnim->startFrame;
        
//         // Interpolação baseada no tempo de transição
//         interpolation = transitionRatio;
//     }
    
// public:
 
//     bool IsPlaying() const { return state == PLAYING || state == TRANSITIONING; }
//     bool IsPlaying(const std::string& name) const {

//         return currentAnimation == name && (state == PLAYING || state == TRANSITIONING); 
//     }
    
//     std::string GetCurrentAnimation() const { return currentAnimation; }
//     AnimatorState GetState() const { return state; }
    
 
//     void SetTransitionDuration(float duration) { transitionDuration = duration; }
// };


#include <functional>

struct MD3Animation 
{
    std::string name;
    int startFrame;
    int endFrame;
    float fps;
    bool loop;
    
    MD3Animation(const std::string& n, int start, int end, float f, bool l = true)
        : name(n), startFrame(start), endFrame(end), fps(f), loop(l) {}
};

// Estados do animator
enum AnimatorState 
{
    STOPPED,
    PLAYING,
    PAUSED,
    TRANSITIONING
};

class MD3Animator 
{
private:
    std::map<std::string, MD3Animation*> animations;
    std::string currentAnimation;
    std::string nextAnimation;
    std::string trasitionName;
    std::string queuedAnimation; 
    float currentTime;
    float transitionTime;
    float transitionDuration;
    u32 m_current_frame{ 0 };
    LoadMD3* md3Model;
    AnimatorState state;
    
    // Callback para quando uma animação termina
    std::function<void(const std::string&)> onAnimationComplete;
    
public:
    MD3Animator(LoadMD3* model) : currentTime(0.0f), transitionTime(0.0f), transitionDuration(0.3f), md3Model(model), state(STOPPED){}
    ~MD3Animator() 
    {
        for (auto& animation : animations) 
        {
            delete animation.second;
        }
    }
    
    void AddAnimation(const std::string& name, int startFrame, int endFrame, float fps, bool loop = true) 
    {
        if (animations.find(name) != animations.end()) 
        {
            LogError("Animation '%s' already exists!", name.c_str());
            return;
        }
 
        animations[name] = new MD3Animation(name, startFrame, startFrame+endFrame, fps, loop);
    }
    
    void PlayAnimation(const std::string& name, bool forceRestart = false) 
    {
        if (animations.find(name) == animations.end()) 
        {
            LogError("Animation '%s' not found!", name.c_str());
            return;
        }
        
        if (currentAnimation == name && !forceRestart && state == PLAYING) 
        {
            return; 
        }
        
        currentAnimation = name;
        currentTime = 0.0f;
        state = PLAYING;
        transitionTime = 0.0f;
        queuedAnimation.clear(); // Limpa qualquer animação na fila
        
       // LogInfo("Playing animation: %s", name.c_str());
    }
    

    void PlayAnimationThen(const std::string& name, const std::string& nextAnim, bool forceRestart = false)
    {
        PlayAnimation(name, forceRestart);
        queuedAnimation = nextAnim;
    //    LogInfo("Queued animation: %s after %s", nextAnim.c_str(), name.c_str());
    }
    

    void PlayAnimationWithCallback(const std::string& name, std::function<void(const std::string&)> callback, bool forceRestart = false)
    {
        PlayAnimation(name, forceRestart);
        onAnimationComplete = callback;
    }
    

    void PlayAnimationSequence(const std::vector<std::string>& animationNames)
    {
        if (animationNames.empty()) return;
        
        // Criar callback que toca a próxima animação da sequência
        static std::vector<std::string> sequence;
        static int currentIndex;
        static MD3Animator* animator = this;
        
        sequence = animationNames;
        currentIndex = 0;
        
        std::function<void(const std::string&)> sequenceCallback = [&](const std::string& completedAnim)
        {
            currentIndex++;
            if (currentIndex <(int) sequence.size())
            {
                animator->PlayAnimationWithCallback(sequence[currentIndex], sequenceCallback);
            }
            else
            {
                LogInfo("Animation sequence completed!");
            }
        };
        
        PlayAnimationWithCallback(animationNames[0], sequenceCallback);
    }
    
    void TransitionToAnimation(const std::string& name, float duration = 0.3f)
    {
        if (animations.find(name) == animations.end()) 
        {
            LogError("Animation '%s' not found!", name.c_str());
            return;
        }
        
        if (currentAnimation == name) return;
        if (trasitionName == name) return;
        
        nextAnimation = name;
        transitionDuration = duration;
        transitionTime = 0.0f;
        state = TRANSITIONING;
        trasitionName = name;
        queuedAnimation.clear(); // Limpa fila durante transição
        
     //   LogInfo("Transitioning to animation: %s", name.c_str());
    }
    
    void Stop() 
    {
        state = STOPPED;
        currentTime = 0.0f;
        queuedAnimation.clear();
        onAnimationComplete = nullptr;
    }
    
    void Pause() { state = PAUSED; }
    void Resume() { if (state == PAUSED) state = PLAYING; }
    
    void Update(float deltaTime) {
        if (state == STOPPED || currentAnimation.empty()) return;
        
        if (state == TRANSITIONING) {
            UpdateTransition(deltaTime);
        } else if (state == PLAYING) {
            UpdateAnimation(deltaTime);
        }
    }

 
    
private:
    void UpdateAnimation(float deltaTime) 
    {
        if (animations.find(currentAnimation) == animations.end()) return;
        
        MD3Animation* anim = animations[currentAnimation];
        currentTime += deltaTime;
        
        float frameDuration = 1.0f / anim->fps;
        int totalFrames = anim->endFrame - anim->startFrame + 1;
        float totalDuration = totalFrames * frameDuration;
        
        if (anim->loop) 
        {
            if (currentTime >= totalDuration) 
            {
                currentTime = fmod(currentTime, totalDuration);
            }
        } else {
            if (currentTime >= totalDuration) 
            {
                currentTime = totalDuration - frameDuration;
                
                // Animação terminou - executar callback ou próxima animação
                std::string completedAnimation = currentAnimation;
                
                if (!queuedAnimation.empty())
                {
                    // Tem animação na fila
                    PlayAnimation(queuedAnimation);
                }
                else if (onAnimationComplete)
                {
                    // Tem callback
                    onAnimationComplete(completedAnimation);
                    onAnimationComplete = nullptr;
                }
                else
                {
                    // Para no final se não for loop
                    state = STOPPED;
                }
            }
        }
    }
    
    void UpdateTransition(float deltaTime) 
    {
        transitionTime += deltaTime;
        
        if (transitionTime >= transitionDuration) 
        {
            currentAnimation = nextAnimation;
            nextAnimation.clear();
            currentTime = 0.0f;
            state = PLAYING;
            transitionTime = 0.0f;
        }
    }
    
public:
    // Obter frames atuais para renderização
    void GetCurrentFrames(int& currentFrame, int& nextFrame, float& interpolation) 
    {
        if (state == STOPPED || currentAnimation.empty()) {
            currentFrame = nextFrame = 0;
            interpolation = 0.0f;
            return;
        }
        
        if (state == TRANSITIONING && !currentAnimation.empty() && !nextAnimation.empty()) 
        {
            GetTransitionFrames(currentFrame, nextFrame, interpolation);
            return;
        }
        
        MD3Animation* anim = animations[currentAnimation];
        float frameDuration = 1.0f / anim->fps;
        
        float frameFloat = currentTime / frameDuration;
        int frameIndex = (int)frameFloat;
        interpolation = frameFloat - frameIndex;
        
        currentFrame = anim->startFrame + frameIndex;
        nextFrame = currentFrame + 1;

        m_current_frame = currentFrame;
        
        // Verificar limites
        if (nextFrame > anim->endFrame) 
        {
            if (anim->loop) 
            {
                nextFrame = anim->startFrame;
            } else {
                nextFrame = anim->endFrame;
                interpolation = 0.0f;
            }
        }
    }
    
private:
    void GetTransitionFrames(int& currentFrame, int& nextFrame, float& interpolation) 
    {
        // Durante transição, mistura entre animação atual e próxima
        float transitionRatio = transitionTime / transitionDuration;
        
        MD3Animation* currentAnim = animations[currentAnimation];
        MD3Animation* nextAnim = animations[nextAnimation];
        
        // Frame da animação atual
        float frameDuration = 1.0f / currentAnim->fps;
        float frameFloat = currentTime / frameDuration;
        int frameIndex = (int)frameFloat;
        
        currentFrame = currentAnim->startFrame + frameIndex;
        if (currentFrame > currentAnim->endFrame) 
        {
            if (currentAnim->loop) 
            {
                currentFrame = currentAnim->startFrame + (frameIndex % (currentAnim->endFrame - currentAnim->startFrame + 1));
            } else {
                currentFrame = currentAnim->endFrame;
            }
        }
        
        // Frame da próxima animação (começa do início)
        nextFrame = nextAnim->startFrame;
        
        // Interpolação baseada no tempo de transição
        interpolation = transitionRatio;
    }
    
public:
    bool IsPlaying() const { return state == PLAYING || state == TRANSITIONING; }
    bool IsPlaying(const std::string& name) const {
        return currentAnimation == name && (state == PLAYING || state == TRANSITIONING); 
    }
    
    std::string GetCurrentAnimation() const { return currentAnimation; }
    std::string GetQueuedAnimation() const { return queuedAnimation; }
    AnimatorState GetState() const { return state; }
    u32 GetAnimationCount() const { return animations.size(); }
    u32 GetFrame() const { return m_current_frame; }
    
    void SetTransitionDuration(float duration) { transitionDuration = duration; }
    
    // Limpar callback atual
    void ClearCallback() { onAnimationComplete = nullptr; }
    
    // Verificar se tem animação na fila
    bool HasQueuedAnimation() const { return !queuedAnimation.empty(); }
};

 
/*
 
animator.AddAnimation("idle", 0, 10, 20.0f, true);
animator.AddAnimation("shoot", 11, 25, 30.0f, false);
animator.AddAnimation("reload", 26, 40, 15.0f, false);
 
animator.PlayAnimationThen("shoot", "idle");

// Exemplo 2: Sequência de animações
std::vector<std::string> sequence = {"shoot", "reload", "idle"};
animator.PlayAnimationSequence(sequence);

 
animator.PlayAnimationWithCallback("shoot", [&](const std::string& completedAnim) {
    LogInfo("Tiro disparado! Voltando para idle...");
    animator.PlayAnimation("idle");
});
*/