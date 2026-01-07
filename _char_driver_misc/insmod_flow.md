```
User Space            Kernel Space
-----------          --------------
insmod
  ↓
syscall (init_module / finit_module)
                      ↓
                Kernel Module Loader
                      ↓
                module_init()
```

---

### Step 1: Shell executes insmod
```text
execve("/sbin/insmod", ["insmod", "mymod.ko"])
```
- `insmod` is a normal user-space program

---

### Step 2: insmod opens the module file
```text
open("mymod.ko", O_RDONLY)
```
- `.ko` is an ELF object
- Contains `.text`, `.data`, `.modinfo`, symbols

---

### Step 3: insmod reads the module into memory
```text
read(fd, buffer, size)
```

---

### Step 4: User → Kernel transition

```c
init_module(void *image, size_t len, char *params);   // old
finit_module(int fd, char *params, int flags);        // modern
```

---

### Step 5: Kernel receives module image
```
sys_init_module()
sys_finit_module()
```

---

### Step 6: ELF validation
Kernel checks:
- ELF magic
- Section headers
 .ko file
 ├── ELF header
 ├── Section Header Table
 │     ├── .text
 │     ├── .data
 │     ├── .bss
 │     ├── .rela.text
 │     └── .modinfo
 └── Section contents

- Relocation info

---

### Step 7: Version Magic check
Kernel compares:
- Kernel version
- SMP / PREEMPT
- MODVERSIONS

---

### Step 8: Allocate kernel memory for module
Kernel allocates memory for:
- `.text`
- `.data`
- `.bss`

---

### Step 9: Symbol resolution
Kernel resolves undefined symbols against:
- vmlinux
- already-loaded modules

---

### Step 10: Apply relocations
- Function addresses fixed
- Global references patched
- Architecture-specific handling

---

### Step 11: Register module metadata
```
/proc/modules
/sys/module/<module_name>
```

---

### Step 12: Call module_init()
```c
module_init(my_init);
```

---

### Step 13: Handle module_init() return value

- `return 0` → module stays loaded
- `return < 0` → module load aborted
  - Memory freed
  - `.exit()` NOT called

---

### Step 14: Return to user space
Syscall returns:
```text
init_module(...) = 0
```

`insmod` exits silently on success.

---

## Failure Cases

| Failure Cause 	| Symptom 				|
|-------------------|-----------------------|
| ELF invalid   	| Exec format error 	|
| vermagic mismatch | Invalid module format |
| Missing symbol 	| Unknown symbol 		|
| init failure 		| insmod fails 			|
