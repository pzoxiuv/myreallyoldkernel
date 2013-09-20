#include "memory.h"
#include "init.h"
#include "task.h"
#include "string.h"

typedef struct task {
	uint32 esp, ebp, eip, eax, ebx, ecx, edx, edi, esi, eflags, id, *pageDir;
	struct task *nextTask;
} task_t;

task_t *firstTask, *currentTask;
uint32 pid;

void initTasks () {
	asm volatile ("cli");

	firstTask = currentTask = kmalloc (sizeof (task_t));
	currentTask->pageDir = pageDirectory;				//first task's pageDir is the original page dir
	currentTask->id = 0;
	currentTask->nextTask = firstTask;
	pid = 1;

	asm volatile ("sti");
}

uint32 fork () {
	asm volatile ("cli");

	task_t *newTask = kmalloc (sizeof (task_t));
	newTask->id = pid;
	newTask->nextTask = firstTask;
	newTask->pageDir = clonePageDirectory (currentTask->pageDir);
	pid++;

	task_t *tmpTask = currentTask;
	while (tmpTask->nextTask != firstTask) {
		tmpTask = tmpTask->nextTask;	
	}

	tmpTask->nextTask = newTask;

	uint32 eip = readEip ();

	if (currentTask->id != (pid-1)) {
		newTask->eip = eip;
		asm volatile ("mov %0, esp" : "=r" (newTask->esp));		
		asm volatile ("mov %0, ebp" : "=r" (newTask->ebp));	
		asm volatile ("mov %0, eax" : "=r" (newTask->eax));
		asm volatile ("mov %0, ebx" : "=r" (newTask->ebx));
		asm volatile ("mov %0, ecx" : "=r" (newTask->ecx));
		asm volatile ("mov %0, edx" : "=r" (newTask->edx));
		asm volatile ("mov %0, esi" : "=r" (newTask->esi));
		asm volatile ("mov %0, edi" : "=r" (newTask->edi));

		asm volatile ("sti");
		return newTask->id;
	}

	return 0;	
}

uint32 switchTasks (uint32 eflags, uint32 esp, uint32 eip, uint32 ebp, uint32 esi, uint32 edi,
		uint32 edx, uint32 ecx, uint32 ebx, uint32 eax) {
	if (firstTask->nextTask == firstTask)	{			//if we only have one task, just return
		return 0;
	}

	currentTask->eip = eip;						//eip is taken from the stack when the ISR is called, and passed into this function
	currentTask->esp = esp;						//(through the previous function, tick, in timer.c), esp is passed in from ISR too
	currentTask->eflags = (eflags & 0xFFFFFDFF);			//clear the IF bit, so we aren't interrupted between when we restore the flags register and jmp to next task
	currentTask->eax = eax;
	currentTask->ebx = ebx;
	currentTask->ecx = ecx;
	currentTask->edx = edx;
	currentTask->esi = esi;
	currentTask->edi = edi;
	currentTask->ebp = ebp;

	currentTask = currentTask->nextTask;				//load next task struct

	asm volatile ("mov cr3, %0" :: "r" (currentTask->pageDir));	//switch to new page directory
	asm volatile ("mov esp, %0" :: "r" (currentTask->esp));		//and update all the registers with the new task's values
	asm volatile ("mov ebp, %0" :: "r" (currentTask->ebp));
	asm volatile ("mov eax, %0" :: "r" (currentTask->eax));
	asm volatile ("mov ebx, %0" :: "r" (currentTask->ebx));
	asm volatile ("mov ecx, %0" :: "r" (currentTask->ecx));
	asm volatile ("mov edi, %0" :: "r" (currentTask->edi));
	asm volatile ("mov edx, %0" :: "r" (currentTask->edx));		//below: put eflags into esi, push it, popfd pops the top value on the stack into the eflags register, then store esi's value in esi
	asm volatile ("mov esi, %1" :: "r" (currentTask->eflags), "r" (currentTask->esi));
	asm volatile ("sti; jmp %0" :: "r" (currentTask->eip)); 	//jmp to next task's eip

	return 0;
}

uint32 getpid () {
	return currentTask->id;
}

uint32 mapTaskPage (uint32 id, void *phys, void *virt) {
	task_t *tmpTask = firstTask;
	while (tmpTask->id != id && tmpTask->nextTask != firstTask) {
		tmpTask = tmpTask->nextTask;
	}
	if (tmpTask->id != id) {
		return -1;	
	}
	
	mapPage (tmpTask->pageDir, phys, virt, 0, 1);
	asm volatile ("mov cr3, %0" :: "r" (currentTask->pageDir));	//incase target task is the current task, we want to make sure the change in mapping is reflected immediatly

	return 0;
}
