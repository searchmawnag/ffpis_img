/***********************************************************************
      LIBRARY: IMAGE - Image Manipulation and Processing Routines

      FILE:    IMGUTIL.C

      AUTHOR:  Michael Garris
      DATE:    05/29/1991

      Contains general image manipulation routines.

      ROUTINES:
#cat: init_image_data - allocates and initializes to zero an image of
#cat:                   specified dimensions and depth.
#cat: PixPerByte - returns the pixel per byte factor given the depth of
#cat:              a pixel in an image.
#cat: SizeFromDepth - computes the byte size of an image based on the
#cat:                 image's pixel width, pixel height, and pixel depth.
#cat: WordAlignFromDepth - computes a byte aligned scanline length given
#cat:                      a pixel width and pixel depth.
#cat: allocate_image - allocates an image of specified width, height, and
#cat:                  depth. uses calloc for zeroed result.
#cat: mallocate_image - allocates an image of specified width, height, and
#cat:                  depth. uses malloc for more speed.
#cat: alloc_short_image - allocates a 16-bit image of specified dimensions.
#cat:
#cat: alloc_char_image - allocates an 8-bit image of specified dimensions.
#cat:
#cat: alloc_int_image - allocates a 32-bit image of specified dimensions.
#cat:
#cat: allocate_aligned_image - allocates an image width a width which is a 
#cat:                          multiple of 16 at least as wide as requested.
#cat: width_16 - calculate a width which is a multiple of 16 at least equal
#cat:            to the supplied width.

***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <malloc.h>
#include "imgutil.h"
#include <ffpis/util/util.h>
#include "memalloc.h"

#define SHORT_DEPTH 16
#define CHAR_DEPTH   8

/************************************************************/
/*         Routine:   Init_image_data()                     */
/*         Author:    Michael D. Garris                     */
/*         Date:      5/29/91                               */
/************************************************************/
/************************************************************/
/* Init_image_data() allocates, initializes to zero, and    */
/* returns a raster image memory of specified dimensions.   */
/************************************************************/
void init_image_data(
unsigned char **data,
int *size,
int width,int  height,int  depth)
{
   (*size) = SizeFromDepth(width, height, depth);
   (*data) = (unsigned char *)malloc((*size) * sizeof(unsigned char));
   if((*data) == NULL)
      syserr("init_image_data", "maloc", "data");
   memset((*data), 0, (*size) * sizeof(unsigned char));
}

/************************************************************/
/*         Routine:   PixPerByte()                          */
/*         Author:    Michael D. Garris                     */
/*                    Darrin Dimmick                        */
/*         Date:      3/07/90                               */
/*         Modifications:                                   */
/*           9/20/90    (Stan Janet) error message          */
/************************************************************/
/************************************************************/
/* PixPerByte() takes the pixel depth of an image and       */
/* returns the corresponding pixperbyte factor.             */
/************************************************************/
float PixPerByte( int depth)
{
float pixperbyte;
   switch (depth)
   {
      case  1: pixperbyte = 8.0; break;
      case  2: pixperbyte = 4.0; break;
      case  4: pixperbyte = 2.0; break;
      case  8: pixperbyte = 1.0; break;
      case 16: pixperbyte = 0.5; break;
      case 32: pixperbyte = 0.25; break;
      case 64: pixperbyte = 0.125; break;
      default: fatalerr("PixPerByte",
	   "depth != power of 2 between 1 and 64 (inclusive)", NULL);
   }
   return pixperbyte;
}

/************************************************************/
/*         Routine:   SizeFromDepth()                       */
/*         Author:    Michael D. Garris                     */
/*                    Darrin Dimmick                        */
/*         Date:      2/20/90                               */
/*         Modifications:                                   */
/*           9/20/90   (Stan Janet) add ceil() [bug]        */
/************************************************************/
/************************************************************/
/* SizeFromDepth() takes the pixel width of an image scan   */
/* line along with the pixel height of the image, and using */
/* the argument "depth" computes the length of the image in */
/* bytes.                                                   */
/************************************************************/
int SizeFromDepth( int pixwidth,int pixheight,int depth)
{
   int filesize;
   float pixperbyte;

   /* If RGB ... */
   if(depth == 24){
      filesize = pixwidth * pixheight * 3;
      return(filesize);
   }

   pixperbyte = PixPerByte(depth);
   filesize = ( ceil(pixwidth/pixperbyte) * pixheight);
   return filesize;
}

