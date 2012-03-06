/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2011 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
 */

/*
 * Darwin support by Julian Einwag <julian@einwag.de>.
 * This is still an early version.
 */

#include "dvdisaster.h"

#include "scsi-layer.h"
#include "udf.h"

#ifdef SYS_DARWIN
#include <IOKit/storage/IOStorageDeviceCharacteristics.h>
#include <IOKit/IOBSD.h>

#include <stdlib.h>

/*
 * Unmount media before trying to access them.
 * Added by Bernd Heller, <bdheller@users.sourceforge.net>
 */

static void unmountDVD(io_object_t scsiDevice)
{
   CFStringRef bsdName = NULL;
	
   bsdName = (CFStringRef) IORegistryEntrySearchCFProperty(scsiDevice,
							   kIOServicePlane,
							   CFSTR(kIOBSDNameKey),
							   kCFAllocatorDefault,
							   kIORegistryIterateRecursively);
   if (bsdName != NULL) {
      // unmount all partitions
      char cmd[4096];
      char bsdNameStr[100];
      CFStringGetCString(bsdName, bsdNameStr, sizeof(bsdNameStr), kCFStringEncodingUTF8);
		
      sprintf(cmd, "/usr/sbin/diskutil unmountDisk /dev/%s", bsdNameStr);
      system(cmd);
		
      CFRelease(bsdName);
   } else {
      // no media to unmount
      return;
   }
}

char *getProductName(io_object_t device)
{
  CFDictionaryRef devCharacteristics;
  CFStringRef nameRef;
  CFIndex length;
  char *prodName;

  devCharacteristics = IORegistryEntryCreateCFProperty(device, CFSTR(kIOPropertyDeviceCharacteristicsKey), kCFAllocatorDefault, 0);
  
  if (CFDictionaryGetValueIfPresent(devCharacteristics, CFSTR("Product Name"), (const void **) &nameRef)) {
    length = CFStringGetLength(nameRef)+1;
    prodName = malloc(length);
    CFStringGetCString(nameRef, prodName, length, 0);
    CFRelease(nameRef);
    return prodName;
  } else  
    return  NULL;
}

io_iterator_t getDVDIterator(char* ioClass) 
{
	io_iterator_t scsiObjectIterator = (io_iterator_t) NULL;
	CFMutableDictionaryRef matchingDict = NULL;
	IOReturn ioReturnValue = kIOReturnSuccess;
	
	matchingDict = IOServiceMatching(ioClass);
	if (matchingDict == (CFMutableDictionaryRef) NULL) {
		return (io_iterator_t) NULL;
	}	
	ioReturnValue = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &scsiObjectIterator);
	if (scsiObjectIterator == (io_iterator_t) NULL || (ioReturnValue != kIOReturnSuccess)) {
		return (io_iterator_t) NULL;
	}
	return scsiObjectIterator;
	
}

IOCFPlugInInterface **getPlugInInterface(io_object_t scsiDevice)
{
	IOReturn ioReturnValue;
	SInt32 score = 0;
	IOCFPlugInInterface **plugInInterface = NULL;
	
	ioReturnValue = IOCreatePlugInInterfaceForService(scsiDevice, kIOMMCDeviceUserClientTypeID, kIOCFPlugInInterfaceID,&plugInInterface, &score);
	if (ioReturnValue != kIOReturnSuccess) {
		return NULL;
	}
	return plugInInterface;
}

MMCDeviceInterface** getMMCInterface(IOCFPlugInInterface** plugInInterface) 
{
	MMCDeviceInterface **mmcDeviceInterface = NULL;
	HRESULT pluginResult;      
	pluginResult = (*plugInInterface)->QueryInterface(plugInInterface,
														  CFUUIDGetUUIDBytes(kIOMMCDeviceInterfaceID),
														  (LPVOID)&mmcDeviceInterface);
	if (pluginResult != KERN_SUCCESS) {
	  return NULL;
	}
	return mmcDeviceInterface;
}

