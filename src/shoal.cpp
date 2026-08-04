/*
 * Shoal - an 8-track generative melody sequencer plug-in for the
 * Expert Sleepers disting NT.
 *
 * An Ormer Modular instrument. Made in Guernsey.
 *
 * Eight voices moving as one: tracks generate seeded random melodies and
 * can follow one another like fish in a shoal; a reseed is the shoal
 * turning.
 *
 *  - 8 tracks, each generating a seeded random melody: 64 steps max,
 *    per-track length (1-64) and clock rate (/64 ... x64)
 *  - Patterns are a pure function of the track's Seed parameter, so they
 *    loop identically and are recalled exactly by presets
 *  - Per-track CHANCE, NOTE-/NOTE+ (scale steps), OCTAVE-/OCTAVE+,
 *    NOTE CHANCE, GATE length, TIE chance, fixed octave offset
 *  - Linked-follow sampling: a track can mirror another track's material
 *    (reseeds propagate) filtered through its own settings
 *  - Global scale quantisation (all tracks), external or internal clock,
 *    reset input, per-track and global reseed (armed, land at loop start)
 *  - Pitch CV at 1V/oct (0V = C3), gates 5V; 16 individually routable outputs
 *  - Custom UI on pots and encoders; buttons 1-4 left to the OS
 *  - Idle screensaver: the shoal itself swims the screen, one fish per
 *    track, darting on every note - an ambient view of the patch
 *
 * MIT License. Built against the distingNT_API (kNT_apiVersionCurrent).
 */

#include <stddef.h>
#include <new>
#include <distingnt/api.h>

enum
{
	kNumTracks = 8,
	kMaxSteps = 64,
};

// ---------------------------------------------------------------------------
// Parameters
// ---------------------------------------------------------------------------

enum
{
	kGClockSource,
	kGBPM,
	kGRun,
	kGFreeze,			// hold every sounding note as a chord; nothing advances
	kGScale,
	kGRoot,
	kGWeight,			// consonance gravity: % of notes snapped to root/3rd/5th
	kGSaver,			// idle screensaver: the shoal swims the screen
	kGClockIn,
	kGResetIn,
	kGReseedAll,
	kNumGlobalParams,
};

enum
{
	kTLength,
	kTRate,
	kTDirection,
	kTChance,
	kTBreathe,			// % chance a whole pass rests
	kTNote,				// bipolar: magnitude = probability + reach, sign = direction
	kTOct,				// bipolar: magnitude = probability + reach, sign = direction
	kTEvolve,			// % of steps that re-roll themselves each pass
	kTGate,
	kTTie,
	kTSlop,
	kTOctave,
	kTTrans,			// fixed transpose, in scale degrees - stays in key
	kTSource,
	kTMute,
	kTSeed,
	kNumTrackParams,
};

enum
{
	kTrackBase = kNumGlobalParams,
	kRoutingBase = kTrackBase + kNumTracks * kNumTrackParams,
	kNumRoutingParams = 4,				// gate out, gate mode, pitch out, pitch mode (gate first - NT convention)
	// Clock out is appended AFTER the routing block so every earlier
	// parameter keeps its index - beta.8 presets load unchanged
	kGClockOut = kRoutingBase + kNumTracks * kNumRoutingParams,
	kGClockOutMode,
	kNumParameters,
};

#define TP( t, off )	( kTrackBase + (t) * kNumTrackParams + (off) )
#define RP( t, off )	( kRoutingBase + (t) * kNumRoutingParams + (off) )