/************************************************************/
/*         Routine:   WordAlignFromDepth()                  */
/*         Author:    Michael D. Garris                     */
/*         Date:      3/02/90                               */
/*         Modifications:                                   */
/*           9/20/90   (Stan Janet) simplified assignment 3 */
/************************************************************/
/************************************************************/
/* WordAlignFromDepth() takes the pixel width of and image  */
/* and determines the actual aligned pixel width of the     */
/* stored scan line.                                        */
/************************************************************/
int WordAlignFromDepth( int pixwidth,int  depth)
{
   int bytesperline, pixperline;
   float pixperbyte;

   pixperbyte = PixPerByte(depth);

   /* byte align pixels */
   bytesperline =  ceil(pixwidth/pixperbyte);

   /* word aling bytes per line */
   if (bytesperline & 0x1)
	++bytesperline;
   /* was:
   bytesperline = (int)(ceil(bytesperline/2.0)) << 1;
   */

   /* calculate aligned pixel per line */
   pixperline = (int)(bytesperline * pixperbyte);
   return pixperline;
}
/************************************************************/
/*         Routine:   allocate_image()                      */
/*         Author:    Darrin L. Dimmick                     */
/*         Date:      3/02/90                               */
/*         Modifications:                                   */
/*	      Patrick Grother July 1995 - added malloc      */
/*            version to avoid calloc zeroing the data      */
/*            thereby gaining speed			    */
/*							    */
/*            changed error[2048] to error[64] to save	    */
/*            some wasted space				    */
/************************************************************/
/************************************************************/
/* Allocate_image() takes the pixel width, height and depth */
/* of an image and uses SizeFromDepth to calloc the         */
/* requested memory. returns zeroed image                   */
/* Mallocate_image() takes the pixel width, height and depth*/
/* of an image and uses SizeFromDepth to malloc the         */
/* requested memory. returns uninitialized image            */
/************************************************************/

unsigned char *allocate_image( int width,int  height,int  depth)
{
  char error[64];
  unsigned char *image = NULL;

  if (width < 1){
     sprintf(error,"width = %d", width);
     fatalerr("allocate_image","Invalid dimension", error);
  }
  if (height < 1){
     sprintf(error,"height = %d", height);
     fatalerr("allocate_image","Invalid dimension", error);
  }
  if (depth < 1){
     sprintf(error,"depth = %d", depth);
     fatalerr("allocate_image","Invalid dimension", error);
  }
  calloc_uchar(&image, SizeFromDepth(width, height, depth),
               "allocate_image : image");
  return(image);
}

unsigned char *mallocate_image( int width,int  height,int  depth)
{
  char error[64];
  unsigned char *image = NULL;
 
  if (width < 1){
     sprintf(error,"width = %d", width);
     fatalerr("mallocate_image","Invalid dimension", error);
  }
  if (height < 1){
     sprintf(error,"height = %d", height);
     fatalerr("mallocate_image","Invalid dimension", error);    
  }
  if (depth < 1){  
     sprintf(error,"depth = %d", depth);
     fatalerr("mallocate_image","Invalid dimension", error);
  }                    
  malloc_uchar(&image, SizeFromDepth(width, height, depth),
               "allocate_image : image");
  return(image);                 
}

/************************************************************/
/*         Routine:   alloc_short_image()                   */
/*         Author:    Darrin L. Dimmick                     */
/*         Date:      9/02/93 added to library              */
/*         Modifications:                                   */
/************************************************************/
/************************************************************/
/* Alloc_short_image() takes the pixel width, height of an  */
/* image and uses SizeFromDepth to calloc the requested     */
/* memory. Returning a pointer to a short.                  */
/************************************************************/
short *alloc_short_image( int width,int  height)
{
  int nbytes = 0;
  short *sdata = NULL;

   nbytes = SizeFromDepth(width, height, SHORT_DEPTH);
   if((sdata = (short *)calloc(nbytes, sizeof(unsigned char))) == NULL)
      syserr("alloc_short_image","calloc","sdata");
  return(sdata);
}

