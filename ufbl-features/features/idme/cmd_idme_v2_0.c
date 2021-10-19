/*
 * Copyright (C) 2015 - 2019 Amazon.com Inc. or its affiliates.  All Rights Reserved.
*/

#ifdef SUPPORT_UBOOT
#include <malloc.h>
#include <common.h>
#include <config.h>
#include <libfdt.h>
#include <libfdt_env.h>
#include "ufbl_debug.h"
#else
#include <string.h>
#include <debug.h>
#ifndef UFBL_TESTS
#include <libfdt.h>
#endif
#endif

#include "idme.h"
#include "idme_default_table.h"
#if !defined(UFBL_PLATFORM_IMX) && !defined(UFBL_TESTS)
#include "fastboot.h"
#endif

#ifndef MIN
#define MIN(x,y) ((x) > (y)? (y):(x))
#endif

#define IDME_ITEM_INIT(pitem, limit, item, max_size, export, item_permission, item_value) \
	if((unsigned long)((char *)pitem + sizeof(struct idme_desc) + max_size) > (unsigned long)limit)\
	{\
		dprintf(CRITICAL, "When init item: %s.\n", item);\
		dprintf(CRITICAL, "The idme size is out of limit(0x%x).\n", CONFIG_IDME_SIZE);\
	}else{\
		memset(&(pitem->data[0]), 0, max_size); \
		memcpy(&(pitem->data[0]), item_value, MIN(max_size, strlen(item_value))); \
		memset(pitem->desc.name, 0, IDME_MAX_NAME_LEN); \
		memcpy(pitem->desc.name, item, MIN(IDME_MAX_NAME_LEN, strlen(item)));\
		pitem->desc.size = max_size;\
		pitem->desc.exportable = export;\
		pitem->desc.permission = item_permission;\
	}

#define IDME_ITEM_NEXT(curr_item) \
	curr_item = (struct item_t *)((char *)curr_item + sizeof(struct idme_desc) + curr_item->desc.size)

#ifdef IDME_UPDATE_TABLE
int idme_update_table_v2p0(void *data)
{
	struct idme_t *pidme = (struct idme_t *)data;
	struct item_t *pitem = (struct item_t *)(&(pidme->item_data[0]));
	struct idme_t *pnew_idme = NULL;
	struct item_t *pnew_item = NULL;
	unsigned int new_items_num = 0;
	char *new_idme_limit = NULL;
	const struct idme_init_values *pdflts = NULL;
	struct item_t *pitem_itr = NULL;
	char item_found = 0;
	int cnt_items_added = 0;
	int cnt_items_deleted = 0;
	int retval = -2;
	int data_presence  = idme_check_magic_number(pidme);

	if (data_presence) {
		if (data_presence == -2)
		dprintf(CRITICAL,
		"%s: Idme data not present (magic number error)\n", __func__);
		return -1;
	}
	dprintf(CRITICAL, "IDME table version %s\n", pidme->version);
	if( 0 != strncmp(pidme->version, IDME_DEFAULT_VERSION, strlen(IDME_DEFAULT_VERSION)) ) {
		dprintf(CRITICAL, "%s: Unrecognized IDME version\n", __func__);
		return -1;
	}

	pnew_idme = (struct idme_t*)(malloc(CONFIG_IDME_SIZE));
	if(!pnew_idme) {
		dprintf(CRITICAL, "%s: memory allocation error\n", __func__);
		return -1;
	}

	memset(pnew_idme, 0x00, CONFIG_IDME_SIZE);
	/* copy magic and version */
	memcpy(pnew_idme->magic, pidme->magic, sizeof(pidme->magic));
	memcpy(pnew_idme->version, pidme->version, sizeof(pidme->version));

	pnew_item = (struct item_t *)(pnew_idme->item_data);
	new_idme_limit = (char *)pnew_idme + CONFIG_IDME_SIZE;

	for (pdflts = idme_default_values; strlen(pdflts->desc.name); pdflts++) {
		unsigned int i;
		dprintf(SPEW, "%s: Looking for item: '%s'\n", __func__, pdflts->desc.name);
		item_found = 0;
		pitem_itr=pitem;

		for(i=0; i<pidme->items_num; i++) {
			if(!strncmp( pdflts->desc.name, pitem_itr->desc.name, IDME_MAX_NAME_LEN)) {
				dprintf(SPEW, "%s: %s ALREADY EXISTS in IDME Data on Device\n",
							__func__, pdflts->desc.name);
				item_found = 1;
				break;
			}
			IDME_ITEM_NEXT(pitem_itr);
		}

		if( item_found ) {

			// item exists in current idme AND in template: not new
			// Use current value
			IDME_ITEM_INIT(pnew_item, new_idme_limit, \
				(const char*)pitem_itr->desc.name, \
				pitem_itr->desc.size, \
				pitem_itr->desc.exportable, \
				pitem_itr->desc.permission, \
				(const char*)pitem_itr->data);

			// force copy all data, in case embedded nulls are in item data
			memcpy(pnew_item->data, pitem_itr->data, pitem_itr->desc.size);

			IDME_ITEM_NEXT(pnew_item);
		}
		else {
			// item only exists in template: new item
			// Use template value
			IDME_ITEM_INIT(pnew_item, new_idme_limit, \
				(const char*)pdflts->desc.name, \
				pdflts->desc.size, \
				pdflts->desc.exportable, \
				pdflts->desc.permission, \
				(const char*)pdflts->value);

			IDME_ITEM_NEXT(pnew_item);
			cnt_items_added++;
			dprintf(SPEW, "%s: Add item: '%s'\n", __func__, pdflts->desc.name);
		}
		new_items_num++;
	}

	cnt_items_deleted = pidme->items_num + cnt_items_added - new_items_num;

	if(cnt_items_added || cnt_items_deleted) {
		pnew_idme->items_num = new_items_num;
		memcpy(pidme, pnew_idme, CONFIG_IDME_SIZE);
		retval = 0;
	}
	free(pnew_idme);

	dprintf(SPEW, "%s: finished, items added: %d, items deleted: %d\n", __func__, cnt_items_added, cnt_items_deleted );
	return retval;
}
#endif

