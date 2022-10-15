# Singing Trombone

This is an experiment on trynig to turn the [Pink Trombone](https://dood.al/pinktrombone/) vocal synthesis model into a singing VST instrument.

## Sound examples
:warning: Beware, it does not sound particularly good :wink:
- [Video](https://www.youtube.com/watch?v=7JDCJf--9zw)

## Description
The plugin takes a text input that it will sing over. A text is composed of individual phrases separated by spaced (or new lines). Each phrase consists of one or two parts - attack and optional release separated bydash `-` character. On each midi note-on event the plugin vocalizes the attack part of the next phrase. The last phoneme gets sustained as the note is held. On node-off the release part gets vocalized (if present). The last phoneme of the release part will decay with the envelope release.

For example:
- `a` - will sing _aaa_ while the key is held,
- `la` - will sing _laaa_ (last _a_ gets sustained),
- `la-i` - will sing _laaa_ and on note-off - _i_

Consonants and especially transitions are not super convincing as the model requires tuning and some clever transitioning between parameters.

## Controls and MIDI CCs mapping
The plugins exposes the following controls:
- Volume (CC 7);
- Expression (CC 11);
- Attack (CC 73);
- Decay;
- Sustain;
- Release (CC 72);
- Vibrato (CC 1);

In polyphonic mode each MIDI note triggers a separate voice. In legato mode there is only one voice that gets retuned on each note-on.
