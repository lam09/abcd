/****************************************************************************
*
*     X-line Linux Kernel driver
*     File:    xline_driver.c
*     Version: 1.2
*     Copyright 2014 Heber Limited (http://www.heber.co.uk)
*
*     This program is free software; you can redistribute it and/or modify it
*     under the terms of the GNU General Public License as published by the
*     Free Software Foundation; either version 2, or (at your option) any
*     later version.
*
*     This program is distributed in the hope that it will be useful, but
*     WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*     General Public License for more details.
*
*     You should have received a copy of the GNU General Public License along
*     with this program; if not, write to the Free Software Foundation, Inc.,
*     675 Mass Ave, Cambridge, MA 02139, USA.
*
*     Notes:
*     This source file is formatted for 4-space tabs
*
*     Changes:
*     13  JOS       Update for kernels up to 3.12.15 (June-14)
*     10  JOS       Update for kernels up to 2.6.37 (Mar-11)
*     6   EML       Updating for use with kernels 2.6.26 - 2.6.30
*     4   MJB       Added support for XTopper board, 2/10/2008
*     1   MJB       Added support for XSpin and XLuminate boards, 15/7/2008
*     0   MJB       First version.
*                   Based on X10 "x10_driver.c" v1.4, 16/8/2005
*
****************************************************************************/

//#define _X10_USB_DEBUG					// directs lots of printk debug messages to syslog

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
 	#include <asm/semaphore.h>
#else
 	#include <linux/semaphore.h>   /* change during 2.6.26 made all sempahores
				   					  generic spinlock+waitqueue wrappers */
#endif
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/device.h>
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
	#include <linux/mutex.h>		/* replacement for lock_kernel/unlock_kernel */
#else
	#include <linux/smp_lock.h>
#endif
#include <linux/stat.h>
#include <linux/proc_fs.h>
#include <linux/wait.h>
#include <asm/uaccess.h>        /* 2.6 change */
#include "x10locode.c"          /* X10 Low 8051 code */
#include "x10ilocode.c"			/* X10i low 8051 code */
#include "x15locode.c"			/* X15 low 8051 code */
#include "xluminatelocode.c"	/* XLuminate low 8051 code */
#include "xspinlocode.c"		/* XSpin low 8051 code */
#include "xtopperlocode.c"		/* XTopper low 8051 code */
#include "loader.c"             /* do as an include rather than seperate compilation to keep symbols static */
                                /* This is to avoid poluting the kernel namespace on loading */
#include <linux/moduleparam.h>

// 'slab.h' (for kmalloc, kfree, etc) no longer included automatically (3.10++) for driver builds.
// 'create_proc_entry_read()' function has been depricated (3.9++), alternative using
// seq_file/seq_printf mechanism now used instead
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	#include <linux/slab.h>
	#include <linux/seq_file.h>
#endif

#ifdef _X10_USB_DEBUG
	#define dprintk(format, arg...) printk(KERN_DEBUG format , ## arg)
#else
	#define dprintk(format, arg...)
#endif

// The X-line Kernel driver is under a GPL release.
MODULE_LICENSE( "GPL" );
MODULE_DESCRIPTION( "Heber X-line Kernel Driver" );
MODULE_AUTHOR( "Heber Limited <http://www.heber.co.uk>" );
MODULE_SUPPORTED_DEVICE( "X-line" );

#define FULL_SPEED								0
#define HIGH_SPEED								1
#define UNKNOWN_SPEED							2

#define HEBER									(0x0fb6)        /* Heber USB Vendor ID */
#define X10i_LOADER								(0x3fc4)        /* X10i Loader Product ID */
#define X10i									(0x3fc3)        /* X10i Product ID */
#define X10_LOADER								(0x3fc6)        /* X10 Loader Product ID */
#define X10										(0x3fc5)        /* X10 Product ID */
#define X15_LOADER								(0x3fc8)        /* X15 Loader Product ID */
#define X15										(0x3fc7)        /* X15 Product ID */
#define XSPIN_LOADER							(0x3fca)		/* XSpin Loader Product ID */
#define XSPIN									(0x3fc9)		/* XSpin Product ID */
#define XLUMINATE_LOADER						(0x3fcc)		/* XLuminate Loader Product ID */
#define XLUMINATE								(0x3fcb)		/* XLuminate Product ID */
#define XTOPPER_LOADER							(0x3fce)		/* XTopper Loader Product ID */
#define XTOPPER									(0x3fcd)		/* XTopper Product ID */
#define LOADER									0
#define INVALID_CONNECTION                                                      -1
static int X10_major_number = 0;

#define TIMEOUT_MULT_HZ							3       		/* Response timeout * HZ */
#define MAX_USB_RETRIES							10

#define MAX_BULK_SIZE							1024
#define MIN_VPIPE_NO							0
#define MAX_VPIPE_NO							8
#define MAX_PPIPE_NO							2


/* Various X10/X10i definitions. These are also defined within the 8051 firmware, so if any */
/* of these definitions are changed then they will need to be updated accordingly in */
/* the 8051 firmware code. */
#define NUMBER_OF_PIPES							7       /* Defined in "x10pipes.h" */

#define ANCHOR_LOAD_INTERNAL					0xA0
#define ANCHOR_LOAD_EXTERNAL					0xA3    /* This command is not implemented in the core. Requires firmware. */
#define MAX_INTERNAL_ADDRESS					0x1B3F  /* This is the highest internal RAM address for the AN2131Q. */
#define CPUCS_REG_EZUSB							0x7F92  /* EZ-USB Control and Status Register.  Bit 0 controls 8051 reset */
#define CPUCS_REG_FX2                           0xE600  /* FX2 Control and Status Register. */

#define FALSE									0
#define TRUE									1

#define USB_MESSAGE_EXECUTION_SUCCESS			0
#define USB_DEVICE_BYTE_COUNT_ERROR				7


// prototype
static int XlineIoctl( struct inode *inode_ptr, struct file *file_ptr, unsigned int command, unsigned long arg );

