# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

installer:
	@echo 'Quark Runtime doesn't have an installer yet.'

package:
	@$(MAKE) -C quark-runtime/installer

install:
	@$(MAKE) -C quark-runtime/installer install

sdk:
	@$(MAKE) -C quark-runtime/installer make-sdk

distclean::
	@$(MAKE) -C quark-runtime/installer distclean

source-package::
	@$(MAKE) -C quark-runtime/installer source-package

upload::
	@$(MAKE) -C quark-runtime/installer upload

source-upload::
	@$(MAKE) -C quark-runtime/installer source-upload

hg-bundle::
	@$(MAKE) -C quark-runtime/installer hg-bundle

ifeq ($(OS_TARGET),Linux)
deb: package
	@$(MAKE) -C quark-runtime/installer deb
endif
