#include <iostream>
#include <cassert>
#include <stdexcept>
#include "core/Models.h"
#include "core/JsonHelper.h"
#include "core/StorageManager.h"
#include "core/EbbinghausEngine.h"
#include "core/QuizEngine.h"
#include "core/CheckinManager.h"
#include "core/AchievementEngine.h"

using namespace shanxuexi;

int main() {
    std::cout << "==========================================\n";
    std::cout << "  闪学习 v1.0 - C++ 核心引擎自动化单元测试\n";
    std::cout << "==========================================\n\n";

    // 1. 测试默认词库生成与同毫秒 UID 唯一性
    std::string uid1 = generateUid();
    std::string uid2 = generateUid();
    assert(uid1 != uid2);

    AppState state;
    state.decks = StorageManager::createDefaultDecks();
    state.stats.firstUseDate = currentTimestampMs() - 8LL * 86400000LL; // 模拟已使用 9 天
    assert(state.decks.size() == 1);
    assert(state.decks[0].words.size() == 5);
    std::cout << "[PASS] 1. 默认词库初始化及同毫秒 UID 防碰撞验证成功。\n";

    // 2. 测试艾宾浩斯遗忘曲线调度引擎
    Word& w0 = state.decks[0].words[0]; // abandon
    assert(EbbinghausEngine::isDue(w0) == true);
    assert(EbbinghausEngine::nextReviewText(w0).text == "待首次复习");

    int64_t nowMs = currentTimestampMs();
    EbbinghausEngine::bumpStat(w0, "know", nowMs);
    assert(w0.stats.level == 1);
    assert(w0.stats.know == 1);
    assert(EbbinghausEngine::isDue(w0, nowMs) == false);
    assert(EbbinghausEngine::isDue(w0, nowMs + 86400000LL) == true); // 1 天后到期
    std::cout << "[PASS] 2. 艾宾浩斯遗忘曲线等级晋升与到期判定验证通过。\n";

    // 3. 测试加权随机抽样与测验会话引擎（含半途统计口径验证）
    QuizEngine quiz;
    std::vector<Word> sampled = QuizEngine::weightedSample(state.decks[0].words, 3);
    assert(sampled.size() == 3);
    quiz.start(sampled);
    assert(quiz.isActive() == true);
    assert(quiz.wrongCount() == 0);   // 尚未答题时错题数应为 0
    assert(quiz.accuracyRate() == 0);

    quiz.submitAnswer("know");
    assert(quiz.wrongCount() == 0);   // 已答 1 题且答对，未答的 2 题不应计入错题
    assert(quiz.accuracyRate() == 100);

    quiz.submitAnswer("fuzzy");
    quiz.submitAnswer("wrong");
    assert(quiz.isFinished() == true);
    assert(quiz.knowCount() == 1);
    assert(quiz.fuzzyCount() == 1);
    assert(quiz.wrongCount() == 1);
    assert(quiz.retryWords().size() == 2);
    std::cout << "[PASS] 3. 加权随机抽样与测验状态机验证通过 (正确率: " << quiz.accuracyRate() << "%)。\n";

    // 4. 测试打卡日历与连续天数计算
    CheckinManager::checkinToday(state.checkins);
    assert(CheckinManager::isCheckedInToday(state.checkins) == true);
    assert(CheckinManager::calcStreak(state.checkins) >= 1);
    assert(CheckinManager::countTotalCheckins(state.checkins) == 1);
    CalendarMonthView cal = CheckinManager::buildCalendarMonth(2026, 10, state.checkins);
    assert(!cal.cells.empty() && cal.cells.size() % 7 == 0);
    std::cout << "[PASS] 4. 打卡日历网格生成与连续天数计算通过 (" << cal.title << ", 单元格数: " << cal.cells.size() << ")。\n";

    // 5. 测试 54 项多主题成就系统与彩蛋边界
    state.stats.quizCount = 1;
    state.stats.perfectCount = 1;
    ComputedStats cs = CheckinManager::computeStats(state);
    std::vector<std::string> unlocked = AchievementEngine::checkAndUnlock(state, cs);
    assert(!unlocked.empty());
    state.badges["legacy_invalid_badge"] = 0;
    assert(AchievementEngine::toggleAllBadgesEasterEgg(state) == true);
    assert(AchievementEngine::toggleAllBadgesEasterEgg(state) == false);
    std::cout << "[PASS] 5. 成就引擎自动判定与彩蛋边界校验通过。\n";

    // 6. 测试 JSON 备份导出、UTF-16 代理对解析与尾部非法字符校验
    JsonValue emojiVal = JsonValue::parse("\"\\uD83D\\uDE00\"");
    assert(emojiVal.asString() == "\xF0\x9F\x98\x80");
    bool caughtTrailingGarbage = false;
    try {
        JsonValue::parse("{\"a\":1}trailing");
    } catch (const std::runtime_error&) {
        caughtTrailingGarbage = true;
    }
    assert(caughtTrailingGarbage);

    std::string backupJson = StorageManager::exportBackupJson(state, true);
    AppState restored = StorageManager::importBackupJson(backupJson);
    assert(restored.decks.size() == state.decks.size());
    assert(restored.decks[0].words.size() == state.decks[0].words.size());
    std::cout << "[PASS] 6. JSON 备份恢复、UTF-16 代理对与语法健壮性验证通过 (JSON 长度: " << backupJson.size() << " 字节)。\n";

    std::cout << "\n🎉 全部 C++ 核心模块单元测试通过！\n";
    return 0;
}
