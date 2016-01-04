#define SHARE_DATADIR ""
#define PATH_SEPARATOR "/"

#define DISABLE_SYSTEM_TORRC

/* Version number of package */
#define VERSION "LiZ"
#define PACKAGE_VERSION VERSION
#define PACKAGE VERSION

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME PACKAGE

/* Define to the full name and version of this package. */
#define PACKAGE_STRING PACKAGE

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME PACKAGE

/* Define to the home page for this package. */
#define PACKAGE_URL ""

#ifdef _WIN32
#include <tor/orconfig.win.h>
#else
#include <tor/orconfig.lin.h>
#endif
