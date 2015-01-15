#include <GeoIP.h>
#include <string.h>

#define __USE_XOPEN
#include <time.h>
#include <stdarg.h>
#include <getopt.h>

char *geoip_path;
char *test_ip;
char *test_country;
int db_age_warning;
int db_age_critical;

// db version is 8 chars (e.g. 20150106)
#define DATE_LEN 8

enum {
  OK = 0,
  ERROR = -1
};

enum {
  FALSE,
  TRUE
};

enum {  
  STATE_OK,
  STATE_WARNING,
  STATE_CRITICAL,
  STATE_UNKNOWN
};

void print_status(int ret, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");

  exit (ret);
}

void print_help() {
  printf("%s\n", "This plugin checks the GeoIP database");
  printf("\n");
  
  printf (" %s\n", "-w, --warning=INTEGER");
  printf ("    %s (default: %d)\n", "Exit with WARNING status if database is older then INTEGER days", db_age_warning);
  printf (" %s\n", "-c, --critical=INTEGER");
  printf ("    %s (default :%d)\n", "Exit with CRITICAL status if database is older then INTEGER days", db_age_critical);
  printf (" %s\n", "-d, --dbpath=PATH");
  printf ("    %s (default: %s)\n", "Overrides the GeoIP database path", geoip_path);
  printf (" %s\n", "-i, --IP-address=ADDRESS");
  printf ("    %s (default: %s)\n", "Overrides the default tested IP address", test_ip);
  printf (" %s\n", "-C, --country=CC");
  printf ("    %s (default: %s)\n", "Overrides the expected country", test_country);

}

int is_numeric(char *number) {
  char tmp[1];
  float x;

  if (!number)
    return FALSE;
  else if (sscanf (number, "%f%c", &x, tmp) == 1)
    return TRUE;
  else
    return FALSE;
}

int is_positive(char *number) {
  if (is_numeric (number) && atof (number) > 0.0)
    return TRUE;
  else
    return FALSE;
}


int process_arguments(int argc, char **argv) {
  int c = 1;
  int option = 0;

  static struct option longopts[] = {
    {"dbpath", required_argument, 0, 'd'},
    {"IP-address", required_argument, 0, 'i'},
    {"country", required_argument, 0, 'C'},
    {"warning", required_argument, 0, 'w'},
    {"critical", required_argument, 0, 'c'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  if(argc < 2)
    return OK;

  while(1) {
    c = getopt_long(argc, argv, "d:i:C:w:c:h", longopts, &option);

    if(c == 1 || c == EOF)
      break;

    switch(c) {
      case 'd':
        geoip_path = strdup(optarg);
	break;

      case 'i':
        test_ip = strdup(optarg);
	break;

      case 'C':
        test_country = strdup(optarg);
	break;

      case 'w':
        if(is_positive(optarg)) {
          db_age_warning = atoi(optarg);
        } else {
          print_status(STATE_UNKNOWN, "ERROR: warning threshold must be positive number");
        }
	break;
      
      case 'c':
        if(is_positive(optarg)) {
          db_age_critical = atoi(optarg);
        } else {
          print_status(STATE_UNKNOWN, "ERROR: critical threshold must be positive number");
        }
	break;

      case 'h':
        print_help();
	exit(STATE_OK);
    }
  }

  return OK;
}

int open_db(GeoIP **gi, char *geoip_path) {
  *gi = GeoIP_open(geoip_path, GEOIP_STANDARD);

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

int main(int argc, char **argv) {
  GeoIP *gi;
  const char *returned_country;
  int db_age;

  // defaults
  geoip_path = strdup("/usr/share/GeoIP/GeoIP.dat");
  test_ip = strdup("8.8.8.8");
  test_country = strdup("US");
  db_age_warning = 60;
  db_age_critical = 300;

  if(process_arguments(argc, argv) != OK) {
    print_status(STATE_UNKNOWN, "ERROR: cannot process arguments");
  }

  if(db_age_critical < db_age_warning) {
    print_status(STATE_UNKNOWN, "ERROR: db_age_critical cannot be less then db_age_warning");
  }

  if(open_db(&gi, geoip_path) != OK) {
    print_status(STATE_CRITICAL, "CRITICAL: cannot open database %s", geoip_path);
  }
  
  if(get_country_by_addr(gi, test_ip, &returned_country) != OK) {
    print_status(STATE_CRITICAL, "CRITICAL: cannot query database %s", geoip_path); 
  } else {
    if(strcmp(returned_country, test_country) != 0) {
      print_status(STATE_CRITICAL, "CRITICAL: country query for %s should return %s but returned %s", test_ip, test_country, returned_country); 
    }
  }

  db_age = get_db_age(gi);
  close_db(&gi);
  
  if(db_age > db_age_critical) {
    print_status(STATE_CRITICAL, "CRITICAL: database age = %d days, query for IP %s returned country %s | db_age=%d", db_age, test_ip, returned_country, db_age);
  } else if (db_age > db_age_warning) {
    print_status(STATE_WARNING, "WARNING: database age = %d days, query for IP %s returned country %s | db_age=%d", db_age, test_ip, returned_country, db_age);
  } else {
    print_status(STATE_OK, "OK: database age = %d days, query for IP %s returned country %s | db_age=%d", db_age, test_ip, returned_country, db_age);
  }

  return 0;
}
