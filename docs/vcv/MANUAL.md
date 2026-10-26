# Shoal for VCV Rack - User Manual

*An 8-track generative melody sequencer.*

**Ormer Modular** · made in Guernsey

*This manual covers the VCV Rack plugin, v2.0.6. Shoal is also
available for the Expert Sleepers disting NT, where it is free - same
engine, same seeds, same melodies; that version has its own manual.*

---

## What is Shoal?

Shoal plays melodies you didn't write - but that always sound like you
meant them.

Eight tracks each generate a looping melody from a **seed**. The same
seed always produces the same melody, so patterns repeat like something
you composed. When you want a new one, you **reseed** - the pattern
changes, landing neatly at the start of the loop. Every note on every
track is locked to one global **scale**, so nothing is ever out of key.

Tracks can also **follow** each other: point track 2 at track 1 and it
plays a variation of the same musical material - an instant bass line
that agrees with your melody, forever, through every reseed.

The name: eight fish, moving as one. A reseed is the shoal turning.

---

## Installation

Shoal is available from the **VCV Library**:

1. Buy Shoal on its page in the
   [VCV Library](https://library.vcvrack.com/OrmerModular/Shoal),
   logged in to your VCV account.
2. In Rack, open the **Library** menu (log in there if you aren't
   already) and let it update - Shoal downloads with your other
   plugins.
3. Restart Rack if it asks. Three modules appear in the module browser
   under **Ormer Modular**: **Shoal** itself, and its two optional
   expanders, **Shoal Expander** and **Shoal Outs**.

Updates arrive the same way, through Rack's Library menu.

---

## Five minutes to a first melody

1. **Add Shoal.** It is already ticking: the internal clock defaults
   to 120 BPM, and ×1 means one step per beat.
2. **Patch track 1.** GATE 1 → an envelope or VCA, PITCH 1 → an
   oscillator's 1V/oct input. Each track has its own pair of jacks
   along the bottom - no routing pages, no polyphony splitting.
3. **Pick a key.** Right-click → **Scale** and **Root note**. Try
   Minor pentatonic to start - it always sounds good.
4. **You should already hear a melody** - every seed carries a base
   line spanning two octaves centred on the root. The bipolar **NOTE**
   and **OCT** knobs add random variation on top when you want it
   wilder (up for upward straying, down for downward); return them to
   centre to come home to the base melody.
5. **Reseed until you like it.** Tap **RSED**. Each tap rolls a new
   melody for the selected track, landing at the top of its next loop
   (the button glows while the reseed is waiting). *Reseed all* lives
   in the right-click menu and on the RESEED jack.
6. **Bring in more fish.** Patch further tracks' jacks when you're
   ready - build the shoal one voice at a time, deliberately.
7. **To sync to your rack**, patch a clock into **CLK IN**. A cable
   in that jack *is* external mode - Shoal follows it immediately,
   one pulse per ×1 step, and returns to its internal clock when you
   unpatch.

That's the whole game. Everything else is refinement.

---

## The big ideas

### Seeds: randomness that repeats
**The seed is the melody.** A track's melody is completely determined
by its **Seed** (a number, 0-999) - think of it as a recipe number,
not a random amount. Same seed = same melody, every loop, every
restart, every patch reload. Reseeding just picks a new number.
Because the seed is an ordinary parameter, your patches and presets
capture your exact patterns - and the SEED CV input can address it
for pattern-switching madness.

### Reseeds land on the grid
A reseed never interrupts mid-loop. It **arms** (the RSED button
glows, the display's fish swims faster) and takes effect when the
track wraps to step 1. Music stays on the rails; new sections start
where sections should.

### Chance shapes the rhythm
**CHANCE** - does this step play a note at all? 100% = every step,
50% = half rests. Seeded, so the same steps rest on every loop until
you reseed.

### The base melody, and non-destructive variation
Every seed produces a **base melody** - a line spanning two octaves
of the scale, centred on the root, so melodies orbit their home note.
That melody exists on its own: with NOTE and OCT at centre (the
defaults), it's exactly what you hear.

Two bipolar knobs then **add** seeded random deviation on top:

- **NOTE** (−100%…+100%) - random variation applied to the notes. The
  magnitude sets both how *often* a note is varied and how *far* it
  may stray (up to 12 scale steps at full); the direction of the knob
  sets the direction of the stray.
- **OCT** (−100%…+100%) - the same idea with whole-octave leaps (up
  to 5 octaves at full).

Turn them up for wildness, and back to centre to return to the pure
base melody - **nothing is ever destroyed**. Notes are *built from*
the scale, so nothing is ever out of key.

### The slow arts: Evolve, Breathe, Freeze, Weight
Four controls made for long-form and ambient playing:

- **Evolve** (per track, 0-100%, in the menu) - each pass, that share
  of steps quietly re-roll themselves, pitch and rhythm alike. At 5%,
  a melody becomes a different melody over ten minutes without your
  hands. Seeded, so the evolution path is repeatable; reseeding
  starts a fresh lineage. Followers evolve with their source.
- **Breathe** (per track, 0-100%, in the menu) - the chance that an
  entire pass rests. Steps rest with Chance; *phrases* rest with
  Breathe. The track's lane dims on the display while it sits out,
  and re-enters on its next pass.
- **FRZE** (panel button + FREEZE gate input) - the shoal holds its
  breath: nothing advances and every running track's current note is
  held, hanging as a chord until you release. Freeze **blooms**:
  engaging it raises every track's gate immediately - including
  tracks that were between notes - so the full chord always sounds
  the instant you freeze. That immediate bloom is deliberate (and
  unclocked, like a sustain pedal): it is what makes a slow gate
  patched into FREEZE produce oceanic swells. By default the **clock
  output keeps running** through a freeze, so delays and anything
  else riding Shoal's clock stay in time while the melody hangs -
  set *Frozen clock out* to `Stops` in the menu if you want the
  whole patch to hold its breath together.
- **Weight** (global, 0-100%, in the menu) - consonance gravity: that
  share of notes snap to the scale's root, third or fifth. High
  weight turns wild settings lush instead of angular.

### Tracks can follow each other
Set a track's **Follow** (right-click menu) to another track and it
mirrors that track's material - same musical contour - filtered
through its **own** settings: its own chance, note/oct amounts,
octave, rate, length, gate. Reseed the source and the follower
changes with it. Set it back to `Off` to unlink (the track's own
seed is untouched underneath).

### Direction: the same melody, walked differently
Because a track's melody is derived from its seed, **Direction**
doesn't change the notes - it changes the *walk*. Reverse plays your
exact melody backwards; Pendulum and Pong fold it into palindromes
(ends played twice or once respectively); Drunk staggers through it
(50% forward, 25% repeat, 25% back); Random abandons order entirely.
Random, Drunk, Pools and Skitter are truly unpredictable - the others
loop identically, and flipping back to Forwards always recovers the
original line.

There are fifteen walks in all. Three generative walks go further:

- **Tide** - the rhythm holds steady while the *melody* drifts one
  step through it on every pass, realigning after *length* passes.
  Watch the lane: the pattern slides left each time the loop wraps.
  Fully deterministic - lovely solo, and it phases against a follower
  playing the un-shifted original.
- **Shuffle** - every step plays exactly once per pass, in a freshly
  dealt order each pass. Unlike Random, nothing repeats and nothing
  is skipped: the same notes, their story retold every loop. The
  deals are seeded, so even the shuffles are reproducible.
- **Pools** - the playhead circles a small pocket of 3-4 adjacent
  steps for a few laps, then hops to another pocket. Motifs repeat,
  mutate position, move on - the closest a sequencer gets to
  sounding like it's thinking about a phrase. (Truly random, like
  Drunk.)

Six more:

- **Stride** (Hopscotch) - an overlapping "two ahead, one back"
  crawl: step 1, step 3, step 2, step 4, step 3, step 5... Each step
  gets played twice across a full cycle, which takes twice the
  pattern's Length to complete. Fixed and deterministic.
- **Gravity** - mostly walks forward, but has a seed-derived chance
  each step of snapping back to step 1 instead - an orbiting, homing
  pull. The same idea **Weight** applies to pitch (gravity towards
  the root), applied here to the walk itself.
- **Converge** - folds the pattern in on itself: step 1, the last
  step, step 2, the second-to-last step, and so on, meeting in the
  middle.
- **Diverge** - the mirror of Converge: starts near the pattern's
  middle and alternates outward to both ends, arriving at step 1 and
  the last step last.
- **Skitter** - like Random, but never plays the same step twice in
  a row. Unpredictable without the stutter Random can produce; truly
  random otherwise (like Random, Drunk and Pools).
- **Anchor** - alternates step 1 with each other step in turn: step
  1, step 2, step 1, step 3, step 1, step 4... A drone note
  punctuated by melodic excursions, always returning home between
  them. Fixed and deterministic, unlike Gravity's similar but
  seed-derived homing pull.

Armed reseeds land at the mode's loop origin: step 1 (including
Stride, Gravity, Converge and Anchor); the last step in Reverse; the
pass boundary in Tide/Shuffle; the middle step in Diverge, where its
cycle begins; immediately in Random/Drunk/Pools/Skitter.

