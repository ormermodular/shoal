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

> **Upgrading from v1.1.0? Read this first.** Turning the Rate encoder
> has always worked the same way: ×1 sits in the middle, turn left for
> dividers, turn right for multipliers. That hasn't changed. In
> v1.1.0, the dial had 21 rates with ×1 dead centre at position 10.
> v1.2 adds 8 new non-integer rates - three dividers (`/1.5`, `/2.6`,
> `/5.3`) and five multipliers (`×1.25`, `×1.3`, `×1.5`, `×2.6`,
> `×5.3`) - each inserted into its correct spot in that same spread,
> so ×1 is still dead centre, now at position 13 of 29. Only the six
> slowest rates (`/64` through `/6`) sit at the same position they did
> in v1.1.0. This means: **any preset saved on v1.1.0 will load into
> v1.2 with different Rate settings than you saved**, on most or all
> of its tracks, silently - the module has no way to warn you when
> this happens. Before you play an old preset, go through each track's
> Rate on the Track pages and reset it to what you actually want. This
> is a one-time cost; the dial won't reorder again.

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

Six more, from v1.2:

- **Stride** (Hopscotch) - an overlapping "two ahead, one back" crawl:
  step 1, step 3, step 2, step 4, step 3, step 5... Each step gets
  played twice across a full cycle, which takes twice the pattern's
  Length to complete. Fixed and deterministic - every seed hops the
  same way, unlike the original v1.2.0-beta.1 version, which picked a
  seed-derived jump size.
- **Gravity** - mostly walks forward, but has a seed-derived chance
  each step of snapping back to step 1 instead - an orbiting, homing
  pull. The same idea **Weight** applies to pitch (gravity towards the
  root), applied here to the walk itself.
- **Converge** - folds the pattern in on itself: step 1, the last
  step, step 2, the second-to-last step, and so on, meeting in the
  middle. Fixed and self-contained - no Sample source needed.
- **Diverge** - the mirror of Converge: starts near the pattern's
  middle and alternates outward to both ends, arriving at step 1 and
  the last step last.
- **Skitter** - like Random, but never plays the same step twice in a
  row. Unpredictable without the stutter Random can produce; truly
  random otherwise (like Random, Drunk and Pools).
- **Anchor** - alternates step 1 with each other step in turn: step 1,
  step 2, step 1, step 3, step 1, step 4... A drone note punctuated by
  melodic excursions, always returning home between them. Fixed and
  deterministic, unlike Gravity's similar but seed-derived homing pull.

Armed reseeds land at the mode's loop origin (step 1; the last step in
Reverse; the pass boundary in Tide/Shuffle; immediately in
Random/Drunk/Pools/Skitter). Stride, Gravity, Converge and Anchor land
the same way - at step 1. Diverge lands at its own starting point
instead - the pattern's middle step, where its cycle actually begins.

### Polymetre and rate
Each track has its own **Length** (1–64 steps) and **Rate** (/64 … ×64 of
the master clock, plus eight non-integer ratios, all in true ascending
order). The dial runs
`/64 /32 /16 /8 /7 /6 /5.3 /5 /4 /3 /2.6 /2 /1.5 ×1 ×1.25 ×1.3 ×1.5 ×2 ×2.6 ×3 ×4 ×5 ×5.3 ×6 ×7 ×8 ×16 ×32 ×64` -
the odd ratios (3, 5, 7, plus 6) sit right next to the even ones, so a
track at ×3 or /5 pushes against the grid while the rest of the shoal
holds it. A 16-step track over a 5-step track at /2 makes phrases
that don't repeat for a long time; a 33-step track over anything makes
phrases that *never* seem to. This is where the "generative" feeling
really comes from.

**The eight non-integer rates (v1.2)** - /5.3, /2.6, /1.5, ×1.25,
×1.3, ×1.5, ×2.6, ×5.3 - slot in at their true position on the dial
(e.g. /1.5 sits between /2 and ×1; ×1.5 between ×1 and ×2). /1.5 and
×1.5 are the simplest: where every other rate keeps a fixed number of
advances every master tick, these alternate - ×1.5 fires 1, then 2,
then 1, then 2 advances per tick (three advances every two ticks, on
average - true 3-over-2, no drift); /1.5 fires 0, then 1, then 1 (two
advances every three ticks). The other six use exact decimal ratios -
not rounded to a "nicer" fraction (×1.3 really is 13/10, not 4/3) -
built the same way: each
spreads its advances as evenly as possible across a repeating window
rather than a fixed count per tick, so the long-run rate is exact with
no drift. ×1.25 is the simplest of those six: four ticks, five
advances (1, 1, 1, 2 repeating) - the classic 5-against-4
cross-rhythm. ×2.6 repeats every five ticks for thirteen advances.
The rest - /5.3, /2.6, ×1.3, ×5.3 - have longer, less obvious repeat
windows (10, 13 and 53 ticks), which is rather the point: a lopsided
pulse that never quite settles into a short, learnable groove - more
texture than tempo.

