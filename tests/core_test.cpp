#include <iostream>
#include <cassert>
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

    // 1. 测试默认词库生成与 JSON 序列化/反序列化
    AppState state;
    state.decks = StorageManager::createDefaultDecks();
    state.stats.firstUseDate = currentTimestampMs() - 8LL * 86400000LL; // 模拟已使用 9 天
    assert(state.decks.size() == 1);
    assert(state.decks[0].words.size() == 5);
    std::cout << "[PASS] 1. 默认词库初始化成功，共 " << state.decks[0].words.size() << " 个初始单词。\n";

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

    // 3. 测试加权随机抽样与测验会话引擎
    QuizEngine quiz;
    std::vector<Word> sampled = QuizEngine::weightedSample(state.decks[0].words, 3);
    assert(sampled.size() == 3);
    quiz.start(sampled);
    assert(quiz.isActive() == true);
    quiz.submitAnswer("know");
    quiz.submitAnswer("fuzzy");
    quiz.submitAnswer("wrong");
    assert(quiz.isFinished() == true);
    assert(quiz.knowCount() == 1);
    assert(quiz.fuzzyCount() == 1);
    assert(quiz.wrongCount() == 1);
    assert(quiz.wrongWords().size() == 2);
    std::cout << "[PASS] 3. 加权随机抽样与测验状态机验证通过 (正确率: " << quiz.accuracyRate() << "%)。\n";

    // 4. 测试打卡日历与连续天数计算
    CheckinManager::checkinToday(state.checkins);
    assert(CheckinManager::isCheckedInToday(state.checkins) == true);
    assert(CheckinManager::calcStreak(state.checkins) >= 1);
    CalendarMonthView cal = CheckinManager::buildCalendarMonth(2026, 10, state.checkins);
    assert(!cal.cells.empty() && cal.cells.size() % 7 == 0);
    std::cout << "[PASS] 4. 打卡日历网格生成与连续天数计算通过 (" << cal.title << ", 单元格数: " << cal.cells.size() << ")。\n";

    // 5. 测试 54 项多主题成就系统
    state.stats.quizCount = 1;
    state.stats.perfectCount = 1;
    ComputedStats cs = CheckinManager::computeStats(state);
    std::vector<std::string> unlocked = AchievementEngine::checkAndUnlock(state, cs);
    assert(!unlocked.empty());
    std::cout << "[PASS] 5. 成就引擎自动判定通过，本次解锁成就数: " << unlocked.size() << " (含: " << unlocked[0] << ")。\n";

    // 6. 测试 JSON 备份导出与导入闭环
    std::string backupJson = StorageManager::exportBackupJson(state, true);
    AppState restored = StorageManager::importBackupJson(backupJson);
    assert(restored.decks.size() == state.decks.size());
    assert(restored.decks[0].words.size() == state.decks[0].words.size());
    assert(restored.badges.size() == state.badges.size());
    std::cout << "[PASS] 6. JSON 备份导出与恢复闭环验证通过 (JSON 长度: " << backupJson.size() << " 字节)。\n";

    std::cout << "\n🎉 全部 C++ 核心模块单元测试通过！\n";
    return 0;
}
