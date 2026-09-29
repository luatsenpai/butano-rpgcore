#include "map.h"

#include "bn_log.h"
#include "bn_math.h"
#include "bn_regular_bg_items_overworld.h"
#include "bn_regular_bg_items_overworld_overlay.h"

namespace rpg
{

namespace
{

int debug_last_tx = -1;
int debug_last_ty = -1;

bool in_range(int value, int min_value, int max_value)
{
    return value >= min_value && value <= max_value;
}

}

bn::fixed tile_center_x(int tx)
{
    return -bn::fixed(map_half) + bn::fixed(tx * tile_px + tile_px / 2);
}

bn::fixed tile_center_y(int ty)
{
    return -bn::fixed(map_half) + bn::fixed(ty * tile_px + tile_px / 2);
}

bn::fixed actor1_tile_center_x(int tx)
{
    return tile_center_x(tx) + actor1_visual_offset_x;
}

bn::fixed actor1_tile_center_y(int ty)
{
    return tile_center_y(ty) + actor1_visual_offset_y;
}

int actor1_to_tile_x(bn::fixed sprite_x)
{
    return bn::max(0, bn::min(map_tiles - 1,
            int((sprite_x - actor1_visual_offset_x + map_half) / tile_px)));
}

int actor1_to_tile_y(bn::fixed sprite_y)
{
    return bn::max(0, bn::min(map_tiles - 1,
            int((sprite_y - actor1_visual_offset_y + map_half) / tile_px)));
}

bn::fixed clamp_to_map_x(bn::fixed value)
{
    const bn::fixed min_value = -bn::fixed(map_half - 1) + actor1_visual_offset_x;
    const bn::fixed max_value = bn::fixed(map_half - 1) + actor1_visual_offset_x;
    return bn::max(min_value, bn::min(max_value, value));
}

bn::fixed clamp_to_map_y(bn::fixed value)
{
    const bn::fixed min_value = -bn::fixed(map_half - 1) + actor1_visual_offset_y;
    const bn::fixed max_value = bn::fixed(map_half - 1) + actor1_visual_offset_y;
    return bn::max(min_value, bn::min(max_value, value));
}

bool is_blocked(int tx, int ty)
{
    // Collision from the original outdoor/houseoutside map, now exposed as
    // the single framework map named "overworld". No story/map objects are
    // instantiated; this is terrain collision only.
    if(tx == 1 && in_range(ty, 7, 13)) return true;
    if(tx == 15 && in_range(ty, 2, 14)) return true;
    if(tx == 12 && in_range(ty, 2, 7)) return true;
    if(in_range(tx, 2, 14) && in_range(ty, 14, 15)) return true;
    if(in_range(tx, 12, 14) && ty == 1) return true;
    if(in_range(tx, 8, 12) && ty == 8) return true;
    if(in_range(tx, 2, 7) && ty == 7) return true;
    if((tx == 0 && ty == 0) || (tx == 8 && ty == 7)) return true;

    return false;
}

bool is_blocked_for_actor1(bn::fixed sprite_x, bn::fixed sprite_y, tile_pos actor2_tile)
{
    const int tx = actor1_to_tile_x(sprite_x);
    const int ty = actor1_to_tile_y(sprite_y);

    if(tx == actor2_tile.x && ty == actor2_tile.y)
    {
        BN_LOG("collision: actor2 at tile x=", tx, " y=", ty);
        return true;
    }

    if(is_blocked(tx, ty))
    {
        BN_LOG("collision: overworld at tile x=", tx, " y=", ty);
        return true;
    }

    return false;
}

void reset_actor1_tile_debug_log()
{
    debug_last_tx = -1;
    debug_last_ty = -1;
}

void debug_log_actor1_tile(bn::fixed sprite_x, bn::fixed sprite_y)
{
    const int tx = actor1_to_tile_x(sprite_x);
    const int ty = actor1_to_tile_y(sprite_y);

    if(tx != debug_last_tx || ty != debug_last_ty)
    {
        BN_LOG("actor1 tile x=", tx, " y=", ty);
        debug_last_tx = tx;
        debug_last_ty = ty;
    }
}

world_map::world_map() :
    _camera(bn::camera_ptr::create(0, 0))
{
    BN_LOG("framework: loading overworld");

    _background = bn::regular_bg_items::overworld.create_bg(0, 0);
    _background->set_camera(_camera);
    _background->set_priority(3);

    // This is part of the map artwork (foreground fence), not a gameplay obj.
    _overlay = bn::regular_bg_items::overworld_overlay.create_bg(0, 0);
    _overlay->set_camera(_camera);
    _overlay->set_priority(1);
}

bn::camera_ptr& world_map::camera()
{
    return _camera;
}

const bn::camera_ptr& world_map::camera() const
{
    return _camera;
}

void world_map::sync_camera_to_actor1(bn::fixed sprite_x, bn::fixed sprite_y)
{
    bn::fixed cx = sprite_x - actor1_visual_offset_x;
    bn::fixed cy = sprite_y - actor1_visual_offset_y;

    const bn::fixed min_cx = -bn::fixed(map_half - screen_half_w);
    const bn::fixed max_cx = bn::fixed(map_half - screen_half_w);
    const bn::fixed min_cy = -bn::fixed(map_half - screen_half_h);
    const bn::fixed max_cy = bn::fixed(map_half - screen_half_h);

    cx = bn::max(min_cx, bn::min(max_cx, cx));
    cy = bn::max(min_cy, bn::min(max_cy, cy));
    _camera.set_position(cx, cy);
}

}
