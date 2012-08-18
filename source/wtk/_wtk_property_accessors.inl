// =============================================================================
// This file is part of the Windowing Toolkit.
// Copyright (C) 2012 Michael Williams <devbug@bitbyte.ca>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// =============================================================================

#include "_wtk_windows.h"
#include "_wtk_controls.h"
#include "_wtk_font.h"
#include "_wtk_image.h"

#include <wtk/wtk_align.h>
#include <wtk/wtk_button.h>
#include <wtk/wtk_checkbox.h>

#include <stdarg.h>

// =============================================================================
// WTK_CONTROL_PROP_UserPtr
// =============================================================================

static void WTK_API wtk_prop_user_ptr_getter( struct wtk_control* control, va_list args )
{
    void** out;
    out = va_arg(args, void**);
    *out = control->user_ptr;
}

static void WTK_API wtk_prop_user_ptr_setter( struct wtk_control* control, va_list args )
{
    void* user_ptr;
    user_ptr = va_arg(args, void*);
    control->user_ptr = user_ptr;
}

// =============================================================================
// WTK_CONTROL_PROP_Position
// =============================================================================

static void WTK_API wtk_prop_position_getter( struct wtk_control* control, va_list args )
{
    HWND hWndParent;
    int *x_out, *y_out;
    
    x_out = va_arg(args, int*);
    y_out = va_arg(args, int*);

    hWndParent = GetParent(control->hWnd);
    if( hWndParent ) {
        POINT p = { 0, };
        MapWindowPoints(control->hWnd, hWndParent, &p, 1);
        *x_out = p.x; *y_out = p.y;
    } else {
        RECT rect;
        GetWindowRect(control->hWnd, &rect);
        *x_out = rect.left; *y_out = rect.top;
    }
}

static void WTK_API wtk_prop_position_setter( struct wtk_control* control, va_list args )
{
    int x, y;
    
    x = va_arg(args, int);
    y = va_arg(args, int);

    SetWindowPos(control->hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

// =============================================================================
// WTK_CONTROL_PROP_Size
// =============================================================================

static void WTK_API wtk_prop_size_getter( struct wtk_control* control, va_list args )
{
    RECT rect;
    int *width_out, *height_out;
    
    width_out = va_arg(args, int*);
    height_out = va_arg(args, int*);

    GetWindowRect(control->hWnd, &rect);

    *width_out = rect.right - rect.left - 1;
    *height_out = rect.bottom - rect.top - 1;
}

static void WTK_API wtk_prop_size_setter( struct wtk_control* control, va_list args )
{
    int width, height;
    
    width = va_arg(args, int);
    height = va_arg(args, int);

    SetWindowPos(control->hWnd, 0, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
}

// =============================================================================
// WTK_CONTROL_PROP_Font
// =============================================================================

static void WTK_API wtk_prop_font_getter( struct wtk_control* control, va_list args )
{
    struct wtk_font** out;
    out = va_arg(args, struct wtk_font**);
    *out = control->font;
}

static void WTK_API wtk_prop_font_setter( struct wtk_control* control, va_list args )
{
    struct wtk_font* font;
    font = va_arg(args, struct wtk_font*);
    PostMessage(control->hWnd, WM_SETFONT, (WPARAM)font->hFont, TRUE);
    control->font = font;
}

// =============================================================================
// WTK_CONTROL_PROP_Icon
// =============================================================================

static void WTK_API wtk_prop_icon_getter( struct wtk_control* control, va_list args )
{
    struct wtk_icon** icon_out;

    WTK_ASSERT( 
        ( control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) )
    );

    icon_out = va_arg(args, struct wtk_icon**);

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Button): {
            *icon_out = ((struct wtk_button*)control)->icon;
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            *icon_out = ((struct wtk_checkbox*)control)->icon;
        } break;
    }
}

