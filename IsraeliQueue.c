#include <stdlib.h>
#include <stdio.h>
#include <IsraeliQueue.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define UNINITIALIZED -1
#define CANNOT_FIND -2
#define ENQUEUE_FAILED -3

typedef enum { FRIEND, RIVAL, NEUTRAL } Relation;

typedef struct{
	int m_friendshipQuota;
	int m_rivalryQuota;
	void* m_object;
	bool improved;
} Quota;

FriendshipFunction* copyFriendshipFunctions(FriendshipFunction* friendshipFunctions, IsraeliQueueError* errorFlag){
	int size=0;
	while (friendshipFunctions[size]!=NULL){
		size++;
	}
	FriendshipFunction* newFriendshipFunctions = malloc((size+1)*sizeof(FriendshipFunction));
	if (newFriendshipFunctions==NULL){
	#ifndef NDEBUG
	printf("copyFriendshipFunctions: malloc failed in copyFriendshipFunctions");
	#endif
		return NULL;
	}
	for (int i=0; i<size; i++){
		newFriendshipFunctions[i]=friendshipFunctions[i];
	}
	newFriendshipFunctions[size]=NULL;
	return newFriendshipFunctions;

}
Quota* copyQuotas(Quota* quotas, int size, IsraeliQueueError* errorFlag){
	Quota* newQuotas = malloc(size*sizeof(Quota));
	if (newQuotas==NULL){
	#ifndef NDEBUG
	printf("copyQuotas: malloc failed in copyQuotas");
	#endif
		*errorFlag=ISRAELIQUEUE_ALLOC_FAILED;
		return NULL;
	}
	for (int i=0; i<size; i++){
		newQuotas[i]=quotas[i];
	}
	return newQuotas;
}

struct IsraeliQueue_t {
	int m_friendshipThreshold;
	int m_rivalryThreshold;
	FriendshipFunction* m_friendshipFunctions;
	ComparisonFunction m_compare;
	void** m_objects;
	int m_size;
	Quota* m_quotas;
};


Relation checkRelation(FriendshipFunction* friendshipFunctions,
 void* firstObject, void* secondObject, int friendshipThreshold, int rivalryThreshold ){
	
	Relation currentRelation=NEUTRAL;
	int sumOfFriendshipPoints=0;
	int count=0;
	while (friendshipFunctions!=NULL && *friendshipFunctions!=NULL){
		if ((*friendshipFunctions)(firstObject, secondObject)>friendshipThreshold){
			currentRelation=FRIEND;
			return currentRelation;
		}
		sumOfFriendshipPoints+=(*friendshipFunctions)(firstObject, secondObject);
		count++;
		friendshipFunctions++;
	}
	double averageFriendshipPoints=0;

	if (count!=0){
	averageFriendshipPoints=(double)sumOfFriendshipPoints/count;
	}
	if (averageFriendshipPoints<(double)rivalryThreshold){
		currentRelation=RIVAL;
		return currentRelation;
	}
	assert(currentRelation==NEUTRAL);
	return currentRelation;

	}

void** copyObjectArray(void** objects, int size, IsraeliQueueError* errorFlag){
	void** newObjects = malloc(size*sizeof(void*));
	if (newObjects==NULL){
	#ifndef NDEBUG
	printf("copyObjectArray: malloc failed in copyObjectArray");
	#endif
		*errorFlag=ISRAELIQUEUE_ALLOC_FAILED;
		return NULL;
	}
	for (int i=0; i<size; i++){
		newObjects[i]=objects[i];
	}
	return newObjects;
}


