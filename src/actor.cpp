#include "actor.h"

#include "bn_sprite_items_actor1.h"
#include "bn_sprite_items_actor2.h"
#include "bn_sprites.h"

namespace rpg
{

namespace
{

bn::sprite_ptr create_actor_sprite(actor_id id, bn::fixed x, bn::fixed y)
{
    if(id == actor_id::actor2)
    {
        return bn::sprite_items::actor2.create_sprite(x, y);
    }

    return bn::sprite_items::actor1.create_sprite(x, y);
}

}

actor::actor(actor_id id, bn::fixed x, bn::fixed y, const bn::camera_ptr& camera) :
    _id(id),
    _sprite(create_actor_sprite(id, x, y))
{
    _sprite.set_camera(camera);
    _sprite.set_bg_priority(2);
    _sprite.set_z_order(bn::sprites::max_z_order());
    set_dir(dir::down);
    set_idle_frame();
}

bn::sprite_ptr& actor::sprite()
{
    return _sprite;
}

const bn::sprite_ptr& actor::sprite() const
{
    return _sprite;
}

actor_id actor::id() const
{
    return _id;
}

void actor::set_dir(dir value)
{
    _dir = value;
    _sprite.set_horizontal_flip(_dir == dir::right);
}

dir actor::get_dir() const
{
    return _dir;
}

int actor::_idle_index_for(dir value) const
{
    switch(value)
    {
        case dir::up:    return 4;
        case dir::down:  return 1;
        case dir::left:  return 7;
        case dir::right: return 7;
        default:         return 4;
    }
}

void actor::_make_walk_anim()
{
    _walk_anim.reset();
    constexpr int wait_frames = 5;

    auto create_for_actor1 = [&]()
    {
        const auto& tiles = bn::sprite_items::actor1.tiles_item();
        switch(_dir)
        {
            case dir::up:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 3, 4, 5);
                break;
            case dir::down:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 0, 1, 2);
                break;
            case dir::left:
            case dir::right:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 6, 7, 8);
                break;
            default:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 3, 4, 5);
                break;
        }
    };

    auto create_for_actor2 = [&]()
    {
        const auto& tiles = bn::sprite_items::actor2.tiles_item();
        switch(_dir)
        {
            case dir::up:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 3, 4, 5);
                break;
            case dir::down:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 0, 1, 2);
                break;
            case dir::left:
            case dir::right:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 6, 7, 8);
                break;
            default:
                _walk_anim = bn::create_sprite_animate_action_forever(_sprite, wait_frames, tiles, 3, 4, 5);
                break;
        }
    };

    if(_id == actor_id::actor2)
    {
        create_for_actor2();
    }
    else
    {
        create_for_actor1();
    }
}

void actor::start_walk()
{
    _make_walk_anim();
}

void actor::stop_walk()
{
    _walk_anim.reset();
    set_idle_frame();
}

void actor::update_anim()
{
    if(_walk_anim)
    {
        _walk_anim->update();
    }
}

void actor::set_idle_frame()
{
    const int index = _idle_index_for(_dir);

    if(_id == actor_id::actor2)
    {
        _sprite.set_tiles(bn::sprite_items::actor2.tiles_item().create_tiles(index));
    }
    else
    {
        _sprite.set_tiles(bn::sprite_items::actor1.tiles_item().create_tiles(index));
    }
}

}
