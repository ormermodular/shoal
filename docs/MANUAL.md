# Shoal - User Manual

*An 8-track generative melody sequencer for the Expert Sleepers disting NT.*

**Ormer Modular** · made in Guernsey

---

## What is Shoal?

Shoal plays melodies you didn't write - but that always sound like you meant them.

Eight tracks each generate a looping melody from a **seed**. The same seed
always produces the same melody, so patterns repeat like something you
composed. When you want a new one, you **reseed** - the pattern changes,
landing neatly at the start of the loop. Every note on every track is
locked to one global **scale**, so nothing is ever out of key.

Tracks can also **follow** each other: point track 2 at track 1 and it plays
a variation of the same musical material - an instant bass line that agrees
with your melody, forever, through every reseed.

The name: eight fish, moving as one. A reseed is the shoal turning.

---

## Installation

Shoal uses disting NT plug-in API v13, introduced in firmware 1.15.0.
Use firmware **1.15.0 or newer**.

The easiest route is the disting NT Plugin Gallery, via the Plugin
Manager in nt_helper. To install by hand instead:

1. Download `shoal-plugin.zip` from the latest release.
2. Unzip it at the root of the disting NT SD card. The archive installs
   `shoal.o` at `/programs/plug-ins/ormermodular/shoal.o`.
3. Restart the module or remount the card so the plug-in folder is
   scanned.
4. **Shoal** now appears in the algorithm list.

No compiler or development toolchain is required.

**macOS note:** if the card already has a `programs` folder, Finder may
create a second folder called `programs 2` instead of merging into it.
If that happens, move `shoal.o` into the existing `programs/plug-ins`
folder and delete the leftover.

---

## Five minutes to a first melody

1. **Add Shoal** to an empty preset.
2. **It's already ticking.** *Clock source* defaults to `Internal` at
   120 BPM (×1 = one step per beat), so a fresh Shoal runs the moment
   it lands. To sync it to your rack instead, set *Global → Clock
   source* to `External` and patch a clock (anything above ~1V) into
   **Input 1** - one pulse advances the ×1 tracks one step, so feed it
   quarter-note pulses.
3. **Give Track 1 outputs.** All outputs start as **None** - a track
   touches nothing until you assign it. On the *Routing 1* page set
   *Gate out* = Output 2 and *Pitch out* = Output 1 (gate listed
   first, matching the other NT sequencers), then patch:
   pitch → oscillator (1V/oct), gate → envelope (5V). Repeat on
   *Routing 2–8* when you want more tracks - every track starts
   unrouted.
4. **Pick a key.** *Global → Scale* and *Root note*. Try Minor pentatonic
   to start - it always sounds good.
5. **You should already hear a melody** - every seed carries a base line
   spanning two octaves centred on the root. The bipolar **Note** and
   **Oct** amounts add random variation on top when you want it wilder
   (up for upward straying, down for downward); return them to 0 to come
   home to the base melody.
6. **Reseed until you like it.** In the custom UI, tap the **right
   encoder**. Each tap rolls a new melody at the top of the next loop.
7. **Bring in more fish.** Give each further track outputs on its
   *Routing* page when you're ready for it - a track with no outputs
   assigned plays silently to itself. Build the shoal one voice at a
   time, deliberately.

That's the whole game. Everything else is refinement.

---

## The big ideas

### Seeds: randomness that repeats
**The seed is the melody.** A track's melody is completely determined
by its **Seed** (a number, 0–999) - think of it as a recipe number, not
a random amount. Same seed = same melody, every loop, every power-up,
every preset
recall. Reseeding just picks a new number. Because the seed is an ordinary
parameter, your presets capture your exact patterns - and you can even
CV-map the seed for pattern-switching madness.

### Reseeds land on the grid
A reseed never interrupts mid-loop. It **arms** (the panel shows
`RESEED ARM`, the fish swims faster) and takes effect when the track wraps
to step 1. Music stays on the rails; new sections start where sections
should.

### Chance shapes the rhythm
**CHANCE** - does this step play a note at all? 100% = every step,
50% = half rests. Seeded, so the same steps rest on every loop until
you reseed.

