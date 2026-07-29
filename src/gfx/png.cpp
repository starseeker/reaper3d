/*
 * $Author: pstrand $
 * $Date: 2002/09/23 18:37:18 $
 * $Log: png.cpp,v $
 * Revision 1.31  2002/09/23 18:37:18  pstrand
 * *** empty log message ***
 *
 * Revision 1.30  2002/09/23 12:08:43  fizzgig
 * Moved all anonymous namespaces into the reaper-namespace
 *
 * Revision 1.29  2002/09/20 23:41:25  pstrand
 * *** empty log message ***
 *
 * Revision 1.28  2002/09/13 07:45:55  pstrand
 * gcc-varningar
 *
 * Revision 1.27  2002/06/12 17:50:39  pstrand
 * int != long
 *
 * Revision 1.26  2001/12/02 21:48:50  peter
 * ''
 *
 * Revision 1.25  2001/12/02 19:47:44  peter
 * get_row -> get_col
 *
 * Revision 1.24  2001/11/26 02:20:06  peter
 * ljudfixar (smartptr), andra mindre fixar..
 *
 * Revision 1.23  2001/08/06 12:16:15  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.22.4.1  2001/07/31 17:34:04  peter
 * testgren...
 *
 * Revision 1.22  2001/05/10 11:40:15  macke
 * häpp
 *
 */


#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "gfx/exceptions.h"
#include "res/res.h"

#include <png.h>

namespace reaper {
namespace {
using reaper::gfx::gfx_fatal_error;
using std::string;

void read_data_fn(png_structp png, png_bytep data, png_size_t length)
{
	std::istream* ds = reinterpret_cast<std::istream*>(png_get_io_ptr(png));
	ds->read(reinterpret_cast<char*>(data), length);
}

void write_data_fn(png_structp png, png_bytep data, png_size_t length)
{
	std::ostream* os = reinterpret_cast<std::ostream*>(png_get_io_ptr(png));
	os->write(reinterpret_cast<char*>(data), length);
}

void flush_data_fn(png_structp png)
{
	std::ostream* os = reinterpret_cast<std::ostream*>(png_get_io_ptr(png));
	os->flush();
}

void error(const std::string& id, const std::string& msg)
{
	throw gfx_fatal_error(string("PngLoadImage: While reading ") + id + ": " + msg + "\n");
}

struct PngReadState
{
	png_structp png = nullptr;
	png_infop info = nullptr;

	~PngReadState()
	{
		if (png != nullptr)
			png_destroy_read_struct(&png, info != nullptr ? &info : nullptr, nullptr);
	}
};

struct PngWriteState
{
	png_structp png = nullptr;
	png_infop info = nullptr;

	~PngWriteState()
	{
		if (png != nullptr)
			png_destroy_write_struct(&png, info != nullptr ? &info : nullptr);
	}
};

// PNG image handler functions
void PngLoadImage (const std::string& id, png_byte **data,
        png_uint_32 *width, png_uint_32 *height, png_byte *channels)
{
	png_byte sig[8]{};
	int bit_depth;
	int color_type;
	double gamma;
	png_color_16* background;

	*data = nullptr;

	reaper::res::res_stream png_file(reaper::res::Texture, id);


	png_file.read(reinterpret_cast<char*>(sig), 8);

	if (!png_check_sig(sig, 8)) {
		error(id, "png signature error");
	}

	PngReadState state;
	state.png = png_create_read_struct(
		PNG_LIBPNG_VER_STRING,
		nullptr,
		nullptr,
		nullptr);
	if (state.png == nullptr) {
		error(id, "png read error");
	}

	state.info = png_create_info_struct(state.png);
	if (state.info == nullptr) {
		error(id, "png read error");
	}

	png_set_read_fn(state.png, &png_file, read_data_fn);

	png_set_sig_bytes(state.png, 8);
	png_read_info(state.png, state.info);
	png_get_IHDR(state.png, state.info, width, height,
		     &bit_depth, &color_type, 0, 0, 0);

	if (bit_depth == 16)
		png_set_strip_16(state.png);
	if (color_type == PNG_COLOR_TYPE_PALETTE || bit_depth < 8)
		png_set_expand(state.png);
	if (png_get_valid(state.png, state.info, PNG_INFO_tRNS))
		png_set_expand(state.png);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(state.png);
	if (png_get_bKGD(state.png, state.info, &background))
		png_set_background(state.png, background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	if (png_get_gAMA(state.png, state.info, &gamma))
		png_set_gamma(state.png, 2.2, gamma);

	png_read_update_info(state.png, state.info);

	png_get_IHDR(state.png, state.info, width, height,
		     &bit_depth, &color_type, 0, 0, 0);

	const png_uint_32 row_bytes =
		png_get_rowbytes(state.png, state.info);
	*channels = png_get_channels(state.png, state.info);

	using PixelData = std::unique_ptr<png_byte, decltype(&std::free)>;
	PixelData pixels(
		static_cast<png_byte*>(std::malloc(row_bytes * *height)),
		&std::free);
	if (!pixels) {
		error(id, "png malloc error");
	}

	std::vector<png_bytep> row_pointers(*height);
	for (png_uint_32 row = 0; row < *height; ++row)
		row_pointers[row] = pixels.get() + row * row_bytes;

	png_read_image(state.png, row_pointers.data());
	png_read_end(state.png, nullptr);
	*data = pixels.release();
}


void PngSaveImage(
	const std::string& id,
	png_byte* data,
	int width,
	int height)
{
	constexpr int bit_depth = 8;
	constexpr int channels = 3;

	reaper::res::res_out_stream ro(reaper::res::Screenshot, id);

	PngWriteState state;
	state.png = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		nullptr,
		nullptr,
		nullptr);
	if (state.png == nullptr) {
		throw gfx_fatal_error(string("PngSaveIMage: Unable to create write structure: ") + id);
	}

	state.info = png_create_info_struct(state.png);
	if (state.info == nullptr) {
		throw gfx_fatal_error(string("PngSaveIMage: Unable to create info structure: ") + id);
	}

	png_set_write_fn(state.png, &ro, write_data_fn, flush_data_fn);

	png_set_IHDR(state.png, state.info, width, height, bit_depth,
		PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);

	png_write_info(state.png, state.info);
	const auto row_bytes =
		static_cast<std::size_t>(width) * channels;
	std::vector<png_bytep> rows(height);
	for (int row = 0; row < height; ++row)
		rows[row] = data + row * row_bytes;

	png_write_image(state.png, rows.data());
	png_write_end(state.png, state.info);
}

} // end anonymous namespace

namespace gfx {
namespace misc {

int load_png(const string &file,char *&data,int &w,int &h,bool check_dim)
{
        png_byte channels;
	png_uint_32 width, height;
        PngLoadImage(file, reinterpret_cast<png_byte**>(&data),
		     &width, &height, &channels);

	w = width; h = height;
	
	if (!check_dim)
		return channels;

        // ensure that all sides are a power of two
        bool w_ok = false;
        bool h_ok = false;

        // Texture dimensions supported are 1 to 4096 texels
        for(int i=0;i<13;++i) {
                if(w == (1 << i))
                        w_ok = true;
                if(h == (1 << i))
                        h_ok = true;
        }

        if(!(w_ok && h_ok)) {
                std::free(data);
                data = nullptr;
                throw gfx_fatal_error(string("Texture: Illegal dimensions for texture ") + file);
        }              

	return channels;
}

void save_png(const string &file, char *data, int w, int h)
{
	PngSaveImage(file, reinterpret_cast<unsigned char*>(data), w, h);

}

}
}
}
