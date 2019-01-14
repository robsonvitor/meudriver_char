#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <asm/uaccess.h>

static unsigned int major; /* variavel para o MAJOR do driver */
static struct class *meudriver_classe;
static struct cdev meudriver_cdev;


/*##################################################################*/

static struct file* filp = NULL;

struct file* file_open(const char* path, int flags, int rights){
    int err = 0;
    struct file* filp = NULL;

    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);

    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }

    return filp;
}

void file_close(struct file* file){
    filp_close(file, NULL);
}

int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size){
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());
    ret = vfs_read(file, data, size, &offset);
    set_fs(oldfs);

    return ret;
}

int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size){
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());
    ret = vfs_write(file, data, size, &offset);
    set_fs(oldfs);

    return ret;
}

static int abre(struct inode *inode, struct file *file){
    filp = file_open("/dev/ttyACM0", O_RDWR, 0666);
    return 0;
}

static int libera(struct inode *inodep, struct file *filp){
    file_close(filp);
    return 0;
}

static ssize_t escrita(struct file *file, const char __user *buf,size_t len, loff_t *ppos){
    ssize_t recv = file_write(filp, *ppos, buf, len);
    return recv;
}

/*##################################################################*/
struct file_operations meudriver_fops = {
    owner:       THIS_MODULE,
    open:       abre,
    release:    libera,
    write:      escrita,
};

struct miscdevice meudriver_device = {
    .name = "meudriver",
    .fops = &meudriver_fops,
};

static int __init inicializa_driver(void) {
    struct device *meudriver_device;
    int error;
    dev_t devt = 0;

    /* Get a range of minor numbers (starting with 0) to work with */
    error = alloc_chrdev_region(&devt, 0, 1, "meudriver_char");
    if (error < 0) {
        pr_err("meudriver - Nao foi possivel alocar major\n");
        return error;
    }
    major = MAJOR(devt);
    pr_info("meudriver - meudriver_char major number = %d\n", major);

    /* Create device class, visible in /sys/class */
    meudriver_classe = class_create(THIS_MODULE, "meudriver_char_class");
    if (IS_ERR(meudriver_classe)) {
        pr_err("meudriver - Erro ao criar classe de caracter.\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return PTR_ERR(meudriver_classe);
    }

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&meudriver_cdev, &meudriver_fops);
    /* Now make the device live for the users to access */
    cdev_add(&meudriver_cdev, devt, 1);

    meudriver_device = device_create(meudriver_classe,
                                     NULL,   /* no parent device */
                                     devt,    /* associated dev_t */
                                     NULL,   /* no additional data */
                                     "meudriver_char");  /* device name */

    if (IS_ERR(meudriver_device)) {
        pr_err("meudriver - Erro ao criar o dispositivo.\n");
        class_destroy(meudriver_classe);
        unregister_chrdev_region(devt, 1);
        return -1;
    }

    pr_info("meudriver - O driver foi inicializado com sucesso\n");
    return 0;
}

static void __exit remove_driver(void) {
    unregister_chrdev_region(MKDEV(major, 0), 1);
    device_destroy(meudriver_classe, MKDEV(major, 0));
    cdev_del(&meudriver_cdev);
    class_destroy(meudriver_classe);

    pr_info("meudriver - O driver foi removido!\n");
}

module_init(inicializa_driver);
module_exit(remove_driver);


MODULE_DESCRIPTION("Driver de caracter");
MODULE_LICENSE("GPL");
