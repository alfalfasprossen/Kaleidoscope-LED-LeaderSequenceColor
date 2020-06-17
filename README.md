# Kaleidoscope-LED-LeaderSequenceColor

Highlight the keys used in the current matching Leader sequence.
Inspired by the likes of Ace-Jump or Avy for Emacs / Vim which
highlight the sequence to jump to a certain position.

This may be helpful if you tend to forget which Leader sequences are
available in your current sketch.

![Recorded gif](./demo.gif)


## Installation

Download or clone into your Arduino library folder, depending on your
OS this should be `$HOME/Documents/Arduino` or `$HOME/Arduino`.

## Using The Plugin

Include the header and initialize `LeaderSequenceColorEffect`
**before** `Leader`, but after any LED-altering effect whose colors
you want to be overwritten by the active leader sequence.

```c++
#include <Kaleidoscope.h>
#include <Kaleidoscope-LEDControl.h>
#include <Kaleidoscope-Leader.h>
#include <Kaleidoscope-LED-LeaderSequenceColor.h>

KALEIDOSCOPE_INIT_PLUGINS(LEDControl,
                          LeaderSequenceColorEffect,
                          Leader);

void setup () {
  Kaleidoscope.setup();

  LeaderSequenceColorEffect.first_color = CRGB(160, 0, 0);
  LeaderSequenceColorEffect.second_color = CRGB(0, 0, 160);
  LeaderSequenceColorEffect.used_color = CRGB(60, 0, 0);
  ...
}
```

Initializing before `Leader` is important as the code currently does
the same as `Leader` itself to record the sequence - if `Leader` comes
first, `LeaderSequenceColorEffect` wouldn't receive the key events
necessary to do that.

Ideally `Leader` would allow to read the recorded sequence from the
outside so that we could track changes without having to duplicate the
code.

## Plugin Properties

The plugin provides the `LeaderSequenceColorEffect` object, which has
the following properties:

### `.first_color`

> Color for keys that come next in any currently matching sequence.
> Meaning pressing any of these keys proceeds in the sequence matching
> process.

> `first_color` takes precedence over `second_color` and
> `used_color` - it is more important if a key proceeds the sequence
> when pressed as the next key than if it already has been pressed
> before.

### `.second_color`

> Color for keys that may be valid after some next key. That would
> include all keys that come after *any* valid next key in *any**
> currently matching sequence.

### `.used_color`

> Color for keys that were used in the recorded sequence - the keys
> that were already typed - and are not valid as a next key.

### `.timeout_color`

> Color for keys that are last in an active sequence that will cause
> an action when the timer runs out. That is if no other key is
> pressed to complete another sequence or cancel the current one.
>
> **Note:** This is currently not working as expected so it is
> disabled for now.

## Plugin Methods

### `.reset()`

> If you call `Leader.reset()` anywhere in your sketch you should call
> this as well.

## Dependencies

* [Kaleidoscope-LEDControl](https://github.com/keyboardio/Kaleidoscope/tree/master/docs/plugins/LEDControl.md)
* [Kaleidoscope-Leader](https://github.com/keyboardio/Kaleidoscope/tree/master/docs/plugins/Leader.md)