static void WTK_API wtk_prop_icon_setter( struct wtk_control* control, va_list args )
{
    struct wtk_icon* icon;

    WTK_ASSERT( 
        ( control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) )
    );

    icon = va_arg(args, struct wtk_icon*);

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Button): {
            ((struct wtk_button*)control)->icon = icon;
            PostMessage(control->hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(icon ? icon->hIcon : NULL));

            // BS_ICON needs to be set if there is no text for the icon to show up:
            SetWindowLongPtr(control->hWnd, GWL_STYLE, (LONG_PTR)(
                (((DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE)) & ~BS_ICON) |
                ((((struct wtk_button*)control)->text && !icon) ? 0x00000000 : BS_ICON)
            ));
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            ((struct wtk_checkbox*)control)->icon = icon;
            PostMessage(control->hWnd, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)(icon ? icon->hIcon : NULL));

            // BS_ICON needs to be set if there is no text for the icon to show up:
            SetWindowLongPtr(control->hWnd, GWL_STYLE, (LONG_PTR)(
                (((DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE)) & ~BS_ICON) |
                ((((struct wtk_checkbox*)control)->text && !icon) ? 0x00000000 : BS_ICON)
            ));
        } break;
    }
}

// =============================================================================
// WTK_CONTROL_PROP_Icons
// =============================================================================

static void WTK_API wtk_prop_icons_getter( struct wtk_control* control, va_list args )
{
    struct wtk_icon** small_icon_out;
    struct wtk_icon** large_icon_out;

    WTK_ASSERT( 
        (control->type == WTK_CONTROL_TYPE(Window))
    );

    small_icon_out = va_arg(args, struct wtk_icon**);
    large_icon_out = va_arg(args, struct wtk_icon**);

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Window): {
            *small_icon_out = ((struct wtk_window*)control)->icons[0];
            *large_icon_out = ((struct wtk_window*)control)->icons[1];
        } break;
    }
}

static void WTK_API wtk_prop_icons_setter( struct wtk_control* control, va_list args )
{
    struct wtk_icon* small_icon;
    struct wtk_icon* large_icon;

    WTK_ASSERT( 
        (control->type == WTK_CONTROL_TYPE(Window))
    );

    small_icon = va_arg(args, struct wtk_icon*);
    large_icon = va_arg(args, struct wtk_icon*);

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Window): {
            ((struct wtk_window*)control)->icons[0] = small_icon;
            ((struct wtk_window*)control)->icons[1] = large_icon;
            PostMessage(control->hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)small_icon->hIcon);
            PostMessage(control->hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)large_icon->hIcon);
        } break;
    }
}

// =============================================================================
// WTK_CONTROL_PROP_Title
// =============================================================================

static void WTK_API wtk_prop_title_getter( struct wtk_control* control, va_list args )
{
    const char** out;

    WTK_ASSERT(control->type == WTK_CONTROL_TYPE(Window));

    out = va_arg(args, const char**);
    *out = ((struct wtk_window*)control)->title;
}

static void WTK_API wtk_prop_title_setter( struct wtk_control* control, va_list args )
{
    const char* value;

    WTK_ASSERT(control->type == WTK_CONTROL_TYPE(Window));

    value = va_arg(args, const char*);
    SetWindowTextA(control->hWnd, value);
    ((struct wtk_window*)control)->title = value;
}

// =============================================================================
// WTK_CONTROL_PROP_Text
// =============================================================================

static void WTK_API wtk_prop_text_getter( struct wtk_control* control, va_list args )
{
    const char** out;

    WTK_ASSERT(
        ( control->type == WTK_CONTROL_TYPE(Label) ||
          control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) ||
          control->type == WTK_CONTROL_TYPE(TextBox) )
    );

    out = va_arg(args, const char**);

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Label): {
            *out = ((struct wtk_label*)control)->text;
        } break;

        case WTK_CONTROL_TYPE(Button): {
            *out = ((struct wtk_button*)control)->text;
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            *out = ((struct wtk_checkbox*)control)->text;
        } break;

        case WTK_CONTROL_TYPE(TextBox): {
            // TODO: Find a more performant, and thread safe way.
            struct wtk_textbox* textbox = ((struct wtk_textbox*)control);
            int text_len = GetWindowTextLength(control->hWnd);
            textbox->text_buffer = wtk_realloc(textbox->text_buffer, text_len + 1);
            memset(textbox->text_buffer, 0, text_len + 1);
            GetWindowTextA(control->hWnd, textbox->text_buffer, text_len + 1);
            *out = textbox->text_buffer;
        } break;
    }
}

