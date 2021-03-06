///
/// \file	bmp.cc
///		BMP conversion routines
///

/*
    Copyright (C) 2009-2012, Net Direct Inc. (http://www.netdirect.ca/)
    Copyright (C) 2008-2009, Nicolas VIVIEN

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    See the GNU General Public License in the COPYING file at the
    root directory of this project for more details.
*/

#include "bmp.h"
#include "bmp-internal.h"
#include "error.h"
#include "endian.h"
#include "data.h"
#include "m_javaloader.h"

namespace Barry {

//
// GetBitmapHeadersSize
//
/// Returns the size of the bitmap headers (both file and info headers).
/// You can use this as an offset into the bitmap produced by
/// ScreenshotToBitmap to get just the 4-byte RGB data.
///
size_t GetBitmapHeadersSize()
{
	return sizeof(bmp_file_header_t) +
		sizeof(bmp_info_header_t);
}

//
// GetTotalBitmapSize
//
/// Returns the total number of bytes needed to convert a
/// screenshot of the given dimensions into a bitmap,
/// using the ScreenshotToBitmap() function.
///
size_t GetTotalBitmapSize(const JLScreenInfo &info)
{
	return sizeof(bmp_file_header_t) +
		sizeof(bmp_info_header_t) +
		(info.width * info.height * 4);	// 4 byte RGB per pixel

}

//
// ScreenshotToRGB
//
/// Converts screenshot data obtained via JavaLoader::GetScreenshot()
/// into uncompressed RGB bitmap format.  The results will not have
/// a bitmap BMP header.  Data will be written to buffer, starting
/// at offset.  The depth variable can be 24 or 32.  If invert is
/// true, the result will be inverted, just like a BMP file; otherwise not.
///
void ScreenshotToRGB(const JLScreenInfo &info,
			     const Data &screenshot,
			     Data &buffer,
			     size_t offset,
			     int depth,
			     bool invert,
			     bool overwrite_alpha,
			     uint8_t alpha)
{
	if( depth != 24 && depth != 32 )
		throw Barry::Error("ScreenshotToRGB: depth must be 24 or 32");

	// if user doesn't want to overwrite alpha channel, then use
	// the value for our own default
	if( !overwrite_alpha )
		alpha = 0xFF;

	size_t width = info.width;
	size_t height = info.height;
	size_t bytes_per_pixel = (depth == 24) ? 3 : 4;
	size_t pixel_count = width * height;
	size_t total_bitmap_size = pixel_count * bytes_per_pixel;
	size_t total_buffer_size = total_bitmap_size + offset;

	// using pixel_count (width*height), determine the size used
	// per pixel
	size_t data_size;
	for( data_size = 2; screenshot.GetSize() > (data_size * pixel_count); data_size++ )
		;
	if( screenshot.GetSize() < (pixel_count * data_size) )
		throw Error("ScreenshotToRGB: Screenshot data size is too small for given width+height");
	if( data_size != 2 && data_size != 4 )
		throw Error("ScreenshotToRGB: Screenshot depth is not supported (Barry supports 2 byte or 4 byte pixels in device screenshots)");

	// setup write pointer
	unsigned char *write = buffer.GetBuffer(total_buffer_size) + offset;

	// pointer into screenshot data (grabbing pixel bytes per data_size)
	const uint8_t *data = (const uint8_t*) screenshot.GetData();

	// For each pixel... (note BMP format is up and backwards, hence
	// offset calculation for each pixel in for loop)
	for( size_t j = 0; j < height; j++ ) {
		for( size_t i = 0; i < width; i++ ) {
			// Read one pixel in the picture

			// offset is in pixels... so multiply it by data_size
			// to read out of data
			int get_offset = 0;
			if( invert )
				get_offset = (pixel_count - 1) - ((width-1 - i) + (width * j));
			else
				get_offset = (j * width) + i;

			// extract the pixel data, using data_size bytes
			uint32_t value;
			switch( data_size )
			{
			case 2:
				value = ((const uint16_t *)data)[get_offset];

				// 16bit pixel format used by the handheld is:
				// MSB < .... .... .... .... > LSB
				//                    ^^^^^^ : Blue (between 0x00 and 0x1F)
				//             ^^^^^^^ : Green (between 0x00 and 0x3F)
				//       ^^^^^^ : Red (between 0x00 and 0x1F)

				if( bytes_per_pixel == 4 )
					write[3] = alpha;

				write[2] = (((value >> 11) & 0x1F) * 0xFF) / 0x1F;	// red
				write[1] = (((value >> 5) & 0x3F) * 0xFF) / 0x3F;	// green
				write[0] = ((value & 0x1F) * 0xFF) / 0x1F;		// blue
				break;
			case 4:
				value = ((const uint32_t *)data)[get_offset];

				// 32bit pixel format used by the handheld is
				// assumed to be RGBA

				if( bytes_per_pixel == 4 ) {
					if( overwrite_alpha )
						write[3] = alpha;
					else
						write[3] = (value >> 24) & 0xFF;// alpha
				}

				write[2] = (value >> 16) & 0xFF;   // red
				write[1] = (value >> 8) & 0xFF;    // green
				write[0] = value & 0xFF;           // blue
				break;
			default:
				throw Error("ScreenshotToRGB: bad switch value, should never happen. Double check the data_size check.");
			}

			write += bytes_per_pixel;
		}
	}

	buffer.ReleaseBuffer(total_buffer_size);
}

//
// ScreenshotToBitmap
//
/// Converts screenshot data obtained via JavaLoader::GetScreenshot()
/// into uncompressed bitmap format, suitable for writing BMP files.
/// Arguments info and screenshot come from GetScreenshot() and the
/// converted data is stored in bitmap.
///
//
// This function assumes that the btoh() converter functions match
// the needs of the bitmap file format.  Namely: little endian.
//
void ScreenshotToBitmap(const JLScreenInfo &info,
				 const Data &screenshot,
				 Data &bitmap)
{
	// Read screen info
	size_t width = info.width;
	size_t height = info.height;
	size_t total_bitmap_size = GetTotalBitmapSize(info);

	// make sure there is enough screeshot pixel data for the
	// given width and height
	if( screenshot.GetSize() < (width * height * 2) ) // 2 byte screenshot pixel data
		throw Error("Screenshot data size is too small for given width+height");


	// setup write pointer
	unsigned char *bitbuf = bitmap.GetBuffer(total_bitmap_size);
	unsigned char *write = bitbuf;

	//
	// Build header BMP file
	//
	bmp_file_header_t *fileheader = (bmp_file_header_t*) write;
	write += sizeof(bmp_file_header_t);

	// BMP
	fileheader->bfType[0] = 'B';
	fileheader->bfType[1] = 'M';

	// Size of file
	fileheader->bfSize = btohl(total_bitmap_size);

	// Always 0x00
	fileheader->bfReserved1 = 0;
	fileheader->bfReserved2 = 0;

	// Offset to find the data
	fileheader->bfOffBits = btohl(sizeof(bmp_file_header_t) + sizeof(bmp_info_header_t));


	//
	// Build info BMP file
	//
	bmp_info_header_t *infoheader = (bmp_info_header_t*) write;
	write += sizeof(bmp_info_header_t);

	// Size of info section
	infoheader->biSize = btohl(sizeof(bmp_info_header_t));

	// Width x Height
	infoheader->biWidth = btohl(width);
	infoheader->biHeight = btohl(height);

	// Planes number
	infoheader->biPlanes = btohs(0x01);

	// Bit count
	infoheader->biBitCount = btohs(0x20);

	// Compression : No
	infoheader->biCompression = 0;

	// Size of image
	infoheader->biSizeImage = btohl(4 * width * height);

	// Pels Per Meter
	infoheader->biXPelsPerMeter = 0;
	infoheader->biYPelsPerMeter = 0;

	// Color palette used : None
	infoheader->biClrUsed = 0;

	// Color palette important : None
	infoheader->biClrImportant = 0;

	// Fill in the RGB data
	ScreenshotToRGB(info, screenshot, bitmap, write - bitbuf, 32, true);

	bitmap.ReleaseBuffer(total_bitmap_size);
}

} // namespace Barry

