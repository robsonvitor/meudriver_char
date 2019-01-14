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


/*##################################################################*/

static struct file* filp = NULL;

struct file* file_open(const char* path, int flags, int rights)
{
    int err = 0;
    struct file* filp = NULL;


    /* Para acessar uma região de memória que está além do limite de Endereço Virtual do Espaço do Usuário
    (ou seja, caindo na região de Endereço Virtual do Espaço do Kernel), você primeiro armazena o limite atual. */
    mm_segment_t oldfs;
    oldfs = get_fs();

    /* Define o limite como o do Kernel - 4 GB/*/
    set_fs(get_ds());


    /* O get_fs() irá recuperar este limite e o set_fs() irá defini-lo com um valor */
    /* Operações em memória (Ex: - Ler de um buffer que está no espaço do kernel de um contexto de usuário através de uma system call)*/
    filp = filp_open(path, flags, rights);

    /* Define o limite de endereço de retorno para o limite original que foi armazenado na variável old_fs */
    set_fs(oldfs);

    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }

    return filp;
}

static int abre(struct inode *inode, struct file *file)
{
    filp = file_open("/dev/ttyACM0", O_RDWR, 0666);
    return 0;
}

/*##################################################################*/
struct file_operations meudriver_fops = {
owner:       THIS_MODULE,
open:       abre,
// release:    fecha,
// read:       leitura,
// write:      escrita,
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
    // dummy_cdev.owner = THIS_MODULE;
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