### The base melody, and non-destructive variation
Every seed produces a **base melody** - a line spanning two octaves of
the scale, centred on the root, so melodies orbit their home note. That
melody exists on its own: with *Note* and *Oct* at 0 (the defaults),
it's exactly what you hear.

Two bipolar amounts then **add** seeded random deviation on top:

- **NOTE** (−100%…+100%) - random variation applied to the notes. The
  magnitude sets both how *often* a note is varied and how *far* it may
  stray (up to 12 scale steps at full); the sign sets the direction -
  positive reaches up, negative reaches down.
- **OCT** (−100%…+100%) - the same idea with whole-octave leaps (up to
  5 octaves at full).

Turn them up for wildness, and back to 0 to return to the pure base
melody - **nothing is ever destroyed**. Notes are *built from* the
scale, so nothing is ever out of key.

### The slow arts: Evolve, Breathe, Freeze, Weight
Four controls made for long-form and ambient playing:

- **EVOLVE** (per track, 0–100%) - each pass, that share of steps
  quietly re-roll themselves, pitch and rhythm alike. At 5%, a melody
  becomes a different melody over ten minutes without your hands.
  Seeded, so the evolution path is repeatable; reseeding starts a
  fresh lineage. Followers evolve with their source.
- **BREATHE** (per track, 0–100%) - the chance that an entire pass
  rests. Steps rest with CHANCE; *phrases* rest with BREATHE. The
  track's row dims on screen while it sits out, and re-enters on its
  next pass.
- **FREEZE** (global, CV-mappable) - the shoal holds its breath:
  nothing advances and every running track's current note is held,
  hanging as a chord until you release. Freeze **blooms**: engaging it
  raises every track's gate immediately - including tracks that were
  between notes - so the full chord always sounds the instant you
  freeze. That immediate bloom is deliberate (and unclocked, like a
  sustain pedal): it is what makes a slow gate patched into Freeze
  produce oceanic swells. The fish stops swimming; `FRZ` shows on
  the panel. By default the **Clock out keeps running** through a
  freeze, so delays and anything else riding Shoal's clock stay in
  time while the melody hangs - set *Global → Frozen clock out* to
  `Stops` if you want the whole patch to hold its breath together.
- **WEIGHT** (global, 0–100%) - consonance gravity: that share of
  notes snap to the scale's root, third or fifth. High weight turns
  wild settings lush instead of angular.

### Tracks can follow each other
Set a track's **Sample source** to another track and it mirrors that
track's material - same musical contour - filtered through its **own**
settings: its own chance, note/oct amounts, octave, rate, length, gate.
Reseed the
source and the follower changes with it. Set source back to `Off` to
unlink (the track's own seed is untouched underneath).

### Direction: the same melody, walked differently
Because a track's melody is derived from its seed, **Direction** doesn't
change the notes - it changes the *walk*. Reverse plays your exact
melody backwards; Pendulum and Pong fold it into palindromes (ends
played twice or once respectively); Drunk staggers through it (50%
forward, 25% repeat, 25% back); Random abandons order entirely. Random
and Drunk are truly unpredictable modes - most others loop identically,
and flipping back to Forwards always recovers the original line.

Three generative walks go further:

- **Tide** - the rhythm holds steady while the *melody* drifts one step
  through it on every pass, realigning after *length* passes. Watch the
  row: the pattern slides left each time the loop wraps. Fully
  deterministic - lovely solo, and it phases against a follower playing
  the un-shifted original.
- **Shuffle** - every step plays exactly once per pass, in a freshly
  dealt order each pass. Unlike Random, nothing repeats and nothing is
  skipped: the same notes, their story retold every loop. The deals are
  seeded, so even the shuffles are reproducible.
- **Pools** - the playhead circles a small pocket of 3–4 adjacent steps
  for a few laps, then hops to another pocket. Motifs repeat, mutate
  position, move on - the closest a sequencer gets to sounding like
  it's thinking about a phrase. (Truly random, like Drunk.)

Armed reseeds land at the mode's loop origin (step 1; the last step in
Reverse; the pass boundary in Tide/Shuffle; immediately in
Random/Drunk/Pools).