### Polymetre and rate
Each track has its own **Length** (1-64 steps) and **RATE** (/64 …
×64 of the master clock). The dial runs 29 rates in ascending order:
`/64 /32 /16 /8 /7 /6 /5.3 /5 /4 /3 /2.6 /2 /1.5 ×1 ×1.25 ×1.3 ×1.5
×2 ×2.6 ×3 ×4 ×5 ×5.3 ×6 ×7 ×8 ×16 ×32 ×64` -
the odd ratios (3, 5, 7, plus 6) sit right next to the even ones, so
a track at ×3 or /5 pushes against the grid while the rest of the
shoal holds it, and the fractional ones land *between* the grid for
polyrhythms proper: ×1.5 is a true 3-over-2, ×1.25 the classic
5-against-4. Fractional rates are exact - their advances are spread as
evenly as possible, so they never drift. Divided rates land on the
downbeat: a /4 track steps with every fourth clock, starting on the
first. A 16-step track over a 5-step track at /2 makes
phrases that don't repeat for a long time; a 33-step track over
anything makes phrases that *never* seem to. This is where the
"generative" feeling really comes from.

---

## The panel

Shoal works on a **selected-track** basis: the TRACK buttons under
the display choose which track the knobs and buttons address, and
the knobs snap to show that track's values when you switch. Every
control is a real Rack parameter, so MIDI-mapping and preset saving
work as usual.

