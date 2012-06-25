# Makefile for compile xflow utilities
# (c) 2002-2006 D.Béréziat
# $Id: Makefile.am,v 1.4 2007/09/24 11:43:35 bereziat Exp $

# common switches
INCLUDES = -I.. @PACKAGE_CFLAGS@
AM_LDFLAGS = -L../lib/ 
LDADD   = -lxflow -lm

# list of programs to build
bin_PROGRAMS = vel2fig vel2lic velpar velset uv2vel veldif velerr velnorm velang velstats vel2col

# @todo move vel2grad in an example folder
# @todo rename error into velerr or something like that
# @todo program error has to be finished :-)


vel2col_SOURCES = vel2col.c colorcode.c	colorcode.h
vel2col_CC = c99

