# Linux Device Tree & Platform Driver
## A Comprehensive Beginner's Guide

## Table of Contents

**Fundamentals**
- Part 1: Understanding Platform Devices & Drivers
- Part 2: Evolution - From Hard-Coded to Device Tree
- Part 3: Complete Platform Device & Driver Example
- Part 4: of_device_id - The Matching Table
- Part 5: Significance of Each Component

**API & Code Examples**
- Part 6: Two Property Access Approaches
- Part 7: Side-by-Side Comparison
- Part 8: Real Working Example
- Part 9: Most Commonly Used APIs Reference

**Hardware Setup**
- Part 10: Raspberry Pi 3 A+ Setup
- Part 11: BeagleBone Black Setup

**Testing & Troubleshooting**
- Part 12: Testing & Verification Checklist
- Part 13: Common Issues & Troubleshooting
- Part 14: Quick Reference Templates

**Quick Reference**
- Part 15: Key Takeaways
- Part 16: Accessing Resources
- Part 17: Complete Working Driver
- Part 18: Struct Definitions & API Reference

---

## Part 1: Understanding Platform Devices & Drivers

### Definition: Platform Device

A **platform device** represents a hardware component—such as a temperature sensor, timer, or GPIO controller—that is integrated into the system without a dedicated hardware bus interface (like PCI or USB). From the kernel's perspective, a platform device requires specific information to function correctly:

- **Hardware existence**: What devices are present in the system
- **Memory location**: The physical address where the device's registers reside
- **Interrupt configuration**: Which interrupt signals the device uses
- **Additional properties**: Device-specific parameters and capabilities

### Definition: Platform Driver

A **platform driver** is the corresponding kernel software component that manages platform devices. Its primary responsibilities include:

- **Device claim**: Assert control over matched platform devices
- **Initialization**: Configure and initialize hardware when a matching device is discovered
- **Resource management**: Handle memory mapping, interrupt setup, and device-specific initialization
- **Cleanup**: Release all resources and reset hardware when the device is removed
- **User interface**: Provide mechanisms (sysfs, character devices, etc.) for user-space applications to interact with the hardware

### The Device Recognition Process

The Linux kernel employs a structured discovery mechanism to match platform devices with their corresponding drivers:

```
System Boot
    ↓
Device Tree Parsed
    ↓
Platform Devices Registered
    ↓
Kernel Searches Driver Match Tables
    ↓
Compatible String Comparison (of_device_id)
    ↓
Match Found: probe() Invoked
    ↓
Driver Initializes Hardware
```

**Critical Design Principle**: The `compatible` property string specified in the device tree node must precisely match an entry in the driver's `of_device_id` table. Case sensitivity and exact string matching are essential for successful device discovery.

### Device Tree vs Platform Driver: Conceptual Relationship

The device tree and platform driver serve complementary but distinct purposes in the embedded Linux ecosystem:

| Aspect | Device Tree | Platform Driver |
|--------|------------|-----------------|
| **Purpose** | Describes what hardware exists | Specifies how to operate the hardware |
| **File format** | `.dts` (text) → `.dtb` or `.dtbo` (binary) | `.c` (C source) → `.ko` (kernel module) |
| **Storage location** | `/boot/` directory or compiled into kernel | Kernel memory (dynamic or built-in) |
| **Loaded when** | Boot time (before driver loading) | After boot or on demand by kernel |
| **Example content** | "Temperature sensor exists at 0x50000000" | Code to read temperature from 0x50000000 |
| **Modification** | Device tree overlays at runtime | Requires module reload |

---

## Part 2: Evolution — The Transition from Hard-Coded Device Registration to Device Tree

### The Legacy Approach: Hardware Registration in Source Code

Prior to the widespread adoption of device tree technology, embedded Linux developers were required to manually register platform devices within kernel source code. This approach bound hardware descriptions to compiled binaries, creating inflexibility and requiring kernel recompilation for hardware configuration changes.

**Example: Hard-Coded Device Registration (sensor_pltdvc.c)**
```c
/* Hardware definition hard-coded in .c file */
static struct resource sensor_resources[] = {
    {
        .name = "sensor_mem",
        .start = 0x50000000,      /* ← Address hard-coded */
        .end = 0x50000FFF,        /* ← Size hard-coded */
        .flags = IORESOURCE_MEM,
    },
};

static struct platform_device sensor_pltdvc = {
    .name = "sensor_driver",
    .resource = sensor_resources,
    .num_resources = ARRAY_SIZE(sensor_resources),
};

/* In module_init() */
platform_device_register(&sensor_pltdvc);  /* Manually register */
```

**Limitations of hard-coded device registration:**
- Hardware parameters are embedded in compiled kernel code
- Modifying device configuration requires kernel recompilation
- Different board variants necessitate separate kernel builds
- Dynamic device addition or removal is not supported without kernel changes
- Scalability challenges for systems with multiple hardware configurations

### The Modern Approach: Declarative Hardware Description via Device Tree

Contemporary embedded Linux systems employ device tree technology to separate hardware descriptions from driver logic. Hardware configuration is declared in device tree files (.dts) and compiled into binary overlays (.dtbo), enabling runtime device loading without kernel recompilation.

**Example: Device Tree Hardware Declaration (temp_sens.dts)**
```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,beaglebone", "ti,beaglebone-black";
    
    fragment@0 {
        target = <&{/}>;
        __overlay__ {
            sensor@0 {
                compatible = "temp-sensor,anis";  /* ← Tell kernel which driver */
                reg = <0x50000000 0x1000>;        /* ← Address in DT, not C code */
                label = "temp-sensor-00A1";
                status = "okay";
            };
        };
    };
};
```

**Advantages of device tree approach:**
- Hardware configuration is separated from driver code
- Configuration changes require only device tree recompilation (no kernel rebuild)
- Device tree overlays enable runtime device loading
- Single driver binary functions with multiple hardware configurations
- Enhanced flexibility for board variants and feature extensions

### Device Discovery with Device Tree

```
Device Tree loaded at boot
         ↓
Kernel parses: sensor@0 { compatible = "temp-sensor,anis" }
         ↓
Kernel automatically registers platform_device
         ↓
Kernel searches driver's of_device_id table:
    { .compatible = "temp-sensor,anis" }  ← MATCH FOUND!
         ↓
probe() function called automatically
```

### Side-by-Side Comparison: Historical vs Contemporary Approaches

The following table illustrates the fundamental differences between hard-coded device registration and device tree declaration:

| Aspect | Legacy: Hard-Coded (C) | Modern: Device Tree |
|--------|---------------------------|------------------|
| **Device definition location** | Driver source code (e.g., sensor_pltdvc.c) | Device tree source file (e.g., temp_sens.dts) |
| **Device registration mechanism** | Explicit `platform_device_register()` in module initialization | Kernel automatic registration from parsed device tree |
| **Hardware address specification** | In `struct resource` array in C code | In `reg` property in device tree |
| **Driver discovery mechanism** | Device name matching via `.name` field | Compatible string matching via `compatible` property |
| **Configuration modification** | Requires editing .c file, kernel recompilation, and reload | Device tree recompilation and overlay loading at runtime |
| **Multiple hardware configurations** | Separate kernel builds required | Single kernel binary with multiple overlays |
| **Operational flexibility** | Minimal (recompilation required) | High (runtime modifications) |