### Track buttons
**TRACK 01-08** - select a track. The active track lights amber,
and its lane is highlighted on the display.

### Big knobs (the performance trio)
| Knob | Range | What it does |
|---|---|---|
| **CHANCE** | 0-100% | Probability a step plays a note |
| **NOTE** | ±100%, centre 0 | Random note variation: distance from centre = how often and how far, direction = up or down |
| **OCTAVE** | ±100%, centre 0 | Random octave leaps, same principle |

### Small knobs
| Knob | Range | What it does |
|---|---|---|
| **LENG** | 1-64 | Steps in this track's loop |
| **RATE** | /64…×64 | Speed relative to the master clock (×1 = one step per beat), 29 rates |
| **DIRN** | 15 modes | The walk (see Directions above) |
| **TRNS** | −7…+7 | Fixed shift in scale degrees - always in key. +2 on a follower = parallel thirds |
| **SHFT** | −63…+63 | Rotate the whole pattern - notes, gates, their timing - by that many steps, non-destructively (0 returns the original). On a follower, this is a canon |
| **OCTA** | −3…+3 | Fixed octave shift for the whole track |

*(OCTAVE the big knob adds random octave leaps; OCTA the small knob
transposes the whole track by fixed octaves.)*

### Buttons
| Button | Acts on | What it does |
|---|---|---|
| **FRZE** | global, latching | Hold every sounding note as a chord; nothing advances. Glows while frozen, including CV-driven freezes |
| **MUTE** | selected track | Silence the track's gate; the pattern keeps running, and unmuting drops back in on the grid. Pitch CV holds its last note, so a gate-less voice (drone, self-oscillating filter) sustains rather than going quiet |
| **SOLO** | selected track | Mute everyone else. Solo another track to move it; press again to clear |
| **RUN** | global, latching | Master play/pause |
| **RSET** | global | All tracks restart at step 1 on the next tick |
| **RSED** | selected track | Reseed: rolls a fresh random seed, armed, landing at the loop origin. Glows while armed |

