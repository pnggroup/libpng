
#include <stdio.h>
convert(char * name)
{
  unsigned long number;
  number=name[3]+name[2]*256+name[1]*256*256+name[0]*256*256*256;
  printf(" #define PNG_%s 0x%xL\n",name, number);
}
main()
{
   convert("AAAA");
   convert("IDAT");
   convert("IEND");
   convert("IHDR");
   convert("PLTE");
   convert("bKGD");
   convert("cHRM");
   convert("gAMA");
   convert("hIST");
   convert("iCCP");
   convert("iTXt");
   convert("oFFs");
   convert("pCAL");
   convert("pHYs");
   convert("sBIT");
   convert("sCAL");
   convert("sPLT");
   convert("sRGB");
   convert("tEXt");
   convert("tIME");
   convert("tRNS");
   convert("zTXt");
   convert("zzzz");
}

