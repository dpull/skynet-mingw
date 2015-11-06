/*
  cpoll - copyright 2009 by Dan "Heron" Myers (heronblademastr)
  cpoll is released under the LGPL 3, which can be obtained at this
    URL: http://www.gnu.org/copyleft/lesser.html
  In Linux, cpoll transparently passes through to epoll; there should
    be no performance impact.
  In Windows, cpoll requires that you link with a DLL for the required
    functionality (which is why it's LGPL, not GPL).
  http://sourceforge.net/projects/cpoll/

 Usage:
 In Linux, do not use this file!  cpoll.h contains all you need.
 In Windows, inlude cpoll.h, and link with cpoll.lib.  Put cpoll.dll
   in your program executable's directory.  You must #define WIN32.
   Do not compile this file into your program, especially if your
   program is not LGPL-compatible!  You're better off using the DLL.
 */

#include "cpoll.h"

#include <Winsock2.h>

#include <conio.h>
#include <errno.h>
#include <map>
#include <vector>

struct fd_t
{
    int fd;
    struct cpoll_event event;
	HANDLE wsa_event;

    fd_t() {
	}

	~fd_t() {
	}
};

struct cs_t
{
	CRITICAL_SECTION cs;

	void lock()
	{ EnterCriticalSection(&cs); }

	void unlock()
	{ LeaveCriticalSection(&cs); }

	cs_t()
	{ InitializeCriticalSection(&cs); }

	~cs_t()
	{ DeleteCriticalSection(&cs); }
};

struct lock_t {
	cs_t& cs;

	lock_t(cs_t& cs) : cs(cs)
	{ cs.lock(); }

	~lock_t()
	{ cs.unlock(); }
};

cs_t cs;
#define GUARD()\
	lock_t lock(cs);

typedef std::vector<fd_t> cp_internal;

int cp_next_id;
std::map<int, cp_internal> cp_data;

long get_wsa_mask(unsigned int cpoll_events)
{
    long mask = 0;
    if(cpoll_events & CPOLLIN)
        mask |= FD_READ;
    if(cpoll_events & CPOLLOUT)
        mask |= FD_WRITE;
    if(cpoll_events & CPOLLRDHUP)
        mask |= 0; // ??
    if(cpoll_events & CPOLLPRI)
        mask |= 0; // ??
    if(cpoll_events & CPOLLERR)
        mask |= 0; // ??
    if(cpoll_events & CPOLLHUP)
        mask |= FD_CLOSE;
    if(cpoll_events & CPOLLET)
        mask |= 0; // ??
    return mask;
}

unsigned int get_cp_mask(WSANETWORKEVENTS* wsa_events)
{
    unsigned int mask = 0;
    if(wsa_events->lNetworkEvents & FD_READ)
        mask |= CPOLLIN;
    if(wsa_events->lNetworkEvents & FD_WRITE)
        mask |= CPOLLOUT;
    if(wsa_events->lNetworkEvents & FD_CLOSE)
        mask |= CPOLLHUP;
    return mask;
}

int cpoll_startup()
{
    cp_next_id = 0;

    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata);
}

/*
Errors:
    EINVAL
        size is not positive.
    ENFILE
        The system limit on the total number of open files has been reached.
    ENOMEM
        There was insufficient memory to create the kernel object.
*/
int cpoll_create(int size)
{
	GUARD();
    // maintaining error condition for compatibility
    // however, this parameter is ignored.
    if(size < 0)
        return EINVAL;

    ++cp_next_id;

    // ran out of ids!  wrapped around.
    if(cp_next_id > (cp_next_id + 1))
        cp_next_id = 0;

    while(cp_next_id < (cp_next_id + 1)) {

        if(cp_data.find(cp_next_id) == cp_data.end())
            break;
        ++cp_next_id;
    }

    if(cp_next_id < 0) {
        // two billion fds, eh...
        return ENFILE;
    }

    cp_data[cp_next_id] = cp_internal();
    return cp_next_id;
}

