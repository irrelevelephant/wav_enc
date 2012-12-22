#define _USE_MATH_DEFINES
#include <cmath>

#include "wav_enc.h"

// This example program generates a sine wave at 440 Hz
// The signal has a duration of 5 seconds
// The output format is a 16-bit, single channel WAVE file

int main( int argc, char* argv[] )
{
	const int sample_rate = 44100;
	const int seconds = 5;

	WavFile fout( "output.wav",
	              WavFile::WAV_16BIT,
	              WavFile::WAV_MONO,
	              sample_rate );

	for( int i = 0; i < sample_rate * seconds; ++i )
	{
		double amplitude = sin( 2 * M_PI * ( 440.0 / sample_rate ) * i ) * 0.5;
		fout.write_mono_16bit( amplitude );
	}

	fout.close();

	return 0;
}
