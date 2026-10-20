#ifndef _3D_SHELL_UI_KEY_CHORD_H
#define _3D_SHELL_UI_KEY_CHORD_H

#include <3ds.h>

namespace UI {

/* True on the press edge of `button` while `modifier` is held,
 * e.g. Chord(k_down, k_held, KEY_L, KEY_A) for L+A. */
inline bool Chord(u32 k_down, u32 k_held, u32 modifier, u32 button) {
    return (k_held & modifier) && (k_down & button);
}

}

#endif
