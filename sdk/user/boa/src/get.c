/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Some changes Copyright (C) 1996 Larry Doolittle <ldoolitt@jlab.org>
 *  Some changes Copyright (C) 1996,97 Jon Nelson <nels0988@tc.umn.edu>
 *  Some changes Copyright (C) 1998 Martin Hinner <martin@tdp.cz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "asp_page.h"
#include "syslog.h"
#include "boa.h"
#ifdef SERVER_SSL
#ifdef USES_MATRIX_SSL
#include <sslSocket.h>
#else /*!USES_MATRIX_SSL*/
#include <openssl/ssl.h>
#endif /*USES_MATRIX_SSL*/
#endif
#include "LINUX/mib.h"

/*
 * Name: init_get
 * Description: Initializes a non-script GET or HEAD request.
 *
 * Return values:
 *   0: finished or error, request will be freed
 *   1: successfully initialized, added to ready queue
 */

#ifdef SUPPORT_ASP

#if MINIMIZE_RAM_USAGE
int req_flush_retry(request * req) {
	int ret, retry_cnt = 0;

	do {
		ret = req_flush(req);
		if (ret == 1)
			return ret;
		retry_cnt ++;
		sleep(1);
	} while (retry_cnt < 5);
	return ret;
}
#endif // MINIMIZE_RAM_USAGE

int init_form(request * req)
{
	struct stat statbuf;
	SQUASH_KA(req);
	complete_env(req);

	fstat(req->post_data_fd, &statbuf);
	if (req->method == M_POST)
	{
//		char *buf;
//		buf=(char *)malloc(statbuf.st_size+1);
//		lseek(req->post_data_fd, SEEK_SET, 0);
		lseek(req->post_data_fd, 0, SEEK_END);
//		read(req->post_data_fd,buf,statbuf.st_size);
//		buf[statbuf.st_size]=0;
//		free(buf);
	}

	req->status = CLOSE;
	handleForm(req);
#if 0
	if(memcmp(req->buffer,"HTTP/1.0 301",12)==0)
	{
		return 0;
	}
#endif
	return 1;
}

