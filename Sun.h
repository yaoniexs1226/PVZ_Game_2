#pragma once
#include "GameObject.h"

class Sun : public GameObject {
private:
    int sunValue;
    float lifeTimer; // 自动消失计时器
    float fallSpeed; // 下落速度
    float targetY;   // 下落的目标Y坐标
public:
    Sun(float initX, float initY, float targetY, int value = 50);

    void update(float deltaTime) override;
    void draw() override;

    // 检测鼠标是否点击到了阳光
    bool isClickOnSun(int mouseX, int mouseY) const;
    int getSunValue() const;
};