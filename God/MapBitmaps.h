#pragma once

#include "Mapper.h"

inline void saveMapBitmaps(PACKFILE* out) {
	PALETTE palette;
	get_palette(palette);
	save_pcx_pf(out, bmpMap, palette);
	for (int i = 1; i <= MapSize; i++) {
		save_pcx_pf(out, pFirstFog[i].Pic, palette);
	}
}

inline void loadMapBitmaps(PACKFILE* out) {
	PALETTE buf;
	bmpMap = load_pcx_pf(out, buf);
	for (int i = 1; i <= MapSize; i++) {
		pFirstFog[i].Pic = load_pcx_pf(out, buf);
	}
}
