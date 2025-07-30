#include "pch.h"
#include "animation.hpp"
#include "binaryfile.hpp"
#include "animation.hpp"



MD3Animator::MD3Animator(LoadMD3* model)
    : currentTime(0.0f), transitionTime(0.0f), transitionDuration(0.3f),
      md3Model(model), state(STOPPED) {}

MD3Animator::~MD3Animator()
{
    for (auto& animation : animations)
    {
        delete animation.second;
    }
}

void MD3Animator::AddAnimation(const std::string& name, int startFrame, int endFrame, float fps, bool loop)
{
    if (animations.find(name) != animations.end())
    {
        LogInfo("Animation '%s' already exists!\n", name.c_str());
        return;
    }

    animations[name] = new MD3Animation(name, startFrame, endFrame, fps, loop);
}

void MD3Animator::PlayAnimation(const std::string& name, bool forceRestart)
{
    if (animations.find(name) == animations.end())
    {
        LogInfo("Animation '%s' not found!\n", name.c_str());
        return;
    }

    if (currentAnimation == name && !forceRestart && state == PLAYING)
        return;

    currentAnimation = name;
    currentTime = 0.0f;
    state = PLAYING;
    transitionTime = 0.0f;
    queuedAnimation.clear();
}

void MD3Animator::PlayAnimationThen(const std::string& name, const std::string& nextAnim, bool forceRestart)
{
    PlayAnimation(name, forceRestart);
    queuedAnimation = nextAnim;
}

void MD3Animator::PlayAnimationWithCallback(const std::string& name, std::function<void(const std::string&)> callback, bool forceRestart)
{
    PlayAnimation(name, forceRestart);
    onAnimationComplete = callback;
}

void MD3Animator::PlayAnimationSequence(const std::vector<std::string>& animationNames)
{
    if (animationNames.empty()) return;

    static std::vector<std::string> sequence;
    static int currentIndex;
    static MD3Animator* animator = this;

    sequence = animationNames;
    currentIndex = 0;

    std::function<void(const std::string&)> sequenceCallback = [&](const std::string&)
    {
        currentIndex++;
        if (currentIndex < (int)sequence.size())
        {
            animator->PlayAnimationWithCallback(sequence[currentIndex], sequenceCallback);
        }
    };

    PlayAnimationWithCallback(animationNames[0], sequenceCallback);
}

void MD3Animator::TransitionToAnimation(const std::string& name, float duration)
{
    if (animations.find(name) == animations.end())
    {
        LogInfo("Animation '%s' not found!\n", name.c_str());
        return;
    }

    if (currentAnimation == name || trasitionName == name)
        return;

    nextAnimation = name;
    transitionDuration = duration;
    transitionTime = 0.0f;
    state = TRANSITIONING;
    trasitionName = name;
    queuedAnimation.clear();
}

void MD3Animator::Stop()
{
    state = STOPPED;
    currentTime = 0.0f;
    queuedAnimation.clear();
    onAnimationComplete = nullptr;
}

void MD3Animator::Pause()
{
    state = PAUSED;
}

void MD3Animator::Resume()
{
    if (state == PAUSED) state = PLAYING;
}

void MD3Animator::Update(float deltaTime)
{
    if (state == STOPPED || currentAnimation.empty()) return;

    if (state == TRANSITIONING)
        UpdateTransition(deltaTime);
    else if (state == PLAYING)
        UpdateAnimation(deltaTime);
}

void MD3Animator::UpdateAnimation(float deltaTime)
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
            currentTime = fmod(currentTime, totalDuration);
    }
    else
    {
        if (currentTime >= totalDuration)
        {
            currentTime = totalDuration - frameDuration;

            std::string completedAnim = currentAnimation;

            if (!queuedAnimation.empty())
                PlayAnimation(queuedAnimation);
            else if (onAnimationComplete)
            {
                onAnimationComplete(completedAnim);
                onAnimationComplete = nullptr;
            }
            else
                state = STOPPED;
        }
    }
}

void MD3Animator::UpdateTransition(float deltaTime)
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

void MD3Animator::GetCurrentFrames(int& currentFrame, int& nextFrame, float& interpolation)
{
     if (currentAnimation.empty())
    {
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
   
    // if (isEnded)
    // {
    //   //   currentFrame =   nextFrame = anim->endFrame;
    //  //       nextFrame = currentFrame +1;
    //     // interpolation = 0.0f;
    //      return;
    // }
    float frameDuration = 1.0f / anim->fps;

    float frameFloat = currentTime / frameDuration;
    int frameIndex = (int)frameFloat;
    interpolation = frameFloat - frameIndex;

     if (state == STOPPED)
    {
      //  LogInfo("Animation '%s' is %d stop !\n", currentAnimation.c_str(),lastFrame);
        currentFrame = nextFrame = lastFrame;    
        return;
    }
    currentFrame = anim->startFrame + frameIndex;
    lastFrame = currentFrame;

    nextFrame = currentFrame + 1;
    m_current_frame = currentFrame;

    if (nextFrame > anim->endFrame)
    {
        if (anim->loop)
        {
            nextFrame = anim->startFrame;

        }
        else
        {
  
            nextFrame = anim->endFrame;
            interpolation = 0.0f;
        }
    }
}

void MD3Animator::GetTransitionFrames(int& currentFrame, int& nextFrame, float& interpolation)
{
    float transitionRatio = transitionTime / transitionDuration;

    MD3Animation* currentAnim = animations[currentAnimation];
    MD3Animation* nextAnim = animations[nextAnimation];

    float frameDuration = 1.0f / currentAnim->fps;
    float frameFloat = currentTime / frameDuration;
    int frameIndex = (int)frameFloat;

    currentFrame = currentAnim->startFrame + frameIndex;
    if (currentFrame > currentAnim->endFrame)
    {
        if (currentAnim->loop)
            currentFrame = currentAnim->startFrame + (frameIndex % (currentAnim->endFrame - currentAnim->startFrame + 1));
        else
            currentFrame = currentAnim->endFrame;
    }

    nextFrame = nextAnim->startFrame;
    interpolation = transitionRatio;
}

// GETTERS

bool MD3Animator::IsPlaying() const
{
    return state == PLAYING || state == TRANSITIONING;
}

bool MD3Animator::IsPlaying(const std::string& name) const
{
    return currentAnimation == name && (state == PLAYING || state == TRANSITIONING);
}

std::string MD3Animator::GetCurrentAnimation() const
{
    return currentAnimation;
}

std::string MD3Animator::GetQueuedAnimation() const
{
    return queuedAnimation;
}

AnimatorState MD3Animator::GetState() const
{
    return state;
}

u32 MD3Animator::GetAnimationCount() const
{
    return animations.size();
}

u32 MD3Animator::GetFrame() const
{
    return m_current_frame;
}

void MD3Animator::SetTransitionDuration(float duration)
{
    transitionDuration = duration;
}

void MD3Animator::ClearCallback()
{
    onAnimationComplete = nullptr;
}

bool MD3Animator::HasQueuedAnimation() const
{
    return !queuedAnimation.empty();
}
