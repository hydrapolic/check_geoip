#include "GeoIP.h"

#define GEOIP_PATH "/usr/share/GeoIP"

#define TEST_IP "8.8.8.8"
#define TEST_COUNTRY "US"

#define NAGIOS_OK 0
#define NAGIOS_ERROR 1

int print_status(char *text, int ret) {
  printf("%s\n", text);

  exit(ret);
}

int main(int argc, char **argv) {
  GeoIP *gi;
  const char *returned_country;

  gi = GeoIP_open(GEOIP_PATH "/GeoIP.dat", GEOIP_STANDARD);

  if (gi == NULL)
    print_status("CRITICAL: cannot open database", NAGIOS_ERROR); 
  
  returned_country = GeoIP_country_code_by_addr(gi, TEST_IP);

  if(returned_country == NULL)
    print_status("CRITICAL: cannot query database", NAGIOS_ERROR); 

  if(strcmp(returned_country, TEST_COUNTRY) != 0) {
    print_status("CRITICAL: country query returned different values", NAGIOS_ERROR); 
  }

  GeoIP_delete(gi);
  print_status("OK", NAGIOS_OK);

  return 0;
}
