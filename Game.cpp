#include "Game.h"
#include "GameObject.h"
#include "Plant.h"
#include "Zombie.h"
#include "Bullet.h"
#include "Sun.h"
#include "GlobalConst.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

// 【新增】定义RGBA宏，解决标识符找不到的错误
#define RGBA(r,g,b,a) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))

// 屏蔽警告
#pragma warning(disable:28159)

using namespace std;

// --- Game 构造与单例 ---
Game::Game() : window(nullptr), gameState(GAME_PLAYING), deltaTime(0.0f), selectedPlant(PLANT_NONE),
ghostMouseX(0), ghostMouseY(0), isShowingGhost(false) {
    // 【必须加这一行】游戏启动时，先加载所有资源
    ResManager::getInstance().loadAllResources();
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    window = GetHWnd();
    lastFrameTime = GetTickCount();
    srand((unsigned int)GetTickCount());
    initPlantCards();
}

Game& Game::getInstance() {
    static Game instance;
    return instance;
}

Game::~Game() {
    for (auto p : plants) delete p;
    for (auto z : zombies) delete z;
    for (auto b : bullets) delete b;
    for (auto s : suns) delete s;
    closegraph();
}

// --- Game 实体添加 ---
void Game::addBullet(Bullet* bullet) { bullets.push_back(bullet); }
void Game::addPlant(Plant* plant) { plants.push_back(plant); }
void Game::addZombie(Zombie* zombie) { zombies.push_back(zombie); }
void Game::addSun(Sun* sun) { suns.push_back(sun); }

SunManager& Game::getSunManager() { return sunManager; }
WaveManager& Game::getWaveManager() { return waveManager; }

const vector<Plant*>& Game::getPlants() const { return plants; }
const vector<Zombie*>& Game::getZombies() const { return zombies; }
const vector<Bullet*>& Game::getBullets() const { return bullets; }

