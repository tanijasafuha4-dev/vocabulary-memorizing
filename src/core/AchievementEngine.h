#ifndef SHANXUEXI_ACHIEVEMENTENGINE_H
#define SHANXUEXI_ACHIEVEMENTENGINE_H

#include "Models.h"
#include <string>
#include <vector>
#include <functional>

namespace shanxuexi {

/**
 * @brief 成就进度结构
 */
struct AchievementProgress {
    int cur = 0;
    int total = 1;
    std::string unit;
};

/**
 * @brief 单个成就定义结构
 */
struct AchievementDef {
    std::string id;
    std::string group;
    int tier = 1;
    std::string defaultName;
    std::function<bool(const ComputedStats&)> checkFn;
    std::function<AchievementProgress(const ComputedStats&)> progressFn;
};

/**
 * @brief 单个成就徽章视图项（供 QML UI 展示）
 */
struct BadgeViewItem {
    std::string id;
    std::string group;
    int tier = 1;
    std::string name;            // 根据当前主题动态计算的称号
    bool unlocked = false;
    std::string dateText;        // "2026.10.20 解锁" 或 "待解锁"
    std::string progressText;    // 例如 "1/3 天"
    double progressRatio = 0.0;  // 0.0 ~ 1.0
    int hue = 215;
    int sat = 60;
    int light = 60;
};

/**
 * @brief 54 项多主题世界观成就系统引擎
 */
class AchievementEngine {
public:
    /**
     * @brief 获取全部 54 个成就定义
     */
    static const std::vector<AchievementDef>& allDefinitions();

    /**
     * @brief 根据当前主题 ID 获取成就的世界观名称
     */
    static std::string getAchievementName(const AchievementDef& def, const std::string& themeId);

    /**
     * @brief 检测并解锁新达成的成就
     * @return 本次新解锁的成就名称列表（用于弹出 Toast 提示）
     */
    static std::vector<std::string> checkAndUnlock(AppState& state, const ComputedStats& stats);

    /**
     * @brief 一键切换全部徽章解锁/锁定（对应原版 en="wsy", cn="bb" 彩蛋）
     * @return 解锁后是否处于全解锁状态
     */
    static bool toggleAllBadgesEasterEgg(AppState& state);

    /**
     * @brief 构建全部 54 个徽章的 UI 视图列表
     */
    static std::vector<BadgeViewItem> buildBadgeViews(const AppState& state, const ComputedStats& stats);
};

} // namespace shanxuexi

#endif // SHANXUEXI_ACHIEVEMENTENGINE_H
