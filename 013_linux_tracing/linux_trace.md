# Linux Tracing

## ftrace available

```bash
mount | grep tracefs
```

- output: `none on /sys/kernel/tracing type tracefs`

## Steps

```bash
cd /sys/kernel/tracing
cat current_tracer           	# Check what's running
echo function > current_tracer  # Turn on tracing
cat trace                    	# View the output
echo nop > current_tracer    	# Turn off tracing
```

## Available Tracers

```bash
cat available_tracers
```

options:

- **function** - 		Traces kernel function calls
- **function_graph** - 	Shows functions with their call hierarchy and timing
- **nop** - 			Turns tracing off

## Function Tracer

See what functions the kernel is calling:

```bash
echo function > current_tracer
cat trace | head -30
echo nop > current_tracer
```

Output shows the process name, PID, CPU, timestamp, and the function being called.

## Function Graph Tracer

See functions nested with timing info:

```bash
echo function_graph > current_tracer
cat trace | head -30
```

Shows which functions call which other functions and how long each takes.

## Trace Specific Functions

```bash
cat available_filter_functions | wc -l

grep kmalloc available_filter_functions
grep ext4 available_filter_functions

echo ext4_* > set_ftrace_filter
echo vfp_* >> set_ftrace_filter
echo > trace
cat trace
```
## Exclude Functions

```bash
echo "irq_*" > set_ftrace_notrace
```

## Trace Process

```bash
ps aux

pid=$$

echo $pid > set_ftrace_pid
echo 1 > tracing_on

# sleep 1

echo 0 > tracing_on
cat trace
```

## Turn Tracing On/Off On-Demand

```bash
echo 1 > tracing_on    # Start

echo 0 > tracing_on    # Stop
cat trace
```

## Clear the Buffer

```bash
echo > trace
```

## Max Graph Depth

Control function_graph depth:

```bash
echo 1 > max_graph_depth
echo 2 > max_graph_depth
```