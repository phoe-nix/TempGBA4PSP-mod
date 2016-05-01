/* unofficial gameplaySP kai
 *
 * Copyright (C) 2006 Exophase <exophase@gmail.com>
 * Copyright (C) 2006 SiberianSTAR
 * Copyright (C) 2007 takka <takka@tfact.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"


#define ZIP_BUFFER_SIZE (128 * 1024)

struct SZIPFileDataDescriptor
{
  s32 CRC32;
  s32 CompressedSize;
  s32 UncompressedSize;
} __attribute__((packed));

struct SZIPFileHeader
{
  char Sig[4];
  s16 VersionToExtract;
  s16 GeneralBitFlag;
  s16 CompressionMethod;
  s16 LastModFileTime;
  s16 LastModFileDate;
  struct SZIPFileDataDescriptor DataDescriptor;
  s16 FilenameLength;
  s16 ExtraFieldLength;
}  __attribute__((packed));

void draw_progress(u32 output_size, u32 total_size);


s32 load_file_zip(char *filename)
{
  SceUID fd;
  struct SZIPFileHeader data;
  char tmp[MAX_FILE];
  s32 retval = -1;
  u8 *buffer = NULL;
  u8 *cbuffer = NULL;
  char *ext;

  FILE_OPEN(fd, filename, READ);

  if (!FILE_CHECK_VALID(fd))
    return -1;

  FILE_READ(fd, &data, sizeof(struct SZIPFileHeader));

  if ((data.Sig[0] != 0x50) || (data.Sig[1] != 0x4B) || (data.Sig[2] != 0x03) || (data.Sig[3] != 0x04))
  {
    goto outcode;
  }

  FILE_READ(fd, tmp, data.FilenameLength);
  tmp[data.FilenameLength] = 0; // end string

  if (data.ExtraFieldLength != 0)
  {
    FILE_SEEK(fd, data.ExtraFieldLength, SEEK_CUR);
  }

  if ((data.GeneralBitFlag & 0x0008) != 0)
  {
    FILE_READ(fd, &data.DataDescriptor, sizeof(struct SZIPFileDataDescriptor));
  }

  // file is too big
  if ((u32)data.DataDescriptor.UncompressedSize > gamepak_ram_buffer_size)
  {
    retval = -2;
    goto outcode;
  }

  ext = strrchr(tmp, '.') + 1;

  if ((strcasecmp(ext, "bin") == 0) || (strcasecmp(ext, "gba") == 0) || (strcasecmp(ext, "agb") == 0))
  {
    buffer = gamepak_rom;

    // ok, found
    switch (data.CompressionMethod)
    {
      case 0: // Z_NO_COMPRESSION
        retval = (u32)data.DataDescriptor.UncompressedSize;
        FILE_READ(fd, buffer, retval);
        break;

      case 8: // Z_DEFLATED
      {
        z_stream stream;
        s32 err;

        if ((cbuffer = (u8 *)malloc(ZIP_BUFFER_SIZE)) == NULL)
          break;
        memset(cbuffer, 0, ZIP_BUFFER_SIZE);

        stream.next_in   = (Bytef*)cbuffer;
        stream.avail_in  = (u32)ZIP_BUFFER_SIZE;
        stream.next_out  = (Bytef*)buffer;
        stream.avail_out = data.DataDescriptor.UncompressedSize;
        stream.zalloc    = (alloc_func)Z_NULL;
        stream.zfree     = (free_func)Z_NULL;
        stream.opaque    = (voidpf)Z_NULL;

        err = inflateInit2(&stream, -MAX_WBITS);

        FILE_READ(fd, cbuffer, ZIP_BUFFER_SIZE);

        if (err == Z_OK)
        {
          retval = (u32)data.DataDescriptor.UncompressedSize;

          while (err != Z_STREAM_END)
          {
            err = inflate(&stream, Z_SYNC_FLUSH);

            if (err == Z_BUF_ERROR)
            {
              stream.avail_in = ZIP_BUFFER_SIZE;
              stream.next_in  = (Bytef*)cbuffer;

              FILE_READ(fd, cbuffer, ZIP_BUFFER_SIZE);
            }

            draw_progress((u32)stream.total_out, (u32)retval);
            flip_screen(0);
          }

          err = Z_OK;
          inflateEnd(&stream);
        }

        free(cbuffer);
        break;
      }
    }
  }

outcode:
  FILE_CLOSE(fd);

  return retval;
}

void draw_progress(u32 output_size, u32 total_size)
{
  draw_box_line(140 - 4, 140 - 4, 340 + 4, 165 + 4, COLOR15_WHITE);
  draw_box_line(140 - 3, 140 - 3, 340 + 3, 165 + 3, COLOR15_WHITE);
  draw_box_fill(140, 140, 140 + ((u64)output_size * 200 / total_size), 165, COLOR15_WHITE);
}

