#include "CheckinManager.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace shanxuexi {

static std::string formatYmd(int year, int month, int day) {
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << year << "-"
        << std::setw(2) << std::setfill('0') << month << "-"
        << std::setw(2) << std::setfill('0') << day;
    return oss.str();
}

static bool parseYmd(const std::string& s, int& year, int& month, int& day) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    try {
        year = std::stoi(s.substr(0, 4));
        month = std::stoi(s.substr(5, 2));
        day = std::stoi(s.substr(8, 2));
        return true;
    } catch (...) {
        return false;
    }
}

// Howard Hinnant 公历日期转天数算法（用于精确计算日期差）
static int64_t daysFromCivil(int y, int m, int d) {
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097LL + static_cast<int64_t>(doe) - 719468LL;
}

static void civilFromDays(int64_t z, int& y, int& m, int& d) {
    z += 719468LL;
    const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    m = static_cast<int>(mp + (mp < 10 ? 3 : -9));
    y += (m <= 2);
}

static int getDaysInMonth(int year, int month) {
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) return 30;
    if (month == 2) {
        bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return isLeap ? 29 : 28;
    }
    return kDays[month - 1];
}

std::string CheckinManager::todayDateString() {
    std::time_t now = std::time(nullptr);
    std::tm tmBuf{};
    getLocalTm(now, tmBuf);
    return formatYmd(tmBuf.tm_year + 1900, tmBuf.tm_mon + 1, tmBuf.tm_mday);
}

std::string CheckinManager::formatUnlockDate(int64_t tsMs) {
    if (tsMs <= 0) return "";
    std::time_t sec = static_cast<std::time_t>(tsMs / 1000);
    std::tm tmBuf{};
    getLocalTm(sec, tmBuf);
    std::ostringstream oss;
    oss << (tmBuf.tm_year + 1900) << "." << (tmBuf.tm_mon + 1) << "." << tmBuf.tm_mday;
    return oss.str();
}

bool CheckinManager::checkinToday(std::map<std::string, bool>& checkins) {
    std::string today = todayDateString();
    auto it = checkins.find(today);
    if (it != checkins.end() && it->second) {
        return false;
    }
    checkins[today] = true;
    return true;
}

bool CheckinManager::isCheckedInToday(const std::map<std::string, bool>& checkins) {
    std::string today = todayDateString();
    auto it = checkins.find(today);
    return it != checkins.end() && it->second;
}

int CheckinManager::calcStreak(const std::map<std::string, bool>& checkins) {
    std::string today = todayDateString();
    int y = 2026, m = 1, d = 1;
    if (!parseYmd(today, y, m, d)) return 0;

    int64_t curDays = daysFromCivil(y, m, d);
    auto it = checkins.find(today);
    if (it == checkins.end() || !it->second) {
        // 如果今天还没打卡，从昨天开始往前统计连续打卡
        curDays -= 1;
    }

    int streak = 0;
    while (true) {
        int cy, cm, cd;
        civilFromDays(curDays, cy, cm, cd);
        std::string key = formatYmd(cy, cm, cd);
        auto found = checkins.find(key);
        if (found != checkins.end() && found->second) {
            streak++;
            curDays -= 1;
        } else {
            break;
        }
    }
    return streak;
}

int CheckinManager::calcMaxStreak(const std::map<std::string, bool>& checkins) {
    std::vector<int64_t> dayNums;
    dayNums.reserve(checkins.size());
    for (const auto& kv : checkins) {
        if (!kv.second) continue;
        int y, m, d;
        if (parseYmd(kv.first, y, m, d)) {
            dayNums.push_back(daysFromCivil(y, m, d));
        }
    }
    if (dayNums.empty()) return 0;

    std::sort(dayNums.begin(), dayNums.end());
    dayNums.erase(std::unique(dayNums.begin(), dayNums.end()), dayNums.end());

    int maxS = 1;
    int curS = 1;
    for (size_t i = 1; i < dayNums.size(); ++i) {
        if (dayNums[i] - dayNums[i - 1] == 1) {
            curS++;
            if (curS > maxS) maxS = curS;
        } else {
            curS = 1;
        }
    }
    return maxS;
}

