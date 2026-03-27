#pragma once

// 全局游戏常量，统一管理
// inline const 保证所有cpp文件都能正常引用，彻底解决链接错误
inline const int WINDOW_WIDTH = 800;
inline const int WINDOW_HEIGHT = 600;
inline const int GRID_COL_COUNT = 9;
inline const int GRID_ROW_COUNT = 5;
inline const int CELL_SIZE = 80;
inline const int LAWN_OFFSET_X = 0;
inline const int LAWN_OFFSET_Y = 0;

// 游戏平衡常量
inline const int PEA_SHOOTER_COST = 100;
inline const float PEA_SHOOTER_CD = 2.0f;
inline const float BULLET_SPEED = 300.0f;
inline const float ZOMBIE_WALK_SPEED = 40.0f;
inline const float ZOMBIE_SPAWN_CD = 5.0f;
inline const int NORMAL_ZOMBIE_HP = 200;

// ==============================================
// 植物选择栏UI常量
// ==============================================
inline const int CARD_WIDTH = 60;       // 卡片宽度
inline const int CARD_HEIGHT = 80;      // 卡片高度
inline const int CARD_MARGIN = 10;      // 卡片间距
inline const int CARD_START_X = 150;    // 卡片栏起始X坐标
inline const int CARD_START_Y = 5;      // 卡片栏起始Y坐标
inline const float PLANT_CD_PEASHOOTER = 7.5f;  // 豌豆射手种植冷却（秒，原版数值）
inline const float PLANT_CD_SUNFLOWER = 7.5f;   // 向日葵种植冷却（秒）
// ==============================================
// 坚果墙常量
// ==============================================
inline const int WALLNUT_COST = 125;       // 坚果墙价格
inline const int WALLNUT_HP = 400;          // 坚果墙血量（简化版，原版4000）
inline const float WALLNUT_CD = 7.5f;       // 坚果墙种植冷却

// ==============================================
// 寒冰射手常量
// ==============================================
inline const int SNOWPEA_COST = 175;       // 寒冰射手价格
inline const float SNOWPEA_CD = 7.5f;       // 寒冰射手种植冷却
inline const float ZOMBIE_SLOW_DURATION = 3.0f; // 僵尸减速持续时间（秒）
inline const float ZOMBIE_SLOW_FACTOR = 0.5f;   // 僵尸减速因子（0.5 = 速度减半）

// ==============================================
// 樱桃炸弹常量
// ==============================================
inline const int CHERRYBOMB_COST = 150;       // 樱桃炸弹价格
inline const float CHERRYBOMB_CD = 30.0f;      // 樱桃炸弹种植冷却（原版30秒）
inline const float CHERRYBOMB_EXPLOSION_DELAY = 1.5f; // 爆炸延迟（秒）
inline const float CHERRYBOMB_EXPLOSION_RADIUS = 120.0f; // 爆炸范围半径
inline const int CHERRYBOMB_DAMAGE = 1000;     // 爆炸伤害（秒杀普通僵尸）

// ==============================================
// 波次系统常量
// ==============================================
inline const int TOTAL_WAVES = 5;              // 总波数
inline const float WAVE_INTERVAL = 5.0f;       // 波次间隔（秒）
inline const float GAME_START_DELAY = 5.0f;    // 【新增】游戏开局等待时间（5秒）
inline const int ZOMBIES_PER_WAVE_BASE = 3;    // 每波基础僵尸数量
inline const int ZOMBIES_PER_WAVE_INCREMENT = 2; // 每波增加的僵尸数量

// ==============================================
// 僵尸类型常量
// ==============================================
// 路障僵尸
inline const int CONE_ZOMBIE_HP = 200;       // 总血量（路障110 + 本体90，简化为200）
inline const int CONE_ZOMBIE_DAMAGE = 10;     // 攻击力和普通僵尸一致
inline const float CONE_ZOMBIE_SPEED = ZOMBIE_WALK_SPEED; // 移动速度一致

// 铁桶僵尸
inline const int BUCKET_ZOMBIE_HP = 450;      // 总血量（铁桶350 + 本体100，简化为450）
inline const int BUCKET_ZOMBIE_DAMAGE = 10;   // 攻击力一致
inline const float BUCKET_ZOMBIE_SPEED = ZOMBIE_WALK_SPEED; // 移动速度一致

// ==============================================
// 土豆地雷常量
// ==============================================
inline const int POTATOMINE_COST = 25;            // 土豆地雷价格（原版25阳光）
inline const float POTATOMINE_CD = 30.0f;         // 种植冷却（原版30秒）
inline const float POTATOMINE_ACTIVATE_DELAY = 3.0f; // 激活延迟（秒）
inline const float POTATOMINE_EXPLOSION_RADIUS = 60.0f; // 爆炸范围（仅覆盖自身格子）
inline const float POTATOMINE_ANIMATION_RADIUS = 100.0f; // 【新增】爆炸动画终止半径
inline const int POTATOMINE_DAMAGE = 1000;        // 爆炸伤害（秒杀所有僵尸）

// ==============================================
// 铲子常量
// ==============================================
inline const float SHOVEL_SUN_REFUND_RATIO = 0.5f; // 阳光回收比例（50%）
inline const int SHOVEL_BUTTON_X = CARD_START_X + 6 * (CARD_WIDTH + CARD_MARGIN); // 铲子按钮X坐标
inline const int SHOVEL_BUTTON_Y = CARD_START_Y; // 铲子按钮Y坐标