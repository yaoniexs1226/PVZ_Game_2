#include "Bullet.h"
#include "ResManager.h"
#include "GlobalConst.h"
#include <graphics.h>

// 【修改】构造函数，新增寒冰状态初始化
Bullet::Bullet(float initX, float initY, int initDamage, bool isIce)
    : GameObject(initX, initY, 1), damage(initDamage), isIceBullet(isIce) {
    speed = BULLET_SPEED;
}

void Bullet::update(float deltaTime) {
    x += speed * deltaTime;
    if (x > WINDOW_WIDTH) alive = false;
}

void Bullet::draw() {
    if (isIceBullet) {
        IMAGE* img = ResManager::getInstance().getImage(RES_PEA_ICE);
        if (img) {
            ResManager::drawTransparentImage((int)x - CELL_SIZE / 2, (int)y - CELL_SIZE / 2, RES_PEA_ICE);
        }
        else {
            setfillcolor(RGB(135, 206, 250));
            solidcircle((int)x, (int)y, 8);
        }
    }
    else {
        IMAGE* img = ResManager::getInstance().getImage(RES_PEA);
        if (img) {
            ResManager::drawTransparentImage((int)x - CELL_SIZE / 2, (int)y - CELL_SIZE / 2, RES_PEA);
        }
        else {
            setfillcolor(YELLOW);
            solidcircle((int)x, (int)y, 8);
        }
    }
}

int Bullet::getDamage() const {
    return damage;
}