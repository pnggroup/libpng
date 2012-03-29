             {
                 int
                   icheck;

                 /* 0: not a known sRGB profile
                  * 1: HP-Microsoft sRGB v2
                  * 2: ICC sRGB v4 perceptual
                  * 3: ICC sRGB v2 perceptual no black-compensation
                  */
                 png_uint_32
                   check_crc[4] = {0, 0xf29e526dUL, 0xbbef7812UL, 0x427ebb21UL},
                   check_len[4] = {0, 3144, 60960, 3052};

                 png_uint_32
                   length,
                   profile_crc;

                 unsigned char
                   *data;

                 length=(png_uint_32) ...;

                 for (icheck=3; icheck > 0; icheck--)
                 {
                   if (length == check_len[icheck])
                   {
                     data=...(profile);
                     profile_crc=crc32(0,data,length);

                     if (profile_crc == check_crc[icheck])
                     {
                        /* set sRGB.... */
                        break;
                     }
                   }
                 }
              }
