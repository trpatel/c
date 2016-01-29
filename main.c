/* part of final project
//Authors: Tej Patel and Grace Li
//Created 12-7-2014
//Modified: Tej patel and Grace Li
//Updated: 12-11-2014
*/


#include <stm32f30x.h> // Pull in include files for F30x standard drivers
#include <f3d_led.h>
#include <f3d_usr_btn.h>
#include <f3d_uart.h>
#include <f3d_gyro.h>
#include <f3d_lcd_sd.h>
#include <f3d_i2c.h>
#include <f3d_accel.h>
#include <f3d_mag.h>
#include <f3d_nunchuk.h>
#include <f3d_rtc.h>
#include <ff.h>
#include <diskio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <bmp.h>

#define TIMER 20000
#define AUDIOBUFSIZE 128

extern uint8_t Audiobuf[AUDIOBUFSIZE];
extern int audioplayerHalf;
extern int audioplayerWhole;

FATFS Fatfs;/* File system object */
FIL fid;/* File object */
BYTE Buf[512];/* File read buffer */
int ret;

struct ckhd {
  uint32_t ckID;
  uint32_t cksize;
};

struct fmtck {
  uint16_t wFormatTag;      
  uint16_t nChannels;
  uint32_t nSamplesPerSec;
  uint32_t nAvgBytesPerSec;
  uint16_t nBlockAlign;
  uint16_t wBitsPerSample;
};

void readckhd(FIL *fid, struct ckhd *hd, uint32_t ckID) {
  f_read(fid, hd, sizeof(struct ckhd), &ret);
  if (ret != sizeof(struct ckhd))
    exit(-1);
  if (ckID && (ckID != hd->ckID))
    exit(-1);
}

void die (FRESULT rc) {
  printf("Failed with rc=%u.\n", rc);
  while (1);
}

//FATFS Fatfs;/* File system object */
FIL Fil;/* File object */
BYTE Buff[128];/* File read buffer */

struct bmpfile_magic magic;
struct bmpfile_header header;
BITMAPINFOHEADER info;