int idme_init_var_v2p0(void *data)
{
	struct idme_t *pidme = (struct idme_t *)data;
	char *idme_limit = (char *)pidme + CONFIG_IDME_SIZE;
	struct item_t *pitem = (struct item_t *)(&(pidme->item_data[0]));
	unsigned int items_num = 0;

	memset(data, 0, CONFIG_IDME_SIZE);
	memcpy(pidme->magic, IDME_MAGIC_NUMBER, strlen(IDME_MAGIC_NUMBER));
	memcpy(pidme->version, IDME_VERSION_2P1, strlen(IDME_VERSION_2P1));

	/* use default values to initialize idme data */
	const struct idme_init_values *ptr = &idme_default_values[0];

	while (strlen(ptr->desc.name)) {
		IDME_ITEM_INIT(pitem, idme_limit,
			ptr->desc.name,
			ptr->desc.size,
			ptr->desc.exportable,
			ptr->desc.permission,
			ptr->value);

		items_num++;
		ptr++;

		if (strlen(ptr->desc.name))
			IDME_ITEM_NEXT(pitem);
	}

	pidme->items_num = items_num;

	return 0;
}

int idme_get_var_v2p0(const char *name, char *buf, unsigned int length, void *data)
{
	int ret = -1;
	unsigned int i = 0;
	struct idme_t *pdata = (struct idme_t *)data;
	struct item_t *pitem = NULL;
	int data_presence  = idme_check_magic_number(pdata);

	if (data_presence) {
		if (data_presence == -2)
		dprintf(CRITICAL,
		"Error: Idme Data not present (magic number error)\n");
		return -1;
	}

	if (NULL == buf)
		return -1;

	pitem = (struct item_t *)(&(pdata->item_data[0]));
	for (i = 0; i < pdata->items_num; i++) {
		if ( 0 == strcmp(name, pitem->desc.name) ) {
			memcpy( buf, &(pitem->data[0]), MIN( pitem->desc.size, length ) );
			ret = 0;
			break;
		}else{
			IDME_ITEM_NEXT(pitem);
		}
	}



	return ret;
}

int idme_update_var_v2p0(const char *name, const char *value, unsigned int length, void *data)
{
	int ret = -1;
	unsigned int i = 0;
	struct idme_t *pdata = (struct idme_t *)data;
	struct item_t *pitem = NULL;
	int data_presence  = idme_check_magic_number(pdata);

	if (data_presence) {
		if (data_presence == -2)
		dprintf(CRITICAL,
		"Error: Idme Data not present (magic number error)\n");
		return -1;
	}

	if (value == NULL){
		dprintf(INFO, "warning: the idme value is null.\n");
		return -1;
	}

	pitem = (struct item_t *)(&(pdata->item_data[0]));
	for (i = 0; i < pdata->items_num; i++) {
		if ( 0 == strcmp(name, pitem->desc.name) ) {
			memset(&(pitem->data[0]), 0, pitem->desc.size);
			memcpy(&(pitem->data[0]), value, MIN( pitem->desc.size, length ) );
			ret = 0;
			break;
		}else{
			IDME_ITEM_NEXT(pitem);
		}
	}

	return ret;
}

