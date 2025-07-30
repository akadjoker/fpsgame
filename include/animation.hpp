
#pragma once
#include "Config.hpp"


class MD3Animator;
class LoadMD3;


 

 
 
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

// Estados possíveis do Animator
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

    u32 lastFrame {0};
    u32 m_current_frame {0};
    LoadMD3* md3Model;
    AnimatorState state;

    // Callback quando animação termina
    std::function<void(const std::string&)> onAnimationComplete;

public:
    MD3Animator(LoadMD3* model);
    ~MD3Animator();

    // Adicionar e tocar animações
    void AddAnimation(const std::string& name, int startFrame, int endFrame, float fps, bool loop = true);
    void PlayAnimation(const std::string& name, bool forceRestart = false);
    void PlayAnimationThen(const std::string& name, const std::string& nextAnim, bool forceRestart = false);
    void PlayAnimationWithCallback(const std::string& name, std::function<void(const std::string&)> callback, bool forceRestart = false);
    void PlayAnimationSequence(const std::vector<std::string>& animationNames);
    void TransitionToAnimation(const std::string& name, float duration = 0.3f);

    // Controlo geral
    void Stop();
    void Pause();
    void Resume();
    void Update(float deltaTime);

    // Frames atuais
    void GetCurrentFrames(int& currentFrame, int& nextFrame, float& interpolation);

    // Estado e info
    bool IsPlaying() const;
    bool IsPlaying(const std::string& name) const;
    std::string GetCurrentAnimation() const;
    std::string GetQueuedAnimation() const;
    AnimatorState GetState() const;
    u32 GetAnimationCount() const;
    u32 GetFrame() const;

    void SetTransitionDuration(float duration);
    void ClearCallback();
    bool HasQueuedAnimation() const;

private:
    void UpdateAnimation(float deltaTime);
    void UpdateTransition(float deltaTime);
    void GetTransitionFrames(int& currentFrame, int& nextFrame, float& interpolation);
};
