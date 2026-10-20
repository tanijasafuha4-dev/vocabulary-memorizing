#ifndef SHANXUEXI_CHECKINMANAGER_H
#define SHANXUEXI_CHECKINMANAGER_H

#include "Models.h"
#include <string>
#include <vector>
#include <map>

namespace shanxuexi {

/**
 * @brief 日历单元格数据结构（用于渲染打卡日历面板）
 */
struct CalendarDayCell {
    int day = 1;
    std::string dateStr;     // "YYYY-MM-DD"
    bool isOtherMonth = false;
    bool isChecked = false;
    bool isToday = false;
};

/**
 * @brief 月度日历视图聚合数据
 */
struct CalendarMonthView {
    int year = 2026;
    int month = 10;          // 1 ~ 12
    std::string title;       // 例如 "2026年10月"
    std::vector<CalendarDayCell> cells;
    int streak = 0;
    int monthCount = 0;
    int totalCount = 0;
};

/**
 * @brief 打卡日历与学习统计管理器
 */
class CheckinManager {
public:
    /**
     * @brief 获取当前日期的 "YYYY-MM-DD" 格式字符串
     */
    static std::string todayDateString();

    /**
     * @brief 格式化毫秒时间戳为 "YYYY.M.D" (用于成就解锁日期显示)
     */
    static std::string formatUnlockDate(int64_t tsMs);

    /**
     * @brief 执行今日打卡
     * @return 若今日是首次打卡返回 true，若已打过卡返回 false
     */
    static bool checkinToday(std::map<std::string, bool>& checkins);

    /**
     * @brief 判断今日是否已打卡
     */
    static bool isCheckedInToday(const std::map<std::string, bool>& checkins);

    /**
     * @brief 计算当前连续打卡天数（若今天尚未打卡，则从昨天开始向前回溯）
     */
    static int calcStreak(const std::map<std::string, bool>& checkins);

    /**
     * @brief 计算历史最长连续打卡天数
     */
    static int calcMaxStreak(const std::map<std::string, bool>& checkins);

    /**
     * @brief 统计累计打卡总天数
     */
    static int countTotalCheckins(const std::map<std::string, bool>& checkins);

    /**
     * @brief 生成指定年月的打卡日历网格视图数据（周一开始，周日结束）
     * @param year  年份，如 2026
     * @param month 月份 1 ~ 12
     */
    static CalendarMonthView buildCalendarMonth(int year, int month,
                                                const std::map<std::string, bool>& checkins);

    /**
     * @brief 聚合计算全局统计数据（供成就检查和备份面板展示）
     */
    static ComputedStats computeStats(const AppState& state);
};

} // namespace shanxuexi

#endif // SHANXUEXI_CHECKINMANAGER_H
