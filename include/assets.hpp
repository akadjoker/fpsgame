#pragma once

#include "Config.hpp"
#include <raylib.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <cstdint>

 

class AssetManager 
{
private:
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<std::string, Music> music;
    std::unordered_map<std::string, Shader> shaders;

    static AssetManager* instance;

    AssetManager() = default;

public:
    static AssetManager& getInstance();

    // Textures
    Texture2D loadTexture(const std::string& path, const std::string& name);
    Texture2D getTexture(const std::string& name);
    u32 getTextureID(const std::string& name);
    void unloadTexture(const std::string& path);

    // Sounds
    Sound loadSound(const std::string& path, const std::string& name);
    Sound getSound(const std::string& path);
    void unloadSound(const std::string& path);
    void playSound(const std::string& path);
    bool isSoundPlaying(const std::string& path);

    // Music
    Music loadMusic(const std::string& path);
    Music getMusic(const std::string& path);
    void unloadMusic(const std::string& path);
    void playMusic(const std::string& path);
    void updateMusic();

    // Shaders
    Shader loadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);
    Shader getShader(const std::string& name);
    void unloadShader(const std::string& name);

    // Global management
    void unloadAllAssets();
    void printStats() const;

    bool isTextureLoaded(const std::string& path) const;
    bool isSoundLoaded(const std::string& path) const;
    bool isMusicLoaded(const std::string& path) const;
    bool isShaderLoaded(const std::string& name) const;
};

#define ASSETS AssetManager::getInstance()
#define UNLOAD_ASSETS() ASSETS.unloadAllAssets()
#define LOAD_TEXTURE(path,name) ASSETS.loadTexture(path,name)
#define GET_TEXTURE(name) ASSETS.getTexture(name)
#define GET_TEXTUREID(name) ASSETS.getTextureID(name)
#define LOAD_SOUND(path,name) ASSETS.loadSound(path,name)
#define GET_SOUND(name) ASSETS.getSound(name)
#define PLAY_SOUND(name) ASSETS.playSound(name)
#define LOAD_MUSIC(path) ASSETS.loadMusic(path)
#define PLAY_MUSIC(path) ASSETS.playMusic(path)
#define IS_SOUND_PLAYING(name) ASSETS.isSoundPlaying(name)
#define LOAD_SHADER(name,vs,fs) ASSETS.loadShader(name,vs,fs)
#define GET_SHADER(name) ASSETS.getShader(name)
#define UNLOAD_SHADER(name) ASSETS.unloadShader(name)
