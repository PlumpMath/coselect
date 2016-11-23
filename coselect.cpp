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
#include "coselect.h"
#include <utility>
#include <algorithm>

namespace coselect {

struct data_t {
    int nfds;
    fd_set *readfds;
    fd_set *writefds;
    fd_set *exceptfds;
    struct timeval *timeout;

    pull_handle_t *new_handle;

    data_t(int n, fd_set *r, fd_set *w, fd_set *e,
            struct timeval *t) :
            nfds(n), readfds(r), writefds(w), exceptfds(e),
            timeout(t), new_handle(nullptr) {}

    data_t(pull_handle_t *p) :
            nfds(0), readfds(nullptr), writefds(nullptr),
            exceptfds(nullptr), timeout(nullptr),
            new_handle(p) {}
};

struct data2_t {
    int nfds;
    bool hasread, haswrite, hasexcept;
    fd_set readfds, writefds, exceptfds;
    data2_t() : nfds(0), hasread(false), haswrite(false),
            hasexcept(false) {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
    }
    fd_set *get_readfds() {
        return hasread ? &readfds : nullptr;
    }
    fd_set *get_writefds() {
        return haswrite ? &writefds : nullptr;
    }
    fd_set *get_exceptfds() {
        return hasexcept ? &exceptfds : nullptr;
    }
    void add(const data_t&);
    bool match(const data_t&) const;
};

int select(handle_t &handle, int nfds, fd_set *readfds,
        fd_set *writefds, fd_set *exceptfds,
        struct timeval *timeout) {
    // XXX handle invalid argument
    data_t data(nfds, readfds, writefds, exceptfds, timeout);
    handle(data);
    struct timeval zero { 0, 0 };
    return ::select(nfds, readfds, writefds, exceptfds, &zero);
}

void add_handle(handle_t& handle, pull_handle_t&& new_handle) {
    data_t data(&new_handle);
    handle(data);
}

void select_loop(std::list<pull_handle_t> &&handle_list) {
    // TODO respect new handle
    // TODO respect timeout
    while(true) {
        handle_list.remove_if(
                [](const pull_handle_t &h){
                    return !h;
                });
        if(handle_list.empty())
            break;
        data2_t data;
        for(pull_handle_t &h : handle_list)
            data.add(h.get());
        ::select(data.nfds, data.get_readfds(),
                data.get_writefds(),
                data.get_exceptfds(),
                nullptr);
        for(pull_handle_t &h : handle_list)
            if(data.match(h.get()))
                h();
    }
}

void add0(int &nfds, bool &has, fd_set &fds,
        int nfds2, const fd_set *fds2) {
    if(!fds2)
        return;
    for(int i = 0; i < nfds2; i++)
        if(FD_ISSET(i, fds2)) {
            nfds = std::max(nfds, i + 1);
            has = true;
            FD_SET(i, &fds);
        }
}

void data2_t::add(const data_t& rhs) {
    add0(nfds, hasread, readfds, rhs.nfds, rhs.readfds);
    add0(nfds, haswrite, writefds, rhs.nfds, rhs.writefds);
    add0(nfds, hasexcept, exceptfds, rhs.nfds, rhs.exceptfds);
}

bool match0(int nfds, bool has, const fd_set &fds,
        int nfds2, const fd_set *fds2) {
    if(!has || !fds2)
        return false;
    for(int i = 0; i < nfds && i < nfds2; i++)
        if(FD_ISSET(i, &fds) && FD_ISSET(i, fds2))
            return true;
    return false;
}

bool data2_t::match(const data_t& rhs) const {
    return match0(nfds, hasread, readfds, rhs.nfds, rhs.readfds) ||
        match0(nfds, haswrite, writefds, rhs.nfds, rhs.writefds) ||
        match0(nfds, hasexcept, exceptfds, rhs.nfds, rhs.exceptfds);
}

}
