//@ts-check
///<reference types="gecko-types" />

/**
 * This component is responsible for launching application windows and
 * handling commandline arguments.
 *
 * Its will try to launch the following:
 * - `-chrome` command line flag (if it exists)
 * - `app.content` if it exists
 * - Warn if neither
 */
export class AppLauncher {
  classID = Components.ID('{2bdc979f-d4da-4c22-8a05-8fc9dd37854d}')

  // nsISupports
  QueryInterface = ChromeUtils.generateQI(['nsICommandLineHandler'])

  // nsICommandLineHandler

  /**
   * @param {nsICommandLineType} cmdLine
   * @see {@link https://searchfox.org/mozilla-central/source/toolkit/components/commandlines/nsICommandLineHandler.idl}
   */
  handle(cmdLine) {
    console.log('AppLauncher hit')

    // Handle chrome flag
    {
      const chromeFlagURI = cmdLine.handleFlagWithParam('chrome', false)

      if (chromeFlagURI) {
        try {
          const flags = Services.prefs.getCharPref(
            'toolkit.defaultChromeFeatures',
            'chrome,dialog=no,all'
          )

          Services.ww.openWindow(null, chromeFlagURI, '_blank', flags, cmdLine)
        } catch (e) {
          console.warn('Failed to open chrome URL', chromeFlagURI)
          console.warn(e)
        }

        return
      }
    }

    {
      try {
        var chromeURI = Services.prefs.getCharPref('app.content')

        var flags = Services.prefs.getCharPref(
          'toolkit.defaultChromeFeatures',
          'chrome,dialog=no,all'
        )

        Services.ww.openWindow(null, chromeURI, '_blank', flags, cmdLine)
        return
      } catch (e) {}
    }

    console.warn(
      'Neither `--chrome` nor `app.content` are set. It is highly likely that your app will launch with no content'
    )
  }
}
