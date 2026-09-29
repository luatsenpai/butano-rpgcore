#pragma once

#include "bn_optional.h"
#include "bn_sprite_ptr.h"

namespace aspr_anim
{

class AsprBlinker
{
public:
    AsprBlinker(int x, int y, int priority = 2);

    void update();
    void hide();
    void show();

private:
    bn::optional<bn::sprite_ptr> _sprite;
    int _counter = 0;
};

}
