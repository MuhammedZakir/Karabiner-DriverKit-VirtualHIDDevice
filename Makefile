VERSION = `head -n 1 version`

all:
	@echo 'Type `make package`'

build:
	$(MAKE) -C src
	$(MAKE) -C client/examples

clean:
	$(MAKE) -C src clean
	$(MAKE) -C client/examples clean
	$(MAKE) -C tests clean

package: clean
	bash make-package.sh

notarize:
	xcrun altool --notarize-app \
		-t osx \
		-f dist/Karabiner-DriverKit-VirtualHIDDevice-$(VERSION).dmg \
		--primary-bundle-id 'org.pqrs.Karabiner-DriverKit-VirtualHIDDevice' \
		-u 'tekezo@pqrs.org' \
		-p '@keychain:pqrs.org-notarize-app'

staple:
		xcrun stapler staple dist/Karabiner-DriverKit-VirtualHIDDevice-$(VERSION).dmg