int init_get2(request * req)
{
	int data_fd;
#ifdef GUNZIP
	char buf[MAX_PATH_LENGTH];
#endif
	struct stat statbuf;
	SQUASH_KA(req);

	complete_env(req);

	req->cgi_env[req->cgi_env_index] = NULL;     /* terminate cgi env */

	if(strstr(req->request_uri,".asp")==NULL && strstr(req->request_uri,"share.js")==NULL) {   
											 /* special handle, let share.js could apply multi-language */
		init_form(req);
		return 1;
	}

#if defined(ACCOUNT_CONFIG) && defined(MULTI_USER_PRIV)
	if (getAccPriv(req->user) != (int)PRIV_ROOT && strstr(req->request_uri, "userconfig.asp")) {
		boaRedirect( req, "/admin/user-password.asp");
		return 0;
	}
#endif

	//ql--check upload_sc.asp
	/*if (strstr(req->request_uri, "upload_sc.asp")) {
		//disableModemLine
		va_cmd("/bin/adslctrl", 1, 1, "DisableModemLine");
	}*/

	data_fd = open(req->pathname, O_RDONLY);

	if (data_fd == -1) {		/* cannot open */
#ifdef GUNZIP
		sprintf(buf, "%s.gz", req->pathname);
		data_fd = open(buf, O_RDONLY);
		if (data_fd == -1) {
#endif
			int errno_save = errno;
			log_error_doc(req);
			errno = errno_save;
#if 0
			perror("document open");
#endif
			syslog(LOG_ERR, "Error opening %s for %s: %s\n", req->pathname,
					req->remote_ip_addr, strerror(errno_save));

			errno = errno_save;

			if (errno == ENOENT)
				send_r_not_found(req);
			else if (errno == EACCES)
				send_r_forbidden(req);
			else
				send_r_bad_request(req);
			return 0;
#ifdef GUNZIP
		}
		close(data_fd);

		req->response_status = R_REQUEST_OK;
		if (!req->simple) {
			req_write(req, "HTTP/1.0 200 OK-GUNZIP\r\n");
			print_http_headers(req);
			print_content_type(req);
			print_last_modified(req);
			req_write(req, "\r\n");
			req_flush(req);
		}
		if (req->method == M_HEAD)
			return 0;
		if (req->pathname)
			free(req->pathname);
		req->pathname = strdup(buf);
		return init_cgi(req);	/* 1 - OK, 2 - die */
#endif
	}
	fstat(data_fd, &statbuf);

	if (S_ISDIR(statbuf.st_mode)) {		/* directory */
		close(data_fd);			/* close dir */

		if (req->pathname[strlen(req->pathname) - 1] != '/') {
			char buffer[3 * MAX_PATH_LENGTH + 128];

			if (server_port != 80)
				sprintf(buffer, "http://%s:%d%s/", req->host?req->host:server_name, server_port,
						req->request_uri);
			else
				sprintf(buffer, "http://%s%s/", req->host?req->host:server_name, req->request_uri);
			send_redirect_perm(req, buffer);
			return 0;
		}
		data_fd = get_dir(req, &statbuf);	/* updates statbuf */

		if (data_fd == -1)		/* couldn't do it */
			return 0;			/* errors reported by get_dir */
		else if (data_fd == 0)
			return 1;
	}
//start modify here : tony
#if 0
	if (req->if_modified_since &&
		!modified_since(&(statbuf.st_mtime), req->if_modified_since)) {
		send_r_not_modified(req);
		close(data_fd);
		return 0;
	}
#endif

	req->filesize = statbuf.st_size;
//	req->last_modified = statbuf.st_mtime;

	if (req->method == M_HEAD) {
		send_r_request_ok(req);
		close(data_fd);
		return 0;
	}
	/* MAP_OPTIONS: see compat.h */
	req->data_mem = mmap(0, req->filesize+10,
#ifdef USE_NLS
			PROT_READ|PROT_WRITE
#else
			PROT_READ
#endif
			, MAP_OPTIONS,data_fd, 0);
	close(data_fd);				/* close data file */

	if ((long) req->data_mem == -1) {
		boa_perror(req, "mmap");
		return 0;
	}

	send_r_request_ok(req);		/* All's well */

#if 0
	{
		int bob;

		bob = BUFFER_SIZE - req->buffer_end;
		if (bob > 0) {
			if (bob > req->filesize - req->filepos)
				bob = req->filesize - req->filepos;
			memcpy(req->buffer + req->buffer_end,
					req->data_mem + req->filepos,
					bob);
			req->buffer_end += bob;
			req->filepos += bob;
		}
	}
#else
	{
		//parse and send asp page
		unsigned char *left,*right,*last_right=req->data_mem;
		int bob;

		while(1)
		{
			left=strstr(last_right,"<%");
			if(left!=NULL) right=strstr(left,"%>");

			if((left!=NULL) && (right!=NULL))
			{
				bob=(unsigned int)left-(unsigned int)last_right;
				while((bob+req->buffer_end)>(req->max_buffer_size))
				{
					int ret;
                                        #if MINIMIZE_RAM_USAGE
					int byte_to_send;

					// andrew, try not to alloc more memory
					byte_to_send = req->max_buffer_size - req->buffer_end;
					if (byte_to_send > bob)
						byte_to_send = bob;

					memcpy(req->buffer + req->buffer_end,	req->data_mem + req->filepos, byte_to_send);
					req->buffer_end += byte_to_send;
					req->filepos += byte_to_send;
					bob -= byte_to_send;

					ret = req_flush_retry(req);
					if (ret!=1) fprintf(stderr, "flush1(%d)\n", ret);
                                        #else // MINIMIZE_RAM_USAGE
                                        ret=allocNewBuffer(req);
					if(ret==FAILED) {bob=req->max_buffer_size- req->buffer_end; break;}
                                        #endif // MINIMIZE_RAM_USAGE

				}
				if(bob>=0)
				{
					memcpy(req->buffer + req->buffer_end,	req->data_mem + req->filepos, bob);
					last_right=right+2;
					req->buffer_end += bob;
					req->filepos += (bob+(unsigned int)last_right-(unsigned int)left);
					handleScript(req,left,right);
				}
			}
			else
			{

				bob=(unsigned int)req->data_mem+req->filesize-(unsigned int)last_right;
				while((bob+req->buffer_end)>req->max_buffer_size)
				{
					int ret;
                                        #if MINIMIZE_RAM_USAGE
					int byte_to_send;

					byte_to_send = req->max_buffer_size - req->buffer_end;
					if (byte_to_send > bob)
						byte_to_send = bob;

					memcpy(req->buffer + req->buffer_end,	req->data_mem + req->filepos, byte_to_send);
					req->buffer_end += byte_to_send;
					req->filepos += byte_to_send;
					bob -= byte_to_send;

					ret = req_flush_retry(req);
					if (ret!=1) fprintf(stderr, "flush2(%d)\n", ret);
                                        #else // MINIMIZE_RAM_USAGE
					ret=allocNewBuffer(req);
					if(ret==FAILED) { bob=req->max_buffer_size- req->buffer_end;break; }
                                        #endif // MINIMIZE_RAM_USAGE
				}
				if (bob > 0)
				{
					memcpy(req->buffer + req->buffer_end,	req->data_mem + req->filepos,	bob);
					req->buffer_end += bob;
					req->filepos += bob;
				}
				break;
			}

		}
	}
#endif

	freeAllTempStr();
	
	if (req->filepos == req->filesize) {
		req->status = CLOSE;
		return 0; /* done! */
	}

	/* We lose statbuf here, so make sure response has been sent */
	return 1;
}
#endif