### Architectural Consistency: Driver Code Remains Fundamentally Unchanged

A critical observation: the driver implementation itself did not undergo fundamental transformation between these approaches. The probe and remove functions maintain their core structure and purpose. What changed was the **source of device information**:

- **Legacy**: Resources obtained from hard-coded structures  
- **Modern**: Resources automatically extracted from device tree by kernel

Both approaches utilize the identical API for resource acquisition:

```c
struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
```

The distinction lies in resource provenance: legacy systems require explicit registration, while device tree systems enable automatic kernel extraction and resource provisioning.

---

## Part 3: Complete Platform Device & Driver Example — A Practical Implementation

This section presents a comprehensive, step-by-step example demonstrating the complete lifecycle of platform device and driver integration using contemporary device tree methodology.

### Step 1: Define the Device Tree Node

**File: sensor-overlay.dts** (Device Tree Specification)
```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,beaglebone", "ti,beaglebone-black";

    fragment@0 {
        target = <&{/}>;
        __overlay__ {
            /* This describes a sensor device */
            sensor@0 {
                compatible = "temp-sensor,anis";    /* MUST match driver */
                reg = <0x50000000 0x1000>;          /* Physical address, size */
                label = "temp-sensor-00A1";         /* Human readable name */
                status = "okay";                    /* Enable this device */
            };
        };
    };
};
```

**Property Specification Reference:**
- `compatible`: Kernel uses this property to match the device with a corresponding driver. This comparison is case-sensitive and must match exactly.
- `reg`: Specifies the physical memory address (0x50000000) and size (0x1000 bytes) for device I/O operations.
- `label`: Optional human-readable identifier for diagnostic and debugging purposes.
- `status`: Indicates device operational state; "okay" enables the device, while "disabled" causes the kernel to ignore it.

### Step 2: Implement the Platform Driver

**File: sensor_driver.c** (Platform Driver Implementation)
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>

MODULE_LICENSE("GPL");

/* Step 1: Define driver private data structure */
struct sensor_dev {
    void __iomem *base_addr;        /* Kernel virtual address */
    unsigned int control_value;     /* Last written value */
};

/* Step 2: Write probe function (called when device found) */
static int sensor_probe(struct platform_device *pdev)
{
    struct sensor_dev *pdata;
    struct resource *res;

    dev_info(&pdev->dev, "Probe: Device found!\n");

    /* Allocate driver private data */
    pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    /* Get memory resource from device tree (reg property) */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
        return -ENOENT;

    /* Map physical memory to kernel virtual space */
    pdata->base_addr = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!pdata->base_addr)
        return -ENOMEM;

    /* Store private data for later use in remove/sysfs */
    platform_set_drvdata(pdev, pdata);

    dev_info(&pdev->dev, "Probe: Initialization complete\n");
    return 0;
}

/* Step 3: Write remove function (called when device removed) */
static int sensor_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Remove: Cleaning up\n");
    /* devm_* functions auto-cleanup, so nothing needed */
    return 0;
}

/* Step 4: Create of_device_id match table (CRITICAL!) */
static const struct of_device_id sensor_match_table[] = {
    { .compatible = "temp-sensor,anis" },  /* Must match DT */
    { }                                      /* MUST end with empty entry */
};
MODULE_DEVICE_TABLE(of, sensor_match_table);

/* Step 5: Create platform driver structure */
static struct platform_driver sensor_driver = {
    .driver = {
        .name = "temp_sensor_driver",
        .of_match_table = sensor_match_table,  /* Tell kernel: match by DT */
    },
    .probe = sensor_probe,
    .remove = sensor_remove,
};

/* Step 6: Register driver (macro handles init/exit) */
module_platform_driver(sensor_driver);
```

**What happens when this loads:**
1. `module_platform_driver` calls `platform_driver_register()`
2. Kernel looks for devices matching `of_device_id` table
3. For each match, calls `sensor_probe()`
4. Probe maps memory and initializes hardware
5. When removed, calls `sensor_remove()`

---

## Part 4: of_device_id — The Device Discovery Matching Table

### Definition: of_device_id Structure

The `of_device_id` structure serves as the primary mechanism by which the Linux kernel matches platform devices (defined in device tree) with their corresponding drivers. The kernel iterates through this table comparing `compatible` strings until a match is found.

```c
static const struct of_device_id sensor_match_table[] = {
    { .compatible = "temp-sensor,anis" },
    { .compatible = "another,sensor" },
    { }  /* Mandatory terminator entry */
};
```

### Device Matching Algorithm

The kernel employs the following matching precedence:

1. **Iterate through device tree**: Each device tree node is examined sequentially
2. **Compare compatible strings**: The device's `compatible` property is compared against all entries in the driver's `of_device_id` table
3. **Exact match verification**: If any entry matches (case-sensitive, exact string comparison), device discovery succeeds
4. **Probe invocation**: Upon successful match, the driver's probe function is invoked automatically
5. **No match handling**: If no match is found, the device remains unclaimed and the driver is bypassed for that device

### Compatible String Naming Convention

The `compatible` property employs a standardized naming format to uniquely identify devices:

```
compatible = "vendor-prefix,device-name";
              ^^^^^^^^^^^^^^  ^^^^^^^^^^^
              Vendor ID       Device identifier
