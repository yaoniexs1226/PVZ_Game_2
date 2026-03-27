#pragma once
#include "GameObject.h"

class Bullet : public GameObject {
private:
    float speed;
    int damage;
    bool isIceBullet; // 【新增】是否是寒冰子弹

public:
    // 【修改】构造函数新增参数，默认是普通子弹
    Bullet(float initX, float initY, int initDamage = 20, bool isIce = false);

    void update(float deltaTime) override;
    void draw() override;

    int getDamage() const;
    bool isIce() const { return isIceBullet; } // 【新增】获取是否是寒冰子弹
};
