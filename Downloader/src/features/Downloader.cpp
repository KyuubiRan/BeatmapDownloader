#include "pch.h"
#include "Downloader.h"

#include "config/I18nManager.h"
#include "misc/Color.h"
#include "utils/gui_utils.h"

features::Downloader::Downloader() :
    Feature(),
    f_GrantOsuAccount("GrantOsuAccount", false),
    f_Mirror("DownloadMirror", DownloadMirror::Sayobot) {
}

void features::Downloader::drawMain() {
    auto &lang = i18n::I18nManager::GetInstance();
    ImGui::Checkbox(lang.GetTextCStr("GrantOsuAccount"), f_GrantOsuAccount.getPtr());
    GuiHelper::ShowTooltip(lang.GetTextCStr("GrantOsuAccountDesc"));

    static const char *mirrorNames[] = {
        "Osu! (official)",
        "Sayobot",
        // "Chimu",
    };
    static int mirrorIdx = (int)f_Mirror.getValue();
    if (ImGui::Combo(lang.GetTextCStr("Mirror"), &mirrorIdx, mirrorNames, IM_ARRAYSIZE(mirrorNames))) {
        f_Mirror.setValue(static_cast<DownloadMirror>(mirrorIdx));
    }

    if (f_Mirror.getValue() == DownloadMirror::OsuOfficial && !f_GrantOsuAccount.getValue()) {
        ImGui::TextColored(color::Orange, "%s", lang.GetTextCStr("NotGrantOsuAccountButUseOfficialWarn"));
    }
}

ui::main::FeatureInfo& features::Downloader::getInfo() {
    static ui::main::FeatureInfo info = {"Downloader", "Download"};
    return info;
}