IsraeliQueue IsraeliQueueCreate(FriendshipFunction *friendshipFunctions,
 								ComparisonFunction compare, int friendshipThreshold, int rivalryThreshold)
{
	if (friendshipFunctions == NULL || compare == NULL){
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: friendshipFunctions or compare is NULL");
	#endif

		return NULL;
	}
	IsraeliQueue newQueue = malloc(sizeof(*newQueue));
	if (newQueue == NULL){
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		return NULL;
	}
	IsraeliQueueError errorFlag=ISRAELIQUEUE_SUCCESS;
	newQueue->m_friendshipThreshold = friendshipThreshold;
	newQueue->m_rivalryThreshold = rivalryThreshold;
	newQueue->m_friendshipFunctions = copyFriendshipFunctions(friendshipFunctions, &errorFlag);
	if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		free(newQueue);
		return NULL;
	}
	newQueue->m_compare = compare;
	newQueue->m_objects = malloc(sizeof(void *));
	if (newQueue->m_objects == NULL){
	#ifndef NDEBUG
		printf("IsraeliQueueCreate: malloc failed in IsraeliQueueCreate");
	#endif
		free(newQueue);
		return NULL;
	}

	newQueue->m_objects[0] = NULL;
	newQueue->m_size = 1;
	newQueue->m_quotas = NULL;
	return newQueue;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue queue){
	
	if (queue==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueClone: queue is NULL");
	#endif

	return NULL;
	}
	IsraeliQueueError errorFlag=ISRAELIQUEUE_SUCCESS;

	if (queue->m_size==1){
		IsraeliQueue newQueue=IsraeliQueueCreate(queue->m_friendshipFunctions, queue->m_compare,
		 queue->m_friendshipThreshold, queue->m_rivalryThreshold);
		 if (newQueue==NULL){
			#ifndef NDEBUG
			printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
			#endif
			return NULL;
		 }
		 return newQueue;
	}

	IsraeliQueue newQueue = malloc(sizeof(*newQueue));
	if (newQueue==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
	#endif
		return NULL;
	}
	newQueue->m_friendshipThreshold=queue->m_friendshipThreshold;
	newQueue->m_rivalryThreshold=queue->m_rivalryThreshold;
	newQueue->m_friendshipFunctions=copyFriendshipFunctions(queue->m_friendshipFunctions, &errorFlag);
	newQueue->m_compare=queue->m_compare;
	newQueue->m_objects=copyObjectArray(queue->m_objects, queue->m_size, &errorFlag);
	newQueue->m_quotas=copyQuotas(queue->m_quotas, queue->m_size, &errorFlag);
	newQueue->m_size=queue->m_size;
	
	if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){
	#ifndef NDEBUG
	printf("IsraeliQueueClone: malloc failed in IsraeliQueueClone");
	#endif
		free(newQueue);
		return NULL;
	}
	return newQueue;
}


void IsraeliQueueDestroy(IsraeliQueue queue){
	if (queue==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueDestroy: queue is NULL");
	#endif
		return;
	}

	if (queue->m_objects!=NULL){
		free(queue->m_objects);
	}
	if (queue->m_quotas!=NULL){
		free(queue->m_quotas);
	}

	if (queue->m_friendshipFunctions!=NULL){
		free(queue->m_friendshipFunctions);
	}
		free(queue);
	return;
}


void checkInsertLocation(IsraeliQueue queue, void* object, int *friendLocation,
														 bool *skippedToFriend){
	bool blocked=false;
		for (int i=0; i<((queue->m_size)-1); i++){
	*friendLocation=UNINITIALIZED;
	*skippedToFriend=false, blocked=false;
		if(checkRelation(queue->m_friendshipFunctions, queue->m_objects[i],
		 				object, queue->m_friendshipThreshold, queue->m_rivalryThreshold)==FRIEND){
			if (queue->m_quotas[i].m_friendshipQuota<FRIEND_QUOTA){
				for(int j=i+1; j<((queue->m_size)-1);  j++){
					if (checkRelation(queue->m_friendshipFunctions, queue->m_objects[j],
					 object, queue->m_friendshipThreshold, queue->m_rivalryThreshold)==RIVAL){
						if (queue->m_quotas[j].m_rivalryQuota<RIVAL_QUOTA){
							queue->m_quotas[j].m_rivalryQuota++;
							i=j;
							blocked=true; //j is the index of the current blocking rival
							break;
						}
					}
				}	
					if (!(blocked)){
					*friendLocation=i; 
					*skippedToFriend=true;
					queue->m_quotas[*friendLocation].m_friendshipQuota++;
					break;
					}
			}
		}
	}		
}


