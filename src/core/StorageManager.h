#ifndef SHANXUEXI_STORAGEMANAGER_H
#define SHANXUEXI_STORAGEMANAGER_H

#include "Models.h"
#include "JsonHelper.h"
#include <string>

namespace shanxuexi {

/**
 * @brief 数据持久化与备份恢复管理器
 * 负责将 AppState 保存至本地 JSON 文件，以及导出/导入兼容原版 Web 端的备份格式
 */
class StorageManager {
public:
    explicit StorageManager(std::string storageFilePath = "shanxuexi_data.json");

    void setStorageFilePath(const std::string& path);
    const std::string& storageFilePath() const;

    /**
     * @brief 生成默认初始词库（Text 1，包含 abandon 等 5 个初始核心词汇）
     */
    static std::vector<Deck> createDefaultDecks();

    /**
     * @brief 从本地存档文件加载 AppState；若不存在则初始化默认数据并保存
     */
    AppState loadState();

    /**
     * @brief 将当前 AppState 持久化保存到本地文件
     */
    bool saveState(const AppState& state);

    /**
     * @brief 构建完整的备份 JSON 字符串（100% 兼容原版 word-memory 备份格式）
     */
    static std::string exportBackupJson(const AppState& state, bool pretty = true);

    /**
     * @brief 从 JSON 字符串解析并恢复 AppState（支持原版导出的 JSON 备份）
     * @throws std::runtime_error 当数据格式非法或缺少 decks 时抛出异常
     */
    static AppState importBackupJson(const std::string& jsonStr);

private:
    std::string m_filePath;
};

} // namespace shanxuexi

#endif // SHANXUEXI_STORAGEMANAGER_H
