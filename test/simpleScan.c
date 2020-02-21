#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "evio.h"
#include "simpleLib.h"

int
main(int argc, char **argv)
{
  int verbose = 1;
  int handle;
  uint32_t *buf, bufLen, nevents = 0, status = 0;
  char filename[128] = "/home/moffit/tmp/vtpCompton_854.dat.0";

  simpleConfigSetDebug(0xffff);
  /* Open file  */
  if((status = evOpen((char *)filename,
		      (char *) "r", &handle)) < 0)
    {
      printf("Unable to open file %s status = %d\n", filename, status);
      exit(-1);
    }
  else
    {
      printf("Opened %s for reading (status = 0x%x, handle = %d)\n\n",
	     filename, status, handle);
    }

  int version = 0;
  /* Get evio version # of file */
  status = evIoctl(handle, (char*)"v", &version);
  if (status == S_SUCCESS)
    {
      printf("Evio file version = %d\n\n", version);
    }
  else
    {
      printf("status = 0x%x\n", status);
    }


  while((status = evReadAlloc(handle, &buf, &bufLen)) == 0)
    {				/* read the event and allocate the correct size buffer */
      uint32_t nWords = 0, bt = 0, dt = 0, blk = 0;
      int pe = 0;
      nWords = buf[0] + 1;
      bt = ((buf[1] & 0xffff0000) >> 16);	/* Bank Tag */
      dt = ((buf[1] & 0xff00) >> 8);	/* Data Type */
      blk = buf[1] & 0xff;	/* Event Block size */

      nevents++;

      if(nevents > 3)
	break;

      if(verbose)
	printf("    BLOCK #%d,  Bank tag = 0x%04x, Data type = 0x%04x,  Total len = %d words\n",
	       nevents, bt, dt, nWords);

      /* Check on what type of event block this is */
      if((bt >= 0xff00)> 0)
	{/* CODA Reserved bank type */
	  switch (bt) {
	  case 0xffd1:
	    if(verbose) printf("    ** Prestart Event **\n");
	    break;
	  case 0xffd2:
	    if(verbose) printf("    ** Go Event **\n");
	    break;
	  case 0xffd4:
	    if(verbose) printf("    ** End Event **\n");
	    break;
	  case 0xff50:
	  case 0xff70:
	    if(verbose) printf("    ** Physics Event Block (%d events in Block) **\n",blk);
	    pe=1;
	    break;
	  default:
	    if(verbose) printf("    ** Undefined CODA Event Type **\n");
	  }
	}
      else
	{ /* User event type */
	  printf("    ** User Event (Type = %d) **\n",bt);
	}

      if(pe)
	{
	  simpleScanCodaEvent(buf);
	}
    }

  if ( status == EOF )
    {
      printf("Found end-of-file; total %d events. \n", nevents);
    }
  else if(status != 0)
    {
      printf("Error reading file (status = 0x%x, quit)\n",status);
    }


  return 0;
}
