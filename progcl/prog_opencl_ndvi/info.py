import pyopencl as cl

def PrintDeviceInfo(device):
    print "Name: %s" % device.name
    print "OpenCL version: %s" % device.version
    print "Global memory size: %.2f Megabytes" % (device.global_mem_size/1024/1024)
    print "Local memory size: %.2f Kilobytes" % (device.local_mem_size/1024)
    print "Max constant buffer size: %.2f Kilobytes" % (device.max_constant_buffer_size/1024)
    print "Max clock frequency: %i Hz" % device.max_clock_frequency
    print "Max compute units: %i" % device.max_compute_units
    print "Max work group size: %i" % device.max_work_group_size
    print "Max work item sizes: %s" % device.max_work_item_sizes

def PrintOpenCLInfo():
    print "\033[92m"
    for platform in cl.get_platforms():
        print "Platform: %s" % platform.name
        for device in platform.get_devices():
            PrintDeviceInfo(device)
        print ''
    print "\033[0m"

PrintOpenCLInfo()


#----------------------------------------------------
# Radeon HD MSI R6870 Hawk
#----------------------------------------------------
#Platform: AMD Accelerated Parallel Processing
#Name: Barts
#OpenCL version: OpenCL 1.1 AMD-APP-SDK-v2.5 (684.213)
#Global memory size: 512.00 Megabytes
#Local memory size: 32.00 Kilobytes
#Max constant buffer size: 64.00 Kilobytes
#Max clock frequency: 930 Hz
#Max compute units: 14
#Max work group size: 256
#Max work item sizes: [256, 256, 256]
#Name: Intel(R) Core(TM) i3 CPU         540  @ 3.07GHz
#OpenCL version: OpenCL 1.1 AMD-APP-SDK-v2.5 (684.213)
#Global memory size: 11968.00 Megabytes
#Local memory size: 32.00 Kilobytes
#Max constant buffer size: 64.00 Kilobytes
#Max clock frequency: 3075 Hz
#Max compute units: 4
#Max work group size: 1024
#Max work item sizes: [1024, 1024, 1024]

