#pragma once
#include <graphics.h>
#include <map>
#include <tchar.h>

// 资源ID枚举（和你现有的完全匹配）
enum ResID {
    RES_PEASHOOTER,
    RES_SUNFLOWER,
    RES_WALLNUT,
    RES_SNOWPEA,
    RES_CHERRYBOMB,
    RES_POTATOMINE,
    RES_ZOMBIE_NORMAL,
    RES_ZOMBIE_CONE,
    RES_ZOMBIE_BUCKET,
    RES_PEA,
    RES_PEA_ICE,
    RES_SUN,
    RES_BACKGROUND,
    RES_SHOVEL,
    RES_COUNT
};

class ResManager {
private:
    ResManager();
    std::map<ResID, IMAGE> resMap;
    std::map<ResID, IMAGE> maskMap; // 【核心新增】每个图片对应的透明掩码图
    bool isLoaded;

    // 生成透明掩码图（老版EasyX透明绘制的核心）
    void generateMask(ResID id);

public:
    // 单例模式
    static ResManager& getInstance();

    // 禁止拷贝
    ResManager(const ResManager&) = delete;
    ResManager& operator=(const ResManager&) = delete;

    // 加载所有资源
    void loadAllResources();
    // 获取原图/掩码图
    IMAGE* getImage(ResID id);
    IMAGE* getMask(ResID id);
    bool isAllLoaded() const { return isLoaded; }

    // 【核心绘制函数】零花屏、完美去黑框，一行代码搞定
    static void drawTransparentImage(int x, int y, ResID id);
};