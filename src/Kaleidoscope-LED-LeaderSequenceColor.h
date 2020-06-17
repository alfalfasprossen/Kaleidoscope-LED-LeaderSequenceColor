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


#pragma once

#include "kaleidoscope/Runtime.h"
#include <Kaleidoscope-LEDControl.h>
#include <Kaleidoscope-Leader.h>

namespace kaleidoscope {
namespace plugin {

class LeaderSequenceColorEffect : public kaleidoscope::Plugin {
  public:
	LeaderSequenceColorEffect(void) {}
	/** Color for keys that come next in any matching sequence. */
	static cRGB first_color;
	/** Color for keys that may be valid after some next key. */
	static cRGB second_color;
	/** Color for keys that were used in the recorded sequence. */
	static cRGB used_color;
	/** Color for keys that will cause an action after a timeout. */
	static cRGB timeout_color;

	/**
	 * Reset the recorded sequence - will reset the respective LEDs.
	 *
	 * Duplicated from Leader - If you call `Leader::reset()` from
	 * your sketch, you should call this as well.
	 */
	static void reset(void);

	EventHandlerResult onKeyswitchEvent(Key &mapped_key, KeyAddr key_addr, uint8_t keyState);
	EventHandlerResult afterEachCycle();
	EventHandlerResult beforeReportingState();

  private:
	// Duplicated from Leader plugin.
	static Key sequence_[LEADER_MAX_SEQUENCE_LENGTH + 1];
	static uint8_t sequence_pos_;
	static uint16_t start_time_;
	static int8_t lookup(void);

	// From this plugin.

	/** A copy of the recorded sequence to track if it changed. */
	static Key last_sequence_[LEADER_MAX_SEQUENCE_LENGTH + 1];
	static uint8_t last_sequence_pos_;

	/**
	 * Mapping of key-index to expected LED related state.
	 *
	 * The expected state is evaluated and applied on each cycle
	 */
	static uint8_t key_led_state_map[Runtime.device().numKeys()];

	/**
	 * Set the LEDs to their respective leader sequence related color.
	 *
	 * This needs to be called every cycle, even if we don't want to
	 * change anything, but we may need to overwrite what animated LED
	 * modes may have set the LEDs to.
	 */
	void updateLEDs(void);

	/**
	 * Evaluate the current and possible followup sequences and store
	 * the respective color flags in the `key_led_state_map`.
	 */
	void updateLEDStateMap(void);
};

}
}

extern kaleidoscope::plugin::LeaderSequenceColorEffect LeaderSequenceColorEffect;
