#pragma once

// #include "Config.hpp"
// #include <raylib.h>
// #include <unordered_map>
// #include <string>
// #include <iostream>

class AssetManager 
{
private:
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<std::string, Music> music;

 
    static AssetManager* instance;

    AssetManager() = default;

public:
 
    static AssetManager& getInstance()
    {
        if (!instance)
        {
            instance =  new AssetManager();
        }
        return *instance;
    }

 

 
    Texture2D loadTexture(const std::string& path, const std::string& name)
    {
        auto it = textures.find(name);
        if (it != textures.end())
        {
            LogWarning("Texture already loaded: %s", path.c_str());
            return it->second;
        }

        Texture2D texture = LoadTexture(path.c_str());
        if (texture.id == 0)
        {
            LogError("Failed to load texture: %s", path.c_str());
            return texture;
        }

        textures[name] = texture;
        LogInfo("Texture loaded: %s", path.c_str());
        return texture;
    }

    Texture2D getTexture(const std::string& name)
    {
        auto it = textures.find(name);
        if (it != textures.end())
        {
            return it->second;
        }

        // Se não encontrar, tenta carregar automaticamente
        LogError("Texture not found in cache, loading: %s", name.c_str());
        return GetShapesTexture();
    }

    u32 getTextureID(const std::string& name)
    {
        auto it = textures.find(name);
        if (it != textures.end())
        {
            return it->second.id;
        }

        // Se não encontrar, tenta carregar automaticamente
        LogError("Texture not found in cache, loading: %s", name.c_str());
        return 0;
    }

    void unloadTexture(const std::string& path)
    {
        auto it = textures.find(path);
        if (it != textures.end())
        {
            UnloadTexture(it->second);
            textures.erase(it);
            LogInfo("Texture unloaded: %s", path.c_str());
        }
    }

    // Gerenciamento de Sons
    Sound loadSound(const std::string& path, const std::string& name)
    {
        auto it = sounds.find(name);
        if (it != sounds.end())
        {
            LogWarning("Sound already loaded: %s", path.c_str());
            return it->second;
        }

        Sound sound = LoadSound(path.c_str());
        if (sound.stream.buffer == nullptr)
        {
            LogError("Failed to load sound: %s", path.c_str());
            return sound;
        }

        sounds[name] = sound;
        
        return sound;
    }

    Sound getSound(const std::string& path)
    {
        auto it = sounds.find(path);
        if (it != sounds.end())
        {
            return it->second;
        }

        LogError("Sound not found in cache, loading: %s", path.c_str());
        return loadSound(path,"undefined");
    }

    void unloadSound(const std::string& path)
    {
        auto it = sounds.find(path);
        if (it != sounds.end())
        {
            UnloadSound(it->second);
            sounds.erase(it);
            LogInfo("Sound unloaded: %s", path.c_str());
        }
    }

    void playSound(const std::string& path)
    {
        Sound sound = getSound(path);
        if (sound.stream.buffer != nullptr)
        {
            PlaySound(sound);
        }
    }

    bool isSoundPlaying(const std::string& path)
    {
         Sound sound = getSound(path);
        if (sound.stream.buffer != nullptr)
        {
            return IsSoundPlaying(sound);
        }
        return false;
    }
 
    Music loadMusic(const std::string& path)
    {
        auto it = music.find(path);
        if (it != music.end())
        {
            LogWarning("Music already loaded: %s", path.c_str());
            return it->second;
        }

        Music musicStream = LoadMusicStream(path.c_str());
        if (musicStream.stream.buffer == nullptr)
        {
            LogError("Failed to load music: %s", path.c_str());
            return musicStream;
        }

        music[path] = musicStream;
        LogInfo("Music loaded: %s", path.c_str());
        return musicStream;
    }

    Music getMusic(const std::string& path)
    {
        auto it = music.find(path);
        if (it != music.end())
        {
            return it->second;
        }

        LogError("Music not found in cache, loading: %s", path.c_str());
        return loadMusic(path);
    }