char *DefaultDevice()
{
  int i;
  io_iterator_t scsiObjectIterator = (io_iterator_t) NULL;
  io_object_t scsiDevice;
  char *deviceName, *prodName;

  scsiObjectIterator = getDVDIterator("IODVDServices");
  for (i = 1; (scsiDevice = (io_object_t) IOIteratorNext(scsiObjectIterator)) != (io_object_t) NULL; i++)  {
    deviceName = g_malloc0(80);
    sprintf(deviceName,"IODVDServices/%d",i);

    prodName = getProductName(scsiDevice);
    g_ptr_array_add(Closure->deviceNodes, g_strdup(deviceName));
    g_ptr_array_add(Closure->deviceNames, g_strdup(prodName));
    g_free(deviceName);
  }
  IOObjectRelease(scsiObjectIterator);
  scsiObjectIterator = getDVDIterator("IOCompactDiscServices");
  for (i = 1; (scsiDevice = IOIteratorNext(scsiObjectIterator)) != (io_object_t) NULL; i++)  {
    deviceName = g_malloc0(80);
    sprintf(deviceName,"IOCompactDiscServices/%d",i);
    g_ptr_array_add(Closure->deviceNodes, g_strdup(deviceName));
    g_ptr_array_add(Closure->deviceNames, g_strdup(deviceName));
    g_free(deviceName);
  }
  if(Closure->deviceNodes->len)
    return g_strdup(g_ptr_array_index(Closure->deviceNodes, 0));
  else { PrintLog(_("No CD/DVD drives found."));
    return NULL;
  }
		    
}

DeviceHandle* OpenDevice(char *device)
{
  DeviceHandle *dh;
  io_iterator_t scsiObjectIterator;
  io_object_t scsiDevice;
  IOReturn ioReturnValue = kIOReturnSuccess;
  int numericalId = 1, i;
  char *realdevice = NULL, *tmp;

  /* use a naming scheme identical to cdrtools */
  realdevice = tmp = strdup(device);
  tmp = strchr(tmp, '/');
  if (tmp != NULL) {
    *tmp++ = '\0';
    numericalId = atoi(tmp);
  }
  dh = g_malloc0(sizeof(DeviceHandle));
  scsiObjectIterator = getDVDIterator(realdevice);
  if (scsiObjectIterator == (io_iterator_t) NULL) {
    g_free(dh);
    Stop("Could not get SCSI-Iterator.");
    return NULL;
  }
  /* look up handle for the selected device */
  for (i = 1; (scsiDevice = IOIteratorNext(scsiObjectIterator)) != (io_object_t) NULL; i++) {
    if (i == numericalId) { 
      break;
    }
  }

  unmountDVD(scsiDevice);

  dh->plugInInterface = getPlugInInterface(scsiDevice);
  if (dh->plugInInterface == NULL) {
    g_free(dh);
    Stop("Could not get PlugInInterface.");
    return NULL;
  }
  dh->mmcDeviceInterface = getMMCInterface(dh->plugInInterface);
  if (dh->mmcDeviceInterface == NULL) {
    g_free(dh);
    Stop("Could not get MMCDeviceInterface.");
    return NULL;    
  }
  (dh->scsiTaskDeviceInterface)  = (*dh->mmcDeviceInterface)->GetSCSITaskDeviceInterface(dh->mmcDeviceInterface);
  if (dh->scsiTaskDeviceInterface == NULL) {
    g_free(dh);
    Stop("Could not get SCSITaskDeviceInterface.");
    return NULL;
  }
  ioReturnValue = (*dh->scsiTaskDeviceInterface)->ObtainExclusiveAccess(dh->scsiTaskDeviceInterface);
  if (ioReturnValue != kIOReturnSuccess) { 
    Stop("Couldn't obtain exclusive access to drive.");
    return NULL;
  }
  dh->taskInterface = (*dh->scsiTaskDeviceInterface)->CreateSCSITask(dh->scsiTaskDeviceInterface);
  if (dh->taskInterface == NULL) {
    Stop("Could not create taskInterface.");
    return NULL;
  }
  (*dh->taskInterface)->SetTimeoutDuration(dh->taskInterface, 120*1000);
  dh->device = g_strdup(device);
  return dh;
}

