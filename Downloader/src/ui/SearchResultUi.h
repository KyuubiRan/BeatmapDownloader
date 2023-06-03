#pragma once
#include "osu/Beatmap.h"

namespace ui::search::result {
bool IsShowed();
void ShowSearchInfo(osu::Beatmap &bm);
void Update();
}