### MIDI (v1.2)
Shoal can send MIDI note out per track, and can be clocked FROM MIDI
instead of a CV or internal clock - useful on its own, and it's how
Shoal talks to other algorithms on the *same* disting NT with no
cabling at all.

**Note out.** Each track's *Routing* page has one MIDI parameter
alongside its CV output config: **MIDI channel** (0 = off - the
default, so updating firmware never starts sending MIDI a patch
wasn't already asking for). **MIDI velocity** (1-127) and **MIDI
dest** - a destination bitmask covering Breakout, Select bus, USB and
Internal in any combination - are global settings on the *Global*
page, shared by every track that has a channel set, rather than a
per-track choice. Internal targets another algorithm on the *same* NT
directly - no patch cable needed. That's a real, tested use case:
feeding Shoal into Poly FM (a MIDI/I²C-only algorithm) in one unit. A
track is monophonic, so at most one MIDI note sounds per track at a
time; a new note always closes the previous one first. Note off fires
on the exact same signal that silences the CV gate - natural gate end,
mute, or a channel change mid-note - so MIDI timing always tracks the
audible gate. Changing the global *MIDI dest* live closes every
track's currently-sounding MIDI note, since one change now affects
every track at once rather than just one. One simplification: *Tie*
retriggers over MIDI (fast note-off then note-on) rather than the true
legato overlap the CV gate achieves; audibly close on a sound that
doesn't retrigger its envelope while gated.

**MIDI clock in.** Set *Clock source* to `MIDI` and Shoal locks to
incoming realtime MIDI clock (24 PPQN - Shoal's ×1 is one step per
quarter note, so 24 clock messages = one ×1 tick) instead of a CV
signal or its own oscillator. MIDI Start resets the pattern to step 1,
same as the Reset input; Continue resumes in place; Stop halts it -
*Run* still has to be On as well, exactly as with the other two clock
sources, so a DAW's transport and Shoal's own Run switch both have to
agree the patch should be playing.

**One known limitation:** the disting NT gives plug-ins no way to run
code when an algorithm is *removed* from a preset outright (as opposed
to muted, frozen, or reseeded, which Shoal does handle). If a track
happens to be mid-note at that exact moment, there's no opportunity to
send its note-off - the same situation as unplugging a MIDI cable
mid-note. Everything else - normal playing, muting, freezing,
reseeding, changing a track's MIDI channel or the global destination
live - closes notes cleanly.

### Currents (v1.2)
Each track can output a **Current** - a slow, smooth, seeded drift CV,
0-10V, on its own bus (*Routing* page, *Current out*). Think of it as
"the water the melody swims in": a gentle wandering voltage for filter
cutoffs, VCA levels, send amounts, or anything else that wants slow
ambient movement alongside the melody, without patching in a separate
LFO.

The Shoal twist versus any LFO: a Current is derived from the track's
**own Seed**, so it repeats exactly with every pass, survives power
cycles, and is shareable by number - seed 347 always comes with the
same current, just like it always comes with the same melody. A fresh
target lands every time the track advances a step - whether or not
that step actually plays a note, so the Current keeps flowing even on
a muted or low-Chance track - and it eases smoothly from wherever it
was towards that target over the step, never jumping.

A Current moves at the **track's own pace**: a fast Rate gives a
quicker, more restless drift; a slow one gives a long, gentle swell.
A follower's Current is its own - it does **not** inherit its source
track's current, even though it borrows the source's melody, so two
followers of the same track get independently-moving water. Currents
hold their last value through a Freeze, same as the melody.

Like every output, *Current out* defaults to **None** - a track sends
nothing until you assign it.

### End-of-sequence (v1.2)
Each track can output **EOS** - a short 5V trigger every time the
track completes a full cycle of its **Length** (*Routing* page, *EOS
out*). Use it to advance another sequencer, ping an envelope once per
phrase, or trigger a reseed on a *different* track for cascading
pattern changes.

EOS counts **advances, not loop origin** - a deliberate difference
from how reseeds/Evolve/Breathe key off the loop-origin flag elsewhere
in Shoal. Random, Drunk, Pools and Skitter have no fixed loop shape, so
their loop-origin flag is true on almost every step; wiring EOS to it
there would pulse almost continuously rather than marking anything
meaningful. Counting Length advances instead gives every one of the 15
Direction modes the same evenly-spaced pulse - Forwards, Reverse,
Pendulum and the rest of the ordered modes get it exactly where you'd
expect (their own loop origin); the four unordered modes get it at
the same steady interval, giving them a sense of "phrase" they don't
otherwise have.

EOS isn't gated by Chance or Mute - it marks the sequence's structure,
not which steps happen to sound, so it keeps a steady pulse even on a
sparse or muted track. It holds silent through a Freeze (nothing
advances, so nothing completes a cycle) and, like every output,
defaults to **None**.

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
parameter - all 233 of them - is CV/MIDI-mappable and saved in presets.
Track parameters are prefixed `T1…T8` in mapping menus. (See *Mappings
worth trying* in the Recipes section.)

### Global
| Parameter | Range | What it does |
|---|---|---|
| Clock source | External / Internal / MIDI | Where ticks come from. MIDI locks to incoming realtime MIDI clock (24 PPQN); Start resets to step 1, Continue resumes, Stop halts - Run must also be On |
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
| MIDI velocity | 1–127 | Fixed velocity for every note any track sends (new in v1.2; global, not per-track - see the MIDI section above) |
| MIDI dest | 16 combinations | Which MIDI output(s) receive every track's notes: any mix of Breakout, Bus, USB and Internal (new in v1.2; global, not per-track) |

### Track 1–8 (each)
| Parameter | Range | What it does |
|---|---|---|
| Length | 1–64 | Steps in this track's loop |
| Rate | /64 … ×64, plus /5.3, /2.6, /1.5, ×1.25, ×1.3, ×1.5, ×2.6, ×5.3 in true ascending order | Speed relative to the master clock (×1 = one step per beat) |
| Direction | 15 modes | Forwards · Reverse · Pendulum (ends play twice) · Random · Drunk (50% on / 25% repeat / 25% back) · Pong (ends play once) · Tide (loop start drifts one step per pass) · Shuffle (every step once per pass, reshuffled) · Pools (dwells in a small pocket, then hops) · Stride (Hopscotch: overlapping two-ahead-one-back crawl) · Gravity (seed-derived pull back to step 1) · Converge (folds inward from both ends) · Diverge (unfolds outward from the middle) · Skitter (like Random, never repeats immediately) · Anchor (alternates step 1 with each other step) |
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

Each routing page also carries the track's **MIDI note out** (new in
v1.2 - see the MIDI section above). Velocity and destination are
global settings (*Global* page, above), shared by every track - only
the channel is set per track:

