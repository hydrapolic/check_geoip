#include "GeoIP.h"

#define GEOIP_PATH "/usr/share/GeoIP"
#define NAGIOS_OK 0
#define NAGIOS_ERROR 1

int main(int argc, char **argv) {
  GeoIP *gi;
  
  gi = GeoIP_open(GEOIP_PATH "/GeoIP.dat", GEOIP_STANDARD);

  if (gi != NULL) {
    printf("OK");
    GeoIP_delete(gi);
    
    return (NAGIOS_OK);
  } else {
    printf("No database found");
    
    return (NAGIOS_ERROR);
  }
 
}