### Polymetre and rate
Each track has its own **Length** (1–64 steps) and **Rate** (/64 … ×64 of
the master clock). The dial runs
`/64 /32 /16 /8 /7 /6 /5 /4 /3 /2 ×1 ×2 ×3 ×4 ×5 ×6 ×7 ×8 ×16 ×32 ×64` -
the odd ratios (3, 5, 7, plus 6) sit right next to the even ones, so a
track at ×3 or /5 pushes against the grid while the rest of the shoal
holds it. A 16-step track over a 5-step track at /2 makes phrases
that don't repeat for a long time; a 33-step track over anything makes
phrases that *never* seem to. This is where the "generative" feeling
really comes from.

---

## The performance controls (custom UI)

When Shoal's display is showing, the pots and encoders are live
performance controls. (Buttons 1–4 keep their normal NT functions.)

| Control | Turn | Push+turn | Tap | Hold |
|---|---|---|---|---|
| **Pot L** | CHANCE | - | **home ⇄ back** (100%) | - |
| **Pot C** | NOTE (bipolar, centre = 0) | - | **home ⇄ back** (0) | - |
| **Pot R** | OCT (bipolar, centre = 0) | - | **home ⇄ back** (0) | - |
| **Enc L** | select track | LENGTH | mute track | solo track |
| **Enc R** | RATE (/64 … ×64) | *(cancels reseed)* | **reseed track** | **reseed ALL** |

**Push-to-home:** pushing a pot snaps its parameter to the default -
the pure seeded melody for NOTE/OCT, full rhythm for CHANCE - and
*remembers where you were*. Push again and your dialled value returns.
Punch a track home mid-jam, punch it back: a mute button for chaos.
The click itself never edits the value - the pot is ignored for a
moment around each push, so a punch-back returns you to exactly the
value you left.
The pot label on the bottom bar glows while a pot is away from home.
(SLOP, GATE and TIE live on the track's parameter page, and are
CV-mappable like everything else.)

- Pots have **soft takeover**: after a home-punch or switching tracks, a
  knob does nothing until you move it through the stored value - no jumps.
- **Holds fire after about half a second, while you're still holding** -
  no need to release. Releasing earlier is a tap.
- The bottom bar names the three pots - CHANCE · NOTE · OCT. A label
  **glows while its pot is away from home** and dims at the default, so
  one glance shows which controls are active.
- **Mute** keeps the pattern running silently - unmute drops back in on
  the grid. **Solo** mutes everyone else. Mute closes the *gates* only:
  the pitch CV holds its last note, so a voice that ignores gates (a
  drone, a self-oscillating filter) sustains rather than going quiet -
  patch the gate into a VCA or level input if you need true silence.

### Reading the screen
- **8 rows** = 8 tracks: bar height suggests pitch, a flat tick is a rest,
  lone dots are steps beyond the track's length, the bright column is the
  play position.
- **Longer loops draw denser** - up to 16 steps in chunky cells, 17–32 at
  half width, 33–64 as a fine texture - so the whole pattern is always
  visible, never scrolled.
- `◄n` = this track is following track *n*. `M` = muted, `S` = solo.
  A row drawn dim (without `M`) is **breathing** - resting this pass.
- While **frozen**, `FRZ` shows on the panel and the fish stops
  swimming.
- Right panel = the selected track's live state: the pot values
  (chance, note/oct amounts) bright, and beside them `DIR` (the walk -
  FWD, REV, PND, RND, DRK, PNG, TID, SHF, POL), `SD` (the current
  seed - jot down the keepers) and `EV` (evolve). Below: rate, length,
  `OCT` (fixed octave offset) and `TR` (transpose, in scale degrees).
  GATE, TIE and SLOP are set-and-forget, so they live on the parameter
  page only.
- The panel's bottom corner shows the **global key** (root + scale,
  e.g. `C# HMIN`) - replaced by a blinking `RESEED ARM` while a
  reseed is waiting to land.
- The fish is decorative. Mostly. It swims faster when a reseed is armed.

### The screensaver: the shoal at rest
Leave the controls alone (1 or 5 minutes - the **Screensaver**
parameter on the Global page) and the display dissolves into open
water: **eight fish, one per track, swimming the screen**. Each fish
darts forward whenever its track sounds a note, so the scene *is* your
patch - busy tracks race, sparse tracks glide, muted tracks drift dim
and slow, a breathing track fades while it rests. Freeze stills the
whole shoal mid-water (the bubbles keep rising; the sea stays alive).

