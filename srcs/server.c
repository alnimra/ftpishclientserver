/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mray <marvin@42.fr>                        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/03/03 19:44:58 by mray              #+#    #+#             */
/*   Updated: 2018/03/03 19:45:01 by mray             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ftp.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

void create_and_bind_socket(int *fd, struct sockaddr_in *addr, int port)
{
	if ((*fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		printf("Creation of socket failed\n");
		exit(1);
	}
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = INADDR_ANY;
	addr->sin_port = htons(port);
	if (bind(*fd, (struct sockaddr *)addr, sizeof(*addr)) < 0)
	{
		printf("Binding of socket to %d port failed\n", port);
		exit(1);
	}
}

void listen_and_accept_connections(int *fd, struct sockaddr_in *addr, int *sock,
								   int port)
{
	int addr_len;
	int backlog;

	backlog = 10;
	addr_len = sizeof(*addr);
	if (listen(*fd, backlog) < 0)
	{
		printf("Unable to listen\n");
		exit(1);
	}
	printf("Listening on port %d\n", port);
	if ((*sock = accept(*fd, (struct sockaddr *)addr,
						(socklen_t *)&(addr_len))) < 0)
		printf("Could not accept the connection\n");
}

void send_to_ftp_client_socket(int sock, char *msg, char *append)
{
	char *telnet_msg;
	char *free_holder;

	telnet_msg = ft_strjoin(msg, append);
	free_holder = telnet_msg;
	telnet_msg = ft_strjoin(telnet_msg, " \r\n");
	free(free_holder);
	// printf("Server sending to client socket - %d | msg - %s", sock,
	// telnet_msg);
	send(sock, telnet_msg, ft_strlen(telnet_msg), 0);
	free(telnet_msg);
}

int parse_port(char *buf)
{
	char **command_args;
	char **port_info;
	char * port_str;

	command_args = ft_strsplit(buf, ' ');
	port_info = ft_strsplit(command_args[1], ',');
	free(command_args);
	port_str = ft_strjoin(port_info[4], port_info[5]);
	free(port_info);
	free(port_str);
	return (ft_atoi(port_str));
}

void init_passive_mode_data_transmission(int port_num);

char *read_file(int fd)
{
	char  buf[2];
	char *user_input;
	char *tmp;

	user_input = ft_strnew(1);
	buf[1] = '\0';
	while (read(fd, buf, 1) > 0)
	{
		tmp = user_input;
		user_input = ft_strjoin(user_input, buf);
		free(tmp);
	}
	return user_input;
}

int get_file_size(int fd)
{

	struct stat s;
	int			status;

	status = fstat(fd, &s);
	return (s.st_size);
}

char *path_to_pidfile(int pid) {
	return ft_strjoin("/tmp/", ft_itoa(pid));
}
char *get_ls_output(char *client_path)
{
	pid_t child_id;
	int   fd;
	char *ret;
	char *filename;

	child_id = fork();
	filename = path_to_pidfile(child_id);
	if (child_id == 0)
	{
		filename = path_to_pidfile(getpid());
		fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0777);
		if (fd < 0)
			ft_putstr("Parent: Error opening file\n");
		ft_putstr("Child: ");
		ft_putendl(filename);
		execl("/bin/cd", "/bin/cd", client_path, (char *)0);
		close(1);
		dup(fd);
		execl("/bin/ls", "/bin/ls", "-l", (char *)0);
		close(fd);
		exit(0);
	}
	wait(NULL);
	fd = open(filename, O_RDWR);
	if (fd < 0)
		ft_putstr("Parent: Error opening file\n");
	ret = mmap(0, get_file_size(fd), PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	unlink(filename);
	return ret;
}
char *create_new_path_from_a_cd(char *client_path, char *cd_dir)
{
	char *slash_dir;
	char *new_path;

	slash_dir = ft_strjoin(client_path, "/");
	new_path = ft_strjoin(slash_dir, cd_dir);
	free(slash_dir);
	return new_path;
}
int do_cd(char **client_path, char *cd_dir)
{
	pid_t child_id;
	int   fd;
	char *new_path;
	char *filename;

	child_id = fork();
	filename = path_to_pidfile(child_id);
	new_path = create_new_path_from_a_cd(*client_path, cd_dir);
	if (child_id == 0)
	{
		filename = path_to_pidfile(getpid());
		fd = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0777);
		if (fd < 0)
			ft_putstr("Parent: Error opening file\n");
		ft_putstr("Child: ");
		ft_putendl(filename);
		close(1);
		dup(fd);
		execl("/bin/cd", "/bin/cd", new_path, (char *)0);
		close(fd);
		exit(0);
	}
	wait(NULL);
	fd = open(filename, O_RDWR);
	if (fd < 0)
		ft_putstr("Parent: Error opening file\n");
	ret = mmap(0, get_file_size(fd), PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	unlink(filename);
	return 1;
}

void clear_buf(char buf[1024])
{
	int i;

	i = -1;
	while (++i < 1024)
		buf[i] = '\0';
}

int main(int argc, char **argv)
{
	int				   fd;
	int				   sock;
	struct sockaddr_in addr;
	char			   buf[1024];
	int				   num;
	int				   listen_and_accept;
	int				   handle_client;
	char*					client_path;

	listen_and_accept = 1;
	handle_client = 1;
	fd = 0;

	client_path = ft_strnew(1024);
	getcwd(client, 1024);
	if (argc != 2)
	{
		printf("USAGE ./server [VALID PORT]\n");
		exit(1);
	}
	create_and_bind_socket(&fd, &addr, atoi(argv[1]));
	while (listen_and_accept)
	{
		listen_and_accept_connections(&fd, &addr, &sock, atoi(argv[1]));
		printf("Recieved from Client\n");
		send_to_ftp_client_socket(sock, "220: Service ready for new user.", "");
		clear_buf(buf);
		while (handle_client)
		{
			printf("Listening for Client...\n");
			if ((num = read(sock, buf, 1024)) == -1)
				printf("Receive Error\n");
			else if (num == 0)
			{
				printf("Client Disconnected...\n");
				break;
			}
			buf[num] = '\0';
			printf("Recieved: %s\n", buf);
			if (strncmp(buf, "PWD", 3) == 0)
			{
				getcwd(buf, 1024);
				printf("Current Working Directory is: %s\n", buf);
				send_to_ftp_client_socket(
					sock, "257: Current Working Directory:  ", buf);
			}
			else if (strncmp(buf, "PORT", 4) == 0)
			{
				printf("%d\n", parse_port(buf));
				send_to_ftp_client_socket(sock, "200 ", "");
			}
			else if (strncmp(buf, "LIST", 4) == 0)
				send_to_ftp_client_socket(sock, "125 ", get_ls_output());
			else if (strncmp(buf, "CD", 2) == 0)
				send_to_ftp_client_socket(sock, "125 ", get_ls_output());
			else if (strncmp(buf, "PASV", 4) == 0)
				send_to_ftp_client_socket(sock, "227 ", "");
			else
				send_to_ftp_client_socket(sock, "503 Bad sequence of commands",
										  "");
			clear_buf(buf);
		}
	}
	return (0);
}
