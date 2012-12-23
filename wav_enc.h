/*
Copyright (c) 2012 Thomas Smith

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef WAV_ENC_H_
#define WAV_ENC_H_

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>

class WavFile
{
	static const std::uint32_t header_size_ = 44;

	std::ofstream fout_;
	std::uint32_t sample_rate_;

	enum bit_depth_t : unsigned int;
	bit_depth_t bit_depth_;
	enum channels_t : unsigned int;
	channels_t channels_;

	std::uint32_t data_size_;

public:
	enum bit_depth_t : unsigned int { WAV_16BIT = 2, WAV_24BIT = 3 };
	enum channels_t : unsigned int { WAV_MONO = 1, WAV_STEREO = 2 };

	WavFile() :
		fout_(),
		sample_rate_( 0 ),
		bit_depth_( WAV_16BIT ),
		channels_( WAV_MONO ),
		data_size_( 0 )
	{
	}

	explicit WavFile( const char* file,
	                  bit_depth_t bit_depth = WAV_16BIT,
	                  channels_t channels = WAV_MONO,
	                  std::uint32_t sample_rate = 44100 ) :
		WavFile()
	{
		open( file, bit_depth, channels, sample_rate );
	}

	WavFile( const WavFile& ) = delete;
	WavFile& operator = ( const WavFile& ) = delete;

	~WavFile()
	{
		close();
	}

	void open( const char* file,
	           bit_depth_t bit_depth = WAV_16BIT,
	           channels_t channels = WAV_MONO,
	           std::uint32_t sample_rate = 44100 )
	{
		fout_.open( file, std::ios::out | std::ios::binary );
		fout_.seekp( header_size_, std::ios::beg );

		bit_depth_ = bit_depth;
		channels_ = channels;
		sample_rate_ = sample_rate;
	}

	void close()
	{
		if( fout_ )
		{
			using namespace std;

			uint32_t data_plus_header = data_size_ + header_size_;
			uint16_t channels = channels_;
			uint32_t byte_rate = sample_rate_ * channels * bit_depth_;
			uint16_t block_align = channels * bit_depth_;
			uint16_t bits_per_sample = bit_depth_ * 8;

			char wav_header[ header_size_ ] =
				{
					0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00,
					0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
					0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x64, 0x61, 0x74, 0x61,
					0x00, 0x00, 0x00, 0x00
				};

			memcpy( &wav_header[4], &data_plus_header, sizeof( uint32_t ) );
			memcpy( &wav_header[22], &channels, sizeof( uint16_t ) );
			memcpy( &wav_header[24], &sample_rate_, sizeof( uint32_t ) );
			memcpy( &wav_header[28], &byte_rate, sizeof( uint32_t ) );
			memcpy( &wav_header[32], &block_align, sizeof( uint16_t ) );
			memcpy( &wav_header[34], &bits_per_sample, sizeof( uint16_t ) );
			memcpy( &wav_header[40], &data_size_, sizeof( uint32_t ) );

			fout_.seekp( 0, ios_base::beg );
			fout_.write( wav_header, header_size_ );
			fout_.close();
		}
	}

	bit_depth_t get_bit_depth()
	{
		return bit_depth_;
	}

	channels_t get_channels()
	{
		return channels_;
	}

	std::uint32_t get_sample_rate()
	{
		return sample_rate_;
	}

	void set_bit_depth( bit_depth_t bit_depth )
	{
		bit_depth_ = bit_depth;
	}

	void set_channels( channels_t channels )
	{
		channels_ = channels;
	}

	void set_sample_rate( std::uint32_t sample_rate )
	{
		sample_rate_ = sample_rate;
	}

	void write_mono_16bit( std::int16_t sample )
	{
		using std::int16_t;
		fout_.write( reinterpret_cast<char*>( &sample ), sizeof( int16_t ) );
		data_size_ += sizeof( int16_t );
	}

	void write_stereo_16bit( std::int16_t sample_left,
	                         std::int16_t sample_right )
	{
		write_mono_16bit( sample_left );
		write_mono_16bit( sample_right );
	}

	void write_mono_16bit( double sample )
	{
		using std::int16_t;
		int16_t data = static_cast<int16_t>( 0x7fff * sample );
		write_mono_16bit( data );
	}

	void write_stereo_16bit( double sample_left, double sample_right )
	{
		write_mono_16bit( sample_left );
		write_mono_16bit( sample_right );
	}

	void write_mono_24bit( std::int32_t sample )
	{
		fout_.write( reinterpret_cast<char*>( &sample ), 3 );
		data_size_ += 3;
	}

	void write_stereo_24bit( std::int32_t sample_left,
	                         std::int32_t sample_right )
	{
		write_mono_24bit( sample_left );
		write_mono_24bit( sample_right );
	}

	void write_mono_24bit( double sample )
	{
		using std::int32_t;
		int32_t data = static_cast<int32_t>( 0x7fffff * sample );
		write_mono_24bit( data );
	}

	void write_stereo_24bit( double sample_left, double sample_right )
	{
		write_mono_24bit( sample_left );
		write_mono_24bit( sample_right );
	}

	operator bool ()
	{
		return fout_;
	}
};

#endif
