#include "AppController.h"
#include <QStandardPaths>
#include <QDir>
#include <QGuiApplication>
#include <QClipboard>
#include <algorithm>

namespace shanxuexi {

static bool matchesSearchQuery(const Word& w, const std::string& lowerQuery) {
    if (lowerQuery.empty()) return true;
    return toLowerAscii(w.en).find(lowerQuery) != std::string::npos ||
           toLowerAscii(w.cn).find(lowerQuery) != std::string::npos;
}

AppController::AppController(QObject* parent)
    : QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!dataDir.isEmpty()) {
        QDir().mkpath(dataDir);
        QString filePath = dataDir + "/shanxuexi_data.json";
        m_storage.setStorageFilePath(filePath.toStdString());
    }
    m_state = m_storage.loadState();
}

AppController::~AppController() {
    m_storage.saveState(m_state);
}

QString AppController::currentTheme() const {
    return QString::fromStdString(m_state.theme);
}

QString AppController::interactionMode() const {
    return QString::fromStdString(m_state.interaction);
}

QString AppController::sortMode() const {
    return QString::fromStdString(m_state.sortMode);
}

QString AppController::filterMode() const {
    return QString::fromStdString(m_state.filterMode);
}

QString AppController::currentDeckTitle() const {
    if (m_currentDeckId.isEmpty() || m_currentDeckId == "__all__") {
        return QStringLiteral("全部单词");
    }
    std::string id = m_currentDeckId.toStdString();
    for (const auto& d : m_state.decks) {
        if (d.id == id) {
            return QString::fromStdString(d.name);
        }
    }
    return QStringLiteral("目录");
}

int AppController::totalWordCount() const {
    int count = 0;
    for (const auto& d : m_state.decks) {
        count += static_cast<int>(d.words.size());
    }
    return count;
}

int AppController::totalDeckCount() const {
    return static_cast<int>(m_state.decks.size());
}

bool AppController::checkedInToday() const {
    return CheckinManager::isCheckedInToday(m_state.checkins);
}

int AppController::unlockedBadgeCount() const {
    int cnt = 0;
    for (const auto& def : AchievementEngine::allDefinitions()) {
        auto it = m_state.badges.find(def.id);
        if (it != m_state.badges.end() && it->second > 0) {
            cnt++;
        }
    }
    return cnt;
}

int AppController::totalBadgeCount() const {
    return static_cast<int>(AchievementEngine::allDefinitions().size());
}

std::vector<Word> AppController::collectAllWords() const {
    return collectRangeWords("__all__");
}

std::vector<Word> AppController::collectRangeWords(const std::string& rangeId) const {
    bool isAll = (rangeId.empty() || rangeId == "__all__");
    std::vector<Word> out;
    if (isAll) {
        out.reserve(static_cast<size_t>(totalWordCount()));
    }

    for (const auto& d : m_state.decks) {
        if (!isAll && d.id != rangeId) continue;
        if (!isAll) {
            out.reserve(d.words.size());
        }
        for (const auto& w : d.words) {
            out.push_back(w);
            if (out.back().deckId.empty()) out.back().deckId = d.id;
            if (out.back().deckName.empty()) out.back().deckName = d.name;
        }
        if (!isAll) break;
    }
    return out;
}

QVariantMap AppController::wordToVariantMap(const Word& w) const {
    QVariantMap m;
    m["en"] = QString::fromStdString(w.en);
    m["cn"] = QString::fromStdString(w.cn);
    m["pos"] = QString::fromStdString(w.pos);
    m["ex"] = QString::fromStdString(w.ex);
    m["deckId"] = QString::fromStdString(w.deckId);
    m["deckName"] = QString::fromStdString(w.deckName);
    m["wrong"] = w.stats.wrong;
    m["fuzzy"] = w.stats.fuzzy;
    m["know"] = w.stats.know;
    m["level"] = w.stats.level;

    NextReviewInfo nr = EbbinghausEngine::nextReviewText(w);
    m["nextReviewCls"] = QString::fromStdString(nr.cls);
    m["nextReviewText"] = QString::fromStdString(nr.text);
    return m;
}

