.pragma library

var themes = [
    {
        id: "tech", name: "星际探索",
        bgBase: "#eef2f9", card: "#ffffff", cardSub: "#f7f9fd", cardHover: "#f8faff",
        border: "#225b8def", borderHi: "#475b8def",
        text: "#1a2233", text2: "#485878", muted: "#8d9bb5", faint: "#b8c3d6",
        primary: "#5b8def", primaryD: "#3d6ed0", accent: "#7c6ee6", accent2: "#3bc9db",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#5b8def", c2: "#7c6ee6"
    },
    {
        id: "midnight", name: "霓虹都市",
        bgBase: "#080b14", card: "#111725", cardSub: "#0d1220", cardHover: "#1a2233",
        border: "#18ffffff", borderHi: "#4d22d3ee",
        text: "#e2e8f0", text2: "#9aa8c0", muted: "#6b7a94", faint: "#485468",
        primary: "#22d3ee", primaryD: "#67e8f9", accent: "#e879f9", accent2: "#60a5fa",
        onBtn: "#0b1220", greenD: "#34d399", redD: "#f87171", amberD: "#fbbf24",
        c1: "#22d3ee", c2: "#e879f9"
    },
    {
        id: "sakura", name: "恋爱物语",
        bgBase: "#fdf3f7", card: "#ffffff", cardSub: "#fdf6fa", cardHover: "#fef8fc",
        border: "#24f472b6", borderHi: "#4df472b6",
        text: "#3d2430", text2: "#6b4d5a", muted: "#a48a97", faint: "#d0bcc6",
        primary: "#ec4899", primaryD: "#be185d", accent: "#a78bfa", accent2: "#fb7185",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#ec4899", c2: "#a78bfa"
    },
    {
        id: "forest", name: "森之精灵",
        bgBase: "#eff7f2", card: "#ffffff", cardSub: "#f5faf7", cardHover: "#f7fcf9",
        border: "#2610b981", borderHi: "#5210b981",
        text: "#13281f", text2: "#395a4a", muted: "#7d9a8c", faint: "#b5cbbf",
        primary: "#10b981", primaryD: "#047857", accent: "#14b8a6", accent2: "#84cc16",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#10b981", c2: "#14b8a6"
    },
    {
        id: "cream", name: "甜品工坊",
        bgBase: "#faf4e8", card: "#fffdf7", cardSub: "#faf4e6", cardHover: "#fdf9ee",
        border: "#26b48214", borderHi: "#52b48214",
        text: "#3d2c14", text2: "#6b5535", muted: "#a08a68", faint: "#cfbda0",
        primary: "#f59e0b", primaryD: "#b45309", accent: "#d97706", accent2: "#fb7185",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#f59e0b", c2: "#d97706"
    },
    {
        id: "ocean", name: "深海冒险",
        bgBase: "#e9f4fc", card: "#ffffff", cardSub: "#f2f8fd", cardHover: "#f5fafd",
        border: "#260ea5e9", borderHi: "#520ea5e9",
        text: "#0c2739", text2: "#325a75", muted: "#7a99b0", faint: "#b2c8d8",
        primary: "#0ea5e9", primaryD: "#0369a1", accent: "#06b6d4", accent2: "#3b82f6",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#0ea5e9", c2: "#06b6d4"
    },
    {
        id: "cyber", name: "机械义体",
        bgBase: "#10061e", card: "#1a0c2e", cardSub: "#150824", cardHover: "#221040",
        border: "#24f0abfc", borderHi: "#59f0abfc",
        text: "#f5eaff", text2: "#b8a3d0", muted: "#7d6a9a", faint: "#4e3f6a",
        primary: "#e879f9", primaryD: "#f0abfc", accent: "#fde047", accent2: "#22d3ee",
        onBtn: "#ffffff", greenD: "#34d399", redD: "#f87171", amberD: "#fbbf24",
        c1: "#e879f9", c2: "#fde047"
    },
    {
        id: "mono", name: "水墨东方",
        bgBase: "#f5f5f5", card: "#ffffff", cardSub: "#fafafa", cardHover: "#f5f5f5",
        border: "#18000000", borderHi: "#33000000",
        text: "#0a0a0a", text2: "#404040", muted: "#737373", faint: "#a3a3a3",
        primary: "#18181b", primaryD: "#000000", accent: "#52525b", accent2: "#71717a",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#18181b", c2: "#52525b"
    },
    {
        id: "sunset", name: "仙侠古风",
        bgBase: "#fdf0e7", card: "#ffffff", cardSub: "#fdf5ee", cardHover: "#fef9f4",
        border: "#24db2777", borderHi: "#4ddb2777",
        text: "#3d1a0f", text2: "#6e4332", muted: "#a37e6d", faint: "#d1b5a6",
        primary: "#f97316", primaryD: "#c2410c", accent: "#db2777", accent2: "#fbbf24",
        onBtn: "#ffffff", greenD: "#1aa562", redD: "#e05555", amberD: "#d99000",
        c1: "#f97316", c2: "#db2777"
    }
];

function getTheme(id) {
    for (var i = 0; i < themes.length; ++i) {
        if (themes[i].id === id) return themes[i];
    }
    return themes[0];
}
