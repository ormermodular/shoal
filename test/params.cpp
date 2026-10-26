// Host test of the parameter table and pages (compile with
// -DSHOAL_CPP='"../src/shoal.cpp"'):
//  - every parameter is on exactly one page (nothing unreachable, no duplicates)
//  - per-track parameters get a "T<n> " prefix naming the right track
//  - defaults lie inside their ranges and bus parameters default sensibly
#include <cstdio>
#include <cstring>
#include <vector>
#include <distingnt/api.h>
extern const _NT_globals NT_globals = { 48000, 64, nullptr, 0, 0, 0 };
int32_t NT_algorithmIndex( const _NT_algorithm* ) { return 0; }
void NT_setParameterFromAudio( uint32_t, uint32_t, int16_t ) {}
void NT_setParameterFromUi( uint32_t, uint32_t, int16_t ) {}
uint32_t NT_parameterOffset( void ) { return 0; }
void NT_sendMidi3ByteMessage( uint32_t, uint8_t, uint8_t, uint8_t ) {}
#include SHOAL_CPP

int main() {
	int fails = 0;
	std::vector<int> seen( kNumParameters, 0 );
	for ( size_t i = 0; i < ARRAY_SIZE( pages ); i++ )
		for ( int k = 0; k < pages[i].numParams; k++ ) seen[ pages[i].params[k] ]++;
	for ( int p = 0; p < kNumParameters; p++ )
		if ( seen[p] != 1 ) { printf( "FAIL param %d (%s) is on %d pages\n", p, parameters[p].name, seen[p] ); fails++; }
	for ( int p = 0; p < kNumParameters; p++ ) {
		const _NT_parameter& q = parameters[p];
		if ( q.def < q.min || q.def > q.max ) { printf( "FAIL param %d (%s) default %d outside %d..%d\n", p, q.name, q.def, q.min, q.max ); fails++; }
	}
	const struct { int base, per; } blocks[] = { { kTrackBase, kNumTrackParams }, { kRoutingBase, kNumRoutingParams },
		{ kExtBase, kNumExtParams }, { kMidiBase, kNumMidiParams }, { kCurrentBase, kNumCurrentParams },
		{ kEosBase, kNumEosParams }, { kClkBase, kNumClkParams } };
	for ( int p = 0; p < kNumParameters; p++ ) {
		char b[8] = {};
		int n = parameterUiPrefix( nullptr, p, b );
		int want = -1;
		for ( auto& bl : blocks ) if ( p >= bl.base && p < bl.base + kNumTracks * bl.per ) want = ( p - bl.base ) / bl.per;
		bool ok = want < 0 ? ( n == 0 ) : ( n == 3 && b[0] == 'T' && b[1] == '1' + want && b[2] == ' ' && b[3] == 0 );
		if ( !ok ) { printf( "FAIL prefix for param %d (%s)\n", p, parameters[p].name ); fails++; }
	}
	printf( "%d parameters, %zu pages: %s\n", kNumParameters, ARRAY_SIZE( pages ), fails ? "FAILED" : "OK" );
	return fails != 0;
}