### Jacks
| Jack | Direction | What it does |
|---|---|---|
| **GATE 1-8** | out | The track's gate. High level per track (1-10V, default 10V, set in the menu). GATE 2/4/6/8 also carry poly channels if *Poly output routing* is set |
| **PITCH 1-8** | out | The track's pitch CV, 1V/oct, 0V = C3. Per-track scale and offset in the menu. PITCH 2/4/6/8 also carry poly channels if *Poly output routing* is set |
| **CLK OUT** | out | Shoal's master clock: 10V pulses at the ×1 step rate, 50% duty. Follows Run; during Freeze see *Frozen clock out* |
| **CLK IN** | in | External clock. **A cable here is external mode** - one pulse advances ×1 tracks one step; unpatch to return to the internal clock |
| **RST IN** | in | Rising edge: all tracks restart at step 1 on the next tick |
| **RESEED** | in | Rising edge: reseed *everything* - a fresh random base, armed, each track landing at its own loop origin. Built for triggers |
| **FREEZE** | in | Gate: freeze while high, OR-ed with the FRZE button |
| **SEED** | in | CV addresses the **selected track's** seed: 0-10V maps to 0-999. Change the voltage and the track reseeds to that exact number (armed, on the grid). Sequence it for repeatable pattern chains |

---

## The display

The tank. Eight lanes, one per track, with the selected track's
detail down the right-hand side - the same screen as the hardware.

- **8 lanes** = 8 tracks: bar height suggests pitch, a flat tick is
  a rest, the bright column is the play position.
- **Longer loops draw denser** - the whole pattern is always visible,
  never scrolled.
- The flag beside a lane: `S` = solo, `M` = muted, `‹n` = following
  track *n*. A lane drawn dim (without `M`) is **breathing** -
  resting this pass.
- Right panel = the selected track's live state: seed, direction,
  rate, length and friends - jot down the keepers.
- **Amber means CV.** When a Shoal Expander is modulating a track,
  that lane's number burns amber, and any value in the right panel the
  CV is moving shows amber at its modulated value. The panel knobs
  stay put - they show what you set, the display shows what's playing.
- Bottom right: the key (root and scale), `RESEED ARM` blinking while
  the selected track's reseed waits, `FRZ` while frozen.
- The fish is decorative. Mostly. It swims faster when a reseed is
  armed.

### The screensaver: the shoal at rest

Leave the controls alone (five minutes by default; Off / 1 min /
5 min in the right-click menu) and the display dissolves into open
water: **eight fish, one per track, swimming the screen**. Each fish
darts forward whenever its track sounds a note, so the scene *is*
your patch - busy tracks race, sparse tracks glide, muted tracks
drift dim and slow, a breathing track fades while it rests. Freeze
stills the whole shoal mid-water (the bubbles keep rising; the sea
stays alive).

Touch any of the module's controls and the normal display returns -
the waking touch is swallowed, so you can't reseed a track just by
waking the screen. CV arriving at SEED, RESEED or FREEZE
deliberately doesn't wake it: a generative patch can fall asleep
and be watched.

---

## The right-click menu

The long tail of settings lives in the context menu, addressed to
the selected track (the menu is headed with its number). Anything
with its own panel control stays on the panel.

**Typing exact values:** click a slider without dragging, or
right-click it, and it becomes a text box. Type the value and press
**Enter** to apply (the menu stays open), or **Escape** to leave it
unchanged.

### Track section
| Item | Range | What it does |
|---|---|---|
| Evolve | 0-100% | Share of steps that re-roll themselves each pass |
| Breathe | 0-100% | Chance a whole pass rests |
| Gate length | 5-100% | Gate time as a share of the step |
| Tie | 0-100% | Chance a note slurs into the next (no retrigger) |
| Slop | 0-100% | Humanised timing: seeded per-note delay, up to half a step |
| Gate level | 1-10V | The gate's high voltage (default 10V) |
| Pitch scale | 5-200% | Scales the pitch CV around 100% = 1V/oct |
| Pitch offset | −10…+10V | Fixed voltage added after the scale, in 0.1V steps |
| Follow (share seed of) | Off / Track 1-8 | Follow another track's material |
| Seed | 0-999 | The pattern. Drag it to audition seeds directly |

