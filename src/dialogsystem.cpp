#include "dialogsystem.h"

#include "bn_log.h"
#include "bn_regular_bg_items_dialog.h"
#include "bn_regular_bg_items_shout_leftbg.h"
#include "bn_regular_bg_items_shout_rightbg.h"
#include "bn_regular_bg_items_speech_leftbg.h"
#include "bn_regular_bg_items_speech_rightbg.h"
#include "bn_regular_bg_items_think_leftbg.h"
#include "bn_regular_bg_items_think_rightbg.h"
#include "bn_sprite_items_actor1_avt.h"
#include "bn_sprite_items_actor2_avt.h"
#include "bn_sprite_items_variable_8x16_font_red.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_sprites.h"

#include "common_variable_8x16_sprite_font.h"

namespace rpg
{

namespace
{

bool is_left_box(dialog_box_type box)
{
    return box == dialog_box_type::left || box == dialog_box_type::hethong;
}

}

dialog_system::dialog_system(bn::sprite_text_generator& text_gen) :
    _text_gen(text_gen)
{
    _text_gen.set_left_alignment();
    _text_gen.set_bg_priority(0);

    _bg = bn::regular_bg_items::dialog.create_bg(0, 0);
    _bg->set_priority(0);
    _bg->set_visible(false);
}

void dialog_system::show(dialog_box_type box, dialog_style style, avatar_id avatar,
                         const bn::string<32>& name, const bn::string<256>& text)
{
    if(_aspr_blinker)
    {
        _aspr_blinker->hide();
        _aspr_blinker.reset();
    }

    _avatar_top.reset();
    _avatar_bottom.reset();
    _text_sprites.clear();
    _name_sprites.clear();

    _current_text = text;
    _current_char = 0;
    _char_timer = 0;
    _unicode_char_count = _count_unicode_chars(text);
    _current_box = box;
    _current_style = style;

    _load_bg(box, style);
    _load_avatar(box, avatar);
    _draw_name(box, name);
    _draw_text(box, "");

    BN_LOG("dialog show tag=", int(box), " style=", int(style), " avatar=", int(avatar));
}

void dialog_system::show(dialog_box_type box, avatar_id avatar,
                         const bn::string<32>& name, const bn::string<256>& text)
{
    show(box, dialog_style::speech, avatar, name, text);
}

void dialog_system::hide()
{
    _clear_content(true);
}

void dialog_system::reset()
{
    _clear_content(false);
    _loaded_bg_box = dialog_box_type::hethong;
    _loaded_bg_style = dialog_style::speech;
    _has_loaded_bg = false;
}

bool dialog_system::is_finished() const
{
    return _current_char >= _unicode_char_count;
}

void dialog_system::_clear_content(bool keep_bg)
{
    if(_aspr_blinker)
    {
        _aspr_blinker->hide();
        _aspr_blinker.reset();
    }

    if(_bg)
    {
        _bg->set_visible(false);
        if(! keep_bg)
        {
            _bg.reset();
        }
    }

    _avatar_top.reset();
    _avatar_bottom.reset();
    _text_sprites.clear();
    _name_sprites.clear();
    _current_text.clear();
    _current_char = 0;
    _unicode_char_count = 0;
    _char_timer = 0;
}

void dialog_system::update()
{
    if(_current_char < _unicode_char_count)
    {
        ++_char_timer;
        if(_char_timer >= frame_delay)
        {
            _current_char += chars_per_frame;
            _char_timer = 0;
            if(_current_char > _unicode_char_count)
            {
                _current_char = _unicode_char_count;
            }

            _text_sprites.clear();
            _draw_text(_current_box, _substring_unicode(_current_text, _current_char));
        }
    }
    else
    {
        if(! _aspr_blinker)
        {
            const int x = is_left_box(_current_box) ? 92 : 57;
            _aspr_blinker = std::make_unique<aspr_anim::AsprBlinker>(x, 70, 2);
            _aspr_blinker->show();
        }
        else
        {
            _aspr_blinker->update();
        }
    }
}

void dialog_system::finish_text()
{
    if(_current_char < _unicode_char_count)
    {
        _current_char = _unicode_char_count;
        _text_sprites.clear();
        _draw_text(_current_box, _current_text);
    }

    if(! _aspr_blinker)
    {
        const int x = is_left_box(_current_box) ? 92 : 57;
        _aspr_blinker = std::make_unique<aspr_anim::AsprBlinker>(x, 70, 2);
        _aspr_blinker->show();
    }
}

int dialog_system::_count_unicode_chars(const bn::string<256>& text) const
{
    int count = 0;

    for(int i = 0; i < text.size();)
    {
        const unsigned char c = text[i];

        if(c == '\\' && i + 1 < text.size())
        {
            if(text[i + 1] == 'n')
            {
                i += 2;
                ++count;
                continue;
            }

            if(text[i + 1] == 'e' && i + 2 < text.size() && text[i + 2] == '(')
            {
                i += 3;
                while(i < text.size() && text[i] != ')')
                {
                    const unsigned char inner = text[i];
                    if(inner <= 0x7F) i += 1;
                    else if((inner & 0xE0) == 0xC0 && i + 1 < text.size()) i += 2;
                    else if((inner & 0xF0) == 0xE0 && i + 2 < text.size()) i += 3;
                    else i += 1;
                    ++count;
                }

                if(i < text.size() && text[i] == ')')
                {
                    ++i;
                }
                continue;
            }
        }

        if(c <= 0x7F) i += 1;
        else if((c & 0xE0) == 0xC0 && i + 1 < text.size()) i += 2;
        else if((c & 0xF0) == 0xE0 && i + 2 < text.size()) i += 3;
        else i += 1;
        ++count;
    }

    return count;
}

bn::string<256> dialog_system::_substring_unicode(const bn::string<256>& text, int char_count) const
{
    bn::string<256> result;
    int current_char = 0;

    for(int i = 0; i < text.size() && current_char < char_count;)
    {
        const unsigned char c = text[i];

        if(c == '\\' && i + 1 < text.size())
        {
            if(text[i + 1] == 'n')
            {
                result += c;
                result += text[i + 1];
                i += 2;
                ++current_char;
                continue;
            }

            if(text[i + 1] == 'e' && i + 2 < text.size() && text[i + 2] == '(')
            {
                result += c;
                result += text[i + 1];
                result += text[i + 2];
                i += 3;

                while(i < text.size() && text[i] != ')' && current_char < char_count)
                {
                    const unsigned char inner = text[i];
                    result += text[i];
                    if(inner <= 0x7F)
                    {
                        i += 1;
                    }
                    else if((inner & 0xE0) == 0xC0 && i + 1 < text.size())
                    {
                        result += text[i + 1];
                        i += 2;
                    }
                    else if((inner & 0xF0) == 0xE0 && i + 2 < text.size())
                    {
                        result += text[i + 1];
                        result += text[i + 2];
                        i += 3;
                    }
                    else
                    {
                        i += 1;
                    }
                    ++current_char;
                }

                if(i < text.size() && text[i] == ')')
                {
                    result += text[i];
                    ++i;
                }
                continue;
            }
        }

        result += text[i];
        if(c <= 0x7F)
        {
            i += 1;
        }
        else if((c & 0xE0) == 0xC0 && i + 1 < text.size())
        {
            result += text[i + 1];
            i += 2;
        }
        else if((c & 0xF0) == 0xE0 && i + 2 < text.size())
        {
            result += text[i + 1];
            result += text[i + 2];
            i += 3;
        }
        else
        {
            i += 1;
        }
        ++current_char;
    }

    return result;
}

void dialog_system::_load_bg(dialog_box_type box, dialog_style style)
{
    if(_bg && _has_loaded_bg && _loaded_bg_box == box && _loaded_bg_style == style)
    {
        _bg->set_visible(true);
        return;
    }

    _bg.reset();

    if(box == dialog_box_type::hethong)
    {
        _bg = bn::regular_bg_items::dialog.create_bg(0, 0);
    }
    else if(box == dialog_box_type::left)
    {
        switch(style)
        {
            case dialog_style::speech:
                _bg = bn::regular_bg_items::speech_leftbg.create_bg(0, 0);
                break;
            case dialog_style::think:
                _bg = bn::regular_bg_items::think_leftbg.create_bg(0, 0);
                break;
            case dialog_style::shout:
                _bg = bn::regular_bg_items::shout_leftbg.create_bg(0, 0);
                break;
            default:
                _bg = bn::regular_bg_items::speech_leftbg.create_bg(0, 0);
                break;
        }
    }
    else
    {
        switch(style)
        {
            case dialog_style::speech:
                _bg = bn::regular_bg_items::speech_rightbg.create_bg(0, 0);
                break;
            case dialog_style::think:
                _bg = bn::regular_bg_items::think_rightbg.create_bg(0, 0);
                break;
            case dialog_style::shout:
                _bg = bn::regular_bg_items::shout_rightbg.create_bg(0, 0);
                break;
            default:
                _bg = bn::regular_bg_items::speech_rightbg.create_bg(0, 0);
                break;
        }
    }

    _loaded_bg_box = box;
    _loaded_bg_style = style;
    _has_loaded_bg = true;
    _bg->set_priority(0);
    _bg->set_visible(true);
}

void dialog_system::_load_avatar(dialog_box_type box, avatar_id avatar)
{
    _avatar_top.reset();
    _avatar_bottom.reset();

    if(avatar == avatar_id::none)
    {
        return;
    }

    const bn::fixed x = is_left_box(box) ? bn::fixed(-100) : bn::fixed(100);
    constexpr int y_bottom = 65;
    constexpr int y_top = y_bottom - 64;

    auto assign_tiles = [&](const auto& item)
    {
        _avatar_bottom = item.create_sprite(x, y_bottom);
        _avatar_top = item.create_sprite(x, y_top);
        const auto& tiles = item.tiles_item();
        _avatar_top->set_tiles(tiles.create_tiles(0));
        _avatar_bottom->set_tiles(tiles.create_tiles(1));
        _avatar_top->set_bg_priority(1);
        _avatar_bottom->set_bg_priority(1);
        _avatar_top->set_z_order(bn::sprites::max_z_order());
        _avatar_bottom->set_z_order(bn::sprites::max_z_order());
    };

    switch(avatar)
    {
        case avatar_id::actor1_avt:
            assign_tiles(bn::sprite_items::actor1_avt);
            break;
        case avatar_id::actor2_avt:
            assign_tiles(bn::sprite_items::actor2_avt);
            break;
        case avatar_id::none:
            break;
        default:
            BN_LOG("unknown avatar id: ", int(avatar));
            break;
    }
}

void dialog_system::_draw_name(dialog_box_type box, const bn::string<32>& name)
{
    _name_sprites.clear();
    const int x = is_left_box(box) ? -45 : 25;

    _text_gen.set_center_alignment();
    _text_gen.set_palette_item(bn::sprite_items::common_variable_8x16_font.palette_item());
    _text_gen.generate(x, 35, name, _name_sprites);

    for(bn::sprite_ptr& sprite : _name_sprites)
    {
        sprite.set_bg_priority(0);
        sprite.set_z_order(bn::sprites::min_z_order());
    }

    _text_gen.set_left_alignment();
}

void dialog_system::_draw_text(dialog_box_type box, const bn::string<256>& text)
{
    _text_sprites.clear();

    const bool left_side = is_left_box(box);
    const int start_x = left_side ? -80 : -115;
    int y = 55;

    constexpr int char_width = 8;
    const int max_end_x = left_side ? 116 : 60;
    const int max_chars = (max_end_x - start_x) / char_width;

    const bn::sprite_palette_item default_palette = bn::sprite_items::common_variable_8x16_font.palette_item();
    const bn::sprite_palette_item red_palette = bn::sprite_items::variable_8x16_font_red.palette_item();

    struct parsed_char
    {
        bn::string<4> bytes;
        bool red = false;
        bool newline = false;
    };

    bn::vector<parsed_char, 256> parsed_chars;
    bool red_text = false;

    for(int i = 0; i < text.size();)
    {
        const unsigned char c = text[i];

        if(c == '\\' && i + 1 < text.size())
        {
            if(text[i + 1] == 'n')
            {
                parsed_char item;
                item.newline = true;
                parsed_chars.push_back(item);
                i += 2;
                continue;
            }

            if(text[i + 1] == 'e' && i + 2 < text.size() && text[i + 2] == '(')
            {
                red_text = true;
                i += 3;
                continue;
            }
        }

        if(c == ')' && red_text)
        {
            red_text = false;
            ++i;
            continue;
        }

        parsed_char item;
        item.red = red_text;
        item.bytes += c;

        int char_bytes = 1;
        if((c & 0xE0) == 0xC0 && i + 1 < text.size())
        {
            item.bytes += text[i + 1];
            char_bytes = 2;
        }
        else if((c & 0xF0) == 0xE0 && i + 2 < text.size())
        {
            item.bytes += text[i + 1];
            item.bytes += text[i + 2];
            char_bytes = 3;
        }

        parsed_chars.push_back(item);
        i += char_bytes;
    }

    auto render_line = [&](int begin, int end)
    {
        if(begin >= end)
        {
            return;
        }

        int current_x = start_x;
        bn::string<256> segment;
        bool segment_red = parsed_chars[begin].red;
        int segment_chars = 0;

        auto flush_segment = [&]()
        {
            if(segment.empty())
            {
                return;
            }

            _text_gen.set_palette_item(segment_red ? red_palette : default_palette);
            _text_gen.generate(current_x, y, segment, _text_sprites);
            current_x += segment_chars * char_width;
            segment.clear();
            segment_chars = 0;
        };

        for(int index = begin; index < end; ++index)
        {
            const parsed_char& item = parsed_chars[index];
            if(item.red != segment_red)
            {
                flush_segment();
                segment_red = item.red;
            }

            segment += item.bytes;
            ++segment_chars;
        }

        flush_segment();
    };

    int line_start = 0;
    int line_length = 0;
    int last_space = -1;

    for(int index = 0; index < parsed_chars.size(); ++index)
    {
        const parsed_char& item = parsed_chars[index];

        if(item.newline)
        {
            render_line(line_start, index);
            y += 16;
            line_start = index + 1;
            line_length = 0;
            last_space = -1;
            continue;
        }

        ++line_length;
        if(item.bytes == " ")
        {
            last_space = index;
        }

        if(line_length >= max_chars)
        {
            if(last_space >= line_start)
            {
                render_line(line_start, last_space + 1);
                y += 16;
                line_start = last_space + 1;
                line_length = index - line_start + 1;
                last_space = -1;

                for(int j = line_start; j <= index; ++j)
                {
                    if(! parsed_chars[j].newline && parsed_chars[j].bytes == " ")
                    {
                        last_space = j;
                    }
                }
            }
            else
            {
                render_line(line_start, index + 1);
                y += 16;
                line_start = index + 1;
                line_length = 0;
                last_space = -1;
            }
        }
    }

    render_line(line_start, parsed_chars.size());

    for(bn::sprite_ptr& sprite : _text_sprites)
    {
        sprite.set_bg_priority(0);
        sprite.set_z_order(bn::sprites::min_z_order());
    }

    _text_gen.set_palette_item(default_palette);
    _text_gen.set_left_alignment();
}

}