QVariantList AppController::decksModel() const {
    QVariantList list;
    for (const auto& d : m_state.decks) {
        QVariantMap item;
        item["id"] = QString::fromStdString(d.id);
        item["name"] = QString::fromStdString(d.name);
        item["wordCount"] = static_cast<int>(d.words.size());
        list.append(item);
    }
    return list;
}

QVariantList AppController::wordsModel() const {
    std::string deckId = m_currentDeckId.toStdString();
    std::vector<Word> raw = collectRangeWords(deckId);

    std::string q = toLowerAscii(m_deckSearchQuery.trimmed().toStdString());
    if (!q.empty()) {
        std::vector<Word> searched;
        for (const auto& w : raw) {
            if (matchesSearchQuery(w, q)) {
                searched.push_back(w);
            }
        }
        raw = std::move(searched);
    }

    std::vector<Word> processed = EbbinghausEngine::applySortFilter(raw, m_state.sortMode, m_state.filterMode);

    QVariantList out;
    for (const auto& w : processed) {
        out.append(wordToVariantMap(w));
    }
    return out;
}

QVariantMap AppController::reviewBrief() const {
    ReviewPoolAnalysis analysis = EbbinghausEngine::analyzeReviewPool(collectAllWords());
    QVariantMap m;
    m["total"] = analysis.total;
    m["wrong"] = analysis.wrong;
    m["fuzzy"] = analysis.fuzzy;
    m["dueOk"] = analysis.dueOk;
    double totalD = analysis.total > 0 ? static_cast<double>(analysis.total) : 1.0;
    m["wrongPct"] = analysis.total > 0 ? (analysis.wrong / totalD) : 0.0;
    m["fuzzyPct"] = analysis.total > 0 ? (analysis.fuzzy / totalD) : 0.0;
    m["dueOkPct"] = analysis.total > 0 ? (analysis.dueOk / totalD) : 0.0;
    return m;
}

QVariantList AppController::badgesModel() const {
    ComputedStats cs = CheckinManager::computeStats(m_state);
    std::vector<BadgeViewItem> badges = AchievementEngine::buildBadgeViews(m_state, cs);
    QVariantList list;
    for (const auto& b : badges) {
        QVariantMap m;
        m["id"] = QString::fromStdString(b.id);
        m["group"] = QString::fromStdString(b.group);
        m["tier"] = b.tier;
        m["name"] = QString::fromStdString(b.name);
        m["unlocked"] = b.unlocked;
        m["dateText"] = QString::fromStdString(b.dateText);
        m["progressText"] = QString::fromStdString(b.progressText);
        m["progressRatio"] = b.progressRatio;
        m["hue"] = b.hue;
        m["sat"] = b.sat;
        m["light"] = b.light;
        list.append(m);
    }
    return list;
}

QVariantMap AppController::backupStats() const {
    ComputedStats cs = CheckinManager::computeStats(m_state);
    QVariantMap m;
    m["words"] = cs.wordsAdded;
    m["decks"] = cs.decksCreated;
    m["checkins"] = cs.totalCheckins;
    m["badges"] = unlockedBadgeCount();
    return m;
}

QVariantMap AppController::quizCurrentWord() const {
    const Word* w = m_quiz.currentWord();
    if (!w) return {};
    return wordToVariantMap(*w);
}

QVariantMap AppController::quizResult() const {
    QVariantMap m;
    m["rate"] = m_quiz.accuracyRate();
    m["ok"] = m_quiz.knowCount();
    m["fz"] = m_quiz.fuzzyCount();
    m["wrong"] = m_quiz.wrongCount();

    QVariantList wrongList;
    for (const auto& w : m_quiz.retryWords()) {
        Word latest = w;
        for (const auto& d : m_state.decks) {
            if (d.id == w.deckId) {
                for (const auto& dw : d.words) {
                    if (equalsIgnoreCaseAscii(dw.en, w.en)) {
                        latest = dw;
                        latest.deckId = d.id;
                        latest.deckName = d.name;
                        break;
                    }
                }
                break;
            }
        }
        wrongList.append(wordToVariantMap(latest));
    }
    m["wrongList"] = wrongList;
    return m;
}

void AppController::saveAndCheckAchievements() {
    ComputedStats cs = CheckinManager::computeStats(m_state);
    std::vector<std::string> newBadges = AchievementEngine::checkAndUnlock(m_state, cs);
    m_storage.saveState(m_state);
    emit stateChanged();
    emit wordsChanged();

    for (const auto& name : newBadges) {
        emit toastMessage(QStringLiteral("🏅 解锁成就：") + QString::fromStdString(name));
    }
}

