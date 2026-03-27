#include "Plant.h"
#include "Bullet.h"
#include "Game.h"
#include "Sun.h"
#include "Zombie.h"
#include "GlobalConst.h"
#include "ResManager.h"
#include <graphics.h>
#include <iostream>
#include <cmath>
using namespace std;
// --- Plant 基类实现 ---
Plant::Plant(int row, int col, int initHp, int initCost, float initCD)
    : GameObject(
        (float)col* CELL_SIZE + CELL_SIZE / 2 + LAWN_OFFSET_X,
        (float)row* CELL_SIZE + CELL_SIZE / 2 + LAWN_OFFSET_Y,
        initHp
    ), gridRow(row), gridCol(col), cost(initCost), shootCD(initCD), shootTimer(0.0f){
}

void Plant::update(float deltaTime) {
    shootTimer += deltaTime;
}

void Plant::draw() {}

int Plant::getGridRow() const { return gridRow; }
int Plant::getGridCol() const { return gridCol; }

// --- Peashooter 豌豆射手实现 ---
Peashooter::Peashooter(int row, int col) : Plant(row, col, 100, PEA_SHOOTER_COST, PEA_SHOOTER_CD) {}

void Peashooter::update(float deltaTime) {
    Plant::update(deltaTime);
    if (shootTimer >= shootCD) {
        shootTimer = 0.0f;
        // 发射子弹
        Game::getInstance().addBullet(new Bullet(x + 20, y));
    }
}

void Peashooter::draw() {
    // 从资源管理器获取豌豆射手图片
    IMAGE* img = ResManager::getInstance().getImage(RES_PEASHOOTER);
    if (img) {
        ResManager::drawTransparentImage((int)x - CELL_SIZE / 2, (int)y - CELL_SIZE / 2, RES_PEASHOOTER);
    }
    else {
        // 图片加载失败，用原来的简单绘制作为后备方案
        setfillcolor(GREEN);
        solidcircle((int)x, (int)y, 30);
        setfillcolor(BLUE);
        solidcircle((int)x + 20, (int)y, 10);
    }
}

// ==============================================
// 【新增】Sunflower 向日葵实现
// ==============================================
Sunflower::Sunflower(int row, int col) : Plant(row, col, 80, 50, 1.0f) {
    // 向日葵：血量80，价格50阳光，生产间隔24秒（原版PVZ的数值）
}

void Sunflower::update(float deltaTime) {
    Plant::update(deltaTime);
    // 冷却到了，生产阳光
    if (shootTimer >= shootCD) {
        shootTimer = 0.0f;
        // 阳光的初始位置：向日葵中心，下落目标位置是向日葵下方一点
        float sunX = x;
        float sunInitY = y - 20;
        float sunTargetY = y + 10;
        // 往游戏里添加阳光
        Game::getInstance().addSun(new Sun(sunX, sunInitY, sunTargetY));
        cout << "向日葵生产了阳光！" << endl;
    }
}

void Sunflower::draw() {
    IMAGE* img = ResManager::getInstance().getImage(RES_SUNFLOWER);
    if (img) {
        ResManager::drawTransparentImage((int)x - CELL_SIZE / 2, (int)y - CELL_SIZE / 2, RES_SUNFLOWER);
    }
    else {
        // 后备绘制代码（原来的圆形绘制）
        setfillcolor(YELLOW);
        solidcircle((int)x, (int)y, 30);
        setfillcolor(BROWN);
        solidcircle((int)x, (int)y, 10);
    }
}

// ==============================================
// Wallnut 坚果墙实现
// ==============================================
Wallnut::Wallnut(int row, int col) : Plant(row, col, WALLNUT_HP, WALLNUT_COST, WALLNUT_CD) {
    // 坚果墙：高血量，不攻击，纯防御
}

void Wallnut::update(float deltaTime) {
    // 坚果墙不攻击，只需要调用基类的update（基类update是空的，这里保留框架一致性）
    Plant::update(deltaTime);
}