struct idme_desc *idme_get_item_desc_v2p0(void *data, char *item_name)
{
	unsigned int i = 0;
	struct idme_t *pdata = (struct idme_t *)data;
	struct item_t *pitem = NULL;
	int data_presence = idme_check_magic_number(pdata);

	if (data_presence) {
		if (data_presence == -2)
		dprintf(CRITICAL,
		"Error: Idme Data not present (magic number error)\n");
		return NULL;
	}

	pitem = (struct item_t *)(&(pdata->item_data[0]));
	for (i = 0; i < pdata->items_num; i++) {
		if ( 0 == strcmp(item_name, pitem->desc.name) ) {
			return &(pitem->desc);
		}else{
			IDME_ITEM_NEXT(pitem);
		}
	}

	return NULL;
}

static char *idme_permission_to_string(int permission)
{
	int i;
	static char buffer[10];

	memset(buffer, 0, sizeof(buffer));
	memset(buffer, '-', sizeof(buffer) - 1);
	/* known number of iterations */
	for (i = 0; i < 3; i++) {
		if (permission & 0x01) buffer[8 - i*3]	= 'x';
		if (permission & 0x02) buffer[7 - i*3]	= 'w';
		if (permission & 0x04) buffer[6 - i*3]	= 'r';
		permission = permission>>3;
	}

	return buffer;
}

#ifdef UFBL_FASTBOOT_RESPONSE
int idme_print_var_v2p0(char *response, void *data)
#else
int idme_print_var_v2p0(void *data)
#endif
{
	unsigned int i;
	char temp[IDME_MAX_PRINT_SIZE + 1];
	struct idme_t *pdata = (struct idme_t *)data;
	struct item_t *pitem = NULL;
	int data_presence = 0;

	dprintf(INFO, "idme_print_var_v2p0\n");
	data_presence = idme_check_magic_number(pdata);
	if (data_presence) {
		if (data_presence == -2)
		dprintf(CRITICAL,
		"Error: Idme Data not present (magic number error)\n");
		return -1;
	}

	dprintf(INFO, "UFBL idme items number:%d\n", pdata->items_num);
	pitem = (struct item_t *)(&(pdata->item_data[0]));

	for(i = 0; i < pdata->items_num;i++){
		memset(temp, 0, sizeof(temp));
		memcpy(temp, &(pitem->data[0]), MIN(pitem->desc.size, IDME_MAX_PRINT_SIZE));
		dprintf(CRITICAL, "name: %20s, size: %3d, exportable: %d, "
				"permission: %s, data: [%s]\n",
			pitem->desc.name,
			pitem->desc.size,
			pitem->desc.exportable,
			idme_permission_to_string(pitem->desc.permission),
			temp);

		/* will print name on terminal */
		if (    (strcmp(pitem->desc.name, "manufacturing") != 0)
		     && (strcmp(pitem->desc.name, "mac_sec") != 0)
		     && (strcmp(pitem->desc.name, "unlock_code") != 0))
		{
			memset(temp, 0, sizeof(temp));
			snprintf(temp, sizeof(temp), "%s: ", pitem->desc.name);
#ifdef UFBL_FASTBOOT_RESPONSE
			fastboot_info(response, temp);
#else
#if !defined(UFBL_PROJ_ABC) && !defined (UFBL_PLATFORM_IMX) && !defined(UFBL_TESTS)
			fastboot_info(temp);
#endif
#endif
			memset(temp, 0, sizeof(temp));
			memcpy(temp, &(pitem->data[0]), MIN(pitem->desc.size, IDME_MAX_PRINT_SIZE));
#ifdef UFBL_FASTBOOT_RESPONSE
			fastboot_info(response, temp);
#else
#if !defined(UFBL_PROJ_ABC) && !defined (UFBL_PLATFORM_IMX) && !defined(UFBL_TESTS)
			fastboot_info(temp);
#endif
#endif
		}
		IDME_ITEM_NEXT(pitem);
	}

	return 0;
}