int CheckinManager::countTotalCheckins(const std::map<std::string, bool>& checkins) {
    int count = 0;
    for (const auto& kv : checkins) {
        if (kv.second) count++;
    }
    return count;
}

CalendarMonthView CheckinManager::buildCalendarMonth(int year, int month,
                                                     const std::map<std::string, bool>& checkins) {
    CalendarMonthView view;
    view.year = year;
    view.month = month;
    view.title = std::to_string(year) + "年" + std::to_string(month) + "月";

    // 计算当月 1 号是星期几 (Monday = 0 ... Sunday = 6)
    int64_t firstDayNum = daysFromCivil(year, month, 1);
    // 1970-01-01 是星期四 (Monday=0 体系下是 3)
    int leadingOtherDays = static_cast<int>(((firstDayNum + 3) % 7 + 7) % 7);

    int daysInCurMonth = getDaysInMonth(year, month);
    int prevYear = (month == 1) ? (year - 1) : year;
    int prevMonth = (month == 1) ? 12 : (month - 1);
    int daysInPrevMonth = getDaysInMonth(prevYear, prevMonth);

    std::string todayStr = todayDateString();

    for (int i = 0; i < leadingOtherDays; ++i) {
        CalendarDayCell cell;
        cell.day = daysInPrevMonth - leadingOtherDays + i + 1;
        cell.dateStr = formatYmd(prevYear, prevMonth, cell.day);
        cell.isOtherMonth = true;
        view.cells.push_back(cell);
    }

    int monthCount = 0;
    for (int d = 1; d <= daysInCurMonth; ++d) {
        CalendarDayCell cell;
        cell.day = d;
        cell.dateStr = formatYmd(year, month, d);
        cell.isOtherMonth = false;
        auto it = checkins.find(cell.dateStr);
        cell.isChecked = (it != checkins.end() && it->second);
        if (cell.isChecked) monthCount++;
        cell.isToday = (cell.dateStr == todayStr);
        view.cells.push_back(cell);
    }

    int totalCells = static_cast<int>(view.cells.size());
    int trailingDays = (7 - (totalCells % 7)) % 7;
    int nextYear = (month == 12) ? (year + 1) : year;
    int nextMonth = (month == 12) ? 1 : (month + 1);
    for (int j = 1; j <= trailingDays; ++j) {
        CalendarDayCell cell;
        cell.day = j;
        cell.dateStr = formatYmd(nextYear, nextMonth, j);
        cell.isOtherMonth = true;
        view.cells.push_back(cell);
    }

    view.streak = calcStreak(checkins);
    view.monthCount = monthCount;
    view.totalCount = countTotalCheckins(checkins);
    return view;
}

ComputedStats CheckinManager::computeStats(const AppState& state) {
    ComputedStats cs;
    cs.quizCount = state.stats.quizCount;
    cs.perfectCount = state.stats.perfectCount;
    cs.consecutivePerfect = state.stats.consecutivePerfect;
    cs.morningQuiz = state.stats.morningQuiz;
    cs.nightQuiz = state.stats.nightQuiz;
    cs.noonQuiz = state.stats.noonQuiz;
    cs.lastAllKnow = state.stats.lastAllKnow;

    int totalWords = 0;
    int totalFuzzy = 0;
    for (const auto& d : state.decks) {
        totalWords += static_cast<int>(d.words.size());
        for (const auto& w : d.words) {
            totalFuzzy += w.stats.fuzzy;
        }
    }
    cs.wordsAdded = totalWords;
    cs.fuzzyCount = totalFuzzy;
    cs.decksCreated = static_cast<int>(state.decks.size());

    cs.streak = calcStreak(state.checkins);
    cs.maxStreak = std::max(calcMaxStreak(state.checkins), cs.streak);
    cs.totalCheckins = countTotalCheckins(state.checkins);

    int64_t nowMs = currentTimestampMs();
    int64_t firstMs = state.stats.firstUseDate > 0 ? state.stats.firstUseDate : nowMs;
    int days = static_cast<int>((nowMs - firstMs) / 86400000LL) + 1;
    cs.daysUsed = std::max(1, days);

    return cs;
}

} // namespace shanxuexi
