#pragma once
#include "Config.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class Screen
{
public:
    std::string name {"Screen"};
    bool isPopup { false };     // Se true, permite ver a screen de baixo (HUD/menus)

    Screen() = default;
    explicit Screen(std::string n, bool popup=false) : name(std::move(n)), isPopup(popup) {}
    virtual ~Screen() = default;
 
    virtual void OnEnter() {}    
    virtual void OnExit()  {}      
    virtual void OnPause() {}     
    virtual void OnResume(){}      

    // Loop
    virtual void HandleInput() {}        
    virtual void Update(float dt) {}    
    virtual void Render() = 0;             

 
    virtual void Clear(Color c) { ClearBackground(c); }
};

 
class ScreenManager
{
public:
    enum class TransitionState { None, FadeOut, Switch, FadeIn };

    static ScreenManager& Get();

    // Registo e gestão
    void Register(Screen* screen);                    // Regista por screen->name
    void Unregister(const std::string& name);         // Não faz delete (a menos que ownScreens=true)
    bool Exists(const std::string& name) const;

    // Troca direta (limpa a stack e coloca uma nova screen no topo)
    void Set(const std::string& name, bool withFade=true, float duration=0.35f);

    // Empilhar screens (por exemplo: jogo -> push(menu))
    void Push(const std::string& name, bool withFade=false, float duration=0.25f);
    void Pop(bool withFade=false, float duration=0.25f);

    // Loop de jogo
    void HandleInput();
    void Update(float dt);
    void Render();

    // Utilidades
    Screen* Current() const;
    u32 StackSize() const { return (u32)stack.size(); }
    void ClearStack(bool callExit=true);

    // Transição
    void SetTransitionColor(Color c) { transitionColor = c; }
    bool IsTransitioning() const { return transitionState != TransitionState::None; }

    // Ownership
    void SetOwnScreens(bool own) { ownScreens = own; }

private:
    ScreenManager() = default;
    ~ScreenManager();

    ScreenManager(const ScreenManager&) = delete;
    ScreenManager& operator=(const ScreenManager&) = delete;

    // Internos
    Screen* Find(const std::string& name) const;
    void BeginFadeOut(const std::string& toName, float duration, bool isPush, bool isSet, bool isPop);
    void ApplySwitch();  // executa a ação pendente na fase "Switch"

private:
    std::unordered_map<std::string, Screen*> registry;
    std::vector<Screen*> stack;

    // Transição
    TransitionState transitionState { TransitionState::None };
    float transitionTimer { 0.0f };
    float transitionDuration { 0.0f };
    float transitionAlpha { 0.0f };
    Color transitionColor { BLACK };

    // Ação pendente para executar no momento "Switch"
    std::string pendingName;
    bool pendingIsPush { false };
    bool pendingIsSet  { false };
    bool pendingIsPop  { false };

    // Ownership opcional
    bool ownScreens { false };
};

