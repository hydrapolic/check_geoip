CFLAGS=-Wall
LDFLAGS=-lGeoIP
SOURCES=check_geoip.c
EXECUTABLE=check_geoip

$(EXECUTABLE): $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -rf $(EXECUTABLE)
