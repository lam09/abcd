#############################################################################
#
#     X-line Linux Kernel driver
#     File:    Makefile
#     Version: 1.0
#     Copyright 2012 Heber Limited (http://www.heber.co.uk)
#
#     This program is free software; you can redistribute it and/or modify it
#     under the terms of the GNU General Public License as published by the
#     Free Software Foundation; either version 2, or (at your option) any
#     later version.
#
#     This program is distributed in the hope that it will be useful, but
#     WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     General Public License for more details.
#
#     You should have received a copy of the GNU General Public License along
#     with this program; if not, write to the Free Software Foundation, Inc.,
#     675 Mass Ave, Cambridge, MA 02139, USA.
#
#     Changes:
#     1.0 MJB       First version. Based on X10 Makefile, 20/12/2005
#
#     This makefile is for 2.6 series kernels only. The upper case 'M' for
#     Makefile is important.
#
#     This makefile should not be called directly rather it needs to be called
#     from within the context of a kernel 'make'. In practice this means that 
#     to build this module you need to have the kernel sources installed and 
#     configured, or the appropriate 'linux-headers package' for the running 
#     kernel installed the machine used to build this module.
#     To make the driver issue the command:
#
#     make -C /usr/src/linux SUBDIRS=`pwd` modules
#     or
#     make -C /usr/src/linux-headers-<kernel-version> SUBDIRS=`pwd` modules
#
#     from the directory containing the Xline driver source code.
#     If the Linux kernel source or headers package are installed in different 
#     directories to those above substitute the correct path.
#     
#     The final driver module created is Xline.ko on both 32 and 64-bit platforms.
#
#     To have this driver added to those available to the running kernel add
#     _install to the same command as issued above.:
#
#     make -C /usr/src/linux SUBDIRS=`pwd` modules_install
#     or
#     make -C /usr/src/linux-headers-<kernel-version> SUBDIRS=`pwd` modules_install
#
#
#############################################################################

obj-m:= xline.o 
xline-objs := xline_driver.o
