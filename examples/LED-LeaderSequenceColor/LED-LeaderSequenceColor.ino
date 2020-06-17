/* -*- mode: c++ -*-
 * Kaleidoscope-LeaderSequenceColor -- Highlight Leader sequence LEDs.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Kaleidoscope.h>
#include <Kaleidoscope-LEDControl.h>
#include <Kaleidoscope-Leader.h>
#include <Kaleidoscope-LED-LeaderSequenceColor.h>

// *INDENT-OFF*
KEYMAPS(
  [0] = KEYMAP_STACKED
  (Key_NoKey,    Key_1, Key_2, Key_3, Key_4, Key_5, Key_NoKey,
   Key_Backtick, Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Tab,
   Key_PageUp,   Key_A, Key_S, Key_D, Key_F, Key_G,
   Key_PageDown, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_Escape,

   Key_LeftControl, Key_Backspace, Key_LeftGui, Key_LeftShift,
   LEAD(0),

   Key_skip,  Key_6, Key_7, Key_8,     Key_9,      Key_0,         Key_skip,
   Key_Enter, Key_Y, Key_U, Key_I,     Key_O,      Key_P,         Key_Equals,
              Key_H, Key_J, Key_K,     Key_L,      Key_Semicolon, Key_Quote,
   Key_skip,  Key_N, Key_M, Key_Comma, Key_Period, Key_Slash,     Key_Minus,

   Key_RightShift, Key_RightAlt, Key_Spacebar, Key_RightControl,
   LEAD(0)),
)
// *INDENT-ON*

static void leaderNoOp(uint8_t seq_index) {}

static const kaleidoscope::plugin::Leader::dictionary_t leader_dictionary[] PROGMEM =
  LEADER_DICT(
// Regular highlighting.
{LEADER_SEQ(LEAD(0), Key_G, Key_B, Key_4), leaderNoOp},
{LEADER_SEQ(LEAD(0), Key_G, Key_B, Key_R), leaderNoOp},
// F will be `first_color` after G, F, not `used_color`.
{LEADER_SEQ(LEAD(0), Key_G, Key_F, Key_F), leaderNoOp},
{LEADER_SEQ(LEAD(0), Key_G, Key_F, Key_V), leaderNoOp},
// G, T turns T to `timeout_color`.
// Note: Timeout color not yet supported.
{LEADER_SEQ(LEAD(0), Key_G, Key_T, Key_1), leaderNoOp},
{LEADER_SEQ(LEAD(0), Key_G, Key_T), leaderNoOp});

// Important: LeaderSequenceColor must currently precede Leader.
KALEIDOSCOPE_INIT_PLUGINS(LEDControl, LEDOff, LeaderSequenceColorEffect, Leader);

void setup() {
  Kaleidoscope.setup();
  LEDOff.activate();

  Leader.dictionary = leader_dictionary;

  LeaderSequenceColorEffect.first_color = CRGB(160, 0, 0);
  LeaderSequenceColorEffect.second_color = CRGB(0, 0, 160);
  LeaderSequenceColorEffect.used_color = CRGB(60, 0, 0);
}

void loop() {
  Kaleidoscope.loop();
}