/*
The user mode library needs to know bus speed and which type of X10 we are dealing with
All of this data is presented within the sysfs file system
The driver creates a new X10 class within sysfs and as X10's are connected creates a new
entry for each one in turn.

The entry contains a symbolic link to the USB bus entry so user space can get from an X10
to its configuration data quickly.
Another benefit of using sysfs is the X10 stuff picks up sensible plug and play behaviour
subject to a suitable line being added to the udev configuration files
KERNEL="X10/X10_*",NAME="%k",MODE="666"
works well, and the udev demon being used to create /dev then a few seconds after plugging in an
X10 a /dev entry is created for it. The /dev X10 entries should only correspond to real working X10
devices.

However the sysfs API has evolved so for older kernels we use the
class_simple api
and for later ones
the class device api
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code variations dependent on kernel version starts here
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
	#define usb_kill_urb(x) usb_unlink_urb(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
	int atomic_dec_return( atomic_t *v )
	{
		unsigned long flags;
		int ret_value;

		local_irq_save( flags );
		atomic_dec( v );
		ret_value = atomic_read( v );
		local_irq_restore( flags );

		return( ret_value );
	}

	int atomic_inc_return( atomic_t * v )
	{
		unsigned long flags;
		int ret_value;

		local_irq_save( flags );
		atomic_inc( v );
		ret_value = atomic_read( v );
		local_irq_restore( flags );

	    return( ret_value );
	}

	unsigned long wait_for_completion_timeout( struct completion *x, unsigned long timeout )
	{
		might_sleep( );
		spin_lock_irq( &x->wait.lock );

		if ( !x->done )
		{
			DECLARE_WAITQUEUE( wait, current );
			wait.flags |= WQ_FLAG_EXCLUSIVE;
			__add_wait_queue_tail( &x->wait, &wait );

			do
			{
				__set_current_state( TASK_UNINTERRUPTIBLE );
				spin_unlock_irq( &x->wait.lock );
				timeout = schedule_timeout( timeout );
				spin_lock_irq( &x->wait.lock );

				if ( !timeout )
				{
					__remove_wait_queue( &x->wait, &wait );
					goto out;
				}
			} while( !x->done );

			__remove_wait_queue( &x->wait, &wait );
		}

		x->done--;

	out:
		spin_unlock_irq( &x->wait.lock );

		return( timeout );
	}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
	static struct class_simple *X10_class = NULL;
	static inline void create_X10_sysfs_class( void )
	{
	  	X10_class = class_simple_create( THIS_MODULE, "Xline" ); //let ths sysfs export nasty stuff like pipe sizes
	}

	static inline void destroy_X10_sysfs_class( void )
	{
	  	if ( X10_class )
	  	{
	    	class_simple_destroy( X10_class );
		}
	}
	static inline void add_X10_to_sysfs_class( dev_t dev, struct device *device, const char *fmt, int no )
	{
	  	class_simple_device_add( X10_class, dev, device, fmt, no );
	}

	static inline void remove_X10_from_sysfs_class( dev_t dev )
	{
	  	class_simple_device_remove( dev );
	}
#else
	static struct class *X10_class = NULL;
	static inline void create_X10_sysfs_class( void )
	{
	  	X10_class = class_create( THIS_MODULE, "Xline" );        //let ths sysfs export nasty stuff like pipe sizes
	}

	static inline void destroy_X10_sysfs_class( void )
	{
	  	if ( X10_class )
	  	{
	    	class_destroy( X10_class );
		}
	}

	# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
		static inline void add_X10_to_sysfs_class( dev_t dev, struct device *device,  char *fmt, int no )
		{
			class_device_create( X10_class, NULL, dev, device, fmt, no );
		}

		static inline void remove_X10_from_sysfs_class( dev_t dev )
		{
		  	class_device_destroy( X10_class, dev );
		}
	# else

		#  if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
			static inline void add_X10_to_sysfs_class( dev_t dev, struct device *device,  char *fmt, int no )
			{
				device_create( X10_class, device, dev, fmt, no );
			}
		#  else
			static inline void add_X10_to_sysfs_class( dev_t dev, struct device *device,  char *fmt, int no )
			{
				device_create( X10_class, device, dev, NULL, fmt, no );
			}
		#  endif

		static inline void remove_X10_from_sysfs_class( dev_t dev )
		{
			device_destroy( X10_class, dev );
		}
	# endif

#endif

// The following two defines are effectively function name changes; same function signature are used
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35)
	#define usb_buffer_alloc  usb_alloc_coherent
	#define usb_buffer_free  usb_free_coherent
#endif

# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
	#define init_MUTEX(sem)   sema_init(sem,1)
#endif


// The end of BKL ... Big Kernel Lock!!
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
	static DEFINE_MUTEX( xline_mutex );
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Code Conditional upon kernel version ends here
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int INTERNAL_RAM( unsigned short int address )
{
	return( address <= MAX_INTERNAL_ADDRESS );
}

struct Pipe
{
  	wait_queue_head_t queue;
  	int InUse;
};

/*====================================================================
   Defines a structure to contain all the data we need about an individual X10
====================================================================*/
struct driver_context
{
	atomic_t reference_count;
	int Type;                     //change if final firmware we now stash USB type code so we can tell difference between x10 and x10i
	int Number;
	int BoardFitted;
	int BoardSpeed;
	struct Pipe Pipes[NUMBER_OF_PIPES];
	struct usb_device *dev;
	char True;                    // Create this structure in DMA accessible memory
	char False;                   // so we predefine flags for the Reset Command
	struct driver_context *next;
};

void release_driver_context( struct driver_context *context )
{
	if ( atomic_dec_return( &context->reference_count ) == 0 )
	{
		dprintk( "freeing context\n" );
		kfree( context );
	}
}

void open_driver_context( struct driver_context *context )
{
	atomic_inc( &context->reference_count );
}


/*
    X10 used more pipe pairs than X10i so X10i simulates the extra X10 pipes
    These simulated pipes are hencefore described as virtual pipes or vpipes.
    To an application virtual pipe operations are straight forward synchronous blocking operations
    only one process /thread is allowed to access each virtual pipe at once (although operations are
    queued and this restriction is hidden from the application)
    An application queues for a vpipe then issues a transmit and then waits for the return packet after
    which it copies the returned data to user space and frees the virtual pipe for the next user
*/
struct vpipe
{
	unsigned char rx_buffer[MAX_BULK_SIZE];       /* return buffer */
	unsigned long size;           /* Number of bytes in buffer */
	struct semaphore pipe_mutex;  /* allow only one process to access vpipe at once */
	struct completion finish;     /* signals rx data received in vpipe */
	int operation_status;         /* signals any receive error codes */
	wait_queue_head_t queue;
	atomic_t in_use;
};

/*
    Allows the URB completion handler to signal the completion of a tx operation and to
    report the operation's final status
*/

struct tx_completion
{
	int operation_status;         /* returns urb status */
	struct completion finish;     /* signals tx urb has completed */
};

/*
    The physical or ppipes correspond to real USB transactions. A virtual transaction chooses a ppipe to
    provide the underlying communications.
    Both transmits and receives to ppipes are interleaved and asynchronous. Ppipe reception is handled by a USB
    URB completion handler. This happens in an interrupt context and it is important that the system tracks
    outstanding operations.
*/

struct ppipe
{
	struct x10i_instance *instance;		//allows interrupt code to back tracking to vpipe array
	atomic_t operation_count;     		//count number of unanswered operations issue
	atomic_t urb_active;
};

/*
    Keep a list of currently active URBs so we can cancel them on an unplug
*/
struct URB_list_entry
{
    struct list_head list;
    struct urb *urb;
};

/*
    Contains X10i specfic data
*/
struct x10i_instance
{
	spinlock_t urb_list_lock ;
	struct usb_device *udev;
	struct list_head urb_list;
	struct vpipe vpipes[MAX_VPIPE_NO];
	struct ppipe ppipes[MAX_PPIPE_NO];
};

static inline void add_urb_to_list( struct x10i_instance * x10i, struct urb * urb)
{
	unsigned long flags ;
	struct URB_list_entry * ptr = (struct URB_list_entry *)kmalloc(sizeof(struct URB_list_entry),GFP_KERNEL);

	if ( ptr )
	{
		spin_lock_irqsave( &x10i->urb_list_lock, flags );
		ptr->urb = urb ;
		list_add_tail( &ptr->list, &x10i->urb_list );
		spin_unlock_irqrestore( &x10i->urb_list_lock, flags );
	}
}

static inline void remove_urb_from_list( struct x10i_instance * x10i, struct urb * urb )
{
	unsigned long flags;
	struct URB_list_entry * ptr;
	struct list_head *p;

	spin_lock_irqsave( &x10i->urb_list_lock, flags );
	list_for_each( p, &x10i->urb_list )
	{
		ptr = list_entry( p, struct URB_list_entry, list );
		if ( ptr->urb == urb )
		{
			list_del( p );
			kfree( ptr );
			break;
		}
	}

	spin_unlock_irqrestore( &x10i->urb_list_lock, flags );
}

static inline void cancel_x10i_urbs( struct x10i_instance * x10i )
{
	struct list_head *p;
	struct URB_list_entry * ptr;

	list_for_each( p, &x10i->urb_list )
	{
		ptr = list_entry( p, struct URB_list_entry, list );
		usb_unlink_urb( ptr->urb );
	}
}

/*
    Initalise physical pipe tracking structure
*/
static inline void init_ppipe( struct ppipe *pipe, struct x10i_instance *x10i )
{
	pipe->instance = x10i;
	atomic_set( &pipe->operation_count, 0 );
	atomic_set( &pipe->urb_active, 0 );
}

/*
    Initalise virtual pipe structure
*/
static inline void init_vpipe( struct vpipe *pipe, struct x10i_instance *x10i )
{
	pipe->size = 0;
	atomic_set( &pipe->in_use, 0 );
	init_MUTEX( &pipe->pipe_mutex );
	init_waitqueue_head( &pipe->queue );
	init_completion( &pipe->finish );
}

/*
    Initalise a new X10i instance
*/
static inline void init_x10i_instance( struct x10i_instance *x10i, struct usb_device *dev )
{
	int x;

	spin_lock_init( &x10i->urb_list_lock );
	INIT_LIST_HEAD( &x10i->urb_list );

	for ( x = 0; x < MAX_VPIPE_NO; x++ )
	{
		init_vpipe( &x10i->vpipes[x], x10i );
	}

	for ( x = 0; x < MAX_PPIPE_NO; x++ )
	{
		init_ppipe( &x10i->ppipes[x], x10i );
	}

	x10i->udev = dev;
}

/*
   X10s were tracked using a linked list (so structures do not need to be the same size) ,
    this structure will satisfy the existing X10 code and also support the X10i extensions
*/
struct X10I_driver_context
{
	struct driver_context context;
	struct x10i_instance instance;
};

