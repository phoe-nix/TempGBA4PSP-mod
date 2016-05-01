/***************************************************************************

  png.c

    PSP PNG format image I/O functions. (based on M.A.M.E. PNG functions)

***************************************************************************/

/* copy from njemu */

#include "common.h"


#define PNG_Signature "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"

#define PNG_CN_IHDR 0x49484452L     /* Chunk names */
#define PNG_CN_PLTE 0x504C5445L
#define PNG_CN_IDAT 0x49444154L
#define PNG_CN_IEND 0x49454E44L
#define PNG_CN_gAMA 0x67414D41L
#define PNG_CN_sBIT 0x73424954L
#define PNG_CN_cHRM 0x6348524DL
#define PNG_CN_tRNS 0x74524E53L
#define PNG_CN_bKGD 0x624B4744L
#define PNG_CN_hIST 0x68495354L
#define PNG_CN_tEXt 0x74455874L
#define PNG_CN_zTXt 0x7A545874L
#define PNG_CN_pHYs 0x70485973L
#define PNG_CN_oFFs 0x6F464673L
#define PNG_CN_tIME 0x74494D45L
#define PNG_CN_sCAL 0x7343414CL


/* PNG support */
struct PngInfo
{
  u32 width;
  u32 height;
  u8  *image;
  u32 rowbytes;
  u8  *zimage;
  u32 zlength;
};

/********************************************************************************

  Helper functions

********************************************************************************/

static void png_error_msg(int number)
{
  GUI_ACTION_TYPE gui_action = CURSOR_NONE;

  draw_box_alpha(110, 50, 370, 220, 0xBF000000);
  draw_box_line(120, 60, 360, 210, COLOR15_WHITE);

  print_string(MSG[MSG_ERR_SS_PNG_0 + number], X_POS_CENTER, 100, COLOR15_WHITE, BG_NO_FILL);
  print_string(MSG[MSG_ERR_CONT], X_POS_CENTER, 160, COLOR15_WHITE, BG_NO_FILL);
  flip_screen(1);

  while (gui_action == CURSOR_NONE)
  {
    gui_action = get_gui_input();
  }
}

/********************************************************************************

  PNG write functions

********************************************************************************/

struct PngText
{
  char *data;
  int length;
  struct PngText *next;
};

static struct PngText *png_text_list = 0;

static void convert_to_network_order(u32 i, u8 *v)
{
  v[0] = (i >> 24) & 0xff;
  v[1] = (i >> 16) & 0xff;
  v[2] = (i >>  8) & 0xff;
  v[3] = (i >>  0) & 0xff;
}

static int png_add_text(const char *keyword, const char *text)
{
  struct PngText *pt;

  if ((pt = (struct PngText *)malloc(sizeof(struct PngText))) == NULL)
    return 0;

  pt->length = strlen(keyword) + strlen(text) + 1;
  if ((pt->data = (char *)malloc(pt->length + 1)) == NULL)
    return 0;

  strcpy(pt->data, keyword);
  strcpy(pt->data + strlen(keyword) + 1, text);
  pt->next = png_text_list;
  png_text_list = pt;

  return 1;
}

static int write_chunk(SceUID fd, u32 chunk_type, u8 *chunk_data, u32 chunk_length)
{
  u32 crc;
  u8 v[4];
  u32 written;

  /* write length */
  convert_to_network_order(chunk_length, v);
  written = sceIoWrite(fd, v, 4);

  /* write type */
  convert_to_network_order(chunk_type, v);
  written += sceIoWrite(fd, v, 4);

  /* calculate crc */
  crc = crc32(0, v, 4);
  if (chunk_length > 0)
  {
    /* write data */
    written += sceIoWrite(fd, chunk_data, chunk_length);
    crc = crc32(crc, chunk_data, chunk_length);
  }
  convert_to_network_order(crc, v);

  /* write crc */
  written += sceIoWrite(fd, v, 4);

  if (written != 3 * 4 + chunk_length)
  {
    png_error_msg(1);
    return 0;
  }

  return 1;
}

static int png_write_sig(SceUID fd)
{
  /* PNG Signature */
  if (sceIoWrite(fd, PNG_Signature, 8) != 8)
  {
    png_error_msg(1);
    return 0;
  }

  return 1;
}

