/**
 * @file lv_freetype.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_freetype.h"
#include "lvgl/src/lv_misc/lv_debug.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
 
 /**********************
 *  STATIC VARIABLES
 **********************/
static FT_Library library;

#if USE_CACHE_MANGER
static FTC_Manager cache_manager;
static FTC_CMapCache cmap_cache;
/*static FTC_ImageCache image_cache;*/
static FTC_SBitCache sbit_cache;
static FTC_SBit sbit;

/**********************
 *      MACROS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
 
static FT_Error  font_Face_Requester(FTC_FaceID  face_id,
                         FT_Library  library,
                         FT_Pointer  req_data,
                         FT_Face*    aface)
{
    *aface = face_id;

    return FT_Err_Ok;
}
static bool get_glyph_dsc_cache_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    if(unicode_letter < 0x20) {
        dsc_out->adv_w = 0;
        dsc_out->box_h = 0;
        dsc_out->box_w = 0;
        dsc_out->ofs_x = 0;
        dsc_out->ofs_y = 0;
        dsc_out->bpp = 0;
        return true;
    }

	FT_UInt glyph_index;
	FT_UInt charmap_index;
	FT_Face face;
	lv_font_fmt_freetype_dsc_t * dsc = (lv_font_fmt_freetype_dsc_t *)(font->user_data);
    face = dsc->face;
	FTC_ImageTypeRec desc_sbit_type;

	desc_sbit_type.face_id = (FTC_FaceID)face;
	desc_sbit_type.flags = FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL;
	desc_sbit_type.height = dsc->font_size;
	desc_sbit_type.width = 0;

	/*FTC_Manager_LookupFace(cache_manager, face, &get_face);*/
	charmap_index = FT_Get_Charmap_Index(face->charmap);
	glyph_index = FTC_CMapCache_Lookup(cmap_cache, face, charmap_index, unicode_letter);
	FTC_SBitCache_Lookup(sbit_cache, &desc_sbit_type, glyph_index, &sbit, NULL);

    dsc_out->adv_w = sbit->xadvance;
    dsc_out->box_h = sbit->height;         /*Height of the bitmap in [px]*/
    dsc_out->box_w = sbit->width;         /*Width of the bitmap in [px]*/
    dsc_out->ofs_x = sbit->left;         /*X offset of the bitmap in [pf]*/
    dsc_out->ofs_y = sbit->top - sbit->height;         /*Y offset of the bitmap measured from the as line*/
    dsc_out->bpp = 8;         /*Bit per pixel: 1/2/4/8*/

    return true;                /*true: glyph found; false: glyph was not found*/
}

/* Get the bitmap of `unicode_letter` from `font`. */
static const uint8_t * get_glyph_bitmap_cache_cb(const lv_font_t * font, uint32_t unicode_letter)
{
	return (const uint8_t *)sbit->buffer;
}
#else
static bool get_glyph_dsc_cb(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    if(unicode_letter < 0x20) {
        dsc_out->adv_w = 0;
        dsc_out->box_h = 0;
        dsc_out->box_w = 0;
        dsc_out->ofs_x = 0;
        dsc_out->ofs_y = 0;
        dsc_out->bpp = 0;
        return true;
    }

    int error;
    FT_Face face;
    lv_font_fmt_freetype_dsc_t * dsc = (lv_font_fmt_freetype_dsc_t *)(font->user_data);
    face = dsc->face;

	FT_UInt glyph_index = FT_Get_Char_Index( face, unicode_letter );

	error = FT_Load_Glyph(
			face,          /* handle to face object */
			glyph_index,   /* glyph index           */
			FT_LOAD_DEFAULT );  /* load flags, see below *///FT_LOAD_MONOCHROME|FT_LOAD_NO_AUTOHINTING
	if ( error )
	{
		printf("Error in FT_Load_Glyph: %d\n", error);
		return error;
	}

	error = FT_Render_Glyph( face->glyph,   /* glyph slot  */
			FT_RENDER_MODE_NORMAL ); /* render mode */ //

	if ( error )
	{
		printf("Error in FT_Render_Glyph: %d\n", error);
		return error;
	}

    dsc_out->adv_w = (face->glyph->metrics.horiAdvance >> 6);
    dsc_out->box_h = face->glyph->bitmap.rows;         /*Height of the bitmap in [px]*/
    dsc_out->box_w = face->glyph->bitmap.width;         /*Width of the bitmap in [px]*/
    dsc_out->ofs_x = face->glyph->bitmap_left;         /*X offset of the bitmap in [pf]*/
    dsc_out->ofs_y = face->glyph->bitmap_top - face->glyph->bitmap.rows;         /*Y offset of the bitmap measured from the as line*/
    dsc_out->bpp = 8;         /*Bit per pixel: 1/2/4/8*/

    return true;
}