void Wallnut::draw() {
    IMAGE* img = ResManager::getInstance().getImage(RES_WALLNUT);
    if (img) {
        ResManager::drawTransparentImage((int)x - CELL_SIZE / 2, (int)y - CELL_SIZE / 2, RES_WALLNUT);
    }
    else {
        // 后备绘制代码
        setfillcolor(RGB(139, 69, 19));
        solidrectangle((int)x - 30, (int)y - 30, (int)x + 30, (int)y + 30);
    }
}

// ==============================================
// SnowPea 寒冰射手实现
// ==============================================
SnowPea::SnowPea(int row, int col) : Plant(row, col, 100, SNOWPEA_COST, SNOWPEA_CD) {
    // 寒冰射手：血量100，价格175阳光，发射间隔和豌豆射手一致
}

void SnowPea::update(float deltaTime) {
    Plant::update(deltaTime);
    if (shootTimer >= shootCD) {
        shootTimer = 0.0f;
        // 【核心】发射寒冰子弹，最后一个参数传true
        Game::getInstance().addBullet(new Bullet(x + 20, y, 20, true));
    }
}

void SnowPea::draw() {
    IMAGE* img = ResManager::getInstance().getImage(RES_SNOWPEA);
    if (img) {
        ResManager::drawTransparentImage((int)x - CELL_SIZE / 2, (int)y - CELL_SIZE / 2, RES_SNOWPEA);
    }
    else {
        // 后备绘制代码
        setfillcolor(RGB(135, 206, 250));
        solidcircle((int)x, (int)y, 30);
        setfillcolor(BLUE);
        solidcircle((int)x + 20, (int)y, 10);
    }
}

// ==============================================
// CherryBomb 樱桃炸弹完整实现
// ==============================================
CherryBomb::CherryBomb(int row, int col)
    : Plant(row, col, 1, CHERRYBOMB_COST, CHERRYBOMB_CD),
    explosionTimer(CHERRYBOMB_EXPLOSION_DELAY),
    isExploding(false),
    explosionRadius(0.0f) {
}

void CherryBomb::update(float deltaTime) {
    Plant::update(deltaTime);

    // 第一阶段：倒计时，还没爆炸
    if (!isExploding) {
        explosionTimer -= deltaTime;
        // 倒计时结束，触发爆炸
        if (explosionTimer <= 0) {
            isExploding = true;

            // ======================
            // 核心：范围爆炸伤害
            // ======================
            // 1. 获取全局游戏实例里的所有僵尸
            auto& allZombies = Game::getInstance().getZombies();

            // 2. 遍历所有僵尸，判断是否在爆炸范围内
            for (auto zombie : allZombies) {
                if (!zombie->isAlive()) continue; // 跳过已经死的僵尸

                // 计算僵尸到樱桃炸弹的距离
                float dx = zombie->getX() - x;
                float dy = zombie->getY() - y;
                float distance = sqrt(dx * dx + dy * dy);

                // 距离小于爆炸半径，就造成伤害
                if (distance <= CHERRYBOMB_EXPLOSION_RADIUS) {
                    zombie->takeDamage(CHERRYBOMB_DAMAGE);
                }
            }
        }
    }
    // 第二阶段：播放爆炸动画，动画结束后自己消失
    else {
        // 爆炸圆形扩散
        explosionRadius += 300.0f * deltaTime;
        // 扩散到最大范围后，自己消失
        if (explosionRadius >= CHERRYBOMB_EXPLOSION_RADIUS) {
            takeDamage(1);
        }
    }
}

void CherryBomb::draw() {
    // 还没爆炸：画两个红色樱桃
    if (!isExploding) {
        // 两个樱桃
        setfillcolor(RGB(220, 20, 60));
        solidcircle((int)x - 15, (int)y, 18);
        solidcircle((int)x + 15, (int)y, 18);
        // 樱桃梗
        setlinecolor(RGB(34, 139, 34));
        setlinestyle(PS_SOLID, 3);
        line((int)x - 15, (int)y - 18, (int)x, (int)y - 35);
        line((int)x + 15, (int)y - 18, (int)x, (int)y - 35);
        setlinestyle(PS_SOLID, 1); // 恢复默认线条
    }
    // 正在爆炸：画扩散的爆炸特效
    else {
        setfillcolor(RGB(255, 69, 0));
        solidcircle((int)x, (int)y, (int)explosionRadius);
        setfillcolor(YELLOW);
        solidcircle((int)x, (int)y, (int)(explosionRadius * 0.5f));
    }
}