void AppController::setTheme(const QString& themeId) {
    std::string tid = themeId.toStdString();
    if (tid.empty() || m_state.theme == tid) return;
    m_state.theme = tid;
    m_storage.saveState(m_state);
    emit themeChanged();
    emit stateChanged();
}

void AppController::setInteractionMode(const QString& mode) {
    std::string m = mode.toStdString();
    if (m != "tap" && m != "swipe") return;
    m_state.interaction = m;
    m_storage.saveState(m_state);
    emit stateChanged();
    emit toastMessage(m == "swipe" ? QStringLiteral("已切换为「滑动模式」") : QStringLiteral("已切换为「点按模式」"));
}

void AppController::setSortAndFilter(const QString& sortMode, const QString& filterMode) {
    if (!sortMode.isEmpty()) m_state.sortMode = sortMode.toStdString();
    if (!filterMode.isEmpty()) m_state.filterMode = filterMode.toStdString();
    m_storage.saveState(m_state);
    emit wordsChanged();
}

void AppController::resetSortAndFilter() {
    m_state.sortMode = "default";
    m_state.filterMode = "all";
    m_storage.saveState(m_state);
    emit wordsChanged();
    emit toastMessage(QStringLiteral("已重置排序与筛选"));
}

void AppController::openDeck(const QString& deckId) {
    m_currentDeckId = deckId.isEmpty() ? QStringLiteral("__all__") : deckId;
    m_deckSearchQuery.clear();
    emit wordsChanged();
}

void AppController::setDeckSearchQuery(const QString& query) {
    m_deckSearchQuery = query;
    emit wordsChanged();
}

QVariantList AppController::searchAllWords(const QString& query) const {
    std::string q = toLowerAscii(query.trimmed().toStdString());
    if (q.empty()) return {};

    QVariantList out;
    for (const auto& w : collectAllWords()) {
        if (matchesSearchQuery(w, q)) {
            out.append(wordToVariantMap(w));
        }
    }
    return out;
}

bool AppController::createDeck(const QString& name) {
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        emit toastMessage(QStringLiteral("名称不能为空"));
        return false;
    }
    std::string stdName = trimmed.toStdString();
    for (const auto& d : m_state.decks) {
        if (d.name == stdName) {
            emit toastMessage(QStringLiteral("已存在同名目录"));
            return false;
        }
    }

    Deck d;
    d.id = generateUid();
    d.name = stdName;
    m_state.decks.push_back(std::move(d));
    saveAndCheckAchievements();
    emit toastMessage(QStringLiteral("已创建：") + trimmed);
    return true;
}

bool AppController::renameDeck(const QString& deckId, const QString& newName) {
    QString trimmed = newName.trimmed();
    if (trimmed.isEmpty()) {
        emit toastMessage(QStringLiteral("名称不能为空"));
        return false;
    }
    std::string id = deckId.toStdString();
    std::string stdName = trimmed.toStdString();

    // 检查是否与其他已有目录重名
    for (const auto& d : m_state.decks) {
        if (d.id != id && d.name == stdName) {
            emit toastMessage(QStringLiteral("已存在同名目录"));
            return false;
        }
    }

    for (auto& d : m_state.decks) {
        if (d.id == id) {
            d.name = stdName;
            for (auto& w : d.words) {
                w.deckName = stdName;
            }
            m_storage.saveState(m_state);
            emit stateChanged();
            emit wordsChanged();
            emit toastMessage(QStringLiteral("已重命名"));
            return true;
        }
    }
    return false;
}

bool AppController::deleteDeck(const QString& deckId) {
    std::string id = deckId.toStdString();
    auto it = std::remove_if(m_state.decks.begin(), m_state.decks.end(),
                             [&id](const Deck& d) { return d.id == id; });
    if (it == m_state.decks.end()) return false;
    m_state.decks.erase(it, m_state.decks.end());
    if (m_currentDeckId == deckId) {
        m_currentDeckId = QStringLiteral("__all__");
    }
    m_storage.saveState(m_state);
    emit stateChanged();
    emit wordsChanged();
    emit toastMessage(QStringLiteral("已删除目录"));
    return true;
}

