/*
Copyright © 2011-2012 Clint Bellanger
Copyright © 2012 Igor Paliychuk
Copyright © 2012 Stefan Beller
Copyright © 2013 Henrik Andersson
Copyright © 2012-2016 Justin Jacobs

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

/**
 * class FogOfWar
 *
 * Contains logic and rendering routines for fog of war.
 */

#include "Avatar.h"
#include "EngineSettings.h"
#include "FogOfWar.h"
#include "MapRenderer.h"
#include "Menu.h"
#include "MenuManager.h"
#include "MenuMiniMap.h"
#include "SharedGameResources.h"
#include "SharedResources.h"

FogOfWar::FogOfWar()
	: layer_id(0)
	, tileset("tilesetdefs/tileset_fogofwar.txt")
	, bounds(0,0,0,0)
	, color_sight(255,255,255)
	, color_visited(128,128,128)
	, color_hidden(0,0,0)
	, update_minimap(true) {
}

int FogOfWar::load() {
	tset.load(tileset);
	return 0;
}

void FogOfWar::logic() {
	calcBoundaries();
	updateTiles(TILE_SIGHT);
	if (update_minimap) {
		menu->mini->update(&mapr->collider, &bounds);
		update_minimap = false;
	}
}

void FogOfWar::handleIntramapTeleport() {
	calcBoundaries();
	updateTiles(TILE_VISITED);
	if (update_minimap) {
		menu->mini->update(&mapr->collider, &bounds);
		update_minimap = false;
	}
}

Color FogOfWar::getTileColorMod(const int_fast16_t x, const int_fast16_t y) {
	if (mapr->layers[layer_id][x][y] == TILE_VISITED || mapr->layers[layer_id][x][y] == TILE_HIDDEN_BOUNDARY)
		return color_visited;
	else if (mapr->layers[layer_id][x][y] == TILE_HIDDEN)
		return color_hidden;
	else
		return color_sight;
}

void FogOfWar::calcBoundaries() {
	bounds.x = static_cast<short>(pc->stats.pos.x-pc->sight-2);
	bounds.y = static_cast<short>(pc->stats.pos.y-pc->sight-2);
	bounds.w = static_cast<short>(pc->stats.pos.x+pc->sight+2);
	bounds.h = static_cast<short>(pc->stats.pos.y+pc->sight+2);

	if (bounds.x < 0) bounds.x = 0;
	if (bounds.y < 0) bounds.y = 0;
	if (bounds.w > mapr->w) bounds.w = mapr->w;
	if (bounds.h > mapr->h) bounds.h = mapr->h;
}

void FogOfWar::updateTiles(unsigned short sight_tile) {
	for (int x = bounds.x; x < bounds.w; x++) {
		for (int y = bounds.y; y < bounds.h; y++) {
			Point tile(x, y);
			float delta = Utils::calcDist(FPoint(tile), FPoint(pc->stats.pos));
			unsigned short prev_tile = mapr->layers[layer_id][x][y];
			if (delta < pc->sight) {
				mapr->layers[layer_id][x][y] = sight_tile;
			}
			else if (mapr->layers[layer_id][x][y] == TILE_SIGHT) {
				mapr->layers[layer_id][x][y] = TILE_VISITED;
			}
			if ((prev_tile == TILE_HIDDEN || prev_tile == TILE_HIDDEN_BOUNDARY) && prev_tile != mapr->layers[layer_id][x][y]) {
				update_minimap = true; 
			}
		}
	}
	
	if (eset->misc.fogofwar == TYPE_OVERLAY) {
		for (int x = bounds.x; x < bounds.w; x++) {
			for (int y = bounds.y; y < bounds.h; y++) {
				if (mapr->layers[layer_id][x][y] == TILE_HIDDEN) {
					for (int i=x-1; i<=x+1; i++) {
						for (int j=y-1; j<=y+1; j++) {
							if (i>=0 && j>=0 && i<mapr->w && j<mapr->h) {
								if (mapr->layers[layer_id][i][j] == TILE_SIGHT || mapr->layers[layer_id][i][j] == TILE_VISITED) {
									mapr->layers[layer_id][x][y] = TILE_HIDDEN_BOUNDARY;
									continue;
								}
							}
						}
					}
				}
			}
		}
	}
}

FogOfWar::~FogOfWar() {
}