// --- Game 主循环 ---
void Game::run() {
    while (IsWindow(window)) {
        DWORD currentTime = GetTickCount();
        deltaTime = (currentTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentTime;

        processInput();
        updateLogic();
        renderFrame();

        Sleep(1);
    }
}

// --- Game 输入处理 ---
void Game::processInput() {
    ExMessage msg;
    while (peekmessage(&msg, EM_MOUSE | EM_KEY)) {
        // ==========================================
        // 【最高优先级：窗口关闭事件，任何状态都处理】
        // ==========================================
        if (msg.message == WM_CLOSE) {
            DestroyWindow(window);
            return;
        }

        // ==========================================
        // 【第二优先级：全局按键事件，任何状态都处理】
        // 包括暂停/继续、重开游戏，不管游戏是玩还是暂停，都能响应
        // ==========================================
        if (msg.message == WM_KEYDOWN) {
            // 按P键：暂停/继续
            if (msg.vkcode == 'P') {
                if (gameState == GAME_PLAYING) {
                    gameState = GAME_PAUSE;
                    cout << "游戏暂停" << endl;
                }
                else if (gameState == GAME_PAUSE) {
                    gameState = GAME_PLAYING;
                    cout << "游戏继续" << endl;
                }
            }
            // 按R键：重置游戏
            if (msg.vkcode == 'R') {
                resetGame();
                cout << "游戏重新开始" << endl;
            }
        }

        // ==========================================
        // 【第三优先级：游戏内操作，只有PLAYING状态才处理】
        // 包括点击阳光、种植植物，只有游戏正常运行时才能操作
        // ==========================================
        if (gameState != GAME_PLAYING) {
            continue; // 非游戏运行状态，直接跳过下面的游戏内操作
        }

        // 鼠标左键点击事件：游戏内操作
        if (msg.message == WM_LBUTTONDOWN) {
            int mouseX = msg.x;
            int mouseY = msg.y;

            // 1. 检测是否点击了铲子按钮（优先级最高）
            bool hasClickedShovel = checkShovelClick(mouseX, mouseY);
            if (hasClickedShovel) continue;

            // 2. 如果处于铲植物模式，尝试铲植物
            if (isShovelMode) {
                bool hasShoveled = tryShovelPlant(mouseX, mouseY);
                if (hasShoveled) continue;
                // 没铲到植物，不执行其他逻辑
                continue;
            }

            // 1. 检测是否点击了阳光
            bool hasClickedSun = false;
            for (auto& sun : suns) {
                if (!sun->isAlive()) continue;
                if (sun->isClickOnSun(mouseX, mouseY)) {
                    sunManager.addSun(sun->getSunValue());
                    sun->takeDamage(1);
                    _tprintf(_T("收集阳光！当前阳光：%d\n"), sunManager.getCurrentSun());
                    hasClickedSun = true;
                    break;
                }
            }
            if (hasClickedSun) continue;

            // 2. 检测是否点击了植物卡片
            // 【核心修改】如果点击了卡片，直接continue，不执行后面的种植逻辑
            bool hasClickedCard = checkCardClick(mouseX, mouseY);
            if (hasClickedCard) continue;

            // 3. 种植植物逻辑
            if (selectedPlant == PLANT_NONE) continue;

            int col = (mouseX - LAWN_OFFSET_X) / CELL_SIZE;
            int row = (mouseY - LAWN_OFFSET_Y) / CELL_SIZE;

            // 校验坐标是否在草坪范围内
            if (col < 0 || col >= GRID_COL_COUNT || row < 0 || row >= GRID_ROW_COUNT) {
                continue;
            }

            // 种植限制：格子已有植物
            if (isGridHasPlant(row, col)) {
                _tprintf(_T("该格子已经有植物了！\n"));
                continue;
            }

            // 根据选中的植物执行种植
            switch (selectedPlant) {
            case PLANT_PEASHOOTER: {
                int cost = PEA_SHOOTER_COST;
                if (!sunManager.spendSun(cost)) {
                    _tprintf(_T("阳光不足！需要%d，当前%d\n"), cost, sunManager.getCurrentSun());
                    continue;
                }
                for (auto& card : plantCards) {
                    if (card.type == PLANT_PEASHOOTER) {
                        card.currentCD = card.coolDown;
                        break;
                    }
                }
                _tprintf(_T("种下豌豆射手：行%d，列%d，剩余阳光：%d\n"), row, col, sunManager.getCurrentSun());
                addPlant(new Peashooter(row, col));
                selectedPlant = PLANT_NONE;
                break;
            }
            case PLANT_SUNFLOWER: {
                int cost = 50;
                if (!sunManager.spendSun(cost)) {
                    _tprintf(_T("阳光不足！需要%d，当前%d\n"), cost, sunManager.getCurrentSun());
                    continue;
                }
                for (auto& card : plantCards) {
                    if (card.type == PLANT_SUNFLOWER) {
                        card.currentCD = card.coolDown;
                        break;
                    }
                }
                _tprintf(_T("种下向日葵：行%d，列%d，剩余阳光：%d\n"), row, col, sunManager.getCurrentSun());
                addPlant(new Sunflower(row, col));
                selectedPlant = PLANT_NONE;
                break;
            }
            case PLANT_WALLNUT: {
                int cost = WALLNUT_COST;
                if (!sunManager.spendSun(cost)) {
                    _tprintf(_T("阳光不足！需要%d，当前%d\n"), cost, sunManager.getCurrentSun());
                    continue;
                }
                for (auto& card : plantCards) {
                    if (card.type == PLANT_WALLNUT) {
                        card.currentCD = card.coolDown;
                        break;
                    }
                }
                _tprintf(_T("种下坚果墙：行%d，列%d，剩余阳光：%d\n"), row, col, sunManager.getCurrentSun());
                addPlant(new Wallnut(row, col));
                selectedPlant = PLANT_NONE;
                break;
            }
            case PLANT_SNOWPEA: {
                int cost = SNOWPEA_COST;
                if (!sunManager.spendSun(cost)) {
                    _tprintf(_T("阳光不足！需要%d，当前%d\n"), cost, sunManager.getCurrentSun());
                    continue;
                }
                for (auto& card : plantCards) {
                    if (card.type == PLANT_SNOWPEA) {
                        card.currentCD = card.coolDown;
                        break;
                    }
                }
                _tprintf(_T("种下寒冰射手：行%d，列%d，剩余阳光：%d\n"), row, col, sunManager.getCurrentSun());
                addPlant(new SnowPea(row, col));
                selectedPlant = PLANT_NONE;
                break;
            }
            case PLANT_CHERRYBOMB: {
                int cost = CHERRYBOMB_COST;
                if (!sunManager.spendSun(cost)) {
                    _tprintf(_T("阳光不足！需要%d，当前%d\n"), cost, sunManager.getCurrentSun());
                    continue;
                }
                for (auto& card : plantCards) {
                    if (card.type == PLANT_CHERRYBOMB) {
                        card.currentCD = card.coolDown;
                        break;
                    }
                }
                _tprintf(_T("种下樱桃炸弹：行%d，列%d，剩余阳光：%d\n"), row, col, sunManager.getCurrentSun());
                addPlant(new CherryBomb(row, col));
                selectedPlant = PLANT_NONE;
                break;
            }
            // 【新增】土豆地雷种植分支
            case PLANT_POTATOMINE: {
                int cost = POTATOMINE_COST;
                if (!sunManager.spendSun(cost)) {
                    _tprintf(_T("阳光不足！需要%d，当前%d\n"), cost, sunManager.getCurrentSun());
                    return;
                }
                // 设置冷却
                for (auto& card : plantCards) {
                    if (card.type == PLANT_POTATOMINE) {
                        card.currentCD = card.coolDown;
                        break;
                    }
                }
                _tprintf(_T("种下土豆地雷：行%d，列%d，剩余阳光：%d\n"), row, col, sunManager.getCurrentSun());
                addPlant(new PotatoMine(row, col));
                selectedPlant = PLANT_NONE;
                break;
            }
            default:
                break;
            }
        }
    }
}

// --- Game 逻辑更新 (核心战斗在这里) ---
void Game::updateLogic() {
    if (gameState != GAME_PLAYING) return;

    waveManager.update(deltaTime);
    // 【新增】更新植物卡片冷却
    updatePlantCards();

    for (auto& plant : plants) plant->update(deltaTime);
    for (auto& zombie : zombies) zombie->update(deltaTime);
    for (auto& bullet : bullets) bullet->update(deltaTime);
    for (auto& sun : suns) sun->update(deltaTime);

    // 【新增】核心：处理所有碰撞
    CollisionManager::processAllCollisions(bullets, zombies, plants, deltaTime);

    // 游戏失败判定：有僵尸走到屏幕最左侧
    for (auto zombie : zombies) {
        if (!zombie->isAlive() && zombie->getX() < 0) {
            gameState = GAME_LOSE;
            MessageBox(window, _T("僵尸进家了！游戏失败！"), _T("游戏结束"), MB_OK);
            DestroyWindow(window); // 关闭游戏
            return;
        }
    }

    // 【新增】5. 游戏胜利判定
    if (waveManager.isAllWavesComplete()) {
        // 检查场上是否还有活着的僵尸
        bool hasAliveZombie = false;
        for (auto zombie : zombies) {
            if (zombie->isAlive()) {
                hasAliveZombie = true;
                break;
            }
        }
        if (!hasAliveZombie) {
            gameState = GAME_WIN;
            MessageBox(window, _T("恭喜你！成功抵御了所有僵尸！游戏胜利！"), _T("游戏结束"), MB_OK);
            DestroyWindow(window);
            return;
        }
    }

    clearDeadObjects();
}

// 【新增】绘制阳光UI（最顶层，先画背景，再画UI）
void Game::drawSunUI() {
    // 绘制阳光图标
    setfillcolor(YELLOW);
    solidcircle(30, 30, 20);
    setfillcolor(WHITE);
    solidcircle(30, 30, 8);

    // 绘制阳光数量文字
    TCHAR sunText[32];
    _stprintf_s(sunText, _T("阳光：%d"), sunCount);
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(24, 0, _T("宋体"));
    outtextxy(60, 20, sunText);
}

// --- Game 渲染 ---
void Game::renderFrame() {
    // 【新增】先画背景图，替换原来的纯色背景
    IMAGE* bgImg = ResManager::getInstance().getImage(RES_BACKGROUND);
    if (bgImg) {
        putimage(0, 0, bgImg);
    }
    else {
        // 背景加载失败，用原来的绿色纯色后备
        setbkcolor(RGB(34, 139, 34));
        cleardevice();
    }
    drawPlantCards(); // 【新增】绘制植物卡片栏
    drawShovelButton(); // 【新增】绘制铲子按钮
    drawWaveUI(); // 【新增】绘制波数UI

    setlinecolor(RGB(200, 200, 200));
    for (int i = 0; i <= GRID_COL_COUNT; i++) {
        line(i * CELL_SIZE + LAWN_OFFSET_X, LAWN_OFFSET_Y,
            i * CELL_SIZE + LAWN_OFFSET_X, GRID_ROW_COUNT * CELL_SIZE + LAWN_OFFSET_Y);
    }
    for (int j = 0; j <= GRID_ROW_COUNT; j++) {
        line(LAWN_OFFSET_X, j * CELL_SIZE + LAWN_OFFSET_Y,
            GRID_COL_COUNT * CELL_SIZE + LAWN_OFFSET_X, j * CELL_SIZE + LAWN_OFFSET_Y);
    }

    for (auto& sun : suns) sun->draw();
    for (auto& plant : plants) plant->draw();
    for (auto& zombie : zombies) zombie->draw();
    for (auto& bullet : bullets) bullet->draw();

    // 【新增】绘制植物虚影
    if (isShowingGhost) {
        // 计算虚影应该在的网格位置
        int col = (ghostMouseX - LAWN_OFFSET_X) / CELL_SIZE;
        int row = (ghostMouseY - LAWN_OFFSET_Y) / CELL_SIZE;

        // 只有在草坪范围内才显示虚影
        if (col >= 0 && col < GRID_COL_COUNT && row >= 0 && row < GRID_ROW_COUNT) {
            float ghostX = (float)col * CELL_SIZE + CELL_SIZE / 2 + LAWN_OFFSET_X;
            float ghostY = (float)row * CELL_SIZE + CELL_SIZE / 2 + LAWN_OFFSET_Y;

            // 设置半透明混合模式
            setbkmode(TRANSPARENT);
            // 这里用简化的圆形代替，后续可以替换成图片
            if (selectedPlant == PLANT_PEASHOOTER) {
                setfillcolor(GREEN);
                solidcircle((int)ghostX, (int)ghostY, 30);
                setfillcolor(BLUE);
                solidcircle((int)ghostX + 20, (int)ghostY, 10);
            }
            else if (selectedPlant == PLANT_SUNFLOWER) {
                setfillcolor(YELLOW);
                solidcircle((int)ghostX, (int)ghostY, 30);
                setfillcolor(RGB(139, 69, 19));
                solidcircle((int)ghostX, (int)ghostY, 15);
            }
        }
    }

    // 【新增】4. 绘制暂停界面（在最顶层）
    if (gameState == GAME_PAUSE) {
        drawPauseUI();
    }

    FlushBatchDraw();
}

// --- Game 清理死亡对象 ---
void Game::clearDeadObjects() {
    bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Bullet* b) {
        if (!b->isAlive()) { delete b; return true; } return false;
        }), bullets.end());

    plants.erase(remove_if(plants.begin(), plants.end(), [](Plant* p) {
        if (!p->isAlive()) { delete p; return true; } return false;
        }), plants.end());

    zombies.erase(remove_if(zombies.begin(), zombies.end(), [](Zombie* z) {
        if (!z->isAlive()) { delete z; return true; } return false;
        }), zombies.end());

    suns.erase(remove_if(suns.begin(), suns.end(), [](Sun* s) {
        if (!s->isAlive()) { delete s; return true; } return false;
        }), suns.end());
}

