#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "node_linked_list.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

/* ------------------- Buffer Manager Interface Pool Handling ----------------------------*/

/*
 initBufferPool creates a new buffer pool with numPages page frames using the page replacement strategy strategy.
 The pool is used to cache pages from the page file with name pageFileName.
 Initially, all page frames should be empty.
 The page file should already exist, i.e., this method should not generate a new page file.
 stratData can be used to pass parameters for the page replacement strategy.
 */

struct node * start;
FILE *fpAEB = NULL;

RC initBufferPool(BM_BufferPool * const bm, const char * const pageFileName,
		const int numPages, ReplacementStrategy strategy, void *stratData) {

	bm->pageFile = pageFileName;
	bm->numPages = numPages;
	bm->strategy = strategy;
	bm->mgmtData = stratData;

	fpAEB = fopen(pageFileName, "rb+");

	intialize_linked_list();

	display_linked_list();

	return RC_OK;

}

/*
 shutdownBufferPool destroys a buffer pool.
 This method should free up all resources associated with buffer pool.
 For example, it should free the memory allocated for page frames. If the buffer pool contains any dirty pages, then these pages should be written back to disk before destroying the pool. It is an error to shutdown a buffer pool that has pinned pages.
 */

RC shutdownBufferPool(BM_BufferPool * const bm) {

	 printf("-------------[START]------[SHUT - DOWN - POOL]----------------\n");
	 bool isDirtyBit;

	 struct node *temp = start;
	 do
	 {
	 if(temp->fix_count!=0)
	 return RC_WRITE_FAILED;
	 else
	 {
	 if (temp->dirty_bit == TRUE)
	 {
	 forceFlushPool(bm);
	 isDirtyBit = TRUE;
	 }
	 }
	 temp=temp->next;
	 }while(temp!=start);

	 display_linked_list();

	 if(!isDirtyBit){
		 printf("Free Done");
		 free(temp);
	 }


	 printf("-------------[FREE AND END]------[SHUT - DOWN - POOL]----------------\n");

	return RC_OK;
}
/*
 forceFlushPool causes all dirty pages (with fix count 0) from the buffer pool to be written to disk.
 */

RC forceFlushPool(BM_BufferPool * const bm) {

	 printf("-------------[START]------[FORCE - FLUSH - POOL]----------------\n");

	 SM_FileHandle fHandle;
	 fHandle.fileName = bm->pageFile;

	 struct node *temp = start;
	 do
	 {
	 if(temp->dirty_bit == TRUE && temp->fix_count == 0){
	 writeBlock1(temp->data,&fHandle, (SM_PageHandle)temp->bm_ph.data);
	 temp->dirty_bit = FALSE;
	 }
	 temp=temp->next;
	 }while(temp!=start);

	 display_linked_list();

	 printf("-------------[END]------[FORCE - FLUSH - POOL]----------------\n");

	return RC_OK;

}

/* ------------------- Buffer Manager Interface Access Pages ----------------------------*/

/*markDirty marks a page as dirty.*/

RC markDirty(BM_BufferPool * const bm, BM_PageHandle * const page) {

	printf("-------------[START]------[MARK - DIRTY]-------{Page Contents are %s}---------\n",page->data);

	struct node* temp = start;

	int i = 0;

	do {
		if (i < n) {
			if (temp->bm_ph.pageNum == page->pageNum) {
				temp->dirty_bit = TRUE;
				break;
			} else {
				i = i + 1;
				temp = temp->next;
			}
		} else
			break;
	} while (temp != NULL);

	display_linked_list();
	printf("-------------[END]------[MARK - DIRTY]----------------\n");
	return RC_OK;
}

/*
 unpinPage unpins the page page. The pageNum field of page should be used to figure out which page to unpin.
 */

