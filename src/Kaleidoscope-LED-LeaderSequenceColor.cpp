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


#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"
#include "kaleidoscope/layers.h"

#include "Kaleidoscope-LED-LeaderSequenceColor.h"

namespace kaleidoscope {
namespace plugin {

// Copied from Leader.
#define PARTIAL_MATCH -1
#define NO_MATCH -2
#define isLeader(k) (k.getRaw() >= ranges::LEAD_FIRST && k.getRaw() <= ranges::LEAD_LAST)
#define isActive() (sequence_[0] != Key_NoKey)

enum LEDStateFlags {
  STATE_OFF = B0000,
  STATE_USED = B0001,
  STATE_NEXT = B0010,
  STATE_UBERNEXT = B0100
};

cRGB LeaderSequenceColorEffect::first_color = CRGB(160, 0, 0);
cRGB LeaderSequenceColorEffect::second_color = CRGB(0, 0, 160);
cRGB LeaderSequenceColorEffect::used_color = CRGB(60, 0, 0);
cRGB LeaderSequenceColorEffect::timeout_color = CRGB(130, 100, 0);

// Copied from Leader.
Key LeaderSequenceColorEffect::sequence_[LEADER_MAX_SEQUENCE_LENGTH + 1];
uint8_t LeaderSequenceColorEffect::sequence_pos_ = 0;
uint16_t LeaderSequenceColorEffect::start_time_ = 0;

Key LeaderSequenceColorEffect::last_sequence_[LEADER_MAX_SEQUENCE_LENGTH + 1];
uint8_t LeaderSequenceColorEffect::last_sequence_pos_ = 255;
uint8_t LeaderSequenceColorEffect::key_led_state_map[Runtime.device().numKeys()];


void LeaderSequenceColorEffect::updateLEDStateMap(void) {
	Key key_at_addr;

	for (KeyAddr key_addr : KeyAddr::all()) {
		uint8_t expected_state = STATE_OFF;
		key_at_addr = ::Layer.lookupOnActiveLayer(key_addr);
		if (key_at_addr == Key_NoKey) {
			continue;
		}
		for (uint8_t i = 0; i <= sequence_pos_; i++) {
			if (sequence_[i] == key_at_addr) {
				expected_state = STATE_USED;
			}
		}

		for (uint8_t seq_index = 0; ; seq_index++) {
			const Key* peeking_sequence{Leader::dictionary[seq_index].sequence};
			if (peeking_sequence[0].readFromProgmem() == Key_NoKey) {
				// End of dictionary.
				break;
			}
			bool is_match = true;
			for (uint8_t seq_peek = 0; seq_peek <= sequence_pos_; seq_peek++) {
				if (sequence_[seq_peek] != peeking_sequence[seq_peek].readFromProgmem()) {
					is_match = false;
					break;
				}
			}
			if (!is_match) {
				continue;
			}
			// TODO: if next pos == NoKey but match we might have
			// a timeout key.
			uint8_t next_pos = sequence_pos_ + 1;
			if (next_pos <= LEADER_MAX_SEQUENCE_LENGTH &&
			    peeking_sequence[next_pos].readFromProgmem() == key_at_addr) {
				expected_state = STATE_NEXT;
				// If this is a next-key we don't care about if it is
				// also an ubernext key or used in any other sequence.
				break;
			}
			uint8_t ubernext_pos = sequence_pos_ + 2;
			if (ubernext_pos <= LEADER_MAX_SEQUENCE_LENGTH &&
			    peeking_sequence[ubernext_pos].readFromProgmem() == key_at_addr) {
				// If this is an ubernext-key, there might still be
				// another sequence where it is already a next-key, so
				// we keep going.
				expected_state = expected_state | STATE_UBERNEXT;
				continue;
			}
		} // End for seq_index

		key_led_state_map[key_addr.toInt()] = expected_state;

		// If we don't claim a key's LED anymore, make it free here so
		// we don't have to do it on every cycle.
		if (expected_state == STATE_OFF) {
			LEDControl::refreshAt(key_addr);
		}
	}


}


void LeaderSequenceColorEffect::updateLEDs(void) {
	// TODO: Make use of timeout color.
	for (KeyAddr key_addr : KeyAddr::all()) {
		uint8_t expected_state = key_led_state_map[key_addr.toInt()];
		// STATE_OFF is handled outside this function - only when
		// things changed - to save performance.
		if (expected_state == STATE_USED) {
			LEDControl::setCrgbAt(key_addr, used_color);
		} else if (expected_state & STATE_NEXT) {
			// STATE_NEXT is more important than STATE_UBERNEXT
			LEDControl::setCrgbAt(key_addr, first_color);
		} else if (expected_state & STATE_UBERNEXT) {
			LEDControl::setCrgbAt(key_addr, second_color);
		}
	}
}


EventHandlerResult LeaderSequenceColorEffect::beforeReportingState() {
	// Not active and was not active.
	if (sequence_[0] == Key_NoKey && last_sequence_[0] == Key_NoKey) {
		return EventHandlerResult::OK;
	}

	// Not active but was active.
	if (sequence_[0] == Key_NoKey && last_sequence_[0] != Key_NoKey) {
		// The last sequence was completed, so reset all touched LEDs.
		for (KeyAddr key_addr : KeyAddr::all()) {
			if (key_led_state_map[key_addr.toInt()] != STATE_OFF) {
				key_led_state_map[key_addr.toInt()] = STATE_OFF;
				LEDControl::refreshAt(key_addr);
			}
		}
		// updateLEDs();
		last_sequence_[0] = Key_NoKey;
		last_sequence_pos_ = 255;
		return EventHandlerResult::OK;
	}

	// Active but nothing changed.
	if (last_sequence_pos_ == sequence_pos_) {
		updateLEDs();
		return EventHandlerResult::OK;
	}

	// Active and pending changes.
	updateLEDStateMap();
	last_sequence_pos_ = sequence_pos_;
	last_sequence_[sequence_pos_] = sequence_[sequence_pos_];
	updateLEDs();

	return EventHandlerResult::OK;
}

// Copied from Leader.
void LeaderSequenceColorEffect::reset(void) {
  sequence_pos_ = 0;
  sequence_[0] = Key_NoKey;
}

// Copied from Leader to track the recorded sequence - only here we
// don't consume the event.
EventHandlerResult LeaderSequenceColorEffect::onKeyswitchEvent(Key &mapped_key, KeyAddr key_addr, uint8_t keyState) {
  if (keyState & INJECTED)
    return EventHandlerResult::OK;

  if (!isActive() && !isLeader(mapped_key))
    return EventHandlerResult::OK;

  if (!isActive()) {
    // Must be a leader key!

    if (keyToggledOff(keyState)) {
      // not active, but a leader key = start the sequence on key release!
      start_time_ = Runtime.millisAtCycleStart();
      sequence_pos_ = 0;
      sequence_[sequence_pos_] = mapped_key;
    }

    // If the sequence was not active yet, ignore the key.
    return EventHandlerResult::OK;
  }

  // active
  int8_t action_index = lookup();

  if (keyToggledOn(keyState)) {
    sequence_pos_++;
    if (sequence_pos_ > LEADER_MAX_SEQUENCE_LENGTH) {
      reset();
      return EventHandlerResult::OK;
    }

    start_time_ = Runtime.millisAtCycleStart();
    sequence_[sequence_pos_] = mapped_key;
    action_index = lookup();

    if (action_index >= 0) {
      return EventHandlerResult::OK;
    }
  } else if (keyIsPressed(keyState)) {
    // held, no need for anything here.
    return EventHandlerResult::OK;
  }

  if (action_index == NO_MATCH) {
    reset();
    return EventHandlerResult::OK;
  }
  if (action_index == PARTIAL_MATCH) {
    return EventHandlerResult::OK;
  }

  return EventHandlerResult::OK;
}

// Copied from Leader.
EventHandlerResult LeaderSequenceColorEffect::afterEachCycle() {
	if (isActive() && Runtime.hasTimeExpired(start_time_, Leader::time_out))
		reset();

  return EventHandlerResult::OK;
}

// Copied from Leader.
int8_t LeaderSequenceColorEffect::lookup(void) {
  bool match;

  for (uint8_t seq_index = 0; ; seq_index++) {
    match = true;

    if (Leader::dictionary[seq_index].sequence[0].readFromProgmem() == Key_NoKey)
      break;

    Key seq_key;
    for (uint8_t i = 0; i <= sequence_pos_; i++) {
	    seq_key = Leader::dictionary[seq_index].sequence[i].readFromProgmem();

      if (sequence_[i] != seq_key) {
        match = false;
        break;
      }
    }

    if (!match)
      continue;

    seq_key = Leader::dictionary[seq_index].sequence[sequence_pos_ + 1]
	    .readFromProgmem();
    if (seq_key == Key_NoKey) {
      return seq_index;
    } else {
      return PARTIAL_MATCH;
    }
  }

  return NO_MATCH;
}

}
}

kaleidoscope::plugin::LeaderSequenceColorEffect LeaderSequenceColorEffect;
