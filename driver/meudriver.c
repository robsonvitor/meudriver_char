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

static unsigned int major;
static struct class *meudriver_classe;
static struct cdev meudriver_cdev;
static struct file* filp = NULL;

/*

Para os metodos de tratamento de acesso a dados entre kernel space e user space, precisa isolar e definidir os limites:
mm_segment_t oldfs -> addr_limit
oldfs = get_fs() -> KERNEL_DS
set_fs(get_ds()) ->  USER_DS
set_fs(oldfs) -> KERNEL_DS

get_fs returns the current segment descriptor stored in FS.
get_ds returns the segment descriptor associated to kernel space, currently stored in DS.
set_fs stores a descriptor into FS, so it will be used for data transfer instructions.
*/

/* metodo para abertura da comunicacao com a porta serial, abre o driver de caracter /dev/ttyACMO */
struct file* abre_arquivo(const char* path, int flags, int rights){
    int erro = 0;
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);

    if (IS_ERR(filp)) {
        erro = PTR_ERR(filp);
        return NULL;
    }

    return filp;
}

/* Fecha o arquivo */
void fecha_arquivo(struct file* file){
    filp_close(file, NULL);
}

int escreve_arquivo(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size){
    int count;
    mm_segment_t oldfs;
    oldfs = get_fs();
    set_fs(get_ds());
    count = vfs_write(file, data, size, &offset);
    set_fs(oldfs);

    return count;
}

static int abre(struct inode *inode, struct file *file){
    filp = abre_arquivo("/dev/ttyACM0", O_RDWR, 0666);
    return 0;
}

static int libera(struct inode *inode, struct file *filp){
    fecha_arquivo(filp);
    return 0;
}

static ssize_t escrita(struct file *file, const char __user *buf,size_t len, loff_t *ppos){
    ssize_t retorno = escreve_arquivo(filp, *ppos, buf, len);
    return retorno;
}

/* Mapeia as operacoes do kernel com funcoes do driver */
struct file_operations meudriver_fops = {
    owner:       THIS_MODULE,
    open:       abre,
    release:    libera,
    write:      escrita,
};

/* metodo executado ao carregar driver para o kernel */
static int __init inicializa_driver(void) {
    struct device *meudriver_device;
    int erro;
    dev_t devt = 0;

    /* Solicita ao kernel a alocacao do dispositivo */
    erro = alloc_chrdev_region(&devt, 0, 1, "meudriver_char");
    if (erro < 0) {
        pr_err("meudriver - Nao foi possivel alocar major\n");
        return erro;
    }

    /* Extrai o major de devt */
    major = MAJOR(devt);

    pr_info("meudriver - meudriver_char major number = %d\n", major);

    /* Cria a classe do dispositivo /sys/class/meudriver_char_class */
    meudriver_classe = class_create(THIS_MODULE, "meudriver_char_class");
    if (IS_ERR(meudriver_classe)) {
        pr_err("meudriver - Erro ao criar classe de caracter.\n");
        unregister_chrdev_region(MKDEV(major, 0), 1);
        return PTR_ERR(meudriver_classe);
    }

    /* Inicializa o driver com os parametros de meudriver_fops */
    cdev_init(&meudriver_cdev, &meudriver_fops);

    /* Associa o dispositivo com os devidos numeros de  char para que usuarios tenham acesso */
    cdev_add(&meudriver_cdev, devt, 1);

    /* Cria e registra o dispositivo (classe,nenhum driver relacionado,numeros "major, minor", dados adicionais, nome do driver)*/
    meudriver_device = device_create(meudriver_classe,NULL,devt,NULL,"meudriver_char");

    if (IS_ERR(meudriver_device)) {
        pr_err("meudriver - Erro ao criar o dispositivo.\n");
        class_destroy(meudriver_classe);
        unregister_chrdev_region(devt, 1);
        return -1;
    }

    pr_info("meudriver - O driver foi inicializado com sucesso\n");
    return 0;
}

/* Metodo executado ao descarregar driver do kernel */
static void __exit remove_driver(void) {
    unregister_chrdev_region(MKDEV(major, 0), 1);
    device_destroy(meudriver_classe, MKDEV(major, 0));
    cdev_del(&meudriver_cdev);
    class_destroy(meudriver_classe);

    pr_info("meudriver - O driver foi removido!\n");
}


/* Mapeia funcoes de inicializacao e remocao do driver */
module_init(inicializa_driver);
module_exit(remove_driver);

/* Informacoes do driver */
MODULE_DESCRIPTION("Driver de caracter");
MODULE_LICENSE("GPL");