// ==============================================
// 【Managers.cpp 的实现内容，直接放在这里避免文件太多】
// ==============================================

// --- GameObject 基类实现 ---
GameObject::GameObject(float initX, float initY, int initHp)
    : x(initX), y(initY), hp(initHp), alive(true) {
}

GameObject::~GameObject() {}

bool GameObject::isAlive() const { return alive; }
void GameObject::takeDamage(int damage) { hp -= damage; if (hp <= 0) alive = false; }

RECT GameObject::getCollisionRect() const {
    RECT rect;
    rect.left = (LONG)x - 15;
    rect.top = (LONG)y - 15;
    rect.right = (LONG)x + 15;
    rect.bottom = (LONG)y + 15;
    return rect;
}

// --- CollisionManager 碰撞管理器实现 (核心战斗逻辑) ---
bool CollisionManager::checkCollision(const RECT& r1, const RECT& r2) {
    return !(r1.right < r2.left || r1.left > r2.right || r1.bottom < r2.top || r1.top > r2.bottom);
}

void CollisionManager::processAllCollisions(
    const vector<Bullet*>& bullets,
    vector<Zombie*>& zombies,
    vector<Plant*>& plants,
    float deltaTime
) {
    // 1. 子弹打僵尸
    for (auto bullet : bullets) {
        if (!bullet->isAlive()) continue;
        for (auto zombie : zombies) {
            if (!zombie->isAlive()) continue;

            // 检测碰撞
            if (checkCollision(bullet->getCollisionRect(), zombie->getCollisionRect())) {
                bullet->takeDamage(1); // 子弹消失
                zombie->takeDamage(bullet->getDamage()); // 僵尸扣血

                // 【核心新增】如果是寒冰子弹，给僵尸施加减速效果
                if (bullet->isIce()) {
                    zombie->applySlow(ZOMBIE_SLOW_DURATION, ZOMBIE_SLOW_FACTOR);
                    cout << "寒冰子弹击中！僵尸减速" << endl;
                }
                cout << "击中僵尸！剩余血量: " << zombie->getHp() << endl;
                break; // 一颗子弹只打一个僵尸
            }
        }
    }

    // 2. 僵尸碰植物 + 啃食逻辑
    for (auto zombie : zombies) {
        if (!zombie->isAlive()) continue;
        bool hasPlantInFront = false;

        // 遍历所有植物，只检测和僵尸同一行的
        for (auto plant : plants) {
            if (!plant->isAlive()) continue;
            if (plant->getGridRow() != zombie->getGridRow()) continue;

            // 检测僵尸和植物是否碰撞
            if (checkCollision(zombie->getCollisionRect(), plant->getCollisionRect())) {
                hasPlantInFront = true;
                zombie->setEating(true); // 切换为啃食状态

                // 攻击冷却计时，到时间就造成伤害
                zombie->getAttackTimer() += deltaTime;
                if (zombie->getAttackTimer() >= zombie->getAttackCD()) {
                    zombie->getAttackTimer() = 0.0f;
                    plant->takeDamage(zombie->getAttackDamage());
                    cout << "僵尸啃食植物！植物剩余血量: " << plant->getHp() << endl;
                }
                break; // 一个僵尸只啃一个植物
            }
        }

        // 前面没有植物，恢复行走状态
        if (!hasPlantInFront) {
            zombie->setEating(false);
        }
    }
}

