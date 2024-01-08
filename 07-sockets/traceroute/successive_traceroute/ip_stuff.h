// union of all types of socket address families
union common_sockaddr {
	struct sockaddr sa;		// common basic type	socket.h(7)
	struct sockaddr_in sin;		// AF_INET		ip(7)
	struct sockaddr_in6 sin6;	// AF_INET6		ipv6(7)
};
typedef union common_sockaddr sockaddr_any;


// get address of a host in newly allocated string
// how:
//	0 == noresolve (IP string)		ex.: "1.2.3.4"
//	1 == resolve (hostname and IP)		ex.: "foo.bar (1.2.3.4)"
//	2 == resolve (hostname only)		ex.: "foo.bar"
#define A2S_NORESOLVE	0
#define A2S_RESOLVE_HIP	1
#define A2S_RESOLVE_H	2
char *addr2str(sockaddr_any *sa, int how);

double get_time(void);
int traceroute_init(char *target_host, sa_family_t af);
int do_one_probe(int pn);
int print_one_probe(int pn);
int traceroute_free_mem_probe(int pn);

extern struct traceroute_info tr;
extern char *host_unknown;
extern char *host_unknown_bad_AF;