/*====================================================================
	The driver has a linked list of X10's
 =====================================================================*/
static struct driver_context *ContextList = NULL;

static int DoX10IO( struct driver_context *context, X10_COMMAND * KernelCommandCopy );
static int DoX10Read( struct driver_context *context, X10_COMMAND * KernelCommandCopy );
static int DoX10Write( struct driver_context *context, X10_COMMAND * KernelCommandCopy );

/*
Tx operations are closely associated with an individual thread/process
so the completion operation just wakes the thread
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static void x10i_tx_complete( struct urb *urb, struct pt_regs *regs )
#else
	static void x10i_tx_complete( struct urb *urb )
#endif
{
	struct tx_completion *tx_complete = (struct tx_completion *)urb->context;
	int status = urb->status;

	if ( ( status == -ENOENT ) || ( status == ECONNRESET ) || ( status == ESHUTDOWN ) )
	{
		status = 0;
	}

	tx_complete->operation_status = status;

	if ( status )
	{
		dprintk( "error tx_complete status returned %d\n", status );
	}

	complete( &tx_complete->finish );
}


/*
Rx is more complex, the RX routine is called by the USB subsystem on receipt of a packet from
the specified endpoint.
If  the packet was OK then we copy it to a vpipe specfic holding buffer (this involves a double copy but
the completion routine is called in an interrupt context so copy_to_user is unavailable)
The thread/process waiting on the vpipe is awakened.

If other read operations are expected then the URB is re submitted otherwise the URB's buffers are released
*/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
	static void x10i_rx_complete( struct urb *urb, struct pt_regs *regs )
#else
	static void x10i_rx_complete( struct urb *urb )
#endif
{
	struct ppipe *ppipe;
	int status;
	char *error_code;
	int vpipe;
	unsigned char *buf;
	int number_to_copy;

	ppipe = (struct ppipe *) urb->context;
	status = urb->status;
	if ( ( status == -ENOENT ) || ( status == ECONNRESET ) || ( status == ESHUTDOWN ) )
	{
		status = 0;
	}

	if ( status )
	{
		switch ( urb->status )
		{
		case -ENOENT:
			error_code = "URB was canceled by unlink_urb\n";
			break;
		case -EINPROGRESS:
			error_code = "URB still pending, no results yet (actually no error until now)\n";
			break;
		case -EPROTO:
			error_code = "Bitstuff error or Unknown USB error\n";
			break;
		case -EILSEQ:
			error_code = "CRC mismatch\n";
			break;
		case -EPIPE:
			error_code = "Babble detect or  Endpoint stalled\n";
			break;
		case -ETIMEDOUT:
			error_code = "Transfer timed out, NAK\n";
			break;
		case -ENODEV:
			error_code = "Device was removed\n";
			break;
		case -EREMOTEIO:
			error_code = "Short packet detected\n";
			break;
		case -EXDEV:
			error_code = "ISO transfer only partially completed look at individual frame status for details\n";
			break;
		case -EINVAL:
			error_code = "ISO madness, if this happens: Log off and go home\n";
			break;
		default:
			error_code = "unknown usb error %d\n";
			break;
		}
		printk( error_code, urb->status );
	}
	else
	{
		buf = ( (unsigned char *)urb->transfer_buffer );
		vpipe = *buf;
		if ( ( vpipe >= 0 ) && ( vpipe < MAX_VPIPE_NO ) )
		{
			ppipe->instance->vpipes[vpipe].size = 0;
			if ( urb->actual_length > 0 )
			{
				number_to_copy = urb->actual_length - 1;
				memcpy( ppipe->instance->vpipes[vpipe].rx_buffer, buf + 1, number_to_copy );
				ppipe->instance->vpipes[vpipe].size = number_to_copy;
			}

			ppipe->instance->vpipes[vpipe].operation_status = status;
			complete( &ppipe->instance->vpipes[vpipe].finish );
		}
	}

	if ( ( atomic_dec_return( &ppipe->operation_count ) > 0 ) && ( urb->status == 0 ) )
	{
		status = usb_submit_urb( urb, GFP_ATOMIC );
		return;
	}

	remove_urb_from_list( ppipe->instance, urb );

	// is mapped to usb_free_coherent(..) on 2.6.35++
	usb_buffer_free( urb->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma );

	usb_free_urb( urb );
}

/*
    X10i offers a choice of two physical pipes
    Which one we use for a given operation isn't critical but
    as we have counts of outstanding operations we may as well
    try and use the least loaded (lowest count)
*/
static int pick_physical_pipe( struct x10i_instance *instance )
{
	int min_outstanding_ops = 0xFFF;
	int min_outstanding_pipe = 0;
	int x;
	int t;

	for ( x = 0; x < MAX_PPIPE_NO; x++ )
	{
		t = atomic_read( &instance->ppipes[x].operation_count );
		if ( t < min_outstanding_ops )
		{
			min_outstanding_ops = t;
			min_outstanding_pipe = x;
		}
	}
	return( min_outstanding_pipe );
}


/*
 these ease mapping logical to physical pipes
*/
static int tx_pipe_no[] = { 2, 4 };
static int rx_pipe_no[] = { 6, 8 };

/*
Transmit is simplier than rx we have a vpipe, a command, a physical pipe and a user pointer
First allocate a URB
then allocate a suitable DMA friendly buffer
then we assemble the X10i command inside the buffer in a sort of poor mans writev operation
then we submit the URB and wait for completion
*/
static int handle_vpipe_tx (    struct x10i_instance *instance,
                                unsigned char command,
                                int vpipe_no, int ppipe_no,
                                unsigned char *user_tx_buffer,   /* user space address */
                                int tx_buffer_size )
{
	struct tx_completion done;
	struct urb *urb;
	int ret_value = 0;
	int tx_size;
	unsigned char *tx_buffer;

	init_completion( &done.finish );

	tx_size = tx_buffer_size + 2;
	urb = usb_alloc_urb( 0, GFP_KERNEL );

	if ( !urb )
	{
		dprintk( "failed to alloc urb\n" );
		return( -1 );
	}

	// is mapped to usb_alloc_coherent(..) on 2.6.35++
	tx_buffer = usb_buffer_alloc( instance->udev, tx_size, GFP_KERNEL, &urb->transfer_dma );

	*tx_buffer = vpipe_no;
	*(tx_buffer + 1) = command;
	if ( user_tx_buffer )
	{
		if ( copy_from_user( tx_buffer + 2, user_tx_buffer, tx_buffer_size ) )
		{
			dprintk( "copy from user failed %d\n", __LINE__ );
			ret_value = -1;
			goto error;
		}
	}

	usb_fill_bulk_urb(	urb,
						instance->udev,
					 	usb_sndbulkpipe( instance->udev, tx_pipe_no[ppipe_no] ),
					 	tx_buffer, tx_size, x10i_tx_complete, &done );

	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	add_urb_to_list( instance, urb );

	ret_value = usb_submit_urb( urb, GFP_KERNEL );
	if ( ret_value )
	{
		dprintk( "submit urb failed ret= %d size =%d\n", ret_value, tx_size );
		ret_value = -1;
		goto error;
	}

	if ( !wait_for_completion_timeout( &done.finish, 15 * HZ ) )
	{
		dprintk( "wait_for_completion_timeout\n" );
		usb_kill_urb( urb );
		ret_value = -EBUSY;
	}
	else
	{
		ret_value = done.operation_status;
	}

error:
	remove_urb_from_list( instance, urb );

	// is mapped to usb_free_coherent(..) on 2.6.35++
	usb_buffer_free( urb->dev, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma );

	usb_free_urb( urb );

	return( ret_value );
}


