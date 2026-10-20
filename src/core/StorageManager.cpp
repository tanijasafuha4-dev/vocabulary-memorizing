#include "StorageManager.h"
#include <fstream>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace shanxuexi {

StorageManager::StorageManager(std::string storageFilePath)
    : m_filePath(std::move(storageFilePath)) {}

void StorageManager::setStorageFilePath(const std::string& path) {
    m_filePath = path;
}

const std::string& StorageManager::storageFilePath() const {
    return m_filePath;
}

std::vector<Deck> StorageManager::createDefaultDecks() {
    Deck d;
    d.id = generateUid();
    d.name = "Text 1";
    d.words = {
        {"abandon",  "放弃；抛弃",   "v.",    "He had to abandon his car in the snow.", WordStats{}, d.id, d.name},
        {"approach", "接近；方法",   "v./n.", "We need a new approach to this problem.", WordStats{}, d.id, d.name},
        {"benefit",  "利益；有益于", "n./v.", "Regular exercise benefits your health.",  WordStats{}, d.id, d.name},
        {"crucial",  "至关重要的",   "adj.",  "Sleep is crucial for memory.",            WordStats{}, d.id, d.name},
        {"decline",  "下降；婉拒",   "v./n.", "Sales declined sharply last year.",       WordStats{}, d.id, d.name}
    };
    return {d};
}

static std::string currentDateTimeString() {
    std::time_t t = std::time(nullptr);
    std::tm tmBuf{};
    getLocalTm(t, tmBuf);
    std::ostringstream oss;
    oss << (tmBuf.tm_year + 1900) << "/"
        << (tmBuf.tm_mon + 1) << "/"
        << tmBuf.tm_mday << " "
        << std::setw(2) << std::setfill('0') << tmBuf.tm_hour << ":"
        << std::setw(2) << std::setfill('0') << tmBuf.tm_min << ":"
        << std::setw(2) << std::setfill('0') << tmBuf.tm_sec;
    return oss.str();
}

std::string StorageManager::exportBackupJson(const AppState& state, bool pretty) {
    JsonValue root = JsonValue::makeObject();
    root["_app"] = "word-memory";
    root["_version"] = 1;
    root["_exportedAt"] = currentTimestampMs();
    root["_exportedAtStr"] = currentDateTimeString();

    JsonValue decksArr = JsonValue::makeArray();
    for (const auto& d : state.decks) {
        JsonValue deckObj = JsonValue::makeObject();
        deckObj["id"] = d.id;
        deckObj["name"] = d.name;

        JsonValue wordsArr = JsonValue::makeArray();
        for (const auto& w : d.words) {
            JsonValue wObj = JsonValue::makeObject();
            wObj["en"] = w.en;
            wObj["cn"] = w.cn;
            wObj["pos"] = w.pos;
            wObj["ex"] = w.ex;

            JsonValue stObj = JsonValue::makeObject();
            stObj["wrong"] = w.stats.wrong;
            stObj["fuzzy"] = w.stats.fuzzy;
            stObj["know"] = w.stats.know;
            stObj["level"] = w.stats.level;
            stObj["lastRev"] = w.stats.lastRev;
            stObj["lastType"] = w.stats.lastType;

            wObj["stats"] = stObj;
            wordsArr.push_back(wObj);
        }
        deckObj["words"] = wordsArr;
        decksArr.push_back(deckObj);
    }
    root["decks"] = decksArr;

    JsonValue checkinsObj = JsonValue::makeObject();
    for (const auto& kv : state.checkins) {
        if (kv.second) {
            checkinsObj[kv.first] = true;
        }
    }
    root["checkins"] = checkinsObj;

    JsonValue statsObj = JsonValue::makeObject();
    statsObj["quizCount"] = state.stats.quizCount;
    statsObj["perfectCount"] = state.stats.perfectCount;
    statsObj["consecutivePerfect"] = state.stats.consecutivePerfect;
    statsObj["morningQuiz"] = state.stats.morningQuiz;
    statsObj["nightQuiz"] = state.stats.nightQuiz;
    statsObj["noonQuiz"] = state.stats.noonQuiz;
    statsObj["lastAllKnow"] = state.stats.lastAllKnow;
    statsObj["firstUseDate"] = state.stats.firstUseDate > 0 ? state.stats.firstUseDate : currentTimestampMs();
    root["stats"] = statsObj;

    JsonValue badgesObj = JsonValue::makeObject();
    for (const auto& kv : state.badges) {
        badgesObj[kv.first] = kv.second;
    }
    root["badges"] = badgesObj;

    root["theme"] = state.theme;
    root["interaction"] = state.interaction;
    root["sortMode"] = state.sortMode;
    root["filterMode"] = state.filterMode;

    return root.dump(pretty ? 2 : -1);
}

