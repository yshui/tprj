/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _FCITX_LOG_H_
#define _FCITX_LOG_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum _ErrorLevel
    {
        DEBUG,
        ERROR,
        INFO,
        FATAL,
        WARNING
    } ErrorLevel;

#define FcitxLog(e, fmt...) FcitxLogFunc(e, __FILE__, __LINE__, fmt)

    /**
     * @brief print a log string to stderr, should use FcitxLog instead of this function
     *
     * @param level log level
     * @param filename current filename
     * @param line line number
     * @param fmt printf fmt
     * @param  ...
     * @return void
     **/
    void FcitxLogFunc(ErrorLevel, const char* filename, const int line, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 0;