/*
    RX is more complex than TX the X10i replies are not ordered so
    we cann't guarentee the next packet we receive was intended for us.
    Solution the driver uses an atomic counter of outstanding operations if
    the counter is zero (we check for one because the atomic op returns the
    value after we have incremented it) then the driver needs to start the asynchrous
    reception.

    The driver submits a rx URB if no reception is happening. It then waits for the
    vpipe operation to complete.
    The URB completetion operation will copy data to the correct v pipe and wait up
    this function
*/
static int handle_vpipe_rx( struct x10i_instance *instance, unsigned char command,
							int vpipe_no, int ppipe_no, unsigned char *user_rx_buffer,   /* user space address */
                            unsigned long *rx_buffer_size, int *error_code)
{
	struct urb *urb = NULL;
	unsigned char *rx_buffer;
	unsigned long retValue;
	int ret_value = 0;
	int error_return;
	int size_return;
	int buffer_size;
	int pipe;
	int our_urb = 0;
	int done = 0;

	while( !done )
	{
		if ( atomic_inc_return( &instance->ppipes[ppipe_no].operation_count ) == 1 )
		// if no others are reading this pipe then start reads
       {
			our_urb = 1;
			pipe = usb_rcvbulkpipe( instance->udev, rx_pipe_no[ppipe_no] );
			buffer_size = MAX_BULK_SIZE;
			urb = usb_alloc_urb( 0, GFP_KERNEL );

			// is mapped to usb_alloc_coherent(..) on 2.6.35++
			rx_buffer = usb_buffer_alloc( instance->udev, buffer_size, GFP_KERNEL, &urb->transfer_dma );

			usb_fill_bulk_urb(	urb,
								instance->udev,
								pipe,
								rx_buffer,
								MAX_BULK_SIZE,
								x10i_rx_complete,
								&instance->ppipes[ppipe_no] );

			urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
			add_urb_to_list( instance,urb );

			ret_value = usb_submit_urb( urb, GFP_KERNEL );
		}                             // this allows the interrupt routine to delete it later

		if ( ret_value )
		{
			dprintk( "  usb_submit_urb failed in %s\n",  __FUNCTION__ );
			return( ret_value );
		}

		if ( !wait_for_completion_timeout( &instance->vpipes[vpipe_no].finish, 5 * HZ ) )        //wait on vpipe
		{
			dprintk( "wait_for_completion_timeout vpipe =%d\n", vpipe_no );

			if ( our_urb )
			{
				usb_kill_urb( urb );
			}

			atomic_dec( &instance->ppipes[ppipe_no].operation_count );
			return( -EBUSY );
		}
		else
		{
			done = 1;
		}
	}

	if ( ( instance->vpipes[vpipe_no].size >= 2 ) &&
		 ( instance->vpipes[vpipe_no].rx_buffer[0] == command ) &&
		 ( instance->vpipes[vpipe_no].rx_buffer[1] == USB_MESSAGE_EXECUTION_SUCCESS ) )
	{
		size_return = instance->vpipes[vpipe_no].size - 2;
		retValue = copy_to_user( user_rx_buffer, instance->vpipes[vpipe_no].rx_buffer + 2, size_return );
		error_return = USB_MESSAGE_EXECUTION_SUCCESS;
	}
	else
	{
		dprintk( "X10I call %d on pipe %d returned error code %d\n", instance->vpipes[vpipe_no].rx_buffer[0],
                                                                     vpipe_no,
                                                                     instance->vpipes[vpipe_no].rx_buffer[1] );

		size_return = 0;
		error_return = (instance->vpipes[vpipe_no].size >= 2) ? instance->vpipes[vpipe_no].rx_buffer[1] : USB_DEVICE_BYTE_COUNT_ERROR;
	}

	retValue = copy_to_user( rx_buffer_size, &size_return, sizeof( size_return ) );
	retValue = copy_to_user( error_code, &error_return, sizeof( error_return ) );

	return( instance->vpipes[vpipe_no].operation_status );
}

/*
    Handles virtual pipe IO requests for an X10i
*/
static int handle_vpipe_request( struct x10i_instance *instance, unsigned char command,
								int vpipe_no, unsigned char *user_tx_buffer,    	/* user space address */
								int tx_buffer_size, unsigned char *user_rx_buffer,  /* user space address	*/
								unsigned long *rx_buffer_size, int *error_code)
{
	int physical_pipe;
	int ret_value = -ENOMEM;

	if ( down_interruptible( &instance->vpipes[vpipe_no].pipe_mutex ) )      //lock virtual pipe
	{
		ret_value = -ERESTARTSYS;
		dprintk("Failed to get pipe \n");

		return( ret_value );
	}

	atomic_set( &instance->vpipes[vpipe_no].in_use, 1 );
	physical_pipe = pick_physical_pipe( instance );        //choose physical pipe to use
	ret_value = handle_vpipe_tx( instance, command, vpipe_no, physical_pipe, user_tx_buffer, tx_buffer_size );
	if ( ret_value == 0 )
	{
		ret_value = handle_vpipe_rx( instance, command, vpipe_no, physical_pipe, user_rx_buffer, rx_buffer_size, error_code );
		if ( ret_value )
		{
			dprintk( "Failed to do rx vpipe= %d\n", vpipe_no );
		}

	}
	else
	{
		dprintk( "Failed to do tx vpipe =%d \n", vpipe_no );
	}

	atomic_set( &instance->vpipes[vpipe_no].in_use, 0 );
	up( &instance->vpipes[vpipe_no].pipe_mutex );  //release virtual pipe

	return( ret_value );
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)

	static int x10i_proc_dump_usb_pipes( struct seq_file *m, struct X10I_driver_context *X10I_context )
	{
		int i;

		for ( i = 0; i < MAX_PPIPE_NO; i++ )
		{
			seq_printf(m, "physical pipe %d op count =%d\n", i, atomic_read( &X10I_context->instance.ppipes[i].operation_count ) );
		}

		for ( i = 0; i < MAX_VPIPE_NO; i++ )
		{
			seq_printf( m, "virtual pipe %d use count =%d\n", i, atomic_read( &X10I_context->instance.vpipes[i].in_use ) );
		}

		return 0;
	}

	static int x10_proc_dump_usb_pipes( struct seq_file *m, struct driver_context *ptr )
	{
		int i;

		for ( i = 0; i < NUMBER_OF_PIPES; i++ )
		{
			seq_printf( m, "pipe no %d use count%d ", i, ptr->Pipes[i].InUse );
		}

		return 0;
	}

	/*====================================================================
	  This function can report the virtual/physical USB pipes being used by
	  attached X10 devices via the /proc filesystem, in response to eg.
	  'cat /proc/driver/Xline_pipe_status' command
	=====================================================================*/
	static int proc_dump_usb_pipes_show(struct seq_file *m, void *v)
	{
		struct driver_context *ptr = ContextList;
		struct X10I_driver_context *X10I_context;

		while ( ptr )
		{
			// Only the X10i and X15 use "virtual pipes".
			// The X10, XSpin and XLuminate boards use real pipes.

			if ( ( ptr->Type == X10i ) || ( ptr->Type == X15 ) )
			{
				X10I_context = (struct X10I_driver_context *) ptr ;
				x10i_proc_dump_usb_pipes( m, X10I_context );
			}
			else
			{
				x10_proc_dump_usb_pipes( m, ptr );
			}

			ptr = ptr->next;
		}

		return 0;
	}

	/*====================================================================
	  This function can report the device minor numbers of attached X10 devices
	  via the /proc filesystem, in response to eg. 'cat /proc/driver/Xline'
	  command.
	=====================================================================*/
	static int procmem_proc_show(struct seq_file *m, void *v )
	{
		struct driver_context *ptr = ContextList;

		while ( ptr )
		{
			if ( ptr->Type != LOADER )
			{
				seq_printf( m, "%d\n", ptr->Number );
			}
			ptr = ptr->next;
		}

		return 0;
	}

	static int proc_dump_usb_pipes_open(struct inode *inode, struct file *file)
	{
		return single_open(file, proc_dump_usb_pipes_show, PDE_DATA(inode));
	}

	static int procmem_proc_open(struct inode *inode, struct file *file)
	{
		return single_open(file, procmem_proc_show, PDE_DATA(inode));
	}

	/* report device minor numbers */
	static const struct file_operations procmem_proc_fops = {
		.open		= procmem_proc_open,
		.read		= seq_read,
		.llseek		= seq_lseek,
		.release	= seq_release,
	};

	/* report virtual/physical pipe numbers */
	static const struct file_operations proc_dump_usb_pipes_fops = {
		.open		= proc_dump_usb_pipes_open,
		.read		= seq_read,
		.llseek		= seq_lseek,
		.release	= seq_release,
	};