static char const * const enumStringsClockSource[] = { "External", "Internal" };
static char const * const enumStringsOffOn[] = { "Off", "On" };
static char const * const enumStringsSaver[] = { "Off", "1 min", "5 min" };
static char const * const enumStringsRate[] = {
	"/64", "/32", "/16", "/8", "/7", "/6", "/5", "/4", "/3", "/2",
	"x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x16", "x32", "x64",
};
static const uint8_t rateDiv[]  = { 64, 32, 16, 8, 7, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
static const uint8_t rateMult[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 16, 32, 64 };
enum { kNumRates = 21, kRateX1 = 10 };

enum
{
	kDirForwards,
	kDirReverse,
	kDirPendulum,		// endpoints play twice on the turn
	kDirRandom,
	kDirDrunk,			// 50% advance, 25% repeat, 25% back up
	kDirPong,			// endpoints play once on the turn
	kDirTide,			// forwards, rotating the loop start one step per pass
	kDirShuffle,		// every step once per pass, new seeded order each pass
	kDirPools,			// dwell in a small pocket of steps, then hop to another
};

static char const * const enumStringsDirection[] = {
	"Forwards", "Reverse", "Pendulum", "Random", "Drunk", "Pong",
	"Tide", "Shuffle", "Pools",
};

static char const * const enumStringsScale[] = {
	"Chromatic",
	"Major",
	"Natural minor",
	"Harmonic minor",
	"Dorian",
	"Phrygian",
	"Lydian",
	"Mixolydian",
	"Major pentatonic",
	"Minor pentatonic",
	"Blues",
	"Hirajoshi",
	"In-Sen",
};

// Bitmasks of allowed pitch classes, relative to the root note.
static const uint16_t scaleMasks[] = {
	0xFFF,
	(1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<7)|(1<<9)|(1<<11),		// Major
	(1<<0)|(1<<2)|(1<<3)|(1<<5)|(1<<7)|(1<<8)|(1<<10),		// Natural minor
	(1<<0)|(1<<2)|(1<<3)|(1<<5)|(1<<7)|(1<<8)|(1<<11),		// Harmonic minor
	(1<<0)|(1<<2)|(1<<3)|(1<<5)|(1<<7)|(1<<9)|(1<<10),		// Dorian
	(1<<0)|(1<<1)|(1<<3)|(1<<5)|(1<<7)|(1<<8)|(1<<10),		// Phrygian
	(1<<0)|(1<<2)|(1<<4)|(1<<6)|(1<<7)|(1<<9)|(1<<11),		// Lydian
	(1<<0)|(1<<2)|(1<<4)|(1<<5)|(1<<7)|(1<<9)|(1<<10),		// Mixolydian
	(1<<0)|(1<<2)|(1<<4)|(1<<7)|(1<<9),						// Major pentatonic
	(1<<0)|(1<<3)|(1<<5)|(1<<7)|(1<<10),					// Minor pentatonic
	(1<<0)|(1<<3)|(1<<5)|(1<<6)|(1<<7)|(1<<10),				// Blues
	(1<<0)|(1<<2)|(1<<3)|(1<<7)|(1<<8),						// Hirajoshi
	(1<<0)|(1<<1)|(1<<5)|(1<<7)|(1<<10),					// In-Sen
};

static char const * const enumStringsSource[] = {
	"Off", "Track 1", "Track 2", "Track 3", "Track 4",
	"Track 5", "Track 6", "Track 7", "Track 8",
};

#define TRACK_PARAMS( seedDef ) \
	{ .name = "Length", .min = 1, .max = kMaxSteps, .def = 16, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Rate", .min = 0, .max = kNumRates - 1, .def = kRateX1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsRate }, \
	{ .name = "Direction", .min = 0, .max = 8, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsDirection }, \
	{ .name = "Chance", .min = 0, .max = 100, .def = 100, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Breathe", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Note", .min = -100, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Oct", .min = -100, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Evolve", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Gate length", .min = 5, .max = 95, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Tie", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Slop", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Octave", .min = -3, .max = 3, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Transpose", .min = -7, .max = 7, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Sample source", .min = 0, .max = 8, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsSource }, \
	{ .name = "Mute", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsOffOn }, \
	{ .name = "Seed", .min = 0, .max = 999, .def = seedDef, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL },

// Like NT_PARAMETER_CV_OUTPUT_WITH_MODE, but: outputs default to None (a
// track touches no bus until you assign it), and the mode defaults to
// Replace - a CV sequencer should own its busses, not sum onto them.
#define ORMER_CV_OUTPUT_REPLACE( n ) \
	NT_PARAMETER_IO( n, 0, 0, kNT_unitCvOutput ) \
	{ .name = n " mode", .min = 0, .max = 1, .def = 1, .unit = kNT_unitOutputMode, .scaling = 0, .enumStrings = NULL },

#define TRACK_ROUTING() \
	ORMER_CV_OUTPUT_REPLACE( "Gate out" ) \
	ORMER_CV_OUTPUT_REPLACE( "Pitch out" )

#define PARAM_TABLE( bpmName ) \
	{ .name = "Clock source", .min = 0, .max = 1, .def = 1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsClockSource }, \
	{ .name = bpmName, .min = 20, .max = 300, .def = 120, .unit = kNT_unitBPM, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Run", .min = 0, .max = 1, .def = 1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsOffOn }, \
	{ .name = "Freeze", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsOffOn }, \
	{ .name = "Scale", .min = 0, .max = (int16_t)(ARRAY_SIZE(scaleMasks)-1), .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsScale }, \
	{ .name = "Root note", .min = 0, .max = 127, .def = 48, .unit = kNT_unitMIDINote, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Weight", .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = NULL }, \
	{ .name = "Screensaver", .min = 0, .max = 2, .def = 1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = enumStringsSaver }, \
	NT_PARAMETER_CV_INPUT( "Clock input", 1, 1 ) \
	NT_PARAMETER_CV_INPUT( "Reset input", 0, 2 ) \
	{ .name = "Reseed all", .min = 0, .max = 999, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = NULL }, \
	TRACK_PARAMS( 1 )   TRACK_PARAMS( 98 )  TRACK_PARAMS( 195 ) TRACK_PARAMS( 292 ) \
	TRACK_PARAMS( 389 ) TRACK_PARAMS( 486 ) TRACK_PARAMS( 583 ) TRACK_PARAMS( 680 ) \
	TRACK_ROUTING() TRACK_ROUTING() TRACK_ROUTING() TRACK_ROUTING() \
	TRACK_ROUTING() TRACK_ROUTING() TRACK_ROUTING() TRACK_ROUTING() \
	ORMER_CV_OUTPUT_REPLACE( "Clock out" )

static const _NT_parameter parameters[] = { PARAM_TABLE( "BPM" ) };

static_assert( ARRAY_SIZE(parameters) == kNumParameters, "parameter count mismatch" );

static const uint8_t pageGlobal[] = {
	kGClockSource, kGBPM, kGRun, kGFreeze, kGScale, kGRoot, kGWeight,
	kGSaver, kGClockIn, kGResetIn, kGClockOut, kGClockOutMode, kGReseedAll,
};

#define TRACK_PAGE( t ) \
	{ TP(t,0), TP(t,1), TP(t,2), TP(t,3), TP(t,4), TP(t,5), TP(t,6), TP(t,7), \
	  TP(t,8), TP(t,9), TP(t,10), TP(t,11), TP(t,12), TP(t,13), TP(t,14), TP(t,15) }
#define ROUT_PAGE( t ) \
	{ RP(t,0), RP(t,1), RP(t,2), RP(t,3) }

static const uint8_t pageT0[] = TRACK_PAGE(0);
static const uint8_t pageT1[] = TRACK_PAGE(1);
static const uint8_t pageT2[] = TRACK_PAGE(2);
static const uint8_t pageT3[] = TRACK_PAGE(3);
static const uint8_t pageT4[] = TRACK_PAGE(4);
static const uint8_t pageT5[] = TRACK_PAGE(5);
static const uint8_t pageT6[] = TRACK_PAGE(6);
static const uint8_t pageT7[] = TRACK_PAGE(7);
static const uint8_t pageR0[] = ROUT_PAGE(0);
static const uint8_t pageR1[] = ROUT_PAGE(1);
static const uint8_t pageR2[] = ROUT_PAGE(2);
static const uint8_t pageR3[] = ROUT_PAGE(3);
static const uint8_t pageR4[] = ROUT_PAGE(4);
static const uint8_t pageR5[] = ROUT_PAGE(5);
static const uint8_t pageR6[] = ROUT_PAGE(6);
static const uint8_t pageR7[] = ROUT_PAGE(7);

static const _NT_parameterPage pages[] = {
	{ .name = "Global",    .numParams = ARRAY_SIZE(pageGlobal), .group = 1, .params = pageGlobal },
	{ .name = "Track 1",   .numParams = kNumTrackParams, .group = 2, .params = pageT0 },
	{ .name = "Track 2",   .numParams = kNumTrackParams, .group = 2, .params = pageT1 },
	{ .name = "Track 3",   .numParams = kNumTrackParams, .group = 2, .params = pageT2 },
	{ .name = "Track 4",   .numParams = kNumTrackParams, .group = 2, .params = pageT3 },
	{ .name = "Track 5",   .numParams = kNumTrackParams, .group = 2, .params = pageT4 },
	{ .name = "Track 6",   .numParams = kNumTrackParams, .group = 2, .params = pageT5 },
	{ .name = "Track 7",   .numParams = kNumTrackParams, .group = 2, .params = pageT6 },
	{ .name = "Track 8",   .numParams = kNumTrackParams, .group = 2, .params = pageT7 },
	{ .name = "Routing 1", .numParams = kNumRoutingParams, .group = 3, .params = pageR0 },
	{ .name = "Routing 2", .numParams = kNumRoutingParams, .group = 3, .params = pageR1 },
	{ .name = "Routing 3", .numParams = kNumRoutingParams, .group = 3, .params = pageR2 },
	{ .name = "Routing 4", .numParams = kNumRoutingParams, .group = 3, .params = pageR3 },
	{ .name = "Routing 5", .numParams = kNumRoutingParams, .group = 3, .params = pageR4 },
	{ .name = "Routing 6", .numParams = kNumRoutingParams, .group = 3, .params = pageR5 },
	{ .name = "Routing 7", .numParams = kNumRoutingParams, .group = 3, .params = pageR6 },
	{ .name = "Routing 8", .numParams = kNumRoutingParams, .group = 3, .params = pageR7 },
};

static const _NT_parameterPages parameterPages = {
	.numPages = ARRAY_SIZE(pages),
	.pages = pages,
};

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

struct TrackState
{
	uint32_t	activeSeed;
	int32_t		pendingSeed;		// -1 = none armed
	int32_t		pos;				// -1 = not started
	int8_t		dir;				// +1 / -1, for Pendulum and Pong
	uint8_t		phase;				// walk counter within a pass (Tide, Shuffle)
	uint8_t		rot;				// Tide's rotation offset
	uint16_t	pass;				// completed-pass counter (Shuffle)
	uint8_t		poolStart, poolLen, poolPos, dwell;		// Pools state
	uint8_t		order[kMaxSteps];	// Shuffle's permutation for the current pass
	uint8_t		epoch[kMaxSteps];	// Evolve: per-step re-roll counters
	bool		resting;			// Breathe: this whole pass is silent
	uint32_t	extraPending;		// remaining multiplied sub-steps this master tick
	uint32_t	subPeriod;
	uint32_t	subCountdown;
	uint32_t	stepPeriod;			// samples per step, for gate length
	uint32_t	gateRemaining;
	uint32_t	slopCountdown;		// samples until a slop-delayed step fires, 0 = none
	uint32_t	slopPeriod;			// step period for the delayed step
	bool		tieHeld;
	float		pitchVolts;
};

struct _melodySeq_DTC
{
	TrackState	tracks[kNumTracks];
	bool		clockHigh;
	bool		resetHigh;
	bool		resetPrime;			// next master tick restarts all tracks at step 1
	bool		seedsInited;
	int32_t		lastReseedAll;		// -1 until first seen; detects real changes
	int32_t		reseedAllPending;	// -1 none; else the base value to scatter
	uint32_t	tickCount;			// global master tick counter - all dividers phase-lock to it
	uint32_t	uiClock;			// sample counter for real-time UI gestures (holds)
	uint32_t	rng;				// audio-context RNG for Random/Drunk directions
	uint32_t	samplesSinceClock;
	uint32_t	extPeriod;
	uint32_t	intPeriod;
	uint32_t	intCountdown;
	uint32_t	clockPulse;			// samples left of the current Clock out high phase
	uint8_t		scalePcs[12];
	uint8_t		scaleCount;
	uint8_t		stableIdx[3];		// Weight: scale indices of root / third / fifth
	uint8_t		stableCount;
	int16_t		scaleCached;		// scale index the table was built for, -1 = dirty
};

struct _melodySeqAlgorithm : public _NT_algorithm
{
	_melodySeqAlgorithm( _melodySeq_DTC* dtc_ ) : dtc( dtc_ ) {}

	_melodySeq_DTC*	dtc;

	// custom UI state (runtime only)
	int32_t		sel;
	bool		potCaptured[3];
	float		potPrev[3];
	uint32_t	potQuiet[3];		// uiClock until which a pushed pot's value is ignored
	int16_t		potStash[kNumTracks][3];	// dialled values stashed by push-to-home
	bool		potStashValid[kNumTracks][3];
	bool		solo[kNumTracks];
	uint32_t	encLDown, encRDown;	// uiClock at press
	bool		encLHeldPrev, encRHeldPrev;
	bool		encLTurned, encRTurned;
	bool		encLFired, encRFired;	// hold action already fired this press
	uint32_t	uiRng;
	uint32_t	drawTick;

	// screensaver (runtime only)
	uint32_t	lastTouch;			// uiClock at the last control touch / edit
	bool		saverActive;		// the next touch only wakes, nothing else
	bool		selfWrite;			// parameterChanged came from our own UI write
	float		fishX[kNumTracks];
	int16_t		bubX[4];
	int16_t		bubY[4];
};

// ---------------------------------------------------------------------------
// Pattern generation - a pure function of (seed, step, parameters)
// ---------------------------------------------------------------------------

static inline uint32_t hash3( uint32_t a, uint32_t b, uint32_t c )
{
	uint32_t h = a * 0x9E3779B1u ^ b * 0x85EBCA77u ^ c * 0xC2B2AE3Du;
	h ^= h >> 16; h *= 0x7FEB352Du;
	h ^= h >> 15; h *= 0x846CA68Bu;
	h ^= h >> 16;
	return h;
}

static void rebuildScale( _melodySeq_DTC* dtc, int scaleIdx )
{
	uint16_t mask = scaleMasks[scaleIdx];
	uint8_t n = 0;
	for ( int i = 0; i < 12; ++i )
		if ( mask & ( 1 << i ) )
			dtc->scalePcs[n++] = i;
	dtc->scaleCount = n;

	// stable degrees for Weight: the root, plus whichever scale degrees
	// sit nearest a third (3-4 semitones) and a fifth (7)
	dtc->stableCount = 0;
	dtc->stableIdx[dtc->stableCount++] = 0;
	int bestT = -1, bt = 99, bestF = -1, bf = 99;
	for ( int i = 1; i < n; ++i )
	{
		int pc = dtc->scalePcs[i];
		int d3 = pc - 3; if ( d3 < 0 ) d3 = -d3;
		int d4 = pc - 4; if ( d4 < 0 ) d4 = -d4;
		if ( d4 < d3 ) d3 = d4;
		if ( d3 < bt ) { bt = d3; bestT = i; }
		int d7 = pc - 7; if ( d7 < 0 ) d7 = -d7;
		if ( d7 < bf ) { bf = d7; bestF = i; }
	}
	if ( bestT > 0 )
		dtc->stableIdx[dtc->stableCount++] = (uint8_t)bestT;
	if ( bestF > 0 && bestF != bestT )
		dtc->stableIdx[dtc->stableCount++] = (uint8_t)bestF;

	dtc->scaleCached = scaleIdx;
}

// Follow the sample-source chain to the terminal track (linked follow).
// Followers share the source's seed AND its evolution epochs.
static int effectiveTrack( const _melodySeqAlgorithm* pThis, int t )
{
	const int16_t* v = pThis->v;
	int cur = t;
	for ( int depth = 0; depth < kNumTracks; ++depth )
	{
		int src = v[ TP( cur, kTSource ) ];
		if ( src == 0 || src - 1 == cur )
			break;
		cur = src - 1;
	}
	return cur;
}

static uint32_t effectiveSeed( const _melodySeqAlgorithm* pThis, int t )
{
	return pThis->dtc->tracks[ effectiveTrack( pThis, t ) ].activeSeed;
}

struct StepEval
{
	bool	fires;
	bool	tie;
	int16_t	note;		// MIDI note number
	int8_t	barH;		// 0 (rest) or 1..5, for the display
};

static void evalStep( const _melodySeqAlgorithm* pThis, int t, int step, StepEval& out )
{
	const int16_t* v = pThis->v;
	_melodySeq_DTC* dtc = pThis->dtc;

	int eff = effectiveTrack( pThis, t );
	uint32_t seed = dtc->tracks[eff].activeSeed;
	const uint8_t* ep = dtc->tracks[eff].epoch;		// Evolve counters ride the follow chain

	// Tide: the rhythm stays anchored while the pitch stream drifts one
	// step through it per pass (rot). Other modes: pitchStep == step.
	int pitchStep = step;
	if ( v[ TP( t, kTDirection ) ] == kDirTide )
	{
		int length = v[ TP( t, kTLength ) ];
		if ( length > 1 )
			pitchStep = ( step + dtc->tracks[t].rot ) % length;
	}

	// evolved steps hash differently: the epoch mixes into the roll input
	uint32_t esR = (uint32_t)step | ( (uint32_t)ep[step] << 8 );
	uint32_t esP = (uint32_t)pitchStep | ( (uint32_t)ep[pitchStep] << 8 );

	out.fires = ( hash3( seed, esR, 0xF17Eu ) % 100 ) < (uint32_t)v[ TP( t, kTChance ) ];
	out.tie = ( hash3( seed, esR, 0x071Eu ) % 100 ) < (uint32_t)v[ TP( t, kTTie ) ];
	if ( !out.fires )
	{
		out.note = 0;
		out.barH = 0;
		return;
	}

	int k = dtc->scaleCount ? dtc->scaleCount : 12;

	// The base melody: two octaves of the scale centred on the root, so
	// lines orbit the root rather than hanging above it. Rolled as a
	// fraction so the contour survives scale changes.
	uint32_t rb = hash3( seed, esP, 0xBA5Eu ) & 1023;
	int degree = (int)( ( rb * (uint32_t)( 2 * k ) ) >> 10 ) - k;

	// Note / Oct amounts ADD random deviation on top, non-destructively -
	// at 0 the pure base melody returns. Magnitude sets both how OFTEN a
	// note is varied and how FAR it may reach; sign sets the direction.
	int noteAmt = v[ TP( t, kTNote ) ];
	int octAmt = v[ TP( t, kTOct ) ];
	int nMag = noteAmt < 0 ? -noteAmt : noteAmt;
	int oMag = octAmt < 0 ? -octAmt : octAmt;

	int o = 0;
	if ( nMag > 0 && ( hash3( seed, esP, 0xA221u ) % 100 ) < (uint32_t)nMag )
	{
		int reach = 1 + ( nMag * 11 ) / 100;						// 1..12 scale steps
		int d = 1 + (int)( hash3( seed, esP, 0x0007u ) % (uint32_t)reach );
		degree += ( noteAmt > 0 ) ? d : -d;
	}
	if ( oMag > 0 && ( hash3( seed, esP, 0x0C7Au ) % 100 ) < (uint32_t)oMag )
	{
		int reachO = 1 + ( oMag * 4 ) / 100;						// 1..5 octaves
		o = 1 + (int)( hash3( seed, esP, 0x0C7Bu ) % (uint32_t)reachO );
		if ( octAmt < 0 )
			o = -o;
	}

	// Weight: consonance gravity - snap this note to the nearest stable
	// degree (root / third / fifth), Weight% of the time
	int w = v[kGWeight];
	if ( w > 0 && ( hash3( seed, esP, 0x0337u ) % 100 ) < (uint32_t)w )
	{
		int q2 = degree / k, r2 = degree - q2 * k;
		if ( r2 < 0 ) { r2 += k; q2 -= 1; }
		int best = 0, bd = 99;
		for ( int i2 = 0; i2 < dtc->stableCount; ++i2 )
		{
			int cand = dtc->stableIdx[i2];
			for ( int oc = 0; oc < 2; ++oc, cand += k )		// this octave's, and the root above
			{
				int d2 = r2 - cand; if ( d2 < 0 ) d2 = -d2;
				if ( d2 < bd ) { bd = d2; best = cand; }
			}
		}
		degree = q2 * k + best;
	}

	degree += v[ TP( t, kTTrans ) ];

	// scale-degree walk from the root
	int q = degree / k, r = degree - q * k;
	if ( r < 0 ) { r += k; q -= 1; }
	int note = v[kGRoot] + 12 * ( v[ TP( t, kTOctave ) ] + q + o ) + dtc->scalePcs[r];
	// fold out-of-range notes back by octaves, preserving the pitch class
	while ( note < 0 ) note += 12;
	while ( note > 127 ) note -= 12;
	out.note = note;

	int maxReach = nMag > 0 ? 1 + ( nMag * 11 ) / 100 : 0;
	int lo = -k - ( noteAmt < 0 ? maxReach : 0 );
	int hi = k - 1 + ( noteAmt > 0 ? maxReach : 0 );
	out.barH = (int8_t)( 1 + ( ( degree - lo ) * 4 ) / ( hi > lo ? hi - lo : 1 ) );
	if ( out.barH < 1 ) out.barH = 1;
	if ( out.barH > 5 ) out.barH = 5;
}

// ---------------------------------------------------------------------------
// Factory callbacks
// ---------------------------------------------------------------------------

void calculateRequirements( _NT_algorithmRequirements& req, const int32_t* specifications )
{
	req.numParameters = kNumParameters;
	req.sram = sizeof(_melodySeqAlgorithm);
	req.dram = 0;
	req.dtc = sizeof(_melodySeq_DTC);
	req.itc = 0;
}

_NT_algorithm* construct( const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications )
{
	_melodySeq_DTC* dtc = (_melodySeq_DTC*)ptrs.dtc;
	for ( int t = 0; t < kNumTracks; ++t )
	{
		TrackState& tr = dtc->tracks[t];
		tr.activeSeed = 0;
		tr.pendingSeed = -1;
		tr.pos = -1;
		tr.dir = 1;
		tr.phase = 0;
		tr.rot = 0;
		tr.pass = 0;
		tr.poolStart = 0; tr.poolLen = 0; tr.poolPos = 0; tr.dwell = 0;
		for ( int s = 0; s < kMaxSteps; ++s )
			tr.epoch[s] = 0;
		tr.resting = false;
		tr.extraPending = 0;
		tr.subPeriod = 0;
		tr.subCountdown = 0;
		tr.stepPeriod = 0;
		tr.gateRemaining = 0;
		tr.slopCountdown = 0;
		tr.slopPeriod = 0;
		tr.tieHeld = false;
		tr.pitchVolts = 0.0f;
	}
	dtc->clockHigh = false;
	dtc->resetHigh = false;
	dtc->resetPrime = false;
	dtc->seedsInited = false;
	dtc->lastReseedAll = -1;
	dtc->reseedAllPending = -1;
	dtc->tickCount = 0;
	dtc->uiClock = 0;
	dtc->rng = 0x7A31C0DEu;
	dtc->samplesSinceClock = 0;
	dtc->extPeriod = 0;
	dtc->intPeriod = NT_globals.sampleRate / 8;
	dtc->intCountdown = 1;
	dtc->clockPulse = 0;
	dtc->scaleCached = -1;
	dtc->scaleCount = 0;

	_melodySeqAlgorithm* alg = new (ptrs.sram) _melodySeqAlgorithm( dtc );
	alg->parameters = parameters;
	alg->parameterPages = &parameterPages;
	alg->sel = 0;
	for ( int i = 0; i < 3; ++i )
	{
		alg->potCaptured[i] = false;
		alg->potPrev[i] = 0.0f;
		alg->potQuiet[i] = 0;
	}
	for ( int t2 = 0; t2 < kNumTracks; ++t2 )
		for ( int i = 0; i < 3; ++i )
		{
			alg->potStash[t2][i] = 0;
			alg->potStashValid[t2][i] = false;
		}
	for ( int t = 0; t < kNumTracks; ++t )
		alg->solo[t] = false;
	alg->encLDown = 0; alg->encRDown = 0;
	alg->encLHeldPrev = false; alg->encRHeldPrev = false;
	alg->encLTurned = false; alg->encRTurned = false;
	alg->encLFired = false; alg->encRFired = false;
	alg->uiRng = 0x1D872B41u;
	alg->drawTick = 0;
	alg->lastTouch = 0;
	alg->saverActive = false;
	alg->selfWrite = false;
	for ( int t = 0; t < kNumTracks; ++t )
		alg->fishX[t] = 24.0f + 30.0f * t;
	for ( int b = 0; b < 4; ++b )
	{
		alg->bubX[b] = (int16_t)( 40 + 60 * b );
		alg->bubY[b] = (int16_t)( 12 + 14 * b );
	}
	return alg;
}

void parameterChanged( _NT_algorithm* self, int p )
{
	_melodySeqAlgorithm* pThis = (_melodySeqAlgorithm*)self;
	_melodySeq_DTC* dtc = pThis->dtc;

	// parameter edits postpone the screensaver - except our own UI writes
	// (their gestures already counted, and pot jitter can flicker a value)
	// and the performance parameters commonly driven by CV mappings
	// (Seed, Reseed all, Freeze), so a generative patch can still fall
	// asleep and show the shoal
	bool isSeed = p >= kTrackBase && p < kRoutingBase
			&& ( ( p - kTrackBase ) % kNumTrackParams ) == kTSeed;
	if ( !pThis->selfWrite && !isSeed && p != kGReseedAll && p != kGFreeze )
		pThis->lastTouch = dtc->uiClock;

	if ( p == kGScale )
	{
		dtc->scaleCached = -1;
		return;
	}
	if ( p == kGBPM )
	{
		int bpm = pThis->v[kGBPM];
		if ( bpm > 0 )
			dtc->intPeriod = ( NT_globals.sampleRate * 60 ) / (uint32_t)bpm;	// quarter notes: x1 = one step per beat
		return;
	}
	if ( p == kGReseedAll )
	{
		// Never touch other parameters from inside parameterChanged() - the
		// host may be mid-initialisation or mid-preset-load. Just note the
		// request; step() performs the scatter. Also ignore the initial
		// value (algorithm add / preset load) so saved seeds survive.
		int32_t value = pThis->v[kGReseedAll];
		if ( dtc->lastReseedAll >= 0 && value != dtc->lastReseedAll )
			dtc->reseedAllPending = value;
		dtc->lastReseedAll = value;
		return;
	}
	if ( p >= kTrackBase && p < kRoutingBase )
	{
		int t = ( p - kTrackBase ) / kNumTrackParams;
		int off = ( p - kTrackBase ) % kNumTrackParams;
		if ( off == kTSeed )
		{
			// arm: the new seed lands when the track wraps to step 1
			dtc->tracks[t].pendingSeed = pThis->v[p];
		}
	}
}

// ---------------------------------------------------------------------------
// Audio / sequencing
// ---------------------------------------------------------------------------

// Seeded Fisher-Yates permutation for Shuffle: every step exactly once per
// pass, in an order that changes each pass but is fully reproducible.
static void buildShuffle( TrackState& tr, int length, uint32_t seed, uint32_t pass )
{
	for ( int i = 0; i < length; ++i )
		tr.order[i] = (uint8_t)i;
	uint32_t r = hash3( seed, pass, 0x5FFEu );
	for ( int i = length - 1; i > 0; --i )
	{
		r = r * 1664525u + 1013904223u;
		int j = (int)( ( r >> 16 ) % (uint32_t)( i + 1 ) );
		uint8_t tmp = tr.order[i]; tr.order[i] = tr.order[j]; tr.order[j] = tmp;
	}
}

static void advanceTrack( _melodySeqAlgorithm* pThis, int t, uint32_t stepPeriod, bool anySolo )
{
	const int16_t* v = pThis->v;
	_melodySeq_DTC* dtc = pThis->dtc;
	TrackState& tr = dtc->tracks[t];

	int length = v[ TP( t, kTLength ) ];
	int mode = v[ TP( t, kTDirection ) ];

	if ( tr.pos >= length )
		tr.pos = length - 1;			// length was shortened mid-flight

	if ( tr.pos < 0 )
	{
		// first step after start/reset: the mode's origin
		tr.dir = 1;
		tr.phase = 0;
		tr.rot = 0;
		tr.pass = 0;
		tr.dwell = 0;
		tr.poolPos = 0;
		if ( mode == kDirShuffle )
		{
			buildShuffle( tr, length, effectiveSeed( pThis, t ), 0 );
			tr.pos = tr.order[0] % length;
		}
		else
			tr.pos = ( mode == kDirReverse ) ? length - 1 : 0;
	}
	else switch ( mode )
	{
	default:
	case kDirForwards:
		tr.pos = ( tr.pos + 1 ) % length;
		break;
	case kDirReverse:
		tr.pos = ( tr.pos + length - 1 ) % length;
		break;
	case kDirPendulum:					// endpoints play twice on the turn
		if ( length > 1 )
		{
			if ( tr.dir > 0 )
			{
				if ( tr.pos >= length - 1 ) tr.dir = -1;	// repeat the end
				else tr.pos += 1;
			}
			else
			{
				if ( tr.pos <= 0 ) tr.dir = 1;				// repeat the start
				else tr.pos -= 1;
			}
		}
		break;
	case kDirPong:						// endpoints play once on the turn
		if ( length > 1 )
		{
			if ( tr.dir > 0 )
			{
				if ( tr.pos >= length - 1 ) { tr.dir = -1; tr.pos -= 1; }
				else tr.pos += 1;
			}
			else
			{
				if ( tr.pos <= 0 ) { tr.dir = 1; tr.pos += 1; }
				else tr.pos -= 1;
			}
		}
		break;
	case kDirRandom:
		dtc->rng = dtc->rng * 1664525u + 1013904223u;
		tr.pos = (int)( ( dtc->rng >> 16 ) % (uint32_t)length );
		break;
	case kDirDrunk:
	{
		dtc->rng = dtc->rng * 1664525u + 1013904223u;
		uint32_t r = ( dtc->rng >> 16 ) % 100;
		if ( r < 50 )
			tr.pos = ( tr.pos + 1 ) % length;
		else if ( r < 75 )
			;								// repeat the step
		else
			tr.pos = ( tr.pos + length - 1 ) % length;
		break;
	}
	case kDirTide:						// pitch stream drifts one step per pass
		tr.pos = ( tr.pos + 1 ) % length;
		if ( tr.pos == 0 && length > 1 )
			tr.rot = (uint8_t)( ( tr.rot + 1 ) % length );
		break;
	case kDirShuffle:					// every step once per pass, reshuffled
		tr.phase = (uint8_t)( ( tr.phase + 1 ) % length );
		tr.pos = tr.order[ tr.phase ] % length;		// re-dealt below at the pass boundary
		break;
	case kDirPools:						// dwell in a pocket, then hop
	{
		if ( tr.dwell == 0 )
		{
			dtc->rng = dtc->rng * 1664525u + 1013904223u;
			tr.poolLen = (uint8_t)( 3 + ( ( dtc->rng >> 16 ) % 2 ) );		// 3-4 steps
			dtc->rng = dtc->rng * 1664525u + 1013904223u;
			tr.poolStart = (uint8_t)( ( dtc->rng >> 16 ) % (uint32_t)length );
			dtc->rng = dtc->rng * 1664525u + 1013904223u;
			tr.dwell = (uint8_t)( tr.poolLen * ( 2 + ( ( dtc->rng >> 16 ) % 3 ) ) );	// 2-4 laps
			tr.poolPos = 0;
		}
		tr.pos = ( tr.poolStart + tr.poolPos ) % length;
		tr.poolPos = (uint8_t)( ( tr.poolPos + 1 ) % tr.poolLen );
		tr.dwell -= 1;
		break;
	}
	}

	// armed reseeds land at the mode's loop origin; the unordered modes
	// have none, so they take the new seed on the next step
	bool atOrigin;
	switch ( mode )
	{
	case kDirReverse:	atOrigin = ( tr.pos == length - 1 ); break;
	case kDirShuffle:	atOrigin = ( tr.phase == 0 ); break;
	case kDirRandom:
	case kDirDrunk:
	case kDirPools:		atOrigin = true; break;
	default:			atOrigin = ( tr.pos == 0 ); break;
	}
	if ( atOrigin )
	{
		if ( tr.pendingSeed >= 0 )
		{
			// a reseed starts a fresh lineage: clear evolution and breathing
			tr.activeSeed = (uint32_t)tr.pendingSeed;
			tr.pendingSeed = -1;
			tr.pass = 0;
			tr.resting = false;
			for ( int s = 0; s < kMaxSteps; ++s )
				tr.epoch[s] = 0;
			if ( mode == kDirShuffle )
			{
				buildShuffle( tr, length, effectiveSeed( pThis, t ), 0 );
				tr.pos = tr.order[ tr.phase ] % length;
			}
		}
		else
		{
			tr.pass += 1;
			if ( mode == kDirShuffle )
			{
				buildShuffle( tr, length, effectiveSeed( pThis, t ), tr.pass );
				tr.pos = tr.order[ tr.phase ] % length;
			}
			// Evolve: this pass, some steps quietly re-roll themselves
			int evAmt = v[ TP( t, kTEvolve ) ];
			if ( evAmt > 0 )
			{
				for ( int s = 0; s < length; ++s )
					if ( ( hash3( tr.activeSeed, ( (uint32_t)tr.pass << 8 ) | (uint32_t)s, 0xE01Fu ) % 100 ) < (uint32_t)evAmt )
						tr.epoch[s] += 1;
			}
			// Breathe: maybe this whole pass rests
			int br = v[ TP( t, kTBreathe ) ];
			tr.resting = br > 0
				&& ( hash3( tr.activeSeed, tr.pass, 0xB4EAu ) % 100 ) < (uint32_t)br;
		}
	}

	tr.stepPeriod = stepPeriod;

	StepEval ev;
	evalStep( pThis, t, tr.pos, ev );

	bool muted = v[ TP( t, kTMute ) ] || ( anySolo && !pThis->solo[t] );

	if ( muted || tr.resting || !ev.fires )
	{
		if ( tr.tieHeld )
			tr.gateRemaining = 0;		// a tie ending in a rest closes the gate
		tr.tieHeld = false;
		if ( muted )
			tr.gateRemaining = 0;
		return;
	}

	tr.pitchVolts = ( ev.note - 48 ) * ( 1.0f / 12.0f );

	if ( ev.tie )
	{
		// hold the gate through this step and into the next
		tr.gateRemaining = stepPeriod + stepPeriod / 2;
	}
	else
	{
		uint32_t g = ( stepPeriod * (uint32_t)v[ TP( t, kTGate ) ] ) / 100;
		tr.gateRemaining = g ? g : 1;
	}
	tr.tieHeld = ev.tie;
}

// Fire a step now, or - with Slop - after a seeded per-note delay of up to
// half a step, so the same notes drift by the same amount every loop.
static void scheduleAdvance( _melodySeqAlgorithm* pThis, int t, uint32_t stepPeriod, bool anySolo )
{
	const int16_t* v = pThis->v;
	TrackState& tr = pThis->dtc->tracks[t];

	if ( tr.slopCountdown )
	{
		// a delayed step is still pending (clock sped up) - fire it first
		tr.slopCountdown = 0;
		advanceTrack( pThis, t, tr.slopPeriod, anySolo );
	}

	int slop = v[ TP( t, kTSlop ) ];
	if ( slop > 0 )
	{
		int length = v[ TP( t, kTLength ) ];
		int nextPos = ( tr.pos < 0 ) ? 0 : ( tr.pos + 1 ) % length;
		// 32-bit-safe: no 64-bit division (the NT does not provide libgcc's
		// __aeabi_uldivmod to plug-ins). 64-bit multiply is a native umull.
		uint32_t r10 = hash3( effectiveSeed( pThis, t ), nextPos, 0x5107u ) & 1023;
		uint32_t maxDelay = stepPeriod >> 1;
		if ( maxDelay > 0xFFFFFF )
			maxDelay = 0xFFFFFF;
		uint32_t num = (uint32_t)( ( (uint64_t)maxDelay * ( (uint32_t)slop * r10 ) ) >> 10 );
		uint32_t delay = num / 100u;
		if ( delay > 0 )
		{
			tr.slopCountdown = delay;
			tr.slopPeriod = stepPeriod;
			return;
		}
	}
	advanceTrack( pThis, t, stepPeriod, anySolo );
}

void step( _NT_algorithm* self, float* busFrames, int numFramesBy4 )
{
	_melodySeqAlgorithm* pThis = (_melodySeqAlgorithm*)self;
	_melodySeq_DTC* dtc = pThis->dtc;
	const int16_t* v = pThis->v;

	int numFrames = numFramesBy4 * 4;
	dtc->uiClock += (uint32_t)numFrames;

	if ( !dtc->seedsInited )
	{
		for ( int t = 0; t < kNumTracks; ++t )
		{
			dtc->tracks[t].activeSeed = (uint32_t)v[ TP( t, kTSeed ) ];
			dtc->tracks[t].pendingSeed = -1;
		}
		dtc->seedsInited = true;
	}
	if ( dtc->scaleCached != v[kGScale] )
		rebuildScale( dtc, v[kGScale] );

	if ( dtc->reseedAllPending >= 0 )
	{
		// deferred "reseed all": scatter seeds into the per-track Seed
		// parameters (so presets capture them) from the audio context,
		// where NT_setParameterFromAudio is documented as safe
		uint32_t base = (uint32_t)dtc->reseedAllPending;
		dtc->reseedAllPending = -1;
		int algIdx = NT_algorithmIndex( self );
		if ( algIdx >= 0 )
		{
			for ( int t = 0; t < kNumTracks; ++t )
			{
				int16_t s = (int16_t)( hash3( base, t, 0x5EEDu ) % 1000 );
				NT_setParameterFromAudio( (uint32_t)algIdx, TP( t, kTSeed ) + NT_parameterOffset(), s );
			}
		}
	}

	const float* clockIn = ( v[kGClockIn] > 0 )
			? busFrames + ( v[kGClockIn] - 1 ) * numFrames : NULL;
	const float* resetIn = ( v[kGResetIn] > 0 )
			? busFrames + ( v[kGResetIn] - 1 ) * numFrames : NULL;
	float* clkOut = ( v[kGClockOut] > 0 )
			? busFrames + ( v[kGClockOut] - 1 ) * numFrames : NULL;
	bool clkRep = v[kGClockOutMode];

	float* pitchOut[kNumTracks];
	float* gateOut[kNumTracks];
	bool pitchRep[kNumTracks], gateRep[kNumTracks];
	bool mutedNow[kNumTracks];
	bool anySolo = false;
	for ( int t = 0; t < kNumTracks; ++t )
		if ( pThis->solo[t] ) { anySolo = true; break; }
	for ( int t = 0; t < kNumTracks; ++t )
	{
		int gb = v[ RP( t, 0 ) ], pb = v[ RP( t, 2 ) ];
		gateOut[t] = gb > 0 ? busFrames + ( gb - 1 ) * numFrames : NULL;
		gateRep[t] = v[ RP( t, 1 ) ];
		pitchOut[t] = pb > 0 ? busFrames + ( pb - 1 ) * numFrames : NULL;
		pitchRep[t] = v[ RP( t, 3 ) ];
		mutedNow[t] = v[ TP( t, kTMute ) ] || ( anySolo && !pThis->solo[t] );
	}

	// Freeze: the shoal holds its breath - nothing advances, every sounding
	// track's gate is held high, the current notes hang as a chord
	if ( v[kGFreeze] )
	{
		dtc->clockPulse = 0;			// the clock holds its breath too
		if ( clkOut && clkRep )
			for ( int i = 0; i < numFrames; ++i )
				clkOut[i] = 0.0f;
		for ( int i = 0; i < numFrames; ++i )
			for ( int t = 0; t < kNumTracks; ++t )
			{
				TrackState& tr = dtc->tracks[t];
				float gate = ( tr.pos >= 0 && !mutedNow[t] ) ? 5.0f : 0.0f;
				if ( pitchOut[t] )
				{
					if ( pitchRep[t] ) pitchOut[t][i] = tr.pitchVolts;
					else pitchOut[t][i] += tr.pitchVolts;
				}
				if ( gateOut[t] )
				{
					if ( gateRep[t] ) gateOut[t][i] = gate;
					else gateOut[t][i] += gate;
				}
			}
		return;
	}

	bool internal = v[kGClockSource];
	bool run = v[kGRun];

	for ( int i = 0; i < numFrames; ++i )
	{
		if ( resetIn )
		{
			float r = resetIn[i];
			if ( !dtc->resetHigh && r > 1.0f )
			{
				dtc->resetHigh = true;
				dtc->resetPrime = true;
			}
			else if ( dtc->resetHigh && r < 0.1f )
				dtc->resetHigh = false;
		}

		// master clock tick
		bool tick = false;
		uint32_t period;
		if ( !internal && clockIn )
		{
			float c = clockIn[i];
			if ( !dtc->clockHigh && c > 1.0f )
			{
				dtc->clockHigh = true;
				if ( dtc->samplesSinceClock > 0 )
					dtc->extPeriod = dtc->samplesSinceClock;
				dtc->samplesSinceClock = 0;
				tick = run;
			}
			else if ( dtc->clockHigh && c < 0.1f )
				dtc->clockHigh = false;
			if ( dtc->samplesSinceClock < 0x7FFFFFFF )
				dtc->samplesSinceClock += 1;
			period = dtc->extPeriod ? dtc->extPeriod : NT_globals.sampleRate / 4;
		}
		else
		{
			if ( run )
			{
				if ( dtc->intCountdown <= 1 )
				{
					tick = true;
					dtc->intCountdown = dtc->intPeriod;
				}
				else
					dtc->intCountdown -= 1;
			}
			period = dtc->intPeriod;
		}

		if ( tick )
		{
			if ( dtc->resetPrime )
				dtc->tickCount = 0;		// the reset tick is a grid origin for every divider

			for ( int t = 0; t < kNumTracks; ++t )
			{
				TrackState& tr = dtc->tracks[t];
				int rate = v[ TP( t, kTRate ) ];
				uint32_t div = rateDiv[rate], mult = rateMult[rate];

				if ( dtc->resetPrime )
				{
					tr.pos = -1;
					tr.extraPending = 0;
					tr.slopCountdown = 0;
					tr.tieHeld = false;
				}

				if ( div > 1 )
				{
					tr.extraPending = 0;
					// phase-locked to the global grid: rate changes never drift
					if ( dtc->tickCount % div == 0 )
					{
						if ( dtc->resetPrime )
							advanceTrack( pThis, t, period * div, anySolo );	// reset lands tight
						else
							scheduleAdvance( pThis, t, period * div, anySolo );
					}
				}
				else
				{
					uint32_t sub = period / mult;
					if ( dtc->resetPrime )
						advanceTrack( pThis, t, sub, anySolo );
					else
						scheduleAdvance( pThis, t, sub, anySolo );
					tr.extraPending = mult - 1;
					tr.subPeriod = sub;
					tr.subCountdown = sub;
				}
			}
			dtc->resetPrime = false;
			dtc->tickCount += 1;
		}

		// Clock out: 5V, high for half the master (x1) period, so other
		// algorithms and external gear can ride Shoal's grid
		if ( tick )
			dtc->clockPulse = period / 2;
		if ( clkOut )
		{
			float cv = dtc->clockPulse ? 5.0f : 0.0f;
			if ( clkRep ) clkOut[i] = cv;
			else clkOut[i] += cv;
		}
		if ( dtc->clockPulse )
			dtc->clockPulse -= 1;

		for ( int t = 0; t < kNumTracks; ++t )
		{
			TrackState& tr = dtc->tracks[t];
			if ( tr.extraPending )
			{
				if ( tr.subCountdown <= 1 )
				{
					tr.extraPending -= 1;
					tr.subCountdown = tr.subPeriod;
					scheduleAdvance( pThis, t, tr.subPeriod, anySolo );
				}
				else
					tr.subCountdown -= 1;
			}

			if ( tr.slopCountdown )
			{
				if ( tr.slopCountdown <= 1 )
				{
					tr.slopCountdown = 0;
					advanceTrack( pThis, t, tr.slopPeriod, anySolo );
				}
				else
					tr.slopCountdown -= 1;
			}

			float gate = 0.0f;
			if ( tr.gateRemaining > 0 && !mutedNow[t] )
			{
				gate = 5.0f;
				tr.gateRemaining -= 1;
			}
			else if ( tr.gateRemaining > 0 )
				tr.gateRemaining -= 1;

			if ( pitchOut[t] )
			{
				if ( pitchRep[t] )
					pitchOut[t][i] = tr.pitchVolts;
				else
					pitchOut[t][i] += tr.pitchVolts;
			}
			if ( gateOut[t] )
			{
				if ( gateRep[t] )
					gateOut[t][i] = gate;
				else
					gateOut[t][i] += gate;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

static void fireEncHolds( _melodySeqAlgorithm* pThis );
static uint32_t nextRand( _melodySeqAlgorithm* pThis );

// An 11x5 pixel fish, two frames of tail flick. Bit 0 = leftmost column.
static const uint16_t fishFrames[2][5] = {
	{
		(1<<2)|(1<<3)|(1<<4)|(1<<9),
		(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<8)|(1<<9),
		0x1FF,
		(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<8)|(1<<9),
		(1<<2)|(1<<3)|(1<<4)|(1<<9),
	},
	{
		(1<<2)|(1<<3)|(1<<4),
		(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<9),
		0x1FF|(1<<10),
		(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<9),
		(1<<2)|(1<<3)|(1<<4),
	},
};

static void drawFish( int x0, int y0, int frame, int colour )
{
	const uint16_t* rows = fishFrames[frame];
	for ( int r = 0; r < 5; ++r )
		for ( int c = 0; c < 11; ++c )
			if ( rows[r] & ( 1 << c ) )
				NT_drawShapeI( kNT_point, x0 + c, y0 + r, x0 + c, y0 + r, colour );
	NT_drawShapeI( kNT_point, x0 + 2, y0 + 1, x0 + 2, y0 + 1, 1 );	// eye
}

// a small hand-rolled sine for the screensaver's swaying and wobbling
static const int8_t swayTab[16] = { 0, 1, 2, 2, 2, 1, 1, 0, 0, -1, -2, -2, -2, -1, -1, 0 };

// The screensaver IS the shoal: one fish per track swimming the open sea,
// darting forward whenever its track sounds a note. Muted tracks drift dim
// and slow; a breathing track fades; Freeze stills the whole scene while
// the bubbles keep rising. Any touch wakes the normal display.
static void drawSaver( _melodySeqAlgorithm* pThis )
{
	_melodySeq_DTC* dtc = pThis->dtc;
	const int16_t* v = pThis->v;
	uint32_t tick = pThis->drawTick;
	bool frozen = v[kGFreeze];

	bool anySolo = false;
	for ( int t = 0; t < kNumTracks; ++t )
		if ( pThis->solo[t] ) { anySolo = true; break; }

	// seabed: an uneven line of sediment
	for ( int x = 0; x < 256; x += 5 )
	{
		int h = (int)( hash3( (uint32_t)x, 0, 0x0BEDu ) % 3 );
		NT_drawShapeI( kNT_point, x, 63 - h, x, 63 - h, 3 );
	}

	// seaweed, swaying from the bed
	static const uint8_t weedX[3] = { 34, 138, 214 };
	for ( int w = 0; w < 3; ++w )
		for ( int seg = 0; seg < 5; ++seg )
		{
			int dx = ( swayTab[ ( ( tick >> 3 ) + w * 5 + seg * 2 ) & 15 ] * ( seg + 1 ) ) / 4;
			int y = 61 - seg * 3;
			NT_drawShapeI( kNT_point, weedX[w] + dx, y, weedX[w] + dx, y - 1, 4 );
		}

	// bubbles rise even while the shoal is frozen - the sea stays alive
	for ( int b = 0; b < 4; ++b )
	{
		if ( ( tick & 1 ) == 0 )
			pThis->bubY[b] -= 1;
		if ( pThis->bubY[b] < 2 )
		{
			pThis->bubY[b] = (int16_t)( 58 + ( nextRand( pThis ) % 5 ) );
			pThis->bubX[b] = (int16_t)( 8 + ( nextRand( pThis ) % 240 ) );
		}
		int bx = pThis->bubX[b] + ( swayTab[ ( ( tick >> 2 ) + b * 4 ) & 15 ] >> 1 );
		NT_drawShapeI( kNT_point, bx, pThis->bubY[b], bx, pThis->bubY[b], 6 );
	}

	// the shoal: track order top to bottom, each fish in its own water
	for ( int t = 0; t < kNumTracks; ++t )
	{
		TrackState& tr = dtc->tracks[t];
		bool muted = v[ TP( t, kTMute ) ] || ( anySolo && !pThis->solo[t] );
		bool sounding = tr.gateRemaining > 0 && !muted;

		float speed;
		if ( frozen )
			speed = 0.0f;
		else
		{
			speed = 0.28f + 0.045f * (float)( ( t * 5 ) % 5 );	// no two fish alike
			if ( muted )
				speed *= 0.4f;
			else if ( sounding )
				speed += 1.1f;									// the dart
		}
		pThis->fishX[t] -= speed;
		if ( pThis->fishX[t] < -12.0f )
			pThis->fishX[t] = 258.0f + (float)( nextRand( pThis ) % 48 );

		int y = 2 + t * 7 + swayTab[ ( ( tick >> 2 ) + t * 3 ) & 15 ];
		int colour = muted ? 3 : ( tr.resting ? 4 : ( sounding ? 15 : 9 ) );
		int frame = frozen ? 0 : (int)( ( ( tick >> 3 ) + t ) & 1 );
		drawFish( (int)pThis->fishX[t], y, frame, colour );
	}
}

bool draw( _NT_algorithm* self )
{
	_melodySeqAlgorithm* pThis = (_melodySeqAlgorithm*)self;
	_melodySeq_DTC* dtc = pThis->dtc;
	const int16_t* v = pThis->v;
	char buff[16];

	pThis->drawTick += 1;
	fireEncHolds( pThis );		// holds must fire even while controls are motionless

	// screensaver: engage after the idle time, until any touch
	if ( v[kGSaver] > 0 )
	{
		uint32_t limit = NT_globals.sampleRate * ( v[kGSaver] == 1 ? 60u : 300u );
		if ( dtc->uiClock - pThis->lastTouch >= limit )
		{
			pThis->saverActive = true;
			drawSaver( pThis );
			return true;
		}
	}
	pThis->saverActive = false;

	bool anySolo = false;
	for ( int t = 0; t < kNumTracks; ++t )
		if ( pThis->solo[t] ) { anySolo = true; break; }

	for ( int t = 0; t < kNumTracks; ++t )
	{
		TrackState& tr = dtc->tracks[t];
		int y = 1 + t * 7;
		bool selT = ( t == pThis->sel );
		bool muted = v[ TP( t, kTMute ) ] || ( anySolo && !pThis->solo[t] );
		int length = v[ TP( t, kTLength ) ];
		int base = muted ? 4 : ( tr.resting ? 5 : ( selT ? 13 : 9 ) );	// breathing rows dim

		if ( selT )
			NT_drawShapeI( kNT_rectangle, 0, y, 1, y + 5, 15 );
		NT_intToString( buff, t + 1 );
		NT_drawText( 4, y + 5, buff, selT ? 15 : ( muted ? 4 : 8 ), kNT_textLeft, kNT_textTiny );

		// adaptive density: longer loops draw with narrower cells, so the
		// whole pattern is always visible - no scrolling
		int cw, bw, cells;
		if ( length <= 16 )      { cw = 9; bw = 7; cells = 16; }
		else if ( length <= 32 ) { cw = 4; bw = 3; cells = length; }
		else                     { cw = 2; bw = 2; cells = length; }

		for ( int s = 0; s < cells; ++s )
		{
			int x = 10 + s * cw;
			if ( s >= length )
			{
				NT_drawShapeI( kNT_point, x + 3, y + 3, x + 3, y + 3, 2 );
				continue;
			}
			bool isPos = ( s == tr.pos );
			if ( isPos )
				NT_drawShapeI( kNT_rectangle, x, y, x + bw - 1, y + 5, 5 );
			StepEval ev;
			evalStep( pThis, t, s, ev );
			if ( !ev.fires )
				NT_drawShapeI( kNT_rectangle, x, y + 4, x + bw - 1, y + 4, muted ? 2 : 3 );
			else
				NT_drawShapeI( kNT_rectangle, x, y + 5 - ev.barH, x + bw - 1, y + 5, isPos ? 15 : base );
		}

		NT_drawText( 156, y + 5, enumStringsRate[ v[ TP( t, kTRate ) ] ], selT ? 12 : 6, kNT_textLeft, kNT_textTiny );
		int src = v[ TP( t, kTSource ) ];
		if ( src > 0 && src - 1 != t )
		{
			buff[0] = '<';
			int n = 1 + NT_intToString( buff + 1, src );
			buff[n] = 0;
			NT_drawText( 172, y + 5, buff, 7, kNT_textLeft, kNT_textTiny );
		}
		if ( pThis->solo[t] )
			NT_drawText( 186, y + 5, "S", 15, kNT_textLeft, kNT_textTiny );
		else if ( muted )
			NT_drawText( 186, y + 5, "M", 12, kNT_textLeft, kNT_textTiny );
	}

	NT_drawShapeI( kNT_line, 190, 0, 190, 56, 3 );

	// selected-track detail panel
	int t = pThis->sel;
	const int P = 194;
	buff[0] = 'T';
	int n = 1 + NT_intToString( buff + 1, t + 1 );
	buff[n] = 0;
	NT_drawText( P, 10, buff, 15, kNT_textLeft, kNT_textNormal );

	// pot params bright; the dim column holds live performance state -
	// direction, seed, evolve (GATE/TIE/SLOP live on the parameter page)
	static char const * const dirShort[9] = {
		"FWD", "REV", "PND", "RND", "DRK", "PNG", "TID", "SHF", "POL",
	};
	NT_drawText( P, 19, "CH", 14, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTChance ) ] );
	NT_drawText( P + 10, 19, buff, 14, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 30, 19, "DIR", 8, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 44, 19, dirShort[ v[ TP( t, kTDirection ) ] ], 8, kNT_textLeft, kNT_textTiny );

	NT_drawText( P, 26, "NT", 14, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTNote ) ] );
	NT_drawText( P + 10, 26, buff, 14, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 30, 26, "SD", 8, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTSeed ) ] );
	NT_drawText( P + 40, 26, buff, 8, kNT_textLeft, kNT_textTiny );

	NT_drawText( P, 33, "OC", 14, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTOct ) ] );
	NT_drawText( P + 10, 33, buff, 14, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 30, 33, "EV", 8, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTEvolve ) ] );
	NT_drawText( P + 40, 33, buff, 8, kNT_textLeft, kNT_textTiny );

	NT_drawText( P, 40, "RATE", 9, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 20, 40, enumStringsRate[ v[ TP( t, kTRate ) ] ], 9, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 36, 40, "LEN", 9, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTLength ) ] );
	NT_drawText( P + 52, 40, buff, 9, kNT_textLeft, kNT_textTiny );

	NT_drawText( P, 47, "OCT", 9, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTOctave ) ] );
	NT_drawText( P + 14, 47, buff, 9, kNT_textLeft, kNT_textTiny );
	NT_drawText( P + 26, 47, "TR", 9, kNT_textLeft, kNT_textTiny );
	NT_intToString( buff, v[ TP( t, kTTrans ) ] );
	NT_drawText( P + 36, 47, buff, 9, kNT_textLeft, kNT_textTiny );
	int selSrc = v[ TP( t, kTSource ) ];
	if ( selSrc > 0 && selSrc - 1 != t )
	{
		buff[0] = '<';
		n = 1 + NT_intToString( buff + 1, selSrc );
		buff[n] = 0;
		NT_drawText( P + 48, 47, buff, 9, kNT_textLeft, kNT_textTiny );
	}

	if ( dtc->tracks[t].pendingSeed >= 0 )
	{
		if ( ( pThis->drawTick >> 2 ) & 1 )
			NT_drawText( P, 55, "RESEED ARM", 15, kNT_textLeft, kNT_textTiny );
	}
	else
	{
		// the global key, quietly, while nothing more urgent needs the corner
		static char const * const noteNames[12] = {
			"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
		};
		static char const * const scaleShort[13] = {
			"CHROM", "MAJ", "NMIN", "HMIN", "DOR", "PHRYG", "LYD",
			"MIXO", "PENT+", "PENT-", "BLUES", "HIRA", "INSEN",
		};
		const char* rn = noteNames[ v[kGRoot] % 12 ];
		const char* sn = scaleShort[ v[kGScale] ];
		int m = 0;
		while ( *rn ) buff[m++] = *rn++;
		buff[m++] = ' ';
		while ( *sn ) buff[m++] = *sn++;
		buff[m] = 0;
		NT_drawText( P, 55, buff, 7, kNT_textLeft, kNT_textTiny );
	}
	if ( v[kGFreeze] )
		NT_drawText( P + 44, 55, "FRZ", 15, kNT_textLeft, kNT_textTiny );

	// the shoal's mascot and wordmark, top right - tail flicks faster while
	// a reseed is armed, and stops entirely while frozen
	NT_drawText( 220, 7, "SHOAL", 7, kNT_textLeft, kNT_textTiny );
	int flickShift = ( dtc->tracks[t].pendingSeed >= 0 ) ? 2 : 4;
	int fishFrame = v[kGFreeze] ? 0 : (int)( ( pThis->drawTick >> flickShift ) & 1 );
	drawFish( 244, 2, fishFrame, 10 );

	// bottom bar: live pot assignments
	NT_drawShapeI( kNT_line, 0, 57, 255, 57, 4 );
	// pot labels: bright while away from home, dim when the pot sits at
	// its default (push a pot to toggle home <-> dialled)
	NT_drawText( 6, 63, "P1", 5, kNT_textLeft, kNT_textTiny );
	NT_drawText( 18, 63, "CHANCE", v[ TP( t, kTChance ) ] != 100 ? 15 : 5, kNT_textLeft, kNT_textTiny );
	NT_drawText( 96, 63, "P2", 5, kNT_textLeft, kNT_textTiny );
	NT_drawText( 108, 63, "NOTE", v[ TP( t, kTNote ) ] != 0 ? 15 : 5, kNT_textLeft, kNT_textTiny );
	NT_drawText( 176, 63, "P3", 5, kNT_textLeft, kNT_textTiny );
	NT_drawText( 188, 63, "OCT", v[ TP( t, kTOct ) ] != 0 ? 15 : 5, kNT_textLeft, kNT_textTiny );

	return true;
}