int init_get(request * req)
{
	int data_fd;
#ifdef GUNZIP
	char buf[MAX_PATH_LENGTH];
#endif
	struct stat statbuf;
	// kaotest
	int homepage;

	req->cgi_env[req->cgi_env_index] = NULL;     /* terminate cgi env */

	data_fd = open(req->pathname, O_RDONLY);

	if (data_fd == -1) {		/* cannot open */
#ifdef GUNZIP
		sprintf(buf, "%s.gz", req->pathname);
		data_fd = open(buf, O_RDONLY);
		if (data_fd == -1) {
#endif
			int errno_save = errno;
			log_error_doc(req);
			errno = errno_save;
#if 0
			perror("document open");
#endif
			syslog(LOG_ERR, "Error opening %s for %s: %s\n", req->pathname,
					req->remote_ip_addr, strerror(errno_save));

			errno = errno_save;

			if (errno == ENOENT)
				send_r_not_found(req);
			else if (errno == EACCES)
				send_r_forbidden(req);
			else
				send_r_bad_request(req);
			return 0;
#ifdef GUNZIP
		}
		close(data_fd);

		req->response_status = R_REQUEST_OK;
		if (!req->simple) {
			req_write(req, "HTTP/1.0 200 OK-GUNZIP\r\n");
			print_http_headers(req);
			print_content_type(req);
			print_last_modified(req);
			req_write(req, "\r\n");
			req_flush(req);
		}
		if (req->method == M_HEAD)
			return 0;
		if (req->pathname)
			free(req->pathname);
		req->pathname = strdup(buf);
		return init_cgi(req);	/* 1 - OK, 2 - die */
#endif
	}
	fstat(data_fd, &statbuf);

	// kaotest
	homepage = 0;
	if (S_ISDIR(statbuf.st_mode)) {		/* directory */
		close(data_fd);			/* close dir */

		if (req->pathname[strlen(req->pathname) - 1] != '/') {
			char buffer[3 * MAX_PATH_LENGTH + 128];

			if (server_port != 80)
				sprintf(buffer, "http://%s:%d%s/", req->host?req->host:server_name, server_port,
						req->request_uri);
			else
				sprintf(buffer, "http://%s%s/", req->host?req->host:server_name, req->request_uri);
			send_redirect_perm(req, buffer);
			return 0;
		}
		// kaotest
		else
			homepage = 1;
		data_fd = get_dir(req, &statbuf);	/* updates statbuf */

		if (data_fd == -1)		/* couldn't do it */
			return 0;			/* errors reported by get_dir */
		else if (data_fd == 0)
			return 1;
	}
	// kaotest
	//if (req->if_modified_since &&
	if (req->if_modified_since && !homepage &&
		!modified_since(&(statbuf.st_mtime), req->if_modified_since)) {
		send_r_not_modified(req);
		close(data_fd);
		return 0;
	}
	req->filesize = statbuf.st_size;
	req->last_modified = statbuf.st_mtime;

	if (req->method == M_HEAD) {
		send_r_request_ok(req);
		close(data_fd);
		return 0;
	}
	/* MAP_OPTIONS: see compat.h */
	req->data_mem = mmap(0, req->filesize+10,
#ifdef USE_NLS
			PROT_READ|PROT_WRITE
#else
			PROT_READ
#endif
			, MAP_OPTIONS,data_fd, 0);
	close(data_fd);				/* close data file */

	if ((long) req->data_mem == -1) {
		boa_perror(req, "mmap");
		return 0;
	}
	send_r_request_ok(req);		/* All's well */
	{
		int bob;
#ifdef SUPPORT_ASP
		bob = req->max_buffer_size - req->buffer_end;
#else
		bob = BUFFER_SIZE - req->buffer_end;
#endif
		if (bob > 0) {
			if (bob > req->filesize - req->filepos)
				bob = req->filesize - req->filepos;
			memcpy(req->buffer + req->buffer_end,
					req->data_mem + req->filepos,
					bob);
			req->buffer_end += bob;
			req->filepos += bob;
		}
	}

	if (req->filepos == req->filesize) {
		req->status = CLOSE;
		return 0; /* done! */
	}

	/* We lose statbuf here, so make sure response has been sent */
	return 1;
}

