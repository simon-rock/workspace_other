
#include "common.h"

#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 128 
typedef struct tagCfgItem
{
	char key[MAX_KEY_LEN + 1];
	char value[MAX_VAL_LEN + 1];
}CFGITEM;
CFGITEM* g_cfg_list = NULL;
int g_cfg_total = 0;
char g_cfg_file[256];
int g_cfg_save = 0;
int g_cfg_size = 256;

static int read_item(char* buf, char* key, char* value)
{
	key[0] = 0;
	value[0] = 0;

	trim_string(buf);
	if (buf[0] == '#' || buf[0] == ';')
	{
		return -1;
	}

	char* p = strchr(buf, '=');
	if (p == NULL)
	{
		return -1;
	}

	int n = p - buf;
	n = (n > MAX_KEY_LEN) ? MAX_KEY_LEN : n; 
	strncpy(key, buf, n);
	key[n] = 0;
	trim_string(key);
	
	strncpy(value, p + 1, MAX_VAL_LEN);
	value[MAX_VAL_LEN] = 0;
	trim_string(value);	

	return 0;
}

int cfg_open(char* file, int size)
{
	g_cfg_size = size;
	g_cfg_save = 0;
	strcpy(g_cfg_file, file);
	FILE* fp = fopen(file, "r");
	if (fp == NULL)
	{
		printf("can not read %s", file);
		return -1;
	}
	
	g_cfg_list = new CFGITEM[size];
	g_cfg_total = 0;
	char buf[1024];

	while (!feof(fp))
	{
		if (fgets(buf, 1024, fp) == NULL)
		{
			break;
		}	
		if (read_item(buf, g_cfg_list[g_cfg_total].key, g_cfg_list[g_cfg_total].value))
		{
			continue;
		}	
		g_cfg_total++;
		if (g_cfg_total > 250)
		{
			break;
		}
	}
	fclose(fp);
	return 0;
}

int cfg_get(int index, char* key, char* value)
{
	if (index < 0 || index >= g_cfg_total)
	{
		return -1;
	}
	strcpy(key, g_cfg_list[index].key);
	strcpy(value, g_cfg_list[index].value);
	return 0;
}

char* cfg_get(char* key, const char* pDefault)
{
	for (int i = 0; i < g_cfg_total; i++)
	{
		if (strcasecmp(key, g_cfg_list[i].key) == 0)
		{
			return g_cfg_list[i].value;
		}
	}	
	return (char*)pDefault;
}
int cfg_getn(char* key, int nDefault)
{
	for (int i = 0; i < g_cfg_total; i++)
	{
		if (strcasecmp(key, g_cfg_list[i].key) == 0)
		{
			return atoi(g_cfg_list[i].value);
		}
	
	}
	return nDefault;
}

