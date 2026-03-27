#pragma once
#include <vector>
#include <graphics.h>
#include "Managers.h"
#include "GlobalConst.h"
#include "ResManager.h"

// 前置声明
class GameObject;
class Plant;
class Zombie;
class Bullet;
class Sun;

// 游戏状态
enum GameState {
    GAME_MENU,
    GAME_PLAYING,
    GAME_PAUSE,
    GAME_WIN,
    GAME_LOSE
};

// 可选的植物类型
enum PlantType {
    PLANT_NONE,       // 未选中
    PLANT_PEASHOOTER, // 豌豆射手
    PLANT_SUNFLOWER,  // 向日葵
    PLANT_WALLNUT,     // 【新增】坚果墙
    PLANT_SNOWPEA,     // 【新增】寒冰射手
    PLANT_CHERRYBOMB,      // 【新增】樱桃炸弹
    PLANT_POTATOMINE      // 【新增】土豆地雷
};

// 【新增】植物卡片结构体
struct PlantCard {
    PlantType type;       // 对应的植物类型
    const TCHAR* name;    // 植物名称
    int cost;             // 阳光价格
    float coolDown;       // 种植冷却总时长
    float currentCD;      // 当前剩余冷却时间
    bool isUnlocked;      // 是否解锁
};

class Game {
private:
    Game();

    HWND window;
    GameState gameState;
    DWORD lastFrameTime;
    float deltaTime;
    PlantType selectedPlant;
    std::vector<PlantCard> plantCards;
    bool isShovelMode; // 【新增】是否处于铲植物模式

    // 【新增】植物虚影相关
    int ghostMouseX; // 鼠标当前X坐标
    int ghostMouseY; // 鼠标当前Y坐标
    bool isShowingGhost; // 是否显示虚影
    // ======================
    // 【必须有】阳光数量成员变量
    // ======================
    int sunCount;

    // 实体列表
    std::vector<Plant*> plants;
    std::vector<Zombie*> zombies;
    std::vector<Bullet*> bullets;
    std::vector<Sun*> suns;

    // 管理器
    SunManager sunManager;
    WaveManager waveManager;

    // 私有辅助函数
    void processInput();
    void updateLogic();
    void renderFrame();
    void clearDeadObjects();
    bool isGridHasPlant(int row, int col) const; // 检测格子是否有植物
    void drawSunUI(); // 绘制左上角阳光UI

    void drawShovelButton(); // 【新增】绘制铲子按钮
    bool checkShovelClick(int mouseX, int mouseY); // 【新增】检测是否点击了铲子按钮
    bool tryShovelPlant(int mouseX, int mouseY); // 【新增】尝试铲掉植物

    // 【新增】卡片相关函数
    void initPlantCards();    // 初始化植物卡片
    void updatePlantCards();  // 更新卡片冷却
    void drawPlantCards();    // 绘制植物卡片栏
    bool checkCardClick(int mouseX, int mouseY); // 检测卡片点击

    void drawWaveUI();    // 新增：绘制波数UI
    void drawPauseUI();   // 新增：绘制暂停界面
    void drawEndUI();     // 新增：绘制结束界面
    void resetGame();     // 新增：重置游戏

public:
    // 单例模式：全局唯一接口
    static Game& getInstance();

    // 禁止拷贝
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // 游戏主入口
    void run();

    // 实体添加接口
    void addBullet(Bullet* bullet);
    void addPlant(Plant* plant);
    void addZombie(Zombie* zombie);
    void addSun(Sun* sun);

    // 获取管理器
    SunManager& getSunManager();
    WaveManager& getWaveManager();

    // 获取实体列表（碰撞检测用）
    const std::vector<Plant*>& getPlants() const;
    const std::vector<Zombie*>& getZombies() const;
    const std::vector<Bullet*>& getBullets() const;

    ~Game();
};