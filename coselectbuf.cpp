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
#include "coselectbuf.h"
#include <stdexcept>

static fd_set onefdset(int fd) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    return fds;
}

namespace coselect {

ibuf::ibuf(handle_t &h, int fd, char * buf,
        size_t bufsz, size_t pb) : handle(h), filedes(fd),
        buffer(buf), bufsize(bufsz), pushback(pb) {
    if(bufsz <= pb)
        throw std::invalid_argument("buffer size <= pushback");
}
int ibuf::underflow() {
    if(gptr() != egptr())
        return traits_type::to_int_type(*gptr());
    size_t n = std::min(size_t(egptr() - eback()), pushback);
    std::copy(egptr() - n, egptr(), buffer);

    fd_set fds = onefdset(filedes);
    select(handle, filedes + 1, &fds, nullptr, nullptr, nullptr);

    ssize_t m = read(filedes, buffer + n, bufsize - n);
    if(m < 0)
        perror("read");
    if(m <= 0)
        return traits_type::eof();
    setg(buffer, buffer + n, buffer + n + m);
    return traits_type::to_int_type(*gptr());
}

obuf::obuf(handle_t &h, int fd, char * buf,
        size_t bufsz) : handle(h), filedes(fd),
        buffer(buf), bufsize(bufsz) {
    setp(buffer, buffer + bufsize);
}
obuf::~obuf() {
    obuf::sync();
}
int obuf::overflow(int c) {
    if(c != EOF) {
        if(sync() < 0)
            return EOF;
        *pptr() = c;
        pbump(1);
        return traits_type::to_int_type(c);
    }
    return EOF;
}
int obuf::sync() {
    char *cur = pbase();
    size_t n = pptr() - pbase();
    while(n) {
        fd_set fds = onefdset(filedes);
        select(handle, filedes + 1, nullptr, &fds, nullptr, nullptr);

        ssize_t m = write(filedes, cur, n);
        if(m < 0)
            perror("write");
        if(m <= 0)
            return -1;
        n -= m;
    }
    pbump(pbase() - pptr());
    return 0;
}

}