void CloseDevice(DeviceHandle *dh)
{
  if(dh->canReadDefective)
     SetRawMode(dh, MODE_PAGE_UNSET);
  
  if(dh->rawBuffer)
     FreeRawBuffer(dh->rawBuffer);

  if (dh->taskInterface) {
    (*dh->taskInterface)->Release(dh->taskInterface);
  }
  if (dh->scsiTaskDeviceInterface) {
    (*dh->scsiTaskDeviceInterface)->ReleaseExclusiveAccess(dh->scsiTaskDeviceInterface);    
    (*dh->scsiTaskDeviceInterface)->Release(dh->scsiTaskDeviceInterface);
  }
  if (dh->mmcDeviceInterface) {
    (*dh->mmcDeviceInterface)->Release(dh->mmcDeviceInterface);
  }
  if(dh->plugInInterface) {
    IODestroyPlugInInterface(dh->plugInInterface);
  }
  if(dh->range) {
    g_free(dh->range);
  }

  if(dh->rs02Header)
    g_free(dh->rs02Header);
  if(dh->typeDescr) 
    g_free(dh->typeDescr);
  if(dh->mediumDescr) 
    g_free(dh->mediumDescr);
  if(dh->isoInfo)
    FreeIsoInfo(dh->isoInfo);
  if(dh->defects)
    FreeBitmap(dh->defects);

  g_free(dh);
 
}

int SendPacket(DeviceHandle *dh, unsigned char *cmd, int cdb_size, unsigned char *buf, int size, Sense *sense, int data_mode)
{
  u_int32_t flags = 0;
  SCSITaskInterface **taskInterface;
  SCSITaskStatus taskStatus = kSCSITaskStatus_No_Status;
  IOReturn ioReturnValue;
 
  switch(data_mode) {
  case DATA_READ:
    flags = kSCSIDataTransfer_FromTargetToInitiator;
    break;
  case DATA_WRITE:
    flags = kSCSIDataTransfer_FromInitiatorToTarget;
    break;
  case DATA_NONE:
    flags = kSCSIDataTransfer_NoDataTransfer;
    break;
  default:
    Stop("illegal data_mode: %d",data_mode);
  }
 
  taskInterface = dh->taskInterface;
  if (flags == kSCSIDataTransfer_FromTargetToInitiator || flags == kSCSIDataTransfer_FromInitiatorToTarget) {
    dh->range = (IOVirtualRange *) g_malloc(sizeof(IOVirtualRange));
    dh->range->address = (IOVirtualAddress) buf;
    dh->range->length = size;
  }
  ioReturnValue = (*taskInterface)->SetCommandDescriptorBlock(taskInterface, cmd, cdb_size);
  if (ioReturnValue != kIOReturnSuccess) {
    Stop("Couldn't set command descriptor block.");
  }
  if (flags == kSCSIDataTransfer_FromTargetToInitiator || flags == kSCSIDataTransfer_FromInitiatorToTarget) {
    ioReturnValue = (*taskInterface)->SetScatterGatherEntries(taskInterface, dh->range, 1, size, flags);
    if (ioReturnValue != kIOReturnSuccess) {
      Stop("Couldn't set scatter-gather.");     
    }
  }
  ioReturnValue = (*taskInterface)->ExecuteTaskSync(taskInterface, (SCSI_Sense_Data*) sense, &taskStatus, NULL);
  if (ioReturnValue != kIOReturnSuccess) {
    return -1;
  }

  if (taskStatus != kSCSITaskStatus_GOOD) {
    return -1;
  }

  return 0;
  
}

#endif
