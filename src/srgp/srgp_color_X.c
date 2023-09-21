#include "HEADERS.h"
#include "srgplocal.h"

/** THIS FILE IS FOR X11 IMPLEMENTATION ONLY
**/

/* The color pallette needs to be global. */
static unsigned long colorPallette[65536];

/* This is the fake colormap since I'm using 16-bit truecolor. */
static struct
{
  unsigned short red;
  unsigned short green;
  unsigned short blue;
} fakeColormap[65536];

/*****************************************************************************

  Name: convertRgbTo16Bit

  Purpose: The purpose of this function is to convert an rgb triple into
  a value that is suitable for 16-bit displays.

  Note: For a 16-bit display, the partitioning of the color values
  are as follows (with r = red, g = green, blue = blue):

  r[4:0], g[5:0], blue[4:0] = rrrrr gggggg bbbbb = 16 bits

  Calling Sequence: pixel = convertRgbTo16Bit(red,green,blue)

  Inputs:

    red - The red contribution [0,65535].

    green - The green contribution [0,65535].

    blue - The blue contribution [0,65535].

  Outputs:

    pixel = The 16-bit pixel representation of the rgb value.

*****************************************************************************/
static unsigned long convertRgbTo16Bit(
  unsigned short red,
  unsigned short green,
  unsigned short blue)
{
  unsigned short pixel;
  unsigned short r, g, b;

  /* Map to [0,255]. */
  red /= 256;
  green /= 256;
  blue /= 256;

  /* /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/ */
  // Form the quantized values.
  /* /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/ */
  // Quantize to 5 bits.
  r = red / 8;

  /* Quantize to 6 bits. */
  g = green / 4;

  /* Quantize to 5 bits. */
  b = blue / 8;
  /* /_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/ */

  /* Construct the 16-bit value. */
  pixel = (r << 11) | (g << 5) | b;
  
  return (pixel);

} /* convertRgbTo16Bit */

/*****************************************************************************

  Name: srgp_storeColor

  Purpose: The purpose of this function is to store the 16-bit
  representation of a color into the color pallette.

  Calling Sequence: srgp_storeColor(char colorName,int palletteIndex)

  Inputs:

    colorName - A textual name for the color.

    colorIndex - The index, into the pallette, for which to store the
    16-bit representation of the color.

  Outputs:

    None.

*****************************************************************************/
static void srgp_storeColor(char *colorName,int palletteIndex)
{
  XColor exact;
  XColor closest;

  /* Retrieve the color attributes by name. */
  XLookupColor(srgpx__display,srgpx__colormap,colorName,&exact,&closest);

  /* Store values in our "color table". */
  fakeColormap[palletteIndex].red = exact.red;
  fakeColormap[palletteIndex].green = exact.green;
  fakeColormap[palletteIndex].blue = exact.blue;

  /* Store 16-bit mapping in the pallette. */
  colorPallette[palletteIndex] =
    convertRgbTo16Bit(exact.red,exact.green,exact.blue);

  return;

} /* srgp_storeColor */

/*****************************************************************************

  Name: SRGP__retrieveColorFromPallette

  Purpose: The purpose of this function is to store the 16-bit
  representation of a color into the color pallette.

  Calling Sequence: value = SRGP__retrieveColorFromPallette(colorIndex)

  Inputs:

    colorIndex - The index, into the pallette, for which to retrieve the
    16-bit representation of the color.

  Outputs:

    value = The 16-bit representation of the rgb value.

*****************************************************************************/
unsigned long SRGP__retrieveColorFromPallette(int colorIndex)
{
  unsigned long value;

  /* Default to black. */
  value = 0;

  if ((colorIndex >= 0) && (colorIndex <= 65535))
  {
    /* We have a valid index. */
    value = colorPallette[colorIndex];
  } /* if */

  return (value);

} /* SRGP__retrieveColorFromPallette */

