// Host tests for Shoal's clocking (compile with -DSHOAL_CPP='"../src/shoal.cpp"').
//  1. regression hashes: master clock only (internal + external pulses),
//     every direction - must match the pre-v1.3 code bit for bit
//     (build with -DNO_OWN against an older shoal.cpp to compare)
//  2. v1.3 per-track clocks: gate counts follow each track's own pulses
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <distingnt/api.h>
extern const _NT_globals NT_globals = { 48000, 64, nullptr, 0, 0, 0 };
int32_t NT_algorithmIndex( const _NT_algorithm* ) { return 0; }
void NT_setParameterFromAudio( uint32_t, uint32_t, int16_t ) {}
void NT_setParameterFromUi( uint32_t, uint32_t, int16_t ) {}
uint32_t NT_parameterOffset( void ) { return 0; }
void NT_sendMidi3ByteMessage( uint32_t, uint8_t, uint8_t, uint8_t ) {}
#include SHOAL_CPP

enum { kFrames = 64, kBuses = 28 };

struct Rig {
	std::vector<uint8_t> sram, dtc;
	std::vector<int16_t> v;
	_NT_algorithm* alg;
	std::vector<float> bus;
	long sample = 0;
	Rig() {
		_NT_algorithmRequirements req = {};
		calculateRequirements( req, nullptr );
		sram.assign( req.sram, 0xA5 ); dtc.assign( req.dtc, 0xA5 );
		_NT_algorithmMemoryPtrs ptrs = {};
		ptrs.sram = sram.data(); ptrs.dtc = dtc.data();
		alg = construct( ptrs, req, nullptr );
		v.resize( kNumParameters );
		for ( int i = 0; i < kNumParameters; i++ ) v[i] = parameters[i].def;
		for ( int t = 0; t < kNumTracks; t++ ) {
			v[ TP( t, kTRate ) ] = kRateX1;
			v[ TP( t, kTChance ) ] = 100;
			v[ RP( t, 0 ) ] = (int16_t)( 13 + 2 * t );		// gate out bus
		}
		alg->v = v.data();
		parameterChanged( alg, kGBPM );		// 120 BPM -> intPeriod 24000 samples
		bus.assign( kBuses * kFrames, 0.f );
	}
	// pulses[b] = period in samples for input bus b (1-based), 0 = silent
	void run( long blocks, const long* period, const long* resetAt, int nReset,
			  std::vector<int>* gates, std::vector<int>* prev ) {
		for ( long b = 0; b < blocks; b++ ) {
			std::fill( bus.begin(), bus.end(), 0.f );
			for ( int f = 0; f < kFrames; f++ ) {
				long n = sample + f;
				for ( int ib = 1; ib <= 12; ib++ )
					if ( period[ib] && ( n % period[ib] ) < 480 ) bus[ ( ib - 1 ) * kFrames + f ] = 5.f;
				for ( int r = 0; r < nReset; r++ )
					if ( n >= resetAt[r] && n < resetAt[r] + 480 ) bus[ ( 2 - 1 ) * kFrames + f ] = 5.f;	// reset on bus 2
			}
			step( alg, bus.data(), kFrames / 4 );
			if ( gates )
				for ( int t = 0; t < kNumTracks; t++ )
					for ( int f = 0; f < kFrames; f++ ) {
						int g = bus[ ( 13 + 2 * t - 1 ) * kFrames + f ] > 0.5f;
						if ( g && !( *prev )[t] ) ( *gates )[t]++;
						( *prev )[t] = g;
					}
			sample += kFrames;
		}
	}
};

static uint64_t regressHash( int clockSrc, int dir ) {
	Rig r;
	for ( int t = 0; t < kNumTracks; t++ ) {
		r.v[ TP( t, kTDirection ) ] = (int16_t)dir;
		r.v[ TP( t, kTRate ) ] = (int16_t)( ( t * 3 ) % kNumRates );	// every rate family
		r.v[ RP( t, 2 ) ] = (int16_t)( 14 + 2 * t );
	}
	r.v[kGClockSource] = (int16_t)clockSrc;
	r.v[kGClockIn] = 1;
	r.v[kGResetIn] = 2;
	long period[13] = {}; period[1] = 6000;
	long resets[3] = { 100000, 190000, 260000 };
	uint64_t h = 1469598103934665603ull;
	for ( int blk = 0; blk < 5000; blk++ ) {
		std::vector<int> g( kNumTracks );
		// run one block at a time so every output sample can be hashed
		for ( int i = 0; i < kBuses * kFrames; i++ ) {}
		r.run( 1, period, resets, 3, nullptr, nullptr );
		for ( float f : r.bus ) { uint32_t u; memcpy( &u, &f, 4 ); h = ( h ^ u ) * 1099511628211ull; }
	}
	return h;
}