```

**Convention Examples:**
- `"ti,am33xx"` — Texas Instruments, AM33xx system-on-chip
- `"temp-sensor,anis"` — Custom vendor "temp-sensor", device "anis"
- `"raspberrypi,bcm2836"` — Raspberry Pi Foundation, BCM2836 processor

### Practical Example: Complete Matching Scenario
```dts
sensor@0 {
    compatible = "temp-sensor,anis";
    reg = <0x50000000 0x1000>;
};
```

**Driver claims it:**
```c
static const struct of_device_id sensor_match_table[] = {
    { .compatible = "temp-sensor,anis" },  /* ✓ MATCH! */
    { }
};
```

**Result:** Kernel automatically invokes `sensor_probe()` when device tree is loaded

---

## Part 5: Significance of Each Component — Architectural Elements

This section details the purpose and significance of each element within the device tree and driver architecture.

### 1. Device Tree Overlay Declaration (/plugin/)

**Purpose and Significance:**
Device tree overlays provide a mechanism for dynamically extending the system device tree at runtime. The `/plugin/` pragma enables overlay mode compilation, allowing additional hardware descriptions to be merged into the main device tree without static compilation.

**Key advantages:**
- Devices can be added without modifying the base device tree
- Runtime loading enables hardware discovery and hotplug-like support
- Facilitates BeagleBone cape expansion and similar plugin architectures
- Bootloader applies overlays at runtime, providing configuration flexibility

```dts
/plugin/;  /* Enable device tree overlay mode */
fragment@0 {
    target = <&{/}>;  /* Root node target for attachment */
    __overlay__ {
        /* New device nodes defined here */
    };
};
```

### 2. Device Tree Fragment Organization

**Purpose and Significance:**
Fragments provide organizational structure for device tree modifications. Multiple fragments within a single compiled overlay (.dtbo) file can target different portions of the device tree, enabling coherent device additions.

**Technical details:**
- `fragment@0`: First overlay fragment (sequential numbering)
- `target = <&{/}>`: Specifies the target node for attachment (root node `/` in this example)
- Multiple fragments can coexist: `fragment@0`, `fragment@1`, etc.
- `__overlay__`: Marker for overlay content to be merged

### 3. The compatible Property — Critical for Device Discovery

**Purpose and Significance:**
The `compatible` property is the fundamental mechanism enabling kernel device-driver matching. This property establishes the binding contract between hardware description and driver implementation.

**Critical characteristics:**
- Case-sensitive string matching (exact comparison required)
- Kernel uses this property as the primary matching criterion
- Missing or mismatched property results in unclaimed devices (kernel log warnings)
- Device remains registered in sysfs but driver probe is never invoked

### 4. The reg Property — Device Resource Location

**Purpose and Significance:**
The `reg` property specifies the physical hardware addressing for memory-mapped devices. This property communicates essential resource information to the driver, enabling correct hardware access.

**Technical specification:**
- Format: `<address size>` in 32-bit cell units (typical configuration)
- Address component: Physical memory location where device registers reside
- Size component: Memory range allocated to the device
- Example: `<0x50000000 0x1000>` specifies 0x50000000 as the starting address, 0x1000 bytes (4KB) as the addressable range
- Driver retrieves this via: `platform_get_resource(pdev, IORESOURCE_MEM, 0)`

### 5. Device-Managed (devm_*) Functions
**Why use them:**
- Automatically clean up on driver removal
- Prevents memory leaks
- Automatically freed when device unbinds
- Examples: `devm_kzalloc()`, `devm_ioremap()`, `devm_request_irq()`

### 6. platform_set_drvdata()
**Why it matters:**
- Stores pointer to driver private data
- Later retrieved with `platform_get_drvdata()`
- Allows probe() and remove() to access same data
- Essential for sysfs attributes and callbacks

---

## Part 6: Accessing Device Tree Properties — Two Approaches

### Approach 1: Platform Helpers — Kernel Pre-Parsed Properties

**Applicability:** Suitable for standard device tree properties such as `reg`, `interrupts`, and `gpios` that the kernel automatically parses and exposes through structured APIs.

**When to use:** For standard properties like `reg` and `interrupts`

```c
/* In probe function */

/* Get memory resource (parsed from "reg" property) */
struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
void __iomem *base = devm_ioremap(&pdev->dev, res->start, resource_size(res));

/* Get IRQ (parsed from "interrupts" property) */
int irq = platform_get_irq(pdev, 0);

/* Get GPIO (parsed from "gpios" property) */
struct gpio_desc *gpio = devm_gpiod_get(&pdev->dev, "sensor", GPIOD_OUT_LOW);
```

**Advantages:**
- Type-safe structure definitions (structured data)
- Kernel pre-processes all properties automatically
- Simple API surface for common use cases
- Error handling built into kernel

**Limitations:**
- Restricted to kernel-recognized standard properties
- Not applicable for driver-specific custom properties

### Approach 2: Device Tree Helpers — Direct Property Access

**Applicability:** Appropriate for driver-specific custom properties or scenarios requiring manual property parsing beyond kernel-standardized properties.

**When to use:** For custom properties or manual parsing

```c
/* In probe function */
struct device_node *np = pdev->dev.of_node;

/* Read custom u32 property */
u32 threshold;
of_property_read_u32(np, "threshold", &threshold);

/* Read custom string property */
const char *label;
of_property_read_string(np, "label", &label);

/* Read boolean property */
bool enabled = of_property_read_bool(np, "enabled");

/* Read u32 array at specific index */
u32 addr, size;
of_property_read_u32_index(np, "my-reg", 0, &addr);   /* First element */
of_property_read_u32_index(np, "my-reg", 1, &size);   /* Second element */

/* Map memory using custom property */
void __iomem *base = devm_ioremap(&pdev->dev, addr, size);
```

**Advantages:**
- Supports any custom property without kernel limitation
- Direct access to raw device tree data
- Flexible for non-standard and vendor-specific properties
- Enables driver-defined property semantics

**Limitations:**
- Manual property parsing required
- Developers must understand property format and structure
- Error handling must be implemented explicitly

---

## Part 7: Side-by-Side Comparison — Both Approaches in Real Code

### Device Tree Definition

```dts
sensor@0 {
    compatible = "temp-sensor,anis";
    
    /* Standard property (Approach 1) */
    reg = <0x50000000 0x1000>;
    
    /* Custom properties (Approach 2) */
    threshold = <75>;
    label = "temperature-sensor";
    my-reg = <0x50000000 0x1000>;  /* Custom format same as reg */
};
```

### Approach 1 Implementation: Platform Helper Functions

This implementation utilizes kernel-provided helper functions for property access:

```c
static int sensor_probe_approach1(struct platform_device *pdev)
{
    struct resource *res;
    void __iomem *base;

    /* Get "reg" property via platform helper */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res)
        return -ENOENT;

    /* Kernel already parsed address and size */
    dev_info(&pdev->dev, "Address: 0x%pa, Size: 0x%x\n",
             &res->start, resource_size(res));

    base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!base)
        return -ENOMEM;

    return 0;
}
```

### Approach 2 Implementation: Device Tree Helper Functions

This implementation employs direct device tree property reading functions:

```c
static int sensor_probe_approach2(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    u32 threshold, addr, size;
    const char *label;
    void __iomem *base;

    /* Get "threshold" (custom u32) */
    if (of_property_read_u32(np, "threshold", &threshold) == 0)
        dev_info(&pdev->dev, "Threshold: %u\n", threshold);

    /* Get "label" (custom string) */
    if (of_property_read_string(np, "label", &label) == 0)
        dev_info(&pdev->dev, "Label: %s\n", label);

    /* Get "my-reg" (custom array) */
    of_property_read_u32_index(np, "my-reg", 0, &addr);
    of_property_read_u32_index(np, "my-reg", 1, &size);

    dev_info(&pdev->dev, "Custom addr: 0x%x, size: 0x%x\n", addr, size);

    base = devm_ioremap(&pdev->dev, addr, size);
    if (!base)
        return -ENOMEM;

    return 0;
}
```

### Comparative Analysis

| Characteristic | Approach 1 (platform_get_*) | Approach 2 (of_property_*) |
|---|---|---|
| **Property scope** | Standard kernel properties (`reg`, `interrupts`, `gpios`) | Any property (custom or standard) |
| **Data parsing** | Kernel handles automatically | Developer handles manually |
| **Type safety** | High (struct resource) | Low (raw data) |
| **Flexibility** | Low (kernel-defined properties only) | High (any custom property) |
| **Built-in error checking** | Yes | Manual required |
| **Common use case** | Memory, interrupts, GPIO | Custom device-specific parameters |

---

## Part 8: Real Working Example from Your Codebase

### Complete Implementation: Temperature Sensor with Integrated Properties

**Device Tree Specification:**
```dts
sensor@0 {
    compatible = "temp-sensor,anis";
    reg = <0x50000000 0x1000>;
    threshold = <75>;
    label = "temperature-sensor";
    status = "okay";
};
```

**Driver Using BOTH Approaches:**
```c
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of.h>

