import matplotlib.pyplot as plt

# Parse the output files
def parse_file(filename):
    ticks = []
    procs = []
    with open(filename, 'r') as f:
        for line in f:
            t, p = line.strip().split(', ')
            ticks.append(int(t.split(': ')[1]))
            procs.append(int(p.split(': ')[1]))
    return ticks, procs

ticks_original, procs_original = parse_file('originalScheduler.txt')
ticks_mlfq, procs_mlfq = parse_file('mlfqScheduler.txt')

# Plotting for original scheduler
plt.figure()
for p in set(procs_original):
    plt.plot([ticks_original[i] for i in range(len(procs_original)) if procs_original[i] == p], label=f'Process {p}')
plt.legend()
plt.title('Original Scheduler')
plt.xlabel('Time')
plt.ylabel('Total ticks')
plt.savefig('originalScheduler.png')
plt.show()

# Plotting for MLFQ scheduler
plt.figure()
for p in set(procs_mlfq):
    plt.plot([ticks_mlfq[i] for i in range(len(procs_mlfq)) if procs_mlfq[i] == p], label=f'Process {p}')
plt.legend()
plt.title('MLFQ Scheduler')
plt.xlabel('Time')
plt.ylabel('Total ticks')
plt.savefig('mlfqScheduler.png')
plt.show()
