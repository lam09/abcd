/****************************************************************************
*
*     X-line Linux Kernel driver
*     File:    loader.c
*     Version: 1.0
*     Copyright 2008 Heber Limited (http://www.heber.co.uk)
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2, or (at your option) any
*     later version.
*
*     This program is distributed in the hope that it will be useful, but
*     WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*     General Public License for more details.
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
*     Notes:
*     This source file is formatted for 4-space tabs
*
*     Changes:
*     1.0 MJB       First version, 30/9/2003
*
****************************************************************************/

static INTEL_HEX_RECORD loader[] = {
{ 16, 0x368, 0, {0x90,0xe6,0x68,0xe0,0xff,0x74,0xff,0xf0,0xe0,0xb4,0x0b,0x04,0xef,0xf0,0xd3,0x22} },
{ 6,  0x378, 0, {0x90,0xe6,0x68,0xef,0xf0,0xc3} },
{ 1,  0x37e, 0, {0x22} },
{ 16, 0x1b5, 0, {0x90,0x7f,0xe9,0xe0,0x64,0xa3,0x60,0x03,0x02,0x02,0xc5,0xa3,0xe0,0x75,0x08,0x00} },
{ 16, 0x1c5, 0, {0xf5,0x09,0xa3,0xe0,0xfe,0xe4,0xee,0x42,0x08,0x90,0x7f,0xee,0xe0,0x75,0x0a,0x00} },
{ 16, 0x1d5, 0, {0xf5,0x0b,0xa3,0xe0,0xfe,0xe4,0xee,0x42,0x0a,0x90,0x7f,0xe8,0xe0,0x64,0x40,0x70} },
{ 16, 0x1e5, 0, {0x64,0xe5,0x0b,0x45,0x0a,0x70,0x03,0x02,0x02,0xd6,0xe4,0x90,0x7f,0xc5,0xf0,0x90} },
{ 16, 0x1f5, 0, {0x7f,0xb4,0xe0,0x20,0xe3,0xf9,0x90,0x7f,0xc5,0xe0,0x75,0x0c,0x00,0xf5,0x0d,0xe4} },
{ 16, 0x205, 0, {0xfc,0xfd,0xc3,0xed,0x95,0x0d,0xec,0x95,0x0c,0x50,0x1f,0x74,0xc0,0x2d,0xf5,0x82} },
{ 16, 0x215, 0, {0xe4,0x34,0x7e,0xf5,0x83,0xe0,0xff,0xe5,0x09,0x2d,0xf5,0x82,0xe5,0x08,0x3c,0xf5} },
{ 16, 0x225, 0, {0x83,0xef,0xf0,0x0d,0xbd,0x00,0x01,0x0c,0x80,0xd8,0xe5,0x0d,0x25,0x09,0xf5,0x09} },
{ 16, 0x235, 0, {0xe5,0x0c,0x35,0x08,0xf5,0x08,0xc3,0xe5,0x0b,0x95,0x0d,0xf5,0x0b,0xe5,0x0a,0x95} },
{ 16, 0x245, 0, {0x0c,0xf5,0x0a,0x80,0x9c,0x90,0x7f,0xe8,0xe0,0x64,0xc0,0x60,0x03,0x02,0x02,0xd6} },
{ 16, 0x255, 0, {0xe5,0x0b,0x45,0x0a,0x60,0x7b,0xc3,0xe5,0x0b,0x94,0x40,0xe5,0x0a,0x94,0x00,0x50} },
{ 16, 0x265, 0, {0x08,0x85,0x0a,0x0c,0x85,0x0b,0x0d,0x80,0x06,0x75,0x0c,0x00,0x75,0x0d,0x40,0xe4} },
{ 16, 0x275, 0, {0xfc,0xfd,0xc3,0xed,0x95,0x0d,0xec,0x95,0x0c,0x50,0x1f,0xe5,0x09,0x2d,0xf5,0x82} },
{ 16, 0x285, 0, {0xe5,0x08,0x3c,0xf5,0x83,0xe0,0xff,0x74,0x00,0x2d,0xf5,0x82,0xe4,0x34,0x7f,0xf5} },
{ 16, 0x295, 0, {0x83,0xef,0xf0,0x0d,0xbd,0x00,0x01,0x0c,0x80,0xd8,0x90,0x7f,0xb5,0xe5,0x0d,0xf0} },
{ 16, 0x2a5, 0, {0x25,0x09,0xf5,0x09,0xe5,0x0c,0x35,0x08,0xf5,0x08,0xc3,0xe5,0x0b,0x95,0x0d,0xf5} },
{ 16, 0x2b5, 0, {0x0b,0xe5,0x0a,0x95,0x0c,0xf5,0x0a,0x90,0x7f,0xb4,0xe0,0x30,0xe2,0x92,0x80,0xf7} },
{ 16, 0x2c5, 0, {0x90,0x7f,0xe9,0xe0,0xb4,0xac,0x0a,0xe4,0x90,0x7f,0x00,0xf0,0x90,0x7f,0xb5,0x04} },
{ 8,  0x2d5, 0, {0xf0,0x90,0x7f,0xb4,0xe0,0x44,0x02,0xf0} },
{ 1,  0x2dd, 0, {0x22} },
{ 16, 0x80,  0, {0x90,0xe6,0xb9,0xe0,0x64,0xa3,0x60,0x03,0x02,0x01,0x98,0xa3,0xe0,0x75,0x08,0x00} },
{ 16, 0x90,  0, {0xf5,0x09,0xa3,0xe0,0xfe,0xe4,0xee,0x42,0x08,0x90,0xe6,0xbe,0xe0,0x75,0x0a,0x00} },
{ 16, 0xa0,  0, {0xf5,0x0b,0xa3,0xe0,0xfe,0xe4,0xee,0x42,0x0a,0x90,0xe6,0xb8,0xe0,0x64,0x40,0x70} },
{ 16, 0xb0,  0, {0x66,0xe5,0x0b,0x45,0x0a,0x70,0x03,0x02,0x01,0xad,0xe4,0x90,0xe6,0x8a,0xf0,0xa3} },
{ 16, 0xc0,  0, {0xf0,0x90,0xe6,0xa0,0xe0,0x20,0xe1,0xf9,0x90,0xe6,0x8b,0xe0,0x75,0x0c,0x00,0xf5} },
{ 16, 0xd0,  0, {0x0d,0xe4,0xfc,0xfd,0xc3,0xed,0x95,0x0d,0xec,0x95,0x0c,0x50,0x1f,0x74,0x40,0x2d} },
{ 16, 0xe0,  0, {0xf5,0x82,0xe4,0x34,0xe7,0xf5,0x83,0xe0,0xff,0xe5,0x09,0x2d,0xf5,0x82,0xe5,0x08} },
{ 16, 0xf0,  0, {0x3c,0xf5,0x83,0xef,0xf0,0x0d,0xbd,0x00,0x01,0x0c,0x80,0xd8,0xe5,0x0d,0x25,0x09} },
{ 16, 0x100, 0, {0xf5,0x09,0xe5,0x0c,0x35,0x08,0xf5,0x08,0xc3,0xe5,0x0b,0x95,0x0d,0xf5,0x0b,0xe5} },
{ 16, 0x110, 0, {0x0a,0x95,0x0c,0xf5,0x0a,0x80,0x9a,0x90,0xe6,0xb8,0xe0,0x64,0xc0,0x60,0x03,0x02} },
{ 16, 0x120, 0, {0x01,0xad,0xe5,0x0b,0x45,0x0a,0x70,0x03,0x02,0x01,0xad,0xc3,0xe5,0x0b,0x94,0x40} },
{ 16, 0x130, 0, {0xe5,0x0a,0x94,0x00,0x50,0x08,0x85,0x0a,0x0c,0x85,0x0b,0x0d,0x80,0x06,0x75,0x0c} },
{ 16, 0x140, 0, {0x00,0x75,0x0d,0x40,0xe4,0xfc,0xfd,0xc3,0xed,0x95,0x0d,0xec,0x95,0x0c,0x50,0x1f} },
{ 16, 0x150, 0, {0xe5,0x09,0x2d,0xf5,0x82,0xe5,0x08,0x3c,0xf5,0x83,0xe0,0xff,0x74,0x40,0x2d,0xf5} },
{ 16, 0x160, 0, {0x82,0xe4,0x34,0xe7,0xf5,0x83,0xef,0xf0,0x0d,0xbd,0x00,0x01,0x0c,0x80,0xd8,0xe4} },
{ 16, 0x170, 0, {0x90,0xe6,0x8a,0xf0,0xa3,0xe5,0x0d,0xf0,0x25,0x09,0xf5,0x09,0xe5,0x0c,0x35,0x08} },
{ 16, 0x180, 0, {0xf5,0x08,0xc3,0xe5,0x0b,0x95,0x0d,0xf5,0x0b,0xe5,0x0a,0x95,0x0c,0xf5,0x0a,0x90} },
{ 16, 0x190, 0, {0xe6,0xa0,0xe0,0x30,0xe1,0x8c,0x80,0xf7,0x90,0xe6,0xb9,0xe0,0xb4,0xac,0x0e,0x90} },
{ 16, 0x1a0, 0, {0xe7,0x40,0x74,0x01,0xf0,0xe4,0x90,0xe6,0x8a,0xf0,0xa3,0x04,0xf0,0x90,0xe6,0xa0} },
{ 4,  0x1b0, 0, {0xe0,0x44,0x80,0xf0} },
{ 1,  0x1b4, 0, {0x22} },
{ 16, 0x2de, 0, {0xc2,0x01,0x12,0x03,0x68,0x92,0x00,0x90,0x7f,0x95,0xe0,0x44,0xc0,0xf0,0xd2,0xe8} },
{ 16, 0x2ee, 0, {0x30,0x00,0x08,0x90,0xe6,0x5d,0x74,0xff,0xf0,0x80,0x06,0x90,0x7f,0xab,0x74,0xff} },
{ 16, 0x2fe, 0, {0xf0,0x30,0x00,0x08,0x90,0xe6,0x68,0x74,0x08,0xf0,0x80,0x07,0x90,0x7f,0xaf,0xe0} },
{ 16, 0x30e, 0, {0x44,0x01,0xf0,0x30,0x00,0x08,0x90,0xe6,0x5c,0x74,0x01,0xf0,0x80,0x06,0x90,0x7f} },
{ 16, 0x31e, 0, {0xae,0x74,0x01,0xf0,0xd2,0xaf,0x30,0x01,0xfd,0x30,0x00,0x05,0x12,0x00,0x80,0x80} },
{ 8,  0x32e, 0, {0x03,0x12,0x01,0xb5,0xc2,0x01,0x80,0xee} },
{ 3,  0x3,   0, {0x02,0x03,0x36} },
{ 16, 0x336, 0, {0xc0,0xe0,0xc0,0x83,0xc0,0x82,0xc0,0x85,0xc0,0x84,0xc0,0x86,0x75,0x86,0x00,0xd2} },
{ 16, 0x346, 0, {0x01,0x53,0x91,0xef,0x30,0x00,0x08,0x90,0xe6,0x5d,0x74,0x01,0xf0,0x80,0x06,0x90} },
{ 16, 0x356, 0, {0x7f,0xab,0x74,0x01,0xf0,0xd0,0x86,0xd0,0x84,0xd0,0x85,0xd0,0x82,0xd0,0x83,0xd0} },
{ 2,  0x366, 0, {0xe0,0x32} },
{ 3,  0x43,  0, {0x02,0x04,0x00} },
{ 4,  0x400, 0, {0x02,0x03,0x36,0x00} },
{ 3,  0x0,   0, {0x02,0x03,0x7f} },
{ 12, 0x37f, 0, {0x78,0x7f,0xe4,0xf6,0xd8,0xfd,0x75,0x81,0x20,0x02,0x02,0xde} },
{ 0,  0x0,   1, {0} }
};