#else // Kernel older than 3.10

	static int x10i_proc_dump_usb_pipes( char *buffer, struct X10I_driver_context *X10I_context )
	{
		int i;
		int len = 0;

		for ( i = 0; i < MAX_PPIPE_NO; i++ )
		{
			len += sprintf( buffer + len, "physical pipe %d op count =%d\n", i, atomic_read( &X10I_context->instance.ppipes[i].operation_count ) );
		}

		for ( i = 0; i < MAX_VPIPE_NO; i++ )
		{
			len += sprintf( buffer + len, "virtual pipe %d use count =%d\n", i, atomic_read( &X10I_context->instance.vpipes[i].in_use ) );
		}

		return( len );
	}

	static int x10_proc_dump_usb_pipes( char *buffer, struct driver_context *ptr )
	{
		int i;
		int len = 0;

		for ( i = 0; i < NUMBER_OF_PIPES; i++ )
		{
			len += sprintf( buffer + len, "physical pipe no %d use count%d ", i, ptr->Pipes[i].InUse );
		}

		return( len );
	}

	/*====================================================================
	  This function can report the virtual/physical USB pipes being used by
	  attached X10 devices via the /proc filesystem, in response to eg.
	  'cat /proc/driver/Xline_pipe_status' command
	=====================================================================*/
	static int proc_dump_usb_pipes( char *buffer, char **start, off_t offset, int len, int *unused, void *data )
	{
		struct driver_context *ptr = ContextList;
		struct X10I_driver_context *X10I_context;

		len = 0;
		while ( ptr )
		{
			// Only the X10i and X15 use "virtual" and "physical".USB pipes.
			// The X10, XSpin and XLuminate boards use "physical" pipes only.
			if ( ( ptr->Type == X10i ) || ( ptr->Type == X15 ) )
			{
				X10I_context = (struct X10I_driver_context *) ptr ;
				len += x10i_proc_dump_usb_pipes( buffer + len, X10I_context );
			}
			else
			{
				len += x10_proc_dump_usb_pipes( buffer + len, ptr );
			}

			ptr = ptr->next;
		}

		return ( len );
	}

	/*====================================================================
	  This function can report the device minor numbers of attached X10 devices
	  via the /proc filesystem, in response to eg. 'cat /proc/driver/Xline'
	  command.
	=====================================================================*/
	static int proc_read_procmem( char *buffer, char **start, off_t offset, int len, int *unused, void *data )
	{
		struct driver_context *ptr = ContextList;

		len = 0;
		while ( ptr )
		{
			if ( ptr->Type != LOADER )
			{
				len += sprintf( buffer + len, "%d\n", ptr->Number );
			}
			ptr = ptr->next;
		}

		return( len );
	}
#endif

/*====================================================================
  All the action starts here
 =====================================================================*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	// Function pointed to by 'unlocked_ioctl' pointer in file-operations pointer structure.
	// Since kernel version 2.6.11, linux has been progressively removing the use of the Big Kernel Lock (BKL).
	// Older kernels activated BKL before calling into 'ioctl' entry point. This could mean the kernel remained
	// locked for long periods. More recently, and exclusively as of 2.6.36, the kernel remains unlocked at the
	// point of entering 'unlocked_ioctl' allowing locking to be moved down to just where its needed inside the
	// driver. This migration will also occur with the Xline driver over time. For now we mimic the earlier
	// BKL-at-entry semantics.
	// See article "The new way of ioctl()" by Jonathan Corbet at http://lwn.net/Articles/119652
	static long XlineIoctlUnlk( struct file *file_ptr, unsigned int command, unsigned long arg )
	{
		unsigned long retValue;
		#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
			mutex_lock( &xline_mutex );
		#else
			lock_kernel( );
		#endif

		retValue = (unsigned long)XlineIoctl( file_ptr->f_dentry->d_inode, file_ptr, command, arg);

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
			mutex_unlock( &xline_mutex );
		#else
			unlock_kernel( );
		#endif

		return retValue;
	}
#endif

// Ioctl entry for traditional drivers (those with BKL active at entry) prior to the 2-way
// unlocked_ioctl/compar_ioctl split.
// The original 'ioctl' file-operations entry point has been removed completely as of 2.6.36
static int XlineIoctl( struct inode *inode_ptr, struct file *file_ptr, unsigned int command, unsigned long arg )
{
	struct driver_context *context = file_ptr->private_data;
	unsigned long retValue;
	int ReturnValue=0;
	X10_COMMAND *X10Command = (X10_COMMAND *)arg;
	X10_COMMAND *KernelCommandCopy;
	struct X10I_driver_context *X10I_context;
	struct X10I_COMMAND user_params;

	if ( context == NULL )
	{
		return( -EFAULT );
	}

	if ( ( context->dev == NULL ) || ( context->Type == INVALID_CONNECTION ) )
	{
		return ( -EFAULT );
	}

	if ( command == IOCTL_X10I_COMMAND )
	{
		// Only the X10i and X15 use "virtual pipes".
		// The X10, XSpin, XLuminate and XTopper boards use real pipes.
		if ( ( context->Type == X10i ) || ( context->Type == X15 ) )
		{
			// We pass user space pointers down to the inner code to avoid wasteful redundant copies
	 		X10I_context = file_ptr->private_data;
			retValue = copy_from_user( &user_params, (void *)arg, sizeof( struct X10I_COMMAND ) );

			ReturnValue = handle_vpipe_request(	&X10I_context->instance,
												user_params.command,
												user_params.virtual_pipe_number,
												user_params.user_tx_buffer,
												user_params.user_tx_buffer_size,
												user_params.user_rx_buffer,
												user_params.user_rx_buffer_size,
												user_params.error_code );

			return( ReturnValue );
		}
	}

	if ( !access_ok( VERIFY_WRITE, X10Command, sizeof( X10_COMMAND ) ) )
	{
		return( -EFAULT );
	}

	KernelCommandCopy = (X10_COMMAND *)kmalloc( sizeof( X10_COMMAND ), GFP_KERNEL | __GFP_DMA );
	if ( !KernelCommandCopy )
	{
		return( -ENOMEM );
	}

	if ( copy_from_user( KernelCommandCopy, X10Command, sizeof( X10_COMMAND ) ) )
	{
		kfree( KernelCommandCopy );
		return( -EFAULT );
	}

	// Check that we know the IOCTL code and the user buffer is accessible
	switch( command )
	{
	case IOCTL_X10_COMMAND:
		ReturnValue = DoX10IO( context, KernelCommandCopy );
		break;

	case IOCTL_X10_READ:
		ReturnValue = DoX10Read( context, KernelCommandCopy );
		break;

	case IOCTL_X10_WRITE:
		ReturnValue = DoX10Write( context, KernelCommandCopy );
		break;

	default:
		dprintk( "X-line Failed to access user buffer \n" );
		ReturnValue = -ENOTTY;
	}

	if ( copy_to_user( X10Command, KernelCommandCopy, sizeof( X10_COMMAND ) ) )
	{
		ReturnValue = -EFAULT;
	}

	kfree( KernelCommandCopy );

	return( ReturnValue );

}

/*====================================================================
  X10 open the driver supports multiple X10's so we must track down the
  right one. Get the device minor number and then search the linked list
  of driver contexts for a device that matches. If we find a match
  then we record the context in the struct file's private data member and
  allow the open to succeed
=====================================================================*/
static int XlineOpen( struct inode *inode_ptr, struct file *file_ptr )
{
	int MinorNumber = MINOR( inode_ptr->i_rdev );
	struct driver_context *ptr = ContextList;

	dprintk( "Open called X-line driver Minor Number =%d\n", MinorNumber );
	while ( ptr && ptr->Number != MinorNumber )
	{
		ptr = ptr->next;
	}

	if ( ptr )
	{
		file_ptr->private_data = ptr;
		dprintk( "Open succeeded MinorNumber =%d \n", ptr->Number );
		open_driver_context( ptr );

		return( 0 );
	}
	else
	{
		return( -ENODEV );           //Failed to find a connected X10 with the right device number
	}
}

/*====================================================================
  The close just decrements the driver modules reference count
=====================================================================*/
static int XlineRelease( struct inode *inode_ptr, struct file *file_ptr )
{
	struct driver_context *context = file_ptr->private_data;

	release_driver_context( context );

	dprintk( "Release Called X-line driver\n" );

	return( 0 );
}