int idme_init_dev_tree_v2p1(void *fdt, void *data)
{
	int ret = 0;
#if defined(DEVICE_TREE_SUPPORT) || defined(CONFIG_OF_LIBFDT)
	int offset = 0, node_offset = 0;
	char buffer[IDME_MAX_NAME_LEN + 6 + 1];
	char *temp = NULL;
	struct idme_t *pidme = (struct idme_t *)data;
	struct item_t *pitem = (struct item_t *)&(pidme->item_data[0]);
	int count = pidme->items_num;
	uint32_t permission = 0;

	temp = (char *)(malloc(IDME_MAX_DT_PRINT_SIZE + 1));
	if(!temp) {
		dprintf(CRITICAL, "idme_init_dev_tree_v2p1: memory allocation error\n");
		ret = -1;
		goto done;
	}

	memset(buffer, 0, sizeof(buffer));

	ret = fdt_path_offset(fdt, "/idme");

	if (ret & FDT_ERR_NOTFOUND) {
		/* Create the IDME root node in FDT */
		if ((ret = fdt_path_offset(fdt, "/")) < 0) {
			dprintf(CRITICAL, "Unable to find root offset\n");
			goto done;
		}

		if ((ret = fdt_add_subnode(fdt, ret, "idme")) < 0) {
			dprintf(CRITICAL, "Unable to add IDME root node\n");
			goto done;
		}

		/* Get the offset of the new IDME root node */
		if ((ret = fdt_path_offset(fdt, "/idme")) < 0) {
			dprintf(CRITICAL,
				"Unable to find IDME root node offset\n");
			goto done;
		}

		offset = ret;

		while (count--) {
			if (!pitem->desc.exportable) {
				IDME_ITEM_NEXT(pitem);
				continue;
			}

			/* Copy the value string */
			memset(temp, 0, IDME_MAX_DT_PRINT_SIZE + 1);
			strncpy(temp, (char const *)(&(pitem->data[0])),
				MIN(pitem->desc.size, IDME_MAX_DT_PRINT_SIZE));

			/* Create the sub-node, and set the value */
			if ((ret = fdt_add_subnode(fdt,
					offset, pitem->desc.name)) < 0) {
				dprintf(CRITICAL,
					"Unable to add new IDME node: %s\n",
					pitem->desc.name);
				goto done;
			}

			snprintf(buffer, sizeof(buffer), "/idme/%s", pitem->desc.name);

			if ((ret = fdt_path_offset(fdt, buffer)) < 0) {
				dprintf(CRITICAL,
					"Unable to locate find new IDME node offset: %s\n",
					pitem->desc.name);
				goto done;
			}

			node_offset = ret;

			ret = fdt_setprop_string(fdt,
					node_offset, "value", temp);
			if (ret < 0) {
				dprintf(CRITICAL,
					"Unable to set IDME node (value): %s\n",
					pitem->desc.name);
				goto done;
			}

			permission = cpu_to_fdt32((uint32_t)pitem->desc.permission);

			if ((ret = fdt_setprop(fdt,
					node_offset, "permission",
					(void *) &permission,
					sizeof(uint32_t))) < 0)  {
				dprintf(CRITICAL,
					"Unable to set IDME node (permission): %s\n",
					pitem->desc.name);
				goto done;
			}

			IDME_ITEM_NEXT(pitem);
		}
		ret = 0;
	} else if (ret != 0) {
		dprintf(CRITICAL, "Error in searching IDME node in FDT\n");
	}
done:
	if (temp) {
		free(temp);
	}
#endif
	return ret;
}

int idme_init_atag_v2p0(void *atag_buffer, void *data, unsigned int size)
{
	struct idme_t *pidme = (struct idme_t *)data;
	struct idme_t *patag_idme = (struct idme_t *)atag_buffer;

	struct item_t *pitem = (struct item_t *)&(pidme->item_data[0]);
	int count = pidme->items_num;
	unsigned int item_size = 0;
	char *atag_limit = atag_buffer + size;
	int data_presence = idme_check_magic_number(pidme);

	if (data_presence) {
		if (data_presence == -2)
		dprintf(CRITICAL,
		"Error: Idme Data not present (magic number error)\n");
		return -1;
	}
	//init atag idme.
	memset(atag_buffer, 0, size);
	memcpy(patag_idme->magic, IDME_MAGIC_NUMBER, strlen(IDME_MAGIC_NUMBER));
	memcpy(patag_idme->version, IDME_VERSION_2P0, strlen(IDME_VERSION_2P0));
	patag_idme->items_num = 0;

	atag_buffer = (struct item_t *)&(patag_idme->item_data[0]);
	/* fill proc items */
	while(count--)
	{
		item_size = sizeof(struct idme_desc) + pitem->desc.size;
		if( (unsigned long)(atag_buffer + item_size) > (unsigned long)atag_limit ){
			dprintf(CRITICAL, "When init atag of idme item: %s.\n", pitem->desc.name);
			dprintf(CRITICAL, "The atag idme size is out of limit(0x%x).\n", size);
			return -1;
		}
		if(pitem->desc.exportable){
			memcpy(atag_buffer, (char *)pitem, item_size);
			patag_idme->items_num++;
			dprintf(INFO, "name: %s, size: %d, exportable: %d, permission: %d, data: %s\n", pitem->desc.name, pitem->desc.size, pitem->desc.exportable, pitem->desc.permission, &pitem->data[0]);
		}
		atag_buffer += item_size;
		IDME_ITEM_NEXT(pitem);
	}
	dprintf(INFO, "The atag idme items number:%d\n", patag_idme->items_num);
	return 0;
}
