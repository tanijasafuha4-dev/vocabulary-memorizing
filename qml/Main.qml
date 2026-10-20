import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import "ThemeData.js" as ThemeData

Window {
    id: root
    width: 430
    height: 820
    visible: true
    title: qsTr("闪学习 - 单词记忆 (C++/Qt版)")

    property var th: ThemeData.getTheme(appController.currentTheme)
    property string currentTab: "accum"        // "accum" | "quiz"
    property bool inWordsView: false           // false: 目录列表, true: 单词列表
    property string activeSheet: ""            // "" | "word" | "newDeck" | "deckMenu" | "theme" | "interaction" | "calendar" | "ach" | "sort" | "backup"

    // 测验设置状态
    property string quizRangeId: "__all__"
    property int quizCountSetting: 10
    property bool answerRevealed: false

    // 添加单词弹窗状态
    property string pendingDeckId: ""

    // 日历弹窗年月状态
    property int calYear: new Date().getFullYear()
    property int calMonth: new Date().getMonth() + 1
    property var calendarData: ({ title: "", streak: 0, monthCount: 0, totalCount: 0, cells: [] })

    // 全局搜索状态
    property string globalSearchText: ""
    property var globalSearchResults: []

    function refreshCalendar() {
        calendarData = appController.getCalendarView(calYear, calMonth)
    }

    Connections {
        target: appController
        function onThemeChanged() {
            root.th = ThemeData.getTheme(appController.currentTheme)
        }
        function onToastMessage(message) {
            toastBanner.showToast(message)
        }
        function onQuizChanged() {
            root.answerRevealed = false
        }
        function onStateChanged() {
            if (root.activeSheet === "calendar") {
                root.refreshCalendar()
            }
            if (root.globalSearchText.length > 0) {
                root.globalSearchResults = appController.searchAllWords(root.globalSearchText)
            }
        }
    }

    // 背景层
    Rectangle {
        anchors.fill: parent
        color: root.th.bgBase
        Behavior on color { ColorAnimation { duration: 250 } }
    }

    // 主容器（支持移动端全屏与桌面端居中限宽）
    Item {
        id: mainContainer
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, 460)
        focus: true

        // Android 物理返回键处理
        Keys.onReleased: function(event) {
            if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                if (root.activeSheet !== "") {
                    root.activeSheet = ""
                    event.accepted = true
                } else if (root.currentTab === "accum" && root.inWordsView) {
                    root.inWordsView = false
                    event.accepted = true
                } else if (root.currentTab === "quiz" && (appController.quizActive || appController.quizFinished)) {
                    appController.abortQuiz()
                    event.accepted = true
                } else if (root.currentTab === "quiz") {
                    root.currentTab = "accum"
                    event.accepted = true
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // ==================== 页面 1：单词积累 ====================
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: root.currentTab === "accum"

                // ---------- 子视图 1A：目录列表 ----------
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    visible: !root.inWordsView

                    // 顶部栏
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        Layout.bottomMargin: 8
                        spacing: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2
                            Text {
                                text: "单词积累"
                                font.pixelSize: 22
                                font.bold: true
                                color: root.th.primaryD
                            }
                            Text {
                                text: "共 " + appController.totalWordCount + " 个单词 · " + appController.totalDeckCount + " 个目录"
                                font.pixelSize: 12
                                color: root.th.muted
                            }
                        }

                        // 打卡日历按钮
                        Rectangle {
                            width: 36; height: 36; radius: 12
                            color: root.th.card
                            border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "📅"; font.pixelSize: 16 }
                            Rectangle {
                                visible: !appController.checkedInToday
                                width: 15; height: 15; radius: 8
                                anchors.right: parent.right; anchors.top: parent.top
                                anchors.rightMargin: -3; anchors.topMargin: -3
                                color: "#f97316"
                                Text { anchors.centerIn: parent; text: "!"; color: "#fff"; font.pixelSize: 10; font.bold: true }
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    var d = new Date()
                                    root.calYear = d.getFullYear()
                                    root.calMonth = d.getMonth() + 1
                                    root.refreshCalendar()
                                    root.activeSheet = "calendar"
                                }
                            }
                        }

                        // 成就按钮
                        Rectangle {
                            width: 36; height: 36; radius: 12
                            color: root.th.card
                            border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "🏆"; font.pixelSize: 16 }
                            MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "ach" }
                        }

                        // 备份按钮
                        Rectangle {
                            width: 36; height: 36; radius: 12
                            color: root.th.card
                            border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "☁️"; font.pixelSize: 16 }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    importTextArea.text = ""
                                    root.activeSheet = "backup"
                                }
                            }
                        }

                        // 主题按钮
                        Rectangle {
                            width: 36; height: 36; radius: 12
                            color: root.th.card
                            border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "🎨"; font.pixelSize: 16 }
                            MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "theme" }
                        }

                        // 新建目录按钮
                        Rectangle {
                            width: 36; height: 36; radius: 12
                            color: root.th.card
                            border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "＋"; font.pixelSize: 18; font.bold: true; color: root.th.primary }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    newDeckInput.text = ""
                                    root.activeSheet = "newDeck"
                                }
                            }
                        }
                    }

                    // 全局搜索框
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.bottomMargin: 10
                        height: 42
                        radius: 14
                        color: root.th.card
                        border.color: globalSearchInput.activeFocus ? root.th.primary : root.th.border

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8
                            Text { text: "🔍"; font.pixelSize: 14; color: root.th.faint }
                            TextInput {
                                id: globalSearchInput
                                Layout.fillWidth: true
                                font.pixelSize: 14
                                color: root.th.text
                                clip: true
                                onTextChanged: {
                                    root.globalSearchText = text.trim()
                                    if (root.globalSearchText.length > 0) {
                                        root.globalSearchResults = appController.searchAllWords(root.globalSearchText)
                                    } else {
                                        root.globalSearchResults = []
                                    }
                                }
                                Text {
                                    anchors.fill: parent
                                    text: "搜索所有单词…"
                                    color: root.th.faint
                                    font.pixelSize: 14
                                    visible: !globalSearchInput.text
                                }
                            }
                            Text {
                                visible: globalSearchInput.text.length > 0
                                text: "✕"
                                color: root.th.muted
                                font.pixelSize: 14
                                MouseArea { anchors.fill: parent; onClicked: globalSearchInput.text = "" }
                            }
                        }
                    }

                    // 搜索结果列表（当有全局搜索词时显示）
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        visible: root.globalSearchText.length > 0
                        spacing: 8

                        Text {
                            text: "找到 " + root.globalSearchResults.length + " 个匹配结果"
                            font.pixelSize: 12
                            color: root.th.muted
                        }

                        ListView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            spacing: 8
                            model: root.globalSearchResults
                            delegate: WordCardDelegate {
                                width: ListView.view.width
                                wordData: modelData
                                showDeckBadge: true
                                showDeleteBtn: false
                            }
                        }
                    }

                    // 目录列表区域（未搜索时显示）
                    Flickable {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        visible: root.globalSearchText.length === 0
                        contentHeight: deckListCol.height + 24
                        clip: true

                        ColumnLayout {
                            id: deckListCol
                            width: parent.width - 32
                            x: 16
                            spacing: 8

                            Text {
                                text: "目录"
                                font.pixelSize: 11
                                font.bold: true
                                color: root.th.faint
                            }

                            // 全部单词置顶卡片
                            Rectangle {
                                Layout.fillWidth: true
                                height: 68
                                radius: 18
                                color: root.th.card
                                border.color: root.th.borderHi
                                border.width: 1.5

                                Rectangle {
                                    width: 4
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                    radius: 2
                                    color: root.th.primary
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 16
                                    anchors.rightMargin: 14
                                    spacing: 12

                                    Rectangle {
                                        width: 40; height: 40; radius: 12
                                        color: root.th.cardSub
                                        border.color: root.th.borderHi
                                        Text { anchors.centerIn: parent; text: "📚"; font.pixelSize: 18 }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 3
                                        Text {
                                            text: "全部单词"
                                            font.pixelSize: 16
                                            font.bold: true
                                            color: root.th.primaryD
                                        }
                                        Text {
                                            text: appController.totalWordCount + " 个单词 · " + appController.totalDeckCount + " 个目录"
                                            font.pixelSize: 12
                                            color: root.th.muted
                                        }
                                    }

                                    Text { text: "›"; font.pixelSize: 22; color: root.th.faint }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        deckSearchInput.text = ""
                                        appController.openDeck("__all__")
                                        root.inWordsView = true
                                    }
                                }
                            }

                            // 各个目录卡片
                            Repeater {
                                model: appController.decksModel
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    height: 68
                                    radius: 18
                                    color: root.th.card
                                    border.color: root.th.border

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 14
                                        anchors.rightMargin: 14
                                        spacing: 12

                                        Rectangle {
                                            width: 40; height: 40; radius: 12
                                            color: root.th.cardSub
                                            border.color: root.th.border
                                            Text { anchors.centerIn: parent; text: "📖"; font.pixelSize: 18 }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 3
                                            Text {
                                                text: modelData.name
                                                font.pixelSize: 15
                                                font.bold: true
                                                color: root.th.text
                                                elide: Text.ElideRight
                                                Layout.fillWidth: true
                                            }
                                            Text {
                                                text: modelData.wordCount + " 个单词"
                                                font.pixelSize: 12
                                                color: root.th.muted
                                            }
                                        }

                                        Text { text: "›"; font.pixelSize: 22; color: root.th.faint }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            deckSearchInput.text = ""
                                            appController.openDeck(modelData.id)
                                            root.inWordsView = true
                                        }
                                    }
                                }
                            }

                            // 新建目录虚线按钮
                            Rectangle {
                                Layout.fillWidth: true
                                height: 48
                                radius: 18
                                color: root.th.cardSub
                                border.color: root.th.primary
                                border.width: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: "＋ 新建目录"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: root.th.primary
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        newDeckInput.text = ""
                                        root.activeSheet = "newDeck"
                                    }
                                }
                            }
                        }
                    }
                }

                // ---------- 子视图 1B：目录内单词列表 ----------
                Item {
                    anchors.fill: parent
                    visible: root.inWordsView

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // 顶部导航
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.margins: 16
                            Layout.bottomMargin: 8
                            spacing: 10

                            Rectangle {
                                width: 36; height: 36; radius: 12
                                color: root.th.card
                                border.color: root.th.border
                                Text { anchors.centerIn: parent; text: "‹"; font.pixelSize: 22; font.bold: true; color: root.th.text2 }
                                MouseArea { anchors.fill: parent; onClicked: root.inWordsView = false }
                            }

                            Text {
                                Layout.fillWidth: true
                                text: appController.currentDeckTitle
                                font.pixelSize: 18
                                font.bold: true
                                color: root.th.text
                                elide: Text.ElideRight
                            }

                            // 排序与筛选按钮
                            Rectangle {
                                width: 36; height: 36; radius: 12
                                color: root.th.card
                                border.color: root.th.border
                                Text { anchors.centerIn: parent; text: "⇅"; font.pixelSize: 16; font.bold: true; color: root.th.text2 }
                                MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "sort" }
                            }

                            // 目录操作菜单（仅具体目录可见）
                            Rectangle {
                                visible: appController.currentDeckId !== "__all__"
                                width: 36; height: 36; radius: 12
                                color: root.th.card
                                border.color: root.th.border
                                Text { anchors.centerIn: parent; text: "⋮"; font.pixelSize: 18; font.bold: true; color: root.th.text2 }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        renameDeckInput.text = appController.currentDeckTitle
                                        root.activeSheet = "deckMenu"
                                    }
                                }
                            }
                        }

                        // 目录内搜索框
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.leftMargin: 16
                            Layout.rightMargin: 16
                            Layout.bottomMargin: 8
                            height: 42
                            radius: 14
                            color: root.th.card
                            border.color: deckSearchInput.activeFocus ? root.th.primary : root.th.border

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 8
                                Text { text: "🔍"; font.pixelSize: 14; color: root.th.faint }
                                TextInput {
                                    id: deckSearchInput
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    color: root.th.text
                                    clip: true
                                    onTextChanged: appController.setDeckSearchQuery(text)
                                    Text {
                                        anchors.fill: parent
                                        text: "在本目录中搜索…"
                                        color: root.th.faint
                                        font.pixelSize: 14
                                        visible: !deckSearchInput.text
                                    }
                                }
                            }
                        }

                        // 当前排序/筛选状态提示条
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 16
                            Layout.rightMargin: 16
                            Layout.bottomMargin: 8
                            visible: appController.sortMode !== "default" || appController.filterMode !== "all"
                            spacing: 6

                            Rectangle {
                                visible: appController.sortMode !== "default"
                                height: 26
                                width: sortBadgeTxt.implicitWidth + 20
                                radius: 13
                                color: root.th.cardSub
                                border.color: root.th.primary
                                Text {
                                    id: sortBadgeTxt
                                    anchors.centerIn: parent
                                    font.pixelSize: 11
                                    font.bold: true
                                    color: root.th.primaryD
                                    text: appController.sortMode === "alpha" ? "字母顺序"
                                        : (appController.sortMode === "mastery" ? "掌握程度" : "最近复习")
                                }
                            }

                            Rectangle {
                                visible: appController.filterMode !== "all"
                                height: 26
                                width: filterBadgeTxt.implicitWidth + 20
                                radius: 13
                                color: root.th.cardSub
                                border.color: root.th.primary
                                Text {
                                    id: filterBadgeTxt
                                    anchors.centerIn: parent
                                    font.pixelSize: 11
                                    font.bold: true
                                    color: root.th.primaryD
                                    text: appController.filterMode === "wrong" ? "有错词"
                                        : (appController.filterMode === "fuzzy" ? "有模糊" : "已掌握")
                                }
                            }
                        }

                        // 单词列表或空状态
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            ColumnLayout {
                                anchors.centerIn: parent
                                visible: appController.wordsModel.length === 0
                                spacing: 8
                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "📭"
                                    font.pixelSize: 36
                                }
                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: deckSearchInput.text.length > 0 ? "没有找到匹配的单词" : "这里还没有单词"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: root.th.muted
                                }
                                Text {
                                    Layout.alignment: Qt.AlignHCenter
                                    text: "点击右下角 ＋ 添加新单词"
                                    font.pixelSize: 12
                                    color: root.th.faint
                                }
                            }

                            ListView {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 16
                                anchors.bottomMargin: 16
                                clip: true
                                spacing: 8
                                model: appController.wordsModel
                                delegate: WordCardDelegate {
                                    width: ListView.view.width
                                    wordData: modelData
                                    showDeckBadge: appController.currentDeckId === "__all__"
                                    showDeleteBtn: true
                                }
                            }
                        }
                    }

                    // 悬浮添加单词按钮 (FAB)
                    Rectangle {
                        width: 52; height: 52; radius: 26
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.rightMargin: 20
                        anchors.bottomMargin: 20
                        color: root.th.primary
                        Text {
                            anchors.centerIn: parent
                            text: "＋"
                            font.pixelSize: 26
                            font.bold: true
                            color: root.th.onBtn
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (appController.totalDeckCount === 0) {
                                    toastBanner.showToast("请先新建一个目录")
                                    return
                                }
                                if (appController.currentDeckId !== "__all__") {
                                    root.pendingDeckId = appController.currentDeckId
                                } else {
                                    root.pendingDeckId = appController.decksModel[0].id
                                }
                                wordEnInput.text = ""
                                wordCnInput.text = ""
                                wordPosInput.text = ""
                                wordExInput.text = ""
                                root.activeSheet = "word"
                            }
                        }
                    }
                }
            }

            // ==================== 页面 2：单词测验 ====================
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: root.currentTab === "quiz"

                // ---------- 子视图 2A：测验设置面板 ----------
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    visible: !appController.quizActive && !appController.quizFinished

                    // 顶部标题与快捷按钮
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.margins: 16
                        Layout.bottomMargin: 10
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 3
                                Text {
                                    text: "单词测验"
                                    font.pixelSize: 23
                                    font.bold: true
                                    color: root.th.primaryD
                                }
                                Text {
                                    text: "艾宾浩斯遗忘曲线智能复习"
                                    font.pixelSize: 12.5
                                    color: root.th.muted
                                }
                            }

                            Rectangle {
                                width: 36; height: 36; radius: 12
                                color: root.th.card
                                border.color: root.th.border
                                Text { anchors.centerIn: parent; text: "🎨"; font.pixelSize: 16 }
                                MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "theme" }
                            }
                        }

                        // 快捷行：今日复习 + 交互模式切换
                        RowLayout {
                            spacing: 8

                            Rectangle {
                                height: 34
                                width: smartRevRow.implicitWidth + 24
                                radius: 17
                                color: appController.reviewBrief.total > 0 ? root.th.cardSub : root.th.card
                                border.color: appController.reviewBrief.total > 0 ? root.th.primary : root.th.border
                                opacity: appController.reviewBrief.total > 0 ? 1.0 : 0.55

                                RowLayout {
                                    id: smartRevRow
                                    anchors.centerIn: parent
                                    spacing: 6
                                    Text { text: "💡"; font.pixelSize: 13 }
                                    Text {
                                        text: "今日复习"
                                        font.pixelSize: 12.5
                                        font.bold: true
                                        color: appController.reviewBrief.total > 0 ? root.th.primaryD : root.th.text2
                                    }
                                    Rectangle {
                                        visible: appController.reviewBrief.total > 0
                                        height: 18
                                        width: Math.max(18, revBadgeTxt.implicitWidth + 10)
                                        radius: 9
                                        color: root.th.primary
                                        Text {
                                            id: revBadgeTxt
                                            anchors.centerIn: parent
                                            text: appController.reviewBrief.total > 99 ? "99+" : appController.reviewBrief.total
                                            font.pixelSize: 10
                                            font.bold: true
                                            color: root.th.onBtn
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    enabled: appController.reviewBrief.total > 0
                                    onClicked: appController.startSmartReviewQuiz()
                                }
                            }

                            Rectangle {
                                height: 34
                                width: interModeRow.implicitWidth + 24
                                radius: 17
                                color: root.th.card
                                border.color: root.th.border

                                RowLayout {
                                    id: interModeRow
                                    anchors.centerIn: parent
                                    spacing: 6
                                    Text { text: appController.interactionMode === "swipe" ? "👆" : "🖱️"; font.pixelSize: 13 }
                                    Text {
                                        text: appController.interactionMode === "swipe" ? "滑动模式" : "点按模式"
                                        font.pixelSize: 12.5
                                        font.bold: true
                                        color: root.th.text2
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.activeSheet = "interaction"
                                }
                            }
                        }
                    }

                    // 滚动设置区
                    Flickable {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        contentHeight: quizSetupCol.height + 24
                        clip: true

                        ColumnLayout {
                            id: quizSetupCol
                            width: parent.width - 32
                            x: 16
                            spacing: 14

                            // 艾宾浩斯复习简报卡片
                            Rectangle {
                                Layout.fillWidth: true
                                implicitHeight: briefCol.implicitHeight + 32
                                radius: 16
                                color: root.th.cardSub
                                border.color: root.th.borderHi

                                ColumnLayout {
                                    id: briefCol
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 8

                                    Text {
                                        text: appController.reviewBrief.total === 0
                                              ? "✓ 今日复习已完成 🎉"
                                              : "💡 今日智能复习 · 共 " + appController.reviewBrief.total + " 个词"
                                        font.pixelSize: 14.5
                                        font.bold: true
                                        color: root.th.primaryD
                                    }

                                    Text {
                                        visible: appController.reviewBrief.total === 0
                                        text: "根据艾宾浩斯遗忘曲线，今天没有到期的单词。\n可以继续添加新词或进行新一轮测验。"
                                        font.pixelSize: 12.5
                                        color: root.th.muted
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }

                                    RowLayout {
                                        visible: appController.reviewBrief.wrong > 0
                                        spacing: 8
                                        Rectangle {
                                            width: 28; height: 22; radius: 6
                                            color: "#26f06b6b"; border.color: "#4df06b6b"
                                            Text { anchors.centerIn: parent; text: appController.reviewBrief.wrong; color: root.th.redD; font.pixelSize: 12; font.bold: true }
                                        }
                                        Text { text: "没记住 · 优先复习"; font.pixelSize: 12.5; color: root.th.text2 }
                                    }

                                    RowLayout {
                                        visible: appController.reviewBrief.fuzzy > 0
                                        spacing: 8
                                        Rectangle {
                                            width: 28; height: 22; radius: 6
                                            color: "#26f5a623"; border.color: "#4df5a623"
                                            Text { anchors.centerIn: parent; text: appController.reviewBrief.fuzzy; color: root.th.amberD; font.pixelSize: 12; font.bold: true }
                                        }
                                        Text { text: "模糊 · 需要加强"; font.pixelSize: 12.5; color: root.th.text2 }
                                    }

                                    RowLayout {
                                        visible: appController.reviewBrief.dueOk > 0
                                        spacing: 8
                                        Rectangle {
                                            width: 28; height: 22; radius: 6
                                            color: "#2234c77b"; border.color: "#4734c77b"
                                            Text { anchors.centerIn: parent; text: appController.reviewBrief.dueOk; color: root.th.greenD; font.pixelSize: 12; font.bold: true }
                                        }
                                        Text { text: "到期的答对词 · 巩固记忆"; font.pixelSize: 12.5; color: root.th.text2 }
                                    }

                                    // 比例进度条
                                    Rectangle {
                                        visible: appController.reviewBrief.total > 0
                                        Layout.fillWidth: true
                                        height: 6
                                        radius: 3
                                        color: root.th.border
                                        clip: true

                                        Row {
                                            anchors.fill: parent
                                            Rectangle {
                                                height: parent.height
                                                width: parent.width * (appController.reviewBrief.wrongPct || 0)
                                                color: "#f06b6b"
                                            }
                                            Rectangle {
                                                height: parent.height
                                                width: parent.width * (appController.reviewBrief.fuzzyPct || 0)
                                                color: "#f5a623"
                                            }
                                            Rectangle {
                                                height: parent.height
                                                width: parent.width * (appController.reviewBrief.dueOkPct || 0)
                                                color: "#34c77b"
                                            }
                                        }
                                    }

                                    Text {
                                        visible: appController.reviewBrief.total > 0
                                        Layout.fillWidth: true
                                        wrapMode: Text.WordWrap
                                        font.pixelSize: 11.5
                                        color: root.th.muted
                                        text: "📚 艾宾浩斯曲线：错词明天必复习，模糊词降一级，答对的词按 1 → 2 → 4 → 7 → 15 → 30 天递增间隔。连续答对 7 次即视为已掌握。"
                                    }
                                }
                            }

                            // 1. 选择范围
                            Text {
                                text: "1. 选择范围"
                                font.pixelSize: 12
                                font.bold: true
                                color: root.th.faint
                            }

                            // 全部单词选项
                            Rectangle {
                                Layout.fillWidth: true
                                height: 54
                                radius: 13
                                color: root.quizRangeId === "__all__" ? root.th.cardSub : root.th.card
                                border.color: root.quizRangeId === "__all__" ? root.th.primary : root.th.border

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 14
                                    anchors.rightMargin: 14
                                    spacing: 10

                                    Rectangle {
                                        width: 18; height: 18; radius: 9
                                        color: "transparent"
                                        border.color: root.quizRangeId === "__all__" ? root.th.primary : root.th.faint
                                        border.width: 2
                                        Rectangle {
                                            visible: root.quizRangeId === "__all__"
                                            width: 8; height: 8; radius: 4
                                            anchors.centerIn: parent
                                            color: root.th.primary
                                        }
                                    }

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2
                                        Text { text: "全部单词"; font.pixelSize: 14.5; font.bold: true; color: root.th.text }
                                        Text { text: appController.totalWordCount + " 个单词"; font.pixelSize: 11.5; color: root.th.muted }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: root.quizRangeId = "__all__"
                                }
                            }

                            Repeater {
                                model: appController.decksModel
                                delegate: Rectangle {
                                    Layout.fillWidth: true
                                    height: 54
                                    radius: 13
                                    color: root.quizRangeId === modelData.id ? root.th.cardSub : root.th.card
                                    border.color: root.quizRangeId === modelData.id ? root.th.primary : root.th.border
                                    opacity: modelData.wordCount === 0 ? 0.5 : 1.0

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.leftMargin: 14
                                        anchors.rightMargin: 14
                                        spacing: 10

                                        Rectangle {
                                            width: 18; height: 18; radius: 9
                                            color: "transparent"
                                            border.color: root.quizRangeId === modelData.id ? root.th.primary : root.th.faint
                                            border.width: 2
                                            Rectangle {
                                                visible: root.quizRangeId === modelData.id
                                                width: 8; height: 8; radius: 4
                                                anchors.centerIn: parent
                                                color: root.th.primary
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text { text: modelData.name; font.pixelSize: 14.5; font.bold: true; color: root.th.text; elide: Text.ElideRight; Layout.fillWidth: true }
                                            Text { text: modelData.wordCount + " 个单词"; font.pixelSize: 11.5; color: root.th.muted }
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: root.quizRangeId = modelData.id
                                    }
                                }
                            }

                            // 2. 测验数量
                            Text {
                                text: "2. 测验数量"
                                font.pixelSize: 12
                                font.bold: true
                                color: root.th.faint
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Repeater {
                                    model: [5, 10, 20, 30, -1]
                                    delegate: Rectangle {
                                        height: 36
                                        width: chipLabel.implicitWidth + 26
                                        radius: 12
                                        color: root.quizCountSetting === modelData ? root.th.cardSub : root.th.card
                                        border.color: root.quizCountSetting === modelData ? root.th.primary : root.th.border

                                        Text {
                                            id: chipLabel
                                            anchors.centerIn: parent
                                            text: modelData === -1 ? "全部" : modelData
                                            font.pixelSize: 13.5
                                            font.bold: true
                                            color: root.quizCountSetting === modelData ? root.th.primaryD : root.th.text2
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: root.quizCountSetting = modelData
                                        }
                                    }
                                }
                            }

                            // 开始测验主按钮
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.topMargin: 6
                                height: 50
                                radius: 14
                                color: root.th.primary

                                Text {
                                    anchors.centerIn: parent
                                    text: "⚡ 开始测验"
                                    font.pixelSize: 15
                                    font.bold: true
                                    color: root.th.onBtn
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        appController.startNormalQuiz(root.quizRangeId, root.quizCountSetting)
                                    }
                                }
                            }
                        }
                    }
                }

                // ---------- 子视图 2B：测验答题运行中 ----------
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14
                    visible: appController.quizActive

                    // 顶部进度条
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            Layout.fillWidth: true
                            height: 6
                            radius: 3
                            color: root.th.border
                            Rectangle {
                                height: parent.height
                                radius: 3
                                width: appController.quizTotal > 0
                                       ? parent.width * (appController.quizIndex / appController.quizTotal)
                                       : 0
                                color: root.th.primary
                                Behavior on width { NumberAnimation { duration: 250 } }
                            }
                        }

                        Text {
                            text: (appController.quizIndex + 1) + " / " + appController.quizTotal
                            font.pixelSize: 12
                            font.bold: true
                            color: root.th.muted
                        }
                    }

                    // 单词闪卡（支持点击翻面 + 滑动模式手势判定）
                    Rectangle {
                        id: flashCard
                        Layout.fillWidth: true
                        Layout.preferredHeight: 260
                        radius: 22
                        color: root.th.card
                        border.color: cardMouse.drag.active
                                      ? (flashCard.x > 40 ? root.th.greenD : (flashCard.x < -40 ? root.th.redD : root.th.amberD))
                                      : root.th.borderHi
                        border.width: cardMouse.drag.active ? 2 : 1

                        property real startX: 0
                        property real startY: 0
                        property real dragDx: 0
                        property real dragDy: 0

                        transform: [
                            Translate { x: flashCard.dragDx; y: flashCard.dragDy },
                            Rotation {
                                origin.x: flashCard.width / 2
                                origin.y: flashCard.height / 2
                                angle: flashCard.dragDx * 0.04
                            }
                        ]

                        ColumnLayout {
                            anchors.centerIn: parent
                            width: parent.width - 40
                            spacing: 14

                            Text {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: appController.quizCurrentWord.en || "—"
                                font.pixelSize: 30
                                font.bold: true
                                color: root.th.text
                                wrapMode: Text.Wrap
                            }

                            Text {
                                visible: root.answerRevealed && (appController.quizCurrentWord.pos || "") !== ""
                                Layout.alignment: Qt.AlignHCenter
                                text: appController.quizCurrentWord.pos || ""
                                font.pixelSize: 12
                                font.italic: true
                                color: root.th.accent
                            }

                            Text {
                                visible: root.answerRevealed
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: appController.quizCurrentWord.cn || ""
                                font.pixelSize: 22
                                font.bold: true
                                color: root.th.primaryD
                                wrapMode: Text.Wrap
                            }

                            Text {
                                visible: root.answerRevealed && (appController.quizCurrentWord.ex || "") !== ""
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: appController.quizCurrentWord.ex || ""
                                font.pixelSize: 12.5
                                font.italic: true
                                color: root.th.muted
                                wrapMode: Text.Wrap
                            }

                            Text {
                                visible: !root.answerRevealed
                                Layout.alignment: Qt.AlignHCenter
                                text: "💡 轻触卡片查看答案"
                                font.pixelSize: 12.5
                                color: root.th.faint
                            }

                            Rectangle {
                                visible: !root.answerRevealed
                                Layout.alignment: Qt.AlignHCenter
                                width: 40; height: 2; radius: 1
                                color: root.th.primary
                                opacity: 0.5
                            }

                            Text {
                                visible: appController.interactionMode === "swipe"
                                Layout.alignment: Qt.AlignHCenter
                                text: "← 左滑没记住   ↑ 上滑模糊   右滑记住 →"
                                font.pixelSize: 11.5
                                color: root.th.muted
                            }
                        }

                        MouseArea {
                            id: cardMouse
                            anchors.fill: parent
                            onPressed: function(mouse) {
                                flashCard.startX = mouse.x
                                flashCard.startY = mouse.y
                                flashCard.dragDx = 0
                                flashCard.dragDy = 0
                            }
                            onPositionChanged: function(mouse) {
                                if (appController.interactionMode === "swipe" && root.answerRevealed && pressed) {
                                    flashCard.dragDx = mouse.x - flashCard.startX
                                    flashCard.dragDy = mouse.y - flashCard.startY
                                }
                            }
                            onReleased: function(mouse) {
                                var dx = flashCard.dragDx
                                var dy = flashCard.dragDy
                                flashCard.dragDx = 0
                                flashCard.dragDy = 0

                                if (!root.answerRevealed) {
                                    root.answerRevealed = true
                                    return
                                }

                                if (appController.interactionMode === "swipe") {
                                    if (Math.abs(dx) >= 70 || Math.abs(dy) >= 70) {
                                        if (Math.abs(dx) > Math.abs(dy)) {
                                            appController.answerQuiz(dx > 0 ? "know" : "wrong")
                                        } else if (dy < 0) {
                                            appController.answerQuiz("fuzzy")
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // 三个判定按钮（答案揭晓后显示）
                    RowLayout {
                        Layout.fillWidth: true
                        visible: root.answerRevealed
                        spacing: 8

                        Rectangle {
                            Layout.fillWidth: true
                            height: 58; radius: 14
                            color: "#1af06b6b"; border.color: "#59f06b6b"
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 4
                                Text { Layout.alignment: Qt.AlignHCenter; text: "✕"; font.pixelSize: 16; font.bold: true; color: root.th.redD }
                                Text { Layout.alignment: Qt.AlignHCenter; text: "没记住"; font.pixelSize: 13; font.bold: true; color: root.th.redD }
                            }
                            MouseArea { anchors.fill: parent; onClicked: appController.answerQuiz("wrong") }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 58; radius: 14
                            color: "#1ff5a623"; border.color: "#66f5a623"
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 4
                                Text { Layout.alignment: Qt.AlignHCenter; text: "◯"; font.pixelSize: 16; font.bold: true; color: root.th.amberD }
                                Text { Layout.alignment: Qt.AlignHCenter; text: "模糊"; font.pixelSize: 13; font.bold: true; color: root.th.amberD }
                            }
                            MouseArea { anchors.fill: parent; onClicked: appController.answerQuiz("fuzzy") }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 58; radius: 14
                            color: "#1a34c77b"; border.color: "#5934c77b"
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 4
                                Text { Layout.alignment: Qt.AlignHCenter; text: "✓"; font.pixelSize: 16; font.bold: true; color: root.th.greenD }
                                Text { Layout.alignment: Qt.AlignHCenter; text: "记住了"; font.pixelSize: 13; font.bold: true; color: root.th.greenD }
                            }
                            MouseArea { anchors.fill: parent; onClicked: appController.answerQuiz("know") }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }

                // ---------- 子视图 2C：测验结果面板 ----------
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 14
                    visible: !appController.quizActive && appController.quizFinished

                    Text {
                        text: "测验结果"
                        font.pixelSize: 22
                        font.bold: true
                        color: root.th.primaryD
                    }

                    // 正确率大卡片
                    Rectangle {
                        Layout.fillWidth: true
                        height: 170
                        radius: 22
                        color: root.th.card
                        border.color: root.th.borderHi

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 6

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: (appController.quizResult.rate || 0) + "%"
                                font.pixelSize: 44
                                font.bold: true
                                color: root.th.primaryD
                            }
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "正确率"
                                font.pixelSize: 12
                                font.bold: true
                                color: root.th.muted
                            }

                            RowLayout {
                                Layout.alignment: Qt.AlignHCenter
                                Layout.topMargin: 8
                                spacing: 28

                                ColumnLayout {
                                    spacing: 2
                                    Text { Layout.alignment: Qt.AlignHCenter; text: appController.quizResult.ok || 0; font.pixelSize: 18; font.bold: true; color: root.th.greenD }
                                    Text { Layout.alignment: Qt.AlignHCenter; text: "记住"; font.pixelSize: 11; color: root.th.faint }
                                }
                                ColumnLayout {
                                    spacing: 2
                                    Text { Layout.alignment: Qt.AlignHCenter; text: appController.quizResult.fz || 0; font.pixelSize: 18; font.bold: true; color: root.th.amberD }
                                    Text { Layout.alignment: Qt.AlignHCenter; text: "模糊"; font.pixelSize: 11; color: root.th.faint }
                                }
                                ColumnLayout {
                                    spacing: 2
                                    Text { Layout.alignment: Qt.AlignHCenter; text: appController.quizResult.wrong || 0; font.pixelSize: 18; font.bold: true; color: root.th.redD }
                                    Text { Layout.alignment: Qt.AlignHCenter; text: "没记住"; font.pixelSize: 11; color: root.th.faint }
                                }
                            }
                        }
                    }

                    // 需要加强的词列表
                    Text {
                        visible: (appController.quizResult.wrongList || []).length > 0
                        text: "需要加强"
                        font.pixelSize: 11
                        font.bold: true
                        color: root.th.faint
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: 8
                        model: appController.quizResult.wrongList || []
                        delegate: WordCardDelegate {
                            width: ListView.view.width
                            wordData: modelData
                            showDeckBadge: false
                            showDeleteBtn: false
                        }
                    }

                    // 操作按钮
                    Rectangle {
                        visible: (appController.quizResult.wrongList || []).length > 0
                        Layout.fillWidth: true
                        height: 46; radius: 14
                        color: root.th.primary
                        Text { anchors.centerIn: parent; text: "只练需要加强的词"; font.pixelSize: 14; font.bold: true; color: root.th.onBtn }
                        MouseArea { anchors.fill: parent; onClicked: appController.retryWrongQuiz() }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 46; radius: 14
                        color: root.th.card
                        border.color: root.th.border
                        Text { anchors.centerIn: parent; text: "重新设置"; font.pixelSize: 14; font.bold: true; color: root.th.text2 }
                        MouseArea { anchors.fill: parent; onClicked: appController.abortQuiz() }
                    }
                }
            }

            // ==================== 底部导航栏 ====================
            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: root.th.card
                border.color: root.th.border

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 12
                        color: root.currentTab === "accum" ? root.th.cardSub : "transparent"

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            Text { Layout.alignment: Qt.AlignHCenter; text: "📖"; font.pixelSize: 16 }
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "单词积累"
                                font.pixelSize: 11.5
                                font.bold: true
                                color: root.currentTab === "accum" ? root.th.primaryD : root.th.muted
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.currentTab = "accum"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 12
                        color: root.currentTab === "quiz" ? root.th.cardSub : "transparent"

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2
                            Text { Layout.alignment: Qt.AlignHCenter; text: "⚡"; font.pixelSize: 16 }
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: "单词测验"
                                font.pixelSize: 11.5
                                font.bold: true
                                color: root.currentTab === "quiz" ? root.th.primaryD : root.th.muted
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: root.currentTab = "quiz"
                        }
                    }
                }
            }
        }

        // ==================== 底部弹窗遮罩层 ====================
        Rectangle {
            anchors.fill: parent
            color: "#660a0e1c"
            visible: root.activeSheet !== ""
            z: 50

            MouseArea {
                anchors.fill: parent
                onClicked: root.activeSheet = ""
            }

            // ---------- 弹窗 1：添加单词 ----------
            Rectangle {
                visible: root.activeSheet === "word"
                width: parent.width
                height: addWordCol.implicitHeight + 44
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card

                MouseArea { anchors.fill: parent } // 阻止点击穿透

                ColumnLayout {
                    id: addWordCol
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Text { text: "添加单词"; font.pixelSize: 17; font.bold: true; color: root.th.text }

                    Text { text: "单词 / 短语"; font.pixelSize: 12; font.bold: true; color: root.th.muted }
                    Rectangle {
                        Layout.fillWidth: true; height: 40; radius: 10
                        color: root.th.cardSub; border.color: root.th.border
                        TextInput {
                            id: wordEnInput
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 14; color: root.th.text; clip: true
                        }
                    }

                    Text { text: "释义"; font.pixelSize: 12; font.bold: true; color: root.th.muted }
                    Rectangle {
                        Layout.fillWidth: true; height: 40; radius: 10
                        color: root.th.cardSub; border.color: root.th.border
                        TextInput {
                            id: wordCnInput
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 14; color: root.th.text; clip: true
                        }
                    }

                    Text { text: "词性（可选，如 v. / n. / adj.）"; font.pixelSize: 12; font.bold: true; color: root.th.muted }
                    Rectangle {
                        Layout.fillWidth: true; height: 40; radius: 10
                        color: root.th.cardSub; border.color: root.th.border
                        TextInput {
                            id: wordPosInput
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 14; color: root.th.text; clip: true
                        }
                    }

                    Text { text: "例句（可选）"; font.pixelSize: 12; font.bold: true; color: root.th.muted }
                    Rectangle {
                        Layout.fillWidth: true; height: 40; radius: 10
                        color: root.th.cardSub; border.color: root.th.border
                        TextInput {
                            id: wordExInput
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 14; color: root.th.text; clip: true
                        }
                    }

                    // 所属目录选择（在全部单词视图下显示）
                    Text {
                        visible: appController.currentDeckId === "__all__"
                        text: "所属目录"
                        font.pixelSize: 12; font.bold: true; color: root.th.muted
                    }
                    Flow {
                        visible: appController.currentDeckId === "__all__"
                        Layout.fillWidth: true
                        spacing: 6
                        Repeater {
                            model: appController.decksModel
                            delegate: Rectangle {
                                height: 30
                                width: dpTxt.implicitWidth + 22
                                radius: 15
                                color: root.pendingDeckId === modelData.id ? root.th.cardSub : root.th.card
                                border.color: root.pendingDeckId === modelData.id ? root.th.primary : root.th.border
                                Text {
                                    id: dpTxt
                                    anchors.centerIn: parent
                                    text: modelData.name
                                    font.pixelSize: 12; font.bold: true
                                    color: root.pendingDeckId === modelData.id ? root.th.primaryD : root.th.text2
                                }
                                MouseArea { anchors.fill: parent; onClicked: root.pendingDeckId = modelData.id }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 8
                        spacing: 10

                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.cardSub; border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "取消"; font.pixelSize: 14; font.bold: true; color: root.th.text2 }
                            MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "" }
                        }

                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.primary
                            Text { anchors.centerIn: parent; text: "添加"; font.pixelSize: 14; font.bold: true; color: root.th.onBtn }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (appController.addWord(root.pendingDeckId, wordEnInput.text, wordCnInput.text, wordPosInput.text, wordExInput.text)) {
                                        root.activeSheet = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---------- 弹窗 2：新建目录 ----------
            Rectangle {
                visible: root.activeSheet === "newDeck"
                width: parent.width
                height: 200
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Text { text: "新建目录"; font.pixelSize: 17; font.bold: true; color: root.th.text }
                    Rectangle {
                        Layout.fillWidth: true; height: 42; radius: 10
                        color: root.th.cardSub; border.color: root.th.border
                        TextInput {
                            id: newDeckInput
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 14; color: root.th.text; clip: true
                            Text {
                                anchors.fill: parent
                                text: "例如：Text 2 / 四级核心词"
                                color: root.th.faint; font.pixelSize: 14
                                visible: !newDeckInput.text
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.cardSub; border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "取消"; font.pixelSize: 14; font.bold: true; color: root.th.text2 }
                            MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "" }
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.primary
                            Text { anchors.centerIn: parent; text: "创建"; font.pixelSize: 14; font.bold: true; color: root.th.onBtn }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (appController.createDeck(newDeckInput.text)) {
                                        root.activeSheet = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---------- 弹窗 3：目录管理（重命名/删除目录） ----------
            Rectangle {
                visible: root.activeSheet === "deckMenu"
                width: parent.width
                height: 250
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Text { text: "管理目录"; font.pixelSize: 17; font.bold: true; color: root.th.text }
                    Rectangle {
                        Layout.fillWidth: true; height: 42; radius: 10
                        color: root.th.cardSub; border.color: root.th.border
                        TextInput {
                            id: renameDeckInput
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 14; color: root.th.text; clip: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.primary
                            Text { anchors.centerIn: parent; text: "保存重命名"; font.pixelSize: 14; font.bold: true; color: root.th.onBtn }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (appController.renameDeck(appController.currentDeckId, renameDeckInput.text)) {
                                        root.activeSheet = ""
                                    }
                                }
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: "#22e05555"; border.color: root.th.redD
                            Text { anchors.centerIn: parent; text: "删除整个目录"; font.pixelSize: 14; font.bold: true; color: root.th.redD }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (appController.deleteDeck(appController.currentDeckId)) {
                                        root.activeSheet = ""
                                        root.inWordsView = false
                                    }
                                }
                            }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true; height: 40; radius: 12
                        color: root.th.cardSub; border.color: root.th.border
                        Text { anchors.centerIn: parent; text: "关闭"; font.pixelSize: 13; font.bold: true; color: root.th.text2 }
                        MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "" }
                    }
                }
            }

            // ---------- 弹窗 4：九大主题风格切换 ----------
            Rectangle {
                visible: root.activeSheet === "theme"
                width: parent.width
                height: 360
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12

                    Text { text: "选择风格（九大主题世界观）"; font.pixelSize: 17; font.bold: true; color: root.th.text }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        columns: 3
                        rowSpacing: 10
                        columnSpacing: 10

                        Repeater {
                            model: ThemeData.themes
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 14
                                color: appController.currentTheme === modelData.id ? root.th.cardSub : root.th.card
                                border.color: appController.currentTheme === modelData.id ? root.th.primary : root.th.border
                                border.width: appController.currentTheme === modelData.id ? 2 : 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 6

                                    Rectangle {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        radius: 8
                                        color: modelData.bgBase
                                        border.color: "#22000000"

                                        Rectangle {
                                            width: 14; height: 14; radius: 7
                                            x: 10; y: parent.height - 20
                                            color: modelData.c1
                                        }
                                        Rectangle {
                                            width: 14; height: 14; radius: 7
                                            x: parent.width - 24; y: 6
                                            color: modelData.c2
                                        }
                                    }

                                    Text {
                                        Layout.alignment: Qt.AlignHCenter
                                        text: modelData.name
                                        font.pixelSize: 11.5
                                        font.bold: true
                                        color: appController.currentTheme === modelData.id ? root.th.primaryD : root.th.text2
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        appController.setTheme(modelData.id)
                                        toastBanner.showToast("已切换到「" + modelData.name + "」")
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---------- 弹窗 5：测验交互方式切换 ----------
            Rectangle {
                visible: root.activeSheet === "interaction"
                width: parent.width
                height: 220
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Text { text: "交互方式"; font.pixelSize: 17; font.bold: true; color: root.th.text }

                    Repeater {
                        model: [
                            { id: "tap", name: "点按模式", sub: "点卡片看答案，再点按钮判定" },
                            { id: "swipe", name: "滑动模式", sub: "右滑=记住，左滑=没记住，上滑=模糊" }
                        ]
                        delegate: Rectangle {
                            Layout.fillWidth: true; height: 58; radius: 13
                            color: appController.interactionMode === modelData.id ? root.th.cardSub : root.th.card
                            border.color: appController.interactionMode === modelData.id ? root.th.primary : root.th.border

                            ColumnLayout {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left; anchors.leftMargin: 14
                                spacing: 2
                                Text { text: modelData.name; font.pixelSize: 14.5; font.bold: true; color: root.th.text }
                                Text { text: modelData.sub; font.pixelSize: 11.5; color: root.th.muted }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    appController.setInteractionMode(modelData.id)
                                    root.activeSheet = ""
                                }
                            }
                        }
                    }
                }
            }

            // ---------- 弹窗 6：打卡日历 ----------
            Rectangle {
                visible: root.activeSheet === "calendar"
                width: parent.width
                height: 440
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Text { text: "打卡日历"; font.pixelSize: 17; font.bold: true; color: root.th.text }

                    // 月份切换行
                    RowLayout {
                        Layout.fillWidth: true
                        Rectangle {
                            width: 32; height: 32; radius: 9; color: root.th.cardSub; border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "‹"; font.pixelSize: 18; font.bold: true; color: root.th.text2 }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.calMonth--
                                    if (root.calMonth < 1) { root.calMonth = 12; root.calYear-- }
                                    root.refreshCalendar()
                                }
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            text: root.calendarData.title || ""
                            font.pixelSize: 15; font.bold: true; color: root.th.text
                        }
                        Rectangle {
                            width: 32; height: 32; radius: 9; color: root.th.cardSub; border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "›"; font.pixelSize: 18; font.bold: true; color: root.th.text2 }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.calMonth++
                                    if (root.calMonth > 12) { root.calMonth = 1; root.calYear++ }
                                    root.refreshCalendar()
                                }
                            }
                        }
                    }

                    // 星期表头
                    RowLayout {
                        Layout.fillWidth: true
                        Repeater {
                            model: ["一", "二", "三", "四", "五", "六", "日"]
                            delegate: Text {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: modelData
                                font.pixelSize: 11; font.bold: true; color: root.th.faint
                            }
                        }
                    }

                    // 日期网格
                    GridLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        columns: 7
                        rowSpacing: 6
                        columnSpacing: 6

                        Repeater {
                            model: root.calendarData.cells || []
                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 10
                                color: modelData.isChecked ? root.th.primary : "transparent"
                                border.color: modelData.isToday ? root.th.primary : "transparent"
                                border.width: modelData.isToday ? 1.5 : 0
                                opacity: modelData.isOtherMonth ? 0.3 : 1.0

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.day
                                    font.pixelSize: 12.5
                                    font.bold: true
                                    color: modelData.isChecked ? root.th.onBtn : (modelData.isToday ? root.th.primaryD : root.th.text2)
                                }
                            }
                        }
                    }

                    // 底部统计栏
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Rectangle {
                            Layout.fillWidth: true; height: 54; radius: 12
                            color: root.th.cardSub; border.color: root.th.border
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 2
                                Text { Layout.alignment: Qt.AlignHCenter; text: root.calendarData.streak || 0; font.pixelSize: 18; font.bold: true; color: root.th.primaryD }
                                Text { Layout.alignment: Qt.AlignHCenter; text: "连续天"; font.pixelSize: 11; color: root.th.muted }
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 54; radius: 12
                            color: root.th.cardSub; border.color: root.th.border
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 2
                                Text { Layout.alignment: Qt.AlignHCenter; text: root.calendarData.monthCount || 0; font.pixelSize: 18; font.bold: true; color: root.th.primaryD }
                                Text { Layout.alignment: Qt.AlignHCenter; text: "本月天"; font.pixelSize: 11; color: root.th.muted }
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 54; radius: 12
                            color: root.th.cardSub; border.color: root.th.border
                            ColumnLayout {
                                anchors.centerIn: parent; spacing: 2
                                Text { Layout.alignment: Qt.AlignHCenter; text: root.calendarData.totalCount || 0; font.pixelSize: 18; font.bold: true; color: root.th.primaryD }
                                Text { Layout.alignment: Qt.AlignHCenter; text: "累计天"; font.pixelSize: 11; color: root.th.muted }
                            }
                        }
                    }
                }
            }

            // ---------- 弹窗 7：54 项成就徽章墙 ----------
            Rectangle {
                visible: root.activeSheet === "ach"
                width: parent.width
                height: parent.height * 0.82
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "成就徽章"; font.pixelSize: 17; font.bold: true; color: root.th.text; Layout.fillWidth: true }
                        Text {
                            text: appController.unlockedBadgeCount + " / " + appController.totalBadgeCount
                            font.pixelSize: 14; font.bold: true; color: root.th.primaryD
                        }
                    }

                    // 成就总进度条
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        Rectangle {
                            Layout.fillWidth: true; height: 8; radius: 4; color: root.th.border
                            Rectangle {
                                height: parent.height; radius: 4
                                width: parent.width * (appController.unlockedBadgeCount / Math.max(1, appController.totalBadgeCount))
                                color: root.th.primary
                            }
                        }
                        Text {
                            text: Math.round(appController.unlockedBadgeCount / Math.max(1, appController.totalBadgeCount) * 100) + "%"
                            font.pixelSize: 12; font.bold: true; color: root.th.primaryD
                        }
                    }

                    // 徽章三列网格
                    GridView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        cellWidth: width / 3
                        cellHeight: 132
                        model: appController.badgesModel
                        delegate: Item {
                            width: GridView.view.cellWidth
                            height: GridView.view.cellHeight

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: 4
                                radius: 14
                                color: modelData.unlocked ? root.th.card : root.th.cardSub
                                border.color: modelData.unlocked ? root.th.borderHi : root.th.border

                                ColumnLayout {
                                    anchors.centerIn: parent
                                    width: parent.width - 12
                                    spacing: 4

                                    Rectangle {
                                        Layout.alignment: Qt.AlignHCenter
                                        width: 46; height: 46; radius: 23
                                        color: modelData.unlocked
                                               ? Qt.hsla(modelData.hue / 360.0, modelData.sat / 100.0, modelData.light / 100.0, 1.0)
                                               : root.th.border
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.unlocked ? "🏅" : "🔒"
                                            font.pixelSize: 20
                                        }
                                    }

                                    Rectangle {
                                        Layout.alignment: Qt.AlignHCenter
                                        height: 18
                                        width: Math.max(58, progTxt.implicitWidth + 14)
                                        radius: 8
                                        color: root.th.border
                                        clip: true
                                        Rectangle {
                                            height: parent.height
                                            width: parent.width * modelData.progressRatio
                                            color: "#335b8def"
                                        }
                                        Text {
                                            id: progTxt
                                            anchors.centerIn: parent
                                            text: modelData.progressText
                                            font.pixelSize: 10; font.bold: true
                                            color: modelData.unlocked ? root.th.primaryD : root.th.muted
                                        }
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        horizontalAlignment: Text.AlignHCenter
                                        text: modelData.name
                                        font.pixelSize: 11; font.bold: true
                                        color: modelData.unlocked ? root.th.text : root.th.faint
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        horizontalAlignment: Text.AlignHCenter
                                        text: modelData.dateText
                                        font.pixelSize: 9.5
                                        color: modelData.unlocked ? root.th.primaryD : root.th.faint
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---------- 弹窗 8：排序与筛选 ----------
            Rectangle {
                visible: root.activeSheet === "sort"
                width: parent.width
                height: 490
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 8

                    Text { text: "排序方式"; font.pixelSize: 16; font.bold: true; color: root.th.text }
                    Repeater {
                        model: [
                            { id: "default", name: "默认顺序", sub: "按添加的先后顺序" },
                            { id: "alpha",   name: "字母顺序", sub: "按英文首字母 A→Z" },
                            { id: "mastery", name: "掌握程度", sub: "错误多的排前面" },
                            { id: "recent",  name: "最近复习", sub: "最近复习的排前面" }
                        ]
                        delegate: Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 10
                            color: appController.sortMode === modelData.id ? root.th.cardSub : root.th.card
                            border.color: appController.sortMode === modelData.id ? root.th.primary : root.th.border
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                                Text { text: modelData.name; font.pixelSize: 13.5; font.bold: true; color: root.th.text }
                                Text { text: modelData.sub; font.pixelSize: 11.5; color: root.th.muted; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
                            }
                            MouseArea { anchors.fill: parent; onClicked: appController.setSortAndFilter(modelData.id, "") }
                        }
                    }

                    Text { text: "筛选条件"; font.pixelSize: 16; font.bold: true; color: root.th.text; Layout.topMargin: 6 }
                    Repeater {
                        model: [
                            { id: "all",      name: "全部单词", sub: "显示所有单词" },
                            { id: "wrong",    name: "有错词",   sub: "只看没记住过的" },
                            { id: "fuzzy",    name: "有模糊",   sub: "只看标记过模糊的" },
                            { id: "mastered", name: "已掌握",   sub: "只看已完全掌握的" }
                        ]
                        delegate: Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 10
                            color: appController.filterMode === modelData.id ? root.th.cardSub : root.th.card
                            border.color: appController.filterMode === modelData.id ? root.th.primary : root.th.border
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                                Text { text: modelData.name; font.pixelSize: 13.5; font.bold: true; color: root.th.text }
                                Text { text: modelData.sub; font.pixelSize: 11.5; color: root.th.muted; Layout.fillWidth: true; horizontalAlignment: Text.AlignRight }
                            }
                            MouseArea { anchors.fill: parent; onClicked: appController.setSortAndFilter("", modelData.id) }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true; Layout.topMargin: 6; spacing: 10
                        Rectangle {
                            Layout.fillWidth: true; height: 40; radius: 10
                            color: root.th.cardSub; border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "重置"; font.pixelSize: 13.5; font.bold: true; color: root.th.text2 }
                            MouseArea { anchors.fill: parent; onClicked: appController.resetSortAndFilter() }
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 40; radius: 10
                            color: root.th.primary
                            Text { anchors.centerIn: parent; text: "完成"; font.pixelSize: 13.5; font.bold: true; color: root.th.onBtn }
                            MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "" }
                        }
                    }
                }
            }

            // ---------- 弹窗 9：数据备份与恢复 ----------
            Rectangle {
                visible: root.activeSheet === "backup"
                width: parent.width
                height: 440
                anchors.bottom: parent.bottom
                radius: 22
                color: root.th.card
                MouseArea { anchors.fill: parent }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Text { text: "数据备份与恢复"; font.pixelSize: 17; font.bold: true; color: root.th.text }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 86; radius: 14
                        color: root.th.cardSub; border.color: root.th.border
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 12; spacing: 4
                            Text { text: "📦 当前存档（兼容原版 JSON 备份格式）"; font.pixelSize: 13; font.bold: true; color: root.th.text }
                            Text { text: "包含全部单词、目录、艾宾浩斯等级、打卡、成就与主题设置。"; font.pixelSize: 11.5; color: root.th.muted }
                            RowLayout {
                                spacing: 14
                                Text { text: "单词 " + (appController.backupStats.words || 0); font.pixelSize: 12; font.bold: true; color: root.th.primaryD }
                                Text { text: "目录 " + (appController.backupStats.decks || 0); font.pixelSize: 12; font.bold: true; color: root.th.primaryD }
                                Text { text: "打卡 " + (appController.backupStats.checkins || 0); font.pixelSize: 12; font.bold: true; color: root.th.primaryD }
                                Text { text: "成就 " + (appController.backupStats.badges || 0); font.pixelSize: 12; font.bold: true; color: root.th.primaryD }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; height: 42; radius: 12
                        color: root.th.primary
                        Text { anchors.centerIn: parent; text: "📋 复制备份 JSON 文本（可发微信）"; font.pixelSize: 13.5; font.bold: true; color: root.th.onBtn }
                        MouseArea { anchors.fill: parent; onClicked: appController.copyBackupToClipboard() }
                    }

                    Text { text: "恢复存档（粘贴备份 JSON 文本）"; font.pixelSize: 12; font.bold: true; color: root.th.muted }

                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        radius: 12; color: root.th.cardSub; border.color: root.th.border
                        TextEdit {
                            id: importTextArea
                            anchors.fill: parent; anchors.margins: 10
                            font.pixelSize: 12; color: root.th.text
                            wrapMode: TextEdit.Wrap
                            clip: true
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 10
                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.cardSub; border.color: root.th.border
                            Text { anchors.centerIn: parent; text: "关闭"; font.pixelSize: 13.5; font.bold: true; color: root.th.text2 }
                            MouseArea { anchors.fill: parent; onClicked: root.activeSheet = "" }
                        }
                        Rectangle {
                            Layout.fillWidth: true; height: 42; radius: 12
                            color: root.th.primary
                            Text { anchors.centerIn: parent; text: "从文本导入"; font.pixelSize: 13.5; font.bold: true; color: root.th.onBtn }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    if (appController.importBackupText(importTextArea.text)) {
                                        root.activeSheet = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // ==================== 悬浮 Toast 提示组件 ====================
        Rectangle {
            id: toastBanner
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 76
            height: 38
            width: Math.min(parent.width * 0.86, toastLabel.implicitWidth + 36)
            radius: 19
            color: root.th.card
            border.color: root.th.primary
            opacity: 0
            visible: opacity > 0
            z: 100

            function showToast(msg) {
                toastLabel.text = msg
                toastBanner.opacity = 1
                toastTimer.restart()
            }

            Behavior on opacity { NumberAnimation { duration: 200 } }

            Text {
                id: toastLabel
                anchors.centerIn: parent
                font.pixelSize: 13
                font.bold: true
                color: root.th.primaryD
                elide: Text.ElideRight
                width: parent.width - 24
                horizontalAlignment: Text.AlignHCenter
            }

            Timer {
                id: toastTimer
                interval: 2200
                onTriggered: toastBanner.opacity = 0
            }
        }
    }

    // ==================== 内部复用组件：单词卡片 Delegate ====================
    component WordCardDelegate : Rectangle {
        property var wordData: ({})
        property bool showDeckBadge: false
        property bool showDeleteBtn: true

        radius: 14
        color: root.th.card
        border.color: root.th.border
        implicitHeight: wcCol.implicitHeight + 24

        ColumnLayout {
            id: wcCol
            anchors.fill: parent
            anchors.margins: 12
            spacing: 5

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: wordData.en || ""
                    font.pixelSize: 16
                    font.bold: true
                    color: root.th.text
                }

                Rectangle {
                    visible: (wordData.pos || "") !== ""
                    height: 18
                    width: posLabel.implicitWidth + 12
                    radius: 5
                    color: root.th.cardSub
                    border.color: root.th.accent
                    Text {
                        id: posLabel
                        anchors.centerIn: parent
                        text: wordData.pos || ""
                        font.pixelSize: 10.5
                        font.italic: true
                        font.bold: true
                        color: root.th.accent
                    }
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    visible: showDeleteBtn
                    width: 24; height: 24; radius: 6
                    color: root.th.cardSub
                    Text { anchors.centerIn: parent; text: "✕"; font.pixelSize: 11; color: root.th.faint }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: appController.deleteWord(wordData.deckId, wordData.en)
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: wordData.cn || ""
                font.pixelSize: 13.5
                color: root.th.text2
                wrapMode: Text.WordWrap
            }

            Text {
                visible: (wordData.ex || "") !== ""
                Layout.fillWidth: true
                text: "“" + (wordData.ex || "") + "”"
                font.pixelSize: 12
                font.italic: true
                color: root.th.muted
                wrapMode: Text.WordWrap
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Rectangle {
                    height: 20; width: stW.implicitWidth + 12; radius: 6
                    color: "#1ff06b6b"
                    Text { id: stW; anchors.centerIn: parent; text: "✕ " + (wordData.wrong || 0); font.pixelSize: 10.5; font.bold: true; color: root.th.redD }
                }
                Rectangle {
                    height: 20; width: stF.implicitWidth + 12; radius: 6
                    color: "#22f5a623"
                    Text { id: stF; anchors.centerIn: parent; text: "◯ " + (wordData.fuzzy || 0); font.pixelSize: 10.5; font.bold: true; color: root.th.amberD }
                }
                Rectangle {
                    height: 20; width: stK.implicitWidth + 12; radius: 6
                    color: "#1f34c77b"
                    Text { id: stK; anchors.centerIn: parent; text: "✓ " + (wordData.know || 0); font.pixelSize: 10.5; font.bold: true; color: root.th.greenD }
                }

                Rectangle {
                    visible: showDeckBadge && (wordData.deckName || "") !== ""
                    height: 20; width: dkTxt.implicitWidth + 12; radius: 5
                    color: root.th.cardSub
                    Text { id: dkTxt; anchors.centerIn: parent; text: wordData.deckName || ""; font.pixelSize: 10; font.bold: true; color: root.th.primaryD }
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    height: 22
                    width: nrTxt.implicitWidth + 14
                    radius: 7
                    color: wordData.nextReviewCls === "due" ? "#1ff06b6b"
                         : (wordData.nextReviewCls === "soon" ? "#22f5a623"
                         : (wordData.nextReviewCls === "later" ? "#1a34c77b" : root.th.cardSub))
                    border.color: wordData.nextReviewCls === "due" ? "#47f06b6b"
                                : (wordData.nextReviewCls === "soon" ? "#4df5a623"
                                : (wordData.nextReviewCls === "later" ? "#4234c77b" : root.th.borderHi))
                    Text {
                        id: nrTxt
                        anchors.centerIn: parent
                        text: wordData.nextReviewText || ""
                        font.pixelSize: 10.5
                        font.bold: true
                        color: wordData.nextReviewCls === "due" ? root.th.redD
                             : (wordData.nextReviewCls === "soon" ? root.th.amberD
                             : (wordData.nextReviewCls === "later" ? root.th.greenD : root.th.primaryD))
                    }
                }
            }
        }
    }
}
