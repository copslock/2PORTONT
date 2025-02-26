/*
 * xHCI host controller driver
 *
 * Copyright (C) 2008 Intel Corp.
 *
 * Author: Sarah Sharp
 * Some code borrowed from the Linux EHCI driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <asm/unaligned.h>

#include "xhci.h"

static void xhci_hub_descriptor(struct xhci_hcd *xhci,
		struct usb_hub_descriptor *desc)
{
	int ports;
	u16 temp;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	/* USB 3.0 hubs have a different descriptor, but we fake this for now */
	desc->bDescriptorType = 0x29;
	desc->bPwrOn2PwrGood = 10;	/* xhci section 5.4.9 says 20ms max */
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = ports;
	temp = 1 + (ports / 8);
	desc->bDescLength = 7 + 2 * temp;

	/* Why does core/hcd.h define bitmap?  It's just confusing. */
	memset(&desc->DeviceRemovable[0], 0, temp);
	memset(&desc->DeviceRemovable[temp], 0xff, temp);

	/* Ugh, these should be #defines, FIXME */
	/* Using table 11-13 in USB 2.0 spec. */
	temp = 0;
	/* Bits 1:0 - support port power switching, or power always on */
	if (HCC_PPC(xhci->hcc_params))
		temp |= 0x0001;
	else
		temp |= 0x0002;
	/* Bit  2 - root hubs are not part of a compound device */
	/* Bits 4:3 - individual port over current protection */
	temp |= 0x0008;
	/* Bits 6:5 - no TTs in root ports */
	/* Bit  7 - no port indicators */
	desc->wHubCharacteristics = (__force __u16) cpu_to_le16(temp);
}

static unsigned int xhci_port_speed(unsigned int port_status)
{
	if (DEV_LOWSPEED(port_status))
		return 1 << USB_PORT_FEAT_LOWSPEED;
	if (DEV_HIGHSPEED(port_status))
		return 1 << USB_PORT_FEAT_HIGHSPEED;
	if (DEV_SUPERSPEED(port_status))
		return 1 << USB_PORT_FEAT_SUPERSPEED;
	/*
	 * FIXME: Yes, we should check for full speed, but the core uses that as
	 * a default in portspeed() in usb/core/hub.c (which is the only place
	 * USB_PORT_FEAT_*SPEED is used).
	 */
	return 0;
}

/*
 * These bits are Read Only (RO) and should be saved and written to the
 * registers: 0, 3, 10:13, 30
 * connect status, over-current status, port speed, and device removable.
 * connect status and port speed are also sticky - meaning they're in
 * the AUX well and they aren't changed by a hot, warm, or cold reset.
 */
#define	XHCI_PORT_RO	((1<<0) | (1<<3) | (0xf<<10) | (1<<30))
/*
 * These bits are RW; writing a 0 clears the bit, writing a 1 sets the bit:
 * bits 5:8, 9, 14:15, 25:27
 * link state, port power, port indicator state, "wake on" enable state
 */
#define XHCI_PORT_RWS	((0xf<<5) | (1<<9) | (0x3<<14) | (0x7<<25))
/*
 * These bits are RW; writing a 1 sets the bit, writing a 0 has no effect:
 * bit 4 (port reset)
 */
#define	XHCI_PORT_RW1S	((1<<4))
/*
 * These bits are RW; writing a 1 clears the bit, writing a 0 has no effect:
 * bits 1, 17, 18, 19, 20, 21, 22, 23
 * port enable/disable, and
 * change bits: connect, PED, warm port reset changed (reserved zero for USB 2.0 ports),
 * over-current, reset, link state, and L1 change
 */
#define XHCI_PORT_RW1CS	((1<<1) | (0x7f<<17))
/*
 * Bit 16 is RW, and writing a '1' to it causes the link state control to be
 * latched in
 */
#define	XHCI_PORT_RW	((1<<16))
/*
 * These bits are Reserved Zero (RsvdZ) and zero should be written to them:
 * bits 2, 24, 28:31
 */
#define	XHCI_PORT_RZ	((1<<2) | (1<<24) | (0xf<<28))

/*
 * Given a port state, this function returns a value that would result in the
 * port being in the same state, if the value was written to the port status
 * control register.
 * Save Read Only (RO) bits and save read/write bits where
 * writing a 0 clears the bit and writing a 1 sets the bit (RWS).
 * For all other types (RW1S, RW1CS, RW, and RZ), writing a '0' has no effect.
 */
static u32 xhci_port_state_to_neutral(u32 state)
{
	/* Save read-only status and port state */
	return (state & XHCI_PORT_RO) | (state & XHCI_PORT_RWS);
}

