/*
 * lib/des425/read_passwd.c
 *
 * Copyright 1985,1986,1987,1988,1991 by the Massachusetts Institute
 * of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * This routine prints the supplied string to standard
 * output as a prompt, and reads a password string without
 * echoing.
 */

#if !defined(_WIN32)

#include "des_int.h"
#include "des.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#ifndef ECHO_PASSWORD
#include <termios.h>
#endif /* ECHO_PASSWORD */

static jmp_buf pwd_jump;

static krb5_sigtype intr_routine (int);

static krb5_sigtype
intr_routine(signo)
    int signo;
{
    longjmp(pwd_jump, 1);
    /*NOTREACHED*/
}

/* This is re-declared here because des.h might not declare it. */
int KRB5_CALLCONV des_read_pw_string(char *, int, char *, int);
static int des_rd_pwstr_2prompt(char *, int, char *, char *);

/*** Routines ****************************************************** */
static int
des_rd_pwstr_2prompt(return_pwd, bufsize_in, prompt, prompt2)
    char *return_pwd;
    int bufsize_in;
    char *prompt;
    char *prompt2;
{
    char *volatile readin_string = 0;
    register char *ptr;
    int scratchchar;
    krb5_sigtype (*volatile ointrfunc)();
    int errcode;
    size_t bufsize = bufsize_in;
#ifndef ECHO_PASSWORD
    struct termios echo_control, save_control;
    int fd;

    /* get the file descriptor associated with stdin */
    fd=fileno(stdin);

    if (tcgetattr(fd, &echo_control) == -1)
	return errno;

    save_control = echo_control;
    echo_control.c_lflag &= ~(ECHO|ECHONL);
    
    if (tcsetattr(fd, TCSANOW, &echo_control) == -1)
	return errno;
#endif /* ECHO_PASSWORD */

    if (setjmp(pwd_jump)) {
	errcode = -1;		/* we were interrupted... */
	goto cleanup;
    }
    /* save intrfunc */
    ointrfunc = signal(SIGINT, intr_routine);

    /* put out the prompt */
    (void) fputs(prompt,stdout);
    (void) fflush(stdout);
    (void) memset(return_pwd, 0, bufsize);

    if (fgets(return_pwd, bufsize_in, stdin) == NULL) {
	(void) putchar('\n');
	errcode = -1;
	goto cleanup;
    }
    (void) putchar('\n');
    /* fgets always null-terminates the returned string */

    /* replace newline with null */
    if ((ptr = strchr(return_pwd, '\n')))
	*ptr = '\0';
    else /* flush rest of input line */
	do {
	    scratchchar = getchar();
	} while (scratchchar != EOF && scratchchar != '\n');

    if (prompt2) {
	/* put out the prompt */
	(void) fputs(prompt2,stdout);
	(void) fflush(stdout);
	readin_string = malloc(bufsize);
	if (!readin_string) {
	    errcode = ENOMEM;
	    goto cleanup;
	}
	(void) memset((char *)readin_string, 0, bufsize);
	if (fgets((char *)readin_string, bufsize_in, stdin) == NULL) {
	    (void) putchar('\n');
	    errcode = -1;
	    goto cleanup;
	}
	(void) putchar('\n');

	if ((ptr = strchr((char *)readin_string, '\n')))
	    *ptr = '\0';
        else /* need to flush */
	    do {
		scratchchar = getchar();
	    } while (scratchchar != EOF && scratchchar != '\n');
	    
	/* compare */
	if (strncmp(return_pwd, (char *)readin_string, bufsize)) {
	    errcode = -1;
	    goto cleanup;
	}
    }
    
    errcode = 0;
    
cleanup:
    (void) signal(SIGINT, ointrfunc);
#ifndef ECHO_PASSWORD
    if ((tcsetattr(fd, TCSANOW, &save_control) == -1) &&
	errcode == 0)
	    return errno;
#endif
    if (readin_string) {
	    memset((char *)readin_string, 0, bufsize);
	    krb5_xfree(readin_string);
    }
    if (errcode)
	    memset(return_pwd, 0, bufsize);
    return errcode;
}

int KRB5_CALLCONV
des_read_password(k,prompt,verify)
    mit_des_cblock *k;
    char *prompt;
    int	verify;
{
    int ok;
    char key_string[BUFSIZ];

    ok = des_read_pw_string(key_string, sizeof(key_string), prompt, verify);
    if (ok == 0)
	des_string_to_key(key_string, *k);

    memset(key_string, 0, sizeof (key_string));
    return ok;
}

int KRB5_CALLCONV
des_read_pw_string(s, max, prompt, verify)
    char *s;
    int max;
    char *prompt;
    int	verify;
{
    int ok;
    char prompt2[BUFSIZ];

    if (verify) {
	strcpy(prompt2, "Verifying, please re-enter ");
	strncat(prompt2, prompt, sizeof(prompt2)-(strlen(prompt2)+1));
	prompt2[sizeof(prompt2)-1] = '\0';
    }
    ok = des_rd_pwstr_2prompt(s, max, prompt, verify ? prompt2 : 0);
    return ok;
}

#else /* !unix */
/*
 * These are all just dummy functions to make the rest of the library happy...
 */
#endif /* _WINDOWS */
