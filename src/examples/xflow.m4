# AC_CHECK_XFLOW (Dominique Béréziat version 0.1)
# -------------------------------------------------
AC_DEFUN([AC_CHECK_XFLOW],[
AC_ARG_WITH(xflow, [  --with-xflow            set XFLOW location (default is $prefix)],[
CPPFLAGS="${CPPFLAGS} -I${withval}/include"
LDFLAGS="${LDFLAGS} -L${withval}/lib"
],
)
AC_CHECK_HEADER(inrimage/xflow.h,,exit)
AC_CHECK_LIB(xflow,xflow_create,,exit)
AC_CHECK_PROG(XFLOW,xflow --version,yes,no)
])