One thing Shoal cannot control: the **disting NT's own screensaver**,
which lives in the module's firmware and sits above every plug-in.
Left untouched long enough, it will step over the fish - and if the
display is showing a different algorithm, Shoal's saver never appears
at all (a plug-in can only draw while its own screen is up). To give
the shoal the night shift, turn the module's screensaver off (or
lengthen it) in the NT's own settings and leave Shoal as the current
algorithm.

Touch anything and the normal display returns - the waking touch is
swallowed, so you can't reseed a track just by waking the screen.
Changing parameters from the NT's pages also keeps it awake, but
CV-driven **Seed**, **Reseed all** and **Freeze** changes deliberately
don't - a generative patch can fall asleep and be watched. It's also
honest OLED care: nothing sits still on an idle screen.

---

## The parameter pages

Exit the custom view and everything lives on 17 standard pages. Every
parameter - all 207 of them - is CV/MIDI-mappable and saved in presets.
Track parameters are prefixed `T1…T8` in mapping menus. (See *Mappings
worth trying* in the Recipes section.)

### Global
| Parameter | Range | What it does |
|---|---|---|
| Clock source | External / Internal | Where ticks come from |
| BPM | 20–300 | Internal clock speed. ×1 = one step per beat (quarter notes); ×4 = 16ths. Ignored while the clock is External (but remembered, so you can set a tempo before switching over) |
| Run | Off / On | Master play/pause (mappable = external transport) |
| Freeze | Off / On | Hold every sounding note as a chord; nothing advances. CV-map it for swells |
| Scale | 13 scales | The key everything plays in |
| Root note | C-1…G9 | The tonal centre; C3 sits at 0V |
| Weight | 0–100% | Consonance gravity: % of notes snapped to root/3rd/5th |
| Screensaver | Off / 1 min / 5 min | Idle animation: the shoal swims the screen, one fish per track |
| Clock input | bus | Which input the clock comes in on |
| Reset input | bus / none | Rising edge → all tracks restart at step 1 on the next tick |
| Reseed input | bus / none | Rising edge → reseed everything, exactly like the encoder gesture: a fresh random *Reseed all* base, armed, landing at each loop origin. Built for short triggers (CV on *Seed* wants held steps; this wants pulses) |
| Clock out | bus / none | Shoal's master clock as 5V pulses (×1 rate, 50% duty) - clock other algorithms or external gear from Shoal's grid. Follows Run |
| Clock out mode | Add / Replace | As with the track outputs; Replace by default |
| Frozen clock out | Stops / Runs | What Clock out does during a Freeze. `Runs` (default) keeps the grid and pulses going so the rest of the patch stays in time; `Stops` holds the whole patch's breath at once. **Changed from v1.0.0**, which always stopped |
| Reseed all | 0–999 | Any change re-rolls every track (armed, on the grid) |

### Track 1–8 (each)
| Parameter | Range | What it does |
|---|---|---|
| Length | 1–64 | Steps in this track's loop |
| Rate | /64 … ×64 | Speed relative to the master clock (×1 = one step per beat) |
| Direction | 9 modes | Forwards · Reverse · Pendulum (ends play twice) · Random · Drunk (50% on / 25% repeat / 25% back) · Pong (ends play once) · Tide (loop start drifts one step per pass) · Shuffle (every step once per pass, reshuffled) · Pools (dwells in a small pocket, then hops) |
| Shift | −63…+63 | Rotate the whole pattern - notes, gates, their timing - by that many steps, non-destructively (0 returns the original). On a follower, this is a canon: the same melody entering behind or ahead of its source |
| Chance | 0–100% | Probability a step plays a note |
| Breathe | 0–100% | Chance a whole pass rests (phrase-level silence) |
| Note | −100…+100% | Random note variation: magnitude = how often & how far, sign = direction (0 = melody as seeded) |
| Oct | −100…+100% | Random octave leaps, same principle |
| Evolve | 0–100% | Share of steps that re-roll themselves each pass - slow, hands-free change |
| Gate length | 5–95% | Gate time as a share of the step |
| Tie | 0–100% | Chance a note slurs into the next (no retrigger; slides) |
| Slop | 0–100% | Humanised timing: seeded per-note delay, up to half a step |
| Octave | −3…+3 | Fixed octave shift for the whole track |
| Transpose | −7…+7 | Fixed shift in scale degrees - always in key. +2 on a follower = parallel thirds |
| Sample source | Off / Track 1–8 | Follow another track's material |
| Mute | Off / On | Silence the gates; pattern keeps running. Pitch CV holds its last note, so a gate-less voice (drone, self-oscillating filter) sustains rather than going quiet |
| Seed | 0–999 | The pattern. Change it (any way) to reseed |

