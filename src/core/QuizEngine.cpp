#include "QuizEngine.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace shanxuexi {

static std::mt19937_64& globalRng() {
    static std::mt19937_64 rng(static_cast<uint64_t>(currentTimestampMs()));
    return rng;
}

std::vector<Word> QuizEngine::shuffleWords(std::vector<Word> list) {
    if (list.size() <= 1) return list;
    std::shuffle(list.begin(), list.end(), globalRng());
    return list;
}

std::vector<Word> QuizEngine::weightedSample(const std::vector<Word>& pool, int count) {
    if (count <= 0 || pool.empty()) return {};
    if (static_cast<int>(pool.size()) <= count) {
        return shuffleWords(pool);
    }

    struct WeightedItem {
        Word word;
        double key;
    };

    std::uniform_real_distribution<double> dist(1e-9, 1.0);
    std::vector<WeightedItem> items;
    items.reserve(pool.size());

    for (const auto& w : pool) {
        const WordStats& s = w.stats;
        double weight = 1.0 + s.wrong * 5.0 + s.fuzzy * 2.0 + (s.know == 0 ? 0.5 : 0.0);
        double r = dist(globalRng());
        double key = std::pow(r, 1.0 / weight);
        items.push_back({w, key});
    }

    std::sort(items.begin(), items.end(), [](const WeightedItem& a, const WeightedItem& b) {
        return a.key > b.key;
    });

    std::vector<Word> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(std::move(items[i].word));
    }
    return result;
}

void QuizEngine::start(const std::vector<Word>& words) {
    m_list = words;
    m_wrong.clear();
    m_idx = 0;
    m_ok = 0;
    m_fz = 0;
    m_active = !m_list.empty();
}

bool QuizEngine::isActive() const {
    return m_active && m_idx < static_cast<int>(m_list.size());
}

bool QuizEngine::isFinished() const {
    return m_active && m_idx >= static_cast<int>(m_list.size());
}

int QuizEngine::wrongCount() const {
    int total = static_cast<int>(m_list.size());
    return std::max(0, total - m_ok - m_fz);
}

int QuizEngine::accuracyRate() const {
    if (m_list.empty()) return 0;
    return static_cast<int>(std::round(static_cast<double>(m_ok) / m_list.size() * 100.0));
}

const Word* QuizEngine::currentWord() const {
    if (m_idx < 0 || m_idx >= static_cast<int>(m_list.size())) {
        return nullptr;
    }
    return &m_list[m_idx];
}

Word QuizEngine::submitAnswer(const std::string& type) {
    Word answered;
    if (!isActive()) return answered;

    answered = m_list[m_idx];
    if (type == "know") {
        m_ok++;
    } else if (type == "fuzzy") {
        m_fz++;
        m_wrong.push_back(answered);
    } else {
        m_wrong.push_back(answered);
    }
    m_idx++;
    return answered;
}

bool QuizEngine::startRetryWrong() {
    if (m_wrong.empty()) return false;
    std::vector<Word> retryPool = shuffleWords(m_wrong);
    start(retryPool);
    return true;
}

} // namespace shanxuexi
