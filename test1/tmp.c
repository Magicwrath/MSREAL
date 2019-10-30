      fp = fopen("/dev/led", "w");

      if (fp == NULL) {
      puts("Problem pri otvaranju /dev/led\n");
      return -1;
      }

      fputs("0x00\n", fp);

      if(fclose(fp)) {
      puts("Problem sa zatvaranjem /dev/led\n");
      return -1;
      }
