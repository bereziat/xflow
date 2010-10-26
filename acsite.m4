# AC_PATH_INRIMAGE (Dominique Béréziat version 0.1)
# -------------------------------------------------
AC_DEFUN(AC_PATH_INRIMAGE,[
AC_ARG_WITH(inrimage, [  --with-inrimage         set INRIMAGE location (default is $prefix)],[
CFLAGS=-I${withval}/include
CPPFLAGS=-I${withval}/include
LDFLAGS=-L${withval}/lib
],[
AC_CHECK_PROG(INRINFO,inrinfo,yes,no)
if test x$INRINFO = xyes ; then
   AC_MSG_CHECKING([if inrinfo supports --cflags option]) 
   if inrinfo --cflags > /dev/null 2>&1 ; then
      CFLAGS=`inrinfo --cflags`
      LDFLAGS=`inrinfo --libs`
      AC_MSG_RESULT(yes)
   else
      AC_MSG_RESULT(no)
      AC_MSG_WARN([inrimage can not be located, use the switch --with-inrimage to 
specify the exact location])
   fi
fi
])
])