### Global section
| Item | Range | What it does |
|---|---|---|
| Scale | 13 scales | The key everything plays in |
| Root note | C…B | The tonal centre |
| Weight | 0-100% | Consonance gravity: % of notes snapped to root/3rd/5th |
| BPM | 20-300 | Internal clock speed in whole BPM (ignored while a cable is in CLK IN, but remembered) |
| Panel | Follow Rack setting / Silver / Black | The module's finish. The default follows Rack's dark-panels preference; the choice saves per instance |
| Screensaver | Off / 1 min / 5 min | Idle animation: the shoal swims the screen, one fish per track (default: after 5 minutes) |
| Frozen clock out | Stops / Runs | What CLK OUT does during a Freeze. `Runs` (default) keeps the grid going so the patch stays in time; `Stops` holds the whole patch's breath at once |
| Poly output routing | Off / Pairs / Split 4+4 / All 8 | Bundles tracks onto fewer poly cables - see *Poly output routing* below. The default, Off, is unchanged: 8 independent mono pairs |
| Currents range (Shoal Outs) | 0-10V / ±5V | The voltage range of the Currents outputs on Shoal Outs. `0-10V` (default) matches the hardware; `±5V` suits bipolar destinations |
| Reseed all | action | Re-rolls every track (armed, on the grid) |

### Adding the expanders
The last two items, *Add Shoal Expander* and *Add Shoal Outs*, place
an expander tight against Shoal - the Expander on the left and Outs on
the right, so signals flow left to right - moving neighbouring modules
along to make room. If that side is already taken by the other
expander, the new one goes on the free side. *Add Shoal Outs* is
greyed out while one is already attached.

### Poly output routing
Fewer cables when you want them. Every mode groups tracks and lands
that group's poly cable on the last jack in the group - `Off`
(default) is groups of 1, i.e. every track on its own mono pair,
nothing changes. `Pairs` groups tracks two at a time: PITCH/GATE 2
carries tracks 1-2, PITCH/GATE 4 carries tracks 3-4, and so on
through 6 and 8, each a 2-channel poly cable. `Split 4+4` groups
four at a time: PITCH/GATE 4 carries tracks 1-4, PITCH/GATE 8
carries tracks 5-8, each 4-channel. `All 8` is one group of eight:
PITCH/GATE 8 carries every track as an 8-channel poly cable. In
every mode the individual mono jacks keep working exactly as
before, so existing patches into any of the eight pairs are
unaffected - poly mode only adds channels to each group's hub jack,
it never takes a working cable away.

> **Hearing Gate length & Tie:** these shape gate *duration*, so
> they're only audible on voices that follow it - an envelope with
> sustain (ADSR → VCA) is the classic test rig. Strike-based voices
> (plucks, drums, LPG pings) respond to the gate's *edge* only and
> will ignore both. For singing ties, add slew on the pitch line:
> tied notes become slides.

Everything - knobs, menu settings, mute/solo, seeds - is saved in
your patch and in module presets, and restores exactly.

---

## The expanders

Two optional 4HP companions, included with Shoal. Each works on
**either side** of Shoal, as long as it sits directly against it (or,
for Expanders, against a chain leading to it) in the same row. A green
light at the top of each shows it's connected. The conventional layout
is inputs on the left, outputs on the right: Expander, then Shoal, then
Outs.

### Shoal Expander: CV in
Twelve CV inputs for the performance controls.

- **TRACK knob** - chooses which track the ten per-track inputs
  address. This is independent of Shoal's own track selection, so you
  can browse tracks on the panel without re-routing live modulation.
- **Per-track inputs** - CHANCE, NOTE, OCTAVE, EVOLVE, BREATHE, SLOP,
  TRNS, SHFT, OCTA and GATE length.
- **Global inputs** - WEIGHT and ROOT.

