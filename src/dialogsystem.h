#pragma once

#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_string.h"
#include "bn_vector.h"

#include <memory>

#include "aspr_anim.h"

namespace rpg
{

// Position/system tags kept by the framework.
enum class dialog_box_type
{
    left,
    right,
    hethong
};

// Visual speech mode is separate from left/right so the framework can keep
// the three position tags while still supporting speech, thought and shout.
enum class dialog_style
{
    speech,
    think,
    shout
};

enum class avatar_id
{
    none,
    actor1_avt,
    actor2_avt
};

class dialog_system
{
public:
    explicit dialog_system(bn::sprite_text_generator& text_gen);

    void show(dialog_box_type box, dialog_style style, avatar_id avatar,
              const bn::string<32>& name, const bn::string<256>& text);

    // Convenience overload: regular speech.
    void show(dialog_box_type box, avatar_id avatar,
              const bn::string<32>& name, const bn::string<256>& text);

    void hide();
    void reset();
    void update();
    void finish_text();

    bool is_finished() const;

private:
    bn::sprite_text_generator& _text_gen;
    bn::optional<bn::regular_bg_ptr> _bg;
    bn::optional<bn::sprite_ptr> _avatar_top;
    bn::optional<bn::sprite_ptr> _avatar_bottom;
    bn::vector<bn::sprite_ptr, 64> _text_sprites;
    bn::vector<bn::sprite_ptr, 8> _name_sprites;

    bn::string<256> _current_text;
    int _current_char = 0;
    int _unicode_char_count = 0;
    int _char_timer = 0;
    static constexpr int chars_per_frame = 1;
    static constexpr int frame_delay = 2;
    dialog_box_type _current_box = dialog_box_type::hethong;
    dialog_style _current_style = dialog_style::speech;
    dialog_box_type _loaded_bg_box = dialog_box_type::hethong;
    dialog_style _loaded_bg_style = dialog_style::speech;
    bool _has_loaded_bg = false;
    std::unique_ptr<aspr_anim::AsprBlinker> _aspr_blinker;

    void _clear_content(bool keep_bg);
    void _load_bg(dialog_box_type box, dialog_style style);
    void _load_avatar(dialog_box_type box, avatar_id avatar);
    void _draw_name(dialog_box_type box, const bn::string<32>& name);
    void _draw_text(dialog_box_type box, const bn::string<256>& text);
    int _count_unicode_chars(const bn::string<256>& text) const;
    bn::string<256> _substring_unicode(const bn::string<256>& text, int char_count) const;
};

}
