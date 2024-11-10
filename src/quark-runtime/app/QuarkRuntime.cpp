/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Global todos:
// These are things that we don't care enough about to do now, but would
// be nice in the future

// Windows todos:
// TODO: Implement dll blocklist

// macOS todos:
// TODO: Allow disabling content processes on macOS

#include "XREChildData.h"
#include "mozilla/Bootstrap.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef XP_WIN
#include <windows.h>
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif
#define strcasecmp _stricmp
#endif

#include "nsAppRunner.h"
#include "nsCOMPtr.h"
#include "nsCRTGlue.h"
#include "nsIFile.h"
#include "nsMemory.h"
// #include "nsStringAPI.h"
// #include "nsServiceManagerUtils.h"
#include "plstr.h"
#include "prenv.h"
#include "prprf.h"
// #include "nsINIParser.h" GRE Versioning stuff that needs to be in libxul :(
#include "application.ini.h"

#ifdef XP_WIN
#define XRE_DONT_SUPPORT_XPSP2 // See https://bugzil.la/1023941#c32
#include "nsWindowsWMain.cpp"
#endif

#include "BinaryPath.h"

#include "nsXPCOMPrivate.h" // for MAXPATHLEN and XPCOM_DLL

// NOTE: Firefox does not allow content processes on android and coccoa.
// We target neither, so we leave our content process code outside of macros

using namespace mozilla;

/**
 * Output a string to the user.  This method is really only meant to be used to
 * output last-ditch error messages designed for developers NOT END USERS.
 *
 * @param isError
 *        Pass true to indicate severe errors.
 * @param fmt
 *        printf-style format string followed by arguments.
 */
static void Output(bool isError, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

#if (defined(XP_WIN) && !MOZ_WINCONSOLE)
  wchar_t msg[2048];
  _vsnwprintf(msg, sizeof(msg) / sizeof(msg[0]),
              NS_ConvertUTF8toUTF16(fmt).get(), ap);

  UINT flags = MB_OK;
  if (isError)
    flags |= MB_ICONERROR;
  else
    flags |= MB_ICONINFORMATION;

  MessageBoxW(nullptr, msg, L"XULRunner", flags);
#else
  vfprintf(stderr, fmt, ap);
#endif

  va_end(ap);
}

/**
 * Return true if |arg| matches the given argument name.
 */
static bool IsArg(const char *arg, const char *s) {
  if (*arg == '-') {
    if (*++arg == '-')
      ++arg;
    return !strcasecmp(arg, s);
  }

  return false;
}

// TODO: Bring back application file parsing
//
// static nsresult
// GetGREVersion(const char *argv0,
//               nsACString *aMilestone,
//               nsACString *aVersion)
// {
//   if (aMilestone)
//     aMilestone->AssignLiteral("<Error>");
//   if (aVersion)
//     aVersion->AssignLiteral("<Error>");

//   nsCOMPtr<nsIFile> iniFile;
//   nsresult rv = XRE_GetBinaryPath(getter_AddRefs(iniFile));
//   if (NS_FAILED(rv))
//     return rv;

//   iniFile->SetNativeLeafName(nsLiteralCString("platform.ini"));

//   nsINIParser parser;
//   rv = parser.Init(iniFile);
//   if (NS_FAILED(rv))
//     return rv;

//   if (aMilestone)
//   {
//     rv = parser.GetString("Build", "Milestone", *aMilestone);
//     if (NS_FAILED(rv))
//       return rv;
//   }
//   if (aVersion)
//   {
//     rv = parser.GetString("Build", "BuildID", *aVersion);
//     if (NS_FAILED(rv))
//       return rv;
//   }
//   return NS_OK;
// }

static void Usage(const char *argv0) {
  // nsAutoCString milestone;
  // GetGREVersion(argv0, &milestone, nullptr);

  // display additional information (XXX make localizable?)
  Output(false,
         "Quark Runtime %s\n\n"
         "Usage: " XULRUNNER_PROGNAME " [OPTIONS]\n"
         "       " XULRUNNER_PROGNAME " APP-FILE [APP-OPTIONS...]\n"
         "\n"
         "OPTIONS\n"
         "      --app                  specify APP-FILE (optional)\n"
         "  -h, --help                 show this message\n"
         "  -v, --version              show version\n"
         "  --gre-version              print the GRE version string on stdout\n"
         "\n"
         "APP-FILE\n"
         "  Application initialization file.\n"
         "\n"
         "APP-OPTIONS\n"
         "  Application specific options.\n",
         "<TODO: app.ini gecko milestone here>");
}

Bootstrap::UniquePtr gBootstrap;

