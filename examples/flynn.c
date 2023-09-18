#include "srgp.h"
#include "geom.h"
#include <stdio.h>

/*
 * rasterop.c: PJF 11/16/90
 *
 * fill a window with random colors, allow the user to select a subregion, and
 * track the mouse by rasteropping the subregion along with it.
 *
 */

int window_width=100,window_height=100;

int windowDidResize(int,int);

void acquire_rop_image();
canvasID rop_canvas,backing_canvas;
int w,h;
point old_location;

main(argc,argv)
int argc;
char *argv[];
{
   unsigned short r[64],g[64],b[64];
   int i;

   /* set up 4x4x4 color cube */
   r[0]=g[0]=b[0]=65535;
   for(i=1;i<64;i++) {
     r[i] = (i/16)*64*256;
     g[i] = ((i&15)/4)*64*256;
     b[i] = (i&3)*64*256;
     };
   
   SRGP_begin("rasterop",window_width,window_height,6,FALSE);
   SRGP_loadColorTable(0,64,r,g,b);

   SRGP_allowResize(TRUE);
   SRGP_registerResizeCallback(windowDidResize);
   SRGP_setKeyboardProcessingMode(RAW);
   SRGP_setInputMode(KEYBOARD,EVENT);

   windowDidResize(window_width,window_height);


   /*now, follow the mouse, rop-ing the selected subregion run this in
     event mode, so that we can catch the 'q'-for-quit keypress */
   
   while(TRUE) {
     inputDevice code=SRGP_waitEvent(1);
     locator_measure m;
     if (code == KEYBOARD) {
       char theKey;
       SRGP_getKeyboard(&theKey,2);
       if ((theKey == 'q')||(theKey == 'Q')) {
         SRGP_end();
         exit(0);
         }
       };
     /* do the rop */
     SRGP_sampleLocator(&m);
     if ((old_location.x!=m.position.x)||(old_location.y!=m.position.y)) {
       rectangle oldr=SRGP_defRectangle(old_location.x,old_location.y,
                                        old_location.x+w-1,old_location.y+h-1);
       rectangle newr=SRGP_defRectangle(m.position.x,m.position.y,
                                        m.position.x+w-1,m.position.y+h-1);
       SRGP_useCanvas(0);
       SRGP_copyPixel(backing_canvas,SRGP_defRectangle(0,0,w-1,h-1),
                      old_location);
       SRGP_useCanvas(backing_canvas);
       SRGP_copyPixel(0,newr,SRGP_defPoint(0,0));
       SRGP_useCanvas(0);
       SRGP_copyPixel(rop_canvas,SRGP_defRectangle(0,0,w-1,h-1),m.position);
       old_location=m.position;
       SRGP_refresh();
       };
     };
}

int windowDidResize(new_w,new_h)
  int new_w, new_h;
  {
    int i,j;
    rectangle new_clip_rect;
    /* update the canvases' clip rectangle */
    new_clip_rect=SRGP_defRectangle(0,0,new_w-1,new_h-1);
    SRGP_setClipRectangle(new_clip_rect);
    window_width=new_w;
    window_height=new_h;
    /* fill the window with garbage */
    srandom(time(0));
    for(i=0;i<new_w;i+=16)
      for(j=0;j<new_h;j+=16) {
        int color=random()&63;
        rectangle r=SRGP_defRectangle(i,j,i+15,j+15);
        SRGP_setColor(color);
        SRGP_fillRectangle(r);
        };
    SRGP_refresh();
    acquire_rop_image();
  } 

void acquire_rop_image()
  {
     point anchor,corner;
     rectangle source_rect;
     char flag=FALSE;

     /* rubberband a rectangle */
     SRGP_setColor(0);
     SRGP_setInputMode(LOCATOR,SAMPLE);
     /* loop until the left mouse button goes down */
     while (!flag) {
       locator_measure m;
       SRGP_sampleLocator(&m);
       if (m.button_chord[LEFT_BUTTON]==DOWN) {
         flag=TRUE;
         anchor=m.position;
         };
       };
     SRGP_setLocatorEchoRubberAnchor(anchor);
     SRGP_setLocatorEchoType(RUBBER_RECT);
     while (flag) {
       locator_measure m;
       SRGP_sampleLocator(&m);
       if (m.button_chord[LEFT_BUTTON]==UP) {
         flag=FALSE;
         corner = m.position;
         };
       };
     SRGP_setLocatorEchoType(CURSOR);
     source_rect=GEOM_rectFromDiagPoints(anchor,corner);
     old_location=source_rect.bottom_left;
     SRGP_setLocatorMeasure(old_location);
     w=source_rect.top_right.x-source_rect.bottom_left.x;
     h=source_rect.top_right.y-source_rect.bottom_left.y;
     /* allocate canvases */
     rop_canvas=SRGP_createCanvas(w,h);
     backing_canvas=SRGP_createCanvas(w,h);
     /* rop the portion of the screen into rop_canvas */
     SRGP_useCanvas(rop_canvas);
     SRGP_copyPixel(0,source_rect,SRGP_defPoint(0,0));
     /* and into backing_canvas */
     SRGP_useCanvas(backing_canvas);
     SRGP_copyPixel(0,source_rect,SRGP_defPoint(0,0));
  }