int main() {
	int fails = 0;
	for ( int clk = 0; clk <= 1; clk++ )
		for ( int d = 0; d <= 14; d++ )
			printf( "REGRESS clk=%d dir=%d %016llx\n", clk, d, (unsigned long long)regressHash( clk, d ) );
#ifndef NO_OWN
	// --- v1.3 per-track clocks: master internal 120 BPM (24000 samples/step)
	{
		Rig r;
		r.v[ KP( 0 ) ] = 3;		// track 1: own clock on bus 3, period 16000
		r.v[ KP( 1 ) ] = 4;		// track 2: own clock on bus 4, period 9600
		// track 3 follows the master (internal)
		long period[13] = {}; period[3] = 16000; period[4] = 9600;
		std::vector<int> gates( kNumTracks ), prev( kNumTracks );
		r.run( 480000 / kFrames, period, nullptr, 0, &gates, &prev );
		printf( "own clocks, 10 s: t1=%d (want 30) t2=%d (want 50) t3=%d (want 20)\n", gates[0], gates[1], gates[2] );
		if ( abs( gates[0] - 30 ) > 1 || abs( gates[1] - 50 ) > 1 || abs( gates[2] - 20 ) > 1 ) { puts( "FAIL counts" ); fails++; }
	}
	// own clock + rate /2 -> half the steps; x2 -> double
	{
		Rig r;
		r.v[ KP( 0 ) ] = 3; r.v[ KP( 1 ) ] = 3; r.v[ KP( 2 ) ] = 3;
		r.v[ TP( 1, kTRate ) ] = 11;		// /2
		r.v[ TP( 2, kTRate ) ] = 17;		// x2
		long period[13] = {}; period[3] = 12000;
		std::vector<int> gates( kNumTracks ), prev( kNumTracks );
		r.run( 480000 / kFrames, period, nullptr, 0, &gates, &prev );
		printf( "own clock rates, 10 s @ 4 Hz: x1=%d (want 40) /2=%d (want 20) x2=%d (want 80)\n", gates[0], gates[1], gates[2] );
		if ( abs( gates[0] - 40 ) > 1 || abs( gates[1] - 20 ) > 1 || abs( gates[2] - 80 ) > 2 ) { puts( "FAIL rates" ); fails++; }
	}
	// an own-clock track with no pulses never advances, even with the master running
	{
		Rig r;
		r.v[ KP( 0 ) ] = 5;		// bus 5 stays silent
		long period[13] = {};
		std::vector<int> gates( kNumTracks ), prev( kNumTracks );
		r.run( 240000 / kFrames, period, nullptr, 0, &gates, &prev );
		printf( "silent own clock, 5 s: t1=%d (want 0) t2=%d (want 10)\n", gates[0], gates[1] );
		if ( gates[0] != 0 || abs( gates[1] - 10 ) > 1 ) { puts( "FAIL silent" ); fails++; }
	}
	// Run off stops own-clock tracks too
	{
		Rig r;
		r.v[ KP( 0 ) ] = 3; r.v[kGRun] = 0;
		long period[13] = {}; period[3] = 12000;
		std::vector<int> gates( kNumTracks ), prev( kNumTracks );
		r.run( 240000 / kFrames, period, nullptr, 0, &gates, &prev );
		printf( "run off: t1=%d (want 0)\n", gates[0] );
		if ( gates[0] != 0 ) { puts( "FAIL run" ); fails++; }
	}
	// Freeze: no gates from an own-clock track; unfreeze resumes in phase
	{
		Rig r;
		r.v[ KP( 0 ) ] = 3; r.v[kGFreeze] = 1;
		long period[13] = {}; period[3] = 12000;
		std::vector<int> gates( kNumTracks ), prev( kNumTracks );
		r.run( 240000 / kFrames, period, nullptr, 0, &gates, &prev );
		printf( "freeze: t1 note-on edges=%d (want 0)\n", gates[0] );
		if ( gates[0] != 0 ) { puts( "FAIL freeze" ); fails++; }
	}
	// reset: an own-clock track restarts at step 1 on its NEXT edge
	{
		Rig r;
		r.v[ KP( 0 ) ] = 3; r.v[kGResetIn] = 2;
		long period[13] = {}; period[3] = 12000;
		long resets[1] = { 40100 };		// between edges 36000 and 48000
		_melodySeq_DTC* d = (_melodySeq_DTC*)r.dtc.data();
		r.run( 625, period, resets, 1, nullptr, nullptr );		// to sample 40000
		int p0 = d->tracks[0].pos;
		r.run( 60, period, resets, 1, nullptr, nullptr );		// reset pulse has come and gone
		int p1 = d->tracks[0].pos;
		r.run( 66, period, resets, 1, nullptr, nullptr );		// crosses the edge at 48000
		int p2 = d->tracks[0].pos; uint32_t tk = d->tracks[0].ownTicks;
		printf( "reset: pos %d -> %d (held until the edge) -> %d, ownTicks=%u (want pos 3,3,0 ticks 1)\n", p0, p1, p2, tk );
		if ( p1 != p0 || p2 != 0 || tk != 1 ) { puts( "FAIL reset" ); fails++; }
	}
#endif
	printf( "%s\n", fails ? "FAILED" : "OK" );
	return fails != 0;
}