MODULE_LICENSE("GPL");

struct sensor_dev {
    void __iomem *base_addr;
    unsigned int threshold;
    unsigned int control_value;
};

/* Sysfs: read temperature from hardware */
static ssize_t temp_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sensor_dev *pdata = dev_get_drvdata(dev);
    u32 temp = ioread32(pdata->base_addr + 0x00);
    return sprintf(buf, "%u\n", temp);
}

/* Sysfs: write control register */
static ssize_t control_store(struct device *dev, struct device_attribute *attr,
                             const char *buf, size_t count)
{
    struct sensor_dev *pdata = dev_get_drvdata(dev);
    unsigned int val;

    if (kstrtouint(buf, 0, &val) == 0) {
        pdata->control_value = val;
        iowrite32(val, pdata->base_addr + 0x04);
        dev_info(dev, "Control: 0x%x\n", val);
    }
    return count;
}

/* Sysfs: read threshold property */
static ssize_t threshold_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct sensor_dev *pdata = dev_get_drvdata(dev);
    return sprintf(buf, "%u\n", pdata->threshold);
}

static DEVICE_ATTR_RO(temp);
static DEVICE_ATTR_WO(control);
static DEVICE_ATTR_RO(threshold);

static int sensor_probe(struct platform_device *pdev)
{
    struct sensor_dev *pdata;
    struct device_node *np = pdev->dev.of_node;
    struct resource *res;
    u32 thr;

    dev_info(&pdev->dev, "Probe: sensor_probe called\n");

    /* Allocate driver private data */
    pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
        return -ENOMEM;

    /* APPROACH 1: Get memory via platform helper */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        dev_err(&pdev->dev, "No memory resource\n");
        return -ENOENT;
    }

    pdata->base_addr = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!pdata->base_addr)
        return -ENOMEM;

    dev_info(&pdev->dev, "Memory mapped: 0x%pa - 0x%pa\n",
             &res->start, &res->end);

    /* APPROACH 2: Get custom properties via DT helpers */
    if (of_property_read_u32(np, "threshold", &thr) == 0)
        pdata->threshold = thr;
    else
        pdata->threshold = 0;

    pdata->control_value = 0;

    /* Store for later use */
    platform_set_drvdata(pdev, pdata);

    /* Create sysfs interface */
    device_create_file(&pdev->dev, &dev_attr_temp);
    device_create_file(&pdev->dev, &dev_attr_control);
    device_create_file(&pdev->dev, &dev_attr_threshold);

    return 0;
}

static int sensor_remove(struct platform_device *pdev)
{
    device_remove_file(&pdev->dev, &dev_attr_temp);
    device_remove_file(&pdev->dev, &dev_attr_control);
    device_remove_file(&pdev->dev, &dev_attr_threshold);
    return 0;
}

static const struct of_device_id sensor_match[] = {
    { .compatible = "temp-sensor,anis" },
    { }
};
MODULE_DEVICE_TABLE(of, sensor_match);

static struct platform_driver sensor_driver = {
    .driver = {
        .name = "sensor_driver",
        .of_match_table = sensor_match,
    },
    .probe = sensor_probe,
    .remove = sensor_remove,
};

module_platform_driver(sensor_driver);
```

**Testing:**
```bash
# Load driver
sudo insmod sensor_driver.ko

# Check if probe called
dmesg | grep "Probe:"

# Read temperature from sysfs
cat /sys/bus/platform/devices/sensor@0/temp

# Write control value
echo 0x42 > /sys/bus/platform/devices/sensor@0/control

# Check threshold
cat /sys/bus/platform/devices/sensor@0/threshold
```

---

## Part 9: Most Commonly Used APIs — Reference Guide

### Memory I/O Operations
```c
/* Read from memory */
u8 val8 = ioread8(base + offset);
u16 val16 = ioread16(base + offset);
u32 val32 = ioread32(base + offset);

/* Write to memory */
iowrite8(0x42, base + offset);
iowrite16(0x4242, base + offset);
iowrite32(0x42424242, base + offset);
```

### Device Tree Property Reading
```c
struct device_node *np = pdev->dev.of_node;

/* Read single u32 */
u32 val;
of_property_read_u32(np, "property-name", &val);

/* Read u32 from array */
u32 val;
of_property_read_u32_index(np, "property-array", 0, &val);

/* Read string */
const char *str;
of_property_read_string(np, "property-name", &str);

/* Check if boolean property exists */
bool present = of_property_read_bool(np, "flag-name");
```

### Resource Access via Platform Helpers
```c
/* Get memory resource (from "reg" property) */
struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

/* Get IRQ resource (from "interrupts" property) */
int irq = platform_get_irq(pdev, 0);

/* Set/get driver private data */
platform_set_drvdata(pdev, private_ptr);
void *data = platform_get_drvdata(pdev);
```

### Device-Managed Memory (Auto-freed)
```c
/* Allocate zeroed memory (freed at device removal) */
void *mem = devm_kzalloc(&pdev->dev, size, GFP_KERNEL);

/* Map I/O memory (unmapped at device removal) */
void __iomem *base = devm_ioremap(&pdev->dev, phys_addr, size);

/* Request IRQ (freed at device removal) */
devm_request_irq(&pdev->dev, irq, handler, flags, name, dev_id);
```

### Sysfs Attributes
```c
/* Define read-only attribute */
static DEVICE_ATTR_RO(name);

/* Define write-only attribute */
static DEVICE_ATTR_WO(name);

/* Define read-write attribute */
static DEVICE_ATTR_RW(name);

/* Create attribute in sysfs */
device_create_file(&pdev->dev, &dev_attr_name);

/* Remove attribute from sysfs */
device_remove_file(&pdev->dev, &dev_attr_name);
```

### Logging/Debugging
```c
dev_err(&pdev->dev, "Error message\n");
dev_warn(&pdev->dev, "Warning message\n");
dev_info(&pdev->dev, "Info message\n");
dev_dbg(&pdev->dev, "Debug message (only if enabled)\n");
```

---

## Part 10: Raspberry Pi 3 A+ — Hardware Setup & Integration

### File Hierarchy on RPi3 A+
```
/boot/
├── bcm2837-rpi-3-a-plus.dtb         /* Main board device tree */
└── overlays/                         /* Additional device trees */
    ├── sensor.dtbo
    ├── i2c-rtc.dtbo
    └── uart0.dtbo

