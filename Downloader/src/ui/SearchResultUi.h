#pragma once
#include "osu/Beatmap.h"

namespace ui::search {
bool IsShowed();
void ShowSearchInfo(osu::Beatmap &bm);
void Update();
}


