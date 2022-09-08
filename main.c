/* No idea which ones are needed*/
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>

unsigned short in_cksum(unsigned short *addr, int len)
{
	int sum = 0;
	for (int i = 0; i < len / 2; i++)
		sum += addr[i];
	if (len % 2)
		sum += *(uint8_t *)(addr + len / 2);
	return 0xffff - sum;
}

struct __attribute__((__packed__)) packet {
	struct iphdr ip;
	struct icmphdr icmp;
};

int main() {
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	socklen_t len;
	// Creating and validating socket
	//sockfd = socket(AF_INET, SOCK_STREAM, 0);
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	int option_value = 1;
	//https://stackoverflow.com/questions/22116873/set-socket-option-is-why-so-important-for-a-socket-ip-hdrincl-in-icmp-request
	setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &option_value, sizeof(option_value));

	if (sockfd == -1)
		exit(1);

	struct iphdr ip;
	//https://www.winlab.rutgers.edu/~zhibinwu/html/c_prog.htm complete ping example
	//https://en.wikipedia.org/wiki/IPv4#Packet_structure
	ip.version = 4;
	ip.ihl = 5; //Internet Header Length, size of the header in words, so 4 bytes, minumum is 5, so 5 x 4 bytes = 20 bytes 
				// it's a 4-bit field, so maximum is 15, or 60 bytes
	ip.tos = 0; // https://en.wikipedia.org/wiki/Type_of_service very messy
	ip.tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
	ip.id = 0; //mainly for fragments, we don't do fragments
	ip.frag_off = 0; //fragment offset
	ip.ttl = 64; //time to live in seconds // "In practice, the field is used as a hop count" what a mess
	ip.protocol = IPPROTO_ICMP;
	ip.saddr = htonl(0x0a00020f); //hardcoded this computer
	ip.daddr = htonl(0x08080808);
	ip.check = in_cksum((unsigned short *)&ip, sizeof(struct iphdr));

	struct icmphdr icmp;

	icmp.type = ICMP_ECHO;
	icmp.code = 0; // subtype, echo request only has one code, 0
	icmp.un.echo.id = random();
	icmp.un.echo.sequence = 0;
	icmp.checksum = in_cksum((unsigned short *)&icmp, sizeof(struct icmphdr));

	struct packet packet;

	packet.ip = ip;
	packet.icmp = icmp;

	struct sockaddr_in connection;

	connection.sin_family = AF_INET;
	connection.sin_addr.s_addr = ip.daddr;

	sendto(sockfd, &packet, ip.tot_len, 0, (const struct sockaddr*)&connection, sizeof(connection));
}