// ==============================================
// PotatoMine 土豆地雷完整实现（修复版）
// ==============================================
PotatoMine::PotatoMine(int row, int col)
    : Plant(row, col, 1, POTATOMINE_COST, POTATOMINE_CD), // 血量直接设为1，爆炸后直接死亡
    activateTimer(POTATOMINE_ACTIVATE_DELAY),
    isActivated(false),
    isExploding(false),
    explosionRadius(0.0f) {
}

void PotatoMine::update(float deltaTime) {
    Plant::update(deltaTime);

    // 【第一优先级】如果正在爆炸，只处理动画，不处理其他逻辑
    if (isExploding) {
        // 动画增长：速度适中
        explosionRadius += 250.0f * deltaTime;

        // 【核心修复】动画达到最大半径后，直接让自己死亡，不依赖takeDamage
        if (explosionRadius >= 100.0f) {
            alive = false; // 直接设置为死亡，确保一定会消失
        }
        return;
    }

    // 第二阶段：未激活，倒计时埋土
    if (!isActivated) {
        activateTimer -= deltaTime;
        if (activateTimer <= 0) {
            isActivated = true;
            cout << "土豆地雷激活完成！" << endl;
        }
        return;
    }

    // 第三阶段：已激活，检测是否有僵尸踩到
    auto& allZombies = Game::getInstance().getZombies();
    for (auto zombie : allZombies) {
        if (!zombie->isAlive()) continue;

        // 计算僵尸到土豆地雷的距离
        float dx = zombie->getX() - x;
        float dy = zombie->getY() - y;
        float distanceSq = dx * dx + dy * dy; // 用距离平方，避免开根号，效率更高

        // 僵尸进入爆炸范围（60半径的平方是3600）
        if (distanceSq <= 3600.0f) {
            isExploding = true;
            explosionRadius = 0.0f; // 确保动画从0开始
            cout << "土豆地雷爆炸！" << endl;

            // 对范围内所有僵尸造成秒杀伤害
            for (auto targetZombie : allZombies) {
                if (!targetZombie->isAlive()) continue;
                float targetDx = targetZombie->getX() - x;
                float targetDy = targetZombie->getY() - y;
                float targetDistanceSq = targetDx * targetDx + targetDy * targetDy;

                if (targetDistanceSq <= 3600.0f) {
                    targetZombie->takeDamage(POTATOMINE_DAMAGE);
                }
            }
            break;
        }
    }
}

void PotatoMine::draw() {
    // 正在爆炸：画爆炸特效
    if (isExploding) {
        // 外层：半透明棕色
        setfillcolor(RGB(139, 69, 19));
        solidcircle((int)x, (int)y, (int)explosionRadius);
        // 内层：亮黄色
        setfillcolor(RGB(255, 215, 0));
        solidcircle((int)x, (int)y, (int)(explosionRadius * 0.6f));
        return;
    }

    // 未激活：画埋在地下的小土包
    if (!isActivated) {
        setfillcolor(RGB(139, 69, 19));
        solidellipse((int)x - 20, (int)y - 10, (int)x + 20, (int)y + 10);
        setfillcolor(RGB(101, 67, 33));
        solidellipse((int)x - 15, (int)y - 5, (int)x + 15, (int)y + 5);
    }
    // 已激活：画冒出来的土豆地雷
    else {
        setfillcolor(RGB(205, 133, 63));
        solidcircle((int)x, (int)y, 20);
        setfillcolor(BLACK);
        solidcircle((int)x - 12, (int)y - 12, 3);
        solidcircle((int)x + 12, (int)y - 12, 3);
        solidcircle((int)x - 12, (int)y + 12, 3);
        solidcircle((int)x + 12, (int)y + 12, 3);
        setfillcolor(WHITE);
        solidcircle((int)x - 6, (int)y, 4);
        solidcircle((int)x + 6, (int)y, 4);
        setfillcolor(BLACK);
        solidcircle((int)x - 6, (int)y, 2);
        solidcircle((int)x + 6, (int)y, 2);
    }
}