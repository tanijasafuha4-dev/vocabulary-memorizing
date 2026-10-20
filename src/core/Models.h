#ifndef SHANXUEXI_MODELS_H
#define SHANXUEXI_MODELS_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <chrono>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace shanxuexi {

/**
 * @brief 获取当前毫秒级时间戳 (Unix Epoch ms)
 */
inline int64_t currentTimestampMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

/**
 * @brief 全局共享的高质量随机数引擎 (结合 random_device 与高精度时钟初始化种子)
 */
inline std::mt19937_64& sharedRng() {
    static std::mt19937_64 rng([]() {
        std::random_device rd;
        auto now = static_cast<uint64_t>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());
        std::seed_seq seq{
            rd(), rd(),
            static_cast<unsigned int>(now & 0xFFFFFFFFULL),
            static_cast<unsigned int>(now >> 32)
        };
        return std::mt19937_64(seq);
    }());
    return rng;
}

/**
 * @brief 生成唯一目录 ID (对齐原版 uid() 逻辑，附带递增序列号防止同毫秒碰撞)
 */
inline std::string generateUid() {
    static uint32_t seq = 0;
    std::uniform_int_distribution<uint32_t> dist(1000, 999999);
    std::ostringstream oss;
    oss << "d" << std::hex << currentTimestampMs() << "_" << (++seq) << "_" << dist(sharedRng());
    return oss.str();
}

/**
 * @brief ASCII 字符串转小写辅助函数
 */
inline std::string toLowerAscii(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

/**
 * @brief ASCII 大小写不敏感字符串相等比较（用于单词匹配）
 */
inline bool equalsIgnoreCaseAscii(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 跨平台线程安全地将 time_t 转换为本地时间 std::tm
 */
inline void getLocalTm(std::time_t t, std::tm& outTm) {
#if defined(_WIN32)
    localtime_s(&outTm, &t);
#else
    localtime_r(&t, &outTm);
#endif
}

/**
 * @brief 获取当前本地时间的小时数 (0 ~ 23)
 */
inline int currentLocalHour() {
    std::time_t now = std::time(nullptr);
    std::tm tmBuf{};
    getLocalTm(now, tmBuf);
    return tmBuf.tm_hour;
}

/**
 * @brief 单个单词的记忆与艾宾浩斯统计数据
 */
struct WordStats {
    int wrong = 0;           // 没记住次数
    int fuzzy = 0;           // 模糊次数
    int know = 0;            // 记住次数
    int level = 0;           // 艾宾浩斯等级 (0 ~ 7)
    int64_t lastRev = 0;     // 上次复习毫秒时间戳
    std::string lastType;    // 上次答题结果: "know" | "fuzzy" | "wrong" | ""
};

/**
 * @brief 单词实体类
 */
struct Word {
    std::string en;          // 英文单词/短语
    std::string cn;          // 中文释义
    std::string pos;         // 词性 (例如 v. / n. / adj.)
    std::string ex;          // 例句
    WordStats stats;         // 学习状态统计

    // 视图聚合辅助字段（跨目录检索或测验时携带所属目录信息）
    std::string deckId;
    std::string deckName;
};

/**
 * @brief 单词目录（卡包）实体类
 */
struct Deck {
    std::string id;
    std::string name;
    std::vector<Word> words;
};

/**
 * @brief 持久化的用户测验统计数据
 */
struct UserStats {
    int quizCount = 0;
    int perfectCount = 0;
    int consecutivePerfect = 0;
    int morningQuiz = 0;
    int nightQuiz = 0;
    int noonQuiz = 0;
    bool lastAllKnow = false;
    int64_t firstUseDate = 0;
};

/**
 * @brief 实时聚合计算的全局统计数据（用于成就系统判定与备份面板展示）
 */
struct ComputedStats {
    int quizCount = 0;
    int perfectCount = 0;
    int consecutivePerfect = 0;
    int wordsAdded = 0;
    int morningQuiz = 0;
    int nightQuiz = 0;
    int noonQuiz = 0;
    bool lastAllKnow = false;
    int fuzzyCount = 0;
    int streak = 0;
    int maxStreak = 0;
    int totalCheckins = 0;
    int decksCreated = 0;
    int daysUsed = 1;
};

/**
 * @brief 下次复习提示标签结构
 */
struct NextReviewInfo {
    std::string cls;   // "due" | "soon" | "later" | "mastered"
    std::string text;  // 例如 "今天该复习" | "待首次复习" | "已掌握" | "3天后复习"
};

/**
 * @brief 智能复习池分析结果
 */
struct ReviewPoolAnalysis {
    int total = 0;
    int wrong = 0;
    int fuzzy = 0;
    int dueOk = 0;
    std::vector<Word> pool;
};

/**
 * @brief 应用全局存档状态（用于本地存储及一键 JSON 备份导出/导入）
 */
struct AppState {
    std::vector<Deck> decks;
    std::map<std::string, bool> checkins;         // "YYYY-MM-DD" -> true
    UserStats stats;
    std::map<std::string, int64_t> badges;        // badgeId -> 解锁时间戳(ms)
    std::string theme = "tech";
    std::string interaction = "tap";              // "tap" | "swipe"
    std::string sortMode = "default";             // "default" | "alpha" | "mastery" | "recent"
    std::string filterMode = "all";               // "all" | "wrong" | "fuzzy" | "mastered"
};

} // namespace shanxuexi

#endif // SHANXUEXI_MODELS_H
