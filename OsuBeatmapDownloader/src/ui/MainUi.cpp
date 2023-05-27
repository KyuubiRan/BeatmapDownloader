#include "pch.h"
#include "MainUi.h"

namespace ui::main {

static bool show = true;

bool IsShowed() {
    return show;
}

void ToggleShow() {
    show = !show;
}

void Update() {
    ImGuiIO &io = ImGui::GetIO();
    io.MouseDrawCursor = show;
    if (!show) return;
    ImGui::Begin("Osu! Beatmap Downloader");
    ImGui::Text("Hello, world!");
    ImGui::End();
}

}