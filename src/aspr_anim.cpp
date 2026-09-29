#include "aspr_anim.h"

#include "bn_sprite_items_aspr.h"
#include "bn_sprites.h"

namespace aspr_anim
{

AsprBlinker::AsprBlinker(int x, int y, int) :
    _sprite(bn::sprite_items::aspr.create_sprite(x, y))
{
    _sprite->set_bg_priority(0);
    _sprite->set_z_order(bn::sprites::max_z_order());
    _sprite->set_visible(false);
}

void AsprBlinker::update()
{
    if(! _sprite || ! _sprite->visible())
    {
        return;
    }

    ++_counter;
    if(_counter % 30 == 0)
    {
        const int tile_index = (_counter / 30) % 2;
        _sprite->set_tiles(bn::sprite_items::aspr.tiles_item(), tile_index);
    }
}

void AsprBlinker::hide()
{
    if(_sprite)
    {
        _sprite->set_visible(false);
    }
}

void AsprBlinker::show()
{
    if(_sprite)
    {
        _sprite->set_visible(true);
        _sprite->set_tiles(bn::sprite_items::aspr.tiles_item(), 0);
        _counter = 0;
    }
}

}