// --- SunManager 实现 ---
SunManager::SunManager() : currentSun(50) {}
void SunManager::addSun(int value) { currentSun += value; }
bool SunManager::spendSun(int cost) { if (currentSun >= cost) { currentSun -= cost; return true; } return false; }
int SunManager::getCurrentSun() const { return currentSun; }

// --- WaveManager 波次管理器完整实现（含开局等待）---
WaveManager::WaveManager()
    : currentWave(1),
    zombiesToSpawnThisWave(ZOMBIES_PER_WAVE_BASE),
    zombiesSpawnedThisWave(0),
    spawnTimer(0.0f),
    waveIntervalTimer(0.0f),
    isWaitingForNextWave(false),
    gameStartTimer(GAME_START_DELAY), // 【新增】初始化开局等待时间
    isGameStarted(false) {}           // 【新增】游戏未正式开始

void WaveManager::update(float deltaTime) {
    // 【核心新增】第一步：先处理开局等待
    if (!isGameStarted) {
        gameStartTimer -= deltaTime;
        if (gameStartTimer <= 0) {
            isGameStarted = true;
            cout << "开局等待结束，游戏正式开始！" << endl;
        }
        return; // 开局等待期间，不生成任何僵尸
    }

    // 第二步：处理波次间隔
    if (isWaitingForNextWave) {
        waveIntervalTimer -= deltaTime;
        if (waveIntervalTimer <= 0) {
            isWaitingForNextWave = false;
            currentWave++;
            zombiesToSpawnThisWave = ZOMBIES_PER_WAVE_BASE + (currentWave - 1) * ZOMBIES_PER_WAVE_INCREMENT;
            zombiesSpawnedThisWave = 0;
            _tprintf(_T("第 %d 波开始！\n"), currentWave);
        }
        return;
    }

    // 第三步：生成僵尸（已放缓节奏）
    if (zombiesSpawnedThisWave < zombiesToSpawnThisWave) {
        spawnTimer -= deltaTime;
        if (spawnTimer <= 0) {
            spawnTimer = ZOMBIE_SPAWN_CD; // 使用调整后的慢节奏生成间隔
            // 【必须补全】随机生成一行
            int randomRow = rand() % GRID_ROW_COUNT;

            zombiesSpawnedThisWave++;
            cout << "生成了第 " << zombiesSpawnedThisWave << " 只僵尸" << endl;
        }
    }

    // 第四步：检查本波是否结束
    if (zombiesSpawnedThisWave >= zombiesToSpawnThisWave && currentWave < TOTAL_WAVES) {
        if (!isWaitingForNextWave) {
            isWaitingForNextWave = true;
            waveIntervalTimer = WAVE_INTERVAL;
            _tprintf(_T("第 %d 波结束！准备下一波...\n"), currentWave);
        }
    }
}

