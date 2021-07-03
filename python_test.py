import roulette
import os
import psutil
process = psutil.Process(os.getpid())
print(f'pid = {os.getpid()}')
start_rec = str(process.memory_info()) 

print(dir(roulette))

for i in range(100):
    print(f'#{i}:{roulette.random_range(0, 100)}')

value_list = [('boy', 1),('hell', 2.5),('fuckton', 2),('tick', 1), ('smelly', 1)]

randomizer = roulette.roulette(value_list)

randomizer.remove('tick')
randomizer.update('boy', 4.5)

counter = {}

for i in range(10000):

    value = randomizer.roll()
    # print(f'{i}#: {value}')

    if value not in counter:
        counter[value] = 1
    else:
        counter[value] += 1

print(f'randomizer has {len(randomizer)} elements')

for key,value in counter.items():
    print(f'{key}:{value}')

iter = iter(randomizer)

print(f'iter type = {type(iter)}')

for val, chance in randomizer:
    print(f'{val} has a chance of {chance}')

mid_rec = str(process.memory_info())

for i in range(1000):
    temp = roulette.roulette(value_list)
    del temp

print(f'before any allocation memory usage: {start_rec}')  # in bytes 
print(f'before allocation test memory usage: {mid_rec}')  # in bytes
print(f'after allocation test memory usage: {process.memory_info()}')  # in bytes
