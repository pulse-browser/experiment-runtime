#! /bin/sh
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

MOZ_APP_NAME=quark-runtime
MOZ_APP_DISPLAYNAME="Quark Runtime"
MOZ_UPDATER=1
MOZ_XULRUNNER=1
MOZ_CHROME_FILE_FORMAT=omni
MOZ_APP_VERSION=$MOZILLA_VERSION
MOZ_PLACES=1
MOZ_EXTENSIONS_DEFAULT=" gio"
MOZ_URL_CLASSIFIER=1
MOZ_SERVICES_COMMON=1
MOZ_SERVICES_CRYPTO=1
MOZ_SERVICES_METRICS=1
MOZ_SERVICES_SYNC=1
MOZ_MEDIA_NAVIGATOR=1
MOZ_SERVICES_HEALTHREPORT=1
MOZ_DISABLE_EXPORT_JS=1

MOZ_APP_ID={ec8030f7-c20a-464f-9b0e-13a3a9e97384}

# Include the DevTools client, not just the server (which is the default)
MOZ_DEVTOOLS=all
