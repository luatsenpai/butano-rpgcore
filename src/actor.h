#pragma once

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_sprite_animate_actions.h"
#include "bn_sprite_ptr.h"

namespace rpg
{

enum class actor_id
{
    actor1,
    actor2
};

enum class dir
{
    up,
    down,
    left,
    right
};

class actor
{
public:
    static constexpr int tile_px = 16;
    static constexpr int step_px = tile_px;

    actor(actor_id id, bn::fixed x, bn::fixed y, const bn::camera_ptr& camera);

    bn::sprite_ptr& sprite();
    const bn::sprite_ptr& sprite() const;

    actor_id id() const;
    void set_dir(dir value);
    dir get_dir() const;

    void start_walk();
    void stop_walk();
    void update_anim();
    void set_idle_frame();

private:
    actor_id _id;
    bn::sprite_ptr _sprite;
    dir _dir = dir::down;
    bn::optional<bn::sprite_animate_action<3>> _walk_anim;

    int _idle_index_for(dir value) const;
    void _make_walk_anim();
};

}
