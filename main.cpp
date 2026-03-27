#include "Game.h"

int main() {
    // 获取全局游戏实例，启动游戏
    Game::getInstance().run();
    return 0;
}