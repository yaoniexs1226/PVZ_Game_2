#pragma once
#include "GameObject.h"
#include "GlobalConst.h"

class Zombie : public GameObject {
protected:
    float walkSpeed;
    float normalSpeed;    // 保存正常速度，减速结束后恢复
    int attackDamage;
    float attackCD;
    float attackTimer;
    bool isEating;
    int gridRow;
    float slowTimer;      // 减速剩余时间
    bool isSlowed;        // 是否处于减速状态
public:
    Zombie(int row, float initX, int initHp, float initSpeed, int initDamage, float initAttackCD);
    virtual ~Zombie() = default;

    void update(float deltaTime) override;
    void draw() override;

    int getGridRow() const;
    void setEating(bool eating); // 只留声明，实现写在cpp里，避免重复定义
    bool getIsEating() const { return isEating; }
    int getAttackDamage() const { return attackDamage; }
    float getAttackCD() const { return attackCD; }
    float& getAttackTimer() { return attackTimer; }

    // 减速相关接口
    void applySlow(float duration, float factor);
    bool getIsSlowed() const { return isSlowed; }
};

// 普通僵尸
class NormalZombie : public Zombie {
public:
    NormalZombie(int row);
    void update(float deltaTime) override;
    void draw() override;
};

// 【新增】路障僵尸
class ConeZombie : public Zombie {
public:
    ConeZombie(int row);
    void update(float deltaTime) override;
    void draw() override;
};

// 【新增】铁桶僵尸
class BucketZombie : public Zombie {
public:
    BucketZombie(int row);
    void update(float deltaTime) override;
    void draw() override;
};