int main(void){
  char footer[20];
  int i, j;
  FRESULT rc;/* Result code */
  DIR dir;/* Directory object */
  FILINFO fno;/* File information object */
  UINT bw, br;
  unsigned int retval;
  int bytesread;
  int counter = 0;
  int count = 0;
  int score = 0;
  char *l;

  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  f3d_uart_init();
  f3d_lcd_init();
  f3d_usr_btn_init();
  f3d_timer2_init();
  f3d_dac_init();
  f3d_delay_init();
  f3d_led_init();
  f3d_rtc_init();
  f3d_i2c1_init();
  delay(10);
  f3d_accel_init();
  delay(10);
  f3d_nunchuk_init();
  delay(10);
  nunchuk_t n;
  int flag = 0;
  int r,t,s,w,a,b,d;
  int help = 1;
  r = 127;
  t = 107;
  s = 87;
  w = 67;
  f_mount(0, &Fatfs);
  rc = f_open(&Fil, "BS.BMP", FA_READ);
  if (rc) die(rc);
  rc = f_read(&Fil, &magic , 2, &br);
  if (rc) die(rc);   
  rc = f_read(&Fil,  &header , sizeof header, &br);
  if (rc) die(rc);   
  rc = f_read(&Fil,  &info , sizeof info, &br);
  if (rc) die(rc);   
  struct bmppixel bp[128];   
  for(i = 159; i >0; i--){
    rc = f_read(&Fil, bp , sizeof bp, &br); 
    if(rc) die(rc);
    for(j = 0; j <127; j++){ 
      uint16_t color = ((uint16_t)bp[j].r>>3)<<11 | ((uint16_t)bp[j].g>>2)<<5 | bp[j].b>>3;
      f3d_lcd_drawPixel(j,i,color);
    }
  }
  rc = f_close(&Fil);
  if (rc) die(rc); 
  rc = disk_ioctl(0,GET_SECTOR_COUNT,&retval);
  while(1){
    f3d_nunchuk_read(&n);
    //    delay(10);
    int joy = n.jx;
    int zbut = n.z;
    int cbut = n.c;
    flag++;
    flag%=1;
    if(zbut == 1){
      counter = counter + 1;
      score = score +1;
    }
    if(cbut == 1){
      counter = counter -1;
      score = score + 1;
    }
    if(counter > 1){
      counter = 0;
    }
    if(counter < 0){
      counter = 1;
    }
    rc = f_open(&Fil, "BS.BMP", FA_READ);
    if (rc) die(rc);
    rc = f_read(&Fil, &magic , 2, &br);
    if (rc) die(rc);   
    rc = f_read(&Fil,  &header , sizeof header, &br);
    if (rc) die(rc);   
    rc = f_read(&Fil,  &info , sizeof info, &br);
    if (rc) die(rc);   
    struct bmppixel bp[128];   
    for(i = 130; i >108; i--){
      rc = f_read(&Fil, bp , sizeof bp, &br); 
      if(rc) die(rc);
      for(j = 0; j <127; j++){ 
	uint16_t color = ((uint16_t)bp[j].r>>3)<<11 | ((uint16_t)bp[j].g>>2)<<5 | bp[j].b>>3;
	f3d_lcd_drawPixel(j,i,color);
      }
    }
    for(i = 61; i >39; i--){
      rc = f_read(&Fil, bp , sizeof bp, &br); 
      if(rc) die(rc);
      for(j = 0; j <127; j++){ 
	uint16_t color = ((uint16_t)bp[j].r>>3)<<11 | ((uint16_t)bp[j].g>>2)<<5 | bp[j].b>>3;
	f3d_lcd_drawPixel(j,i,color);
      }
    }
    
    rc = f_close(&Fil);
    if (rc) die(rc); 
    rc = disk_ioctl(0,GET_SECTOR_COUNT,&retval);
    int z, y;
    if(t < 8){
      r = 127;
      t = 107;
    }
    for(z = 129; z > 109; z--){
      for(y = r; y > t; y--){
	f3d_lcd_drawPixel(y,z,BLACK);
      }
    }
    if(counter == 1){
      f3d_lcd_drawChar(60, 50,'O',RED,YELLOW);
      help = 0;
      if(s > 59 && w < 61){
	f3d_lcd_drawString(50, 50,"UMADBRO?!",RED,YELLOW);
	//////////////////////////////////sound//////////////////////////
	//  f_mount(0, &Fatfs);
	rc = f_open(&fid, "3.wav", FA_READ);
	if(!rc){
	  f3d_led_all_on();
	  struct ckhd hd;
	  uint32_t waveid;
	  struct fmtck fck;
	  readckhd(&fid, &hd, 'FFIR');
	  rc = f_read(&fid, &waveid, sizeof(waveid), &ret);
	  if ((ret != sizeof(waveid)) || (waveid != 'EVAW'))
	    return -1;
	  readckhd(&fid, &hd, ' tmf');
	  f_read(&fid, &fck, sizeof(fck), &ret);
	  if (hd.cksize != 16) {
	    f_lseek(&fid, hd.cksize - 16);
	  }
	  while(1){
	    readckhd(&fid, &hd, 0);
	    if (hd.ckID == 'atad')
	      break;
	    f_lseek(&fid, hd.cksize);
	  }
	  f_read(&fid, Audiobuf, AUDIOBUFSIZE, &ret);
	  f3d_led_all_off();
	  hd.cksize -= ret;
	  audioplayerStart();
	  while (hd.cksize) {
	    f3d_led_all_on();
	    int next = hd.cksize > AUDIOBUFSIZE/2 ? AUDIOBUFSIZE/2 : hd.cksize;
	    if (audioplayerHalf) {
	      if (next < AUDIOBUFSIZE/2)
		bzero(Audiobuf, AUDIOBUFSIZE/2);
	      f_read(&fid, Audiobuf, next, &ret);
	      hd.cksize -= ret;
	      audioplayerHalf = 0;
	    }
	    if (audioplayerWhole) {
	      if (next < AUDIOBUFSIZE/2)
		bzero(&Audiobuf[AUDIOBUFSIZE/2], AUDIOBUFSIZE/2);
	      f_read(&fid, &Audiobuf[AUDIOBUFSIZE/2], next, &ret);
	      hd.cksize -= ret;
	      audioplayerWhole = 0;
	    }
	    f3d_led_all_off();
	  }
	  audioplayerStop();
	}
	f3d_led_all_off();
	printf("\nClose the file.\n"); 
	rc = f_close(&fid);
	if (rc) die(rc);
	f3d_led_on(10);
	///////////////////////////////////////////////////////////////////
	while(1);
      }
    }
    r = r - 15;
    t = t - 15;
    if(w < 8){
      s = 127;
      w = 107;
    }
    for(z = 60; z > 40; z--){
      for(y = s; y > w; y--){
	f3d_lcd_drawPixel(y,z,BLACK);
      }
    }
    if(counter == 0){
      f3d_lcd_drawChar(60, 119,'O',RED,YELLOW);
      if(r > 59 && t < 61){
	f3d_lcd_drawString(50, 119,"UMADBRO?!",RED,YELLOW);
	//f3d_lcd_drawString(50, 80,"Score: ",RED,YELLOW);
	//l = (char *)score;
	//f3d_lcd_drawString(60, 90,l,RED,YELLOW);
	//////////////////////////////////sound//////////////////////////
	//  f_mount(0, &Fatfs);
	rc = f_open(&fid, "3.wav", FA_READ);
	if(!rc){
	  f3d_led_all_on();
	  struct ckhd hd;
	  uint32_t waveid;
	  struct fmtck fck;
	  readckhd(&fid, &hd, 'FFIR');
	  rc = f_read(&fid, &waveid, sizeof(waveid), &ret);
	  if ((ret != sizeof(waveid)) || (waveid != 'EVAW'))
	    return -1;
	  readckhd(&fid, &hd, ' tmf');
	  f_read(&fid, &fck, sizeof(fck), &ret);
	  if (hd.cksize != 16) {
	    f_lseek(&fid, hd.cksize - 16);
	  }
	  while(1){
	    readckhd(&fid, &hd, 0);
	    if (hd.ckID == 'atad')
	      break;
	    f_lseek(&fid, hd.cksize);
	  }
	  f_read(&fid, Audiobuf, AUDIOBUFSIZE, &ret);
	  f3d_led_all_off();
	  hd.cksize -= ret;
	  audioplayerStart();
	  while (hd.cksize) {
	    f3d_led_all_on();
	    int next = hd.cksize > AUDIOBUFSIZE/2 ? AUDIOBUFSIZE/2 : hd.cksize;
	    if (audioplayerHalf) {
	      if (next < AUDIOBUFSIZE/2)
		bzero(Audiobuf, AUDIOBUFSIZE/2);
	      f_read(&fid, Audiobuf, next, &ret);
	      hd.cksize -= ret;
	      audioplayerHalf = 0;
	    }
	    if (audioplayerWhole) {
	      if (next < AUDIOBUFSIZE/2)
		bzero(&Audiobuf[AUDIOBUFSIZE/2], AUDIOBUFSIZE/2);
	      f_read(&fid, &Audiobuf[AUDIOBUFSIZE/2], next, &ret);
	      hd.cksize -= ret;
	      audioplayerWhole = 0;
	    }
	    f3d_led_all_off();
	  }
	  audioplayerStop();
	}
	f3d_led_all_off();
	printf("\nClose the file.\n"); 
	rc = f_close(&fid);
	if (rc) die(rc);
	f3d_led_on(10);
	///////////////////////////////////////////////////////////////////
	while(1);
      }
    }
    s = s - 25;
    w = w - 25;
  }
 
  //   }
  /*   if(counter == 1){
      f3d_lcd_drawChar(27, 60, '>', BLUE, BLUE);
      f3d_lcd_drawChar(27, 80, '>', BLACK, BLUE);
      if(button_read()){
      while(button_read()){
        count = 0;
	  count = count + 1;
	      delay();
	            if(count > 2){
		              count = 0;
			          }
				      }
				          printf("%d\n",count);
      }
      if(count == 1){
      f3d_lcd_fillScreen(BLUE);
      f3d_lcd_drawString(15, 10, "To play this game, you will have to avoid the rocks and thunder bolts that will randomly appear as the game progresses. When encountering a rock press the 'c' button. And when encountering a lightning bolt press 'z' button.", WHITE, BLUE);
      f3d_lcd_drawString(5,130,"To Start, Press 'z'",WHITE,BLUE);
      if(n.z == 1){
      counter = 0;
}
      while(1);
}   
}
}*/
}


#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  /* Infinite loop */
  /* Use GDB to find out why we're here */
  while (1);
}
#endif
