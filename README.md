# check_geoip

This plugin checks the GeoIP database.

It tries to open the database (GeoIP.dat), does a ip-to-country query and checks the database age.

 -w, --warning=INTEGER
    Exit with WARNING status if database is older then INTEGER days (default: 60)
 -c, --critical=INTEGER
    Exit with CRITICAL status if database is older then INTEGER days (default :300)
 -d, --dbpath=PATH
    Overrides the GeoIP database path (default: /usr/share/GeoIP/GeoIP.dat)
 -i, --IP-address=ADDRESS
    Overrides the default tested IP address (default: 8.8.8.8)
 -C, --country=CC
    Overrides the expected country (default: US)
