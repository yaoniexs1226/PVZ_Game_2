#pragma once
#include "GameObject.h"
#include "GlobalConst.h"
#include <vector>

// 前置声明
class Bullet;
class Game;
class Sun;

class Plant : public GameObject {
protected:
    int cost;
    float shootCD;
    float shootTimer;
    int gridRow;
    int gridCol;
public:
    Plant(int row, int col, int initHp, int initCost, float initCD);
    virtual ~Plant() = default;

    void update(float deltaTime) override;
    void draw() override;

    int getCost() const { return cost; } // 【新增】获取植物价格
    int getGridRow() const;
    int getGridCol() const;
};

// 豌豆射手
class Peashooter : public Plant {
public:
    Peashooter(int row, int col);
    void update(float deltaTime) override;
    void draw() override;
};

// 【新增】向日葵
class Sunflower : public Plant {
public:
    Sunflower(int row, int col);
    void update(float deltaTime) override;
    void draw() override;
};

// 【新增】坚果墙
class Wallnut : public Plant {
public:
    Wallnut(int row, int col);
    void update(float deltaTime) override;
    void draw() override;
};

// 【新增】寒冰射手
class SnowPea : public Plant {
public:
    SnowPea(int row, int col);
    void update(float deltaTime) override;
    void draw() override;
};

// 【新增】樱桃炸弹
class CherryBomb : public Plant {
private:
    float explosionTimer; // 爆炸倒计时
    bool isExploding;     // 是否正在爆炸
    float explosionRadius; // 当前爆炸动画半径
public:
    CherryBomb(int row, int col);
    void update(float deltaTime) override;
    void draw() override;
};

// 土豆地雷
class PotatoMine : public Plant {
private:
    float activateTimer;   // 激活倒计时
    bool isActivated;      // 是否已经激活
    bool isExploding;      // 是否正在爆炸
    float explosionRadius; // 爆炸动画半径
public:
    PotatoMine(int row, int col);
    void update(float deltaTime) override;
    void draw() override;
};