// ---------------------------------------------------------------------------
// Custom UI
// ---------------------------------------------------------------------------

// The three pots own CHANCE / NOTE / OCT. Pushing a pot toggles its
// parameter between "home" (the default) and the last dialled value.
static const uint8_t potParam[3] = { kTChance, kTNote, kTOct };
static const int16_t potHome[3]  = { 100, 0, 0 };

static void potRange( int pot, int& mn, int& mx )
{
	if ( pot == 0 ) { mn = 0; mx = 100; }					// chance
	else { mn = -100; mx = 100; }							// note / oct (bipolar, centre = 0)
}

uint32_t hasCustomUi( _NT_algorithm* self )
{
	return kNT_potL | kNT_potC | kNT_potR
		 | kNT_potButtonL | kNT_potButtonC | kNT_potButtonR
		 | kNT_encoderL | kNT_encoderR
		 | kNT_encoderButtonL | kNT_encoderButtonR;
}

static void setTrackParam( _melodySeqAlgorithm* pThis, int t, int off, int value )
{
	int algIdx = NT_algorithmIndex( pThis );
	if ( algIdx < 0 )
		return;
	pThis->selfWrite = true;
	NT_setParameterFromUi( (uint32_t)algIdx, TP( t, off ) + NT_parameterOffset(), (int16_t)value );
	pThis->selfWrite = false;
}

