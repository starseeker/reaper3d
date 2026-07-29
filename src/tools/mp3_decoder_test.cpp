#include "hw/snd_mp3.h"
#include "res/res.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>

int main()
{
	using namespace reaper;

	hw::snd::Mp3Decoder decoder;
	if (!decoder.init(std::make_unique<res::res_stream>(
		    res::Music, "reaper.small", res::throw_on_error))) {
		std::cerr << "MP3 decoder initialization failed\n";
		return 1;
	}

	auto source = decoder.get();
	if (!source) {
		std::cerr << "MP3 decoder returned no source\n";
		return 1;
	}

	const auto info = source->info();
	if (info.channels != 2 || info.samplerate != 22050 ||
	    info.bits_per_sample != 16) {
		std::cerr << "Unexpected decoded format: " << info.channels
			  << " channels, " << info.samplerate << " Hz, "
			  << info.bits_per_sample << " bits\n";
		return 1;
	}

	std::uint64_t decoded_bytes = 0;
	std::uint64_t nonzero_bytes = 0;
	hw::snd::Samples samples;
	bool more = false;
	do {
		samples.clear();
		more = source->read(samples);
		decoded_bytes += samples.size();
		nonzero_bytes += static_cast<std::uint64_t>(std::count_if(
			samples.begin(), samples.end(),
			[](char sample) { return sample != 0; }));
	} while (more);

	if (decoded_bytes < 1'000'000 || nonzero_bytes == 0) {
		std::cerr << "Decoded output is missing or silent: "
			  << decoded_bytes << " bytes\n";
		return 1;
	}

	std::cout << "Decoded " << decoded_bytes << " bytes of stereo PCM\n";
	return 0;
}