static void WTK_API wtk_prop_text_setter( struct wtk_control* control, va_list args )
{
    const char* text;

    WTK_ASSERT(
        ( control->type == WTK_CONTROL_TYPE(Label) ||
          control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) ||
          control->type == WTK_CONTROL_TYPE(TextBox) )
    );

    text = va_arg(args, const char*);
    SetWindowTextA(control->hWnd, text);

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Label): {
            ((struct wtk_label*)control)->text = text;
        } break;

        case WTK_CONTROL_TYPE(Button): {
            ((struct wtk_button*)control)->text = text;

            // BS_ICON needs to be set if there is no text for the icon to show up:
            SetWindowLongPtr(control->hWnd, GWL_STYLE, (LONG_PTR)(
                (((DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE)) & ~BS_ICON) |
                ((((struct wtk_button*)control)->icon && !text) ? BS_ICON : 0x00000000)
            ));
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            ((struct wtk_checkbox*)control)->text = text;

            // BS_ICON needs to be set if there is no text for the icon to show up:
            SetWindowLongPtr(control->hWnd, GWL_STYLE, (LONG_PTR)(
                (((DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE)) & ~BS_ICON) |
                ((((struct wtk_checkbox*)control)->icon && !text) ? BS_ICON : 0x00000000)
            ));
        } break;

        case WTK_CONTROL_TYPE(TextBox): {
            // ...
        } break;
    }
}

// =============================================================================
// WTK_CONTROL_PROP_TextAlignment
// =============================================================================

static void WTK_API wtk_prop_text_align_getter( struct wtk_control* control, va_list args )
{
    WTK_ASSERT(
        ( control->type == WTK_CONTROL_TYPE(Label) ||
          control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) ||
          control->type == WTK_CONTROL_TYPE(TextBox) )
    );

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Label): {
            *va_arg(args, wtk_align*) = ((struct wtk_label*)control)->text_align;
        } break;

        case WTK_CONTROL_TYPE(Button): {
            *va_arg(args, wtk_align*) = ((struct wtk_button*)control)->text_h_align;
            *va_arg(args, wtk_align*) = ((struct wtk_button*)control)->text_v_align;
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            *va_arg(args, wtk_align*) = ((struct wtk_checkbox*)control)->text_align;
        } break;

        case WTK_CONTROL_TYPE(TextBox): {
            *va_arg(args, wtk_align*) = ((struct wtk_textbox*)control)->text_align;
        } break;
    }
}

static DWORD WTK_API wtk_convert_text_align_to_style( struct wtk_control* control, wtk_align align )
{
    switch( control->type ) {
        case WTK_CONTROL_TYPE(Label): {
            switch( align ) {
                case WTK_ALIGN_Left: return SS_LEFT; break;
                case WTK_ALIGN_Center: return SS_CENTER; break;
                case WTK_ALIGN_Right: return SS_RIGHT; break;
            }
        } break;

        case WTK_CONTROL_TYPE(Button): {
            switch( align ) {
                case WTK_ALIGN_Left: return BS_LEFT; break;
                case WTK_ALIGN_Center: return BS_CENTER; break;
                case WTK_ALIGN_Right: return BS_RIGHT; break;
                case WTK_ALIGN_Top: return BS_TOP; break;
                case WTK_ALIGN_Middle: return BS_VCENTER; break;
                case WTK_ALIGN_Bottom: return BS_BOTTOM; break;
            }
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            switch( align ) {
                case WTK_ALIGN_Left: return BS_LEFTTEXT; break;
                case WTK_ALIGN_Right: return 0x00000000; break;
            }
        } break;

        case WTK_CONTROL_TYPE(TextBox): {
            switch( align ) {
                case WTK_ALIGN_Left: return ES_LEFT; break;
                case WTK_ALIGN_Center: return ES_CENTER; break;
                case WTK_ALIGN_Right: return ES_RIGHT; break;
            }
        } break;
    }

    return 0x00000000;
}