static uint32_t nextRand( _melodySeqAlgorithm* pThis )
{
	pThis->uiRng = pThis->uiRng * 1664525u + 1013904223u;
	return pThis->uiRng >> 16;
}

// Encoder holds fire on real elapsed time (the audio clock), while still
// held. Checked from draw() as well as customUi(), because customUi() is
// not called while the controls are motionless.
static void fireEncHolds( _melodySeqAlgorithm* pThis )
{
	uint32_t hold = NT_globals.sampleRate / 2;		// ~0.5 s
	uint32_t now = pThis->dtc->uiClock;

	if ( pThis->encLHeldPrev && !pThis->encLTurned && !pThis->encLFired
			&& now - pThis->encLDown >= hold )
	{
		pThis->encLFired = true;
		pThis->solo[ pThis->sel ] = !pThis->solo[ pThis->sel ];
	}
	if ( pThis->encRHeldPrev && !pThis->encRTurned && !pThis->encRFired
			&& now - pThis->encRDown >= hold )
	{
		pThis->encRFired = true;
		int algIdx = NT_algorithmIndex( pThis );
		if ( algIdx >= 0 )
			NT_setParameterFromUi( (uint32_t)algIdx, kGReseedAll + NT_parameterOffset(),
								   (int16_t)( nextRand( pThis ) % 1000 ) );
	}
}