RC unpinPage(BM_BufferPool * const bm, BM_PageHandle * const page) {

	printf("-------------[START]------[UN - PIN - PAGE]----{and PAGE CONTENT IS %s}------------\n",page->data);

	int pageNum = page->pageNum;

	struct node *frameToUnpinPage;
	frameToUnpinPage = start;

	if (frameToUnpinPage == NULL) {
		return RC_BM_POOL_INIT_ERROR;
	} else {
		do {
			if (frameToUnpinPage->data == pageNum){
				frameToUnpinPage->fix_count = 0;
				if (frameToUnpinPage->dirty_bit) {
					frameToUnpinPage->bm_ph.data = page->data ;
					//printf("TESTINGGGG DATAAAAAAAAa [%s]\n",frameToUnpinPage->bm_ph.data);
					int status = forcePage(bm, page);
					if(status == 0){
						printf("Innn");
						frameToUnpinPage->dirty_bit = FALSE;
					}
				}
				if (frameToUnpinPage->fix_count < 0) {
					return RC_BM_CANNOT_UNPIN_A_PAGE;
					break;
				}
				break;
			}
			else{
			frameToUnpinPage = frameToUnpinPage->next;
			}
		} while (frameToUnpinPage != start);

		display_linked_list();

		printf("-------------[END]------[UN - PIN - PAGE]----------------\n");

		return RC_OK;
	}
}

/*
 forcePage should write the current content of the page back to the page file on disk.
 */

RC forcePage(BM_BufferPool * const bm, BM_PageHandle * const page) {

	 printf("-------------[START]------[FORCE - PAGE]--%s--------------\n",(SM_PageHandle)page->data);

	 SM_FileHandle fHandle;
	 fHandle.fileName = bm->pageFile;

	 if(bm == NULL){
	 return RC_BM_POOL_INIT_ERROR;
	 }
	 else{
	 int statusofWriteBlock = writeBlock1(page->pageNum,&fHandle,(SM_PageHandle)page->data);
	 printf("Status of Write Block is %d \n",statusofWriteBlock);
	 printf("-------------[END]------[FORCE - PAGE]----------------\n");
	 return RC_OK;
	 }

}

/*
 pinPage pins the page with page number pageNum.
 The buffer manager is responsible to set the pageNum field of the page handle passed to the method.
 Similarly, the data field should point to the page frame the page is stored in (the area in memory storing the content of the page).
 */
RC pinPage(BM_BufferPool * const bm, BM_PageHandle * const page,
		const PageNumber pageNum) {

	printf("-------------[START]------[PIN - PAGE]----------------\n");

	SM_FileHandle fHandle;
	//printf("Page data is %s \n",page->data);
	//page->data = malloc(4096);
	//printf("Page data After is %s \n",page->data);
	page->pageNum = pageNum;

	int hitOrFault = search_linked_list(pageNum);

	if (hitOrFault) {
		printf("HIT!!\n");
	} else {
		printf("FAULT!!! CALL PAGE REPLACEMENT ALGORITHM !!!\n");
		insertFrame_linked_list(bm, pageNum);
		display_linked_list();
	}

	struct node* temp = return_frame_linked_list(pageNum);
	/*printf("Temp Data = [%s] and BMPH Data = [%d] \n", temp->bm_ph.data,
			temp->bm_ph.pageNum);*/

	fHandle.fileName = bm->pageFile;
	printf("hhhhhh[%s]\n", fHandle.fileName);

	int statusofEnsureCapacity = ensureCapacity1(pageNum + 1, &fHandle);
	printf("Status from Ensure Capacity is [%d]\n", statusofEnsureCapacity);

	int statusofReadBlock = readBlock1(pageNum, &fHandle,
			(SM_PageHandle) temp->bm_ph.data);
	printf("Status from read block is [%d]\n", statusofReadBlock);

	page->pageNum = pageNum;
	page->data = temp->bm_ph.data;


	if (hitOrFault == 1) {
		temp->fix_count = 1;
	}

	printf("-------------[END]------[PIN - PAGE]----------------\n");

	return RC_OK;
}

PageNumber *getFrameContents(BM_BufferPool * const bm) {

	return 0;
}
bool *getDirtyFlags(BM_BufferPool * const bm) {

	return FALSE;
}
int *getFixCounts(BM_BufferPool * const bm) {

	return 0;
}