/************************************************************/
/*         Routine:   alloc_char_image()                    */
/*         Author:    Darrin L. Dimmick                     */
/*         Date:      9/02/93 added to library              */
/*         Modifications:                                   */
/************************************************************/
/************************************************************/
/* Alloc_char_image() takes the pixel width, height of an   */
/* image and uses SizeFromDepth to calloc the requested     */
/* memory. Returning a pointer to an unsigned char.         */
/************************************************************/
unsigned char *alloc_char_image( int width,int  height)
{
  int nbytes = 0;
  unsigned char *cdata = NULL;

   nbytes = SizeFromDepth(width, height, CHAR_DEPTH);
   if((cdata = (unsigned char *)calloc(nbytes, sizeof(unsigned char))) == NULL)
      syserr("alloc_char_image","calloc","cdata");
  return(cdata);
}

/************************************************************/
/*         Routine:   alloc_int_image()                     */
/*         Author:    Darrin L. Dimmick                     */
/*         Date:      9/02/93 added to library              */
/*         Modifications:                                   */
/************************************************************/
/************************************************************/
/* Alloc_int_image() takes the pixel width, height of an    */
/* image and uses SizeFromDepth to calloc the requested     */
/* memory. Returning a pointer to an int.                   */
/************************************************************/
int *alloc_int_image( int width,int  height)
{
   int *image = NULL;

   if ((image = (int *)calloc(width*height, sizeof(int))) == NULL)
      syserr("alloc_int_image", "calloc", "image");
   return(image);
}

/************************************************************/
/*         Routine:   Allocate_aligned_image()              */
/*         Author:    Darrin L. Dimmick                     */
/*         Date:      8/04/93                               */
/*         Modifications:                                   */
/************************************************************/
/************************************************************/
/* Allocate_aligned_image() takes the pixel width, height   */
/* and depth of an image sets the width to a multiple of 16 */
/* and uses SizeFromDepth to calloc the requested memory    */
/************************************************************/

int allocate_aligned_image( unsigned char **adata, int *awidth,
		int  width,int  height,int  depth)
{
  int aligned_width, aligned_length, width16;
  unsigned char *image = NULL, *aligned_image = NULL;
  char num_str[12];

  if (width < 1) {
     sprintf(num_str,"%d",width);
     fatalerr("allocate_aligned_image","Invalid width", num_str);
  }
  if (height < 1) {
     sprintf(num_str,"%d",height);
     fatalerr("allocate_aligned_image","Invalid height", num_str);
  }
  if (depth < 1) {
     sprintf(num_str,"%d",depth);
     fatalerr("allocate_aligned_image","Invalid depth", num_str);
  }
  width16 = width_16(width);
  if ((image = (unsigned char *)calloc(SizeFromDepth(width16, height, depth),
                                       sizeof(unsigned char))) == NULL)
      syserr("allocate_aligned_image","calloc","image");
  if (WordAlignImage(&aligned_image, &aligned_width, &aligned_length,
		 image, width16, height, depth)){
     *adata = aligned_image;
     *awidth = aligned_width;
     free(image);
  }
  else{
     *adata = image;
     *awidth = width16;
     aligned_length = SizeFromDepth(width16, height, depth);
  }
  return(aligned_length);
}

/************************************************************/
/*         Routine:   Width_16()                            */
/*         Author:    Darrin L. Dimmick                     */
/*         Date:      8/04/93                               */
/*         Modifications:                                   */
/************************************************************/
/************************************************************/
/* Width_16()  calculate a width which is a multiple of 16  */
/* at least equal to the supplied width.                    */  
/************************************************************/

int width_16(int width)
{
  int width16;
  double curwords = 0, newwords = 0;

  if (width%16){
     curwords = (double)width/16;
     newwords = ceil(curwords);
     width16 = (int)newwords * 16;
  }
  else
     width16 = width;
  return(width16);
}