int WaveManager::getCurrentWave() const { return currentWave; }

bool WaveManager::isAllWavesComplete() const {
    return currentWave >= TOTAL_WAVES && zombiesSpawnedThisWave >= zombiesToSpawnThisWave;
}

void WaveManager::reset() {
    currentWave = 1;
    zombiesToSpawnThisWave = ZOMBIES_PER_WAVE_BASE;
    zombiesSpawnedThisWave = 0;
    spawnTimer = 0.0f;
    waveIntervalTimer = 0.0f;
    isWaitingForNextWave = false;
    gameStartTimer = GAME_START_DELAY; // 【新增】重置开局等待
    isGameStarted = false;             // 【新增】重置游戏开始状态
}


// 【新增】检测格子是否有植物
bool Game::isGridHasPlant(int row, int col) const {
    for (auto& plant : plants) {
        if (!plant->isAlive()) continue;
        if (plant->getGridRow() == row && plant->getGridCol() == col) {
            return true;
        }
    }
    return false;
}

void Game::drawWaveUI() {
    TCHAR waveText[64];

    // 【核心新增】如果还在开局等待，显示倒计时
    if (!waveManager.isGameFullyStarted()) {
        int remainingSeconds = (int)ceil(waveManager.getRemainingStartDelay());
        _stprintf_s(waveText, _T("游戏即将开始：%d 秒"), remainingSeconds);
        settextcolor(RED);
        setbkmode(TRANSPARENT);
        settextstyle(28, 0, _T("宋体"));
        // 居中显示倒计时
        int textWidth = textwidth(waveText);
        outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 - 50, waveText);

        // 显示提示文字
        _stprintf_s(waveText, _T("请先种植向日葵收集阳光！"));
        textWidth = textwidth(waveText);
        outtextxy((WINDOW_WIDTH - textWidth) / 2, WINDOW_HEIGHT / 2 + 20, waveText);
        return;
    }

    // 游戏正式开始后，显示正常的波次信息
    _stprintf_s(waveText, _T("第 %d / %d 波"), waveManager.getCurrentWave(), TOTAL_WAVES);
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(24, 0, _T("宋体"));
    outtextxy(WINDOW_WIDTH - 180, 10, waveText);
}

