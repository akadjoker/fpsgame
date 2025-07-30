
#include "assets.hpp"

AssetManager* AssetManager::instance = nullptr;

AssetManager& AssetManager::getInstance()
{
    if (!instance) instance = new AssetManager();
    return *instance;
}

// ---------------- TEXTURES ----------------
Texture2D AssetManager::loadTexture(const std::string& path, const std::string& name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        TraceLog(LOG_WARNING, "Texture already loaded: %s", path.c_str());
        return it->second;
    }

    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load texture: %s", path.c_str());
        return texture;
    }

    textures[name] = texture;
    TraceLog(LOG_INFO, "Texture loaded: %s", path.c_str());
    return texture;
}

Texture2D AssetManager::getTexture(const std::string& name)
{
    auto it = textures.find(name);
    if (it != textures.end()) return it->second;

    TraceLog(LOG_ERROR, "Texture not found: %s", name.c_str());
    return GetShapesTexture();
}

u32 AssetManager::getTextureID(const std::string& name)
{
    auto it = textures.find(name);
    return (it != textures.end()) ? it->second.id : 0;
}

void AssetManager::unloadTexture(const std::string& name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        UnloadTexture(it->second);
        textures.erase(it);
        TraceLog(LOG_INFO, "Texture unloaded: %s", name.c_str());
    }
}

// ---------------- SOUNDS ----------------
Sound AssetManager::loadSound(const std::string& path, const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        TraceLog(LOG_WARNING, "Sound already loaded: %s", path.c_str());
        return it->second;
    }

    Sound sound = LoadSound(path.c_str());
    if (sound.stream.buffer == nullptr)
    {
        TraceLog(LOG_ERROR, "Failed to load sound: %s", path.c_str());
        return sound;
    }

    sounds[name] = sound;
    return sound;
}

Sound AssetManager::getSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end()) return it->second;

    TraceLog(LOG_ERROR, "Sound not found: %s", name.c_str());
    return { 0 };
}

void AssetManager::unloadSound(const std::string& name)
{
    auto it = sounds.find(name);
    if (it != sounds.end())
    {
        UnloadSound(it->second);
        sounds.erase(it);
        TraceLog(LOG_INFO, "Sound unloaded: %s", name.c_str());
    }
}

void AssetManager::playSound(const std::string& name)
{
    Sound sound = getSound(name);
    if (sound.stream.buffer) PlaySound(sound);
}

bool AssetManager::isSoundPlaying(const std::string& name)
{
    Sound sound = getSound(name);
    return sound.stream.buffer ? IsSoundPlaying(sound) : false;
}

// ---------------- MUSIC ----------------
Music AssetManager::loadMusic(const std::string& path)
{
    auto it = music.find(path);
    if (it != music.end())
    {
        TraceLog(LOG_WARNING, "Music already loaded: %s", path.c_str());
        return it->second;
    }

    Music m = LoadMusicStream(path.c_str());
    if (m.stream.buffer == nullptr)
    {
        TraceLog(LOG_ERROR, "Failed to load music: %s", path.c_str());
        return m;
    }

    music[path] = m;
    return m;
}

Music AssetManager::getMusic(const std::string& path)
{
    auto it = music.find(path);
    if (it != music.end()) return it->second;

    return loadMusic(path);
}

void AssetManager::unloadMusic(const std::string& path)
{
    auto it = music.find(path);
    if (it != music.end())
    {
        UnloadMusicStream(it->second);
        music.erase(it);
    }
}

void AssetManager::playMusic(const std::string& path)
{
    Music m = getMusic(path);
    if (m.stream.buffer) PlayMusicStream(m);
}

void AssetManager::updateMusic()
{
    for (auto& m : music) UpdateMusicStream(m.second);
}

// ---------------- SHADERS ----------------
Shader AssetManager::loadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath)
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        TraceLog(LOG_WARNING, "Shader already loaded: %s", name.c_str());
        return it->second;
    }

    Shader shader = LoadShader(vsPath.c_str(), fsPath.c_str());
    if (shader.id == 0)
    {
        TraceLog(LOG_ERROR, "Failed to load shader: %s / %s", vsPath.c_str(), fsPath.c_str());
        return shader;
    }

    shaders[name] = shader;
    return shader;
}

Shader AssetManager::getShader(const std::string& name)
{
    auto it = shaders.find(name);
    if (it != shaders.end()) return it->second;

    TraceLog(LOG_ERROR, "Shader not found: %s", name.c_str());
    return { 0 };
}

void AssetManager::unloadShader(const std::string& name)
{
    auto it = shaders.find(name);
    if (it != shaders.end())
    {
        UnloadShader(it->second);
        shaders.erase(it);
        TraceLog(LOG_INFO, "Shader unloaded: %s", name.c_str());
    }

}

void AssetManager::unloadAllAssets() 
{
    for (auto& texture : textures) UnloadTexture(texture.second);
    for (auto& sound : sounds) UnloadSound(sound.second);
    for (auto& music : music) UnloadMusicStream(music.second);
    for (auto& shader : shaders) UnloadShader(shader.second);
}
