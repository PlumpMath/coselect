/*
 * IO multiplexing with coroutine and select.
 * Copyright (C) 2016 Pochang Chen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COSELECTBUF_H
#define COSELECTBUF_H
#include "coselect.h"
#include <streambuf>

namespace coselect {

class ibuf : public std::streambuf {
    handle_t &handle;
    const int filedes;
    char * const buffer;
    const size_t bufsize, pushback;
public:
    ibuf(handle_t &, int filedes, char * buffer,
            size_t buffer_size, size_t pushback = 1);
protected:
    virtual int underflow() override;
};

class obuf : public std::streambuf {
    handle_t &handle;
    const int filedes;
    char * const buffer;
    const size_t bufsize;
public:
    obuf(handle_t &, int filedes, char * buffer,
            size_t buffer_size);
    ~obuf();
protected:
    virtual int overflow(int) override;
    virtual int sync() override;
};

}
#endif // COSELECTBUF_H
