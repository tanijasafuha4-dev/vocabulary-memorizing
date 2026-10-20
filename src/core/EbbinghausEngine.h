#ifndef SHANXUEXI_EBBINGHAUSENGINE_H
#define SHANXUEXI_EBBINGHAUSENGINE_H

#include "Models.h"
#include <vector>
#include <string>

namespace shanxuexi {

/**
 * @brief 艾宾浩斯遗忘曲线调度与复习引擎
 * 负责计算单词复习周期、判定到期状态、构建智能复习优先级队列以及排序筛选
 */
class EbbinghausEngine {
public:
    static constexpr int MAX_LEVEL = 7;
    static const std::vector<int>& intervals();

    /**
     * @brief 获取单词当前等级对应的复习间隔天数
     */
    static int getIntervalDays(const Word& word);

    /**
     * @brief 判断单词当前是否到达复习时间点
     */
    static bool isDue(const Word& word, int64_t nowMs = currentTimestampMs());

    /**
     * @brief 获取经过艾宾浩斯优先级排序的待复习单词列表
     * 排序优先级：等级0(没记住/新词) > 上次标记模糊 > 低等级 > 更早复习时间
     */
    static std::vector<Word> getSmartReview(const std::vector<Word>& allWords,
                                            int64_t nowMs = currentTimestampMs());

    /**
     * @brief 分析当前智能复习池构成（错词数、模糊数、到期巩固数）
     */
    static ReviewPoolAnalysis analyzeReviewPool(const std::vector<Word>& allWords,
                                                int64_t nowMs = currentTimestampMs());

    /**
     * @brief 计算单词卡片上展示的下次复习提示文案与样式类
     */
    static NextReviewInfo nextReviewText(const Word& word,
                                         int64_t nowMs = currentTimestampMs());

    /**
     * @brief 根据答题判定结果 ("know" | "fuzzy" | "wrong") 更新单词统计与艾宾浩斯等级
     */
    static void bumpStat(Word& word, const std::string& type,
                         int64_t nowMs = currentTimestampMs());

    /**
     * @brief 对单词列表应用筛选条件与排序规则
     * @param filterMode "all" | "wrong" | "fuzzy" | "mastered"
     * @param sortMode   "default" | "alpha" | "mastery" | "recent"
     */
    static std::vector<Word> applySortFilter(const std::vector<Word>& list,
                                             const std::string& sortMode,
                                             const std::string& filterMode);
};

} // namespace shanxuexi

#endif // SHANXUEXI_EBBINGHAUSENGINE_H
