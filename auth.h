/* RT/NTS -- a lightweight, high performance news transit server. */
/* 
 * Copyright (c) 2011-2013 River Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

#ifndef	NTS_AUTH_H
#define	NTS_AUTH_H

extern int	auth_enabled;
extern int	allow_unauthed;
extern int	insecure_auth;

int	auth_init(void);
int	auth_run(void);

int	 auth_check(char const *username, char const *password);
char	*auth_hash_password(char const *pw);

#endif	/* !NTS_AUTH_H */
