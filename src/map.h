#pragma once

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"

namespace rpg
{

constexpr int map_px = 256;
constexpr int map_half = map_px / 2;
constexpr int tile_px = 16;
constexpr int map_tiles = map_px / tile_px;
constexpr int screen_half_w = 120;
constexpr int screen_half_h = 80;

// actor1 keeps the original outdoor character alignment: the logical tile
// is 7 px above the sprite center.
constexpr int actor1_visual_offset_x = 0;
constexpr int actor1_visual_offset_y = 7;

struct tile_pos
{
    int x;
    int y;
};

bn::fixed tile_center_x(int tx);
bn::fixed tile_center_y(int ty);
bn::fixed actor1_tile_center_x(int tx);
bn::fixed actor1_tile_center_y(int ty);
int actor1_to_tile_x(bn::fixed sprite_x);
int actor1_to_tile_y(bn::fixed sprite_y);

bn::fixed clamp_to_map_x(bn::fixed value);
bn::fixed clamp_to_map_y(bn::fixed value);

bool is_blocked(int tx, int ty);
bool is_blocked_for_actor1(bn::fixed sprite_x, bn::fixed sprite_y, tile_pos actor2_tile);
void debug_log_actor1_tile(bn::fixed sprite_x, bn::fixed sprite_y);
void reset_actor1_tile_debug_log();

class world_map
{
public:
    world_map();

    bn::camera_ptr& camera();
    const bn::camera_ptr& camera() const;
    void sync_camera_to_actor1(bn::fixed sprite_x, bn::fixed sprite_y);

private:
    bn::camera_ptr _camera;
    bn::optional<bn::regular_bg_ptr> _background;
    bn::optional<bn::regular_bg_ptr> _overlay;
};

}
