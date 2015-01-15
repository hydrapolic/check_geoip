#include <GeoIP.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>
#include <stdarg.h>

#define GEOIP_PATH "/usr/share/GeoIP/GeoIP.dat"

#define TEST_IP "8.8.8.8"
#define TEST_COUNTRY "US"

// db version: 20150106
#define DATE_LEN 8
#define DB_AGE_WARNING 60
#define DB_AGE_CRITICAL 300

enum {
  OK = 0,
  ERROR = -1
};

enum {  
  STATE_OK,
  STATE_WARNING,
  STATE_CRITICAL
};

int open_db(GeoIP **gi) {
  *gi = GeoIP_open(GEOIP_PATH, GEOIP_STANDARD);

  if (*gi == NULL)
    return ERROR;

  return OK;
}
  
void close_db(GeoIP **gi) {
  GeoIP_delete(*gi);
}

int get_country_by_addr(GeoIP *gi, char *address, const char **returned_country) {
  *returned_country = GeoIP_country_code_by_addr(gi, address);

  if(*returned_country == NULL)
    return ERROR;

  return OK;
}

int get_db_age(GeoIP *gi) {
  const char *info;
  char *p;
  char db_date[DATE_LEN];
  struct tm db_date_tm;
  time_t db_date_timet;
  time_t now;

  bzero(db_date, DATE_LEN);
  bzero(&db_date_tm, sizeof(struct tm));

  info = GeoIP_database_info(gi);
  p = strstr(info, " ");
  strncpy(db_date, p+1, DATE_LEN);
  
  strptime(db_date, "%Y%m%d", &db_date_tm);
  db_date_timet = mktime(&db_date_tm);
  time(&now);
  
  return difftime(now, db_date_timet) / 3600 / 24;

}

void print_status(int ret, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");

  exit (ret);
}

int main(int argc, char **argv) {
  GeoIP *gi;
  const char *returned_country;
  int db_age;

  if(open_db(&gi) != OK) {
    print_status(STATE_CRITICAL, "CRITICAL: cannot open database %s", GEOIP_PATH);
  }
  
  if(get_country_by_addr(gi, TEST_IP, &returned_country) != OK) {
    print_status(STATE_CRITICAL, "CRITICAL: cannot query database %s", GEOIP_PATH); 
  } else {
    if(strcmp(returned_country, TEST_COUNTRY) != 0) {
      print_status(STATE_CRITICAL, "CRITICAL: country query for %s should return %s but returned %s", TEST_IP, TEST_COUNTRY, returned_country); 
    }
  }

  db_age = get_db_age(gi);
  close_db(&gi);
  
  if(db_age > DB_AGE_CRITICAL) {
    print_status(STATE_CRITICAL, "CRITICAL: database age = %d days, query for IP %s returned country %s", db_age, TEST_IP, returned_country);
  } else if (db_age > DB_AGE_WARNING) {
    print_status(STATE_CRITICAL, "WARNING: database age = %d days, query for IP %s returned country %s", db_age, TEST_IP, returned_country);
  } else {
    print_status(STATE_OK, "OK: database age = %d days, query for IP %s returned country %s", db_age, TEST_IP, returned_country);
  }

  return 0;
}