static int png_write_datastream(SceUID fd, struct PngInfo *p)
{
  u8 ihdr[13];
  struct PngText *pt;

  /* IHDR */
  convert_to_network_order(p->width, ihdr);
  convert_to_network_order(p->height, ihdr + 4);
  *(ihdr +  8) = 8;  // bit depth;
  *(ihdr +  9) = 2;  // color type
  *(ihdr + 10) = 0;  // compression method;
  *(ihdr + 11) = 0;  // fliter
  *(ihdr + 12) = 0;  // interlace

  if (write_chunk(fd, PNG_CN_IHDR, ihdr, 13) == 0)
    return 0;

  /* IDAT */
  if (write_chunk(fd, PNG_CN_IDAT, p->zimage, p->zlength) == 0)
    return 0;

  /* tEXt */
  while (png_text_list)
  {
    pt = png_text_list;
    if (write_chunk(fd, PNG_CN_tEXt, (u8 *)pt->data, pt->length) == 0)
      return 0;
    free(pt->data);

    png_text_list = pt->next;
    free(pt);
  }

  /* IEND */
  if (write_chunk(fd, PNG_CN_IEND, NULL, 0) == 0)
    return 0;

  return 1;
}

static int png_deflate_image(struct PngInfo *p)
{
  unsigned long zbuff_size;
  z_stream stream;

  zbuff_size = (p->height * (p->rowbytes + 1)) * 1.1 + 12;

  if ((p->zimage = (u8 *)malloc(zbuff_size)) == NULL)
  {
    png_error_msg(0);
    return 0;
  }

  stream.next_in   = (Bytef*)p->image;
  stream.avail_in  = p->height * (p->rowbytes + 1);
  stream.next_out  = p->zimage;
  stream.avail_out = (uInt)&zbuff_size;
  stream.zalloc    = (alloc_func)0;
  stream.zfree     = (free_func)0;
  stream.opaque    = (voidpf)0;

  if (deflateInit(&stream, Z_BEST_COMPRESSION) == Z_OK)
  {
    if (deflate(&stream, Z_FINISH) == Z_STREAM_END)
    {
      deflateEnd(&stream);
      p->zlength = stream.total_out;
      return 1;
    }
    deflateEnd(&stream);
  }

  png_error_msg(1);
  return 0;
}

static int png_create_datastream(SceUID fd, u16 *screen_image)
{
  u32 x, y;
  u8 *dst;
  struct PngInfo p;

  memset(&p, 0, sizeof (struct PngInfo));

  p.width    = GBA_SCREEN_WIDTH;
  p.height   = GBA_SCREEN_HEIGHT;
  p.rowbytes = p.width * 3;

  if ((p.image = (u8 *)malloc(p.height * (p.rowbytes + 1))) == NULL)
  {
    png_error_msg(0);
    return 0;
  }

  dst = p.image;

  u16 *image, *src;

  image = screen_image;

  for (y = 0; y < p.height; y++)
  {
    src = &image[y * GBA_SCREEN_WIDTH];
    *dst++ = 0;

    for (x = 0; x < p.width; x++)
    {
      u16 color = src[x];

      *dst++ = (u8)COL15_GET_R8(color);
      *dst++ = (u8)COL15_GET_G8(color);
      *dst++ = (u8)COL15_GET_B8(color);
    }
  }

  if (png_deflate_image(&p) == 0)
    return 0;

  if (png_write_datastream(fd, &p) == 0)
    return 0;

  if (p.image)  free(p.image);
  if (p.zimage) free(p.zimage);

  return 1;
}


/*--------------------------------------------------------
  PNG•Û‘¶
--------------------------------------------------------*/

int save_png(const char *path, u16 *screen_image)
{
  SceUID fd;
  int res = 0;

  if ((fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT, 0777)) >= 0)
  {
    if ((res = png_add_text("Software", "TempGBA")))
    {
      if ((res = png_add_text("System", "PSP")))
      {
        if ((res = png_write_sig(fd)))
        {
          res = png_create_datastream(fd, screen_image);
        }
      }
    }

    sceIoClose(fd);
  }

  return res;
}