/lib/modules/$(uname -r)/kernel/
└── drivers/                          /* Compiled kernel modules */
    └── sensor_driver.ko
```

### GPIO Pin Mapping (RPi 3 A+ - 40 pin header)
```
GPIO 2  = I2C SDA (pin 3)
GPIO 3  = I2C SCL (pin 5)
GPIO 4  = GPCLK0 (pin 7)
GPIO 17 = GPIO (pin 11)
GPIO 27 = GPIO (pin 13)
GPIO 22 = GPIO (pin 15)
GPIO 23 = GPIO (pin 16)
GPIO 24 = GPIO (pin 18)
GPIO 10 = SPI MOSI (pin 19)
GPIO 9  = SPI MISO (pin 21)
GPIO 25 = GPIO (pin 22)
GPIO 11 = SPI SCLK (pin 23)
GPIO 8  = SPI CE0 (pin 24)
GPIO 7  = SPI CE1 (pin 26)
```

### Device Tree for RPi3 A+ Sensor
```dts
&i2c1 {
    status = "okay";
    clock-frequency = <100000>;
    
    sensor@48 {
        compatible = "temp-sensor,anis";
        reg = <0x48>;
        threshold = <75>;
    };
};

&spi0 {
    status = "okay";
    cs-gpios = <&gpio 8 1>, <&gpio 7 1>;
};
```

### Build & Install on RPi
```bash
# Build driver
make -C /lib/modules/$(uname -r)/build M=$(PWD) modules

# Install temporarily
sudo insmod sensor_driver.ko

# Check if loaded
lsmod | grep sensor_driver
dmesg | tail -20

# Permanent installation
sudo cp sensor_driver.ko /lib/modules/$(uname -r)/kernel/drivers/
sudo depmod
```

### Device Tree Overlay Management
```bash
# Compile overlay
dtc -@ -I dts -O dtb -o sensor.dtbo sensor-overlay.dts

# Copy to boot
sudo cp sensor.dtbo /boot/overlays/

# Enable in config
sudo nano /boot/config.txt
# Add: dtoverlay=sensor

# Reboot to apply
sudo reboot
```

---

## Part 11: BeagleBone Black — Hardware Setup & Integration

### File Hierarchy on BeagleBone Black
```
/boot/
├── am335x-boneblack.dtb             /* AM335x SoC device tree */
└── overlays/
    ├── sensor-overlay.dtbo
    ├── pwm-overlay.dtbo
    └── uart5.dtbo

/lib/modules/$(uname -r)/kernel/
└── drivers/
    └── sensor_driver.ko

/sys/devices/platform/ocp/           /* On-chip peripherals */
├── 44e07000.gpio/
├── 44e0b000.i2c/
└── 48030000.spi/
```

### GPIO Pin Mapping (BeagleBone Black Headers)

**P9 Header (Most commonly used):**
```
P9_1  = GND
P9_11 = UART4_RX (GPIO0_30)
P9_12 = GPIO1_28
P9_13 = UART4_TX (GPIO0_31)
P9_14 = PWM1A (GPIO1_18)
P9_15 = GPIO1_16
P9_16 = PWM1B (GPIO1_19)
P9_17 = I2C1_SCL
P9_18 = I2C1_SDA
P9_21 = UART2_TX (GPIO0_3)
P9_22 = UART2_RX (GPIO0_2)
P9_23 = GPIO1_17
P9_24 = UART1_TX (GPIO0_15)
P9_26 = UART1_RX (GPIO0_14)
```

### Device Tree Overlay for BeagleBone
```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "ti,beaglebone", "ti,beaglebone-black";

    fragment@0 {
        target = <&i2c1>;
        __overlay__ {
            status = "okay";
            
            sensor@48 {
                compatible = "temp-sensor,anis";
                reg = <0x48>;
                threshold = <75>;
            };
        };
    };
};
```

### Build & Install on BeagleBone Black
```bash
# SSH into board
ssh debian@192.168.7.2         # USB Ethernet
# or
ssh debian@beaglebone.local

# Check kernel version
uname -r

# Build driver
make -C /lib/modules/$(uname -r)/build M=$(PWD) modules

# Install
sudo insmod sensor_driver.ko

# Permanent installation
sudo cp sensor_driver.ko /lib/modules/$(uname -r)/kernel/drivers/
sudo depmod

# Auto-load on boot
sudo nano /etc/modules
# Add: sensor_driver
```

### Device Tree Overlay on BeagleBone
```bash
# Compile overlay
dtc -@ -I dts -O dtb -o sensor-overlay.dtbo sensor-overlay.dts

# Copy to boot
sudo cp sensor-overlay.dtbo /boot/dtbs/overlays/

# Enable overlay
sudo nano /boot/uEnv.txt

# Add line:
# uboot_overlay_addr0=/boot/dtbs/overlays/sensor-overlay.dtbo

# Verify overlay loaded
cat /proc/device-tree/chosen/overlays

# Reboot
sudo reboot
```

### BeagleBone vs RPi Comparison

| Feature | BeagleBone Black | Raspberry Pi 3 A+ |
|---------|------------------|-------------------|
| **SoC** | TI AM335x (1 GHz) | Broadcom BCM2837 (1.4 GHz) |
| **RAM** | 512 MB DDR3 | 512 MB LPDDR2 |
| **GPIO** | 69 GPIO (4 banks) | 54 GPIO (1 bank) |
| **I2C** | I2C0, I2C1, I2C2 | I2C0, I2C1 |
| **SPI** | SPI0, SPI1 | SPI0 only |
| **UART** | 6 UARTs | 2 UARTs |
| **PWM** | 8 channels | Limited |
| **Headers** | P8 (26), P9 (46) | 40-pin header |
| **Kernel** | TI vendor kernel | Mainline Linux |
| **Boot** | U-Boot (uEnv.txt) | U-Boot (config.txt) |
| **Overlays** | U-Boot slots 0-7 | config.txt entries |

---

## Part 12: Testing & Verification — Systematic Validation Procedures

### Pre-Installation Testing
```bash
# 1. Verify device tree syntax
dtc -I dts -O null sensor-overlay.dts

# 2. Verify driver compiles
make clean
make

# 3. Check for compilation errors
dmesg | grep -i error
```

### Post-Installation Testing
```bash
# 4. Load driver and check messages
sudo insmod sensor_driver.ko
dmesg | tail -20           # Should show "Probe: ..."

# 5. Verify device registered
ls /sys/bus/platform/devices/ | grep sensor

# 6. Check device tree content
dtc -I fs /proc/device-tree/ | grep -A5 sensor

# 7. Access sysfs attributes
cat /sys/bus/platform/devices/sensor@0/temp
echo 0x42 > /sys/bus/platform/devices/sensor@0/control

# 8. Monitor in real-time
watch -n1 "cat /sys/bus/platform/devices/sensor@0/temp"

# 9. Unload and check cleanup
sudo rmmod sensor_driver
dmesg | tail -5             # Should show "Remove: ..."