/*====================================================================
  Defines a table of file operations for an X10 device
  This is a table of callbacks
=====================================================================*/
static struct file_operations X10_file_operations = {
							// struct module * owner;
	open:XlineOpen,			// int (*open)          ( struct inode *, struct file * );
	release:XlineRelease,	// int (*release)       ( struct inode *, struct file * );
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
		unlocked_ioctl:XlineIoctlUnlk,			// long (*ioctl)         ( struct file *, unsigned int, unsigned long );
		// compat_ioctl used for parameter translation in 64-bit linux
		// compat_ioctl:XlineIoctlCompat,		// long (*ioctl)         ( struct file *, unsigned int, unsigned long );
	#else
		// older 'ioctl' pointer removed in 2.6.36, being replaced with 'unlocked_ioctl'
		ioctl:XlineIoctl,		// int (*ioctl)         ( struct inode *, struct file *, unsigned int, unsigned long );
	#endif
							// loff_t (*llseek)     ( struct file *, loff_t, int );
							// ssize_t (*read)      ( struct file *, char *, size_t, loff_t * );
							// ssize_t (*write)     ( struct file *, const char *, size_t, loff_t * );
							// int (*readdir)       ( struct file *, void *, filldir_t );
							// unsigned int (*poll) ( struct file *, struct poll_table_struct * );
							// int (*mmap)          ( struct file *, struct vm_area_struct * );
							// int (*flush)         ( struct file * );
							// int (*fsync)         ( struct file *, struct dentry *, int datasync );
							// int (*fasync)        ( int, struct file *, int );
							// int (*lock)          ( struct file *, int, struct file_lock * );
							// ssize_t (*readv)     ( struct file *, const struct iovec *, unsigned long, loff_t * );
							// ssize_t (*writev)    ( struct file *, const struct iovec *, unsigned long, loff_t * );
};

/*====================================================================
  Each USB Pipe was an individual wait queue and flag this is to
  lock a pipe so each pair of operations write then read appears effectively
  atomic from user space
  Each device context has an array of pipe stuctures
====================================================================*/
static void InitPipe( struct Pipe *pipe )
{
	init_waitqueue_head( &pipe->queue );
	pipe->InUse = 0;
}


/*====================================================================
  For each X10 attached we have a driver context structure
  this function initialises it
====================================================================*/
static void InitDriverContext( struct driver_context *context )
{
	int x;
	struct driver_context *ptr = ContextList;

	context->True = 1;
	context->False = 0;
	context->Number = 0;
	while ( ptr )
	{
		if ( ptr->Number == context->Number )
		{
			context->Number = ptr->Number + 1;
			ptr = ContextList;
		}
		else
		{
			ptr = ptr->next;
		}
	}

	printk( "X-line Minor Number = %d\n", context->Number );
	atomic_set( &context->reference_count, 1 );

	for ( x = 0; x < NUMBER_OF_PIPES; x++ )
	{
		InitPipe( &context->Pipes[x] );
	}
}

/*====================================================================
  This function performs most X10 operations write/read pairs from
  the bulk pipes specified in the X10_COMMAND structure
====================================================================*/
static int DoX10IO( struct driver_context *context, X10_COMMAND * KernelCommandCopy )
{
	int CommandPipeIndex;
	int ReturnValue = 0;
	wait_queue_t wait;
	int Length;
	int retry_count;

	CommandPipeIndex = KernelCommandCopy->CommandpipeNum - 1;
	init_waitqueue_entry( &wait, current );
	add_wait_queue( &context->Pipes[CommandPipeIndex].queue, &wait );

	while( 1 )
	{
		set_current_state( TASK_INTERRUPTIBLE );

		if ( context->Pipes[CommandPipeIndex].InUse == 0 )
			break;

		schedule( );
	}

	set_current_state( TASK_RUNNING );
	remove_wait_queue( &context->Pipes[CommandPipeIndex].queue, &wait );
	context->Pipes[CommandPipeIndex].InUse = 1;

	retry_count = 0;
	while ( retry_count < MAX_USB_RETRIES )
	{
		if ( usb_bulk_msg(	context->dev,
							usb_sndbulkpipe( context->dev, KernelCommandCopy->CommandpipeNum ),
							&KernelCommandCopy->Buffer,
							KernelCommandCopy->CommandLength,
							&Length, HZ * TIMEOUT_MULT_HZ ) < 0 )
		{
			dprintk( "<1>X-line usb bulk write to  pipe %d failed\n", KernelCommandCopy->CommandpipeNum );
			retry_count++;
		}
		else
		{
			break;
		}
	}

	if ( retry_count >= MAX_USB_RETRIES )
	{
		dprintk( "X-line retry count exceeded %d\n", KernelCommandCopy->CommandpipeNum );
		ReturnValue = -ENOTTY;
	}
	else
	{
		retry_count = 0;
		while ( retry_count < MAX_USB_RETRIES )
		{
			if ( usb_bulk_msg (	context->dev,
								usb_rcvbulkpipe( context->dev, KernelCommandCopy->AnswerpipeNum ),
								&KernelCommandCopy->Buffer,
								sizeof( KernelCommandCopy->Buffer ),
								&KernelCommandCopy->AnswerLength,
								HZ * TIMEOUT_MULT_HZ ) < 0 )
			{
				dprintk( "X-line usb bulk read from  pipe %d  failed\n", KernelCommandCopy->AnswerpipeNum );
				retry_count++;
			}
			else
			{
				break;
			}
		}

		if ( retry_count >= MAX_USB_RETRIES )
		{
			dprintk( "<1>X-line retry count exceeded %d\n", KernelCommandCopy->CommandpipeNum );
			ReturnValue = -ENOTTY;
		}
	}

	context->Pipes[CommandPipeIndex].InUse = 0;
	wake_up( &context->Pipes[CommandPipeIndex].queue );

	return( ReturnValue );
}

/*====================================================================
  This function performs an X10 "read"
====================================================================*/
static int DoX10Read( struct driver_context *context, X10_COMMAND * KernelCommandCopy )
{
	int ReturnValue = 0;
	wait_queue_t wait;
	int pipeNumber;

	pipeNumber = KernelCommandCopy->AnswerpipeNum;
	KernelCommandCopy->AnswerpipeNum = (pipeNumber * 2) + 2;
	init_waitqueue_entry( &wait, current );
	add_wait_queue( &context->Pipes[pipeNumber].queue, &wait );

	while( 1 )
	{
		set_current_state( TASK_INTERRUPTIBLE );

		if ( context->Pipes[pipeNumber].InUse == 0 )
			break;

		schedule( );
	}

	set_current_state( TASK_RUNNING );
	remove_wait_queue( &context->Pipes[pipeNumber].queue, &wait );
	context->Pipes[pipeNumber].InUse = 1;

	if ( usb_bulk_msg(	context->dev,
						usb_rcvbulkpipe( context->dev, KernelCommandCopy->AnswerpipeNum ),
						&KernelCommandCopy->Buffer,
						sizeof( KernelCommandCopy->Buffer ),
						&KernelCommandCopy->AnswerLength,
						HZ * TIMEOUT_MULT_HZ ) < 0 )
	{
		ReturnValue = -ENOTTY;
	}

	context->Pipes[pipeNumber].InUse = 0;
	wake_up( &context->Pipes[pipeNumber].queue );

	return( ReturnValue );
}

/*====================================================================
  This function performs an X10 "write"
====================================================================*/
static int DoX10Write( struct driver_context *context, X10_COMMAND * KernelCommandCopy )
{
	int ReturnValue = 0;
	wait_queue_t wait;
	int pipeNumber;
	int retry_count;

	pipeNumber = KernelCommandCopy->CommandpipeNum;
	KernelCommandCopy->CommandpipeNum = (pipeNumber * 2) + 2;

	init_waitqueue_entry( &wait, current );
	add_wait_queue( &context->Pipes[pipeNumber].queue, &wait );

	while (1)
	{
		set_current_state( TASK_INTERRUPTIBLE );

		if (context->Pipes[pipeNumber].InUse == 0)
			break;

		schedule( );
	}

	set_current_state( TASK_RUNNING );
	remove_wait_queue( &context->Pipes[pipeNumber].queue, &wait );
	context->Pipes[pipeNumber].InUse = 1;

	retry_count = 0;
	while ( retry_count < MAX_USB_RETRIES )
	{
		ReturnValue = usb_bulk_msg( context->dev,
									usb_sndbulkpipe (context->dev, KernelCommandCopy->CommandpipeNum),
									&KernelCommandCopy->Buffer,
									KernelCommandCopy->CommandLength,
									&KernelCommandCopy->AnswerLength,
									HZ * TIMEOUT_MULT_HZ);
		if ( ReturnValue < 0 )
		{
			retry_count++;
		}
		else
		{
			break;
		}
	}

	if ( retry_count >= MAX_USB_RETRIES )
	{
		ReturnValue = -ENOTTY;
	}

	context->Pipes[pipeNumber].InUse = 0;
	wake_up( &context->Pipes[pipeNumber].queue );

	return( ReturnValue );
}

