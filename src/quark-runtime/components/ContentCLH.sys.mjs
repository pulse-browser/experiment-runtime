//@ts-check
///<reference types="gecko-types" />

/**
 * This is intended to be overridden by your program to provide
 * CLI argument parsing once your profile and everything has been
 * loaded
 */
export class ContentCLH {
  classID = Components.ID('{12238385-abbf-4fdb-b6ce-083549a96ba0}')

  // nsISupports
  QueryInterface = ChromeUtils.generateQI(['nsICommandLineHandler'])

  // nsICommandLineHandler

  /**
   * @param {nsICommandLineType} cmdLine
   * @see {@link https://searchfox.org/mozilla-central/source/toolkit/components/commandlines/nsICommandLineHandler.idl}
   */
  handle(cmdLine) { }
}
