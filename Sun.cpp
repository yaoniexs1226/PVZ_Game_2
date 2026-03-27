#include "Sun.h"
#include "GlobalConst.h"
#include <graphics.h>

Sun::Sun(float initX, float initY, float targetY, int value)
    : GameObject(initX, initY, 1), sunValue(value), lifeTimer(10.0f), fallSpeed(60.0f), targetY(targetY) {
}

void Sun::update(float deltaTime) {
    // 阳光下落逻辑：还没落到目标位置就继续下落
    if (y < targetY) {
        y += fallSpeed * deltaTime;
    }

    // 自动消失计时
    lifeTimer -= deltaTime;
    if (lifeTimer <= 0) {
        alive = false;
    }
}

void Sun::draw() {
    // 画阳光主体（黄色圆形）
    setfillcolor(YELLOW);
    solidcircle((int)x, (int)y, 18);
    // 画阳光高光
    setfillcolor(WHITE);
    solidcircle((int)x - 6, (int)y - 6, 6);

    // 显示阳光数值
    settextcolor(BLACK);
    setbkmode(TRANSPARENT);
    settextstyle(14, 0, _T("宋体"));
    outtextxy((int)x - 10, (int)y - 7, _T("50"));
}

// 检测鼠标是否点击到阳光
bool Sun::isClickOnSun(int mouseX, int mouseY) const {
    // 计算鼠标到阳光中心的距离，小于半径就算点击到
    float dx = mouseX - x;
    float dy = mouseY - y;
    return dx * dx + dy * dy <= 18 * 18;
}

int Sun::getSunValue() const {
    return sunValue;
}