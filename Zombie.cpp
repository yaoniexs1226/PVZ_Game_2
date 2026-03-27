#include "Zombie.h"
#include "GlobalConst.h"
#include "ResManager.h"
#include <graphics.h>
#include <iostream>
using namespace std;

// ==============================================
// Zombie 基类完整实现
// ==============================================
Zombie::Zombie(int row, float initX, int initHp, float initSpeed, int initDamage, float initAttackCD)
    : GameObject(
        initX,
        (float)row* CELL_SIZE + CELL_SIZE / 2 + LAWN_OFFSET_Y,
        initHp
    ),
    gridRow(row),
    walkSpeed(initSpeed),
    normalSpeed(initSpeed),
    attackDamage(initDamage),
    attackCD(initAttackCD),
    attackTimer(0.0f),
    isEating(false),
    slowTimer(0.0f),
    isSlowed(false)
{
}

void Zombie::update(float deltaTime) {
    // 减速状态更新
    if (isSlowed) {
        slowTimer -= deltaTime;
        if (slowTimer <= 0) {
            isSlowed = false;
            walkSpeed = normalSpeed;
        }
    }
}

void Zombie::draw() {}

int Zombie::getGridRow() const { return gridRow; }

void Zombie::setEating(bool eating) {
    isEating = eating;
}

void Zombie::applySlow(float duration, float factor) {
    isSlowed = true;
    slowTimer = duration;
    walkSpeed = normalSpeed * factor;
}

// ==============================================
// NormalZombie 普通僵尸完整实现（无任何重复）
// ==============================================
NormalZombie::NormalZombie(int row)
    : Zombie(row, (float)WINDOW_WIDTH, NORMAL_ZOMBIE_HP, ZOMBIE_WALK_SPEED, 10, 1.0f) {
}

void NormalZombie::update(float deltaTime) {
    Zombie::update(deltaTime);
    if (isEating) return;
    x -= walkSpeed * deltaTime;
    if (x < 0) alive = false;
}

void NormalZombie::draw() {
    // 优先用图片绘制
    IMAGE* img = ResManager::getInstance().getImage(RES_ZOMBIE_NORMAL);
    if (img) {
        // 【一行代码，零花屏、完美去黑框】
        ResManager::drawTransparentImage((int)x - 30, (int)y - 40, RES_ZOMBIE_NORMAL);
    }
    // 图片加载失败，后备绘制
    else {
        if (isSlowed) {
            setfillcolor(RGB(100, 149, 237));
        }
        else {
            setfillcolor(RGB(100, 100, 100));
        }
        solidrectangle((int)x - 20, (int)y - 30, (int)x + 20, (int)y + 30);

        setfillcolor(RGB(150, 150, 150));
        solidcircle((int)x, (int)y - 40, 20);
        setfillcolor(RED);
        solidcircle((int)x - 8, (int)y - 40, 4);
        solidcircle((int)x + 8, (int)y - 40, 4);

        if (isEating) {
            settextcolor(RED);
            setbkmode(TRANSPARENT);
            outtextxy((int)x - 15, (int)y + 40, _T("啃食中"));
        }
    }
}

// ==============================================
// ConeZombie 路障僵尸完整实现
// ==============================================
ConeZombie::ConeZombie(int row)
    : Zombie(row, (float)WINDOW_WIDTH, CONE_ZOMBIE_HP, CONE_ZOMBIE_SPEED, CONE_ZOMBIE_DAMAGE, 1.0f) {
}

void ConeZombie::update(float deltaTime) {
    Zombie::update(deltaTime);
    if (isEating) return;
    x -= walkSpeed * deltaTime;
    if (x < 0) alive = false;
}

void ConeZombie::draw() {
    IMAGE* img = ResManager::getInstance().getImage(RES_ZOMBIE_CONE);
    if (img) {
        // 【一行代码，零花屏、完美去黑框】
        ResManager::drawTransparentImage((int)x - 30, (int)y - 40, RES_ZOMBIE_CONE);
    }
    // 后备绘制
    if (isSlowed) {
        setfillcolor(RGB(100, 149, 237));
    }
    else {
        setfillcolor(RGB(100, 100, 100));
    }
    solidrectangle((int)x - 20, (int)y - 30, (int)x + 20, (int)y + 30);

    setfillcolor(RGB(150, 150, 150));
    solidcircle((int)x, (int)y - 40, 20);
    setfillcolor(RED);
    solidcircle((int)x - 8, (int)y - 40, 4);
    solidcircle((int)x + 8, (int)y - 40, 4);

    // 路障
    setfillcolor(RGB(255, 140, 0));
    solidrectangle((int)x - 15, (int)y - 65, (int)x + 15, (int)y - 40);
    solidrectangle((int)x - 10, (int)y - 75, (int)x + 10, (int)y - 65);

    if (isEating) {
        settextcolor(RED);
        setbkmode(TRANSPARENT);
        outtextxy((int)x - 15, (int)y + 40, _T("啃食中"));
    }
}

// ==============================================
// BucketZombie 铁桶僵尸完整实现
// ==============================================
BucketZombie::BucketZombie(int row)
    : Zombie(row, (float)WINDOW_WIDTH, BUCKET_ZOMBIE_HP, BUCKET_ZOMBIE_SPEED, BUCKET_ZOMBIE_DAMAGE, 1.0f) {
}

void BucketZombie::update(float deltaTime) {
    Zombie::update(deltaTime);
    if (isEating) return;
    x -= walkSpeed * deltaTime;
    if (x < 0) alive = false;
}

void BucketZombie::draw() {
    IMAGE* img = ResManager::getInstance().getImage(RES_ZOMBIE_BUCKET);
    if (img) {
        // 【一行代码，零花屏、完美去黑框】
        ResManager::drawTransparentImage((int)x - 30, (int)y - 40, RES_ZOMBIE_BUCKET);
    }
    // 后备绘制
    if (isSlowed) {
        setfillcolor(RGB(100, 149, 237));
    }
    else {
        setfillcolor(RGB(100, 100, 100));
    }
    solidrectangle((int)x - 20, (int)y - 30, (int)x + 20, (int)y + 30);

    setfillcolor(RGB(150, 150, 150));
    solidcircle((int)x, (int)y - 40, 20);
    setfillcolor(RED);
    solidcircle((int)x - 8, (int)y - 40, 4);
    solidcircle((int)x + 8, (int)y - 40, 4);

    // 铁桶
    setfillcolor(RGB(105, 105, 105));
    solidcircle((int)x, (int)y - 55, 18);
    solidrectangle((int)x - 18, (int)y - 40, (int)x + 18, (int)y - 55);

    if (isEating) {
        settextcolor(RED);
        setbkmode(TRANSPARENT);
        outtextxy((int)x - 15, (int)y + 40, _T("啃食中"));
    }
}