# 10. Check for memory leaks
cat /proc/meminfo           # Use dmesg for kmemleak if enabled
```

---

## Part 13: Common Issues & Troubleshooting — Diagnostic Solutions

| Issue | Root Cause | Solution |
|-------|-----------|----------|
| **Probe never called** | compatible mismatch | Check: `cat /proc/device-tree/*/compatible` |
| **Module won't compile** | Missing headers | Install: `linux-headers-$(uname -r)` |
| **MODULE_DEVICE_TABLE error** | Missing macro | Add: `MODULE_DEVICE_TABLE(of, sensor_match)` |
| **Memory not mapped** | ioremap failed | Check `dmesg` for error, verify address in DT |
| **Sysfs not created** | device_create_file failed | Ensure DEVICE_ATTR_RO() defined before probe |
| **Device not found** | Overlay not loaded | Verify: `/boot/config.txt` has `dtoverlay=` |
| **GPIO access fails** | Wrong GPIO name in DT | Match name exactly: `gpios` vs `sensor-gpios` |
| **Permission denied** | Non-root execution | Use: `sudo cat /sys/.../temp` |
| **Driver load hangs** | Infinite loop in probe | Add timeouts, check for deadlocks |
| **No kernel messages** | printk disabled | Use: `dmesg` to retrieve after loading |

---

## Part 14: Quick Reference — Implementation Templates

### Minimal Driver Template
```c
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

static int my_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Probe called\n");
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Remove called\n");
    return 0;
}

static const struct of_device_id my_match[] = {
    { .compatible = "vendor,device" },
    { }
};
MODULE_DEVICE_TABLE(of, my_match);

static struct platform_driver my_driver = {
    .driver = {
        .name = "my_driver",
        .of_match_table = my_match,
    },
    .probe = my_probe,
    .remove = my_remove,
};

module_platform_driver(my_driver);
```

### Device Tree Template
```dts
/dts-v1/;
/plugin/;

/ {
    compatible = "vendor,board";
    
    fragment@0 {
        target = <&{/}>;
        __overlay__ {
            mydevice@0 {
                compatible = "vendor,device";
                reg = <0x50000000 0x1000>;
                interrupts = <73 4>;
                gpios = <&gpio 17 0>;
                custom-property = <42>;
                status = "okay";
            };
        };
    };
};
```

### Makefile Template
```makefile
obj-m += sensor_driver.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
```

---

## Part 15: Key Takeaways — Essential Concepts Summary

✅ **Device Tree** — Declarative hardware specification describing system devices

✅ **Platform Driver** — Software component providing operational hardware control

✅ **of_device_id Table** — Kernel mechanism for device tree node to driver matching

✅ **compatible Property** — String identifier for device-driver matching (case-sensitive)

✅ **Approach 1 (platform_get_*)** — Kernel-provided access for standard properties

✅ **Approach 2 (of_property_*)** — Flexible access for custom driver-specific properties

✅ **Device-Managed Resources (devm_*)** — Automatic cleanup preventing resource leaks

✅ **platform_set_drvdata()** — Device-to-driver private data association

✅ **Sysfs Interface** — User-space application-to-driver communication mechanism

✅ **Diagnostic Methods** — `dmesg` and kernel logging for development and troubleshooting

### Foundational Principle

The device tree and platform driver form a complementary pair: the device tree declares what hardware exists, while the platform driver specifies how the kernel should operate that hardware. Successful implementation requires precise alignment between device tree property specifications and driver implementation details.

### Architectural Workflow

```
Device Tree Node          Platform Driver
───────────────────       ──────────────────
compatible="sensor"  →    of_match_table: "sensor"
reg = <0x50000000>  →    probe() gets called
threshold = <75>    →    Driver initializes hardware
```

### Minimal Platform Driver

```c
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");

static int sensor_probe(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Device found and initialized\n");
    return 0;
}

static int sensor_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Device removed\n");
    return 0;
}

static const struct of_device_id sensor_match[] = {
    { .compatible = "myvendor,mysensor" },
    { }  /* MUST END WITH EMPTY ENTRY */
};
MODULE_DEVICE_TABLE(of, sensor_match);

static struct platform_driver sensor_driver = {
    .probe = sensor_probe,
    .remove = sensor_remove,
    .driver = {
        .name = "sensor_driver",
        .of_match_table = sensor_match,
    },
};

module_platform_driver(sensor_driver);
```

**CRITICAL**: 
- `compatible` string MUST match exactly in device tree
- Match table MUST end with empty `{ }`
- `MODULE_DEVICE_TABLE(of, sensor_match)` is MANDATORY

---

## Part 16: Accessing Resources — Property Resolution Mechanisms

### Device Tree Properties
```dts
sensor@0 {
    compatible = "myvendor,temp-sensor";
    reg = <0x50000000 0x1000>;        /* Memory address, size */
    interrupts = <73 4>;              /* IRQ number, flags */
    sensor-gpios = <&gpio0 2 0>;      /* GPIO controller, pin, flags */
    threshold = <75>;                 /* Custom property */
    label = "temp-sensor-01";         /* String property */
    enabled;                          /* Boolean property */
};
```

### Reading in Driver

```c
static int sensor_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    
    /* Get memory resource */
    struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) return -ENOENT;
    void __iomem *base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    
    /* Get IRQ */
    int irq = platform_get_irq(pdev, 0);
    
    /* Get GPIO */
    struct gpio_desc *gpio = devm_gpiod_get(&pdev->dev, "sensor", GPIOD_OUT_LOW);
    if (IS_ERR(gpio)) return PTR_ERR(gpio);
    
    /* Get custom properties */
    u32 threshold;
    of_property_read_u32(np, "threshold", &threshold);
    
    const char *label;
    of_property_read_string(np, "label", &label);
    
    bool enabled = of_property_read_bool(np, "enabled");
    
    dev_info(&pdev->dev, "Sensor: %s, threshold: %u\n", label, threshold);
    return 0;
}
```

---

## Part 17: Complete Working Driver — Fully Integrated Implementation

### Device Tree
```dts
sensor@0 {
    compatible = "demo,temp-sensor";
    reg = <0x50000000 0x1000>;
    interrupts = <73 4>;
    sensor-gpios = <&gpio0 2 0>;
    threshold = <75>;
    label = "temperature-sensor";
    status = "okay";
};
```

### Full Driver Code
```c
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>

MODULE_LICENSE("GPL");

struct sensor_dev {
    void __iomem *base;
    struct gpio_desc *gpio;
    int irq;
};

static ssize_t temp_show(struct device *dev, 
                         struct device_attribute *attr, char *buf)
{
    struct platform_device *pdev = to_platform_device(dev);
    struct sensor_dev *sensor = platform_get_drvdata(pdev);
    u32 temp = ioread32(sensor->base);
    return sprintf(buf, "%u\n", temp);
}
static DEVICE_ATTR_RO(temp);

static irqreturn_t sensor_irq(int irq, void *dev_id)
{
    dev_info(dev_id, "IRQ triggered\n");
    return IRQ_HANDLED;
}

