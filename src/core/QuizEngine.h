#ifndef SHANXUEXI_QUIZENGINE_H
#define SHANXUEXI_QUIZENGINE_H

#include "Models.h"
#include <vector>
#include <string>

namespace shanxuexi {

/**
 * @brief 单词测验与加权抽样引擎
 * 实现 Efraimidis-Spirakis 加权不放回随机抽样算法，以及测验进度与错题重练管理
 */
class QuizEngine {
public:
    QuizEngine() = default;

    /**
     * @brief Fisher-Yates 随机洗牌算法
     */
    static std::vector<Word> shuffleWords(std::vector<Word> list);

    /**
     * @brief 加权随机不放回抽样（错词、模糊词、未掌握词获得更高被抽中概率）
     * 权重公式：weight = 1 + wrong * 5 + fuzzy * 2 + (know == 0 ? 0.5 : 0)
     * 排序键值：key = rand^(1 / weight)
     */
    static std::vector<Word> weightedSample(const std::vector<Word>& pool, int count);

    // 测验会话管理
    void start(const std::vector<Word>& words);
    bool isActive() const;
    bool isFinished() const;

    int currentIndex() const { return m_idx; }
    int totalCount() const { return static_cast<int>(m_list.size()); }
    int knowCount() const { return m_ok; }
    int fuzzyCount() const { return m_fz; }
    int wrongCount() const;
    int accuracyRate() const;

    const Word* currentWord() const;
    const std::vector<Word>& quizList() const { return m_list; }

    /**
     * @brief 获取需要强化重练的单词列表（包含本次标记为 "fuzzy" 或 "wrong" 的单词）
     */
    const std::vector<Word>& retryWords() const { return m_retryWords; }
    const std::vector<Word>& wrongWords() const { return m_retryWords; }

    /**
     * @brief 记录当前题目的回答 ("know" | "fuzzy" | "wrong") 并推进到下一题
     * @return 被回答的单词副本（方便外层同步更新持久化词库）
     */
    Word submitAnswer(const std::string& type);

    /**
     * @brief 使用本次测验中模糊/没记住的词重新开始一轮强化测验
     */
    bool startRetryWrong();

private:
    std::vector<Word> m_list;
    std::vector<Word> m_retryWords;
    int m_idx = 0;
    int m_ok = 0;
    int m_fz = 0;
    bool m_active = false;
};

} // namespace shanxuexi

#endif // SHANXUEXI_QUIZENGINE_H