    void unloadMusic(const std::string& path)
    {
        auto it = music.find(path);
        if (it != music.end())
        {
            UnloadMusicStream(it->second);
            music.erase(it);
            LogInfo("Music unloaded: %s", path.c_str());
        }
    }

    void playMusic(const std::string& path)
    {
        Music musicStream = getMusic(path);
        if (musicStream.stream.buffer != nullptr)
        {
            PlayMusicStream(musicStream);
        }
    }

    void updateMusic()
    {
        for (auto& pair : music)
        {
            UpdateMusicStream(pair.second);
        }
    }

 
    void unloadAllAssets()
    {
 
        for (auto& pair : textures)
        {
            UnloadTexture(pair.second);
        }
        textures.clear();

 
        for (auto& pair : sounds)
        {
            UnloadSound(pair.second);
        }
        sounds.clear();

 
        for (auto& pair : music)
        {
            UnloadMusicStream(pair.second);
        }
        music.clear();

        LogInfo("All assets unloaded.");
    }

    void printStats() const
    {
        LogInfo("Asset Manager Stats:");
        LogInfo("========================");
        LogInfo("Total Assets: %zu", textures.size() + sounds.size() + music.size());
        LogInfo("Total Textures: %zu", textures.size());
        LogInfo("Total Sounds: %zu", sounds.size());
        LogInfo("Total Music: %zu", music.size());


        if (!textures.empty())
        {
            LogInfo("Textures:");
            for (const auto& pair : textures)
            {
                LogInfo("  - %s", pair.first.c_str());
            }
        }

        if (!sounds.empty())
        {
            LogInfo("Sounds:");
            for (const auto& pair : sounds)
            {
                LogInfo("  - %s", pair.first.c_str());
            }
        
            for (const auto& pair : sounds)
            {
               LogInfo("  - %s", pair.first.c_str());
            }
        }

        if (!music.empty())
        {
           LogInfo("Music:");
            for (const auto& pair : music)
            {
                LogInfo("  - %s", pair.first.c_str());
            }
        }
        LogInfo("========================");
    }

    bool isTextureLoaded(const std::string& path) const
    {
        return textures.find(path) != textures.end();
    }

    bool isSoundLoaded(const std::string& path) const
    {
        return sounds.find(path) != sounds.end();
    }

    bool isMusicLoaded(const std::string& path) const
    {
        return music.find(path) != music.end();
    }
};

 
AssetManager* AssetManager::instance = nullptr;


#define ASSETS AssetManager::getInstance()
#define LOAD_TEXTURE(path,name) ASSETS.loadTexture(path,name)
#define GET_TEXTURE(path) ASSETS.getTexture(path)
#define GET_TEXTUREID(path) ASSETS.getTextureID(path)
#define LOAD_SOUND(path,name) ASSETS.loadSound(path,name)
#define GET_SOUND(path) ASSETS.getSound(path)
#define PLAY_SOUND(path) ASSETS.playSound(path)
#define LOAD_MUSIC(path) ASSETS.loadMusic(path)
#define PLAY_MUSIC(path) ASSETS.playMusic(path)
#define IS_SOUND_PLAYING(path) ASSETS.isSoundPlaying(path)

/*


#include "raylib.h"
#include "AssetManager.h"

int main() {
    InitWindow(800, 600, "Asset Manager Test");
    InitAudioDevice();

    // Carregar assets
    Texture2D playerTexture = LOAD_TEXTURE("assets/player.png");
    Sound jumpSound = LOAD_SOUND("assets/jump.wav");

    // Loop principal
    while (!WindowShouldClose()) {
        ASSETS.updateMusic(); // Importante para música

        if (IsKeyPressed(KEY_SPACE)) {
            PLAY_SOUND("assets/jump.wav");
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(GET_TEXTURE("assets/player.png"), 100, 100, WHITE);
            DrawText("Press SPACE to play sound", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    // Cleanup automático via destructor
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
*/