void insertObject(IsraeliQueue queue, void* object, int* friendLocation, bool* skippedToFriend, bool improving){
	if (*skippedToFriend){
		for (int i=queue->m_size-1; i>*friendLocation; i--){
			queue->m_objects[i]=queue->m_objects[i-1];
			queue->m_quotas[i]=queue->m_quotas[i-1];
		}
		queue->m_objects[*friendLocation+1]=object;
		queue->m_quotas[*friendLocation+1].m_friendshipQuota=0;
		queue->m_quotas[*friendLocation+1].m_rivalryQuota=0;
		queue->m_quotas[*friendLocation+1].m_object=object;
		if (!improving){
			queue->m_quotas[*friendLocation+1].improved=false;
		}
	}
	else{
		queue->m_objects[queue->m_size-2]=object;
		queue->m_quotas[queue->m_size-2].m_friendshipQuota=0;
		queue->m_quotas[queue->m_size-2].m_rivalryQuota=0;
		queue->m_quotas[queue->m_size-2].m_object=object;
		if (!improving){
			queue->m_quotas[queue->m_size-2].improved=false;
		}
	}
		queue->m_quotas[queue->m_size-1].m_object=NULL;

}


IsraeliQueueError resizeQueue(IsraeliQueue queue){
	void *tempPtr=queue->m_objects;
	queue->m_objects=realloc(queue->m_objects, (queue->m_size+1)*sizeof(void*));
	if (queue->m_objects==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		free (tempPtr);
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	queue->m_size+=1;
	queue->m_objects[queue->m_size-1]=NULL;
	tempPtr=queue->m_quotas;
	queue->m_quotas=realloc(queue->m_quotas, (queue->m_size+1)*sizeof(Quota));
	if (queue->m_quotas==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueEnqueue: realloc failed in IsraeliQueueEnqueue");
	#endif
		free (tempPtr);
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	return ISRAELIQUEUE_SUCCESS;
}

int enqueueAndGetIndex(IsraeliQueue queue, void* object, IsraeliQueueError* errorFlag){
	int friendLocation=UNINITIALIZED;
	bool skippedToFriend=false;

	checkInsertLocation(queue, object, &friendLocation, &skippedToFriend);
	if (resizeQueue(queue)==ISRAELIQUEUE_ALLOC_FAILED){					//Resizing
		return ENQUEUE_FAILED;
		*errorFlag=ISRAELIQUEUE_ALLOC_FAILED;
	}
	insertObject(queue, object, &friendLocation, &skippedToFriend, true);
	if (skippedToFriend){
		return friendLocation+1;
	}
	else{
		return queue->m_size-2;
	}
}

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void* object){
	if (queue==NULL){
	#ifndef NDEBUG
	printf("IsraeliQueueEnqueue: queue or object is NULL");
	#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}
	int friendLocation=UNINITIALIZED;
	bool skippedToFriend=false;

	checkInsertLocation(queue, object, &friendLocation, &skippedToFriend);
	if (resizeQueue(queue)==ISRAELIQUEUE_ALLOC_FAILED){					//Resizing
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	insertObject(queue, object, &friendLocation, &skippedToFriend, false);
	return ISRAELIQUEUE_SUCCESS;
	}



void* IsraeliQueueDequeue(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeue: queue is NULL");
		#endif
		return NULL;
	}

	if (queue->m_size==1){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeue: queue is empty");
		#endif
		return NULL;
	}

	void* object=queue->m_objects[0];
	for (int i=0; i<queue->m_size-1; i++){
		queue->m_objects[i]=queue->m_objects[i+1];
		queue->m_quotas[i]=queue->m_quotas[i+1];
	}
	void *tempPtr=queue->m_objects;
	queue->m_objects=realloc(queue->m_objects, (queue->m_size-1)*sizeof(void*));
	if (queue->m_objects==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeue: realloc failed in IsraeliQueueDequeue");
		#endif
		free(tempPtr);
		return NULL;
	}
	queue->m_size-=1;
	return object;

}


bool IsraeliQueueContains(IsraeliQueue queue, void * object){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueContains: queue is NULL");
		#endif
		return false;
	}

	if (object==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueContains: object is NULL");
		#endif
		return false;
	}

	for (int i=0; i<queue->m_size-1; i++){
		if (queue->m_compare(queue->m_objects[i], object)){
			return true;
		}
	}
	return false;


}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue queue, FriendshipFunction newFriendshipFunction) {
    if (queue == NULL || newFriendshipFunction == NULL) {
        #ifndef NDEBUG
        printf("IsraeliQueueAddFriendshipMeasure: queue or newFriendshipFunction is NULL");
        #endif
        return ISRAELIQUEUE_BAD_PARAM;
    }

    int count = 0;
    while (queue->m_friendshipFunctions[count] != NULL) {
        count++;
    }
	void *tempPtr=queue->m_friendshipFunctions;
    queue->m_friendshipFunctions = realloc(queue->m_friendshipFunctions,(count + 2) * sizeof(FriendshipFunction));
    if (queue->m_friendshipFunctions == NULL) {
        #ifndef NDEBUG
        printf("IsraeliQueueAddFriendshipMeasure: malloc failed");
        #endif
		free(tempPtr);
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    queue->m_friendshipFunctions[count] = newFriendshipFunction;
    queue->m_friendshipFunctions[count + 1] = NULL;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue queue, int newThreshold){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueUpdateFriendshipThreshold: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	queue->m_friendshipThreshold=newThreshold;
	return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue queue, int newThreshold){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueUpdateRivalryThreshold: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}

	queue->m_rivalryThreshold=newThreshold;
	return ISRAELIQUEUE_SUCCESS;

}