void customUi( _NT_algorithm* self, const _NT_uiData& data )
{
	_melodySeqAlgorithm* pThis = (_melodySeqAlgorithm*)self;
	const int16_t* v = pThis->v;
	int t = pThis->sel;

	// A "touch" must be meaningful: real hardware pots jitter by a count
	// or two at rest, and the OS reports that as control activity - which
	// silently kept resetting the idle timer, so the screensaver never
	// engaged. Buttons and encoder clicks always count; a pot counts only
	// once it has moved visibly. (While asleep, potPrev is frozen as the
	// anchor, so even a slow deliberate turn accumulates into a wake.)
	static const uint16_t wakeButtons =
		  kNT_encoderButtonL | kNT_encoderButtonR
		| kNT_potButtonL | kNT_potButtonC | kNT_potButtonR;
	bool touched = data.encoders[0] != 0 || data.encoders[1] != 0
		|| ( data.controls & wakeButtons ) || ( data.lastButtons & wakeButtons );
	for ( int i = 0; i < 3; ++i )
	{
		float d = data.pots[i] - pThis->potPrev[i];
		if ( d > 0.01f || d < -0.01f )
			touched = true;
	}

	if ( pThis->saverActive )
	{
		if ( !touched )
			return;						// jitter, not a touch: stay asleep
		// wake, and swallow the waking touch - you can't reseed a track
		// by accident just waking the screen
		pThis->saverActive = false;
		pThis->lastTouch = pThis->dtc->uiClock;
		for ( int i = 0; i < 3; ++i )
		{
			pThis->potCaptured[i] = false;
			pThis->potPrev[i] = data.pots[i];
		}
		return;
	}
	if ( touched )
		pThis->lastTouch = pThis->dtc->uiClock;

	// ---- left encoder: track select / length / mute / solo ----
	bool lHeld = data.controls & kNT_encoderButtonL;
	if ( lHeld && !pThis->encLHeldPrev )
	{
		pThis->encLDown = pThis->dtc->uiClock;
		pThis->encLTurned = false;
		pThis->encLFired = false;
	}
	if ( data.encoders[0] )
	{
		if ( lHeld )
		{
			int len = v[ TP( t, kTLength ) ] + data.encoders[0];
			if ( len < 1 ) len = 1;
			if ( len > kMaxSteps ) len = kMaxSteps;
			setTrackParam( pThis, t, kTLength, len );
			pThis->encLTurned = true;
		}
		else
		{
			int s = pThis->sel + data.encoders[0];
			if ( s < 0 ) s = 0;
			if ( s >= kNumTracks ) s = kNumTracks - 1;
			if ( s != pThis->sel )
			{
				pThis->sel = s;
				for ( int i = 0; i < 3; ++i )
					pThis->potCaptured[i] = false;
			}
		}
	}
	if ( !lHeld && pThis->encLHeldPrev && !pThis->encLTurned && !pThis->encLFired )
		setTrackParam( pThis, t, kTMute, !v[ TP( t, kTMute ) ] );		// tap = mute
	pThis->encLHeldPrev = lHeld;

	// ---- right encoder: rate / gate / reseed ----
	bool rHeld = data.controls & kNT_encoderButtonR;
	if ( rHeld && !pThis->encRHeldPrev )
	{
		pThis->encRDown = pThis->dtc->uiClock;
		pThis->encRTurned = false;
		pThis->encRFired = false;
	}
	if ( data.encoders[1] )
	{
		if ( rHeld )
		{
			// turning while pressed cancels the reseed gesture (GATE lives
			// on Pot C's push now)
			pThis->encRTurned = true;
		}
		else
		{
			int r = v[ TP( t, kTRate ) ] + data.encoders[1];
			if ( r < 0 ) r = 0;
			if ( r > kNumRates - 1 ) r = kNumRates - 1;
			setTrackParam( pThis, t, kTRate, r );
		}
	}
	if ( !rHeld && pThis->encRHeldPrev && !pThis->encRTurned && !pThis->encRFired )
		setTrackParam( pThis, t, kTSeed, nextRand( pThis ) % 1000 );		// tap = reseed track
	pThis->encRHeldPrev = rHeld;

	fireEncHolds( pThis );

	// ---- pot pushes: toggle home <-> dialled value ----
	// Pressing a pot physically nudges its ADC by a few percent, and after
	// a punch-back the pot sits exactly at the restored value - so takeover
	// captures instantly and the click's wobble would write straight
	// through. Each press or release therefore starts a short quiet period
	// during which the pot's value is ignored.
	uint32_t quietUntil = pThis->dtc->uiClock + NT_globals.sampleRate / 5;	// ~200 ms
	static const uint16_t potBtn[3] = { kNT_potButtonL, kNT_potButtonC, kNT_potButtonR };
	static const uint16_t potBit[3] = { kNT_potL, kNT_potC, kNT_potR };
	for ( int i = 0; i < 3; ++i )
	{
		if ( ( data.controls & potBtn[i] ) && !( data.lastButtons & potBtn[i] ) )
		{
			int16_t cur = v[ TP( t, potParam[i] ) ];
			if ( cur != potHome[i] )
			{
				pThis->potStash[t][i] = cur;			// punch home, remember where we were
				pThis->potStashValid[t][i] = true;
				setTrackParam( pThis, t, potParam[i], potHome[i] );
			}
			else if ( pThis->potStashValid[t][i] )
				setTrackParam( pThis, t, potParam[i], pThis->potStash[t][i] );	// punch back
			pThis->potCaptured[i] = false;
			pThis->potQuiet[i] = quietUntil;
		}
		else if ( !( data.controls & potBtn[i] ) && ( data.lastButtons & potBtn[i] ) )
			pThis->potQuiet[i] = quietUntil;			// release wobbles too
	}

	// ---- pots, with soft takeover ----
	for ( int i = 0; i < 3; ++i )
	{
		float pot = data.pots[i];
		bool quiet = ( data.controls & potBtn[i] )
			|| (int32_t)( pThis->potQuiet[i] - pThis->dtc->uiClock ) > 0;
		if ( quiet )
		{
			pThis->potPrev[i] = pot;					// track it, act on none of it
			continue;
		}
		if ( data.controls & potBit[i] )
		{
			int off = potParam[i];
			int mn, mx;
			potRange( i, mn, mx );
			float target = ( v[ TP( t, off ) ] - mn ) / (float)( mx - mn );
			if ( !pThis->potCaptured[i] )
			{
				float prev = pThis->potPrev[i];
				bool crossed = ( prev <= target && pot >= target )
							|| ( prev >= target && pot <= target );
				float dist = pot - target;
				if ( crossed || ( dist > -0.03f && dist < 0.03f ) )
					pThis->potCaptured[i] = true;
			}
			if ( pThis->potCaptured[i] )
			{
				int value = mn + (int)( pot * ( mx - mn ) + 0.5f );
				if ( value < mn ) value = mn;
				if ( value > mx ) value = mx;
				setTrackParam( pThis, t, off, value );
			}
		}
		pThis->potPrev[i] = pot;
	}
}