> **Hearing Gate & Tie:** these shape gate *duration*, so they're only
> audible on voices that follow it - an envelope with sustain (ADSR →
> VCA) is the classic test rig. Strike-based voices (Rings, plucks,
> drums, LPG pings) respond to the gate's *edge* only and will ignore
> both. For singing ties, add slew on the pitch line: tied notes
> become slides.

### Routing 1–8 (each)
Pitch out / Gate out bus assignments, each with Add/Replace mode
(Replace by default - a track owns its busses). **Everything defaults to
None**: a track writes to no bus at all until you assign its outputs,
so nothing ever appears on an output or aux bus you didn't choose.
Assign to physical Outputs for your rack, or to aux busses to drive
other algorithms inside the preset.

Each routing page also carries the track's **output voltages** (new in
v1.1; the defaults are exactly the old behaviour):

| Parameter | Range | What it does |
|---|---|---|
| Gate volts | 1–10V | The gate's high level (default 5V). 10V for vintage-style gear, 1V for LZX-standard video synthesis |
| Pitch scale | 5–200% | Scales the pitch CV around 100% = 1V/oct. 120% ≈ 1.2V/oct gear; small values compress the melody into a narrow CV window |
| Pitch offset | −10…+10V | Fixed voltage added after the scale - shift the whole track's CV range, or make a bipolar melody unipolar |

Pitch scale changes what "in tune" means downstream, so scale first,
then tune your oscillator - or leave 100% and use only the offset,
which transposes without touching the volt-per-octave law.

---

## Recipes

**A bass line that follows your melody**
Track 1: your melody. Track 2: *Sample source* = Track 1, *Octave* = −2,
*Rate* = /4, *Chance* ≈ 50%, *Gate* ≈ 80%. Done - slow, sparse, sustained,
always in agreement.

**Parallel harmony**
Track 2: *Sample source* = Track 1, same Rate and Length,
*Transpose* = +2 (thirds) or +4 (fifths-ish, scale permitting). The same
melody harmonises itself, in key, through every reseed. Thin it with
*Chance* so the harmony only appears on some notes.

**A true canon** *(new in v1.1)*
Track 2: *Sample source* = Track 1, same Rate and Length,
*Shift* = −4 (or wherever the imitation sits well), *Octave* = −1.
The same melody enters four steps behind its leader, an octave down -
a round, and it survives every reseed. Shift the follower, never the
leader, and the leader stays the reference voice.

**Fitting other voltage standards** *(new in v1.1)*
Each track's routing page has *Gate volts*, *Pitch scale* and *Pitch
offset*, so non-Euro gear needs no external attenuators. For
LZX-standard video synthesis (0-1V full scale): *Gate volts* = 1V,
*Pitch scale* = 40%, *Pitch offset* = +0.5V - the two-octave melody
(roughly ±1V) lands inside 0-1V with headroom for Note/Oct deviation.
For 1.2V/oct gear, *Pitch scale* = 120%. For vintage +10V gates,
*Gate volts* = 10V.

**Shoal as the master clock**
Set *Clock out* to an aux bus (or a physical output for external gear)
and point other algorithms' clock inputs at that bus - drums, LFOs and
envelopes all ride Shoal's grid, in either clock mode. Put Shoal above
the algorithms that read it. The pulse runs at the ×1 step rate, 50%
duty, and stops with Run. During a Freeze it keeps running by default,
so synced delays keep their tails while the melody hangs; set *Frozen
clock out* to `Stops` if you'd rather freeze the whole patch's breath
at once.

