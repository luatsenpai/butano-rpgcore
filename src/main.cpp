#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_log.h"
#include "bn_math.h"
#include "bn_sprite_text_generator.h"
#include "bn_string.h"

#include "actor.h"
#include "common_variable_8x16_sprite_font.h"
#include "dialogsystem.h"
#include "map.h"

namespace
{

using namespace rpg;

// Both actors spawn on clear terrain in overworld. actor1 only needs to move
// one tile upward, face actor2 and press A to start the interaction.
constexpr tile_pos actor1_spawn = { 8, 12 };
constexpr tile_pos actor2_spawn = { 8, 10 };
constexpr int movement_speed_px = 1;

bool actor1_is_facing_actor2(const actor& actor1)
{
    int tx = actor1_to_tile_x(actor1.sprite().x());
    int ty = actor1_to_tile_y(actor1.sprite().y());

    switch(actor1.get_dir())
    {
        case dir::up:    --ty; break;
        case dir::down:  ++ty; break;
        case dir::left:  --tx; break;
        case dir::right: ++tx; break;
    }

    return tx == actor2_spawn.x && ty == actor2_spawn.y;
}

void play_line(dialog_system& dialog, dialog_box_type tag, dialog_style style,
               avatar_id avatar, const bn::string<32>& name,
               const bn::string<256>& text)
{
    dialog.show(tag, style, avatar, name, text);

    while(true)
    {
        dialog.update();

        if(bn::keypad::a_pressed())
        {
            if(dialog.is_finished())
            {
                dialog.hide();
                bn::core::update();
                break;
            }

            dialog.finish_text();
        }

        bn::core::update();
    }
}

void run_intro(dialog_system& dialog)
{
    BN_LOG("intro dialog start");

    play_line(dialog, dialog_box_type::left, dialog_style::think,
              avatar_id::actor1_avt, "actor1", "Đây là nơi quái nào thế?");
    play_line(dialog, dialog_box_type::left, dialog_style::think,
              avatar_id::actor1_avt, "actor1", "Đằng kia có người, lại hỏi xem");

    dialog.reset();
    BN_LOG("intro dialog end; player control enabled");
}

void run_actor2_dialog(dialog_system& dialog)
{
    BN_LOG("actor2 interaction start");

    play_line(dialog, dialog_box_type::left, dialog_style::speech,
              avatar_id::actor1_avt, "actor1", "Chào, đây là đâu vậy?");
    play_line(dialog, dialog_box_type::right, dialog_style::think,
              avatar_id::actor2_avt, "actor2", "Hỏi gì lạ vậy?");
    play_line(dialog, dialog_box_type::right, dialog_style::speech,
              avatar_id::actor2_avt, "actor2", "Đây là thế giới game");
    play_line(dialog, dialog_box_type::left, dialog_style::shout,
              avatar_id::actor1_avt, "actor1", "Hả?");
    play_line(dialog, dialog_box_type::right, dialog_style::shout,
              avatar_id::actor2_avt, "actor2", "Có gì ngạc nhiên mà hét?");

    dialog.reset();
    BN_LOG("actor2 interaction end");
}

}

int main()
{
    bn::core::init();
    BN_LOG("rc_framework overworld boot");

    rpg::world_map world;
    rpg::actor actor1(rpg::actor_id::actor1,
                      rpg::actor1_tile_center_x(actor1_spawn.x),
                      rpg::actor1_tile_center_y(actor1_spawn.y),
                      world.camera());

    rpg::actor actor2(rpg::actor_id::actor2,
                      rpg::tile_center_x(actor2_spawn.x),
                      rpg::tile_center_y(actor2_spawn.y),
                      world.camera());
    actor2.set_dir(rpg::dir::down);
    actor2.set_idle_frame();

    bn::sprite_text_generator text_gen(common::variable_8x16_sprite_font);
    rpg::dialog_system dialog(text_gen);

    int move_left_px = 0;
    int dx = 0;
    int dy = 0;
    bool actor2_dialog_finished = false;

    world.sync_camera_to_actor1(actor1.sprite().x(), actor1.sprite().y());
    rpg::reset_actor1_tile_debug_log();
    rpg::debug_log_actor1_tile(actor1.sprite().x(), actor1.sprite().y());

    // Required opening thoughts play once before normal movement begins.
    run_intro(dialog);

    while(true)
    {
        if(! actor2_dialog_finished && move_left_px == 0 &&
           bn::keypad::a_pressed() && actor1_is_facing_actor2(actor1))
        {
            actor1.stop_walk();
            run_actor2_dialog(dialog);
            actor2_dialog_finished = true;
        }

        if(move_left_px == 0)
        {
            dx = 0;
            dy = 0;

            if(bn::keypad::up_held())
            {
                dy = -1;
                actor1.set_dir(rpg::dir::up);
            }
            else if(bn::keypad::down_held())
            {
                dy = 1;
                actor1.set_dir(rpg::dir::down);
            }
            else if(bn::keypad::left_held())
            {
                dx = -1;
                actor1.set_dir(rpg::dir::left);
            }
            else if(bn::keypad::right_held())
            {
                dx = 1;
                actor1.set_dir(rpg::dir::right);
            }

            if(dx != 0 || dy != 0)
            {
                const bn::fixed next_x = dx != 0 ?
                        rpg::clamp_to_map_x(actor1.sprite().x() + bn::fixed(dx * rpg::actor::step_px)) :
                        actor1.sprite().x();
                const bn::fixed next_y = dy != 0 ?
                        rpg::clamp_to_map_y(actor1.sprite().y() + bn::fixed(dy * rpg::actor::step_px)) :
                        actor1.sprite().y();

                if(! rpg::is_blocked_for_actor1(next_x, next_y, actor2_spawn))
                {
                    move_left_px = rpg::actor::step_px;
                    actor1.start_walk();
                }
                else
                {
                    actor1.stop_walk();
                }
            }
            else
            {
                actor1.stop_walk();
            }
        }

        if(move_left_px > 0)
        {
            const int step = bn::min(movement_speed_px, move_left_px);

            if(dx != 0)
            {
                actor1.sprite().set_x(rpg::clamp_to_map_x(actor1.sprite().x() + bn::fixed(dx * step)));
            }
            if(dy != 0)
            {
                actor1.sprite().set_y(rpg::clamp_to_map_y(actor1.sprite().y() + bn::fixed(dy * step)));
            }

            move_left_px -= step;
        }
        else
        {
            actor1.set_idle_frame();
        }

        actor1.update_anim();
        world.sync_camera_to_actor1(actor1.sprite().x(), actor1.sprite().y());
        rpg::debug_log_actor1_tile(actor1.sprite().x(), actor1.sprite().y());

        bn::core::update();
    }
}
