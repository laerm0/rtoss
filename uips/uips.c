/*
 * UNIVERSAL IPS PATCHER VERSION 1.00
 * Copyright 2003 Steve Nickolas, Pathologic Software.
 * Based on IPS Plus by $CePTiC.  Rewritten in C for portability.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>

/* Forward declarations */
void getrle(void);
void closefiles(void);

#define ADDRESSBYTES 3
#define MAXARRAYSIZE 50000UL
char sourcename[1024], patchname[1024];
FILE *patchfile, *sourcefile, *f;
unsigned char header[5],
	      addarray[3],
	      rlearray[3],
	      bytesarray[2],
	      chunkarray[MAXARRAYSIZE];
unsigned long address,numbytes,rlebytes;
unsigned char rlebyte;
unsigned long count;
int totalchunks;
unsigned long a1,a2,a3;
int overload;

void isitanips(void)
{
 int checksum;

 fread(header,sizeof(header),1,patchfile);
 if (header[0]!='P' ||
     header[1]!='A' ||
     header[2]!='T' ||
     header[3]!='C' ||
     header[4]!='H')
 {
  fprintf (stderr,"Invalid IPS format\n");
  exit(-1);
 }
}

void getaddress(void)
{
 fread(addarray,3,1,patchfile);
 if ((addarray[0]=='E')&&
     (addarray[1]=='O')&&
     (addarray[2]=='F'))    closefiles();
 totalchunks++;
 address=addarray[2]+(addarray[1]*256UL)+(addarray[0]*65536UL);
}

void getnumbytes(void)
{
 unsigned long b1,b2;

 fread(bytesarray,2,1,patchfile);
 b1=bytesarray[0];
 b2=bytesarray[1];
 numbytes=b2+(b1*256UL);
 if (!numbytes) getrle();
 if (numbytes>MAXARRAYSIZE)
 {
  printf ("Debug: numbytes=%lu\n",numbytes);
  overload=1;
  closefiles();
 }
}

void getchunk(void)
{
 fread(chunkarray,numbytes,1,patchfile);
}

void getrle(void)
{
 unsigned long r1,r2;

 fread(rlearray,3,1,patchfile);
 r1=rlearray[0];
 r2=rlearray[1];
 rlebytes=r2+(r1*256UL);
 rlebyte=rlearray[2];
}

void applypatch(void)
{
 fseek(sourcefile,address,SEEK_SET);
 fwrite(chunkarray,numbytes,1,sourcefile);
}

void applyrle(void)
{
 fseek(sourcefile,address,SEEK_SET);
 if (rlebytes>MAXARRAYSIZE)
 {
  for (count=0; count<MAXARRAYSIZE; count++) chunkarray[count]=rlebyte;
  fwrite (chunkarray,MAXARRAYSIZE,1,sourcefile);
  rlebytes-=MAXARRAYSIZE;
 }
 for (count=0;count<rlebytes;count++) chunkarray[count]=rlebyte;
 fwrite (chunkarray,rlebytes,1,sourcefile);
}

void closefiles(void)
{
 fclose(sourcefile);
 fclose(patchfile);
 if (overload)
 {
  printf (
"The patch file has requested a patch size greater than the\n"
"maximum patch size this version can handle.  The maximum\n"
"patch size is currently set to: %u\n",MAXARRAYSIZE);
 }
 exit(overload);
}

int main (int argc, char **argv)
{
 fprintf (stderr,"Universal IPS Patcher - Copyright 2003 Steve Nickolas\n");

 if (argc!=3)
 {
  fprintf (stderr,"usage: uips source.ext patch.ips\n");
  return -1;
 }
 sourcefile=fopen(argv[1],"r+b");
 if (!sourcefile)
 {
  fprintf (stderr,"Source file ");
  perror(argv[1]);
  return -1;
 }
 patchfile=fopen(argv[2],"rb");
 if (!patchfile)
 {
  fprintf (stderr,"Patch file ");
  perror(argv[2]);
  return -1;
 }
 isitanips();
 while (!feof(patchfile))
 {
  getaddress();
  getnumbytes();
  if (numbytes) getchunk();
  if (numbytes) applypatch(); else applyrle();
 }
 closefiles();
 /* avert warning */
 return 40;
}
