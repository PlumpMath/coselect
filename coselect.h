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
#include <boost/coroutine2/coroutine.hpp>
#include <sys/select.h>
#include <list>
#include <functional>

namespace coselect {

struct data_t;

using coro_t = boost::coroutines2::coroutine<
        std::reference_wrapper<data_t>>;
using handle_t = coro_t::push_type;
using pull_handle_t = coro_t::pull_type;

int select(handle_t &handle, int nfds, fd_set *readfds,
        fd_set *writefds, fd_set *exceptfds,
        struct timeval *timeout);

void add_handle(handle_t& handle, pull_handle_t&& new_handle);

void select_loop(std::list<pull_handle_t> &&handles);

}

