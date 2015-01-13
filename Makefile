LDFLAGS=-lGeoIP
SOURCES=check_geoip.c
EXECUTABLE=check_geoip

$(EXECUTABLE): $(SOURCES)
	$(CC) $(SOURCES) $(LDFLAGS) -o $@

clean:
	rm -rf $(EXECUTABLE)