static int sensor_probe(struct platform_device *pdev)
{
    struct device_node *np = pdev->dev.of_node;
    struct sensor_dev *sensor;
    struct resource *res;
    
    sensor = devm_kzalloc(&pdev->dev, sizeof(*sensor), GFP_KERNEL);
    if (!sensor) return -ENOMEM;
    
    /* Map memory */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) return -ENOENT;
    sensor->base = devm_ioremap(&pdev->dev, res->start, resource_size(res));
    if (!sensor->base) return -ENOMEM;
    
    /* Get IRQ */
    sensor->irq = platform_get_irq(pdev, 0);
    if (sensor->irq >= 0)
        devm_request_irq(&pdev->dev, sensor->irq, sensor_irq, 0, "sensor", pdev);
    
    /* Get GPIO */
    sensor->gpio = devm_gpiod_get(&pdev->dev, "sensor", GPIOD_IN);
    if (IS_ERR(sensor->gpio)) return PTR_ERR(sensor->gpio);
    
    /* Create sysfs attribute */
    device_create_file(&pdev->dev, &dev_attr_temp);
    platform_set_drvdata(pdev, sensor);
    
    dev_info(&pdev->dev, "Sensor initialized\n");
    return 0;
}

static int sensor_remove(struct platform_device *pdev)
{
    device_remove_file(&pdev->dev, &dev_attr_temp);
    return 0;
}

static const struct of_device_id sensor_match[] = {
    { .compatible = "demo,temp-sensor" },
    { }
};
MODULE_DEVICE_TABLE(of, sensor_match);

static struct platform_driver sensor_driver = {
    .probe = sensor_probe,
    .remove = sensor_remove,
    .driver = {
        .name = "temp_sensor",
        .of_match_table = sensor_match,
    },
};

module_platform_driver(sensor_driver);
```

---

## Part 18: Struct Definitions & API Reference — Complete Technical Reference

### Struct: platform_driver
```c
struct platform_driver {
    int (*probe)(struct platform_device *);     /* Called when device found */
    int (*remove)(struct platform_device *);    /* Called on device removal */
    void (*shutdown)(struct platform_device *); /* Called on system shutdown */
    int (*suspend)(struct platform_device *, pm_message_t state);     /* Suspend */
    int (*resume)(struct platform_device *);    /* Resume after suspend */
    struct device_driver driver;                /* Core driver structure */
};

/* Embedded driver structure contains: */
struct device_driver {
    const char *name;                           /* Driver name */
    struct bus_type *bus;                       /* Bus type (platform bus) */
    struct module *owner;                       /* Module owner */
    const struct of_device_id *of_match_table;  /* Device tree match table */
    int (*probe)(struct device *dev);           /* Generic probe */
    int (*remove)(struct device *dev);          /* Generic remove */
};
```

### Struct: platform_device
```c
struct platform_device {
    const char *name;                           /* Device name */
    int id;                                     /* Device ID (or -1 for auto) */
    struct device dev;                          /* Embedded device structure */
    u64 platform_dma_mask;                      /* DMA mask */
    struct resource *resource;                  /* Array of resources */
    unsigned int num_resources;                 /* Number of resources */
    struct platform_device_id *id_entry;       /* Matched ID entry */
};

/* Embedded device structure contains: */
struct device {
    struct device *parent;                      /* Parent device */
    struct device_node *of_node;                /* Device tree node */
    const char *init_name;                      /* Initial name */
    struct kobject kobj;                        /* Sysfs entry */
    const struct device_type *type;
    spinlock_t devres_lock;                     /* Devres lock */
    struct list_head devres_head;               /* Devres resources */
};
```

### Struct: resource
```c
struct resource {
    resource_size_t start;                      /* Start address */
    resource_size_t end;                        /* End address */
    const char *name;                           /* Resource name */
    unsigned long flags;                        /* Type flags */
    struct resource *parent, *sibling, *child;  /* Resource tree */
};

/* Resource flags: */
#define IORESOURCE_MEM      0x00000200           /* Memory resource */
#define IORESOURCE_IRQ      0x00000400           /* IRQ resource */
#define IORESOURCE_DMA      0x00000800           /* DMA resource */
#define IORESOURCE_IO       0x00000100           /* I/O port resource */
```

### Struct: device_node (Device Tree Node)
```c
struct device_node {
    const char *name;                           /* Node name */
    const char *type;                           /* Node type */
    phandle phandle;                            /* phandle value */
    const char *full_name;                      /* Full path name */
    struct property *properties;                /* Linked list of properties */
    struct device_node *parent;                 /* Parent node */
    struct device_node *child;                  /* First child */
    struct device_node *sibling;                /* Next sibling */
    struct kref kref;                           /* Reference count */
    unsigned long _flags;                       /* Flags */
    void *data;                                 /* Driver-specific data */
};

struct property {
    char *name;                                 /* Property name */
    int length;                                 /* Data length in bytes */
    void *value;                                /* Property data */
    struct property *next;                      /* Next property */
};
```

### Struct: of_device_id (Device Tree Matching)
```c
struct of_device_id {
    char name[32];                              /* Device name */
    char type[32];                              /* Device type */
    char compatible[128];                       /* Compatible string */
    const void *data;                           /* Driver-specific data */
};

/* Matching order: */
/* 1. Match by compatible string (PREFERRED) */
/* 2. Match by type string */
/* 3. Match by name string */
```

### Struct: gpio_desc (GPIO Descriptor)
```c
struct gpio_desc {
    struct gpio_chip *gdev;                     /* GPIO chip device */
    unsigned long flags;                        /* Flag bitmask */
    const char *label;                          /* Consumer label */
    const char *name;                           /* GPIO name from DT */
};

/* GPIO flags: */
#define GPIOD_OUT_LOW       GPIOD_OUT | GPIOD_OUT_LOW_BIT  /* Output low */
#define GPIOD_OUT_HIGH      GPIOD_OUT | GPIOD_OUT_HIGH_BIT /* Output high */
#define GPIOD_IN            0                              /* Input */
#define GPIOD_OUT           BIT(0)                         /* Output direction */
#define GPIOD_ACTIVE_LOW    BIT(4)                         /* Active low */
```

### Struct: clk (Clock Structure)
```c
struct clk {
    const char *name;                           /* Clock name */
    const struct clk_ops *ops;                  /* Clock operations */
    struct clk *parent;                         /* Parent clock */
    unsigned long rate;                         /* Current rate (Hz) */
    struct hlist_head children;                 /* Child clocks */
    struct hlist_node child_node;
};

/* Clock operations: */
struct clk_ops {
    int (*enable)(struct clk_hw *hw);
    void (*disable)(struct clk_hw *hw);
    int (*is_enabled)(struct clk_hw *hw);
    unsigned long (*recalc_rate)(struct clk_hw *hw, unsigned long parent_rate);
    int (*set_rate)(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate);
    int (*round_rate)(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate);
};
```

### Struct: regulator_dev (Voltage Regulator)
```c
struct regulator_dev {
    const struct regulator_desc *desc;          /* Regulator description */
    int open_count;                             /* Number of consumers */
    unsigned int use_count;                     /* Use counter */
    unsigned int deferred_disables;             /* Deferred disable count */
    struct regulation_constraints *constraints; /* Operating constraints */
    struct notifier_block *rdev_entry;
    int exclusive;                              /* Exclusive access flag */
};

