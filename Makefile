TOPTARGETS=all clean install
SUBDIRS=lib tests
SNAPCRAFT_PROVIDER?=multipass
SNAPCRAFT_OUTPUT?=cdbc.snap

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

snap: $(SNAPCRAFT_OUTPUT)
$(SNAPCRAFT_OUTPUT):
	snapcraft snap --provider=$(SNAPCRAFT_PROVIDER) -o $(SNAPCRAFT_OUTPUT)

prepare-test-snap:
	mkdir -p tests/cdbc

test-snap: SNAPCRAFT_OUTPUT=tests/cdbc/cdbc.snap
test-snap: prepare-test-snap snap
	@if echo $(SNAPCRAFT_PROVIDER) | grep host >/dev/null; then \
	    sudo rm -rf parts prime stage; \
	fi
	$(MAKE) -C tests $@