/*
 * Name: process_get
 * Description: Writes a chunk of data to the socket.
 *
 * Return values:
 *  -1: request blocked, move to blocked queue
 *   0: EOF or error, close it down
 *   1: successful write, recycle in ready queue
 */

int process_get(request * req)
{
	int bytes_written, bytes_to_write;

	bytes_to_write = req->filesize - req->filepos;
#ifdef USE_NLS
	if (req->method != M_HEAD)
	{
		if (bytes_to_write>BYTES_TO_WRITE)
			bytes_to_write = BYTES_TO_WRITE;

		if (req->bytesconverted<req->filepos)
			req->bytesconverted = req->filepos;

		if (req->cp_table)
		{
			nls_convert(req->data_mem + req->bytesconverted,req->cp_table,
					req->filepos + bytes_to_write - req->bytesconverted );
			req->bytesconverted = req->filepos + bytes_to_write;
		}
	}
#endif

#ifdef SERVER_SSL
	if(req->ssl == NULL){
#endif /*SERVER_SSL*/
	bytes_written = write(req->fd, req->data_mem + req->filepos,
						  bytes_to_write);
#ifdef SERVER_SSL
	}else{
#ifdef USES_MATRIX_SSL
		int	mtrx_status;
		if (bytes_to_write  >SSL_MAX_PLAINTEXT_LEN)
			bytes_to_write = SSL_MAX_PLAINTEXT_LEN;
		bytes_written = sslWrite(req->ssl, req->data_mem + req->filepos, bytes_to_write,&mtrx_status);
#else /*!USES_MATRIX_SSL*/
		bytes_written = SSL_write(req->ssl, req->data_mem + req->filepos, bytes_to_write);
#endif /*USES_MATRIX_SSL*/
#if 0
		printf("SSL_write\n");
#endif /*0*/
	}
#endif /*SERVER_SSL*/
	if (bytes_written == -1) {
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return -1;			/* request blocked at the pipe level, but keep going */
		else {
			if (errno != EPIPE) {
				log_error_doc(req);	/* Can generate lots of log entries, */
#if 0
				perror("write");	/* OK to disable if your logs get too big */
#endif
			}
			return 0;
		}
	}
	req->filepos += bytes_written;
	if (req->filepos == req->filesize)	/* EOF */
		return 0;
	else
		return 1;				/* more to do */
}

/*
 * Name: get_dir
 * Description: Called from process_get if the request is a directory.
 * statbuf must describe directory on input, since we may need its
 *   device, inode, and mtime.
 * statbuf is updated, since we may need to check mtimes of a cache.
 * returns:
 *  -1 error
 *  0  cgi (either gunzip or auto-generated)
 *  >0  file descriptor of file
 */

int get_dir(request * req, struct stat *statbuf)
{

	char pathname_with_index[MAX_PATH_LENGTH];
	int data_fd;

	sprintf(pathname_with_index, "%s%s", req->pathname, directory_index);

	data_fd = open(pathname_with_index, O_RDONLY);

	if (data_fd != -1) {		/* user's index file */
		strcat(req->request_uri, directory_index); /* for mimetype */
		fstat(data_fd, statbuf);
		return data_fd;
	}
	if (errno == EACCES) {
		send_r_forbidden(req);
		return -1;
	}
#ifdef GUNZIP
	strcat(pathname_with_index, ".gz");
	data_fd = open(pathname_with_index, O_RDONLY);
	if (data_fd != -1) {		/* user's index file */
		close(data_fd);

		req->response_status = R_REQUEST_OK;
		if (req->method == M_HEAD) {
			req_write(req, "HTTP/1.0 200 OK-GUNZIP\r\n");
			print_http_headers(req);
			print_last_modified(req);
			req_write(req, "Content-Type: ");
			req_write(req, get_mime_type(directory_index));
			req_write(req, "\r\n\r\n");
			req_flush(req);
			return 0;
		}
		if (req->pathname)
			free(req->pathname);
		req->pathname = strdup(pathname_with_index);
		if (init_cgi(req) == 0)
			return -1;
		return 0;				/* in this case, 0 means success */
	} else
#endif

	if (dirmaker != NULL) {
		req->response_status = R_REQUEST_OK;

		/* the indexer should take care of all headers */
		if (!req->simple || req->method == M_HEAD) {
			req_write(req, "HTTP/1.0 200 OK\r\n");
			print_http_headers(req);
			print_last_modified(req);
			req_write(req, "Content-Type: ");
			req_write(req, get_mime_type(directory_index));
			req_write(req, "\r\n\r\n");
			req_flush(req);
		}
		if (req->method == M_HEAD)
			return 0;

		return (init_cgi(req) == 0 ? -1 : 0);
		/* in this case, 0 means success */
	}
	send_r_forbidden(req);
	return -1;					/* nothing worked */
}
