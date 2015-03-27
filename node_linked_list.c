/*
 * node_linked_list.c
 *
 *  Created on: Mar 23, 2015
 *      Author: VJonnala and Team
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include "node_linked_list.h"
#include "dberror.h"

struct node * start;

RC intialize_linked_list() {
	printf("In initialize method");
	struct node *new_node;
	int i;
	//start = NULL;
	for (i = 0; i < n; i++) {
		new_node = (struct node *) malloc(sizeof(struct node));
		if (new_node == NULL) {
			printf("Node creation failed");
			return RC_BM_POOL_INIT_ERROR;

		}
		new_node->data = -1;     // -99 = NULL or Empty Characcter
		new_node->next = NULL;
		new_node->bm_ph.pageNum = -1;
		new_node->bm_ph.data = NULL;
		new_node->dirty_bit=FALSE;
		new_node->fix_count=0;
		if (start == NULL || start->data!=-1) {
			start = new_node;
			current = new_node;
		} else {
			current->next = new_node;
			current = new_node;
		}
	}
	current->next = start;

	return RC_OK;
}

void display_linked_list() {

	struct node *new_node = start;

	printf("\n ----[DISPLAY METHOD]---Printing list Start------- \n");
	int i = 0;

	do {
		if (i < n) {
			printf("VALUE = [%d] and FRAME [%d] and Mark Dirty Bit [%d] and Fix Count [%d] Page Data [%s] \n",new_node->data, i,new_node->dirty_bit,new_node->fix_count,new_node->bm_ph.data);
			i = i + 1;
			new_node = new_node->next;
		} else
			break;
	} while (new_node != NULL);

	printf("\n ----[DISPLAY METHOD]---Printing list End------- \n");

}

struct node* return_frame_linked_list(int pageNumber) {

	struct node* temp = start;

	int i = 0;

	do {
		if (i < n) {
			if(temp->data == pageNumber){
				temp->bm_ph.data = malloc(4096);
				return temp;
				break;
			}
			else{
			i = i + 1;
			temp = temp->next;
			}
		} else
			break;
	} while (temp != NULL);

return temp;
}


int search_linked_list(int pageNumber) {

	printf("In Search for page number :- %d\n", pageNumber);
	struct node *new_node = start;
	struct node *temp = NULL;

	bool found = false;
	int i = 0;
	do {
		if (i < n) {
			if (new_node->data == pageNumber) {
				found = true;
				break;
			} else {
				temp = new_node;
				new_node = new_node->next;
			}
			i = i + 1;
		} else {
			break;
		}
	} while (new_node != NULL);

	if (true == found) {
		return 1;
	} else {
		return 0;
	}

}

void insertFrame_linked_list(BM_BufferPool * const bm, int pageNumber) {

	struct node *temp = (struct node*) bm->mgmtData;
	if (temp == NULL) {
		printf("NULL");
		temp = start;
		temp->bm_ph.pageNum = pageNumber;
		temp->data = pageNumber;
		temp->bm_ph.data = NULL;
		temp = temp->next;
		bm->mgmtData = temp;
	} else {
		temp->bm_ph.pageNum = pageNumber;
		temp->data = pageNumber;
		temp->bm_ph.data = NULL;
		temp = temp->next;
		bm->mgmtData = temp;
	}

}
