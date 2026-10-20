#include "AchievementEngine.h"
#include "CheckinManager.h"
#include <algorithm>
#include <map>

namespace shanxuexi {

static AchievementDef makeThresholdAch(const std::string& id, const std::string& group, int tier,
                                       const std::string& name, int target, const std::string& unit,
                                       std::function<int(const ComputedStats&)> getter) {
    return AchievementDef{
        id, group, tier, name,
        [getter, target](const ComputedStats& s) { return getter(s) >= target; },
        [getter, target, unit](const ComputedStats& s) {
            return AchievementProgress{std::min(getter(s), target), target, unit};
        }
    };
}

const std::vector<AchievementDef>& AchievementEngine::allDefinitions() {
    static const std::vector<AchievementDef> kDefs = {
        // 连续打卡组 (10 级)
        makeThresholdAch("s3",   "streak", 1,  "微光", 3,   "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s7",   "streak", 2,  "初雪", 7,   "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s14",  "streak", 3,  "燃灯", 14,  "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s21",  "streak", 4,  "拂晓", 21,  "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s30",  "streak", 5,  "满月", 30,  "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s60",  "streak", 6,  "聚星", 60,  "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s100", "streak", 7,  "百炼", 100, "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s180", "streak", 8,  "极境", 180, "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s365", "streak", 9,  "岁月", 365, "天", [](const ComputedStats& s){ return s.maxStreak; }),
        makeThresholdAch("s500", "streak", 10, "封神", 500, "天", [](const ComputedStats& s){ return s.maxStreak; }),

        // 测验次数组 (8 级)
        makeThresholdAch("q1",   "quiz", 1, "初启", 1,   "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q5",   "quiz", 2, "试剑", 5,   "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q10",  "quiz", 3, "渐悟", 10,  "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q25",  "quiz", 4, "精进", 25,  "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q50",  "quiz", 5, "破境", 50,  "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q100", "quiz", 6, "融会", 100, "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q200", "quiz", 7, "造极", 200, "次", [](const ComputedStats& s){ return s.quizCount; }),
        makeThresholdAch("q500", "quiz", 8, "宗师", 500, "次", [](const ComputedStats& s){ return s.quizCount; }),

        // 词汇积累组 (10 级)
        makeThresholdAch("w10",    "words", 1,  "初窥", 10,    "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w50",    "words", 2,  "积微", 50,    "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w100",   "words", 3,  "百词", 100,   "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w300",   "words", 4,  "浩瀚", 300,   "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w500",   "words", 5,  "博识", 500,   "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w1000",  "words", 6,  "千词", 1000,  "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w2000",  "words", 7,  "万里", 2000,  "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w5000",  "words", 8,  "大儒", 5000,  "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w8000",  "words", 9,  "学者", 8000,  "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("w10000", "words", 10, "词圣", 10000, "词", [](const ComputedStats& s){ return s.wordsAdded; }),

        // 满分测验组 (8 级)
        makeThresholdAch("p1",   "perfect", 1, "初捷", 1,   "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p3",   "perfect", 2, "三连", 3,   "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p5",   "perfect", 3, "五福", 5,   "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p10",  "perfect", 4, "十全", 10,  "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p25",  "perfect", 5, "无双", 25,  "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p50",  "perfect", 6, "辉煌", 50,  "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p100", "perfect", 7, "全胜", 100, "次", [](const ComputedStats& s){ return s.perfectCount; }),
        makeThresholdAch("p200", "perfect", 8, "常胜", 200, "次", [](const ComputedStats& s){ return s.perfectCount; }),

        // 时段与特殊成就组
        makeThresholdAch("tm",    "tm",    1, "晨光", 1,   "次", [](const ComputedStats& s){ return s.morningQuiz; }),
        makeThresholdAch("tn",    "tn",    1, "夜行", 1,   "次", [](const ComputedStats& s){ return s.nightQuiz; }),
        makeThresholdAch("tnn",   "tnn",   1, "午后", 1,   "次", [](const ComputedStats& s){ return s.noonQuiz; }),
        makeThresholdAch("d1",    "d1",    1, "开卷", 1,   "个", [](const ComputedStats& s){ return s.decksCreated; }),
        makeThresholdAch("d3",    "d3",    2, "立册", 3,   "个", [](const ComputedStats& s){ return s.decksCreated; }),
        makeThresholdAch("d5",    "d5",    3, "藏经", 5,   "个", [](const ComputedStats& s){ return s.decksCreated; }),
        makeThresholdAch("wa1",   "wa1",   1, "启程", 1,   "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("wa50",  "wa50",  2, "拾贝", 50,  "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("wa200", "wa200", 3, "藏珍", 200, "词", [](const ComputedStats& s){ return s.wordsAdded; }),
        makeThresholdAch("fz20",  "fz20",  1, "迷津", 20,  "次", [](const ComputedStats& s){ return s.fuzzyCount; }),
        AchievementDef{
            "zall", "zall", 2, "澄明",
            [](const ComputedStats& s){ return s.lastAllKnow; },
            [](const ComputedStats& s){ return AchievementProgress{s.lastAllKnow ? 1 : 0, 1, "次"}; }
        },
        makeThresholdAch("cp2",   "cp2",   3, "贯通", 2,   "次", [](const ComputedStats& s){ return s.consecutivePerfect; }),

        // 陪伴使用天数组 (6 级)
        makeThresholdAch("u7",   "usage", 1, "初心", 7,   "天", [](const ComputedStats& s){ return s.daysUsed; }),
        makeThresholdAch("u30",  "usage", 2, "同行", 30,  "天", [](const ComputedStats& s){ return s.daysUsed; }),
        makeThresholdAch("u90",  "usage", 3, "季度", 90,  "天", [](const ComputedStats& s){ return s.daysUsed; }),
        makeThresholdAch("u180", "usage", 4, "挚友", 180, "天", [](const ComputedStats& s){ return s.daysUsed; }),
        makeThresholdAch("u365", "usage", 5, "伙伴", 365, "天", [](const ComputedStats& s){ return s.daysUsed; }),
        makeThresholdAch("u500", "usage", 6, "传奇", 500, "天", [](const ComputedStats& s){ return s.daysUsed; })
    };
    return kDefs;
}

using GroupNameTable = std::map<std::string, std::vector<std::string>>;

static const std::map<std::string, GroupNameTable>& themeNameTables() {
    static const std::map<std::string, GroupNameTable> kTables = {
        {"tech", {
            {"streak",  {"点星","启航","巡航","跃迁","领航","穿越星门","征服星域","主宰星海","星海传奇","宇宙传说"}},
            {"quiz",    {"信标","航线","探测","突围","星图","引力弹弓","虫洞","归航"}},
            {"words",   {"尘埃","陨石","星核","星云","星座","星河","星系","星团","星海","宇宙"}},
            {"perfect", {"校准","同步","锁定","精准","完美轨道","双星共鸣","星门全开","永恒星辰"}},
            {"tm", {"晨星初升"}}, {"tn", {"暗夜航行"}}, {"tnn", {"正午恒星"}},
            {"d1", {"信标塔"}}, {"d3", {"空间站"}}, {"d5", {"星际港"}},
            {"wa1", {"首航"}}, {"wa50", {"星尘采集"}}, {"wa200", {"星藏发掘"}},
            {"fz20", {"星雾"}}, {"zall", {"星图明朗"}}, {"cp2", {"星光通道"}},
            {"usage",   {"舱内","出舱","巡航","深空","星际","永恒"}}
        }},
        {"midnight", {
            {"streak",  {"霓虹","暗巷","夜巡","潜行","暗影","夜枭","无光者","深渊行者","长夜之主","永夜君王"}},
            {"quiz",    {"接入","破解","防火墙","数据流","神经链接","深层潜入","系统核心","完全入侵"}},
            {"words",   {"像素","数据包","芯片","主板","服务器","数据库","云端","神经网络","量子核心","意识上传"}},
            {"perfect", {"连接","同步","超频","过载","完美协议","双重加密","系统全开","绝对权限"}},
            {"tm", {"晨曦信号"}}, {"tn", {"午夜协议"}}, {"tnn", {"正午脉冲"}},
            {"d1", {"安全屋"}}, {"d3", {"地下基地"}}, {"d5", {"霓虹要塞"}},
            {"wa1", {"初次接入"}}, {"wa50", {"数据五十"}}, {"wa200", {"数据两百"}},
            {"fz20", {"信号干扰"}}, {"zall", {"信号清明"}}, {"cp2", {"全频畅通"}},
            {"usage",   {"上线","常驻","骨干","元老","传说","神话"}}
        }},
        {"sakura", {
            {"streak",  {"初遇","心动","暗恋","告白","交往","热恋","心意相通","灵魂伴侣","命中注定","三生三世"}},
            {"quiz",    {"脸红","牵手","约会","表白","承诺","戒指","婚礼","白首"}},
            {"words",   {"心语","情书","花语","星愿","恋歌","诗篇","誓言","回忆录","时光胶囊","永恒之约"}},
            {"perfect", {"微笑","心动瞬间","双人舞","完美约会","双心共鸣","浪漫满屋","天生一对","永结同心"}},
            {"tm", {"晨间问候"}}, {"tn", {"深夜絮语"}}, {"tnn", {"午后阳光"}},
            {"d1", {"日记本"}}, {"d3", {"相册集"}}, {"d5", {"时光信箱"}},
            {"wa1", {"初次心动"}}, {"wa50", {"情话五十"}}, {"wa200", {"情诗两百"}},
            {"fz20", {"心绪迷乱"}}, {"zall", {"心意相通"}}, {"cp2", {"双人同心"}},
            {"usage",   {"初识","相知","相伴","相守","白头","永恒"}}
        }},
        {"forest", {
            {"streak",  {"萌芽","抽枝","展叶","成树","开花","结果","参天","灵木","世界树","森之起源"}},
            {"quiz",    {"探路","识途","追踪","狩猎","通灵","唤风","驭兽","森之意志"}},
            {"words",   {"种子","露珠","叶片","藤蔓","树根","年轮","花冠","果核","林海","生命之树"}},
            {"perfect", {"新绿","双叶","花蕾","繁花","双生树","灵木共鸣","万木朝宗","永恒之森"}},
            {"tm", {"晨露"}}, {"tn", {"夜萤"}}, {"tnn", {"午荫"}},
            {"d1", {"苗圃"}}, {"d3", {"林地"}}, {"d5", {"古老森林"}},
            {"wa1", {"播下种子"}}, {"wa50", {"半百林木"}}, {"wa200", {"两百灵植"}},
            {"fz20", {"迷雾林"}}, {"zall", {"拨云见日"}}, {"cp2", {"双木成林"}},
            {"usage",   {"幼芽","幼苗","小树","大树","古树","森之古神"}}
        }},
        {"cream", {
            {"streak",  {"面粉","面团","烘焙","出炉","裱花","装饰","招牌","名店","甜品大师","甜品之神"}},
            {"quiz",    {"试味","调味","配方","火候","秘方","招牌菜","米其林","甜品宗师"}},
            {"words",   {"糖粒","奶油","巧克力","果酱","蛋糕胚","慕斯","马卡龙","千层","甜品塔","梦幻甜点"}},
            {"perfect", {"微甜","香甜","双倍甜","蜜糖","完美配方","双料甜心","甜度爆表","甜梦永恒"}},
            {"tm", {"晨间甜点"}}, {"tn", {"深夜甜品"}}, {"tnn", {"午后甜茶"}},
            {"d1", {"小烤箱"}}, {"d3", {"甜品店"}}, {"d5", {"甜品王国"}},
            {"wa1", {"第一勺"}}, {"wa50", {"半百甜点"}}, {"wa200", {"两百甜心"}},
            {"fz20", {"甜度迷乱"}}, {"zall", {"甜味清晰"}}, {"cp2", {"双重甜心"}},
            {"usage",   {"学徒","帮厨","甜点师","主厨","大师","传奇甜品师"}}
        }},
        {"ocean", {
            {"streak",  {"涟漪","浅滩","近海","深潜","海沟","海底火山","深海秘境","海渊","海神领域","深海之王"}},
            {"quiz",    {"起锚","扬帆","破浪","远航","风暴","海图","幽灵船","海神三叉戟"}},
            {"words",   {"水滴","浪花","贝壳","珊瑚","海星","珍珠","沉船","海沟","深海巨兽","海洋之心"}},
            {"perfect", {"微波","双浪","潮汐","满潮","双潮共鸣","海啸","海神祝福","永恒之海"}},
            {"tm", {"晨光海面"}}, {"tn", {"夜海荧光"}}, {"tnn", {"正午碧波"}},
            {"d1", {"小码头"}}, {"d3", {"航海港"}}, {"d5", {"深海基地"}},
            {"wa1", {"第一滴"}}, {"wa50", {"半百浪花"}}, {"wa200", {"两百珍珠"}},
            {"fz20", {"迷雾海"}}, {"zall", {"风平浪静"}}, {"cp2", {"双潮共振"}},
            {"usage",   {"泅水","泛舟","航海","远洋","深海","海之传说"}}
        }},
        {"cyber", {
            {"streak",  {"义肢","改造","强化","义体化","神经接入","意识上传","机械飞升","超脑","数字永生","机械之神"}},
            {"quiz",    {"启动","校准","编译","超频","越权","内核入侵","系统吞噬","神级权限"}},
            {"words",   {"零件","模块","芯片","电路","主板","处理器","量子核","神经网络","机甲核心","机械宇宙"}},
            {"perfect", {"通电","双机同步","完美校准","过载","双核共鸣","机械飞升","全系统掌控","机械永恒"}},
            {"tm", {"晨间启动"}}, {"tn", {"夜间运行"}}, {"tnn", {"午间充能"}},
            {"d1", {"工作台"}}, {"d3", {"改造室"}}, {"d5", {"机械堡垒"}},
            {"wa1", {"首个零件"}}, {"wa50", {"半百模块"}}, {"wa200", {"两百核心"}},
            {"fz20", {"系统紊乱"}}, {"zall", {"系统清明"}}, {"cp2", {"双核同步"}},
            {"usage",   {"初装","半械","全械","义体","机械生命","机械神明"}}
        }},
        {"mono", {
            {"streak",  {"落笔","描红","临帖","成字","行书","草书","狂草","自成一家","书圣","墨道飞升"}},
            {"quiz",    {"磨墨","研墨","运笔","挥毫","泼墨","点睛","画龙","墨韵天成"}},
            {"words",   {"一点","一横","一竖","一撇","一捺","一字","一句","一篇","一卷","墨海书山"}},
            {"perfect", {"落墨","双钩","三折","藏锋","双笔合璧","力透纸背","入木三分","墨染千秋"}},
            {"tm", {"晨起研墨"}}, {"tn", {"夜半提笔"}}, {"tnn", {"午后临帖"}},
            {"d1", {"笔架"}}, {"d3", {"砚台"}}, {"d5", {"藏书阁"}},
            {"wa1", {"第一笔"}}, {"wa50", {"半百墨字"}}, {"wa200", {"两百书卷"}},
            {"fz20", {"墨色朦胧"}}, {"zall", {"墨色分明"}}, {"cp2", {"双笔生辉"}},
            {"usage",   {"习字","学书","成家","宗师","大家","墨圣"}}
        }},
        {"sunset", {
            {"streak",  {"引气","炼气","筑基","金丹","元婴","化神","炼虚","合体","大乘","渡劫飞升"}},
            {"quiz",    {"入门","小成","精通","大成","圆满","通玄","证道","飞升"}},
            {"words",   {"灵石","灵草","丹药","法宝","灵器","仙器","灵宝","仙宝","神器","混沌至宝"}},
            {"perfect", {"一气","两仪","三才","四象","五行圆满","六合归一","七星耀世","八荒独尊"}},
            {"tm", {"朝霞采气"}}, {"tn", {"月华炼神"}}, {"tnn", {"正阳炼丹"}},
            {"d1", {"洞府"}}, {"d3", {"灵脉"}}, {"d5", {"洞天福地"}},
            {"wa1", {"第一灵石"}}, {"wa50", {"半百灵草"}}, {"wa200", {"两百丹药"}},
            {"fz20", {"心魔缠身"}}, {"zall", {"道心澄明"}}, {"cp2", {"阴阳调和"}},
            {"usage",   {"炼气","筑基","金丹","元婴","化神","大乘"}}
        }}
    };
    return kTables;
}

std::string AchievementEngine::getAchievementName(const AchievementDef& def, const std::string& themeId) {
    const auto& tables = themeNameTables();
    auto tIt = tables.find(themeId);
    if (tIt != tables.end()) {
        auto gIt = tIt->second.find(def.group);
        if (gIt != tIt->second.end()) {
            const auto& vec = gIt->second;
            int idx = def.tier - 1;
            if (idx >= 0 && idx < static_cast<int>(vec.size())) {
                return vec[idx];
            }
            if (!vec.empty()) {
                return vec[0];
            }
        }
    }
    return def.defaultName;
}

std::vector<std::string> AchievementEngine::checkAndUnlock(AppState& state, const ComputedStats& stats) {
    std::vector<std::string> newlyUnlocked;
    int64_t nowMs = currentTimestampMs();
    for (const auto& def : allDefinitions()) {
        auto it = state.badges.find(def.id);
        bool alreadyUnlocked = (it != state.badges.end() && it->second > 0);
        if (!alreadyUnlocked && def.checkFn(stats)) {
            state.badges[def.id] = nowMs;
            newlyUnlocked.push_back(getAchievementName(def, state.theme));
        }
    }
    return newlyUnlocked;
}

bool AchievementEngine::toggleAllBadgesEasterEgg(AppState& state) {
    const auto& defs = allDefinitions();
    size_t validUnlockedCount = 0;
    for (const auto& d : defs) {
        auto it = state.badges.find(d.id);
        if (it != state.badges.end() && it->second > 0) {
            validUnlockedCount++;
        }
    }

    if (validUnlockedCount >= defs.size()) {
        state.badges.clear();
        return false;
    } else {
        int64_t nowMs = currentTimestampMs();
        for (const auto& d : defs) {
            state.badges[d.id] = nowMs;
        }
        return true;
    }
}

static void calcBadgeHsl(const std::string& theme, const std::string& group, int tier,
                         int indexInGroup, int& outH, int& outS, int& outL) {
    struct ThemeHues { int streak, quiz, words, perfect, usage; };
    static const std::map<std::string, ThemeHues> kThemeHues = {
        {"tech",     {215, 245, 190, 35,  165}},
        {"midnight", {185, 300, 55,  155, 215}},
        {"sakura",   {335, 275, 0,   40,  355}},
        {"forest",   {145, 115, 90,  45,  170}},
        {"cream",    {35,  15,  50,  20,  55}},
        {"ocean",    {205, 185, 225, 45,  245}},
        {"cyber",    {305, 270, 55,  185, 335}},
        {"mono",     {220, 220, 220, 220, 220}},
        {"sunset",   {25,  340, 45,  15,  5}}
    };
    auto it = kThemeHues.find(theme);
    const ThemeHues& hMap = (it != kThemeHues.end()) ? it->second : kThemeHues.at("tech");

    int baseH = hMap.streak;
    if (group == "quiz") baseH = hMap.quiz;
    else if (group == "words") baseH = hMap.words;
    else if (group == "perfect") baseH = hMap.perfect;
    else if (group == "usage") baseH = hMap.usage;

    int hue = baseH + (indexInGroup - 4) * 4;
    while (hue < 0) hue += 360;
    while (hue >= 360) hue -= 360;

    static const int kSat[]   = {60, 60, 68, 74, 82, 88, 95};
    static const int kLight[] = {60, 60, 56, 52, 48, 44, 40};
    int clampedTier = std::max(1, std::min(tier, 6));

    outH = hue;
    outS = kSat[clampedTier];
    outL = kLight[clampedTier];
}

std::vector<BadgeViewItem> AchievementEngine::buildBadgeViews(const AppState& state, const ComputedStats& stats) {
    const auto& defs = allDefinitions();
    std::vector<BadgeViewItem> out;
    out.reserve(defs.size());

    std::map<std::string, int> groupCounters;
    for (const auto& def : defs) {
        int idxInGroup = groupCounters[def.group]++;
        BadgeViewItem item;
        item.id = def.id;
        item.group = def.group;
        item.tier = def.tier;
        item.name = getAchievementName(def, state.theme);

        auto uIt = state.badges.find(def.id);
        if (uIt != state.badges.end() && uIt->second > 0) {
            item.unlocked = true;
            item.dateText = CheckinManager::formatUnlockDate(uIt->second) + " 解锁";
        } else {
            item.unlocked = false;
            item.dateText = "待解锁";
        }

        AchievementProgress prog = def.progressFn(stats);
        item.progressText = std::to_string(prog.cur) + "/" + std::to_string(prog.total) + " " + prog.unit;
        item.progressRatio = prog.total > 0
            ? std::min(1.0, static_cast<double>(prog.cur) / static_cast<double>(prog.total))
            : 0.0;

        calcBadgeHsl(state.theme, def.group, def.tier, idxInGroup, item.hue, item.sat, item.light);
        out.push_back(std::move(item));
    }
    return out;
}

} // namespace shanxuexi
