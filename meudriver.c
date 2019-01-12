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
static struct class *classe_meudriver;
static struct cdev dummy_cdev;

static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;

int abre(struct inode * inode, struct file * filp) {
    pr_info("## meudriver - abre! ##\n");

    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open("/dev/ttyACM0", O_RDWR, 0666);
    
    if(IS_ERR(filp)){
        pr_info("meudriver - Nao foi possivel abrir dispositivo!!");
    }else{
        pr_info("meudriver - dispositivo aberto com sucesso!");
    }
    set_fs(oldfs);

    return 0;
}

int fecha(struct inode * inode, struct file * filp) {
    pr_info("## meudriver - fecha! ##\n");
    return 0;
}

ssize_t leitura (struct file *filp, char __user * buf, size_t count,
                 loff_t * offset) {

    pr_info("## meudriver - leitura! ##\n");

    int error_count;
    error_count = 0;
    error_count = copy_to_user(buf, message, size_of_message);

   if (error_count==0){
      pr_info("meudriver: Copiados %d caracteres para usuario\n", size_of_message);
      return (size_of_message=0);
   }
   else {
      pr_info("meudriver: Falha ao copiar %d caracteres para usuario\n", error_count);
      return -EFAULT;
   }

   return 0;
}


ssize_t escrita(struct file * filp, const char __user * buf, size_t count, loff_t * offset) {

    pr_info("## meudriver - escrita! ##\n");
    sprintf(message, "%s", buf);                // Coloca na variavel message o conteudo lido em buf
    size_of_message = strlen(message);                // armazena o tamanho da string em size_of_message
    pr_info("meudriver - %s", message);
    pr_info("meudriver: %zu caracteres recebidos do usuario\n", count);
    return count;
}

struct file_operations meudriver_fops = {
    open:       abre,
    release:    fecha,
    read:       leitura,
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
    classe_meudriver = class_create(THIS_MODULE, "meudriver_char_class");
    if (IS_ERR(classe_meudriver)) {
        pr_err("meudriver - Erro ao criar classe de caracter.\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return PTR_ERR(classe_meudriver);
    }

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&dummy_cdev, &meudriver_fops);
    dummy_cdev.owner = THIS_MODULE;
    /* Now make the device live for the users to access */
    cdev_add(&dummy_cdev, devt, 1);

    meudriver_device = device_create(classe_meudriver,
                                     NULL,   /* no parent device */
                                     devt,    /* associated dev_t */
                                     NULL,   /* no additional data */
                                     "meudriver_char");  /* device name */

    if (IS_ERR(meudriver_device)) {
        pr_err("meudriver - Erro ao criar o dispositivo.\n");
        class_destroy(classe_meudriver);
        unregister_chrdev_region(devt, 1);
        return -1;
    }

    pr_info("meudriver - O driver foi inicializado com sucesso\n");
    return 0;
}

static void __exit remove_driver(void) {
    unregister_chrdev_region(MKDEV(major, 0), 1);
    device_destroy(classe_meudriver, MKDEV(major, 0));
    cdev_del(&dummy_cdev);
    class_destroy(classe_meudriver);

    pr_info("meudriver - O driver foi removido!\n");
}

module_init(inicializa_driver);
module_exit(remove_driver);


MODULE_DESCRIPTION("Driver de caracter");
MODULE_LICENSE("GPL");
