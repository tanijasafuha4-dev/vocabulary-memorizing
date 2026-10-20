#include "EbbinghausEngine.h"
#include <algorithm>
#include <cmath>
#include <cctype>

namespace shanxuexi {

const std::vector<int>& EbbinghausEngine::intervals() {
    static const std::vector<int> kIntervals = {0, 1, 2, 4, 7, 15, 30, 60};
    return kIntervals;
}

int EbbinghausEngine::getIntervalDays(const Word& word) {
    const auto& iv = intervals();
    int lvl = std::max(0, std::min(word.stats.level, static_cast<int>(iv.size()) - 1));
    return iv[lvl];
}

bool EbbinghausEngine::isDue(const Word& word, int64_t nowMs) {
    const WordStats& s = word.stats;
    if (s.level >= MAX_LEVEL) return false;
    if (s.lastRev <= 0) return true;
    double elapsedDays = static_cast<double>(nowMs - s.lastRev) / 86400000.0;
    return elapsedDays >= static_cast<double>(getIntervalDays(word));
}

std::vector<Word> EbbinghausEngine::getSmartReview(const std::vector<Word>& allWords, int64_t nowMs) {
    std::vector<Word> dueList;
    dueList.reserve(allWords.size());
    for (const auto& w : allWords) {
        if (isDue(w, nowMs)) {
            dueList.push_back(w);
        }
    }

    std::stable_sort(dueList.begin(), dueList.end(), [](const Word& a, const Word& b) {
        const WordStats& sa = a.stats;
        const WordStats& sb = b.stats;

        // 1. 等级为 0（错词或未学词）最优先
        if (sa.level == 0 && sb.level != 0) return true;
        if (sb.level == 0 && sa.level != 0) return false;

        // 2. 上次标记为模糊 ("fuzzy") 的优先于已记住词
        int aFz = (sa.lastType == "fuzzy") ? 1 : 0;
        int bFz = (sb.lastType == "fuzzy") ? 1 : 0;
        if (aFz != bFz) return aFz > bFz;

        // 3. 掌握等级越低越优先
        if (sa.level != sb.level) return sa.level < sb.level;

        // 4. 从未复习或上次复习时间更早的优先
        if (sa.lastRev <= 0 && sb.lastRev <= 0) return false;
        if (sa.lastRev <= 0) return true;
        if (sb.lastRev <= 0) return false;
        return sa.lastRev < sb.lastRev;
    });

    return dueList;
}

ReviewPoolAnalysis EbbinghausEngine::analyzeReviewPool(const std::vector<Word>& allWords, int64_t nowMs) {
    ReviewPoolAnalysis res;
    res.pool = getSmartReview(allWords, nowMs);
    res.total = static_cast<int>(res.pool.size());
    for (const auto& w : res.pool) {
        if (w.stats.level == 0) {
            res.wrong++;
        } else if (w.stats.lastType == "fuzzy") {
            res.fuzzy++;
        } else {
            res.dueOk++;
        }
    }
    return res;
}

NextReviewInfo EbbinghausEngine::nextReviewText(const Word& word, int64_t nowMs) {
    const WordStats& s = word.stats;
    if (s.level >= MAX_LEVEL) {
        return {"mastered", "已掌握"};
    }
    if (s.lastRev <= 0) {
        return {"due", "待首次复习"};
    }
    int interval = getIntervalDays(word);
    double days = static_cast<double>(nowMs - s.lastRev) / 86400000.0;
    double remain = static_cast<double>(interval) - days;
    if (remain <= 0.0) {
        return {"due", "今天该复习"};
    }
    if (remain < 1.0) {
        return {"soon", "即将到期"};
    }
    int remainDays = static_cast<int>(std::ceil(remain));
    if (remainDays == 1) {
        return {"soon", "明天复习"};
    }
    if (remainDays <= 3) {
        return {"soon", std::to_string(remainDays) + "天后复习"};
    }
    return {"later", std::to_string(remainDays) + "天后复习"};
}

void EbbinghausEngine::bumpStat(Word& word, const std::string& type, int64_t nowMs) {
    WordStats& s = word.stats;
    if (type == "know") {
        s.know++;
        s.level = std::min(s.level + 1, MAX_LEVEL);
    } else if (type == "fuzzy") {
        s.fuzzy++;
        s.level = std::max(0, s.level - 1);
    } else {
        s.wrong++;
        s.level = 0;
    }
    s.lastRev = nowMs;
    s.lastType = type;
}

std::vector<Word> EbbinghausEngine::applySortFilter(const std::vector<Word>& list,
                                                    const std::string& sortMode,
                                                    const std::string& filterMode) {
    std::vector<Word> filtered;
    filtered.reserve(list.size());

    for (const auto& w : list) {
        const WordStats& s = w.stats;
        if (filterMode == "wrong") {
            if (s.wrong <= 0) continue;
        } else if (filterMode == "fuzzy") {
            if (s.fuzzy <= 0) continue;
        } else if (filterMode == "mastered") {
            if (!(s.wrong == 0 && s.fuzzy == 0 && s.know > 0)) continue;
        }
        filtered.push_back(w);
    }

    if (sortMode == "alpha") {
        std::stable_sort(filtered.begin(), filtered.end(), [](const Word& a, const Word& b) {
            return toLowerAscii(a.en) < toLowerAscii(b.en);
        });
    } else if (sortMode == "mastery") {
        std::stable_sort(filtered.begin(), filtered.end(), [](const Word& a, const Word& b) {
            int scoreA = a.stats.wrong * 3 + a.stats.fuzzy * 2;
            int scoreB = b.stats.wrong * 3 + b.stats.fuzzy * 2;
            return scoreB < scoreA; // 错误多的排前面
        });
    } else if (sortMode == "recent") {
        std::stable_sort(filtered.begin(), filtered.end(), [](const Word& a, const Word& b) {
            int64_t ra = a.stats.lastRev;
            int64_t rb = b.stats.lastRev;
            if (ra <= 0 && rb <= 0) return false;
            if (ra <= 0) return true;
            if (rb <= 0) return false;
            return rb < ra; // 最近复习的排前面
        });
    }

    return filtered;
}

} // namespace shanxuexi