**Full external transport (DAW-style start/stop from one gate)**
Clock → Input 1. Run/transport gate → Input 2. Set *Reset input* =
Input 2 **and** CV-map *Run* to Input 2. Gate high = play from the top;
gate low = stop.

**Evolving ambient**
Two or three tracks at odd lengths (say 16, 11, 7), rates /2 and /4,
low Chance (30–50%), Breathe 20%, Evolve 5–10%, Tie 30%, Slop 10%,
Weight 60%, In-Sen or Hirajoshi scale, slow attack voices. Then don't
touch anything for twenty minutes - the Evolve/Breathe pair does the
composing, Weight keeps it lush. CV-map Freeze to a slow LFO for
swelling chords out of whatever happens to be playing.

**Live key changes**
CV-map *Root note* (or *Scale*). Each track adopts the new key on its next
note, cascading over a beat or two.

**A human drummer's hi-hat line** (melodic module optional)
Chance 65%, Note and Oct at 0, Gate 10%, Slop 15%, ×8 rate - take just the
gate output. The seeded rests and drift make a repeating, groovy pattern.

**Mappings worth trying**
Every one of Shoal's 171 parameters is CV/MIDI-mappable through the
NT's Mappings menu (track parameters show as `T1…T8`). The ones that
change what the instrument *is*:

- **Seed** - sequence the sequencer: CV stepping a track's Seed
  switches whole melodies, each landing neatly at the loop start.
  (For plain triggers, patch the dedicated *Reseed input* instead -
  it fires on edges, no held CV needed.)
- **Freeze** - a slow LFO or foot-controlled gate breathes the shoal
  into hanging chords. Built for it.
- **Root note / Scale** - key and mode changes from CV; each track
  adopts them on its next note, cascading over a beat or two.
- **Transpose** - CV chord progressions on one voice, always in key.
- **Evolve / Weight / Chance** - a ramp rising over your set, or an
  envelope follower on the mix: form-level automation, no hands.

(Solo and the pot punch-home gestures are deliberately *not*
parameters - those stay under your fingers. Mute is mappable.)

---

## Troubleshooting

**Nothing is moving on screen.**
Run is Off, or *Clock source* is `External` and no clock is arriving on
the input the *Clock input* parameter points at. (A fresh Shoal defaults
to `Internal` and runs on its own.)

**A parameter seems dead, or it's stuck at one tempo.**
Check the algorithm list for a **second Shoal** - with two loaded it is
easy to edit one while hearing the other.

**It's running but silent.**
First check the track's *Routing* page - **outputs default to None** and
a track makes no sound until its Pitch/Gate outs are assigned. Then:
Chance at 0? Muted (`M`)? Another track solo'd (`S`)? Patched from the
output you actually assigned?

**Notes seem out of time.**
That's probably *Slop* - set it to 0 for machine timing.

**The melody won't get wilder.**
*Note* and *Oct* are at (or near) 0 - they're bipolar, and the centre of
the pot is zero variation. Turn away from centre in either direction;
positive strays upward, negative downward.

**A knob does nothing.**
Soft takeover - sweep it through its stored position and it picks up.

**My preset sounds different after a plug-in update.**
If a Shoal update adds parameters, older presets may not line up -
re-save them with the new version.

---

## Specifications

- Pitch outputs: 1V/oct, 0V = C3 (MIDI note 48), folded to stay in key at
  the extremes; per-track scale (5-200%) and offset (±10V)
- Gate outputs: 0V low, high level per track (1-10V, default 5V)
- Clock, reset and reseed input thresholds: rising through ~1V (with
  hysteresis)
- Clock output: 0/5V pulses at the ×1 step rate, 50% duty; follows Run,
  and Freeze only if *Frozen clock out* is set to `Stops`
- 8 pitch + 8 gate outputs plus the clock output, freely assignable to
  any bus, Add or Replace
- Memory: under 0.5KB per instance; CPU: light
- Algorithm guid: `Shol`

---

*Shoal is an Ormer Modular instrument - named for the ormer, the Guernsey
shellfish the sea only reveals on the lowest tides of the year.*

Happy swimming. 🐟
