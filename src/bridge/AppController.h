#ifndef SHANXUEXI_APPCONTROLLER_H
#define SHANXUEXI_APPCONTROLLER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "core/Models.h"
#include "core/StorageManager.h"
#include "core/EbbinghausEngine.h"
#include "core/QuizEngine.h"
#include "core/CheckinManager.h"
#include "core/AchievementEngine.h"

namespace shanxuexi {

/**
 * @brief Qt / QML 桥接控制器
 * 将纯 C++ 业务逻辑层包装为 Q_OBJECT，供移动端 QML 界面直接响应式绑定与调用
 */
class AppController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QString interactionMode READ interactionMode WRITE setInteractionMode NOTIFY stateChanged)
    Q_PROPERTY(QString sortMode READ sortMode NOTIFY wordsChanged)
    Q_PROPERTY(QString filterMode READ filterMode NOTIFY wordsChanged)
    Q_PROPERTY(QString currentDeckId READ currentDeckId NOTIFY wordsChanged)
    Q_PROPERTY(QString currentDeckTitle READ currentDeckTitle NOTIFY wordsChanged)

    Q_PROPERTY(int totalWordCount READ totalWordCount NOTIFY stateChanged)
    Q_PROPERTY(int totalDeckCount READ totalDeckCount NOTIFY stateChanged)
    Q_PROPERTY(bool checkedInToday READ checkedInToday NOTIFY stateChanged)
    Q_PROPERTY(int unlockedBadgeCount READ unlockedBadgeCount NOTIFY stateChanged)
    Q_PROPERTY(int totalBadgeCount READ totalBadgeCount CONSTANT)

    Q_PROPERTY(QVariantList decksModel READ decksModel NOTIFY stateChanged)
    Q_PROPERTY(QVariantList wordsModel READ wordsModel NOTIFY wordsChanged)
    Q_PROPERTY(QVariantMap reviewBrief READ reviewBrief NOTIFY stateChanged)
    Q_PROPERTY(QVariantList badgesModel READ badgesModel NOTIFY stateChanged)
    Q_PROPERTY(QVariantMap backupStats READ backupStats NOTIFY stateChanged)

    Q_PROPERTY(bool quizActive READ quizActive NOTIFY quizChanged)
    Q_PROPERTY(bool quizFinished READ quizFinished NOTIFY quizChanged)
    Q_PROPERTY(int quizIndex READ quizIndex NOTIFY quizChanged)
    Q_PROPERTY(int quizTotal READ quizTotal NOTIFY quizChanged)
    Q_PROPERTY(QVariantMap quizCurrentWord READ quizCurrentWord NOTIFY quizChanged)
    Q_PROPERTY(QVariantMap quizResult READ quizResult NOTIFY quizChanged)

public:
    explicit AppController(QObject* parent = nullptr);
    ~AppController() override;

    // 属性 Getter
    QString currentTheme() const;
    QString interactionMode() const;
    QString sortMode() const;
    QString filterMode() const;
    QString currentDeckId() const { return m_currentDeckId; }
    QString currentDeckTitle() const;

    int totalWordCount() const;
    int totalDeckCount() const;
    bool checkedInToday() const;
    int unlockedBadgeCount() const;
    int totalBadgeCount() const;

    QVariantList decksModel() const;
    QVariantList wordsModel() const;
    QVariantMap reviewBrief() const;
    QVariantList badgesModel() const;
    QVariantMap backupStats() const;

    bool quizActive() const { return m_quiz.isActive(); }
    bool quizFinished() const { return m_quiz.isFinished(); }
    int quizIndex() const { return m_quiz.currentIndex(); }
    int quizTotal() const { return m_quiz.totalCount(); }
    QVariantMap quizCurrentWord() const;
    QVariantMap quizResult() const;

    // 供 QML 调用的业务动作接口
    Q_INVOKABLE void setTheme(const QString& themeId);
    Q_INVOKABLE void setInteractionMode(const QString& mode);
    Q_INVOKABLE void setSortAndFilter(const QString& sortMode, const QString& filterMode);
    Q_INVOKABLE void resetSortAndFilter();

    Q_INVOKABLE void openDeck(const QString& deckId);
    Q_INVOKABLE void setDeckSearchQuery(const QString& query);
    Q_INVOKABLE QVariantList searchAllWords(const QString& query) const;

    Q_INVOKABLE bool createDeck(const QString& name);
    Q_INVOKABLE bool renameDeck(const QString& deckId, const QString& newName);
    Q_INVOKABLE bool deleteDeck(const QString& deckId);

    Q_INVOKABLE bool addWord(const QString& deckId, const QString& en,
                             const QString& cn, const QString& pos, const QString& ex);
    Q_INVOKABLE bool deleteWord(const QString& deckId, const QString& en);

    Q_INVOKABLE int getRangeWordCount(const QString& rangeId) const;
    Q_INVOKABLE bool startNormalQuiz(const QString& rangeId, int count);
    Q_INVOKABLE bool startSmartReviewQuiz();
    Q_INVOKABLE void answerQuiz(const QString& type);
    Q_INVOKABLE bool retryWrongQuiz();
    Q_INVOKABLE void abortQuiz();

    Q_INVOKABLE QVariantMap getCalendarView(int year, int month) const;

    Q_INVOKABLE QString exportBackupText() const;
    Q_INVOKABLE void copyBackupToClipboard();
    Q_INVOKABLE bool importBackupText(const QString& jsonText);

signals:
    void stateChanged();
    void wordsChanged();
    void quizChanged();
    void themeChanged();
    void toastMessage(const QString& message);

private:
    std::vector<Word> collectAllWords() const;
    std::vector<Word> collectRangeWords(const std::string& rangeId) const;
    QVariantMap wordToVariantMap(const Word& w) const;
    void saveAndCheckAchievements();
    void finishQuizSession();

    StorageManager m_storage;
    AppState m_state;
    QuizEngine m_quiz;

    QString m_currentDeckId = "__all__";
    QString m_deckSearchQuery;
};

} // namespace shanxuexi

#endif // SHANXUEXI_APPCONTROLLER_H
