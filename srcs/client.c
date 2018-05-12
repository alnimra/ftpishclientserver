/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mray <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/03/03 20:07:49 by mray              #+#    #+#             */
/*   Updated: 2018/03/03 20:07:51 by mray             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include "ftp.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

void create_and_bind_to_host_ip(int *sock, struct sockaddr_in *serv_addr,
								char *host_ip, int port)
{
	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Creation of socket failed\n");
		exit(1);
	}
	(*serv_addr).sin_family = AF_INET;
	(*serv_addr).sin_port = htons(port);
	if (inet_pton(AF_INET, host_ip, &((*serv_addr).sin_addr)) <= 0)
	{
		printf("Ip Address is not valid\n");
		exit(1);
	}
}

void send_to_ftp_server_socket(int sock, char *msg, char *append)
{
	char *telnet_msg;
	char *free_holder;

	telnet_msg = ft_strjoin(msg, append);
	free_holder = telnet_msg;
	telnet_msg = ft_strjoin(telnet_msg, " \r\n");
	free(free_holder);
	// printf("Client sending to server socket - %d | msg - %s", sock,
	// telnet_msg);
	send(sock, telnet_msg, ft_strlen(telnet_msg), 0);
	free(telnet_msg);
}
void clear_buf(char buf[1024])
{
	int i;

	i = -1;
	while (++i < 1024)
		buf[i] = '\0';
}
char *read_user_input()
{
	char  buf[2];
	char *user_input;
	char *tmp;

	user_input = ft_strnew(1);
	buf[1] = '\0';
	while (read(0, buf, 1) > 0)
	{
		tmp = user_input;
		user_input = ft_strjoin(user_input, buf);
		free(tmp);
		if (buf[0] == '\n')
			break;
	}
	return user_input;
}
int main(int argc, char **argv)
{
	struct sockaddr_in serv_addr;
	int				   sock;
	char *			   msg;
	char			   buf[1024];
	int				   client_isconnected;

	msg = argv[3];
	client_isconnected = 1;
	sock = 0;
	if (argc != 3)
	{
		printf("USAGE ./server [HOST IP] [VALID PORT]\n");
		exit(1);
	}
	create_and_bind_to_host_ip(&sock, &serv_addr, argv[1], atoi(argv[2]));
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Conection to host failed");
		return (0);
	}

	send_to_ftp_server_socket(sock, "", "test");
	read(sock, buf, 1024);
	printf("%s\n", buf);
	clear_buf(buf);
	while (client_isconnected)
	{
		ft_putstr("ftp> ");
		send_to_ftp_server_socket(sock, read_user_input(), "");
		read(sock, buf, 1024);
		printf("%s\n", buf);
		clear_buf(buf);
	}
	return (0);
}