bool AppController::addWord(const QString& deckId, const QString& en,
                            const QString& cn, const QString& pos, const QString& ex) {
    QString trimEn = en.trimmed();
    QString trimCn = cn.trimmed();

    // 原版彩蛋：输入 en="wsy", cn="bb" 一键解锁/关闭全部 54 个徽章
    if (trimEn == QStringLiteral("wsy") && trimCn == QStringLiteral("bb")) {
        bool allOn = AchievementEngine::toggleAllBadgesEasterEgg(m_state);
        m_storage.saveState(m_state);
        emit stateChanged();
        if (allOn) {
            emit toastMessage(QStringLiteral("🔓 已解锁全部 %1 个徽章").arg(totalBadgeCount()));
        } else {
            emit toastMessage(QStringLiteral("🔒 已关闭全部徽章解锁"));
        }
        return true;
    }

    if (trimEn.isEmpty() || trimCn.isEmpty()) {
        emit toastMessage(QStringLiteral("单词和释义不能为空"));
        return false;
    }

    std::string targetDeckId = deckId.toStdString();
    if ((targetDeckId.empty() || targetDeckId == "__all__") && !m_state.decks.empty()) {
        targetDeckId = m_state.decks.front().id;
    }

    std::string stdEn = trimEn.toStdString();
    for (auto& d : m_state.decks) {
        if (d.id == targetDeckId) {
            for (const auto& existing : d.words) {
                if (equalsIgnoreCaseAscii(existing.en, stdEn)) {
                    emit toastMessage(QStringLiteral("该目录中已存在「%1」").arg(trimEn));
                    return false;
                }
            }
            Word w;
            w.en = stdEn;
            w.cn = trimCn.toStdString();
            w.pos = pos.trimmed().toStdString();
            w.ex = ex.trimmed().toStdString();
            w.deckId = d.id;
            w.deckName = d.name;
            d.words.push_back(std::move(w));

            saveAndCheckAchievements();
            emit toastMessage(QStringLiteral("已添加：") + trimEn);
            return true;
        }
    }

    emit toastMessage(QStringLiteral("请先新建一个目录"));
    return false;
}

bool AppController::deleteWord(const QString& deckId, const QString& en) {
    std::string did = deckId.toStdString();
    std::string stdEn = en.toStdString();
    for (auto& d : m_state.decks) {
        if (d.id == did) {
            auto it = std::find_if(d.words.begin(), d.words.end(),
                                   [&stdEn](const Word& w) { return equalsIgnoreCaseAscii(w.en, stdEn); });
            if (it != d.words.end()) {
                d.words.erase(it);
                saveAndCheckAchievements();
                emit toastMessage(QStringLiteral("已删除"));
                return true;
            }
        }
    }
    return false;
}

int AppController::getRangeWordCount(const QString& rangeId) const {
    std::string rid = rangeId.toStdString();
    if (rid.empty() || rid == "__all__") {
        return totalWordCount();
    }
    for (const auto& d : m_state.decks) {
        if (d.id == rid) {
            return static_cast<int>(d.words.size());
        }
    }
    return 0;
}

bool AppController::startNormalQuiz(const QString& rangeId, int count) {
    std::vector<Word> pool = collectRangeWords(rangeId.toStdString());
    if (pool.empty()) {
        emit toastMessage(QStringLiteral("该范围还没有单词"));
        return false;
    }
    int n = (count <= 0) ? static_cast<int>(pool.size())
                         : std::min(count, static_cast<int>(pool.size()));
    std::vector<Word> sampled = QuizEngine::weightedSample(pool, n);
    m_quiz.start(sampled);
    emit quizChanged();
    return true;
}

bool AppController::startSmartReviewQuiz() {
    ReviewPoolAnalysis analysis = EbbinghausEngine::analyzeReviewPool(collectAllWords());
    if (analysis.total == 0) {
        emit toastMessage(QStringLiteral("今天没有需要复习的单词"));
        return false;
    }
    int n = std::min(static_cast<int>(analysis.pool.size()), 60);
    std::vector<Word> list(analysis.pool.begin(), analysis.pool.begin() + n);
    m_quiz.start(list);
    emit quizChanged();
    emit toastMessage(QStringLiteral("已生成 %1 个复习词").arg(n));
    return true;
}

