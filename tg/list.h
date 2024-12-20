/**
 * File              : list.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 23.03.2022
 * Last Modified Date: 20.12.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/**
 * list.h
 * Copyright (c) 2024 Igor V. Sementsov <ig.kuzm@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Simple list functions
 *
 * list_t * list_new(); //create new list
 * void list_append( &list, "world", perror("append"));  // append item to list 
 * void list_prepend(&list, "hello", perror("prepend")); // add item to list at start
 * void list_insert( &list, "!", 2,  perror("insert")); // add item to index 2
 * 
 * // iterete the list:
 * char *str;
 * list_for_each(list, str){
 *		printf("%s ", str);	
 * } 
 */

#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct list {
	struct list *next;
	void *data;
} list_t;

//prepend list item
#define list_prepend(listp, item, on_error) \
({ \
	list_t *next = *listp; \
	list_t *new  = _list_alloc(on_error); \
	if (new){ \
		new->next = next; \
		new->data = item; \
		*listp = new; \
	} \
})

//append list item
#define list_append(listp, item, on_error) \
({ \
	list_t *l = *listp; \
	if (!l){ \
		l = _list_alloc(on_error); \
		l->data = item; \
		*listp = l; \
	} \
	else { \
		list_t *new = _list_alloc(on_error); \
		if (new){\
			new->data = item; \
			new->next = NULL; \
			while(l->next) { \
				l = l->next; \
			} \
			l->next = new; \
		}\
	}\
})

//return data value of item at index
#define list_at(list, index) \
({ \
	void *data = NULL; \
	list_t *l = list; \
	int i; \
	for(i=0;l; l=l->next, ++i) {\
		if (i == index){ \
			data = l->data;\
			break; \
		} \
	}\
	data; \
})

//remove item at index, return data value
#define list_remove(listp, index) \
({ \
	void *data = NULL; \
	list_t *l = *listp; \
	if (!l){ \
	} else if (index == 0 ){\
		data = l->data;\
		*listp = l->next;\
		free(l);\
	} else {\
		list_t *prev = NULL; \
		int i; \
		for(i=0; l; ++i) {\
			if (i == index) {\
				data = l->data;\
				prev->next = l->next; \
				free(l); \
				break; \
			}\
			prev = l; \
			l = l->next; \
		}\
	}\
	data; \
})

//insert item at index
#define list_insert(listp, item, index, on_error) \
({ \
	int isAdded = 0;\
	list_t *l = *listp; \
	list_t *prev = NULL; \
	if (index == 0 ){\
		list_prepend(listp, item, on_error);\
		isAdded = 1;\
	} else {\
		int i; \
		for(i=0; l; ++i) {\
			if (i == index) {\
				break; \
			}\
			prev = l; \
			l = l->next; \
		}\
		list_t *new = _list_alloc(on_error);\
		if (new){ \
			new->data = item;\
			new->next = l; \
			if (!prev) { \
				*listp = new; \
			} else { \
				prev->next = new; \
			} \
			isAdded = 1;\
		} \
	}\
	isAdded; \
})

//move item from index to index
#define list_move(listp, from, to, on_error) \
({ \
	void *data = list_remove(listp, from);\
	if (data) { \
		list_insert(listp, data, to, on_error);\
	} \
})

//foreach item at list
#define list_for_each(list, item) \
	list_t *__p = list; \
	for (item=__p->data; __p; __p=__p->next, item = __p?__p->data:NULL)

// get size of list
#define list_size(list) \
({ \
	int i = 0; \
	list_t *l = list; \
	while(l) {\
		l = l->next; \
		i++; \
	} \
	i; \
})

//allocate and init new LIST
#define list_new() NULL;
#define _list_alloc(on_error)\
({\
	list_t * l = malloc(sizeof(list_t));\
	if(!l){ \
		on_error; \
	} else { \
		l->next = NULL; l->data = NULL;\
	}\
	l;\
})

//free memory of LIST
#define list_free(list) \
({\
	list_t *l = list; \
	while(l) { \
		list_t * next = l->next; \
		free(l); \
		l = next; \
	} \
})

#endif // LIST_H