/* Get the bitmap of `unicode_letter` from `font`. */
static const uint8_t * get_glyph_bitmap_cb(const lv_font_t * font, uint32_t unicode_letter)
{
	FT_Face face;
	lv_font_fmt_freetype_dsc_t * dsc = (lv_font_fmt_freetype_dsc_t *)(font->user_data);
	face = dsc->face;
	return (const uint8_t *)(face->glyph->bitmap.buffer);
}
#endif


/**********************
 *   GLOBAL FUNCTIONS
 **********************/
 
/**
* init freetype library
* @param max_faces Maximum number of opened @FT_Face objects managed by this cache
* @return FT_Error
* example: if you have two faces,max_faces should >= 2
*/
int lv_freetype_init(uint8_t max_faces)
{
    FT_Error error;
    error = FT_Init_FreeType(&library);
    if ( error )
	{
		printf("Error in FT_Init_FreeType: %d\n", error);
		return error;
	}
#if USE_CACHE_MANGER
    //cache
    error = FTC_Manager_New(library, max_faces, 1, 0, font_Face_Requester, NULL, &cache_manager);
	if(error)
	{
		printf("Failed to open cache manager\r\n");
		return error;
	}

	error = FTC_CMapCache_New(cache_manager, &cmap_cache);
	if(error)
	{
		printf("Failed to open Cmap Cache\r\n");
		return error;
	}
	/*
	error = FTC_ImageCache_New(cache_manager, &image_cache);
	if(error)
	{
		printf("Failed to open image cache\r\n");
		return error;
	}
	*/
	error = FTC_SBitCache_New(cache_manager, &sbit_cache);
	if(error)
	{
		printf("Failed to open sbit cache\r\n");
		return error;
	}
#endif
    return FT_Err_Ok;
}

/**
* init lv_font_t struct
* @param font pointer to a font
* @param font_path the font path
* @param font_size the height of font
* @return FT_Error
*/
int lv_freetype_font_init(lv_font_t * font, const char * font_path, uint16_t font_size)
{
    FT_Error error;

    lv_font_fmt_freetype_dsc_t * dsc = lv_mem_alloc(sizeof(lv_font_fmt_freetype_dsc_t));
    LV_ASSERT_MEM(dsc);
    if(dsc == NULL) return FT_Err_Out_Of_Memory;

    dsc->font_size = font_size;

    error = FT_New_Face(library, font_path, 0, &dsc->face);
    if ( error ) {
		printf("Error in FT_New_Face: %d\n", error);
		return error;
	}
    error = FT_Set_Pixel_Sizes(dsc->face, 0,font_size);
    if ( error ) {
		printf("Error in FT_Set_Char_Size: %d\n", error);
		return error;
	}

#if USE_CACHE_MANGER
    font->get_glyph_dsc = get_glyph_dsc_cache_cb;        /*Set a callback to get info about gylphs*/
	font->get_glyph_bitmap = get_glyph_bitmap_cache_cb;  /*Set a callback to get bitmap of a glyp*/
#else
	font->get_glyph_dsc = get_glyph_dsc_cb;        /*Set a callback to get info about gylphs*/
	font->get_glyph_bitmap = get_glyph_bitmap_cb;  /*Set a callback to get bitmap of a glyp*/
#endif

#ifndef LV_USE_USER_DATA
#error "lv_freetype : user_data is required.Enable it lv_conf.h(LV_USE_USER_DATA 1)"
#endif
    font->user_data = dsc;
	font->line_height = (dsc->face->size->metrics.height >> 6);
    font->base_line = -(dsc->face->size->metrics.descender >> 6);  /*Base line measured from the top of line_height*/
	font->subpx = LV_FONT_SUBPX_NONE;

    
    return FT_Err_Ok;
}

