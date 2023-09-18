#include "srgp.h"
#include <stdio.h>

static unsigned short *redd, *greenn, *bluee;
static int width=500, height=500;
static int ncolors;



void Draw()
{
   register i;

   SRGP_setPlaneMask (ncolors-1);

   SRGP_setLineWidth (1);
   for (i=0; i<height; i++) {
      SRGP_setColor (i & (ncolors-1));
      SRGP_lineCoord (0,i*2, width,i*2);
   }

   i = (3<<5) | 7;
   SRGP_setPlaneMask (i);
   SRGP_setLineWidth (10);
   SRGP_setColor (0);
   SRGP_lineCoord (width>>1, 0, width>>1, height);
   SRGP_lineCoord (0, height>>1, width, height>>1);
}


int DealWithResizeEvent (w,h)
{
   width = w;
   height = h;
   SRGP_setClipRectangle (SRGP_defRectangle(0,0,w-1,h-1));
   Draw();
}


main()
{
   register i;
   unsigned short maxpixval = (unsigned short) -1;


   SRGP_beginWithDebug ("Test of resizing", width, height, 8, FALSE);
   SRGP_enableBlockedWait();
   SRGP_allowResize (TRUE);
   SRGP_registerResizeCallback (DealWithResizeEvent);

   ncolors = 1 << SRGP_inquireCanvasDepth();
   redd = (unsigned short *) malloc (ncolors * sizeof(unsigned short));
   greenn = (unsigned short *) malloc (ncolors * sizeof(unsigned short));
   bluee = (unsigned short *) malloc (ncolors * sizeof(unsigned short));
   
   for (i=0; i < ncolors; i++) {
      redd[i] = maxpixval * (i*1.0/ncolors);
      greenn[i] = (maxpixval/2.0) * (i*1.0/ncolors);
      bluee[i] = maxpixval - redd[i];
   }
   SRGP_loadColorTable (0, ncolors, redd+2, greenn+2, bluee+2);

   Draw();

   SRGP_setInputMode (LOCATOR, EVENT);
   while (1) {
      SRGP_waitEvent(-1);
      SRGP_changeScreenCanvasSize (100,100);
   }
}