/*====================================================================
  Adds new driver context into linked list
=====================================================================*/
static inline int link_new_driver_context( struct driver_context *new_context )
{
	new_context->next = ContextList;
	ContextList = new_context;

	return( 0 );
}

/*====================================================================
  Removes an X10 from the master linked list
=====================================================================*/
static inline void unlink_driver_context( struct driver_context *context )
{
	struct driver_context **ptr = &ContextList;

	while( *ptr )
	{
		if( *ptr == context )
		{
			*ptr = context->next;
			return;
		}

		ptr = &(*ptr)->next;
	}
}


/*====================================================================
  Unlinks a context from linked list and frees its resources
  Because an application could still be holding a pointer to
  the invalid context. We unlink the context make it as invalid
  and decrement the context reference count
=====================================================================*/
static void free_driver_resources (struct driver_context *context)
{
	unlink_driver_context( context );
	context->Type = INVALID_CONNECTION;
	release_driver_context( context );
}



/*====================================================================
  Allocates and initalises a driver context structure
=====================================================================*/
static struct driver_context *XlineAllocateDriverResources( int ID, struct usb_device *dev )
{
	struct driver_context *ptr;
	struct X10I_driver_context *ptr1;

	// Only the X10i and X15 use "virtual pipes".
	// The X10, XSpin, XLuminate and XTopper boards use real pipes.
	if ( ( ID == X10i ) || ( ID == X15 ) )
	{
		ptr1 = (struct X10I_driver_context *)kmalloc( sizeof( struct X10I_driver_context ), GFP_KERNEL | __GFP_DMA );
		init_x10i_instance( &ptr1->instance, dev );
		ptr = &ptr1->context;
	}
	else
	{
		ptr = (struct driver_context *)kmalloc( sizeof( struct driver_context ), GFP_KERNEL | __GFP_DMA );
	}

	if ( ptr )
	{
		InitDriverContext( ptr );
		link_new_driver_context( ptr );
	}

	return( ptr );
}


/*====================================================================
  Issues a reset command to the X10
=====================================================================*/
static int XlineReset( struct usb_device *dev, struct driver_context *context, unsigned char Reset )
{
	int ReturnValue;
	unsigned char *Ptr;

	Ptr = Reset ? &context->True : &context->False;

	// Toggle the EZ-USB reset bit (harmless on the FX2 )
	ReturnValue = usb_control_msg(	dev,
								 	usb_sndctrlpipe (dev, 0),                           /* pipe */
								 	ANCHOR_LOAD_INTERNAL,                               /* request */
								 	USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,   /* request Type */
								 	CPUCS_REG_EZUSB,                                    /* message value */
								 	0,                                                  /* message Index */
								 	Ptr,                                                /* Data Ptr */
								 	1,                                                  /* Data Size */
								 	HZ * 10 );                                          /* timeout */

	// Toggle the FX2 reset bit (harmless on the EZ-USB )
	ReturnValue = usb_control_msg(	dev,
								 	usb_sndctrlpipe (dev, 0),                          /* pipe */
								 	ANCHOR_LOAD_INTERNAL,                              /* request */
								 	USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,  /* request Type */
								 	CPUCS_REG_FX2,                                     /* message value */
								 	0,                                                 /* message Index */
								 	Ptr,                                               /* Data Ptr */
								 	1,                                                 /* Data Size */
								 	HZ * 10 );                                         /* timeout */
	return( ReturnValue < 0 );
}

/*====================================================================
  Sends an Intel Hex record via the control pipe to an X10
=====================================================================*/
static int SendHexRecordToControl( struct usb_device *dev, unsigned int Type, PINTEL_HEX_RECORD SourcePtr )
{
	PINTEL_HEX_RECORD Ptr;
	unsigned int NumberToDo = SourcePtr->Length;
	unsigned short int TargetAddress = SourcePtr->Address;
	unsigned int NumberThisTime;
	unsigned int NumberDone = 0;
	int retry_count, ReturnValue;
	const unsigned int MaxToDo = 16;

	Ptr = kmalloc( sizeof( INTEL_HEX_RECORD ), GFP_KERNEL | __GFP_DMA );
	if (!Ptr)
	{
		dprintk( "X-line kmalloc failed in %s Line %d\n", __FUNCTION__, __LINE__ );
		return( 0 );
	}

	memcpy( Ptr, SourcePtr, sizeof( INTEL_HEX_RECORD ) );
	while ( NumberToDo )
	{
		NumberThisTime = NumberToDo < MaxToDo ? NumberToDo : MaxToDo;
		retry_count = 0;
		while ( retry_count < MAX_USB_RETRIES )
		{
			// This seems to fail commonly on the first retry when running the X10i in high speed mode. It always
			// seems to correct itself on the second retry - a timeout of only one second is therefore required.
			ReturnValue = usb_control_msg(	dev,
											usb_sndctrlpipe( dev, 0 ),
											Type,
											USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_DIR_OUT,
											TargetAddress,
											0,
											&Ptr->Data[NumberDone],
											NumberThisTime,
											HZ * 1 );
			if ( ReturnValue < 0 )
			{
				dprintk( "<1>Control pipe retry %d\n", retry_count );
				retry_count++;
			}
			else
			{
				break;
			}
		}
		if (retry_count >= MAX_USB_RETRIES)
		{
			dprintk( "<1>Transfer to control Pipe zero Failed\n" );
			kfree( Ptr );
			return( 0 );
		}
		NumberDone += NumberThisTime;
		NumberToDo -= NumberThisTime;
		TargetAddress += NumberThisTime;
	}

	kfree( Ptr );
	return( 1 );
}

/*====================================================================
  Downloads a block of Intel hex records to the X10
=====================================================================*/
static int DownloadIntelHex( struct usb_device *dev, struct driver_context *context, PINTEL_HEX_RECORD hexRecord )
{
	PINTEL_HEX_RECORD SrcPtr = hexRecord;
	int TransferCount = 0;

	while ( SrcPtr->Type == 0 )
	{
		if ( !INTERNAL_RAM( SrcPtr->Address ) )
		{
			if ( !SendHexRecordToControl( dev, ANCHOR_LOAD_EXTERNAL, SrcPtr ) )		// vendor-A3 command to access memory over 0x4000
			{
				return( 0 );
			}
			else
			{
				TransferCount += SrcPtr->Length;
			}
		}
		SrcPtr++;
	}

	XlineReset( dev, context, 1 );
	SrcPtr = hexRecord;
	TransferCount = 0;

	while ( SrcPtr->Type == 0 )
	{
		if ( INTERNAL_RAM( SrcPtr->Address ) )
		{
			if ( !SendHexRecordToControl( dev, ANCHOR_LOAD_INTERNAL, SrcPtr ) )		// vendor-A0 command thats part of ezusb core
			{
				return( 0 );
			}
			else
			{
				TransferCount += SrcPtr->Length;
			}
		}
		SrcPtr++;
	}

	return( 1 );
}


static int handle_loader_connect( struct usb_interface *interface, struct usb_device *dev, const struct usb_device_id *id )
{
    struct driver_context *context;
    int downloadSuccess = 0;

	context = XlineAllocateDriverResources( dev->descriptor.idProduct, dev );
	context->Type = LOADER;

	/* Download the stub loader (implements vendor-A3 command) -- not strictly necessary given the Heber locode/highcode mechanism */
	XlineReset( dev, context, 1 );
	if ( !DownloadIntelHex( dev, context, loader ) )
	{
		free_driver_resources( context );
		return( -ENOMEM );
	}

	XlineReset( dev, context, 0 );

	/* Download locode portion of device firmware (between 0-0x3FFF), hicode will be downloaded by init of fflyusb.so */
	switch( dev->descriptor.idProduct )
	{
	case X10_LOADER:
		downloadSuccess = DownloadIntelHex( dev, context, firmware );
		break;

	case X10i_LOADER:
		downloadSuccess = DownloadIntelHex( dev, context, X10I_firmware );
		break;

	case X15_LOADER:
		downloadSuccess = DownloadIntelHex( dev, context, X15_firmware );
		break;

	case XSPIN_LOADER:
		downloadSuccess = DownloadIntelHex( dev, context, XSpin_firmware );
		break;

	case XLUMINATE_LOADER:
		downloadSuccess = DownloadIntelHex( dev, context, xluminate_firmware );
		break;

	case XTOPPER_LOADER:
		downloadSuccess = DownloadIntelHex( dev, context, XTopper_firmware );
		break;

	default:
		downloadSuccess = 0;
	}

	if ( downloadSuccess == 0 )
	{
		dprintk( "<1>Firmware download error.\n" );
		free_driver_resources( context );
		return( -ENOMEM );
	}

	/* Issue a reset to the X board. The board will disconnect and reconnect with */
	/* alternate Product ID ( eg X10i_LOADER will become X10i, triggering handle_device_connect() ) */
	XlineReset( dev, context, 1 );
	XlineReset( dev, context, 0 );

	/* Probe return changes from 2.4 to 2.6 */
	dev_set_drvdata( &interface->dev, context );

	return( 0 );
}