AppState StorageManager::importBackupJson(const std::string& jsonStr) {
    JsonValue root = JsonValue::parse(jsonStr);
    if (!root.isObject()) {
        throw std::runtime_error("无效数据：根节点不是 JSON 对象");
    }
    const JsonValue& decksVal = root.get("decks");
    if (!decksVal.isArray()) {
        throw std::runtime_error("缺少单词数据（decks 字段无效）");
    }

    AppState state;
    state.stats.firstUseDate = currentTimestampMs();

    for (const auto& dVal : decksVal.asArray()) {
        if (!dVal.isObject()) continue;
        Deck d;
        d.id = dVal.get("id").asString();
        if (d.id.empty()) d.id = generateUid();
        d.name = dVal.get("name").asString("目录");

        const JsonValue& wordsVal = dVal.get("words");
        if (wordsVal.isArray()) {
            for (const auto& wVal : wordsVal.asArray()) {
                if (!wVal.isObject()) continue;
                Word w;
                w.en = wVal.get("en").asString();
                w.cn = wVal.get("cn").asString();
                if (w.en.empty() && w.cn.empty()) continue;

                // 同一目录下按大小写不敏感去重保护
                bool duplicate = std::any_of(d.words.begin(), d.words.end(),
                    [&w](const Word& existing) { return equalsIgnoreCaseAscii(existing.en, w.en); });
                if (duplicate) continue;

                w.pos = wVal.get("pos").asString();
                w.ex = wVal.get("ex").asString();
                w.deckId = d.id;
                w.deckName = d.name;

                const JsonValue& stVal = wVal.get("stats");
                if (stVal.isObject()) {
                    w.stats.wrong = stVal.get("wrong").asInt(0);
                    w.stats.fuzzy = stVal.get("fuzzy").asInt(0);
                    w.stats.know = stVal.get("know").asInt(0);
                    w.stats.level = stVal.get("level").asInt(0);
                    w.stats.lastRev = stVal.get("lastRev").asInt64(0);
                    w.stats.lastType = stVal.get("lastType").asString("");
                }
                d.words.push_back(std::move(w));
            }
        }
        state.decks.push_back(std::move(d));
    }

    const JsonValue& checkinsVal = root.get("checkins");
    if (checkinsVal.isObject()) {
        for (const auto& kv : checkinsVal.asObject()) {
            if (kv.second.asBool(false)) {
                state.checkins[kv.first] = true;
            }
        }
    }

    const JsonValue& statsVal = root.get("stats");
    if (statsVal.isObject()) {
        state.stats.quizCount = statsVal.get("quizCount").asInt(0);
        state.stats.perfectCount = statsVal.get("perfectCount").asInt(0);
        state.stats.consecutivePerfect = statsVal.get("consecutivePerfect").asInt(0);
        state.stats.morningQuiz = statsVal.get("morningQuiz").asInt(0);
        state.stats.nightQuiz = statsVal.get("nightQuiz").asInt(0);
        state.stats.noonQuiz = statsVal.get("noonQuiz").asInt(0);
        state.stats.lastAllKnow = statsVal.get("lastAllKnow").asBool(false);
        state.stats.firstUseDate = statsVal.get("firstUseDate").asInt64(currentTimestampMs());
    }

    const JsonValue& badgesVal = root.get("badges");
    if (badgesVal.isObject()) {
        for (const auto& kv : badgesVal.asObject()) {
            int64_t ts = kv.second.asInt64(0);
            if (ts > 0) {
                state.badges[kv.first] = ts;
            }
        }
    }

    std::string theme = root.get("theme").asString("tech");
    if (!theme.empty()) state.theme = theme;

    std::string interaction = root.get("interaction").asString("tap");
    if (interaction == "tap" || interaction == "swipe") state.interaction = interaction;

    std::string sortMode = root.get("sortMode").asString("default");
    if (!sortMode.empty()) state.sortMode = sortMode;

    std::string filterMode = root.get("filterMode").asString("all");
    if (!filterMode.empty()) state.filterMode = filterMode;

    return state;
}

AppState StorageManager::loadState() {
    std::filesystem::path fsPath = std::filesystem::u8path(m_filePath);
    std::ifstream ifs(fsPath, std::ios::in | std::ios::binary);
    if (ifs.is_open()) {
        std::ostringstream ss;
        ss << ifs.rdbuf();
        std::string content = ss.str();
        ifs.close();
        if (!content.empty()) {
            try {
                return importBackupJson(content);
            } catch (...) {
                // 解析失败时走下方默认初始化
            }
        }
    }

    AppState defaultState;
    defaultState.decks = createDefaultDecks();
    defaultState.stats.firstUseDate = currentTimestampMs();
    saveState(defaultState);
    return defaultState;
}

bool StorageManager::saveState(const AppState& state) {
    std::filesystem::path fsPath = std::filesystem::u8path(m_filePath);
    std::ofstream ofs(fsPath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) return false;
    std::string json = exportBackupJson(state, true);
    ofs.write(json.data(), static_cast<std::streamsize>(json.size()));
    ofs.close();
    return true;
}

} // namespace shanxuexi