int xhci_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue,
		u16 wIndex, char *buf, u16 wLength)
{
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int ports;
	unsigned long flags;
	u32 temp, status;
	int retval = 0;
	u32 __iomem *addr;
	char *port_change_bit;
	u32 old_port_status = 0;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	spin_lock_irqsave(&xhci->lock, flags);
	switch (typeReq) {
	case GetHubStatus:
		/* No power source, over-current reported per port */
		memset(buf, 0, 4);
		break;
	case GetHubDescriptor:
//printk("GetHubDescriptor: %s %d\n",__FUNCTION__,__LINE__);			
		xhci_hub_descriptor(xhci, (struct usb_hub_descriptor *) buf);
		break;
	case GetPortStatus:
//printk("GetPortStatus: %s %d\n",__FUNCTION__,__LINE__);					
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		status = 0;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(wIndex & 0xff);
        temp = xhci_readl(xhci, addr);
		xhci_dbg(xhci, "get port status, actual port %d status  = 0x%x\n", wIndex, temp);

		/* wPortChange bits */
		if (temp & PORT_CSC)
			status |= 1 << USB_PORT_FEAT_C_CONNECTION;
		if (temp & PORT_PEC)
			status |= 1 << USB_PORT_FEAT_C_ENABLE;
		if ((temp & PORT_OCC))
			status |= 1 << USB_PORT_FEAT_C_OVER_CURRENT;
		/*
		 * FIXME ignoring suspend, reset, and USB 2.1/3.0 specific
		 * changes
		 */
		if (temp & PORT_CONNECT) {
			status |= 1 << USB_PORT_FEAT_CONNECTION;
			status |= xhci_port_speed(temp);
		}
		if (temp & PORT_PE)
			status |= 1 << USB_PORT_FEAT_ENABLE;
		if (temp & PORT_OC)
			status |= 1 << USB_PORT_FEAT_OVER_CURRENT;
		if (temp & PORT_RESET)
			status |= 1 << USB_PORT_FEAT_RESET;
		if (temp & PORT_POWER)
			status |= 1 << USB_PORT_FEAT_POWER;
		xhci_dbg(xhci, "Get port status returned 0x%x\n", status);
		put_unaligned(cpu_to_le32(status), (__le32 *) buf);
		break;
	case SetPortFeature:
		wIndex &= 0xff;
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		addr = &xhci->op_regs->port_status_base + NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		switch (wValue) {
		case USB_PORT_FEAT_POWER:
			/*
			 * Turn on ports, even if there isn't per-port switching.
			 * HC will report connect events even before this is set.
			 * However, khubd will ignore the roothub events until
			 * the roothub is registered.
			 */
			xhci_writel(xhci, temp | PORT_POWER, addr);

			temp = xhci_readl(xhci, addr);
			xhci_dbg(xhci, "set port power, actual port %d status  = 0x%x\n", wIndex, temp);

#if 1 // REALTEK patch
			printk("#@# %s(%d) wIndex 0x%x 0x%x\n", __func__, __LINE__, wIndex, xhci_readl(xhci, addr));
			// if port connect, set warm reset
			if(temp & PORT_CONNECT) {
				if(!wIndex) { // highspeed device
					temp |= PORT_RESET;
					temp &= ~PORT_PE;
					printk("#### REALTEK PATCH ####  %s(%d) if port %d connected, set port reset, write 0x%x\n", __func__, __LINE__, wIndex,temp);
					xhci_writel(xhci, temp, addr);
					mdelay(500);					
					temp = xhci_readl(xhci, addr);
				} else { // superspeed device
					temp |= PORT_RESET;
					temp &= ~PORT_PE;
					printk("#### REALTEK PATCH #### Super Speed device,  %s(%d) if port %d connected, set port reset, write 0x%x\n", __func__, __LINE__, wIndex,temp);
					xhci_writel(xhci, temp, addr);
					mdelay(500);
					temp = xhci_readl(xhci, addr);
				}
			}

			printk("#@# %s(%d) wIndex 0x%x 0x%x\n", __func__, __LINE__, wIndex, xhci_readl(xhci, addr));

			// hack for usb3 portsc, clear all change bit
			printk("#### REALTEK PATCH ####  %s(%d) hack for usb3 portsc\n", __func__, __LINE__);
			temp  = xhci_readl(xhci, addr);
			xhci_writel(xhci, temp & ~PORT_PE, addr);
			mdelay(500);
			temp = xhci_readl(xhci, addr);

			printk("#@# %s(%d) wIndex 0x%x 0x%x\n", __func__, __LINE__, wIndex,  xhci_readl(xhci, addr));
			temp = xhci_readl(xhci, addr);
			// warm reset for usb3 polling state
			#define PORT_PLS_MASK   (0xf << 5)
			if (wIndex && (temp & PORT_PLS_MASK) == (0x7 << 5)) {
				printk("#@# %s(%d) set port reset\n", __func__, __LINE__);
				temp |= PORT_RESET;
				temp &= ~PORT_PE;
				xhci_writel(xhci, temp, addr);
				temp = xhci_readl(xhci,addr);
			}
			printk("#@# %s(%d) wIndex 0x%x 0x%x\n", __func__, __LINE__, wIndex, xhci_readl(xhci, addr));

			// add by cfyeh ---
#endif

#ifdef U3_COMPLIANCE_PATCH
	#define REG_LTSSM_INT_ENABLE	(0xb8140028)
	#define COMPLIANCE_STS  (1<<11)
 
			printk("Enable xhci compliance interrupt!\n");
			REG32(REG_LTSSM_INT_ENABLE) = REG32(REG_LTSSM_INT_ENABLE) | COMPLIANCE_STS;
#endif
			break;
		case USB_PORT_FEAT_RESET:
//printk("SetPortFeature:%s %d\n",__FUNCTION__,__LINE__);			
			temp = (temp | PORT_RESET);
			xhci_writel(xhci, temp, addr);

			temp = xhci_readl(xhci, addr);
			xhci_dbg(xhci, "set port reset, actual port %d status  = 0x%x\n", wIndex, temp);
			break;
		default:
			goto error;
		}
		temp = xhci_readl(xhci, addr); /* unblock any posted writes */
		break;
	case ClearPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*(wIndex & 0xff);
		temp = xhci_readl(xhci, addr);
		temp = xhci_port_state_to_neutral(temp);
		switch (wValue) {
		case USB_PORT_FEAT_C_RESET:
			status = PORT_RC;
			port_change_bit = "reset";
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			status = PORT_CSC;
			port_change_bit = "connect";
			break;
		case USB_PORT_FEAT_C_OVER_CURRENT:
			status = PORT_OCC;
			port_change_bit = "over-current";
			break;
		default:
			goto error;
		}
		/* Change bits are all write 1 to clear */
		old_port_status = xhci_readl(xhci, addr);
		xhci_writel(xhci, old_port_status & ~PORT_PE, addr);
		xhci_dbg(xhci, "clear port %s change, actual port %d status  = 0x%x\n",
			port_change_bit, wIndex, port_status);
/*
		xhci_writel(xhci, temp | status, addr);
		temp = xhci_readl(xhci, addr);
		xhci_dbg(xhci, "clear port %s change, actual port %d status  = 0x%x\n",
				port_change_bit, wIndex, temp);
*/
		temp = xhci_readl(xhci, addr); /* unblock any posted writes */
		break;
	default:
error:
        /* "stall" on error */
		retval = -EPIPE;
	}
	spin_unlock_irqrestore(&xhci->lock, flags);
	return retval;
}