int IsraeliQueueSize(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueSize: queue is NULL");
		#endif
		return 0;
	}

	return queue->m_size-1;
}

void* IsraeliQueueDequeueAtIndex(IsraeliQueue queue, int index, IsraeliQueueError* errorFlag){
	if (queue == NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeueAtIndex: queue is NULL");
		#endif
		return NULL;
	}
	if (index < 0 || index >= queue->m_size - 1){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeueAtIndex: invalid index");
		#endif
		return NULL;
	}
	void *tempPtr=queue->m_objects;
	void** objects = realloc(queue->m_objects, (queue->m_size - 1) * sizeof(void*));
	if (objects == NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueDequeueAtIndex: realloc failed in IsraeliQueueDequeueAtIndex");
		#endif
		free(tempPtr);
		*errorFlag = ISRAELIQUEUE_ALLOC_FAILED;
		return NULL;
	}
	void* object = queue->m_objects[index];
	for (int i = index; i < queue->m_size - 1; i++){
		queue->m_objects[i] = queue->m_objects[i + 1];
		queue->m_quotas[i] = queue->m_quotas[i + 1];
	}
	queue->m_objects = objects;
	queue->m_size -= 1;
	return object;
}

IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue){
	if (queue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueImprovePosition: queue is NULL");
		#endif
		return ISRAELIQUEUE_BAD_PARAM;
	}
	if (queue->m_size<=1){
		return ISRAELIQUEUE_SUCCESS;
	}
	IsraeliQueueError errorFlag=ISRAELIQUEUE_SUCCESS;
	IsraeliQueue originalQueue=IsraeliQueueClone(queue);
	if (originalQueue==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueImprovePosition: clone failed");
		#endif
		return ISRAELIQUEUE_ALLOC_FAILED;
	}
	for (int i=originalQueue->m_size-2; i>=0; i--){
		void* currentObject=originalQueue->m_objects[i];
		for (int j=originalQueue->m_size-2; j>=0; j--){
			if ((currentObject==(queue->m_objects[j])) && !(queue->m_quotas[j].improved)){
				Quota currentQuota=queue->m_quotas[j];
				IsraeliQueueDequeueAtIndex(queue, j, &errorFlag);
				if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){
					#ifndef NDEBUG
					printf("IsraeliQueueImprovePosition: dequeue failed");
					#endif
					IsraeliQueueDestroy(originalQueue);
					return ISRAELIQUEUE_ALLOC_FAILED;
				}
				int index=enqueueAndGetIndex(queue, currentObject, &errorFlag);
				if (errorFlag==ISRAELIQUEUE_ALLOC_FAILED){
					#ifndef NDEBUG
					printf("IsraeliQueueImprovePosition: enqueue failed");
					#endif
					IsraeliQueueDestroy(originalQueue);
					return ISRAELIQUEUE_ALLOC_FAILED;
				}
				queue->m_quotas[index]=currentQuota; //retain quota
				queue->m_quotas[index].improved=true;
				break;
			}
		}
	}
	for (int i=0; i<queue->m_size-1; i++){	//reset improved flags
		queue->m_quotas[i].improved=false;
	}
	IsraeliQueueDestroy(originalQueue);
	return ISRAELIQUEUE_SUCCESS;
}

