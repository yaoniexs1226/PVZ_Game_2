#include "ResManager.h"
#include "GlobalConst.h"
#include <iostream>
using namespace std;

// 单例构造
ResManager::ResManager() : isLoaded(false) {}

ResManager& ResManager::getInstance() {
    static ResManager instance;
    return instance;
}

// ==============================================
// 【核心】生成透明掩码图，完美处理黑边
// ==============================================
void ResManager::generateMask(ResID id) {
    IMAGE* pImg = &resMap[id];
    // 老版EasyX标准用法：先切换工作区，再获取宽高
    SetWorkingImage(pImg);
    int imgWidth = getwidth();
    int imgHeight = getheight();
    SetWorkingImage(NULL);

    // 创建和原图等大的掩码图
    IMAGE mask(imgWidth, imgHeight);
    SetWorkingImage(&mask);
    setbkcolor(WHITE); // 透明区域掩码为白色
    cleardevice();
    SetWorkingImage(NULL);

    // 逐像素生成掩码：非透明区域掩码为黑色
    SetWorkingImage(pImg);
    for (int y = 0; y < imgHeight; y++) {
        for (int x = 0; x < imgWidth; x++) {
            COLORREF pixelColor = getpixel(x, y);
            // 【优化】接近黑色的区域都算透明，彻底消除边缘黑边
            BYTE r = GetRValue(pixelColor);
            BYTE g = GetGValue(pixelColor);
            BYTE b = GetBValue(pixelColor);

            // 非黑色区域，掩码设为黑色
            if (r > 10 || g > 10 || b > 10) {
                SetWorkingImage(&mask);
                putpixel(x, y, BLACK);
                SetWorkingImage(pImg);
            }
        }
    }
    SetWorkingImage(NULL);

    // 保存生成好的掩码图
    maskMap[id] = mask;
}

// ==============================================
// 加载所有资源，自动生成掩码
// ==============================================
void ResManager::loadAllResources() {
    if (isLoaded) return;
    cout << "正在加载游戏资源..." << endl;

    // 加载植物图片（和你的文件名完全匹配）
    loadimage(&resMap[RES_PEASHOOTER], _T("res/peashooter.png"), CELL_SIZE, CELL_SIZE);
    loadimage(&resMap[RES_SUNFLOWER], _T("res/SunFlower.png"), CELL_SIZE, CELL_SIZE);
    loadimage(&resMap[RES_WALLNUT], _T("res/WallNut.png"), CELL_SIZE, CELL_SIZE);
    loadimage(&resMap[RES_SNOWPEA], _T("res/SnowPea.png"), CELL_SIZE, CELL_SIZE);
    loadimage(&resMap[RES_CHERRYBOMB], _T("res/CherryBomb.png"), CELL_SIZE, CELL_SIZE);
    loadimage(&resMap[RES_POTATOMINE], _T("res/PotatoMine.png"), CELL_SIZE, CELL_SIZE);

    // 加载僵尸图片（和你的文件名完全匹配）
    loadimage(&resMap[RES_ZOMBIE_NORMAL], _T("res/ZombieWalk1.gif"), 60, 80);
    loadimage(&resMap[RES_ZOMBIE_CONE], _T("res/ConeZombieWalk.gif"), 60, 80);
    loadimage(&resMap[RES_ZOMBIE_BUCKET], _T("res/BucketZombieWalk.gif"), 60, 80);

    // 加载子弹、阳光、背景、铲子
    loadimage(&resMap[RES_PEA], _T("res/Pea.png"), 20, 20);
    loadimage(&resMap[RES_PEA_ICE], _T("res/PeaSnow.png"), 20, 20);
    loadimage(&resMap[RES_SUN], _T("res/Sun.gif"), 40, 40);
    loadimage(&resMap[RES_BACKGROUND], _T("res/Background.jpg"), WINDOW_WIDTH, WINDOW_HEIGHT);
    loadimage(&resMap[RES_SHOVEL], _T("res/Shovel.png"), CARD_WIDTH - 10, CARD_HEIGHT - 10);

    // 【关键】给所有图片自动生成透明掩码
    for (int i = 0; i < RES_COUNT; i++) {
        generateMask((ResID)i);
    }

    isLoaded = true;
    cout << "游戏资源加载完成！" << endl;
}

// 获取原图
IMAGE* ResManager::getImage(ResID id) {
    if (resMap.find(id) != resMap.end()) {
        return &resMap[id];
    }
    return nullptr;
}

// 获取掩码图
IMAGE* ResManager::getMask(ResID id) {
    if (maskMap.find(id) != maskMap.end()) {
        return &maskMap[id];
    }
    return nullptr;
}

// ==============================================
// 【核心绘制函数】零花屏、完美去黑框
// ==============================================
void ResManager::drawTransparentImage(int x, int y, ResID id) {
    IMAGE* pImg = getInstance().getImage(id);
    IMAGE* pMask = getInstance().getMask(id);
    if (!pImg || !pMask) return;

    // 老版EasyX标准透明绘制：先画掩码，再画原图
    putimage(x, y, pMask, SRCAND);  // 掩码抠出透明区域
    putimage(x, y, pImg, SRCPAINT); // 绘制原图内容
}