/*
    EPOLL_CTL_ADD
        Add the target file descriptor fd to the epoll descriptor epfd and associate the event event with the internal file linked to fd.
    EPOLL_CTL_MOD
        Change the event event associated with the target file descriptor fd.
    EPOLL_CTL_DEL
        Remove the target file descriptor fd from the epoll file descriptor, epfd. The event is ignored and can be NULL (but see BUGS below).
Errors:
    EBADF
        cpfd or fd is not a valid file descriptor.
    EEXIST
        op was EPOLL_CTL_ADD, and the supplied file descriptor fd is already in cpfd.
    EINVAL
        cpfd is not an epoll file descriptor, or fd is the same as cpfd, or the requested operation op is not supported by this interface.
    ENOENT
        op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd is not in cpfd.
    ENOMEM
        There was insufficient memory to handle the requested op control operation.
    EPERM
        The target file fd does not support epoll.
*/
int cpoll_ctl(int cpfd, int opcode, int fd, struct cpoll_event* event)
{
	GUARD();
    if(cpfd < 0 || cp_data.find(cpfd) == cp_data.end())
        return EBADF;

    // TODO: find out if it's possible to tell whether fd is a socket
    // descriptor.  If so, make sure it is; if not, set EPERM and return -1.

    cp_internal& cpi = cp_data[cpfd];

    if(opcode == CPOLL_CTL_ADD) {

        for(cp_internal::size_type i = 0; i < cpi.size(); ++i) { 
            if(cpi[i].fd == fd)
                return EEXIST;
        }

        fd_t f;
		f.fd = fd;
		f.event = *event;
		f.wsa_event = WSACreateEvent();
        f.event.events |= CPOLLHUP;
        f.event.events |= CPOLLERR;
        WSAEventSelect(f.fd, f.wsa_event, FD_ACCEPT | FD_CLOSE | get_wsa_mask(f.event.events));
        cpi.push_back(f);
		return 0;
    }
    else if(opcode == CPOLL_CTL_MOD) {

        for(cp_internal::size_type i = 0; i < cpi.size(); ++i) {

            if(cpi[i].fd == fd) {

                cpi[i].event = *event;
                cpi[i].event.events |= CPOLLHUP;
                cpi[i].event.events |= CPOLLERR;
		        WSAEventSelect(cpi[i].fd, cpi[i].wsa_event, FD_ACCEPT | get_wsa_mask(cpi[i].event.events));
                return 0;
            }
        }
        return ENOENT;
    }
    else if(opcode == CPOLL_CTL_DEL) {

		for(cp_internal::iterator itr = cpi.begin(); itr != cpi.end(); ++itr) {

			if(itr->fd == fd) {
				// now unset the event notifications
				WSAEventSelect(itr->fd, 0, 0);
				// clean up event
				WSACloseEvent(itr->wsa_event);
				cpi.erase(itr);
				return 0;
			}
		}
        return ENOENT;
    }
    return EINVAL;
}

int cpoll_wait(int cpfd, struct cpoll_event* events, int maxevents, int timeout)
{
	GUARD();
    if(cpfd < 0 || cp_data.find(cpfd) == cp_data.end() || maxevents < 1)
        /* EINVAL */
        return -1;

    cp_internal& cpi = cp_data[cpfd];

    WSAEVENT* wsa_events = new WSAEVENT[cpi.size()];
    for(cp_internal::size_type i = 0; i < cpi.size(); ++i)
        wsa_events[i] = cpi[i].wsa_event;

	int num_ready = 0;
	DWORD wsa_result = 0;
	for(;;) {
		wsa_result = WSAWaitForMultipleEvents(cpi.size(), wsa_events, FALSE, 13, FALSE);
		if(wsa_result != WSA_WAIT_TIMEOUT)
			break;
		if(_kbhit()) {
			// console input handle
			cpoll_event& ev = events[num_ready++];
			ev.data.ptr = NULL;
			ev.events = FD_READ;
			for(int i = 0; i < cpi.size(); i++) {
				if(cpi[i].fd == 0) {
					ev.data.ptr = cpi[i].event.data.ptr;
					break;
				}
			}
			// if console service not startup, ignore this
			if(ev.data.ptr == NULL) {
				num_ready --;
				continue;
			}
			break;
		}
	}
    if(wsa_result != WSA_WAIT_TIMEOUT) {

        int e = wsa_result - WSA_WAIT_EVENT_0;
        for(cp_internal::size_type i = e; i < cpi.size() && num_ready < maxevents; ++i)
        {
            WSANETWORKEVENTS ne;
            if(WSAEnumNetworkEvents(cpi[i].fd, wsa_events[i], &ne) != 0) {

				// ignore stdin handle
				if(cpi[i].fd == 0)
					continue;
                // error?
                return -1;
			}

			cpoll_event& ev = events[num_ready++];
            if(ne.lNetworkEvents != 0) {

                if(cpi[i].event.events & CPOLLONESHOT)
                    cpi[i].event.events = 0;

				ev.events = (ne.lNetworkEvents & FD_ACCEPT) ? FD_CONNECT : 0;
				ev.events |= (ne.lNetworkEvents & FD_CLOSE) ? (FD_CLOSE | FD_READ) : 0;
				if(ne.lNetworkEvents & FD_READ)
					ev.events |= get_cp_mask(&ne);
				ev.data.ptr = cpi[i].event.data.ptr;
            } else {
				// empty event? add an dummy event
				ev.events = 0;
				ev.data.ptr = NULL;
			}
        }
    }
    return num_ready;
}

int cpoll_close(int cpfd)
{
	GUARD();
    if(cpfd < 1 || cp_data.find(cpfd) == cp_data.end())
        return ENOENT;

    cp_internal& cpi = cp_data[cpfd];
    for(cp_internal::size_type i = 0; i < cpi.size(); ++i) {

		// now unset the event notifications
		WSAEventSelect(cpi[i].fd, 0, 0);
		// clean up event
		WSACloseEvent(cpi[i].wsa_event);
	}
    cp_data.erase(cpfd);
    return 0;
}

void cpoll_cleanup()
{
	GUARD();
    WSACleanup();
	for(std::map<int, cp_internal>::iterator itr = cp_data.begin(); itr != cp_data.end(); ++itr)
		cpoll_close(itr->first);
    cp_data.clear();
}
