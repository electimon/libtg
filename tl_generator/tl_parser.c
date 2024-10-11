#include "tl_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct arg_t {
	char *name;
	char *type;
	int flag;
};

void print_method(const struct method_t *m)
{
	printf("METHOD: {\n");
	printf("\tname: %s\n", m->name);
	printf("\tid: %.8x\n", m->id);
	printf("\tret: %s\n", m->ret);
	printf("\targs: [\n");
	int i;
	for (i = 0; i < m->argc; ++i) {
		printf("\t\t{\n");	
		printf("\t\t\tname: %s\n", m->args[i].name);	
		printf("\t\t\ttype: %s\n", m->args[i].type);	
		printf("\t\t\tflag: %d\n", m->args[i].flag);	
		printf("\t\t}\n");	
	}
	printf("\t]\n");
	printf("}\n");
}

static int parse_method_args(
		struct arg_t *t,
		char *buf)
{
	t->flag = -1;
	if (strcmp(buf, "{X:Type}") == 0){
		t->type = strtok(buf, "{:");
		t->name = strtok(NULL, ":}");
		return 0;
	}

	t->name = strtok(buf, ":");
	char *ftype = strtok(NULL, ":"); 
	
	char *flag = strtok(ftype, "?");
	char *type = strtok(NULL, "?");
	if (type){
		// get flag number
		sscanf(flag, "flags.%d", &t->flag);
	} else
		type = ftype;

	t->type = type;
	return 0;
}

static int parse_method_id_and_args(
		struct method_t *m,
		char *buf, int hasFlags)
{
	int i;
	char *arg = NULL;
	for (arg = strtok(buf, " "), i=0;
			 arg;
			 arg = strtok(NULL, " "), i++) 
	{
		if (i==0 && !hasFlags)
			sscanf(arg, "%x", &m->id);
		else
			parse_method_args(
					(struct arg_t *)(&m->args[m->argc++]), 
					arg);
	}
	return 0;
}

static int parse_method(struct method_t *m, char *buf)
{
	// get method name
	char *methodName = strtok(buf, "#");
	if (!methodName)
		return 0;
	m->name = methodName;

	// get method id and flargs
	char *idAndFlags = strtok(NULL, "#");
	if (!idAndFlags)
		return 0;

	// check vector declaration
	if (strstr(idAndFlags, "{t:Type}")){
		char *arg = strtok(idAndFlags, " ");
		sscanf(arg, "%x", &m->id);
		m->ret = strtok(NULL, "{t:}");
		return 0;
	}
	
	// get args
	char *args = strtok(NULL, "#");
	if (args){
		parse_method_id_and_args(m, idAndFlags, 0);
		parse_method_id_and_args(m, args, 1);
	} else
		parse_method_id_and_args(m, idAndFlags, 0);

	return 0;
}

static int parse_method_return(struct method_t *m, char *buf)
{
	// get method return type
	char *ret = strtok(buf, " ;");
	if (!ret)
		return 1;
	m->ret = ret;
	return 0;
}

static int parse_schema(
		FILE *fp, void *userdata,
		int (*callback)(
			void *userdata,
			const struct method_t *m,
			const char *error))
{
	int i;
	char buf[BUFSIZ], *a;
	for(a = fgets(buf, BUFSIZ, fp), i=1;
			a;
			a = fgets(buf, BUFSIZ, fp), i++)
	{
		// skip empty lines
		if (!*buf || *buf == ' ' || *buf == '\n')
			continue;
		
		char *s = strdup(buf);
		if (!s)
			return i;
		
		// get method
		char *method = strtok(s, "=");
		if (!method)
			return i;
		
		// get return type
		char *ret = strtok(NULL, "="); 
		if (!ret) // skip this line - this is not declaration
			continue;

		struct method_t m;
		memset(&m, 0, sizeof(m));
		int err;
		err = parse_method_return(&m, ret);
		err = parse_method(&m, method);
		
		// drop simple types
		if (strrchr(m.name, '?'))
			continue;
		if (m.id == 0)
			continue;

		if (callback)
			if (callback(userdata, &m, NULL))
				break;
		
		free(s);
		if (err)
			return i;
	}

	return 0;
}

int tl_parse(
		const char *schema_file, 
		void *userdata,
		int (*callback)(
			void *userdata, 
			const struct method_t *m,
			const char *error))
{
	// open schema
	FILE *fp = fopen(schema_file, "r");
	if (!fp){
		if (callback)
			callback(userdata, NULL, "can't open file");
		return 1;
	}

	int err = parse_schema(
			fp, userdata, callback);
	fclose(fp);
	return err;
}