// ==============================================
// 植物卡片系统实现
// ==============================================
// 初始化植物卡片
void Game::initPlantCards() {
    plantCards = {
        // 豌豆射手卡片
        { PLANT_PEASHOOTER, _T("豌豆射手"), PEA_SHOOTER_COST, PLANT_CD_PEASHOOTER, 0.0f, true},
        // 向日葵卡片
        { PLANT_SUNFLOWER, _T("向日葵"), 50, PLANT_CD_SUNFLOWER, 0.0f, true},
        // 【新增】坚果墙卡片
        { PLANT_WALLNUT, _T("坚果墙"), WALLNUT_COST, WALLNUT_CD, 0.0f, true},
        // 【新增】寒冰射手卡片
        { PLANT_SNOWPEA, _T("寒冰射手"), SNOWPEA_COST, SNOWPEA_CD, 0.0f, true },
        // 【新增】樱桃炸弹卡片
        { PLANT_CHERRYBOMB, _T("樱桃炸弹"), CHERRYBOMB_COST, CHERRYBOMB_CD, 0.0f, true },
        // 【新增】土豆地雷卡片
        { PLANT_POTATOMINE, _T("土豆地雷"), POTATOMINE_COST, POTATOMINE_CD, 0.0f, true }
    };
}

// 更新卡片冷却
void Game::updatePlantCards() {
    for (auto& card : plantCards) {
        if (card.currentCD > 0) {
            card.currentCD -= deltaTime;
            if (card.currentCD < 0) card.currentCD = 0;
        }
    }
}

// 【修改】检测卡片点击，返回true表示点击了卡片
bool Game::checkCardClick(int mouseX, int mouseY) {
    for (int i = 0; i < plantCards.size(); i++) {
        int cardX = CARD_START_X + i * (CARD_WIDTH + CARD_MARGIN);
        int cardY = CARD_START_Y;

        if (mouseX >= cardX && mouseX <= cardX + CARD_WIDTH &&
            mouseY >= cardY && mouseY <= cardY + CARD_HEIGHT) {

            auto& card = plantCards[i];
            // 冷却中、阳光不足，不能选中
            if (card.currentCD > 0 || sunManager.getCurrentSun() < card.cost || !card.isUnlocked) {
                return true; // 虽然不能选，但确实点击了卡片，返回true
            }

            // 切换选中状态
            if (selectedPlant == card.type) {
                selectedPlant = PLANT_NONE;
                _tprintf(_T("取消选中植物\n"));
            }
            else {
                selectedPlant = card.type;
                _tprintf(_T("选中：%s（需要%d阳光）\n"), card.name, card.cost);
            }
            return true; // 点击了卡片，返回true
        }
    }
    return false; // 没有点击卡片，返回false
}