struct regulation_constraints {
    struct voltage_range_step_info voltage_step_info;
    int min_uV, max_uV;                         /* Voltage limits (µV) */
    int min_uA, max_uA;                         /* Current limits (µA) */
    int microvolt_offset;                       /* Voltage offset */
    unsigned int valid_modes_mask;              /* Valid modes */
    unsigned int valid_ops_mask;                /* Valid operations */
};
```

---

## API Reference

### Device Tree Reading Functions
```c
/* Read u32 property */
int of_property_read_u32(const struct device_node *np,
                         const char *propname, u32 *out_value);

/* Read u32 from array at index */
int of_property_read_u32_index(const struct device_node *np,
                               const char *propname, u32 index, u32 *out_value);

/* Read string property */
int of_property_read_string(const struct device_node *np,
                            const char *propname, const char **out_string);

/* Read string array at index */
int of_property_read_string_index(const struct device_node *np,
                                  const char *propname, int index, const char **out_string);

/* Check if boolean property exists */
bool of_property_read_bool(const struct device_node *np, const char *propname);

/* Get property pointer */
struct property *of_find_property(const struct device_node *np,
                                  const char *name, int *lenp);

/* Get child node by name */
struct device_node *of_get_child_by_name(const struct device_node *node,
                                         const char *name);

/* Get next child node */
struct device_node *of_get_next_child(const struct device_node *node,
                                      struct device_node *prev);
```

### Platform Device Functions
```c
/* Get resource by type */
struct resource *platform_get_resource(struct platform_device *dev,
                                       unsigned int type, unsigned int num);

/* Get IRQ number */
int platform_get_irq(struct platform_device *pdev, unsigned int num);

/* Set private driver data */
int platform_set_drvdata(struct platform_device *pdev, void *data);

/* Get private driver data */
void *platform_get_drvdata(const struct platform_device *pdev);

/* Register platform device */
int platform_device_register(struct platform_device *pdev);

/* Unregister platform device */
void platform_device_unregister(struct platform_device *pdev);
```

### Memory I/O Functions
```c
/* Memory-mapped I/O reads */
u8 ioread8(const volatile void __iomem *addr);
u16 ioread16(const volatile void __iomem *addr);
u32 ioread32(const volatile void __iomem *addr);
u64 ioread64(const volatile void __iomem *addr);

/* Memory-mapped I/O writes */
void iowrite8(u8 value, volatile void __iomem *addr);
void iowrite16(u16 value, volatile void __iomem *addr);
void iowrite32(u32 value, volatile void __iomem *addr);
void iowrite64(u64 value, volatile void __iomem *addr);

/* Device-managed I/O mapping */
void __iomem *devm_ioremap(struct device *dev, resource_size_t offset,
                           unsigned long size);
void __iomem *devm_ioremap_resource(struct device *dev, struct resource *res);
```

### GPIO Functions
```c
/* Get GPIO descriptor from DT */
struct gpio_desc *devm_gpiod_get(struct device *dev, const char *con_id,
                                 enum gpiod_flags flags);

/* Get GPIO by index from DT */
struct gpio_desc *devm_gpiod_get_index(struct device *dev,
                                       const char *con_id, unsigned int idx,
                                       enum gpiod_flags flags);

/* Set GPIO value */
void gpiod_set_value(struct gpio_desc *desc, int value);
void gpiod_set_value_cansleep(struct gpio_desc *desc, int value);

/* Get GPIO value */
int gpiod_get_value(const struct gpio_desc *desc);
int gpiod_get_value_cansleep(const struct gpio_desc *desc);

/* Set GPIO direction */
int gpiod_direction_input(struct gpio_desc *desc);
int gpiod_direction_output(struct gpio_desc *desc, int value);
```

### Device-Managed (devm_*) Functions
```c
/* Device-managed memory allocation */
void *devm_kzalloc(struct device *dev, size_t size, gfp_t gfp);
void *devm_kmalloc(struct device *dev, size_t size, gfp_t gfp);

/* Device-managed IRQ request */
int devm_request_irq(struct device *dev, unsigned int irq,
                     irq_handler_t handler, unsigned long irqflags,
                     const char *devname, void *dev_id);

/* Device-managed clock get */
struct clk *devm_clk_get(struct device *dev, const char *id);
int devm_clk_enable(struct device *dev, struct clk *clk);

/* Device-managed regulator get */
struct regulator *devm_regulator_get(struct device *dev, const char *id);
int regulator_enable(struct regulator *regulator);
int regulator_disable(struct regulator *regulator);

/* Generic device-managed resource tracking */
void *__devm_alloc_pages(struct device *dev, gfp_t gfp_mask, unsigned int order);
void devm_kfree(struct device *dev, void *p);
```

### Sysfs Attribute Macros & Functions
```c
/* Define read-only attribute */
#define DEVICE_ATTR_RO(name) \
    struct device_attribute dev_attr_##name = __ATTR_RO(name)

/* Define write-only attribute */
#define DEVICE_ATTR_WO(name) \
    struct device_attribute dev_attr_##name = __ATTR_WO(name)

/* Define read-write attribute */
#define DEVICE_ATTR_RW(name) \
    struct device_attribute dev_attr_##name = __ATTR_RW(name)

/* Manual attribute definition */
struct device_attribute {
    struct attribute attr;
    ssize_t (*show)(struct device *, struct device_attribute *, char *);
    ssize_t (*store)(struct device *, struct device_attribute *,
                     const char *, size_t);
};

/* Create/remove attributes */
int device_create_file(struct device *device, const struct device_attribute *entry);
void device_remove_file(struct device *device, const struct device_attribute *entry);
int device_create_group(struct device *dev, const struct attribute_group *grp);
void device_remove_group(struct device *dev, const struct attribute_group *grp);
```

### IRQ Handler Functions
```c
/* IRQ handler return values */
typedef irqreturn_t (*irq_handler_t)(int, void *);

enum irqreturn {
    IRQ_NONE = (0),                             /* Not our IRQ */
    IRQ_HANDLED = (1),                          /* IRQ handled */
    IRQ_WAKE_THREAD = (2),                      /* Wake handler thread */
};

/* Disable/enable interrupt */
void disable_irq(unsigned int irq);
void enable_irq(unsigned int irq);
void disable_irq_nosync(unsigned int irq);
```

### Printk/Logging Functions
```c
/* Development macros (no output in production) */
dev_dbg(&pdev->dev, "message %d\n", value);
dev_info(&pdev->dev, "message %d\n", value);
dev_warn(&pdev->dev, "message %d\n", value);
dev_err(&pdev->dev, "message %d\n", value);

/* Levels: dbg, info, notice, warn, crit, alert, emerg */
```