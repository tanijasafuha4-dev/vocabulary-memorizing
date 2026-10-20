# 闪学习 v1.0 (C++17 / Qt for Android 跨平台重制版)

本项目是原版「闪学习 v1.0」的 **C++17 + Qt Quick (QML)** 完整重构工程。所有核心业务逻辑（词库管理、艾宾浩斯遗忘曲线调度、Efraimidis-Spirakis 加权不放回随机抽样、打卡日历计算、54 项九大主题世界观成就系统、JSON 持久化与备份恢复）均使用标准现代 C++17 编写，支持同时编译为 **桌面端应用 (Windows / macOS / Linux)** 与 **移动端应用 (Android APK)**。

---

## 一、项目目录结构

```text
ShanXueXi_Qt/
├── CMakeLists.txt                  # CMake 构建配置（支持 Qt5 / Qt6 及纯 C++ 单元测试）
├── ShanXueXi.pro                   # QMake 构建配置（兼容传统 Qt Creator 工程）
├── resources.qrc                   # Qt 资源文件清单
├── android/
│   └── AndroidManifest.xml         # Qt for Android 移动端打包配置与权限声明
├── src/
│   ├── main.cpp                    # Qt 应用启动入口与 QML 上下文注册
│   ├── core/                       # 【纯标准 C++17 核心业务层（零第三方依赖）】
│   │   ├── Models.h                # Word, Deck, WordStats, AppState 等核心数据结构
│   │   ├── JsonHelper.h            # 零依赖 C++17 递归下降 JSON 解析与序列化器
│   │   ├── StorageManager.h/.cpp   # 本地文件持久化与原版兼容的 JSON 备份导出/导入
│   │   ├── EbbinghausEngine.h/.cpp # 艾宾浩斯遗忘曲线调度、智能复习池排序、筛选与排序
│   │   ├── QuizEngine.h/.cpp       # 加权随机抽样算法与测验会话状态机
│   │   ├── CheckinManager.h/.cpp   # 公历日期演算、连续打卡天数、月度日历网格生成
│   │   └── AchievementEngine.h/.cpp# 54 项成就判定与 9 大主题世界观动态称号映射
│   └── bridge/                     # 【Qt / QML 桥接控制层】
│       └── AppController.h/.cpp    # Q_OBJECT 控制器，暴露属性与 Q_INVOKABLE 接口
├── qml/                            # 【移动端声明式 UI 层】
│   ├── Main.qml                    # 主界面、滑动闪卡、打卡日历、成就墙、备份弹窗等
│   └── ThemeData.js                # 9 大主题配色参数表
└── tests/
    └── core_test.cpp               # 独立纯 C++ 核心逻辑单元测试（无需 Qt 即可编译运行）
```

---

## 二、如何编译与运行

### 方式 1：使用 Qt Creator 编译桌面端与 Android 移动端（推荐）

1. 打开 **Qt Creator**，点击 `文件 -> 打开文件或项目`，选择 `ShanXueXi_Qt/CMakeLists.txt`（或 `ShanXueXi.pro`）。
2. **桌面端运行（用于日常调试与课堂电脑演示）**：
   * 勾选 **Desktop Qt (MinGW / MSVC)** 构建套件（Kit）。
   * 点击左下角绿色运行按钮即可直接在电脑上启动移动端比例窗口。
3. **Android 移动端打包（生成 `.apk` 安装到手机）**：
   * 在 Qt Creator 的 `编辑 -> Preferences -> 设备 -> Android` 中配置好 JDK 17 与 Android SDK / NDK（可直接复用 Android Studio 已安装的 SDK 目录）。
   * 在项目构建套件中启用 **Android Qt (arm64-v8a)**。
   * 点击左下角构建按钮，生成的 APK 位于构建目录下的 `android-build/build/outputs/apk/debug/` 中，连接手机即可一键安装运行。

### 方式 2：仅使用任意 C++17 编译器运行核心单元测试（无需安装 Qt）

如果需要在纯 C++ 环境下快速验证核心算法与数据结构，可直接用 `g++` / `clang++` / `cl` 编译 `tests/core_test.cpp`：

```bash
g++ -std=c++17 -Isrc tests/core_test.cpp \
    src/core/EbbinghausEngine.cpp \
    src/core/QuizEngine.cpp \
    src/core/CheckinManager.cpp \
    src/core/AchievementEngine.cpp \
    src/core/StorageManager.cpp \
    -o core_test
./core_test
```

---

## 三、C++ 期末大作业答辩核心亮点

1. **严格的架构分层（Core 与 UI 解耦）**：
   * `src/core/` 下全部采用纯标准 C++17 编写，不依赖任何平台特定库，体现了良好的高内聚低耦合面向对象设计。
2. **手写零依赖 JSON 引擎 (`JsonHelper.h`)**：
   * 采用递归下降法完整实现了 JSON 词法与语法解析及格式化输出，100% 兼容原版 Web 导出的备份文件。
3. **经典算法与 STL 深度应用**：
   * **艾宾浩斯智能复习队列**：利用 `std::stable_sort` 多级优先级比较器（错词 > 模糊词 > 低掌握度 > 早复习时间）实现复习调度。
   * **Efraimidis-Spirakis 加权抽样**：使用 `std::mt19937_64` 与指数权重公式 $k = u^{1/w}$ 实现错词高概率抽测。
   * **Howard Hinnant 历法转换**：纯算法实现公历日期与儒略日转换，精确计算跨月/跨年/闰年的连续打卡天数（Streak）。
   * **函数式与多态映射**：使用 `std::function` 与 Lambda 表达式构建 54 项成就规则引擎。
