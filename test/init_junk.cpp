// Host harness: junk-fill the plug-in's sram+dtc blocks, construct(), run
// silent input, hash every bus. Same result for every fill = no dependence
// on uninitialised memory.
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
#include SHOAL_CPP

static uint64_t run( int fill, int dir, int clockSrc ) {
	_NT_algorithmRequirements req = {};
	calculateRequirements( req, nullptr );
	std::vector<uint8_t> sram( req.sram + 64 ), dtc( req.dtc + 64 );
	memset( sram.data(), fill, sram.size() );
	memset( dtc.data(), fill, dtc.size() );
	_NT_algorithmMemoryPtrs ptrs = {};
	ptrs.sram = sram.data(); ptrs.dtc = dtc.data();
	_NT_algorithm* alg = construct( ptrs, req, nullptr );
	std::vector<int16_t> v( kNumParameters );
	for ( int i = 0; i < kNumParameters; i++ ) v[i] = parameters[i].def;
	for ( int t = 0; t < kNumTracks; t++ ) {
		v[ TP( t, kTDirection ) ] = (int16_t)dir;
		v[ TP( t, kTChance ) ] = 100;
		v[ RP( t, 0 ) ] = (int16_t)( 13 + 2 * t );		// gate out bus
		v[ RP( t, 2 ) ] = (int16_t)( 14 + 2 * t );		// pitch out bus
	}
	v[kGClockSource] = (int16_t)clockSrc;
	v[kGClockOut] = 28;
	( (_NT_algorithm*)alg )->v = v.data();
	const int frames = 64;
	std::vector<float> bus( 28 * frames, 0.f );
	static long nz = 0; if ( fill == 0 && dir == 0 && clockSrc == 1 ) nz = 0;
	uint64_t h = 1469598103934665603ull;
	for ( int b = 0; b < 3000; b++ ) {
		std::fill( bus.begin(), bus.end(), 0.f );		// silent input
		step( alg, bus.data(), frames / 4 );
		for ( float f : bus ) { if ( f != 0.f ) nz++; uint32_t u; memcpy( &u, &f, 4 ); h = ( h ^ u ) * 1099511628211ull; }
	}
	if ( fill == 0 && dir == 0 && clockSrc == 1 ) printf( "nonzero output samples in the zero-fill baseline: %ld\n", nz );
	return h;
}
int main() {
	int bad = 0, n = 0;
	for ( int clk = 0; clk <= 1; clk++ )
		for ( int d = 0; d <= 14; d++ ) {
			uint64_t z = run( 0x00, d, clk );
			for ( int fill : { 0xA5, 0xFF, 0x5A } ) {
				n++;
				if ( run( fill, d, clk ) != z ) { printf( "MISMATCH clock=%d dir=%d fill=%02X\n", clk, d, fill ); bad++; }
			}
		}
	printf( "(nonzero output samples in last run: sanity)\n" );
	printf( "%d comparisons, %d mismatches\n", n, bad );
	return bad != 0;
}