// 绘制植物卡片栏
void Game::drawPlantCards() {
    int currentSun = sunManager.getCurrentSun();

    for (int i = 0; i < plantCards.size(); i++) {
        auto& card = plantCards[i];
        int cardX = CARD_START_X + i * (CARD_WIDTH + CARD_MARGIN);
        int cardY = CARD_START_Y;

        // 1. 绘制卡片背景
        COLORREF bgColor;
        // 冷却中 → 深灰色
        if (card.currentCD > 0) {
            bgColor = RGB(80, 80, 80);
        }
        // 阳光不足 → 浅灰色
        else if (currentSun < card.cost) {
            bgColor = RGB(150, 150, 150);
        }
        // 正常可选中 → 米黄色
        else {
            bgColor = RGB(255, 250, 205);
        }
        setfillcolor(bgColor);
        solidrectangle(cardX, cardY, cardX + CARD_WIDTH, cardY + CARD_HEIGHT);

        // 2. 选中的卡片，画高亮绿色边框
        if (selectedPlant == card.type) {
            setlinecolor(GREEN);
            setlinestyle(PS_SOLID, 3);
            rectangle(cardX - 2, cardY - 2, cardX + CARD_WIDTH + 2, cardY + CARD_HEIGHT + 2);
            setlinestyle(PS_SOLID, 1); // 恢复默认线条样式
        }

        // 3. 绘制植物图标（简化版，用圆形代替，后续可以替换成图片）
        if (card.type == PLANT_PEASHOOTER) {
            setfillcolor(GREEN);
            solidcircle(cardX + CARD_WIDTH / 2, cardY + 30, 20);
            setfillcolor(BLUE);
            solidcircle(cardX + CARD_WIDTH / 2 + 10, cardY + 30, 8);
        }
        else if (card.type == PLANT_SUNFLOWER) {
            setfillcolor(YELLOW);
            solidcircle(cardX + CARD_WIDTH / 2, cardY + 30, 20);
            setfillcolor(RGB(139, 69, 19));
            solidcircle(cardX + CARD_WIDTH / 2, cardY + 30, 10);
        }
        // 【新增】坚果墙卡片图标
        else if (card.type == PLANT_WALLNUT) {
            setfillcolor(RGB(139, 69, 19));
            solidrectangle(cardX + CARD_WIDTH / 2 - 20, cardY + 10, cardX + CARD_WIDTH / 2 + 20, cardY + 50);
        }
        // 【新增】寒冰射手卡片图标
        else if (card.type == PLANT_SNOWPEA) {
            setfillcolor(GREEN);
            solidcircle(cardX + CARD_WIDTH / 2, cardY + 30, 20);
            setfillcolor(RGB(135, 206, 250)); // 天蓝色
            solidcircle(cardX + CARD_WIDTH / 2 + 10, cardY + 30, 8);
        }
        // 【新增】樱桃炸弹卡片图标
        else if (card.type == PLANT_CHERRYBOMB) {
            setfillcolor(RGB(220, 20, 60)); // 猩红色
            solidcircle(cardX + CARD_WIDTH / 2 - 10, cardY + 30, 12);
            solidcircle(cardX + CARD_WIDTH / 2 + 10, cardY + 30, 12);
        }
        else if (card.type == PLANT_POTATOMINE) {
            setfillcolor(RGB(205, 133, 63)); // 土豆色
            solidcircle(cardX + CARD_WIDTH / 2, cardY + 30, 18);
            setfillcolor(BLACK);
            solidcircle(cardX + CARD_WIDTH / 2 - 8, cardY + 25, 2);
            solidcircle(cardX + CARD_WIDTH / 2 + 8, cardY + 25, 2);
        }

        // 4. 绘制阳光价格
        settextcolor(BLACK);
        setbkmode(TRANSPARENT);
        settextstyle(16, 0, _T("宋体"));
        TCHAR costText[10];
        _stprintf_s(costText, _T("%d"), card.cost);
        outtextxy(cardX + CARD_WIDTH / 2 - 8, cardY + CARD_HEIGHT - 20, costText);

        // 5. 绘制冷却遮罩
        if (card.currentCD > 0) {
            // 计算冷却遮罩的高度
            float cdPercent = card.currentCD / card.coolDown;
            int maskHeight = (int)(CARD_HEIGHT * cdPercent);
            setfillcolor(RGBA(0, 0, 0, 150)); // 半透明黑色
            solidrectangle(cardX, cardY, cardX + CARD_WIDTH, cardY + maskHeight);
        }
    }
}

