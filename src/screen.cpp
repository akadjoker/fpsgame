#include "screen.hpp"

ScreenManager& ScreenManager::Get()
{
    static ScreenManager instance;
    return instance;
}

ScreenManager::~ScreenManager()
{
    // Se quiseres que destrua screens registadas
    if (ownScreens)
    {
        for (auto& kv : registry)
            delete kv.second;
        registry.clear();
    }
}

void ScreenManager::Register(Screen* screen)
{
    if (!screen) { LogError("ScreenManager::Register: screen null"); return; }
    if (screen->name.empty()) { LogError("ScreenManager::Register: name vazio"); return; }

    if (registry.find(screen->name) != registry.end())
    {
        LogError("ScreenManager::Register: '%s' já existe!", screen->name.c_str());
        return;
    }

    registry[screen->name] = screen;
    LogInfo("Screen registada: %s", screen->name.c_str());
}

void ScreenManager::Unregister(const std::string& name)
{
    auto it = registry.find(name);
    if (it == registry.end()) return;

    // Garante que não está na stack
    for (auto* s : stack)
    {
        if (s == it->second)
        {
            LogError("ScreenManager::Unregister: '%s' está na stack!", name.c_str());
            return;
        }
    }

    if (ownScreens) delete it->second;
    registry.erase(it);
}

bool ScreenManager::Exists(const std::string& name) const
{
    return registry.find(name) != registry.end();
}

Screen* ScreenManager::Find(const std::string& name) const
{
    auto it = registry.find(name);
    return it == registry.end() ? nullptr : it->second;
}

Screen* ScreenManager::Current() const
{
    return stack.empty() ? nullptr : stack.back();
}

void ScreenManager::ClearStack(bool callExit)
{
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
    {
        if (callExit) (*it)->OnExit();
    }
    stack.clear();
}

void ScreenManager::BeginFadeOut(const std::string& toName, float duration, bool isPush, bool isSet, bool isPop)
{
    pendingName = toName;
    pendingIsPush = isPush;
    pendingIsSet  = isSet;
    pendingIsPop  = isPop;

    transitionDuration = (duration <= 0.0f) ? 0.001f : duration;
    transitionTimer = 0.0f;
    transitionAlpha = 0.0f;
    transitionState = TransitionState::FadeOut;
}

void ScreenManager::ApplySwitch()
{
    // Executa a ação pendente (set/push/pop)
    if (pendingIsSet)
    {
        // Set: limpar tudo e colocar a nova no topo
        if (Current()) Current()->OnExit();
        ClearStack(false);
        Screen* s = Find(pendingName);
        if (s)
        {
            stack.push_back(s);
            s->OnEnter();
        }
        else
        {
            LogError("ScreenManager::ApplySwitch: Set para '%s' não encontrada!", pendingName.c_str());
        }
    }
    else if (pendingIsPush)
    {
        Screen* top = Current();
        if (top) top->OnPause();

        Screen* s = Find(pendingName);
        if (s)
        {
            stack.push_back(s);
            s->OnEnter();
        }
        else
        {
            LogError("ScreenManager::ApplySwitch: Push para '%s' não encontrada!", pendingName.c_str());
        }
    }
    else if (pendingIsPop)
    {
        if (!stack.empty())
        {
            Screen* top = stack.back();
            top->OnExit();
            stack.pop_back();
        }
        // Retoma a anterior
        if (Current()) Current()->OnResume();
    }

    // Limpa pendências
    pendingName.clear();
    pendingIsPush = pendingIsSet = pendingIsPop = false;
}

void ScreenManager::Set(const std::string& name, bool withFade, float duration)
{
    if (!Exists(name))
    {
        LogError("ScreenManager::Set: screen '%s' não existe!", name.c_str());
        return;
    }

    if (!withFade)
    {
        if (Current()) Current()->OnExit();
        ClearStack(false);
        Screen* s = Find(name);
        stack.push_back(s);
        s->OnEnter();
        return;
    }

    BeginFadeOut(name, duration, /*push=*/false, /*set=*/true, /*pop=*/false);
}

void ScreenManager::Push(const std::string& name, bool withFade, float duration)
{
    if (!Exists(name))
    {
        LogError("ScreenManager::Push: screen '%s' não existe!", name.c_str());
        return;
    }

    if (!withFade)
    {
        Screen* top = Current();
        if (top) top->OnPause();

        Screen* s = Find(name);
        stack.push_back(s);
        s->OnEnter();
        return;
    }

    BeginFadeOut(name, duration, /*push=*/true, /*set=*/false, /*pop=*/false);
}

void ScreenManager::Pop(bool withFade, float duration)
{
    if (stack.empty())
    {
        LogError("ScreenManager::Pop: stack vazia!");
        return;
    }

    if (!withFade)
    {
        Screen* top = stack.back();
        top->OnExit();
        stack.pop_back();
        if (Current()) Current()->OnResume();
        return;
    }

    BeginFadeOut(/*toName*/"", duration, /*push=*/false, /*set=*/false, /*pop=*/true);
}

void ScreenManager::HandleInput()
{
    if (IsTransitioning())
    {
        // Normalmente ignoramos input durante transições
        return;
    }

    // Só a(s) screen(s) visíveis recebem input.
    // Se o topo for popup, a de baixo ainda pode desenhar, mas o input vai para o topo.
    Screen* top = Current();
    if (top) top->HandleInput();
}

void ScreenManager::Update(float dt)
{
    if (IsTransitioning())
    {
        transitionTimer += dt;
        float t = (transitionTimer / transitionDuration);
        if (t < 0.0f) t = 0.0f;

        if (transitionState == TransitionState::FadeOut)
        {
            transitionAlpha = (t > 1.0f) ? 1.0f : t;
            if (t >= 1.0f)
            {
                // fazer switch
                transitionState = TransitionState::Switch;
                ApplySwitch();
                // começar fade in
                transitionState = TransitionState::FadeIn;
                transitionTimer = 0.0f;
                transitionAlpha = 1.0f;
            }
        }
        else if (transitionState == TransitionState::FadeIn)
        {
            transitionAlpha = 1.0f - ((t > 1.0f) ? 1.0f : t);
            if (t >= 1.0f)
            {
                transitionAlpha = 0.0f;
                transitionState = TransitionState::None;
            }
        }
        return;
    }

    // Update das screens (apenas as ativas)
    // Regra: atualiza do fundo até ao topo, mas podes alterar conforme o design.
    for (auto* s : stack)
        s->Update(dt);
}

void ScreenManager::Render()
{
    // Determinar a partir de onde desenhar (se topo é popup, desenhar também a de baixo)
    int startIndex = 0;
    if (!stack.empty())
    {
        // Caminha de cima para baixo até encontrar uma screen não-popup,
        // e desenha a partir dela (incluindo popups acima).
        int i = (int)stack.size() - 1;
        for (; i >= 0; --i)
        {
            if (!stack[i]->isPopup)
                break;
        }
        startIndex = (i < 0) ? 0 : i; // se todas forem popups, desenha todas
    }

    // Desenhar screens visíveis
    for (int i = startIndex; i < (int)stack.size(); ++i)
        stack[i]->Render();

    // Desenhar fade se estiver em transição
    if (IsTransitioning() && transitionAlpha > 0.0f)
    {
        // overlay full-screen
        Color c = transitionColor;
        c.a = (unsigned char)(transitionAlpha * 255.0f);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), c);
    }
}
