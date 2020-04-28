```c
#include "ov2640.h" 
 
uint8_t ov2640_mode=0;						/*!< Working mode :0,RGB565 mode; 1, JPEG mode */

/*!< JPEG size support list */
const uint16_t jpeg_img_size_tbl[][2]=
{
	160,120,	/*!< QQVGA */
	176,144,	/*!< QCIF */
	320,240,	/*!< QVGA */
	400,240,	/*!< WQVGA */
	352,288,	/*!< CIF */
	640,480,	/*!< VGA */
	800,600,	/*!< SVGA */
	1024,768,	/*!< XGA */
	1280,800,	/*!< WXGA */
	1280,960,	/*!< XVGA */
	1440,900,	/*!< WXGA+ */
	1280,1024,	/*!< SXGA */
	1600,1200,	/*!< UXGA	 */
};
const uint8_t*EFFECTS_TBL[7]={"Normal","Negative","B&W","Redish","Greenish","Bluish","Antique"};	/*!< Seven kinds of special effects  */
const uint8_t*JPEG_SIZE_TBL[13]={"QQVGA","QCIF","QVGA","WQVGA","CIF","VGA","SVGA","XGA","WXGA","XVGA","WXGA+","SXGA","UXGA"};/*!< JPEG images in 13 sizes  */

/*!< JPEG test */
/*!< JPEG data, sent to the computer via serial port 2. */
void jpeg_test(void)
{
 	OV2640_JPEG_Mode();		/*!< JPEG mode  */
	OV2640_OutSize_Set(jpeg_img_size_tbl[size][0],jpeg_img_size_tbl[size][1]);/*!< Set output size */
} s

/*!< RGB565 test */
/*!< RGB data is displayed directly on the LCD */
void rgb565_test(void)
{ 
	OV2640_RGB565_Mode();	/*!< RGB565 mode*/
  	OV2640_OutSize_Set(lcddev.width,lcddev.height); 
} 
```