// ==============================================
// 游戏状态管理辅助函数
// ==============================================
void Game::resetGame() {
    // 1. 清理所有实体
    for (auto plant : plants) delete plant;
    for (auto zombie : zombies) delete zombie;
    for (auto bullet : bullets) delete bullet;
    for (auto sun : suns) delete sun;
    plants.clear();
    zombies.clear();
    bullets.clear();
    suns.clear();

    // 2. 重置管理器
    sunManager = SunManager();
    waveManager.reset();

    // 3. 重置游戏状态
    gameState = GAME_PLAYING;
    selectedPlant = PLANT_NONE;
    isShovelMode = false; // 【新增】重置铲子模式

    // 4. 重置卡片冷却
    for (auto& card : plantCards) {
        card.currentCD = 0.0f;
    }
}

void Game::drawPauseUI() {
    // 绘制暂停界面（半透明黑色遮罩）
    setfillcolor(RGBA(0, 0, 0, 150));
    solidrectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    // 绘制暂停文字
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(40, 0, _T("宋体"));
    outtextxy(WINDOW_WIDTH / 2 - 60, WINDOW_HEIGHT / 2 - 20, _T("游戏暂停"));
    settextstyle(20, 0, _T("宋体"));
    outtextxy(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 30, _T("按 P 键继续，按 R 键重开"));
}

void Game::drawEndUI() {
    // 结束界面（已经用MessageBox实现，这里可以留空）
}

// ==============================================
// 铲子功能辅助函数
// ==============================================
void Game::drawShovelButton() {
    // 绘制铲子按钮背景
    int buttonX = SHOVEL_BUTTON_X;
    int buttonY = SHOVEL_BUTTON_Y;

    // 铲子模式下，按钮背景变亮
    if (isShovelMode) {
        setfillcolor(RGB(200, 200, 200));
    }
    else {
        setfillcolor(RGB(150, 150, 150));
    }
    solidrectangle(buttonX, buttonY, buttonX + CARD_WIDTH, buttonY + CARD_HEIGHT);

    // 绘制铲子图片
    ResManager::drawTransparentImage(buttonX + 5, buttonY + 5, RES_SHOVEL);


    // 铲子模式下，显示提示文字
    if (isShovelMode) {
        settextcolor(RED);
        setbkmode(TRANSPARENT);
        settextstyle(16, 0, _T("宋体"));
        outtextxy(buttonX - 20, buttonY + CARD_HEIGHT + 5, _T("点击植物铲掉"));
    }
}

bool Game::checkShovelClick(int mouseX, int mouseY) {
    int buttonX = SHOVEL_BUTTON_X;
    int buttonY = SHOVEL_BUTTON_Y;

    if (mouseX >= buttonX && mouseX <= buttonX + CARD_WIDTH &&
        mouseY >= buttonY && mouseY <= buttonY + CARD_HEIGHT) {
        // 点击了铲子按钮，切换模式
        isShovelMode = !isShovelMode;
        if (isShovelMode) {
            selectedPlant = PLANT_NONE; // 铲子模式下，取消植物选中
            cout << "进入铲植物模式" << endl;
        }
        else {
            cout << "退出铲植物模式" << endl;
        }
        return true;
    }
    return false;
}

bool Game::tryShovelPlant(int mouseX, int mouseY) {
    // 计算点击的网格坐标
    int col = (mouseX - LAWN_OFFSET_X) / CELL_SIZE;
    int row = (mouseY - LAWN_OFFSET_Y) / CELL_SIZE;

    // 校验坐标是否在草坪范围内
    if (col < 0 || col >= GRID_COL_COUNT || row < 0 || row >= GRID_ROW_COUNT) {
        return false;
    }

    // 查找该格子的植物
    for (auto it = plants.begin(); it != plants.end(); ++it) {
        Plant* plant = *it;
        if (plant->getGridRow() == row && plant->getGridCol() == col) {
            // 找到了植物，铲掉它
            int refundSun = (int)(plant->getCost() * SHOVEL_SUN_REFUND_RATIO);
            sunManager.addSun(refundSun);
            _tprintf(_T("铲掉植物，回收%d阳光！当前阳光：%d\n"), refundSun, sunManager.getCurrentSun());

            // 删除植物
            delete plant;
            plants.erase(it);

            // 退出铲植物模式
            isShovelMode = false;
            return true;
        }
    }
    return false;
}