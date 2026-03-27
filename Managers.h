#pragma once
#include <Windows.h>
#include <vector>

// 前置声明
class Zombie;
class Plant;
class Bullet;
class Sun;

// 碰撞检测管理器
class CollisionManager {
public:
    static bool checkCollision(const RECT& rect1, const RECT& rect2);
    // 核心战斗逻辑：处理所有碰撞
    static void processAllCollisions(
        const std::vector<Bullet*>& bullets,
        std::vector<Zombie*>& zombies,
        std::vector<Plant*>& plants,
        float deltaTime
    );
};

// 阳光管理器
class SunManager {
private:
    int currentSun;
public:
    SunManager();
    void addSun(int value);
    bool spendSun(int cost);
    int getCurrentSun() const;
};

// 波次管理器
class WaveManager {
private:
    int currentWave;
    int zombiesToSpawnThisWave; // 本波还需要生成的僵尸数量
    int zombiesSpawnedThisWave;  // 本波已经生成的僵尸数量
    float spawnTimer;
    float waveIntervalTimer;     // 波次间隔计时器
    bool isWaitingForNextWave;   // 是否在等待下一波
    float gameStartTimer; // 【新增】开局等待计时器
    bool isGameStarted;   // 【新增】游戏是否正式开始
public:
    WaveManager();
    void update(float deltaTime);

    int getCurrentWave() const;
    bool isAllWavesComplete() const; // 所有波次是否完成
    void reset(); // 重置波次（重开游戏用）

    // 【新增】检查游戏是否正式开始
    bool isGameFullyStarted() const { return isGameStarted; }
    // 【新增】获取开局等待剩余时间
    float getRemainingStartDelay() const { return gameStartTimer; }
};