static void WTK_API wtk_prop_text_align_setter( struct wtk_control* control, va_list args )
{
    DWORD dwStyle;
    wtk_align h_align = WTK_ALIGN(Default), v_align = WTK_ALIGN(Default);

    WTK_ASSERT(
        ( control->type == WTK_CONTROL_TYPE(Label) ||
          control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) ||
          control->type == WTK_CONTROL_TYPE(TextBox) )
    );

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Label): {
            h_align = va_arg(args, wtk_align);
            ((struct wtk_label*)control)->text_align = h_align;
            dwStyle = (DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE);
            dwStyle &= ~(SS_CENTER | SS_LEFT | SS_RIGHT);
        } break;

        case WTK_CONTROL_TYPE(Button): {
            h_align = va_arg(args, wtk_align);
            v_align = va_arg(args, wtk_align);
            ((struct wtk_button*)control)->text_h_align = h_align;
            ((struct wtk_button*)control)->text_v_align = v_align;
            dwStyle = (DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE);
            dwStyle &= ~(BS_BOTTOM | BS_CENTER | BS_LEFT | BS_RIGHT | BS_TOP | BS_VCENTER);
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            h_align = va_arg(args, wtk_align);
            ((struct wtk_checkbox*)control)->text_align = h_align;
            dwStyle = (DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE);
            dwStyle &= ~(BS_LEFTTEXT);
        } break;

        case WTK_CONTROL_TYPE(TextBox): {
            h_align = va_arg(args, wtk_align);
            ((struct wtk_textbox*)control)->text_align = h_align;
            dwStyle = (DWORD)GetWindowLongPtr(control->hWnd, GWL_STYLE);
            dwStyle &= ~(ES_CENTER | ES_LEFT | ES_RIGHT);
        } break;
    }

    SetWindowLongPtr(control->hWnd, GWL_STYLE, (LONG_PTR)(dwStyle | wtk_convert_text_align_to_style(control, h_align) | wtk_convert_text_align_to_style(control, v_align)));
}

// =============================================================================
// WTK_CONTROL_PROP_Value
// =============================================================================

static void WTK_API wtk_prop_value_getter( struct wtk_control* control, va_list args )
{
    WTK_ASSERT(
        ( control->type == WTK_CONTROL_TYPE(Button) ||
          control->type == WTK_CONTROL_TYPE(Checkbox) )
    );

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Button): {
            *va_arg(args, wtk_button_state*) = (Button_GetState(control->hWnd) == BST_PUSHED) ? WTK_BUTTON_STATE(Down) : WTK_BUTTON_STATE(Up);
        } break;

        case WTK_CONTROL_TYPE(Checkbox): {
            switch( Button_GetCheck(control->hWnd) ) {
                case BST_CHECKED: *va_arg(args, wtk_checkbox_state*) = WTK_CHECKBOX_STATE(Checked); break;
                case BST_INDETERMINATE: *va_arg(args, wtk_checkbox_state*) = WTK_CHECKBOX_STATE(Indeterminate); break;
                case BST_UNCHECKED: *va_arg(args, wtk_checkbox_state*) = WTK_CHECKBOX_STATE(Unchecked); break;
            }
        } break;
    }
}

static void WTK_API wtk_prop_value_setter( struct wtk_control* control, va_list args )
{
    WTK_ASSERT(
        (control->type == WTK_CONTROL_TYPE(Checkbox))
    );

    switch( control->type ) {
        case WTK_CONTROL_TYPE(Checkbox): {
            switch( va_arg(args, wtk_checkbox_state) ) {
                case WTK_CHECKBOX_STATE(Checked): {
                    Button_SetCheck(control->hWnd, BST_CHECKED);
                } break;

                case WTK_CHECKBOX_STATE(Indeterminate): {
                    Button_SetCheck(control->hWnd, BST_INDETERMINATE);
                } break;

                case WTK_CHECKBOX_STATE(Unchecked): {
                    Button_SetCheck(control->hWnd, BST_UNCHECKED);
                } break;
            }
        } break;
    }
}