#pragma once
#include <Windows.h>

class GameObject {
protected:
    float x;
    float y;
    int hp;
    bool alive;
public:
    GameObject(float initX, float initY, int initHp);
    virtual ~GameObject();

    // 纯虚函数：子类必须实现
    virtual void update(float deltaTime) = 0;
    virtual void draw() = 0;

    // 虚函数：获取碰撞盒
    virtual RECT getCollisionRect() const;

    // 通用工具
    bool isAlive() const;
    void takeDamage(int damage);
    float getX() const { return x; }
    float getY() const { return y; }
    int getHp() const { return hp; } // 新增：获取当前血量，解决报错
};