/*
 * Returns 0 if the status hasn't changed, or the number of bytes in buf.
 * Ports are 0-indexed from the HCD point of view,
 * and 1-indexed from the USB core pointer of view.
 * xHCI instances can have up to 127 ports, so FIXME if you see more than 15.
 *
 * Note that the status change bits will be cleared as soon as a port status
 * change event is generated, so we use the saved status from that event.
 */
int xhci_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	unsigned long flags;
	u32 temp, status;
	int i, retval;
	struct xhci_hcd	*xhci = hcd_to_xhci(hcd);
	int ports;
	u32 __iomem *addr;

	ports = HCS_MAX_PORTS(xhci->hcs_params1);

	/* Initial status is no changes */
	buf[0] = 0;
	status = 0;
	if (ports > 7) {
		buf[1] = 0;
		retval = 2;
	} else {
		retval = 1;
	}

	spin_lock_irqsave(&xhci->lock, flags);
	/* For each port, did anything change?  If so, set that bit in buf. */
	for (i = 0; i < ports; i++) {
		addr = &xhci->op_regs->port_status_base +
			NUM_PORT_REGS*i;
		temp = xhci_readl(xhci, addr);
		if (temp & (PORT_CSC | PORT_PEC | PORT_OCC)) {
			if (i < 7)
				buf[0] |= 1 << (i + 1);
			else
				buf[1] |= 1 << (i - 7);
			status = 1;
		}
	}
	spin_unlock_irqrestore(&xhci->lock, flags);
	return status ? retval : 0;
}