//******************************************************************
//******************************************************************
void 
SRGP__initColor (requested_planes)
{

   srgp__visual_class = DefaultVisual(srgpx__display, srgpx__screen)->class;
   srgp__available_depth = DefaultDepth(srgpx__display, srgpx__screen);

   SRGP_BLACK = 1;
   SRGP_WHITE = 0;

   if (requested_planes < 0)
   {
      fprintf(stderr, "Fatal Error: insane parameter to SRGP_begin()\n\
              Application requesting negative number of planes.\n");
      exit(1);
   }

   if ((requested_planes == 0) || 
       (requested_planes > srgp__available_depth))
   {
      srgp__application_depth = srgp__available_depth;
   } // if
   else
   {
      srgp__application_depth = requested_planes;
   } // else

   srgp__max_pixel_value = (1 << srgp__application_depth) - 1;

   srgpx__colormap = DefaultColormap(srgpx__display,srgpx__screen);

   srgp__base_colorindex = 0;

   /* Only first two entries of LUT are init'd */
   srgp_storeColor("white",COLORINDEX(0));
   srgp_storeColor("black",COLORINDEX(1));

   /*** DONE FOR ALL CONFIGURATIONS. */
   XSetWindowBackground (srgpx__display, 
			 srgp__curActiveCanvasSpec.drawable.win,
                         XWHITE);
   XSetWindowBorder (srgpx__display, 
		     srgp__curActiveCanvasSpec.drawable.win,
                     XBLACK);
}



void SRGP_loadColorTable
   (int startentry, int count,
    unsigned short *redi, 
    unsigned short *greeni,
    unsigned short *bluei)
{
   register int i, j;
   int endi;

   /* LEAVE IMMEDIATELY IF EXECUTING ON BILEVEL DISPLAY */
   if (srgp__available_depth == 1 || srgp__visual_class == StaticGray)
      return;

   endi = startentry + count;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_loadColorTable  %d  %d  %x %x %x\n",
		  startentry, count, redi, greeni, bluei);

      /* PERFORM CHECKING LEGALITY OF THE RANGE OF INDICES. */
      srgp_check_pixel_value (startentry, "start");
      srgp_check_pixel_value (endi-1, "end");
   }

   /* COPY INTENSITY VALUES INTO ARRAY. */
   for (i = 0, j = startentry; i < count; i++, j++)
   {
      /* Update the colormap. */
      fakeColormap[j].red = redi[i];
      fakeColormap[j].green = greeni[i];
      fakeColormap[j].blue = bluei[i];

      /* Update the pallette. */
      colorPallette[j] = convertRgbTo16Bit(redi[i],greeni[i],bluei[i]);
   }

}




void
SRGP_inquireColorTable 
   (int startentry, int count,
    unsigned short *redi, 
    unsigned short *greeni,
    unsigned short *bluei)
{
   register int i, j;
   int endi;

   /* LEAVE IMMEDIATELY IF EXECUTING ON BILEVEL DISPLAY */
   if (srgp__available_depth == 1 || srgp__visual_class == StaticGray)
      return;

   endi = startentry + count;

   DEBUG_AIDS{
      /* PERFORM CHECKING LEGALITY OF THE RANGE OF INDICES. */
      srgp_check_pixel_value (startentry, "start");
      srgp_check_pixel_value (endi-1, "end");
   }

   /* COPY INTENSITY VALUES INTO USER'S ARRAY. */
   for (i = 0, j = startentry; i < count; i++, j++)
   {
      redi[i] = fakeColormap[j].red;
      greeni[i] = fakeColormap[j].green;
      bluei[i] = fakeColormap[j].blue;
   }
}


void
SRGP_loadCommonColor (entry, name)
int entry;
char *name;   /* Null-terminated string of characters */
{
   /* IGNORE IF MONOCHROME */
   if (srgp__available_depth == 1 || srgp__visual_class == StaticGray)
      return;

   DEBUG_AIDS{
      SRGP_trace (SRGP_logStream, "SRGP_loadCommonColor  %d  %s\n", entry, name);
      srgp_check_pixel_value (entry, "start/end");
   }

   /* Store the color in our fake colormap. */
   srgp_storeColor(name,COLORINDEX(entry));

}
