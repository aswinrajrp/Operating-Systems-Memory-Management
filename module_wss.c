/*
*	Module to print the working set size of a process.
*/
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/hrtimer.h>	// for timing
#include <linux/ktime.h>	// for timing
#define DRIVER_AUTHOR "Trey Manuszak <tmanusza@asu.edu>, Omair Neazi <oneazi@asu.edu>, Aswinraj Ravichandran Prema <aravic18@asu.edu>"
#define DRIVER_DESCRIPTION "Prints the working set size of a process."

// Set interval to 5 second
unsigned long timer_interval_ns = 5e9;
static struct hrtimer hr_timer;

/*
 * Static variable declarations
 */
static int pid;

/*
 * Module parameters
 */
module_param(pid, int, S_IRUGO);	// get the PID as module parameter
MODULE_PARM_DESC(pid, "The process to print the working set size of."); // Parameter description

// function to print the working set size of a given process
void print_wss(int pid)
{
	unsigned long totalSize = 0;
	struct task_struct * task;
	for_each_process(task)		// go through all tasks till the one with the right pid is found
	{
		if (task->pid == pid)
		{
			// go Through page tables of process
			struct mm_struct * mmTemp = task->mm;
			struct vm_area_struct * temp = mmTemp->mmap;
			while(temp != NULL)
			{
				unsigned long addr = temp->vm_start;
				while(addr < temp->vm_end){
					pgd_t *pgd;
					p4d_t *p4d; 
					pmd_t *pmd;
					pud_t *pud;
					pte_t *ptep, pte;
					    
					pgd = pgd_offset(mmTemp, addr);           // get pgd from mm and the page address
					if (pgd_none(*pgd) || pgd_bad(*pgd)){           // check if pgd is bad or does not exist
						return;}

					p4d = p4d_offset(pgd, addr);         // get p4d from from pgd and the page address
					if (p4d_none(*p4d) || p4d_bad(*p4d)){          // check if p4d is bad or does not exist
						return;}

					pud = pud_offset(p4d, addr);         // get pud from from p4d and the page address
					if (pud_none(*pud) || pud_bad(*pud)){          // check if pud is bad or does not exist
						return;}

					pmd = pmd_offset(pud, addr);      // get pmd from from pud and the page address
					if (pmd_none(*pmd) || pmd_bad(*pmd)){       // check if pmd is bad or does not exist
						return;} 
					ptep = pte_offset_map(pmd, addr);      // get pte from pmd and the page address
					if (!ptep){return;}                                         // check if pte does not exist
					pte = *ptep;
					
					if (pte_young (pte))				// if the page has been accessed
					{
						totalSize = totalSize + PAGE_SIZE;
					}
					addr = addr + PAGE_SIZE;
				}
				temp = temp->vm_next;
			}
			break;
		}
	}
	
	printk("[PID] : [WSS]\n");
	printk("[%d] : [%lu kB]\n", pid, totalSize/1024);
	return;
}

// Callback executed periodically, reset timer
enum hrtimer_restart timer_callback( struct hrtimer *timer_for_restart )
{
	ktime_t currtime , interval;
    	currtime  = ktime_get();
    	interval = ktime_set(0,timer_interval_ns);
    	hrtimer_forward(timer_for_restart, currtime , interval);
    	print_wss(pid);
    	return HRTIMER_RESTART;
}

static int __init print_wss_init(void)
{
	ktime_t ktime;
    	ktime = ktime_set( 0, timer_interval_ns );
    	hrtimer_init( &hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL );
    	hr_timer.function = &timer_callback;
    	hrtimer_start( &hr_timer, ktime, HRTIMER_MODE_REL );  // Start timer
	return 0;
}

static void __exit print_wss_exit(void)
{
	int ret;
    	ret = hrtimer_cancel( &hr_timer );
    	if (ret) printk("Timer was still in use!\n");
    	printk("HR Timer removed\n");
	return;
}



module_init(print_wss_init);
module_exit(print_wss_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_LICENSE("GPL");