| Parameter | Range | What it does |
|---|---|---|
| MIDI channel | 0–16 | 0 = off (default). 1–16 = the MIDI channel this track's notes send on |

Each routing page also carries the track's **Current out** (new in
v1.2 - see the Currents section above). Like End-of-sequence, it
defaults to None and is Replace-only - no Add/Replace mode, a track
always owns its Current bus outright:

| Parameter | Range | What it does |
|---|---|---|
| Current out | bus / none | Which bus receives this track's Current - a slow, seeded 0-10V drift CV |

Each routing page also carries the track's **EOS out** (new in v1.2 -
see the End-of-sequence section above). Unlike the other outputs it
has no Add/Replace mode - a trigger bus always owns whatever it's
patched to:

| Parameter | Range | What it does |
|---|---|---|
| EOS out | bus / none | Which bus receives a 5V trigger every time this track completes a full Length cycle |

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
Every one of Shoal's 233 parameters is CV/MIDI-mappable through the
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

**My tracks sound like different rates after updating from v1.1.0.**
Expected - see the upgrade note at the top of this manual. v1.2
reordered the Rate dial into true ascending order, which moved almost
every rate to a new position, ×1 (the default) included. Old presets
load with whatever Rate now sits at the position they saved - go
through each track and reset Rate to what you want.

**Nothing is moving on screen.**
Run is Off, or *Clock source* is `External` and no clock is arriving on
the input the *Clock input* parameter points at, or *Clock source* is
`MIDI` and no realtime MIDI clock (and a Start or Continue) has arrived
yet. (A fresh Shoal defaults to `Internal` and runs on its own.)

**No MIDI notes, even with a channel set.**
*MIDI dest* is `None`, or set to destinations nothing is listening on.
Check the track isn't muted or soloed-out either - MIDI note out follows
the same mute/solo logic as the CV gate.

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
- 8 pitch + 8 gate + 8 Current + 8 EOS outputs plus the clock output,
  freely assignable to any bus (Current and EOS are Replace-only; the
  rest are Add or Replace)
- Current outputs: 0-10V, seeded per track, independent of Follow (a
  follower generates its own current from its own seed)
- EOS outputs: 5V trigger, fired from a per-track "every Length
  advances" counter - independent of Direction mode, not gated by
  Chance or Mute
- MIDI note out: per track, 1 note at a time, per-track channel;
  velocity and destination are global (Breakout, Bus, USB, Internal,
  any combination)
- MIDI clock in: 24 PPQN realtime clock as an alternative to CV or
  internal clocking, with Start/Continue/Stop transport support
- Memory: under 0.5KB per instance; CPU: light
- Algorithm guid: `Shol`

---

*Shoal is an Ormer Modular instrument - named for the ormer, the Guernsey
shellfish the sea only reveals on the lowest tides of the year.*

Happy swimming. 🐟