**CV adds to the knob.** Each input adds to that setting's current
value and is clamped to its range - the knob is the centre, the CV
moves around it. Unipolar settings (0-100%, like CHANCE or EVOLVE)
take 0-10V for the full range; bipolar ones (NOTE, OCTAVE, TRNS, SHFT,
OCTA) take ±5V. The one exception is **ROOT**, which is a 12-way
choice rather than a range: it sets the root outright from 1V/oct,
0V = C, so a keyboard or quantised sequence changes key directly.

Modulation never moves Shoal's knobs and is never saved into your
patch - saving mid-sweep stores the values you set. The display shows
it instead, in amber.

**Chaining:** add more Expanders beside the first, working outward
from Shoal (up to eight in all, on one side or split across both - the
menu's *Add another Shoal Expander* grows the chain for you), each on
its own TRACK. Several can address the same track; their CV adds up. WEIGHT
inputs add up across the chain; for ROOT, the patched Expander nearest
Shoal wins.

### Shoal Outs: Currents and end-of-sequence
Sixteen outputs, one row per track: **Current** on the left, **EOS**
on the right. One Shoal Outs carries all eight tracks.

- **Currents** - a slow, smooth, seeded drift CV per track - the water
  the melody swims in. For filter cutoffs, VCA levels, send amounts:
  gentle ambient movement without a separate LFO. It comes from the
  track's own seed, so it repeats with every pass and returns with the
  seed. A new target lands each time the track advances, whether or
  not the step sounds, and the voltage eases towards it over the step,
  never jumping - so it moves at the track's own rate. A follower's
  Current is its own, not its source's. Currents hold through a
  Freeze. 0-10V by default, or ±5V via Shoal's *Currents range* menu
  item.
- **End-of-sequence (EOS)** - a 10V pulse, half a step long, every
  time the track completes a full cycle of its Length. Use it to
  advance another sequencer, ping an envelope once per phrase, or
  reseed the shoal. It counts *advances*, so every direction gets a
  steady once-per-Length pulse - even Random, Drunk, Pools and
  Skitter, which have no fixed loop of their own. It ignores Chance
  and Mute (it marks the structure, not which notes sound) and stays
  silent through a Freeze.

---

## Recipes

**A bass line that follows your melody**
Track 1: your melody. Select track 2: *Follow* = Track 1,
OCTA = −2, RATE = /4, CHANCE ≈ 50%, *Gate length* ≈ 80%. Done -
slow, sparse, sustained, always in agreement.

**Parallel harmony**
Track 2: *Follow* = Track 1, same RATE and LENG, TRNS = +2 (thirds)
or +4 (fifths-ish, scale permitting). The same melody harmonises
itself, in key, through every reseed. Thin it with CHANCE so the
harmony only appears on some notes.

**A true canon**
Track 2: *Follow* = Track 1, same RATE and LENG, SHFT = −4 (or
wherever the imitation sits well), OCTA = −1. The same melody enters
four steps behind its leader, an octave down - a round, and it
survives every reseed. Shift the follower, never the leader, and the
leader stays the reference voice.

**Shoal as the master clock**
Patch CLK OUT to your drum module, clocked LFOs and synced delays -
everything rides Shoal's grid whether Shoal is on its internal clock
or following CLK IN. During a Freeze the pulse keeps running by
default, so delay tails stay in time while the melody hangs.

**Full external transport**
Clock → CLK IN, transport gate → RST IN, and MIDI-map RUN (or just
use the button). Gate high with the clock running = play from the
top.

**Evolving ambient**
Two or three tracks at odd lengths (say 16, 11, 7), rates /2 and /4,
low CHANCE (30-50%), Breathe 20%, Evolve 5-10%, Tie 30%, Slop 10%,
Weight 60%, In-Sen or Hirajoshi scale, slow attack voices. Then
don't touch anything for twenty minutes - the Evolve/Breathe pair
does the composing, Weight keeps it lush. A slow LFO into FREEZE
swells chords out of whatever happens to be playing.

**Live key changes**
MIDI-map *Root note* (or *Scale*), or automate them from a
controller. Each track adopts the new key on its next note,
cascading over a beat or two.

**Sequence the sequencer**
A step sequencer into SEED switches the selected track between
melodies you've chosen, each landing neatly at the loop start.
0-10V maps to seeds 0-999, so about 10mV per seed: quantised or
sample-and-held sources work better than drifting ones.

**A human drummer's hi-hat line**
CHANCE 65%, NOTE and OCTAVE centred, *Gate length* 10%, Slop 15%,
×8 rate - take just the gate output. The seeded rests and drift
make a repeating, groovy pattern.

**A new section every phrase**
With Shoal Outs attached, patch EOS 8 into Shoal's own RESEED input
and set track 8's LENG to 64. Every 64 steps the whole shoal turns.

**Water for the filters**
Patch each voice's Current (Shoal Outs) into its filter cutoff. Every
track breathes at its own pace, and the movement comes back with the
seed. Slow rates give long swells.

**Hands-free dynamics**
Set a Shoal Expander's TRACK to 1, turn track 1's CHANCE knob to 0,
and patch a slow 0-10V LFO into the Expander's CHANCE: the track thins
out and fills back in. Another LFO into NOTE makes it wander further
on the peaks.

**Fitting other voltage standards**
Each track's *Gate level*, *Pitch scale* and *Pitch offset* remove
the need for external attenuators: 5V gates for picky envelopes,
*Pitch scale* 120% for 1.2V/oct gear, or compress the melody into a
narrow CV window for video synthesis.

---

## Troubleshooting

**Nothing is moving on the display.**
RUN is off, or there's a cable in CLK IN but no clock arriving - a
patched clock input silences the internal clock entirely.

**It's running but silent.**
CHANCE at 0? MUTE lit? Another track solo'd (`S` on its lane)?
Patched from the right track's jacks?

**Notes seem out of time.**
That's probably *Slop* - set it to 0 for machine timing.

**The melody won't get wilder.**
NOTE and OCTAVE are at (or near) centre - they're bipolar, and the
centre is zero variation. Turn away from centre in either direction.

**The knobs jumped when I pressed a TRACK button.**
By design: the knobs always show the selected track's real values,
so switching tracks moves them. Every track remembers its own
settings.

**An expander does nothing, and its light is off.**
It isn't touching Shoal. Either expander can go on either side, but it
must sit directly against Shoal in the same row (an Expander can also
sit at the end of an unbroken chain of Expanders). The two types can't
share a side, since only one module fits directly against Shoal there.
The menu's *Add* items place them correctly.

**Expander CV is patched but the knobs don't move.**
By design - CV adds on top without moving the knobs. Watch the
display: the modulated values show in amber. Also check the
Expander's TRACK knob is on the track you expect.

**My oscillator plays in a different octave than expected.**
Shoal's 0V = C3, matching the hardware original. Tune your
oscillator to taste, or use *Pitch offset* (+1V = up an octave).

---

## Specifications

- 8 tracks, polymetric: per-track length 1-64, 29 rates from /64 to
  ×64 (odd and fractional ratios included), 15 play directions
- Pitch outputs: 1V/oct, 0V = C3 (MIDI note 48), folded to stay in
  key at the extremes; per-track scale (5-200%) and offset (±10V)
- Gate outputs: 0V low; high level per track 1-10V, default 10V
- Clock, reset, reseed inputs: rising edge above ~1V (with
  hysteresis); Freeze is level-sensitive (a gate)
- Seed input: 0-10V maps to seed 0-999 for the selected track
- Clock output: 0/10V pulses at the ×1 step rate, 50% duty; follows
  Run, and Freeze only if *Frozen clock out* is `Stops`
- Internal clock 20-300 BPM; external clock via CLK IN (cable
  presence switches mode)
- Engine identical to the disting NT version (same seeds produce the
  same melodies on both)
- Width: 24HP
- Shoal Expander (4HP): 10 per-track CV inputs addressing its chosen
  track, plus global Weight and Root; additive, 0-10V unipolar / ±5V
  bipolar; Root 1V/oct absolute; either side of Shoal, chains up to 8
- Shoal Outs (4HP): per track, a Current (0-10V or ±5V) and an
  end-of-sequence output (10V, half a step long); either side of Shoal

---

*Shoal is an Ormer Modular instrument - named for the ormer, the
Guernsey shellfish the sea only reveals on the lowest tides of the
year.*

Happy swimming. 🐟