static nsresult initXPCOMGlue(LibLoadingStrategy libLoadingStrategy) {
  if (gBootstrap)
    return NS_OK;

  UniqueFreePtr<char> exePathPtr = BinaryPath::Get();
  if (!exePathPtr) {
    Output(true, "Couldn't calculate the application directory.\n");
    return NS_ERROR_FAILURE;
  }

  char *exePath = exePathPtr.get();

  char *lastSlash = strrchr(exePath, XPCOM_FILE_PATH_SEPARATOR[0]);
  if (!lastSlash ||
      (size_t(lastSlash - exePath) > MAXPATHLEN - sizeof(XPCOM_DLL) - 1))
    return NS_ERROR_FAILURE;

  strcpy(++lastSlash, XPCOM_DLL);

  auto bootstrapResult = mozilla::GetBootstrap(exePath, libLoadingStrategy);
  if (bootstrapResult.isErr()) {
    Output(true, "Couldn't load XPCOM.\n");
    return NS_ERROR_FAILURE;
  }

  gBootstrap = bootstrapResult.unwrap();

  gBootstrap->NS_LogInit();

  return NS_OK;
}

int main(int argc, char *argv[]) {
#if defined(MOZ_ENABLE_FORKSERVER)
  if (strcmp(argv[argc - 1], "forkserver") == 0) {
    nsresult rv = InitXPCOMGlue(LibLoadingStrategy::NoReadAhead);
    if (NS_FAILED(rv)) {
      return 255;
    }

    // Run a fork server in this process, single thread.  When it
    // returns, it means the fork server have been stopped or a new
    // content process is created.
    //
    // For the later case, XRE_ForkServer() will return false, running
    // in a content process just forked from the fork server process.
    // argc & argv will be updated with the values passing from the
    // chrome process.  With the new values, this function
    // continues the reset of the code acting as a content process.
    if (gBootstrap->XRE_ForkServer(&argc, &argv)) {
      // Return from the fork server in the fork server process.
      // Stop the fork server.
      gBootstrap->NS_LogTerm();
      return 0;
    }
    // In a content process forked from the fork server.
    // Start acting as a content process.
  }
#endif

  if (argc > 1 && IsArg(argv[1], "contentproc")) {
    // Set the process type. We don't remove the arg here as that will be done
    // later in common code.
    SetGeckoProcessType(argv[argc - 1]);
    SetGeckoChildID(argv[--argc]);

    nsresult initXPCOMGlueResult =
        initXPCOMGlue(LibLoadingStrategy::NoReadAhead);
    if (NS_FAILED(initXPCOMGlueResult)) {
      Output(true, "Failed to load xpcom glue");
      return 255;
    }

    XREChildData childData;
    nsresult contentProcMainResult =
        gBootstrap->XRE_InitChildProcess(argc, argv, &childData);

    // InitXPCOMGlue calls NS_LogInit, so we need to balance it here.
    gBootstrap->NS_LogTerm();

    return NS_FAILED(contentProcMainResult) ? 1 : 0;
  }

  if (argc > 1 &&
      (IsArg(argv[1], "h") || IsArg(argv[1], "help") || IsArg(argv[1], "?"))) {
    Usage(argv[0]);
    return 0;
  }

  if (argc == 2 && (IsArg(argv[1], "v") || IsArg(argv[1], "version"))) {
    // nsAutoCString milestone;
    // nsAutoCString version;
    // GetGREVersion(argv[0], &milestone, &version);
    // Output(false, "Mozilla XULRunner %s - %s\n",
    //        milestone.get(), version.get());
    Output(false, "Mozilla XULRunner <TODO: version here>\n");
    return 0;
  }

  if (argc > 1) {
    // nsAutoCString milestone;
    // nsresult rv = GetGREVersion(argv[0], &milestone, nullptr);
    // if (NS_FAILED(rv))
    //   return 2;

    if (IsArg(argv[1], "gre-version")) {
      if (argc != 2) {
        Usage(argv[0]);
        return 1;
      }

      printf("TODO: gre-version here\n");
      // printf("%s\n", milestone.get());
      return 0;
    }

    if (IsArg(argv[1], "install-app")) {
      Output(true, "--install-app support has been removed.  Use 'python "
                   "install-app.py' instead.\n");
      return 1;
    }
  }

  const char *appDataFile = getenv("XUL_APP_FILE");

  if (!(appDataFile && *appDataFile)) {
    if (argc < 2) {
      Usage(argv[0]);
      return 1;
    }

    if (IsArg(argv[1], "app")) {
      if (argc == 2) {
        Usage(argv[0]);
        return 1;
      }
      argv[1] = argv[0];
      ++argv;
      --argc;
    }

    appDataFile = argv[1];
    argv[1] = argv[0];
    ++argv;
    --argc;

    static char kAppEnv[MAXPATHLEN];
    snprintf(kAppEnv, MAXPATHLEN, "XUL_APP_FILE=%s", appDataFile);
    putenv(kAppEnv);
  }

  BootstrapConfig config;

  if (appDataFile[0] != '-') {
    config.appData = nullptr;
    config.appDataPath = appDataFile;
  } else {
    config.appData = &sAppData;
    config.appDataPath = "quark-runtime";
  }

  nsresult initXPCOMGlueResult = initXPCOMGlue(LibLoadingStrategy::NoReadAhead);
  if (NS_FAILED(initXPCOMGlueResult)) {
    Output(true, "Failed to load xpcom glue");
    return 255;
  }

  gBootstrap->XRE_EnableSameExecutableForContentProc();
  int xreMainResult = gBootstrap->XRE_main(argc, argv, config);

  gBootstrap->NS_LogTerm();
  gBootstrap.reset();

  return xreMainResult;
}