RC ensureCapacity1 (int numberOfPages, SM_FileHandle *fHandle){

		int i, j, bytesWritten;

		 // Open the file in Append mode to append an empty block at the file end.
		if (fpAEB == NULL) {
			return RC_FILE_NOT_FOUND;
		}

		int seekStatus = fseek(fpAEB, 0, SEEK_END); // Move the cursor to the end postion to append an empty block.

		int ctr = 0;
		if (seekStatus == -1) {
			return RC_SEEK_FAILED;
		}

		int TOTAL_BLOCKS = getTotalNumberOfBlocks1(fpAEB) + 1; // Since the block counting starts from zero , increase the total by 1.
		int ADD_BLOCKS = numberOfPages - TOTAL_BLOCKS; // Get the number of blocks needed to be appended at the end.

		if (ADD_BLOCKS <= 0) {
			printf("Sufficient Not Adding \n");
			return RC_OK; // If the number is less than zero , it has sufficient capacity and hence no need to append blocks.
		} else {
			printf("Need To ADD!");
			//Need to ensure capacity by adding ADD_BLOCKS Blocks.
			for (i = 1; i <= ADD_BLOCKS; i++) {

				for (ctr = 0 ; ctr < PAGE_SIZE ; ctr ++)
					fprintf (fpAEB, "%c", '\0');
					}

				/*char *buffer[PAGE_SIZE]; //  Create a Buffer anf instantiate a memory of 4096 Bytes to it.
				memset(buffer, '\0', PAGE_SIZE);

				bytesWritten = fwrite(buffer, 1, PAGE_SIZE, fpAEB);*/
			}

			int seekStatus1 = fseek(fpAEB, 0, SEEK_END); // Move the cursor to the end postion to append an empty block.
			if (seekStatus1 == -1) {
				return RC_SEEK_FAILED;
			}

			fHandle->mgmtInfo = fpAEB;
			/*
			if (bytesWritten < PAGE_SIZE) {
				return RC_WRITE_FAILED; // Return error if the number of bytes written are less than PAGE_SIZE.
			} else
				return RC_OK;
*/
			return RC_OK;
		}


RC readBlock1 (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	FILE *fpRB = fpAEB; // Open the file in Read mode to read the pageNumth block.
		if (fpRB == NULL) {
			return RC_FILE_NOT_FOUND; // Return error if file does not exists.
		}

		int seekStatus = fseek(fpRB, pageNum * PAGE_SIZE, SEEK_SET); // Seek to the initial position to read the pageNumth block.
		if (seekStatus == -1) {
			return RC_SEEK_FAILED; // Return error if failed to move to the pageNumth position.
		}

		//memPage = malloc(PAGE_SIZE); // Allocate PAGE_SIZE memory to Page Handler.
		int bytesRead = fread(memPage, PAGE_SIZE, 1 , fpRB); // Read the blocks from File Handler to Page Handler.

		fHandle->mgmtInfo = fpRB;

		if (bytesRead != 1) {
					printf("Bytes Read are :- %d \n", bytesRead);
					return RC_READ_FAILED;  // Read failed to read PAGE_SIZE bytes.
				} else
					return RC_OK;

}

RC writeBlock1 (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	int bytesWritten;

	FILE *fpWB = fpAEB; // Open the file in Write mode to write the pageNumth block.
	if (fpWB == NULL) {
		return RC_FILE_NOT_FOUND;  // Return error if file does not exists.
	}

	int TOTAL_BLOCKS = getTotalNumberOfBlocks1(fpWB); // Find the total Number of Blocks in the given file.(Block Number starts with 0)
	printf("TOTAL_BLOCKS=%d\n",TOTAL_BLOCKS);

	int seekStatus = fseek(fpWB, PAGE_SIZE * pageNum, SEEK_SET); // Seek to the initial position to write the pageNumth block.
	if (seekStatus == -1) {
		return RC_SEEK_FAILED;
	}

	//printf("MEMPAGE:-\n", memPage);
	//memPage = malloc(PAGE_SIZE);
	bytesWritten = fwrite(memPage, PAGE_SIZE,1, fpWB); //Write the bytes from Page Handler to File Handler.
	fHandle->mgmtInfo = fpWB;

	printf("Bytes Write=ten [%d]\n", bytesWritten);

	if (bytesWritten !=1) {
		return RC_WRITE_FAILED; // Return error if the number of bytes written are less than PAGE_SIZE.
	} else
		return RC_OK;

}

int getTotalNumberOfBlocks1(FILE *fpNOB) {
	struct stat status;
	fstat(fileno(fpNOB), &status);
	/*Get the file size using built-in function fstat()*/
	int FILE_SIZE = status.st_size;
	int numberOfBlocks;
	if (FILE_SIZE <= PAGE_SIZE) {
		numberOfBlocks = 0; // If the file size is less than zero , it will be the 0th Block.
	} else {
		numberOfBlocks = (FILE_SIZE / PAGE_SIZE) - 1;
	}

	return numberOfBlocks;
}