FriendshipFunction* getAllFriendshipFunctions(IsraeliQueue* queues) {
    int totalSize = 0;

    for (IsraeliQueue* queuePointer=queues; *queuePointer != NULL; queuePointer++) {
        for (FriendshipFunction* functionsPointer = (*queuePointer)->m_friendshipFunctions;
		 *functionsPointer != NULL; functionsPointer++) {
            
			totalSize++;
        }
    }

    FriendshipFunction* result = (FriendshipFunction*) malloc((totalSize + 1) * sizeof(FriendshipFunction));
    if (result == NULL) {
		#ifndef NDEBUG
		printf("getAllFriendShipFunctions: malloc failed");
		#endif
        return NULL;
    }

    int currentIndex = 0;
    for (IsraeliQueue* queuePointer=queues; *queuePointer != NULL; queuePointer++) {
        for (FriendshipFunction* functionsPointer = (*queuePointer)->m_friendshipFunctions;
		 *functionsPointer != NULL; functionsPointer++) {
            result[currentIndex] = *functionsPointer;
            currentIndex++;
        }
    }

    result[currentIndex] = NULL;

    return result;
}


IsraeliQueue IsraeliQueueMerge(IsraeliQueue* queueArray, ComparisonFunction compare){
	if (queueArray==NULL || compare==NULL || *queueArray==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueMerge: queueArray or compare is NULL");
		#endif
		return NULL;
	}
	int count=0, sizeOfNewQueue=0, sumOfFriendshipThresholds=0, productOfRivalryThresholds=1;
	while (queueArray[count]!=NULL){
		sizeOfNewQueue+=(queueArray[count]->m_size-1);
		sumOfFriendshipThresholds+=queueArray[count]->m_friendshipThreshold;
		productOfRivalryThresholds*=queueArray[count]->m_rivalryThreshold;
		count++;
	}
	sizeOfNewQueue+=1;
	FriendshipFunction* friendshipFunctions=getAllFriendshipFunctions(queueArray);
	int averageFriendship=(int)ceil((double)sumOfFriendshipThresholds/count);
	int geometricMeanOfRivalry = (int)ceil(pow((double)abs(productOfRivalryThresholds), 1.0 / count));
	IsraeliQueue result=IsraeliQueueCreate(friendshipFunctions, compare, averageFriendship, geometricMeanOfRivalry);
	if (result==NULL){
		#ifndef NDEBUG
		printf("IsraeliQueueMerge: IsraeliQueueCreate failed");
		#endif
		return result;
	}
	int temp=sizeOfNewQueue-1;
	void *currentObject;
	while (temp>0){
		for (int i=0; i<count; i++){
			if (queueArray[i]->m_size>1){
				currentObject=IsraeliQueueDequeue(queueArray[i]);
				if(IsraeliQueueEnqueue(result, currentObject)!=ISRAELIQUEUE_SUCCESS){
					#ifndef NDEBUG
					printf("IsraeliQueueMerge: IsraeliQueueEnqueue failed");
					#endif
					return NULL;
				}
				temp--;
			}
		}
	}
	result->m_objects[sizeOfNewQueue-1]=NULL;
	return result;
} 