static int handle_device_connect( struct usb_interface *interface, struct usb_device *dev, const struct usb_device_id *id )
{
	struct driver_context *context;
	unsigned long maxPacketSize;

	switch( dev->descriptor.idProduct )
	{
	case X10:
		printk( "<1>Heber X10 board found.\n" );
		break;

	case X10i:
		printk( "<1>Heber X10i board found.\n" );
		break;

	case X15:
		printk( "<1>Heber X15 board found.\n" );
		break;

	case XSPIN:
		printk( "<1>Heber XSpin board found.\n" );
		break;

	case XLUMINATE:
		printk( "<1>Heber XLuminate board found.\n" );
		break;

	case XTOPPER:
		printk( "<1>Heber XTopper board found.\n" );
		break;

	default:
		printk( "<1>Unknown board connected.\n" );
	}

	context = XlineAllocateDriverResources( dev->descriptor.idProduct, dev );
	context->Type = dev->descriptor.idProduct;
	context->dev = dev;
	context->BoardFitted = dev->descriptor.idProduct;

	add_X10_to_sysfs_class( MKDEV( X10_major_number, context->Number ), &dev->dev, "Xline_%d", context->Number );
	if ( dev->config )
	{
		maxPacketSize = dev->config[0].interface[0]->altsetting[0].endpoint[0].desc.wMaxPacketSize;

		switch ( maxPacketSize )
		{
		case 64:
			printk( "<1>X-line board running at full speed.\n");
			context->BoardSpeed = FULL_SPEED;
			break;

		case 512:
			printk( "<1>X-line board running at high speed.\n" );
			context->BoardSpeed = HIGH_SPEED;
			break;

		default:
			printk( "<1>X-line board running at an unknown speed (max packet size = %ld)\n", maxPacketSize );
			context->BoardSpeed = UNKNOWN_SPEED;
		}
	}
	else
	{
		dprintk( "Couldn't retrieve descriptors. Unknown board speed.\n" );
		context->BoardSpeed = UNKNOWN_SPEED;
	}

	mdelay( 10 );

	if ( usb_set_interface( dev, 0, 0 ) )
	{
		dprintk( "set interface failed\n" );
		free_driver_resources( context );
		return( -ENOMEM );
	}

	printk( "<1>Heber X-line Board ready.\n" );
	dev_set_drvdata( &interface->dev, context );

	return( 0 );
}

/*====================================================================
  Called by the USB subsystem to see if we recognise any newly attached
  USB devices. Use this as the opportunity to download code into Xline product
=====================================================================*/
static int XlineUSBProbe( struct usb_interface *interface, const struct usb_device_id *id )
{
	struct usb_device *dev = interface_to_usbdev( interface );

	if ( dev->descriptor.idVendor == HEBER )
	{
		switch( dev->descriptor.idProduct )
		{
		case X10_LOADER:
		case X10i_LOADER:
		case X15_LOADER:
		case XSPIN_LOADER:
		case XLUMINATE_LOADER:
		case XTOPPER_LOADER:
			return( handle_loader_connect( interface, dev, id ) );
			break;

		case X10:
		case X10i:
		case X15:
		case XSPIN:
		case XLUMINATE:
		case XTOPPER:
			return( handle_device_connect( interface, dev, id ) );
			break;
		}
	}

	return( -ENODEV );
}

/*====================================================================
  Handle a sudden X10 disconnext
  We track down the context then move it from the linked list so no
  new processes can connect to it
  we also remove it from sysfs so eventually udev will remove the /dev
  entry
=====================================================================*/
static void XlineUSBDisconnect( struct usb_interface *interface )
{
	struct driver_context *context = dev_get_drvdata( &interface->dev );
    struct X10I_driver_context *X10I_context;

    if ( ( context->Type == X10i ) || ( context->Type == X15 ) )
    {
		// Only the X10i and X15 use "virtual pipes".
		// The X10, XSpin and XLuminate boards use real pipes.
        X10I_context = (struct X10I_driver_context *)context;
        cancel_x10i_urbs( &X10I_context->instance );
    }

    remove_X10_from_sysfs_class( MKDEV( X10_major_number, context->Number ) );
	free_driver_resources( context );
}

/*====================================================================
  List of device ID's we want to handle
=====================================================================*/
static struct usb_device_id X10_usb_ids[] = {
	{ USB_DEVICE( HEBER, X10 )              },
	{ USB_DEVICE( HEBER, X10_LOADER )       },
	{ USB_DEVICE( HEBER, X10i )             },
	{ USB_DEVICE( HEBER, X10i_LOADER )      },
	{ USB_DEVICE( HEBER, X15 )              },
	{ USB_DEVICE( HEBER, X15_LOADER )       },
	{ USB_DEVICE( HEBER, XSPIN )            },
	{ USB_DEVICE( HEBER, XSPIN_LOADER )     },
	{ USB_DEVICE( HEBER, XLUMINATE )        },
	{ USB_DEVICE( HEBER, XLUMINATE_LOADER ) },
	{ USB_DEVICE( HEBER, XTOPPER )          },
	{ USB_DEVICE( HEBER, XTOPPER_LOADER )   },
	{                                       },
};

MODULE_DEVICE_TABLE (usb, X10_usb_ids);

/*====================================================================
  structure to pass to USB subsystem
=====================================================================*/
static struct usb_driver XlineDriverDef = {
	name:"Xline",
	probe:XlineUSBProbe,
	disconnect:XlineUSBDisconnect,
	id_table:X10_usb_ids,
};

/*====================================================================
	Defines a name for this driver
=====================================================================*/
static char *X10_name = "Xline";

/*====================================================================
  Called when X10 module is first loaded
=====================================================================*/
static int __init XlineInitModule (void)
{
	int result;

	create_X10_sysfs_class( );                        //let ths sysfs export nasty stuff like pipe sizes

	#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		proc_create_data("driver/Xline", 0, NULL, &procmem_proc_fops, NULL);
		proc_create_data("driver/Xline_pipe_status", 0, NULL, &proc_dump_usb_pipes_fops, NULL);

	#else // kernel older than 3.10
		create_proc_read_entry( "driver/Xline", 0, NULL, proc_read_procmem, NULL );
		create_proc_read_entry( "driver/Xline_pipe_status", 0, NULL, proc_dump_usb_pipes , NULL );

	#endif

	if ( ( result = register_chrdev( X10_major_number, X10_name, &X10_file_operations ) ) < 0 )
	{
		dprintk( "<1> Failed to register X-line char device returned %x\n", result );
		return( result );
	}

	/* major number was dynamically allocated */
	if ( X10_major_number == 0 )
	{
		X10_major_number = result;
	}

	if ( ( result = usb_register( &XlineDriverDef ) ) < 0 )
	{
		dprintk( "<1>Failed to register X-line driver register returned %x\n", result );
		return( -1 );
	}

	return( 0 );
}

/*====================================================================
  Called when X10 module is unloaded
=====================================================================*/
static void __exit XlineCleanupModule (void)
{
	remove_proc_entry( "driver/Xline", NULL );
	remove_proc_entry( "driver/Xline_pipe_status", NULL );

	usb_deregister( &XlineDriverDef );
	unregister_chrdev( X10_major_number, X10_name );

	if ( X10_class )
	{
		destroy_X10_sysfs_class( ); //remove sysfs X10 directory
	}

	printk( "<1> Xline Driver UnLoaded\n" );
}

module_init( XlineInitModule );
module_exit( XlineCleanupModule );