void setupUi( _NT_algorithm* self, _NT_float3& pots )
{
	_melodySeqAlgorithm* pThis = (_melodySeqAlgorithm*)self;
	const int16_t* v = pThis->v;
	int t = pThis->sel;
	pThis->lastTouch = pThis->dtc->uiClock;		// entering the UI is a touch
	pThis->saverActive = false;
	for ( int i = 0; i < 3; ++i )
	{
		int off = potParam[i];
		int mn, mx;
		potRange( i, mn, mx );
		float norm = ( v[ TP( t, off ) ] - mn ) / (float)( mx - mn );
		pots[i] = norm;
		pThis->potPrev[i] = norm;
		pThis->potCaptured[i] = true;
	}
}

// ---------------------------------------------------------------------------
// Misc callbacks
// ---------------------------------------------------------------------------

int parameterUiPrefix( _NT_algorithm* self, int p, char* buff )
{
	int t = -1;
	if ( p >= kTrackBase && p < kRoutingBase )
		t = ( p - kTrackBase ) / kNumTrackParams;
	else if ( p >= kRoutingBase && p < kRoutingBase + kNumTracks * kNumRoutingParams )
		t = ( p - kRoutingBase ) / kNumRoutingParams;		// Clock out sits past the routing block: no track prefix
	if ( t < 0 )
		return 0;
	buff[0] = 'T';
	buff[1] = (char)( '1' + t );
	buff[2] = ' ';
	buff[3] = 0;
	return 3;
}

static const _NT_factory factory =
{
	.guid = NT_MULTICHAR( 'S', 'h', 'o', 'l' ),
	.name = "Shoal",
	// version first: the algorithm browser truncates long descriptions,
	// and the version is the one part that must always be visible
	.description = "v1.0.0 - 8-track generative melody sequencer",
	.numSpecifications = 0,
	.calculateRequirements = calculateRequirements,
	.construct = construct,
	.parameterChanged = parameterChanged,
	.step = step,
	.draw = draw,
	.tags = kNT_tagInstrument | kNT_tagUtility,
	.hasCustomUi = hasCustomUi,
	.customUi = customUi,
	.setupUi = setupUi,
	.parameterUiPrefix = parameterUiPrefix,
};

uintptr_t pluginEntry( _NT_selector selector, uint32_t data )
{
	switch ( selector )
	{
	case kNT_selector_version:
		return kNT_apiVersionCurrent;
	case kNT_selector_numFactories:
		return 1;
	case kNT_selector_factoryInfo:
		return (uintptr_t)( ( data == 0 ) ? &factory : NULL );
	}
	return 0;
}
