#include <GeoIP.h>
#include <string.h>
#include <time.h>

#define GEOIP_PATH "/usr/share/GeoIP"

#define TEST_IP "8.8.8.8"
#define TEST_COUNTRY "US"

// db version: 20150106
#define DATE_LEN 8
#define DB_AGE_WARNING 60
#define DB_AGE_ERROR 300

#define NAGIOS_OK 0
#define NAGIOS_WARNING 1
#define NAGIOS_ERROR 2

void open_db(GeoIP **gi) {
  *gi = GeoIP_open(GEOIP_PATH "/GeoIP.dat", GEOIP_STANDARD);

  if (*gi == NULL)
    print_status("CRITICAL: cannot open database", NAGIOS_ERROR);
}
  
void close_db(GeoIP **gi) {
  GeoIP_delete(*gi);
}

void check_country(GeoIP *gi) {
  const char *returned_country;
  
  returned_country = GeoIP_country_code_by_addr(gi, TEST_IP);

  if(returned_country == NULL)
    print_status("CRITICAL: cannot query database", NAGIOS_ERROR); 

  if(strcmp(returned_country, TEST_COUNTRY) != 0) {
    print_status("CRITICAL: country query returned different values", NAGIOS_ERROR); 
  }
}

void check_version(GeoIP *gi) {
  const char *info;
  char *p;
  char db_date[DATE_LEN+1];
  struct tm db_date_tm;
  time_t db_date_timet;
  time_t now;
  double seconds;
  int days;
  char message[100];

  bzero(db_date, DATE_LEN+1);
  bzero(&db_date_tm, sizeof(struct tm));

  info = GeoIP_database_info(gi);
  p = strstr(info, " ");
  strncpy(db_date, p+1, DATE_LEN);
  
  strptime(db_date, "%Y%m%d", &db_date_tm);
  db_date_timet = mktime(&db_date_tm);
  time(&now);
 
  seconds = difftime(now, db_date_timet);
  days = seconds / 3600 / 24;

  if(days > DB_AGE_ERROR) {
    sprintf(message, "ERROR: database is %d old\n", days);
    print_status(message, NAGIOS_ERROR);
  } else if (days > DB_AGE_WARNING) {
    sprintf(message, "WARNING: database is %d old\n", days);
    print_status(message, NAGIOS_WARNING);
  } 
}

int print_status(char *text, int ret) {
  printf("%s\n", text);

  exit(ret);
}

int main(int argc, char **argv) {
  GeoIP *gi;

  open_db(&gi);
  check_country(gi);
  check_version(gi);
  close_db(&gi);
  
  print_status("OK", NAGIOS_OK);

  return 0;
}
