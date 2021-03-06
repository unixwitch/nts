/* RT/NTS -- a lightweight, high performance news transit server. */
/* 
 * Copyright (c) 2011-2013 River Tarnell.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely. This software is provided 'as-is', without any express or implied
 * warranty.
 */

/*
 * msgcomp: process a message file (.m) and produce a header and source file
 * containing the messages.
 */

#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	"setup.h"

char const	*escape(char const *);

int
main(ac, av)
	char	**av;
{
FILE	*inf, *srcf, *hdrf;
char	*basenm, *srcnm, *hdrnm;
char	*p;
char	 line[1024];
char	*subsys;
int	 lineno = 1;
int	 msgnum = 0;
int	 facnum;

	if (!av[1]) {
		fprintf(stderr, "usage: %s <file>\n", av[0]);
		return 1;
	}

	if ((inf = fopen(av[1], "r")) == NULL) {
		perror(av[1]);
		return 1;
	}

	basenm = strdup(av[1]);
	if (p = rindex(basenm, '.'))
		*p = 0;

	if (p = rindex(basenm, '/'))
		basenm = p + 1;

	srcnm = malloc(strlen(basenm) + 3);
	sprintf(srcnm, "%s.c", basenm);
	hdrnm = malloc(strlen(basenm) + 3);
	sprintf(hdrnm, "%s.h", basenm);

	if ((srcf = fopen(srcnm, "w")) == NULL) {
		perror(srcnm);
		return 1;
	}
	fprintf(srcf, "/* Automatically generated by msgcomp %s */\n",
		PACKAGE_VERSION);
	fprintf(srcf, "#include \"msg.h\"\n");
	fprintf(srcf, "#include \"%s\"\n", hdrnm);

	if ((hdrf = fopen(hdrnm, "w")) == NULL) {
		perror(hdrnm);
		return 1;
	}
	fprintf(hdrf, "/* Automatically generated by msgcomp %s */\n",
		PACKAGE_VERSION);
	fprintf(hdrf, "#ifndef NTS__msgcomp_%s\n", basenm);
	fprintf(hdrf, "#define NTS__msgcomp_%s\n", basenm);
	fprintf(hdrf, "#include \"msg.h\"\n");

	/*
	 * First line should be the subsystem name.
	 */
	if (!fgets(line, sizeof(line), inf)) {
		if (ferror(inf))
			perror(av[1]);
		else
			fprintf(stderr, "\"%s\", line %d: unexpected end of file\n",
				av[1], lineno);
		goto err;
	}

	line[strlen(line) - 1] = 0;

	if (line[0] != '!' || !line[1]) {
		fprintf(stderr, "\"%s\", line %d: expected !<subsystem> <facility number>\n",
			av[1], lineno);
		goto err;
	}

	if ((p = index(line, ' ')) == NULL) {
		fprintf(stderr, "\"%s\", line %d: expected !<subsystem> <facility number>\n",
			av[1], lineno);
		goto err;
	}
	*p++ = 0;

	subsys = strdup(line + 1);
	facnum = atoi(p);

	fprintf(srcf, "msg_t %s_fac[] = {\n", subsys);
	fprintf(hdrf, "#define %s_facn (%d << 16)\n", subsys, facnum);
	fprintf(hdrf, "extern msg_t %s_fac[];\n", subsys);

	while (fgets(line, sizeof(line), inf)) {
	char	*sev, *msg;
	char	 helpln[1024];

		++lineno;
		line[strlen(line) - 1] = 0;

		if (!line[0] || line[0] == '#')
			continue;

		if ((sev = index(line, '\t')) == NULL) {
			fprintf(stderr, "\"%s\", line %d: syntax error: "
					"expected message definition\n",
				av[1], lineno);
			goto err;
		}
		*sev++ = 0;
		while (*sev == '\t')
			sev++;

		if ((msg = index(sev, '\t')) == NULL) {
			fprintf(stderr, "\"%s\", line %d: syntax error: "
					"expected message definition\n",
				av[1], lineno);
			goto err;
		}
		*msg++ = 0;

		fprintf(srcf, "{ \"%s\", \"%s\", '%c', \"%s\", \n",
			subsys, line, *sev, escape(msg));

		/* Now read the explanation message, terminated by '.' */
		while (fgets(helpln, sizeof(helpln), inf)) {
			++lineno;
			helpln[strlen(helpln) - 1] = 0;

			if (strcmp(helpln, ".") == 0)
				break;

			fprintf(srcf, " \"%s\\n\"\n", escape(helpln));
		}

		fprintf(srcf, "},\n");

		fprintf(hdrf, "#define M_%s_%s %d\n",
			subsys, line, msgnum);
		msgnum++;
	}

	if (ferror(inf)) {
		perror(av[1]);
		goto err;
	}

	fprintf(hdrf, "#endif\n");

	fprintf(srcf, "{ }\n");
	fprintf(srcf, "};\n");
	fclose(inf);
	fclose(srcf);
	fclose(hdrf);
	return 0;

err:
	unlink(hdrnm);
	unlink(srcnm);
	return 1;
}

char const *
escape(s)
	char const	*s;
{
char		*ret, *r;
char const	*p;
	ret = malloc(strlen(s) * 2 + 1);
	for (p = s, r = ret; *p; p++) {
		if (*p == '"' || *p == '\\')
			*r++ = '\\';
		*r++ = *p;
	}
	*r = 0;
	return ret;
}