void AppController::answerQuiz(const QString& type) {
    if (!m_quiz.isActive()) return;
    std::string stdType = type.toStdString();
    Word answered = m_quiz.submitAnswer(stdType);

    // 同步更新内存词库中的对应单词统计
    for (auto& d : m_state.decks) {
        if (d.id == answered.deckId) {
            for (auto& w : d.words) {
                if (equalsIgnoreCaseAscii(w.en, answered.en)) {
                    EbbinghausEngine::bumpStat(w, stdType);
                    break;
                }
            }
            break;
        }
    }

    if (m_quiz.isFinished()) {
        finishQuizSession();
    } else {
        // 答题进行中仅通知测验卡片更新，避免逐题全量写盘与整表 UI 模型重算
        emit quizChanged();
    }
}

void AppController::finishQuizSession() {
    CheckinManager::checkinToday(m_state.checkins);
    m_state.stats.quizCount++;

    int total = m_quiz.totalCount();
    int rate = m_quiz.accuracyRate();
    if (rate == 100 && total > 0) {
        m_state.stats.perfectCount++;
        m_state.stats.consecutivePerfect++;
        m_state.stats.lastAllKnow = true;
    } else {
        m_state.stats.consecutivePerfect = 0;
        m_state.stats.lastAllKnow = false;
    }

    int hour = currentLocalHour();
    if (hour >= 6 && hour < 9) {
        m_state.stats.morningQuiz++;
    } else if (hour >= 23 || hour < 2) {
        m_state.stats.nightQuiz++;
    } else if (hour >= 12 && hour < 14) {
        m_state.stats.noonQuiz++;
    }

    saveAndCheckAchievements();
    emit quizChanged();
}

bool AppController::retryWrongQuiz() {
    if (!m_quiz.startRetryWrong()) return false;
    emit quizChanged();
    return true;
}

void AppController::abortQuiz() {
    bool hadProgress = (m_quiz.currentIndex() > 0 && !m_quiz.isFinished());
    m_quiz.start({});
    if (hadProgress) {
        m_storage.saveState(m_state);
        emit stateChanged();
        emit wordsChanged();
    }
    emit quizChanged();
}

QVariantMap AppController::getCalendarView(int year, int month) const {
    CalendarMonthView view = CheckinManager::buildCalendarMonth(year, month, m_state.checkins);
    QVariantMap out;
    out["year"] = view.year;
    out["month"] = view.month;
    out["title"] = QString::fromStdString(view.title);
    out["streak"] = view.streak;
    out["monthCount"] = view.monthCount;
    out["totalCount"] = view.totalCount;

    QVariantList cellsList;
    for (const auto& c : view.cells) {
        QVariantMap cm;
        cm["day"] = c.day;
        cm["dateStr"] = QString::fromStdString(c.dateStr);
        cm["isOtherMonth"] = c.isOtherMonth;
        cm["isChecked"] = c.isChecked;
        cm["isToday"] = c.isToday;
        cellsList.append(cm);
    }
    out["cells"] = cellsList;
    return out;
}

QString AppController::exportBackupText() const {
    return QString::fromStdString(StorageManager::exportBackupJson(m_state, false));
}

void AppController::copyBackupToClipboard() {
    QString json = exportBackupText();
    if (QClipboard* cb = QGuiApplication::clipboard()) {
        cb->setText(json);
        emit toastMessage(QStringLiteral("已复制备份文本，可粘贴到微信发送"));
    } else {
        emit toastMessage(QStringLiteral("复制失败"));
    }
}

bool AppController::importBackupText(const QString& jsonText) {
    QString trimmed = jsonText.trimmed();
    if (trimmed.isEmpty()) {
        emit toastMessage(QStringLiteral("请先粘贴备份文本"));
        return false;
    }
    try {
        AppState restored = StorageManager::importBackupJson(trimmed.toStdString());
        m_state = std::move(restored);
        m_storage.saveState(m_state);
        emit themeChanged();
        emit stateChanged();
        emit wordsChanged();
        emit toastMessage(QStringLiteral("✅ 已从备份恢复成功"));
        return true;
    } catch (const std::exception& e) {
        emit toastMessage(QStringLiteral("解析失败：") + QString::fromUtf8(e.what()));
        return false;
    }
}

} // namespace shanxuexi
